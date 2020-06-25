// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Component to associate a SkookumScript class and data members with a UE4 actor
// and allow SkookumScript ctors and dtors to be called when the actor (i.e. the component) gets created/destroyed
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptInstanceProperty.h"
#include "Bindings/Engine/SkUEActor.hpp"

#include "Engine/World.h"
#include "Runtime/Launch/Resources/Version.h" // TEMP HACK for ENGINE_MINOR_VERSION

//=======================================================================================
// Class Data
//=======================================================================================

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

USkookumScriptClassDataComponent::USkookumScriptClassDataComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  {
  PrimaryComponentTick.bCanEverTick = false;
  bTickInEditor = false;
  bAutoActivate = true;
  bWantsInitializeComponent = true;
  #if ENGINE_MINOR_VERSION < 14
    bWantsBeginPlay = true;
  #endif
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::create_sk_instance()
  {
  SK_ASSERTX(!m_actor_instance_p, "Tried to create actor instance when instance already present!");

  // Find the actor I belong to
  AActor * actor_p = GetOwner();
  SK_ASSERTX(actor_p, "SkookumScriptClassDataComponent must be attached to an actor.");

  // Determine SkookumScript class of my actor
  SkClass * sk_class_p = nullptr;
  FString class_name = ScriptActorClassName;
  if (!class_name.IsEmpty())
    {
    AString class_name_ascii(*class_name, class_name.Len());
    sk_class_p = SkBrain::get_class(class_name_ascii.as_cstr());
    SK_ASSERTX(sk_class_p, a_cstr_format("Cannot find Script Class Name '%s' specified in SkookumScriptClassDataComponent of '%S'. Misspelled?", class_name_ascii.as_cstr(), *actor_p->GetName()));
    if (!sk_class_p) goto set_default_class; // Recover from bad user input

    // Do some extra checking in non-shipping builds
    #if (SKOOKUM & SK_DEBUG)
      UClass * known_ue_superclass_p;
      SkClass * super_class_known_to_ue_p = SkUEClassBindingHelper::find_most_derived_super_class_known_to_ue(sk_class_p, &known_ue_superclass_p);
      UClass * allowed_ue_superclass_p = known_ue_superclass_p;
      while (!actor_p->GetClass()->IsChildOf(allowed_ue_superclass_p))
        {
        allowed_ue_superclass_p = allowed_ue_superclass_p->GetSuperClass();
        }
      SK_ASSERTX(actor_p->GetClass()->IsChildOf(known_ue_superclass_p), a_cstr_format(
        "Owner Script Class Name '%s' in SkookumScriptClassDataComponent of '%S' is derived from the UE4 class '%S', however '%S's class '%S' is not. "
        "This can lead to problems since '%s' might try to use functionality of '%S' which '%S' does not have.\n\n"
        "To fix this, either make sure that '%S' is derived from '%S', or change '%s' so that instead from '%S' it derives from just '%S' (or a superclass of it).",
        class_name_ascii.as_cstr(),
        *actor_p->GetName(),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(known_ue_superclass_p),
        *actor_p->GetName(),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(actor_p->GetClass()),
        class_name_ascii.as_cstr(),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(known_ue_superclass_p),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(actor_p->GetClass()),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(actor_p->GetClass()),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(known_ue_superclass_p),
        class_name_ascii.as_cstr(),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(known_ue_superclass_p),
        *SkUEClassBindingHelper::get_ue_class_name_sans_c(allowed_ue_superclass_p)));
    #endif
    }
  else
    {
  set_default_class:
    sk_class_p = SkUEClassBindingHelper::find_most_derived_super_class_known_to_sk(actor_p->GetClass());
    SK_ASSERTX(sk_class_p, a_cstr_format("No parent class of %S is known to SkookumScript!", *actor_p->GetClass()->GetName()));
    if (!sk_class_p)
      {
      sk_class_p = SkBrain::ms_actor_class_p; // Recover to prevent crash
      }
    }

  // Currently, we support only actors
  SK_ASSERTX(sk_class_p->is_actor_class(), a_str_format("Trying to create a SkookumScriptClassDataComponent of class '%s' which is not an actor.", sk_class_p->get_name_cstr_dbg()));
  SkInstance * instance_p;
  uint32_t instance_offset = sk_class_p->get_user_data_int();
  if (instance_offset)
    {
    #if !UE_BUILD_SHIPPING
      if (instance_offset >= (uint32_t)actor_p->GetClass()->PropertiesSize)
        {
        SK_ERRORX(a_str_format("Instance offset out of range for actor '%S' of class '%S'!", *actor_p->GetName(), *actor_p->GetClass()->GetName()));
        goto DontEmbed;
        }
    #endif

    // If this object stores its own instance, create it here
    instance_p = FSkookumScriptInstanceProperty::construct_instance((uint8_t *)actor_p + instance_offset, actor_p, sk_class_p);
    #if WITH_EDITOR
      // Check if this component's class could be also just auto-generated
      SkClass * sk_actor_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(actor_p->GetClass());
      if (sk_actor_class_p && sk_actor_class_p->is_class(*sk_class_p))
        {
        A_DPRINT("Note: Due to recent plugin improvements, the SkookumScriptClassDataComponent '%S' of actor '%S' is no longer needed and can be deleted.\n", *this->GetName(), *actor_p->GetName());
        }
    #endif
    }
  else
    {
  #if !UE_BUILD_SHIPPING
  DontEmbed:
  #endif
    // Based on the desired class, create SkInstance or SkDataInstance
    instance_p = sk_class_p->new_instance();
    instance_p->construct<SkUEActor>(actor_p); // Keep track of owner actor
    instance_p->call_default_constructor();
    }
  m_actor_instance_p = instance_p;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::delete_sk_instance()
  {
  SK_ASSERTX(m_actor_instance_p, "No Sk instance to delete!");
  m_actor_instance_p->abort_coroutines_on_this();
  m_actor_instance_p->dereference();
  m_actor_instance_p = nullptr;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::OnRegister()
  {
  Super::OnRegister();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::InitializeComponent()
  {
  Super::InitializeComponent();

  // Create SkookumScript instance, but only if we are located inside the game world
  if (GetOwner()->GetWorld()->IsGameWorld())
    {
    SK_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_gameplay, "SkookumScript must be in gameplay mode when InitializeComponent() is invoked.");

    create_sk_instance();
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::BeginPlay()
  {
  Super::BeginPlay();
  SK_ASSERTX(m_actor_instance_p != nullptr, a_str_format("SkookumScriptClassDataComponent '%S' on actor '%S' has no SkookumScript instance upon BeginPlay. This means its InitializeComponent() method was never called during initialization. Please check your initialization sequence and make sure this component gets properly initialized.", *GetName(), *GetOwner()->GetName()));
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::EndPlay(const EEndPlayReason::Type end_play_reason)
	{
  Super::EndPlay(end_play_reason);
	}

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::UninitializeComponent()
  {
  // Delete SkookumScript instance if present
  if (m_actor_instance_p)
    {
    SK_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_gameplay, "SkookumScript must be in gameplay mode when UninitializeComponent() is invoked.");

    delete_sk_instance();
    }

  Super::UninitializeComponent();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptClassDataComponent::OnUnregister()
  {
  Super::OnUnregister();
  }

