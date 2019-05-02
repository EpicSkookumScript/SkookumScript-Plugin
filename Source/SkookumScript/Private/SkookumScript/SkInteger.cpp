// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Integer class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <AgogCore/AMath.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkInteger_Impl
{

  //---------------------------------------------------------------------------------------
  // Sk Params Integer@!max() Integer
  static void mthd_ctor_max(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkInteger>() = INT_MAX;
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Integer@!min() Integer
  static void mthd_ctor_min(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkInteger>() = INT_MIN;
    }

//---------------------------------------------------------------------------------------
// Sk Params Integer@abs() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_abs(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(a_abs(scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@Integer() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_Integer(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Boolean() Boolean
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_Boolean(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(scope_p->this_as<SkInteger>() != 0);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params as_real() Real
// Returns real (floating point) representation of itself
// Returns:    itself as a real
// C++ Arg     scope_p - Calling scope and working data for the invoked code - this is
//             the path for accessing run-time information, passed arguments, data
//             members, the call stack, etc. - see SkInvokedMethod for a description of
//             its methods and data members.
// C++ Arg     result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Author(s):   Conan Reis
static void mthd_Real(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkReal::new_instance(tSkReal(scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@String() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(AString::ctor_int(scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_and(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_and(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() & scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_not() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_not(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(~scope_p->this_as<SkInteger>());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_or(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_or(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() | scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_shift_down(Integer bits) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_shift_down(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() >> scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_shift_up(Integer bits) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_shift_up(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() << scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_xor(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_xor(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() ^ scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@between?(Integer min Integer max) Boolean
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
    tSkInteger this_num = scope_p->this_as<SkInteger>();

    *result_pp = SkBoolean::new_instance(
      (this_num >= scope_p->get_arg<SkInteger>(SkArg_1))
      && (this_num <= scope_p->get_arg<SkInteger>(SkArg_2)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@bit_set?(Integer bit) Boolean
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_bit_setQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(a_is_bit_on(
      scope_p->get_arg<SkInteger>(SkArg_1),
      scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@clamp(Integer min Integer max) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_clamp(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInstance * this_obj_p = scope_p->get_this();
    SkInstance * min_obj_p  = scope_p->get_arg(SkArg_1);
    SkInstance * max_obj_p  = scope_p->get_arg(SkArg_2);
    tSkInteger   this_num   = this_obj_p->as<SkInteger>();
    SkInstance * result_p   = (this_num < min_obj_p->as<SkInteger>())
      ? min_obj_p
      : ((this_num > max_obj_p->as<SkInteger>())
        ? max_obj_p
        : this_obj_p);

    result_p->reference();
    *result_pp = result_p;
    }
  }

//---------------------------------------------------------------------------------------
// # Sk Params: Integer@do((Integer idx) code)
// # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// # Author(s): Conan Reis
static void mthd_do(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger this_num = scope_p->this_as<SkInteger>();

  // Quit early if zero or less - without an error or warning if negative
  if (this_num <= 0)
    {
    return;
    }

  SkInstance * idx_obj_p = SkInteger::new_instance(0);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  SkClosure *  closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);

  // Ensure that the arg has extra reference counts for the call
  idx_obj_p->reference(this_num);

  do
    {
    closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
    (*idx_p)++;
    }
  while (*idx_p < this_num);

  // Clean up the arg
  idx_obj_p->dereference();
  }

//---------------------------------------------------------------------------------------
// # Sk Params: Integer@do_by(Integer step, (Integer idx) code)
// # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// # Author(s): Conan Reis
static void mthd_do_by(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger this_num = scope_p->this_as<SkInteger>();

  // Quit early if zero or less - without an error or warning if negative
  if (this_num <= 0)
    {
    return;
    }

  SkInstance * idx_obj_p = SkInteger::new_instance(0);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  tSkInteger   step      = scope_p->get_arg<SkInteger>(SkArg_1);
  SkClosure *  closure_p = scope_p->get_arg_data<SkClosure>(SkArg_2);

  do
    {
    // Ensure that the arg has an extra reference count for the call
    idx_obj_p->reference();
    closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
    (*idx_p) += step;
    }
  while (*idx_p < this_num);

  // Clean up the arg
  idx_obj_p->dereference();
  }

//---------------------------------------------------------------------------------------
// # Sk Params: Integer@do_reverse((Integer idx) code)
// # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// # Author(s): Conan Reis
static void mthd_do_reverse(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger this_num = scope_p->this_as<SkInteger>();

  // Quit early if zero or less - without an error or warning if negative
  if (this_num <= 0)
    {
    return;
    }

  SkInstance * idx_obj_p = SkInteger::new_instance(this_num);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  SkClosure *  closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);

  // Ensure that the arg has an extra reference counts for the call
  idx_obj_p->reference(this_num);

  do
    {
    (*idx_p)--;
    closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
    }
  while (*idx_p);

  // Clean up the arg
  idx_obj_p->dereference();
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@fibonacci() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):  Markus Breyer
static void mthd_fibonacci(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    tSkInteger idx    = scope_p->this_as<SkInteger>();
    tSkInteger result = 0; // Fibonacci number at index 0 is 0

    if (idx > 0)
      {
      // Compute Fibonacci series for positive index
      tSkInteger prev = 0;
      tSkInteger sum  = 1;

      while (--idx > 0)
        {
        tSkInteger t = prev;
        prev = sum;
        sum += t;
        }
      result = sum;
      }
    else if (idx < 0)
      {
      // Compute Fibonacci series for negative index
      tSkInteger prev = 0;
      tSkInteger sum = 1;

      while (++idx < 0)
        {
        tSkInteger t = prev;
        prev = sum;
        sum = t - sum;
        }
      result = sum;
      }

    *result_pp = SkInteger::new_instance(result);
    }
  }

//---------------------------------------------------------------------------------------
// Integer@max(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_max(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInstance * this_obj_p = scope_p->get_this();
    SkInstance * num_obj_p  = scope_p->get_arg(SkArg_1);
    SkInstance * result_p   = (this_obj_p->as<SkInteger>() >= num_obj_p->as<SkInteger>())
      ? this_obj_p
      : num_obj_p;

    result_p->reference();
    *result_pp = result_p;
    }
  }

//---------------------------------------------------------------------------------------
// Integer@min(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_min(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInstance * this_obj_p = scope_p->get_this();
    SkInstance * num_obj_p  = scope_p->get_arg(SkArg_1);
    SkInstance * result_p   = (this_obj_p->as<SkInteger>() <= num_obj_p->as<SkInteger>())
      ? this_obj_p
      : num_obj_p;

    result_p->reference();
    *result_pp = result_p;
    }
  }

//---------------------------------------------------------------------------------------
// Integer@mod(Integer num) Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_mod(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() % scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@negate() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_negate(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  SkInstance * this_p = scope_p->get_this();
  tSkInteger & value  = this_p->as<SkInteger>();

  value = -value;

  if (result_pp)
    {
    this_p->reference();
    *result_pp = this_p;
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params -num num.negated() Integer@negated() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_op_negated(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(-scope_p->this_as<SkInteger>());
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
// Sk Params () Integer
// Modifiers:   static
// Author(s):   Conan Reis
static void mthd_op_increment(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    (this_p->as<SkInteger>())++;

    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    (scope_p->this_as<SkInteger>())++;
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
// Sk Params () Integer
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

    (this_p->as<SkInteger>())--;

    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    (scope_p->this_as<SkInteger>())--;
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
// Sk Params (Integer num) Integer
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
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() + scope_p->get_arg<SkInteger>(SkArg_1));
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
// Sk Params (Integer num) Integer
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

    this_p->as<SkInteger>() += scope_p->get_arg<SkInteger>(SkArg_1);
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkInteger>() += scope_p->get_arg<SkInteger>(SkArg_1);
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
// Sk Params (Integer num) Integer
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
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() - scope_p->get_arg<SkInteger>(SkArg_1));
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
// Sk Params (Integer num) Integer
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

    this_p->as<SkInteger>() -= scope_p->get_arg<SkInteger>(SkArg_1);
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkInteger>() -= scope_p->get_arg<SkInteger>(SkArg_1);
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
// Sk Params (Integer num) Integer
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
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() * scope_p->get_arg<SkInteger>(SkArg_1));
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
// Sk Params (Integer num) Integer
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

    this_p->as<SkInteger>() *= scope_p->get_arg<SkInteger>(SkArg_1);
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkInteger>() *= scope_p->get_arg<SkInteger>(SkArg_1);
    }
  }

//---------------------------------------------------------------------------------------
// /
// Arg         scope_p - Calling scope and working data for the invoked code - this is
//             the path for accessing run-time information, passed arguments, data
//             members, the call stack, etc. - see SkInvokedMethod for a description of
//             its methods and data members.
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Sk Params (Integer num) Integer
// Modifiers:   static
// Author(s):   Conan Reis
static void mthd_op_divide(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkInteger>() / scope_p->get_arg<SkInteger>(SkArg_1));
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
// Sk Params (Integer num) Integer
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

    this_p->as<SkInteger>() /= scope_p->get_arg<SkInteger>(SkArg_1);
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkInteger>() /= scope_p->get_arg<SkInteger>(SkArg_1);
    }
  }

//---------------------------------------------------------------------------------------
// = equal?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() == scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// ~= not_equal?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() != scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// > greater?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() > scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// >= greater_or_equal?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() >= scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// < less?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() < scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// <= less_or_equal?()
// Sk Params (Integer num) Boolean
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
      (scope_p->this_as<SkInteger>() <= scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@pow2?() Boolean
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_pow2Q(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(
      a_is_pow_2(scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@radix(Integer base) String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):  Conan Reis
static void mthd_radix(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(
      AString::ctor_int(scope_p->this_as<SkInteger>(),
        scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Integer@sign() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_sign(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInstance::new_instance<SkInteger>(a_sign(scope_p->this_as<SkInteger>()));
    }
  }

//---------------------------------------------------------------------------------------
// Integer@swap(Integer num) Integer
static void mthd_swap(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    a_swap(this_p->as<SkInteger>(), scope_p->get_arg<SkInteger>(SkArg_1));
    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    a_swap(scope_p->this_as<SkInteger>(), scope_p->get_arg<SkInteger>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// # Sk Params: Integer@to(Integer last, (Integer idx) code)
// # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// # Author(s): Conan Reis
static void mthd_to(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger   first     = scope_p->this_as<SkInteger>();
  tSkInteger   last      = scope_p->get_arg<SkInteger>(SkArg_1);
  SkInstance * idx_obj_p = SkInteger::new_instance(first);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  SkClosure *  closure_p = scope_p->get_arg_data<SkClosure>(SkArg_2);

  if (first <= last)
    {
    // Increasing index
    do
      {
      // Ensure that the arg has an extra reference count for the call
      idx_obj_p->reference();
      closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
      (*idx_p)++;
      }
    while (*idx_p <= last);
    }
  else
    {
    // Decreasing index
    do
      {
      // Ensure that the arg has an extra reference count for the call
      idx_obj_p->reference();
      closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
      (*idx_p)--;
      }
    while (*idx_p >= last);
    }

  // Clean up the arg
  idx_obj_p->dereference();
  }

//---------------------------------------------------------------------------------------
// # Sk Params: Integer@to_pre(Integer last, (Integer idx) code)
// # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// # Author(s): Conan Reis
static void mthd_to_pre(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger first = scope_p->this_as<SkInteger>();
  tSkInteger last  = scope_p->get_arg<SkInteger>(SkArg_1);

  if (first == last)
    {
    return;
    }

  SkInstance * idx_obj_p = SkInteger::new_instance(first);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  SkClosure *  closure_p = scope_p->get_arg_data<SkClosure>(SkArg_2);

  if (first < last)
    {
    // Increasing index
    do
      {
      // Ensure that the arg has an extra reference count for the call
      idx_obj_p->reference();
      closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
      (*idx_p)++;
      }
    while (*idx_p < last);
    }
  else
    {
    // Decreasing index
    do
      {
      // Ensure that the arg has an extra reference count for the call
      idx_obj_p->reference();
      closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
      (*idx_p)--;
      }
    while (*idx_p > last);
    }

  // Clean up the arg
  idx_obj_p->dereference();
  }

//---------------------------------------------------------------------------------------
// <Integer>@abs(Integer num) Integer
static void mthdc_abs(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(a_abs(scope_p->get_arg<SkInteger>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@clamp(Integer value Integer min_value Integer max_value) Integer
static void mthdc_clamp(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(a_clamp(
      scope_p->get_arg<SkInteger>(SkArg_1),
      scope_p->get_arg<SkInteger>(SkArg_2),
      scope_p->get_arg<SkInteger>(SkArg_3)));
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@cube(Integer num) Integer
static void mthdc_cube(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    tSkInteger num = scope_p->get_arg<SkInteger>(SkArg_1);
    *result_pp = SkInteger::new_instance(num * num * num);
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@lerp(Integer begin Integer end Real alpha) Integer
static void mthdc_lerp(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(a_lerp_round(
      scope_p->get_arg<SkInteger>(SkArg_1),
      scope_p->get_arg<SkInteger>(SkArg_2),
      scope_p->get_arg<SkReal>(SkArg_3)));
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@max(Integer num1 Integer num2) Integer
static void mthdc_max(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(
      a_max(scope_p->get_arg<SkInteger>(SkArg_1), scope_p->get_arg<SkInteger>(SkArg_2)));
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@min(Integer num1 Integer num2) Integer
static void mthdc_min(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(
      a_min(scope_p->get_arg<SkInteger>(SkArg_1), scope_p->get_arg<SkInteger>(SkArg_2)));
    }
  }

//---------------------------------------------------------------------------------------
// <Integer>@square(Integer num) Integer
static void mthdc_square(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    tSkInteger num = scope_p->get_arg<SkInteger>(SkArg_1);
    *result_pp = SkInteger::new_instance(num * num);
    }
  }

/*
//---------------------------------------------------------------------------------------
// # Sk Params: Integer@_do(+(Integer idx) code)
// # C++ Args:  See tSkCoroutineFunc or tSkCoroutineMthd in SkookumScript/SkCoroutine.hpp
// # Author(s): Conan Reis
static void coro_do(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  tSkInteger this_num  = scope_p->this_as<SkInteger>();

  // Quit early if zero or less - without an error or warning if negative
  if (this_num <= 0)
    {
    return;
    }

  SkInstance *    idx_obj_p = SkInteger::new_instance(0);
  tSkInteger * idx_p     = &idx_obj_p->as<SkInteger>();
  SkClosure *     closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);

  // $TODO - CReis Rewrite for coroutine closures
  do
    {
    // Ensure that the arg has an extra reference count for the call
    idx_obj_p->reference();
    closure_p->closure_method_call(&idx_obj_p, 1u, nullptr, scope_p);
    (*idx_p)++;
    }
  while (*idx_p < this_num);

  // Clean up the arg
  idx_obj_p->dereference();
  }
*/

//---------------------------------------------------------------------------------------
// Instance methods
static const SkClass::MethodInitializerFunc methods_i[] =
  {
    // Constructors
    { "!max",               mthd_ctor_max },
    { "!min",               mthd_ctor_min },

    // Conversion methods
    { "Integer",            mthd_Integer },
    { "Boolean",            mthd_Boolean },
    { "Real",               mthd_Real },
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

    // Non-modifying methods
    { "abs",                mthd_abs },
    { "between?",           mthd_betweenQ },
    { "bit_and",            mthd_bit_and },
    { "bit_not",            mthd_bit_not },
    { "bit_or",             mthd_bit_or },
    { "bit_set?",           mthd_bit_setQ },
    { "bit_shift_down",     mthd_bit_shift_down },
    { "bit_shift_up",       mthd_bit_shift_up },
    { "bit_xor",            mthd_bit_xor },
    { "clamp",              mthd_clamp },
    { "fibonacci",          mthd_fibonacci },
    { "max",                mthd_max },
    { "min",                mthd_min },
    { "mod",                mthd_mod },
    { "pow2?",              mthd_pow2Q },
    { "radix",              mthd_radix },
    { "sign",               mthd_sign },
    // even?
    // odd?

    // Modifying methods
    { "negate",             mthd_negate },
    { "swap",               mthd_swap },

    // Iterating methods
    { "do",                 mthd_do },
    { "do_by",              mthd_do_by },
    { "do_reverse",         mthd_do_reverse },
    { "to",                 mthd_to },
    { "to_pre",             mthd_to_pre },

    // Class methods
  };

//---------------------------------------------------------------------------------------
// Class methods
static const SkClass::MethodInitializerFunc methods_c[] =
  {
    { "abs",                mthdc_abs },
    { "clamp",              mthdc_clamp },
    { "cube",               mthdc_cube },
    { "lerp",               mthdc_lerp },
    { "max",                mthdc_max },
    { "min",                mthdc_min },
    { "square",             mthdc_square },
    // div_modulo
    // log2floor
    // log2ceil
    // ordered?

  };

//---------------------------------------------------------------------------------------
// Coroutines
//static const SkClass::CoroutineInitializerFunc coroutines_i[] =
//  {
//    { "_do",              coro_do },
//  };

} // namespace

//---------------------------------------------------------------------------------------

void SkInteger::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Integer);

  ms_class_p->register_method_func_bulk(SkInteger_Impl::methods_i, A_COUNT_OF(SkInteger_Impl::methods_i), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkInteger_Impl::methods_c, A_COUNT_OF(SkInteger_Impl::methods_c), SkBindFlag_class_no_rebind);
  //ms_class_p->register_coroutine_func_bulk(SkInteger_Impl::coroutines_i, A_COUNT_OF(SkInteger_Impl::coroutines_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkInteger::get_class()
  {
  return ms_class_p;
  }
