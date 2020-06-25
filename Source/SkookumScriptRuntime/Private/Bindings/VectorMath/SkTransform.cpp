// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript transform (position + rotation + scale) class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkTransform.hpp"
#include "SkVector3.hpp"
#include "SkRotation.hpp"

#include "UObject/Package.h"

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkTransform_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FTransform &  transform = scope_p->this_as<SkTransform>();
      const FVector &     location = transform.GetLocation();
      const FRotator      rotator(transform.GetRotation());
      const FVector &     scale = transform.GetScale3D();
      AString             str(128u, "t=(%g, %g, %g) yaw=%g pitch=%g roll=%g s=(%g, %g, %g)", 
                            double(location.X), double(location.Y), double(location.Z),
                            double(rotator.Yaw), double(rotator.Pitch), double(rotator.Roll),
                            double(scale.X), double(scale.Y), double(scale.Z));

      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@identity() Transform
  // # Author(s): Markus Breyer
  static void mthd_identity(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkTransform>().SetIdentity();

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@unit_axis_x() Vector3
  // # Author(s): Zachary Burke
  static void mthd_unit_axis_x(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();
      *result_pp = SkVector3::new_instance(this_p->as<SkTransform>().GetUnitAxis(EAxis::X));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@unit_axis_y() Vector3
  // # Author(s): Zachary Burke
  static void mthd_unit_axis_y(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();
      *result_pp = SkVector3::new_instance(this_p->as<SkTransform>().GetUnitAxis(EAxis::Y));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@unit_axis_z() Vector3
  // # Author(s): Zachary Burke
  static void mthd_unit_axis_z(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();
      *result_pp = SkVector3::new_instance(this_p->as<SkTransform>().GetUnitAxis(EAxis::Z));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@relative_transform() Transform
  // # Author(s): Zachary Burke
  static void mthd_relative_transform(SkInvokedMethod* scope_p, SkInstance** result_pp)
  {
    if (result_pp)
      {
      const FTransform & t  = scope_p->this_as<SkTransform>();
      const FTransform & other = scope_p->get_arg<SkTransform>(SkArg_1);
      *result_pp = SkTransform::new_instance(t.GetRelativeTransform(other));
      }
  }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Transform@relative_transform_reverse() Transform
  // # Author(s): Zachary Burke
  static void mthd_relative_transform_reverse(SkInvokedMethod* scope_p, SkInstance** result_pp)
  {
    if (result_pp)
      {
      const FTransform & t = scope_p->this_as<SkTransform>();
      const FTransform & other = scope_p->get_arg<SkTransform>(SkArg_1);
      *result_pp = SkTransform::new_instance(t.GetRelativeTransformReverse(other));
      }
  }
  //---------------------------------------------------------------------------------------

  // Instance method array
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "String",                       mthd_String },
      { "identity",                     mthd_identity },
      { "unit_axis_x",                  mthd_unit_axis_x },
      { "unit_axis_y",                  mthd_unit_axis_y },
      { "unit_axis_z",                  mthd_unit_axis_z },
      { "relative_transform",           mthd_relative_transform },
      { "relative_transform_reverse",   mthd_relative_transform_reverse },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkTransform::register_bindings()
  {
  tBindingBase::register_bindings("Transform");

  ms_class_p->register_method_func_bulk(SkTransform_Impl::methods_i, A_COUNT_OF(SkTransform_Impl::methods_i), SkBindFlag_instance_no_rebind);

  ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_struct<SkTransform>);

  // Handle special case here - in UE4, the scale variable is called "Scale3D" while in Sk, we decided to call it just "scale"
  UStruct * ue_struct_p = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Transform"), false);
  FProperty * ue_scale_var_p = FindFProperty<FProperty>(ue_struct_p, TEXT("Scale3D"));  
  ms_class_p->resolve_raw_data("@scale", SkUEClassBindingHelper::compute_raw_data_info(ue_scale_var_p));
  SkUEClassBindingHelper::resolve_raw_data(ms_class_p, ue_struct_p); // Resolve the remaining raw data members as usual
  }

//---------------------------------------------------------------------------------------

SkClass * SkTransform::get_class()
  {
  return ms_class_p;
  }
