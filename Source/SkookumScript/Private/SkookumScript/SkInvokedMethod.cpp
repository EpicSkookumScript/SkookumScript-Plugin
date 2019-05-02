// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for executed/called/invoked methods definition file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvokedMethod.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvokedMethod.inl>
#endif

#if (SKOOKUM & SK_DEBUG)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// SkInvokedMethod Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Returns itself as an instance
// Modifiers:   virtual
// Author(s):   Conan Reis
SkInstance * SkInvokedMethod::as_new_instance() const
  {
  return SkInstance::new_instance(SkBrain::ms_invoked_method_class_p, const_cast<SkInvokedMethod *>(this));
  }

//---------------------------------------------------------------------------------------
// Abort all sub-calls, abort this invoked method and optionally notify any
//             caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Conan Reis
void SkInvokedMethod::abort_invoke(
  eSkNotify      notify_caller, // = SkNotify_fail
  eSkNotifyChild notify_child   // = SkNotifyChild_abort
  )
  {
  // This should never need to be called - methods do not call coroutines and should
  // therefore always return immediately.  This function is put here to meet the pure
  // requirement set by SkookumScript/SkInvokedBase.  The code that follows is *just in case.

  // Abort/detach sub-calls
  abort_common(notify_caller, notify_child);
  }

//---------------------------------------------------------------------------------------
// Called when a pending sub-coroutine completes/returns
// Arg         completed - true if coroutine completed successfully, false if coroutine was
//             aborted before it was complete.  (Default true)
// Modifiers:   virtual - overriding from SkInvokedBase
// Author(s):   Conan Reis
void SkInvokedMethod::pending_return(
  bool completed // = true
  )
  {
  // This should never need to be called - methods do not call coroutines and should
  // therefore always return immediately.  This function is put here to meet the pure
  // requirement set by SkookumScript/SkInvokedBase.
  }


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Returns a AString representation of itself for debugging purposes
// Returns:    ADebug AString
// See:        get_scope_context()
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
AString SkInvokedMethod::as_string_debug() const
  {
  AString debug_str;

  debug_str.append("InvokedMethod ", 14u);
  debug_str.append(m_method_p->as_string_name());
  debug_str.append("  this = ", 9u);

  SkInstance * scope_top_p = m_scope_p->get_topmost_scope();

  if (scope_top_p)
    {
    scope_top_p->as_code_append(&debug_str, SkCodeFlag__default, const_cast<SkInvokedMethod *>(this));
    }
  else
    {
    debug_str.append("[scope lost]", 12u);
    }

  // $Revisit - CReis [A_NOTE] ***Enhancement*** - Add more description to invoked method (call stack).

  return debug_str;
  }

#endif  // SK_AS_STRINGS


//---------------------------------------------------------------------------------------
// Returns type of member that it is
// Returns:    member type
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
eSkMember SkInvokedMethod::get_invoke_type() const
  {
  return eSkMember(m_method_p->get_invoke_type());
  }

//---------------------------------------------------------------------------------------
// Returns wrapped invokable
// Returns:    wrapped invokable
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
SkInvokableBase * SkInvokedMethod::get_invokable() const
  {
  return (SkInvokableBase *)m_method_p;
  }

//---------------------------------------------------------------------------------------
// If the scope is a Mind return it, otherwise do the default.
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Markus Breyer
SkMind * SkInvokedMethod::get_updater() const
  {
  SkObjectBase * scope_p = m_scope_p;
  return scope_p->is_mind() 
    ? static_cast<SkMind *>(scope_p) 
    : SkInvokedBase::get_updater();
  }

//---------------------------------------------------------------------------------------
//  Registers the atomic classes, methods, etc.
// Notes:       This method is called by Brain::initialize_post_load()
// Modifiers:    static
// Author(s):    Conan Reis
void SkInvokedMethod::register_bindings()
  {
  SkBrain::ms_invoked_method_class_p->register_method_func(ASymbol_String,  mthd_String);
  }

//---------------------------------------------------------------------------------------
// Skoo Params InvokedBase@String() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkInvokedMethod::mthd_String(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInvokedMethod * imethod_p = scope_p->this_as_data<SkInvokedMethod>();

    #if (SKOOKUM & SK_DEBUG)
      *result_pp = SkString::new_instance(imethod_p ? imethod_p->as_string_debug() : "InvokedMethod[stale]");
    #else
      *result_pp = SkString::new_instance(imethod_p ? "InvokedMethod" : "InvokedMethod[stale]");
    #endif
    }
  }


//=======================================================================================
// SkInvokedDeferrableMethod Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
SkInvokedDeferrableMethod::SkInvokedDeferrableMethod(SkObjectBase * scope_p, SkMethodBase * method_p) : SkInvokedMethod(nullptr, scope_p, method_p)
  {
  }

//---------------------------------------------------------------------------------------
SkInvokedDeferrableMethod::~SkInvokedDeferrableMethod()
  {
  // Make sure heap memory is freed
  data_empty_compact();
  }

//---------------------------------------------------------------------------------------
// Abort all sub-calls, abort this invoked deferred method and optionally 
//             notify any caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Conan Reis
void SkInvokedDeferrableMethod::abort_invoke(
  eSkNotify      notify_caller, // = SkNotify_fail
  eSkNotifyChild notify_child   // = SkNotifyChild_abort
  )
  {
  // Abort/detach sub-calls
  abort_common(notify_caller, notify_child);

  ADebug::print_format("[Background Code aborted - invoked method 0x%p]\n", this);

  // Done its task, so delete it.
  delete m_method_p;
  delete this;
  }

//---------------------------------------------------------------------------------------
// Override to allow invoked methods to handle durational code to a limited
//             extent.
// Arg         completed - true if completed successfully, false if it was unable to
//             complete successfully.
// Modifiers:   virtual - overridden from SkInvokedMethod
// Author(s):   Conan Reis
void SkInvokedDeferrableMethod::pending_return(
  bool completed // = true
  )
  {
  m_pending_count--;

  if (m_pending_count == 0u)
    {
    ADebug::print_format(
      "[Background Code %s - invoked method 0x%p]\n",
      completed ? "completed" : "aborted",
      this);

    // Don't delete here as it might still be used by branch'ed code!
    // Will eventually get deleted via the global program update record
    //delete m_method_p;
    delete this;
    }
  }


//=======================================================================================
// SkIExternalMethodCallWrapper Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Abort all sub-calls, abort this invoked deferred method and optionally 
//             notify any caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Conan Reis
void SkIExternalMethodCallWrapper::abort_invoke(
  eSkNotify      notify_caller, // = SkNotify_fail
  eSkNotifyChild notify_child   // = SkNotifyChild_abort
  )
  {
  // Abort/detach sub-calls
  abort_common(notify_caller, notify_child);

  delete this;
  }

//---------------------------------------------------------------------------------------
// Override to allow invoked methods to handle durational code to a limited
//             extent.
// Arg         completed - true if completed successfully, false if it was unable to
//             complete successfully.
// Modifiers:   virtual - overridden from SkInvokedMethod
// Author(s):   Richard Orth, Conan Reis
void SkIExternalMethodCallWrapper::pending_return(
  bool completed // = true
  )
  {
  m_pending_count--;

  if (m_pending_count == 0u)
    {
    delete this;
    }
  }

