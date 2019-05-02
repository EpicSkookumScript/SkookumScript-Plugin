// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Identifier for named code objects - i.e. temporary variables, etc.
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
// Notes      SkookumScript Identifier - for local variables and arguments [and
//            potentially variables higher on the call stack].
//            Note that the 'reserved' identifiers "this", "this_class", "this_code",
//            "nil", and classes are handled by SkookumScript/SkLiteral.
// Subclasses SkIdentifierMember, SkIdentifierClassMember, SkIdentifierRawMember
// See Also   SkLiteral
// Author(s)  Conan Reis
class SkIdentifierLocal : public SkExpressionBase, public SkNamedIndexed
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkIdentifierLocal);

    SkIdentifierLocal(const ASymbol & ident_name, uint32_t data_idx);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkIdentifierLocal(const void ** binary_pp) : SkNamedIndexed(binary_pp) {}
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override   { SkNamedIndexed::as_binary(binary_pp); }
      virtual uint32_t as_binary_length() const override             { return SkNamedIndexed::as_binary_length(); }
    #endif

    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override  { return this; }
      virtual eSkSideEffect            get_side_effect() const override                { return SkSideEffect_none; }
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override { return get_name_str_dbg(); }
    #endif


  // Methods

    virtual bool    is_local() const;

    virtual void bind_data(SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false);
    #if (SKOOKUM & SK_DEBUG)
      virtual void bind_data(SkBind * bind_p, SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false);
    #endif

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  };  // SkIdentifierLocal


//---------------------------------------------------------------------------------------
// Notes      SkookumScript Identifier for instance data members.
// See Also   SkIdentifierLocal, SkIdentifierClassMember
// Author(s)  Conan Reis
class SkIdentifierMember : public SkIdentifierLocal
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkIdentifierMember);

    SkIdentifierMember(const ASymbol & member_name, uint32_t data_idx, SkExpressionBase * owner_p);
    virtual ~SkIdentifierMember() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkIdentifierMember(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

    #if (SKOOKUM & SK_CODE_IN)
      virtual eSkSideEffect get_side_effect() const override;
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    virtual bool is_local() const override;

    virtual void bind_data(SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override;
    #if (SKOOKUM & SK_DEBUG)
      virtual void bind_data(SkBind * bind_p, SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override;
    #endif

    SkExpressionBase *      get_owner_expr() const { return m_owner_expr_p; }

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            null_receiver(SkExpressionBase * receiver_p) override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // The object that results from the evaluation of this expression is the owner of the instance member named like this.  
    // If 'm_owner_expr_p' is nullptr then 'this' - i.e. the topmost scope - is inferred.
    SkExpressionBase * m_owner_expr_p;

  };  // SkIdentifierMember


//---------------------------------------------------------------------------------------
// Notes      SkookumScript Identifier for raw instance data members 
//            (represented by direct byte offset into the owner's data structure)
// See Also   SkIdentifierMember, SkIdentifierLocal, SkIdentifierClassMember
// Author(s)  Markus Breyer
class SkIdentifierRawMember : public SkIdentifierMember
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkIdentifierRawMember);

    SkIdentifierRawMember(const ASymbol & member_name, uint32_t data_idx, SkExpressionBase * owner_expr_p, SkClass * owner_class_p) : SkIdentifierMember(member_name, data_idx, owner_expr_p), m_owner_class_p(owner_class_p) {}
    virtual ~SkIdentifierRawMember() override {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkIdentifierRawMember(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

  // Methods

    virtual void bind_data(SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override { SK_ERRORX("bind_data() must not be called on a raw member!"); }
    #if (SKOOKUM & SK_DEBUG)
      virtual void bind_data(SkBind * bind_p, SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override { SK_ERRORX("bind_data() must not be called on a raw member!"); }
    #endif

    SkClass *               get_owner_class() const { return m_owner_class_p; }

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  protected:

  // Data Members

    // The class that contains the raw data entry for this identifier
    // The parser makes sure that the class actually owns the data member
    // so that it can be directly accessed
    SkClass * m_owner_class_p;

  };  // SkIdentifierRawMember


//---------------------------------------------------------------------------------------
// Notes      SkookumScript Identifier for class data members.
//            SkIdentifierMember could have been used instead of this class, but this
//            class skips the look-up in the instance data members.
// See Also   SkIdentifierLocal, SkIdentifierMember
// Author(s)  Conan Reis
class SkIdentifierClassMember : public SkIdentifierLocal
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkIdentifierClassMember);

    SkIdentifierClassMember(const ASymbol & class_member_name, uint32_t data_idx, SkClass * owner_class_p);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkIdentifierClassMember(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    virtual bool is_local() const override;

    virtual void bind_data(SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override;
    #if (SKOOKUM & SK_DEBUG)
      virtual void bind_data(SkBind * bind_p, SkInstance * obj_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, bool return_result = false) override;
    #endif

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  protected:

  // Data Members

    // The class that contains the data of this class member
    // The parser makes sure that the class actually owns the data member
    // so that it can be directly accessed
    SkClass * m_owner_class_p;

  };  // SkIdentifierClassMember


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkIdentifier.inl>
#endif
