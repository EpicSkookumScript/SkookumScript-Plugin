// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for simplest type of object in language - instance of a
//             class without data members
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AObjReusePool.hpp>

//=======================================================================================
// SkDataInstance Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
A_INLINE SkDataInstance::SkDataInstance()
  {
  // Mark this instance as a data instance - allows for sanity checking down the road
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    m_user_data.m_data.m_ptr[1] = ms_magic_marker;
  #endif
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         class_p - Class type for this instance
// Author(s):   Conan Reis
A_INLINE SkDataInstance::SkDataInstance(
  SkClass * class_p // = nullptr
  ) :
  SkInstance(class_p)
  {
  add_data_members();

  // Mark this instance as a data instance - allows for sanity checking down the road
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    m_user_data.m_data.m_ptr[1] = ms_magic_marker;
  #endif
  }

//---------------------------------------------------------------------------------------

A_INLINE SkInstance * SkDataInstance::get_data_by_idx(uint32_t data_idx) const
  {
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    if (m_user_data.m_data.m_ptr[1] != ms_magic_marker)
      {
      return *on_magic_marker_mismatch(data_idx);
      }
  #endif

  return m_data[data_idx];
  }

//---------------------------------------------------------------------------------------

A_INLINE SkInstance ** SkDataInstance::get_data_addr_by_idx(uint32_t data_idx) const
  {
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    if (m_user_data.m_data.m_ptr[1] != ms_magic_marker)
      {
      return on_magic_marker_mismatch(data_idx);
      }
  #endif

  return m_data.get_array() + data_idx;
  }

//---------------------------------------------------------------------------------------
// 
A_INLINE void SkDataInstance::set_data_by_idx(uint32_t data_idx, SkInstance * obj_p)
  {
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    if (m_user_data.m_data.m_ptr[1] != ms_magic_marker)
      {
      on_magic_marker_mismatch(data_idx);
      return;
      }
  #endif

  m_data.set_at(data_idx, *obj_p);
  }

//---------------------------------------------------------------------------------------
//  Retrieves an instance object from the dynamic pool and initializes it for
//              use.  This method should be used instead of 'new' because it prevents
//              unnecessary allocations by reusing previously allocated objects.
// Returns:     a dynamic SkInstance
// See:         pool_delete() 
// Notes:       To 'deallocate' an object that was retrieved with this method, use
//              'pool_delete()' rather than 'delete'.
// Modifiers:    static
// Author(s):    Conan Reis
A_INLINE SkDataInstance * SkDataInstance::new_instance(SkClass * class_p)
  {
  SkDataInstance * instance_p = get_pool().allocate();

  instance_p->m_class_p = class_p;
  instance_p->m_ref_count = 1u;
  instance_p->m_ptr_id = ++ms_ptr_id_prev;
  instance_p->add_data_members();

  // Mark this instance as a data instance - allows for sanity checking down the road
  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    instance_p->m_user_data.m_data.m_ptr[1] = ms_magic_marker;
  #endif

  return instance_p;
  }

