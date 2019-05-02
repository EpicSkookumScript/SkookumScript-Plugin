// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Boolean (true/false) class - allows short circuit evaluation.
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkBoolean_Impl
  {

  //---------------------------------------------------------------------------------------
  // Skoo Params Integer() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_Integer(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(tSkInteger(scope_p->this_as<SkBoolean>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params as_string() AString
  // Returns AString representation of itself
  // Returns:    "true" or "false"
  // C++ Arg     scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // C++ Arg     result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Author(s):   Conan Reis
  static void mthd_as_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkString::new_instance(scope_p->this_as<SkBoolean>() ? AString("true", 4u) : AString("false", 5u));
      }
    }

  //---------------------------------------------------------------------------------------
  // =
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Skoo Params (Boolean operand) Boolean
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->this_as<SkBoolean>() == scope_p->get_arg<SkBoolean>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // ~
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Skoo Params (Boolean operand) Boolean
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_not(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(!scope_p->this_as<SkBoolean>());
      }
    }

  //---------------------------------------------------------------------------------------
  // :=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Skoo Params (Boolean operand) Boolean
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->this_as<SkBoolean>() != scope_p->get_arg<SkBoolean>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // :=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Skoo Params (Boolean operand) Boolean
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_nxor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        !(scope_p->this_as<SkBoolean>() ^ scope_p->get_arg<SkBoolean>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // :=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Skoo Params (Boolean operand) Boolean
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_xor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->this_as<SkBoolean>() ^ scope_p->get_arg<SkBoolean>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
        { "Integer", mthd_Integer },
        { "String", mthd_as_string },
        { "equal?", mthd_op_equals },
        { "not", mthd_op_not },
        { "not_equal?", mthd_op_not_equal },
        { "nxor", mthd_op_nxor },
        { "xor", mthd_op_xor },
    };

  } // namespace

//---------------------------------------------------------------------------------------
// Registers the atomic classes, coroutines, etc.
// Notes:      This method is called by Brain::register_bindings()
// Modifiers:   static
// Author(s):   Conan Reis
void SkBoolean::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Boolean);

  ms_class_p->register_method_func_bulk(SkBoolean_Impl::methods_i, A_COUNT_OF(SkBoolean_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkBoolean::get_class()
  {
  return ms_class_p;
  }

