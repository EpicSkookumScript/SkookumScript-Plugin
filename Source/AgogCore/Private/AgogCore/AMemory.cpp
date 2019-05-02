// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Memory management
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/AMemory.hpp>
#include <AgogCore/AChecksum.hpp>
#include <AgogCore/APSorted.hpp>

#if defined(A_PLAT_PC)
#include <intrin.h>
#endif

//=======================================================================================
// Local Data Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
struct AMemoryInfo
  {
  friend struct AMemoryStats;


  // Nested Structures

    enum eSortType
      {
      SortType_id,
      SortType_size_actual
      };


  // Public Data
    
    // Name of type this info structure represents
    const char * m_type_cstr_p;

    // CRC32 hash id based on m_type_cstr_p
    uint32_t m_type_id;

    // Amount in bytes of constant/data-structure memory used by one of the specified types
    // without debug or validation info.
    uint32_t m_size_static;

    // Amount in bytes of extra constant/data-structure memory used by one of the specified
    // types when extra debugging or validation info is present.
    uint32_t m_size_debug;

    // Totals

      // Total number of the specified type that have been allocated.
      uint32_t m_alloc_count;

      // Accumulated amount in bytes of dynamic/heap allocated memory used by the specified
      // type.  [Accumulated since it is assumed that the size can be different from instance
      // to instance.]
      uint32_t m_size_dynamic_total;

      // Actual amount of dynamic/heap memory actually allocated - which might be larger
      // than requested.  [Accumulated]
      uint32_t m_size_dynamic_total_actual;


  // Methods

    AMemoryInfo(const char * type_cstr_p, uint32_t type_id, uint32_t size_static, uint32_t size_debug) :
      m_type_cstr_p(type_cstr_p),
      m_type_id(type_id),
      m_size_static(size_static),
      m_size_debug(size_debug),
      m_alloc_count(0u),
      m_size_dynamic_total(0u),
      m_size_dynamic_total_actual(0u)
      {
      }

    // For searching by id
    AMemoryInfo(uint32_t type_id) :
      m_type_id(type_id)
      {
      }

    uint32_t get_total_size_static() const            { return m_alloc_count * m_size_static; }
    uint32_t get_total_size_static_actual() const     { return m_alloc_count * AgogCore::get_app_info()->request_byte_size(m_size_static); }
    uint32_t get_total_size() const                   { return get_total_size_static() + m_size_dynamic_total; }
    uint32_t get_total_size_actual() const            { return get_total_size_static_actual() + m_size_dynamic_total_actual; }
    float    get_size_dynamic_average() const         { return (m_alloc_count > 0u) ? float(m_size_dynamic_total) / float(m_alloc_count) : 0.0f; }

    bool operator==(const AMemoryInfo & rhs) const;
    bool operator<(const AMemoryInfo & rhs) const;


  protected:

  // Class Data

    static eSortType ms_sort_type;

  };  // AMemoryInfo

AMemoryInfo::eSortType AMemoryInfo::ms_sort_type = AMemoryInfo::SortType_id;


//---------------------------------------------------------------------------------------
class AMemoryInfoList : public APSorted<AMemoryInfo, AMemoryInfo, ACompareLogicalReverse<AMemoryInfo> >
  {

  };

//=======================================================================================
// AMemoryInfo Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
bool AMemoryInfo::operator==(const AMemoryInfo & rhs) const
  {
  return (ms_sort_type != SortType_id)
    // secondary sort by allocation count
    ? (get_total_size_actual() == rhs.get_total_size_actual()) && (m_alloc_count == rhs.m_alloc_count)
    : m_type_id == rhs.m_type_id;
  }

//---------------------------------------------------------------------------------------
inline bool AMemoryInfo::operator<(const AMemoryInfo & rhs) const
  {
  if (ms_sort_type != SortType_id)
    {
    uint32_t this_size = get_total_size_actual();
    uint32_t rhs_size  = rhs.get_total_size_actual();

    return (this_size == rhs_size)
      ? m_alloc_count < rhs.m_alloc_count  // secondary sort by allocation count
      : this_size < rhs_size;
    }

  return m_type_id < rhs.m_type_id;
  }

//=======================================================================================
// AMemoryStats Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
AMemoryStats::AMemoryStats(
  eTrack track // = Track_type
  ) :
  m_track(track),
  m_size_needed(0u),
  m_size_needed_debug(0u),
  m_info_list_p(nullptr)
  {
  if (track == Track_type)
    {
    m_info_list_p = new AMemoryInfoList;
    }
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
AMemoryStats::~AMemoryStats()
  {
  if (m_track == Track_type)
    {
    m_info_list_p->free();
    delete m_info_list_p;
    }
  }

//---------------------------------------------------------------------------------------
// Sums up memory usage statistics and prints out a summary to the default
//             output.
// Author(s):   Conan Reis
uint32_t AMemoryStats::print_summary(
  uint32_t * debug_bytes_p // = nullptr
  )
  {
  uint32_t result_size       = m_size_needed;
  uint32_t result_size_debug = m_size_needed_debug;

  if (m_track == Track_type)
    {
    // [] - Summation of requested size, {} - Summation of actual size adjusted by fixed size pools
    // 25    8      8       8       8         8         9          9          8       8      12
    // Type  Count  {Size}  [Size]  {Static}  [Static]  {Dynamic}  [Dynamic]  Static  Debug  Dynamic Avg.
    // Totals
    // Extra Debug Memory

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Sort memory info by actual allocated memory size
    AMemoryInfo::ms_sort_type = AMemoryInfo::SortType_size_actual;
    m_info_list_p->sort();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Print Header
    bool fixed_size_pools = AgogCore::get_app_info()->is_using_fixed_size_pools();

    ADebug::print(fixed_size_pools
      ? "                      Type |    Count |   {Size} |   [Size] | {Static} | [Static] | {Dynamic} | [Dynamic] | Static | Debug | Dyn Avg.\n"
        "---------------------------+----------+----------+----------+----------+----------+-----------+-----------+--------+-------+---------\n"
      : "                      Type |    Count |   [Size] | [Static] | [Dynamic] | Static | Debug | Dyn Avg.\n"
        "---------------------------+----------+----------+----------+-----------+--------+-------+---------\n");

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Iterate through types
    AMemoryInfo *  info_p;
    uint32_t       type_count  = m_info_list_p->get_length();
    AMemoryInfo ** info_pp     = m_info_list_p->get_array();
    AMemoryInfo ** info_end_pp = info_pp + type_count;

    uint32_t total_size_actual         = 0u;
    uint32_t total_size                = 0u;
    uint32_t total_size_static_actual  = 0u;
    uint32_t total_size_static         = 0u;
    uint32_t total_size_dynamic_actual = 0u;
    uint32_t total_size_dynamic        = 0u;
    uint32_t total_size_debug          = 0u;

    while (info_pp < info_end_pp)
      {
      info_p = *info_pp;

      if (fixed_size_pools)
        {
        ADebug::print_format(
          " %25s | %8u | %8u | %8u | %8u | %8u | %9u | %9u | %6u | %5u | %7.2f\n",
          info_p->m_type_cstr_p,
          info_p->m_alloc_count,
          info_p->get_total_size_actual(),
          info_p->get_total_size(),
          info_p->get_total_size_static_actual(),
          info_p->get_total_size_static(),
          info_p->m_size_dynamic_total_actual,
          info_p->m_size_dynamic_total,
          info_p->m_size_static,
          info_p->m_size_debug,
          info_p->get_size_dynamic_average());
        }
      else
        {
        ADebug::print_format(
          " %25s | %8u | %8u | %8u | %9u | %6u | %5u | %7.2f\n",
          info_p->m_type_cstr_p,
          info_p->m_alloc_count,
          info_p->get_total_size(),
          info_p->get_total_size_static(),
          info_p->m_size_dynamic_total,
          info_p->m_size_static,
          info_p->m_size_debug,
          info_p->get_size_dynamic_average());
        }

      total_size_actual         += info_p->get_total_size_actual();
      total_size                += info_p->get_total_size();
      total_size_static_actual  += info_p->get_total_size_static_actual();
      total_size_static         += info_p->get_total_size_static();
      total_size_dynamic_actual += info_p->m_size_dynamic_total_actual;
      total_size_dynamic        += info_p->m_size_dynamic_total;
      total_size_debug          += info_p->m_alloc_count * info_p->m_size_debug;

      info_pp++;
      }

    if (fixed_size_pools)
      {
      ADebug::print(
        "---------------------------+----------+----------+----------+----------+----------+-----------+-----------+--------+-------+---------\n");

      ADebug::print_format(
        "                              Totals: | %8u | %8u | %8u | %8u | %9u | %9u\n\n",
        total_size_actual,
        total_size,
        total_size_static_actual,
        total_size_static,
        total_size_dynamic_actual,
        total_size_dynamic);

      ADebug::print(
        "  {} - Actual Size Sum using fixed size pools\n");
      }
    else
      {
      ADebug::print(
        "---------------------------+----------+----------+----------+-----------+--------+-------+---------\n");

      ADebug::print_format(
        "                              Totals: | %8u | %8u | %9u\n\n",
        total_size,
        total_size_static,
        total_size_dynamic);
      }

    ADebug::print(
      "  [] - Requested Size Sum\n  * Global/static memory not tracked\n");

    if (fixed_size_pools)
      {
      ADebug::print_format(
        "\n  Fixed size pool waste: %u\n", total_size_actual - total_size);
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Sort memory info by type
    AMemoryInfo::ms_sort_type = AMemoryInfo::SortType_id;
    m_info_list_p->sort();

    result_size       = fixed_size_pools ? total_size_actual : total_size;
    result_size_debug = total_size_debug;
    }

  ADebug::print_format(
    "\n"
    "  Total needed bytes:    %9u\n"
    "    + Extra debug bytes: %9u\n",
    m_size_needed,
    m_size_needed_debug);

  if (debug_bytes_p)
    {
    *debug_bytes_p = result_size_debug;
    }

  return result_size;
  }

//---------------------------------------------------------------------------------------
// Successively called to accumulate memory used
// Arg         type_cstr_p - name of type.  Usually a class or struct, but can just be a
//             descriptor of any memory allocation type.
// Arg         size_sizeof - byte size of the data structure being allocated (not counting
//             any debug size or dynamic size).  Generally sizeof(Type).  0 if all dynamic
//             or all debug.
// Arg         size_debug - byte size of the optional debug portion of the data structure
//             being allocated.  Often 0 if final version and debug version of type is
// Arg         size_dynamic_needed - minimum amount of dynamic size in bytes needed to
//             store this type.
// Arg         size_dynamic - byte size of dynamic memory currently being taken up which
//             may include additional space that is not actually needed.
// Arg         alloc_count - number of this type being tracked in this call
// Author(s):   Conan Reis
void AMemoryStats::track_memory(
  const char * type_cstr_p,
  uint32_t     size_sizeof,
  uint32_t     size_debug,          // = 0u
  uint32_t     size_dynamic_needed, // = 0u
  uint32_t     size_dynamic,        // = 0u
  uint32_t     alloc_count          // = 1u
  )
  {
  if (m_track == Track_type)
    {
    uint32_t      insert_idx;
    uint32_t      type_id = AChecksum::generate_crc32_cstr(type_cstr_p);
    AMemoryInfo * info_p  = m_info_list_p->get(type_id, AMatch_first_found, &insert_idx);

    if (info_p == nullptr)
      {
      info_p = new AMemoryInfo(type_cstr_p, type_id, size_sizeof, size_debug);

      m_info_list_p->insert(*info_p, insert_idx);
      }

    info_p->m_size_dynamic_total        += size_dynamic;
    info_p->m_size_dynamic_total_actual += AgogCore::get_app_info()->request_byte_size(size_dynamic);
    info_p->m_alloc_count               += alloc_count;
    }

  m_size_needed       += (alloc_count * size_sizeof) + size_dynamic_needed;
  m_size_needed_debug += (alloc_count * size_debug);
  }

//---------------------------------------------------------------------------------------
// Successively called to accumulate *shared* memory used for specified type.
//             This does not affect the allocation count for a type.
// Arg         type_cstr_p - name of type.  Usually a class or struct, but can just be a
//             descriptor of any memory allocation type.
// Arg         size_static - byte size of shared static memory.  0 if all dynamic.
// Arg         size_dynamic_needed - minimum amount of shared dynamic size in bytes needed
//             for this type.
// Arg         size_dynamic - byte size of shared dynamic memory currently being taken up
//             which may include additional space that is not actually needed.
// Author(s):   Conan Reis
void AMemoryStats::track_memory_shared(
  const char * type_cstr_p,
  uint32_t     size_static,
  uint32_t     size_dynamic_needed, // = 0u
  uint32_t     size_dynamic         // = 0u
  )
  {
  track_memory(type_cstr_p, size_static, 0u, size_dynamic_needed, size_dynamic, 0u);
  }


//=======================================================================================
// AMemory Class Data
//=======================================================================================

//=======================================================================================
// AMemory Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Obtain a callstack at the current code location
void ACallStack::set()
  {
  #if defined(A_PLAT_PC) && defined(A_UNOPTIMIZED)
    //WINBASEAPI VOID WINAPI GetCurrentThreadStackLimits(PULONG_PTR LowLimit, PULONG_PTR HighLimit);
    //uintptr_t stack_low_limit, stack_high_limit;
    //GetCurrentThreadStackLimits(&stack_low_limit, &stack_high_limit); // Windows 8 and up
    void * return_p = _AddressOfReturnAddress();
    void ** frame_pp = ((void ***)return_p)[-1];
    void ** stack_high_limit_pp = (void **)return_p + 100000; // Estimate
    uint32_t i;
    for (i = 0; 
         i < A_COUNT_OF(m_stack_p) 
           && ((uintptr_t)frame_pp & 7) == 0
           && frame_pp > (void **)return_p
           && frame_pp < stack_high_limit_pp;
         ++i)
      {
      m_stack_p[i] = frame_pp[1];
      frame_pp = (void **)frame_pp[0];
      }
    for (; i < A_COUNT_OF(m_stack_p); ++i)
      {
      m_stack_p[i] = nullptr;
      }
  #else
    // Unimplemented, just set to NULL
    ::memset(m_stack_p, 0, sizeof(m_stack_p));
  #endif
  }
