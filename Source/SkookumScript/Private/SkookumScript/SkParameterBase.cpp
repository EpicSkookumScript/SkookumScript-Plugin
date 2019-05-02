// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Parameter Abstract Base Class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkParameterBase.hpp>
#include <AgogCore/ABinaryParse.hpp>

#include <SkookumScript/SkGroupParam.hpp>
#include <SkookumScript/SkUnaryParam.hpp>



//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Destructor
// Examples:    called by system
// Notes:       This method is virtual to ensure that subclasses call their appropriate
//              destructor.
// Modifiers:    virtual
// Author(s):    Conan Reis
SkParameterBase::~SkParameterBase()
  {
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Parameter constructor from binary info - creating a SkUnaryParam or a
//             SkookumScript/SkGroupParam.
// Returns:    a dynamically allocated parameter object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//                2 bits  - parameter type (SkParameter_unary, SkParameter_unary_default, or SkParameter_group)
//                6 bits  - parameter type specific info
//                n bytes - parameter type specific info
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkParameterBase * SkParameterBase::from_binary_new(const void ** binary_pp)
  {
  uint8_t header = A_BYTE_STREAM_UI8_INC(binary_pp);

  // 2 bits - parameter type (SkParameter_unary, SkParameter_unary_default, or SkParameter_group)
  eSkParameter type = static_cast<eSkParameter>(header & SkParam_type_mask);

  // 6 bits - parameter type specific info
  header >>= SkParam_type_bits;

  if (type == SkParameter_group)
    {
    return SK_NEW(SkGroupParam)(header, binary_pp);
    }
  else
    {
    return SK_NEW(SkUnaryParam)(type, header, binary_pp);
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)
