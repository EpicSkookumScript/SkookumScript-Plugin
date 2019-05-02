// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for executed/called/invoked coroutines include file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AObjReusePool.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkCoroutine.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Converts to a symbol representation of itself - its name
// Returns:    a symbol representation of itself - its name
// Notes:      Used with sorted arrays as a key
// Author(s):   Conan Reis
A_INLINE SkInvokedCoroutine::operator const ASymbol & () const
  {
  return m_coroutine_p->get_name();
  }

//---------------------------------------------------------------------------------------
// Equal to
// Returns:    true or false
// Arg         icoroutine - invoked coroutine to compare
// See:        <
// Author(s):   Conan Reis
A_INLINE bool SkInvokedCoroutine::operator==(const SkInvokedCoroutine & icoroutine) const
  {
  return m_coroutine_p == icoroutine.m_coroutine_p;
  }

//---------------------------------------------------------------------------------------
// Less than
// Returns:    true or false
// Arg         icoroutine - invoked coroutine to compare
// See:        ==
// Author(s):   Conan Reis
A_INLINE bool SkInvokedCoroutine::operator<(const SkInvokedCoroutine & icoroutine) const
  {
  return *m_coroutine_p < *icoroutine.m_coroutine_p;
  }

//---------------------------------------------------------------------------------------
// Removes this invoked coroutine from any updater
// Author(s):   Conan Reis
A_INLINE void SkInvokedCoroutine::stop_tracking()
  {
  if (m_flags & Flag_tracked_mask)
    {
    SkMind * mind_p = m_mind_p;

    if (mind_p)
      {
      mind_p->coroutine_track_stop(this);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Gets the name of this object
// Returns:    The symbol version of the name of this object
// See:        set_name()
// Author(s):   Conan Reis
A_INLINE const ASymbol & SkInvokedCoroutine::get_name() const
  {
  return m_coroutine_p->get_name();
  }

//---------------------------------------------------------------------------------------
// Gets the associated coroutine.
// Returns:    the associated coroutine
// Author(s):   Conan Reis
A_INLINE const SkCoroutineBase & SkInvokedCoroutine::get_coroutine() const
  {
  return *m_coroutine_p;
  }

//---------------------------------------------------------------------------------------
// Retrieves an invoked coroutine object from the dynamic pool and initializes it for use.
// This should be used instead of 'new' because it prevents unnecessary allocations by
// reusing previously allocated objects.
//
// #Notes
//   To "deallocate" an object that was retrieved with this method, use pool_delete()
//   rather than 'delete'.
//
// #Modifiers static
// #See Also  pool_delete(), alternate pool_new()
// #Author(s) Conan Reis
A_INLINE SkInvokedCoroutine * SkInvokedCoroutine::pool_new(const SkCoroutineBase * coroutine_p)
  {
  SkInvokedCoroutine * icoroutine_p = get_pool().allocate();

  icoroutine_p->m_coroutine_p = coroutine_p;

  uint32_t data_count = coroutine_p->get_invoked_data_array_size();
  // Check if we need more storage
  if (icoroutine_p->m_data.get_size() < data_count)
    {
    if (data_count <= Data_quick_storage_size)
      {
      icoroutine_p->m_data.set_custom_memory_empty_unsafe(icoroutine_p->m_data_quick_storage, Data_quick_storage_size);
      }
    else
      {
      // Remove quick storage if present so APArray will not try to free it to the heap
      if (icoroutine_p->m_data.get_array() == icoroutine_p->m_data_quick_storage)
        {
        icoroutine_p->m_data.set_custom_memory_empty_unsafe(nullptr, 0);
        }
      icoroutine_p->m_data.ensure_size(data_count);
      }
    }

  return icoroutine_p;
  }

//---------------------------------------------------------------------------------------
// Frees up an invoked coroutine and puts it into the dynamic pool ready for
//             its next use.  This should be used instead of 'delete' because it
//             prevents unnecessary deallocations by saving previously allocated
//             objects.
// Arg         icoroutine_p - pointer to invoked coroutine to free up and put into
//             the dynamic pool ready for its next use.
// Notes:      To "allocate" an object use 'pool_new' rather than 'new'.
// Examples:   SkInvokedCoroutine::pool_delete(icoroutine_p);
// Author(s):   Conan Reis
A_INLINE void SkInvokedCoroutine::pool_delete(SkInvokedCoroutine * icoroutine_p)
  {
  // $Revisit - CReis Track down when invoked coroutines are redundantly freed and see if it can be limited to once.
  // Ignore invoked coroutines that are already in the free list
  if (!icoroutine_p->is_valid_id())
    {
    return;
    }

  icoroutine_p->stop_tracking();       // $Revisit - CReis Probably redundant - likely to already have been removed from updater.
  icoroutine_p->data_empty();          // Dereference instances that live here, leave storage as-is
  icoroutine_p->m_calls.empty();       // $Revisit - Empty call list - might be redundant
  icoroutine_p->AListNode<SkInvokedBase>::remove();
  icoroutine_p->m_ptr_id = AIdPtr_null;

  SkObjectBase * scope_p = icoroutine_p->m_scope_p;

  if (scope_p)
    {
    static_cast<SkInstance *>(scope_p)->dereference();
    }

  get_pool().recycle(icoroutine_p);
  }

//---------------------------------------------------------------------------------------
//  Sets the coroutine to invoke/run
// Arg          coroutine - coroutine to invoke/run
// Author(s):    Conan Reis
A_INLINE void SkInvokedCoroutine::set_coroutine(const SkCoroutineBase & coroutine)
  {
  m_coroutine_p = &coroutine;
  }

