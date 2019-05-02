// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Reference Counting Base Class and Smart Pointer declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// High bit indicating that the reference count has been zero and on_no_references()
// has been called.
const uint32_t ARefCount_zero_refs       = 1u << 31u;
const uint32_t ARefCount_zero_refs_mask  = ~ARefCount_zero_refs;


//---------------------------------------------------------------------------------------
// Mixin super/base class for objects that need to be reference counted and calls
// on_no_references() when decrementing from 1 to 0 or when 0 and ensure_reference() is
// called.  Methods can be overridden or specialized for custom behaviour - a rewrite of
// the on_no_references() call in particular is an easy way to give differnt behaviour.
// 
// As a mixin template it avoids the (minor) speed cost of virtual function calls and the
// virtual table memory cost (1 pointer).  It uses the coding technique known as
// "mix-in from above"/"Curiously Recurring Template Pattern".
//
// #See Also  ARefPtr<> below
// #Author(s) Conan Reis
template<class _Subclass>
class ARefCountMix
  {
  public:

  // Common Methods

    ARefCountMix()                                      : m_ref_count(0u)  {}
    ARefCountMix(eALeaveMemoryUnchanged)                {}
    ARefCountMix(uint32_t init_refs)                    : m_ref_count(init_refs)  {}
    ARefCountMix(const ARefCountMix & ref)              : m_ref_count(0u)  {}  // Reference count not copied

    ARefCountMix & operator=(const ARefCountMix & ref)  { return *this; }  // Reference count not copied

  // Methods

    void     dereference();
    void     dereference_delay() const;
    void     ensure_reference();
    uint32_t get_references() const                     { return (m_ref_count & ARefCount_zero_refs_mask); }
    void     reference() const;
    void     reference(uint32_t increment_by) const;

  // Events

    void on_no_references();

  protected:
  // Data Members

    // Number of references to this object.
    mutable uint32_t m_ref_count;

  };  // ARefCountMix


//---------------------------------------------------------------------------------------
// Notes    ARefPtr<> is a convenience class - it wraps around a pointer to an object
//          that is a subclass of ARefCountMix<> [or any class that has the methods:
//          reference() & dereference()] and  acts just like a regular pointer except that
//          it automatically references and dereferences the object as needed.
// Author   Conan Reis
template<class _PtrType>
class ARefPtr
  {
  public:

  // Methods

    ARefPtr()                         : m_obj_p(nullptr)  {}
    ARefPtr(const ARefPtr & ref_ptr)  : m_obj_p(ref_ptr.m_obj_p) { if (ref_ptr.m_obj_p) { ref_ptr.m_obj_p->reference(); } }
    ARefPtr(const _PtrType * obj_p)   : m_obj_p(const_cast<_PtrType *>(obj_p)) { if (m_obj_p) { m_obj_p->reference(); } }
    ~ARefPtr();

    // Assignment

    ARefPtr & operator=(const ARefPtr & ref_ptr);
    ARefPtr & operator=(const _PtrType * obj_p);

    // Conversion methods
    operator _PtrType*() const        { return m_obj_p; }
    _PtrType & operator*() const      { return *m_obj_p; }
    _PtrType * operator->() const     { return m_obj_p; }
    _PtrType *       get_obj()        { return m_obj_p; }
    const _PtrType * get_obj() const  { return m_obj_p; }
    void             null()           { if (m_obj_p) { m_obj_p->dereference(); } m_obj_p = nullptr; }
    void             null_delay()     { if (m_obj_p) { m_obj_p->dereference_delay(); } m_obj_p = nullptr; }

    // Comparison operators

    bool operator==(const ARefPtr & ref_ptr) const  { return m_obj_p == ref_ptr.m_obj_p; }
    bool operator==(const _PtrType * obj_p) const   { return m_obj_p == obj_p; }
    bool operator==(const intptr_t addr) const      { return m_obj_p == reinterpret_cast<_PtrType *>(addr); }
    bool operator!=(const ARefPtr & ref_ptr) const  { return m_obj_p != ref_ptr.m_obj_p; }
    bool operator!=(const _PtrType * obj_p) const   { return m_obj_p != obj_p; }
    bool operator!=(const intptr_t addr) const      { return m_obj_p != reinterpret_cast<_PtrType *>(addr); }
    bool operator<(const ARefPtr & ref_ptr) const   { return m_obj_p < ref_ptr.m_obj_p; }
    bool operator<(const _PtrType * obj_p) const    { return m_obj_p < obj_p; }
    bool operator>(const ARefPtr & ref_ptr) const   { return m_obj_p > ref_ptr.m_obj_p; }
    bool operator>(const _PtrType * obj_p) const    { return m_obj_p > obj_p; }

    // Validation methods

    operator bool()         { return (m_obj_p != nullptr); }
    operator bool() const   { return (m_obj_p != nullptr); }
    bool operator!() const  { return (m_obj_p == nullptr); }  // This must be defined here due to a 7.1 compiler problem

    bool is_valid() const   { return (m_obj_p != nullptr); }

  protected:
  // Data Members

    // Pointer to original object
    _PtrType * m_obj_p;
    
  };  // ARefPtr


//=======================================================================================
// ARefCountMix Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Called when the number of references to this object reaches zero - by
//             default it deletes this object.
//             Specialize or override / make virtual in subclass custom behaviour.
// See:        dereference(), ensure_reference()
// Notes:      called by dereference() and ensure_reference()
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::on_no_references()
  {
  // Cast to subclass so if destructor is virtual/overridden it will be called properly.
  delete static_cast<_Subclass *>(this);
  }

//---------------------------------------------------------------------------------------
// Decrements the reference count to this object and if the reference count
//             becomes 0 call on_no_references()
// See:        reference(), dereference_delay(), on_no_references()
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::dereference()
  {
  // Equivalent to calling dereference_delay()

  #ifdef A_EXTRA_CHECK
    if ((m_ref_count & ARefCount_zero_refs_mask) == 0u)
      {
      A_ERRORX(AErrMsg("Tried to dereference an object that has no references!", AErrLevel_notify));

      return;
      }
  #endif

  // Coded like this to avoid load-hit-store penalty
  // Equivalent to calling ensure_reference()
  uint32_t ref_count = m_ref_count - 1;
  if (ref_count == 0u)
    {
    m_ref_count = ARefCount_zero_refs;

    // Cast to subclass so if this method is virtual/overridden it will be called properly.
    static_cast<_Subclass *>(this)->on_no_references();
    }
  else
    {
    m_ref_count = ref_count;
    }
  }

//---------------------------------------------------------------------------------------
// Same as dereference() in that it decrements the reference count to this
//             object, but it does not call on_no_references() if the reference count
//             becomes 0.  The call to on_no_references() is delayed until the method
//             ensure_reference() is called.
// See:        ensure_reference(), reference(), dereference(), on_no_references()
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::dereference_delay() const
  {
  #ifdef A_EXTRA_CHECK
    if ((m_ref_count & ARefCount_zero_refs_mask) == 0u)
      {
      A_ERRORX(AErrMsg("Tried to dereference an object that has no references!", AErrLevel_notify));

      return;
      }
  #endif

  m_ref_count--;
  }

//---------------------------------------------------------------------------------------
// Calls on_no_references() if the reference count is 0
// See:        dereference_delay(), on_no_references()
// Notes:      Do not call ensure_reference() on an object that has already had the method
//             on_no_references() called on it - i.e. do not call ensure_reference() twice
//             in a row or after a call to dereference().
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::ensure_reference()
  {
  if (m_ref_count == 0u)
    {
	  m_ref_count = ARefCount_zero_refs;

    // Cast to subclass so if this method is virtual/overridden it will be called properly.
    static_cast<_Subclass *>(this)->on_no_references();
    }
  }

//---------------------------------------------------------------------------------------
// Increments the reference count to this object.
// See:        dereference(), dereference_delay()
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::reference() const
  {
  m_ref_count++;
  }

//---------------------------------------------------------------------------------------
// Increments the reference count to this object by the specified amount.
// See:        dereference(), dereference_delay()
// Author(s):   Conan Reis
template<class _Subclass>
inline void ARefCountMix<_Subclass>::reference(uint32_t increment_by) const
  {
  m_ref_count += increment_by;
  }


//=======================================================================================
// ARefPtr Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline ARefPtr<_PtrType>::~ARefPtr()
  {
  if (m_obj_p)
    {
    m_obj_p->dereference();

    #ifdef A_EXTRA_CHECK
      m_obj_p = nullptr;  // A little extra insurance
    #endif
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline ARefPtr<_PtrType> & ARefPtr<_PtrType>::operator=(const ARefPtr<_PtrType> & ref_ptr)
  {
  if (m_obj_p != ref_ptr.m_obj_p)
    {
    if (ref_ptr.m_obj_p)
      {
      const_cast<_PtrType *>(ref_ptr.m_obj_p)->reference();
      }

    if (m_obj_p)
      {
      m_obj_p->dereference();
      }

    m_obj_p = const_cast<_PtrType *>(ref_ptr.m_obj_p);
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline ARefPtr<_PtrType> & ARefPtr<_PtrType>::operator=(const _PtrType * obj_p)
  {
  if (m_obj_p != obj_p)
    {
    if (obj_p)
      {
      const_cast<_PtrType *>(obj_p)->reference();
      }

    if (m_obj_p)
      {
      m_obj_p->dereference();
      }

    m_obj_p = const_cast<_PtrType *>(obj_p);
    }

  return *this;
  }
