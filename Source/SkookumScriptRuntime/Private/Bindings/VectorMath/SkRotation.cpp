// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript rotation/quaternion class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkRotation.hpp"
#include "SkRotationAngles.hpp"

#include <SkookumScript/SkBoolean.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkRotation_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Rotation@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FQuat & rotation = scope_p->this_as<SkRotation>();
      FRotator rotator(rotation);
      AString str(128u, "(yaw=%g, pitch=%g, roll=%g)", double(rotator.Yaw), double(rotator.Pitch), double(rotator.Roll));

      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Rotation@RotationAngles() String
  // # Author(s): Markus Breyer
  static void mthd_RotationAngles(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FQuat & rotation = scope_p->this_as<SkRotation>();
      *result_pp = SkRotationAngles::new_instance(FRotator(rotation));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Rotation@zero() Rotation
  // # Author(s): Markus Breyer
  static void mthd_zero(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkRotation>() = FQuat::Identity;

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Rotation@zero?() Boolean
  // # Author(s): Markus Breyer
  static void mthd_zeroQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FQuat & rotation = scope_p->this_as<SkRotation>();
      *result_pp = SkBoolean::new_instance(rotation.Equals(FQuat::Identity));
      }
    }

  //---------------------------------------------------------------------------------------

  // Instance method array
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "String",         mthd_String },
      { "RotationAngles", mthd_RotationAngles },

      { "zero?",          mthd_zeroQ },
      { "zero",           mthd_zero },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkRotation::register_bindings()
  {
  tBindingBase::register_bindings("Rotation");

  ms_class_p->register_method_func_bulk(SkRotation_Impl::methods_i, A_COUNT_OF(SkRotation_Impl::methods_i), SkBindFlag_instance_no_rebind);

  ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_struct<SkRotation>);
  SkUEClassBindingHelper::resolve_raw_data_struct(ms_class_p, TEXT("Quat"));
  }

//---------------------------------------------------------------------------------------

SkClass * SkRotation::get_class()
  {
  return ms_class_p;
  }
