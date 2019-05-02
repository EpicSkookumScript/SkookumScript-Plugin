// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Parameter Abstract Base Class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ANamed.hpp>
#include <SkookumScript/Sk.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkExpressionBase;

// SkParameterBase enumerated constants
enum
  {
  // Number of bits used by the eSkParameter when in binary form.  If changed,
  // SkGroupParam_max_class_count defined in SkookumScript/SkGroupParam.h should also change.
  SkParam_type_bits = 2,
  // Bit mask for type info when in binary form
  SkParam_type_mask = 3
  };

// Possible parameter types
enum eSkParameter
  {
  SkParameter_unary                = 0,    // Unary parameter without a default expression
  SkParameter_unary_default        = 1,    // Unary parameter with a default expression using method's scope when evaluated (->)
  //SkParameter_unary_default_caller = 2,  // $Note - CReis Future - Unary parameter with a default expression using the caller's scope when evaluated (=>)
  SkParameter_group                = 3     // Variable length parameter that is grouped into a single List argument
  };

//---------------------------------------------------------------------------------------
// Notes      Parameter Abstract Base Class
// Subclasses SkUnaryParam, SkGroupParam
// Author(s)  Conan Reis
class SK_API SkParameterBase : public ANamed
  {
  public:

  // Common Methods

    explicit SkParameterBase(const ASymbol & name = ASymbol::get_null());
    virtual ~SkParameterBase();

  // Comparison Methods
  
    virtual bool      compare_equal(const SkParameterBase & param) const = 0;
    virtual bool      compare_less(const SkParameterBase & param) const = 0;
    virtual uint32_t  generate_crc32() const = 0;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_OUT)

      virtual void     as_binary(void ** binary_pp) const = 0;
      virtual uint32_t as_binary_length() const = 0;

      // Also see from_binary_new()

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const = 0;
    #endif


  // Methods

    virtual SkExpressionBase * get_default_expr() const  { return nullptr; }
    virtual eSkParameter       get_kind() const = 0;
    virtual bool               is_defaultable() const = 0;
    virtual void               track_memory(AMemoryStats * mem_stats_p) const = 0;

    // Type-checking Methods

      virtual SkParameterBase * as_finalized_generic(const SkClassDescBase & scope_type) const = 0;
      virtual SkClassDescBase * get_expected_type() const = 0;
      virtual bool              is_generic() const = 0;

  // Class Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      static SkParameterBase * from_binary_new(const void ** binary_pp);
    #endif

  };  // SkParameterBase


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Constructor
// Returns:     itself
// Arg          name - name of the parameter (Default ASymbol::ms_null)
// Author(s):    Conan Reis
inline SkParameterBase::SkParameterBase(
  const ASymbol & name // = ASymbol::ms_null
  ) :
  ANamed(name)
  {
  }
