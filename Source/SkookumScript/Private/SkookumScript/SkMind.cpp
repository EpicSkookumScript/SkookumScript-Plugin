// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Mind object - tracks and updates coroutines.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkMind.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkMind.inl>
#endif

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Local Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Stored in the meta class for a particular type of Mind
struct SkUserDataMind
  {
  // The _one_ singleton instance for this class of mind. Valid only when `m_count` is 1.
  SkMind * m_instance_p;

  // The instance count for objects of this class of mind.
  uint32_t m_count;
  };

//---------------------------------------------------------------------------------------
// Storage specialization - SkUserDataMind stored in SkUserData
template<> inline SkUserDataMind * SkUserDataBase::as<SkUserDataMind>() const         { return as_stored<SkUserDataMind>(); }
template<> inline void             SkUserDataBase::set(const SkUserDataMind & value)  { *as_stored<SkUserDataMind>() = value; }


//=======================================================================================
// Class Data Members
//=======================================================================================

AList<SkMind>  SkMind::ms_minds_to_update;
AList<SkMind>  SkMind::ms_minds_updating;
AList<SkMind>  SkMind::ms_minds_no_update;

AList<SkInvokedCoroutine> SkMind::ms_icoroutines_updating;


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// 
// Note:
//   It starts with `Flag_updatable` set so that it will update any coroutines that run it.
SkMind::SkMind(
  SkClass * class_p // = nullptr
  ) :
  SkDataInstance(class_p ? class_p : SkBrain::ms_mind_class_p),
  m_mind_flags(Flag__default)
  {
  // Increment instance count and set singleton/latest mind in class for reference
  SkUserDataMind * mind_instances_p;
  SkClass *        mind_class_p  = m_class_p;
  SkClass *        mind_parent_p = SkBrain::ms_mind_class_p->get_superclass();

  do
    {
    mind_instances_p = mind_class_p->get_metaclass().as_data<SkUserDataMind>();
    mind_instances_p->m_instance_p = this;  // `m_instance_p` only valid when `m_count == 1`.
    mind_instances_p->m_count++;
    mind_class_p = mind_class_p->get_superclass();
    }
  while (mind_class_p != mind_parent_p);
  }

//---------------------------------------------------------------------------------------
// Destructor
SkMind::~SkMind()
  {
  abort_coroutines();
  enable_updatable(false);

  // Increment instance count and set singleton/latest mind in class for reference
  SkUserDataMind * mind_instances_p;
  SkClass *        mind_class_p  = m_class_p;
  SkClass *        mind_parent_p = SkBrain::ms_mind_class_p->get_superclass();

  do
    {
    mind_instances_p = mind_class_p->get_metaclass().as_data<SkUserDataMind>();
    mind_instances_p->m_count--;

    if (mind_instances_p->m_count == 1u)
      {
      // Find the _one_ singleton
      mind_instances_p->m_instance_p = find_by_class(*mind_class_p);
      }

    mind_class_p = mind_class_p->get_superclass();
    }
  while (mind_class_p != mind_parent_p);
  }

//---------------------------------------------------------------------------------------
// Specifies whether the mind will update any coroutines it is tracking as needed
// otherwise it is dormant and its coroutines are tracked though they are not updated.
// 
// Params:
//   updatable:
//     specifies whether the mind's coroutines should be updated (true) or not (false).
//     
// See: on_update(), enable_on_update()
void SkMind::enable_updatable(
  bool updatable // = true
  )
  {
  if (updatable == is_updatable())
    {
    return;
    }

  if (updatable)
    {
    // $HACK - CReis Test for adding deleted minds
    if (!is_valid_id())
      {
      //A_DPRINT("Tried to enable behaviour of previously deleted mind '%s'!\n", get_name_cstr_dbg());
      return;
      }

    // Flag set after delete test
    m_mind_flags |= Flag_updatable;

    if (is_update_needed())
      {
      enable_on_update();
      }
    }
  else  // Disable
    {
    m_mind_flags &= ~Flag_updatable;

    // Remove from update list even if active
    enable_on_update(false);
    }
  }

//---------------------------------------------------------------------------------------
// Activates or deactivates the mind's on_update() method
// 
// Params:
//   activate:
//     specifies whether the mind's on_update() method should be activated (true) or
//     deactivated (false).
//     
// See:   on_update(), enable_updatable()
// Notes: Note needs the name for insert position in update list
void SkMind::enable_on_update(
  bool activate // = true
  )
  {
  if (activate == ((m_mind_flags & Flag_on_update_list) != 0u))
    {
    return;
    }

  if (activate)
    {
    // $HACK - CReis Ensure not deleted and behaving
    if (!is_valid_id() || ((m_mind_flags & Flag_updatable) == 0u))
      {
      //A_DPRINT("Tried to add previously deleted mind '%s' to update list!\n", get_name_cstr_dbg());
      return;
      }

    m_mind_flags |= Flag_on_update_list;

    bool signal_b = ms_minds_to_update.is_empty();

    remove();  // Remove from whichever list it is in.
    ms_minds_to_update.append(this);

    // Signal that updates are required if this is the first mind added to list
    if (signal_b)
      {
      SkookumScript::update_request();
      }
    }
  else  // Disable
    {
    m_mind_flags &= ~Flag_on_update_list;

    remove();                         // Remove from whichever list it is in.
    ms_minds_no_update.append(this);  // Continue to track it

    // Signal that updates are not required if this is the last mind removed from list
    if (ms_minds_to_update.is_empty() && ms_minds_updating.is_empty())
      {
      SkookumScript::update_request(false);
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Frees up a mind
// Examples:    called by dereference()
// See:         dereference()
// Modifiers:    virtual
// Author(s):    Conan Reis
void SkMind::delete_this()
  {
  m_ptr_id = AIdPtr_null;

  // minds are not pooled by default
  delete this;
  }

//---------------------------------------------------------------------------------------
// Returns a string representation of itself for debugging purposes
// 
// Returns: Debug string
// Modifiers: virtual - overridden from SkInstance
AString SkMind::as_string() const
  {
  AString class_name(m_class_p->get_name_str_dbg());
  AString str(nullptr, 2u + class_name.get_length(), 0u);

  str.append('\'');
  str.append(class_name);
  str.append('\'');

  return str;
  }

//---------------------------------------------------------------------------------------
// Appends invoked coroutine to this mind's active coroutine list as its first track
// action - i.e. it is not currently otherwise being tracked.
// 
// Author(s):   Conan Reis
void SkMind::coroutine_track_init(SkInvokedCoroutine * icoro_p)
  {
  m_icoroutines_to_update.append(icoro_p);

  icoro_p->m_flags |= SkInvokedCoroutine::Flag_tracked_updating;

  // Ensure that it is in the update list
  if (!(m_mind_flags & Flag_on_update_list))
    {
    enable_on_update();
    }
  }

//---------------------------------------------------------------------------------------
// Appends invoked coroutine to this mind's active coroutine list.
// Author(s):   Conan Reis
void SkMind::coroutine_track_updating(SkInvokedCoroutine * icoro_p)
  {
  if ((icoro_p->m_flags & SkInvokedCoroutine::Flag_tracked_updating) == 0u)
    {
    icoro_p->AListNode<SkInvokedCoroutine>::remove();
    m_icoroutines_to_update.append(icoro_p);

    icoro_p->m_flags &= ~SkInvokedCoroutine::Flag_tracked_pending;
    icoro_p->m_flags |= SkInvokedCoroutine::Flag_tracked_updating;

    // Ensure that it is in the update list
    if (!(m_mind_flags & Flag_on_update_list))
      {
      enable_on_update();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Puts invoked coroutine on its pending coroutine list.
// Author(s):   Conan Reis
void SkMind::coroutine_track_pending(SkInvokedCoroutine * icoro_p)
  {
  if ((icoro_p->m_flags & SkInvokedCoroutine::Flag_tracked_pending) == 0u)
    {
    icoro_p->AListNode<SkInvokedCoroutine>::remove();
    m_icoroutines_pending.append(icoro_p);

    icoro_p->m_flags &= ~SkInvokedCoroutine::Flag_tracked_updating;
    icoro_p->m_flags |= SkInvokedCoroutine::Flag_tracked_pending;
    }
  }

//---------------------------------------------------------------------------------------
// Abort and pool_delete given invoked coroutine
// Author(s):   Markus Breyer
void SkMind::coroutine_track_abort(SkInvokedCoroutine * icoro_p, eSkNotify notify_caller)
  {
  // $Revisit - CReis Should need to test if valid - track down and try to remove it.
  // Ignore invoked objects that have already in the free list
  if (icoro_p->is_valid_id())
    {
    icoro_p->m_flags &= ~SkInvokedCoroutine::Flag_tracked_mask;

    // Similar contents to SkInvokedCoroutine::abort_invoke()

    // Ensure coroutine is no longer pending on anything.
    icoro_p->pending_set(0u);

    // Note that if `SkNotify_ignore` used then invoked expressions may not be 
    // properly cleaned up.
    icoro_p->abort_subcalls(notify_caller);
    SkInvokedCoroutine::pool_delete(icoro_p);
    }
  }

//---------------------------------------------------------------------------------------
// Has this mind track the specified invoked coroutine
// 
// Author(s): Conan Reis
void SkMind::coroutine_track(SkInvokedCoroutine * icoro_p)
  {
  if (icoro_p->m_pending_count == 0u)
    {
    coroutine_track_updating(icoro_p);
    }
  else
    {
    coroutine_track_pending(icoro_p);
    }
  }

//---------------------------------------------------------------------------------------
// Remove invoked coroutines
// 
// Notes:     virtual - overridden from SkInstance.
// Author(s): Conan Reis
void SkMind::abort_coroutines(eSkNotify notify_caller)
  {
  // Put all coroutines in m_icoroutines_to_update
  m_icoroutines_to_update.append_take(&m_icoroutines_pending);

  if (m_mind_flags & Flag_updating)
    {
    m_icoroutines_to_update.append_take(&ms_icoroutines_updating);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clear coroutines
  for (const SkInvokedCoroutine * sentinel_p = m_icoroutines_to_update.get_sentinel(); sentinel_p->AListNode<SkInvokedCoroutine>::is_in_list(); )
    {
    SkInvokedCoroutine * icoro_p = sentinel_p->AListNode<SkInvokedCoroutine>::get_next();
    coroutine_track_abort(icoro_p, notify_caller);
    icoro_p->AListNode<SkInvokedCoroutine>::remove();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove from the update list [see is_update_needed()]
  enable_on_update(false);
  }

//---------------------------------------------------------------------------------------
// Clear all coroutines that use the given instance as their scope
void SkMind::abort_coroutines_on_object(SkObjectBase * object_p, eSkNotify notify_caller)
  {
  SkInvokedCoroutine * sentinel_p;
  SkInvokedCoroutine * icoro_next_p;

  // Ensure that if it is fully dereferenced within the loop that it doesn't get deleted
  // until the end of this call.
  reference();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look through pending coroutines for coroutines to abort
  sentinel_p = const_cast<SkInvokedCoroutine *>(m_icoroutines_pending.get_sentinel());
  for (SkInvokedCoroutine * icoro_p = sentinel_p->AListNode<SkInvokedCoroutine>::get_next(); icoro_p != sentinel_p; icoro_p = icoro_next_p)
    {
    icoro_next_p = icoro_p->AListNode<SkInvokedCoroutine>::get_next();
    if (icoro_p->get_scope() == object_p)
      {
      coroutine_track_abort(icoro_p, notify_caller);
      icoro_p->AListNode<SkInvokedCoroutine>::remove();
      // Since sub calls might also have been aborted, icoro_next_p might no longer be valid
      // If so, start over at beginning of list
      if (!icoro_next_p->AListNode<SkInvokedCoroutine>::is_in_list())
        {
        icoro_next_p = sentinel_p->AListNode<SkInvokedCoroutine>::get_next();
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look through coroutines to update for coroutines to abort
  sentinel_p = const_cast<SkInvokedCoroutine *>(m_icoroutines_to_update.get_sentinel());
  for (SkInvokedCoroutine * icoro_p = sentinel_p->AListNode<SkInvokedCoroutine>::get_next(); icoro_p != sentinel_p; icoro_p = icoro_next_p)
    {
    icoro_next_p = icoro_p->AListNode<SkInvokedCoroutine>::get_next();
    if (icoro_p->get_scope() == object_p)
      {
      coroutine_track_abort(icoro_p, SkNotify_ignore);
      icoro_p->AListNode<SkInvokedCoroutine>::remove();
      // Since sub calls might also have been aborted, icoro_next_p might no longer be valid
      // If so, start over at beginning of list
      if (!icoro_next_p->AListNode<SkInvokedCoroutine>::is_in_list())
        {
        icoro_next_p = sentinel_p->AListNode<SkInvokedCoroutine>::get_next();
        }
      }
    }

  // Matched with reference() call above.
  dereference();
  }

//---------------------------------------------------------------------------------------
// Suspends all currently tracked coroutines from updating.
// See:        resume_coroutines()
// Author(s):   Conan Reis
void SkMind::suspend_coroutines()
  {
  m_icoroutines_pending.apply_method(&SkInvokedCoroutine::suspend);
  m_icoroutines_to_update.apply_method(&SkInvokedCoroutine::suspend);
  }

//---------------------------------------------------------------------------------------
// Resumes all previously suspended coroutines to their previous state - which
//             might run the next update or be waiting on other scripts.
// See:        resume_coroutines()
// Author(s):   Conan Reis
void SkMind::resume_coroutines()
  {
  m_icoroutines_pending.apply_method(&SkInvokedCoroutine::resume);
  }


#if (SKOOKUM & SK_DEBUG)
  static uint32_t g_mind_search_id = 0x0;  // Replace with desired name id
#endif

//---------------------------------------------------------------------------------------
// Called every SkMind::update_all()
// 
// Notes:
//   This method can be overridden by a subclass, but the subclass should call its
//   parent's version of this method.
//   
// See:       update_all(), enable_on_update()
// Modifiers: virtual
// Author(s): Conan Reis
void SkMind::on_update()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that it is behaving and updating
  if ((m_mind_flags & Flag_updatable) != Flag_updatable)
    {
    return;
    }

  #if (SKOOKUM & SK_DEBUG)
    // Hacky yet simple mechanism to stop on a particular mind on each update
    if (g_mind_search_id && (get_name().get_id() == g_mind_search_id))
      {
      ADebug::print_format("Updating mind: %s\n", m_class_p->get_name_cstr_dbg());
      }
  #endif


  // Ensure that if it is fully dereferenced within the update that it doesn't get deleted
  // until the end of the update.
  reference();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through current snapshot of invoked coroutines.
  if (m_icoroutines_to_update.is_filled() || ms_icoroutines_updating.is_filled())
    {
    m_mind_flags |= Flag_updating;

    // Copy invoked coroutines to update this round - coroutines can still be be removed
    // New invoked coroutines are updated the next round.
    ms_icoroutines_updating.append_take(&m_icoroutines_to_update);

    SkInvokedCoroutine * icoro_p;

    do
      {
      icoro_p = ms_icoroutines_updating.pop_first();
      m_icoroutines_to_update.append(icoro_p);
      icoro_p->on_update();
      }
    while (ms_icoroutines_updating.is_filled());

    m_mind_flags &= ~Flag_updating;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that it is in the update list only if it needs to be [see is_update_needed()]
  // $Note - CReis This could be called whenever a coroutine is removed from
  // m_icoroutines_to_update, but while correct, it is inefficient since enable_on_update()
  // ends up being called too frequently.
  if (m_icoroutines_to_update.is_empty())
    {
    enable_on_update(false);
    }

  // Matched with reference() call above.
  // $Revisit - CReis This won't be called during the unwinding of an exception.
  dereference();
  }

//---------------------------------------------------------------------------------------
// Updates all the updating minds
// See:        enable_on_update()
// Modifiers:   static
// Author(s):   Conan Reis
void SkMind::update_all()
  {
  SkookumScript::enable_flag(SkookumScript::Flag_updating);

  // Append and transfer minds to update this round - minds can still be be removed using
  // `enable_on_update()`. New minds are updated the next round.
  // If there  is an error, minds that are yet to be updated will stay in the
  // `ms_minds_updating` list until the next call to `update_all()`
  ms_minds_updating.append_take(&ms_minds_to_update);

  SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_origin_actor_update);

  SkMind * mind_p;

  while (ms_minds_updating.is_filled())
    {
    mind_p = ms_minds_updating.pop_first();
    ms_minds_to_update.append(mind_p);
    mind_p->on_update();
    }

  SKDEBUG_HOOK_SCRIPT_EXIT();

  SkookumScript::enable_flag(SkookumScript::Flag_updating, false);
  }

//---------------------------------------------------------------------------------------
void SkMind::abort_all_coroutines()
  {
  // Purge to update list
  while (!ms_minds_to_update.is_empty())
    {
    SkMind * mind_p = ms_minds_to_update.pop_first();
    mind_p->abort_coroutines();
    }

  // Purge updating list
  while (!ms_minds_updating.is_empty())
    {
    SkMind * mind_p = ms_minds_updating.pop_first();
    mind_p->abort_coroutines();
    }

  // Purge not to update list
  while (!ms_minds_no_update.is_empty())
    {
    SkMind * mind_p = ms_minds_no_update.pop_first();
    mind_p->abort_coroutines();
    }
  }

//---------------------------------------------------------------------------------------
void SkMind::abort_all_coroutines_on_object(SkObjectBase * object_p, eSkNotify notify_caller)
  {
  SkMind * next_mind_p;

  // Process to update list  
  for (SkMind * mind_p = ms_minds_to_update.get_first(); mind_p != ms_minds_to_update.get_sentinel(); mind_p = next_mind_p)
    {
    next_mind_p = mind_p->get_next();
    mind_p->abort_coroutines_on_object(object_p, notify_caller);
    }

  // Process updating list
  for (SkMind * mind_p = ms_minds_updating.get_first(); mind_p != ms_minds_updating.get_sentinel(); mind_p = next_mind_p)
    {
    next_mind_p = mind_p->get_next();
    mind_p->abort_coroutines_on_object(object_p, notify_caller);
    }

  // Process not to update list
  for (SkMind * mind_p = ms_minds_no_update.get_first(); mind_p != ms_minds_no_update.get_sentinel(); mind_p = next_mind_p)
    {
    next_mind_p = mind_p->get_next();
    mind_p->abort_coroutines_on_object(object_p, notify_caller);
    }
  }

//---------------------------------------------------------------------------------------
SkMind * SkMind::find_by_class(const SkClass & mind_class)
  {
  // Look in to update list
  for (SkMind * mind_p = ms_minds_to_update.get_first(); mind_p != ms_minds_to_update.get_sentinel(); mind_p = mind_p->get_next())
    {
    if (mind_p->get_class()->is_class(mind_class))
      {
      return mind_p;
      }
    }

  // Look in updating list
  for (SkMind * mind_p = ms_minds_updating.get_first(); mind_p != ms_minds_updating.get_sentinel(); mind_p = mind_p->get_next())
    {
    if (mind_p->get_class()->is_class(mind_class))
      {
      return mind_p;
      }
    }

  // Look in not to update list
  for (SkMind * mind_p = ms_minds_no_update.get_first(); mind_p != ms_minds_no_update.get_sentinel(); mind_p = mind_p->get_next())
    {
    if (mind_p->get_class()->is_class(mind_class))
      {
      return mind_p;
      }
    }

  // No such mind found
  return nullptr;
  }


namespace SkMind_Impl
  {

  //---------------------------------------------------------------------------------------
  // Sk Params !()
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkMind * this_p = &scope_p->this_as<SkMind>();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params =(Mind operand) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() == scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params ~=(Mind operand) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() != scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params name() Symbol
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_name(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkSymbol::new_instance(scope_p->this_as<SkMind>().get_name());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params abort_coroutines()
  static void mthd_abort_coroutines(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkMind * mind_p = &scope_p->this_as<SkMind>();
    bool is_success = scope_p->get_arg<SkBoolean>(SkArg_1);
    mind_p->abort_coroutines(is_success ? SkNotify_success : SkNotify_fail);
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Mind@String() String
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkString::new_instance(scope_p->this_as<SkMind>().as_string());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Mind@object() ThisClass_
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instance(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkMetaClass *    mclass_p         = (SkMetaClass *)scope_p->get_topmost_scope();
      SkUserDataMind * mind_instances_p = mclass_p->as_data<SkUserDataMind>();

      #if (SKOOKUM & SK_DEBUG)
        if (mind_instances_p->m_count != 1u)
          {
          A_ERRORX(a_str_format(
            "Tried to get mind singleton object of class '%s', but it has %u instances!\n",
            mclass_p->get_class_info()->get_name_cstr_dbg(),
            mind_instances_p->m_count));

          *result_pp = SkBrain::ms_nil_p;

          return;
          }
      #endif

      mind_instances_p->m_instance_p->reference();
      *result_pp = mind_instances_p->m_instance_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Mind@object_or_nil() <ThisClass_|None>
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instance_or_nil(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkMetaClass *    mclass_p         = (SkMetaClass *)scope_p->get_topmost_scope();
      SkUserDataMind * mind_instances_p = mclass_p->as_data<SkUserDataMind>();

      *result_pp = SkBrain::ms_nil_p;

      if (mind_instances_p->m_count == 1u)
        {
        mind_instances_p->m_instance_p->reference();
        *result_pp = mind_instances_p->m_instance_p;
        }
      #if (SKOOKUM & SK_DEBUG)
        else
          {
          if (mind_instances_p->m_count > 1u)
            {
            A_ERRORX(a_str_format(
              "Tried to get mind singleton object of class '%s', but it has several (%u) instances!\n",
              mclass_p->get_class_info()->get_name_cstr_dbg(),
              mind_instances_p->m_count));

            return;
            }
          }
      #endif
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Mind@object?() Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instanceQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkMetaClass *    mclass_p         = (SkMetaClass *)scope_p->get_topmost_scope();
      SkUserDataMind * mind_instances_p = mclass_p->as_data<SkUserDataMind>();

      *result_pp = SkBoolean::new_instance(mind_instances_p->m_count == 1u);
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Mind@instance_length() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instances_length(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkMetaClass *    mclass_p         = (SkMetaClass *)scope_p->get_topmost_scope();
      SkUserDataMind * mind_instances_p = mclass_p->as_data<SkUserDataMind>();

      *result_pp = SkInteger::new_instance(mind_instances_p->m_count);
      }
    }

  //---------------------------------------------------------------------------------------

  // Instance methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!",                mthd_ctor },
      { "String",           mthd_string },
      { "equal?",           mthd_op_equals },
      { "not_equal?",       mthd_op_not_equal },
      { "name",             mthd_name },
      { "abort_coroutines", mthd_abort_coroutines },
    };

  // Class methods
  static const SkClass::MethodInitializerFunc methods_c[] =
    {
      { "instance",         mthdc_instance },
      { "instance_or_nil",  mthdc_instance_or_nil },
      { "instance?",        mthdc_instanceQ },
      { "instances_length", mthdc_instances_length },
    };

  } // SkMind_Impl namespace


//---------------------------------------------------------------------------------------
// Registers the atomic classes, coroutines, etc.
// Examples:   This method is called by SkAtomic::register_atomic()
// Modifiers:   static
// Author(s):   Conan Reis
void SkMind::register_bindings()
  {
  initialize_class("Mind");

  ms_class_p->register_method_func_bulk(SkMind_Impl::methods_i, A_COUNT_OF(SkMind_Impl::methods_i), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkMind_Impl::methods_c, A_COUNT_OF(SkMind_Impl::methods_c), SkBindFlag_class_no_rebind);
  }
