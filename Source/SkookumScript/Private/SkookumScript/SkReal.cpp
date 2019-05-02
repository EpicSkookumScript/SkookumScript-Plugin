// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Real (floating point) class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <AgogCore/AMath.hpp>
#include <limits>
#include <cmath>


//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkReal_Impl
  {

  //---------------------------------------------------------------------------------------
  // Real@!golden_ratio()
  static void mthd_ctor_golden_ratio(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = A_golden_ratio;
    }

  //---------------------------------------------------------------------------------------
  // Real@!infinity()
  static void mthd_ctor_infinity(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = std::numeric_limits<float>::infinity();
    }

  //---------------------------------------------------------------------------------------
  // Real@!max()
  static void mthd_ctor_max(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = FLT_MAX;
    }

  //---------------------------------------------------------------------------------------
  // Real@!min()
  static void mthd_ctor_min(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = FLT_MIN;
    }

  //---------------------------------------------------------------------------------------
  // Real@!nan()
  static void mthd_ctor_nan(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = std::numeric_limits<float>::quiet_NaN();
    }

  //---------------------------------------------------------------------------------------
  // Real@!pi()
  static void mthd_ctor_pi(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkReal>() = A_pi;
    }

  //---------------------------------------------------------------------------------------
  // Real@Integer() Integer
  static void mthd_Integer(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      // $Revisit - Consider changing to std::lround()
      *result_pp = SkInteger::new_instance(tSkInteger(a_round(scope_p->this_as<SkReal>())));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@String() String
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkString::new_instance(AString::ctor_float(scope_p->this_as<SkReal>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@max(Real num) Real
  static void mthd_max(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * this_obj_p = scope_p->get_this();
      SkInstance * num_obj_p  = scope_p->get_arg(SkArg_1);
      SkInstance * result_p   = (this_obj_p->as<SkReal>() >= num_obj_p->as<SkReal>())
        ? this_obj_p
        : num_obj_p;

      result_p->reference();
      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@min(Real num) Real
  static void mthd_min(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * this_obj_p = scope_p->get_this();
      SkInstance * num_obj_p  = scope_p->get_arg(SkArg_1);
      SkInstance * result_p   = (this_obj_p->as<SkReal>() <= num_obj_p->as<SkReal>())
        ? this_obj_p
        : num_obj_p;

      result_p->reference();
      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@negate() Real
  static void mthd_negate(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    tSkReal & value = this_p->as<SkReal>();

    value = -value;

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@negated() Real
  static void mthd_op_negated(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(-scope_p->this_as<SkReal>());
      }
    }

  //---------------------------------------------------------------------------------------
  // ++
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params () Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_increment(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      this_p->as<SkReal>()++;

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      scope_p->this_as<SkReal>()++;
      }
    }

  //---------------------------------------------------------------------------------------
  // ++
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params () Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_decrement(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      (this_p->as<SkReal>())--;

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      (scope_p->this_as<SkReal>())--;
      }
    }

  //---------------------------------------------------------------------------------------
  // +
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_add(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->this_as<SkReal>() + scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // +=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_add_assign(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      this_p->as<SkReal>() += scope_p->get_arg<SkReal>(SkArg_1);

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      scope_p->this_as<SkReal>() += scope_p->get_arg<SkReal>(SkArg_1);
      }
    }

  //---------------------------------------------------------------------------------------
  // -
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_subtract(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->this_as<SkReal>() - scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // -=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_subtract_assign(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      this_p->as<SkReal>() -= scope_p->get_arg<SkReal>(SkArg_1);

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      scope_p->this_as<SkReal>() -= scope_p->get_arg<SkReal>(SkArg_1);
      }
    }

  //---------------------------------------------------------------------------------------
  // *
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_multiply(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->this_as<SkReal>() * scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // *=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_multiply_assign(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      this_p->as<SkReal>() *= scope_p->get_arg<SkReal>(SkArg_1);

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      scope_p->this_as<SkReal>() *= scope_p->get_arg<SkReal>(SkArg_1);
      }
    }

  //---------------------------------------------------------------------------------------
  // /
  // Real@divide(Real num) Real
  static void mthd_op_divide(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->this_as<SkReal>() / scope_p->get_arg<SkReal>(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // /=
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params (Real num) Real
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_divide_assign(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      this_p->as<SkReal>() /= scope_p->get_arg<SkReal>(SkArg_1);

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      scope_p->this_as<SkReal>() /= scope_p->get_arg<SkReal>(SkArg_1);
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params = equal?(Real num) Boolean
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
        (scope_p->this_as<SkReal>() == scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params ~= not_equal?(Real num) Boolean
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
        (scope_p->this_as<SkReal>() != scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }
    
  //---------------------------------------------------------------------------------------
  // Sk Params > greater?(Real num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_greater(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkReal>() > scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }
    
  //---------------------------------------------------------------------------------------
  // Sk Params >= greater_or_equal?(Real num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_greater_or_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkReal>() >= scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }
    
  //---------------------------------------------------------------------------------------
  // Sk Params < less?(Real num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_less(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkReal>() < scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }
    
  //---------------------------------------------------------------------------------------
  // Sk Params <= less_or_equal?(Real num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_less_or_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkReal>() <= scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Real@between?(Real min Real max) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_betweenQ(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      tSkReal this_num = scope_p->this_as<SkReal>();

      *result_pp = SkBoolean::new_instance(
        (this_num >= scope_p->get_arg<SkReal>(SkArg_1))
        && (this_num <= scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@clamp(Real min_value Real max_value) Real
  static void mthd_clamp(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    SkInstance * this_p    = scope_p->get_this();
    tSkReal &    this_real = this_p->as<SkReal>();

    this_real = a_clamp(this_real, scope_p->get_arg<SkReal>(SkArg_1), scope_p->get_arg<SkReal>(SkArg_2));

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@finite?() Boolean
  static void mthd_finiteQ(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        std::isfinite( scope_p->this_as<SkReal>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@infinite?() Boolean
  static void mthd_infiniteQ(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        std::isinf( scope_p->this_as<SkReal>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@nan?() Boolean
  static void mthd_nanQ(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        std::isnan( scope_p->this_as<SkReal>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Real@pow(Real num) Real
  static void mthd_pow(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(std::pow(
        scope_p->this_as<SkReal>(),
        scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

//---------------------------------------------------------------------------------------
// Real@swap(Real num) Real
static void mthd_swap(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    a_swap(this_p->as<SkReal>(), scope_p->get_arg<SkReal>(SkArg_1));
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    a_swap(scope_p->this_as<SkReal>(), scope_p->get_arg<SkReal>(SkArg_1));
    }
  }

  //---------------------------------------------------------------------------------------
  // <Real>@abs(Real num) Real
  static void mthdc_abs(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_abs(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@acos(Real rad) Real
  static void mthdc_acos(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_acos(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@asin(Real rad) Real
  static void mthdc_asin(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_asin(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@atan2(Real y Real x) Real
  static void mthdc_atan2(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_atan2(
        scope_p->get_arg<SkReal>(SkArg_1),
        scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@clamp(Real value Real min_value Real max_value) Real
  static void mthdc_clamp(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_clamp(
        scope_p->get_arg<SkReal>(SkArg_1),
        scope_p->get_arg<SkReal>(SkArg_2),
        scope_p->get_arg<SkReal>(SkArg_3)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@cos(Real rad) Real
  static void mthdc_cos(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_cos(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@cube(Real num) Real
  static void mthdc_cube(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_cube(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@deg_to_rad(Real degrees) Real
  static void mthdc_deg_to_rad(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->get_arg<SkReal>(SkArg_1) * A_deg_rad);
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@hypot(Real x Real y) Real
  static void mthdc_hypot(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_hypot(
        scope_p->get_arg<SkReal>(SkArg_1),
        scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@lerp(Real begin Real end Real alpha) Real
  static void mthdc_lerp(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_lerp(
        scope_p->get_arg<SkReal>(SkArg_1),
        scope_p->get_arg<SkReal>(SkArg_2),
        scope_p->get_arg<SkReal>(SkArg_3)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@max(Real num1 Real num2) Real
  static void mthdc_max(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        a_max(scope_p->get_arg<SkReal>(SkArg_1), scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@min(Real num1 Real num2) Real
  static void mthdc_min(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        a_min(scope_p->get_arg<SkReal>(SkArg_1), scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@rad_to_deg(Real radians) Real
  static void mthdc_rad_to_deg(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(
        scope_p->get_arg<SkReal>(SkArg_1) * A_rad_deg);
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@reciprocal(Real num) Real
  static void mthdc_reciprocal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_reciprocal(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@sin(Real rads) Real
  static void mthdc_sin(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_sin(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@sqrt(Real radicand) Real
  static void mthdc_sqrt(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_sqrt(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@square(Real num) Real
  static void mthdc_square(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_sqr(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <Real>@tan(Real rads) Real
  static void mthdc_tan(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(a_tan(scope_p->get_arg<SkReal>(SkArg_1)));
      }
    }


  //---------------------------------------------------------------------------------------

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      // Constructors / constants
      { "!golden_ratio",      mthd_ctor_golden_ratio },
      { "!infinity",          mthd_ctor_infinity },
      { "!max",               mthd_ctor_max },
      { "!min",               mthd_ctor_min },
      { "!nan",               mthd_ctor_nan },
      { "!pi",                mthd_ctor_pi },

      // Convert
      { "Integer",            mthd_Integer },
      { "String",             mthd_String },

      // Operators
      { "add",                mthd_op_add },
      { "add_assign",         mthd_op_add_assign },
      { "decrement",          mthd_op_decrement },
      { "divide",             mthd_op_divide },
      { "divide_assign",      mthd_op_divide_assign },
      { "equal?",             mthd_op_equals },
      { "greater?",           mthd_op_greater },
      { "greater_or_equal?",  mthd_op_greater_or_equal },
      { "increment",          mthd_op_increment },
      { "less?",              mthd_op_less },
      { "less_or_equal?",     mthd_op_less_or_equal },
      { "multiply",           mthd_op_multiply },
      { "multiply_assign",    mthd_op_multiply_assign },
      { "negated",            mthd_op_negated },
      { "not_equal?",         mthd_op_not_equal },
      { "subtract",           mthd_op_subtract },
      { "subtract_assign",    mthd_op_subtract_assign },

      // Non-modifying
      { "between?",           mthd_betweenQ },
      { "finite?",            mthd_finiteQ },
      { "infinite?",          mthd_infiniteQ },
      { "max",                mthd_max },
      { "min",                mthd_min },
      { "nan?",               mthd_nanQ },
      { "pow",                mthd_pow },
      // normal?

      // Modifying
      { "clamp",              mthd_clamp },   // *deprecate - use <Real>@clamp()
      { "negate",             mthd_negate },
      { "swap",               mthd_swap },
      // round
      // floor
      // ceiling
      // truncate
    };

  // Class methods
  static const SkClass::MethodInitializerFunc methods_c[] =
    {
      { "abs",                mthdc_abs },
      { "acos",               mthdc_acos },
      { "asin",               mthdc_asin },
      { "atan2",              mthdc_atan2 },
      { "clamp",              mthdc_clamp },
      { "cos",                mthdc_cos },
      { "cube",               mthdc_cube },
      { "deg_to_rad",         mthdc_deg_to_rad },
      { "hypot",              mthdc_hypot },
      { "lerp",               mthdc_lerp },
      { "max",                mthdc_max },
      { "min",                mthdc_min },
      { "rad_to_deg",         mthdc_rad_to_deg },
      { "reciprocal",         mthdc_reciprocal },
      { "sin",                mthdc_sin },
      { "sqrt",               mthdc_sqrt },
      { "square",             mthdc_square },
      { "tan",                mthdc_tan },
      // log(a base)
      // loge(a)   // natural log
      // exp(a)    // e^a
      // fraction_part(num)
      // ordered?
      // sign

    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkReal::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Real);

  ms_class_p->register_method_func_bulk(SkReal_Impl::methods_i, A_COUNT_OF(SkReal_Impl::methods_i), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkReal_Impl::methods_c, A_COUNT_OF(SkReal_Impl::methods_c), SkBindFlag_class_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkReal::get_class()
  {
  return ms_class_p;
  }

