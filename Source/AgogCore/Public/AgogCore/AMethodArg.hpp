// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AMethodArg, AMethodArgRtn and AMethodArg2 class templates
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFunctionArgBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      This is a function callback object template for single argument methods.
//            This template class is used as an alternative to a function argument since
//            it can also use member data from its owner object whenever its invoke(arg)
//            method is called.
//  
//            The fact that the data that the method acts upon is stored within the owner
//            object is especially important for concurrent processing in that it does
//            not rely on global / static data and thus may operate safely in more than
//            one thread of execution simultaneously.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AMethodArg<AString, uint32_t>;
// Arg        _ArgType - Parameter type for the method that this class is wrapped
//            around.  It should probably be a constant reference to some type if it is
//            info only and it should be a pointer to some type if it is to be  modified
//            (and possibly read).  Note that if the argument type is a structure or
//            class, it can also have data members with "return" information.
// Subclasses 
// See Also   AFunctionArg<>
//            The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// UsesLibs     
// InLibs     AgogCore/AgogCore.lib
// Examples:      
// Author(s)  Conan Reis
template<class _OwnerType, class _ArgType>
class AMethodArg : public AFunctionArgBase<_ArgType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AMethodArg<_OwnerType, _ArgType> tAMethodArg;
    typedef AFunctionArgBase<_ArgType>       tAFunctionArgBase;

  // Common Methods

    AMethodArg(_OwnerType * owner_p = nullptr, void (_OwnerType::* method_p)(_ArgType arg) = nullptr);
    AMethodArg(const AMethodArg & method);
    AMethodArg & operator=(const AMethodArg & method);
    
  // Accessor Methods

    _OwnerType * get_owner() const;
    void         set_method(void (_OwnerType::* method_p)(_ArgType arg));
    void         set_owner(_OwnerType * owner_p);
    //void (_OwnerType::*)() get_method() const;

  // Modifying Methods

    virtual void invoke(_ArgType arg) override;  // Calls the function

  // Non-Modifying Methods

    virtual tAFunctionArgBase * copy_new() const override;

  protected:
  // Data Members

    void (_OwnerType::* m_method_p)(_ArgType arg); // pointer to method to call
    _OwnerType *        m_owner_p;  // pointer to owner of method to call

  };  // AMethodArg


//---------------------------------------------------------------------------------------
// Notes      This is a function callback object template for single argument methods.
//            This template class is used as an alternative to a function argument since
//            it can also use member data from its owner object whenever its invoke(arg)
//            method is called.
//  
//            The fact that the data that the method acts upon is stored within the owner
//            object is especially important for concurrent processing in that it does
//            not rely on global / static data and thus may operate safely in more than
//            one thread of execution simultaneously.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AMethodArgRtn<AString, uint32_t, bool>;
// Arg        _ArgType - Parameter type for the method that this class is wrapped
//            around.  It should probably be a constant reference to some type if it is
//            info only and it should be a pointer to some type if it is to be  modified
//            (and possibly read).  Note that if the argument type is a structure or
//            class, it can also have data members with "return" information.
// Subclasses 
// See Also   AFunctionArgRtn<>
//            The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// UsesLibs     
// InLibs     AgogCore/AgogCore.lib
// Examples:      
// Author(s)  Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
class AMethodArgRtn : public AFunctionArgRtnBase<_ArgType, _ReturnType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AMethodArgRtn<_OwnerType, _ArgType, _ReturnType> tAMethodArgRtn;
    typedef AFunctionArgRtnBase<_ArgType, _ReturnType>       tAFunctionArgRtnBase;

  // Common Methods

    AMethodArgRtn(_OwnerType * owner_p = nullptr, _ReturnType (_OwnerType::* method_p)(_ArgType arg) = nullptr);
    AMethodArgRtn(const AMethodArgRtn & method);
    AMethodArgRtn & operator=(const AMethodArgRtn & method);
    
  // Accessor Methods

    _OwnerType * get_owner() const;
    void         set_method(_ReturnType (_OwnerType::* method_p)(_ArgType arg));
    void         set_owner(_OwnerType * owner_p);
    //_ReturnType (_OwnerType::*)() get_method() const;

  // Modifying Methods

    virtual _ReturnType invoke(_ArgType arg) override;  // Calls the function

  // Non-Modifying Methods

    virtual tAFunctionArgRtnBase * copy_new() const override;

	virtual bool is_invokable() const override  { return m_owner_p && m_method_p; }

  protected:
  // Data Members

    // Pointer to method to call
    _ReturnType (_OwnerType::* m_method_p)(_ArgType arg);

    // Pointer to owner of method to call
    _OwnerType * m_owner_p;

  };  // AMethodArgRtn


//---------------------------------------------------------------------------------------
// Same as AMethodArg, except taking two arguments
template<class _OwnerType, class _ArgType1, class _ArgType2>
class AMethodArg2 : public AFunctionArgBase2<_ArgType1, _ArgType2>
  {
  public:
  // Common types

    // Local shorthand
    typedef AMethodArg2<_OwnerType, _ArgType1, _ArgType2> tAMethodArg2;
    typedef AFunctionArgBase2<_ArgType1, _ArgType2>       tAFunctionArgBase2;

  // Common Methods

    AMethodArg2(_OwnerType * owner_p = nullptr, void (_OwnerType::* method_p)(_ArgType1 arg1, _ArgType2 arg2) = nullptr);
    AMethodArg2(const AMethodArg2 & method);
    AMethodArg2 & operator=(const AMethodArg2 & method);
    
  // Accessor Methods

    _OwnerType * get_owner() const;
    void         set_method(void (_OwnerType::* method_p)(_ArgType1 arg1, _ArgType2 arg2));
    void         set_owner(_OwnerType * owner_p);
    //void (_OwnerType::*)() get_method() const;

  // Modifying Methods

    virtual void invoke(_ArgType1 arg1, _ArgType2 arg2) override;  // Calls the function

  // Non-Modifying Methods

    virtual tAFunctionArgBase2 * copy_new() const override;

  protected:
  // Data Members

    void (_OwnerType::* m_method_p)(_ArgType1 arg1, _ArgType2 arg2); // pointer to method to call
    _OwnerType *        m_owner_p;  // pointer to owner of method to call

  };  // AMethodArg

//=======================================================================================
// AMethodArg Methods
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
// Examples:    AMethodArg<AWindow, bool> method(this, enable_sizing);                // inside a AWindow method
//              AMethodArg<AWindow, bool> method(&win, AWindow::enable_sizing);  // or this ouside a AWindow
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
inline AMethodArg<_OwnerType, _ArgType>::AMethodArg(
  _OwnerType *        owner_p,                     // = nullptr
  void (_OwnerType::* method_p)(_ArgType arg) // = nullptr
  ) :
  m_method_p(method_p),
  m_owner_p(owner_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          method - AMethodArg<> to copy
// Examples:    AMethodArg<SomeClass, ArgType> method1;
//              AMethodArg<SomeClass, ArgType> method2(method1);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
inline AMethodArg<_OwnerType, _ArgType>::AMethodArg(const tAMethodArg & method) :
  m_method_p(method.m_method_p),
  m_owner_p(method.m_owner_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              method1 = method2 = method3;
// Arg          method - AMethodArg<> to copy
// Examples:     method1 = method2;
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
inline AMethodArg<_OwnerType, _ArgType> & AMethodArg<_OwnerType, _ArgType>::operator=(const tAMethodArg & method)
  {
  m_method_p = method.m_method_p;
  m_owner_p  = method.m_owner_p;
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
template<class _OwnerType, class _ArgType>
inline _OwnerType * AMethodArg<_OwnerType, _ArgType>::get_owner() const
  {
  return m_owner_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the method pointer.
// Arg          method_p - the method to be used, if it is nullptr it is not called
// Examples:    method.set_method(SomeClass::some_method);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
inline void AMethodArg<_OwnerType, _ArgType>::set_method(void (_OwnerType::* method_p)(_ArgType arg))
  {
  m_method_p = method_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the owner object for the method.
// Arg          owner - the object that owns the method to be wrapped around
// Examples:    method.set_owner(&some_object);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
inline void AMethodArg<_OwnerType, _ArgType>::set_owner(_OwnerType * owner_p)
  {
  m_owner_p = owner_p;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored method on the stored owner object.
// Examples:    method.invoke();
// Notes:       overridden from AFunctionArgBase
// Modifiers:    virtual
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
void AMethodArg<_OwnerType, _ArgType>::invoke(_ArgType arg)
  {
  if (m_owner_p && m_method_p)
    {
    (m_owner_p->*m_method_p)(arg);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.  Used virtually when a dynamic copy
//              is needed, but the class is unknown.
// Returns:     new dynamic copy of itself
// Examples:    AFunctionArgBase<_ArgType> * new_method_p = some_method.copy_new();
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType>
AFunctionArgBase<_ArgType> * AMethodArg<_OwnerType, _ArgType>::copy_new() const
  {
  tAMethodArg * method_p = new tAMethodArg(*this);

  A_VERIFY_MEMORY(method_p != nullptr, tAMethodArg);

  return method_p;
  } 


//=======================================================================================
// AMethodArgRtn Methods
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
// Examples:    AMethodArgRtn<AWindow, bool> method(this, enable_sizing);                // inside a AWindow method
//              AMethodArgRtn<AWindow, bool> method(&win, AWindow::enable_sizing);  // or this ouside a AWindow
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
inline AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::AMethodArgRtn(
  _OwnerType * owner_p,  // = nullptr
  _ReturnType (_OwnerType::* method_p)(_ArgType arg) // = nullptr
  ) :
  m_owner_p(owner_p),
  m_method_p(method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          method - AMethodArgRtn<> to copy
// Examples:    AMethodArgRtn<SomeClass, ArgType> method1;
//              AMethodArgRtn<SomeClass, ArgType> method2(method1);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
inline AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::AMethodArgRtn(const tAMethodArgRtn & method) :
  m_owner_p(method.m_owner_p),
  m_method_p(method.m_method_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              method1 = method2 = method3;
// Arg          method - AMethodArgRtn<> to copy
// Examples:     method1 = method2;
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
inline AMethodArgRtn<_OwnerType, _ArgType, _ReturnType> & AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::operator=(const tAMethodArgRtn & method)
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
template<class _OwnerType, class _ArgType, class _ReturnType>
inline _OwnerType * AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::get_owner() const
  {
  return m_owner_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the method pointer.
// Arg          method_p - the method to be used, if it is nullptr it is not called
// Examples:    method.set_method(SomeClass::some_method);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
inline void AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::set_method(_ReturnType (_OwnerType::* method_p)(_ArgType arg))
  {
  m_method_p = method_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the owner object for the method.
// Arg          owner - the object that owns the method to be wrapped around
// Examples:    method.set_owner(&some_object);
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
inline void AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::set_owner(_OwnerType * owner_p)
  {
  m_owner_p = owner_p;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored method on the stored owner object.
// Examples:    method.invoke();
// Notes:       If this functional object can be in an "unset" state call is_invokable()
//              to determine if it is properly set up and can be invoked.
// Modifiers:    virtual - overridden from AFunctionBase
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
_ReturnType AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::invoke(_ArgType arg)
  {
  return (m_owner_p->*m_method_p)(arg);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.  Used virtually when a dynamic copy
//              is needed, but the class is unknown.
// Returns:     new dynamic copy of itself
// Examples:    AFunctionArgRtnBase<_ArgType, _ReturnType> * new_method_p = some_method.copy_new();
// Author(s):    Conan Reis
template<class _OwnerType, class _ArgType, class _ReturnType>
AFunctionArgRtnBase<_ArgType, _ReturnType> * AMethodArgRtn<_OwnerType, _ArgType, _ReturnType>::copy_new() const
  {
  tAMethodArgRtn * method_p = new tAMethodArgRtn(*this);

  A_VERIFY_MEMORY(method_p != nullptr, tAMethodArgRtn);

  return method_p;
  } 

//=======================================================================================
// AMethodArg2 Methods
//=======================================================================================


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::AMethodArg2(
  _OwnerType *        owner_p,                     // = nullptr
  void (_OwnerType::* method_p)(_ArgType1 arg1, _ArgType2 arg2) // = nullptr
  ) :
  m_method_p(method_p),
  m_owner_p(owner_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::AMethodArg2(const tAMethodArg2 & method) :
  m_method_p(method.m_method_p),
  m_owner_p(method.m_owner_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline AMethodArg2<_OwnerType, _ArgType1, _ArgType2> & AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::operator=(const tAMethodArg2 & method)
  {
  m_method_p = method.m_method_p;
  m_owner_p  = method.m_owner_p;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets the pointer to the owner object for the method.
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline _OwnerType * AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::get_owner() const
  {
  return m_owner_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the method pointer.
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline void AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::set_method(void (_OwnerType::* method_p)(_ArgType1 arg1, _ArgType2 arg2))
  {
  m_method_p = method_p;
  }

//---------------------------------------------------------------------------------------
//  Sets the owner object for the method.
template<class _OwnerType, class _ArgType1, class _ArgType2>
inline void AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::set_owner(_OwnerType * owner_p)
  {
  m_owner_p = owner_p;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored method on the stored owner object.
template<class _OwnerType, class _ArgType1, class _ArgType2>
void AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::invoke(_ArgType1 arg1, _ArgType2 arg2)
  {
  if (m_owner_p && m_method_p)
    {
    (m_owner_p->*m_method_p)(arg1, arg2);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.
template<class _OwnerType, class _ArgType1, class _ArgType2>
AFunctionArgBase2<_ArgType1, _ArgType2> * AMethodArg2<_OwnerType, _ArgType1, _ArgType2>::copy_new() const
  {
  tAMethodArg2 * method_p = new tAMethodArg2(*this);

  A_VERIFY_MEMORY(method_p != nullptr, tAMethodArg2);

  return method_p;
  } 


