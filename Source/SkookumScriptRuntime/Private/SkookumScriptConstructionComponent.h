// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Component to call SkookumScript ctor and dtor at the proper moment
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "Components/ActorComponent.h"

#include "SkookumScriptConstructionComponent.generated.h"

//=======================================================================================
// Global Defines / Macros
//=======================================================================================

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Facilitates construction/destruction of an embedded SkInstance
UCLASS()
class USkookumScriptConstructionComponent : public UActorComponent
  {

    GENERATED_UCLASS_BODY()

  protected:

    // Begin UActorComponent interface
    virtual void InitializeComponent() override;
    virtual void UninitializeComponent() override;

  };  // USkookumScriptConstructionComponent

