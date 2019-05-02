// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Validated Object IDs
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// SkObjectID Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
A_INLINE SkObjectID::SkObjectID(
  const AString & name,
  SkClass * class_p,
  uint32_t flags
  )
  : m_bind_name(name)
  , m_class_p(class_p)
  , m_flags(flags)
  {
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Construction from binary info
// 
// Params:
//   binary_pp:
//     Pointer to address to read binary serialization info from and to increment
//     - previously filled using as_binary() or a similar mechanism
//     
// Notes:
//   Little error checking is done on the binary info as it assumed that it previously
//   validated upon input.
//   
//     Binary composition:
//               n bytes - bind name
//               4 bytes - class name id
//               1 byte  - flags
//
A_INLINE SkObjectID::SkObjectID(const void ** binary_pp)
  : m_bind_name(binary_pp) // n bytes - bind name
  {
  // 4 bytes - class name id
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // 1 byte - flags (only bother storing first 8 flags)
  m_flags = A_BYTE_STREAM_UI8_INC(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Assignment from binary info - see above
A_INLINE void SkObjectID::assign_binary(const void ** binary_pp)
  {
  // n bytes - bind name
  m_bind_name.assign_binary(binary_pp);

  // 4 bytes - class name id
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // 1 byte - flags (only bother storing first 8 flags)
  m_flags = A_BYTE_STREAM_UI8_INC(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Ensures this is name id is valid for the associated class.
//
// #Author(s) Conan Reis
A_INLINE SkClass * SkObjectID::validate(
  bool validate_deferred // = true
)
  {
  return m_class_p->object_id_validate(m_bind_name, validate_deferred);
  }

#endif // (SKOOKUM & SK_CODE_IN)
