// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFunctionArgBase and AFunctionArgRtnBase class templates
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

// This is intentionally placed outside the header ifdef block.
#include <AgogCore/AgogCore.hpp>
#include <AgogCore/AMemory.hpp>


#ifndef __AFUNCTIONARGBASE_HPP
#define __AFUNCTIONARGBASE_HPP


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

// Returns a new dynamic copy of a class instance that is or is derived from the class
// specified by _BaseClass.  Used virtually when a dynamic copy is needed, but the class
// is unknown.
#define AFUNC_COPY_NEW_DEF(_BaseClass) public: virtual _BaseClass * copy_new() const override;

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.  Used virtually when a dynamic copy
//              is needed, but the class is unknown.
// Returns:     new dynamic copy of itself
// Examples:     _BaseClass * new_obj_p = some_obj.copy_new();
// Notes:       The class indicated by _FuncClass must have a copy constructor.
// Author(s):    Conan Reis
#define AFUNC_COPY_NEW_DECL(_FuncClass, _BaseClass) \
  _BaseClass * _FuncClass::copy_new() const \
    { \
    _BaseClass * func_obj_p = (new _FuncClass(*this)); \
    A_VERIFY_MEMORY(func_obj_p != nullptr, _FuncClass); \
    return func_obj_p; \
    } 


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      AFunctionArgBase defines a class of function callback template objects that
//            take a parameterized argument.  This template class is used as an
//            alternative to a function argument since it can also use data members
//            included with it (if derived) whenever its invoke(arg) method is called.
//
//            The fact that the data that the function acts upon is stored with this
//            function object is especially important for concurrent processing in that
//            it does not rely on global / static data and thus may operate safely in
//            more than one thread of execution simultaneously.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AFunctionArgBase<uint32_t>;
// Arg        _ArgType - Parameter type for the function / method that this class is
//            wrapped around.  It should probably be a constant reference to some type if
//            it is info only and it should be a pointer to some type if it is to be
//            modified (and possibly read).  Note that if the argument type is a
//            structure or class, it can also have data members with "return" information.
// Subclasses AFunctionArg<>, AMethodArg<>
// See Also   The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// Examples:      AFunctionArgBase<const InfoClass &>
// Author(s)  Conan Reis
template<class _ArgType>
class AFunctionArgBase
  {
  public:

    A_NEW_OPERATORS(AFunctionArgBase);

  // Common types

    typedef AFunctionArgBase<_ArgType> tAFunctionArgBase;  // Local shorthand for AFunctionArgBase template

  // Common Methods

    virtual ~AFunctionArgBase() {}

  // Modifying Methods

    virtual void invoke(_ArgType arg) = 0;

  // Non-Modifying Methods

    virtual tAFunctionArgBase * copy_new() const = 0;

	  virtual bool                is_invokable() const  { return true; }

  };  // AFunctionArgBase


//---------------------------------------------------------------------------------------
// AFunctionArgRtnBase defines a class of function callback template objects that take a
// parameterized argument and return type.  This template class is used as an alternative
// to a function argument since it can also use data members included with it (if derived)
// whenever its invoke(arg) method is called.
//
// The fact that the data that the function acts upon is stored with this function object
// is especially important for concurrent processing in that it does not rely on global /
// static data and thus may operate safely in more than one thread of execution
// simultaneously.
//
// Any modifications to this template should be compile-tested by adding an explicit
// instantiation declaration such as:
//   template class AFunctionArgRtnBase<uint32_t, bool>;
//   
// Arg        _ArgType - Parameter type for the function / method that this class is
//            wrapped around.  It should probably be a constant reference to some type if
//            it is info only and it should be a pointer to some type if it is to be
//            modified (and possibly read).  Note that if the argument type is a
//            structure or class, it can also have data members that can be written to
//            with "return" information.
// Arg        _ReturnType - return type.
// Subclasses AFunctionArgRtn<>, AMethodArgRtn<>
// See Also   The AFunctionBase class and its derived classes (AFunction, AMethod<>, and
//            SimpleFunc) for function objects that have no parameters (i.e. void).
// Examples:      AFunctionArgRtnBase<const InfoClass &>
// Author(s)  Conan Reis
template<class _ArgType, class _ReturnType>
class AFunctionArgRtnBase
  {
  public:
  // Common types

    typedef AFunctionArgRtnBase<_ArgType, _ReturnType> tAFunctionArgRtnBase;  // Local shorthand for AFunctionArgRtnBase template

  // Common Methods

    virtual ~AFunctionArgRtnBase() {}

  // Modifying Methods

    virtual _ReturnType invoke(_ArgType arg) = 0;

  // Non-Modifying Methods

    virtual AFunctionArgRtnBase * copy_new() const = 0;

  };  // AFunctionArgRtnBase

//---------------------------------------------------------------------------------------
// Same as AFunctionArgBase, but taking two arguments
template<class _ArgType1, class _ArgType2>
class AFunctionArgBase2
  {
  public:

    A_NEW_OPERATORS(AFunctionArgBase2);

  // Common types

    typedef AFunctionArgBase2<_ArgType1, _ArgType2> tAFunctionArgBase2;  // Local shorthand for AFunctionArgBase template

  // Common Methods

    virtual ~AFunctionArgBase2() {}

  // Modifying Methods

    virtual void invoke(_ArgType1 arg1, _ArgType2 arg2) = 0;

  // Non-Modifying Methods

    virtual tAFunctionArgBase2 * copy_new() const = 0;

	  virtual bool                 is_invokable() const  { return true; }

  };  // AFunctionArgBase2

#endif  // __AFUNCTIONARGBASE_HPP

