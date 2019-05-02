// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript 2D vector class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkVector2.hpp"

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkReal.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkVector2_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@!xy(Real x, Real y) Vector2
  // # Author(s): Markus Breyer
  static void mthd_ctor_xy(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector2>(
      scope_p->get_arg<SkReal>(SkArg_1),
      scope_p->get_arg<SkReal>(SkArg_2));
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@!axis_x() Vector2
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_x(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector2>(1.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@!axis_x_neg() Vector2
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_x_neg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector2>(-1.0f, 0.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@!axis_y() Vector2
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_y(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector2>(0.0f, 1.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@!axis_y_neg() Vector2
  // # Author(s): Markus Breyer
  static void mthd_ctor_axis_y_neg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->get_this()->construct<SkVector2>(0.0f, -1.0f);
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FVector2D & vector = scope_p->this_as<SkVector2>();
      AString str(128u, "(%g, %g)", double(vector.X), double(vector.Y));

      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@=(Vector2 vec) Boolean
  // # Author(s): Markus Breyer
  static void mthd_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector2>() == scope_p->get_arg<SkVector2>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@~=(Vector2 vec) Boolean
  // # Author(s): Markus Breyer
  static void mthd_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector2>() != scope_p->get_arg<SkVector2>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@+(Vector2 vec) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_add(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector2::new_instance(scope_p->this_as<SkVector2>() + scope_p->get_arg<SkVector2>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@+=(Vector2 vec) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_add_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector2>() += scope_p->get_arg<SkVector2>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@-(Vector2 vec) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_subtract(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector2::new_instance(scope_p->this_as<SkVector2>() - scope_p->get_arg<SkVector2>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@-=(Vector2 vec) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_subtract_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector2>() -= scope_p->get_arg<SkVector2>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@*(Real num) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_multiply(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector2::new_instance(scope_p->this_as<SkVector2>() * scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@*=(Real num) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_multiply_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector2>() *= scope_p->get_arg<SkReal>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@/(Real num) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_divide(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector2::new_instance(scope_p->this_as<SkVector2>() / scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@/=(Real num) Vector2
  // # Author(s): Markus Breyer
  static void mthd_op_divide_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkVector2>() /= scope_p->get_arg<SkReal>(SkArg_1);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:	  -Vector2
  // # Author(s): Zachary Burke
  static void mthd_op_negated(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkVector2::new_instance(-scope_p->this_as<SkVector2>());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@set(Real x, Real y) Vector2
  // # Author(s): Markus Breyer
  static void mthd_set(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    FVector2D & vec = this_p->as<SkVector2>();

    vec.X = scope_p->get_arg<SkReal>(SkArg_1);
    vec.Y = scope_p->get_arg<SkReal>(SkArg_2);

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@zero() Vector2
  // # Author(s): Markus Breyer
  static void mthd_zero(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    FVector2D & vec = this_p->as<SkVector2>();

    vec.X = 0.0f;
    vec.Y = 0.0f;

    // Return this if result desired
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@zero?() Boolean
  // # Author(s): Markus Breyer
  static void mthd_zeroQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkVector2>().IsZero());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@cross(Vector2 vec) Vector2
  // # Author(s): Markus Breyer
  static void mthd_cross(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector2D::CrossProduct(
        scope_p->this_as<SkVector2>(),
        scope_p->get_arg<SkVector2>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@distance(Vector2 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_distance(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector2D::Distance(
        scope_p->this_as<SkVector2>(),
        scope_p->get_arg<SkVector2>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@distance_squared(Vector2 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_distance_squared(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector2D::DistSquared(
        scope_p->this_as<SkVector2>(),
        scope_p->get_arg<SkVector2>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@dot(Vector2 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_dot(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(FVector2D::DotProduct(
        scope_p->this_as<SkVector2>(),
        scope_p->get_arg<SkVector2>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@length() Real
  // # Author(s): Markus Breyer
  static void mthd_length(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkVector2>().Size());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@length_squared() Real
  // # Author(s): Markus Breyer
  static void mthd_length_squared(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkVector2>().SizeSquared());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@near?(Vector2 vector, Real epsilon_sq -> 0.0025) Boolean
  // # Author(s): Markus Breyer
  static void mthd_nearQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        FVector2D::DistSquared(scope_p->this_as<SkVector2>(), scope_p->get_arg<SkVector2>(SkArg_1))
          <= scope_p->get_arg<SkReal>(SkArg_2));
      }
    }

  /*
  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@angle(Vector2 vec) Real
  // # Author(s): Markus Breyer
  static void mthd_angle(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(qAngleBetween(
        scope_p->this_as<SkVector2>(),
        scope_p->get_arg<SkVector2>(SkArg_1)));
      }
    }
 
  //---------------------------------------------------------------------------------------
  // # Skookum:   Vector2@normalize(Real length -> 1.0) Vector2
  // # Author(s): Markus Breyer
  static void mthd_normalize(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    FVector2D &  vec    = this_p->as<SkVector2>();

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
      { "!xy",              mthd_ctor_xy },
      { "!axis_x",          mthd_ctor_axis_x },
      { "!axis_x_neg",      mthd_ctor_axis_x_neg },
      { "!axis_y",          mthd_ctor_axis_y },
      { "!axis_y_neg",      mthd_ctor_axis_y_neg },

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

      //{ "angle",            mthd_angle },
      //{ "normalize",        mthd_normalize },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkVector2::register_bindings()
  {
  tBindingBase::register_bindings("Vector2");

  ms_class_p->register_method_func_bulk(SkVector2_Impl::methods_i, A_COUNT_OF(SkVector2_Impl::methods_i), SkBindFlag_instance_no_rebind);

  ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_struct<SkVector2>);
  SkUEClassBindingHelper::resolve_raw_data_struct(ms_class_p, TEXT("Vector2D"));
  }

//---------------------------------------------------------------------------------------

SkClass * SkVector2::get_class()
  {
  return ms_class_p;
  }
