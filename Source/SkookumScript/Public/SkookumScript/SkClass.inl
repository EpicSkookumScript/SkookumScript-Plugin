// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// SkookumScript C++ library.
//
// Data structures for class descriptors and class objects
// Author(s):   Conan Reis
// Notes:          
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// SkMetaClass Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Standard sort by name id.
// Returns:    AEquate_less, AEquate_equal, AEquate_greater
// Arg         mclass - metaclass to compare
// See:        SkClass::compare() - hierarchy sort
// Author(s):   Conan Reis
A_INLINE eAEquate SkMetaClass::compare(const SkMetaClass & mclass) const
  {
  return m_class_info_p->compare_ids(*mclass.m_class_info_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE uint32_t SkMetaClass::generate_crc32() const
  {
  return m_class_info_p->generate_crc32();
  }

//---------------------------------------------------------------------------------------
// Returns class type
// Returns:    class type
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE eSkClassType SkMetaClass::get_class_type() const
  {
  return SkClassType_metaclass;
  }

//---------------------------------------------------------------------------------------
// Returns key/main class
// Returns:    key/main class
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkClass * SkMetaClass::get_key_class() const
  {
  return m_class_info_p;
  }

//---------------------------------------------------------------------------------------
// Get the name of the wrapped class
// Returns:    name of the wrapped class
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE const ASymbol & SkMetaClass::get_key_class_name() const
  {
  return m_class_info_p->m_name;
  }

//---------------------------------------------------------------------------------------
// Returns a MetaClass version of itself - i.e. the class as an instance.
// Returns:    MetaClass version of itself
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMetaClass & SkMetaClass::get_metaclass() const
  {
  return *const_cast<SkMetaClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Gets the named method.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkMetaClass::find_method(const ASymbol & method_name, bool * is_class_member_p) const
  {
  if (is_class_member_p)
    {
    *is_class_member_p = true;
    }

  return m_class_info_p->find_class_method(method_name);
  }

//---------------------------------------------------------------------------------------
// Gets the named method from this class or a superclass.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkMetaClass::find_method_inherited(const ASymbol & method_name, bool * is_class_member_p) const
  {
  return m_class_info_p->find_class_method_inherited(method_name, is_class_member_p);
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) class scope method with the given name and
//             parameters.  The method will use this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE void SkMetaClass::append_method(SkMethodBase * method_p, bool * has_signature_changed_p)
  {
  m_class_info_p->append_class_method(method_p, has_signature_changed_p);
  }

//---------------------------------------------------------------------------------------
// Returns true if it is a metaclass - i.e. using class scope.
// Returns:    true if it is a metaclass
// Modifiers:   virtual - overridden from SkClassDescBase & SkInstance
// Author(s):   Conan Reis
A_INLINE bool SkMetaClass::is_metaclass() const
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Determines if class method has been previously registered in this class
//             (without searching through inherited methods).
// Returns:    true if method has been previously registered, false if not
// Arg         method_name - name of method
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkMetaClass::is_method_valid(const ASymbol & method_name) const
  {
  return m_class_info_p->is_class_method_valid(method_name);
  }

//---------------------------------------------------------------------------------------
// Determines if the class method is available
// Returns:    true if method is available, false if not.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkMetaClass::is_method_inherited_valid(const ASymbol & method_name) const
  {
  return m_class_info_p->is_class_method_inherited_valid(method_name);
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) coroutine with the given name and parameters
//             using this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE void SkMetaClass::append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p)
  {


  A_ERRORX("Class coroutines not yet implemented!")


  //m_class_p->append_class_coroutine(coroutine_p);
  }

//---------------------------------------------------------------------------------------
// Adds an class data member with the specified name to this class.
// Arg         name - name of class data member.  It must be unique - no superclass nor
//             subclass may have it and there should be no instance data member with the
//             same name either.
// Arg         type_p - class type of data member
// See:        append_class_data()
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkTypedName * SkMetaClass::append_data_member(
  const ASymbol &   name,
  SkClassDescBase * type_p
  )
  {
  return m_class_info_p->append_class_data(name, type_p, true);
  }


//---------------------------------------------------------------------------------------
// Adds an class data member with the specified name to this class.
// Arg         name - name of class data member.  It must be unique - no superclass nor
//             subclass may have it and there should be no instance data member with the
//             same name either.
// Arg         type_p - class type of data member
// See:        append_class_data()
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkTypedNameRaw * SkMetaClass::append_data_member_raw(
  const ASymbol &   name,
  SkClassDescBase * type_p,
  const AString &   bind_name
  )
  {
  SK_ERRORX("Raw class data is not supported. The parser should make sure this gets never called.");
  return nullptr;
  }


//=======================================================================================
// SkClass Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Standard sort by name id.
// Returns:    AEquate_less, AEquate_equal, AEquate_greater
// Arg         ssclass - class to compare
// See:        compare() - hierarchy sort
// Author(s):   Conan Reis
A_INLINE eAEquate SkClass::compare_ids(const SkClass & ssclass) const
  {
  return m_name.compare(ssclass.m_name);
  }

//---------------------------------------------------------------------------------------
// Comparison function
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         sslcass - other class to compare against
// Notes:      This method really only makes sense if the classes being compared are
//             ancestor/descendant or descendant/ancestor
// Author(s):   Conan Reis
A_INLINE eAEquate SkClass::compare(const SkClass & ssclass) const
  {
  if (this == &ssclass)
    {
    return AEquate_equal;
    }

  if (is_subclass(ssclass))
    {
    return AEquate_less;
    }

  return AEquate_greater;
  }

//---------------------------------------------------------------------------------------
// Sets whether the class should indicate that it has been loaded or not
// Author(s):   Conan Reis
A_INLINE void SkClass::set_flag_load(
  bool loaded // = true
  )
  {
  if (loaded)
    {
    m_flags = (m_flags & ~Flag_demand_unload) | Flag_loaded;
    }
  else
    {
    m_flags &= ~(Flag_loaded | Flag_demand_unload);
    }
  }

//---------------------------------------------------------------------------------------
// Uses the raw member accessor to get an instance with the value of a raw member

A_INLINE SkInstance * SkClass::new_instance_from_raw_data(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p) const
  {
  #if (SKOOKUM & SK_DEBUG)
    if (raw_data_info == SkRawDataInfo_Invalid)
      {
      // Report error unless we are already in the process of reporting an error (and likely are gathering a callstack)
      SK_ASSERTX(ADebug::is_nested_error(), a_str_format("Tried to access invalid raw data member of class '%s'", get_name_cstr_dbg()));
      return SkBrain::ms_nil_p;
      }
  #endif
  return (*m_raw_member_accessor_f)(obj_p, raw_data_info, data_type_p, nullptr);
  }

//---------------------------------------------------------------------------------------
// Uses the raw member accessor to set the value of a raw member from a given instance

A_INLINE void SkClass::assign_raw_data(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p) const
  {
  #if (SKOOKUM & SK_DEBUG)
    if (raw_data_info == SkRawDataInfo_Invalid)
      {
      // Report error unless we are already in the process of reporting an error (and likely are gathering a callstack)
      SK_ASSERTX(ADebug::is_nested_error(), a_str_format("Tried to access invalid raw data member of class '%s'", get_name_cstr_dbg()));
      return;
      }
  #endif
  (*m_raw_member_accessor_f)(obj_p, raw_data_info, data_type_p, value_p);
  }

//---------------------------------------------------------------------------------------

A_INLINE void * SkClass::get_raw_pointer(SkInstance * obj_p) const
  {
  return (*m_raw_pointer_f)(obj_p);
  }

#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Gets existing object ID validation list or creates one if set to use a validation list.
//
// #Notes
//   To get existing object ID validation list without creating one if it doesn't already
//   exist use get_object_id_valid_list().
//
// #See Also  get_object_id_valid_list()
// #Author(s) Conan Reis
A_INLINE ASymbolTable * SkClass::get_object_id_valid_list_merge()
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    if ((m_object_ids_p == nullptr) && (m_flags & Flag_object_id_parse_list))
      {
      m_object_ids_p = SK_NEW(ASymbolTable)(true);
      }
  #endif

  return m_object_ids_p;
  }

#endif  // (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
// Returns:    closest superclass that this class and cls share.
// Author(s):   Conan Reis
A_INLINE SkClass * SkClass::find_common_class(const SkClass & cls) const
  {
  SkClass * class_p = const_cast<SkClass *>(&cls);

  do
    {
    if (is_class(*class_p))
      {
      return class_p;
      }

    class_p = class_p->get_superclass();
    }
  while (class_p);

  // Should never get here since the "Object" class is a superclass of all classes.
  return nullptr;
  }

//---------------------------------------------------------------------------------------
//  Determines if this class is equal to or a subclass of (derived from) the
//              supplied class 'cls'.
// Returns:     true if same class or subclass of 'cls', false if not
// Arg          cls - class to determine if this class is the same class or a subclass of
// Examples:    bool correct_class = some_class.is_class(test_class);
// Author(s):    Conan Reis
A_INLINE bool SkClass::is_class(const SkClass & cls) const
  {
  const SkClass * class_p = this;
  do 
    {
    if (class_p == &cls) return true;
    class_p = class_p->m_superclass_p;
    } while (class_p);
  return false;
  }

//---------------------------------------------------------------------------------------
//  Determines if this class is equal to or a subclass of (derived from) a class named 'name_id'
// Author(s):    Markus Breyer
A_INLINE bool SkClass::is_class(uint32_t name_id) const
  {
  const SkClass * class_p = this;
  do
    {
    if (class_p->get_name_id() == name_id) return true;
    class_p = class_p->m_superclass_p;
    } while (class_p);
  return false;
  }

//---------------------------------------------------------------------------------------
//  Determines if this class is a subclass of (derived from) the supplied
//              class 'superclass'.
// Returns:     true if subclass of 'superclass', false if not
// Arg          superclass - class to determine if this class is a subclass of
// Examples:    bool correct_class = some_class.is_subclass(test_class);
// Author(s):    Conan Reis
A_INLINE bool SkClass::is_subclass(const SkClass & superclass) const
  {
  SkClass * super_p = m_superclass_p;

  return (super_p == &superclass) || (super_p && super_p->is_subclass(superclass));
  // Or this: return (m_superclass_p == &superclass) || (m_superclass_p && m_superclass_p->is_subclass(superclass));
  // Or this: return (m_superclass_p && m_superclass_p->is_class(superclass));
  }

//---------------------------------------------------------------------------------------
//  Determines if this class is a superclass of (parent of) the supplied
//              class 'subclass'.
// Returns:     true if superclass of 'subclass', false if not
// Arg          subclass - class to determine if this class is a superclass of
// Examples:    bool correct_class = some_class.is_superclass(test_class);
// Author(s):    Conan Reis
A_INLINE bool SkClass::is_superclass(const SkClass & subclass) const
  {
  SkClass * sub_super_p = subclass.m_superclass_p;

  return (sub_super_p == this) || (sub_super_p && is_superclass(*sub_super_p));
  // Or this: return (subclass.m_superclass_p == this) || (subclass.m_superclass_p && is_superclass(subclass.m_superclass_p));
  // Or this: return subclass.is_subclass(*this);
  }

//---------------------------------------------------------------------------------------
// Determines if this class is a valid scope qualifier by the specified
//             receiver scope - i.e. receiver ThisClass@member
// Returns:    true if valid scope qualifier, false if not
// Arg         recv_type_p - class type of receiver
// See:        SkClassDescBase::qualify()
// Author(s):   Conan Reis
A_INLINE bool SkClass::is_scope_qualifier(SkClassDescBase * recv_type_p) const
  {
  return (recv_type_p == nullptr)
    || ((recv_type_p->get_class_type() == SkClassType_metaclass)
      ? static_cast<SkMetaClass *>(recv_type_p)->m_class_info_p->is_class(*this)
      : recv_type_p->is_class_type(this));
  }

//---------------------------------------------------------------------------------------
// Determines if this class has been deleted/removed from the class tree
// See: removed_subclass()
A_INLINE bool SkClass::is_deleted() const
  {
  return !m_superclass_p && get_name_id() != ASymbolId_Object;
  }

//---------------------------------------------------------------------------------------
//  Gets the qualified (or unqualified) method.
// Returns:     A pointer to the requested method or nullptr if it does not exist.
// Arg          method_qual - name and optional highest level scope of method.  Note
//              that a SkQualifier may be coerced from a ASymbol if the topmost scoped
//              method is desired.
// Author(s):    Conan Reis
A_INLINE SkMethodBase * SkClass::find_instance_method_scoped_inherited(const SkQualifier & method_qual) const
  {
  // $Revisit - CReis [Index Look-up] This look-up should eventually be replaced with an index into a method table.
  SkClass * scope_p = method_qual.get_scope();

  return (scope_p ?
    scope_p->find_instance_method_inherited(method_qual.get_name()) :
    find_instance_method_inherited(method_qual.get_name()));
  }

//---------------------------------------------------------------------------------------
//  Gets the named coroutine.
// Returns:     A pointer to the requested coroutine or nullptr if it does not exist.
// Arg          coroutine_name - name of coroutine.
// See:         find_coroutine(coroutine_qual), find_coroutine(coroutine_call)
// Author(s):    Conan Reis
A_INLINE SkCoroutineBase * SkClass::find_coroutine_inherited(const ASymbol & coroutine_name) const
  {
  SkCoroutineBase * coroutine_p = m_coroutines.get(coroutine_name);

  return (coroutine_p == nullptr)
    ? (m_superclass_p
      ? m_superclass_p->find_coroutine_inherited(coroutine_name)
      : nullptr)
    : coroutine_p;
  }

//---------------------------------------------------------------------------------------
//  Gets the qualified (or unqualified) coroutine.
// Returns:     A pointer to the requested coroutine or nullptr if it does not exist.
// Arg          coroutine_qual - name and optional highest level scope of coroutine.  Note
//              that a SkQualifier may be coerced from a ASymbol if the topmost scoped
//              coroutine is desired.
// Author(s):    Conan Reis
A_INLINE SkCoroutineBase * SkClass::find_coroutine_scoped_inherited(const SkQualifier & coroutine_qual) const
  {
  SkClass * scope_p = coroutine_qual.get_scope();

  return ((scope_p) && (scope_p != this)) ?
    scope_p->find_coroutine_inherited(coroutine_qual.get_name()) :
    find_coroutine_inherited(coroutine_qual.get_name());
  }

//---------------------------------------------------------------------------------------
// Determines if coroutine is registered - i.e. it exists and it is not a
//             placeholder.
// Returns:    true if coroutine is registered
// Arg         coroutine_name - name of coroutine to check
// Author(s):   Conan Reis
A_INLINE bool SkClass::is_coroutine_registered(const ASymbol & coroutine_name) const
  {
  SkCoroutineBase * coro_p = m_coroutines.get(coroutine_name);

  return !((coro_p == nullptr) || coro_p->is_placeholder());
  }

//---------------------------------------------------------------------------------------
// Instantiates / creates a new instance of this class and adds any data members and
// initializes them to nil
// 
// Returns:   an instance of this class
// Modifiers: virtual - override for custom behaviour
// Author(s): Conan Reis
A_INLINE SkInstance * SkClass::new_instance()
  {
  if (is_mind_class())
    {
    return new_mind_instance();
    }
  else if (m_total_data_count)
    {
    return SkDataInstance::new_instance(this);
    }
  else
    {
    return SkInstance::new_instance(this);
    }
  }

//---------------------------------------------------------------------------------------
// Returns class type
// Returns:    class type
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE eSkClassType SkClass::get_class_type() const
  {
  return SkClassType_class;
  }

//---------------------------------------------------------------------------------------
// Returns key/main class
// Returns:    key/main class
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkClass * SkClass::get_key_class() const
  {
  return const_cast<SkClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Get the name of the class
// Returns:    name of the class
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE const ASymbol & SkClass::get_key_class_name() const
  {
  return m_name;
  }

//---------------------------------------------------------------------------------------
// Returns component item class (as with a List) or nullptr if does not have component items.
//
// #Modifiers virtual
// #See Also  SkTypedClass
// #Author(s) Conan Reis
A_INLINE SkClassDescBase * SkClass::get_item_type() const
  {
  return is_class(*SkBrain::ms_list_class_p)
    ? SkBrain::ms_object_class_p
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the named method.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClass::find_method(const ASymbol & method_name, bool * is_class_member_p) const
  {
  bool is_class_member = false;
  SkMethodBase * method_p = find_instance_method(method_name);
  if (!method_p)
    {
    method_p = find_class_method(method_name);
    is_class_member = (method_p != nullptr);
    }

  if (is_class_member_p)
    {
    *is_class_member_p = is_class_member;
    }

  return method_p;
  }

//---------------------------------------------------------------------------------------
// Gets instance method from this class or a superclass.
A_INLINE SkInvokableBase * SkClass::get_invokable_from_vtable(eSkScope scope, int16_t vtable_index) const
  {
  const tSkVTable & vtable = (scope == SkScope_instance ? m_vtable_i : m_vtable_c);

  // Recover from bad vtable index
  #ifdef SK_RUNTIME_RECOVER
    if ((uint32_t)vtable_index >= vtable.get_length())
      {
      return nullptr;
      }
  #endif

  return vtable.get_at(vtable_index);
  }

//---------------------------------------------------------------------------------------
// Gets instance method from this class or a superclass.
A_INLINE SkInvokableBase * SkClass::get_invokable_from_vtable_i(int16_t vtable_index) const
  {
  // Recover from bad vtable index
  #ifdef SK_RUNTIME_RECOVER
    if ((uint32_t)vtable_index >= m_vtable_i.get_length())
      {
      return nullptr;
      }
  #endif

  return m_vtable_i.get_at(vtable_index);
  }

//---------------------------------------------------------------------------------------
// Gets class method from this class or a superclass.
A_INLINE SkInvokableBase * SkClass::get_invokable_from_vtable_c(int16_t vtable_index) const
  {
  // Recover from bad vtable index
  #ifdef SK_RUNTIME_RECOVER
    if ((uint32_t)vtable_index >= m_vtable_c.get_length())
      {
      return nullptr;
      }
  #endif

  return m_vtable_c.get_at(vtable_index);
  }

//---------------------------------------------------------------------------------------
// Attempts to find invokable in vtable by name
A_INLINE int16_t SkClass::find_invokable_in_vtable_i(const ASymbol & name) const
  {
  uint32_t vtable_index;
  if (m_vtable_i.find(name, AMatch_first_found, &vtable_index))
    {
    return (int16_t)vtable_index;
    }

  return SkQualifier::ms_invalid_vtable_index;
  }

//---------------------------------------------------------------------------------------
// Attempts to find invokable in vtable by name
A_INLINE int16_t SkClass::find_invokable_in_vtable_c(const ASymbol & name) const
  {
  uint32_t vtable_index;
  if (m_vtable_c.find(name, AMatch_first_found, &vtable_index))
    {
    return (int16_t)vtable_index;
    }

  return SkQualifier::ms_invalid_vtable_index;
  }

//---------------------------------------------------------------------------------------
// Gets the named instance method from this class or a superclass.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// See:        find_instance_method_scoped_inherited(method_qual), find_class_method_inherited()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClass::find_instance_method_inherited(const ASymbol & method_name) const
  {
  // $Revisit - CReis [Index Look-up] This look-up should eventually be replaced with an index into a method table.
  SkMethodBase * method_p = m_methods.get(method_name);

  return ((method_p == nullptr) && m_superclass_p)
    ? m_superclass_p->find_instance_method_inherited(method_name)
    : method_p;
  }

//---------------------------------------------------------------------------------------
// Gets the named class method from this class or a superclass
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// See:        find_instance_method_scoped_inherited(method_qual), find_instance_method_inherited(method_name)
// Notes:      Any returned class method must have a metaclass as a receiver.
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClass::find_class_method_inherited(const ASymbol & method_name, bool * is_class_member_p) const
  {
  bool is_class_member = true;
  SkMethodBase * method_p = m_class_methods.get(method_name);
  if (!method_p)
    { 
    if (m_superclass_p)
      {
      method_p = m_superclass_p->find_class_method_inherited(method_name, &is_class_member);
      }
    else
      {
      // Note that instance methods of "Object" are also valid for any class instance.
      // If we have no super class, we must be the `Object` class
      method_p = m_methods.get(method_name);
      is_class_member = !method_p;
      }
    }

  if (is_class_member_p)
    {
    *is_class_member_p = is_class_member;
    }

  return method_p;
  }

//---------------------------------------------------------------------------------------
// Gets the named method from this class or a superclass.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClass::find_method_inherited(const ASymbol & method_name, bool * is_class_member_p) const
  {
  bool is_class_member = false;
  SkMethodBase * method_p = find_instance_method_inherited(method_name);
  if (!method_p)
    {
    method_p = find_class_method_inherited(method_name, &is_class_member);
    }

  if (is_class_member_p)
    {
    *is_class_member_p = is_class_member;
    }

  return method_p;
  }

//---------------------------------------------------------------------------------------
// Returns a MetaClass version of itself - i.e. the class as an instance.
// Returns:    MetaClass version of itself
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMetaClass & SkClass::get_metaclass() const
  {
  return const_cast<SkMetaClass &>(m_metaclass);
  }

//---------------------------------------------------------------------------------------
// Determines if method is registered - i.e. it exists and it is not a
//             placeholder.
// Returns:    true if method is registered
// Arg         method_name - name of method to check
// Author(s):   Conan Reis
A_INLINE bool SkClass::is_method_registered(const ASymbol & method_name, bool allow_placeholder) const
  {
  // Note that only instance methods are checked - SkMetaClass checks class methods
  SkMethodBase * method_p = find_instance_method(method_name);

  return method_p && (allow_placeholder || !method_p->is_placeholder());
  }

//---------------------------------------------------------------------------------------
// Determines if the method is available
// Returns:    true if method is available, false if not.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkClass::is_method_valid(const ASymbol & method_name) const
  {
  return (find_instance_method(method_name)
    || (find_class_method(method_name) != nullptr));
  }

//---------------------------------------------------------------------------------------
// Determines if the method is available
// Returns:    true if method is available, false if not.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE bool SkClass::is_method_inherited_valid(const ASymbol & method_name) const
  {
  return ((find_instance_method_inherited(method_name) != nullptr)
    || (find_class_method_inherited(method_name) != nullptr));
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) instance method with the given name and
//             parameters.  The method will use this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE void SkClass::append_method(SkMethodBase * method_p, bool * has_signature_changed_p)
  {
  // Note that only instance methods are checked.
  append_instance_method(method_p, has_signature_changed_p);
  }

//---------------------------------------------------------------------------------------
// Adds an instance data member with the specified name to this class.
//             When an instance of this class or a instance of a subclass is constructed
//             it will have a data member with this name.
// Arg         name - name of instance data member.  It must be unique - no superclass
//             nor subclass may have it and there should be no class data member with the
//             same name either.
// Arg         type_p - class type of data member
// See:        append_class_data()
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkTypedName * SkClass::append_data_member(
  const ASymbol &   name,
  SkClassDescBase * type_p
  )
  {
  return append_instance_data(name, type_p, true);
  }

//---------------------------------------------------------------------------------------
// Adds a raw instance data member with the specified name to this class.
A_INLINE SkTypedNameRaw * SkClass::append_data_member_raw(
  const ASymbol &   name,
  SkClassDescBase * type_p,
  const AString &   bind_name
  )
  {
  return append_instance_data_raw(name, type_p, bind_name);
  }


//=======================================================================================
// SkClassUnion Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Copy Constructor
// Arg         class_union - other union class to copy
// Author(s):   Conan Reis
A_INLINE SkClassUnion::SkClassUnion(const SkClassUnion & class_union) :
  m_common_class_p(class_union.m_common_class_p),
  m_union(class_union.m_union)
  {
  if (m_common_class_p)
    {
    m_common_class_p->reference();
    }

  m_union.apply_method(&SkClassUnaryBase::reference);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual - overridden from ARefCountMix<>
// Author(s):   Conan Reis
A_INLINE SkClassUnion::~SkClassUnion()
  {
  if (m_common_class_p)
    {
    m_common_class_p->dereference_delay();
    }

  m_union.apply_method(&SkClassUnaryBase::dereference_delay);
  }

//---------------------------------------------------------------------------------------
// Assignment
// Arg         class_union - other union class to copy
// Author(s):   Conan Reis
A_INLINE SkClassUnion & SkClassUnion::operator=(const SkClassUnion & class_union)
  {
  set_common_class(class_union.m_common_class_p);

  class_union.m_union.apply_method(&SkClassUnaryBase::reference);
  m_union.apply_method(&SkClassUnaryBase::dereference_delay);
  m_union = class_union.m_union;

  return *this;
  }

// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Binary reference length in bytes.
// Modifiers:   virtual
// Author(s):   Conan Reis
A_INLINE uint32_t SkClassUnion::as_binary_ref_typed_length() const
  {
  return ms_compounds_use_ref ? Binary_ref_size_typed : (1u + as_binary_length());
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from compiled binary code to data structures
#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Returns pointer to class based on the binary reference info or nullptr if not
//             found
// Returns:    pointer to class or nullptr
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_ref() or a similar
//             mechanism.
// See:        as_binary_ref_typed(), as_binary_ref_typed_length(), from_binary_ref_typed()
// Notes:      Binary composition:
//               4 bytes - class union index
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkClassUnion * SkClassUnion::from_binary_ref(const void ** binary_pp)
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis This method assumes that the order and number of shared class unions
    // is the same now as when the reference was saved.
    return ms_shared_unions.get_at(A_BYTE_STREAM_UI32_INC(binary_pp));
    }
  else
    {
    return get_or_create(SkClassUnion(binary_pp));
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//---------------------------------------------------------------------------------------
// Clears class union
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::clear()
  {
  m_union.apply_method(&SkClassUnaryBase::dereference_delay);
  m_union.empty();

  if (m_common_class_p)
    {
    m_common_class_p->dereference_delay();
    }

  m_common_class_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Returns a matching class union from the shared global list of class unions.
//             If a matching class union does not already exist, it creates one.
// Returns:    global class union
// Arg         class_union - class union to find match for
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkClassUnion * SkClassUnion::get_or_create(const SkClassUnion & class_union)
  {
  uint32_t       find_pos;
  SkClassUnion * union_p = ms_shared_unions.get(class_union, AMatch_first_found, &find_pos);

  if (union_p == nullptr)
    {
    union_p = SK_NEW(SkClassUnion)(class_union);
    // All SkClassUnions stored in ms_shared_unions start out with 1 refcount
    // so that ARefPtr can never free them
    // They only ever get deleted in SkClassUnion::shared_ensure_references()
    union_p->reference();
    ms_shared_unions.insert(*union_p, find_pos);
    }

  return union_p;
  }

//---------------------------------------------------------------------------------------
// Returns class type
// Returns:    class type
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE eSkClassType SkClassUnion::get_class_type() const
  {
  return SkClassType_class_union;
  }

//---------------------------------------------------------------------------------------
// Returns class type of data member
// 
// Returns: class type of data member
// 
// Params:
//   data_name: name of data member
//   scope_p: address to store 
A_INLINE SkTypedName * SkClassUnion::get_data_type(
  const ASymbol & data_name,
  eSkScope *      scope_p,  // = nullptr
  uint32_t *      data_idx_p,         // = nullptr
  SkClass **      data_owner_class_pp // = nullptr
  ) const
  {
  // Without a cast to a specific class use nearest shared superclass.
  return m_common_class_p->get_data_type(data_name, scope_p, data_idx_p, data_owner_class_pp);
  }

//---------------------------------------------------------------------------------------
// Returns key/main class
// Returns:    key/main class
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkClass * SkClassUnion::get_key_class() const
  {
  return m_common_class_p->get_key_class();
  }

//---------------------------------------------------------------------------------------
// Get the name of the wrapped class
// Returns:    name of the wrapped class
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE const ASymbol & SkClassUnion::get_key_class_name() const
  {
  return m_common_class_p->get_key_class_name();
  }

//---------------------------------------------------------------------------------------
// Returns a MetaClass version of itself - i.e. the class as an instance.
// Returns:    MetaClass version of itself
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMetaClass & SkClassUnion::get_metaclass() const
  {
  // Without a cast to a specific class use nearest shared superclass.
  return m_common_class_p->get_metaclass();
  }

//---------------------------------------------------------------------------------------
// Gets the named method.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClassUnion::find_method(const ASymbol & method_name, bool * is_class_member_p) const
  {
  // Without a cast to a specific class use nearest shared superclass.
  return m_common_class_p->find_method(method_name, is_class_member_p);
  }

//---------------------------------------------------------------------------------------
// Gets the named method from this class or a superclass.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkClassUnion::find_method_inherited(const ASymbol & method_name, bool * is_class_member_p) const
  {
  // Without a cast to a specific class use nearest shared superclass.
  return m_common_class_p->find_method_inherited(method_name, is_class_member_p);
  }

//---------------------------------------------------------------------------------------
//  Gets the named coroutine.
// Returns:     A pointer to the requested coroutine or nullptr if it does not exist.
// Arg          coroutine_name - name of coroutine.
// See:         find_coroutine(coroutine_qual), find_coroutine(coroutine_call)
// Author(s):    Conan Reis
A_INLINE SkCoroutineBase * SkClassUnion::find_coroutine_inherited(const ASymbol & coroutine_name) const
  {
  return m_common_class_p->find_coroutine_inherited(coroutine_name);
  }

//---------------------------------------------------------------------------------------
// Returns true if it is an "Actor" class or a subclass of "Actor"
//             (SkActorClass) or false if not.
// Returns:    true if it is an actor class instance (SkActorClass)
// Notes:      Same as calling is_class(*SkBrain::ms_actor_class_p), but faster.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE bool SkClassUnion::is_builtin_actor_class() const
  {
  return m_common_class_p->is_builtin_actor_class();
  }

//---------------------------------------------------------------------------------------
// Returns true if it is a metaclass - i.e. using class scope.
// Returns:    true if it is a metaclass
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE bool SkClassUnion::is_metaclass() const
  {
  return m_common_class_p->is_metaclass();
  }

//---------------------------------------------------------------------------------------
// Determines if the method is available
// Returns:    true if method is available, false if not.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkClassUnion::is_method_inherited_valid(const ASymbol & method_name) const
  {
  // Without a cast to a specific class use nearest shared superclass.
  return m_common_class_p->is_method_inherited_valid(method_name);
  }

//---------------------------------------------------------------------------------------
// Decrements the reference count to this object and if the reference count
//             becomes 0 call on_no_references()
// See:        reference(), dereference_delay(), on_no_references()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::dereference()
  {
  ARefCountMix<SkClassUnion>::dereference();
  }

//---------------------------------------------------------------------------------------
// Same as dereference() in that it decrements the reference count to this
//             object, but it does not call on_no_references() if the reference count
//             becomes 0.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::dereference_delay() const
  {
  ARefCountMix<SkClassUnion>::dereference_delay();
  }

//---------------------------------------------------------------------------------------
// Increments the reference count to this object.
// See:        dereference(), dereference_delay()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::reference() const
  {
  ARefCountMix<SkClassUnion>::reference();
  }

//---------------------------------------------------------------------------------------
// Called when the number of references to this object reaches zero - by
//             default it deletes this object and removes it from the list of available
//             class unions.
// See:        dereference(), ensure_reference()
// Notes:      called by dereference() and ensure_reference()
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::on_no_references()
  {
  ms_shared_unions.free(*this);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE void SkClassUnion::set_common_class(SkClassUnaryBase * class_p)
  {
  if (class_p)
    {
    class_p->reference();
    }

  if (m_common_class_p)
    {
    m_common_class_p->dereference_delay();
    }

  m_common_class_p = class_p;
  }

