// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Base classes for executed/called/invoked objects inline file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)

#include <AgogCore/AMath.hpp>

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#ifdef A_INL_IN_CPP
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkInvokedBase.inl>
#else
#include <SkookumScript/SkInvokedBase.hpp>
#endif

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkInvocation.hpp>
#include <SkookumScript/SkInvokableBase.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkTyped.hpp>
#include <SkookumScript/SkUnaryParam.hpp>


//=======================================================================================
// SkInvokedBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Returns the scope context (SkInvokedMethod or SkInvokedCoroutine)
// Returns:    SkInvokedMethod, SkInvokedCoroutine, or nullptr
// Notes:      It is useful when a data retrieval error occurs to determine which
//             coroutine / method had the error.
// Modifiers:   virtual - overriding from SkObjectBase
// #Authors   Conan Reis
SkInvokedContextBase * SkInvokedBase::get_scope_context() const
  {
  SkObjectBase * scope_p = m_scope_p;

  return scope_p ? scope_p->get_scope_context() : nullptr;
  }

//---------------------------------------------------------------------------------------

SkInvokedBase * SkInvokedBase::get_topmost_caller() const
  {
  SkInvokedBase * caller_p = const_cast<SkInvokedBase *>(this);
  while (SkInvokedBase * caller_caller_p = caller_p->m_caller_p)
    {
    caller_p = caller_caller_p;
    }
  return caller_p;
  }

//---------------------------------------------------------------------------------------
// Returns the caller context (SkInvokedMethod or SkInvokedCoroutine)
// Returns:    SkInvokedMethod, SkInvokedCoroutine or nullptr
// Notes:      It is useful to determine which coroutine / method had an error 
// Modifiers:   virtual - overriding from SkInvokedBase
// #Authors   Conan Reis
SkInvokedContextBase * SkInvokedBase::get_caller_context() const
  {
  SkInvokedBase * caller_p = m_caller_p;

  return caller_p ? caller_p->get_caller_context() : nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the *known* 0-based "stack" depth of this invokable - i.e. how many
//             *existing* ancestor method/coroutine callers does it have.
//             [1 or more invokable ancestors could be out-of-scope/stale if this
//             invokable is part of a branch - in which case only the known/existing depth
//             will be given.]
// Returns:    0-based "stack" depth
// #Authors   Conan Reis
uint32_t SkInvokedBase::get_context_depth() const
  {
  uint32_t depth = 0u;

  SkInvokedContextBase * context_p = get_caller_context();
  SkInvokedBase *        caller_p  = context_p ? context_p->m_caller_p : (SkInvokedBase *)nullptr;

  while (caller_p)
    {
    depth++;

    context_p = caller_p->get_caller_context();
    caller_p  = context_p ? context_p->m_caller_p : (SkInvokedBase *)nullptr;
    }

  return depth;
  }

//---------------------------------------------------------------------------------------
// Determines if the supplied invokable base is in the current call stack of
//             of this invoked object.
// Returns:    true if caller_p in callstack or false if not
// Arg         caller_p - invoked object to search for in call stack
// #Authors   Conan Reis
bool SkInvokedBase::is_caller(SkInvokedBase * caller_p) const
  {
  SkInvokedBase * stack_caller_p = m_caller_p;

  while (stack_caller_p)
    {
    if (stack_caller_p == caller_p)
      {
      return true;
      }

    stack_caller_p  = stack_caller_p->m_caller_p;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Returns the 'this' using this object as the scope.
// Returns:    the 'this' using this object as the scope
// Modifiers:   virtual (overridden from SkObjectBase)
// #Authors   Conan Reis
SkInstance * SkInvokedBase::get_topmost_scope() const
  {
  SkObjectBase * scope_p = m_scope_p;

  // Check if scope is lost
  return scope_p ? scope_p->get_topmost_scope() : SkBrain::ms_nil_p;
  }

//---------------------------------------------------------------------------------------
// Aborts all child/sub-calls.
// Arg         notify_caller - if true and pending count is decremented to 0 then notify
//             caller if it has one that this has *successfully* completed.  [If an
//             unsuccessful completion is desired - call abort_invoke() instead.]
// #Authors   Conan Reis
void SkInvokedBase::abort_subcalls(
  eSkNotify notify_caller // = SkNotify_success
  )
  {
  // $Revisit - CReis Track this down and try to remove it.
  // Ignore invoked objects that have already in the free list
  if (!is_valid_id())
    {
    return;
    }

  SkInvokedBase * subcall_p;

  // Convert to signed to allow for negative values
  int32_t pending_count = int32_t(m_pending_count);

  while (m_calls.is_filled())
    {
    subcall_p = m_calls.pop_first();

    // $Revisit - CReis Track this down and try to remove it.
    // Ignore invoked objects that have already in the free list
    if (subcall_p->is_valid_id())
      {
      subcall_p->abort_invoke(SkNotify_ignore);
      }

    // Assume that all sub-calls were pending
    pending_count--;
    }

  if (pending_count > 0)
    {
    // Still more pending
    m_pending_count = uint32_t(pending_count);
    }
  else
    {
    // No more pending
    m_pending_count = 0u;

    if (notify_caller != SkNotify_ignore)
      {
      SkInvokedBase * caller_p = m_caller_p;

      if (caller_p)
        {
        caller_p->pending_unregister(this, notify_caller == SkNotify_success);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Detaches from caller
// Arg         notify_caller - if true and pending count is decremented to 0 then notify
//             caller if it has one that this has *successfully* completed.
// #Authors   Conan Reis
void SkInvokedBase::detach(
  eSkNotify notify_caller // = SkNotify_success
  )
  {
  SkInvokedBase * caller_p = m_caller_p;

  m_caller_p = nullptr;

  // Ignore pending count
  if (caller_p && (notify_caller != SkNotify_ignore))
    {
    caller_p->pending_unregister(this, notify_caller == SkNotify_success);
    }
  }

//---------------------------------------------------------------------------------------
// Detaches all child/sub-calls.
// Arg         notify_caller - if true and pending count is decremented to 0 then notify
//             caller if it has one that this has *successfully* completed.
// #Authors   Conan Reis
void SkInvokedBase::detach_subcalls(
  eSkNotify notify_caller // = SkNotify_success
  )
  {
  SkInvokedBase * subcall_p;

  // Convert to signed to allow for negative values
  int32_t pending_count = int32_t(m_pending_count);

  while (m_calls.is_filled())
    {
    subcall_p = m_calls.pop_first();
    subcall_p->m_caller_p = nullptr;

    // Assume that all sub-calls were pending
    pending_count--;
    }

  if (pending_count > 0)
    {
    // Still more pending
    m_pending_count = uint32_t(pending_count);
    }
  else
    {
    // No more pending
    m_pending_count = 0u;

    if (notify_caller != SkNotify_ignore)
      {
      SkInvokedBase * caller_p = m_caller_p;

      if (caller_p)
        {
        caller_p->pending_unregister(this, notify_caller == SkNotify_success);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@abort(Boolean caller_success: true, Boolean abort_subcalls: true)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_abort(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->abort_invoke(
      scope_p->get_arg<SkBoolean>(SkArg_1) ? SkNotify_success : SkNotify_fail,
      eSkNotifyChild(scope_p->get_arg<SkBoolean>(SkArg_2)));
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@mind() Mind
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_mind(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

    SkInstance * mind_p = nullptr;
    
    if (ibase_p)
      {
      mind_p = ibase_p->get_updater();
      }

    // Use default mind if not found or if invoked object is stale
    if (mind_p == nullptr)
      {
      mind_p = SkookumScript::get_master_mind();
      }

    #ifdef SK_RUNTIME_RECOVER
      // If still null, use `nil` object
      if (mind_p == nullptr)
        {
        mind_p = SkBrain::ms_nil_p;
        }
    #endif

    mind_p->reference();
    *result_pp = mind_p;
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@abort_subcalls(Boolean caller_success: true)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_abort_subcalls(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->abort_subcalls(
      scope_p->get_arg<SkBoolean>(SkArg_1) ? SkNotify_success : SkNotify_fail);
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@detach(Boolean caller_success: true)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_detach(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->detach(
      scope_p->get_arg<SkBoolean>(SkArg_1) ? SkNotify_success : SkNotify_fail);
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@detach_subcalls(Boolean notify_success: true)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_detach_subcalls(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->detach_subcalls(
      scope_p->get_arg<SkBoolean>(SkArg_1) ? SkNotify_success : SkNotify_fail);
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@get_pending_count() Integer
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_pending_count(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

    *result_pp = SkInteger::new_instance(ibase_p ? ibase_p->pending_count() : 0u);
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@valid?() Boolean
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_validQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

    *result_pp = SkBoolean::new_instance(ibase_p != nullptr);
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@equals() Boolean  [or =]
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_op_equals(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(scope_p->get_this() == scope_p->get_arg(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@not_equal() Boolean  [or ~=]
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_op_not_equal(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(scope_p->get_this() != scope_p->get_arg(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@pending_decrement(Boolean success: true)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_pending_decrement(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->pending_return(scope_p->get_arg<SkBoolean>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// #SkParams InvokedBase@pending_increment(Integer increment: 1)
// [See script file.]
// #CppParams See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// #Authors   Conan Reis
void SkInvokedBase::mthd_pending_increment(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInvokedContextBase * ibase_p = scope_p->this_as_data<SkInvokedContextBase>();

  if (ibase_p)
    {
    ibase_p->pending_increment(uint32_t(a_max(scope_p->get_arg<SkInteger>(SkArg_1), 0)));
    }
  }


//=======================================================================================
// SkInvokedExpression Data Definitions
//=======================================================================================

AObjReusePool<SkInvokedExpression> SkInvokedExpression::ms_pool;

//=======================================================================================
// SkInvokedExpression Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Abort all sub-calls, abort this invoked expression and optionally notify
//             any caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// #Authors   Conan Reis
void SkInvokedExpression::abort_invoke(
  eSkNotify      notify_caller, // = SkNotify_fail
  eSkNotifyChild notify_child   // = SkNotifyChild_abort
  )
  {
  if (is_valid_id())
    {
    m_expr_p->on_abort(this);

    // Abort/detach sub-calls
    abort_common(notify_caller, notify_child);

    // Free up for reuse.
    pool_delete(this);
    }
  // $Vital - CReis Eliminate the need for this guard
  //#if (SKOOKUM & SK_DEBUG)
  //else
  //  {
  //  ADebug::print("Aborting an SkInvokedExpression that is already freed!\n");
  //  }
  //#endif
  }

//---------------------------------------------------------------------------------------
// Called when a pending sub-coroutine completes/returns
// Arg         completed - true if coroutine completed successfully, false if coroutine was
//             aborted before it was complete.  (Default true)
// #Authors   Conan Reis
void SkInvokedExpression::pending_return(
  bool completed // = true
  )
  {
  if (is_valid_id())
    {
    m_pending_count--;

    if (completed)
      {
      if (m_pending_count == 0u)
        {
        m_index++;

        // Resumes previously suspended invocation.
        m_expr_p->invoke_iterate(this);
        }
      }
    else
      {
      abort_invoke();
      }
    }
  //#if (SKOOKUM & SK_DEBUG)
  //else
  //  {
  //  // $Revisit - CReis Find and fix this
  //  ADebug::print("Tried to return SkInvokedExpression that is in the free pool!\n");
  //  }
  //#endif
  }

#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Get the expression that called/created this invoked object
// See:        get_debug_info()
// Modifiers:   virtual - overridden from SkInvokedBase
// #Authors   Conan Reis
SkExpressionBase * SkInvokedExpression::get_caller_expr() const
  {
  return const_cast<SkExpressionBase *>(m_expr_p);
  }

//---------------------------------------------------------------------------------------
// Gets the current source character index or ADef_uint32 if it cannot be
//             determined at runtime.
// #Authors   Conan Reis
SkDebugInfo SkInvokedExpression::get_debug_info() const
  {
  // Expressions may have a better idea of the sub-expression that is currently running so
  // this iexpr is passed on to get more info.
  return m_expr_p->get_debug_info(*this);
  }

#endif

#ifdef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkInvokedExpressions
AObjReusePool<SkInvokedExpression> & SkInvokedExpression::get_pool()
  {
  return ms_pool;
  }

#endif

//=======================================================================================
// SkInvokedContextBase Class Data
//=======================================================================================

#if defined(SKDEBUG_COMMON)

  // Last expression about to invoke a method/coroutine - passed globally (yuck!) to avoid
  // different parameters for debug/non-debug.
  const SkExpressionBase * SkInvokedContextBase::ms_last_expr_p = SkDebugInfo::get_expr_default();

#endif


//=======================================================================================
// SkInvokedContextBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Clean up before deletion
SkInvokedContextBase::~SkInvokedContextBase()
  {
  data_empty();
  }

//---------------------------------------------------------------------------------------
// Fills the invoked routine with appropriate arguments - using object instances as
// arguments.
//
// #See Also
//   other data_append_*() methods, data_bind_return_args(), data_empty(),
//   data_append_vars(), data_remove_vars()
//   
// #Author(s) Conan Reis
void SkInvokedContextBase::data_append_args(
  // Pointers to objects to use as arguments - each one present should have its reference
  // count incremented and each defaulted/skipped argument should be a nullptr element.
  // If arg_count is 0 this is ignored
  SkInstance ** arguments_pp,
  // number of arguments (including default/skipped nullptr arguments) in arguments_pp
  uint32_t arg_count,
  // method / coroutine being called
  const SkParameters & invokable_params
  )
  {
  // $Note - CReis This method is very similar to data_append_args_exprs() below, but the
  // arguments are already instances rather than expressions that need to be evaluated.
  // Any changes here may need to be reflected below.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Fill positioned arguments & defaults

  const tSkParamList & params       = invokable_params.get_param_list();
  uint32_t             params_count = params.get_length();

  const tSkParamReturnList & return_params       = invokable_params.get_param_return_list();
  uint32_t                   return_params_count = return_params.get_length();

  // Get ready to bind temp variables (positioned according to parameters) with objects
  // - all indexes should be replaced with objects resolved from args or defaults

  uint32_t data_count = m_data.get_length();

  SK_ASSERTX(m_data.get_size() >= data_count + params_count + return_params_count, "Not enough data storage space.");

  SkInstance ** data_pp = m_data.get_array_end();

  if (params_count)
    {
    SkInstance *        instance_p    = nullptr;
    SkInstance **       args_pp       = arguments_pp;
    SkInstance **       args_end_pp   = arguments_pp + arg_count;
    SkParameterBase *   param_p       = nullptr;
    SkParameterBase **  params_pp     = params.get_array();
    SkParameterBase **  params_end_pp = params_pp + params_count;

    while (params_pp < params_end_pp)
      {
      param_p = *params_pp;

      // Determine if more args need to be placed or if the remaining args use defaults
      if (args_pp < args_end_pp)
        {
        instance_p = *args_pp;
        args_pp++;
        }

      if (instance_p == nullptr)
        {
        switch (param_p->get_kind())
          {
          case SkParameter_unary:
            // Unary parameter without a default expression
            SK_ERRORX(
                // Extra check for missing argument when there is no default
                SkDebug::get_context_string(
                  AString(512u,
                    "Called method '%s' with argument '%s' missing, but it has no default!",
                    as_invoke_string().as_cstr(), param_p->get_name_cstr_dbg()), m_caller_p, m_scope_p));

            // Give value so that it might be possible to recover
            instance_p = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
            break;

          case SkParameter_unary_default:
            // Unary parameter with a default expression using method's scope when evaluated (->)
            // Uses "this" for defaults rather than m_scope_p to allow later args to reference earlier args.
            instance_p = static_cast<SkUnaryParam *>(param_p)->get_default_expr()->invoke_now(this, m_caller_p);
            break;

          case SkParameter_group:
            // Group parameter
            // - Use default group argument - an empty list
            instance_p = SkList::new_instance();
          }
        }

      // $Revisit - CReis Since this method is only called via C++ it might be an idea to do
      // type checking on the arguments to ensure that they are of the expected type - i.e.:
      // else
      //   {
      //   type check
      //   }

      *data_pp = instance_p;

      // Note that the length is set as each argument added since default expressions may
      // reference earlier arguments when they are invoked.
      m_data.set_length_unsafe(++data_count);

      data_pp++;
      params_pp++;  // Move to next parameter
      }  // while
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add return arguments with nil as default
  if (return_params_count)
    {
    // $Revisit - CReis Perhaps something could be done to flag the return parameters that
    // are not actually desired - i.e. not present in the return arguments.
    // This functionality would also likely require a change to the parser since it ensures
    // that all return parameters are set with appropriate types by the end of an invokable.

    SkTypedName ** rparams_pp     = return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + return_params_count;

    m_data.set_length_unsafe(data_count + return_params_count);

    while (rparams_pp < rparams_end_pp)
      {
      // nil does not need to be referenced/dereferenced
      *data_pp = SkBrain::ms_nil_p;
      data_pp++;
      rparams_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Copies variables into data array, increments refcount
void SkInvokedContextBase::data_append_vars_ref(SkInstance ** var_pp, uint32_t var_count)
  {
  for (uint32_t i = 0; i < var_count; ++i)
    {
    SkInstance * var_p = var_pp[i];
    m_data.append(*var_p);
    var_p->reference();
    }
  }

//---------------------------------------------------------------------------------------
// Fills the invoked routine with appropriate arguments - using expressions to invoke as
// arguments.
//
// #See Also
//   other data_append_*() methods, data_bind_return_args(), data_empty(),
//   data_append_vars(), data_remove_vars()
//   
// #Author(s) Conan Reis
void SkInvokedContextBase::data_append_args_exprs(
  // argument expressions
  const APArrayBase<SkExpressionBase> & args,
  // parameters of method / coroutine being called
  const SkParameters & invokable_params,
  // scope context to use for non-default argument invocation
  SkObjectBase * arg_scope_p
  )
  {
  // $Note - CReis This method is very similar to data_append_args() above, but the
  // arguments are expressions that need to be evaluated rather than instances.
  // Any changes here may need to be reflected above.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Fill positioned arguments & defaults

  const tSkParamList & params       = invokable_params.get_param_list();
  uint32_t             params_count = params.get_length();

  const tSkParamReturnList & return_params       = invokable_params.get_param_return_list();
  uint32_t                   return_params_count = return_params.get_length();

  uint32_t data_count = m_data.get_length();

  SK_ASSERTX(m_data.get_size() >= data_count + params_count + return_params_count, "Not enough data storage space.");

  SkInstance ** data_pp = m_data.get_array_end();

  if (params_count)
    {
    SkExpressionBase *  arg_expr_p;
    SkInstance *        instance_p    = nullptr;
    SkInvokedBase *     caller_p      = m_caller_p;
    SkExpressionBase ** args_pp       = args.get_array();
    SkExpressionBase ** args_end_pp   = args_pp + args.get_length();
    SkParameterBase *   param_p;
    SkParameterBase **  params_pp     = params.get_array();
    SkParameterBase **  params_end_pp = params_pp + params_count;

    while (params_pp < params_end_pp)
      {
      // Determine if more args need to be placed or if the remaining args use defaults
      arg_expr_p = nullptr;
      if (args_pp < args_end_pp)
        {
        arg_expr_p = *args_pp;
        args_pp++;
        }

      param_p = *params_pp;

      if (arg_expr_p)
        {
        // If expression given, just invoke it
        arg_expr_p->invoke(arg_scope_p, caller_p, &instance_p);
        }
      else
        {
        // Else figure out a default to use
        eSkParameter kind = param_p->get_kind();
        if (kind == SkParameter_unary_default)
          {
          #if defined(SKDEBUG_COMMON)
            SkDebug::ms_no_step_default_hack = true;
          #endif
          // Use default argument with scope of invokable
          static_cast<SkUnaryParam *>(param_p)->get_default_expr()->invoke(this, caller_p, &instance_p);
          #if defined(SKDEBUG_COMMON)
            SkDebug::ms_no_step_default_hack = false;
          #endif
          }
        else
          {
          SK_ASSERTX(kind == SkParameter_group, "SkParameter_unary must have non-null value.");
          // Use default group argument - an empty list
          instance_p = SkList::new_instance();
          }
        }

      *data_pp = instance_p;

      // Note that the length is set as each argument added since default expressions may
      // reference earlier arguments when they are invoked.
      m_data.set_length_unsafe(++data_count);

      data_pp++;
      params_pp++;  // Move to next parameter
      }  // while
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add return arguments with nil as default
  if (return_params_count)
    {
    // $Revisit - CReis Perhaps something could be done to flag the return parameters that
    // are not actually desired - i.e. not present in return arguments.
    // This functionality would also likely require a change to the parser since it ensures
    // that all return parameters are set with appropriate types by the end of an invokable.

    SkTypedName ** rparams_pp     = return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + return_params_count;

    m_data.set_length_unsafe(data_count + return_params_count);

    while (rparams_pp < rparams_end_pp)
      {
      // nil does not need to be referenced/dereferenced
      *data_pp = SkBrain::ms_nil_p;

      data_pp++;
      rparams_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Binds return parameters to any return arguments.
//             Called after data_append_args() and the invocation.
// Arg         invocation_args - call info used to supply arguments (SkMethodCallBaseor
//             SkCoroutineCall)
// Arg         invokable_params - parameters of method / coroutine being called
// #Authors   Conan Reis
void SkInvokedContextBase::data_bind_return_args(
  const APArrayBase<SkIdentifierLocal> & return_args,
  const SkParameters & invokable_params
  )
  {
  uint32_t return_arg_count = return_args.get_length();

  SK_ASSERTX(return_arg_count, "Caller of SkInvokedContextBase::data_bind_return_args() must check that return_args is not empty before calling - this is an optimization to avoid unnecessary calls.");

  SkObjectBase * scope_p = m_scope_p;

  // Ensure that the scope is still valid
  if (scope_p)
    {
    SkIdentifierLocal ** return_arg_pp     = return_args.get_array();
    SkIdentifierLocal ** return_arg_end_pp = return_arg_pp + return_arg_count;
    SkInvokedBase * caller_p               = m_caller_p;

    // Position pointer in temp data variables just past the arguments and starting at the return arguments
    SkInstance ** return_data_pp = m_data.get_array() + invokable_params.get_param_list().get_length();

    while (return_arg_pp < return_arg_end_pp)
      {
      // Return args that are nullptr are skipped
      if (*return_arg_pp)
        {
        // We don't know if return arg is bound for first time, so safe assumption is not
        (*return_arg_pp)->bind_data(*return_data_pp, caller_p, caller_p, false);
        }

      return_data_pp++;
      return_arg_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// 
void SkInvokedContextBase::data_empty_compact()
  {
  data_destroy_vars(0, m_data.get_length());
  m_data.empty_compact();
  }

//---------------------------------------------------------------------------------------
// Returns the caller context (SkInvokedMethod or SkInvokedCoroutine)
// Returns:    SkInvokedMethod, SkInvokedCoroutine or nullptr
// Notes:      It is useful to determine which coroutine / method had an error 
// Modifiers:   virtual - overriding from SkInvokedBase
// #Authors   Conan Reis
SkInvokedContextBase * SkInvokedContextBase::get_caller_context() const
  {
  return const_cast<SkInvokedContextBase *>(this);
  }

//---------------------------------------------------------------------------------------
// Returns the topmost context (SkInvokedMethod or SkInvokedCoroutine)
// Returns:    SkInvokedMethod, SkInvokedCoroutine or nullptr
// Notes:      It is useful when a data retrieval error occurs to determine which
//             topmost coroutine / method had the error.
// Modifiers:   virtual - overriding from SkObjectBase
// #Authors   Conan Reis
SkInvokedContextBase * SkInvokedContextBase::get_scope_context() const
  {
  return const_cast<SkInvokedContextBase *>(this);
  }


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Returns a name representation of itself for debugging purposes
// Returns:    Name of this coroutine
// Arg         flags - See eSkInvokeInfo (only the flags SkInvokeInfo_scope, SkInvokeInfo_args
//             and SkInvokeInfo_this are examined - the other flags are ignored) 
// See:        get_scope_context()
// Modifiers:   virtual - overridden from SkInvokedContextBase
// #Authors   Conan Reis
AString SkInvokedContextBase::as_invoke_string(
  uint32_t flags // = SkInvokeInfo_scope
  ) const
  {
  AString               str;
  AString               scope_name;
  AString               invoke_name("<anonymous>");
  uint32_t              init_size = invoke_name.get_length() + 2u;
  const tSkParamList *  params_p = nullptr;
  uint32_t              param_count = 0;
  SkClass *             scope_p = nullptr;
  SkInvokableBase *     invokable_p = get_invokable();
  if (invokable_p)
    {
    params_p    = &invokable_p->get_params().get_param_list();
    param_count = params_p->get_length();
    invoke_name = invokable_p->get_name_str_dbg();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Guess string size - it will expand the string buffer if needed, but it will be fastest
    // if the guessed size is large enough.

    if (flags & SkInvokeInfo_scope)
      {
      // Closures may not have a scope so test first
      scope_p = invokable_p->get_scope();

      if (scope_p)
        {
        scope_name = scope_p->get_name_str_dbg();
        init_size += scope_name.get_length() + 1u;
        }
      }

    if (flags & SkInvokeInfo_args)
      {
      // Assume average of 24 characters per argument
      init_size += param_count * 24u;
      }
    }

  if (flags & SkInvokeInfo_this)
    {
    init_size += 50u;
    }

  bool updater_info = (flags & SkInvokeInfo_updater) && (get_invoke_type() >= SkMember_coroutine);

  if (updater_info)
    {
    init_size += 50u;
    }

  if (flags & SkInvokeInfo_depth)
    {
    init_size += 10u;
    }

  str.ensure_size_empty(init_size);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add scope
  if (scope_p)
    {
    str.append(scope_name);
    str.append('@');
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add name
  str.append(invoke_name);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append arguments
  if (param_count && (flags & SkInvokeInfo_args))
    {
    str.append('(');

    // The indexes of the arguments and the parameters should match so there should be no
    // need to look them up by name.
    // The number of arguments can be less than the number of parameters if printing out
    // during the initialization of the argument list - such as with default arguments.
    SkInstance **      args_pp        = m_data.get_array();
    uint32_t           args_remaining = m_data.get_length();
    SkParameterBase ** params_pp      = params_p->get_array();
    SkParameterBase ** params_end_pp  = params_pp + param_count;

    while (params_pp < params_end_pp)
      {
      str.append((*params_pp)->get_name_str_dbg());
      str.append(": ", 2u);

      if (args_remaining)
        {
        (*args_pp)->as_code_append(&str);
        args_pp++;
        args_remaining--;
        }
      else
        {
        str.append("???", 3u);
        }

      params_pp++;

      if (params_pp < params_end_pp)
        {
        str.append(", ", 2u);
        }
      }

    str.append(')');
    }
  else
    {
    str.append("()", 2u);
    }


  // $Revisit - CReis Could add primary return and return arguments

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append scope/this
  if (flags & SkInvokeInfo_this)
    {
    str.append(", this = ", 9u);

    SkInstance * scope_top_p = m_scope_p ? m_scope_p->get_topmost_scope() : nullptr;

    if (scope_top_p)
      {
      scope_top_p->as_code_append(&str, SkCodeFlag__default, const_cast<SkInvokedContextBase *>(this));
      }
    else
      {
      str.append("???", 3u);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append updater
  if (updater_info)
    {
    SkMind * updater_p = get_updater();

    if (updater_p)
      {
      // $Revist - CReis Could use as_code_append() though it calls String() and should inlclude class type
      str.append(", updater = ", 12);
      updater_p->as_code_append(&str, SkCodeFlag__default, const_cast<SkInvokedContextBase *>(this));
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append call depth
  if (flags & SkInvokeInfo_depth)
    {
    str.append(a_str_format(", depth:%u", get_context_depth()));
    }

  return str;
  }

#endif  // SK_AS_STRINGS


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Get the expression that called/created this invoked object
// See:        get_debug_info()
// Modifiers:   virtual - overridden from SkInvokedBase
// #Authors   Conan Reis
SkExpressionBase * SkInvokedContextBase::get_caller_expr() const
  {
  SkInvokedBase * caller_p = m_caller_p;

  if (caller_p)
    {
    SkExpressionBase * expr_p = caller_p->get_caller_expr();

    if (expr_p)
      {
      return expr_p->find_expr_by_pos(m_source_idx);
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the current source character index or ADef_uint32 if it cannot be
//             determined at runtime.
// #Authors   Conan Reis
SkDebugInfo SkInvokedContextBase::get_debug_info() const
  {
  SkDebugInfo debug_info = {m_source_idx, m_debug_info};

  return debug_info;
  }

#endif // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Registers the atomic classes, methods, etc.
// Notes:       This method is called by Brain::initialize_post_load()
// Modifiers:    static
// #Authors  Conan Reis
void SkInvokedBase::register_bindings()
  {
  SkBrain::ms_invoked_base_class_p->register_method_func("abort",             mthd_abort);
  SkBrain::ms_invoked_base_class_p->register_method_func("abort_subcalls",    mthd_abort_subcalls);
  SkBrain::ms_invoked_base_class_p->register_method_func("detach",            mthd_detach);
  SkBrain::ms_invoked_base_class_p->register_method_func("detach_subcalls",   mthd_detach_subcalls);
  SkBrain::ms_invoked_base_class_p->register_method_func("mind",              mthd_mind);
  SkBrain::ms_invoked_base_class_p->register_method_func("pending_count",     mthd_pending_count);
  SkBrain::ms_invoked_base_class_p->register_method_func("pending_decrement", mthd_pending_decrement);
  SkBrain::ms_invoked_base_class_p->register_method_func("pending_increment", mthd_pending_increment);
  SkBrain::ms_invoked_base_class_p->register_method_func("valid?",            mthd_validQ);
  SkBrain::ms_invoked_base_class_p->register_method_func(ASymbolX_equalQ,     mthd_op_equals);
  SkBrain::ms_invoked_base_class_p->register_method_func(ASymbolX_not_equalQ, mthd_op_not_equal);
  }

