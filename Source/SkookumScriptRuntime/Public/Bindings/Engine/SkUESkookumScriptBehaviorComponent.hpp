// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Bindings for the SkookumScriptBehaviorComponent class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"

//---------------------------------------------------------------------------------------

class SKOOKUMSCRIPTRUNTIME_API SkUESkookumScriptBehaviorComponent : public SkUEClassBindingEntity<SkUESkookumScriptBehaviorComponent, USkookumScriptBehaviorComponent>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
