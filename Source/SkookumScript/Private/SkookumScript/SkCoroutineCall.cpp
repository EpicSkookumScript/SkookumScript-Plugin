// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for info used to make a method call/invocation - i.e. coroutine
//             identifier (name/index) and passed argument info.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkCoroutineCall.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkCoroutineCall.inl>
#endif

#if (SKOOKUM & SK_DEBUG)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkCoroutine.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkMind.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invoke calls when it is only known that an instance is of type
//             SkInvokeBase
// Returns:    SkInvokeType_coroutine
// Modifiers:   virtual from SkInvokeBase
// Author(s):   Conan Reis
eSkInvokeType SkCoroutineCall::get_invoke_type() const
  {
  return SkInvokeType_coroutine;
  }

//---------------------------------------------------------------------------------------
// Evaluates invocation expression and returns the resulting instance if
//             desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         receiver_p - - receiver instance of the call - also used for scope for
//             for default argument data/method/etc. look-ups.
// Arg         scope_p - data scope where call was made for non-default argument look-ups            
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        SkInvocation::invoke(), SkInvokeCascade::invoke()
// Modifiers:   virtual (overriding pure from SkInvokeBase)
// Author(s):   Conan Reis
SkInvokedBase * SkCoroutineCall::invoke_call(
  SkInstance *    receiver_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  SkInstance **   result_pp
  ) const
  {
  // Look for coroutine
  SkCoroutineBase * coroutine_p = nullptr;
  SkClass * call_scope_p = m_scope_p ? m_scope_p : receiver_p->get_class();
  int16_t vtable_index = m_data_idx;
  #if SKOOKUM & SK_DEBUG
    if (vtable_index == SkQualifier::ms_invalid_vtable_index)
      {
      coroutine_p = call_scope_p->find_coroutine_inherited(get_name());
      }
    else
  #endif
      {
      coroutine_p = static_cast<SkCoroutineBase *>(call_scope_p->get_invokable_from_vtable_i(vtable_index));
      }

  #if SKOOKUM & SK_DEBUG
    // If not found, might be due to recent live update and the vtable not being updated yet - try finding it by name
    if (coroutine_p == nullptr || coroutine_p->get_name() != get_name())
      {
      coroutine_p = call_scope_p->find_coroutine_inherited(get_name());
      }
  #endif

  #ifdef SK_RUNTIME_RECOVER
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for missing coroutine
    if (coroutine_p == nullptr || coroutine_p->get_name() != get_name())
      {
      // $Note - CReis This should not need to be checked here at runtime - the parser
      // should have already prevented any misuse.  However an invalid type cast can be
      // used to fool the parser.
      SK_ERROR_INFO(
        a_str_format(
          "Requested non-existing coroutine %s from class %s!\n"
          "[Bad type cast or didn't check for nil?]",
          as_string_name().as_cstr(), call_scope_p->get_name().as_cstr_dbg()),
        caller_p);

      if (result_pp)
        {
        // Wanted a return so return a nil so there is something
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return nullptr;
      }
  #endif

  // Create invoked coroutine
  SkInvokedCoroutine * icoroutine_p = SkInvokedCoroutine::pool_new(coroutine_p);

  // Set parameters
  icoroutine_p->reset(SkCall_interval_always, caller_p, receiver_p, nullptr, m_return_args.is_filled() ? &m_return_args : nullptr);

  #if defined(SKDEBUG_COMMON)
    // Set with SKDEBUG_ICALL_STORE_GEXPR stored here before calls to argument expressions
    // overwrite it.
    const SkExpressionBase * call_expr_p = SkInvokedContextBase::ms_last_expr_p;
  #endif

  SKDEBUG_ICALL_SET_EXPR(icoroutine_p, call_expr_p);

  // Fill invoked coroutine's argument list
  icoroutine_p->data_append_args_exprs(m_arguments, icoroutine_p->m_coroutine_p->get_params(), scope_p);

  SKDEBUG_HOOK_EXPR(call_expr_p, scope_p, icoroutine_p, nullptr, SkDebug::HookContext_peek);

  // Return the invoked coroutine as a result
  if (result_pp)
    {
    *result_pp = icoroutine_p->as_new_instance();
    }

  // Invoke the coroutine on the receiver - try to have it complete this frame
  return icoroutine_p->on_update()
    ? nullptr        // Completed this frame/immediately
    : icoroutine_p;  // Deferred completion
  }

//---------------------------------------------------------------------------------------
// Evaluates the coroutine call on the next update cycle.
// Arg         receiver_p - receiver instance of the call - also used for scope for
//             data/method/etc. look-ups for default arguments
// Arg         update_interval - Specifies how often the coroutine should be updated in
//             seconds.  (Default SkCall_interval_always)
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             notified when this invocation is complete.  (Default nullptr)
// Arg         updater_p - mind that will update invoked coroutine as needed - generally
//             same updater as the caller.  If nullptr the caller's updater is used and if
//             the caller is nullptr scope_p is used.
void SkCoroutineCall::invoke_schedule(
  SkInstance * receiver_p,
  f32 update_interval,      // = SkCall_interval_always
  SkInvokedBase * caller_p, // = nullptr
  SkMind * updater_p        // = nullptr
  )
  {
  // Look for coroutine
  SkCoroutineBase * coroutine_p = nullptr;
  SkClass * call_scope_p = m_scope_p ? m_scope_p : receiver_p->get_class();
  int16_t vtable_index = m_data_idx;
  #if SKOOKUM & SK_DEBUG
    if (vtable_index == SkQualifier::ms_invalid_vtable_index)
      {
      coroutine_p = call_scope_p->find_coroutine_scoped_inherited(*this);
      }
    else
  #endif
      {
      coroutine_p = static_cast<SkCoroutineBase *>(call_scope_p->get_invokable_from_vtable_i(vtable_index));
      SK_ASSERTX(coroutine_p->get_name() == get_name(), "Bad vtable lookup!");
      }

  // Create invoked coroutine
  SkInvokedCoroutine * icoroutine_p = SkInvokedCoroutine::pool_new(coroutine_p);

  // Set parameters
  icoroutine_p->reset(update_interval, caller_p, receiver_p, updater_p, m_return_args.is_filled() ? &m_return_args : nullptr);

  // Fill invoked coroutine's argument list
  icoroutine_p->data_append_args_exprs(m_arguments, coroutine_p->get_params(), caller_p->get_caller_context());

  SKDEBUG_ICALL_SET_INTERNAL(icoroutine_p);

  // Append to coroutine update list
  icoroutine_p->m_mind_p->coroutine_track_init(icoroutine_p);
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCoroutineCall::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkCoroutineCall, 0u),
    (m_arguments.get_length() + m_return_args.get_length()) * sizeof(void *),
    m_arguments.track_memory(mem_stats_p) + m_return_args.track_memory(mem_stats_p));
  }

