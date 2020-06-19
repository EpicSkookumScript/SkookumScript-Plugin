// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Binding classes for UE4 
//=======================================================================================

#include "Bindings/SkUEClassBinding.hpp"
#include "ISkookumScriptRuntime.h"
#include "SkUERuntime.hpp"
#include "SkUESymbol.hpp"
#include "SkUEUtils.hpp"
#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptInstanceProperty.h"
#include "Engine/SkUEEntity.hpp"
#include "VectorMath/SkColor.hpp"
#include <SkUEWorld.generated.hpp>
#include "../SkookumScriptRuntimeGenerator.h"

#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "Engine/UserDefinedStruct.h"

#include <AgogCore/AMath.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkEnum.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkList.hpp>


//---------------------------------------------------------------------------------------

TMap<UClass*, SkClass*>                             SkUEClassBindingHelper::ms_static_class_map_u2s;
TMap<SkClassDescBase*, UClass*>                     SkUEClassBindingHelper::ms_static_class_map_s2u;
TMap<UStruct*, SkClass*>                            SkUEClassBindingHelper::ms_static_struct_map_u2s;
TMap<SkClassDescBase*, UStruct*>                    SkUEClassBindingHelper::ms_static_struct_map_s2u;
TMap<UEnum*, SkClass*>                              SkUEClassBindingHelper::ms_static_enum_map_u2s;

#if WITH_EDITORONLY_DATA
TMap<SkClassDescBase*, TWeakObjectPtr<UBlueprint>>  SkUEClassBindingHelper::ms_dynamic_class_map_s2u;
TMap<UBlueprint*, SkClass*>                         SkUEClassBindingHelper::ms_dynamic_class_map_u2s;
#endif

int32_t                                             SkUEClassBindingHelper::ms_world_data_idx = -1;

const FName                                         SkUEClassBindingHelper::NAME_Entity("Entity");

//---------------------------------------------------------------------------------------
// Compute data idx to world variable
int32_t SkUEClassBindingHelper::get_world_data_idx()
  {
  if (ms_world_data_idx < 0)
    {
    if (!SkBrain::ms_object_class_p->get_class_data().find(ASymbolX_c_world, AMatch_first_found, (uint32_t*)&ms_world_data_idx))
      {
      SK_ASSERTX(false, "Couldn't find the @@world class member variable!");
      }
    }

  return ms_world_data_idx;
  }

//---------------------------------------------------------------------------------------
// Get pointer to UWorld from global variable
UWorld * SkUEClassBindingHelper::get_world()
  {
  if (SkookumScript::get_initialization_level() < SkookumScript::InitializationLevel_program)
    {
    return nullptr;
    }

  SkInstance * world_var_p = SkBrain::ms_object_class_p->get_class_data_value_by_idx(get_world_data_idx());
  SK_ASSERTX(world_var_p && (world_var_p == SkBrain::ms_nil_p || world_var_p->get_class() == SkBrain::get_class(ASymbol_World)), "@@world variable does not have proper type."); // nil is ok
  return world_var_p == SkBrain::ms_nil_p ? nullptr : world_var_p->as<SkUEWorld>();
  }

//---------------------------------------------------------------------------------------
// Set the game world (C++ and Skookum variable) to a specific world object
void SkUEClassBindingHelper::set_world(UWorld * world_p)
  {
  if (SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_program)
    {
    SkBrain::ms_object_class_p->set_class_data_value_by_idx_no_ref(get_world_data_idx(), world_p ? SkUEWorld::new_instance(world_p) : SkBrain::ms_nil_p);
    }
  }

//---------------------------------------------------------------------------------------
// Determine SkookumScript class from UClass
SkClass * SkUEClassBindingHelper::get_object_class(UObject * obj_p, UClass * def_ue_class_p /*= nullptr*/, SkClass * def_sk_class_p /*= nullptr*/)
  {
  SkClass * sk_class_p = def_sk_class_p;

  if (obj_p)
    {
    UClass * ue_class_p = obj_p->GetClass();
    if (ue_class_p != def_ue_class_p)
      {
      // Crawl up class hierarchy until we find a class known to Sk
      do 
        {
        sk_class_p = get_sk_class_from_ue_class(ue_class_p);
        } while (!sk_class_p && (ue_class_p = ue_class_p->GetSuperClass()) != nullptr);
      SK_ASSERTX(sk_class_p, a_str_format("UObject of type '%S' has no matching SkookumScript type!", *obj_p->GetClass()->GetName()));
      }
    }

  return sk_class_p;
  }

//---------------------------------------------------------------------------------------
// Get the embedded, dedicated SkInstance from an object
// Note: The instance is _not_ referenced
SkInstance * SkUEClassBindingHelper::get_embedded_instance(UObject * obj_p, SkClass * sk_class_p)
  {
  if (obj_p)
    {
    // Check if we have an instance stored inside the UObject itself
    uint32_t instance_offset = sk_class_p->get_user_data_int();
    if (instance_offset)
      {
      #if !UE_BUILD_SHIPPING
        if (instance_offset >= (uint32_t)obj_p->GetClass()->PropertiesSize)
          {
          SK_ERRORX(a_str_format("Instance offset out of range for actor '%S' of class '%S'!", *obj_p->GetName(), *obj_p->GetClass()->GetName()));
          return nullptr;
          }
      #endif

      SkInstance * instance_p = USkookumScriptInstanceProperty::get_instance((uint8_t *)obj_p + instance_offset);
      if (instance_p)
        {
        return instance_p;
        }
      }

    // Else, look for components that might have an instance stored inside
    if (sk_class_p->is_actor_class())
      {
      USkookumScriptClassDataComponent * component_p = static_cast<USkookumScriptClassDataComponent *>(static_cast<AActor *>(obj_p)->GetComponentByClass(USkookumScriptClassDataComponent::StaticClass()));
      if (component_p)
        {
        SkInstance * instance_p = component_p->get_sk_actor_instance();
        if (instance_p)
          {
          return instance_p;
          }
        }
      }
    else if (sk_class_p->is_component_class())
      {
      USkookumScriptBehaviorComponent * component_p = static_cast<USkookumScriptBehaviorComponent *>(static_cast<UObject *>(obj_p));
      SkInstance * instance_p = component_p->get_sk_component_instance();
      if (instance_p)
        {
        return instance_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Optimized version of get_embedded_instance() for known actors
// Note: The instance is _not_ referenced
SkInstance * SkUEClassBindingHelper::get_embedded_instance(AActor * actor_p, SkClass * sk_class_p)
  {
  if (actor_p)
    {
    // Check if we have an instance stored inside the UObject itself
    uint32_t instance_offset = sk_class_p->get_user_data_int();
    if (instance_offset)
      {
      #if !UE_BUILD_SHIPPING
        if (instance_offset >= (uint32_t)actor_p->GetClass()->PropertiesSize)
          {
          SK_ERRORX(a_str_format("Instance offset out of range for actor '%S' of class '%S'!", *actor_p->GetName(), *actor_p->GetClass()->GetName()));
          return nullptr;
          }
      #endif

      SkInstance * instance_p = USkookumScriptInstanceProperty::get_instance((uint8_t *)actor_p + instance_offset);
      if (instance_p)
        {
        return instance_p;
        }
      }

    // Else, look for component
    USkookumScriptClassDataComponent * component_p = static_cast<USkookumScriptClassDataComponent *>(actor_p->GetComponentByClass(USkookumScriptClassDataComponent::StaticClass()));
    if (component_p)
      {
      SkInstance * instance_p = component_p->get_sk_actor_instance();
      if (instance_p)
        {
        return instance_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Chop off trailing "_C" if exists
FString SkUEClassBindingHelper::get_ue_class_name_sans_c(UClass * ue_class_p)
  {
  FString class_name = ue_class_p->GetName();
  class_name.RemoveFromEnd(TEXT("_C"));
  return class_name;
  }

//---------------------------------------------------------------------------------------
// Resolve the raw data info of each raw data member of the given class
void SkUEClassBindingHelper::resolve_raw_data(SkClass * sk_class_p, UStruct * ue_struct_or_class_p)
  {
  // This loop assumes that the data members of the Sk class were created from this very UE4 class
  // I.e. that therefore, except for unsupported properties, they must be in the same order
  // So all we should have to do is loop forward and skip the occasional non-exported UE4 property
  UProperty * ue_var_p = nullptr;
  TFieldIterator<UProperty> property_it(ue_struct_or_class_p, EFieldIteratorFlags::ExcludeSuper);
  tSkTypedNameRawArray & raw_data = sk_class_p->get_instance_data_raw_for_resolving();
  bool is_all_resolved = true;
  for (auto var_p : raw_data)
    {
    // Skip variable if already resolved
    if (var_p->m_raw_data_info.IsValid())
      { 
      continue;
      }

    // The original UE4 property name of the raw data member is stored in the bind name
    const FName & bind_name = reinterpret_cast<const FName &>(var_p->m_bind_name);

    // Try to find it in the UE4 reflection data
    bool found_match = false;
    while (property_it)
      {
      ue_var_p = *property_it;
      ++property_it;
      if (bind_name.IsNone())
        {
        // Legacy heuristic code - remove 9/2017
        found_match = FSkookumScriptGeneratorHelper::compare_var_name_skookified(*ue_var_p->GetName(), var_p->get_name_cstr());
        }
      else
        {
        found_match = (bind_name == ue_var_p->GetFName());
        }
      if (found_match)
        {
        // Store raw data info in the raw data member object
        var_p->m_raw_data_info = compute_raw_data_info(ue_var_p);
        break;
        }
      }

    if (!found_match)
      {
      // Oops didn't find matching variable
      // This is probably due to an unsaved blueprint variable change in the UE4 editor during the previous session
      // If this is the case, a recompile would have been triggered when this class was loaded by get_ue_class_from_sk_class()
      // Which means binaries would be recompiled and reloaded once more, fixing this issue
      // So make sure this assumption is true
      SK_ASSERTX(FModuleManager::Get().GetModulePtr<ISkookumScriptRuntime>("SkookumScriptRuntime")->is_freshen_binaries_pending(), a_str_format("Sk Variable '%s.%s' (= UE4 property '%s.%s') not found in UE4 reflection data.", sk_class_p->get_name_cstr_dbg(), var_p->get_name_cstr(), sk_class_p->get_bind_name().as_string().as_cstr(), var_p->m_bind_name.as_string().as_cstr()));
      is_all_resolved = false;
      }
    }
  // Remember if all raw data was resolved
  sk_class_p->set_raw_data_resolved(is_all_resolved);
  }

//---------------------------------------------------------------------------------------
// Resolve the raw data info of each raw data member of the given class
bool SkUEClassBindingHelper::resolve_raw_data_static(SkClass * sk_class_p)
  {
  // We need to ensure that structs get their raw data functions fully resolved in both editor and packaged builds.
  // For a struct, the call stack that leads from game launch to resolving of its raw data functions is:
  //
  // EDITOR
  // SkUEClassBindingHelper::resolve_raw_data_funcs(..)
  // FSkookumScriptRuntime::on_struct_added_or_modified(.)
  // FSkookumScriptRuntime::on_new_asset(.)
  //
  // PACKAGED
  // SkUEClassBindingHelper::resolve_raw_data_funcs(..)
  // SkUEClassBindingHelper::resolve_raw_data_static(.) <---- THIS IS US
  // SkClass::resolve_raw_data_recurse()
  // SkBrain::initialize_post_load()
  // SkookumScript::initialize_program()
  // SkUERuntime::bind_compiled_scripts(....)
  // FSkookumScriptRuntime::on_world_init_pre(..)
  //
  // The important thing to note here is that for the packaged path, this function is the one chance to correctly resolve the raw data funcs.
  // This means that we must pass in both the sk_class AND the UStruct to resolve_raw_data_funcs. Failure to pass in the UStruct will result 
  // in the struct forever having SkUEClassBindingHelper::get_raw_pointer_null bound for its raw pointer function. 
  // So we must attempt to resolve the UStruct before calling resolve_raw_data_funcs.
  UStruct * ue_struct_or_class_p = get_ue_struct_or_class_from_sk_class(sk_class_p);

  // Resolve function pointers and determine if there's anything to do
  if (!resolve_raw_data_funcs(sk_class_p, ue_struct_or_class_p))
    {
    return true;
    }

  // Check if it's known
  if (ue_struct_or_class_p)
    {
    // Resolve raw data
    resolve_raw_data(sk_class_p, ue_struct_or_class_p);
    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::resolve_raw_data_struct(SkClass * sk_class_p, const TCHAR * ue_struct_name_p)
  {
  resolve_raw_data(sk_class_p, FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), ue_struct_name_p, false));
  }

//---------------------------------------------------------------------------------------
// Returns if there's anything to resolve
bool SkUEClassBindingHelper::resolve_raw_data_funcs(SkClass * sk_class_p, UStruct * ue_struct_or_class_p)
  {
  if (ue_struct_or_class_p && ue_struct_or_class_p->IsA<UUserDefinedStruct>())
    {
    // Always re-resolve UUserDefinedStructs as they might change size
    sk_class_p->register_raw_pointer_func(SkInstance::is_data_stored_by_val(ue_struct_or_class_p->PropertiesSize) 
      ? static_cast<tSkRawPointerFunc>(&SkInstance::get_raw_pointer_val) 
      : static_cast<tSkRawPointerFunc>(&SkInstance::get_raw_pointer_ref));
    sk_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_user_struct);
    }
  else if ((sk_class_p->get_annotation_flags() & SkAnnotation_reflected_data) && sk_class_p->is_class(ASymbolId_UStruct))
    {
    // This appears to be a UUserDefinedStruct but is UE size is unknown, so use placeholder raw_pointer_func
    sk_class_p->register_raw_pointer_func(&SkUEClassBindingHelper::get_raw_pointer_null);
    sk_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_user_struct);
    }
  else
    {
    // By default, inherit raw pointer and accessor functions from super class
    if (!sk_class_p->get_raw_pointer_func())
      {
      sk_class_p->register_raw_pointer_func(sk_class_p->get_raw_pointer_func_inherited());
      }
    if (!sk_class_p->get_raw_accessor_func())
      {
      sk_class_p->register_raw_accessor_func(sk_class_p->get_raw_accessor_func_inherited());
      }
    }

  // Return if there's anything to resolve
  bool has_no_raw_data = sk_class_p->get_instance_data_raw().is_empty();
  sk_class_p->set_raw_data_resolved(has_no_raw_data);
  return !has_no_raw_data;
  }

//---------------------------------------------------------------------------------------

tSkRawDataInfo SkUEClassBindingHelper::compute_raw_data_info(UProperty * ue_var_p)
  {
  tSkRawDataInfo raw_data_info;
  raw_data_info.InternalOffset = ue_var_p->GetOffset_ForInternal();
  raw_data_info.Size = ue_var_p->GetSize();

  FSkookumScriptGeneratorHelper::eSkTypeID type_id = FSkookumScriptGeneratorHelper::get_skookum_property_type(ue_var_p, true);
  if (type_id == FSkookumScriptGeneratorHelper::SkTypeID_Integer)
    {
    // If integer, specify sign bit
    if (ue_var_p->IsA<UInt64Property>()
     || ue_var_p->IsA<UIntProperty>()
     || ue_var_p->IsA<UInt16Property>()
     || ue_var_p->IsA<UInt8Property>())
      { // Mark as signed
      raw_data_info.bIsSigned = true;
      }
    }
  else if (type_id == FSkookumScriptGeneratorHelper::SkTypeID_Boolean)
    {
    // If boolean, store bit shift as well
    static_assert(sizeof(HackedBoolProperty) == sizeof(UBoolProperty), "Must match so this hack will work.");
    HackedBoolProperty * bool_var_p = static_cast<HackedBoolProperty *>(ue_var_p);
    SK_ASSERTX(bool_var_p->FieldSize != 0, "BoolProperty must be initialized.");
    
    raw_data_info.InternalOffset = ue_var_p->GetOffset_ForInternal() + bool_var_p->ByteOffset;
    raw_data_info.Size = 1;
    raw_data_info.BoolByteMask = a_ceil_log_2((uint)bool_var_p->ByteMask);
    }
  else if (type_id == FSkookumScriptGeneratorHelper::SkTypeID_List)
    {
    // If a list, store type information for elements as well
    const UArrayProperty * array_property_p = Cast<UArrayProperty>(ue_var_p);
    tSkRawDataInfo item_raw_data_info = compute_raw_data_info(array_property_p->Inner);
    
    raw_data_info.ListTypeOffset = item_raw_data_info.InternalOffset;
    raw_data_info.ListTypeSize = item_raw_data_info.Size;
    raw_data_info.bListTypeIsWeakPointer = item_raw_data_info.bIsWeakPointer;
    }
  else if (type_id == FSkookumScriptGeneratorHelper::SkTypeID_UObjectWeakPtr)
    {
    raw_data_info.bIsWeakPointer = true;
    }

  return raw_data_info;
  }

//---------------------------------------------------------------------------------------
// Get a raw pointer to the underlying UObject of a Sk Entity
void * SkUEClassBindingHelper::get_raw_pointer_entity(SkInstance * obj_p)
  {
  #if WITH_EDITORONLY_DATA
    // In editor, due to unreliable delegate callbacks, try a JIT resolve of unresolved classes
    SkClass * sk_class_p = obj_p->get_class();
    if (!sk_class_p->is_raw_data_resolved())
      {
      if (SkUEClassBindingHelper::resolve_raw_data_funcs(sk_class_p))
        {
        UClass * ue_class_p = SkUEClassBindingHelper::get_ue_class_from_sk_class(sk_class_p);
        if (ue_class_p)
          {
          SkUEClassBindingHelper::resolve_raw_data(sk_class_p, ue_class_p);
          }
        }
      }
  #endif

  return obj_p->as<SkUEEntity>().get_obj();
  }

//---------------------------------------------------------------------------------------
// Dummy placeholder returning null
void * SkUEClassBindingHelper::get_raw_pointer_null(SkInstance * obj_p)
  {
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Access an Entity/UObject reference
SkInstance * SkUEClassBindingHelper::access_raw_data_entity(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  SK_ASSERTX(raw_data_info.Size == sizeof(UObject *), "Size of data type and data member must match!");

  // We know the data lives here
  uint8_t * data_p = (uint8_t*)obj_p + byte_offset;

  // Check special bit indicating it's stored as a weak pointer
  UObject * entity_p;
  if (raw_data_info.bIsWeakPointer)
    {
    // Stored a weak pointer
    FWeakObjectPtr * weak_p = (FWeakObjectPtr *)data_p;

    // Set or get?
    if (value_p)
      {
      // Set value
      *weak_p = value_p->as<SkUEEntity>();
      return nullptr;
      }

    // Get value
    entity_p = weak_p->Get();
    }
  else
    {
    // Stored as naked pointer
    UObject ** obj_pp = (UObject **)data_p;

    // Set or get?
    if (value_p)
      {
      // Set value
      *obj_pp = value_p->as<SkUEEntity>();
      return nullptr;
      }

    entity_p = *obj_pp;
    }

  // Get value
  SkClass * sk_class_p = get_object_class(entity_p);
  SkInstance * instance_p = get_embedded_instance(entity_p, sk_class_p);
  if (instance_p)
    {
    instance_p->reference();
    }
  else
    {
    if (!sk_class_p)
      {
      sk_class_p = data_type_p->get_key_class();
      }
    // If (entity_p is valid and not pending kill) and class has data, get_embedded_instance() above should have succeeded
    // When UObjects are destroyed they set flag PENDING_KILL and then get fully destroyed on some later tick. This can cause
    // data count to be zero while the entity is still valid.
    SK_ASSERTX((!entity_p || entity_p->IsPendingKill()) || !sk_class_p->get_total_data_count(), a_str_format("Class '%s' has data but UObject has no embedded instance!", sk_class_p->get_name_cstr()));

    // We always allocate a simple SkInstance even if class has data 
    // because either entity_p is null in which case the data should not be accessed, 
    // or get_embedded_instance() above would have succeeded
    instance_p = SkInstance::new_instance(sk_class_p);
    new (instance_p) SkRawDataInstance(ALeaveMemoryUnchanged);
    instance_p->construct<SkUEEntity>(entity_p);
    }
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Access a Boolean
SkInstance * SkUEClassBindingHelper::access_raw_data_boolean(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  int32_t byte_size = raw_data_info.Size;
  uint8_t bit_shift = raw_data_info.BoolByteMask;
  
  uint8_t * data_p = (uint8_t*)obj_p + byte_offset;

  // Set or get?
  uint32_t bit_mask = 1 << bit_shift;
  if (value_p)
    {
    // Set value
    uint8_t data = *data_p & ~bit_mask;
    if (value_p->as<SkBoolean>())
      {
      data |= bit_mask;
      }
    *data_p = data;
    return nullptr;
    }

  // Get value
  return SkBoolean::new_instance(*data_p & bit_mask);
  }

//---------------------------------------------------------------------------------------
// Access an Integer
SkInstance * SkUEClassBindingHelper::access_raw_data_integer(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  int32_t byte_size = raw_data_info.Size;
  uint32_t is_signed = raw_data_info.bIsSigned;
  SK_ASSERTX(byte_size == 1 || byte_size == 2 || byte_size == 4 || byte_size == 8, "Integer must have proper size.");

  uint8_t * raw_data_p = (uint8_t*)obj_p + byte_offset;

  // Set or get?
  if (value_p)
    {
    // Set value
    tSkInteger value = value_p->as<SkInteger>();
    if (byte_size == 4)
      {
      if (is_signed)
        {
        *(int32_t *)raw_data_p = value;
        }
      else
        {
        *(uint32_t *)raw_data_p = value;
        }
      }
    else if (byte_size == 2)
      {
      if (is_signed)
        {
        *(int16_t *)raw_data_p = (int16_t)value;
        }
      else
        {
        *(uint16_t *)raw_data_p = (uint16_t)value;
        }
      }
    else if (byte_size == 1)
      {
      if (is_signed)
        {
        *(int8_t *)raw_data_p = (int8_t)value;
        }
      else
        {
        *(uint8_t *)raw_data_p = (uint8_t)value;
        }
      }
    else // byte_size == 8
      {
      if (is_signed)
        {
        *(int64_t *)raw_data_p = value;
        }
      else
        {
        *(uint64_t *)raw_data_p = value;
        }
      }

    return nullptr;
    }

  // Get value
  tSkInteger value;
  if (byte_size == 4)
    {
    if (is_signed)
      {
      value = *(int32_t *)raw_data_p;
      }
    else
      {
      value = *(uint32_t *)raw_data_p;
      }
    }
  else if (byte_size == 2)
    {
    if (is_signed)
      {
      value = *(int16_t *)raw_data_p;
      }
    else
      {
      value = *(uint16_t *)raw_data_p;
      }
    }
  else if (byte_size == 1)
    {
    if (is_signed)
      {
      value = *(int8_t *)raw_data_p;
      }
    else
      {
      value = *(uint8_t *)raw_data_p;
      }
    }
  else // byte_size == 8
    {
    if (is_signed)
      {
      value = (tSkInteger)*(int64_t *)raw_data_p;
      }
    else
      {
      value = (tSkInteger)*(uint64_t *)raw_data_p;
      }
    }

  return SkInteger::new_instance(value);
  }

//---------------------------------------------------------------------------------------
// Access a String
SkInstance * SkUEClassBindingHelper::access_raw_data_string(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  SK_ASSERTX(raw_data_info.Size == sizeof(FString), "Size of data type and data member must match!");

  FString * data_p = (FString *)((uint8_t*)obj_p + byte_offset);

  // Set or get?
  if (value_p)
    {
    // Set value
    *data_p = AStringToFString(value_p->as<SkString>());
    return nullptr;
    }

  // Get value
  return SkString::new_instance(FStringToAString(*data_p));
  }

//---------------------------------------------------------------------------------------
// Access an Enum
SkInstance * SkUEClassBindingHelper::access_raw_data_enum(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  SK_ASSERTX(raw_data_info.Size == 1, "Only byte enums supported at this point!");

  uint8_t * data_p = (uint8_t*)obj_p + byte_offset;

  // Set or get?
  if (value_p)
    {
    // Set value
    *data_p = (uint8_t)value_p->as<SkEnum>();
    return nullptr;
    }

  // Get value
  return SkEnum::new_instance(*data_p, data_type_p->get_key_class());
  }

//---------------------------------------------------------------------------------------
// Access a Color
SkInstance * SkUEClassBindingHelper::access_raw_data_color(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_offset = raw_data_info.InternalOffset;
  int32_t byte_size = raw_data_info.Size;
  SK_ASSERTX(byte_size == sizeof(FColor) || byte_size == sizeof(FLinearColor), "Size of data type and data member must match!");

  // FColor or FLinearColor?
  if (byte_size == sizeof(FColor))
    {
    // FColor
    FColor * data_p = (FColor *)((uint8_t*)obj_p + byte_offset);

    // Set or get?
    if (value_p)
      {
      // Set value
      *data_p = value_p->as<SkColor>().ToFColor(true);
      return nullptr;
      }

    // Get value
    return SkColor::new_instance(*data_p);
    }

  // FLinearColor
  FLinearColor * data_p = (FLinearColor *)((uint8_t*)obj_p + byte_offset);

  // Set or get?
  if (value_p)
    {
    // Set value
    *data_p = value_p->as<SkColor>();
    return nullptr;
    }

  // Get value
  return SkColor::new_instance(*data_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEClassBindingHelper::access_raw_data_list(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  SK_ASSERTX(raw_data_info.Size == sizeof(TArray<void *>), "Size of data type and data member must match!");

  HackedTArray *    data_p             = (HackedTArray *)((uint8_t*)obj_p + raw_data_info.InternalOffset);
  SkClassDescBase * item_type_p        = data_type_p->get_item_type();
  SkClass *         item_class_p       = item_type_p->get_key_class();

  tSkRawDataInfo    item_raw_data_info;
  item_raw_data_info.InternalOffset = raw_data_info.ListTypeOffset;
  item_raw_data_info.Size = raw_data_info.ListTypeSize;
  item_raw_data_info.bIsWeakPointer = raw_data_info.bListTypeIsWeakPointer;

  // Set or get?
  if (value_p)
    {
    // Set value
    SkInstanceList & list = value_p->as<SkList>();
    APArray<SkInstance> & list_instances = list.get_instances();
    uint32_t num_elements = list_instances.get_length();
    data_p->resize_uninitialized(num_elements, item_raw_data_info.Size);
    uint8_t * item_array_p = (uint8_t *)data_p->GetData();
    for (uint32_t i = 0; i < num_elements; ++i)
      {
      item_class_p->assign_raw_data(item_array_p, item_raw_data_info, item_type_p, list_instances[i]);
      item_array_p += item_raw_data_info.Size;
      }
    return nullptr;
    }

  // Get value
  SkInstance * instance_p = SkList::new_instance(data_p->Num());
  SkInstanceList & list = instance_p->as<SkList>();
  APArray<SkInstance> & list_instances = list.get_instances();
  uint8_t * item_array_p = (uint8_t *)data_p->GetData();
  for (uint32_t i = data_p->Num(); i; --i)
    {
    list_instances.append(*item_class_p->new_instance_from_raw_data(item_array_p, item_raw_data_info, item_type_p));
    item_array_p += item_raw_data_info.Size;
    }
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEClassBindingHelper::access_raw_data_user_struct(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  int32_t byte_size = raw_data_info.Size;
  int32_t byte_offset = raw_data_info.InternalOffset;
  void * ue_data_p = (uint8_t*)obj_p + byte_offset;

  SK_ASSERTX(data_type_p->get_class_type() == SkClassType_class, a_str_format("Sk type of struct '%s' is not a class!", data_type_p->get_key_class_name().as_cstr()));
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(static_cast<SkClass *>(data_type_p));
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", data_type_p->get_key_class_name().as_cstr()));

  // Set or get?
  if (value_p)
    {
    // Set value
    #ifdef SK_RUNTIME_RECOVER
      if (!ue_struct_p)
        {
        // Unknown struct - use memcpy and hope that works
        FMemory::Memcpy(ue_data_p, value_p->get_raw_pointer(byte_size), byte_size);
        }
      else
    #endif
        {
        // Use proper copy that correctly copies arrays etc.
        ue_struct_p->InitializeStruct(ue_data_p);
        ue_struct_p->CopyScriptStruct(ue_data_p, value_p->get_raw_pointer(byte_size));
        }
    return nullptr;
    }

  // Get value
  SkInstance * instance_p = SkInstance::new_instance(data_type_p->get_key_class());
  void * sk_data_p = instance_p->allocate_raw(byte_size);
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Unknown struct - use memcpy and hope that works
      FMemory::Memcpy(sk_data_p, ue_data_p, byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->InitializeStruct(sk_data_p);
      ue_struct_p->CopyScriptStruct(sk_data_p, ue_data_p);
      }
  return instance_p;
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::reset_static_class_mappings(uint32_t reserve)
  {
  ms_static_class_map_u2s.Reset();
  ms_static_class_map_s2u.Reset();
  ms_static_class_map_u2s.Reserve(reserve);
  ms_static_class_map_s2u.Reserve(reserve);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::reset_static_struct_mappings(uint32_t reserve)
  {
  ms_static_struct_map_u2s.Reset();
  ms_static_struct_map_s2u.Reset();
  ms_static_struct_map_u2s.Reserve(reserve);
  ms_static_struct_map_s2u.Reserve(reserve);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::reset_static_enum_mappings(uint32_t reserve)
  {
  ms_static_enum_map_u2s.Reset();
  ms_static_enum_map_u2s.Reserve(reserve);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_slack_to_static_class_mappings(uint32_t slack)
  {
  ms_static_class_map_u2s.Reserve(ms_static_class_map_u2s.Num() + slack);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_slack_to_static_struct_mappings(uint32_t slack)
  {
  ms_static_struct_map_u2s.Reserve(ms_static_struct_map_u2s.Num() + slack);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_slack_to_static_enum_mappings(uint32_t slack)
  {
  ms_static_enum_map_u2s.Reserve(ms_static_enum_map_u2s.Num() + slack);
  }

//---------------------------------------------------------------------------------------
// Forget all Sk classes

void SkUEClassBindingHelper::forget_sk_classes_in_all_mappings()
  {
  // Set all sk classes to null in ue->sk maps
  for (auto pair_iter = ms_static_class_map_u2s.CreateIterator(); pair_iter; ++pair_iter)
    {
    pair_iter.Value() = nullptr;
    }
  for (auto pair_iter = ms_static_struct_map_u2s.CreateIterator(); pair_iter; ++pair_iter)
    {
    pair_iter.Value() = nullptr;
    }
  for (auto pair_iter = ms_static_enum_map_u2s.CreateIterator(); pair_iter; ++pair_iter)
    {
    pair_iter.Value() = nullptr;
    }  

  // Completely clear sk->ue map
  ms_static_class_map_s2u.Reset();
  ms_static_struct_map_s2u.Reset();
  //ms_static_enum_map_s2u.Reset();

  // Also clear out dynamic mappings if we got any
  #if WITH_EDITORONLY_DATA
    reset_dynamic_class_mappings();
  #endif
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::register_static_class(UClass * ue_class_p)
  {
  ms_static_class_map_u2s.Add(ue_class_p, nullptr);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::register_static_struct(UStruct * ue_struct_p)
  {
  ms_static_struct_map_u2s.Add(ue_struct_p, nullptr);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::register_static_enum(UEnum * ue_enum_p)
  {
  ms_static_enum_map_u2s.Add(ue_enum_p, nullptr);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_static_class_mapping(SkClass * sk_class_p, UClass * ue_class_p)
  {
  SK_ASSERTX(sk_class_p && ue_class_p, a_str_format("Tried to add static class mapping between `%s` and `%S` one of which is null.", sk_class_p ? sk_class_p->get_name_cstr() : "(null)", ue_class_p ? *ue_class_p->GetName() : TEXT("(null)")));
  ms_static_class_map_u2s.Add(ue_class_p, sk_class_p);
  ms_static_class_map_s2u.Add(sk_class_p, ue_class_p);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_static_struct_mapping(SkClass * sk_class_p, UStruct * ue_struct_p)
  {
  SK_ASSERTX(sk_class_p && ue_struct_p, a_str_format("Tried to add static class mapping between `%s` and `%S` one of which is null.", sk_class_p ? sk_class_p->get_name_cstr() : "(null)", ue_struct_p ? *ue_struct_p->GetName() : TEXT("(null)")));
  ms_static_struct_map_u2s.Add(ue_struct_p, sk_class_p);
  ms_static_struct_map_s2u.Add(sk_class_p, ue_struct_p);
  }

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::add_static_enum_mapping(SkClass * sk_class_p, UEnum * ue_enum_p)
  {
  SK_ASSERTX(sk_class_p && ue_enum_p, a_str_format("Tried to add static class mapping between `%s` and `%S` one of which is null.", sk_class_p ? sk_class_p->get_name_cstr() : "(null)", ue_enum_p ? *ue_enum_p->GetName() : TEXT("(null)")));
  ms_static_enum_map_u2s.Add(ue_enum_p, sk_class_p);
  }

//---------------------------------------------------------------------------------------

UClass * SkUEClassBindingHelper::find_ue_class_from_sk_class(SkClass * sk_class_p)
  {
  // Only UObjects have UE4 equivalents
  if (!sk_class_p->is_entity_class())
    {
    return nullptr;
    }

  UClass * ue_class_p;
  FName bind_name = reinterpret_cast<const FName &>(sk_class_p->get_bind_name());
  if (bind_name.IsNone())
    {
    // Legacy heuristic code - remove 9/2017

    // Convert class name to its UE4 equivalent
    TCHAR ue_class_name[260];
    const ANSICHAR * sk_class_name_p;
    switch (sk_class_p->get_name_id())
      {
      case ASymbolId_Entity:      sk_class_name_p = "Object"; break;
      case ASymbolId_EntityClass: sk_class_name_p = "Class"; break;
      case ASymbolId_GameEntity:  sk_class_name_p = "Entity"; break;
      default:                    sk_class_name_p = sk_class_p->get_name_cstr(); break;
      }
    uint32_t sk_class_name_length = FPlatformString::Strlen(sk_class_name_p);
    SK_ASSERTX(sk_class_name_length < A_COUNT_OF(ue_class_name) - 3, a_str_format("Name of class '%s' is too large for conversion buffer.", sk_class_name_p));
    FPlatformString::Convert(ue_class_name, A_COUNT_OF(ue_class_name), sk_class_name_p, sk_class_name_length + 1);
    if (sk_class_p->get_annotation_flags() & SkAnnotation_reflected_data)
      {
      // It's a Blueprint generated class - append "_C" to the name
      FPlatformString::Strncpy(ue_class_name + sk_class_name_length, TEXT("_C"), 3);
      }
    bind_name = FName(ue_class_name);
    }

  // Now look it up (case-insensitive, allow failure)
  ue_class_p = FindObject<UClass>(ANY_PACKAGE, *bind_name.ToString());

  // Link classes together for future use
  if (ue_class_p)
    {
    // Check that parent classes match, as we might have gotten the wrong class due to case-insensitive search
    UClass * ue_superclass_p = get_ue_class_from_sk_class(sk_class_p->get_superclass());
    if (ue_superclass_p && ue_superclass_p != ue_class_p->GetSuperClass())
      {
      // Parent classes differ - do slooooow search for proper class
      // This will happen _extremely seldom_ so no performance worry
      for (TObjectIterator<UClass> class_it; class_it; ++class_it)
        {
        if (class_it->GetFName() == bind_name && class_it->GetSuperClass() == ue_superclass_p)
          {
          ue_class_p = *class_it;
          break;
          }
        }
      }

    // Store pointer to UClass inside the SkClass
    set_ue_struct_ptr_on_sk_class(sk_class_p, ue_class_p);

    // Store index of SkClass inside the UClass
    uint32_t sk_class_idx = 0;
    SkBrain::get_classes().find(sk_class_p->get_name(), AMatch_first_found, &sk_class_idx);
    set_sk_class_idx_on_ue_class(ue_class_p, sk_class_idx);
    }

  return ue_class_p;
  }

//---------------------------------------------------------------------------------------

UScriptStruct * SkUEClassBindingHelper::find_ue_struct_from_sk_class(SkClass * sk_class_p)
  {
  UScriptStruct * ue_struct_p;
  FName bind_name = reinterpret_cast<const FName &>(sk_class_p->get_bind_name());
  if (bind_name.IsNone())
    {
    // Legacy heuristic code - remove 9/2017

    // Convert struct name to its UE4 equivalent
    TCHAR ue_struct_name[260];
    const ANSICHAR * sk_struct_name_p;
    switch (sk_class_p->get_name_id())
      {
      case ASymbolId_Vector2:         sk_struct_name_p = "Vector2D"; break;
      case ASymbolId_Vector3:         sk_struct_name_p = "Vector"; break;
      case ASymbolId_Rotation:        sk_struct_name_p = "Quat"; break;
      case ASymbolId_RotationAngles:  sk_struct_name_p = "Rotator"; break;
      default:                        sk_struct_name_p = sk_class_p->get_name_cstr(); break;
      }
    uint32_t sk_struct_name_length = FPlatformString::Strlen(sk_struct_name_p);
    SK_ASSERTX(sk_struct_name_length < A_COUNT_OF(ue_struct_name), a_str_format("Name of struct '%s' is too large for conversion buffer.", sk_struct_name_p));
    FPlatformString::Convert(ue_struct_name, A_COUNT_OF(ue_struct_name), sk_struct_name_p, sk_struct_name_length + 1);
    bind_name = FName(ue_struct_name);
    }
    
  // Now look it up (case-insensitive, allow failure)
  ue_struct_p = FindObject<UScriptStruct>(ANY_PACKAGE, *bind_name.ToString());

  // Link structures together for future use
  if (ue_struct_p)
    {
    // Store pointer to UScriptStruct inside the SkClass
    set_ue_struct_ptr_on_sk_class(sk_class_p, ue_struct_p);
    }

  return ue_struct_p;
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEClassBindingHelper::find_sk_class_from_ue_class(UClass * ue_class_p)
  {
  if (!ue_class_p) { return nullptr; }

  bool is_temp_ue_class = false; 
  // Convert class name to its Sk equivalent
  FName ue_class_name = ue_class_p->GetFName();
  ASymbol sk_class_name;
  if (ue_class_name == NAME_Object)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_Entity);
    }
  else if (ue_class_name == NAME_Class)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_EntityClass);
    }
  else if (ue_class_name == NAME_Entity)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_GameEntity);
    }
  else
    {
    FString ue_name_string = ue_class_name.GetPlainNameString();
    if (ue_class_p->UObject::IsA<UBlueprintGeneratedClass>())
      {
      // It's a Blueprint generated class
      // Remove REINST_ prefix and _C postfix if present
      if (ue_name_string.StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
        {
        ue_name_string = ue_name_string.RightChop(7);
        }
      if (ue_name_string.EndsWith(TEXT("_C"), ESearchCase::CaseSensitive))
        {
        ue_name_string = ue_name_string.LeftChop(2);
        }
      }
    
    sk_class_name = ASymbol::create_existing(*ue_name_string);
    }

  // Now look up the class
  uint32_t sk_class_idx = 0;
  SkClass * sk_class_p = SkBrain::get_classes().get(sk_class_name, AMatch_first_found, &sk_class_idx);

  // Verify bind name
  if (!is_temp_ue_class 
   && (!sk_class_p || reinterpret_cast<const FName &>(sk_class_p->get_bind_name()) != ue_class_p->GetFName()))
    {
    // Does not match - try to look up the class by bind name
    FName ue_name(ue_class_p->GetFName());
    for (SkClass * class_p : SkBrain::get_classes())
      {
      if (reinterpret_cast<const FName &>(class_p->get_bind_name()) == ue_name)
        {
        sk_class_p = class_p;
        goto FoundClass;
        }
      }

    // Didn't find it
    return nullptr;
    }

  // Link classes together for future use
  if (sk_class_p)
    {
  FoundClass:
    // Store pointer to UClass inside the SkClass
    if (!is_temp_ue_class)
      {
      set_ue_struct_ptr_on_sk_class(sk_class_p, ue_class_p);
      }

    // Store index of SkClass inside the UClass
    set_sk_class_idx_on_ue_class(ue_class_p, sk_class_idx);
    }

  return sk_class_p;
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEClassBindingHelper::find_sk_class_from_ue_struct(UStruct * ue_struct_p)
  {
  // Convert struct name to its Sk equivalent
  FName ue_struct_name = ue_struct_p->GetFName();
  ASymbol sk_class_name;
  if (ue_struct_name == NAME_Vector2D)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_Vector2);
    }
  else if (ue_struct_name == NAME_Vector)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_Vector3);
    }
  else if (ue_struct_name == NAME_Quat)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_Rotation);
    }
  else if (ue_struct_name == NAME_Rotator)
    {
    sk_class_name = ASymbol::create_existing(ASymbolId_RotationAngles);
    }
  else
    {
    sk_class_name = ASymbol::create_existing(*ue_struct_name.GetPlainNameString());
    }

  // Now look up the struct's class
  SkClass * sk_class_p = SkBrain::get_classes().get(sk_class_name);

  // Link struct to class for future use
  if (sk_class_p)
    {
    // Store pointer to UStruct inside the SkClass
    set_ue_struct_ptr_on_sk_class(sk_class_p, ue_struct_p);
    }

  return sk_class_p;
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEClassBindingHelper::find_sk_class_from_ue_enum(UEnum * ue_enum_p)
  {
  // Convert enum name to its Sk equivalent
  ASymbol sk_enum_name = ASymbol::create_existing(*ue_enum_p->GetFName().GetPlainNameString());

  // Now lookup the class
  SkClass * sk_class_p = SkBrain::get_classes().get(sk_enum_name);

  SK_ASSERTX(!sk_class_p || sk_class_p->is_class(*SkEnum::get_class()), a_str_format("UEnum '%S' is not an Sk Enum!", *ue_enum_p->GetName()));

  return sk_class_p;
  }

#if WITH_EDITORONLY_DATA

//---------------------------------------------------------------------------------------

void SkUEClassBindingHelper::reset_dynamic_class_mappings()
  {
  ms_dynamic_class_map_u2s.Reset();
  ms_dynamic_class_map_s2u.Reset();
  }

//---------------------------------------------------------------------------------------

UClass * SkUEClassBindingHelper::add_dynamic_class_mapping(SkClassDescBase * sk_class_desc_p)
  {
  // Get fully derived SkClass
  SkClass * sk_class_p = sk_class_desc_p->get_key_class();

  // Dynamic classes have blueprints - look it up by name
  FString class_name(sk_class_p->get_name_cstr());
  UBlueprint * blueprint_p = FindObject<UBlueprint>(ANY_PACKAGE, *class_name);
  if (!blueprint_p)
    {
    return nullptr;
    }

  // Add to map of known class equivalences
  ms_dynamic_class_map_u2s.Add(blueprint_p, sk_class_p);
  ms_dynamic_class_map_s2u.Add(sk_class_p, blueprint_p);

  // Return latest generated class belonging to this blueprint
  return blueprint_p->GeneratedClass;
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEClassBindingHelper::add_dynamic_class_mapping(UBlueprint * blueprint_p)
  {
  // Look up SkClass by blueprint name
  AString class_name(*blueprint_p->GetName(), blueprint_p->GetName().Len());
  SkClass * sk_class_p = SkBrain::get_class(ASymbol::create(class_name, ATerm_short));

  // If found, add to map of known class equivalences
  if (sk_class_p)
    {
    ms_dynamic_class_map_u2s.Add(blueprint_p, sk_class_p);
    ms_dynamic_class_map_s2u.Add(sk_class_p, blueprint_p);
    }

  return sk_class_p;
  }

#endif

//---------------------------------------------------------------------------------------

UClass * SkUEClassBindingHelper::add_static_class_mapping(SkClassDescBase * sk_class_desc_p)
  {
  // Get fully derived SkClass
  SkClass * sk_class_p = sk_class_desc_p->get_key_class();

  // Look up the plain class name first (i.e. a C++ class)
  FString class_name(sk_class_p->get_name_cstr());
  UClass * ue_class_p = FindObject<UClass>(ANY_PACKAGE, *class_name);
  if (ue_class_p)
    {
    add_static_class_mapping(sk_class_p, ue_class_p);
    return ue_class_p;
    }

  // In cooked builds, also look up class name + "_C"
  // We don't do this in editor builds as these classes are dynamically generated and might get deleted at any time
  // there we use dynamic mapping of Blueprints instead
  #if !WITH_EDITORONLY_DATA
    ue_class_p = FindObject<UClass>(ANY_PACKAGE, *(class_name + TEXT("_C")));
    if (ue_class_p)
      {
      add_static_class_mapping(sk_class_p, ue_class_p);
      }
  #endif

  return ue_class_p;
  }

//---------------------------------------------------------------------------------------

SkClass * SkUEClassBindingHelper::add_static_class_mapping(UClass * ue_class_p)
  {
  // Look up SkClass by class name
  const FString & ue_class_name = ue_class_p->GetName();
  int32 ue_class_name_len = ue_class_name.Len();
  AString class_name(*ue_class_name, ue_class_name_len);
  ASymbol class_symbol = ASymbol::create_existing(class_name);
  if (!class_symbol.is_null())
    {
    SkClass * sk_class_p = SkBrain::get_class(class_symbol);
    if (sk_class_p)
      {
      // Add to map of known class equivalences
      add_static_class_mapping(sk_class_p, ue_class_p);
      return sk_class_p;
      }
    }

  // In cooked builds, also look up class name - "_C"
  // We don't do this in editor builds as these classes are dynamically generated and might get deleted at any time
  // there we use dynamic mapping of Blueprints instead
  #if !WITH_EDITORONLY_DATA

    // When we get to this function, we are looking for a class name that has "_C" appended at the end
    // So we subtract two from the length to truncate those two characters
    // We don't check here if the last two characters actually _are_ "_C" because it does not matter
    // since in that case it would be an error anyway
    if (ue_class_name_len < 3) return nullptr;
    class_name.set_length(ue_class_name_len - 2);
    class_symbol = ASymbol::create_existing(class_name);
    if (!class_symbol.is_null())
      {
      SkClass * sk_class_p = SkBrain::get_class(ASymbol::create_existing(class_name));
      if (sk_class_p)
        {
        // Add to map of known class equivalences
        add_static_class_mapping(sk_class_p, ue_class_p);
        return sk_class_p;
        }
      }

  #endif

  return nullptr;
  }

//=======================================================================================
// SkUEClassBindingHelper::HackedTArray
//=======================================================================================

FORCENOINLINE void SkUEClassBindingHelper::HackedTArray::resize_to(int32 new_max, int32 num_bytes_per_element)
  {
  if (new_max)
    {
    new_max = AllocatorInstance.CalculateSlackReserve(new_max, num_bytes_per_element);
    }
  if (new_max != ArrayMax)
    {
    ArrayMax = new_max;
    AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, num_bytes_per_element);
    }
  }
