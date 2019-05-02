// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for info used to make a method call/invocation - i.e. coroutine
//             identifier (name/index) and passed argument info.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkInvocation.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Info used to make a coroutine call/invocation - i.e. coroutine identifier
//            (name/index) and passed argument info.
// Author(s)  Conan Reis
class SK_API SkCoroutineCall : public SkInvokeBase
  {
  friend class SkParser;

  public:
	  SK_NEW_OPERATORS(SkCoroutineCall);
  // Common Methods

    SkCoroutineCall() {}
    SkCoroutineCall(const SkInvokableBase * invokable_p, SkClass * scope_p = nullptr) : SkInvokeBase(invokable_p, scope_p) {}
    SkCoroutineCall(SkInvokeBase * call_p) : SkInvokeBase(call_p) {}

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)

      SkCoroutineCall(const void ** binary_pp);

    #endif // (SKOOKUM & SK_COMPILED_IN)


  // Methods

    // Overriding from SkInvokeBase

    virtual eSkInvokeType   get_invoke_type() const override;
    virtual SkInvokedBase * invoke_call(SkInstance * receiver_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, SkInstance ** result_pp) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

    // Special

    void                    invoke_schedule(SkInstance * receiver_p, f32 update_interval = SkCall_interval_always, SkInvokedBase * caller_p = nullptr, SkMind * updater_p = nullptr);

  };  // SkCoroutineCall


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkCoroutineCall.inl>
#endif
