// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AStringRef class definition module
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/AStringRef.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/AStringRef.inl>
#endif
#include <AgogCore/AObjReusePool.hpp>
#include <AgogCore/AConstructDestruct.hpp>

//=======================================================================================
// Data Definitions
//=======================================================================================

AObjReusePool<AStringRef> AStringRef::ms_pool;

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Reuses this string reference if it can - otherwise retrieves a string reference object
// from the dynamic pool and initializes it for use.
// This should be used instead of 'new' because it prevents unnecessary allocations by
// reusing previously allocated objects.
// 
// Notes:  
//   To 'deallocate' an object that was retrieved with this method, use `dereference()`
//   or `pool_delete()` rather than `delete`.
//   
// Returns:     a dynamic AStringRef
// See:         pool_new_copy(), pool_delete(), reuse_or_new()
// Modifiers:   static
// Author(s):   Conan Reis
AStringRef * AStringRef::reuse_or_new(
  const char * cstr_p,
  uint32_t     length,
  uint32_t     size,
  bool         deallocate // = true
  )
  {
  // If unique
  if (m_ref_count == 1u)
    {
    if (m_deallocate)
      {
      AStringRef::free_buffer(m_cstr_p);
      }

    m_cstr_p     = const_cast<char *>(cstr_p);
    m_size       = size;
    m_length     = length;
    m_deallocate = deallocate;
    m_read_only  = false;

    return this;
    }

  // Shared
  
  // Decrement prior to getting a new AStringRef
  dereference();
  return AStringRef::pool_new(cstr_p, length, size, 1u, deallocate, false);
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Returns common empty AStringRef Reference used by the empty AString and null symbol.
// 0-length empty AString, i.e. ""
// 
// #Notes
//   Uses Scott Meyers' tip "Make sure that objects are initialized before they're used"
//   from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd Edition]
//   This is instead of using a non-local static object for a singleton.
//   
// #Modifiers  static
// #Author(s)  Conan Reis
AStringRef * AStringRef::get_empty()
  {
  // Note that the initial reference count is 2 to ensure that it not be added to the
  // pool once it is dereferenced since it is not dynamically allocated.
  static AStringRef s_empty("", 0u, 1u, 2u, false, true);
  //A_DSINGLETON_GUARD;

  return &s_empty;
  }

#ifdef A_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of AStringRefs
AObjReusePool<AStringRef> & AStringRef::get_pool()
  {
  return ms_pool;
  }

#endif
