// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Classes for custom/compound expressions
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkLiteralClosure.hpp>
#include <AgogCore/AVCompactArray.hpp>

//=======================================================================================
// Global Structures
//=======================================================================================

// Forward declarations - to avoid unnecessary includes
class SkInvokedExpression;
class SkIdentifierLocal;


//---------------------------------------------------------------------------------------
// Code block - groups 0+ expressions into a single expression.
// They are also used to specify evaluation order.
//
// #Notes
// 
//   [expr1 expr2 expr3]
//   
//   Syntax:
//     code-block = '[' ws [statement {wsr statement} ws] ']'
//     statement  = expression | create-temporary | loop-exit
//     
class SkCode : public SkExpressionBase
  {
  friend class SkParser;
  friend class SkInvokedExpression;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkCode);

    SkCode(uint32_t temp_vars_start_idx) : m_temp_vars_start_idx(temp_vars_start_idx) {}
    virtual ~SkCode() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkCode(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
      virtual eSkSideEffect            get_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
      virtual AString as_callstack_label() const override;
    #endif


  // Methods

    const AVCompactArray<ASymbol> & get_temp_vars() const { return m_temp_vars; }
    uint32_t                        get_temp_vars_start_idx() const { return m_temp_vars_start_idx.m_data_idx; }
    SkIndexed *                     get_temp_vars_start_idx_ptr() { return &m_temp_vars_start_idx; }

    // Overridden from SkExpressionBase

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Called by SkInvokedExpression

    virtual bool invoke_iterate(SkInvokedExpression * iexpr_p, SkInstance ** result_pp = nullptr) const override;
    virtual void invoke_exit(SkInvokedExpression * iexpr_p, SkInvokedExpression * sub_exit_p = nullptr) const override;
    virtual void on_abort(SkInvokedExpression * iexpr_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual SkDebugInfo        get_debug_info(const SkInvokedExpression & iexpr) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Temporary variables made by this code block.
    AVCompactArray<ASymbol> m_temp_vars;

    // Where in the local variable storage this code block stores its local variables
    SkIndexed m_temp_vars_start_idx;

    // Statements in order of iteration
    APCompactArrayFree<SkExpressionBase> m_statements;

  };  // SkCode

//---------------------------------------------------------------------------------------
// Concurrent convergent durational expressions. Call each expression in code block
// simultaneously and do not call the next expression until the *last* expression has
// completed.
//
// #Notes
// 
//   sync
//     [
//     _coro1
//     _coro2
//     _coro3
//     ]
//
// Author(s)  Conan Reis
class SkConcurrentSync : public SkExpressionBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkConcurrentSync);

    SkConcurrentSync(SkExpressionBase * expr_p);
    SkConcurrentSync(APCompactArrayBase<SkExpressionBase> * exprs_p);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkConcurrentSync(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
      virtual AString as_callstack_label() const override;
    #endif


  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            invoke_exit(SkInvokedExpression * iexpr_p, SkInvokedExpression * sub_exit_p = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Durational expressions
    APCompactArrayFree<SkExpressionBase> m_exprs;

  };  // SkConcurrentSync


//---------------------------------------------------------------------------------------
// Concurrent racing expressions.
// 2+ durational expressions run concurrently and next expression executed when *fastest*
// expression returns (result nil, return args of fastest expression bound) and other
// expressions are *aborted*.
//
// #Notes
// 
//   race
//     [
//     _coro1
//     _coro2
//     _coro3
//     ]
//
// This class is very similar to SkInvokeRace and changes to this class might need to be
// reflected there and vice-versa.
class SkConcurrentRace : public SkExpressionBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkConcurrentRace);

    SkConcurrentRace(SkExpressionBase * expr_p);
    SkConcurrentRace(APCompactArrayBase<SkExpressionBase> * exprs_p);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkConcurrentRace(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
      virtual AString as_callstack_label() const override;
    #endif


  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            invoke_exit(SkInvokedExpression * iexpr_p, SkInvokedExpression * sub_exit_p = nullptr) const override;
    virtual bool            invoke_iterate(SkInvokedExpression * iexpr_p, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Durational expressions
    APCompactArrayFree<SkExpressionBase> m_exprs;

  };  // SkConcurrentRace


//---------------------------------------------------------------------------------------
// Durational expression run concurrently with surrounding context and the next
// expression executed immediately (result InvokedCoroutine). expression is essentially a
// closure with captured temporary variables to ensure temporal scope safety. Any return
// arguments will be bound to the captured variables.
//
// #Notes
// 
//   branch _coro
//  
//   branch
//     [
//     _coro1
//     _coro2
//     ]
//   
//   !icoro: branch _coro
//
class SkConcurrentBranch : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkConcurrentBranch);

    SkConcurrentBranch(SkClosureInfoCoroutine * info_p);
    virtual ~SkConcurrentBranch() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkConcurrentBranch(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            invoke_iterate(SkInvokedExpression * iexpr_p, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Reference counted SkClosureInfoCoroutine shared by this branch expression and any closure objects/invokables.
    ARefPtr<SkClosureInfoCoroutine> m_info_p;

  };  // SkConcurrentBranch


//---------------------------------------------------------------------------------------
// Change mind expression
//
// Infer mind from receiver or two expressions with first being mind
// 
//   change update_mind _do_stuff
//   change update_mind robot._do_stuff
class SkChangeMind : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkChangeMind);

    SkChangeMind(SkExpressionBase * mind_p, SkExpressionBase * expr_p);
    virtual ~SkChangeMind() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkChangeMind(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
      virtual AString as_callstack_label() const override;
    #endif

  // Methods

    SkMind * get_changed_mind(const SkInvokedExpression & iexpr) const;

    virtual eSkExprType     get_type() const override;
    virtual SkMind *        get_updater(const SkInvokedExpression & iexpr) const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            invoke_iterate(SkInvokedExpression * iexpr_p, SkInstance ** result_pp = nullptr) const override;
    virtual void            on_abort(SkInvokedExpression * iexpr_p) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    SkExpressionBase * m_mind_p;
    SkExpressionBase * m_expr_p;

  };  // SkChangeMind


//---------------------------------------------------------------------------------------
// Notes      SkookumScript variable bind expression.  Could be a member variable, a
//            parameter variable, or a temporary variable.
//
//            [expression "."] identifier_name ":" expr
//
//            * Instead of this data structure, a bind could have been a method called on
//            an inferred 'this_code' with a method call similar to:
//              this_code bind('identifier_name', expr)
//            However, using this data structure should be more efficient.
// Author(s)  Conan Reis
class SkBind : public SkExpressionBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkBind);

    SkBind();
    SkBind(SkIdentifierLocal * ident_p, SkExpressionBase * expr_p);
    virtual ~SkBind() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkBind(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
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

    SkIdentifierLocal *   m_ident_p;
    SkExpressionBase *    m_expr_p;

  };  // SkBind


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkCode.inl>
#endif