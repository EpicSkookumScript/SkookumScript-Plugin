// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript transform (position + rotation + scale) class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "Math/UnrealMath.h" // Vector math functions
#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript transform (position + rotation + scale) class
class SKOOKUMSCRIPTRUNTIME_API SkTransform : public SkClassBindingSimple<SkTransform, FTransform>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
