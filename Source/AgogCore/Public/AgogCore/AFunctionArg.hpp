// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFunctionArg and AFunctionArgRtn class templates
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
// Notes      This is a function callback object template for static functions that take
//            a single argument.  This template class is used as an alternative to a
//            function argument since it can also use its member data (if derived)
//            whenever its invoke(arg) method is called.
//  
//            The fact that the data that the function acts upon is stored with this
//            function object is especially important for concurrent processing in that
//            it does not rely on global / static data and thus may operate safely in
//            more than one thread of execution simultaneously.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AFunctionArg<uint32_t>;
// Arg        _ArgType - Parameter type for the method that this class is wrapped
//            around.  It should probably be a constant reference to some type if it is
//            info only and it should be a pointer to some type if it is to be  modified
//            (and possibly read).  Note that if the argument type is a structure or
//            class, it can also have data members with "return" information.
// See Also   AMethodArg<>
//            The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// Author(s)  Conan Reis
template<class _ArgType>
class AFunctionArg : public AFunctionArgBase<_ArgType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AFunctionArg<_ArgType>     tAFunctionArg;
    typedef AFunctionArgBase<_ArgType> tAFunctionArgBase;

  // Common Methods

    AFunctionArg(void (*function_p)(_ArgType arg) = nullptr);
    AFunctionArg(const AFunctionArg & func);
    AFunctionArg & operator=(const AFunctionArg & func);
    
    // Create/allocate a lambda-based function object with one argument
    // Example AFunction::new_lambda([frank](){ int bob = frank; ADebug::print("Hello"); })
    // Note that this does not actually create an AFunction object but rather an AFunctionLambda object
    template<typename _FunctorType>
    static AFunctionArgBase<_ArgType> * new_lambda(_FunctorType&& lambda_functor);

  // Accessor Methods

    void set_function(void (*function_p)(_ArgType arg));

  // Modifying Methods

    void invoke(_ArgType arg) override;

  // Non-Modifying Methods

    virtual AFunctionArgBase<_ArgType> * copy_new() const override;

    virtual bool is_invokable() const override { return m_function_p != nullptr; }

  protected:
  // Data Members

    // Address of function to call
    void (* m_function_p)(_ArgType arg);

  };  // AFunctionArg


//---------------------------------------------------------------------------------------
// This is a function callback object template for static functions that take a single
// argument.  This template class is used as an alternative to a function argument since
// it can also use its member data (if derived) whenever its invoke(arg) method is called.
//  
// The fact that the data that the function acts upon is stored with this function object
// is especially important for concurrent processing in that it does not rely on global /
// static data and thus may operate safely in more than one thread of execution
// simultaneously.
//
// Any modifications to this template should be compile-tested by adding an explicit
// instantiation declaration such as:
//   template class AFunctionArgRtn<uint32_t, bool>;
//   
// Arg        _ArgType - Parameter type for the function / method that this class is
//            wrapped around.  It should probably be a constant reference to some type if
//            it is info only and it should be a pointer to some type if it is to be
//            modified (and possibly read).  Note that if the argument type is a
//            structure or class, it can also have data members that can be written to
//            with "return" information.
// Arg        _ReturnType - Parameter return type.           
// See Also   AMethodArgRtn<>
//            The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// Author(s)  Conan Reis
template<class _ArgType, class _ReturnType>
class AFunctionArgRtn : public AFunctionArgRtnBase<_ArgType, _ReturnType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AFunctionArgRtn<_ArgType, _ReturnType>     tAFunctionArg;
    typedef AFunctionArgRtnBase<_ArgType, _ReturnType> tAFunctionArgRtnBase;

  // Common Methods

    AFunctionArgRtn(_ReturnType (*function_p)(_ArgType arg) = nullptr);
    AFunctionArgRtn(const AFunctionArgRtn & func);
    AFunctionArgRtn & operator=(const AFunctionArgRtn & func);
    
  // Accessor Methods

    void set_function(_ReturnType (*function_p)(_ArgType arg));

  // Modifying Methods

    _ReturnType invoke(_ArgType arg);

  // Non-Modifying Methods

    virtual AFunctionArgRtnBase<_ArgType, _ReturnType> * copy_new() const override;

  protected:
  // Data Members

    // Address of function to call
    _ReturnType (* m_function_p)(_ArgType arg);

  };  // AFunctionArgRtn

//---------------------------------------------------------------------------------------
// Function object based on lambda expression
// Use AFunction::new_lambda() to create one
template<typename _FunctorType, class _ArgType>
class AFunctionArgLambda : public AFunctionArgBase<_ArgType>
  {
  public:

    AFunctionArgLambda(_FunctorType && lambda_functor) : m_functor(lambda_functor) {}

    virtual void invoke(_ArgType arg) override { m_functor(arg); }
    virtual AFunctionArgBase<_ArgType> * copy_new() const override { return new AFunctionArgLambda(*this); }

  protected:
    _FunctorType  m_functor;
  };

//=======================================================================================
// AFunctionArg Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Arg          function_p - the function to be used, if it is nullptr it is not called
//              (default nullptr)
// Examples:    AFunctionArg func(comparison_func);
// Author(s):    Conan Reis
template<class _ArgType>
AFunctionArg<_ArgType>::AFunctionArg(void (*function_p)(_ArgType arg)) :
  m_function_p(function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Arg          func - AFunctionArg to copy
// Examples:    AFunctionArg func;
//              AFunctionArg func2(func);
// Author(s):    Conan Reis
template<class _ArgType>
AFunctionArg<_ArgType>::AFunctionArg(const AFunctionArg<_ArgType> & func) :
  m_function_p(func.m_function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              func1 = func2 = func3;
// Arg          func - AFunctionArg to copy
// Examples:    func1 = func2;
// Author(s):    Conan Reis
template<class _ArgType>
AFunctionArg<_ArgType> & AFunctionArg<_ArgType>::operator=(const AFunctionArg<_ArgType> & func)
  {
  m_function_p = func.m_function_p;
  return *this;
  }

//---------------------------------------------------------------------------------------
// Create/allocate a lambda-based function object
// Example AFunctionArg<int>::new_lambda([frank](int jim){ int bob = frank + jim; ADebug::print("Hello"); })
template<class _ArgType>
template<typename _FunctorType>
AFunctionArgBase<_ArgType> * AFunctionArg<_ArgType>::new_lambda(_FunctorType&& lambda_functor)
  {
  return new AFunctionArgLambda<_FunctorType, _ArgType>((_FunctorType &&)lambda_functor);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Sets the function pointer.
// Arg          function_p - a function pointer
// Examples:    func.set_function(some_func);
// Author(s):    Conan Reis
template<class _ArgType>
inline void AFunctionArg<_ArgType>::set_function(void (*function_p)(_ArgType arg))
  {
  m_function_p = function_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored function.
// Examples:    func.invoke();
// Notes:       overridden from AFunctionBase
// Modifiers:    virtual
// Author(s):    Conan Reis
template<class _ArgType>
void AFunctionArg<_ArgType>::invoke(_ArgType arg)
  {
  if (m_function_p)
    {
    (m_function_p)(arg);
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
template<class _ArgType>
AFunctionArgBase<_ArgType> * AFunctionArg<_ArgType>::copy_new() const
  {
  tAFunctionArgBase * func_p = new tAFunctionArg(*this);

  A_VERIFY_MEMORY(func_p != nullptr, tAFunctionArg);

  return func_p;
  } 


//=======================================================================================
// AFunctionArgRtn Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Arg          function_p - the function to be used, if it is nullptr it is not called
//              (default nullptr)
// Examples:    AFunctionArgRtn func(comparison_func);
// Author(s):    Conan Reis
template<class _ArgType, class _ReturnType>
AFunctionArgRtn<_ArgType, _ReturnType>::AFunctionArgRtn(_ReturnType (*function_p)(_ArgType arg)) :
  m_function_p(function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Arg          func - AFunctionArgRtn to copy
// Examples:    AFunctionArgRtn func;
//              AFunctionArgRtn func2(func);
// Author(s):    Conan Reis
template<class _ArgType, class _ReturnType>
AFunctionArgRtn<_ArgType, _ReturnType>::AFunctionArgRtn(const AFunctionArgRtn<_ArgType, _ReturnType> & func) :
  m_function_p(func.m_function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              func1 = func2 = func3;
// Arg          func - AFunctionArgRtn to copy
// Examples:    func1 = func2;
// Author(s):    Conan Reis
template<class _ArgType, class _ReturnType>
AFunctionArgRtn<_ArgType, _ReturnType> & AFunctionArgRtn<_ArgType, _ReturnType>::operator=(const AFunctionArgRtn<_ArgType, _ReturnType> & func)
  {
  m_function_p = func.m_function_p;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Sets the function pointer.
// Arg          function_p - a function pointer
// Examples:    func.set_function(some_func);
// Author(s):    Conan Reis
template<class _ArgType, class _ReturnType>
inline void AFunctionArgRtn<_ArgType, _ReturnType>::set_function(_ReturnType (*function_p)(_ArgType arg))
  {
  m_function_p = function_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored function.
// Examples:    func.invoke();
// Notes:       If this functional object can be in an "unset" state call is_invokable()
//              to determine if it is properly set up and can be invoked.
// Modifiers:    virtual - overridden from AFunctionBase
// Author(s):    Conan Reis
template<class _ArgType, class _ReturnType>
_ReturnType AFunctionArgRtn<_ArgType, _ReturnType>::invoke(_ArgType arg)
  {
  return (m_function_p)(arg);
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
template<class _ArgType, class _ReturnType>
AFunctionArgRtnBase<_ArgType, _ReturnType> * AFunctionArgRtn<_ArgType, _ReturnType>::copy_new() const
  {
  tAFunctionArgRtnBase * func_p = new tAFunctionArg(*this);

  A_VERIFY_MEMORY(func_p != nullptr, tAFunctionArg);

  return func_p;
  } 
