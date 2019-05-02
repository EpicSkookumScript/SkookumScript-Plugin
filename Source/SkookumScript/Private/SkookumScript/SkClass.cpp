// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structures for class descriptors and class objects
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkClass.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkClass.inl>
#endif

#include <AgogCore/AMath.hpp>
#include <AgogCore/AString.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkObjectID.hpp>
#include <SkookumScript/SkTypedClass.hpp>


//=======================================================================================
// SkMetaClass Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// 
// Returns: itself
// 
// Params:
//   class_info_p: The class that this MetaClass instance represents/is wrapped around
SkMetaClass::SkMetaClass(
  SkClass * class_info_p // = nullptr
  ) :
  // Note that SkBrain::ms_class_class_p will initially be nullptr for Object and Class and
  // then it will fixed in SkBrain::initialize().
  SkInstanceUnreffed(SkBrain::ms_class_class_p),  
  m_class_info_p(class_info_p)
  {
  // $Revisit - CReis [A_NOTE] ***Future?*** - implement class coroutines

  // Zero out user data - currently required by SkMind instances.
  ::memset(&m_user_data, 0, sizeof(m_user_data));
  }


// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create
//             a reference to this MetaClass and increments the memory address  to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit 4 bytes of binary data.
// Notes:      Binary composition:
//               4 bytes - class name id
//
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
void SkMetaClass::as_binary_ref(void ** binary_pp) const
  {
  // 4 bytes - method binary
  m_class_info_p->m_name.as_binary(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this class into its source code string equivalent.
//             This is essentially a disassembly of the internal script data-structures
//             into source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkClassDescBase) 
// Author(s):   Conan Reis
AString SkMetaClass::as_code() const
  {
  AString class_name(m_class_info_p->m_name.as_str_dbg());
  AString str(nullptr, class_name.get_length() + 3u, 0u);

  str.append('<');
  str.append(class_name);
  str.append('>');

  return str;
  }

//---------------------------------------------------------------------------------------
// Get a description of the class and indicate whether it is using instance
//             or class scope.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
AString SkMetaClass::get_scope_desc() const
  {
  AString str("the class '");

  str.append(m_class_info_p->m_name.as_str_dbg());
  str.append('\'');

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// If this is a generic/reflective class, it will be replaced with its
//             finalized/specific class using scope_type as its scope
//             For example: "<ThisClass_>" could become "<String>"
// Returns:    Finalized non-generic class
// Arg         scope_type - current scope class type
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkClassDescBase * SkMetaClass::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  if (m_class_info_p == SkBrain::ms_this_class_p)
    {
    return &scope_type.get_metaclass();
    }

  if (m_class_info_p == SkBrain::ms_item_class_p)
    {
    // ItemClass_ should only be used on SkTypedClass, but check anyway
    // $Revisit - CReis scope_type could be a union of typed classes
    SkClassDescBase * item_type_p = (scope_type.get_class_type() == SkClassType_typed_class)
      ? static_cast<const SkTypedClass *>(&scope_type)->get_item_type()
      : SkBrain::ms_object_class_p;

    return &item_type_p->get_metaclass();
    }

  return const_cast<SkMetaClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share - either a shared
// class (SkMetaClass) or the Object class (SkClass) if they share no other superclass.
//
// #See Also  find_common_class
// #Author(s) Conan Reis
SkClassUnaryBase * SkMetaClass::find_common_class(const SkClass & cls) const
  {
  SkClass * class_p = m_class_info_p->find_common_class(cls);

  if (class_p != SkBrain::ms_object_class_p)
    {
    return const_cast<SkMetaClass *>(&class_p->m_metaclass);
    }

  return SkBrain::ms_object_class_p;
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
//
// #Modifiers virtual
// #Author(s) Conan Reis
SkClassUnaryBase * SkMetaClass::find_common_type(const SkClassDescBase & cls) const
  {
  const SkClass * class_p;

  switch (cls.get_class_type())
    {
    case SkClassType_class:
      class_p = static_cast<const SkClass *>(&cls);
      break;

    case SkClassType_metaclass:
      return &m_class_info_p->find_common_class(*static_cast<const SkMetaClass *>(&cls)->m_class_info_p)
        ->get_metaclass();

    case SkClassType_class_union:
      return find_common_type(*cls.as_unary_class());

    //case SkClassType_typed_class:
    //case SkClassType_invokable_class:
    default:
      class_p = cls.get_key_class();
    }

  return find_common_class(*class_p);
  }

//---------------------------------------------------------------------------------------
// Returns class type of data member or nullptr if it does not exist
// Returns:    class type of data member or nullptr if it does not exist
// Arg         data_name - name of data member
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
SkTypedName * SkMetaClass::get_data_type(
  const ASymbol & data_name,
  eSkScope *      scope_p,  // = nullptr
  uint32_t *      data_idx_p,         // = nullptr
  SkClass **      data_owner_class_pp // = nullptr
  ) const
  {
  SkTypedName * type_p = m_class_info_p->get_class_data_type(data_name, data_idx_p, data_owner_class_pp);
  if (type_p)
    {
    if (scope_p)
      {
      *scope_p = SkScope_class;
      }

    return type_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if this metaclass is compatible with the specified class type
//             - i.e. can this class be passed as an argument to type_p.
// Returns:    true if compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkMetaClass::is_class_type(const SkClassDescBase * type_p) const
  {
  switch (type_p->get_class_type())
    {
    case SkClassType_class:
      // $Revisit - CReis In the future "Class" may be disallowed replaced with "<Object>".
      return ((type_p == SkBrain::ms_object_class_p)
        || (type_p == SkBrain::ms_class_class_p));

    case SkClassType_metaclass:
      {
      SkClass * type_class_p = static_cast<const SkMetaClass *>(type_p)->m_class_info_p;

      return ((type_class_p == SkBrain::ms_object_class_p)
        || m_class_info_p->is_class(*type_class_p));
      }

    case SkClassType_class_union:
      return static_cast<const SkClassUnion *>(type_p)->is_valid_param_for(this);
     
    default:
      // SkClassType_typed_class
      // SkClassType_invokable_class
      return false;
    }
  }

//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "<ThisClass_>" with "String" as a scope type becomes "<String>"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkMetaClass::is_generic() const
  {
  return (m_class_info_p == SkBrain::ms_this_class_p)
    || (m_class_info_p == SkBrain::ms_item_class_p);
  }

//---------------------------------------------------------------------------------------
// Evaluates the method and returns immediately
// Arg         method_name - name of method to call
// Arg         args_pp - Optional pointers to object instances to use as arguments - each
//             one present should have its reference count incremented and each
//             defaulted/skipped argument should be a nullptr element.  If arg_count is 0
//             this is ignored
// Arg         arg_count - number of arguments to use in args_pp.  If it is 0 then no
//             arguments are passed and args_pp is ignored.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             notified when this invocation is complete.
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.
// Notes:      This is a convenience method to use instead of SkMethodCall<>::invoke_call() - if more
//             arguments or control is desired, then use SkMethodCall<>::invoke_call()
// See:        method_query(), SkMethodCall<>::invoke_call(), call_destructor(), call_default_constructor()
// Author(s):   Conan Reis
void SkMetaClass::method_call(
  const ASymbol & method_name,
  SkInstance **   args_pp,
  uint32_t        arg_count,
  SkInstance **   result_pp, // = nullptr
  SkInvokedBase * caller_p   // = nullptr
  )
  {
  // Look for class method
  // $Revisit - CReis [Index Look-up] This look-up should eventually be replaced with an index into a method table.
  SkMethodBase * method_p = m_class_info_p->find_class_method_inherited(method_name);

  #if (SKOOKUM & SK_DEBUG)
    if (method_p == nullptr)
      {
      // $Vital - CReis This should not need to be checked here at runtime - the parser
      // should have already prevented any misuse.  However an invalid type cast can be
      // used to fool the parser.
      A_ERRORX(
        a_str_format(
          "Requested non-existing method '%s()' from class '%s'!\n"
          "[A common cause of this error is when an invalid type cast is used.]",
          method_name.as_cstr_dbg(), m_class_info_p->get_name().as_cstr_dbg()));

      if (result_pp)
        {
        // Wanted a return so return a nil so there is something
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return;
      }
  #endif

  // Note that when 'this' is used in class methods, it refers to the class not an
  // instance (if called on an instance)
  SkInvokedMethod imethod(caller_p, this, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  SKDEBUG_ICALL_SET_INTERNAL(&imethod);
  SKDEBUG_HOOK_SCRIPT_ENTRY(method_name);

  // Fill invoked method's argument list
  imethod.data_append_args(args_pp, arg_count, method_p->get_params());

  // Call method
  method_p->invoke(&imethod, caller_p, result_pp);

  SKDEBUG_HOOK_SCRIPT_EXIT();
  }


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Returns a string representation of itself for debugging purposes
// Returns:    Debug string
// Modifiers:   virtual - overridden from SkInstance
// Author(s):   Conan Reis
AString SkMetaClass::as_string_debug() const
  {
  AString class_name(m_class_info_p->m_name.as_str_dbg());
  AString str(nullptr, class_name.get_length() + 3u, 0u);

  str.append('<');
  str.append(class_name);
  str.append('>');

  return str;
  }

#endif  // SK_AS_STRINGS

//---------------------------------------------------------------------------------------
// Look up the given variable by name
SkInstance * SkMetaClass::get_data_by_name(const ASymbol & name) const
  {
  // Get class data
  uint32_t data_idx = 0;
  SkClass * owner_class_p = nullptr;
  if (m_class_info_p->get_class_data_type(name, &data_idx, &owner_class_p))
    {
    return owner_class_p->get_class_data_value_by_idx(data_idx);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Look up the given variable by name and sets it to given value
bool SkMetaClass::set_data_by_name(const ASymbol & name, SkInstance * data_p)
  {
  // Set class data
  uint32_t data_idx = 0;
  SkClass * owner_class_p = nullptr;
  if (m_class_info_p->get_class_data_type(name, &data_idx, &owner_class_p))
    {
    owner_class_p->set_class_data_value_by_idx(data_idx, data_p);
    return true;
    }

  return false;
  }


//=======================================================================================
// SkClass Class Data
//=======================================================================================

tSkRawResolveFunc SkClass::ms_raw_resolve_f;

#if (SKOOKUM & SK_DEBUG)
  SkClass::ReparseInfo SkClass::ms_reparse_info;

  void SkClass::ReparseInfo::begin()
    {
    m_is_active = true;
    }

  void SkClass::ReparseInfo::end()
    {
    m_is_active = false;
    free_all();
    }

  void SkClass::ReparseInfo::free_all()
    {
    ms_reparse_info.m_data.free_all();
    ms_reparse_info.m_data_raw.free_all();
    ms_reparse_info.m_class_data.free_all();

    ms_reparse_info.m_methods.free_all();
    ms_reparse_info.m_class_methods.free_all();
    ms_reparse_info.m_coroutines.free_all();
    }

  void SkClass::ReparseInfo::free_all_compact()
    {
    ms_reparse_info.m_data.free_all_compact();
    ms_reparse_info.m_data_raw.free_all_compact();
    ms_reparse_info.m_class_data.free_all_compact();

    ms_reparse_info.m_methods.free_all_compact();
    ms_reparse_info.m_class_methods.free_all_compact();
    ms_reparse_info.m_coroutines.free_all_compact();
    }

#endif

//=======================================================================================
// SkClass Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

void SkClass::initialize()
  {
  #if (SKOOKUM & SK_DEBUG)
    ms_reparse_info.end();
  #endif
  }

//---------------------------------------------------------------------------------------

void SkClass::deinitialize()
  {
  #if (SKOOKUM & SK_DEBUG)
    ms_reparse_info.free_all_compact();
  #endif
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - Name of the class.  It should follow the form of "ClassName"
//             uppercase {alphanumeric}
// Arg         superclass_p - pointer to superclass of this class.  Only "Object" should
//             have no superclass.
// Author(s):   Conan Reis
SkClass::SkClass(
  const ASymbol & name,
  SkClass *       superclass_p,     // = nullptr
  uint32_t        flags,            // = Flag__default
  uint32_t        annotation_flags  // = 0
  ) :
  ANamed(name),
  m_flags(flags),
  m_annotation_flags(annotation_flags),
  m_superclass_p(superclass_p),
  m_bind_name(AString::ms_empty),
  m_raw_member_accessor_f(nullptr),
  m_raw_pointer_f(nullptr),
  m_object_id_lookup_f(nullptr),
  m_object_id_lookup_p(nullptr),
  m_object_ids_p(nullptr),
  m_total_data_count(0),
  m_destructor_p(nullptr),
  m_total_class_data_count(0),
  m_user_data(0),
  m_user_data_int(0)
  {
  m_metaclass.m_class_info_p = this;

  // Set flags for fast determination if class is derived from Actor
  if (m_name == SkBrain::ms_actor_class_name)
    {
    m_flags |= Flag_is_actor;
    }

  // Set flags for fast determination if class is derived from an entity
  if (m_name == SkBrain::ms_entity_class_name)
    {
    m_flags |= Flag_is_entity;
    }

  // Set flags for fast determination if class is derived from a component
  if (m_name == SkBrain::ms_component_class_name)
    {
    m_flags |= Flag_is_component;
    }

  // Set flags for fast determination if class is derived from Mind
  if (m_name == ASymbol_Mind)
    {
    m_flags |= Flag_is_mind;
    }

  if (superclass_p)
    {
    superclass_p->m_subclasses.append(*this);

    // Inherit Actor, Mind and Component flags
    m_flags |= (superclass_p->m_flags & (Flag_is_actor|Flag_is_mind|Flag_is_entity|Flag_is_component));

    // Inherit data counts from superclass
    m_total_data_count       = superclass_p->m_total_data_count;
    m_total_class_data_count = superclass_p->m_total_class_data_count;
    }

  // Sanity checking
  #if (SKOOKUM & SK_DEBUG)
    m_ref_count = 0;
  #endif
  }

//---------------------------------------------------------------------------------------
// Destructor
// Examples:   called by system
// See:        recurse_class_deinit()
// Notes:      Ensure that recurse_class_deinit() is called prior to destructing any
//             class object.
//             This method is virtual to ensure that subclasses call their appropriate
//             destructor.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkClass::~SkClass()
  {
  SK_MAD_ASSERTX_NO_THROW(m_ref_count == 0, a_str_format("Class `%s` is being deleted while still having %u references.", get_name_cstr(), m_ref_count));

  clear_members();
  }

//---------------------------------------------------------------------------------------
// Reset that part of SkCass that is affected by the class meta info file
// Called before parsing meta info files
void SkClass::clear_meta_info()
  {
  enable_demand_load(false);
  set_annotation_flags(0);
  m_bind_name = AString::ms_empty;
  #if (SKOOKUM & SK_CODE_IN)
    set_object_id_validate(SkClass::Flag__id_valid_none);
  #endif
  }

//---------------------------------------------------------------------------------------
// Removes all members from class.
// See:        destructor ~SkClass(), clear_members_compact()
// Author(s):   Conan Reis
void SkClass::clear_members()
  {
  m_data.free_all();
  m_data_raw.free_all();
  m_class_data.free_all();
  m_class_data_values.empty();

  m_methods.free_all();
  m_class_methods.free_all();

  m_coroutines.free_all();

  m_vtable_i.empty();
  m_vtable_c.empty();

  m_flags &= ~(Flag_loaded | Flag_demand_unload);
  m_destructor_p = nullptr;

  // Assuming that clear_members() is being called on all parents as well
  m_total_data_count = 0;
  m_total_class_data_count = 0;

  #if (SKOOKUM & SK_CODE_IN)
    clear_object_id_valid_list();
  #endif
  }

//---------------------------------------------------------------------------------------
// Removes all members from class and reduces memory footprint.
// See:        clear_members()
// Author(s):   Conan Reis
void SkClass::clear_members_compact()
  {
  m_data.free_all();
  m_data.empty_compact();
  m_total_data_count = m_superclass_p ? m_superclass_p->m_total_data_count : 0;

  m_data_raw.free_all();
  m_data_raw.empty_compact();

  m_class_data.free_all();
  m_class_data.empty_compact();

  m_class_data_values.empty_compact();

  m_methods.free_all();
  m_class_data.empty_compact();
  m_class_methods.free_all();
  m_class_data.empty_compact();
  m_total_class_data_count = m_superclass_p ? m_superclass_p->m_total_class_data_count : 0;

  m_coroutines.free_all();
  m_coroutines.empty_compact();

  m_flags &= ~(Flag_loaded | Flag_demand_unload);
  m_destructor_p = nullptr;

  #if (SKOOKUM & SK_CODE_IN)
    clear_object_id_valid_list();
  #endif
  }

//---------------------------------------------------------------------------------------
// Get the root class that this demand loaded class belongs to.  [It could be
//             itself or nullptr if the class is not demand loaded.]
// Returns:    SkClass or nullptr
// Author(s):   Conan Reis
SkClass * SkClass::get_demand_loaded_root() const
  {
  SkClass * root_p = m_superclass_p ? m_superclass_p->get_demand_loaded_root() : nullptr;

  return root_p
    ? root_p
    : is_demand_loaded_root()
      ? const_cast<SkClass *>(this)
      : nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its contained sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkClass::track_memory(
  AMemoryStats * mem_stats_p,
  bool           skip_demand_loaded
  ) const
  {
  bool demand_loaded = is_demand_loaded();

  if (!skip_demand_loaded || !demand_loaded)
    {
    mem_stats_p->track_memory(
      SKMEMORY_ARGS(SkClass, 0u),
      sizeof(void *) * (1u + m_subclasses.get_length() + m_data.get_length() + m_data_raw.get_length()
         + m_class_data.get_length() + m_class_data_values.get_length() + m_methods.get_length()
         + m_class_methods.get_length() + m_coroutines.get_length()),
      sizeof(void *) * (1u + m_subclasses.get_size() + m_data.get_size() + m_data_raw.get_size()
         + m_class_data.get_size() + m_class_data_values.get_size() + m_methods.get_size()
         + m_class_methods.get_size() + m_coroutines.get_size()));
      // Added 1 x pointer dynamic bytes for space in SkBrain::ms_classes

    uint32_t member_count = m_data.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedName, 0u), 0u, 0u, member_count);
      }

    member_count = m_data_raw.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedNameRaw, 0u), 0u, 0u, member_count);
      }

    member_count = m_class_data.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedName, 0u), 0u, 0u, member_count);
      }

    m_methods.track_memory(mem_stats_p);
    m_class_methods.track_memory(mem_stats_p);
    m_coroutines.track_memory(mem_stats_p);
    }
  else
    {
    // Demand loaded classes still have an empty shell (no members) in memory so that
    // SkClassUnion and SkTypedClass objects can reference them.
    mem_stats_p->track_memory(
      SKMEMORY_ARGS(SkClass, 0u),
      sizeof(void *) * (1u + m_subclasses.get_length()),
      sizeof(void *) * (1u + m_subclasses.get_size() + m_data.get_size() + m_data_raw.get_size()
         + m_class_data.get_size()+ m_class_data_values.get_size() + m_methods.get_size()
         + m_class_methods.get_size() + m_coroutines.get_size()));
      // Added 1 x pointer dynamic bytes for space in SkBrain::ms_classes
    }

  // Count demand loaded classes - though their size totals are captured by SkClass
  if (demand_loaded)
    {
    mem_stats_p->track_memory("SkClass.demand", 0u);
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and contained sub-objects and then do
//             the same recursively for its subclasses
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkClass::track_memory_recursive(
  AMemoryStats * mem_stats_p,
  bool           skip_demand_loaded
  ) const
  {
  track_memory(mem_stats_p, skip_demand_loaded);

  SkClass ** classes_pp     = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  while (classes_pp < classes_end_pp)
    {
    (*classes_pp)->track_memory_recursive(mem_stats_p, skip_demand_loaded);

    classes_pp++;
    }
  }

//---------------------------------------------------------------------------------------

void SkClass::register_raw_resolve_func(tSkRawResolveFunc raw_resolve_f)
  {
  ms_raw_resolve_f = raw_resolve_f;
  }

//---------------------------------------------------------------------------------------

void SkClass::register_raw_accessor_func(tSkRawAccessorFunc raw_member_accessor_f)
  {
  m_raw_member_accessor_f = raw_member_accessor_f;
  }

//---------------------------------------------------------------------------------------

void SkClass::register_raw_pointer_func(tSkRawPointerFunc raw_pointer_f)
  {
  m_raw_pointer_f = raw_pointer_f;
  }

//---------------------------------------------------------------------------------------
// Crawl up the hierarchy and return the first function we find
tSkRawAccessorFunc SkClass::get_raw_accessor_func_inherited() const
  {
  for (const SkClass * class_p = this; class_p; class_p = class_p->get_superclass())
    {
    if (class_p->m_raw_member_accessor_f)
      {
      return class_p->m_raw_member_accessor_f;
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Crawl up the hierarchy and return the first function we find
tSkRawPointerFunc SkClass::get_raw_pointer_func_inherited() const
  {
  for (const SkClass * class_p = this; class_p; class_p = class_p->get_superclass())
    {
    if (class_p->m_raw_pointer_f)
      {
      return class_p->m_raw_pointer_f;
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determine whether named object instances of this class may be looked up via object IDs
// - i.e. Class@'name'. This setting is propagated to all subclasses.
//
// #See  object_id_lookup(), object_id_validate(), SkObjectID
bool SkClass::is_object_id_lookup() const
  {
  if ((m_flags & Flag__id_valid_mask) != 0u)
    {
    return true;
    }

  SkClass * super_p = m_superclass_p;

  while (super_p)
    {
    if ((super_p->m_flags & Flag__id_valid_mask) != 0u)
      {
      return true;
      }

    super_p = super_p->m_superclass_p;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Set named object look-up function for object IDs.
//
// #See Also  *object_id_*()
// #Author(s) Conan Reis
void SkClass::set_object_id_lookup_func(tSkObjectIdLookupFunc object_id_lookup_f)
  {
  m_object_id_lookup_f = object_id_lookup_f;
  }

//---------------------------------------------------------------------------------------
// [Called during runtime when an object ID is used.]
// Determines the object reference by a Validated Object ID or returns nullptr if it
// cannot be found.
// 
// The supplied obj_id_p may be modified to cache the object so that it may be returned
// immediately if the same object ID is invoked repeatedly.  Any other aspects of the
// object ID such as its flags may also be modified in the process.
// 
// It uses any C++ function stored via set_object_id_lookup_func() and if no C++ function
// is set it calls the Sk class method ojbect_id_find(NameType name) <ThisClass_|None> 
// 
// #Returns referenced object or nullptr
// #Params
//   obj_id_p: Object ID to get info from to look up object
//   caller_p: Call stack from SkObjectID::invoke()
//
// #Modifiers virtual
// #See       object_id_validate()
// #Author    Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns named object or nullptr if not found.
SkInstance * SkClass::object_id_lookup(
  const SkBindName & name,
  SkInvokedBase *    caller_p // = nullptr
  ) const
  {
  // If custom C++ lookup is present, use it
  if (m_object_id_lookup_f)
    {
    return (m_object_id_lookup_f)(name);
    }

  // Cache ojbect_id_find(NameType name) <ThisClass_|None> if it isn't already cached.
  if (m_object_id_lookup_p == nullptr)
    {
    m_object_id_lookup_p = find_class_method_inherited(ASymbol_object_id_find);
    }

  if (m_object_id_lookup_p)
    {
    SkInvokedMethod imethod(
      caller_p, const_cast<SkMetaClass *>(&m_metaclass), m_object_id_lookup_p, a_stack_allocate(m_object_id_lookup_p->get_invoked_data_array_size(), SkInstance*));

    SKDEBUG_ICALL_SET_INTERNAL(&imethod);
    SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_object_id_find);

    // Fill invoked method's argument list
    SkInstance * name_p = name.new_instance();
    imethod.data_append_args(&name_p, 1u, m_object_id_lookup_p->get_params());

    // Call method
    SkInstance * result_p = nullptr;

    m_object_id_lookup_p->invoke(&imethod, caller_p, &result_p);

    SKDEBUG_HOOK_SCRIPT_EXIT();

    // Note that `imethod` will dereference `name_p`.

    if (result_p != SkBrain::ms_nil_p)
      {
      return result_p;
      }
    }

  return nullptr;
  }


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Clear object ID validation list.
//
// #See Also  *object_id_*()
// #Author(s) Conan Reis
void SkClass::clear_object_id_valid_list()
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    if (m_object_ids_p)
      {
      delete m_object_ids_p;
      m_object_ids_p = nullptr;
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Set validation type for object IDs.
//
// #See Also  *object_id_*()
// #Author(s) Conan Reis
void SkClass::set_object_id_validate(
  uint32_t validate_mask // = Flag__id_valid_any
  )
  {
  m_flags = (m_flags & ~Flag__id_valid_mask) | validate_mask;

  #if defined(A_SYMBOLTABLE_CLASSES)
    if ((m_flags & Flag_object_id_parse_list) == 0u)
      {
      clear_object_id_valid_list();
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Get object ID flags for this class. This setting may be inherited from its superclasses.
//
// #See  object_id_lookup(), object_id_validate(), SkObjectID
uint32_t SkClass::get_object_id_validate() const
  {
  if ((m_flags & Flag__id_valid_mask) != 0u)
    {
    return m_flags & Flag__id_valid_mask;
    }

  SkClass * super_p = m_superclass_p;

  while (super_p)
    {
    if ((super_p->m_flags & Flag__id_valid_mask) != 0u)
      {
      return super_p->m_flags & Flag__id_valid_mask;
      }

    super_p = super_p->m_superclass_p;
    }

  return Flag__id_valid_none;
  }

//---------------------------------------------------------------------------------------
// [Called during parsing whenever an object ID is used.] Determines if the supplied
// object ID may be a valid named object.  The supplied object ID may be modified by this
// method by changing flags, setting a more appropriate look-up class, etc.
//
// #Modifiers virtual
// #See Also  object_id_lookup()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns class type of named object [usually the same class as the look-up class in
  // obj_id_p - i.e. this class - though it could be a closer derived/sub-class] or nullptr
  // if name object not valid for this class.
  SkClass *
SkClass::object_id_validate(
  const SkBindName & name,
  bool               validate_deferred // = true
  ) const
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    switch (get_object_id_validate())
      {
      case Flag__id_valid_none:
        // Accept none during compile
        return nullptr;

      case Flag__id_valid_any:
        // Accept any during compile
        return const_cast<SkClass *>(this);

      case Flag__id_valid_parse:
        // Validate using list during compile
        break;
    
      case Flag__id_valid_exist:
        // Validate using list during compile if it exists (parse) - otherwise accept any
        // during compile and validate using list in separate pass/run (defer).
        if (!validate_deferred && (m_object_ids_p == nullptr))
          {
          // Ids not available yet - accept any during compile
          return const_cast<SkClass *>(this);
          }

        // validate object IDs - either they are present or it validation requested
        break;

      case Flag__id_valid_defer:
        if (validate_deferred)
          {
          break;
          }

        // Accept any during compile and validate using list in separate pass/run
        return const_cast<SkClass *>(this);
      }

    // $Revisit - CReis Engine run-time does not have object ID validation lists loaded
    if (!SkDebug::is_engine_present())
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Ensure validation ids are available
      if (m_object_ids_p == nullptr)
        {
        return nullptr;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Validate ids
      // $Revisit - CReis Object IDs Shouldn't convert from a string every time
      if (!m_object_ids_p->is_registered(ASYMBOL_STR_TO_ID(name.as_string())))
        {
        return nullptr;
        }
      }
  #endif  // defined(A_SYMBOLTABLE_CLASSES)

  return const_cast<SkClass *>(this);
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Validate all object ID expressions in this class and its subclasses (as specified by "hierarchy").
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Number of validation errors encountered
  uint32_t
SkClass::object_id_validate_recurse(
  // Do deferred validations now (true) or later (false)
  bool validate_deferred,
  // Optional address to store number of object ID expressions validated
  uint32_t * obj_id_count_p // = nullptr
  )
  {
  struct NestedValidateObjIds : public SkApplyExpressionBase
    {
    uint32_t m_count;
    uint32_t m_errors;
    bool     m_validate_deferred;

    virtual eAIterateResult apply_expr(SkExpressionBase * expr_p, const SkInvokableBase * invokable_p) override
      {
      if (expr_p->get_type() == SkExprType_object_id)
        {
        SkObjectID * obj_id_p = static_cast<SkObjectID *>(expr_p);

        if (obj_id_p->validate(m_validate_deferred) == nullptr)
          {
          SkDebug::print_script_context(
            a_str_format(
              "\nObject ID %s invalid - no instance named '%s' exists for the class '%s'!",
              obj_id_p->as_code().as_cstr(),
              obj_id_p->get_name().as_string().as_cstr(),
              obj_id_p->m_class_p->get_name_cstr_dbg()),
            invokable_p,
            expr_p,
            SkDPrintType_error);

          m_errors++;
          }

        m_count++;
        }

      return AIterateResult_entire;
      }
    };

  NestedValidateObjIds validater;
  
  validater.m_count  = 0u;
  validater.m_errors = 0u;
  validater.m_validate_deferred = validate_deferred;

  iterate_expressions_recurse(&validater);

  if (obj_id_count_p)
    {
    *obj_id_count_p = validater.m_count;
    }

  return validater.m_errors;
  }

#endif  // (SKOOKUM & SK_DEBUG)

#endif  // (SKOOKUM & SK_CODE_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this Class and its components and increments the memory address
//             to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length(), SkActorClass(binary_pp), ADatum
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - name id [Ordered for SkBrain::assign_binary()]
//               2 bytes - number of data members 
//               4 bytes - data member name id \_ Repeating
//               5*bytes - class type          /
//               2 bytes - number of raw data members 
//               4 bytes - data member name id \
//               5*bytes - class type          |_ Repeating
//               n*bytes - bind name           /
//               2 bytes - number of class data members 
//               4 bytes - class data member name id \_ Repeating
//               5*bytes - class type                /
//               4 bytes - number of methods
//               1 byte  - method type   \_ Repeating
//               n bytes - method binary /
//               4 bytes - number of class methods
//               1 byte  - method type         \_ Repeating
//               n bytes - class method binary /
//               4 bytes - number of coroutines
//               1 byte  - coroutine type   \_Repeating
//               n bytes - coroutine binary /
//
//             $Note - CReis This could be packed more
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkClass::as_binary(void ** binary_pp, bool include_routines) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClass::as_binary_length(include_routines));

  uint32_t ** data_pp = (uint32_t **)binary_pp;

  // 4 bytes - name id
  m_name.as_binary(binary_pp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t length = m_data.get_length();

  // 2 bytes - number of data members 
  **(uint16_t **)binary_pp = static_cast<uint16_t>(length);
  (*(uint16_t **)binary_pp)++;


  // 4 bytes - data member name id \_ Repeating
  // 5*bytes - class type          /
  SkTypedName ** names_pp = m_data.get_array();
  SkTypedName ** names_end_pp = names_pp + length;

  for (; names_pp < names_end_pp; names_pp++)
    {
    (*names_pp)->as_binary(binary_pp);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  length = m_data_raw.get_length();

  // 2 bytes - number of data members 
  **(uint16_t **)binary_pp = static_cast<uint16_t>(length);
  (*(uint16_t **)binary_pp)++;


  // 4 bytes - data member name id \
  // 5*bytes - class type          |_ Repeating
  // n*bytes - bind name           /
  SkTypedNameRaw ** names_raw_pp = m_data_raw.get_array();
  SkTypedNameRaw ** names_raw_end_pp = names_raw_pp + length;

  for (; names_raw_pp < names_raw_end_pp; names_raw_pp++)
    {
    (*names_raw_pp)->as_binary(binary_pp);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - number of class data members 
  length = m_class_data.get_length();
  **(uint16_t **)binary_pp = static_cast<uint16_t>(length);
  (*(uint16_t **)binary_pp)++;


  // 4 bytes - class data member name id \_ Repeating
  // 5*bytes - class type                /
  SkTypedName ** csdata_pp     = m_class_data.get_array();
  SkTypedName ** csdata_end_pp = csdata_pp + length;

  for (; csdata_pp < csdata_end_pp; csdata_pp++)
    {
    (*csdata_pp)->as_binary(binary_pp);
    }

  if (include_routines)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 4 bytes - number of methods
    length = m_methods.get_length();
    **data_pp = length;
    (*data_pp)++;


    // 1 byte  - method type   \_Repeating
    // n bytes - method binary /
    SkMethodBase ** methods_pp = m_methods.get_array();
    SkMethodBase ** methods_end_pp = methods_pp + length;

    for (; methods_pp < methods_end_pp; methods_pp++)
      {
      // 1 byte - method type
      **(uint8_t **)binary_pp = static_cast<uint8_t>((*methods_pp)->get_invoke_type());
      (*(uint8_t **)binary_pp)++;

      // n bytes - method binary
      (*methods_pp)->as_binary(binary_pp, true);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 4 bytes - number of class methods
    length = m_class_methods.get_length();
    **data_pp = length;
    (*data_pp)++;

    // 1 byte  - method type         \_Repeating
    // n bytes - class method binary /
    methods_pp = m_class_methods.get_array();
    methods_end_pp = methods_pp + length;

    for (; methods_pp < methods_end_pp; methods_pp++)
      {
      // 1 byte - method type
      **(uint8_t **)binary_pp = static_cast<uint8_t>((*methods_pp)->get_invoke_type());
      (*(uint8_t **)binary_pp)++;

      // n bytes - method binary
      (*methods_pp)->as_binary(binary_pp, true);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 4 bytes - number of coroutines
    length = m_coroutines.get_length();
    **data_pp = length;
    (*data_pp)++;

    // 1 byte  - coroutine type   \_Repeating
    // n bytes - coroutine binary /
    SkCoroutineBase ** coroutines_pp = m_coroutines.get_array();
    SkCoroutineBase ** coroutines_end_pp = coroutines_pp + length;

    for (; coroutines_pp < coroutines_end_pp; coroutines_pp++)
      {
      // 1 byte - coroutine type
      **(uint8_t **)binary_pp = static_cast<uint8_t>((*coroutines_pp)->get_invoke_type());
      (*(uint8_t **)binary_pp)++;

      // n bytes - coroutine binary
      (*coroutines_pp)->as_binary(binary_pp, true);
      }
    }

  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//             Binary composition:
//               4 bytes - name id [Ordered for SkBrain::assign_binary()]
//               2 bytes - number of data members 
//               4 bytes - data member name id \_ Repeating
//               5*bytes - class type          /
//               2 bytes - number of raw data members 
//               4 bytes - data member name id \
//               5*bytes - class type          |_ Repeating
//               n*bytes - bind name           /
//               2 bytes - number of class data members 
//               4 bytes - class data member name id \_ Repeating
//               5*bytes - class type                /
//               4 bytes - number of methods
//               1 byte  - method type   \_ Repeating
//               n bytes - method binary /
//               4 bytes - number of class methods
//               1 byte  - method type         \_ Repeating
//               n bytes - class method binary /
//               4 bytes - number of coroutines
//               1 byte  - coroutine type   \_Repeating
//               n bytes - coroutine binary /
// Author(s):   Conan Reis
uint32_t SkClass::as_binary_length(bool include_routines) const
  {
  uint32_t binary_length = include_routines ? 22u : 10u;
  uint32_t length;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (ms_compounds_use_ref)
    {
    binary_length += (m_data.get_length() + m_class_data.get_length()) * (4u + Binary_ref_size_typed);
    }
  else
    {
    length = m_data.get_length();

    SkTypedName ** names_pp     = m_data.get_array();
    SkTypedName ** names_end_pp = names_pp + length;

    for (; names_pp < names_end_pp; names_pp++)
      {
      binary_length += (*names_pp)->as_binary_length();
      }

    length = m_class_data.get_length();

    SkTypedName ** csdata_pp     = m_class_data.get_array();
    SkTypedName ** csdata_end_pp = csdata_pp + length;

    for (; csdata_pp < csdata_end_pp; csdata_pp++)
      {
      binary_length += (*csdata_pp)->as_binary_length();
      }
    }

  // Raw data members store their bind names so have to always look at each of them
  length = m_data_raw.get_length();

  SkTypedNameRaw ** names_raw_pp = m_data_raw.get_array();
  SkTypedNameRaw ** names_raw_end_pp = names_raw_pp + length;

  for (; names_raw_pp < names_raw_end_pp; names_raw_pp++)
    {
    binary_length += (*names_raw_pp)->as_binary_length();
    }


  if (include_routines)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1 byte  - method type   \_Repeating
    // n bytes - method binary /
    SkMethodBase ** methods_pp = m_methods.get_array();
    SkMethodBase ** methods_end_pp = methods_pp + m_methods.get_length();

    for (; methods_pp < methods_end_pp; methods_pp++)
      {
      binary_length += 1u + (*methods_pp)->as_binary_length(true);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1 byte  - method type         \_Repeating
    // n bytes - class method binary /
    methods_pp = m_class_methods.get_array();
    methods_end_pp = methods_pp + m_class_methods.get_length();

    for (; methods_pp < methods_end_pp; methods_pp++)
      {
      binary_length += 1u + (*methods_pp)->as_binary_length(true);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // + 1 byte  - coroutine type   \_Repeating
    // + n bytes - coroutine binary /
    SkCoroutineBase ** coroutines_pp = m_coroutines.get_array();
    SkCoroutineBase ** coroutines_end_pp = coroutines_pp + m_coroutines.get_length();

    for (; coroutines_pp < coroutines_end_pp; coroutines_pp++)
      {
      binary_length += 1u + (*coroutines_pp)->as_binary_length(true);
      }
    }

  return binary_length;
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recursively recreate a class and its components and increments the memory
//             address to just past the last byte written.  Only non-demand loaded classes
//             are written out at this phase.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// Examples:   Called by as_binary()
// See:        as_binary_recurse_length()
// Author(s):   Conan Reis
void SkClass::as_binary_recurse(
  void ** binary_pp,
  bool    skip_demand_loaded
  ) const
  {
  if (!skip_demand_loaded || !is_demand_loaded())
    {
    // Write out binary for 'class_p'
    as_binary(binary_pp);

    uint32_t length = m_subclasses.get_length();

    // Write out binary for subclasses of 'class_p' if any
    if (length)
      {
      SkClass ** classes_pp     = m_subclasses.get_array();
      SkClass ** classes_end_pp = classes_pp + length;

      // n bytes - SkClass or SkActorClass binary }- Repeating
      for (; classes_pp < classes_end_pp; classes_pp++)
        {
        (*classes_pp)->as_binary_recurse(binary_pp, skip_demand_loaded);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself and subclasses in bytes.
//             Optionally skips demand loaded classes
// Returns:    length in bytes
// Notes:      Used in combination with as_binary_recurse()
// Author(s):   Conan Reis
uint32_t SkClass::as_binary_recurse_length(bool skip_demand_loaded) const
  {
  uint32_t bytes = 0u;

  if (!skip_demand_loaded || !is_demand_loaded())
    {
    // Write out binary for 'class_p'
    bytes = as_binary_length();

    uint32_t sub_count = m_subclasses.get_length();

    // Write out binary for subclasses of 'class_p' if any
    if (sub_count)
      {
      SkClass ** classes_pp     = m_subclasses.get_array();
      SkClass ** classes_end_pp = classes_pp + sub_count;

      // n bytes - SkClass or SkActorClass binary }- Repeating
      for (; classes_pp < classes_end_pp; classes_pp++)
        {
        bytes += (*classes_pp)->as_binary_recurse_length(skip_demand_loaded);
        }
      }
    }

  return bytes;
  }

//---------------------------------------------------------------------------------------
// Just like as_binary_recurse() except that it first writes out the number
//             of classes that are being written out.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_group_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_group_length(), from_binary_group()
// Author(s):   Conan Reis
void SkClass::as_binary_group(
  void ** binary_pp,
  bool    skip_demand_loaded
  ) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClass::as_binary_group_length(skip_demand_loaded));

  // 4 bytes - number of classes (excluding demand loaded)
  uint32_t class_count = get_class_recurse_count(skip_demand_loaded);

  A_BYTE_STREAM_OUT32(binary_pp, &class_count);

  if (class_count)
    {
    as_binary_recurse(binary_pp, skip_demand_loaded);
    }
  }

//---------------------------------------------------------------------------------------
// Just like as_binary_recurse_length() except that it includes space to
//             write out the number of classes that are being written out.
// Returns:    length in bytes
// Notes:      Used in combination with as_binary_group()
// Author(s):   Conan Reis
uint32_t SkClass::as_binary_group_length(bool skip_demand_loaded) const
  {
  return 4u + as_binary_recurse_length(skip_demand_loaded);
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recursively recreate the class hierarchy (with empty classes) and
//             increments the memory address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_placeholder_recurse_length()
// Notes:      Binary composition:
//               4 bytes - class name id
//               4 bytes - class flags
//               4 bytes - annotation flags
//               n bytes - bind name
//               2 bytes - number of subclasses
//               n bytes - subclasses placeholder binary }-  Repeating & Recursive
// Author(s):   Conan Reis
void SkClass::as_binary_placeholder_recurse(void ** binary_pp) const
  {
  // 4 bytes - name id
  as_binary_ref(binary_pp);

  // 4 bytes - class flags
  uint32_t flags = m_flags & SkClass::Flag__mask_binary;
  A_BYTE_STREAM_OUT32(binary_pp, &flags);

  // 4 bytes - annotation flags
  A_BYTE_STREAM_OUT32(binary_pp, &m_annotation_flags);

  // n bytes - bind name
  m_bind_name.as_binary(binary_pp);

  uint32_t length = m_subclasses.get_length();

  // 2 bytes - number of subclasses members 
  A_BYTE_STREAM_OUT16(binary_pp, &length);

  // Write out binary for subclasses of 'class_p' if any
  if (length)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + length;

    // n bytes - SkClass or SkActorClass binary }- Repeating
    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      (*classes_pp)->as_binary_placeholder_recurse(binary_pp);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Returns size in bytes needed for as_binary_placeholder_recurse() called on
//             the "Object" class - i.e. all the classes.
// See:        as_binary_placeholder_recurse()
// Notes:      Binary composition:
//               4 bytes - class name id
//               4 bytes - class flags
//               4 bytes - annotation flags
//               n bytes - bind name
//               2 byte  - number of subclasses
//               n bytes - subclasses placeholder binary }-  Repeating & Recursive
// Author(s):   Conan Reis
uint32_t SkClass::as_binary_placeholder_recurse_length()
  {
  uint32_t binary_length = SkClass::Binary_ref_size + 10u + m_bind_name.as_binary_length();

  for (SkClass * subclass_p : m_subclasses)
    {
    binary_length += subclass_p->as_binary_placeholder_recurse_length();
    }

  return binary_length;
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create
//             a reference to this Class and increments the memory address  to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit 4 bytes of binary data.
// Notes:      Binary composition:
//               4 bytes - class name id
//
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
void SkClass::as_binary_ref(void ** binary_pp) const
  {
  // 4 bytes - class name id
  m_name.as_binary(binary_pp);
  }


#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Assign binary info to this object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Assumes that the binary info has already been partially parsed (the name
//             was parse to determine which class to apply this binary to) with
//             SkBrain::assign_binary().
//
//             Binary composition:
//               2 bytes - number of data members 
//               4 bytes - data member name id \_ Repeating
//               5*bytes - class type          /
//               2 bytes - number of raw data members 
//               4 bytes - data member name id \
//               5*bytes - class type          |_ Repeating
//               n*bytes - bind name           /
//               2 bytes - number of class data members 
//               4 bytes - class data member name id \_ Repeating
//               5*bytes - class type                /
//               4 bytes - number of methods
//               1 byte  - method type   \_ Repeating
//               n bytes - method binary /
//               4 bytes - number of class methods
//               1 byte  - method type         \_ Repeating
//               n bytes - class method binary /
//               4 bytes - number of coroutines
//               1 byte  - coroutine type   \_Repeating
//               n bytes - coroutine binary /
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkClass::assign_binary(
  const void ** binary_pp,
  bool          include_routines      // = true
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - number of data members 
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);

  m_data.ensure_size(length);
  recurse_modify_total_data_count(length);

  ASymbol name;
  SkClassDescBase * type_p;

  // 4 bytes - data member name id \_ Repeating
  // 5*bytes - class type          /
  for (; length > 0u ; length--)
    {
    // Note that this cannot be placed in the call to append_instance_data() since the evaluation
    // order of the arguments cannot be guaranteed.
    name = ASymbol::create_from_binary(binary_pp);
    append_instance_data(name, from_binary_ref_typed(binary_pp), false);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - number of raw data members 
  length = A_BYTE_STREAM_UI16_INC(binary_pp);

  m_data_raw.ensure_size(length);

  // 4 bytes - data member name id \
  // 5*bytes - class type          |_ Repeating
  // n*bytes - bind name           /
  for (; length > 0u; length--)
    {
    // Note that this cannot be placed in the call to append_instance_data() since the evaluation
    // order of the arguments cannot be guaranteed.
    name = ASymbol::create_from_binary(binary_pp);
    type_p = from_binary_ref_typed(binary_pp);
    append_instance_data_raw(name, type_p, SkBindName(binary_pp).as_string());
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - number of class data members 
  length = A_BYTE_STREAM_UI16_INC(binary_pp);

  m_class_data.ensure_size(length);
  m_class_data_values.ensure_size(length);
  recurse_modify_total_class_data_count(length);

  // 4 bytes - class data member name id \_ Repeating
  // 5*bytes - class type                /
  for (; length > 0u ; length--)
    {
    // Note that this cannot be placed in the call to append_class_data() since the evaluation
    // order of the arguments cannot be guaranteed.
    name = ASymbol::create_from_binary(binary_pp);
    append_class_data(name, from_binary_ref_typed(binary_pp), false);
    }

  if (include_routines)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Instance methods
    // 4 bytes - number of methods
    length = A_BYTE_STREAM_UI32_INC(binary_pp);
    m_methods.ensure_size(length);
    for (; length > 0u; length--)
      {
      // n bytes - method binary
      append_instance_method(binary_pp);
      }
    // Make sure class has a destructor - assumes superclasses always created before subclasses
    if (!m_destructor_p && m_superclass_p)
      {
      m_destructor_p = m_superclass_p->m_destructor_p;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Class methods
    // 4 bytes - number of class methods
    length = A_BYTE_STREAM_UI32_INC(binary_pp);
    m_class_methods.ensure_size(length);
    for (; length > 0u; length--)
      {
      // n bytes - method binary
      append_class_method(binary_pp);
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Coroutines
    // 4 bytes - number of coroutines
    length = A_BYTE_STREAM_UI32_INC(binary_pp);
    m_coroutines.ensure_size(length);
    for (; length > 0u; length--)
      {
      // n bytes - coroutine binary
      append_coroutine(binary_pp);
      }
    }

  set_loaded();
  }

//---------------------------------------------------------------------------------------
// Returns pointer to class based on the binary reference info or nullptr if not
//             found
// Returns:    pointer to class or nullptr
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_ref() or a similar
//             mechanism.
// See:        as_binary_ref_typed(), as_binary_ref_typed_length(), from_binary_ref_typed()
// Notes:      Binary composition:
//               4 bytes - class name id
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkClass * SkClass::from_binary_ref(const void ** binary_pp)
  {
  // $Revisit - CReis [Optimization] Make this inline
  return SkBrain::get_class(ASymbol::create_from_binary(binary_pp));
  }

//---------------------------------------------------------------------------------------
// Loads some number of classes stored as a binary.
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_group() or a similar
//             mechanism.
// See:        as_binary_group()
// Modifiers:   static
// Author(s):   Conan Reis
void SkClass::from_binary_group(const void ** binary_pp)
  {
  // 4 bytes - number of classes (excluding demand loaded)
  uint32_t class_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  // n bytes - SkClass or SkActorClass binary }- Repeating
  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default_source);
  #endif

  SkClass * class_p;

  for (; class_count > 0u ; class_count--)
    {
    class_p = SkClass::from_binary_ref(binary_pp);
    class_p->assign_binary(binary_pp);
    }

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default);
  #endif
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) instance method with the given binary method
// The method will use this class as its class scope.
void SkClass::append_instance_method(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p /*= nullptr*/)
  {
  // 1 byte  - method type
  // 4 bytes - method name
  eSkInvokable itype = eSkInvokable(A_BYTE_STREAM_UI8_INC(binary_pp));
  ASymbol name  = ASymbol::create_from_binary(binary_pp);
  SkMethodBase * method_p = nullptr;

  #if (SKOOKUM & SK_DEBUG)
    // Keep track of current routine being serialized in
    SkRuntimeBase::ms_singleton_p->m_current_routine = SkMemberInfo(SkQualifier(name, this), SkMember_method, false);

    // Get old method to replace if it exists
    SkQualifier ident(name, this);
    SkMethodBase * old_method_p;
    if (ms_reparse_info.m_is_active)
      {
      old_method_p = ms_reparse_info.m_methods.pop(ident);
      }
    else
      {
      old_method_p = m_methods.pop(ident);
      }

    // Keep old method around if possible
    int16_t vtable_index = -1;
    method_p = old_method_p;
    if (method_p)
      {
      vtable_index = method_p->get_vtable_index();

      // Different type? (we ignore differences between SkInvokable_method_func and SkInvokable_method_mthd)
      if ((itype == SkInvokable_method) != (method_p->get_invoke_type() == SkInvokable_method))
        {
        // Yes, discard old cached version
        if (update_record_p)
          {
          method_p->copy_to_update_record(update_record_p);
          update_record_p->m_routine_p = nullptr;
          update_record_p->m_previous_routine_p = method_p;
          }
        else
          {
          delete method_p;
          }
        method_p = nullptr;
        }
      else
        {
        method_p->assign_binary_no_name(binary_pp, update_record_p);
        method_p->set_vtable_index(vtable_index); // Keep same vtable index
        }
      }

    if (method_p == nullptr)
  #endif
      {
      // n bytes - class method binary
      switch (itype)
        {
        case SkInvokable_method:       // Method using custom expressions
          method_p = SK_NEW(SkMethod)(name, this, binary_pp);
          break;

        case SkInvokable_method_mthd:  // Method using atomic C++ method
          method_p = SK_NEW(SkMethodMthd)(name, this, binary_pp);
          break;

        default: // SkInvokable_method_func:  Method using atomic C++ function
          method_p = SK_NEW(SkMethodFunc)(name, this, binary_pp);
          break;
        }
      }

  #if (SKOOKUM & SK_DEBUG)
    // If there's a vtable index, assign it appropriately
    if (vtable_index >= 0)
      {
      method_p->set_vtable_index(vtable_index);
      recurse_replace_vtable_entry_i(vtable_index, old_method_p, method_p);
      }

    // Done serializing in this routine
    SkRuntimeBase::ms_singleton_p->m_current_routine.invalidate();
  #endif

  m_methods.append(*method_p);

  // Set shortcut to method if it is a destructor.
  // $Revisit - CReis [Optimization] The binary info will be in sorted order, so if there
  // is a destructor (!!) it will be the first method
  if (name == ASymbolX_dtor)
    {
    //set_destructor(method_p)
    m_destructor_p = method_p;
    }
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) class method with the given binary method
// The method will use this class as its class scope.
void SkClass::append_class_method(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p /*= nullptr*/)
  {
  // 1 byte  - method type
  // 4 bytes - method name
  eSkInvokable itype = eSkInvokable(A_BYTE_STREAM_UI8_INC(binary_pp));
  ASymbol name = ASymbol::create_from_binary(binary_pp);
  SkMethodBase * method_p = nullptr;

  #if (SKOOKUM & SK_DEBUG)
    // Keep track of current routine being serialized in
    SkRuntimeBase::ms_singleton_p->m_current_routine = SkMemberInfo(SkQualifier(name, this), SkMember_method, true);

    // Get old method to replace if it exists
    SkQualifier ident(name, this);
    SkMethodBase * old_method_p;
    if (ms_reparse_info.m_is_active)
      {
      old_method_p = ms_reparse_info.m_class_methods.pop(ident);
      }
    else
      {
      old_method_p = m_class_methods.pop(ident);
      }

    // Keep old method around if possible
    int16_t vtable_index = -1;
    method_p = old_method_p;
    if (method_p)
      {
      vtable_index = method_p->get_vtable_index();

      // Different type? (we ignore differences between SkInvokable_method_func and SkInvokable_method_mthd)
      if ((itype == SkInvokable_method) != (method_p->get_invoke_type() == SkInvokable_method))
        {
        // Yes, discard old cached version
        if (update_record_p)
          {
          method_p->copy_to_update_record(update_record_p);
          update_record_p->m_routine_p = nullptr;
          update_record_p->m_previous_routine_p = method_p;
          }
        else
          {
          delete method_p;
          }
        method_p = nullptr;
        }
      else
        {
        method_p->assign_binary_no_name(binary_pp, update_record_p);
        method_p->set_vtable_index(vtable_index); // Keep same vtable index
        }
      }

    if (method_p == nullptr)
    #endif
      {
      // n bytes - class method binary
      switch (itype)
        {
        case SkInvokable_method:       // Method using custom expressions
          method_p = SK_NEW(SkMethod)(name, this, binary_pp);
          break;

        case SkInvokable_method_mthd:  // Method using atomic C++ method
          method_p = SK_NEW(SkMethodMthd)(name, this, binary_pp);
          break;

        default: // SkInvokable_method_func:  // Method using atomic C++ function
          method_p = SK_NEW(SkMethodFunc)(name, this, binary_pp);
          break;
        }
      }

  #if (SKOOKUM & SK_DEBUG)
    // If there's a vtable index, assign it appropriately
    if (vtable_index >= 0)
      {
      method_p->set_vtable_index(vtable_index);
      recurse_replace_vtable_entry_c(vtable_index, old_method_p, method_p);
      }
 
    // Done serializing in this routine
    SkRuntimeBase::ms_singleton_p->m_current_routine.invalidate();
  #endif

  m_class_methods.append(*method_p);
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) coroutine with the given binary coroutine
// The coroutine will use this class as its class scope.
void SkClass::append_coroutine(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p /*= nullptr*/)
  {
  // 1 byte  - coroutine type
  // 4 bytes - coroutine name
  eSkInvokable itype = eSkInvokable(A_BYTE_STREAM_UI8_INC(binary_pp));
  ASymbol name = ASymbol::create_from_binary(binary_pp);
  SkCoroutineBase * coroutine_p = nullptr;

  #if (SKOOKUM & SK_DEBUG)
    // Keep track of current routine being serialized in
    SkRuntimeBase::ms_singleton_p->m_current_routine = SkMemberInfo(SkQualifier(name, this), SkMember_coroutine, false);

    // Get old coroutine to replace if it exists
    SkQualifier ident(name, this);
    SkCoroutineBase * old_coroutine_p;
    if (ms_reparse_info.m_is_active)
      {
      old_coroutine_p = ms_reparse_info.m_coroutines.pop(ident);
      }
    else
      {
      old_coroutine_p = m_coroutines.pop(ident);
      }

    // Keep old coroutine around if possible
    int16_t vtable_index = -1;
    coroutine_p = old_coroutine_p;
    if (coroutine_p)
      {
      vtable_index = coroutine_p->get_vtable_index();

      // Different type? (we ignore differences between SkInvokable_coroutine_func and SkInvokable_coroutine_mthd)
      if ((itype == SkInvokable_coroutine) != (coroutine_p->get_invoke_type() == SkInvokable_coroutine))
        {
        // Yes, discard old cached version
        if (update_record_p)
          {
          coroutine_p->copy_to_update_record(update_record_p);
          update_record_p->m_routine_p = nullptr;
          update_record_p->m_previous_routine_p = coroutine_p;
          }
        else
          {
          delete coroutine_p;
          }
        coroutine_p = nullptr;
        }
      else
        {
        uint32_t old_invoked_data_array_size = coroutine_p->get_invoked_data_array_size();
        coroutine_p->assign_binary_no_name(binary_pp, update_record_p);
        coroutine_p->set_vtable_index(vtable_index); // Keep same vtable index
        coroutine_p->set_invoked_data_array_size(a_max(uint32_t(coroutine_p->get_invoked_data_array_size()), old_invoked_data_array_size)); // Keep same or larger invoked data array size
        }
      }

    if (coroutine_p == nullptr)
  #endif
      {
      // n bytes - coroutine binary
      switch (itype)
        {
        case SkInvokable_coroutine:
          // Compound (Custom) coroutine
          coroutine_p = SK_NEW(SkCoroutine)(name, this, binary_pp);
          break;

        case SkInvokable_coroutine_mthd:
          // Atomic coroutine (C++ method)
          coroutine_p = SK_NEW(SkCoroutineMthd)(name, this, binary_pp);
          break;

        default:  // SkInvokable_coroutine_func
          // Atomic coroutine (C++ function)
          coroutine_p = SK_NEW(SkCoroutineFunc)(name, this, binary_pp);
        }
      }

  #if (SKOOKUM & SK_DEBUG)
    // If there's a vtable index, assign it appropriately
    if (vtable_index >= 0)
      {
      coroutine_p->set_vtable_index(vtable_index);
      recurse_replace_vtable_entry_i(vtable_index, old_coroutine_p, coroutine_p);
      }

    // Done serializing in this routine
    SkRuntimeBase::ms_singleton_p->m_current_routine.invalidate();
  #endif

  m_coroutines.append(*coroutine_p);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// If the class belongs to a demand load group - ensure that it is loaded and
//             locked into memory.  This ensures that the member data-structures can be
//             reused as much as possible allowing pointers to members to persist.
// See:        SkRuntimeBase::load_compiled_class_group()
// Modifiers:   virtual            
// Author(s):   Conan Reis
void SkClass::ensure_loaded_debug()
  {
  // Make sure compound refs is switched off for the duration of this method call
  bool compounds_use_ref_saved = ms_compounds_use_ref;
  ms_compounds_use_ref = false;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If class_p in a demand loaded group ensure loaded and lock changes in memory
  SkClass * demand_root_p = get_demand_loaded_root();

  if (demand_root_p)
    {
    if (!demand_root_p->is_loaded())
      {
      SkDebug::print(
        a_str_format("\nAuto loading Skookum class group '%s' into memory.\n", demand_root_p->get_name_cstr_dbg()),
        SkLocale_all,
        SkDPrintType_warning);

      SkRuntimeBase::ms_singleton_p->load_compiled_class_group(demand_root_p);
      }

    if (!demand_root_p->is_load_locked())
      {
      demand_root_p->lock_load();

      SkDebug::print(
        a_str_format("\nDemand loaded Skookum class group '%s' now locked in memory (changes will not auto-unload).\n", demand_root_p->get_name_cstr_dbg()),
        SkLocale_all,
        SkDPrintType_warning);
      }
    }

  // Restore state of compounds_use_ref
  ms_compounds_use_ref = compounds_use_ref_saved;
  }

//---------------------------------------------------------------------------------------
// Get existing class ready for a reparse of its members - ensuring that the
//             member data-structures can be reused as much as possible allowing pointers
//             to members to persist.
// Modifiers:   virtual            
// Author(s):   Conan Reis
void SkClass::reparse_begin(bool include_routines)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If class is in a demand loaded group ensure loaded and lock changes in memory
  ensure_loaded_debug();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Save members for reintegration and clear the rest

  // Save/clear data members
  ms_reparse_info.m_data.append_all(m_data);
  ms_reparse_info.m_data_raw.append_all(m_data_raw);
  ms_reparse_info.m_class_data.append_all(m_class_data);
  recurse_modify_total_data_count(-(int32_t)m_data.get_length());
  recurse_modify_total_class_data_count(-(int32_t)m_class_data.get_length());
  m_data.remove_all();
  m_data_raw.remove_all();
  m_class_data.remove_all();

  if (include_routines)
    {
    // Save method/coroutine members to reuse data-structures which may have C++ bindings and might be
    // referenced and then clear them to get ready for reparsing.
    ms_reparse_info.m_methods.append_all(m_methods);
    ms_reparse_info.m_class_methods.append_all(m_class_methods);
    ms_reparse_info.m_coroutines.append_all(m_coroutines);
    m_methods.remove_all();
    m_class_methods.remove_all();
    m_coroutines.remove_all();
    }

  ms_reparse_info.begin();

  // Disable compound refs during reparsing
  SkClassDescBase::enable_compound_refs(false);
  }

//---------------------------------------------------------------------------------------

void SkClass::reparse_end()
  {
  // If destructor method was deleted, fix the destructor pointer
  if (m_destructor_p && ms_reparse_info.m_methods.find(*m_destructor_p))
    {
    m_destructor_p = m_superclass_p ? m_superclass_p->m_destructor_p : nullptr;
    }

  ms_reparse_info.end();

  // Reenable compound refs
  SkClassDescBase::enable_compound_refs();
  }

//---------------------------------------------------------------------------------------
// Iterates through all expressions in all invokables (methods and coroutines) in this
// class applying operation supplied by apply_expr_p and exiting early if its apply_expr()
// returns AIterateResult_early_exit.
//
// #See Also
//   SkApplyExpressionBase, SkClass::iterate_expressions_recurse(),
//   SkExpressionBase::iterate_expressions()
//   
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkClass::iterate_expressions(SkApplyExpressionBase * apply_expr_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Instance methods
  SkMethodBase ** method_pp     = m_methods.get_array();
  SkMethodBase ** method_end_pp = method_pp + m_methods.get_length();

  while (method_pp < method_end_pp)
    {
    if ((*method_pp)->iterate_expressions(apply_expr_p))
      {
      // Exit early
      return AIterateResult_early_exit;
      }

    method_pp++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Class methods
  method_pp     = m_class_methods.get_array();
  method_end_pp = method_pp + m_class_methods.get_length();

  while (method_pp < method_end_pp)
    {
    if ((*method_pp)->iterate_expressions(apply_expr_p))
      {
      // Exit early
      return AIterateResult_early_exit;
      }

    method_pp++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Instance coroutines
  SkCoroutineBase ** coro_pp     = m_coroutines.get_array();
  SkCoroutineBase ** coro_end_pp = coro_pp + m_coroutines.get_length();

  while (coro_pp < coro_end_pp)
    {
    if ((*coro_pp)->iterate_expressions(apply_expr_p))
      {
      // Exit early
      return AIterateResult_early_exit;
      }

    coro_pp++;
    }

  // Full iteration performed
  return AIterateResult_entire;
  }

//---------------------------------------------------------------------------------------
// Iterates through all expressions in all invokables (methods and coroutines) in this
// class *and all sub-classes* applying operation supplied by apply_expr_p and exiting
// early if its apply_expr() returns AIterateResult_early_exit.
//
// #See Also
//   SkApplyExpressionBase, iterate_expressions(), iterate_recurse(),
//   SkExpressionBase::iterate_expressions()
//   
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkClass::iterate_expressions_recurse(
  SkApplyExpressionBase * apply_expr_p,
  eAHierarchy hierarchy // = AHierarchy__all
  )
  {
  if (hierarchy & AHierarchy_current)
    {
    iterate_expressions(apply_expr_p);
    }

  if (hierarchy & AHierarchy_recurse)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    while (classes_pp < classes_end_pp)
      {
      if ((*classes_pp)->iterate_expressions_recurse(apply_expr_p, AHierarchy__all))
        {
        // Exit early
        return AIterateResult_early_exit;
        }

      classes_pp++;
      }
    }

  // Full iteration performed
  return AIterateResult_entire;
  }

//---------------------------------------------------------------------------------------
// Count all expressions in this class and its subclasses (as specified by "hierarchy").
//
// #Author(s) Conan Reis
uint32_t SkClass::count_expressions_recurse(
  eAHierarchy hierarchy // = AHierarchy__all
  )
  {
  struct NestedCountExprs : public SkApplyExpressionBase
    {
    uint32_t m_count;

    virtual eAIterateResult apply_expr(SkExpressionBase * expr_p, const SkInvokableBase * invokable_p) override
      {
      m_count++;

      return AIterateResult_entire;
      }
    };

  NestedCountExprs counter;
  
  counter.m_count = 0u;
  iterate_expressions_recurse(&counter, hierarchy);

  return counter.m_count;
  }

#endif  // (SKOOKUM & SK_DEBUG)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this class into its source code string equivalent.
//             This is essentially a disassembly of the internal script data-structures
//             into source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkClassDescBase) 
// Author(s):   Conan Reis
AString SkClass::as_code() const
  {
  return m_name.as_str_dbg();
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "ThisClass_" with "String" as a scope type becomes "String"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkClass::is_generic() const
  {
  return (this == SkBrain::ms_this_class_p)
    || (this == SkBrain::ms_item_class_p);
  }

//---------------------------------------------------------------------------------------

void SkClass::set_user_data_int_recursively(uint32_t value)
  {
  set_user_data_int(value);

  for (SkClass * sk_class_p : m_subclasses)
    {
    sk_class_p->set_user_data_int_recursively(value);
    }
  }

//---------------------------------------------------------------------------------------
// Creates a new SkMind instance
// Caller must already know that this is a Mind class, will fail otherwise
SkInstance * SkClass::new_mind_instance()
  {
  SK_ASSERTX(is_mind_class(), "Can create Mind instance only from Mind class.");

  SkMind * mind_p = SK_NEW(SkMind)(this);
  mind_p->reference();
  return mind_p;
  }

//---------------------------------------------------------------------------------------
// Determines if the instance method is overridden by *any* subclass and returns first
// such method it finds or nullptr.
// 
// #Author(s) Conan Reis
SkMethodBase * SkClass::find_instance_method_overridden(const ASymbol & method_name) const
  {
  SkClass *  subclass_p;
  SkClass ** classes_pp     = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  SkMethodBase * method_p = nullptr;

  while (classes_pp < classes_end_pp)
    {
    subclass_p = *classes_pp;
    method_p = subclass_p->find_instance_method(method_name);

    if (method_p)
      {
      return method_p;
      }

    method_p = subclass_p->find_instance_method_overridden(method_name);

    if (method_p)
      {
      return method_p;
      }

    classes_pp++;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the class method is overridden by *any* subclass and returns first
// such method it finds or nullptr.
// 
// #Author(s) Conan Reis
SkMethodBase * SkClass::find_class_method_overridden(const ASymbol & method_name) const
  {
  SkClass *  subclass_p;
  SkClass ** classes_pp     = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  SkMethodBase * method_p = nullptr;

  while (classes_pp < classes_end_pp)
    {
    subclass_p = *classes_pp;
    method_p = subclass_p->find_class_method(method_name);

    if (method_p)
      {
      return method_p;
      }

    method_p = subclass_p->find_class_method_overridden(method_name);

    if (method_p)
      {
      return method_p;
      }

    classes_pp++;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the coroutine is overridden by *any* subclass and returns first such
// coroutine it finds or nullptr.
// 
// #Author(s) Conan Reis
SkCoroutineBase * SkClass::find_coroutine_overridden(const ASymbol & coroutine_name) const
  {
  SkClass *  subclass_p;
  SkClass ** classes_pp     = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  SkCoroutineBase * coroutine_p = nullptr;

  while (classes_pp < classes_end_pp)
    {
    subclass_p = *classes_pp;
    coroutine_p = subclass_p->find_coroutine(coroutine_name);

    if (coroutine_p)
      {
      return coroutine_p;
      }

    coroutine_p = subclass_p->find_coroutine_overridden(coroutine_name);

    if (coroutine_p)
      {
      return coroutine_p;
      }

    classes_pp++;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// If this is a generic/reflective class, it will be replaced with its
//             finalized/specific class using scope_type as its scope
//             For example: "ThisClass_" could become "String"
// Returns:    Finalized non-generic class
// Arg         scope_type - current scope class type
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkClassDescBase * SkClass::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  if (this == SkBrain::ms_this_class_p)
    {
    return scope_type.is_metaclass()
      ? scope_type.get_key_class()
      : const_cast<SkClassDescBase *>(&scope_type);
    }

  if (this == SkBrain::ms_item_class_p)
    {
    // ItemClass_ should only be used on SkTypedClass, but check anyway
    // $Revisit - CReis scope_type could be a union of typed classes
    return (scope_type.get_class_type() == SkClassType_typed_class)
      ? static_cast<const SkTypedClass *>(&scope_type)->get_item_type()
      : SkBrain::ms_object_class_p;
    }

  return const_cast<SkClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Appends the specified subclass to this class
// Arg         subclass_p - pointer to subclass to append
// Author(s):   Conan Reis
void SkClass::append_subclass(SkClass * subclass_p)
  {
  SK_ASSERTX(subclass_p->m_subclasses.is_empty(), "Subclass to append must have no subclasses itself.");

  // Link into hierarchy
  m_subclasses.append_absent(*subclass_p);
  subclass_p->m_superclass_p = this;

  // Inherit Actor, Mind and Component flags
  subclass_p->m_flags = (subclass_p->m_flags & ~(Flag_is_actor|Flag_is_mind|Flag_is_entity|Flag_is_component)) | (m_flags & (Flag_is_actor|Flag_is_mind|Flag_is_entity|Flag_is_component));

  // Inherit data counts from superclass
  subclass_p->m_total_data_count = m_total_data_count + subclass_p->m_data.get_length();
  subclass_p->m_total_class_data_count = m_total_class_data_count + subclass_p->m_class_data.get_length();
  }

//---------------------------------------------------------------------------------------
// (Re-)builds this class's vtable and that of all subclasses
// IMPORTANT: Assumes that the vtables of all superclasses have already been built
void SkClass::build_vtables_recurse(bool force_new)
  {
  build_vtables(force_new);

  for (auto subclass_p : m_subclasses)
    {
    subclass_p->build_vtables_recurse(force_new);
    }
  }

//---------------------------------------------------------------------------------------
// (Re-)builds this class's vtable
// IMPORTANT: Assumes that the vtables of all superclasses have already been built
void SkClass::build_vtables(bool force_new)
  {
  // Pass 1: Determine how much space we need in the new vtable

  uint32_t this_count_i = m_methods.get_length() + m_coroutines.get_length(); // Total number of invokables in this class
  uint32_t this_count_c = m_class_methods.get_length(); // Total number of invokables in this class
  uint32_t override_count_i = 0; // How many of those are overridden from a superclass
  uint32_t override_count_c = 0; // How many of those are overridden from a superclass
  uint32_t super_vtable_length_i = 0;
  uint32_t super_vtable_length_c = 0;
  const tSkVTable * super_vtable_i_p = nullptr;
  const tSkVTable * super_vtable_c_p = nullptr;
  if (m_superclass_p)
    {
    super_vtable_i_p = &m_superclass_p->m_vtable_i;
    super_vtable_c_p = &m_superclass_p->m_vtable_c;
    super_vtable_length_i = super_vtable_i_p->get_length();
    super_vtable_length_c = super_vtable_c_p->get_length();

    // Instance methods
    for (auto invokable_p : m_methods)
      {
      int32_t vtable_index = SkQualifier::ms_invalid_vtable_index;
      override_count_i += (uint32_t)super_vtable_i_p->find(invokable_p->get_name(), AMatch_first_found, (uint32_t *)&vtable_index);
      SK_ASSERTX(force_new || vtable_index < 0 || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index((int16_t)vtable_index); // Remember vtable index if any
      }

    // Class methods
    for (auto invokable_p : m_class_methods)
      {
      int32_t vtable_index = SkQualifier::ms_invalid_vtable_index;
      override_count_c += (uint32_t)super_vtable_c_p->find(invokable_p->get_name(), AMatch_first_found, (uint32_t *)&vtable_index);
      SK_ASSERTX(force_new || vtable_index < 0 || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index((int16_t)vtable_index); // Remember vtable index if any
      }

    // Coroutines
    for (auto invokable_p : m_coroutines)
      {
      int32_t vtable_index = SkQualifier::ms_invalid_vtable_index;
      override_count_i += (uint32_t)super_vtable_i_p->find(invokable_p->get_name(), AMatch_first_found, (uint32_t *)&vtable_index);
      SK_ASSERTX(force_new || vtable_index < 0 || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index((int16_t)vtable_index); // Remember vtable index if any
      }
    }
  uint32_t this_vtable_length_i = super_vtable_length_i + this_count_i - override_count_i;
  uint32_t this_vtable_length_c = super_vtable_length_c + this_count_c - override_count_c;

  // Pass 2: Allocate and populate vtable

  m_vtable_i.set_size(this_vtable_length_i, this_vtable_length_i);
  m_vtable_c.set_size(this_vtable_length_c, this_vtable_length_c);

  // First, duplicate the super class vtable if present
  if (super_vtable_i_p && super_vtable_length_i)
    {
    memcpy(m_vtable_i.get_array(), super_vtable_i_p->get_array(), super_vtable_length_i * sizeof(SkInvokableBase *));
    }
  if (super_vtable_c_p && super_vtable_length_c)
    {
    memcpy(m_vtable_c.get_array(), super_vtable_c_p->get_array(), super_vtable_length_c * sizeof(SkInvokableBase *));
    }

  // Then, populate with invokables of this class
  int16_t new_index_i = (int16_t)super_vtable_length_i;
  int16_t new_index_c = (int16_t)super_vtable_length_c;

  // Instance methods
  for (auto invokable_p : m_methods)
    {
    int16_t vtable_index = invokable_p->get_vtable_index();
    if (!super_vtable_i_p || vtable_index < 0)
      { // If not an override, create new entry
      vtable_index = new_index_i++;
      SK_ASSERTX(force_new || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index(vtable_index);
      }
    m_vtable_i[vtable_index] = invokable_p;
    }

  // Class methods
  for (auto invokable_p : m_class_methods)
    {
    int16_t vtable_index = invokable_p->get_vtable_index();
    if (!super_vtable_c_p || vtable_index < 0)
      { // If not an override, create new entry
      vtable_index = new_index_c++;
      SK_ASSERTX(force_new || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index(vtable_index);
      }
    m_vtable_c[vtable_index] = invokable_p;
    }

  // Coroutines
  for (auto invokable_p : m_coroutines)
    {
    int16_t vtable_index = invokable_p->get_vtable_index();
    if (!super_vtable_i_p || vtable_index < 0)
      { // If not an override, create new entry
      vtable_index = new_index_i++;
      SK_ASSERTX(force_new || invokable_p->get_vtable_index() < 0 || invokable_p->get_vtable_index() == (int16_t)vtable_index, "If vtable index exists, must be the same (for existing calls to work)!");
      invokable_p->set_vtable_index(vtable_index);
      }
    m_vtable_i[vtable_index] = invokable_p;
    }

  SK_ASSERTX((uint32_t)new_index_i == this_vtable_length_i, "Must exactly populate all members of the new vtable.");
  SK_ASSERTX((uint32_t)new_index_c == this_vtable_length_c, "Must exactly populate all members of the new vtable.");
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
//
// #Modifiers virtual
// #Author(s) Conan Reis
SkClassUnaryBase * SkClass::find_common_type(const SkClassDescBase & cls) const
  {
  const SkClass * class_p = nullptr;

  switch (cls.get_class_type())
    {
    case SkClassType_class:
      class_p = static_cast<const SkClass *>(&cls);
      break;

    case SkClassType_metaclass:
      return static_cast<const SkMetaClass *>(&cls)->find_common_class(*this);

    case SkClassType_typed_class:
    case SkClassType_invokable_class:
      class_p = cls.get_key_class();
      break;

    case SkClassType_class_union:
      return find_common_type(*cls.as_unary_class());
    }

  return find_common_class(*class_p);
  }

//---------------------------------------------------------------------------------------
// Determines if data member exists and if so its class type and optionally the scope
// where it was found.
// 
// Returns: class type of data member or nullptr if not found
// 
// Params:
//   data_name: name of data member
//   scope_p:
//     Optional address to store scope location where data member was found - see eSkScope.
//     If set to nullptr or not found then it will be unchanged.
SkTypedName * SkClass::get_data_type(
  const ASymbol & data_name,
  eSkScope *      scope_p,  // = nullptr
  uint32_t *      data_idx_p,         // = nullptr
  SkClass **      data_owner_class_pp // = nullptr
  ) const
  {
  eSkScope scope = SkScope_instance;
  SkTypedName * type_p = get_instance_data_type(data_name, data_idx_p, data_owner_class_pp);
  if (!type_p)
    {
    scope = SkScope_instance_raw;
    type_p = get_instance_data_type_raw(data_name, data_idx_p, data_owner_class_pp);
    }
  if (!type_p)
    {
    scope = SkScope_class;
    type_p = get_class_data_type(data_name, data_idx_p, data_owner_class_pp);
    }
  if (type_p)
    {
    if (scope_p)
      {
      *scope_p = scope;
      }

    return type_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Looks up an instance data member by data index
SkTypedName * SkClass::get_instance_data_type(uint32_t data_idx, SkClass ** data_owner_class_pp /*= nullptr*/) const
  {
  // Try to find this instance member in this class
  SkClass * owner_class_p = const_cast<SkClass *>(this);
  uint32_t inherited_instance_data_count = get_inherited_instance_data_count();
  A_LOOP_INFINITE
    {
    if (data_idx >= inherited_instance_data_count)
      {
      if (data_owner_class_pp)
        {
        *data_owner_class_pp = owner_class_p;
        }

      return owner_class_p->m_data[data_idx - inherited_instance_data_count];
      }

    owner_class_p = owner_class_p->get_superclass();
    if (!owner_class_p) break;
    inherited_instance_data_count -= owner_class_p->m_data.get_length();
    }

    return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if an instance data member exists and if so its class type 
// and optionally where it was found.
SkTypedName * SkClass::get_instance_data_type(
  const ASymbol & data_name, 
  uint32_t * data_idx_p,         // = nullptr
  SkClass ** data_owner_class_pp // = nullptr
  ) const
  {
  // Try to find this instance member in this class or any of its superclasses
  uint32_t data_idx = 0;
  SkTypedName * typed_p = nullptr;
  SkClass * owner_class_p = const_cast<SkClass *>(this);
  do
    {
    if ((typed_p = owner_class_p->m_data.get(data_name, AMatch_first_found, &data_idx)) != nullptr)
      {
      if (data_idx_p)
        {
        *data_idx_p = data_idx + owner_class_p->get_inherited_instance_data_count();
        }
      if (data_owner_class_pp)
        {
        *data_owner_class_pp = owner_class_p;
        }

      return typed_p;
      }

    owner_class_p = owner_class_p->get_superclass();
    } while (owner_class_p);

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if an instance data member exists and if so its class type 
SkTypedNameRaw * SkClass::get_instance_data_type_raw(const ASymbol & data_name, uint32_t * data_idx_p, SkClass ** data_owner_class_pp /*= nullptr*/) const
  {
  // Try to find this instance member in this class
  uint32_t data_idx = 0;
  SkTypedNameRaw * typed_p = nullptr;
  SkClass * owner_class_p = const_cast<SkClass *>(this);
  do
    {
    if ((typed_p = owner_class_p->m_data_raw.get(data_name, AMatch_first_found, &data_idx)) != nullptr)
      {
      if (data_idx_p)
        {
        *data_idx_p = data_idx;
        }
      if (data_owner_class_pp)
        {
        *data_owner_class_pp = owner_class_p;
        }

      return typed_p;
      }

    owner_class_p = owner_class_p->get_superclass();
    } while (owner_class_p);

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if a class data member exists and if so its class type 
// and optionally where it was found.
SkTypedName * SkClass::get_class_data_type(
  const ASymbol & data_name,
  uint32_t * data_idx_p,         // = nullptr
  SkClass ** data_owner_class_pp // = nullptr
  ) const
  {
  // Try to find this class member in this class or any of its superclasses
  uint32_t data_idx = 0;
  SkTypedName * typed_p = nullptr;
  SkClass * owner_class_p = const_cast<SkClass *>(this);
  do
    {
    if ((typed_p = owner_class_p->m_class_data.get(data_name, AMatch_first_found, &data_idx)) != nullptr)
      {
      if (data_idx_p)
        {
        *data_idx_p = data_idx;
        }
      if (data_owner_class_pp)
        {
        *data_owner_class_pp = owner_class_p;
        }

      return typed_p;
      }

    owner_class_p = owner_class_p->get_superclass();
    } while (owner_class_p);

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Count how many instance data members exist in all superclasses combined
uint32_t SkClass::get_inherited_instance_data_count() const
  {
  uint32_t inherited_data_count = 0;
  SkClass * superclass_p;
  for (superclass_p = get_superclass(); superclass_p; superclass_p = superclass_p->get_superclass())
    { 
    inherited_data_count += superclass_p->m_data.get_length();
    }
  return inherited_data_count;
  }

//---------------------------------------------------------------------------------------
// Store flag if raw data has been resolved
void SkClass::set_raw_data_resolved(bool is_resolved)
  {
  if (is_resolved)
    {
    m_flags |= Flag_raw_resolved;
    }
  else
    {
    m_flags &= ~Flag_raw_resolved;
    }
  }

//---------------------------------------------------------------------------------------
// Resolve single raw data member
bool SkClass::resolve_raw_data(const ASymbol & name, tSkRawDataInfo raw_data_info)
  {
  uint32_t idx;
  if (m_data_raw.find(name, AMatch_first_found, &idx))
    {
    m_data_raw[idx]->m_raw_data_info = raw_data_info;
    return true;
    }
  SK_ERRORX(a_str_format("Cound not resolve data member '%s@%s'", get_name_cstr_dbg(), name.as_cstr_dbg()));
  return false;
  }

//---------------------------------------------------------------------------------------
// Resolve single raw data member
bool SkClass::resolve_raw_data(const char * name_p, tSkRawDataInfo raw_data_info)
  {
  return resolve_raw_data(ASymbol::create_existing(name_p), raw_data_info);
  }

//---------------------------------------------------------------------------------------
// Resolve all raw data members via callback into the engine
void SkClass::resolve_raw_data()
  {
  if (!(m_flags & Flag_raw_resolved) && ms_raw_resolve_f)
    {
    if ((*ms_raw_resolve_f)(this))
      {
      m_flags |= Flag_raw_resolved;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Recurse through all classes and call back into the engine to resolve their raw data
void SkClass::resolve_raw_data_recurse()
  {
  if (ms_raw_resolve_f)
    {
    if (!(m_flags & Flag_raw_resolved))
      {
      if ((*ms_raw_resolve_f)(this))
        {
        m_flags |= Flag_raw_resolved;
        }
      }

    for (auto subclass_p : m_subclasses)
      {
      subclass_p->resolve_raw_data_recurse();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Get inherited instance or class method and set appropriate receiver.
// Returns:    Method found or nullptr if not found
// Arg         method_name - 
// Arg         receiver_pp - pointer to address of receiver.  It should be preset with an
//             instance receiver and it will be changed to the metaclass if a matching
//             method is a class method.
// Arg         caller_p - [Used for debugging]  Object that called/invoked this method and
//             that may await a result.  If it is nullptr, then there is no object that needs
//             to be notified when this invocation is complete.
// Author(s):   Conan Reis
SkMethodBase * SkClass::find_method_inherited_receiver(
  const ASymbol & method_name,
  SkInstance **   receiver_pp,
  SkInvokedBase * caller_p
  ) const
  {
  // Look for instance method
  SkMethodBase * method_p = find_instance_method_inherited(method_name);

  if (method_p == nullptr)
    {
    // Try to find a class method
    method_p = find_class_method_inherited(method_name);

    if (method_p)
      {
      // Found class method so use class as receiver.
      *receiver_pp = const_cast<SkMetaClass *>(&m_metaclass);
      }
    else
      {
      // $Note - CReis This should not need to be checked here at runtime - the parser
      // should have already prevented any misuse.  However an invalid type cast can be
      // used to fool the parser.
      SK_ERROR_INFO(
        a_str_format(
          "Requested non-existing method %s() from %s!\n"
          "[Bad type cast or didn't check for nil?]",
          method_name.as_cstr_dbg(), (*receiver_pp)->as_code().as_cstr()),
        caller_p);

      return nullptr;
      }
    }

  return method_p;
  }

//---------------------------------------------------------------------------------------
// Returns number of classes in the class hierarchy using this class as a
//             branch point - i.e. this class + its subclasses recursively.  Optionally
//             skips demand loaded classes.
// Author(s):   Conan Reis
uint32_t SkClass::get_class_recurse_count(bool skip_demand_loaded) const
  {
  uint32_t class_count = 0u;

  if (!skip_demand_loaded || !is_demand_loaded())
    {
    class_count++;

    uint32_t sub_count = m_subclasses.get_length();

    // Count subclasses
    if (sub_count)
      {
      SkClass ** classes_pp     = m_subclasses.get_array();
      SkClass ** classes_end_pp = classes_pp + sub_count;

      for (; classes_pp < classes_end_pp; classes_pp++)
        {
        class_count += (*classes_pp)->get_class_recurse_count(skip_demand_loaded);
        }
      }
    }

  return class_count;
  }

//---------------------------------------------------------------------------------------
// Recursively appends all subclasses/descendants of this class to classes_p
//             for any subclass that is not already in classes_p.
// See:        get_superclasses_all()
// Author(s):   Conan Reis
void SkClass::get_subclasses_all(tSkClasses * classes_p) const
  {
  uint32_t sub_count = m_subclasses.get_length();

  // Count subclasses
  if (sub_count)
    {
    classes_p->append_absent_all(m_subclasses);

    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + sub_count;

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      (*classes_pp)->get_subclasses_all(classes_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Recursively appends all superclasses/ancestors of this class to classes_p
//             for any superclass that is not already in classes_p.
// See:        get_subclasses_all()
// Author(s):   Conan Reis
void SkClass::get_superclasses_all(tSkClasses * classes_p) const
  {
  SkClass * super_p = m_superclass_p;

  while (super_p)
    {
    classes_p->append_absent(*super_p);
    super_p = super_p->m_superclass_p;
    }
  }

//---------------------------------------------------------------------------------------
// Determines the depth of this class - i.e. how many superclasses it is away from the
// `Object` class with `Object = 0`.
// 
// See: get_class_depth_at()
uint32_t SkClass::get_superclass_depth() const
  {
  uint32_t  depth   = 0u;
  SkClass * super_p = m_superclass_p;

  while (super_p)
    {
    depth++;
    super_p = super_p->m_superclass_p;
    }

  return depth;
  }

//---------------------------------------------------------------------------------------
// Gets superclass at specified depth with of this class - i.e. how many superclasses it is away from the
// `Object` class with `Object = 0`.
// 
// See: get_superclass_depth()
SkClass * SkClass::get_class_depth_at(uint32_t depth) const
  {
  uint32_t super_depth = get_superclass_depth();

  if (depth > super_depth)
    {
    return nullptr;
    }

  const SkClass * class_p = this;

  while (super_depth > depth)
    {
    super_depth--;
    class_p = class_p->m_superclass_p;
    }

  return const_cast<SkClass *>(class_p);
  }

//---------------------------------------------------------------------------------------
// Gets a string representing the chain of this class' and its superclasses' names
// starting from `Object` and separated by forward slashes
// If scripts_path_depth is given, path is truncated (if necessary) and the last class is of the form 'parent.child'
AString SkClass::get_class_path_str(int32_t scripts_path_depth) const
  {
  SK_ASSERTX(scripts_path_depth >= 0, "SkClass::get_class_path_str() only supports overlays with specified path depth.");

  // Determine total size of string so we have to allocate memory only once
  int32_t class_count = 0;
  uint32_t est_length = 0;
  for (const SkClass * class_p = this; class_p; class_p = class_p->get_superclass())
    {
    ++class_count;
    est_length += (uint32_t)::strlen(class_p->get_name_cstr()) + 1;
    }

  // Assemble string from back to front
  AString class_path;
  class_path.ensure_size(est_length);
  class_path.set_length(est_length - 1);
  char * end_p = &class_path[est_length - 1];
  int32_t path_count = 0;
  for (const SkClass * class_p = this; class_p; class_p = class_p->get_superclass(), ++path_count)
    {
    // Always use the last two class names, and the first scripts_path_depth class names
    if (path_count < 2 || path_count >= class_count - scripts_path_depth)
      {
      const char * name_p = class_p->get_name_cstr();
      uint32_t name_length = (uint32_t)::strlen(name_p);
      char * start_p = end_p - name_length;
      ::memcpy(start_p, name_p, name_length);
      char separator = (path_count == 0 && class_count > scripts_path_depth + 1) ? '.' : '/';
      if (class_p->get_superclass()) *--start_p = separator;
      end_p = start_p;
      }
    }
  SK_ASSERTX(end_p >= &class_path[0], "Must match");
  if (end_p > &class_path[0])
    {
    uint32_t new_length = (uint32_t)(&class_path[0] + est_length - end_p);
    ::memmove(&class_path[0], end_p, new_length);
    class_path.set_length(new_length - 1);
    }

  return class_path;
  }

//---------------------------------------------------------------------------------------
// Iterates through this class *and all sub-classes* applying operation supplied by
// apply_class_p and exiting early if its invoke() returns AIterateResult_early_exit.
//
// See Also  SkClass: :iterate_expressions_recurse()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkClass::iterate_recurse(
  AFunctionArgRtnBase<SkClass *, eAIterateResult> * apply_class_p,
  eAHierarchy hierarchy // = AHierarchy__all
  )
  {
  if ((hierarchy & AHierarchy_current) && apply_class_p->invoke(this))
    {
    return AIterateResult_early_exit;
    }

  if (hierarchy & AHierarchy_recurse)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    while (classes_pp < classes_end_pp)
      {
      if ((*classes_pp)->iterate_recurse(apply_class_p, AHierarchy__all))
        {
        // Exit early
        return AIterateResult_early_exit;
        }

      classes_pp++;
      }
    }

  // Full iteration performed
  return AIterateResult_entire;
  }

//---------------------------------------------------------------------------------------
// Iterates through this class *and all sub-classes* applying operation supplied by
// apply_class_p.
//
// See Also  SkClass: :iterate_expressions_recurse()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SkClass::iterate_recurse(
  AFunctionArgBase<SkClass *> * apply_class_p,
  eAHierarchy hierarchy // = AHierarchy__all
  )
  {
  if (hierarchy & AHierarchy_current)
    {
    apply_class_p->invoke(this);
    }

  if (hierarchy & AHierarchy_recurse)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    while (classes_pp < classes_end_pp)
      {
      (*classes_pp)->iterate_recurse(apply_class_p, AHierarchy__all);
      classes_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Gets sibling class (sharing the same superclass) that comes next (using 
//             name id as order).
// Returns:    SkClass
// See:        next_class(), get_superclass(), get_subclasses()
// Author(s):   Conan Reis
SkClass * SkClass::next_sibling() const
  {
  if (m_superclass_p == nullptr)
    {
    return nullptr;
    }

  const tSkClasses & subclasses  = m_superclass_p->m_subclasses;
  uint32_t           sub_indexes = subclasses.get_length() - 1u;

  if (sub_indexes == 0u)
    {
    return nullptr;
    }

  uint32_t find_idx;

  subclasses.find(*this, AMatch_first_found, &find_idx);

  return (find_idx < sub_indexes)
    ? subclasses.get_array()[find_idx + 1u]
    : nullptr;
  }

//---------------------------------------------------------------------------------------

void SkClass::remove_subclass(SkClass * subclass_p)
  {
  // Unlink from hierarchy
  m_subclasses.remove(*subclass_p);
  subclass_p->m_superclass_p = nullptr;
  subclass_p->m_total_data_count = subclass_p->m_data.get_length();
  subclass_p->m_total_class_data_count = subclass_p->m_class_data.get_length();

  // Recurse and cut loose all subclasses as well
  while (SkClass * subsubclass_p = subclass_p->m_subclasses.get_first())
    {
    subclass_p->remove_subclass(subsubclass_p);
    }
  }

//---------------------------------------------------------------------------------------
// Iterates/recurses through class hierarchy after this class in subclass/
//             depth-first order.
// Returns:    SkClass
// Arg         root_p - root to iterate from
// Examples:   
//   SkClass * my_root_p = SkBrain::get_class("MyClass");
//   SkClass * current_p = my_root_p;
// 
//   do 
//     {
//     do_stuff(current_p)
//     current_p = current_p->next_class(my_root_p);
//     }
//   while (current_p);
//
// See:        next_sibling(), get_superclass(), get_subclasses()
// Author(s):   Conan Reis
SkClass * SkClass::next_class(SkClass * root_p) const
  {
  // $Revisit - CReis More efficient to use an m_next_p member in SkClass

  // Subclasses?
  SkClass * subclass_p = m_subclasses.get_first();

  if (subclass_p)
    {
    return subclass_p;
    }

  SkClass * sibling_p;
  SkClass * class_p = const_cast<SkClass *>(this);

  while (class_p != root_p)
    {
    sibling_p = class_p->next_sibling();

    if (sibling_p)
      {
      return sibling_p;
      }

    class_p = class_p->m_superclass_p;
    }

  // End of branch
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if this Class is compatible with the specified class type
//             - i.e. can this class be passed as an argument to type_p.
// Returns:    true if compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkClass::is_class_type(const SkClassDescBase * type_p) const
  {
  switch (type_p->get_class_type())
    {
    case SkClassType_class:
      return (type_p == SkBrain::ms_object_class_p)
        || is_class(*static_cast<const SkClass *>(type_p));

    case SkClassType_typed_class:
      {
      const SkTypedClass * tclass_p = static_cast<const SkTypedClass *>(type_p);

      // Only accept if type_p is MatchType{Object}
      return (tclass_p->m_item_type_p == SkBrain::ms_object_class_p)
        && is_class(*tclass_p->m_class_p);
      }

    case SkClassType_metaclass:
      // $Revisit - CReis In the future "Class" may be disallowed replaced with "<Object>".
      return ((this == SkBrain::ms_class_class_p)
        && (static_cast<const SkMetaClass *>(type_p)->get_class_info() == SkBrain::ms_object_class_p));

    case SkClassType_class_union:
      return static_cast<const SkClassUnion *>(type_p)->is_valid_param_for(this);
  
    default:
      // SkClassType_invokable_class
      // $Revisit - CReis Currently there is no way to have a "match all" parameter list so
      // a generic non-parameter list Closure will not match any SkInvokableClass.
      return false;
    }
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) instance method with the given name and
//             parameters.  The method will use this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Author(s):   Conan Reis
void SkClass::append_instance_method(SkMethodBase * method_p, bool * has_signature_changed_p)
  {
  method_p->set_scope(this);

  SkMethodBase * old_method_p = m_methods.append_replace(*method_p);

  // Set shortcut to method if it is a destructor.
  if (method_p->get_name() == ASymbolX_dtor)
    {
    set_destructor(method_p);
    }

  if (old_method_p)
    {
    // Retain vtable index if any
    int16_t vtable_index = old_method_p->get_vtable_index();
    if (vtable_index >= 0)
      {
      method_p->set_vtable_index(vtable_index);
      m_vtable_i.set_at(vtable_index, method_p);
      }

    // Check if signature changed
    if (has_signature_changed_p)
      {
      *has_signature_changed_p = !(method_p->get_params() == old_method_p->get_params());
      }

    delete old_method_p;
    }
  else if (has_signature_changed_p)
    {
    // If the method is brand new, consider it a change in signature
    *has_signature_changed_p = true;
    }
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) class method with the given name and
//             parameters.  The method will use this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Author(s):   Conan Reis
void SkClass::append_class_method(SkMethodBase * method_p, bool * has_signature_changed_p)
  {
  method_p->set_scope(this);

  SkMethodBase * old_method_p = m_class_methods.append_replace(*method_p);

  if (old_method_p)
    {
    // Retain vtable index if any
    int16_t vtable_index = old_method_p->get_vtable_index();
    if (vtable_index >= 0)
      {
      method_p->set_vtable_index(vtable_index);
      m_vtable_c.set_at(vtable_index, method_p);
      }

    // Check if signature changed
    if (has_signature_changed_p)
      {
      *has_signature_changed_p = !(method_p->get_params() == old_method_p->get_params());
      }

    delete old_method_p;
    }
  else if (has_signature_changed_p)
    {
    // If the method is brand new, consider it a change in signature
    *has_signature_changed_p = true;
    }
  }

//---------------------------------------------------------------------------------------
// Associates method with the given name with the C++ function to call when
//             invoking / calling the method.  The method will use this class as its
//             class scope.
// Arg         method_name - identifier name for the method 
// Arg         atomic_f - Atomic function to call when the method is invoked.
// Arg         flags - See eSkBindFlag.
// See:        register_method_mthd(), append_instance_method(), tSkMethodFunc
// Notes:      This registers both instance and class methods.
// Author(s):   Conan Reis
void SkClass::register_method_func(
  const ASymbol & method_name,
  tSkMethodFunc   atomic_f,
  eSkBindFlag     flags // = SkBindFlag_default
  )
  {
  // Look for method
  SkMethodBase * method_p = (flags & SkBindFlag__class)
    ? m_class_methods.get(method_name)
    : m_methods.get(method_name);

  #if defined(SK_RUNTIME_RECOVER)
    if (method_p == nullptr)
      {
      SK_ERROR_ID(AErrMsg(a_cstr_format("SkookumScript Parameters for the method named '%s@%s' are not available so no atomic C++ function can be bound to it.\n[It is either a name typo or it needs a script file that specifies the parameters.]", m_name.as_cstr_dbg(), method_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_method_regd, SkClass);

      return;
      }
  #endif

  // If this method needs to append user data, add one entry to the required data storage
  if ((flags & SkBindFlag__arg_user_data) && !method_p->is_bound())
    {
    method_p->set_invoked_data_array_size(method_p->get_invoked_data_array_size() + 1);
    }

  #if (SKOOKUM & SK_DEBUG)
    if (((flags & SkBindFlag__rebind) == 0u) && method_p->is_bound())
      {
      SK_ERROR_ID(
        AErrMsg(
          a_cstr_format("The %s method named '%s@%s'\n has already been bound / registered to %s\nand so cannot be bound again internally as an atomic C++ function call.",
            (flags & SkBindFlag__class) ? "class" : "instance",
            m_name.as_cstr_dbg(),
            method_name.as_cstr_dbg(),
            (method_p->get_invoke_type() == SkInvokable_method) ? "custom code" : "a C++ atomic call"),
          AErrLevel_notify),
        ErrId_nonexistent_method_regd,
        SkClass);
      }
    else
      if (method_p->get_invoke_type() == SkInvokable_method)
        {
        SK_ERROR_ID(
          AErrMsg(
            a_cstr_format(
              "A %s method named '%s@%s'\n"
              "exists, but it is registered as a custom script method rather\n"
              "than an atomic (C++) method.  The custom method will override the\n"
              "C++ method.",
              (flags & SkBindFlag__class) ? "class" : "instance",
              m_name.as_cstr_dbg(),
              method_name.as_cstr_dbg()),
            AErrLevel_notify),
          ErrId_nonexistent_method_regd,
          SkClass);
        }
      else
  #endif
        {
        if (method_p->get_invoke_type() != SkInvokable_method_func)
          {
          // Swap func/mthd types using the same memory.  The mthd types can be larger so
          // they are the default.
          // $Revisit - CReis This swapping could be avoided if the compiler could know
          // the type of C++ code being bound.
          SkClass *             scope_p                 = method_p->get_scope();
          ARefPtr<SkParameters> params_p                = &method_p->get_params();
          uint32_t              annotation_flags        = method_p->get_annotation_flags();
          uint16_t              invoked_data_array_size = method_p->get_invoked_data_array_size();
          uint16_t              user_data               = method_p->get_user_data();

          method_p->set_params(nullptr);

          new (method_p) SkMethodFunc(method_name, scope_p, params_p, annotation_flags, atomic_f);

          method_p->set_invoked_data_array_size(invoked_data_array_size);
          method_p->set_user_data(user_data);
          }
        else
          {
          static_cast<SkMethodFunc *>(method_p)->set_function(atomic_f);
          }
        }
  }

//---------------------------------------------------------------------------------------
// Associates method with the given name with the C++ function to call when
//             invoking / calling the method.  The method will use this class as its
//             class scope.
// Arg         method_name - string identifier name for the method 
// Arg         atomic_f - Atomic function to call when the method is invoked.
// Arg         flags - See eSkBindFlag.
// See:        register_method_mthd(), append_instance_method(), tSkMethodFunc
// Notes:      This registers both instance and class methods.
// Author(s):   Conan Reis
void SkClass::register_method_func(
  const char *  method_name_p,
  tSkMethodFunc atomic_f,
  eSkBindFlag   flags // = SkBindFlag_default
  )
  {
  register_method_func(
    ASymbol::create(method_name_p),
    atomic_f,
    flags);
  }

//---------------------------------------------------------------------------------------
// Associates method with the given name with the C++ method to call when
//             invoking / calling the method.  The method will use this class as its
//             class scope.
// Arg         method_name - identifier name for the method 
// Arg         atomic_m - Atomic method to call when the method is invoked.
// Arg         flags - See eSkBindFlag.
// See:        register_method_func(), append_instance_method(), tSkMethodMthd
// Notes:      This registers both instance and class methods.
// Author(s):   Conan Reis
void SkClass::register_method_mthd(
  const ASymbol & method_name,
  tSkMethodMthd   atomic_m,
  eSkBindFlag     flags // = SkBindFlag_default
  )
  {
  // Look for method
  uint32_t       pos;
  SkMethodBase * method_p = (flags & SkBindFlag__class)
    ? m_class_methods.get(method_name, AMatch_first_found, &pos)
    : m_methods.get(method_name, AMatch_first_found, &pos);

  #if defined(SK_RUNTIME_RECOVER)
    if (method_p == nullptr)
      {
      SK_ERROR_ID(AErrMsg(a_cstr_format("Parameters for the method named '%s@%s' are not specified so no atomic C++ function can be bound to it.\n[It is either a name typo or it needs a parameters prototype file.]", m_name.as_cstr_dbg(), method_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_method_regd, SkClass);

      return;
      }
  #endif

  // If this method needs to append user data, add one entry to the required data storage
  if ((flags & SkBindFlag__arg_user_data) && !method_p->is_bound())
    {
    method_p->set_invoked_data_array_size(method_p->get_invoked_data_array_size() + 1);
    }

  #if (SKOOKUM & SK_DEBUG)
    if (((flags & SkBindFlag__rebind) == 0u) && method_p->is_bound())
      {
      SK_ERROR_ID(
        AErrMsg(
          a_cstr_format("The %s method named '%s@%s'\n has already been bound / registered to %s\nand so cannot be bound again internally as an atomic C++ function call.",
            (flags & SkBindFlag__class) ? "class" : "instance",
            m_name.as_cstr_dbg(),
            method_name.as_cstr_dbg(),
            (method_p->get_invoke_type() == SkInvokable_method) ? "custom code" : "a C++ atomic call"),
          AErrLevel_notify),
        ErrId_nonexistent_method_regd,
        SkClass);
      }
    else
      if (method_p->get_invoke_type() == SkInvokable_method)
        {
        SK_ERROR_ID(
          AErrMsg(
            a_cstr_format(
              "A %s method named '%s@%s'\n"
              "exists, but it is registered as a custom/compound method rather\n"
              "than an atomic (C++) method.  The custom method will override the\n"
              "C++ method.",
              (flags & SkBindFlag__class) ? "class" : "instance",
              m_name.as_cstr_dbg(),
              method_name.as_cstr_dbg()),
            AErrLevel_notify),
          ErrId_nonexistent_method_regd,
          SkClass);
        }
      else
  #endif
        {
        static_cast<SkMethodMthd *>(method_p)->set_function(atomic_m);
        }
  }

//---------------------------------------------------------------------------------------
// Associates method with the given name with the C++ method to call when
//             invoking / calling the method.  The method will use this class as its
//             class scope.
// Arg         method_name - identifier name for the method 
// Arg         atomic_m - Atomic method to call when the method is invoked.
// Arg         flags - See eSkBindFlag.
// See:        register_method_func(), append_instance_method(), tSkMethodMthd
// Notes:      This registers both instance and class methods.
// Author(s):   Conan Reis
void SkClass::register_method_mthd(
  const char *  method_name_p,
  tSkMethodMthd atomic_m,
  eSkBindFlag   flags // = SkBindFlag_default
  )
  {
  register_method_mthd(
    ASymbol::create(method_name_p),
    atomic_m,
    flags);
  }

//---------------------------------------------------------------------------------------
// Associates a number of methods with their C++ binding
// See:   register_method_func
void SkClass::register_method_func_bulk(const MethodInitializerFunc * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const MethodInitializerFunc & binding = bindings_p[i];
    ASymbol method_name = ASymbol::create_existing(binding.m_method_name_p);
    SK_ASSERTX(method_name != ASymbol::ms_null, a_str_format("Tried to bind unknown method '%s'!", binding.m_method_name_p));
    register_method_func(method_name, binding.m_atomic_f, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of methods with their C++ binding
// See:   register_method_func
void SkClass::register_method_func_bulk(const MethodInitializerFuncId * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const MethodInitializerFuncId & binding = bindings_p[i];
    ASymbol method_name = ASymbol::create_existing(binding.m_method_name_id);
    SK_ASSERTX(method_name != ASymbol::ms_null, "Tried to bind unknown method!");
    register_method_func(method_name, binding.m_atomic_f, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of methods with their C++ binding
// See:   register_method_mthd
void SkClass::register_method_mthd_bulk(const MethodInitializerMthd * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const MethodInitializerMthd & binding = bindings_p[i];
    ASymbol method_name = ASymbol::create_existing(binding.m_method_name_p);
    SK_ASSERTX(method_name != ASymbol::ms_null, a_str_format("Tried to bind unknown method '%s'!", binding.m_method_name_p));
    register_method_mthd(method_name, binding.m_atomic_m, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of methods with their C++ binding
// See:   register_method_mthd
void SkClass::register_method_mthd_bulk(const MethodInitializerMthdId * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const MethodInitializerMthdId & binding = bindings_p[i];
    ASymbol method_name = ASymbol::create_existing(binding.m_method_name_id);
    SK_ASSERTX(method_name != ASymbol::ms_null, "Tried to bind unknown method!");
    register_method_mthd(method_name, binding.m_atomic_m, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Appends coroutine (or replaces existing) with the given name and parameters.
//             The coroutine will use this class as its class scope.
// Arg         coroutine_p - coroutine to append / replace with
// Notes:      Used when parsing / reading in compiled binary code
// Author(s):   Conan Reis
void SkClass::append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p)
  {
  coroutine_p->set_scope(this);

  SkCoroutineBase * old_coroutine_p = m_coroutines.append_replace(*coroutine_p);

  if (old_coroutine_p)
    {
    // Retain vtable index if any
    int16_t vtable_index = old_coroutine_p->get_vtable_index();
    if (vtable_index >= 0)
      {
      coroutine_p->set_vtable_index(vtable_index);
      m_vtable_i.set_at(vtable_index, coroutine_p);
      }

    // Check if signature changed
    if (has_signature_changed_p)
      {
      *has_signature_changed_p = !(coroutine_p->get_params() == old_coroutine_p->get_params());
      }

    delete old_coroutine_p;
    }
  else if (has_signature_changed_p)
    {
    // If the method is brand new, consider it a change in signature
    *has_signature_changed_p = true;
    }
  }

//---------------------------------------------------------------------------------------
// Associates coroutine with the given name with the function to call when
//             invoking / updating the coroutine.  The coroutine will use this class as
//             its class scope.
// Arg         coroutine_name - identifier name for the coroutine 
// Arg         update_f - Atomic function to call when the coroutine is updating after it
//             is invoked.
// Arg         flags - See eSkBindFlag.  Currently SkBindFlag__class has no effect since
//             all coroutines are instance members.
// See:        Description for tSkCoroutineFunc
// Author(s):   Conan Reis
void SkClass::register_coroutine_func(
  const ASymbol &  coroutine_name,
  tSkCoroutineFunc update_f,
  eSkBindFlag      flags // = SkBindFlag_default
  )
  {
  SkCoroutineBase * coroutine_p = m_coroutines.get(coroutine_name);

  #if defined(SK_RUNTIME_RECOVER)
    if (coroutine_p == nullptr)
      {
      SK_ERROR_ID(AErrMsg(a_cstr_format("A coroutine named '%s@%s' does not exist so no atomic C++ function can be bound to it.\n[It is either a name typo or it needs a parameters prototype file.]", m_name.as_cstr_dbg(), coroutine_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_coroutine, SkClass);

      return;
      }
  #endif

  // If this coroutine needs to append user data, add one entry to the required data storage
  if ((flags & SkBindFlag__arg_user_data) && !coroutine_p->is_bound())
    {
    coroutine_p->set_invoked_data_array_size(coroutine_p->get_invoked_data_array_size() + 1);
    }

  #if (SKOOKUM & SK_DEBUG)
    if (((flags & SkBindFlag__rebind) == SkBindFlag__none) && coroutine_p->is_bound())
      {
      SK_ERROR_ID(
        AErrMsg(
          a_cstr_format("The coroutine named '%s@%s'\n has already been bound / registered to %s\nand so cannot be bound again internally as an atomic C++ function call.",
            m_name.as_cstr_dbg(),
            coroutine_name.as_cstr_dbg(),
            (coroutine_p->get_invoke_type() == SkInvokable_coroutine) ? "custom code" : "a C++ atomic call"),
          AErrLevel_notify),
        ErrId_nonexistent_coroutine,
        SkClass);
      }
    else
      if (coroutine_p->get_invoke_type() == SkInvokable_coroutine)
        {
        SK_ERROR_ID(AErrMsg(a_cstr_format("A coroutine named '%s@%s'\nexists, but it is a custom script coroutine rather than an atomic (C++) coroutine.", m_name.as_cstr_dbg(), coroutine_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_coroutine, SkClass);
        }
      else
  #endif
        {
        if (coroutine_p->get_invoke_type() != SkInvokable_coroutine_func)
          {
          // Swap func/mthd types using the same memory.  The mthd types can be larger so
          // they are the default.
          // $Revisit - CReis This swapping could be avoided if the compiler could know
          // the type of C++ code being bound.
          SkClass *             scope_p                 = coroutine_p->get_scope();
          ARefPtr<SkParameters> params_p                = &coroutine_p->get_params();
          uint32_t              annotation_flags        = coroutine_p->get_annotation_flags();
          uint16_t              invoked_data_array_size = coroutine_p->get_invoked_data_array_size();
          uint16_t              user_data               = coroutine_p->get_user_data();

          coroutine_p->set_params(nullptr);

          new (coroutine_p) SkCoroutineFunc(coroutine_name, scope_p, params_p, annotation_flags, update_f);

          coroutine_p->set_invoked_data_array_size(invoked_data_array_size);
          coroutine_p->set_user_data(user_data);
          }
        else
          {
          static_cast<SkCoroutineFunc *>(coroutine_p)->set_update(update_f);
          }
        }
  }

//---------------------------------------------------------------------------------------
// Associates coroutine with the given name with the function to call when
//             invoking / updating the coroutine.  The coroutine will use this class as
//             its class scope.
// Arg         coroutine_name - identifier name for the coroutine 
// Arg         update_f - Atomic function to call when the coroutine is updating after it
//             is invoked.
// Arg         flags - See eSkBindFlag.  Currently SkBindFlag__class has no effect since
//             all coroutines are instance members.
// See:        Description for tSkCoroutineFunc
// Author(s):   Conan Reis
void SkClass::register_coroutine_func(
  const char *     coroutine_name_p,
  tSkCoroutineFunc update_f,
  eSkBindFlag      flags // = SkBindFlag_default
  )
  {
  register_coroutine_func(ASymbol::create(coroutine_name_p), update_f, flags);
  }

//---------------------------------------------------------------------------------------
// Associates coroutine with the given name with the method to call when
//             invoking / updating the coroutine.  The coroutine will use this class as
//             its class scope.
// Arg         coroutine_name - identifier name for the coroutine 
// Arg         update_m - Atomic method to call when the coroutine is updating after it is
//             invoked.
// Arg         flags - See eSkBindFlag.  Currently SkBindFlag__class has no effect since
//             all coroutines are instance members.
// See:        Description for tSkCoroutineMthd
// Author(s):   Conan Reis
void SkClass::register_coroutine_mthd(
  const ASymbol & coroutine_name,
  tSkCoroutineMthd update_m,
  eSkBindFlag     flags // = SkBindFlag_default
  )
  {
  SkCoroutineBase * coroutine_p = m_coroutines.get(coroutine_name);

  #if defined(SK_RUNTIME_RECOVER)
    if (coroutine_p == nullptr)
      {
      SK_ERROR_ID(AErrMsg(a_cstr_format("A coroutine named '%s@%s' does not exist so no atomic C++ method can be bound to it.\n[It is either a name typo or it needs a parameters prototype file.]", m_name.as_cstr_dbg(), coroutine_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_coroutine, SkClass);

      return;
      }

    // $Revisit - CReis This might cause problems for methods that are optimized out, but it probably should be checked.
    //if (update_m == nullptr)
    //  {
    //  SK_ERROR_ID(AErrMsg(a_cstr_format("Tried to register a nullptr C++ method to coroutine named '%s@%s'.]", m_name.as_cstr_dbg(), coroutine_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_coroutine, SkClass);
    //
    //  return;
    //  }
  #endif

  // If this coroutine needs to append user data, add one entry to the required data storage
  if ((flags & SkBindFlag__arg_user_data) && !coroutine_p->is_bound())
    {
    coroutine_p->set_invoked_data_array_size(coroutine_p->get_invoked_data_array_size() + 1);
    }

  #if (SKOOKUM & SK_DEBUG)
    if (((flags & SkBindFlag__rebind) == SkBindFlag__none) && coroutine_p->is_bound())
      {
      SK_ERROR_ID(
        AErrMsg(
          a_cstr_format("The coroutine named '%s@%s'\n has already been bound / registered to %s\nand so cannot be bound again internally as an atomic C++ method call.",
            m_name.as_cstr_dbg(),
            coroutine_name.as_cstr_dbg(),
            (coroutine_p->get_invoke_type() == SkInvokable_coroutine) ? "custom code" : "a C++ atomic call"),
          AErrLevel_notify),
        ErrId_nonexistent_coroutine,
        SkClass);
      }
    else
      if (coroutine_p->get_invoke_type() == SkInvokable_coroutine)
        {
        SK_ERROR_ID(AErrMsg(a_cstr_format("A coroutine named '%s@%s'\nexists, but it is a custom script coroutine rather than an atomic (C++) coroutine.", m_name.as_cstr_dbg(), coroutine_name.as_cstr_dbg()), AErrLevel_notify), ErrId_nonexistent_coroutine, SkClass);
        }
      else
  #endif
        {
        static_cast<SkCoroutineMthd *>(coroutine_p)->set_update(update_m);
        }
  }

//---------------------------------------------------------------------------------------
// Associates coroutine with the given name with the method to call when
//             invoking / updating the coroutine.  The coroutine will use this class as
//             its class scope.
// Arg         coroutine_name - identifier name for the coroutine 
// Arg         update_m - Atomic method to call when the coroutine is updating after it is
//             invoked.
// Arg         flags - See eSkBindFlag.  Currently SkBindFlag__class has no effect since
//             all coroutines are instance members.
// See:        Description for tSkCoroutineMthd
// Author(s):   Conan Reis
void SkClass::register_coroutine_mthd(
  const char *    coroutine_name_p,
  tSkCoroutineMthd update_m,
  eSkBindFlag     flags // = SkBindFlag_default
  )
  {
  register_coroutine_mthd(ASymbol::create(coroutine_name_p), update_m, flags);
  }

//---------------------------------------------------------------------------------------
// Associates a number of coroutines with their C++ binding
// See:   register_coroutine_func
void SkClass::register_coroutine_func_bulk(const CoroutineInitializerFunc * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const CoroutineInitializerFunc & binding = bindings_p[i];
    ASymbol coroutine_name = ASymbol::create_existing(binding.m_coroutine_name_p);
    SK_ASSERTX(coroutine_name != ASymbol::ms_null, a_str_format("Tried to bind unknown coroutine '%s'!", binding.m_coroutine_name_p));
    register_coroutine_func(coroutine_name, binding.m_atomic_f, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of coroutines with their C++ binding
// See:   register_coroutine_func
void SkClass::register_coroutine_func_bulk(const CoroutineInitializerFuncId * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const CoroutineInitializerFuncId & binding = bindings_p[i];
    ASymbol coroutine_name = ASymbol::create_existing(binding.m_coroutine_name_id);
    SK_ASSERTX(coroutine_name != ASymbol::ms_null, "Tried to bind unknown coroutine!");
    register_coroutine_func(coroutine_name, binding.m_atomic_f, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of coroutines with their C++ binding
// See:   register_coroutine_mthd
void SkClass::register_coroutine_mthd_bulk(const CoroutineInitializerMthd * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const CoroutineInitializerMthd & binding = bindings_p[i];
    ASymbol coroutine_name = ASymbol::create_existing(binding.m_coroutine_name_p);
    SK_ASSERTX(coroutine_name != ASymbol::ms_null, a_str_format("Tried to bind unknown coroutine '%s'!", binding.m_coroutine_name_p));
    register_coroutine_mthd(coroutine_name, binding.m_atomic_m, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Associates a number of coroutines with their C++ binding
// See:   register_coroutine_mthd
void SkClass::register_coroutine_mthd_bulk(const CoroutineInitializerMthdId * bindings_p, uint32_t count, eSkBindFlag flags /*= SkBindFlag_default*/)
  {
  for (uint32_t i = 0; i < count; ++i)
    {
    const CoroutineInitializerMthdId & binding = bindings_p[i];
    ASymbol coroutine_name = ASymbol::create_existing(binding.m_coroutine_name_id);
    SK_ASSERTX(coroutine_name != ASymbol::ms_null, "Tried to bind unknown coroutine!");
    register_coroutine_mthd(coroutine_name, binding.m_atomic_m, flags);
    }
  }

//---------------------------------------------------------------------------------------
// Remove all instance data from this class and clear all references to those
//             data members from any subclasses.
// Author(s):   Conan Reis
void SkClass::remove_instance_data_all()
  {
  recurse_modify_total_data_count(-(int32_t)m_data.get_length());
  m_data.free_all();
  m_data_raw.free_all();
  }

//---------------------------------------------------------------------------------------
// Generate a order-sensitive CRC32 of all instance data type definitions
uint32_t SkClass::generate_crc32_instance_data() const
  {
  uint32_t crc = 0;

  for (const SkTypedName * data_p : m_data)
    {
    crc = AChecksum::generate_crc32_uint32(data_p->generate_crc32(), crc);
    }

  for (const SkTypedNameRaw * data_p : m_data_raw)
    {
    crc = AChecksum::generate_crc32_uint32(data_p->generate_crc32(), crc);
    }

  return crc;
  }

//---------------------------------------------------------------------------------------
// Generate a order-sensitive CRC32 of all class data type definitions
uint32_t SkClass::generate_crc32_class_data() const
  {
  uint32_t crc = 0;

  for (const SkTypedName * data_p : m_class_data)
    {
    crc = AChecksum::generate_crc32_uint32(data_p->generate_crc32(), crc);
    }

  return crc;
  }

//---------------------------------------------------------------------------------------
// Remove all class data from this class and clear all references to those
//             data members from any subclasses.
// Author(s):   Conan Reis
void SkClass::remove_class_data_all()
  {
  recurse_modify_total_class_data_count(-(int32_t)m_class_data.get_length());

  m_class_data.free_all();
  m_class_data.empty();
  m_class_data_values.empty();
  }

//---------------------------------------------------------------------------------------
// Adds an instance data member with the specified name to this class.
//             When an instance of this class or a instance of a subclass is constructed
//             it will have a data member with this name.
// Arg         name - name of instance data member.  It must be unique - no superclass
//             nor subclass may have it and there should be no class data member with the
//             same name either.
// Arg         type_p - class type of data member
// Arg         increment_total_data_count - if total_data_count should be recursively updated
// See:        append_class_data()
// Author(s):   Conan Reis
SkTypedName * SkClass::append_instance_data(
  const ASymbol &   name,
  SkClassDescBase * type_p,
  bool              increment_total_data_count
  )
  {
  SkTypedName * typed_p = SK_NEW(SkTypedName)(name, type_p);

  m_data.append(*typed_p);

  if (increment_total_data_count)
    {
    recurse_modify_total_data_count(1);
    }

  return typed_p;
  }

//---------------------------------------------------------------------------------------
// Adds a raw instance data member with the specified name to this class.
SkTypedNameRaw * SkClass::append_instance_data_raw(const ASymbol & name, SkClassDescBase * type_p, const AString & bind_name)
  {
  SkTypedNameRaw * typed_p = SK_NEW(SkTypedNameRaw)(name, type_p, bind_name);

  #if (SKOOKUM & SK_DEBUG)
    // Transfer old raw data info if exists
    if (ms_reparse_info.m_is_active)
      {
      SkTypedNameRaw * old_typed_p = ms_reparse_info.m_data_raw.pop(name);
      if (old_typed_p)
        {
        typed_p->m_raw_data_info = old_typed_p->m_raw_data_info;
        delete old_typed_p;
        }
      }
  #endif

  m_data_raw.append(*typed_p);

  return typed_p;
  }

//---------------------------------------------------------------------------------------
// Compute total number of raw instance data members by adding up
// all raw data members in this class and all super classes
uint32_t SkClass::compute_total_raw_data_count() const
  {
  uint32_t raw_data_count = 0;

  SkClass * class_p = const_cast<SkClass *>(this);
  do
    {
    raw_data_count += class_p->m_data_raw.get_length();
    class_p = class_p->get_superclass();
    } while (class_p);

  return raw_data_count;
  }

//---------------------------------------------------------------------------------------
// Adds a class data member with the specified name to this class.
// Arg         name - name of class data member.  It must be unique - no superclass nor
//             subclass may have it and there should be no class data member with the
//             same name either.
// Arg         type_p - class type of class data member
// Arg         increment_total_data_count - if total_data_count should be recursively updated
// See:        append_instance_data()
// Author(s):   Conan Reis
SkTypedName * SkClass::append_class_data(
  const ASymbol &   name,
  SkClassDescBase * type_p,
  bool              increment_total_data_count
  )
  {
  // nil does not need to be referenced/dereferenced
  SkTypedName * typed_p = SK_NEW(SkTypedName)(name, type_p);

  m_class_data.append(*typed_p);
  m_class_data_values.append_nil();

  if (increment_total_data_count)
    {
    recurse_modify_total_class_data_count(1);
    }

  return typed_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Determines if the specified instance data name is unique for this class - i.e. it
// is it an instance data member for this class or for its superclasses nor subclasses.
// 
// #See Also append_instance_data(), append_class_data()
// 
// #Notes
//   Useful to call just prior to calling append_instance_data() to ensure that the data
//   member is not already present.
//   Instance data names start with '@' and cannot conflict with class data names which
//   start with '@@'.
//   
// #Modifiers virtual - overriding from SkClassUnaryBase
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // nullptr if unique or the class that already has the data member
  SkClass *
SkClass::find_instance_data_scope(
  // data member name to determine the uniqueness of.
  const ASymbol & name,
  // See SkClassUnaryBase::ePath
  ePath check_path // = Path_super_sub
  )
  {
  // Check this class and possibly super classes
  bool check_super = (check_path & Path_superclass) != 0;
  SkClass * class_p = this;
  do
    {
    if (class_p->m_data.find(name))
      {
      return class_p;
      }

    if (class_p->m_data_raw.find(name))
      {
      return class_p;
      }

    class_p = class_p->m_superclass_p;

    } while (class_p && check_super);

  // Check sub classes if desired
  if (check_path & Path_subclass)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      class_p = (*classes_pp)->find_instance_data_scope(name, Path_subclass);

      if (class_p)
        {
        return class_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the specified instance data name is unique for this class - i.e. it
// is it an instance data member for this class or for its superclasses nor subclasses.
// 
// #See Also append_instance_data(), append_class_data()
// 
// #Notes
//   Useful to call just prior to calling append_instance_data() to ensure that the data
//   member is not already present.
//   Class data names start with '@@' and cannot conflict with instance data names which
//   start with '@'.
//   
// #Modifiers virtual - overriding from SkClassUnaryBase
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // nullptr if unique or the class that already has the data member
  SkClass *
SkClass::find_class_data_scope(
  // data member name to determine the uniqueness of.
  const ASymbol & name,
  // See SkClassUnaryBase::ePath
  ePath check_path // = Path_super_sub
  )
  {
  // Is data member present in this class?
  if (m_class_data.find(name))
    {
    return this;
    }

  // Id data member present in superclass?
  SkClass * class_p;

  if (check_path & Path_superclass)
    {
    class_p = m_superclass_p;
    
    while (class_p)
      {
      if (class_p->m_class_data.find(name))
        {
        return class_p;
        }

      class_p = class_p->m_superclass_p;
      }
    }

  if (check_path & Path_subclass)
    {
    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      class_p = (*classes_pp)->find_class_data_scope(name, Path_subclass);

      if (class_p)
        {
        return class_p;
        }
      }
    }

  return nullptr;
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//---------------------------------------------------------------------------------------
// Adjusts m_total_data_count on this class and propagates it through its
//             subclasses if it has any.
// Arg         delta - value to add to m_total_data_count of each class 
void SkClass::recurse_modify_total_data_count(int32_t delta)
  {
  m_total_data_count += delta;

  SkClass ** classes_pp     = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    (*classes_pp)->recurse_modify_total_data_count(delta);
    }
  }

//---------------------------------------------------------------------------------------
// Adjusts m_total_class_data_count on this class and propagates it through its
//             subclasses if it has any.
// Arg         delta - value to add to m_total_data_count of each class 
void SkClass::recurse_modify_total_class_data_count(int32_t delta)
  {
  m_total_class_data_count += delta;

  SkClass ** classes_pp = m_subclasses.get_array();
  SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    (*classes_pp)->recurse_modify_total_class_data_count(delta);
    }
  }

//---------------------------------------------------------------------------------------
// Calls the class method '!()' if it exists for this class.
// Author(s):   Conan Reis
void SkClass::invoke_class_ctor()
  {
  if (is_loaded())
    {
    // Call class constructor for this class (if one exists) before calling the class
    // constructors of any subclasses.
    SkMethodBase * method_p = m_class_methods.get(ASymbolX_ctor);

    #if (SKOOKUM & SK_DEBUG)
      if (method_p && method_p->is_bound())
    #else
      if (method_p)
    #endif
        {
        SkInvokedMethod imethod(nullptr, &m_metaclass, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

        SKDEBUG_ICALL_SET_INTERNAL(&imethod);
        SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_origin_class_ctors);

        method_p->invoke(&imethod);

        SKDEBUG_HOOK_SCRIPT_EXIT();
        }
    }
  }

//---------------------------------------------------------------------------------------
// Calls the class method '!()' if it exists for this class and
//             does the same to all of its subclasses if it has any.
// Notes:      called by append_class_data()
// Modifiers:   protected, virtual - override for custom behaviour
// Author(s):   Conan Reis
void SkClass::recurse_class_ctor()
  {
  if (is_loaded())
    {
    // Call class constructor for this class (if one exists) before calling the class
    // constructors of any subclasses.
    SkMethodBase * method_p = m_class_methods.get(ASymbolX_ctor);

    #if (SKOOKUM & SK_DEBUG)
      if (method_p && method_p->is_bound())
    #else
      if (method_p)
    #endif
        {
        SkInvokedMethod imethod(nullptr, &m_metaclass, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));
        SKDEBUG_ICALL_SET_INTERNAL(&imethod);
        method_p->invoke(&imethod);
        }

    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      (*classes_pp)->recurse_class_ctor();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Dereferences all class data for this class and does the same to all of its
//             subclasses if it has any.
// Notes:      called by append_class_data()
// Modifiers:   protected
// Author(s):   Conan Reis
void SkClass::recurse_class_dtor()
  {
  if (is_loaded())
    {
    // Recurse through subclasses first
    m_subclasses.apply_method(&SkClass::recurse_class_dtor);


    // $Vital - CReis [Incomplete] Call class destructor if one exists


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Dereference any class data
    SkInstance ** csdata_pp     = m_class_data_values.get_array();
    SkInstance ** csdata_end_pp = csdata_pp + m_class_data_values.get_length();

    for (; csdata_pp < csdata_end_pp; csdata_pp++)
      {
      (*csdata_pp)->dereference();
      // nil does not need to be referenced/dereferenced
      (*csdata_pp) = SkBrain::ms_nil_p;
      }
    }
  }

//---------------------------------------------------------------------------------------

void SkClass::recurse_replace_vtable_entry_i(int16_t vtable_index, SkInvokableBase * old_entry_p, SkInvokableBase * new_entry_p)
  {
  // This might be a new live updated class that does not have a this vtable entry yet
  // If not, don't worry and just skip, since method/coroutine calls have fallback lookup by name
  if (uint32_t(vtable_index) < m_vtable_i.get_length())
    {
    SkInvokableBase *& vtable_entry_p = m_vtable_i[vtable_index];
    if (vtable_entry_p == old_entry_p)
      {
      vtable_entry_p = new_entry_p;
      }
    }

  for (auto class_p : m_subclasses)
    {
    class_p->recurse_replace_vtable_entry_i(vtable_index, old_entry_p, new_entry_p);
    }
  }

//---------------------------------------------------------------------------------------

void SkClass::recurse_replace_vtable_entry_c(int16_t vtable_index, SkInvokableBase * old_entry_p, SkInvokableBase * new_entry_p)
  {
  // This might be a new live updated class that does not have a this vtable entry yet
  // If not, don't worry and just skip, since method/coroutine calls have fallback lookup by name
  if (uint32_t(vtable_index) < m_vtable_c.get_length())
    {
    SkInvokableBase *& vtable_entry_p = m_vtable_c[vtable_index];
    if (vtable_entry_p == old_entry_p)
      {
      vtable_entry_p = new_entry_p;
      }
    }

  for (auto class_p : m_subclasses)
    {
    class_p->recurse_replace_vtable_entry_c(vtable_index, old_entry_p, new_entry_p);
    }
  }

//---------------------------------------------------------------------------------------
// Sets shortcut to destructor recursively depending on method scope
// Arg         destructor_p - destructor method to set
// Author(s):   Conan Reis
void SkClass::set_destructor(SkMethodBase * destructor_p)
  {
  if ((m_destructor_p == nullptr) ||
    destructor_p->get_scope()->is_class(*m_destructor_p->get_scope()))
    {
    m_destructor_p = destructor_p;

    SkClass ** classes_pp     = m_subclasses.get_array();
    SkClass ** classes_end_pp = classes_pp + m_subclasses.get_length();

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      (*classes_pp)->set_destructor(destructor_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Recursively call the class constructor for this class and all its
//             subclasses.
// See:        invoke_class_dtor_recurse()
// Author(s):   Conan Reis
void SkClass::invoke_class_ctor_recurse()
  {
  SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_origin_class_ctors);

  recurse_class_ctor();

  SKDEBUG_HOOK_SCRIPT_EXIT();
  }

//---------------------------------------------------------------------------------------
// Recursively call the class destructor for this class and all its subclasses.
// See:        invoke_class_ctor_recurse()
// Author(s):   Conan Reis
void SkClass::invoke_class_dtor_recurse()
  {
  //SkInvokedMethod imethod(nullptr, nullptr, nullptr, nullptr);

  recurse_class_dtor();
  }

//---------------------------------------------------------------------------------------
// Recursively call the class destructor and clear out all members for this
//             class and all its subclasses.
// See:        invoke_class_dtor_recurse()
// Author(s):   Conan Reis
void SkClass::demand_unload_recurse()
  {
  // Recurse through subclasses first
  m_subclasses.apply_method(&SkClass::demand_unload_recurse);

  // $Vital - CReis [Incomplete] Call class destructor if one exists

  clear_members_compact();
  }

//---------------------------------------------------------------------------------------
// Recursively call the class destructor and clear out all members for this
//             class and all its subclasses.  [Already assumes that it is the demand load
//             group root class.]
// Returns:    true if unloaded immediately and false if unload is deferred, load is
//             locked or if it is already unloaded or not a demand loaded class.
// See:        invoke_class_dtor_recurse(), SkRuntimeBase::load_compiled_class_group()
// Author(s):   Conan Reis
bool SkClass::demand_unload()
  {
  if (((m_flags & Flag__demand_loaded) == Flag__demand_loaded) && !is_load_locked())
    {
    demand_unload_recurse();

    return true;
    }

  return false;
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Finds any invokable atomics (methods, coroutines, etc.) have not been
//             properly registered/bound to a C++ call.
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkClass::find_unregistered_atomics(APArray<SkInvokableBase> * atomics_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure instance methods bound
  SkMethodBase *  method_p;
  SkMethodBase ** methods_pp     = m_methods.get_array();
  SkMethodBase ** methods_end_pp = methods_pp + m_methods.get_length();

  for (; methods_pp < methods_end_pp; methods_pp++)
    {
    method_p = *methods_pp;

    if (!method_p->is_bound() && !method_p->is_placeholder() && (method_p->get_invoke_type() != SkInvokable_method) && !(method_p->get_annotation_flags() & SkAnnotation_ue4_blueprint))
      {
      atomics_p->append(*method_p);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure class methods bound
  methods_pp     = m_class_methods.get_array();
  methods_end_pp = methods_pp + m_class_methods.get_length();

  for (; methods_pp < methods_end_pp; methods_pp++)
    {
    method_p = *methods_pp;

    if (!method_p->is_bound() && !method_p->is_placeholder() && (method_p->get_invoke_type() != SkInvokable_method) && !(method_p->get_annotation_flags() & SkAnnotation_ue4_blueprint))
      {
      atomics_p->append(*method_p);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure coroutines bound
  SkCoroutineBase *  coroutine_p;
  SkCoroutineBase ** coroutines_pp     = m_coroutines.get_array();
  SkCoroutineBase ** coroutines_end_pp = coroutines_pp + m_coroutines.get_length();

  for (; coroutines_pp < coroutines_end_pp; coroutines_pp++)
    {
    coroutine_p = *coroutines_pp;

    if (!coroutine_p->is_bound() && !coroutine_p->is_placeholder() && (coroutine_p->get_invoke_type() != SkInvokable_coroutine) && !(coroutine_p->get_annotation_flags() & SkAnnotation_ue4_blueprint))
      {
      atomics_p->append(*coroutine_p);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure object ID find function is set if Flag_object_id_find is set for non-actor classes.
  // if (is_object_id_lookup() && !is_builtin_actor_class() && (m_object_id_lookup_f == nullptr))
  //   {
  //   // $Revisit - CReis Could make this added to the "unregistered" pop-up rather than asserting.
  //   SK_ERRORX(a_str_format(
  //     "Class '%s' is set to do look-ups for object IDs in its !Class.sk-meta file though "
  //     "it has not set a look-up function to use at runtime!\n"
  //     "[Any object IDs for that class will return nil unless "
  //     "SkClass::set_object_id_lookup_func() is called on it with a valid function.]",
  //     get_name_cstr_dbg()));
  //   }
  }

//---------------------------------------------------------------------------------------
// Finds any raw data members that cannot be accessed because accessor pointers are not registered
void SkClass::find_inaccessible_raw_members(APArray<SkRawMemberRecord> * raw_members_p)
  {
  if (!get_raw_pointer_func() && !m_data_raw.is_empty())
    {
    raw_members_p->append(*SK_NEW(SkRawMemberRecord)(this, nullptr));
    return;
    }

  for (auto * raw_member_p : m_data_raw)
    {
    if (!raw_member_p->m_type_p.is_valid() || !raw_member_p->m_type_p->get_key_class()->get_raw_accessor_func())
      {
      raw_members_p->append(*SK_NEW(SkRawMemberRecord)(this, raw_member_p));
      }
    }
  }

#endif // (SKOOKUM & SK_DEBUG)


//=======================================================================================
// SkClassUnion Class Data Members
//=======================================================================================

// Class union objects that are shared amongst various data-structures
APSortedLogicalFree<SkClassUnion> SkClassUnion::ms_shared_unions;


//=======================================================================================
// SkClassUnion Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Comparison function
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         class_union - other class to compare against
// Notes:      This method really only makes sense if the classes being compared are
//             ancestor/descendant or descendant/ancestor
// Author(s):   Conan Reis
eAEquate SkClassUnion::compare(const SkClassUnion & class_union) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If they are the equivalent object then they are equal
  if (this == &class_union)
    {
    return AEquate_equal;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // First sort by number of classes in the union - only unions with the same number of
  // classes may be equal
  uint32_t length1 = m_union.get_length();
  uint32_t length2 = class_union.m_union.get_length();

  if (length1 != length2)
    {
    return (length1 < length2)
      ? AEquate_less
      : AEquate_greater;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If all classes are the same then the two class unions are equal, the first class that
  // is different is sorted first.
  SkClassUnaryBase ** class1_pp     = m_union.get_array();
  SkClassUnaryBase ** class1_end_pp = class1_pp + length1;
  SkClassUnaryBase ** class2_pp     = class_union.m_union.get_array();

  for (; class1_pp < class1_end_pp; class1_pp++, class2_pp++)
    {
    eAEquate result = (*class1_pp)->compare(**class2_pp);

    if (result != AEquate_equal)
      {
      return result;
      }
    }

  // Class unions are of the same length and have the same classes
  return AEquate_equal;
  }

//---------------------------------------------------------------------------------------
uint32_t SkClassUnion::generate_crc32() const
  {
  SkClassUnaryBase ** class_pp = m_union.get_array();
  SkClassUnaryBase ** class_end_pp = class_pp + m_union.get_length();

  uint32_t crc_union = 0;
  for (; class_pp < class_end_pp; class_pp++)
    {
    crc_union = AChecksum::generate_crc32_uint32((*class_pp)->generate_crc32(), crc_union);
    }

  return crc_union;
  }

//---------------------------------------------------------------------------------------
// Appends the specified class to the set of unioned classes and reduces if
//             the class is a superclass or subclass of an existing class.
// Arg         new_class - class to append
// See:        is_trivial(), find_common_type(), get_common_class()
// Author(s):   Conan Reis
void SkClassUnion::merge_class(const SkClassUnaryBase & new_class)
  {
  // If this union has no other classes - add the new class directly
  if (m_common_class_p == nullptr)
    {
    new_class.reference();
    new_class.reference();
    m_union.append(new_class);
    m_common_class_p = const_cast<SkClassUnaryBase *>(&new_class);

    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that the new class adds more info and that it does not make any existing
  // classes redundant.

  bool                merge_b        = true;
  SkClassUnaryBase ** class_pp       = m_union.get_array();
  SkClassUnaryBase ** class_start_pp = class_pp;
  SkClassUnaryBase ** class_end_pp   = class_pp + m_union.get_length();

  while (class_pp < class_end_pp)
    {
    if (new_class.is_class_type(*class_pp))
      {
      // New class is already represented in union, so it does not need to be added.
      merge_b = false;
      break;
      }

    if ((*class_pp)->is_class_type(&new_class))
      {
      // Existing class in union is represented by new class, so previous class should
      // be removed.
      (*class_pp)->dereference();
      m_union.remove(uint32_t(class_pp - class_start_pp));
      class_end_pp--;
      }
    else
      {
      class_pp++;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If new class is not redundant merge it in the union.
  if (merge_b)
    {
    new_class.reference();
    m_union.append(new_class);

    SkClassUnaryBase * common_p = new_class.find_common_type(*m_common_class_p);

    common_p->reference();
    m_common_class_p->dereference_delay();
    m_common_class_p = common_p;
    }
  }

//---------------------------------------------------------------------------------------
// Appends the specified class to the set of unioned classes and reduces if
//             the class is a superclass or subclass of an existing class.
// Arg         new_class - class to append
// See:        is_trivial(), find_common_type(), get_common_class()
// Author(s):   Conan Reis
void SkClassUnion::merge_class(const SkClassDescBase & new_class)
  {
  if (new_class.get_class_type() != SkClassType_class_union)
    {
    merge_class(*static_cast<const SkClassUnaryBase *>(&new_class));
    }
  else
    {
    const SkClassUnion & class_union = *static_cast<const SkClassUnion *>(&new_class);

    if (class_union.m_common_class_p)
      {
      if (m_common_class_p)
        {
        SkClassUnaryBase ** class_pp     = class_union.m_union.get_array();
        SkClassUnaryBase ** class_end_pp = class_pp + class_union.m_union.get_length();

        for (; class_pp < class_end_pp; class_pp++)
          {
          merge_class(**class_pp);
          }
        }
      else
        {
        *this = class_union;
        }
      }
    }
  }


// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//             Binary composition:
//               5*bytes - common typed class reference [cached - could calc using unioned classes]
//               1 byte  - number of unioned classes
//               5*bytes - typed class reference }- Repeating
// Author(s):   Conan Reis
uint32_t SkClassUnion::as_binary_length() const
  {
  if (ms_compounds_use_ref)
    {
    return 6u + (m_union.get_length() * Binary_ref_size_typed);
    }
  else
    {
    uint32_t bytes  = 1 + m_common_class_p->as_binary_ref_typed_length();
    uint32_t length = m_union.get_length();

    SkClassUnaryBase ** classes_pp     = m_union.get_array();
    SkClassUnaryBase ** classes_end_pp = classes_pp + length;

    // n bytes - typed class reference }- Repeating
    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      bytes += (*classes_pp)->as_binary_ref_typed_length();
      }

    return bytes;
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this class union and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Binary composition:
//               5*bytes - common typed class reference [cached - could calc using unioned classes]
//               1 byte  - number of unioned classes
//               5*bytes - typed class reference }- Repeating
// Author(s):   Conan Reis
void SkClassUnion::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClassUnion::as_binary_length());

  // 5*bytes - common class name id
  m_common_class_p->as_binary_ref_typed(binary_pp);


  // 1 byte  - number of unioned classes
  uint32_t length = m_union.get_length();

  **(uint8_t **)binary_pp = static_cast<uint8_t>(length);
  (*(uint8_t **)binary_pp)++;


  // Write out unioned classes - must be 2 or more
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + length;

  // 5*bytes - typed class reference }- Repeating
  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    (*classes_pp)->as_binary_ref_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create
//             a reference to this SkClassUnion and increments the memory address  to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit 4 bytes of binary data.
// Notes:      Binary composition:
//               4 bytes - union class id [index in global class union list]
//
//               [2 bytes would undoubtedly be sufficient, but the other class types use
//               4 bytes for a reference so 4 bytes is used for consistency.]
//
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
void SkClassUnion::as_binary_ref(void ** binary_pp) const
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis This method assumes that the number and order of shared class unions
    // will be the same when the reference is loaded.

    uint32_t index = 0u;

    ms_shared_unions.find(*this, AMatch_first_found, &index);

    // 4 bytes - union class id
    **(uint32_t **)binary_pp = index;
    (*(uint32_t **)binary_pp)++;
    }
  else
    {
    as_binary(binary_pp);
    }
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from compiled binary code to data structures
#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Assign binary info to this object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               5*bytes - common typed class reference [cached - could calc using unioned classes]
//               1 byte  - number of unioned classes
//               5*bytes - typed class reference }- Repeating
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkClassUnion::assign_binary(const void ** binary_pp)
  {
  // 5*bytes - common typed class reference [cached - could calc using unioned classes]
  set_common_class(static_cast<SkClassUnaryBase *>(SkClassDescBase::from_binary_ref_typed(binary_pp)));


  // 1 byte - number of unioned classes
  uint32_t length = A_BYTE_STREAM_UI8_INC(binary_pp);

  m_union.set_length_null(length);


  // 5*bytes - typed class reference }- Repeating
  SkClassUnaryBase *  class_p;
  SkClassUnaryBase ** class_pp = m_union.get_array();

  for (; length > 0u ; length--)
    {
    // The types should already be sorted so just add them incrementally
    class_p = static_cast<SkClassUnaryBase *>(from_binary_ref_typed(binary_pp));
    class_p->reference();
    *class_pp = class_p;
    class_pp++;
    }
  }

#endif  // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this nil union class into its source code string equivalent.
//             This is essentially a disassembly of the internal script data-structures
//             into source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
//
//               class-union     = '<' class {'|' class}1+ '>'
//               class           = class-instance | meta-class
//               class-instance  = class-name
//               meta-class      = '<' class-name '>'
//
// Modifiers:   virtual (overriding pure method from SkClassDescBase) 
// Author(s):   Conan Reis
AString SkClassUnion::as_code() const
  {
  AString str(nullptr, 32u, 0u);

  str.append('<');

  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  str.append((*classes_pp)->as_code());
  classes_pp++;

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    str.append('|');
    str.append((*classes_pp)->as_code());
    }

  str.append('>');

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "<ThisClass_|None>" with "String" as a scope type becomes "<String|None>"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkClassUnion::is_generic() const
  {
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  while (classes_pp < classes_end_pp)
    {
    if ((*classes_pp)->is_generic())
      {
      return true;
      }

    classes_pp++;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// If this is a generic/reflective class, it will be replaced with its
//             finalized/specific class using scope_type as its scope
//             For example: "ThisClass_" could become "String"
// Returns:    Finalized non-generic class
// Arg         scope_type - current scope class type
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkClassDescBase * SkClassUnion::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  if (is_generic())
    {
    SkClassUnaryBase *  type_p;
    SkClassDescBase *   final_type_p;
    SkClassUnion        final_union;
    SkClassUnaryBase ** classes_pp     = m_union.get_array();
    SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();
    bool                generic_b      = false;

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      type_p       = *classes_pp;
      final_type_p = type_p->as_finalized_generic(scope_type);
      final_union.merge_class(*final_type_p);

      if (type_p != final_type_p)
        {
        generic_b = true;
        }
      }

    if (generic_b)
      {
      if (final_union.is_trivial())
        {
        return final_union.get_common_class();
        }

      return get_or_create(final_union);
      }
    }

  return const_cast<SkClassUnion *>(this);
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
//
// #Modifiers virtual
// #Author(s) Conan Reis
SkClassUnaryBase * SkClassUnion::find_common_type(const SkClassDescBase & cls) const
  {
  return m_common_class_p->find_common_type(cls);
  }

//---------------------------------------------------------------------------------------
// Returns component item class (as with a List) or nullptr if does not have component items.
//
// #Modifiers virtual
// #See Also  SkTypedClass
// #Author(s) Conan Reis
SkClassDescBase * SkClassUnion::get_item_type() const
  {
  // $Revisit - CReis More correct as union of item types if it is a union of List types.
  return m_common_class_p->get_item_type();


  /*
  SkClassUnaryBase *  item_class_p;
  SkClassUnaryBase *  common_class_p = nullptr;
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    item_class_p = (*classes_pp)->get_item_type();

    if ()
      {
      return true;
      }
    }

  return common_class_p;
  */
  }

//---------------------------------------------------------------------------------------
// Determines if this ClassUnion *might* be compatible with the specified
//             class type - i.e. is one of the classes that make up this class union the
//             same class type as type_p.
// Returns:    true if might be compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Author(s):   Conan Reis
bool SkClassUnion::is_class_maybe(const SkClassDescBase * type_p) const
  {
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    if ((*classes_pp)->is_class_type(type_p))
      {
      return true;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determines if arg_type_p would be a valid argument type for this class union when it
// is used as the parameter type.
//
// #See Also  is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// #Author(s) Conan Reis
bool SkClassUnion::is_valid_param_for(const SkClassDescBase * arg_type_p) const
  {
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    if (arg_type_p->is_class_type(*classes_pp))
      {
      return true;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determines if this ClassUnion is compatible with the specified class type
//             - i.e. can this class be passed as an argument to type_p.
// Returns:    true if compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkClassUnion::is_class_type(const SkClassDescBase * type_p) const
  {
  SkClassUnaryBase ** classes_pp     = m_union.get_array();
  SkClassUnaryBase ** classes_end_pp = classes_pp + m_union.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    if (!(*classes_pp)->is_class_type(type_p))
      {
      return false;
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Returns a union of the two classes and returns a SkClassUnion, SkClass, or
//             SkMetaClass - whichever is the simplest.
// Returns:    Union of the two classes in SkClassUnion, SkClass, or SkMetaClass form.
// Arg         class1 - class to union
// Arg         class2 - class to union
// See:        get_or_create(), merge_class()
// Modifiers:   static
// Author(s):   Conan Reis
SkClassDescBase * SkClassUnion::get_merge(
  const SkClassDescBase & class1,
  const SkClassDescBase & class2
  )
  {
  SkClassDescBase * type_p;
  SkClassUnion      class_union(class1);

  // Increment the reference count so no class is inadvertently freed during the merge
  class1.reference();
  class2.reference();

  class_union.merge_class(class2);

  type_p = class_union.is_trivial()
    ? static_cast<SkClassDescBase *>(class_union.get_common_class())
    : get_or_create(class_union);

  // Clear now rather than wait for destructor
  class_union.clear();

  // Decrement the reference count to normal again without doing any garbage collection yet
  class1.dereference_delay();
  class2.dereference_delay();

  return type_p;
  }

//---------------------------------------------------------------------------------------
// Returns a union of the classes and returns a SkClassUnion, SkClass, 
//             SkMetaClass or some other class - whichever is the simplest.
// Returns:    Union of the classes in SkClassUnion, SkClass or SkMetaClass form.
// Arg         classes - classes to union
// Arg         object_on_empty_b - if classes is empty returns <Object> if true and nullptr
//             if false.
// See:        get_or_create(), merge_class()
// Modifiers:   static
// Author(s):   Conan Reis
SkClassDescBase * SkClassUnion::get_merge(
  const APArrayBase<SkClassDescBase> & classes,
  bool                                 object_on_empty_b // = true
  )
  {
  uint32_t length = classes.get_length();

  switch (length)
    {
    case 0:
      return object_on_empty_b
        ? SkBrain::ms_object_class_p
        : nullptr;

    case 1u:
      return classes.get_first();

    case 2u:
      return get_merge(*classes.get_first(), *classes.get_last());
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Merging 3 or more classes
  SkClassDescBase *  type_p;
  SkClassDescBase ** classes_pp     = classes.get_array();
  SkClassDescBase ** classes_end_pp = classes_pp + length;
  SkClassUnion       class_union(**classes_pp);

  // Increment the reference count so no class is inadvertently freed during the merge
  (*classes_pp)->reference();
  classes_pp++;

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    (*classes_pp)->reference();
    class_union.merge_class(**classes_pp);
    }

  type_p = class_union.is_trivial()
    ? static_cast<SkClassDescBase *>(class_union.get_common_class())
    : get_or_create(class_union);

  // Clear now rather than wait for destructor
  class_union.clear();

  // Decrement the reference count to normal again without doing any garbage collection yet
  classes.apply_method(&SkClassDescBase::dereference_delay);

  return type_p;
  }

//---------------------------------------------------------------------------------------
// Returns a new class that is the same as the specified class union less
//             the class_to_remove - it is simplified to be either a class union or a
//             unary class.
// Returns:    a class union or class that is this class union less the class specified.
// Arg         class_union - union to remove class from
// Arg         class_to_remove - class to remove from union
// Modifiers:   static
// Author(s):   Conan Reis
SkClassDescBase * SkClassUnion::get_reduced(
  const SkClassUnion &     class_union,
  const SkClassUnaryBase & class_to_remove
  )
  {
  uint32_t idx;

  if (!class_union.m_union.find(class_to_remove, AMatch_first_found, &idx))
    {
    return const_cast<SkClassUnion *>(&class_union);
    }

  // $Revisit - CReis It would be nice to use a temporary buffer from the stack rather than the heap
  tSkSortedTypes classes(class_union.m_union);

  classes.remove(idx);

  // Casts element type
  return get_merge(*(APSorted<SkClassDescBase> *)&classes);
  }

//---------------------------------------------------------------------------------------
// Ensures that all the globally available class unions are referenced.
//             Parsing may create some temporary class unions - this method frees them
//             from memory.
// Modifiers:   static
// Author(s):   Conan Reis
bool SkClassUnion::shared_ensure_references()
  {
  bool anything_changed = false;

  SkClassUnion ** union_pp     = ms_shared_unions.get_array();
  SkClassUnion ** union_end_pp = union_pp + ms_shared_unions.get_length();

  // Traverse in reverse order so class unions without references can be easily freed.
  while (union_pp < union_end_pp)
    {
    union_end_pp--;

    // All SkClassUnions stored in ms_shared_unions start out with 1 refcount
    // so that ARefPtr can never free them
    // So if the refcount is 1 (or less) it means they are not actually referenced
    // and should be removed
    if ((*union_end_pp)->m_ref_count <= 1u)
      {
      ms_shared_unions.free(uint32_t(union_end_pp - union_pp));
      anything_changed = true;
      }
    }

  // If empty, get rid of memory
  if (ms_shared_unions.is_empty())
    {
    ms_shared_unions.compact();
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this class of object
// See:        SkDebug, AMemoryStats
// Modifiers:   static
// Author(s):   Conan Reis
void SkClassUnion::shared_track_memory(AMemoryStats * mem_stats_p)
  {
  // Note that the SkClassUnion array buffer is added as dynamic memory
  uint32_t count      = ms_shared_unions.get_length();
  uint32_t dyn_needed = count * sizeof(void *);
  uint32_t dyn_actual = ms_shared_unions.get_size() * sizeof(void *);

  SkClassUnion ** unions_pp     = ms_shared_unions.get_array();
  SkClassUnion ** unions_end_pp = unions_pp + count;

  for (; unions_pp < unions_end_pp; unions_pp++)
    {
    dyn_needed += (*unions_pp)->m_union.get_length() * sizeof(void *);
    dyn_actual += (*unions_pp)->m_union.get_size() * sizeof(void *);
    }

  mem_stats_p->track_memory(SKMEMORY_ARGS(SkClassUnion, 0u), dyn_needed, dyn_actual, count);
  }

//=======================================================================================
// SkRoutineUpdateRecord Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkRoutineUpdateRecord::~SkRoutineUpdateRecord()
  {
  if (m_previous_routine_p)
    {
    delete m_previous_routine_p;
    }
  else if (m_previous_custom_expr_p) // Delete only if m_previous_routine_p dtor didn't already delete it
    {
    delete m_previous_custom_expr_p;
    }
  }

//=======================================================================================
// SkProgramUpdateRecord Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkClassUpdateRecord * SkProgramUpdateRecord::get_or_create_class_update_record(const ASymbol & class_name)
  {
  uint32_t  insert_pos;
  SkClassUpdateRecord * class_p = m_updated_classes.get(class_name, AMatch_first_found, &insert_pos);

  if (class_p == nullptr)
    {
    class_p = new SkClassUpdateRecord(class_name);
    m_updated_classes.insert(*class_p, insert_pos);
    }

  return class_p;
  }
