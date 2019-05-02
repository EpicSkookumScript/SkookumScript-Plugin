// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AMethod class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFunctionBase.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

// This casts a void method() of some object that is a _OwnerType so that it may be
// stored in a AMethod function object.
//define METHOD(method_name) (void (_OwnerType: :*)())method_name


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    This is a function callback object template for methods.  This template class
//          is used in the place of a function argument since it can also use member data
//          from its owner object whenever it is called/invoked.  The fact that the data
//          that the method acts upon is stored within the owner object is especially
//          important for concurrent processing in that it does not rely on global data
//          and thus may operate safely in more than one thread of execution
//          simultaneously.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class AMethod<AStringBM>;
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Examples:    
// Author   Conan Reis
template<class _OwnerType>
class AMethod : public AFunctionBase
  {
  public:
  // Common types

    typedef AMethod<_OwnerType> tAMethod;  // Local shorthand for tAMethod template

  // Common Methods

    AMethod(_OwnerType * owner_p = nullptr, void (_OwnerType::* method_p)() = nullptr);
    AMethod(const AMethod & method);
    AMethod<_OwnerType> & operator=(const AMethod & method);
    
  // Accessor Methods

    _OwnerType * get_owner() const;
    void         set_method(void (_OwnerType::* method_p)());
    void         set_owner(_OwnerType * owner_p);
    //void (_OwnerType::*)() get_method() const;

  // Modifying Methods

    void invoke();  // Calls the function

  // Non-Modifying Methods

    virtual AFunctionBase * copy_new() const override;

  protected:
  // Data Members

    void (_OwnerType::* m_method_p)(); // pointer to method to call
    _OwnerType *        m_owner_p;     // pointer to owner of method to call

  };  // AMethod


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
// Examples:    AMethod<Window> method(this, on_tick);          // inside a Window method
//              AMethod<Window> method(&win, Window::on_tick);  // or this ouside a Window
// Author(s):    Conan Reis
template<class _OwnerType>
inline AMethod<_OwnerType>::AMethod(
  _OwnerType *        owner_p,    // = nullptr
  void (_OwnerType::* method_p)() // = nullptr
  ) :
  m_owner_p(owner_p),
  m_method_p(method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          method - AMethod<> to copy
// Examples:    AMethod<SomeClass> method;
//              AMethod<SomeClass> method2(method);
// Author(s):    Conan Reis
template<class _OwnerType>
inline AMethod<_OwnerType>::AMethod(const tAMethod & method) :
  m_owner_p(method.m_owner_p),
  m_method_p(method.m_method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              method1 = method2 = method3;
// Arg          method - AMethod<> to copy
// Examples:     method1 = method2;
// Author(s):    Conan Reis
template<class _OwnerType>
inline AMethod<_OwnerType> & AMethod<_OwnerType>::operator=(const tAMethod & method)
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
template<class _OwnerType>
inline _OwnerType * AMethod<_OwnerType>::get_owner() const
  {
  return m_owner_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the method pointer.
// Arg          method_p - the method to be used, if it is nullptr it is not called
// Examples:    method.set_method(SomeClass::some_method);
// Author(s):    Conan Reis
template<class _OwnerType>
inline void AMethod<_OwnerType>::set_method(void (_OwnerType::* method_p)())
  {
  m_method_p = method_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the owner object for the method.
// Arg          owner - the object that owns the method to be wrapped around
// Examples:    method.set_owner(&some_object);
// Author(s):    Conan Reis
template<class _OwnerType>
inline void AMethod<_OwnerType>::set_owner(_OwnerType * owner_p)
  {
  m_owner_p = owner_p;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored method on the stored owner object.
// Examples:    method.invoke();
// Notes:       overridden from AFunctionBase
// Modifiers:    virtual
// Author(s):    Conan Reis
template<class _OwnerType>
void AMethod<_OwnerType>::invoke()
  {
  if ((m_owner_p) && (m_method_p != nullptr))
    {
    (m_owner_p->*m_method_p)();
    }
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
template<class _OwnerType>
AFunctionBase * AMethod<_OwnerType>::copy_new() const
  {
  AFunctionBase * func_p = new tAMethod(*this);

  A_VERIFY_MEMORY(func_p != nullptr, tAMethod);

  return func_p;
  } 
