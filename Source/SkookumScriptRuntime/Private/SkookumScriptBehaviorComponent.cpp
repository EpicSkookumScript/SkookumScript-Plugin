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

#include "SkookumScriptBehaviorComponent.h"
#include "Bindings/Engine/SkUESkookumScriptBehaviorComponent.hpp"

#include "VectorField/VectorField.h" // HACK to fix broken dependency on UVectorField 
#include <SkUEEEndPlayReason.generated.hpp>
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Runtime/Launch/Resources/Version.h" // TEMP HACK for ENGINE_MINOR_VERSION

//=======================================================================================
// Class Data
//=======================================================================================

ASymbol USkookumScriptBehaviorComponent::ms_symbol_on_attach;
ASymbol USkookumScriptBehaviorComponent::ms_symbol_on_detach;
ASymbol USkookumScriptBehaviorComponent::ms_symbol_on_begin_play;
ASymbol USkookumScriptBehaviorComponent::ms_symbol_on_end_play;

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::initialize()
  {
  ms_symbol_on_attach     = ASymbol::create_existing("on_attach");
  ms_symbol_on_detach     = ASymbol::create_existing("on_detach");
  ms_symbol_on_begin_play = ASymbol::create_existing("on_begin_play");
  ms_symbol_on_end_play   = ASymbol::create_existing("on_end_play");
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::deinitialize()
  {
  ms_symbol_on_attach     = ASymbol::get_null();
  ms_symbol_on_detach     = ASymbol::get_null();
  ms_symbol_on_begin_play = ASymbol::get_null();
  ms_symbol_on_end_play   = ASymbol::get_null();
  }

//---------------------------------------------------------------------------------------

USkookumScriptBehaviorComponent::USkookumScriptBehaviorComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , m_is_instance_externally_owned(false)
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

void USkookumScriptBehaviorComponent::create_sk_instance()
  {
  SK_ASSERTX(!m_component_instance_p, "Tried to create actor instance when instance already present!");

  // Find the actor I belong to
  AActor * actor_p = GetOwner();
  SK_ASSERTX(actor_p, "SkookumScriptBehaviorComponent must be attached to an actor.");

  FString class_name = ScriptComponentClassName;
  AString class_name_ascii(*class_name, class_name.Len());
  SkClass * class_p = nullptr;
  if (!class_name_ascii.is_empty())
    {
    // Try to find explicitly specified class
    class_p = SkBrain::get_class(class_name_ascii.as_cstr());
    SK_ASSERTX(class_p, a_cstr_format("Cannot find Script Class Name '%s' specified in SkookumScriptBehaviorComponent of '%S'. Misspelled?", class_name_ascii.as_cstr(), *actor_p->GetName()));
    }
  if (!class_p)
    {
    // Default class is SkookumScriptBehaviorComponent itself
    class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(this->GetClass());
    SK_ASSERTX(class_p, a_cstr_format("Cannot find equivalent SkookumScript class for SkookumScriptBehaviorComponent class '%S'!", *this->GetClass()->GetName()));
    if (!class_p)
      {
      // Recover if class not found
      class_p = SkBrain::get_class(SkBrain::ms_component_class_name);
      }
    }

  // Based on the desired class, create SkInstance or SkDataInstance
  // Must be derived from SkookumScriptBehaviorComponent
  SK_ASSERTX(class_p->is_component_class(), a_str_format("Trying to create a SkookumScriptBehaviorComponent of class '%s' which is not derived from SkookumScriptBehaviorComponent.", class_p->get_name_cstr_dbg()));
  m_component_instance_p = class_p->new_instance();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::delete_sk_instance()
  {
  SK_ASSERTX(m_component_instance_p, "No Sk instance to delete!");
  m_component_instance_p->abort_coroutines_on_this();
  m_component_instance_p->dereference();
  m_component_instance_p = nullptr;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::set_sk_component_instance(SkInstance * instance_p)
  {
  SK_ASSERTX(!m_component_instance_p, "Tried to create actor instance when instance already present!");

  instance_p->reference();
  m_component_instance_p = instance_p;
  m_is_instance_externally_owned = true;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::attach(SkInstance * instance_p)
  {
  set_sk_component_instance(instance_p);
  RegisterComponent();
  // Workaround: This code might be called from within AActor::InitializeComponents(), so make sure InitializeComponent() is called
  if (!GetOwner()->IsActorInitialized())
    {
    InitializeComponent();
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::detach()
  {
  DestroyComponent();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::OnRegister()
  {
  Super::OnRegister();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::InitializeComponent()
  {
  Super::InitializeComponent();

  // Create SkookumScript instance, but only if we are located inside the game world
  if (GetOwner()->GetWorld()->IsGameWorld())
    {
    if (!m_is_instance_externally_owned)
      {
      SK_MAD_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_gameplay, "SkookumScript must be in gameplay mode when InitializeComponent() is invoked.");

      create_sk_instance();
      m_component_instance_p->get_class()->resolve_raw_data();
      m_component_instance_p->call_default_constructor();
      }

    m_component_instance_p->as<SkUESkookumScriptBehaviorComponent>() = this;
    m_component_instance_p->method_call(ms_symbol_on_attach);
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::BeginPlay()
  {
  // We might have been detached in the on_attached function - bail if that's the case
  if (IsRegistered())
    {
    Super::BeginPlay();

    SK_ASSERTX(m_component_instance_p.is_valid(), a_str_format("SkookumScriptBehaviorComponent '%S' on actor '%S' has no SkookumScript instance upon BeginPlay. This means its InitializeComponent() method was never called during initialization. Please check your initialization sequence and make sure this component gets properly initialized.", *GetName(), *GetOwner()->GetName()));
    if (m_component_instance_p.is_valid())
      {
      m_component_instance_p->method_call(ms_symbol_on_begin_play);
      }
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::EndPlay(const EEndPlayReason::Type end_play_reason)
  {
  if (m_component_instance_p.is_valid())
    {
    m_component_instance_p->method_call(ms_symbol_on_end_play, SkUEEEndPlayReason::new_instance(end_play_reason));
    }

  Super::EndPlay(end_play_reason);
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::UninitializeComponent()
  {
  // Delete SkookumScript instance if present
  if (m_component_instance_p.is_valid())
    {
    SK_MAD_ASSERTX(SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_gameplay, "SkookumScript must be in gameplay mode when UninitializeComponent() is invoked.");

    m_component_instance_p->method_call(ms_symbol_on_detach);
    m_component_instance_p->as<SkUESkookumScriptBehaviorComponent>() = nullptr;

    if (m_is_instance_externally_owned)
      {
      m_component_instance_p->dereference();
      m_component_instance_p = nullptr;
      m_is_instance_externally_owned = false;
      }
    else
      {
      delete_sk_instance();
      }
    }

  Super::UninitializeComponent();
  }

//---------------------------------------------------------------------------------------

void USkookumScriptBehaviorComponent::OnUnregister()
  {
  // This will get called as part of PreEditChange when editing live properties, e.g.: manually changing the transform of an object when in edit scene mode.
  // So there is a use-case where it's completely normal for this to get called while m_component_instance_p is valid. In this use-case, this will always
  // be followed up by a call to OnRegister.
  //SK_MAD_ASSERTX(m_component_instance_p.is_null(), "Instance should have been destroyed at this point.");

  Super::OnUnregister();
  }

