// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Id Pointer and associated data structures declaration header
//
// Terminology:
//   Id Pointer  - AIdPtr<Class> smart pointer that points to a persistant object that
//                 could become invalid or reused at any time.
//   Pointer Id  - intrusive uint32_t that uniquely identifies an object for a AIdPtr
//   Id Pointer Holder - AIdPtrHolder<Class> helper class to manage pointer id in objects
//
// Id Pointers are designed to be used with persistant/pooled objects that may become
// invalid or reused for a different logical object - thus leading to a dangling or
// invalid/stale pointer.  An id pointer should be used rather than a 'naked' pointer to
// an object to ensure that a valid object pointer always is returned (in constant time)
// or nullptr if it has gone 'stale' or if it truly was nullptr.  Pointer ids can also be a
// good means to uniquely identify (shallowly) an object during serialization without
// mucking about with pointers.
//
// To use id pointers:
//
//   1) Store an m_ptr_id member in a class [i.e. ClasType::m_ptr_id] to be used by
//      AIdPtr<> and initialize any object instance with AIdPtr<>::get_next_ptr_id().
//      [Alternatively derive the class from AIdPtrHolder<> or have a AIdPtrHolder<>
//      m_ptr_id data member which will set the id in its constructor or with
//      reacquire().]
//   2) Use AIdPtr<> in the place of any normal pointer to that class - i.e. use
//        AIdPtr<ClassType> m_obj_p;
//      rather than
//        ClassType * m_obj_p;
//   3) Set m_ptr_id in the object to AIdPtr_null or a new id when the object becomes
//      invalid or the data structure is to be reused/retasked.  [Alternatively derive
//      the class from AIdPtrHolder<> or have a AIdPtrHolder<> m_ptr_id data member
//      which will clear the id in its destructor or with release().]
//   4) Any AIdPtr<> smart pointers will become stale and start to return nullptr instead of
//      the address of the object.
//
//
// ####Example####
// 
// class AIdPtrStr;  // Forward declaration
// 
// // Technique using AIdPtrHolder<>
// class AIdPtrStr : public AString
//   {
//   public:
//     AIdPtrStr(const char * cstr_p) : AString(cstr_p) {}
// 
//     AIdPtrHolder<AIdPtrStr> m_ptr_id;
//   };
//
//
// // Alternate technique using just uint32_t pointer ids
// class AIdPtrStr : public AString
//   {
//   public:
//     AIdPtrStr(const char * cstr_p) : AString(cstr_p), m_ptr_id(AIdPtr<AIdPtrStr>::get_next_ptr_id())  {}
//     ~AIdPtrStr()                     { m_ptr_id = AIdPtr_null; }
// 
//     uint32_t m_ptr_id;
//   };
// 
// 
// void test_func()
//   {
//   AIdPtrStr *       id_str_p = new AIdPtrStr("Test3");
//   AIdPtr<AIdPtrStr> str_p(idx_str_p);
// 
//     if (str_p)  // Tests true
//       str_p->toupper();
// 
//     // Data structure is reused by system...
//     idx_str_p->m_ptr_id = AIdPtr<AIdPtrStr>::get_next_ptr_id();
// 
//     if (str_p)  // Tests false
//       str_p->tolower();
// }
//
//
//
// *Note: Any modifications to these templates should be compile-tested by adding one of
// the AIdPtrStr example classes defined above and also explicit instantiation
// declarations for each of the index pointer templates such as:
//   template class AIdPtr<AIdPtrStr>;
//   template class AIdPtrHolder<AIdPtrStr>;
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Constants & Macros
//=======================================================================================

// The null id (0) is always invalid
const uint32_t AIdPtr_null = 0u;


//=======================================================================================
// Global Structures
//=======================================================================================

// Forward declaration
template<class _PtrType> class AIdPtrBase;

//---------------------------------------------------------------------------------------
// Notes    AIdPtr() is a convenience class - it wraps around an index id and treats it
//          like a standard pointer to the object.
//
//          Some of this class's methods assume that objects that are referred to by
//          index ids (_PtrType) can retrieve their index ids via a call to
//          _PtrType::get_ptr_id().
//
// Author   Conan Reis
template<class _PtrType>
class AIdPtr
  {
  public:

  // Methods

    explicit AIdPtr();
    explicit AIdPtr(_PtrType * obj_p);
    AIdPtr(uint32_t ptr_id, _PtrType * obj_p);
    AIdPtr(const AIdPtr & id_ptr);

    // Assignment

    AIdPtr & operator=(const AIdPtr & id_ptr);
    AIdPtr & operator=(const _PtrType * obj_p);

    // Conversion methods
    operator _PtrType*() const;
    _PtrType &       operator*() const;
    _PtrType *       operator->() const;
    _PtrType *       get_obj();
    const _PtrType * get_obj() const;
    void             null();

    // Comparison operators

    bool operator==(const AIdPtr & id_ptr) const;
    bool operator==(const _PtrType * obj_p) const;
    //bool operator!=(const AIdPtr & id_ptr) const;
    //bool operator>(const AIdPtr & id_ptr) const;
    //bool operator<(const AIdPtr & id_ptr) const;

    // Validation methods

    operator bool();
    operator bool() const;
    bool operator !() const  { return !m_obj_p || (m_obj_p->m_ptr_id != m_ptr_id); }  // This must be defined here due to a 7.1 compiler problem

    bool is_null() const;
    bool is_set() const;
    bool is_stale() const;
    bool is_valid() const;

    // Index Id Accessors

    uint32_t get_ptr_id() const;

  // Class Methods

    static uint32_t get_next_ptr_id();

  protected:

    friend class AIdPtrBase<_PtrType>;

  // Data Members

    // Direct pointer to actual object - might be stale.  It is only valid if its pointer
    // id is the same as m_ptr_id in this smart pointer.
    _PtrType * m_obj_p;

    // Unique id shared between the object and its id smart pointers.
    uint32_t m_ptr_id;

  // Class Members

    // Last pointer id that was used.
    static uint32_t ms_ptr_id_prev;

  };  // AIdPtr


//---------------------------------------------------------------------------------------
// Notes    A convenience class, AIdPtrBase<> automatically sets m_ptr_id in its
//          constructor & reaquire() and clears m_ptr_id in its destructor & release().
//          Just make it a parent of an object that will be referred to by id pointers.
//
// Author   Conan Reis
template<class _PtrType>
class AIdPtrBase
  {
  public:

  // Public Data Members

    // Pointer id of the object that must match the id in any id pointer for it to be valid.
    uint32_t m_ptr_id;

  // Methods

    AIdPtrBase();
    ~AIdPtrBase();

    void release();
    void reacquire();

  };  // AIdPtrBase


//---------------------------------------------------------------------------------------
// Notes    A convenience class, AIdPtrHolder<> automatically sets m_ptr_id in its
//          constructor and clears m_ptr_id in its destructor.  Just make it a m_ptr_id
//          data member or a parent of an object that will be referred to by id pointers.
//
// Author   Conan Reis
template<class _PtrType>
class AIdPtrHolder
  {
  public:

  // Public Data Members

    // Pointer id of the object that must match the id in any id pointer for it to be valid.
    uint32_t m_ptr_id;

  // Methods

    AIdPtrHolder();
    AIdPtrHolder(uint32_t ptr_id);
    ~AIdPtrHolder();

    operator uint32_t() const;

    void release();
    void reacquire();

  };  // AIdPtrHolder


//=======================================================================================
// AIdPtr Class Data
//=======================================================================================

template<class _PtrType>
uint32_t AIdPtr<_PtrType>::ms_ptr_id_prev = AIdPtr_null;


//=======================================================================================
// AIdPtr Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default constructor
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::AIdPtr() :
  m_obj_p(nullptr),
  m_ptr_id(AIdPtr_null)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::AIdPtr(const AIdPtr & id_ptr) :
  m_obj_p(id_ptr.m_obj_p),
  m_ptr_id(id_ptr.m_ptr_id)
  {
  }

//---------------------------------------------------------------------------------------
// Normal pointer constructor
// Examples:   AIdPtrString   str("Test")
//             AIdPtr<String> str_p(&str);
// Notes:      Assumes that _PtrType has a m_ptr_id member.
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::AIdPtr(_PtrType * obj_p) :
  m_obj_p(obj_p),
  m_ptr_id(obj_p ? obj_p->m_ptr_id : AIdPtr_null)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::AIdPtr(
  uint32_t   ptr_id,
  _PtrType * obj_p
  ) :
  m_obj_p(obj_p),
  m_ptr_id(ptr_id)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType> & AIdPtr<_PtrType>::operator=(const AIdPtr<_PtrType> & id_ptr)
  {
  m_obj_p  = id_ptr.m_obj_p;
  m_ptr_id = id_ptr.m_ptr_id;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType> & AIdPtr<_PtrType>::operator=(const _PtrType * obj_p)
  {
  m_obj_p  = const_cast<_PtrType *>(obj_p);
  m_ptr_id = obj_p ? obj_p->m_ptr_id : AIdPtr_null;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::operator _PtrType*() const
  {
  _PtrType * obj_p = m_obj_p;

  return (obj_p && (m_ptr_id == obj_p->m_ptr_id))
    ? obj_p
    : nullptr;

  // Could set m_obj_p to nullptr if it is stale, but it might provide some extra clues when
  // debugging.
  }


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType & AIdPtr<_PtrType>::operator*() const
  {
  _PtrType * obj_p = m_obj_p;

  return (obj_p && (m_ptr_id == obj_p->m_ptr_id))
    ? *m_obj_p
    : *(_PtrType *)nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType * AIdPtr<_PtrType>::operator->() const
  {
  _PtrType * obj_p = m_obj_p;

  return (obj_p && (m_ptr_id == obj_p->m_ptr_id))
    ? obj_p
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Comparison operator
// Notes:      Stale pointers are considered to be equivalent to nullptr - for example:
//               stale_id_ptr1_p == nullptr             // true
//               stale_id_ptr1_p == stale_id_ptr2_p  // true
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::operator==(const AIdPtr<_PtrType> & id_ptr) const
  {
  return (m_obj_p == id_ptr.m_obj_p) || (get_obj() == id_ptr.get_obj());
  }

//---------------------------------------------------------------------------------------
// Comparison operator
// Notes:      Stale pointers are considered to be equivalent to nullptr - for example:
//               stale_id_ptr1_p == nullptr             // true
//               stale_id_ptr1_p == stale_id_ptr2_p  // true
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::operator==(const _PtrType * obj_p) const
  {
  return (m_obj_p == obj_p) || (!obj_p && (m_obj_p->m_ptr_id != m_ptr_id));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdPtr<_PtrType>::get_ptr_id() const
  {
  return m_ptr_id;
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::operator bool()
  {
  return m_obj_p && (m_obj_p->m_ptr_id == m_ptr_id);
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtr<_PtrType>::operator bool() const
  {
  return m_obj_p && (m_obj_p->m_ptr_id == m_ptr_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType * AIdPtr<_PtrType>::get_obj()
  {
  _PtrType * obj_p = m_obj_p;

  return (obj_p && (m_ptr_id == obj_p->m_ptr_id))
    ? obj_p
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline const _PtrType * AIdPtr<_PtrType>::get_obj() const
  {
  _PtrType * obj_p = m_obj_p;

  return (obj_p && (m_ptr_id == obj_p->m_ptr_id))
    ? obj_p
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::is_null() const
  {
  return !m_obj_p || (m_obj_p->m_ptr_id != m_ptr_id);
  }

//---------------------------------------------------------------------------------------
// Determines if this pointer has been set with an object - whether it is
//             currently stale or not.
// Returns:    true if set false if not
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::is_set() const
  {
  return (m_obj_p != nullptr);
  }

//---------------------------------------------------------------------------------------
// Determines if the object referred to by this AIdPtr has gone stale.  In
//             other words this AIdPtr used to refer to a valid object, but it has since
//             been destroyed / deleted / or otherwise removed from the index table.
// Returns:    true if object stale or if AIdPtr points to nullptr, false if object still
//             valid.
// Notes:      An AIdPtr is not considered 'stale' if it was explicitly set to nullptr.
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::is_stale() const
  {
  return m_obj_p && (m_obj_p->m_ptr_id != m_ptr_id);
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdPtr<_PtrType>::is_valid() const
  {
  return m_obj_p && (m_obj_p->m_ptr_id == m_ptr_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdPtr<_PtrType>::null()
  {
  m_obj_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the next pointer id for this type of id pointer
// Returns:    next pointer id
// Notes:      Alternatively just use the following:
//               m_ptr_id = ++AIdPtr<_PtrType>::ms_ptr_id_prev;
// Modifiers:   static
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdPtr<_PtrType>::get_next_ptr_id()
  {
  return ++ms_ptr_id_prev;
  }


//=======================================================================================
// AIdPtrHolder Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Normal constructor - sets m_ptr_id to the next valid id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrBase<_PtrType>::AIdPtrBase() :
  m_ptr_id(++AIdPtr<_PtrType>::ms_ptr_id_prev)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor - invalidates pointer id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrBase<_PtrType>::~AIdPtrBase()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Invalidates pointer id
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdPtrBase<_PtrType>::release()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Sets pointer id to the next valid id
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdPtrBase<_PtrType>::reacquire()
  {
  m_ptr_id = ++AIdPtr<_PtrType>::ms_ptr_id_prev;
  }


//=======================================================================================
// AIdPtrHolder Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Normal constructor - sets m_ptr_id to the next valid id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrHolder<_PtrType>::AIdPtrHolder() :
  m_ptr_id(++AIdPtr<_PtrType>::ms_ptr_id_prev)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor using specific pointer id
// Arg         ptr_id - previously reserved id to use.
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrHolder<_PtrType>::AIdPtrHolder(uint32_t ptr_id) :
  m_ptr_id(ptr_id)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor - invalidates pointer id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrHolder<_PtrType>::~AIdPtrHolder()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Gets the pointer id that is being held
// Returns:    Pointer id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdPtrHolder<_PtrType>::operator uint32_t() const
  {
  return m_ptr_id;
  }

//---------------------------------------------------------------------------------------
// Invalidates pointer id
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdPtrHolder<_PtrType>::release()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Sets pointer id to the next valid id
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdPtrHolder<_PtrType>::reacquire()
  {
  m_ptr_id = ++AIdPtr<_PtrType>::ms_ptr_id_prev;
  }
