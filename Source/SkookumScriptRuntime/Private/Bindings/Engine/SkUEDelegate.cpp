// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Delegate (= FScriptDelegate) class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkUEDelegate.hpp"
#include "Bindings/SkUEUtils.hpp"
#include "Bindings/SkUEReflectionManager.hpp"

#include "UObject/Object.h"

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkParameters.hpp>
#include <SkookumScript/SkString.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkUEDelegate_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Skookum:   Delegate@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const FScriptDelegate & script_delegate = scope_p->this_as<SkUEDelegate>();
      *result_pp = SkString::new_instance(FStringToAString(script_delegate.ToString<UObject>()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params = equal?(Delegate num) Boolean
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
        (scope_p->this_as<SkUEDelegate>() == scope_p->get_arg<SkUEDelegate>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params ~= not_equal?(Delegate num) Boolean
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
        (scope_p->this_as<SkUEDelegate>() != scope_p->get_arg<SkUEDelegate>(SkArg_1)));
      }
    }
    
  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "String",       mthd_String },
      { "equal?",       mthd_op_equals },
      { "not_equal?",   mthd_op_not_equal },
    };

} // namespace

//---------------------------------------------------------------------------------------
void SkUEDelegate::register_bindings()
  {
  tBindingBase::register_bindings("Delegate");

  ms_class_p->register_method_func_bulk(SkUEDelegate_Impl::methods_i, A_COUNT_OF(SkUEDelegate_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEDelegate::get_class()
  {
  return ms_class_p;
  }

//---------------------------------------------------------------------------------------

void SkUEDelegate::Instance::delete_this()
  {
  new (this) SkInstance(ALeaveMemoryUnchanged); // Reset v-table back to SkInstance
  SkInstance::delete_this(); // And recycle SkInstance
  }

//---------------------------------------------------------------------------------------

void SkUEDelegate::Instance::invoke_as_method(SkObjectBase * scope_p, SkInvokedBase * caller_p, SkInstance ** result_pp, const SkClosureInvokeInfo & invoke_info, const SkExpressionBase * invoking_expr_p) const
  {
  uint32_t invoked_data_array_size = invoke_info.m_params_p->get_arg_count_total();
  SkInvokedMethod imethod(caller_p, const_cast<Instance *>(this), invoked_data_array_size, a_stack_allocate(invoked_data_array_size, SkInstance*));

  // Store expression debug info for next invoked method/coroutine.
  SKDEBUG_ICALL_STORE_GEXPR(invoking_expr_p);

  // Must be called before calling argument expressions
  SKDEBUG_ICALL_SET_EXPR(&imethod, invoking_expr_p);

  // Append argument list
  imethod.data_append_args_exprs(invoke_info.m_arguments, *invoke_info.m_params_p, scope_p);

  // Hook must be called after argument expressions and before invoke()
  SKDEBUG_HOOK_EXPR(invoking_expr_p, scope_p, &imethod, nullptr, SkDebug::HookContext_peek);

  // Invoke the UE4 function
  SkUEReflectionManager::get()->invoke_k2_delegate(as<SkUEDelegate>(), invoke_info.m_params_p, &imethod, result_pp);

  /* $Revisit - MBreyer Enable this when we support return arguments
  // Bind any return arguments
  if (!invoke_info.m_return_args.is_empty())
    {
    imethod.data_bind_return_args(invoke_info.m_return_args, *invoke_info.m_params_p);
    }
  */
  }
