// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for objects in language - instance of a class with data
//             members
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkDataInstance.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkDataInstance.inl>
#endif
#include <AgogCore/AObjReusePool.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkTyped.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkSymbol.hpp>

//=======================================================================================
// Class Data
//=======================================================================================

AObjReusePool<SkDataInstance> SkDataInstance::ms_pool;

#ifdef SK_DATA_INSTANCE_HAS_MAGIC_MARKER
// This value is chosen to minimize likelihood to be mistaken for pointer or floating point number
void * const SkDataInstance::ms_magic_marker = (void *)uintptr_t(UINT64_C(0xffffdadaffdadaff));

SkInstance ** SkDataInstance::on_magic_marker_mismatch(uint32_t data_idx) const
  {
  SK_ERRORX(a_str_format("Tried to access data member '%s' on an instance of class '%s' but there is no instance data present. Maybe you forgot to add a SkookumScriptClassDataComponent to its Blueprint?", \
    m_class_p->get_instance_data_type(data_idx)->get_name_cstr(), m_class_p->get_name_cstr()));
  return &SkBrain::ms_nil_p;
  }
#endif

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkInstance
// Author(s):   Conan Reis
SkDataInstance::~SkDataInstance()
  {
  data_empty();
  }

//---------------------------------------------------------------------------------------
// Adds all the data members for this instance and initializes them to nil.
// Author(s):   Conan Reis
void SkDataInstance::add_data_members()
  {
  m_data.append_all(m_class_p->get_total_data_count(), *SkBrain::ms_nil_p);
  }

//---------------------------------------------------------------------------------------
//  Frees up an instance and puts it into the dynamic pool ready for its next
//              use.  This should be used instead of 'delete' because it prevents
//              unnecessary deallocations by saving previously allocated objects.
// Arg          str_ref_p - pointer to instance to free up and put into the dynamic pool.
// Examples:    called by dereference()
// See:         pool_new(), dereference()
// Notes:       To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:    virtual - Overriding from SkInstance
// Author(s):    Conan Reis
void SkDataInstance::delete_this()
  {
  data_empty();
  m_ptr_id = AIdPtr_null;
  get_pool().recycle(this);
  }

//---------------------------------------------------------------------------------------
// Look up the given variable by name
SkInstance * SkDataInstance::get_data_by_name(const ASymbol & name) const
  {
  // First check instance data
  uint32_t data_idx = 0;
  if (m_class_p->get_instance_data_type(name, &data_idx))
    {
    return get_data_by_idx(data_idx);
    }

  // If that fails, try class data
  return SkInstance::get_data_by_name(name);
  }

//---------------------------------------------------------------------------------------
// Look up the given variable by name and set it to given value
bool SkDataInstance::set_data_by_name(const ASymbol & name, SkInstance * data_p)
  {
  // First check instance data
  uint32_t data_idx = 0;
  if (m_class_p->get_instance_data_type(name, &data_idx))
    {
    set_data_by_idx(data_idx, data_p);
    return true;
    }

  // If that fails, try raw instance / class data
  return SkInstance::set_data_by_name(name, data_p);
  }

#ifdef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkDataInstances
AObjReusePool<SkDataInstance> & SkDataInstance::get_pool()
  {
  return ms_pool;
  }

#endif
