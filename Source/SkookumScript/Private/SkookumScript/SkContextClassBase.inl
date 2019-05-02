// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// SkookumScript C++ library.
//
// Object class with extra context abstract base class
// Author(s):   Conan Reis
// Notes:          
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>


//=======================================================================================
// SkContextClassBase Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Decrements the reference count to this object and if the reference count
//             becomes 0 call on_no_references()
// See:        reference(), dereference_delay(), on_no_references()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkContextClassBase::dereference()
  {
  ARefCountMix<SkContextClassBase>::dereference();
  }

//---------------------------------------------------------------------------------------
// Same as dereference() in that it decrements the reference count to this
//             object, but it does not call on_no_references() if the reference count
//             becomes 0.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkContextClassBase::dereference_delay() const
  {
  ARefCountMix<SkContextClassBase>::dereference_delay();
  }

//---------------------------------------------------------------------------------------
// Increments the reference count to this object.
// See:        dereference(), dereference_delay()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE void SkContextClassBase::reference() const
  {
  ARefCountMix<SkContextClassBase>::reference();
  }

//---------------------------------------------------------------------------------------
// Returns class type of data member or nullptr if it does not exist
// Returns:    class type of data member or nullptr if it does not exist
// Arg         data_name - name of data member
// See:        as_finalized_generic()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkTypedName * SkContextClassBase::get_data_type(
  const ASymbol & data_name,
  eSkScope *      scope_p,  // = nullptr
  uint32_t *      data_idx_p,         // = nullptr
  SkClass **      data_owner_class_pp // = nullptr
  ) const
  {
  return m_class_p->get_data_type(data_name, scope_p, data_idx_p, data_owner_class_pp);
  }

//---------------------------------------------------------------------------------------
// Returns key/main class
// Returns:    key/main class
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkClass * SkContextClassBase::get_key_class() const
  {
  return m_class_p;
  }

//---------------------------------------------------------------------------------------
// Get the name of the wrapped class
// Returns:    name of the wrapped class
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE const ASymbol & SkContextClassBase::get_key_class_name() const
  {
  return m_class_p->m_name;
  }

//---------------------------------------------------------------------------------------
// Returns a MetaClass version of itself - i.e. the class as an instance.
// Returns:    MetaClass version of itself
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMetaClass & SkContextClassBase::get_metaclass() const
  {
  return m_class_p->m_metaclass;
  }

//---------------------------------------------------------------------------------------
// Gets the named method.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkContextClassBase::find_method(const ASymbol & method_name, bool * is_class_member_p) const
  {
  return m_class_p->find_method(method_name, is_class_member_p);
  }

//---------------------------------------------------------------------------------------
// Gets the named method from this class or a superclass.
// Returns:    A pointer to the requested method or nullptr if it does not exist.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE SkMethodBase * SkContextClassBase::find_method_inherited(const ASymbol & method_name, bool * is_class_member_p) const
  {
  return m_class_p->find_method_inherited(method_name, is_class_member_p);
  }

//---------------------------------------------------------------------------------------
// Appends (or replaces existing) method with the given name and parameters
//             using this class as its class scope.
// Arg         method_p - method to append / replace
// See:        Transfer constructor of SkParameters.
// Notes:      Used when parsing / reading in compiled binary code
//             The parser ensures that there are no methods with duplicate names - this
//             method does not.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE void SkContextClassBase::append_method(SkMethodBase * method_p, bool * has_signature_changed_p)
  {
  m_class_p->append_method(method_p, has_signature_changed_p);
  }

//---------------------------------------------------------------------------------------
// Determines if class method has been previously registered in this class
//             (without searching through inherited methods).
// Returns:    true if method has been previously registered, false if not
// Arg         method_name - name of method
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkContextClassBase::is_method_valid(const ASymbol & method_name) const
  {
  return m_class_p->is_method_valid(method_name);
  }

//---------------------------------------------------------------------------------------
// Determines if the class method is available
// Returns:    true if method is available, false if not.
// Arg         method_name - name of method.
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkContextClassBase::is_method_inherited_valid(const ASymbol & method_name) const
  {
  return m_class_p->is_method_inherited_valid(method_name);
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
A_INLINE void SkContextClassBase::append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p)
  {
  m_class_p->append_coroutine(coroutine_p, has_signature_changed_p);
  }

//---------------------------------------------------------------------------------------
//  Gets the named coroutine.
// Returns:     A pointer to the requested coroutine or nullptr if it does not exist.
// Arg          coroutine_name - name of coroutine.
// See:         find_coroutine(coroutine_qual), find_coroutine(coroutine_call)
// Author(s):    Conan Reis
A_INLINE SkCoroutineBase * SkContextClassBase::find_coroutine_inherited(const ASymbol & coroutine_name) const
  {
  return m_class_p->find_coroutine_inherited(coroutine_name);
  }

//---------------------------------------------------------------------------------------
// Determines if coroutine has been previously registered in this class
//             (without searching through inherited methods).
// Returns:    true if coroutine has been previously registered, false if not
// Arg         method_name - name of method
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE bool SkContextClassBase::is_coroutine_valid(const ASymbol & coroutine_name) const
  {
  return m_class_p->is_coroutine_valid(coroutine_name);
  }

//---------------------------------------------------------------------------------------
// Determines if coroutine is registered - i.e. it exists and it is not a
//             placeholder.
// Returns:    true if coroutine is registered
// Arg         coroutine_name - name of coroutine to check
// Author(s):   Conan Reis
A_INLINE bool SkContextClassBase::is_coroutine_registered(const ASymbol & coroutine_name) const
  {
  return m_class_p->is_coroutine_registered(coroutine_name);
  }

//---------------------------------------------------------------------------------------
// Adds a data member with the specified name to this class.
// Arg         name - name of class data member.  It must be unique - no superclass nor
//             subclass may have it and there should be no instance data member with the
//             same name either.
// Arg         type_p - class type of data member
// See:        append_class_data()
// Modifiers:   virtual - overridden from SkClassUnaryBase
// Author(s):   Conan Reis
A_INLINE SkTypedName * SkContextClassBase::append_data_member(
  const ASymbol &   name,
  SkClassDescBase * type_p
  )
  {
  return m_class_p->append_data_member(name, type_p);
  }

//---------------------------------------------------------------------------------------
// Adds a data member with the specified name to this class.
A_INLINE SkTypedNameRaw * SkContextClassBase::append_data_member_raw(
  const ASymbol &   name,
  SkClassDescBase * type_p,
  const AString &   bind_name
  )
  {
  return m_class_p->append_data_member_raw(name, type_p, bind_name);
  }

