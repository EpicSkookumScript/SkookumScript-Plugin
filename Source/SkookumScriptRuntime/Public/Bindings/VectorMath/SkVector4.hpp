// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript 4D vector class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript 4D vector class
class SKOOKUMSCRIPTRUNTIME_API SkVector4 : public SkClassBindingSimpleForceInit<SkVector4, FVector4>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
