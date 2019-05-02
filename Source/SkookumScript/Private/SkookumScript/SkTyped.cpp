// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Typed name and typed data classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkTyped.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkTyped.inl>
#endif


//=======================================================================================
// SkTypedName Method Definitions
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
//               4 bytes - name id
//               5 bytes - class type
//
// Author(s):   Conan Reis
void SkTypedName::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkTypedName::as_binary_length());

  // 4 bytes - name id
  m_name.as_binary(binary_pp);

  // 5 bytes - class type
  m_type_p->as_binary_ref_typed(binary_pp);
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
//               5 bytes - class type
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
SkTypedName::SkTypedName(const void ** binary_pp) :
  // 4 bytes - name id
  ANamed(ASymbol::create_from_binary(binary_pp)),
  // 5 bytes - class type
  m_type_p(SkClassDescBase::from_binary_ref_typed(binary_pp))
  {
  }

//---------------------------------------------------------------------------------------
// Assignment from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - name id
//               5 bytes - class type
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkTypedName::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - name id
  m_name = ASymbol::create_from_binary(binary_pp);

  // 5 bytes - class type
  m_type_p = SkClassDescBase::from_binary_ref_typed(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

//=======================================================================================
// SkTypedNameRaw Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkTypedNameRaw::SkTypedNameRaw() 
  : m_bind_name(AString::ms_empty)
  , m_raw_data_info(SkRawDataInfo_Invalid)
  {
  }

//---------------------------------------------------------------------------------------

SkTypedNameRaw::SkTypedNameRaw(const ASymbol & name, const SkClassDescBase * type_p, const AString & bind_name) 
  : SkTypedName(name, type_p)
  , m_bind_name(bind_name)
  , m_raw_data_info(SkRawDataInfo_Invalid)
  {
  }

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------

SkTypedNameRaw::SkTypedNameRaw(const void ** binary_pp)
  : SkTypedName(binary_pp)
  , m_bind_name(binary_pp)
  , m_raw_data_info(SkRawDataInfo_Invalid)
  {
  }

//---------------------------------------------------------------------------------------

void SkTypedNameRaw::assign_binary(const void ** binary_pp)
  {
  SkTypedName::assign_binary(binary_pp);
  m_bind_name.assign_binary(binary_pp);
  m_raw_data_info = SkRawDataInfo_Invalid;
  }

#endif

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------

void SkTypedNameRaw::as_binary(void ** binary_pp) const
  {
  SkTypedName::as_binary(binary_pp);
  m_bind_name.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------

uint32_t SkTypedNameRaw::as_binary_length() const
  {
  return SkTypedName::as_binary_length() 
       + m_bind_name.as_binary_length();
  }

#endif