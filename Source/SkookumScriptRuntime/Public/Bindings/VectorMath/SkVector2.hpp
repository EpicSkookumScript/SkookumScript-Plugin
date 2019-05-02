// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript 2D vector class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript 2D vector class
class SKOOKUMSCRIPTRUNTIME_API SkVector2 : public SkClassBindingSimpleForceInit<SkVector2, FVector2D>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
