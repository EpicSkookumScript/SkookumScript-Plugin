// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Method parameters & body classes
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

class ASymbol;

#include <SkookumScript/SkInvokableBase.hpp>
#include <SkookumScript/SkInstance.hpp>       // Used by tSkMethodMthd to determine size of method pointer


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declration
class SkInvokedMethod;

//---------------------------------------------------------------------------------------
// Notes      Method parameters & body abstract class
// Subclasses SkMethod, SkMethodFunc, SkMethodMthd
// Author(s)  Conan Reis
class SK_API SkMethodBase : public SkInvokableBase
  {
  public:
  // Common Methods

    SkMethodBase(const ASymbol & name, SkClass * scope_p, uint32_t invoked_data_array_size, uint32_t annotation_flags) : SkInvokableBase(name, scope_p, invoked_data_array_size, annotation_flags) {}
    SkMethodBase(const ASymbol & name, SkClass * scope_p, SkParameters * params_p, uint32_t invoked_data_array_size, uint32_t annotation_flags) : SkInvokableBase(name, scope_p, params_p, invoked_data_array_size, annotation_flags) {}
    SkMethodBase(const ASymbol & name, SkClassDescBase * result_type_p, SkParameterBase * param_p, uint32_t invoked_data_array_size, uint32_t annotation_flags) : SkInvokableBase(name, result_type_p, param_p, invoked_data_array_size, annotation_flags) {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkMethodBase(const ASymbol & name, SkClass * scope_p, const void ** binary_pp) : SkInvokableBase(name, scope_p, binary_pp) {}
    #endif

    #if defined(SK_AS_STRINGS)
      virtual AString as_code_params() const override;
    #endif

  // Methods

    SkMethodBase & assign(const SkMethodBase & method)      { SkInvokableBase::assign(method); return *this; }

    virtual void invoke(SkInvokedMethod * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const = 0;
    virtual bool is_class_member() const override;
    virtual void track_memory(AMemoryStats * mem_stats_p) const = 0;

  };  // SkMethodBase


//---------------------------------------------------------------------------------------
// Notes      Custom/Compound Method Class
// Author(s)  Conan Reis
class SK_API SkMethod : public SkMethodBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkMethod);
    SkMethod(const ASymbol & name, SkClass * scope_p, uint32_t invoked_data_array_size, uint32_t annotation_flags) : SkMethodBase(name, scope_p, invoked_data_array_size, annotation_flags), m_expr_p(nullptr) {}
    SkMethod(const ASymbol & name, SkClass * scope_p, SkParameters * params_p, uint32_t invoked_data_array_size, uint32_t annotation_flags, SkExpressionBase * expr_p = nullptr) : SkMethodBase(name, scope_p, params_p, invoked_data_array_size, annotation_flags), m_expr_p(expr_p) {}
    virtual ~SkMethod() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkMethod(const ASymbol & name, SkClass * scope_p, const void ** binary_pp);
      virtual void assign_binary_no_name(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p) override;
      virtual void copy_to_update_record(SkRoutineUpdateRecord * update_record_p) override;
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp, bool include_name) const override;
      virtual uint32_t as_binary_length(bool include_name) const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


    // Debugging Methods
    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * get_custom_expr() const override;
    #endif


  // Methods

    SkMethod & assign_take(SkMethod * method_p);
    void       set_expression(SkExpressionBase * expr_p);
    void       replace_expression(SkExpressionBase * expr_p);

    // Overridden from SkMethodBase

      virtual void    invoke(SkInvokedMethod * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
      SkInvokedBase * invoke_deferred(SkInvokedMethod * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const;
      virtual void    track_memory(AMemoryStats * mem_stats_p) const override;

    // Overridden from SkInvokableBase

      virtual eSkInvokable get_invoke_type() const override;
      virtual bool         is_bound() const override;
      virtual bool         is_empty() const override;
      virtual bool         is_placeholder() const override;

  protected:

  // Data Members

    // Optimized code block containing the statements to execute when this coroutine is
    // invoked.  It is either a SkCode, SkLiteral(nil), or some other expression.
    SkExpressionBase * m_expr_p;

  };  // SkMethod


//---------------------------------------------------------------------------------------
// Typedef for pointer to C++ method function - which must be a static or global function.
// It is associated with a Skookum method and called whenever the method is invoked.
// 
// Params:  
//   scope_p:
//     Calling scope and working data for the invoked code - this is the path for
//     accessing run-time information, passed arguments, data members, the call stack,
//     etc. - see `SkInvokedMethod` for a description of its methods and data members.
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is `nullptr`, then the result does not need to be returned and
//     only side-effects are desired.  (Default `nullptr`)
//     
// See:   tSkMethodMthd, tSkCoroutineFunc, tSkCoroutineMthd
typedef void (* tSkMethodFunc)(SkInvokedMethod * scope_p, SkInstance ** result_pp);


//---------------------------------------------------------------------------------------
// SkookumScript method wrapper for atomic/C++ function Class
class SK_API SkMethodFunc : public SkMethodBase
  {
  public:
	  SK_NEW_OPERATORS(SkMethodFunc);

  // Public Data Members

    tSkMethodFunc m_atomic_f;

  // Common Methods

    SkMethodFunc(const ASymbol & name, SkClass * scope_p, SkParameters * params_p, uint32_t annotation_flags, tSkMethodFunc atomic_f = nullptr);
    SkMethodFunc(const ASymbol & name, SkClassDescBase * result_type_p, SkParameterBase * param_p, uint32_t annotation_flags, tSkMethodFunc atomic_f = nullptr) : SkMethodBase(name, result_type_p, param_p, 1, annotation_flags), m_atomic_f(atomic_f) {}
    SkMethodFunc(const ASymbol & name, SkClass * scope_p, uint32_t annotation_flags, tSkMethodFunc atomic_f = nullptr) : SkMethodBase(name, scope_p, 0u, annotation_flags), m_atomic_f(atomic_f) {}


  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkMethodFunc(const ASymbol & name, SkClass * scope_p, const void ** binary_pp);
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    void set_function(tSkMethodFunc atomic_f) { m_atomic_f = atomic_f; }

    // Overridden from SkMethodBase

      virtual void invoke(SkInvokedMethod * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
      virtual void track_memory(AMemoryStats * mem_stats_p) const override;

    // Overridden from SkInvokableBase

      virtual eSkInvokable get_invoke_type() const override;
      virtual bool         is_bound() const override;

  };  // SkMethodFunc


//---------------------------------------------------------------------------------------
// Typedef for pointer to C++ method function - which must be an instance method from a
// class derived from `SkInstance`.
// It is associated with a Skookum method and called whenever the method is invoked.
// 
// Params:  
//   scope_p:
//     Calling scope and working data for the invoked code - this is the path for
//     accessing run-time information, passed arguments, data members, the call stack,
//     etc. - see `SkInvokedMethod` for a description of its methods and data members.
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is `nullptr`, then the result does not need to be returned and
//     only side-effects are desired.  (Default `nullptr`)
// 
// Notes:  
//   *** Pointer to member methods can be of different sizes depending on the class that
//   is used and for different compilers! ***
//     
// See:   tSkMethodFunc, tSkCoroutineFunc, tSkCoroutineMthd
typedef void (SkInstance::* tSkMethodMthd)(SkInvokedMethod * scope_p, SkInstance ** result_pp);


//---------------------------------------------------------------------------------------
// SkookumScript method wrapper for atomic/C++ instance method Class
class SK_API SkMethodMthd : public SkMethodBase
  {
  public:
	  SK_NEW_OPERATORS(SkMethodMthd);

  // Public Data Members

    // $Vital - CReis This sometimes gives the warning C4121: alignment of a member was sensitive to packing
    // Investigate it and fix it or remove warning if it is not useful.
    // *** Pointer to member methods can be of different sizes depending on the class
    // that is used and for different compilers!  On the PC this is 12 bytes ***
    tSkMethodMthd m_atomic_m;

  // Common Methods

    SkMethodMthd(const ASymbol & name, SkClass * scope_p, SkParameters * params_p, uint32_t annotation_flags, tSkMethodMthd atomic_m = nullptr);
    SkMethodMthd(const ASymbol & name, SkClassDescBase * result_type_p, SkParameterBase * param_p, uint32_t annotation_flags, tSkMethodMthd atomic_m = nullptr) : SkMethodBase(name, result_type_p, param_p, 1, annotation_flags), m_atomic_m(atomic_m) {}
    SkMethodMthd(const ASymbol & name, SkClassDescBase * result_type_p, uint32_t annotation_flags, tSkMethodMthd atomic_m = nullptr) : SkMethodBase(name, result_type_p, nullptr, 0, annotation_flags), m_atomic_m(atomic_m) {}


  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkMethodMthd(const ASymbol & name, SkClass * scope_p, const void ** binary_pp);
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    void set_function(tSkMethodMthd atomic_m) { m_atomic_m = atomic_m; }

    // Overridden from SkMethodBase

      virtual void invoke(SkInvokedMethod * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
      virtual void track_memory(AMemoryStats * mem_stats_p) const override;

    // Overridden from SkInvokableBase

      virtual eSkInvokable get_invoke_type() const override;
      virtual bool         is_bound() const override;

  };  // SkMethodMthd


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkMethod.inl>
#endif
