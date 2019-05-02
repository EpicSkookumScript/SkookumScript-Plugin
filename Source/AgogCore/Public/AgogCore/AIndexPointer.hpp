// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Index Pointer and associated data structures declaration header
//
// Terminology:
//   Index Pointer (IdxPtr) - AIdxPtr<Class>
//   Index Id (IdxId) - 4 bytes (uint32_t)
//     Comprised of:
//       Index (Idx) - 2 bytes (uint16_t)
//       Index Increment (IdxIncr) - 2 bytes (uint16_t)
//   Index Pointer Holder (IdxIdHolder) - AIdxIdHolder<Class>
//   Index Pointer Table - AIdxPtrTable<Class>
//
// An index id should be used rather than a 'naked' pointer to an object if the object
// being referred to may be deallocated or if the object data structure is reused for a
// different logical object - thus leading to a dangling or invalid/stale pointer.  Index
// ids can be used to retrieve the pointer to an associated object and ensure that a
// valid object pointer is returned (in constant time) or nullptr if it has gone 'stale' or
// if it truly was nullptr.  Index ids are also a good means to uniquely identify
// (shallowly) an object during serialization without mucking about with pointers.
//
// To use index ids:
//
//   1) Create an index pointer table (AIdxPtrTable<>) - generally at global scope, as a
//      class static data member, or in the program main()/initialization.
//   2) Call AIdxPtrTable<>::acquire(obj_p) to associate a pointer to an object and to
//      get an index id for that object.  This is usually done in an object's constructor
//      and the index id is stored as a data member in the object.
//   3) Store the index id wherever a pointer to the object would normally be stored.
//   4) As needed call AIdxPtrTable<>::get_obj(idx_id) with the index id to get the
//      pointer to the associated object.  It will return nullptr if the referenced object
//      is not available.
//   5) When the object is no longer available call AIdxPtrTable<>::release(idx_id).
//      This will usually be done in the object's destructor.
//
// For convenience, the AIdxIdHolder<> class automatically calls AIdxPtrTable<>::acquire(obj_p)
// in its constructor and AIdxPtrTable<>::release(obj_p) in its destructor.  Just make it a
// data member of an object that will be referred to by index ids.
//
// Another convenience class is AIdxPtr().  It wraps around an index id and treats it
// like a standard pointer to the object.
//
// An index is all that is required to refer to an object for shallow serialization since
// a new index increment value will be created when the object pointer is remapped to
// that index location.
//
//
// ####Example####
// 
// class AIdxStr;  // Forward declaration
// 
// // Make table with 100 max
// AIdxPtrTable<AIdxStr> g_str_table(100u);
// 
// // Technique using AIdxIdHolder<>
// class AIdxStr : public AString
//   {
//   public:
//     AIdxStr(const char * cstr_p) : AString(cstr_p), m_idx_id(this) {}
//     uint32_t get_idx_id() const  { return m_idx_id; }
// 
//     AIdxIdHolder<AIdxStr> m_idx_id;
//   };
//
//
// // Alternate technique using just uint32_t index ids
// class AIdxStr : public AString
//   {
//   public:
//     AIdxStr(const char * cstr_p) : AString(cstr_p)  { m_idx_id = g_str_table.acquire(this); }
//     ~AIdxStr()                                      { g_str_table.release(m_idx_id); }
//     uint32_t get_idx_id() const                         { return m_idx_id; }
// 
//     uint32_t m_idx_id;
//   };
// 
// 
// void test_func()
//   {
//   AIdxStr *        idx_str_p = new AIdxStr("Test3");
//   AIdxPtr<AIdxStr> str_p(idx_str_p);
// 
//     if (str_p)  // Tests true
//       str_p->toupper();
// 
//     delete idx_str_p;
// 
//     if (str_p)  // Tests false
//       str_p->tolower();
// }
//
//
//
// *Note: Any modifications to these templates should be compile-tested by adding one of
// the AIdxStr classes defined above and also explicit instantiation declarations for
// each of the index pointer templates such as:
//   template class AIdxPtr<AIdxStr>;
//   template class AIdxIdHolder<AIdxStr>;
//   template class AIdxPtrTable<AIdxStr>;
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ADebug.hpp>


//=======================================================================================
// Constants & Macros
//=======================================================================================

#if defined(A_EXTRA_CHECK) && !defined(AIDX_NO_CHECK)

  #ifndef AIDX_PTR_GUARD
    #define AIDX_PTR_GUARD  // Enable bounds/integrity checks
    #define AIDX_PTR_STATS  // Enable statistics for tracking/debugging
  #endif

#endif

// Enumerated constants
enum
  {
  AIdxId_index_bits  = 16,
  AIdxId_obj_max     = 0xFFFF - 1,  // 2^16 (-1 for nullptr at zeroth index)
  AIdxId_obj_max_def = AIdxId_obj_max,
  AIdxId_index_mask  = 0xFFFF
  };

// The zeroth index is always nullptr with an index increment of zero
const uint32_t AIdxId_null = 0u;

// This is the max value an index increment may be before it loops around to one again
const uint16_t AIdxId_incr_max = 0xFFFF;


//=======================================================================================
// Global Structures
//=======================================================================================

template <class _PtrType> class AIdxPtrTable;

//---------------------------------------------------------------------------------------
// Notes    AIdxPtr() is a convenience class - it wraps around an index id and treats it
//          like a standard pointer to the object.
//
//          Some of this class's methods assume that objects that are referred to by
//          index ids (_PtrType) can retrieve their index ids via a call to
//          _PtrType::get_idx_id().
//
// Author   Conan Reis
template<class _PtrType>
class AIdxPtr
  {
  public:

  // Methods

    explicit AIdxPtr(uint32_t idx_id = AIdxId_null);
    explicit AIdxPtr(uint16_t idx);
    explicit AIdxPtr(_PtrType * obj_p);
    AIdxPtr(uint32_t idx_id, _PtrType * obj_p);
    AIdxPtr(const AIdxPtr & idx_ptr)              : m_idx_id(idx_ptr.m_idx_id) {}

    // Assignment

    AIdxPtr & operator=(const AIdxPtr & idx_ptr);
    AIdxPtr & operator=(const _PtrType * obj_p);

    // Conversion methods
    operator _PtrType*() const;
    _PtrType &       operator*() const;
    _PtrType *       operator->() const;
    _PtrType *       get_obj();
    const _PtrType * get_obj() const;
    void             null()            { m_idx_id = AIdxId_null; }

    // Comparison operators

    bool operator==(const AIdxPtr & idx_ptr) const;
    bool operator==(const _PtrType * obj_p) const;
    //bool operator>(const AIdxPtr & idx_ptr) const;
    //bool operator<(const AIdxPtr & idx_ptr) const;

    // Validation methods

    operator bool();
    operator bool() const;
    bool operator !() const  { return (m_idx_id == AIdxId_null) || !get_table()->is_valid(m_idx_id); }  // This must be defined here due to a 7.1 compiler problem

    bool is_valid() const;
    bool is_stale() const;

    // Index Id Accessors

    uint32_t get_idx_id() const;
    uint16_t get_idx() const;

  // Class Methods

    static void                     set_table(AIdxPtrTable<_PtrType> * table_p);
    static AIdxPtrTable<_PtrType> * get_table();

  protected:
  // Data Members

    // Index id that is used to mimic pointer behavior, but with better robustness since
	// pointers to stale objects will return nullptr rather than deleted/reused memory.
    //   Comprised of:
    //     2 bytes (uint16_t) - Index (Idx)
    //     2 bytes (uint16_t) - Index Increment (IdxIncr)
    uint32_t m_idx_id;
    
  // Class Data Members

    // AIdxPtr<> objects need to know which index pointer table to use when executing
    // their methods.  This class data member is used so that the table would not have to
    // be passed as an argument to every method.
    // This table is also used by AIdxIdHolder<> via get_table()
    static AIdxPtrTable<_PtrType> * ms_idx_table_p;

  };  // AIdxPtr


//---------------------------------------------------------------------------------------
// Notes    A convenience class, AIdxIdHolder<> automatically calls AIdxPtrTable<>::acquire(obj_p)
//          in its constructor and AIdxPtrTable<>::release(idx_id) in its destructor.  Just
//          make it a data member of an object that will be referred to by index ids.
//
// Author   Conan Reis
template<class _PtrType>
class AIdxIdHolder
  {
  public:

  // Methods

    explicit AIdxIdHolder(_PtrType * obj_p);
    AIdxIdHolder(_PtrType * obj_p, uint16_t idx);
    AIdxIdHolder(_PtrType * obj_p, uint32_t idx_id);
    ~AIdxIdHolder();

    operator uint32_t() const;

    uint32_t get_idx_id() const;
    uint16_t get_idx() const;

    void release();
    void reacquire(_PtrType * obj_p);

  protected:
  // Data Members

    // Cached index id so that it does not need to be looked-up via a more expensive
	// search in an index table.
    //   Comprised of:
    //     2 bytes (uint16_t) - Index (Idx)
    //     2 bytes (uint16_t) - Index Increment (IdxIncr)
    uint32_t m_idx_id;

  };  // AIdxIdHolder


// $Revisit - CReis Add a AIdxIdHolderLazy class that is the same as AIdxIdHolder, but it
// delays storing the pointer in its table and getting its index id until the index id
// is first needed.


//---------------------------------------------------------------------------------------
// Notes    This class is used internally by AIdxPtrTable, but for some reason the
//          compiler doesn't seem to like it if it is protected and nested in AIdxPtrTable
//
// Author   Conan Reis
template<class _PtrType>
struct AIdxEntry
  {
    // Boolean indicating whether this entry is in use or free.  uint16_t is used in the
    // place of a boolean to ensure that 16 bits are used rather than 8 or 32.
    uint16_t m_used;

    // Index increment - incremented each time this index position is used.
    uint16_t m_idx_incr;

    union
      {
      // When this entry is used, this points to the object that the index refers
      // to or nullptr if this entry is reserved, but the pointer to the object has
      // not been specified yet.
      _PtrType * m_obj_p;

      // When this entry is not used, this points to the next free entry in the
      // table or nullptr if it is the last unused entry.
      AIdxEntry * m_next_free_p;
      };
  };

//---------------------------------------------------------------------------------------
// Notes    Holds the associations between indexes and objects and is used to validate
//          index ids and retrieve associated object pointers.
//
// Author   Conan Reis
template<class _PtrType>
class AIdxPtrTable
{
  public:

  // Methods

    AIdxPtrTable(uint32_t obj_max = AIdxId_obj_max_def);
    ~AIdxPtrTable();

    uint32_t   acquire(_PtrType * obj_p);
    bool       is_valid(uint32_t idx_id) const;
    _PtrType * get_obj(uint32_t idx_id) const;
    void       release(uint32_t idx_id);

    // Methods for specific indexes and serialization

    uint32_t acquire_specific(uint16_t idx, _PtrType * obj_p);
    uint32_t reserve();
    uint32_t reserve(int16_t idx);
    uint32_t idx_to_idx_id(int16_t idx);
    void     use_reserved(uint32_t idx_id, _PtrType * obj_p);

  protected:

  // Internal methods

    void remove_from_free_list(AIdxEntry<_PtrType> * entry_p);

  // Data Members

    // This is the array of entries that can be indexed into to get to referred objects.
    AIdxEntry<_PtrType> * m_entries_p;

    AIdxEntry<_PtrType> * m_first_free_p;

    #if defined(AIDX_PTR_STATS) || defined(AIDX_PTR_GUARD)
        uint32_t m_size;
        uint32_t m_count;
        uint32_t m_count_max;
    #endif

  };  // AIdxPtrTable


//=======================================================================================
// AIdxPtr Methods
//=======================================================================================

template<class _PtrType>
AIdxPtrTable<_PtrType> * AIdxPtr<_PtrType>::ms_idx_table_p = nullptr;


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::AIdxPtr(uint32_t idx_id) :
  m_idx_id(idx_id)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::AIdxPtr(uint16_t idx) :
  m_idx_id(get_table()->reserve(idx))
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::AIdxPtr(_PtrType * obj_p) :
  m_idx_id(obj_p ? obj_p->get_idx_id() : AIdxId_null)
  {
  // CReis - Could assert that index still contains object
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::AIdxPtr(
  uint32_t   idx_id,
  _PtrType * obj_p
  ) :
  m_idx_id(idx_id)
  {
  // CReis - Could assert that index still contains object
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType> & AIdxPtr<_PtrType>::operator=(const AIdxPtr<_PtrType> & idx_ptr)
  {
  m_idx_id = idx_ptr.m_idx_id;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType> & AIdxPtr<_PtrType>::operator=(const _PtrType * obj_p)
  {
  // CReis - Could assert that index still contains object
  m_idx_id = obj_p ? obj_p->get_idx_id() : AIdxId_null;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::operator _PtrType*() const
  {
  return get_table()->get_obj(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType & AIdxPtr<_PtrType>::operator*() const
  {
  return *get_table()->get_obj(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType * AIdxPtr<_PtrType>::operator->() const
  {
  return get_table()->get_obj(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Notes:      Stale pointers are considered to be equivalent to nullptr - for example:
//               stale_idx_ptr1_p == nullptr              // true
//               stale_idx_ptr1_p == stale_idx_ptr2_p  // true
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdxPtr<_PtrType>::operator==(const AIdxPtr<_PtrType> & idx_ptr) const
  {
  return (m_idx_id == idx_ptr.m_idx_id) || (get_obj() == idx_ptr.get_obj());
  }

//---------------------------------------------------------------------------------------
// Notes:      Stale pointers are considered to be equivalent to nullptr - for example:
//               stale_idx_ptr1_p == nullptr              // true
//               stale_idx_ptr1_p == stale_idx_ptr2_p  // true
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdxPtr<_PtrType>::operator==(const _PtrType * obj_p) const
  {
  return (obj_p)
    ? m_idx_id == obj_p->get_idx_id()
    : ((m_idx_id == AIdxId_null) || get_table()->is_valid(m_idx_id));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdxPtr<_PtrType>::get_idx_id() const
  {
  return m_idx_id;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline uint16_t AIdxPtr<_PtrType>::get_idx() const
  {
  return uint16_t(m_idx_id & AIdxId_index_mask);
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdxPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdxPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::operator bool()
  {
  return (m_idx_id != AIdxId_null) && get_table()->is_valid(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdxPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdxPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtr<_PtrType>::operator bool() const
  {
  return (m_idx_id != AIdxId_null) && get_table()->is_valid(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Determines if a non-nullptr value would be returned if this AIdxPtr were used.
// Returns:    true if non-nullptr, false if nullptr
// Notes:      The AIdxPtr will be nullptr if it was explicitly set to nullptr *or* if the
//             object that it points to has gone stale. 
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdxPtr<_PtrType>::is_valid() const
  {
  return (m_idx_id != AIdxId_null) && get_table()->is_valid(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Determines if the object referred to by this AIdxPtr has gone stale.  In
//             other words this AIdxPtr used to refer to a valid object, but it has since
//             been destroyed / deleted / or otherwise removed from the index table.
// Returns:    true if object stale or if AIdxPtr points to nullptr, false if object still
//             valid.
// Notes:      An AIdxPtr is not considered 'stale' if it was explicitly set to nullptr.
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdxPtr<_PtrType>::is_stale() const
  {
  return (m_idx_id != AIdxId_null) && get_table()->is_valid(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType * AIdxPtr<_PtrType>::get_obj()
  {
  return get_table()->get_obj(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline const _PtrType * AIdxPtr<_PtrType>::get_obj() const
  {
  return get_table()->get_obj(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Sets the current/global Index Pointer Table.
// Arg         table_p - table to use for all              
// See:        get_table()
// Notes:      This method is called automatically by the AIdxPtrTable constructor,
//             making this method public allows for the possibility to remap to a
//             different table
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdxPtr<_PtrType>::set_table(AIdxPtrTable<_PtrType> * table_p)
  {
  ms_idx_table_p = table_p;
  }

//---------------------------------------------------------------------------------------
// Gets the current/global Index Pointer Table.
// See:        set_table()
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxPtrTable<_PtrType> * AIdxPtr<_PtrType>::get_table()
  {
  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(ms_idx_table_p, "Tried to use a Pointer Index Table before it was created.");
  #endif

  return ms_idx_table_p;
  }


//=======================================================================================
// AIdxIdHolder Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Normal constructor - associates the object with an index id.
// Arg         obj_p - object to use index to refer to.  It is assumed that obj_p has not
//             already been associated with an index id.
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxIdHolder<_PtrType>::AIdxIdHolder(_PtrType * obj_p) :
  m_idx_id(AIdxPtr<_PtrType>::get_table()->acquire(obj_p))
  {
  }

//---------------------------------------------------------------------------------------
// Constructor using specific index
// Arg         obj_p - object to use index to refer to
// Arg         idx - specific index location to use.  If it is free, it will create a
//             full index id.  If it has been reserved, it will use the index id already
//             created.
// See:        AIdxPtrTable<>::reserve()
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxIdHolder<_PtrType>::AIdxIdHolder(
  _PtrType * obj_p,
  uint16_t   idx
  ) :
  m_idx_id(AIdxPtr<_PtrType>::get_table()->acquire_specific(idx, obj_p))
  {
  }

//---------------------------------------------------------------------------------------
// Constructor using specific index
// Arg         obj_p - object to use index to refer to
// Arg         idx_id - previously reserved index id to use.
// See:        AIdxPtrTable<>::reserve()
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxIdHolder<_PtrType>::AIdxIdHolder(
  _PtrType * obj_p,
  uint32_t   idx_id
  ) :
  m_idx_id(idx_id)
  {
  AIdxPtr<_PtrType>::get_table()->use_reserved(idx_id, obj_p);
  }

//---------------------------------------------------------------------------------------
// Destructor - releases index from currently associated object
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxIdHolder<_PtrType>::~AIdxIdHolder()
  {
  AIdxPtr<_PtrType>::get_table()->release(m_idx_id);
  }

//---------------------------------------------------------------------------------------
// Gets the index id that is being held
// Returns:    Index id
// Author(s):   Conan Reis
template<class _PtrType>
inline AIdxIdHolder<_PtrType>::operator uint32_t() const
  {
  return m_idx_id;
  }

//---------------------------------------------------------------------------------------
// Gets the index id that is being held
// Returns:    Index id
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdxIdHolder<_PtrType>::get_idx_id() const
  {
  return m_idx_id;
  }

//---------------------------------------------------------------------------------------
// Gets the index that is being held - used for serialization
// Returns:    Index
// Author(s):   Conan Reis
template<class _PtrType>
inline uint16_t AIdxIdHolder<_PtrType>::get_idx() const
  {
  return uint16_t(m_idx_id & AIdxId_index_mask);
  }

//---------------------------------------------------------------------------------------
// Releases index from currently associated object
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdxIdHolder<_PtrType>::release()
  {
  AIdxPtr<_PtrType>::get_table()->release(m_idx_id);
  m_idx_id = AIdxId_null;
  }

//---------------------------------------------------------------------------------------
// Releases index from currently associated object if there is one and gets
//             a new index id.
// Arg         obj_p - object to use index to refer to.
// Author(s):   Conan Reis
template<class _PtrType>
inline void AIdxIdHolder<_PtrType>::reacquire(_PtrType * obj_p)
  {
  AIdxPtrTable<_PtrType> * table_p = AIdxPtr<_PtrType>::get_table();

  if (m_idx_id != AIdxId_null)
    {
    table_p->release(m_idx_id);
    }

  m_idx_id = table_p->acquire(obj_p);
  }


//=======================================================================================
// AIdxPtrTable Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
AIdxPtrTable<_PtrType>::AIdxPtrTable(
  uint32_t obj_max // = AIdxId_obj_max_def
  ) :
  m_entries_p(nullptr)
  {
  #ifdef AIDX_PTR_GUARD
  A_VERIFYX(obj_max <= AIdxId_obj_max, a_cstr_format("Tried to create an index pointer table with %u entries,\nbut the maximum possible is %u!", obj_max, AIdxId_obj_max));
  #endif
    
  // Initialize entries

  // Note +1 for null entry
  m_entries_p = new AIdxEntry<_PtrType> [obj_max + 1u];

  AIdxEntry<_PtrType> * entry_p     = m_entries_p;
  AIdxEntry<_PtrType> * entry_end_p = entry_p + obj_max;

  // Zeroth entry always represents nullptr/invalid
  entry_p->m_used     = true;
  entry_p->m_idx_incr = 0u;
  entry_p->m_obj_p    = nullptr;
  entry_p++;

  m_first_free_p = entry_p;

  while (entry_p < entry_end_p)
    {
    entry_p->m_used        = false;
    entry_p->m_idx_incr    = 0u;
    entry_p->m_next_free_p = entry_p + 1;
    entry_p++;
    }

  // Last entry has no next free
  entry_p->m_used        = false;
  entry_p->m_idx_incr    = 0u;
  entry_p->m_next_free_p = nullptr;

  #if defined(AIDX_PTR_STATS) || defined(AIDX_PTR_GUARD)
    m_size      = obj_max + 1u;
    m_count     = 0u;  // null entry not counted
    m_count_max = 0u;
  #endif


  // Connect this table to AIdxPtr<>
  AIdxPtr<_PtrType>::set_table(this);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
AIdxPtrTable<_PtrType>::~AIdxPtrTable()
  {
  delete []m_entries_p;

  // Disconnect this table to AIdxPtr<> - assumes it was the one connected
  AIdxPtr<_PtrType>::set_table(nullptr);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
uint32_t AIdxPtrTable<_PtrType>::acquire(_PtrType * obj_p)
  {
  if (obj_p)
    {
    // This code is similar to reserve() - ensure that changes here are reflected there.
    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(m_first_free_p, a_cstr_format("Tried to associate an object with an index id,n\but all %u index positions are full!", m_size));
    #endif

    #ifdef AIDX_PTR_STATS
      m_count++;
      if (m_count_max < m_count)
        {
        m_count_max = m_count;
        }
    #endif

    AIdxEntry<_PtrType> * entry_p = m_first_free_p;

    m_first_free_p = entry_p->m_next_free_p;

    entry_p->m_used  = true;
    entry_p->m_obj_p = obj_p;

    // Note that the m_idx_incr data member should not need to be incremented since it
    // is incremented when the index id is released.
    return (uint32_t(entry_p->m_idx_incr) << AIdxId_index_bits) + uint32_t(entry_p - m_entries_p);
    }

  return AIdxId_null;
  }

//---------------------------------------------------------------------------------------
// Determines if the index id is still valid - i.e. is it not stale.
// Notes:      This method considers a nullptr value (AIdxId_null) to be 'valid' since it
//             never goes stale.
// Author(s):   Conan Reis
template<class _PtrType>
inline bool AIdxPtrTable<_PtrType>::is_valid(uint32_t idx_id) const
  {
  uint index = idx_id & AIdxId_index_mask;

  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
  #endif

  // Note that the m_used data member should not need to be checked since m_idx_incr is
  // incremented when the index id is released.
  return m_entries_p[index].m_idx_incr == (idx_id >> AIdxId_index_bits);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline _PtrType * AIdxPtrTable<_PtrType>::get_obj(uint32_t idx_id) const
  {
  uint index = idx_id & AIdxId_index_mask;

  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
  #endif

  AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

  // Note that the m_used data member should not need to be checked since m_idx_incr is
  // incremented when the index id is released.
  return (entry_p->m_idx_incr == (idx_id >> AIdxId_index_bits))
    ? entry_p->m_obj_p
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
void AIdxPtrTable<_PtrType>::release(uint32_t idx_id)
  {
  uint index = idx_id & AIdxId_index_mask;

  if (index != AIdxId_null)
    {
    AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
      A_VERIFYX(entry_p->m_idx_incr == (idx_id >> AIdxId_index_bits), a_cstr_format("Tried to release the same index id more than once!\nIndex: %u", index));
    #endif

    // Note that the m_idx_incr data member is incremented here rather than in acquire()
    // - this ensures that m_idx_incr is different as soon as it is released.
    entry_p->m_idx_incr    = (entry_p->m_idx_incr == AIdxId_incr_max) ? 0u : (entry_p->m_idx_incr + 1u);
    entry_p->m_used        = false;
    entry_p->m_next_free_p = m_first_free_p;

    m_first_free_p = entry_p;

    #ifdef AIDX_PTR_STATS
      m_count--;
    #endif
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
void AIdxPtrTable<_PtrType>::remove_from_free_list(AIdxEntry<_PtrType> * entry_p)
  {
  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(m_first_free_p, a_cstr_format("Tried to associate an object with an index id,n\but all %u index positions are full!", m_size));
    A_VERIFYX(!entry_p->m_used, a_cstr_format("Tried to reserve index id at index location %i,\nbut it was already in use!", entry_p - m_entries_p));
  #endif

  entry_p->m_used = true;

  if (m_first_free_p == entry_p)
    {
    m_first_free_p = entry_p->m_next_free_p;
    }
  else
    {
    AIdxEntry<_PtrType> * entry_find_p = m_first_free_p->m_next_free_p;

    // $Revisit - CReis The entry_find_p->m_next_free_p on the end is me being paranoid.
    // The same goes for the assert following.  It should be impossible for this
    // entry *not* to be found, so the end check just makes the iteration
    // unnecessarily slower.  Test it out then remove it.
    while ((entry_find_p->m_next_free_p != entry_p) && entry_find_p->m_next_free_p)
      {
      entry_find_p = entry_find_p->m_next_free_p;
      }

    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(entry_find_p->m_next_free_p == entry_p, "Tried to remove an index entry from the free list,n\but it was not in the free list!");
    #endif

    m_first_free_p = entry_p->m_next_free_p;
    }

  #ifdef AIDX_PTR_STATS
    m_count++;
    if (m_count_max < m_count)
      {
      m_count_max = m_count;
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
uint32_t AIdxPtrTable<_PtrType>::acquire_specific(
  uint16_t   idx,
  _PtrType * obj_p
  )
  {
  uint32_t index = idx;

  if (index != AIdxId_null)
    {
    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
    #endif

    AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

    if (!entry_p->m_used)
      {
      remove_from_free_list(entry_p);
      }
    entry_p->m_obj_p = obj_p;

    return (uint32_t(entry_p->m_idx_incr) << AIdxId_index_bits) + index;
    }

  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(obj_p == nullptr, "Tried to associate a non-nullptr object with the null index id!");
  #endif

  return AIdxId_null;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
uint32_t AIdxPtrTable<_PtrType>::reserve()
  {
  // This code is similar to acquire() - ensure that changes here are reflected there.
  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(m_first_free_p, a_cstr_format("Tried to associate an object with an index id,n\but all %u index positions are full!", m_size));
  #endif

  #ifdef AIDX_PTR_STATS
    m_count++;
    if (m_count_max < m_count)
      {
      m_count_max = m_count;
      }
  #endif

  AIdxEntry<_PtrType> * entry_p = m_first_free_p;

  m_first_free_p = entry_p->m_next_free_p;

  entry_p->m_used  = true;
  entry_p->m_obj_p = nullptr;

  // Note that the m_idx_incr data member should not need to be incremented since it
  // is incremented when the index id is released.
  return (uint32_t(entry_p->m_idx_incr) << AIdxId_index_bits) + uint32_t(entry_p - m_entries_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdxPtrTable<_PtrType>::reserve(int16_t idx)
  {
  uint index = idx;

  if (index != AIdxId_null)
    {
    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
    #endif

    AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

    remove_from_free_list(entry_p);
    entry_p->m_obj_p = nullptr;

    return (uint32_t(entry_p->m_idx_incr) << AIdxId_index_bits) + index;
    }

  return AIdxId_null;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
inline uint32_t AIdxPtrTable<_PtrType>::idx_to_idx_id(int16_t idx)
  {
  uint32_t index = idx;

  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
  #endif

  AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

  if (!entry_p->m_used)
    {
    remove_from_free_list(entry_p);
    entry_p->m_obj_p = nullptr;
    }

  return (uint32_t(entry_p->m_idx_incr) << AIdxId_index_bits) + index;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _PtrType>
void AIdxPtrTable<_PtrType>::use_reserved(
  uint32_t   idx_id,
  _PtrType * obj_p
  )
  {
  uint32_t index = idx_id & AIdxId_index_mask;

  if (index != AIdxId_null)
    {
    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(index < m_size, a_cstr_format("Index id out of bounds - gave %u,\nbut max possible is %u!", index, m_size));
    #endif

    AIdxEntry<_PtrType> * entry_p = m_entries_p + index;

    #ifdef AIDX_PTR_GUARD
      A_VERIFYX(entry_p->m_idx_incr == (idx_id >> AIdxId_index_bits), a_cstr_format("Tried to associate object with index id at index %u,\nbut index id had already been released!", index));
    #endif

    entry_p->m_obj_p = obj_p;
    }

  #ifdef AIDX_PTR_GUARD
    A_VERIFYX(obj_p == nullptr, "Tried to associate a non-nullptr object with the null index id!");
  #endif
  }

