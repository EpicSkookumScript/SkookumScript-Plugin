// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for storing user data
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ARefCount.hpp>
#include <AgogCore/AString.hpp>
#include <AgogCore/ADebug.hpp>
#include <SkookumScript/Sk.hpp>

//=======================================================================================
//
//=======================================================================================

// Base class for accessing user data stored inside a class instance
struct SkUserDataBase
  {
  // Gets an object stored by value in this data structure cast to the desired type
  // Each specialization _must_ be explicitly template-specialized to make sure it's properly stored
  template <typename _UserType> _UserType * as() const  { static_assert(sizeof(_UserType) == 0, "as() _must_ be specialized for each type that is stored to make sure each type is stored correctly!"); return nullptr; }

  // Handy helper for POD types that are stored by value in this object
  template <typename _UserType> _UserType * as_stored() const { return (_UserType *)this; }

  // Sets user data of this object
  // Each specialization _must_ be explicitly template-specialized to make sure it's properly stored
  template <typename _UserType> void set(_UserType const & value)  { static_assert(sizeof(_UserType) == 0, "set() _must_ be specialized for each type that is stored to make sure each type is stored correctly!"); }

  };

// Class for storing user data
template <int _SizeInPtrs>
struct SkUserData : private SkUserDataBase
  {
  template <typename _UserType> _UserType * as() const;
  template <typename _UserType> void        set(const _UserType & value) { SkUserDataBase::set(value); }

  // placeholder union to reserve appropriate space
  union
    {
    tSkBoolean   m_boolean;
    tSkInteger   m_integer;
    tSkReal      m_real;
    tSkEnum      m_enum;
    uintptr_t       m_uintptr;
    void *          m_ptr[_SizeInPtrs];
    }               m_data;
  };

//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// A few common specializations of SkUserDataBase::as()

template<> inline tSkInteger * SkUserDataBase::as<tSkInteger>() const { return as_stored<tSkInteger>(); }
template<> inline tSkReal    * SkUserDataBase::as<tSkReal   >() const { return as_stored<tSkReal   >(); }
template<> inline tSkBoolean * SkUserDataBase::as<tSkBoolean>() const { return as_stored<tSkBoolean>(); }
template<> inline tSkEnum    * SkUserDataBase::as<tSkEnum   >() const { return as_stored<tSkEnum   >(); }
template<> inline AString    * SkUserDataBase::as<AString   >() const { return as_stored<AString      >(); }
template<> inline ASymbol    * SkUserDataBase::as<ASymbol   >() const { return as_stored<ASymbol      >(); }

//---------------------------------------------------------------------------------------
// A few common specializations of SkUserDataBase::set()

template<> inline void SkUserDataBase::set(const tSkInteger & value) { *as_stored<tSkInteger>() = value; }
template<> inline void SkUserDataBase::set(const tSkReal    & value) { *as_stored<tSkReal   >() = value; }
template<> inline void SkUserDataBase::set(const tSkBoolean & value) { *as_stored<tSkBoolean>() = value; }
template<> inline void SkUserDataBase::set(const tSkEnum    & value) { *as_stored<tSkEnum   >() = value; }
template<> inline void SkUserDataBase::set(const AString    & value) { *as_stored<AString      >() = value; }
template<> inline void SkUserDataBase::set(const ASymbol    & value) { *as_stored<ASymbol      >() = value; }

//---------------------------------------------------------------------------------------
// Gets an object stored in this data structure cast to the desired type
// relies on SkUserDataBase::as() and its specializations to actually know how to do this
// 
template <int _SizeInPtrs>
template <typename _UserType>
inline _UserType * SkUserData<_SizeInPtrs>::as() const
  {
  _UserType * data_p = SkUserDataBase::as<_UserType>();
  A_ASSERTX((void*)data_p != (void*)this || sizeof(_UserType) <= sizeof(*this), "_UserType does not fit into this instance of SkUserData.");
  return data_p;
  }
