// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Instance of a class with data members
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APSorted.hpp>
#include <SkookumScript/SkInstanceList.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

// Store a magic number in the user data and perform sanity checking
#if (SKOOKUM & SK_DEBUG) && defined(A_BITS64)
  #define SK_DATA_INSTANCE_HAS_MAGIC_MARKER
#endif

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Class instance objects with one or more data members
// Subclasses SkDataInstance(SkActor), SkMetaClass
// Author(s)  Conan Reis
class SK_API SkDataInstance : public SkInstance
  {
  public:
  
  friend class AObjReusePool<SkDataInstance>;

  // Common Methods

    SK_NEW_OPERATORS(SkDataInstance);
    SkDataInstance(SkClass * class_p);
    virtual ~SkDataInstance() override;


  // Methods

    void          add_data_members();
    void          data_empty()          { m_data.empty(); }
    SkInstance *  get_data_by_idx(uint32_t data_idx) const;
    SkInstance ** get_data_addr_by_idx(uint32_t data_idx) const;
    void          set_data_by_idx(uint32_t data_idx, SkInstance * obj_p);

    // Overriding from SkInstance

    virtual void          delete_this() override;
    virtual SkInstance *  get_data_by_name(const ASymbol & name) const override;
    virtual bool          set_data_by_name(const ASymbol & name, SkInstance * data_p) override;

  // Pool Allocation Methods

    static SkDataInstance * new_instance(SkClass * class_p);
    static AObjReusePool<SkDataInstance> & get_pool();

  protected:

    friend class AObjReusePool<SkDataInstance>;

  // Internal Methods

    // Default constructor only may be called by pool_new()
    SkDataInstance();

    SkDataInstance ** get_pool_unused_next() { return (SkDataInstance **)&m_user_data.m_data.m_uintptr; } // Area in this class where to store the pointer to the next unused object when not in use

  // Data Members

    // Array of class instance data members - accessed by symbol name.
    // $Revisit - CReis It may be possible to rewrite this so that a direct index can be
    // used rather than a binary search of the symbols
    SkInstanceList m_data;

  #ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
    // This magic number is stored in the user data to allow sanity checking code to verify we are dealing in fact with a data instance
    // It's not really a pointer, thus no _p postfix
    static void * const ms_magic_marker;

    // Called when a mismatch is detected - alerts the user and returns nil
    SkInstance ** on_magic_marker_mismatch(uint32_t data_idx) const;
  #endif

    // The global pool of SkDataInstances
    static AObjReusePool<SkDataInstance> ms_pool;

  };  // SkDataInstance


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkDataInstances
A_FORCEINLINE AObjReusePool<SkDataInstance> & SkDataInstance::get_pool()
  {
  return ms_pool;
  }

#endif

#ifndef A_INL_IN_CPP
#include <SkookumScript/SkDataInstance.inl>
#endif

