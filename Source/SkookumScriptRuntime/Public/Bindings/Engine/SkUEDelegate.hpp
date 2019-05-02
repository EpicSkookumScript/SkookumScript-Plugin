// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Delegate (= FScriptDelegate) class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "UObject/WeakObjectPtr.h"

#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript Delegate (= FScriptDelegate) class
class SKOOKUMSCRIPTRUNTIME_API SkUEDelegate : public SkClassBindingSimple<SkUEDelegate, FScriptDelegate>
  {
  public:

    static void         register_bindings();
    static SkClass *    get_class();
    static SkInstance * new_instance(const FScriptDelegate & script_delegate);

  protected:

    //---------------------------------------------------------------------------------------
    // Special flavor of SkInstance that handles invocation of Delegates
    class Instance : public SkInstance
      {
      public:
      
        // Empty constructor for vtable stamping
        Instance(eALeaveMemoryUnchanged) : SkInstance(ALeaveMemoryUnchanged) {}

        // This is an invokable instance
        virtual eSkObjectType get_obj_type() const override { return SkObjectType_invokable; }

        // Destructor
        virtual void delete_this() override;

        // Invoke the Delegate represented by this SkInstance
        virtual void invoke_as_method(
          SkObjectBase * scope_p,
          SkInvokedBase * caller_p,
          SkInstance ** result_pp,
          const SkClosureInvokeInfo & invoke_info,
          const SkExpressionBase * invoking_expr_p) const override;
      };

  };

//=======================================================================================
// Inline Function Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

inline SkInstance * SkUEDelegate::new_instance(const FScriptDelegate & script_delegate)
  {
  // Create an instance the usual way
  SkInstance * instance_p = SkClassBindingSimple<SkUEDelegate, FScriptDelegate>::new_instance(script_delegate);

  // Then change v-table to SkUEDelegate::Instance
  new (instance_p) Instance(ALeaveMemoryUnchanged);

  return instance_p;
  }
