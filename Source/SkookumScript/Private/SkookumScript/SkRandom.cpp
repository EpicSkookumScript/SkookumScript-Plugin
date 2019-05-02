// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Atomic Random Number Generator class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkRandom.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkRandom_Impl
  {

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@!seed(Integer seed) Random
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_ctor_seed(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Results are ignored for constructors
    scope_p->this_as<SkRandom>().set_seed(uint32_t(scope_p->get_arg<SkInteger>(SkArg_1)));
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@get_seed() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_seed(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(tSkInteger(scope_p->this_as<SkRandom>().get_seed()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@set_seed(Integer seed)
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_seed_set(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    scope_p->this_as<SkRandom>().set_seed(uint32_t(scope_p->get_arg<SkInteger>(SkArg_1)));
    //scope_p->get_this()->m_user_data = scope_p->get_arg(SkArg_1)->m_user_data;
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@coin_toss() Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_coin_toss(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(scope_p->this_as<SkRandom>().coin_toss());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@uniform_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_uniform_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().uniform(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@triangle_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_triangle_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().triangle(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@normal_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_normal_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().normal(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@up_slope_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_up_slope_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().up_slope(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@down_slope_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_down_slope_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().down_slope(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@thorn_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_thorn_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().thorn(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@nose_int(Integer limit) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_nose_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkRandom>().nose(scope_p->get_arg<SkInteger>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@uniform() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_uniform(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().uniform());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@uniform_range(Real min, Real max) Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Modifiers:   static
  // Author(s):   Conan Reis
  void mthd_uniform_range(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().uniform_range(
        scope_p->get_arg<SkReal>(SkArg_1),
        scope_p->get_arg<SkReal>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@uniform_symm() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_uniform_symm(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().uniform_symm());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@triangle() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_triangle(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().triangle());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@triangle_symm() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_triangle_symm(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().triangle_symm());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@normal() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_normal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().normal());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@up_slope() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_up_slope(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().up_slope());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@down_slope() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_down_slope(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().down_slope());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@thorn() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_thorn(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().thorn());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Random@nose() Real
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_nose(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkReal::new_instance(scope_p->this_as<SkRandom>().nose());
      }
    }

  //---------------------------------------------------------------------------------------

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!seed",          mthd_ctor_seed },
      { "seed",           mthd_seed },
      { "seed_set",       mthd_seed_set },
      { "coin_toss",      mthd_coin_toss },  // $Revisit - after deprecate flag on it for a while
      { "coin_toss?",     mthd_coin_toss },
      { "uniform_int",    mthd_uniform_int },
      { "triangle_int",   mthd_triangle_int },
      { "normal_int",     mthd_normal_int },
      { "up_slope_int",   mthd_up_slope_int },
      { "down_slope_int", mthd_down_slope_int },
      { "thorn_int",      mthd_thorn_int },
      { "nose_int",       mthd_nose_int },
      { "uniform",        mthd_uniform },
      { "uniform_range",  mthd_uniform_range },
      { "uniform_symm",   mthd_uniform_symm },
      { "triangle",       mthd_triangle },
      { "triangle_symm",  mthd_triangle_symm },
      { "normal",         mthd_normal },
      { "up_slope",       mthd_up_slope },
      { "down_slope",     mthd_down_slope },
      { "thorn",          mthd_thorn },
      { "nose",           mthd_nose },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkRandom::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Random);

  ms_class_p->register_method_func_bulk(SkRandom_Impl::methods_i, A_COUNT_OF(SkRandom_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkRandom::get_class()
  {
  return ms_class_p;
  }
