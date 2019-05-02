// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Additional bindings for the EntityClass (= UClass) class 
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkUEEntityClass.generated.hpp>

//=======================================================================================
// Global Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Bindings for the EntityClass (= UClass) class 
class SkUEEntityClass_Ext : public SkUEEntityClass
  {
  public:
    static void register_bindings();
  };

