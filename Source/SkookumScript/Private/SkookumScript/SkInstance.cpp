// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for simplest type of object in language - instance of a
//             class without data members
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInstance.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInstance.inl>
#endif

#if (SKOOKUM & SK_DEBUG)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkInteger.hpp>

//=======================================================================================
// Data Definitions
//=======================================================================================

AObjReusePool<SkInstance> SkInstance::ms_pool;

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Returns true if class type, false if not
// Modifiers:   virtual
// Author(s):   Conan Reis
SkClass * SkInstance::get_key_class() const
  {
  return is_metaclass()
    ? static_cast<const SkMetaClass *>(this)->get_class_info()
    : m_class_p;
  }

//---------------------------------------------------------------------------------------
void SkInstance::method_call(
  const ASymbol & method_name,
  SkInstance **   args_pp,
  uint32_t        arg_count,
  SkInstance **   result_pp, // = nullptr
  SkInvokedBase * caller_p   // = nullptr
  )
  {
  SkInstance * receiver_p = this;

  // Look for instance method - asserts if not found.
  // $Revisit - CReis [Index Look-up] This look-up should eventually be replaced with an index into a method table.
  SkMethodBase * method_p = m_class_p->find_method_inherited_receiver(method_name, &receiver_p, caller_p);

  #if (SKOOKUM & SK_DEBUG)
    if (method_p == nullptr)
      {
      if (result_pp)
        {
        // Wanted a return so return a nil so there is something
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return;
      }
  #endif

  SkInvokedMethod imethod(caller_p, receiver_p, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  SKDEBUG_ICALL_SET_INTERNAL(&imethod);
  SKDEBUG_HOOK_SCRIPT_ENTRY(method_name);

  // Fill invoked method's argument list
  imethod.data_append_args(args_pp, arg_count, method_p->get_params());

  // Call method
  method_p->invoke(&imethod, caller_p, result_pp);

  SKDEBUG_HOOK_SCRIPT_EXIT();
  }

//---------------------------------------------------------------------------------------
bool SkInstance::method_query(
  const ASymbol & method_name,
  SkInstance *    arg_p,     // = nullptr
  SkInvokedBase * caller_p   // = nullptr
  )
  {
  SkInstance * bool_p = nullptr;

  method_call(method_name, &arg_p, arg_p ? 1u : 0u, &bool_p, caller_p);

  bool result = bool_p && bool_p->as<SkBoolean>();

  bool_p->dereference();

  return result;
  }

//---------------------------------------------------------------------------------------
void SkInstance::abort_coroutines_on_this(eSkNotify notify_caller)
  {
  SkMind::abort_all_coroutines_on_object(this, notify_caller);
  }

//---------------------------------------------------------------------------------------
void SkInstance::call_default_constructor()
  {
  SkMethodBase * method_p = m_class_p->find_instance_method_inherited(ASymbolX_ctor);

  if (method_p)
    {
    SkInvokedMethod imethod(nullptr, this, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

    SKDEBUG_ICALL_SET_INTERNAL(&imethod);
    SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_origin_default_ctor);
     
    // If the constructor expects arguments try to use defaults
    if (method_p->get_params().get_param_list().is_filled())
      {
      imethod.data_append_args(nullptr, 0u, method_p->get_params());
      }

    method_p->invoke(&imethod);

    // Return arguments are ignored

    SKDEBUG_HOOK_SCRIPT_EXIT();
    }
  }

//---------------------------------------------------------------------------------------
void SkInstance::call_destructor()
  {
  // Call its destructor - if it has one
  SkMethodBase * dtor_p = m_class_p->get_instance_destructor_inherited();

  if (dtor_p)
    {
    // $Revisit - CReis It would be nice to have the caller for this so that a
    // callstack could be constructed when desired.
    SkInvokedMethod imethod(nullptr, this, dtor_p, a_stack_allocate(dtor_p->get_invoked_data_array_size(), SkInstance*));

    SKDEBUG_ICALL_SET_INTERNAL(&imethod);

    // Call destructor - no arguments
    dtor_p->invoke(&imethod);
    }
  }

//---------------------------------------------------------------------------------------
SkInvokedCoroutine * SkInstance::coroutine_call(
  const ASymbol & coroutine_name,
  SkInstance **   args_pp,
  uint32_t        arg_count,
  bool            immediate,       // = true
  f32             update_interval, // = SkCall_interval_always
  SkInvokedBase * caller_p,        // = nullptr
  SkMind *        updater_p        // = nullptr
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find coroutine
  // $Revisit - CReis [Index Look-up] Could make alternate version that does index look-up
  SkCoroutineBase * coroutine_p = m_class_p->find_coroutine_inherited(coroutine_name);

  if (!coroutine_p)
    {
    SK_ERROR(
      a_str_format(
        "The coroutine '%s()' does not exist for %s.",
        coroutine_name.as_cstr_dbg(),
        m_class_p->get_scope_desc().as_cstr()),
      SkInstance);

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialize invoked coroutine
  SkInvokedCoroutine * icoroutine_p = SkInvokedCoroutine::pool_new(coroutine_p);

  icoroutine_p->reset(update_interval, caller_p, this, updater_p);

  SKDEBUG_ICALL_SET_INTERNAL(icoroutine_p);
  SKDEBUG_HOOK_SCRIPT_ENTRY(coroutine_name);

  // Fill invoked coroutine's argument list
  icoroutine_p->data_append_args(args_pp, arg_count, coroutine_p->get_params());


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call/register coroutine
  if (immediate)
    {
    if (icoroutine_p->on_update())
      {
      SKDEBUG_HOOK_SCRIPT_EXIT();
      return nullptr;
      }
    }
  else
    {
    // Append to coroutine update list
    icoroutine_p->m_mind_p->coroutine_track_init(icoroutine_p);
    }

  SKDEBUG_HOOK_SCRIPT_EXIT();
  return icoroutine_p;
  }

//---------------------------------------------------------------------------------------

SkInvokedBase * SkInstance::invoke(
  SkObjectBase * scope_p,
  SkInvokedBase * caller_p,
  SkInstance ** result_pp,
  const SkClosureInvokeInfo & invoke_info,
  const SkExpressionBase * invoking_expr_p) const
  {
  SK_ERRORX(a_str_format("Tried to invoke an SkInstance of type '%s' but it does not support invocation!", get_class()->get_name_cstr()));
  return nullptr;
  }

//---------------------------------------------------------------------------------------

void SkInstance::invoke_as_method(
  SkObjectBase * scope_p,
  SkInvokedBase * caller_p,
  SkInstance ** result_pp,
  const SkClosureInvokeInfo & invoke_info,
  const SkExpressionBase * invoking_expr_p) const
  {
  SK_ERRORX(a_str_format("Tried to invoke an SkInstance of type '%s' but it does not support invocation!", get_class()->get_name_cstr()));
  }

//---------------------------------------------------------------------------------------
// Look up the given variable by name
SkInstance * SkInstance::get_data_by_name(const ASymbol & name) const
  {
  uint32_t data_idx = 0;

  // First try instance raw data
  SkClass * data_owner_class_p = nullptr;
  if (m_class_p->get_instance_data_type_raw(name, &data_idx, &data_owner_class_p))
    {
    SkTypedNameRaw * typed_p = data_owner_class_p->get_instance_data_raw()[data_idx];
    void * obj_p = data_owner_class_p->get_raw_pointer(const_cast<SkInstance *>(this)); // Pointer to raw memory of the object containing the data member
    SK_ASSERTX(obj_p, a_str_format("Tried to access %s.%s but the instance is null!", m_class_p->get_name_cstr(), typed_p->get_name_cstr()));
    if (obj_p)
      {
      SkClassDescBase * data_type_p = typed_p->m_type_p; // Type of the data member itself
      return data_type_p->get_key_class()->new_instance_from_raw_data(obj_p, typed_p->m_raw_data_info, data_type_p);
      }

    // On error, return nil
    return SkBrain::ms_nil_p;
    }

  // Then class data
  SkClass * owner_class_p = nullptr;
  if (m_class_p->get_class_data_type(name, &data_idx, &owner_class_p))
    {
    return owner_class_p->get_class_data_value_by_idx(data_idx);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Look up the given variable by name and sets it to given value
bool SkInstance::set_data_by_name(const ASymbol & name, SkInstance * data_p)
  {
  uint32_t data_idx = 0;

  // Try instance raw data
  SkClass * data_owner_class_p = nullptr;
  if (m_class_p->get_instance_data_type_raw(name, &data_idx, &data_owner_class_p))
    {
    SkTypedNameRaw * typed_p = data_owner_class_p->get_instance_data_raw()[data_idx];
    void * obj_p = data_owner_class_p->get_raw_pointer(const_cast<SkInstance *>(this)); // Pointer to raw memory of the object containing the data member
    SK_ASSERTX(obj_p, a_str_format("Tried to access %s.%s but the instance is null!", m_class_p->get_name_cstr(), typed_p->get_name_cstr()));
    if (obj_p)
      {
      SkClassDescBase * data_type_p = typed_p->m_type_p; // Type of the data member itself
      data_type_p->get_key_class()->assign_raw_data(obj_p, typed_p->m_raw_data_info, data_type_p, data_p);
      return true;
      }

    return false;
    }

  // Then class data
  SkClass * owner_class_p = nullptr;
  if (m_class_p->get_class_data_type(name, &data_idx, &owner_class_p))
    {
    owner_class_p->set_class_data_value_by_idx(data_idx, data_p);
    return true;
    }

  return false;
  }

#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Returns a string representation of itself for debugging purposes
// Returns:    Debug string
// Modifiers:   virtual
// #See         as_code_append()
// Author(s):   Conan Reis
AString SkInstance::as_string_debug() const
  {
  if (this == SkBrain::ms_nil_p)
    {
    return AString("nil", 3u);
    }

  AString class_name(m_class_p->get_name_str_dbg());
  AString str(nullptr, class_name.get_length() + 12u, 0u);

  str.append('<');
  str.append(class_name);
  str.append("> instance");

  return str;
  }

#endif  // SK_AS_STRINGS


//---------------------------------------------------------------------------------------
// Appends a code/literal string representation of itself to str_p using native
// representation for simple types and calling as_code() script method for other types.
// Used in debugging when building callstack, etc.
//
// Params:  
//   str_p:      String to append to
//   code_flags: flags for building string - see eSkCodeFlag
//   caller_p:   Used for call stack info if .String() conversion method is called.
//   
// See:         as_code(), as_string_debug()
// Author(s):   Conan Reis
void SkInstance::as_code_append(
  AString *       str_p,
  uint32_t        code_flags, // = SkCodeFlag__none
  SkInvokedBase * caller_p    // = nullptr
  ) const
  {
  // $Note - CReis Best to keep this method non-virtual so the this==nullptr check works.
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for common object types
  uint32_t str_length = str_p->get_length();

  switch (m_class_p->get_name_id())
    {
    case ASymbolId_Boolean:
      if (as<SkBoolean>())
        {
        str_p->append("true", 4u);
        }
      else
        {
        str_p->append("false", 5u);
        }
      return;

    case ASymbolId_Class:
      {
      AString class_name(static_cast<const SkMetaClass *>(this)->get_class_info()->get_name_str_dbg());

      str_p->ensure_size(str_length + 2u + class_name.get_length());
      str_p->append('<');
      str_p->append(class_name);
      str_p->append('>');
      return;
      }

    case ASymbolId_Closure:
      str_p->append(static_cast<const SkClosure *>(this)->as_string());
      return;

    case ASymbolId_Integer:
      str_p->append(AString::ctor_int(as<SkInteger>()));
      return;

    case ASymbolId_None:
      str_p->append("nil", 3u);
      return;

    case ASymbolId_Real:
      str_p->append(AString::ctor_float(as<SkReal>()));
      return;

    case ASymbolId_String:
      {
      AString str(SkString::to_escape_string(
        as<SkString>(),
        (code_flags & SkCodeFlag_break_lines) != 0u));

      str_p->ensure_size(str_length + 2u + str.get_length());
      str_p->append('\"');
      str_p->append(str);
      str_p->append('\"');
      return;
      }

    case ASymbolId_Symbol:
      {
      AString str(as<SkSymbol>().as_str_dbg());

      str_p->ensure_size(str_length + 2u + str.get_length());
      str_p->append('\'');
      str_p->append(str);
      str_p->append('\'');
      return;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call as_code() script conversion method for more complex classes
  SkMethodBase *    as_str_method_p = m_class_p->find_instance_method_inherited(ASymbol_as_code);

  // If default `Object@as_code()` is used just call `String()` conversion method
  if (as_str_method_p && as_str_method_p->get_scope() == SkBrain::ms_object_class_p)
    {
    as_str_method_p = m_class_p->find_instance_method_inherited(ASymbol_String);
    }

  bool include_type = (code_flags & SkCodeFlag_include_type) != 0u;

  if (as_str_method_p && as_str_method_p->is_bound())
    {
    // $Revisit - CReis Ensure defaults used
    SkInvokedMethod istrmthd(caller_p, const_cast<SkInstance *>(this), as_str_method_p, a_stack_allocate(as_str_method_p->get_invoked_data_array_size(), SkInstance*));

    SKDEBUG_ICALL_SET_INTERNAL(&istrmthd);

    SkInstance * str_result_p;
    reference();  // Ensure that conversion method does not garbage collect this instance.
    as_str_method_p->invoke(&istrmthd, nullptr, &str_result_p);

    if (str_result_p->get_class() == SkBrain::ms_string_class_p)
      {
      str_p->append(str_result_p->as<SkString>());
      }
    else
      {
      str_p->append("???", 3u);
      }

    // Garbage collect String instance
    str_result_p->dereference();
    dereference_delay();
    }
  else
    {
    // If run on IDE it might not be bound
    include_type = true;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append optional class type
  if (include_type)
    {
    AString class_name(m_class_p->get_name_str_dbg());

    str_p->ensure_size_extra(class_name.get_length() + 11u);
    str_p->append('<');
    str_p->append(class_name);
    str_p->append("> instance");
    }
  }

//---------------------------------------------------------------------------------------
// Invokes this instance's String method and appends the result to the given string
void SkInstance::as_string_invoke_append(AString * str_p, SkInvokedBase * caller_p /*= nullptr*/) const
  {
  // Is it a String?
  if (m_class_p == SkString::get_class())
    {
    // Already a string, so just append it.
    str_p->append(as<SkString>());
    }
  else
    {
    // Not a string, so convert it then append it.

    // All objects are guaranteed to have a String() method since there is a Object@String() method.
    SkMethodBase * method_p = m_class_p->find_instance_method_inherited(ASymbol_String);

    // Call String() on the object to get its string representation.
    SkInvokedMethod imethod(caller_p, const_cast<SkInstance *>(this), method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));
    SKDEBUG_ICALL_SET_INTERNAL(&imethod);

    // Object string instance stored in str_result_p
    SkInstance * str_result_p = SkBrain::ms_nil_p;
    method_p->invoke(&imethod, nullptr, &str_result_p);

    // Append it
    str_p->append(str_result_p->as<SkString>());

    // Garbage collect object string result
    str_result_p->dereference();
    }
  }

#ifdef SK_INSTANCE_REF_DBG

SkClass * g_ref_class_dbg_p = nullptr;

//---------------------------------------------------------------------------------------
// Increments the reference count to this object.
// 
// See:       dereference(), dereference_delay()
// Author(s): Conan Reis
void SkInstance::reference() const
  {
  m_ref_count++;

  if (m_class_p == g_ref_class_dbg_p)
    {
    A_DPRINT("%s (0x%x id:%u) Refs: %u\n", g_ref_class_dbg_p->get_name_cstr(), this, m_ptr_id, m_ref_count);
    }
  }

//---------------------------------------------------------------------------------------
// Increments the reference count to this object by specified amount.
// 
// See:       dereference(), dereference_delay()
// Author(s): Conan Reis
void SkInstance::reference(uint32_t increment_by) const
  {
  m_ref_count += increment_by;

  if (m_class_p == g_ref_class_dbg_p)
    {
    A_DPRINT("%s (0x%x id:%u) Refs: %u\n", g_ref_class_dbg_p->get_name_cstr(), this, m_ptr_id, m_ref_count);
    }
  }

#endif

//---------------------------------------------------------------------------------------
// Registers/connects atomic classes, methods, coroutines, etc. with the parameters
// previously loaded during the preload phase.
// 
// Modifiers: static
// Author(s): Conan Reis
void SkInstance::initialize_post_load()
  {
  #ifdef SK_INSTANCE_REF_DBG
    g_ref_class_dbg_p = SkBrain::get_class("Enemy");
  #endif
  }

#ifdef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkInstances
AObjReusePool<SkInstance> & SkInstance::get_pool()
  {
  return ms_pool;
  }

#endif
