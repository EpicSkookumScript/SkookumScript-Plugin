// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript color class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkUEClassBinding.hpp"
#include "Math/UnrealMath.h" // Vector math functions

//---------------------------------------------------------------------------------------
// SkookumScript color class
class SKOOKUMSCRIPTRUNTIME_API SkColor : public SkClassBindingSimpleForceInit<SkColor, FLinearColor>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
