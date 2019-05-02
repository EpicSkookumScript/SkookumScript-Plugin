// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for info used to make a method call/invocation - i.e. method
//             identifier (name/index) and passed argument info.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <AgogCore/AStringRef.hpp>

//=======================================================================================
// SkMethodBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:         SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkMethodCallBase::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkMethodCallBase, 0u),
    (m_arguments.get_length() + m_return_args.get_length()) * sizeof(void *),
    m_arguments.track_memory(mem_stats_p) + m_return_args.track_memory(mem_stats_p));
  }

//---------------------------------------------------------------------------------------
// Evaluates invocation expression and returns the resulting instance if desired.
// Arg         receiver_p - - receiver instance of the call - also used for scope for
//             for default argument data/method/etc. look-ups.
// Arg         vtable_scope - denotes if the method is a instance or class method            
// Arg         default_call_scope_p - the scope to use if m_scope_p is null (class of the object that we are invoking the method on)           
// Arg         scope_p - data scope where call was made for non-default argument look-ups            
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        SkInvocation::invoke(), SkInvokeCascade::invoke()
// Modifiers:   virtual (overriding pure from SkInvokeBase)
// Author(s):   Conan Reis, Markus Breyer
A_FORCEINLINE_OPTIMIZED SkInvokedBase * SkMethodCallBase::invoke_call_internal(
  SkInstance *    receiver_p,
  eSkScope        vtable_scope,
  SkClass *       default_call_scope_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  // Look for method
  SkMethodBase * method_p = nullptr;
  SkClass * call_scope_p = m_scope_p ? m_scope_p : default_call_scope_p;
  int16_t vtable_index = m_data_idx;
  #if SKOOKUM & SK_DEBUG
    if (vtable_index == SkQualifier::ms_invalid_vtable_index)
      {
      method_p = (vtable_scope == SkScope_instance ? call_scope_p->find_instance_method_inherited(get_name()) : call_scope_p->find_class_method_inherited(get_name()));
      }
    else
  #endif
      {
      method_p = static_cast<SkMethodBase *>(call_scope_p->get_invokable_from_vtable(vtable_scope, vtable_index));
      }

  #if SKOOKUM & SK_DEBUG
    // If not found, might be due to recent live update and the vtable not being updated yet - try finding it by name
    if (method_p == nullptr || method_p->get_name() != get_name())
      {
      method_p = (vtable_scope == SkScope_instance ? call_scope_p->find_instance_method_inherited(get_name()) : call_scope_p->find_class_method_inherited(get_name()));
      }

    #ifdef SK_RUNTIME_RECOVER
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Test for missing method
      if (method_p == nullptr || method_p->get_name() != get_name())
        {
        // $Note - CReis This should not need to be checked here at runtime - the parser
        // should have already prevented any misuse.  However an invalid type cast can be
        // used to fool the parser.
        SK_ERROR_INFO(
          a_str_format(
            "Requested non-existing method %s from class %s!\n"
            "[Bad type cast or didn't check for nil?]",
            as_string_name().as_cstr(), call_scope_p->get_name().as_cstr_dbg()),
          caller_p);

        if (result_pp)
          {
          // Wanted a return so return a nil so there is something
          *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
          }

        return nullptr;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Test for stack overflow/infinite recursion

      #if 0 // $Revisit MBreyer - this test is no longer possible since there is no pool for SkInvokedMethods any more
        // Amount of `SkInvokedMethod` Objects that can be in current call stack before an
        // alert about a stack overflow / infinite recursion has occurred.
        const uint32_t Stack_limit        = 1000u;
        const uint32_t Stack_reset_limit  = Stack_limit - 100u;
        static bool    s_near_stack_limit = false;

        uint32_t used_count = SkInvokedMethod::get_pool().get_count_used();

        if (s_near_stack_limit)
          {
          if (used_count <= Stack_reset_limit)
            {
            // Reset stack overflow warning
            s_near_stack_limit = false;
            }
          }
        else
          {
          if (used_count >= Stack_limit)
            {
            s_near_stack_limit = true;

            SK_ERRORX(AErrMsg(
              a_str_format(
                "Stack overflow!\n"
                "The callstack is more than %u methods deep which seems like infinite recursion.\n\n"
                "[Skipping the next method to try unwind the call stack and prevent getting any deeper.\n"
                "There may be a - non-existing method X from 'nil' - error that follows.]",
                Stack_limit),
              AErrLevel_notify));

            // Break recursion chain
            if (result_pp)
              {
              *result_pp = SkBrain::ms_nil_p;
              }

            return nullptr;
            }
          }
      #endif // 0

    #endif // RUNTIME_RECOVER

  #endif // SKOOKUM & SK_DEBUG

  SkInvokedMethod imethod(caller_p, receiver_p, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  #if defined(SKDEBUG_COMMON)
    // Set with SKDEBUG_ICALL_STORE_GEXPR stored here before calls to argument expressions
    // overwrite it.
    const SkExpressionBase * call_expr_p = SkInvokedContextBase::ms_last_expr_p;
  #endif

  // Must be called before calling argument expressions
  SKDEBUG_ICALL_SET_GEXPR(&imethod);

  // Fill invoked method's argument list
  imethod.data_append_args_exprs(get_args(), method_p->get_params(), scope_p);

  // Hook must be called after argument expressions and before invoke()
  SKDEBUG_HOOK_EXPR(call_expr_p, scope_p, &imethod, nullptr, SkDebug::HookContext_peek);
  // Call method
  method_p->invoke(&imethod, caller_p, result_pp);

  // Bind any return arguments
  if (!m_return_args.is_empty())
    {
    imethod.data_bind_return_args(m_return_args, method_p->get_params());
    }

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallOnInstance Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Evaluates invocation expression and returns the resulting instance if desired.
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_on_instance>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  return invoke_call_internal(receiver_p, SkScope_instance, receiver_p->get_class(), scope_p, caller_p, result_pp);
  }

//=======================================================================================
// SkMethodCallOnClass Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_on_class>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->is_metaclass(), "SkMethodCallOnClass::invoke_call() must only be called on meta class receivers.");

  return invoke_call_internal(receiver_p, SkScope_class, static_cast<SkMetaClass *>(receiver_p)->get_class_info(), scope_p, caller_p, result_pp);
  }

//=======================================================================================
// SkMethodCallOnInstanceClass Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_on_instance_class>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  // Since this is a class method call, make sure we invoke always on the metaclass 
  SK_ASSERTX(!receiver_p->is_metaclass(), "Receiver must not be a meta class, SkMethodCallOnClass should be used for that.");

  return invoke_call_internal(&receiver_p->get_class()->get_metaclass(), SkScope_class, receiver_p->get_class(), scope_p, caller_p, result_pp);
  }

//=======================================================================================
// SkMethodCallOnClassInstance Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_on_class_instance>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->is_metaclass(), "SkMethodCallOnClassInstance::invoke_call() must only be called on meta class receivers.");

  return invoke_call_internal(receiver_p, SkScope_instance, receiver_p->get_class(), scope_p, caller_p, result_pp);
  }

//=======================================================================================
// SkMethodCallBooleanAnd Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_boolean_and>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_class() == SkBoolean::get_class(), "Receiver must be of type Boolean in logical operations.");

  // If this is true evaluate conjunct
  if (receiver_p->as<SkBoolean>())
    {
    get_args().get_first()->invoke(scope_p, caller_p, result_pp);
    }
  else if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(false);
    }

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallBooleanOr Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_boolean_or>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_class() == SkBoolean::get_class(), "Receiver must be of type Boolean in logical operations.");

  // If this is false evaluate disjunct
  if (!receiver_p->as<SkBoolean>())
    {
    get_args().get_first()->invoke(scope_p, caller_p, result_pp);
    }
  else
    {
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(true);
      }
    }

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallBooleanNand Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_boolean_nand>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_class() == SkBoolean::get_class(), "Receiver must be of type Boolean in logical operations.");

  // If this is true evaluate conjunct
  if (receiver_p->as<SkBoolean>())
    {
    get_args().get_first()->invoke(scope_p, caller_p, result_pp);

    if (result_pp)
      {
      bool & result = (*result_pp)->as<SkBoolean>();
      result = !result;
      }
    }
  else if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(true);
    }

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallBooleanNor Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_boolean_nor>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_class() == SkBoolean::get_class(), "Receiver must be of type Boolean in logical operations.");

  // If this is false evaluate disjunct
  if (!receiver_p->as<SkBoolean>())
    {
    get_args().get_first()->invoke(scope_p, caller_p, result_pp);

    if (result_pp)
      {
      bool & result = (*result_pp)->as<SkBoolean>();
      result = !result;
      }
    }
  else if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(false);
    }

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallAssert Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_assert>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_key_class() == SkBrain::ms_debug_class_p, "Receiver must be of type Debug in assert() calls.");

  // Implementation not needed in shipping builds
  #if SKOOKUM & SK_DEBUG

    // Evaluate the condition
    SkInstance * result_p = nullptr;
    m_arguments[SkArg_1]->invoke(scope_p, caller_p, &result_p);
    bool success = result_p->as<SkBoolean>();
    result_p->dereference();
    if (!success)
      {
      // Evaluate the message string
      AString message;
      if (m_arguments.get_length() > SkArg_2)
        {
        m_arguments[SkArg_2]->invoke(receiver_p, caller_p, &result_p);
        message = result_p->as<SkString>();
        result_p->dereference();
        }

      // Display message
      SK_ERRORX(a_str_format("Assertion failed: %s (%s)", message.as_cstr(), m_arguments[SkArg_1]->as_code().as_cstr()));
      }

  #endif

  // Return nil result
  if (result_pp) *result_pp = SkBrain::ms_nil_p;

  // Methods always complete immediately.
  return nullptr;
  }

//=======================================================================================
// SkMethodCallAssertNoLeak Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// See SkMethodCall<SkInvokeType_method_on_instance>::invoke_call()
template<>
SkInvokedBase * SkMethodCall<SkInvokeType_method_assert_no_leak>::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  SK_ASSERTX(receiver_p->get_key_class() == SkBrain::ms_debug_class_p, "Receiver must be of type Debug in assert() calls.");

  // Implementation not needed in shipping builds
  #if SKOOKUM & SK_DEBUG

    // Remember memory usage before call
    AObjReusePool<SkInstance> &          instance_pool = SkInstance::get_pool();
    AObjReusePool<SkInvokedExpression> & iexpr_pool = SkInvokedExpression::get_pool();
    AObjReusePool<SkInvokedCoroutine> &  icoroutine_pool = SkInvokedCoroutine::get_pool();
    AObjReusePool<AStringRef> &          str_ref_pool = AStringRef::get_pool();
    int32_t instance_use_count   = instance_pool.get_count_used();
    int32_t iexpr_use_count      = iexpr_pool.get_count_used();
    int32_t icoroutine_use_count = icoroutine_pool.get_count_used();
    int32_t str_ref_use_count    = str_ref_pool.get_count_used();

    // Evaluate the condition
    SkInstance * result_p = nullptr;
    m_arguments[SkArg_1]->invoke(scope_p, caller_p, &result_p);
    bool success = result_p->as<SkBoolean>();
    result_p->dereference();
    if (!success)
      {
      // Evaluate the message string
      AString message;
      if (m_arguments.get_length() > SkArg_2)
        {
        m_arguments[SkArg_2]->invoke(receiver_p, caller_p, &result_p);
        message = result_p->as<SkString>();
        result_p->dereference();
        }

      // Display message
      SK_ERRORX(a_str_format("Assertion failed: %s (%s)", message.as_cstr(), m_arguments[SkArg_1]->as_code().as_cstr()));
      }

    // Check memory usage after call
    int32_t instance_leak_count   = (int32_t)instance_pool.get_count_used()   - instance_use_count;
    int32_t iexpr_leak_count      = (int32_t)iexpr_pool.get_count_used()      - iexpr_use_count;
    int32_t icoroutine_leak_count = (int32_t)icoroutine_pool.get_count_used() - icoroutine_use_count;
    int32_t str_ref_leak_count    = (int32_t)str_ref_pool.get_count_used()    - str_ref_use_count;
    if (instance_leak_count || iexpr_leak_count || icoroutine_leak_count || str_ref_leak_count)
      {
      AString leak_desc;
      leak_desc.ensure_size(128);
      if (instance_leak_count)
        {
        leak_desc.append_format("%d instances", instance_leak_count);
        }
      if (iexpr_leak_count)
        {
        if (leak_desc.is_filled()) leak_desc.append(", ");
        leak_desc.append_format("%d invoked expressions", instance_leak_count);
        }
      if (icoroutine_leak_count)
        {
        if (leak_desc.is_filled()) leak_desc.append(", ");
        leak_desc.append_format("%d invoked coroutines", icoroutine_leak_count);
        }
      if (str_ref_leak_count)
        {
        if (leak_desc.is_filled()) leak_desc.append(", ");
        leak_desc.append_format("%d string refs", str_ref_leak_count);
        }

      // Evaluate the message string
      AString message;
      if (m_arguments.get_length() > SkArg_2)
        {
        m_arguments[SkArg_2]->invoke(receiver_p, caller_p, &result_p);
        message = result_p->as<SkString>();
        result_p->dereference();
        }

      // Display message
      SK_ERRORX(a_str_format("Memory leak detected: %s (while executing %s) - leaked %s", message.as_cstr(), m_arguments[SkArg_1]->as_code().as_cstr(), leak_desc.as_cstr()));
      }

  #endif

  // Return nil result
  if (result_pp) *result_pp = SkBrain::ms_nil_p;

  // Methods always complete immediately.
  return nullptr;
  }

