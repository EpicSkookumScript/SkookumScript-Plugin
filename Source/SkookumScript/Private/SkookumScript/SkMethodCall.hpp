// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for info used to make a method call/invocation - i.e. method
//             identifier (name/index) and passed argument info.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvocation.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Generic method call base class
class SkMethodCallBase : public SkInvokeBase
  {
  public:

    SK_NEW_OPERATORS(SkMethodCallBase);

    // Overriding from SkInvokeBase

    virtual void track_memory(AMemoryStats * mem_stats_p) const override;

  protected: // This class should never be instantiated by itself, so the constructors are available only to subclasses

    SkMethodCallBase() {}
    SkMethodCallBase(const ASymbol & name, int16_t vtable_index, SkClass * scope_p = nullptr) : SkInvokeBase(name, vtable_index, scope_p) {}
    SkMethodCallBase(const SkInvokableBase * invokable_p, SkClass * scope_p = nullptr) : SkInvokeBase(invokable_p, scope_p) {}
    SkMethodCallBase(SkInvokeBase * call_p) : SkInvokeBase(call_p) {}

    SkInvokedBase * invoke_call_internal(SkInstance * receiver_p, eSkScope method_scope, SkClass * default_call_scope_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, SkInstance ** result_pp) const;

  };  // SkMethodCallBase

//---------------------------------------------------------------------------------------
// Template for specialized method call
template<eSkInvokeType _InvokeType>
class SkMethodCall : public SkMethodCallBase
  {
  friend class SkParser;

  public:
  // Common Methods

    SK_NEW_OPERATORS(SkMethodCall);
    SkMethodCall() {}
    SkMethodCall(const ASymbol & name, int16_t vtable_index, SkClass * scope_p = nullptr) : SkMethodCallBase(name, vtable_index, scope_p) {}
    SkMethodCall(const SkInvokableBase * invokable_p, SkClass * scope_p = nullptr) : SkMethodCallBase(invokable_p, scope_p) {}
    SkMethodCall(SkInvokeBase * call_p) : SkMethodCallBase(call_p) {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkMethodCall(const void ** binary_pp) { assign_binary(binary_pp); }

    #endif // (SKOOKUM & SK_COMPILED_IN)


  // Methods

    // Overriding from SkInvokeBase

    virtual eSkInvokeType   get_invoke_type() const override { return _InvokeType; }
    virtual SkInvokedBase * invoke_call(SkInstance * receiver_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, SkInstance ** result_pp) const override;

  };  // SkMethodCallOnInstance

//---------------------------------------------------------------------------------------
// Shortcuts for the specializations of SkMethodCall

typedef SkMethodCall<SkInvokeType_method_on_instance>       SkMethodCallOnInstance;
typedef SkMethodCall<SkInvokeType_method_on_class>          SkMethodCallOnClass;
typedef SkMethodCall<SkInvokeType_method_on_instance_class> SkMethodCallOnInstanceClass;
typedef SkMethodCall<SkInvokeType_method_on_class_instance> SkMethodCallOnClassInstance;
typedef SkMethodCall<SkInvokeType_method_boolean_and>       SkMethodCallBooleanAnd;
typedef SkMethodCall<SkInvokeType_method_boolean_or>        SkMethodCallBooleanOr;
typedef SkMethodCall<SkInvokeType_method_boolean_nand>      SkMethodCallBooleanNand;
typedef SkMethodCall<SkInvokeType_method_boolean_nor>       SkMethodCallBooleanNor;
typedef SkMethodCall<SkInvokeType_method_assert>            SkMethodCallAssert;
typedef SkMethodCall<SkInvokeType_method_assert_no_leak>    SkMethodCallAssertNoLeak;


//=======================================================================================
// Inline Methods
//=======================================================================================
