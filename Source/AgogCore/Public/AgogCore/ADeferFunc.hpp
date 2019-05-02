// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADeferFunc class declaration header
//=======================================================================================

#pragma once 

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFunction.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogCore/APArray.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
class A_API ADeferFunc
  {
  public:

  // Class Methods

    static void post_func_obj(AFunctionBase * func_p);
    static void post_func(void (*function_f)());

    template<class _OwnerType>
      static void post_method(_OwnerType * owner_p, void (_OwnerType::* method_m)());

    static void invoke_deferred();


  // Class Data Members

    static APArrayFree<AFunctionBase> ms_deferred_funcs;

  };


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Calls specified function object once invoke_deferred() is called - usually
//             at the end of a main loop or frame update.
//             This is convenient for some tasks that cannot occur immediately - which
//             is often true for events.  It allows the callstack to unwind and calls
//             the function at a less 'deep' location.
// Arg         func_p - pointer to function object to invoke at a later time.
// See:        ATimer
// Author(s):   Conan Reis
inline void ADeferFunc::post_func_obj(AFunctionBase * func_p)
  {
  ms_deferred_funcs.append(*func_p);
  }

//---------------------------------------------------------------------------------------
// Calls specified function once invoke_deferred() is called - usually at the
//             end of a main loop or frame update.
//             This is convenient for some tasks that cannot occur immediately - which
//             is often true for events.  It allows the callstack to unwind and calls
//             the function at a less 'deep' location.
// Arg         function_f - pointer to method to invoke at a later time.
// See:        ATimer
// Author(s):   Conan Reis
inline void ADeferFunc::post_func(void (*function_f)())
  {
  post_func_obj(new AFunction(function_f));
  }

//---------------------------------------------------------------------------------------
// Calls specified method once invoke_deferred() is called - usually at the
//             end of a main loop or frame update.
//             This is convenient for some tasks that cannot occur immediately - which
//             is often true for events.  It allows the callstack to unwind and calls
//             the function at a less 'deep' location.
// Arg         method_m - pointer to method to invoke at a later time.
// See:        ATimer
// Author(s):   Conan Reis
template<class _OwnerType>
inline void ADeferFunc::post_method(
  _OwnerType *        owner_p,
  void (_OwnerType::* method_m)())
  {
  post_func_obj(new AMethod<_OwnerType>(owner_p, method_m));
  }
