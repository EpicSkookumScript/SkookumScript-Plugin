// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ACompareMethod class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ACompareBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    This is a comparison function object template for methods.  This template
//          class is used in the place of a comparison function argument since it can
//          also use member data from its owner object whenever it is called/invoked.
//          The fact that the data that the method acts upon is stored within the owner
//          object is especially important for concurrent processing in that it does not
//          rely on global data and thus may operate safely in more than one thread of
//          execution simultaneously.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class ACompareMethod<AString, AStringBM>;
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Examples:    
// Author   Conan Reis
template<class _OwnerType, class _ElementType>
class ACompareMethod : public ACompareBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand
    typedef ACompareMethod<_OwnerType, _ElementType> tACompareMethod;  // Local shorthand for this template
    typedef ACompareBase<_ElementType>               tACompareBase;

  // Common Methods

    ACompareMethod(_OwnerType * owner_p = nullptr, eAEquate (_OwnerType::* method_p)(const _ElementType & lhs, const _ElementType & rhs) = nullptr);
    ACompareMethod(const ACompareMethod & method);
    ACompareMethod & operator=(const ACompareMethod & method);
    
  // Accessor Methods

    _OwnerType * get_owner() const;
    void         set_method(eAEquate (_OwnerType::* method_p)(const _ElementType & lhs, const _ElementType & rhs));
    void         set_owner(_OwnerType * owner_p);
    //void (_OwnerType::*)() get_method() const;

  // Modifying Methods

    eAEquate compare(const _ElementType & lhs, const _ElementType & rhs);

  // Non-Modifying Methods

    virtual ACompareBase<_ElementType> * copy_new() const override;

  protected:
  // Data Members

    // Pointer to method to call
    eAEquate (_OwnerType::* m_method_p)(const _ElementType & lhs, const _ElementType & rhs);

    // Pointer to owner of method to call
    _OwnerType * m_owner_p;

  };  // ACompareMethod


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Arg          owner - the object that owns the method to be wrapped around
// Arg          method_p - the method to be used, if it is nullptr it is not called
//              (default nullptr)
// Examples:    ACompareMethod<AString, Window> method(this, compare);          // inside a Window method
//              ACompareMethod<AString, Window> method(&win, Window::compare);  // or this ouside a Window
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline ACompareMethod<_OwnerType, _ElementType>::ACompareMethod(
  _OwnerType *            owner_p,                                                      // = nullptr
  eAEquate (_OwnerType::* method_p)(const _ElementType & lhs, const _ElementType & rhs) // = nullptr
  ) :
  m_owner_p(owner_p),
  m_method_p(method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          method - ACompareMethod<> to copy
// Examples:    ACompareMethod<ElementClass, MethodClass> method;
//              ACompareMethod<ElementClass, MethodClass> method2(method);
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline ACompareMethod<_OwnerType, _ElementType>::ACompareMethod(const tACompareMethod & method) :
  m_owner_p(method.m_owner_p),
  m_method_p(method.m_method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              method1 = method2 = method3;
// Arg          method - ACompareMethod<> to copy
// Examples:    method1 = method2;
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline ACompareMethod<_OwnerType, _ElementType> & ACompareMethod<_OwnerType, _ElementType>::operator=(const tACompareMethod & method)
  {
  m_owner_p  = method.m_owner_p;
  m_method_p = method.m_method_p;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets the pointer to the owner object for the method.
// Returns:     pointer to the owner object for the method.
// Examples:    _OwnerType * owner_p = method.get_owner();
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline _OwnerType * ACompareMethod<_OwnerType, _ElementType>::get_owner() const
  {
  return m_owner_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the method pointer.
// Arg          method_p - the method to be used, if it is nullptr it is not called
// Examples:    method.set_method(SomeClass::some_method);
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline void ACompareMethod<_OwnerType, _ElementType>::set_method(eAEquate (_OwnerType::* method_p)(const _ElementType & lhs, const _ElementType & rhs))
  {
  m_method_p = method_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the owner object for the method.
// Arg          owner - the object that owns the method to be wrapped around
// Examples:    method.set_owner(&some_object);
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
inline void ACompareMethod<_OwnerType, _ElementType>::set_owner(_OwnerType * owner_p)
  {
  m_owner_p = owner_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored comparison method on the stored owner object.
// Arg          lhs - left hand side element to compare
// Arg          rhs - right hand side element to compare
// Examples:    method.compare();
// Notes:       If the internally stored comparison method or object owning the method is
//              nullptr, the elements are sorted by their memory address locations.
// Modifiers:    virtual - overridden from ACompareBase
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
eAEquate ACompareMethod<_OwnerType, _ElementType>::compare(
  const _ElementType & lhs,
  const _ElementType & rhs
  )
  {
  if ((m_owner_p) && (m_method_p))
    {
    return (m_owner_p->*m_method_p)(lhs, rhs);
    }

  // Compare memory address by default
  ptrdiff_t result = &lhs - &rhs;

  return A_INT_AS_EQUATE(result);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.  Used virtually when a dynamic copy
//              is needed, but the class is unknown.
// Returns:     new dynamic copy of itself
// Examples:    AFunctionBase * new_method_p = some_method.copy_new();
// Author(s):    Conan Reis
template<class _OwnerType, class _ElementType>
ACompareBase<_ElementType> * ACompareMethod<_OwnerType, _ElementType>::copy_new() const
  {
  ACompareMethod<_OwnerType, _ElementType> * func_p = new tACompareMethod(*this);

  A_VERIFY_MEMORY(func_p != nullptr, tACompareMethod);

  return func_p;
  } 
