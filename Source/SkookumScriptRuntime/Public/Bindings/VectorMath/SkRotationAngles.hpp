// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Euler angles class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript Euler angles class
class SKOOKUMSCRIPTRUNTIME_API SkRotationAngles : public SkClassBindingSimpleForceInit<SkRotationAngles, FRotator>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
