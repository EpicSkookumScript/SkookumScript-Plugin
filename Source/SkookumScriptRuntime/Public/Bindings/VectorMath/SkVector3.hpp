// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript 3D vector class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript 3D vector class
class SKOOKUMSCRIPTRUNTIME_API SkVector3 : public SkClassBindingSimpleForceInit<SkVector3, FVector>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
