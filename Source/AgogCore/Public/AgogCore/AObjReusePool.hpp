// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Object Reuse Pool class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AList.hpp>
#include <AgogCore/AMemory.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#if defined(A_EXTRA_CHECK) && !defined(AORPOOL_NO_USAGE_COUNT) && !defined(AORPOOL_USAGE_COUNT)
  // If this is defined track the current and maximum number of objects used.
  #define AORPOOL_USAGE_COUNT
  // If this is defined store a callstack with each object allocated, and link to list of allocated objects
  // CAUTION: Big overhead!
  //#define AORPOOL_ALLOCATION_TRACKING
#endif


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    The AObjReusePool class template provides a population of contiguous blocks of
//          dynamically objects that call there constructors once when created, then are
//          taken out of the pool and put back on the pool as they are used and finished
//          being used respectively, and then are finally destructed at the end of this
//          objects lifetime.  Additional blocks of objects are allocated as needed.
//
//          allocate() is used effectively as a new.
//          recycle() is used effectively as a delete.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class AObjReusePool<AStringRef>;
// Arg      _ObjectType - the class/type of elements to be pointed to by the array.
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Examples:    
// Author   Conan Reis
template<class _ObjectType>
class AObjReusePool
  {
  public:

  // Public Data Members

    #ifdef AORPOOL_USAGE_COUNT

      // Number of objects currently used / outstanding
      uint32_t m_count_now;

      // Maximum number of objects used at once - i.e. peak usage
      uint32_t m_count_max;

      // Total number of objects, allocated or not
      uint32_t m_count_total;

      // Optional callback that is called just prior to adding
      void (* m_grow_f)(const AObjReusePool & pool);

    #endif

  // Common types

    // Local shorthand for templates
    typedef AObjReusePool<_ObjectType> tObjReusePool;

  // Common Methods

    AObjReusePool(uint32_t initial_size = 0, uint32_t expand_size = 0);
    ~AObjReusePool();


  // Accessor Methods

    uint32_t get_initial_size() const     { return m_initial_size; }
    uint32_t get_expand_size() const      { return m_expand_size; }
    uint32_t get_count_initial() const    { return m_blocks.get_first()->m_size; }
  #ifdef AORPOOL_USAGE_COUNT
    uint32_t get_count_used() const       { return m_count_now; }
    uint32_t get_count_max() const        { return m_count_max; }
    uint32_t get_count_overflow() const;
    uint32_t get_count_available() const  { return m_count_total - m_count_now; }
    uint32_t get_bytes_allocated() const  { return m_count_now * sizeof(_ObjectType); }
  #else
    uint32_t get_count_used() const       { return 0; }
    uint32_t get_count_max() const        { return 0; }
    uint32_t get_count_overflow() const   { return 0; }
    uint32_t get_count_available() const  { return 0; }
    uint32_t get_bytes_allocated() const  { return 0; }
  #endif

  // Modifying Methods

    _ObjectType * allocate();
    void          recycle(_ObjectType * obj_p);
    void          recycle_all(_ObjectType ** objs_a, uint length);

    void          reset(uint32_t initial_size, uint32_t expand_size, bool pre_allocate = true);
    void          add_block();
    void          empty();
    void          remove_expanded();
    void          repool();


  protected:

  // Types

    #ifdef AORPOOL_ALLOCATION_TRACKING

      // For each object, store a callstack and link into list of allocations
      struct AllocObject : _ObjectType, AListNode<AllocObject>
        {
        ACallStack m_call_stack;
        };

      // List of all allocated objects
      AList<AllocObject>  m_allocated_list;

    #else

      // Just allocate the plain objects themselves
      struct AllocObject : _ObjectType {};

    #endif

    struct ObjBlock : AListNode<ObjBlock>
      {
      ObjBlock(uint32_t size) : m_size(size) {}

      // Number of objects contained (stored and optionally initialized) by this block
      uint32_t m_size;

      // The object array - with m_size elements
      // Located right after this header structure in memory
      AllocObject * get_array() const { return reinterpret_cast<AllocObject *>(a_align_up((uintptr_t)(this + 1), 16)); }
      };

  // Methods

    void recycle_all(AllocObject * objs_a, uint length);

  // Data Members

    // Pool of previously constructed objects that are ready for use.
    AllocObject * m_pool_first_p;

    // List of blocks where the objects are actually stored
    // The first list element is the special "initial" block
    AList<ObjBlock> m_blocks;

    // If we ever completely free all memory, and then start allocating again,
    // need to know how big the initial block is supposed to be
    uint32_t m_initial_size;

    // If there is need of additional objects, this is the size to use for additional
    // object blocks.
    uint32_t m_expand_size;

  };  // AObjReusePool



//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         initial_size - initial object population for the reuse pool
// Arg         expand_size - additional number of objects to allocate if all the objects
//             in the reuse pool are in use and more objects are required.
// Author(s):   Conan Reis
template<class _ObjectType>
inline AObjReusePool<_ObjectType>::AObjReusePool(
  uint32_t initial_size,
  uint32_t expand_size
  ) :
  #ifdef AORPOOL_USAGE_COUNT
    m_count_now(0u), m_count_max(0u), m_count_total(0u), m_grow_f(nullptr),
  #endif
  m_pool_first_p(nullptr),
  m_initial_size(initial_size),
  m_expand_size(expand_size)
  {
  if (initial_size)
    {
    add_block();
    }
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
template<class _ObjectType>
inline AObjReusePool<_ObjectType>::~AObjReusePool()
  {
  }

#ifdef AORPOOL_USAGE_COUNT

//---------------------------------------------------------------------------------------
// Determines number of objects used above and beyond the initial amount that
//             was allocated.
// Returns:    number of objects that have overflowed initial allocation
// Author(s):   Conan Reis
template<class _ObjectType>
inline uint32_t AObjReusePool<_ObjectType>::get_count_overflow() const
  {
  return (m_count_max <= m_blocks.get_first()->m_size) ? 0u : (m_count_max - m_blocks.get_first()->m_size);
  }

#endif // AORPOOL_USAGE_COUNT

//---------------------------------------------------------------------------------------
// Retrieves a previously allocated object from the dynamic pool.  This
//             method should be used instead of 'new' because it prevents unnecessary
//             allocations by reusing previously allocated objects.
// Returns:    a dynamic object
// See:        recycle() 
// Notes:      To 'deallocate' an object that was retrieved with this method, use
//             'recycle()' rather than 'delete'.
// Author(s):   Conan Reis
template<class _ObjectType>
inline _ObjectType * AObjReusePool<_ObjectType>::allocate()
  {
  #ifdef AORPOOL_USAGE_COUNT
    if (++m_count_now > m_count_max)
      {
      m_count_max = m_count_now;
      }
  #endif

  // Unlink one object from the linked list of free objects
  AllocObject * obj_p = m_pool_first_p;
  if (!obj_p)
    {
    // No free objects, so make more
    add_block();
    obj_p = m_pool_first_p;
    }
  m_pool_first_p = static_cast<AllocObject *>(*obj_p->get_pool_unused_next());

  #ifdef AORPOOL_ALLOCATION_TRACKING
    obj_p->m_call_stack.set();
    m_allocated_list.append(obj_p);
  #endif

  return obj_p;
  }

//---------------------------------------------------------------------------------------
// Frees up an Object and returns it into the dynamic pool ready for its next
//             use.  This method should be used instead of 'delete' because it prevents
//             unnecessary deallocations by saving previously allocated objects.
// Arg         obj_p - pointer to object to free up and put into the dynamic pool.
// See:        allocate(), recycle_all()
// Notes:      To 'allocate' an object use 'allocate()' rather than 'new'.
// Author(s):   Conan Reis
template<class _ObjectType>
inline void AObjReusePool<_ObjectType>::recycle(_ObjectType * obj_p)
  {
  #ifdef AORPOOL_USAGE_COUNT
    --m_count_now;
  #endif

  #ifdef AORPOOL_ALLOCATION_TRACKING    
    m_allocated_list.remove(static_cast<AllocObject *>(obj_p));
  #endif

  *obj_p->get_pool_unused_next() = m_pool_first_p;
  m_pool_first_p = static_cast<AllocObject *>(obj_p);
  }

//---------------------------------------------------------------------------------------
// Frees up 'length' Objects and returns them into the dynamic pool ready for
//             their next use.  This method should be used instead of 'delete' because it
//             prevents unnecessary deallocations by saving previously allocated objects.
// Arg         objs_a - pointer to the array of objects to free up and put into the
//             dynamic pool.
// See:        recycle(), allocate()
// Notes:      To 'allocate' an object use 'allocate()' rather than 'new'.
// Author(s):   Conan Reis
template<class _ObjectType>
void AObjReusePool<_ObjectType>::recycle_all(
  _ObjectType ** objs_a,
  uint           length
  )
  {
  #ifdef AORPOOL_USAGE_COUNT
    m_count_now -= length;
  #endif

  AllocObject *  next_obj_p = m_pool_first_p;
  AllocObject ** objs_end_a = (AllocObject **)objs_a + length;
  while ((AllocObject **)objs_a < objs_end_a)
    {
    AllocObject * obj_p = static_cast<AllocObject *>(*objs_a++);
    *obj_p->get_pool_unused_next() = next_obj_p;
    next_obj_p = obj_p;

    #ifdef AORPOOL_ALLOCATION_TRACKING    
      m_allocated_list.remove(obj_p);
    #endif
    }
  m_pool_first_p = next_obj_p;
  }

//---------------------------------------------------------------------------------------
// Frees up 'length' Objects and returns them into the dynamic pool
template<class _ObjectType>
void AObjReusePool<_ObjectType>::recycle_all(
  AllocObject * objs_a,
  uint          length
  )
  {
  #ifdef AORPOOL_USAGE_COUNT
    m_count_now -= length;
  #endif

  AllocObject * next_obj_p = m_pool_first_p;
  AllocObject * objs_end_a = objs_a + length;
  while (objs_a < objs_end_a)
    {
    AllocObject * obj_p = objs_a++;
    *obj_p->get_pool_unused_next() = next_obj_p;
    next_obj_p = obj_p;

    #ifdef AORPOOL_ALLOCATION_TRACKING    
      m_allocated_list.remove(obj_p);
    #endif
    }
  m_pool_first_p = next_obj_p;
  }


//---------------------------------------------------------------------------------------
// Empties and resets the pool
template<class _ObjectType>
void AObjReusePool<_ObjectType>::reset(uint32_t initial_size, uint32_t expand_size, bool pre_allocate)
  {
  empty();

  m_initial_size = initial_size;
  m_expand_size  = expand_size;

  if (initial_size && pre_allocate)
    {
    add_block();
    }
  }

//---------------------------------------------------------------------------------------
// Creates and recycles a block of objects to the object pool
// Arg         size - number of objects to allocate in the block
// Author(s):   Conan Reis
template<class _ObjectType>
void AObjReusePool<_ObjectType>::add_block()
  {
  uint32_t size = m_blocks.is_empty() ? m_initial_size : m_expand_size;

  #ifdef AORPOOL_USAGE_COUNT
    // Got more objects
    m_count_now += size;
    m_count_total += size;
    // Notify that the pool has grown
    if (!m_blocks.is_empty() && m_grow_f)
      {
      (m_grow_f)(*this);
      }
  #endif

  // Allocate a new block from the system heap
  ObjBlock * obj_block_p = new(AgogCore::get_app_info()->malloc(a_align_up((uintptr_t)sizeof(ObjBlock), 16) + size * sizeof(AllocObject), "ObjBlock")) ObjBlock(size);
  A_VERIFY_MEMORY(obj_block_p != nullptr, tObjReusePool);

  // Invoke constructors on all objects
  AllocObject * obj_p = obj_block_p->get_array();
  AllocObject * obj_end_p = obj_p + size;
  while (obj_p < obj_end_p)
    {
    new ((void*)obj_p++) AllocObject();
    }

  // Add block to list of blocks
  m_blocks.append(obj_block_p);
  recycle_all(obj_block_p->get_array(), size);
  }

//---------------------------------------------------------------------------------------
// Clears out pools
// See:        remove_expanded()
// Author(s):   Conan Reis
template<class _ObjectType>
void AObjReusePool<_ObjectType>::empty()
  {
  #ifdef AORPOOL_USAGE_COUNT
    A_ASSERTX(!m_count_now, AErrMsg(a_cstr_format("Tried to empty object pool with %u objects still in use!", m_count_now), AErrLevel_internal));
    m_count_now = 0u;
  #endif

  #ifdef AORPOOL_ALLOCATION_TRACKING    
    #ifndef AORPOOL_USAGE_COUNT
      A_ASSERTX(m_allocated_list.is_empty(), AErrMsg("Tried to empty object pool with objects still in use!", AErrLevel_internal));
    #endif
    m_allocated_list.empty();
  #endif

  while (!m_blocks.is_empty())
    {
    AgogCore::get_app_info()->free(m_blocks.pop_last());
    }
    
  m_pool_first_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Removes / frees memory of all expanded / grown objects above and beyond
//             the initial object block.
// See:        empty()
// Author(s):   Conan Reis
template<class _ObjectType>
void AObjReusePool<_ObjectType>::remove_expanded()
  {
  #ifdef AORPOOL_USAGE_COUNT
    A_ASSERTX(!m_count_now, "Tried to destroy object pool with objects still in the pool.");
    m_count_now = m_blocks.is_empty() ? 0u : m_blocks.get_first()->m_size;
  #endif

  while (!m_blocks.is_empty() && m_blocks.get_first() != m_blocks.get_last())
    {
    AgogCore::get_app_info()->free(m_blocks.pop_last());
    }
    
  m_pool_first_p = nullptr;
  if (!m_blocks.is_empty())
    {
    recycle_all(m_blocks.get_first()->get_array(), m_blocks.get_first()->m_size);
    }

  #ifdef AORPOOL_ALLOCATION_TRACKING    
    A_ASSERTX(m_allocated_list.is_empty(), "Tried to destroy object pool with objects still in the pool.");
    m_allocated_list.empty();
  #endif
  }

//---------------------------------------------------------------------------------------
// Removes / frees memory of all expanded / grown objects above and beyond
//             the initial object block and ensures that all of the objects in the
//             initial pool are available whether they were returned for use or not.
// See:        empty()
// Author(s):   Conan Reis
template<class _ObjectType>
void AObjReusePool<_ObjectType>::repool()
  {
  remove_expanded();
  }
