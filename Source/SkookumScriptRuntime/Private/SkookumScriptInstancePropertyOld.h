// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// This is a legacy data type used prior to UE4 4.25. This type is necessary to allow 
// deserialization of UObjects containing a stored USkookumScriptInstanceProperty 
// saved prior to 4.25. This type has been replaced by FSkookumScriptInstanceProperty.
//=======================================================================================

#pragma once
//=======================================================================================
// Includes
//=======================================================================================
//#include "UObject/UnrealTypePrivate.h"
#include <AgogCore/AIdPtr.hpp>
#include <SkookumScript/SkInstance.hpp>
#include "SkookumScriptInstanceProperty.h"
#include "UObject/ObjectMacros.h"
#include "CoreMinimal.h"
#include "UObject/UnrealTypePrivate.h"
// WARNING: This should always be the last include in any file that needs it (except .generated.h)
#include "UObject/UndefineUPropertyMacros.h"

//=======================================================================================
// Global Defines / Macros
//=======================================================================================

class SkClass;

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Property representing a SkookumScript SkInstance/SkDataInstance pointer

#define USE_UPROPERTY_LOAD_DEFERRING (USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING && WITH_EDITORONLY_DATA)

class USkookumScriptInstanceProperty : public UProperty
  {
  DECLARE_CASTED_CLASS_INTRINSIC_NO_CTOR(USkookumScriptInstanceProperty, UProperty, 0, TEXT("/Script/SkookumScriptRuntime"), CASTCLASS_FProperty, NO_API)

  public:

  USkookumScriptInstanceProperty(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  protected:

  virtual void Serialize(FArchive & ar) override;

#if WITH_EDITORONLY_DATA
  virtual FField* GetAssociatedFField() override;
#endif
  };

#include "UObject/DefineUPropertyMacros.h"