// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Property representing a SkookumScript SkInstance/SkDataInstance pointer
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================
#include "SkookumScriptInstancePropertyOld.h"

#include "SkookumScriptInstanceProperty.h"
#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptConstructionComponent.h"
#include "Bindings/SkUEClassBinding.hpp"
#include "SkUEEntity.generated.hpp"

#include "Engine/World.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UnrealTypePrivate.h"
#include "UObject/UndefineUPropertyMacros.h"
//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
USkookumScriptInstanceProperty::USkookumScriptInstanceProperty(const FObjectInitializer & object_initializer)
  : UProperty(object_initializer)
  {
  ArrayDim = 1;
  #if WITH_EDITORONLY_DATA
    ElementSize = sizeof(AIdPtr<SkInstance>);
  #else
    ElementSize = sizeof(SkInstance *);
  #endif
  }

#if WITH_EDITORONLY_DATA
FField* USkookumScriptInstanceProperty::GetAssociatedFField()
{
  if (!AssociatedField)
  {
    SetAssociatedFField(new FSkookumScriptInstanceProperty(this));
  }
  return Super::GetAssociatedFField();
}

#endif

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::Serialize(FArchive & ar)
  {
  // For now, we're not storing any additional data when we are serialized
  Super::Serialize(ar);
  }

IMPLEMENT_INTRINSIC_CLASS(USkookumScriptInstanceProperty, SKOOKUMSCRIPTRUNTIME_API, UProperty, SKOOKUMSCRIPTRUNTIME_API, "/Script/SkookumScriptRuntime",
  {
  }
);

#include "UObject/DefineUPropertyMacros.h"