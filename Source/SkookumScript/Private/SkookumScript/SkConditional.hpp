// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Classes for expressions that can be conditionally evaluated/invoked
//=======================================================================================


#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkExpressionBase.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// This is used by both SkConditional and SkCase
struct SkClause
  {
  // Methods

	SK_NEW_OPERATORS(SkClause);

    SkClause(SkExpressionBase * test_p = nullptr, SkExpressionBase * clause_p = nullptr) : m_test_p(test_p), m_clause_p(clause_p) {}
    SkClause(SkClause * clause_p) : m_test_p(clause_p->m_test_p), m_clause_p(clause_p->m_clause_p) { clause_p->m_test_p = nullptr; clause_p->m_clause_p = nullptr; }
    ~SkClause()   { delete m_test_p; delete m_clause_p; }

    void track_memory(AMemoryStats * mem_stats_p) const;

    #if (SKOOKUM & SK_COMPILED_IN)

      SkClause(const void ** binary_pp)
        { m_test_p = SkExpressionBase::from_binary_typed_new(binary_pp); m_clause_p = SkExpressionBase::from_binary_typed_new(binary_pp); }

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if (SKOOKUM & SK_COMPILED_OUT)

      void as_binary(void ** binary_pp) const
        { SkExpressionBase::as_binary_typed(m_test_p, binary_pp); m_clause_p->as_binary_typed(binary_pp); }

      uint32_t as_binary_length() const
        { return SkExpressionBase::as_binary_typed_length(m_test_p) + m_clause_p->as_binary_typed_length(); }

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if defined(SK_AS_STRINGS)
      AString as_code() const;
    #endif


    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const;
      eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr);
    #endif


  // Data Members

    // Expression to test.  If it is nullptr it acts an 'else' clause which always succeeds.
    // When used by SkConditional, it should evaluate to a Boolean (true or false).
    // When used by SkCase, it should evaluate as a valid argument to be passed to the
    // equals operator (=) compared against the comparison expression.
    // It must return immediately when invoked and thus may not contain any calls to
    // coroutines.
    SkExpressionBase * m_test_p;

    // Expression to evaluate if the test succeeds - the "then" of the clause.
    SkExpressionBase * m_clause_p;

  };  // SkClause


//---------------------------------------------------------------------------------------
// Notes      SkookumScript "if" (conditional) expression
// See Also   SkCase, SkWhen, SkUnless
// Examples:      if test_expr1 [clause1]
//              test_expr2  [clause2]
//              else        [else_clause]
// Author(s)  Conan Reis
class SkConditional : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkConditional);

    SkConditional() {}
    virtual ~SkConditional() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkConditional(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if (SKOOKUM & SK_COMPILED_OUT)

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    // Overridden from SkExpressionBase

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

    // If there is more than one clause, the last clause is a default/else clause if its
    // test expression is nullptr
    APCompactArrayFree<SkClause> m_clauses;

  };  // SkConditional


//---------------------------------------------------------------------------------------
// Notes      SkookumScript case (switch) expression
// See Also   SkConditional, SkWhen, SkUnless
// Examples:      case compare_expr
//              test_expr1 [clause1]
//              test_expr2 [clause2]
//              else       [else_clause]
// Author(s)  Conan Reis
class SkCase : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkCase);

    SkCase() : m_compare_expr_p(nullptr) {}
    virtual ~SkCase() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkCase(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if (SKOOKUM & SK_COMPILED_OUT)

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    // Overridden from SkExpressionBase

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

    // Expression to compare the clauses' test expression against
    // Note - this pointer should never be nullptr (representing an implied this) since it
    // would be parsed as a SkConditional if this were implied.
    SkExpressionBase * m_compare_expr_p;

    // If there is more than one clause, the last clause is a default/else clause if its
    // test expression is nullptr
    APCompactArrayFree<SkClause> m_clauses;

  // Class Data Members

  };  // SkCase


//---------------------------------------------------------------------------------------
// SkookumScript "when" (conditional) statement modifier expression - the clause
// expression is only evaluated if the test expression evaluates to true.
// 
// Similar to if expression though *modifies* an earlier statement, has only one
// test/clause and does not need a code block surrounding either the test or the clause.
// 
// #Examples
//   clause when test_expr
//   
// #Notes
//   "if" is not used since the parser would not be able to determine if it was two
//   expressions or a single *modified* expression.
// 
//   Could be written in terms of SkConditional though this is simpler, takes up less
//   memory and is slightly more efficient.
//   
// #See Also   SkUnless, SkConditional, SkCase
// #Author(s)  Conan Reis
class SkWhen : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkWhen);

    SkWhen(SkExpressionBase * test_p, SkExpressionBase * clause_p)  : m_test_p(test_p), m_clause_p(clause_p) {}
    virtual ~SkWhen() override                                               { delete m_test_p; delete m_clause_p; }

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkWhen(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if (SKOOKUM & SK_COMPILED_OUT)

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
      virtual eSkSideEffect            get_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    // Overridden from SkExpressionBase

    virtual eSkExprType     get_type() const override          { return SkExprType_when; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            null_receiver(SkExpressionBase * receiver_p) override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Expression to test - it should evaluate to a Boolean (true or false).
    // It must be an immediate (non-durational) call when invoked.
    SkExpressionBase * m_test_p;

    // Expression to evaluate if the test succeeds - the "then".
    SkExpressionBase * m_clause_p;

  };  // SkWhen


//---------------------------------------------------------------------------------------
// SkookumScript "unless" (negated conditional) statement modifier expression - the clause
// expression is only evaluated if the test expression evaluates to false.
// 
// Same as when expression though test is negated.  Similar to if expression though test
// negated and *modifies* an earlier statement, has only one test/clause and does not need
// a code block surrounding either the test or the clause.
// 
// #Examples
//   clause unless test_expr
//   
// #See Also   SkWhen, SkConditional, SkCase
// #Author(s)  Conan Reis
class SkUnless : public SkWhen
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkUnless);

    SkUnless(SkExpressionBase * test_p, SkExpressionBase * clause_p)  : SkWhen(test_p, clause_p) {}
    virtual ~SkUnless() override                                       {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkUnless(const void ** binary_pp)  : SkWhen(binary_pp)  {}

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    // Overridden from SkExpressionBase

    virtual eSkExprType     get_type() const override               { return SkExprType_unless; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  };  // SkUnless


//---------------------------------------------------------------------------------------
// SkookumScript nil coalescing operator `expr1??expr2` expession - the trial expression
// is evaluated and returned if it is non-nil and the alternate expression is evaluated
// and returned if the trial expression is nil.
// 
// #Examples
//   expr1??expr2
class SkNilCoalescing : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkNilCoalescing);

    SkNilCoalescing(SkExpressionBase * trial_p, SkExpressionBase * alternate_p) : m_trial_p(trial_p), m_alternate_p(alternate_p) {}
    virtual ~SkNilCoalescing() override                        { delete m_trial_p; delete m_alternate_p; }

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkNilCoalescing(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);

    #endif // (SKOOKUM & SK_COMPILED_IN)


    #if (SKOOKUM & SK_COMPILED_OUT)

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override;
      virtual eSkSideEffect            get_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    // Overridden from SkExpressionBase

    virtual eSkExprType     get_type() const override          { return SkExprType_nil_coalescing; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;
    virtual void            null_receiver(SkExpressionBase * receiver_p) override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // Trial expression - use when result is non-nil
    // It must be an immediate (non-durational) call when invoked.
    SkExpressionBase * m_trial_p;

    // Alternate expression - use when result of m_trial_p is nil.
    // It must be an immediate (non-durational) call when invoked.
    SkExpressionBase * m_alternate_p;

  };  // SkNilCoalescing


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkConditional.inl>
#endif

