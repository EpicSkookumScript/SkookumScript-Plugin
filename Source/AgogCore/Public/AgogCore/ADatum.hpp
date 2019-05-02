// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADatum class - Objects that need to be persistent (saved and loaded), or converted to
// to a contiguous memory block form should have conversion methods to and from ADatum.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>
#include <AgogCore/AMemory.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class AString;  
template<class _ObjectType> class AObjReusePool;


//---------------------------------------------------------------------------------------
// ADatum enumerated constants
enum
  {
  ADatum_header_size      = sizeof(uint32_t),  // Size in bytes of a ADatum header in the buffer that precedes the data
  ADatum_use_current_size = ADef_uint32
  };


//---------------------------------------------------------------------------------------
// Any objects that need to be persistent (saved and loaded), or converted to a
// contiguous memory block form should have conversion methods to and from ADatum.
class A_API ADatum
  {
  public:

  // Common methods

    A_NEW_OPERATORS(ADatum);

    explicit ADatum(uint32_t data_length = 0u);
    explicit ADatum(const void * data_p, uint32_t data_length = ALength_in_header, uint32_t size = 0u, eATerm term = ATerm_short);
    explicit ADatum(int32_t data_length_pair_count, ...);
    ADatum(const ADatum & datum);
    ~ADatum();
    ADatum & operator=(const ADatum & datum);
    // $Revisit - CReis add stream constructor?

  // Converter Methods

    AString as_string() const;
    operator const uint8_t * () const;

  // Accessor methods

    const uint8_t * get_buffer() const;
    uint8_t *       get_buffer_writable(bool keep_data = true, uint32_t min_length = ADatum_use_current_size);
    const uint8_t * get_data() const;
    uint8_t *       get_data_writable(bool keep_data = true, uint32_t min_length = ADatum_use_current_size);
    uint8_t *       get_data_end_writable(uint32_t append_data_length, bool min_expand = true);
    uint32_t        get_data_length() const;
    uint32_t        get_length() const;
    uint32_t        get_size() const;
    bool            is_empty() const;
    void            ref_count_increment();
    void            set_buffer(const void * data_p = nullptr, uint32_t data_length = ALength_in_header, uint32_t size = 0u);
    void            set_data(const void * data_p, uint32_t data_length);
    void            set_data_length(uint32_t data_length);

  // Modifying Methods

    void empty();
    void ensure_size(uint32_t min_data_length, bool keep_data = true, bool min_expand = true);

  // Non-Modifying Methods

  // Class Methods

    static uint8_t * alloc_buffer(uint32_t buffer_size);
    static void      free_buffer(uint8_t * buffer_p);

  protected:
  // Internal Classes

    struct Reference
      {
      A_NEW_OPERATORS(Reference);

      Reference(uint32_t data_length);
      Reference(void * buffer_p, uint32_t size, eATerm term = ATerm_short);

      void decrement();

      uint32_t  m_references;
      uint32_t  m_size;
      uint8_t * m_buffer_p;

      // Specifies whether m_buffer_p should be deallocated (ATerm_short) or not (ATerm_long).
      eATerm m_term;
      };

  // Internal Class Methods

    static uint32_t calc_size(uint32_t needed, uint32_t current_size);
    static uint32_t bytes4_init();

  // Data members

    Reference * m_dref_p;

  // Class Data Members

    // Size 4 in stream byte order used for default/empty ADatum objects
    static uint32_t ms_bytes4;

  };  // ADatum

      
//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/ADatum.inl>
#endif
