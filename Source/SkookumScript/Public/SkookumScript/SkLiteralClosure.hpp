// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Literal expression for closures & anonymous/lambda code/functions
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AVCompactSorted.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkMethod.hpp>
#include <SkookumScript/SkCoroutine.hpp>
#include <SkookumScript/SkNamed.hpp>


//=======================================================================================
// Defines
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Shared info for closure literals and closure objects/invokables which
//            ensures that common data is available even if original expression is
//            unloaded from memory (via being a demand load class, IDE console workspace
//            command, etc.)
// Subclasses SkClosureMethodInfo, SkClosureCoroutineInfo           
// See Also   Other *Closure* classes
// Author(s)  Conan Reis
class SK_API SkClosureInfoBase : public ARefCountMix<SkClosureInfoBase>
  {
  friend class SkLiteralClosure;
  friend class SkParser;

  public:

  // Common Methods

    SkClosureInfoBase() {}
    SkClosureInfoBase(SkClosureInfoBase * info_p);
    virtual ~SkClosureInfoBase();

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkClosureInfoBase(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const;
      virtual uint32_t as_binary_length() const;
    #endif


  // Debugging Methods

    #if defined(SK_AS_STRINGS)
      uint32_t get_captured_name_lengths() const;
    #endif


  // Methods

    const tSkIndexedNames &    get_captured() const  { return m_captured; }

    virtual SkExpressionBase * get_closure_expr() const = 0;
    virtual SkParameters &     get_closure_params() const = 0;
    virtual SkInvokableBase *  get_invokable() const = 0;
    virtual bool               is_method() const     { return true; }
    virtual void               track_memory(AMemoryStats * mem_stats_p) const = 0;
    //virtual SkInvokedBase *   invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;

    virtual void               set_scope(const SkClass * scope_p) = 0;
    virtual void               set_name(const ASymbol & name) = 0;

  #if (SKOOKUM & SK_DEBUG)
    void                       get_member_info(SkMemberInfo * member_info_p) const;
  #endif

  protected:

  // Data Members

    // Captured local variables.
    // $Revisit - CReis - What about scoped variables?
    tSkIndexedNames m_captured;

    // Location where code literal was defined (which is useful for debugging) is stored
    // in SkMethod/SkCoroutine qualifier.  It is set on load - there is no need to
    // serialize it.

  };  // SkClosureInfoBase


//---------------------------------------------------------------------------------------
// Notes      Shared info for closure literals and closure objects/invokables for methods
// See Also   SkClosure*
// Author(s)  Conan Reis
class SK_API SkClosureInfoMethod : public SkClosureInfoBase, public SkMethod
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkClosureInfoMethod);

    SkClosureInfoMethod();
    SkClosureInfoMethod(SkClosureInfoMethod * closure_p);
    virtual ~SkClosureInfoMethod() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkClosureInfoMethod(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      using SkMethod::as_binary; // To make clang happy
      using SkMethod::as_binary_length; // To make clang happy

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


  // Methods

    virtual SkExpressionBase *        get_closure_expr() const override   { return m_expr_p; }
    virtual SkParameters &            get_closure_params() const override { return *m_params_p; }
    virtual SkInvokableBase *         get_invokable() const override      { return const_cast<SkClosureInfoMethod *>(this); }
    virtual void                      track_memory(AMemoryStats * mem_stats_p) const override;

    virtual void                      set_scope(const SkClass * scope_p) override  { SkQualifier::set_scope(scope_p); }
    virtual void                      set_name(const ASymbol & name) override      { ANamed::set_name(name); }

    virtual const SkClosureInfoBase * get_closure_info() const override { return this; }

  protected:

  // Data Members

    // SkQualifier::m_scope_p - indicates class where literal was defined or nullptr if
    //   from outside class hierarchy like workspace window, tool, etc.

    // ANamed::m_name - method name or some sort of location identifier if m_scope_p=nullptr

  };  // SkClosureInfoMethod


//---------------------------------------------------------------------------------------
// Notes      Shared info for closure literals and closure objects/invokables for coroutines
// See Also   SkClosure*
// Author(s)  Conan Reis
class SK_API SkClosureInfoCoroutine : public SkClosureInfoBase, public SkCoroutine
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkClosureInfoCoroutine);

    SkClosureInfoCoroutine();
    SkClosureInfoCoroutine(SkClosureInfoMethod * closure_p);
    virtual ~SkClosureInfoCoroutine() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkClosureInfoCoroutine(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      using SkCoroutine::as_binary; // To make clang happy
      using SkCoroutine::as_binary_length; // To make clang happy

      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


  // Methods

    virtual SkExpressionBase *        get_closure_expr() const override   { return m_expr_p; }
    virtual SkParameters &            get_closure_params() const override { return *m_params_p; }
    virtual SkInvokableBase *         get_invokable() const override      { return const_cast<SkClosureInfoCoroutine *>(this); }
    //virtual SkInvokedBase *         invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual bool                      is_method() const override          { return false; }
    virtual void                      track_memory(AMemoryStats * mem_stats_p) const override;

    virtual void                      set_scope(const SkClass * scope_p) override  { SkQualifier::set_scope(scope_p); }
    virtual void                      set_name(const ASymbol & name) override      { ANamed::set_name(name); }

    virtual const SkClosureInfoBase * get_closure_info() const override { return this; }

  protected:

  // Data Members

   // SkQualifier::m_scope_p - indicates class where literal was defined or nullptr if
    //   from outside class hierarchy like workspace window, tool, etc.

    // ANamed::m_name - method name or some sort of location identifier if m_scope_p=nullptr

  };  // SkClosureInfoCoroutine


//---------------------------------------------------------------------------------------
// Notes      Literal for closures & anonymous/lambda code/functions.
// See Also   SkClosure*
// Author(s)  Conan Reis
class SK_API SkLiteralClosure : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SkLiteralClosure(SkExpressionBase * recv_p = nullptr);
    SkLiteralClosure(SkLiteralClosure * closure_method_p, eSkExprType type);
    virtual ~SkLiteralClosure() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkLiteralClosure(const void ** binary_pp, eSkExprType type);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override  { return this; }
      virtual eSkSideEffect            get_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif


  // Methods

    SkClosureInfoBase *     get_closure_info() const  { return m_info_p; }
    virtual eSkExprType     get_type() const override          { return m_info_p->is_method() ? SkExprType_closure_method : SkExprType_closure_coroutine; }
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  protected:

  // Data Members

    // Specified receiver object or nullptr if local this is captured.
    SkExpressionBase * m_receiver_p;

    // Reference counted closure info shared by this literal expression and any closure
    // objects/invokables.
    ARefPtr<SkClosureInfoBase> m_info_p;

  };  // SkLiteralClosure


//=======================================================================================
// Inline Methods
//=======================================================================================


