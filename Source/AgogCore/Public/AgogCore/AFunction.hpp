// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFunction class declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFunctionBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    This is a function callback object.  This class is used in the place of a
//          function argument since it can also use data included with it in any
//          evaluations that take place (if derived).  The fact that the data that the
//          function acts upon is stored with this functional object is especially
//          important for concurrent processing in that it does not rely on global data
//          and thus may operate safely in more than one thread of execution
//          simultaneously.
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Examples:    
// Author   Conan Reis
class A_API AFunction : public AFunctionBase
  {
  public:
  // Common Methods

    AFunction(void (*function_f)() = nullptr);
    AFunction(const AFunction & func);
    AFunction & operator=(const AFunction & func);
    
    // Create/allocate a lambda-based function object with no arguments
    // Example AFunction::new_lambda([frank](){ int bob = frank; ADebug::print("Hello"); })
    // Note that this does not actually create an AFunction object but rather an AFunctionLambda object
    template<typename _FunctorType>
    static AFunctionBase * new_lambda(_FunctorType&& lambda_functor);

    AFUNC_COPY_NEW_DEF(AFunctionBase)  // Adds copy_new()

  // Accessor Methods

    void set_function(void (*function_f)());

  // Modifying Methods

    virtual void invoke() override;

  // Comparison

    virtual bool operator == (void (*function_f)()) const override;

  protected:
  // Data Members

    // Address of function to call
    void (* m_function_f)();

  };  // AFunction

//---------------------------------------------------------------------------------------
// Function object based on lambda expression
// Use AFunction::new_lambda() to create one
template<typename _FunctorType>
class AFunctionLambda : public AFunctionBase
  {
  public:

    AFunctionLambda(_FunctorType && lambda_functor) : m_functor(lambda_functor) {}

    virtual void invoke() override { m_functor(); }
    virtual AFunctionBase * copy_new() const override { return new AFunctionLambda(*this); }

  protected:
    _FunctorType  m_functor;
  };

//=======================================================================================
// Inline Functions
//=======================================================================================


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Default constructor
// Returns:    itself
// Arg         function_f - the function to be used, if it is nullptr it is not called
// Examples:   AFunction func(comparison_func);
// Author(s):   Conan Reis
inline AFunction::AFunction(void (*function_f)()) :
  m_function_f(function_f)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Returns:    itself
// Arg         func - AFunction to copy
// Examples:   AFunction func;
//             AFunction func2(func);
// Author(s):   Conan Reis
inline AFunction::AFunction(const AFunction & func) :
  m_function_f(func.m_function_f)
  {
  }

//---------------------------------------------------------------------------------------
// Assignment operator
// Returns:    a reference to itself to allow assignment stringization
//             func1 = func2 = func3;
// Arg         func - AFunction to copy
// Examples:   func1 = func2;
// Author(s):   Conan Reis
inline AFunction & AFunction::operator=(const AFunction & func)
  {
  m_function_f = func.m_function_f;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Create/allocate a lambda-based function object with no arguments
// Example AFunction::new_lambda([frank](){ int bob = frank; ADebug::print("Hello"); })
template<typename _FunctorType>
AFunctionBase * AFunction::new_lambda(_FunctorType&& lambda_functor)
  {
  return new AFunctionLambda<_FunctorType>((_FunctorType &&)lambda_functor);
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Sets the function pointer.
// Arg         function_f - a function pointer
// Examples:   func.set_function(some_func);
// Author(s):   Conan Reis
inline void AFunction::set_function(void (*function_f)())
  {
  m_function_f = function_f;
  }

