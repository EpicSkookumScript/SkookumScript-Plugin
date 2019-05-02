// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Invoked closure classes: SkInvokeClosureBase, SkInvokeClosureMethod and
//             SkInvokeClosureCoroutine
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <AgogCore/APCompactArray.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkIdentifierLocal;

//---------------------------------------------------------------------------------------
class SkInvokeClosureBase : public SkExpressionBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SkInvokeClosureBase(SkExpressionBase * receiver_p, const SkParameters * params_p, APCompactArray<SkExpressionBase> * send_args_p, APCompactArray<SkIdentifierLocal> * return_args_p);
    virtual ~SkInvokeClosureBase() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkInvokeClosureBase(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    virtual void null_receiver(SkExpressionBase * receiver_p) override;
    virtual void track_memory(AMemoryStats * mem_stats_p) const override;

  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif

  protected:

  // Data Members

    // The object that results from the evaluation of this expression is the
    // target/subject/receiver of subroutine 'm_call_p'.  If 'm_receiver_p' is nullptr
    // then 'this' - i.e. the topmost scope - is inferred.
    SkExpressionBase * m_receiver_p;

    // Stores formal parameters and actual arguments for invoking the closure
    SkClosureInvokeInfo m_invoke_info;

  };  // SkInvokeClosureBase


//---------------------------------------------------------------------------------------
class SkInvokeClosureMethod : public SkInvokeClosureBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkInvokeClosureMethod);

    SkInvokeClosureMethod(SkExpressionBase * receiver_p, const SkParameters * params_p, APCompactArray<SkExpressionBase> * send_args_p, APCompactArray<SkIdentifierLocal> * return_args_p)
      : SkInvokeClosureBase(receiver_p, params_p, send_args_p, return_args_p) {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkInvokeClosureMethod(const void ** binary_pp)
        : SkInvokeClosureBase(binary_pp) {}
    #endif

  // Methods

    virtual eSkExprType     get_type() const override                                    { return SkExprType_invoke_closure_method; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override  { return true; }

  };  // SkInvokeClosureMethod

//---------------------------------------------------------------------------------------
class SkInvokeClosureCoroutine : public SkInvokeClosureBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkInvokeClosureCoroutine);

    SkInvokeClosureCoroutine(SkExpressionBase * receiver_p, const SkParameters * params_p, APCompactArray<SkExpressionBase> * send_args_p, APCompactArray<SkIdentifierLocal> * return_args_p)
      : SkInvokeClosureBase(receiver_p, params_p, send_args_p, return_args_p) {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkInvokeClosureCoroutine(const void ** binary_pp)
        : SkInvokeClosureBase(binary_pp) {}
    #endif

  // Methods

    virtual eSkExprType     get_type() const override                                    { return SkExprType_invoke_closure_coroutine; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool            is_immediate(uint32_t * durational_idx_p = nullptr) const override;

  };  // SkInvokeClosureCoroutine


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkInvokeClosure.inl>
#endif

