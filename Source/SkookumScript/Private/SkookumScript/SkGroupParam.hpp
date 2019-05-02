// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Group parameter class - variable length parameter that has matching
//             arguments grouped into a single List argument
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APArray.hpp>
#include <SkookumScript/SkParameterBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

class SkClassDescBase;  // Pre-declaration


// Group parameter enumerated constants
enum
  {
  // This is the maximum number of classes permissible in the class pattern.  It should
  // change whenever SkParam_type_bits defined in SkookumScript/SkParameterBase.h changes.
  // Its length is limited due to the packing of bits in its compiled binary code form.
  SkGroupParam_max_class_count = 63
  };

//---------------------------------------------------------------------------------------
// Notes      Group parameter class - variable length parameter that has matching
//            arguments grouped into a single List argument
// Author(s)  Conan Reis
class SkGroupParam : public SkParameterBase
  {
  public:
	  SK_NEW_OPERATORS(SkGroupParam);
  // Common Methods

    explicit SkGroupParam(const ASymbol & name = ASymbol::get_null());
    SkGroupParam(SkGroupParam * group_p);
    ~SkGroupParam();

  // Comparison Methods
  
    virtual bool      compare_equal(const SkParameterBase & param) const override;
    virtual bool      compare_less(const SkParameterBase & param) const override;
    virtual uint32_t  generate_crc32() const override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkGroupParam(uint32_t class_count, const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods
 
    void                             append_class(const SkClassDescBase & cls);
    const APArray<SkClassDescBase> & get_pattern() const;
    uint32_t                         get_pattern_length() const;
    SkClassDescBase *                get_pattern_type(uint32_t index) const;
    virtual eSkParameter             get_kind() const override;
    virtual bool                     is_defaultable() const override;
    virtual void                     track_memory(AMemoryStats * mem_stats_p) const override;

    // Type-checking Methods

      virtual SkParameterBase * as_finalized_generic(const SkClassDescBase & scope_type) const override;
      virtual SkClassDescBase * get_expected_type() const override;
      virtual bool              is_generic() const override;
      bool                      is_match_all() const;
      bool                      is_valid_arg_to(const SkGroupParam & param) const;

  protected:

  // Data Members

    // Used as a hint to parser/compiler so that arguments passed are of the type expected.
    // An empty class pattern is equivalent to the default of {Object} - this saves on
    // unnecessary 1 length arrays being allocated.
    // $Revisit - CReis In theory this hint should not be needed during run-time if not
    // debugging or parsing - i.e. if only SK_COMPILED_IN is defined.  Currently only used
    // if SK_CODE_IN, SK_CODE_OUT or SK_COMPILED_OUT is defined.]
    APArray<SkClassDescBase> m_class_pattern;

  };  // SkGroupParam


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkGroupParam.inl>
#endif
