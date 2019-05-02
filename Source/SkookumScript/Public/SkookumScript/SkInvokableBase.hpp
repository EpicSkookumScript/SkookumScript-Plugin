// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Invokable parameters & body class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkQualifier.hpp>
#include <SkookumScript/SkParameters.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
struct SkApplyExpressionBase;
class  SkExpressionBase;
struct SkRoutineUpdateRecord;
class  SkClosureInfoBase;

//---------------------------------------------------------------------------------------
// Notes      SkookumScript InvokableBase
// Subclasses SkMethodBase (SkMethod, SkMethodFunc, SkMethodMthd), SkCoroutineBase (SkCoroutine,
//            SkCoroutineMthd, SkCoroutineFunc)
// Author(s)  Conan Reis
class SK_API SkInvokableBase : public SkQualifier
  {
  public:

  // Common Methods

    SkInvokableBase();
    SkInvokableBase(const ASymbol & name, SkClass * scope_p, uint32_t invoked_data_array_size, uint32_t annotation_flags);
    SkInvokableBase(const ASymbol & name, SkClass * scope_p, SkParameters * params_p, uint32_t invoked_data_array_size, uint32_t annotation_flags);
    SkInvokableBase(const ASymbol & name, SkClassDescBase * result_type_p, SkParameterBase * param_p, uint32_t invoked_data_array_size, uint32_t annotation_flags);
    virtual ~SkInvokableBase();

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkInvokableBase(const ASymbol & name, SkClass * scope_p, const void ** binary_pp);
      virtual void assign_binary_no_name(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p);
      virtual void copy_to_update_record(SkRoutineUpdateRecord * update_record_p);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp, bool include_name) const;
      virtual uint32_t as_binary_length(bool include_name) const;
    #endif

  // Methods

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const = 0;
      virtual AString as_code_params() const = 0;
    #endif

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      SkExpressionBase * find_expr_by_pos(uint32_t pos, eSkExprFind type = SkExprFind_all) const;
      SkExpressionBase * find_expr_on_pos(uint32_t pos, eSkExprFind type = SkExprFind_all) const;
      SkExpressionBase * find_expr_span(uint32_t source_idx, uint32_t * idx_begin_p = nullptr, uint32_t * idx_end_p = nullptr, eSkExprFind type = SkExprFind_all) const;
      void               get_expr_span(const SkExpressionBase & expr, uint32_t * idx_begin_p, uint32_t * idx_end_p) const;
      eSkMember          get_member_type() const;
      eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p);
      const tSkAkas &    get_akas() const { return m_akas; }
      void               set_akas(tSkAkas && akas) { m_akas = akas; }
    #endif

    AString                           as_string_name(bool qualified = true) const;
    SkInvokableBase &                 assign(const SkInvokableBase & invokable);
    virtual SkExpressionBase *        get_custom_expr() const;
    const SkParameters &              get_params() const                         { return *m_params_p; }
    SkParameters &                    get_params()                               { return *m_params_p; }
    void                              set_params(SkParameters * params_p)        { m_params_p = params_p; }
    uint16_t                          get_invoked_data_array_size() const        { return m_invoked_data_array_size; }
    void                              set_invoked_data_array_size(uint32_t invoked_data_array_size) { m_invoked_data_array_size = (uint16_t)invoked_data_array_size; }
    uint32_t                          get_annotation_flags() const               { return m_annotation_flags; }
    void                              set_annotation_flags(uint32_t flags)       { m_annotation_flags = flags; }
    uint16_t                          get_user_data() const                      { return m_user_data; }
    void                              set_user_data(uint32_t user_data)          { m_user_data = (uint16_t)user_data; }
    virtual eSkInvokable              get_invoke_type() const = 0;
    virtual bool                      is_class_member() const = 0;
    virtual bool                      is_bound() const = 0;
    virtual bool                      is_empty() const;
    virtual bool                      is_placeholder() const;
    virtual const SkClosureInfoBase * get_closure_info() const;
    //virtual SkInvokableBase * evaluate(SkObjectBase * scope_p, SkInvokedBase * caller_p) = 0;
    //virtual SkInvokableBase * give_result(SkInstance * result_p, SkInvokableBase * sub_expr_p, SkInvokedBase * caller_p) = 0;

  protected:

  // Data Members

    ARefPtr<SkParameters> m_params_p;

    uint16_t              m_invoked_data_array_size; // How many entries we need in data storage array in the invoked method/coroutine
    uint16_t              m_user_data;               // Custom data storage to be utilized by the engine integration
    uint32_t              m_annotation_flags;        // Which annotations are present on this invokable

  #if (SKOOKUM & SK_DEBUG)
    tSkAkas               m_akas;                    // Alternative names ("aliases") for this invokable (used just in IDE)
  #endif

    // Future: ADebug / content creation data structure for parameter descriptions

  };  // SkInvokableBase


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkInvokableBase.inl>
#endif

