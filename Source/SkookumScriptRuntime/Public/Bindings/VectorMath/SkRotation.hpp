// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript rotation/quaternion class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript rotation/quaternion class
class SKOOKUMSCRIPTRUNTIME_API SkRotation : public SkClassBindingSimpleForceInit<SkRotation, FQuat>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
