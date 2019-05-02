// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Name (= FName) class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "UObject/NameTypes.h"

#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript Name (= FName) class
class SKOOKUMSCRIPTRUNTIME_API SkUEName : public SkClassBindingSimple<SkUEName, FName>
  {
  public:

    enum { Binding_has_ctor = false }; // Do not auto-generate constructor since we have a special one taking a String argument

    static void       register_bindings();
    static SkClass *  get_class();

  };
