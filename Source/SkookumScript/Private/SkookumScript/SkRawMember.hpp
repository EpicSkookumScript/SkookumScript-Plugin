// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Raw member access functionality
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AVCompactArray.hpp>
#include <SkookumScript/SkExpressionBase.hpp>

//=======================================================================================
// Helper Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Stores info to access a raw member
struct SkRawMemberInfo
  {
  // Data members

    // The class that contains the data of this class member
    // The parser makes sure that the class actually owns the data member
    // so that it can be directly accessed
    SkClass * m_owner_class_p;

    // Index into the the owner class' raw data member array
    uint16_t m_data_idx;

  // Methods 

    SkRawMemberInfo(SkClass * owner_class_p, uint32_t data_idx) : m_owner_class_p(owner_class_p), m_data_idx((uint16_t)data_idx) {}

    const SkTypedNameRaw * get_typed_name() const;

    #if (SKOOKUM & SK_COMPILED_IN)
      SkRawMemberInfo(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const;
    #endif

  };

//---------------------------------------------------------------------------------------
// Handle to access an actual raw data member
class SkRawMemberHandle
  {
  public:
    
    SkRawMemberHandle(const SkRawMemberInfo & raw_member_info, SkInstance * owner_p);

    SkInstance *  new_instance() const;
    void          assign(SkInstance * value_p) const;

  protected:

    void *                  m_obj_p;
    const SkTypedNameRaw *  m_typed_name_p;
  };

//---------------------------------------------------------------------------------------
// Keeps track of evaluated owner cascade
class SkRawMemberEvaluator : SkRawMemberHandle
  {
  public:

    SkRawMemberEvaluator(const SkRawMemberInfo & raw_member_info, SkInstance * owner_p);
    ~SkRawMemberEvaluator();

    SkInstance * get_member_instance() const { return m_member_instance_p; }

  protected:

    SkInstance * m_member_instance_p;

  };

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Common functionality for raw member access
class SkRawMemberBase : public SkExpressionBase
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkRawMemberBase);

  protected:

    SkRawMemberBase(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade);
    ~SkRawMemberBase();

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkRawMemberBase(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString  as_code() const override;
    #endif

  // Methods

    virtual void       null_receiver(SkExpressionBase * receiver_p) override;

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

    // Class and data index of the member we want to access
    SkRawMemberInfo m_member_info;

    // Optional cascade of enclosing raw member accesses that depend on this one
    // Cascade counts down from owner to member
    AVCompactArray<SkRawMemberInfo> m_raw_owner_cascade;

  };  // SkRawMemberBase


//---------------------------------------------------------------------------------------
// Assignment to raw data member
class SkRawMemberAssignment : public SkRawMemberBase
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkRawMemberAssignment);

    SkRawMemberAssignment(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade, SkExpressionBase * value_expr_p);
    ~SkRawMemberAssignment();

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkRawMemberAssignment(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif

  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // The value assigned to the owner's data member
    SkExpressionBase * m_value_expr_p;

  };  // SkRawMemberAssignment


//---------------------------------------------------------------------------------------
// Modifying invocation on raw data member
class SkRawMemberModifyingInvocation : public SkRawMemberBase
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkRawMemberModifyingInvocation);

    SkRawMemberModifyingInvocation(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade, SkInvokeBase * call_p);
    ~SkRawMemberModifyingInvocation();

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkRawMemberModifyingInvocation(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif

  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Subroutine to call - SkCoroutineCall or SkMethodCallBase
    SkInvokeBase * m_call_p;

  };  // SkRawMemberModifyingInvocation


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkRawMember.inl>
#endif
