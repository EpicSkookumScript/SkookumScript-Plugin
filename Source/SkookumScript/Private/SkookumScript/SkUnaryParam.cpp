// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Single parameter class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkUnaryParam.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkUnaryParam.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual
// Author(s):   Conan Reis
SkUnaryParam::~SkUnaryParam()
  {
  delete m_default_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         type - type of parameter (SkParameter_unary or SkParameter_unary_default from the
//             first two bits of the binary stream)
// Arg         type_info - type of expression if 'type' is SkParameter_unary_default - from
//             the last 6 bits in the first byte of the binary stream.
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//                4 bytes  - parameter name id
//                5*bytes  - expected class type
//               [2 bytes] - *SkDebug binary* expression source character position
//               [n bytes] - expression binary (if SkParameter_unary_default)
//
//             Note that the first byte created with SkUnaryParam::as_binary() should
//             have already been parsed by SkParameterBase::from_binary_new() and passed
//             in as 'type' and 'type_info'.
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkUnaryParam::SkUnaryParam(
  eSkParameter  type,
  uint8_t           type_info,
  const void ** binary_pp
  ) :
  // 4 bytes - parameter name id
  SkParameterBase(ASymbol::create_from_binary(binary_pp))
  {
  // 5*bytes - expected class type
  m_type_p = SkClassDescBase::from_binary_ref_typed(binary_pp);

  if (type == SkParameter_unary_default)
    {
    eSkExprType expr_type = static_cast<eSkExprType>(type_info);

    // $Revisit - CReis Currently the compiled code binary always includes debug info
    // [2 bytes] - *SkDebug binary* expression source character position
    #if (SKOOKUM & SK_DEBUG)
      uint16_t source_idx = 
    #endif
        A_BYTE_STREAM_UI16_INC(binary_pp);

    // n bytes - expression binary
    m_default_p = SkExpressionBase::from_binary_new(expr_type, binary_pp);

    #if (SKOOKUM & SK_DEBUG)
      m_default_p->m_source_idx = source_idx;
    #endif
    }
  else
    {
    m_default_p = nullptr;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this unary parameter and its components and increments the memory
//             address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//                2 bits   - parameter type (SkParameter_unary or SkParameter_unary_default)
//                6 bits   - type of expression (if SkParameter_unary_default)
//                4 bytes  - parameter name id
//                5*bytes  - expected class type
//               [2 bytes] - *SkDebug binary* expression source character position 
//               [n bytes] - expression binary (if SkParameter_unary_default)
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
void SkUnaryParam::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkUnaryParam::as_binary_length());

  uint8_t ** data_pp = (uint8_t **)binary_pp;
  uint8_t    header;

  if (m_default_p)
    {
    // 6 bits - type of expression (if SkParameter_unary_default)
    // Note that this limits the number of expression types to 63
    header   = static_cast<uint8_t>(m_default_p->get_type());
    header <<= SkParam_type_bits;

    // 2 bits - parameter type (SkParameter_unary or SkParameter_unary_default)
    header  |= SkParameter_unary_default;
    }
  else
    {
    header = SkParameter_unary;
    }

  // 2 bits - parameter type (SkParameter_unary or SkParameter_unary_default)
  // 6 bits - type of expression (if SkParameter_unary_default)
  A_BYTE_STREAM_OUT8(data_pp, &header);

  // 4 bytes - parameter name id
  m_name.as_binary(binary_pp);

  // 5*bytes - expected class type
  m_type_p->as_binary_ref_typed(binary_pp);

  if (m_default_p)
    {
    // $Note - CReis A modified version of this code is in SkExpressionBase::as_binary_typed()
    // - any changes here may need to be reflected there as well.

    // 2 bytes - *ADebug* expression source character position]
    #if (SKOOKUM & SK_DEBUG)
      A_BYTE_STREAM_OUT16(binary_pp, &m_default_p->m_source_idx);
    #else
      // No debug info available so use 0 for index
      uint16_t source_idx = 0u;
      A_BYTE_STREAM_OUT16(binary_pp, &source_idx);
    #endif
    
    // [n bytes] - expression binary
    m_default_p->as_binary(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//                2 bits   - parameter type (SkParameter_unary or SkParameter_unary_default)
//                6 bits   - type of expression (if SkParameter_unary_default)
//                4 bytes  - parameter name id
//                5*bytes  - expected class type
//               [2 bytes] - *SkDebug binary* expression source character position 
//               [n bytes] - expression binary (if SkParameter_unary_default)
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
uint32_t SkUnaryParam::as_binary_length() const
  {
  // When default expression present add extra 2 bytes for debug source index position.
  return (m_default_p ? (5u + 2u + m_default_p->as_binary_length()) : 5u) + m_type_p->as_binary_ref_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this into its source code string equivalent.  This is essentially
//             a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
AString SkUnaryParam::as_code() const
  {
  AString str(nullptr, 128u, 0u);

  str.append(m_type_p->as_code());
  str.append(' ');
  str.append(m_name.as_str_dbg());

  if (m_default_p)
    {
    str.append(": ", 2u);
    str.append(m_default_p->as_code());
    }

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// Gets parameter type.
// Returns:    SkParameter_unary_default or SkParameter_unary
// Modifiers:   virtual - overriding pure method from SkParameterBase
// Author(s):   Conan Reis
eSkParameter SkUnaryParam::get_kind() const
  {
  return m_default_p ? SkParameter_unary_default : SkParameter_unary;
  }

//---------------------------------------------------------------------------------------
//  Returns a new unary parameter with any generic/reflective classes replaced with their
//  finalized/specific class using scope_type as its scope.
//
// #Examples
//   "ThisClass_ arg" with "String" as a scope type becomes "String arg"
//
// See Also  is_generic(), SkInvokableClass: :as_finalized_generic()
// #Author(s) Conan Reis
SkParameterBase * SkUnaryParam::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  SkExpressionBase * default_p = nullptr;

  if (m_default_p)
    {
    default_p = m_default_p->as_copy();
    }

  return SK_NEW(SkUnaryParam)(m_name, m_type_p->as_finalized_generic(scope_type), default_p);
  }

//---------------------------------------------------------------------------------------
// Gets parameter type.
// Returns:    SkParameter_unary_default or SkParameter_unary
// Modifiers:   virtual - overriding pure method from SkParameterBase
// Author(s):   Conan Reis
SkClassDescBase * SkUnaryParam::get_expected_type() const
  {
  return m_type_p;
  }

//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "ThisClass_ arg" with "String" as a scope type becomes "String arg"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkUnaryParam::is_generic() const
  {
  return m_type_p->is_generic();
  }

//---------------------------------------------------------------------------------------
// Determines whether this parameter has a default expression if it is
//             omitted during an invocation call.
// Returns:    true if it has a default, false if not
// Modifiers:   virtual - overriding pure method from SkParameterBase
// Author(s):   Conan Reis
bool SkUnaryParam::is_defaultable() const
  {
  return (m_default_p != nullptr);
  }

//---------------------------------------------------------------------------------------
//  Sets the default expression of the unary parameter
// Arg          expr_p - pointer to expression to use whenever this parameter is not
//              explicitly passed an expression.
// Author(s):    Conan Reis
void SkUnaryParam::set_default_expr(SkExpressionBase * expr_p)
  {
  // It is a dynamic object, so the old one should be deleted if present
  delete m_default_p;
  m_default_p = expr_p;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// Returns:    amount of dynamic memory in bytes
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkUnaryParam::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkUnaryParam, 0u));

  if (m_default_p)
    {
    m_default_p->track_memory(mem_stats_p);
    }
  }
