// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Property representing a SkookumScript SkInstance/SkDataInstance pointer
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptInstanceProperty.h"
#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptConstructionComponent.h"
#include "Bindings/SkUEClassBinding.hpp"
#include "SkUEEntity.generated.hpp"

#include <SkookumScript/SkInstance.hpp>

#include "Engine/World.h"
#include "UObject/PropertyPortFlags.h"

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

USkookumScriptInstanceProperty::USkookumScriptInstanceProperty(const FObjectInitializer & object_initializer)
  : Super(object_initializer)
  {
  PropertyFlags |= CPF_SkipSerialization;
  ArrayDim = 1;
  #if WITH_EDITORONLY_DATA
    ElementSize = sizeof(AIdPtr<SkInstance>);
  #else
    ElementSize = sizeof(SkInstance *);
  #endif
  }

//---------------------------------------------------------------------------------------

USkookumScriptInstanceProperty::~USkookumScriptInstanceProperty()
  {
  }

//---------------------------------------------------------------------------------------
// Create an SkInstance and call its default constructor
SkInstance * USkookumScriptInstanceProperty::construct_instance(void * data_p, UObject * obj_p, SkClass * sk_class_p)
  {
  SkInstance * instance_p = sk_class_p->new_instance(); // SkInstance or SkDataInstance
  instance_p->construct<SkUEEntity>(obj_p);             // SkInstance points back to the owner object
  set_instance(data_p, instance_p);                     // Store SkInstance in object
  instance_p->call_default_constructor();               // Call script constructor    
  instance_p->construct<SkUEEntity>(obj_p);             // Initialize object pointer a second time as the default constructor might have overclobbered it (e.g. SkookumScriptBehaviorComponent)
  return instance_p;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::destroy_instance(void * data_p)
  {
  SkInstance * instance_p = get_instance(data_p); // Get SkInstance from object
  if (instance_p)
    {
    instance_p->abort_coroutines_on_this();
    // Destructor not explicitly called here as it will be automagically called 
    // when the instance is dereferenced to zero
    instance_p->dereference();
    // Zero the pointer so it's fresh in case UE4 wants to recycle this object
    set_instance(data_p, nullptr);
    }
  }

//---------------------------------------------------------------------------------------

FORCEINLINE UObject * USkookumScriptInstanceProperty::get_owner(const void * data_p) const
  {
  return (UObject *)((uint8_t *)data_p - GetOffset_ForInternal());
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::LinkInternal(FArchive & ar)
  {
  // Nothing to do here, but function must exist
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::Serialize(FArchive & ar)
  {
  // For now, we're not storing any additional data when we are serialized
  Super::Serialize(ar);
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::SerializeItem(FStructuredArchive::FSlot Slot, void* Value, void const* Defaults) const
  {
  // https://udn.unrealengine.com/questions/467186/view.html
  Slot.EnterStream();
  }

//---------------------------------------------------------------------------------------

FString USkookumScriptInstanceProperty::GetCPPType(FString * extended_type_text_p, uint32 cpp_export_flags) const
  {
  // This property reserves storage - return dummy place holders
  #if WITH_EDITORONLY_DATA
    return TEXT("struct { void * m_p; uint32 id; }");
  #else
    return TEXT("void *");
  #endif
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::ExportTextItem(FString & value_str, const void * data_p, const void * default_data_p, UObject * owner_p, int32 port_flags, UObject * export_root_scope_p) const
  {
  // This property merely reserves storage but doesn't store any actual data
  // So return nullptr just to return something
  value_str += (port_flags & PPF_ExportCpp) ? TEXT("nullptr") : TEXT("NULL");
  }

//---------------------------------------------------------------------------------------

const TCHAR * USkookumScriptInstanceProperty::ImportText_Internal(const TCHAR * buffer_p, void * data_p, int32 port_flags, UObject * owner_p, FOutputDevice * error_text_p) const
  {
  // Consume the identifier that we stored ("NULL")
  FString temp; 
  buffer_p = UPropertyHelpers::ReadToken(buffer_p, temp);

  // Initialize value
  InitializeValueInternal(data_p);

  return buffer_p;
  }

//---------------------------------------------------------------------------------------

int32 USkookumScriptInstanceProperty::GetMinAlignment() const
  {
  return alignof(uintptr_t);
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::InitializeValueInternal(void * data_p) const
  {
  UObject * owner_p = get_owner(data_p);

  // Leave untouched on CDOs and for temporary editor assets
  if (!(owner_p->HasAnyFlags(RF_ClassDefaultObject | RF_Transient | RF_Transactional)))
    {
    // Clear SkInstance storage in object
    set_instance(data_p, nullptr);

    AActor * actor_p = Cast<AActor>(owner_p);
    if (actor_p)
      {
      // If an actor, append a component to it that will take care of the proper construction at the proper time
      // i.e. after all other components are initialized
      // But only if there's not a SkookumScriptClassDataComponent already
      if (!actor_p->GetComponentByClass(USkookumScriptClassDataComponent::StaticClass()))
        {
        FName component_name = USkookumScriptConstructionComponent::StaticClass()->GetFName();
        USkookumScriptConstructionComponent * component_p = NewObject<USkookumScriptConstructionComponent>(actor_p, component_name);
        actor_p->AddOwnedComponent(component_p);
        //component_p->RegisterComponent();
        }
      }
    else if (!owner_p->IsA<USkookumScriptBehaviorComponent>())
      {
      // Objects loaded by the editor are just loaded into the editor world, not the game world
      #if WITH_EDITOR
        if (GIsEditorLoadingPackage) return;
      #endif

      // Construct right here
      SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(owner_p->GetClass());
      sk_class_p->resolve_raw_data(); // In case it wasn't resolved before, in packaged builds we need to resolve this here to avoid errors accessing raw data in the constructor.
      USkookumScriptInstanceProperty::construct_instance(data_p, owner_p, sk_class_p);
      }
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::InstanceSubobjects(void * data_p, void const * default_data_p, UObject * owner_p, struct FObjectInstancingGraph * instance_graph_p)
  {
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::ClearValueInternal(void * data_p) const
  {
  // This property merely reserves storage but doesn't store any actual data
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::DestroyValueInternal(void * data_p) const
  {
  UObject * owner_p = get_owner(data_p);
  // Leave untouched on CDOs
  if (!(owner_p->GetFlags() & (RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed)))
    {
    // Leave actors alone as the component we attached will take care of itself
    if (!owner_p->IsA<AActor>()
     && !owner_p->IsA<USkookumScriptBehaviorComponent>())
      {
      // Not an actor, so take care of destruction here
      destroy_instance(data_p);
      }
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptInstanceProperty::CopyValuesInternal(void * dst_p, void const * src_p, int32 count) const
  {
  // Copying instances between objects makes no sense, so we simply don't do anything here
  }

//---------------------------------------------------------------------------------------

bool USkookumScriptInstanceProperty::SameType(const UProperty * other_p) const
  {
  return Super::SameType(other_p);
  }

//---------------------------------------------------------------------------------------

bool USkookumScriptInstanceProperty::Identical(const void * ldata_p, const void * rdata_p, uint32 port_flags) const
  {
  // By definition, no object should use the same SkInstance as another object
  return false;
  }

