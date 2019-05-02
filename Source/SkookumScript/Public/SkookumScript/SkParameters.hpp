// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Formal Parameter List/Interface Class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APSorted.hpp>
#include <AgogCore/APCompactArray.hpp>
#include <AgogCore/ARefCount.hpp>
#include <SkookumScript/SkMemberInfo.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
struct SkApplyExpressionBase;
class  SkClassDescBase;
class  SkParameterBase;
class  SkParser;
struct SkTypedName;

typedef APCompactArrayLogical<SkParameterBase, ASymbol> tSkParamList;
typedef APCompactArrayLogical<SkTypedName, ASymbol>     tSkParamReturnList;


//---------------------------------------------------------------------------------------
// Notes      Formal Parameter/Interface List - used by SkInvokableBase [SkMethodBase
//            (SkMethod, SkMethodFunc, SkMethodMthd), SkCoroutineBase (SkCoroutine,
//            SkCoroutineMthd, SkCoroutineFunc)]
// Author(s)  Conan Reis
class SK_API SkParameters : public ARefCountMix<SkParameters>
  {
  friend class SkParser;  // For quick access during construction with parsing

  public:

  // Nested Structures

    enum eStrFlag
      {
      StrFlag_return      = 1 << 0,  // Include return type
      StrFlag_names_only  = 1 << 1,  // Only output parameter names - no type info or default

      StrFlag__default           = StrFlag_return,
      StrFlag__default_no_return = 0x0,
      StrFlag__simple            = StrFlag_return | StrFlag_names_only
      };

    // Used to differentiate between the 2 parameter types
    enum eType
      {
      Type_send,   // Send parameter prior to ;
      Type_return  // Return parameter following ;
      };

  // Public Class Data

    // $Revisit - CReis Temp count of parameter lists with defaults
    static uint32_t ms_param_count;

  // Common Methods

    SK_NEW_OPERATORS(SkParameters);
    SkParameters();
    explicit SkParameters(SkParameters * params_p);
    ~SkParameters();


  // Comparison Methods

    bool operator==(const SkParameters & params) const;
    bool operator<(const SkParameters & params) const;
    bool is_valid_arg_to(const SkParameters & params) const;
    uint32_t generate_crc32() const;


  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const;
    #endif


    #ifdef SK_AS_STRINGS
      AString as_code(uint32_t str_flags = StrFlag__default) const;
    #endif

  // Methods

    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      SkExpressionBase * find_expr_by_pos(uint32_t source_idx, eSkExprFind type = SkExprFind_all) const;
      eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr);
    #endif

    SkParameters &             assign(SkParameters * parms_p);
    uint                       get_arg_count_total() const            { return m_params.get_length() + m_return_params.get_length(); }
    uint                       get_arg_count_min() const;
    uint                       get_arg_count_min_after_arg1() const;
    const tSkParamList &       get_param_list() const                 { return m_params; }
    const tSkParamReturnList & get_param_return_list() const          { return m_return_params; }
    SkClassDescBase *          get_result_class() const;
    bool                       is_defaulted() const;
    bool                       is_result_params() const               { return m_return_params.is_filled(); }
    bool                       is_sharable() const;
    void                       set_result_type(const SkClassDescBase & rclass);
    void                       track_memory(AMemoryStats * mem_stats_p) const;

    // Type-checking Methods

      bool           is_generic() const;
      bool           is_last_closure() const;
      SkParameters * as_finalized_generic(const SkClassDescBase & scope_type) const;

  // Class Methods

    static SkParameters * get_or_create(SkParameters * params_p);
    static SkParameters * get_or_create(SkClassDescBase * result_type_p, SkParameterBase * param_p = nullptr);

    #if (SKOOKUM & SK_COMPILED_IN)
      static SkParameters * get_or_create(const void ** binary_pp);
    #endif

    static void shared_empty()               { ms_shared_params.free_all(); }  
    static bool shared_ensure_references();
    static void shared_track_memory(AMemoryStats * mem_stats_p);


  protected:

  // Internal Methods

    SkParameters(SkClassDescBase * result_type_p, SkParameterBase * param_p);

    #if (SKOOKUM & SK_COMPILED_IN)
      SkParameters(const void ** binary_pp);
    #endif

  // Data Members

    // Send Parameters (incoming/outgoing) Dynamically allocated unary and group parameters
    tSkParamList m_params;

	// Optional Return Parameters (outgoing only)
    tSkParamReturnList m_return_params;

    // Return type (outgoing only)
    // $Revisit - CReis In theory this hint should not be needed during run-time if not
    // debugging or parsing - i.e. if only SK_COMPILED_IN is defined.  Currently only used
    // if SK_CODE_IN, SK_CODE_OUT or SK_COMPILED_OUT is defined.]
    ARefPtr<SkClassDescBase> m_result_type_p;


  // Class Data Members

    static APSortedLogical<SkParameters> ms_shared_params;

  };  // SkParameters


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkParameters.inl>
#endif
