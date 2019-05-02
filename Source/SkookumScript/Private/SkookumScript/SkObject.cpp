// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic object class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkObject.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkObject_Impl
  {
  //---------------------------------------------------------------------------------------
  // Sk Params Object@same?(Object operand) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_sameQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() == scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params abort_coroutines_on_this()
  static void mthd_abort_coroutines_on_this(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    bool is_success = scope_p->get_arg<SkBoolean>(SkArg_1);
    scope_p->get_this()->abort_coroutines_on_this(is_success ? SkNotify_success : SkNotify_fail);
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Object@String() String
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_as_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      // This works for both classes and class instances
      *result_pp = SkString::new_instance(scope_p->get_this()->get_key_class()->get_name_str_dbg());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params = equal?(Class class) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() == scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params ~= not_equal?(Class class) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() != scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Object@class_of?(Class class) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_class_ofQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this()->get_key_class()->is_class(
        *scope_p->get_arg_data<SkClass>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Object@class_actor?() Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_class_actorQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this()->get_key_class()->is_builtin_actor_class());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Object@subclass_of?(Class superclass) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_subclass_ofQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this()->get_key_class()->is_subclass(
        *scope_p->get_arg_data<SkClass>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Object@superclass_of?(Class subclass) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_superclass_ofQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this()->get_key_class()->is_superclass(
        *scope_p->get_arg_data<SkClass>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params print({Object} objs_as_strs)
  // [See script file.]
  // Notes:      This is a class method.
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_print(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    AString str;
    const SkInstanceList & objs = scope_p->get_arg<SkList>(SkArg_1);

    objs.append_elems_as_strings(&str, scope_p);

    // @Revisit - CReis May need to ensure printing to standard output
    SkDebug::print_agog(str, SkLocale_all, SkDPrintType_standard);
    }

  //---------------------------------------------------------------------------------------
  // Sk Params println({Object} objs_as_strs)
  // [See script file.]
  // Notes:      This is a class method.
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_println(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    AString str;
    const SkInstanceList & objs = scope_p->get_arg<SkList>(SkArg_1);

    objs.append_elems_as_strings(&str, scope_p);
    str.append('\n');

    SkDebug::print_agog(str, SkLocale_all, SkDPrintType_standard);
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: Object@_wait(Real seconds: 0.0)
  // # C++ Args:  See tSkCoroutineFunc or tSkCoroutineMthd in SkookumScript/SkCoroutine.hpp
  // # Author(s): Conan Reis
  static bool coro_wait(SkInvokedCoroutine * scope_p)
    {
    if (scope_p->m_update_count == 0u)
      {
      tSkReal seconds = scope_p->get_arg<SkReal>(SkArg_1);

      //A_DPRINT(
      //  "\nWait: %g until: %g  - current sim time: %g\n",
      //  seconds, SkookumScript::get_sim_time() + seconds, SkookumScript::get_sim_time());
      scope_p->m_update_next = SkookumScript::get_sim_time() + f64(seconds);

      return (seconds < 0.0f);
      }

    //A_DPRINT("\nWaited: %g  - current sim time: %g\n", scope_p->get_arg<SkReal>(SkArg_1), SkookumScript::get_sim_time());
    return true;
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: Object@_wait_until(Real sim_time)
  // # C++ Args:  See tSkCoroutineFunc or tSkCoroutineMthd in SkookumScript/SkCoroutine.hpp
  // # Author(s): Conan Reis
  static bool coro_wait_until(SkInvokedCoroutine * scope_p)
    {
    if (scope_p->m_update_count == 0u)
      {
      // $Revisit - CReis This is now a f64 so the "sim_time" argument may not have the precision that is desired.
      scope_p->m_update_next = scope_p->get_arg<SkReal>(SkArg_1);

      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------------------

  // Instance methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "String",                   mthd_as_string },
      { "same?",                    mthd_sameQ },
      { "abort_coroutines_on_this", mthd_abort_coroutines_on_this },
    };

  // Class methods
  static const SkClass::MethodInitializerFunc methods_c[] =
    {
      //{ "String",           mthdc_as_string },
      { "class_of?",        mthdc_class_ofQ },
      { "class_actor?",     mthdc_class_actorQ },
      { "equal?",           mthdc_op_equals },
      { "not_equal?",       mthdc_op_not_equal },
      { "print",            mthdc_print },
      { "println",          mthdc_println },
      { "subclass_of?",     mthdc_subclass_ofQ },
      { "superclass_of?",   mthdc_superclass_ofQ },
    };

  // Coroutines
  static const SkClass::CoroutineInitializerFunc coroutines_i[] =
    {
      { "_wait",            coro_wait },
      { "_wait_until",      coro_wait_until },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkObject::register_bindings()
  {
  initialize_class(ASymbolId_Object);

  ms_class_p->register_method_func_bulk(SkObject_Impl::methods_i, A_COUNT_OF(SkObject_Impl::methods_i), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkObject_Impl::methods_c, A_COUNT_OF(SkObject_Impl::methods_c), SkBindFlag_class_no_rebind);
  ms_class_p->register_coroutine_func_bulk(SkObject_Impl::coroutines_i, A_COUNT_OF(SkObject_Impl::coroutines_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkObject::get_class()
  {
  return ms_class_p;
  }

