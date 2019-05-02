// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Validated Object IDs
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkExpressionBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// SkookumScript Validated Object Identifier - used to identify/look-up an object
// instance based on its class and name and validated before runtime.
// 
// #Author(s) Conan Reis
class SK_API SkObjectID : public SkExpressionBase
  {
  public:

  // Nested structures

    // Class flags - stored in m_flags
    // Currently serialized out as
    enum eFlag
      {
      Flag_possible    = 1 << 0,  // Class@?'name' Indeterminate find which may return nil or object and does not have error if nil
      Flag_identifier  = 1 << 1,  // Class@#'name' Identifier name result instead of object

      Flag__custom_bit   = 3,       // This bit and up may be used by custom look-up
      Flag__none         = 0,
      Flag__variant_mask = Flag_possible | Flag_identifier,  // Used with eVariant
      Flag__default      = Flag__none
      };

    // The variant/kind of an Object ID
    enum eVariant
      {
      Variant_reference    = Flag__none,     // Class@'name'
      Variant_possible_ref = Flag_possible,  // Class@?'name'
      Variant_identifier   = Flag_identifier // Class@#'name'
      };


  // Public Data Members

    // The name of the object this object ID is referring to
    SkBindName m_bind_name;

    // Class to retrieve object from
    SkClass * m_class_p;

    // Flags - see SkObjectID::eFlag
    uint32_t m_flags;

    // Cached smart pointer to object
    mutable AIdPtr<SkInstance> m_obj_p;

  // Common Methods

    SK_NEW_OPERATORS(SkObjectID);

    SkObjectID(const AString & name, SkClass * class_p, uint32_t flags);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkObjectID(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override  { return this; }
      virtual eSkSideEffect            get_side_effect() const override                { return SkSideEffect_none; }
      SkClass *                        validate(bool validate_deferred = true);
    #endif


    #if defined(SK_AS_STRINGS)
      static  AString as_code(const AString & name, SkClass * class_p, eVariant variant);
      virtual AString as_code() const override { return as_code(m_bind_name.as_string(), m_class_p, get_variant()); }
    #endif


  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    const SkBindName &  get_name() const            { return m_bind_name; }
    eVariant            get_variant() const         { return eVariant(m_flags & Flag__variant_mask); }
    static eVariant     get_variant(uint32_t flags) { return eVariant(flags & Flag__variant_mask); }

  };  // SkObjectID

//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkObjectID.inl>
#endif

