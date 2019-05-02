// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Name (= FName) class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkUEName.hpp"

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkString.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkUEName_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Name@!(String name) Name
  // # Author(s): Markus Breyer
  static void mthd_ctor_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    const AString & str = scope_p->get_arg<SkString>(SkArg_1);
    this_p->construct<SkUEName>(FName(str.as_cstr()));
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Name@!none() Name
  // # Author(s): Markus Breyer
  static void mthd_ctor_none(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    this_p->construct<SkUEName>(NAME_None);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Name@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FName & name = scope_p->this_as<SkUEName>();
      ANSICHAR string_name[NAME_SIZE];
      name.GetPlainANSIString(string_name);
      *result_pp = SkString::new_instance(AString(string_name));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params = equal?(Name num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_equals(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkUEName>() == scope_p->get_arg<SkUEName>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params ~= not_equal?(Name num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_not_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkUEName>() != scope_p->get_arg<SkUEName>(SkArg_1)));
      }
    }
    
  //---------------------------------------------------------------------------------------
  // # Skookum:   String@Name() Name
  // # Author(s): Markus Breyer
  static void mthd_String_to_Name(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const AString & str = scope_p->this_as<SkString>();
      *result_pp = SkUEName::new_instance(FName(str.as_cstr()));
      }
    }

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!",            mthd_ctor_string },
      { "!none",        mthd_ctor_none },
      { "String",       mthd_String },
      { "equal?",       mthd_op_equals },
      { "not_equal?",   mthd_op_not_equal },
    };

} // namespace

//---------------------------------------------------------------------------------------
void SkUEName::register_bindings()
  {
  tBindingBase::register_bindings("Name");

  ms_class_p->register_method_func_bulk(SkUEName_Impl::methods_i, A_COUNT_OF(SkUEName_Impl::methods_i), SkBindFlag_instance_no_rebind);

  // Hook up extra String methods
  SkString::get_class()->register_method_func("Name", SkUEName_Impl::mthd_String_to_Name);
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEName::get_class()
  {
  return ms_class_p;
  }

