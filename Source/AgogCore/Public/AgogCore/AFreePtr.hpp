// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Free Smart Pointer declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Constants & Macros
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    AFreePtr<> is a convenience class - it wraps around an existing pointer and
//          acts just like a regular pointer except that it automatically deletes the
//          object pointed to on its destruction.  This class is especially useful for
//          class data members and static data - it could be a bit of an overkill for
//          simple instance data members since they can easily be deleted in the
//          destructor.
// Author   Conan Reis
template<class _PtrType>
class AFreePtr
  {
  public:

  // Methods

    AFreePtr(_PtrType * obj_p = nullptr)  : m_obj_p(obj_p) {}
    ~AFreePtr();

    // Assignment

    AFreePtr & operator=(const _PtrType * obj_p);

    // Conversion methods
    operator _PtrType*() const        { return m_obj_p; }
    _PtrType & operator*() const      { return *m_obj_p; }
    _PtrType * operator->() const     { return m_obj_p; }
    _PtrType *       get_obj()        { return m_obj_p; }
    const _PtrType * get_obj() const  { return m_obj_p; }
    void             null()           { delete m_obj_p; m_obj_p = nullptr; }

    // Comparison operators

    bool operator==(const AFreePtr & free_ptr) const  { return m_obj_p == free_ptr.m_obj_p; }
    bool operator==(const _PtrType * obj_p) const     { return m_obj_p == obj_p; }
    bool operator==(const intptr_t addr) const        { return m_obj_p == reinterpret_cast<_PtrType *>(addr); }
    bool operator!=(const AFreePtr & free_ptr) const  { return m_obj_p != free_ptr.m_obj_p; }
    bool operator!=(const _PtrType * obj_p) const     { return m_obj_p != obj_p; }
    bool operator!=(const intptr_t addr) const        { return m_obj_p != reinterpret_cast<_PtrType *>(addr); }
    bool operator<(const AFreePtr & free_ptr) const   { return m_obj_p < free_ptr.m_obj_p; }
    bool operator<(const _PtrType * obj_p) const      { return m_obj_p < obj_p; }
    bool operator>(const AFreePtr & free_ptr) const   { return m_obj_p > free_ptr.m_obj_p; }
    bool operator>(const _PtrType * obj_p) const      { return m_obj_p > obj_p; }

    // Validation methods

    operator bool()         { return (m_obj_p != nullptr); }
    operator bool() const   { return (m_obj_p != nullptr); }
    bool operator !() const { return (m_obj_p == nullptr); }  // This must be defined here due to a 7.1 compiler problem

    bool is_valid() const   { return (m_obj_p != nullptr); }

  protected:

    AFreePtr(const AFreePtr & free_ptr)  : m_obj_p(free_ptr.m_obj_p) {}
    AFreePtr & operator=(const AFreePtr & free_ptr);

  // Data Members

    // Pointer to original object
    _PtrType * m_obj_p;
    
  };  // AFreePtr


//=======================================================================================
// AFreePtr Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AFreePtr<_PtrType>::~AFreePtr()
  {
  if (m_obj_p)
    {
    delete m_obj_p;

    #ifdef A_EXTRA_CHECK
      m_obj_p = nullptr;  // A little extra insurance
    #endif
    }
  }

//---------------------------------------------------------------------------------------
// Call this method with caution since the same pointer will now be freed
//             twice if there is no more interaction with free_ptr.
// Author(s):   Conan Reis
template<class _PtrType>
inline AFreePtr<_PtrType> & AFreePtr<_PtrType>::operator=(const AFreePtr<_PtrType> & free_ptr)
  {
  if (m_obj_p != free_ptr.m_obj_p)
    {
    delete m_obj_p;
    m_obj_p = const_cast<_PtrType *>(free_ptr.m_obj_p);
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AFreePtr<_PtrType> & AFreePtr<_PtrType>::operator=(const _PtrType * obj_p)
  {
  if (m_obj_p != obj_p)
    {
    delete m_obj_p;
    m_obj_p = const_cast<_PtrType *>(obj_p);
    }

  return *this;
  }
