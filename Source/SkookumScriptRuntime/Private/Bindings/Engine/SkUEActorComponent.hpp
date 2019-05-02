// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Additional bindings for the ActorComponent (= UActorComponent) class 
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkUEActorComponent.generated.hpp>

//=======================================================================================
// Global Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Bindings for the Actor (= AActor) class 
class SkUEActorComponent_Ext : public SkUEActorComponent
  {
  public:
    static void register_bindings();
  };

