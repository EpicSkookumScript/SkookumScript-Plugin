// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class Wrapper for a method/coroutine and the receiver object (this) that it
//             acts on.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkLiteralClosure.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Stores formal parameters and actual arguments for invoking a closure
struct SkClosureInvokeInfo
  {
  SkClosureInvokeInfo() {}
  SkClosureInvokeInfo(const SkParameters * params_p, APCompactArray<SkExpressionBase> * send_args_p, APCompactArray<SkIdentifierLocal> * return_args_p);
  ~SkClosureInvokeInfo();

  #if (SKOOKUM & SK_COMPILED_IN)
    SkClosureInvokeInfo(const void ** binary_pp);
  #endif

  #if (SKOOKUM & SK_COMPILED_OUT)
    void     as_binary(void ** binary_pp) const;
    uint32_t as_binary_length() const;
  #endif

  #if defined(SK_AS_STRINGS)
    AString as_code() const;
  #endif

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const;
    eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr);
  #endif

  void track_memory(AMemoryStats * mem_stats_p) const;

  // Pointer to parameters (e.g. for invoking UE4 delegates - redundant for SkClosures)
  ARefPtr<SkParameters> m_params_p;

  // Array of dynamic expressions to be invoked and have their instance results passed
  // to the method/coroutine.
  //   - Arguments fill the parameters based on the order of their index position
  //   - Arguments that are nullptr indicate that they should be filled with the
  //     corresponding default argument from the parameter list
  //
  // $Revisit - CReis Future: Each default expression could be put directly in place [the
  // same data structure as the parameter list could be used, but a mechanism so that
  // they are not deleted more than once must be determined].  This future plan for
  // argument lists requires that the available invokables (and their parameter lists)
  // are in memory and that it is possible to determine the class scope of the
  // invokable call.  [See SkInvokedContextBase::data_append_args()]
  APCompactArray<SkExpressionBase> m_arguments;

  // Return argument bindings
  //   - Argument identifiers fill the return parameters based on the order of their index position
  //   - Argument identifiers that are nullptr indicate that they are skipped/ignored.
  // $Revisit - [Memory] CReis since few closures use return args it would save memory
  // to have this class without them and a separate SkInvokeClosureRArgs with them.
  APCompactArray<SkIdentifierLocal> m_return_args;
  };

//---------------------------------------------------------------------------------------
// #Description
//   Wrapper for a method/coroutine, the receiver object (this) that it acts on and any
//   captured variables from its source scope (where its literal was defined).
//   $Note - CReis [Hack for less memory+speed] It has different memory behaviour 
//   depending on its capture count:
//     0  used as a "swap-in memory replacement" for SkInstance - it occupies the same
//        space with just a different virtual table.
//     1+ uses its own memory rather than hijacking an SkInstance and includes references
//        to captured variables.
class SK_API SkClosure : public SkInstance
  {
  public:
  // Common Methods

    SK_NEW_OPERATORS(SkClosure);

  // Methods

    AString             as_string() const;
    SkInstance *        get_receiver() const             { return reinterpret_cast<SkInstance *>(m_user_data.m_data.m_ptr[1]); } // $Revisit - MBreyer HACK
    SkClosureInfoBase * get_info() const                 { return reinterpret_cast<SkClosureInfoBase *>(m_user_data.m_data.m_ptr[0]); } // $Revisit - MBreyer HACK
    uint32_t            get_captured_count() const       { return get_info()->get_captured().get_count(); }
    SkInstance **       get_captured_array() const       { return (SkInstance **)(ptrdiff_t(this) + sizeof(SkClosure)); }

    static SkClosure * new_instance(SkClosureInfoBase * closure_info_p, SkInstance * receiver_p);
    virtual void       delete_this() override;

    #if defined(SK_AS_STRINGS)
      virtual AString as_string_debug() const override   { return as_string(); }
    #endif

    virtual eSkObjectType get_obj_type() const override  { return SkObjectType_closure; }
    virtual SkInstance *  get_topmost_scope() const override;
    virtual void          on_no_references() override;

    virtual SkInvokedBase * invoke(
      SkObjectBase * scope_p,
      SkInvokedBase * caller_p,
      SkInstance ** result_pp,
      const SkClosureInvokeInfo & invoke_info,
      const SkExpressionBase * invoking_expr_p) const override;

    virtual void invoke_as_method(
      SkObjectBase * scope_p,
      SkInvokedBase * caller_p,
      SkInstance ** result_pp,
      const SkClosureInvokeInfo & invoke_info,
      const SkExpressionBase * invoking_expr_p) const override;

    //---------------------------------------------------------------------------------------
    // Evaluates the closure as a method with 0 or more arguments and returns immediately
    // 
    // Params:
    //   method_name: name of method to call
    //   args_pp:
    //     Optional pointers to object instances to use as arguments - each one present
    //     should have its reference count incremented and each defaulted/skipped argument
    //     should be a `nullptr` element. If `arg_count` is 0 this is ignored
    //   arg_count:
    //     number of arguments to use in `args_pp`. If it is 0 then no arguments are passed
    //     and `args_pp` is ignored.
    //   caller_p:
    //     object that called/invoked this expression and that may await a result.  If it is
    //     nullptr, then there is no object that needs to be notified when this invocation is
    //     complete.
    //   result_pp:
    //     Pointer to a pointer to store the instance resulting from the invocation of this
    //     expression.  If it is nullptr, then the result does not need to be returned and
    //     only side-effects are desired.
    //     
    // Notes:
    //   Essentially the same as calling `some_closure(---)` in script.
    //   
    // See:
    //   method_call(), method_query(), SkMethodCall<>::invoke_call(), call_destructor(),
    //   call_default_constructor()
    void closure_method_call(
      SkInstance **   args_pp,
      uint32_t        arg_count,
      SkInstance **   result_pp = nullptr,
      SkInvokedBase * caller_p = nullptr
      );

    //---------------------------------------------------------------------------------------
    // Evaluates the closure as a method and returns immediately
    // 
    // Params:
    //   method_name: name of method to call
    //   arg_p:
    //     Optional pointer to object instance to use as an argument.  If it is present it
    //     should have its reference count incremented.  If it is nullptr, then no arguments
    //     are passed.
    //   caller_p:
    //     object that called/invoked this expression and that may await a result.  If it is
    //     nullptr, then there is no object that needs to be notified when this invocation is
    //     complete.
    //   result_pp:
    //     Pointer to a pointer to store the instance resulting from the invocation of this
    //     expression.  If it is nullptr, then the result does not need to be returned and
    //     only side-effects are desired.
    //     
    // Notes:
    //   Essentially the same as calling `some_closure(---)` in script.
    //   This is a convenience method to use instead of `closure_method_call(name, args_pp, --)`
    //   - if more arguments or control is desired, then use it instead
    //   
    // See:
    //   method_call(), method_query(), SkMethodCall<>::invoke_call(), call_destructor(),
    //   call_default_constructor()
    inline void closure_method_call(
      SkInstance *    arg_p     = nullptr,
      SkInstance **   result_pp = nullptr,
      SkInvokedBase * caller_p  = nullptr
      )
      {
      closure_method_call(&arg_p, arg_p ? 1u : 0u, result_pp, caller_p);
      }

    //---------------------------------------------------------------------------------------
    // Evaluates the closure as a method with 0/1 arguments and returns a Boolean `true` or
    // `false` result immediately.
    // 
    // Returns: the result of the method call as `true` or `false`.
    // 
    // Params:
    //   method_name: name of method to call
    //   arg_p:
    //     Optional argument to be passed to method.  If it is nullptr, then no arguments are
    //     passed.
    //   caller_p:
    //     Object that called/invoked this expression and that may await a result.  If it is
    //     `nullptr`, then there is no object that needs to be notified when this invocation
    //     is complete.
    // 
    // Notes:
    //   Essentially the same as calling `!result?: some_closure(---)` in script.
    //   This is a convenience method to use instead of `closure_method_call(name, args_pp, --)`
    //   - if more arguments or control is desired, then use it instead
    // 
    // See: method_query(), SkMethodCall<>::invoke_call(), call_destructor(), call_default_constructor()
    bool closure_method_query(
      SkInstance *    arg_p    = nullptr,
      SkInvokedBase * caller_p = nullptr
      );

  // Class Methods

    // SkookumScript Atomic Methods

      static void register_bindings();

      //static void mthd_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp);
      //static void mthd_dtor(SkInvokedMethod * scope_p, SkInstance ** result_pp);
      static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp);

  protected:

  // Internal Methods

    SkClosure(SkClosureInfoBase * literal_p, SkInstance * receiver_p);

  // Data Members

    // Inherited from SkInstance

      // Store link to closure literal.  If the code that the closure literal is nested in
      // is unloaded *bad things will happen*.  Use get_literal() and get_captured_count()
      // to access it.
      // $Vital - CReis Probably need to ref count closure literal for at least 2 reasons:
      //   1) Dynamic code origin may be unloaded
      //   2) Closure written in workspace window, editor, etc. may have no permanent
      //   store of the code.
      //m_user_data.m_data.m_ptr[0]  - used as SkClosureInfoBase * m_closure_info_p [ref counted]
       
      // Reference counted pointer to the receiver/this for the closure.  Use
      // get_receiver() to access it.
      //m_user_data.m_data.m_ptr[1]  - used as SkInstance * m_receiver_p

      // $Note - CReis [Bit of a hack to save memory] When captured variables present then
      // extra memory past m_user_data is array of reference counted pointers to
      // SkInstance * for each captured variable.  Use get_captured_array() to access them.

  };  // SkClosure

//---------------------------------------------------------------------------------------
// Specialization - also ensures that `SkInvokedBase::get_arg_data<SkClosure>(--)`,
// `scope_p->this_as<SkClosure>()` etc. work properly
// 
// See: SkInstance::as<SkClass>
template<> inline SkClosure * SkInstance::as_data<SkClosure>() const { return static_cast<SkClosure *>(const_cast<SkInstance *>(this)); }


//=======================================================================================
// Inline Methods
//=======================================================================================

