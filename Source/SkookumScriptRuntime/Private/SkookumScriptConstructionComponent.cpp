// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Component to call SkookumScript ctor and dtor at the proper moment
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptConstructionComponent.h"
#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptInstanceProperty.h"
#include "Bindings/Engine/SkUEActor.hpp"
#include "SkUEEntity.generated.hpp"

#include "Engine/World.h"
#include "Runtime/Launch/Resources/Version.h" // TEMP HACK for ENGINE_MINOR_VERSION

//=======================================================================================
// Class Data
//=======================================================================================

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

USkookumScriptConstructionComponent::USkookumScriptConstructionComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  {
  PrimaryComponentTick.bCanEverTick = false;
  bTickInEditor = false;
  bAutoActivate = true;
  bWantsInitializeComponent = true;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptConstructionComponent::InitializeComponent()
  {
  Super::InitializeComponent();

  // Create SkookumScript instance, but only if we are located inside the game world
  // If there's a USkookumScriptClassDataComponent, it must be ahead in the queue since we checked before this component was attached
  // Therefore, if there's one, we'll leave the construction of the instance to it
  AActor * actor_p = GetOwner();
  if (actor_p->GetWorld()->IsGameWorld()
   && !actor_p->GetComponentByClass(USkookumScriptClassDataComponent::StaticClass()))
    {
    SK_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_gameplay, "SkookumScript must be in gameplay mode when InitializeComponent() is invoked.");

    SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(actor_p->GetClass());
    if (sk_class_p)
      {
      uint32_t instance_offset = sk_class_p->get_user_data_int();
      if (!instance_offset)
        {
        // If offset has not been computed yet, compute it now
        UProperty * property_p = actor_p->GetClass()->FindPropertyByName(USkookumScriptInstanceProperty::StaticClass()->GetFName());
        SK_ASSERTX(property_p, a_str_format("Class '%s' has no USkookumScriptInstanceProperty needed for actor '%S'!", sk_class_p->get_name_cstr(), *actor_p->GetName()));
        if (property_p)
          {
          instance_offset = property_p->GetOffset_ForInternal();

          #if !UE_BUILD_SHIPPING
            if (instance_offset >= (uint32_t)actor_p->GetClass()->PropertiesSize)
              {
              SK_ERRORX(a_str_format("Instance offset out of range for actor '%S' of class '%S'!", *actor_p->GetName(), *actor_p->GetClass()->GetName()));
              instance_offset = 0;
              }
          #endif

          sk_class_p->set_user_data_int_recursively(instance_offset);
          }
        }
      SK_ASSERTX(instance_offset, a_str_format("Class '%s' has no embedded instance offset to create an SkInstance for actor '%S'!", sk_class_p->get_name_cstr(), *actor_p->GetName()));
      if (instance_offset)
        {
        USkookumScriptInstanceProperty::construct_instance((uint8_t *)actor_p + instance_offset, actor_p, sk_class_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptConstructionComponent::UninitializeComponent()
  {
  AActor * actor_p = GetOwner();
  // Only uninitialize those components that we initialized to begin with
  // If there's a USkookumScriptClassDataComponent, it has already taken care of the destruction
  if (actor_p->GetWorld() && actor_p->GetWorld()->IsGameWorld()
   && !actor_p->GetComponentByClass(USkookumScriptClassDataComponent::StaticClass()))
    {
    // Delete SkookumScript instance if present
    SK_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_sim, "SkookumScript must be at least in sim mode when UninitializeComponent() is invoked.");

    SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(actor_p->GetClass());
    if (sk_class_p)
      {
      uint32_t instance_offset = sk_class_p->get_user_data_int();
      
      #if !UE_BUILD_SHIPPING
        if (instance_offset >= (uint32_t)actor_p->GetClass()->PropertiesSize)
          {
          SK_ERRORX(a_str_format("Instance offset out of range for actor '%S' of class '%S'!", *actor_p->GetName(), *actor_p->GetClass()->GetName()));
          instance_offset = 0;
          }
        else
          {
          SK_ASSERTX(instance_offset, a_str_format("Class '%s' has no embedded instance offset to destroy the SkInstance of actor '%S'!", sk_class_p->get_name_cstr(), *actor_p->GetName()));
          }
      #endif

      if (instance_offset)
        {
        USkookumScriptInstanceProperty::destroy_instance((uint8_t *)actor_p + instance_offset);
        }
      }
    }

  Super::UninitializeComponent();
  }
