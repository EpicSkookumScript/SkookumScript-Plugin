// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Typed name and typed data classes
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ANamed.hpp>
#include <AgogCore/AIdPtr.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AVCompactSorted.hpp>
#include <SkookumScript/Sk.hpp>

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// SkookumScript Runtime Data Index

struct SK_API SkIndexed
  {
  SK_NEW_OPERATORS(SkIndexed);

  // Common Methods

    SkIndexed() : m_data_idx(0), m_ptr_id(uint16_t(AIdPtr<SkIndexed>::get_next_ptr_id() & 0xffff) | 0x8000) {} // or with 0x8000 to make sure it will _never_ match AIdPtr_null
    SkIndexed(int32_t data_idx) : m_data_idx((int16_t)data_idx), m_ptr_id(uint16_t(AIdPtr<SkIndexed>::get_next_ptr_id() & 0xffff) | 0x8000) {}
    ~SkIndexed() { m_ptr_id = (uint16_t)AIdPtr_null; }

    uint16_t    get_data_idx() const { return m_data_idx; }
    void        set_data_idx(uint32_t data_idx) { m_data_idx = (uint16_t)data_idx; }

  // Conversion Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkIndexed(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const { return 2u; }
    #endif

  // Data Members

    // The data index - might get patched during its lifespan
    int16_t  m_data_idx;

    // So we can use an AIdPtr to point at this
    uint16_t m_ptr_id;
  };
 
//---------------------------------------------------------------------------------------
// SkookumScript Name + Runtime Data Index

struct SK_API SkNamedIndexed : ANamed, SkIndexed
  {
  SK_NEW_OPERATORS(SkNamedIndexed);

  // Common Methods

    SkNamedIndexed() {}
    SkNamedIndexed(const ASymbol & name, int32_t data_idx) : ANamed(name), SkIndexed(data_idx) {}

  // Conversion Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkNamedIndexed(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const { return 4u + SkIndexed::as_binary_length(); }
    #endif

  };

//---------------------------------------------------------------------------------------
// Sorted array of SkNamedIndexed
typedef AVCompactSortedLogical<SkNamedIndexed, ASymbol> tSkIndexedNames;

//=======================================================================================
// Inline Functions
//=======================================================================================
        
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this typed name and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               2 bytes - data_idx
//
// Author(s):   Conan Reis
inline void SkIndexed::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkIndexed::as_binary_length());

  // 2 bytes - data_idx
  A_BYTE_STREAM_OUT16(binary_pp, &m_data_idx);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               2 bytes - data_idx
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
inline SkIndexed::SkIndexed(const void ** binary_pp) :
  // 2 bytes - data_idx
  m_data_idx(A_BYTE_STREAM_UI16_INC(binary_pp)),
  // Create id
  m_ptr_id(uint16_t(AIdPtr<SkIndexed>::get_next_ptr_id() & 0xffff) | 0x8000)
  {
  }

//---------------------------------------------------------------------------------------
// Assignment from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               2 bytes - data_idx
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
inline void SkIndexed::assign_binary(const void ** binary_pp)
  {
  // 2 bytes - data_idx
  m_data_idx = A_BYTE_STREAM_UI16_INC(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this typed name and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - name id
//               2 bytes - data_idx
//
// Author(s):   Conan Reis
inline void SkNamedIndexed::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkNamedIndexed::as_binary_length());

  // 4 bytes - name id
  m_name.as_binary(binary_pp);

  // 2 bytes - data_idx
  SkIndexed::as_binary(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - name id
//               2 bytes - data_idx
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
inline SkNamedIndexed::SkNamedIndexed(const void ** binary_pp) :
  // 4 bytes - name id
  ANamed(ASymbol::create_from_binary(binary_pp)),
  // 2 bytes - data_idx
  SkIndexed(binary_pp)
  {
  }

//---------------------------------------------------------------------------------------
// Assignment from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - name id
//               2 bytes - data_idx
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
inline void SkNamedIndexed::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - name id
  m_name = ASymbol::create_from_binary(binary_pp);

  // 2 bytes - data_idx
  SkIndexed::assign_binary(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)
