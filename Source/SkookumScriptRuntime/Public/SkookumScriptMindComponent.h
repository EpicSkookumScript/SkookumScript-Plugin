// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Component to associate a SkookumScript Mind with a UE4 actor
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "Components/ActorComponent.h"

#include <AgogCore/AIdPtr.hpp>
#include <SkookumScript/SkInstance.hpp>
#include "SkookumScriptMindComponent.generated.h"

//=======================================================================================
// Global Defines / Macros
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Allows you to instantiate and delete a custom SkookumScript Mind instance along with the actor this component belongs to
UCLASS(classGroup=Scripting, editinlinenew, BlueprintType, meta=(BlueprintSpawnableComponent), hideCategories=(Object, ActorComponent))
class SKOOKUMSCRIPTRUNTIME_API USkookumScriptMindComponent : public UActorComponent
  {

    GENERATED_UCLASS_BODY()

  public:

  // Public Data Members

    // SkookumScript class type of the Mind instance that this component should spawn.
    // Cannot be blank.
    UPROPERTY(Category = Script, EditAnywhere, BlueprintReadOnly)
    FString ScriptMindClassName;

  // Methods

    // Gets our SkookumScript instance
    SkInstance * get_sk_mind_instance() const { return m_mind_instance_p; }

  protected:

    // UActorComponent interface
    virtual void OnRegister() override;
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type end_play_reason) override;
    virtual void UninitializeComponent() override;
    virtual void OnUnregister() override;

    // Creates/deletes our SkookumScript instance
    void        create_sk_instance();
    void        delete_sk_instance();

    // Keep the SkookumScript instance belonging to this mind around
    AIdPtr<SkInstance> m_mind_instance_p;

  };  // USkookumScriptMindComponent


//=======================================================================================
// Inline Functions
//=======================================================================================

