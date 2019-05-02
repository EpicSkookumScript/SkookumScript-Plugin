// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript 3D vector class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkVector3.hpp"
#include "SkRotation.hpp"
#include "SkTransform.hpp"
#include "SkRotationAngles.hpp"

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkReal.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkVector3_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!xyz(Real x, Real y, Real z) Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_xyz(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(
      scope_p->get_arg<SkReal>(SkArg_1),
      scope_p->get_arg<SkReal>(SkArg_2),
      scope_p->get_arg<SkReal>(SkArg_3));
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!xy(Real x, Real y) Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_xy(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(
      scope_p->get_arg<SkReal>(SkArg_1),
      scope_p->get_arg<SkReal>(SkArg_2),
      0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!scalar(Real s) Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_scalar(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(scope_p->get_arg<SkReal>(SkArg_1));
  }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_x() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_x(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(1.0f, 0.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_x_neg() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_x_neg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(-1.0f, 0.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_y() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_y(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(0.0f, 1.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_y_neg() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_y_neg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(0.0f, -1.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_z() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_z(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(0.0f, 0.0f, 1.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@!axis_z_neg() Vector3
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_z_neg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector3>(0.0f, 0.0f, -1.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector & vector = scope_p->this_as<SkVector3>();
      AString str(128u, "(%g, %g, %g)", double(vector.X), double(vector.Y), double(vector.Z));

      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@=(Vector3 vec) Boolean
  // # Author(s): Markus Breyer
  static void mthd_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector3>() == scope_p->get_arg<SkVector3>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@~=(Vector3 vec) Boolean
  // # Author(s): Markus Breyer
  static void mthd_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector3>() != scope_p->get_arg<SkVector3>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@+(Vector3 vec) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_add(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(scope_p->this_as<SkVector3>() + scope_p->get_arg<SkVector3>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@+=(Vector3 vec) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_add_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector3>() += scope_p->get_arg<SkVector3>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@-(Vector3 vec) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_subtract(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(scope_p->this_as<SkVector3>() - scope_p->get_arg<SkVector3>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@-=(Vector3 vec) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_subtract_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector3>() -= scope_p->get_arg<SkVector3>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@*(Real num) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_multiply(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(scope_p->this_as<SkVector3>() * scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@*=(Real num) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_multiply_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector3>() *= scope_p->get_arg<SkReal>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@/(Real num) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_divide(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(scope_p->this_as<SkVector3>() / scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@/=(Real num) Vector3
  // # Author(s): Markus Breyer
  static void mthd_op_divide_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector3>() /= scope_p->get_arg<SkReal>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:	  -Vector3
  // # Author(s): Zachary Burke
  static void mthd_op_negated(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(-scope_p->this_as<SkVector3>());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@rotate_by(Rotation rot) Vector3
  // # Author(s): Markus Breyer
  static void mthd_rotate_by(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector & vec = scope_p->this_as<SkVector3>();
      const FQuat & rot = scope_p->get_arg<SkRotation>(SkArg_1);

      *result_pp = SkVector3::new_instance(rot.RotateVector(vec));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@unrotate_by(Rotation rot) Vector3
  // # Author(s): Markus Breyer
  static void mthd_unrotate_by(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector & vec = scope_p->this_as<SkVector3>();
      const FQuat rot = scope_p->get_arg<SkRotation>(SkArg_1);

      *result_pp = SkVector3::new_instance(rot.Inverse().RotateVector(vec));
      }
    }

  //---------------------------------------------------------------------------------------
  // #Skookum Vector3@transform_by(Transform xform) Vector3
  static void mthd_transform_by(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector &    vec   = scope_p->this_as<SkVector3>();
      const FTransform & xform = scope_p->get_arg<SkTransform>(SkArg_1);

      *result_pp = SkVector3::new_instance(xform.TransformPosition(vec));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@untransform_by(Transform xform) Vector3
  // # Author(s): Markus Breyer
  static void mthd_untransform_by(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector & vec = scope_p->this_as<SkVector3>();
      const FTransform & xform = scope_p->get_arg<SkTransform>(SkArg_1);

      *result_pp = SkVector3::new_instance(xform.InverseTransformPosition(vec));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@set(Real x, Real y, Real z) Vector3
  // # Author(s): Markus Breyer
  static void mthd_set(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    FVector & vec = this_p->as<SkVector3>();

    vec.X = scope_p->get_arg<SkReal>(SkArg_1);
    vec.Y = scope_p->get_arg<SkReal>(SkArg_2);
    vec.Z = scope_p->get_arg<SkReal>(SkArg_3);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@zero() Vector3
  // # Author(s): Markus Breyer
  static void mthd_zero(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector3>().Set(0.0f, 0.0f, 0.0f);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@zero?() Boolean
  // # Author(s): Markus Breyer
  static void mthd_zeroQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector3>().IsZero());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@cross(Vector3 vec) Vector3
  // # Author(s): Markus Breyer
  static void mthd_cross(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector3::new_instance(FVector::CrossProduct(
        scope_p->this_as<SkVector3>(),
        scope_p->get_arg<SkVector3>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@distance(Vector3 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_distance(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector::Dist(
        scope_p->this_as<SkVector3>(),
        scope_p->get_arg<SkVector3>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@distance_squared(Vector3 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_distance_squared(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector::DistSquared(
        scope_p->this_as<SkVector3>(),
        scope_p->get_arg<SkVector3>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@dot(Vector3 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_dot(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector::DotProduct(
        scope_p->this_as<SkVector3>(),
        scope_p->get_arg<SkVector3>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@length() Real
  // # Author(s): Markus Breyer
  static void mthd_length(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkVector3>().Size());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@length_squared() Real
  // # Author(s): Markus Breyer
  static void mthd_length_squared(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkVector3>().SizeSquared());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@near?(Vector3 vector, Real epsilon_sq -> 0.0025) Boolean
  // # Author(s): Markus Breyer
  static void mthd_nearQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        FVector::DistSquared(scope_p->this_as<SkVector3>(), scope_p->get_arg<SkVector3>(SkArg_1))
          <= scope_p->get_arg<SkReal>(SkArg_2));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@RotationAngles() RotationAngles
  // # Author(s): Zachary Burke
  static void mthd_RotationAngles(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FRotator & rotation = scope_p->this_as<SkVector3>().Rotation();
      *result_pp = SkRotationAngles::new_instance(rotation);
      }
    }

  /*
  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@angle(Vector3 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_angle(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(qAngleBetween(
        scope_p->this_as<SkVector3>(),
        scope_p->get_arg<SkVector3>(SkArg_1)));
      }
    }
 
  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector3@normalize(Real length -> 1.0) Vector3
  // # Author(s): Markus Breyer
  static void mthd_normalize(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    FVector & vec = this_p->as<SkVector3>();

    vec = qNormalize(vec, scope_p->get_arg<SkReal>(SkArg_1));

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }
  */

  //---------------------------------------------------------------------------------------

  // Instance method array
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!xyz",             mthd_ctor_xyz },
      { "!xy",              mthd_ctor_xy },
      { "!scalar",          mthd_ctor_scalar },
      { "!axis_x",          mthd_ctor_axis_x },
      { "!axis_x_neg",      mthd_ctor_axis_x_neg },
      { "!axis_y",          mthd_ctor_axis_y },
      { "!axis_y_neg",      mthd_ctor_axis_y_neg },
      { "!axis_z",          mthd_ctor_axis_z },
      { "!axis_z_neg",      mthd_ctor_axis_z_neg },
      { "!forward",         mthd_ctor_axis_x },
      { "!backward",        mthd_ctor_axis_x_neg },
      { "!right",           mthd_ctor_axis_y },
      { "!left",            mthd_ctor_axis_y_neg },
      { "!up",              mthd_ctor_axis_z },
      { "!down",            mthd_ctor_axis_z_neg },

      { "String",           mthd_String },

      { "equal?",           mthd_op_equals },
      { "not_equal?",       mthd_op_not_equal },
      { "add",              mthd_op_add },
      { "add_assign",       mthd_op_add_assign },
      { "subtract",         mthd_op_subtract },
      { "subtract_assign",  mthd_op_subtract_assign },
      { "multiply",         mthd_op_multiply },
      { "multiply_assign",  mthd_op_multiply_assign },
      { "divide",           mthd_op_divide },
      { "divide_assign",    mthd_op_divide_assign },
      { "negated",          mthd_op_negated },

      { "rotate_by",        mthd_rotate_by },
      { "unrotate_by",      mthd_unrotate_by },
      { "transform_by",     mthd_transform_by },
      { "untransform_by",   mthd_untransform_by },

      { "set",              mthd_set },
      { "zero?",            mthd_zeroQ },
      { "zero",             mthd_zero },

      { "cross",            mthd_cross },
      { "distance",         mthd_distance },
      { "distance_squared", mthd_distance_squared },
      { "dot",              mthd_dot },
      { "length",           mthd_length },
      { "length_squared",   mthd_length_squared },
      { "near?",            mthd_nearQ },
      { "RotationAngles",   mthd_RotationAngles },
      //{ "angle",            mthd_angle },
      //{ "normalize",        mthd_normalize },
    };

  } // namespace

//---------------------------------------------------------------------------------------
void SkVector3::register_bindings()
  {
  tBindingBase::register_bindings("Vector3");

  ms_class_p->register_method_func_bulk(SkVector3_Impl::methods_i, A_COUNT_OF(SkVector3_Impl::methods_i), SkBindFlag_instance_no_rebind);

  ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_struct<SkVector3>);
  SkUEClassBindingHelper::resolve_raw_data_struct(ms_class_p, TEXT("Vector"));
  }

//---------------------------------------------------------------------------------------

SkClass * SkVector3::get_class()
  {
  return ms_class_p;
  }
