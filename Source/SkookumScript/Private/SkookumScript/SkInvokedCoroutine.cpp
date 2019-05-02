// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for executed/called/invoked coroutines include file
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)

#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvokedCoroutine.hpp>
  #include <SkookumScript/SkInvokedCoroutine.inl>
  #include <SkookumScript/SkClass.hpp>
#else
  #include <SkookumScript/SkClass.hpp>
  #include <SkookumScript/SkInvokedCoroutine.hpp>
#endif

#if (SKOOKUM & SK_DEBUG)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

// Default value indicating that invoked coroutine is not in its actors update list.
const uint32_t SkInvokedCoroutine_scheduled_return = ADef_uint32;

//=======================================================================================
// Data Definitions
//=======================================================================================

AObjReusePool<SkInvokedCoroutine> SkInvokedCoroutine::ms_pool;

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Default Constructor
// Returns:     itself
// Author(s):    Conan Reis
SkInvokedCoroutine::SkInvokedCoroutine() :
  m_update_count(0u),
  m_update_interval(0.0f),
  m_update_next(SkookumScript::get_sim_time()),
  m_coroutine_p(nullptr),
  m_flags(Flag__default)
  {
  // $Revisit - CReis Might want to not set any data members since it will always be grabbed from a pool.
  }

//---------------------------------------------------------------------------------------
// Destructor
SkInvokedCoroutine::~SkInvokedCoroutine()
  {
  // Remove quick storage so APArray destructor will not try to free it to the heap
  if (m_data.get_array() == m_data_quick_storage)
    {
    m_data.set_custom_memory_empty_unsafe(nullptr, 0);
    }
  }

//---------------------------------------------------------------------------------------
// Returns itself as an instance
// Modifiers:   virtual
// Author(s):   Conan Reis
SkInstance * SkInvokedCoroutine::as_new_instance() const
  {
  return SkInstance::new_instance(SkBrain::ms_invoked_coroutine_class_p, const_cast<SkInvokedCoroutine *>(this));
  }

//---------------------------------------------------------------------------------------
// Returns an instance object wrapper for the supplied invoked coroutine
//             - which may be stale/nullptr.
// Modifiers:   static
// Author(s):   Conan Reis
SkInstance * SkInvokedCoroutine::new_instance(SkInvokedCoroutine * icoroutine_p)
  {
  return SkInstance::new_instance(SkBrain::ms_invoked_coroutine_class_p, icoroutine_p);
  }

//---------------------------------------------------------------------------------------
// Abort all sub-calls, abort this invoked expression and optionally notify
//             any caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// Notes:       This code should be duplicated in SkMind::abort_coroutines()
// Author(s):   Conan Reis
void SkInvokedCoroutine::abort_invoke(
  eSkNotify      notify_caller, // = SkNotify_fail
  eSkNotifyChild notify_child   // = SkNotifyChild_abort
  )
  {
  // $Revisit - CReis Track this down and try to remove it.
  // Ignore invoked objects that have already in the free list
  if (!is_valid_id())
    {
    return;
    }

  stop_tracking();

  // Ignore return arguments

  // Abort/detach sub-calls
  abort_common(notify_caller, notify_child);

  // Free up for reuse.
  pool_delete(this);
  }

//---------------------------------------------------------------------------------------
// Called when a pending sub-coroutine completes/returns and should be
//             *scheduled* to return on the next SkookumScript update.
// Arg         completed - true if coroutine completed successfully, false if coroutine was
//             aborted before it was complete.
// See:        pending_return()
// Author(s):   Conan Reis
void SkInvokedCoroutine::pending_schedule(
  bool completed // = true
  )
  {
  m_pending_count--;

  if (completed)
    {
    const SkCoroutineBase * coroutine_p = m_coroutine_p;

    if (m_pending_count == 0u)
      {
      // $Revisit - CReis The updater might always be non-nullptr so tests for nullptr could be redundant
      SkMind * updater_p = m_mind_p;

      if (coroutine_p->get_invoke_type() != SkInvokable_coroutine)
        {
        // It is an C++ coroutine
        if (updater_p)
          {
          // Add to list for next update
          updater_p->coroutine_track_updating(this);
          }
        else
          {
          // There is no updater, so update immediately
          on_update();
          }
        }
      else
        {
        // It is a custom coroutine
        SkInvokedBase * caller_p = m_caller_p;

        // Return to caller if one is waiting
        if (caller_p && caller_p->pending_count())
          {
          // Call pending_return() on next update
          m_update_count = SkInvokedCoroutine_scheduled_return;

          if (updater_p)
            {
            // Add to list for next update
            updater_p->coroutine_track_updating(this);
            }
          }
        else
          {
          // Bind any return arguments
          if (m_return_args_p && !m_return_args_p->is_empty())
            {
            data_bind_return_args(*m_return_args_p, m_coroutine_p->get_params());
            }

          pool_delete(this);
          }
        }
      }
    }
  else
    {
    // Did not complete
    abort_invoke();
    }
  }

//---------------------------------------------------------------------------------------
// Called when a pending sub-coroutine completes/returns and it should
//             immediately return.
// Arg         completed - true if coroutine completed successfully, false if coroutine was
//             aborted before it was complete.  (Default true)
// See:        pending_schedule()
// Author(s):   Conan Reis
void SkInvokedCoroutine::pending_return(
  bool completed // = true
  )
  {
  m_pending_count--;

  if (completed)
    {
    const SkCoroutineBase * coroutine_p = m_coroutine_p;

    if (m_pending_count == 0u)
      {
      // $Revisit - CReis The updater might always be non-nullptr so tests for nullptr could be redundant
      SkMind * updater_p = m_mind_p;

      if (coroutine_p->get_invoke_type() != SkInvokable_coroutine)
        {
        // It is an C++ coroutine
        if (updater_p)
          {
          updater_p->coroutine_track_updating(this);
          }

        // Update immediately
        on_update();
        }
      else
        {
        // It is a custom coroutine
        SkInvokedBase * caller_p  = m_caller_p;

        if (updater_p)
          {
          updater_p->coroutine_track_stop(this);
          }

        // Bind any return arguments
        if (m_return_args_p && !m_return_args_p->is_empty())
          {
          data_bind_return_args(*m_return_args_p, coroutine_p->get_params());
          }

        // Return to caller if one is waiting
        if (caller_p && caller_p->pending_count())
          {
          caller_p->pending_unregister(this, coroutine_p != nullptr);
          }

        pool_delete(this);
        }
      }
    }
  else
    {
    // Did not complete
    abort_invoke();
    }
  }

//---------------------------------------------------------------------------------------
// Suspends coroutine - stops it from being called, sets the suspended flag
//             and increments the pending count.  It also moves it from the updater's
//             list of running coroutines to the list of pending coroutines as necessary.
//             [Be careful with multiple calls to suspend() and resume() - successive 
//             calls to suspend are ignored so they do not nest.]
// See:        resume(), is_suspended()
// Author(s):   Conan Reis
void SkInvokedCoroutine::suspend()
  {
  if ((m_flags & Flag_suspended) == 0u)
    {
    m_flags |= Flag_suspended;
    pending_increment();

    SkMind * updater_p = m_mind_p;

    if (updater_p)
      {
      updater_p->coroutine_track_pending(this);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Resumes a suspended coroutine - clears the suspended flag and decrements
//             the pending count.  It also moves it from the updater's list of pending
//             coroutines to the list of running coroutines if its pending count becomes 0.
//             [Be careful with multiple calls to suspend() and resume() - successive 
//             calls to suspend are ignored so they do not nest.]
// Author(s):   Conan Reis
void SkInvokedCoroutine::resume()
  {
  if (m_flags & Flag_suspended)
    {
    m_flags &= ~Flag_suspended;
    m_pending_count--;

    SkMind * updater_p = m_mind_p;

    if (updater_p)
      {
      updater_p->coroutine_track_updating(this);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Called every SkMind::on_update()
// 
// Returns:
//   Boolean indicating whether the coroutine has completed.  false if the invoked
//   coroutine should be called again in the next update, true if the coroutine has
//   completed and it should be removed from the mind's invoked coroutines list.
//   
// Notes:
//   Adds or removes this invoked coroutine from its mind's update list as needed.
bool SkInvokedCoroutine::on_update()
  {
  // Determine if scheduled return or updater/scope is gone

  // $Revisit - CReis The updater might always be non-nullptr so tests for nullptr could be redundant
  SkMind *        updater_p = m_mind_p;
  SkInvokedBase * caller_p  = m_caller_p;

  if ((updater_p == nullptr) || (m_update_count == SkInvokedCoroutine_scheduled_return) || m_scope_p.is_null())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Scheduled return or updater/scope is gone
    bool completed = (m_coroutine_p != nullptr && m_scope_p.is_valid());

    if (updater_p)
      {
      updater_p->coroutine_track_stop(this);
      }

    // Bind any return arguments
    if (m_return_args_p && !m_return_args_p->is_empty())
      {
      data_bind_return_args(*m_return_args_p, m_coroutine_p->get_params());
      }

    if (caller_p)
      {
      caller_p->pending_unregister(this, completed);
      }

    pool_delete(this);

    return completed;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Regular update
  if (m_update_next <= SkookumScript::get_sim_time())
    {
    // Do one update pass of the coroutine - determine if it is completed this frame

    SKDEBUG_STORE_CALL(this);

    #if (SKOOKUM & SK_DEBUG)
      if (updater_p->is_tracing())
        {
        SkDebug::print_agog(a_str_format("'%s' running coroutine '%s'", updater_p->as_string_debug().as_cstr(), m_coroutine_p->as_string_name(false).as_cstr()));
        }
    #endif

    m_update_next += m_update_interval;

    // $Revisit - CReis The check for m_coroutine_p == nullptr should not be necessary
    bool completed = m_coroutine_p && m_coroutine_p->on_update(this);

    SKDEBUG_RESTORE_CALL();

    if (completed)
      {
      // Coroutine successfully completed
      uint32_t update_count = m_update_count;

      // Remove it from the list if it is in the list
      updater_p->coroutine_track_stop(this);

      // Bind any return arguments
      if (m_return_args_p && !m_return_args_p->is_empty())
        {
        data_bind_return_args(*m_return_args_p, m_coroutine_p->get_params());
        }

      // If this is not the first time it was called return to caller if one is waiting
      if (update_count && (caller_p && caller_p->pending_count()))
        {
        caller_p->pending_unregister(this);
        }

      // Free this invoked coroutine
      pool_delete(this);

      return true;
      }

    // Coroutine did not complete this frame

    m_update_count++;
    }

  updater_p->coroutine_track(this);

  return false;
  }

//---------------------------------------------------------------------------------------
// Reset attributes of the invoked coroutine.
//
// #Author(s) Conan Reis
void SkInvokedCoroutine::reset(
  // Specifies how often the coroutine should be updated
  // - also see SkCall_interval_always and SkCall_interval_never
  f32 update_interval,
  // The invoked custom coroutine or method that invoked this SkInvokedCoroutine.
  // nullptr if there is nothing to return to.
  SkInvokedBase * caller_p,
  // Path for data look-ups (the owner).  nullptr if this object is at the topmost scope.
  SkInstance * scope_p,
  // Mind that will do the actual updating/tracking of the coroutine - generally same
  // updater as the caller.  If nullptr the caller's updater is used and if the caller is
  // nullptr scope_p is used.
  SkMind * updater_p, // = nullptr
  // Variable identifiers to bind any return argument results to or nullptr if no return
  // arguments used.
  const APArrayBase<SkIdentifierLocal> * rargs_p // = nullptr
  )
  {
  // Determine updater
  if (updater_p == nullptr)
    {
    // If scope is a Mind, always use that no matter what
    if (scope_p->is_mind())
      {
      updater_p = static_cast<SkMind *>(scope_p);
      }
    else
      {
      // Else crawl up the stack and find an updater
      if (caller_p && caller_p->is_valid_id())
        {
        updater_p = caller_p->get_updater();
        }

      // If there is no caller updater use the master mind object.
      if (updater_p == nullptr)
        {
        // $Revisit - CReis In future could use its class - have all `SkMetaClass` objects
        // be derived from `SkMind`.
        updater_p = SkookumScript::get_master_mind();
        }
      }
    }

  scope_p->reference();  // Ensure that receiver is around for lifetime of coroutine call.

  m_update_count     = 0u;
  m_pending_count    = 0u;
  m_flags            = Flag__default;
  m_update_interval  = update_interval;
  m_caller_p         = caller_p;
  m_scope_p          = scope_p;
  m_return_args_p    = rargs_p;
  m_mind_p           = updater_p;
  m_ptr_id           = ++ms_ptr_id_prev;
  m_update_next      = SkookumScript::get_sim_time();
  }


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Returns a AString representation of itself for debugging purposes
// Returns:    ADebug AString
// See:        get_scope_context()
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
AString SkInvokedCoroutine::as_string_debug() const
  {
  AString debug_str;

  debug_str.append("InvokedCoroutine ", 17u);
  debug_str.append(m_coroutine_p->as_string_name());
  debug_str.append("  this = ", 9u);

  SkInstance * scope_top_p = m_scope_p->get_topmost_scope();

  if (scope_top_p)
    {
    scope_top_p->as_code_append(&debug_str, SkCodeFlag__default, const_cast<SkInvokedCoroutine *>(this));
    }
  else
    {
    debug_str.append("[scope lost]", 12u);
    }

  // $Revisit - CReis [A_NOTE] ***Enhancement*** - Add more description to invoked coroutine (like call stack).

  return debug_str;
  }

#endif  // SK_AS_STRINGS


//---------------------------------------------------------------------------------------
// Returns type of member that it is
// Returns:    member type
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
eSkMember SkInvokedCoroutine::get_invoke_type() const
  {
  return eSkMember(m_coroutine_p->get_invoke_type());
  }

//---------------------------------------------------------------------------------------
// Returns wrapped invokable
// Returns:    wrapped invokable
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
SkInvokableBase * SkInvokedCoroutine::get_invokable() const
  {
  return const_cast<SkCoroutineBase *>(m_coroutine_p);
  }

//---------------------------------------------------------------------------------------
// Gets the mind that is updating/owning this invoked object.
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Conan Reis
SkMind * SkInvokedCoroutine::get_updater() const
  {
  SK_ASSERTX(m_mind_p, "An invoked coroutine should always have a valid Mind.");
  return m_mind_p;
  }

//---------------------------------------------------------------------------------------
//  Registers the atomic classes, methods, etc.
// Notes:       This method is called by Brain::initialize_post_load()
// Modifiers:    static
// Author(s):    Conan Reis
void SkInvokedCoroutine::register_bindings()
  {
  SkBrain::ms_invoked_coroutine_class_p->register_method_func(ASymbol_String, mthd_String);
  }

//---------------------------------------------------------------------------------------
// Skoo Params InvokedBase@String() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkInvokedCoroutine::mthd_String(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInvokedCoroutine * icoro_p = scope_p->this_as_data<SkInvokedCoroutine>();

    #if (SKOOKUM & SK_DEBUG)
      *result_pp = SkString::new_instance(icoro_p ? icoro_p->as_string_debug() : "InvokedCoroutine[stale]");
    #else
      *result_pp = SkString::new_instance(icoro_p ? "InvokedCoroutine" : "InvokedCoroutine[stale]");
    #endif
    }
  }

#ifdef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkInvokedCoroutines
AObjReusePool<SkInvokedCoroutine> & SkInvokedCoroutine::get_pool()
  {
  return ms_pool;
  }

#endif
