// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Memory management
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Defines
//=======================================================================================

#define A_NEW(_CLASS_NAME)    new (#_CLASS_NAME) _CLASS_NAME
#define SK_NEW(_CLASS_NAME)   new (#_CLASS_NAME) _CLASS_NAME

#define A_NEW_OPERATORS(_CLASS_NAME) \
    void* operator new   (size_t size, void *       mem_p )     { return mem_p; }                                            \
    void* operator new   (size_t size, const char * name_p)     { return AgogCore::get_app_info()->malloc(size, name_p); }        \
    void* operator new   (size_t size)                          { return AgogCore::get_app_info()->malloc(size, #_CLASS_NAME); }  \
    void* operator new[] (size_t size, const char * name_p)     { return AgogCore::get_app_info()->malloc(size, name_p); }        \
    void* operator new[] (size_t size)                          { return AgogCore::get_app_info()->malloc(size, #_CLASS_NAME); }  \
    void  operator delete(void *, void *)                       {}                                                           \
    void  operator delete(void * mem_p, const char * name_p)    { AgogCore::get_app_info()->free(mem_p); }                        \
    void  operator delete(void * mem_p)                         { AgogCore::get_app_info()->free(mem_p); }                        \
    friend class AMemory; // to grant new_array() and delete_array() access to this class

#define SK_NEW_OPERATORS(_CLASS_NAME)   A_NEW_OPERATORS(_CLASS_NAME)


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
class AMemoryInfoList;

//---------------------------------------------------------------------------------------
// Structure for gathering a callstack
// Note: Limited platform support - check implementation!
struct A_API ACallStack
  {
  void * m_stack_p[8];

  // Obtain a callstack from current point in code
  void set();
  };

//---------------------------------------------------------------------------------------
struct A_API AMemoryStats
  {
  // Nested Types

    enum eTrack
      {
      Track_type,    // Breakdown by allocation type
      Track_needed   // Just interested in size needed + extra debug size needed
      };

  // Methods

    AMemoryStats(eTrack track = Track_type);
    ~AMemoryStats();

    uint32_t print_summary(uint32_t * debug_bytes_p = nullptr);
    void     track_memory(const char * type_cstr_p, uint32_t size_sizeof, uint32_t size_debug = 0u, uint32_t size_dynamic_needed = 0u, uint32_t size_dynamic = 0u, uint32_t alloc_count = 1u);
    void     track_memory_shared(const char * type_cstr_p, uint32_t size_static, uint32_t size_dynamic_needed = 0u, uint32_t size_dynamic = 0u);

  // Data Members

    // Memory tracking method/detail
    eTrack m_track;

    uint32_t m_size_needed;
    uint32_t m_size_needed_debug;

    // Memory breakdown by type of allocation
    AMemoryInfoList * m_info_list_p;

  };


//---------------------------------------------------------------------------------------
class A_API AMemory
  {
  public:

  // Class Methods

    static uint32_t       request_pointer_count(uint32_t pointers_requested);
    static uint32_t       request_pointer_count_expand(uint32_t pointers_requested);

    template<typename _ObjectType, typename... _ParamClasses>
    static _ObjectType *  new_object(const char * name_p, const _ParamClasses & ... constructor_args);
    template<typename _ObjectType, typename... _ParamClasses>
    static _ObjectType *  new_array(size_t num_objects, const char * name_p, const _ParamClasses & ... constructor_args);

    template<typename _ObjectType>
    static void           delete_object(_ObjectType * obj_p);
    template<typename _ObjectType>
    static void           delete_array(_ObjectType * array_p, size_t num_objects);

  };  // AMemory


//=======================================================================================
// AMemory Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Determines the actual amount of memory allocated that will fit at least
//             the requested number of pointers.  Often memory systems use fixed size
//             pools so though a particular size is requested, a larger size may actually
//             be given and the extra space will be wasted if it is not used.
// Returns:    Actual number pointers that can be stored in the memory allocated
// Arg         pointers_requested - number of pointers requested to allocate space for
// Author(s):   Conan Reis
inline uint32_t AMemory::request_pointer_count(uint32_t pointers_requested)
  {
  return AgogCore::get_app_info()->request_byte_size(pointers_requested * sizeof(void *)) / sizeof(void *);
  }

//---------------------------------------------------------------------------------------
// Determines the actual amount of memory allocated that will fit at least
//             the requested number of pointers plus some expansion space.  Often memory
//             systems use fixed size pools so though a particular size is requested, a
//             larger size may actually be given and the extra space will be wasted if it
//             is not used.
// Returns:    Actual number pointers that can be stored in the memory allocated
// Arg         pointers_requested - number of pointers requested to allocate space for
// Author(s):   Conan Reis
inline uint32_t AMemory::request_pointer_count_expand(uint32_t pointers_requested)
  {
  // The old system used to double the existing count (multiple times if needed) if it
  // needed to expand.
  return (pointers_requested > 0u)
    ? AgogCore::get_app_info()->request_byte_size(((((pointers_requested - 1u) >> 2u) + 1u) << 2u) * sizeof(void *)) / sizeof(void *)
    : 0u;
  }

//---------------------------------------------------------------------------------------
// Similar to operator new, except
// explicitely uses well-defined allocator ms_malloc_func() to get the memory
template<typename _ObjectType, typename... _ParamClasses>
inline _ObjectType * AMemory::new_object(const char * name_p, const _ParamClasses & ... constructor_args)
  {
  // Get memory
  _ObjectType * obj_p = reinterpret_cast<_ObjectType *>(AgogCore::get_app_info()->malloc(sizeof(_ObjectType), name_p));
  // Invoke constructor (this will be optimized out if no constructor exists)
  new ((void*)obj_p) _ObjectType(constructor_args...);
  return obj_p;
  }

//---------------------------------------------------------------------------------------
// Similar to operator new[], except
// 1) does not store array size inside the memory block to not mess with alignment
// 2) uses well-defined allocator ms_malloc_func() to get the memory
// 3) allows constructor arguments to be passed to each object construction
template<typename _ObjectType, typename... _ParamClasses>
inline _ObjectType * AMemory::new_array(size_t num_objects, const char * name_p, const _ParamClasses & ... constructor_args)
  {
  // Get memory
  _ObjectType * array_p = reinterpret_cast<_ObjectType *>(AgogCore::get_app_info()->malloc(num_objects * sizeof(_ObjectType), name_p));
  // Invoke constructors (this will be optimized out if no constructor exists)
  for (size_t i = 0; i < num_objects; ++i)
    {
    new ((void*)(array_p + i)) _ObjectType(constructor_args...);
    }
  return array_p;
  }

//---------------------------------------------------------------------------------------
// Similar to operator delete
// counterpart to new_object()
template<typename _ObjectType>
inline void AMemory::delete_object(_ObjectType * obj_p)
  {
  // Invoke destructor
  obj_p->~_ObjectType();
  // Release memory
  AgogCore::get_app_info()->free(obj_p);
  }

//---------------------------------------------------------------------------------------
// Similar to operator delete[]
// counterpart to new_array()
template<typename _ObjectType>
inline void AMemory::delete_array(_ObjectType * array_p, size_t num_objects)
  {
  // Invoke destructors 
  for (size_t i = 0; i < num_objects; ++i)
    {
    (array_p + i)->~_ObjectType();
    }
  // Release memory
  AgogCore::get_app_info()->free(array_p);
  }
