// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Base classes for executed/called/invoked objects inline file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AObjReusePool.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkExpressionBase.hpp>


//=======================================================================================
// SkInvokedBase Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Constructor
// Returns:     itself
// Arg          caller_p - Object that invoked / called this routine and now expects to
//              be returned to upon the completion of this routine.  nullptr if there is
//              nothing to return to.  (Default nullptr)
// Arg          scope_p - Path for data look-ups (the owner).  nullptr if this object is
//              at the topmost scope.  (Default nullptr)
// Author(s):    Conan Reis
A_INLINE SkInvokedBase::SkInvokedBase(
  SkInvokedBase * caller_p, // = nullptr
  SkObjectBase *  scope_p   // = nullptr
  ) :
  m_scope_p(scope_p),
  m_caller_p(caller_p),
  m_pending_count(0u)
  {
  m_ptr_id = ++ms_ptr_id_prev;
  }

//---------------------------------------------------------------------------------------
// virtual destructor
// Author(s):   Conan Reis
A_INLINE SkInvokedBase::~SkInvokedBase()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Gets the mind that is updating/owning this invoked object.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
A_INLINE SkMind * SkInvokedBase::get_updater() const
  {
  SkInvokedBase * caller_p = m_caller_p;

  return caller_p ? caller_p->get_updater() : nullptr;
  }

//---------------------------------------------------------------------------------------
// Called when an invoked sub-expression is deferred/did not complete
//             immediately and this invokable's completion is pending on the eventual
//             completion of the sub-expression.
// Arg         deferred_p - invoked sub-expression that was deferred
// Author(s):   Conan Reis
A_INLINE void SkInvokedBase::pending_deferred(SkInvokedBase * deferred_p)
  {
  m_calls.append(deferred_p);
  pending_increment();
  }

//---------------------------------------------------------------------------------------
// Called when an invoked sub-expression returns from a pending task and
//             notifies this invokable that it no longer needs to wait for it.
// Arg         deferred_p - invoked sub-expression that was deferred, but is now completed
// Arg         completed - true if sub-expression completed successfully, false if the
//             sub-expression was aborted/did not complete successfully - thus this
//             invokable should also probably clean up and quit.
// Author(s):   Conan Reis
A_INLINE void SkInvokedBase::pending_unregister(
  SkInvokedBase * deferred_p,
  bool            completed // = true
  )
  {
  // $Revisit - CReis Shouldn't need the in list test.
  if (deferred_p->is_in_list())
    {
    m_calls.remove(deferred_p);
    }
  //else
  //  {
  //  SK_DPRINTF("\nSkookum: Tried to remove invokeable that was already removed!\n");
  //  }

  pending_return(completed);
  }

//---------------------------------------------------------------------------------------
// Common portion of abort calls - aborts all sub-calls, abort this invoked
//             expression and optionally notify any caller that this has failed to complete.
// Arg         notify_caller - see eSkNotify
// Arg         notify_child - see eSkNotifyChild
// Modifiers:   virtual - overridden from SkInvokedBase
// Author(s):   Conan Reis
A_INLINE void SkInvokedBase::abort_common(
  eSkNotify      notify_caller,
  eSkNotifyChild notify_child
  )
  {
  // Abort/detach sub-calls

  // Ensure that pending count is not > 0 for these calls so that this invocation is
  // no longer pending on anything.
  m_pending_count = 0u;

  if (notify_child == SkNotifyChild_abort)
    {
    abort_subcalls(notify_caller);
    }
  else
    {
    detach_subcalls(notify_caller);
    }

  // Generally followed up with cleaning out any other context memory and putting this
  // invokable back in a pool or deleting it.
  // For example:
  //pool_delete(this);
  }


//=======================================================================================
// SkInvokedExpression Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Retrieves an invoked expression object from the dynamic pool and
//             initializes it for use.  This should be used instead of 'new' because it
//             prevents unnecessary allocations by reusing previously allocated objects.
// Returns:    a reusable dynamic SkInvokedExpression 
// Arg         expr - the expression that is invoked/running
// Arg         caller_p - the invoked custom coroutine or method that invoked this
//             SkookumScript/SkInvokedCoroutine.  nullptr if there is nothing to return to.
// Arg         scope_p - Path for data look-ups (the owner).  nullptr if this object is
//             at the topmost scope.
// See:        pool_delete()
// Notes:      To "deallocate" an object that was retrieved with this method, use
//             pool_delete() rather than 'delete'.
// Author(s):   Conan Reis
A_INLINE SkInvokedExpression * SkInvokedExpression::pool_new(
  const SkExpressionBase & expr,
  SkInvokedBase *          caller_p,
  SkObjectBase *           scope_p
  )
  {
  SkInvokedExpression * iexpr_p = get_pool().allocate();

  iexpr_p->m_expr_p        = &expr;
  iexpr_p->m_caller_p      = caller_p;
  iexpr_p->m_scope_p       = scope_p;
  iexpr_p->m_pending_count = 0u;
  iexpr_p->m_index         = 0u;
  iexpr_p->m_ptr_id        = ++ms_ptr_id_prev;

  return iexpr_p;
  }

//---------------------------------------------------------------------------------------
// Frees up an invoked expression and puts it into the dynamic pool ready for
//             its next use.  This should be used instead of 'delete' because it prevents
//             unnecessary deallocations by saving previously allocated objects.
// Arg         iexpr_p - pointer to invoked expression to free up and put into the dynamic
//             pool ready for its next use.
// Notes:      To "allocate" an object use 'pool_new' rather than 'new'.
// Examples:   SkInvokedExpression::pool_delete(iexpr_p);
// Author(s):   Conan Reis
A_INLINE void SkInvokedExpression::pool_delete(SkInvokedExpression * iexpr_p)
  {
  if (iexpr_p->is_valid_id())
    {
    iexpr_p->AListNode<SkInvokedBase>::remove();
    iexpr_p->m_ptr_id = AIdPtr_null;

    get_pool().recycle(iexpr_p);
    }
  //#if (SKOOKUM & SK_DEBUG)
  //else
  //  {
  //  ADebug::print("Tried to free an SkInvokedExpression that is already in the free pool!\n");
  //  }
  //#endif
  }


//---------------------------------------------------------------------------------------
// Gets the mind that is updating/owning this invoked object.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
A_INLINE SkMind * SkInvokedExpression::get_updater() const
  {
  return m_expr_p->get_updater(*this);
  }


//=======================================================================================
// SkInvokedContextBase Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE SkInstance * SkInvokedContextBase::get_this()
  {
  return static_cast<SkInstance *>(m_scope_p.get_obj());
  }

//---------------------------------------------------------------------------------------
// Adds the given temporary variable name so that it may be used to refer
//             to instances of objects making it part of the 'temporary variable stack'.
//             It is initially set to the 'nil' object.
// Arg         var_name - The identifier name of the temporary variable to append
// See:        data_remove_vars(), data_empty(), data_append_args() 
// Author(s):   Conan Reis
A_INLINE void SkInvokedContextBase::data_append_var()
  {
  // nil does not need to be referenced/dereferenced
  m_data.append(*SkBrain::ms_nil_p);
  }

//---------------------------------------------------------------------------------------
// Initializes a given number of variables to nil
// Arg         var_names - The identifier names of the temporary variables to append
// See:        data_remove_vars(), data_empty(), data_append_args() 
// Notes:      This is used by SkookumScript/SkCode.
// Author(s):   Conan Reis
A_INLINE void SkInvokedContextBase::data_create_vars(uint32_t start_idx, uint32_t count)
  {
  if (count)
    {
    // 'nil' will be used as an initial value for the temporary variables.
    // nil does not need to be referenced/dereferenced
    SkInstance * nil_p = SkBrain::ms_nil_p;
    SkInstance ** var_pp = m_data.get_array() + start_idx;
    SkInstance ** var_end_pp = var_pp + count;
    do
      {
      *var_pp++ = nil_p;
      } while (var_pp < var_end_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Destroys the given temporary variables = releases their references to the instances 
// that they refer to - thus removing them from the "temporary variable stack".
A_INLINE void SkInvokedContextBase::data_destroy_vars(uint32_t start_idx, uint32_t count)
  {
  if (count)
    {
    // Dereference variables
    SkInstance * nil_p = SkBrain::ms_nil_p;
    SkInstance ** var_pp = m_data.get_array() + start_idx;
    SkInstance ** var_end_pp = var_pp + count;
    do    
      {
      (*var_pp)->dereference();
      *var_pp++ = nil_p; // Set to nil as this slot might be re-used by another local variable
      } while (var_pp < var_end_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Destroys the given temporary variables = releases their references to the instances 
// that they refer to - thus removing them from the "temporary variable stack".
// Arg         count - The number of variables to be removed from the top of the "stack".
// Arg         delay_collect_p - if non-nullptr any variable associated with this data is
//             dereferenced without performing garbage collection until ensure_reference()
//             is called.
// See:        data_append_vars(), data_empty(), data_append_args() 
// Notes:      This is used by SkookumScript/SkCode.
// Author(s):   Conan Reis
A_INLINE void SkInvokedContextBase::data_destroy_vars(
  uint32_t     start_idx,
  uint32_t     count,
  SkInstance * delay_collect_p
  )
  {
  SK_ASSERTX(count > 0, "Caller must make sure we are not called to do nothing.");

  if (delay_collect_p)
    {
    // Ensure delay_collect_p has a reference so it is not garbage collected - usually a returned object
    delay_collect_p->reference();
    }

  data_destroy_vars(start_idx, count);

  if (delay_collect_p)
    {
    // Remove temporary reference from delay_collect_p, but do not garbage collect it
    delay_collect_p->dereference_delay();
    }
  }

//---------------------------------------------------------------------------------------
// 
A_INLINE void SkInvokedContextBase::data_empty()
  {
  data_destroy_vars(0, m_data.get_length());
  m_data.empty();
  }

//---------------------------------------------------------------------------------------
// Sets/binds *previously referenced* argument at specified position to the object
// specified.
//
// #See Also  set_arg_and_ref() and all the other set_*() and get_*() methods
// #Author(s) Conan Reis
A_INLINE void SkInvokedContextBase::set_arg(
  // 0-based index position of argument to set/bind - use values from eSkArgNum to make it
  // more readable.
  uint32_t pos,
  // Object to set/bind - assumes that it already has its reference count incremented
  SkInstance * obj_p
  )
  {
  SK_ASSERTX(pos < m_data.get_size(), "Access out of range");
  SkInstance ** elem_pp = m_data.get_array() + pos;
  (*elem_pp)->dereference();
  *elem_pp = obj_p;
  }

//---------------------------------------------------------------------------------------
// Sets/binds argument *and auto-references it* at specified position to the object
// specified.
//
// #See Also  set_arg() and all the other set_*() and get_*() methods
// #Author(s) Conan Reis
A_INLINE void SkInvokedContextBase::set_arg_and_ref(
  // 0-based index position of argument to set/bind - use values from eSkArgNum to make it
  // more readable.
  uint32_t pos,
  // Object to set/bind - assumes that it already has its reference count incremented
  SkInstance * obj_p
  )
  {
  SK_ASSERTX(pos < m_data.get_size(), "Access out of range");
  SkInstance ** elem_pp = m_data.get_array() + pos;
  obj_p->reference();
  (*elem_pp)->dereference(); // Deref after ref as it might refer to the same instance
  *elem_pp = obj_p;
  }

//---------------------------------------------------------------------------------------
// Sets/binds *previously referenced* argument at specified position to the object
// specified.
//
// #See Also  set_arg_and_ref() and all the other set_*() and get_*() methods
// #Author(s) Conan Reis
A_INLINE void SkInvokedContextBase::bind_arg(
  // 0-based index position of argument to set/bind - use values from eSkArgNum to make it
  // more readable.
  uint32_t pos,
  // Object to set/bind - assumes that it already has its reference count incremented
  SkInstance * obj_p
  )
  {
  SK_ASSERTX(pos < m_data.get_size(), "Access out of range");
  SkInstance ** elem_pp = m_data.get_array() + pos;
  (*elem_pp)->dereference();
  *elem_pp = obj_p;
  }

//---------------------------------------------------------------------------------------
// Sets/binds argument *and auto-references it* at specified position to the object
// specified.
//
// #See Also  set_arg() and all the other set_*() and get_*() methods
// #Author(s) Conan Reis
A_INLINE void SkInvokedContextBase::bind_arg_and_ref(
  // 0-based index position of argument to set/bind - use values from eSkArgNum to make it
  // more readable.
  uint32_t pos,
  // Object to set/bind - assumes that it already has its reference count incremented
  SkInstance * obj_p
  )
  {
  SK_ASSERTX(pos < m_data.get_size(), "Access out of range");
  SkInstance ** elem_pp = m_data.get_array() + pos;
  obj_p->reference();
  (*elem_pp)->dereference(); // Deref after ref as it might refer to the same instance
  *elem_pp = obj_p;
  }
