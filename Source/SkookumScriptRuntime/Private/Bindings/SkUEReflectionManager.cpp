// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Class for interfacing with UE4 Blueprint graphs 
//=======================================================================================

#include "SkUEReflectionManager.hpp"
#include "VectorMath/SkVector2.hpp"
#include "VectorMath/SkVector3.hpp"
#include "VectorMath/SkVector4.hpp"
#include "VectorMath/SkRotationAngles.hpp"
#include "VectorMath/SkTransform.hpp"
#include "Engine/SkUEEntity.hpp"
#include "Engine/SkUEActor.hpp"
#include "SkUEUtils.hpp"
#include "SkookumScriptInstanceProperty.h"
#include "../../../SkookumScriptGenerator/Private/SkookumScriptGeneratorBase.h"

#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "Engine/UserDefinedStruct.h"
#include "Modules/ModuleManager.h"
#if WITH_EDITOR
#include "Kismet2/KismetReinstanceUtilities.h"
#endif

#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkEnum.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkReal.hpp>

//=======================================================================================
// Helper classes
//=======================================================================================

class Hacked_FScriptDelegate : public FScriptDelegate
  {
  public:
    UFunction * get_signature() const
      {
      UObject * obj_p = Object.Get();
      return obj_p->FindFunctionChecked(FunctionName);
      }
  };

class Hacked_FMulticastScriptDelegate : public FMulticastScriptDelegate
  {
  public:
    UFunction * get_signature() const
      {
      return static_cast<const Hacked_FScriptDelegate &>(InvocationList[0]).get_signature();
      }
  };

//=======================================================================================
// TypedName Implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

SkUEReflectionManager::TypedName::TypedName(const ASymbol & name, SkClassDescBase * sk_type_p)
  : ANamed(name)
  , m_byte_size(0) // Yet unknown
  {
  SkClass * sk_class_p = sk_type_p->get_key_class();
  eContainerType container_type = ContainerType_scalar;
  if (sk_class_p == SkBrain::ms_list_class_p)
    {
    container_type = ContainerType_array;
    sk_class_p = sk_type_p->get_item_type()->get_key_class();
    }
  m_sk_class_p = sk_class_p;
  m_sk_class_name = sk_class_p->get_name();
  m_container_type = container_type;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::TypedName::set_byte_size(UProperty * ue_property_p)
  {
  if (ue_property_p)
    {
    UArrayProperty * array_property_p = Cast<UArrayProperty>(ue_property_p);
    m_byte_size = array_property_p ? array_property_p->Inner->GetSize() : ue_property_p->GetSize();
    }
  }

//=======================================================================================
// SkUEReflectionManager Implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

SkUEReflectionManager * SkUEReflectionManager::ms_singleton_p;

SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_boolean         = { &fetch_k2_param_boolean        , &fetch_k2_value_boolean        , &assign_k2_value_boolean        , &store_sk_value_boolean         };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_integer         = { &fetch_k2_param_integer        , &fetch_k2_value_integer        , &assign_k2_value_integer        , &store_sk_value_integer         };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_real            = { &fetch_k2_param_real           , &fetch_k2_value_real           , &assign_k2_value_real           , &store_sk_value_real            };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_string          = { &fetch_k2_param_string         , &fetch_k2_value_string         , &assign_k2_value_string         , &store_sk_value_string          };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_vector2         = { &fetch_k2_param_vector2        , &fetch_k2_value_vector2        , &assign_k2_value_vector2        , &store_sk_value_vector2         };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_vector3         = { &fetch_k2_param_vector3        , &fetch_k2_value_vector3        , &assign_k2_value_vector3        , &store_sk_value_vector3         };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_vector4         = { &fetch_k2_param_vector4        , &fetch_k2_value_vector4        , &assign_k2_value_vector4        , &store_sk_value_vector4         };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_rotation_angles = { &fetch_k2_param_rotation_angles, &fetch_k2_value_rotation_angles, &assign_k2_value_rotation_angles, &store_sk_value_rotation_angles };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_transform       = { &fetch_k2_param_transform      , &fetch_k2_value_transform      , &assign_k2_value_transform      , &store_sk_value_transform       };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_struct_val      = { &fetch_k2_param_struct_val     , &fetch_k2_value_struct_val     , &assign_k2_value_struct_val     , &store_sk_value_struct_val      };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_struct_ref      = { &fetch_k2_param_struct_ref     , &fetch_k2_value_struct_ref     , &assign_k2_value_struct_ref     , &store_sk_value_struct_ref      };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_entity          = { &fetch_k2_param_entity         , &fetch_k2_value_entity         , &assign_k2_value_entity         , &store_sk_value_entity          };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_enum            = { &fetch_k2_param_enum           , &fetch_k2_value_enum           , &assign_k2_value_enum           , &store_sk_value_enum            };
SkUEReflectionManager::ReflectedAccessors const SkUEReflectionManager::ms_accessors_array           = { &fetch_k2_param_array          , nullptr                        , &assign_k2_value_array          , &store_sk_value_array           };

UScriptStruct * SkUEReflectionManager::ms_struct_vector2_p;
UScriptStruct * SkUEReflectionManager::ms_struct_vector3_p;
UScriptStruct * SkUEReflectionManager::ms_struct_vector4_p;
UScriptStruct * SkUEReflectionManager::ms_struct_rotation_angles_p;
UScriptStruct * SkUEReflectionManager::ms_struct_transform_p;

//---------------------------------------------------------------------------------------

SkUEReflectionManager::SkUEReflectionManager()
  {
  SK_ASSERTX(!ms_singleton_p, "There can be only one instance of this class.");
  ms_singleton_p = this;

  ms_struct_vector2_p          = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Vector2D"), false);
  ms_struct_vector3_p          = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Vector"), false);
  ms_struct_vector4_p          = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Vector4"), false);
  ms_struct_rotation_angles_p  = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Rotator"), false);
  ms_struct_transform_p        = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetOutermost(), TEXT("Transform"), false);

  m_result_name = ASymbol::create("result");

  // Get package to attach reflected classes to
  m_module_package_p = FindObject<UPackage>(nullptr, TEXT("/Script/SkookumScriptRuntime"));
  SK_ASSERTX(m_module_package_p, "SkookumScriptRuntime module package not found!");
  if (!m_module_package_p)
    {
    m_module_package_p = GetTransientPackage();
    }
  }

//---------------------------------------------------------------------------------------

SkUEReflectionManager::~SkUEReflectionManager()
  {
  clear(nullptr);

  SK_ASSERTX_NO_THROW(ms_singleton_p == this, "There can be only one instance of this class.");
  ms_singleton_p = nullptr;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::clear(tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f)
  {
  // Destroy all UFunctions and UProperties we allocated
  for (uint32_t i = 0; i < m_reflected_functions.get_length(); ++i)
    {
    delete_reflected_function(i);
    }

  // And forget pointers to them
  m_reflected_functions.empty();

  // Clear out references in classes
  for (ReflectedClass * reflected_class_p : m_reflected_classes)
    {
    #if WITH_EDITOR
      // Invoke callback for each affected class
      if (!reflected_class_p->m_functions.is_empty() && on_function_removed_from_class_f && reflected_class_p->m_ue_static_class_p.IsValid())
        {
        on_function_removed_from_class_f->invoke(reflected_class_p->m_ue_static_class_p.Get());
        }
    #endif

    reflected_class_p->m_functions.empty();
    }

  // Also forget all cached delegate signatures
  m_reflected_delegates.free_all();
  }

//---------------------------------------------------------------------------------------
// Build list of all &blueprint annotated routines, but do not bind them to UE4 yet
bool SkUEReflectionManager::sync_all_from_sk(tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f)
  {
  // Mark all bindings for delete
  for (ReflectedFunction * reflected_function_p : m_reflected_functions)
    {
    if (reflected_function_p)
      {
      reflected_function_p->m_marked_for_delete_all = true;
      }
    }

  // Traverse Sk classes & structs and gather methods that want to be exposed
  bool anything_changed = sync_class_from_sk_recursively(SkUEEntity::get_class(), on_function_removed_from_class_f);
  anything_changed |= sync_class_from_sk_recursively(SkBrain::get_class("UStruct"), on_function_removed_from_class_f);

  // Now go and delete anything still marked for delete
  for (ReflectedClass * reflected_class_p : m_reflected_classes)
    {
    bool removed_function_from_class = false;
    for (uint32_t i = 0; i < reflected_class_p->m_functions.get_length(); ++i)
      {      
      uint32_t function_index = reflected_class_p->m_functions[i].m_idx;
      ReflectedFunction * reflected_function_p = m_reflected_functions[function_index];
      if (reflected_function_p && reflected_function_p->m_marked_for_delete_all)
        {
        delete_reflected_function(function_index);
        reflected_class_p->m_functions.remove_fast(i--);
        removed_function_from_class = true;
        anything_changed = true;
        }
      }

    #if WITH_EDITOR
      // Invoke callback for each affected class
      if (removed_function_from_class && on_function_removed_from_class_f && reflected_class_p->m_ue_static_class_p.IsValid())
        {
        on_function_removed_from_class_f->invoke(reflected_class_p->m_ue_static_class_p.Get());
        }
    #endif
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------
// Bind all routines in the binding list to UE4 by generating UFunction objects
bool SkUEReflectionManager::sync_class_from_sk(SkClass * sk_class_p, tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f)
  {
  // Find existing methods of this class and mark them for delete
  ReflectedClass * reflected_class_p = m_reflected_classes.get(sk_class_p->get_name());
  if (reflected_class_p)
    {
    for (FunctionIndex function_index : reflected_class_p->m_functions)
      {
      ReflectedFunction * reflected_function_p = m_reflected_functions[function_index.m_idx];
      if (reflected_function_p)
        {
        reflected_function_p->m_marked_for_delete_class = true;
        }
      }
    }

  // Make sure reflected classes exist for all classes that need to store an SkInstance
  if ((!reflected_class_p || !reflected_class_p->m_store_sk_instance)
   && sk_class_p->is_entity_class()
   && does_class_need_instance_property(sk_class_p) 
   && !does_class_need_instance_property(sk_class_p->get_superclass()))
    {
    if (!reflected_class_p)
      {
      reflected_class_p = new ReflectedClass(sk_class_p->get_name());
      m_reflected_classes.append(*reflected_class_p);
      }
    reflected_class_p->m_store_sk_instance = true;
    }

  // Gather new functions/events
  int32_t change_count = 0;
  for (auto method_p : sk_class_p->get_instance_methods())
    {
    change_count += (int32_t)try_add_reflected_function(method_p);
    }
  for (auto method_p : sk_class_p->get_class_methods())
    {
    change_count += (int32_t)try_add_reflected_function(method_p);
    }
  for (auto coroutine_p : sk_class_p->get_coroutines())
    {
    change_count += (int32_t)try_add_reflected_function(coroutine_p);
    }

  // Now go and delete anything still marked for delete
  uint32_t delete_count = 0;
  if (reflected_class_p)
    {
    for (uint32_t i = 0; i < reflected_class_p->m_functions.get_length(); ++i)
      {
      uint32_t function_index = reflected_class_p->m_functions[i].m_idx;
      ReflectedFunction * reflected_function_p = m_reflected_functions[function_index];
      if (reflected_function_p && reflected_function_p->m_marked_for_delete_class)
        {
        delete_reflected_function(function_index);
        reflected_class_p->m_functions.remove_fast(i--);
        ++delete_count;
        }
      }

    // Notify caller
    #if WITH_EDITOR
      if (delete_count && on_function_removed_from_class_f && reflected_class_p->m_ue_static_class_p.IsValid())
        {
        on_function_removed_from_class_f->invoke(reflected_class_p->m_ue_static_class_p.Get());
        }
    #endif
    }

  return (change_count + delete_count > 0);
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::sync_class_from_sk_recursively(SkClass * sk_class_p, tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f)
  {
  // Sync this class
  bool anything_changed = sync_class_from_sk(sk_class_p, on_function_removed_from_class_f);

  // Gather sub classes
  for (SkClass * sk_subclass_p : sk_class_p->get_subclasses())
    {
    anything_changed |= sync_class_from_sk_recursively(sk_subclass_p, on_function_removed_from_class_f);
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::try_add_reflected_function(SkInvokableBase * sk_invokable_p)
  {
  if (sk_invokable_p->get_annotation_flags() & SkAnnotation_ue4_blueprint)
    {
    // If it's a method with no body...
    if (sk_invokable_p->get_invoke_type() == SkInvokable_method_func
     || sk_invokable_p->get_invoke_type() == SkInvokable_method_mthd)
      { // ...it's an event
      return add_reflected_event(static_cast<SkMethodBase *>(sk_invokable_p));
      }
    else if (sk_invokable_p->get_invoke_type() == SkInvokable_method
          || sk_invokable_p->get_invoke_type() == SkInvokable_coroutine)
      { // ...otherwise it's a function/coroutine
      return add_reflected_call(sk_invokable_p);
      }
    else
      {
      SK_ERRORX(a_str_format("Trying to export coroutine %s to Blueprints which is atomic. Currently only scripted coroutines can be invoked via Blueprints.", sk_invokable_p->get_name_cstr()));
      }
    }
  else if (sk_invokable_p->get_scope()->get_annotation_flags() & SkAnnotation_reflected_data)
    {
    // If it's a method with no body inside a Blueprint generated class...
    if (sk_invokable_p->get_invoke_type() == SkInvokable_method_func
     || sk_invokable_p->get_invoke_type() == SkInvokable_method_mthd)
      { // ...it's a Blueprint function or custom event
      return add_reflected_event(static_cast<SkMethodBase *>(sk_invokable_p));
      }    
    }

  // Nothing changed
  return false;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::try_update_reflected_function(SkInvokableBase * sk_invokable_p, ReflectedClass ** out_reflected_class_pp, int32_t * out_function_index_p)
  {
  SK_ASSERTX(out_function_index_p, "Must be non-null");

  const tSkParamList & param_list = sk_invokable_p->get_params().get_param_list();

  // See if we find any compatible entry already present:  
  ReflectedClass * reflected_class_p = m_reflected_classes.get(sk_invokable_p->get_scope()->get_name());
  if (reflected_class_p)
    {
    *out_reflected_class_pp = reflected_class_p;

    for (FunctionIndex function_index : reflected_class_p->m_functions)
      {
      ReflectedFunction * reflected_function_p = m_reflected_functions[function_index.m_idx];
      if (reflected_function_p
       && reflected_function_p->get_name()        == sk_invokable_p->get_name()
       && reflected_function_p->m_is_class_member == sk_invokable_p->is_class_member())
        {
        // There is no overloading in SkookumScript
        // Therefore if the above matches we found our slot
        *out_function_index_p = function_index.m_idx;

        // Can't update if signatures don't match
        if (reflected_function_p->m_num_params != param_list.get_length())
          {
          return false;
          }
        if (reflected_function_p->m_type == ReflectedFunctionType_call)
          {
          ReflectedCall * reflected_call_p = static_cast<ReflectedCall *>(reflected_function_p);
          if (!have_identical_signatures(param_list, reflected_call_p->get_param_array())
           || reflected_call_p->m_result.m_sk_class_name != sk_invokable_p->get_params().get_result_class()->get_key_class()->get_name())
            {
            return false;
            }

          // Re-resolve pointers to parameter types to make sure they point to the correct SkClass objects
          rebind_params_to_sk(param_list, reflected_call_p->get_param_array());
          // Re-resolve result type too
          reflected_call_p->m_result.m_sk_class_p = sk_invokable_p->get_params().get_result_class()->get_key_class();
          }
        else
          {
          ReflectedEvent * reflected_event_p = static_cast<ReflectedEvent *>(reflected_function_p);
          if (!have_identical_signatures(param_list, reflected_event_p->get_param_array()))
            {
            return false;
            }

          // Re-resolve pointers to parameter types to make sure they point to the correct SkClass objects
          rebind_params_to_sk(param_list, reflected_event_p->get_param_array());
          // For events, remember which binding index to invoke...
          sk_invokable_p->set_user_data(function_index.m_idx);
          // ...and which atomic function to use
          bind_event_method(static_cast<SkMethodBase *>(sk_invokable_p));
          }

        // We're good to update
        reflected_function_p->m_sk_invokable_p = sk_invokable_p; // Update Sk method pointer
        reflected_function_p->m_marked_for_delete_class = false; // Keep around
        reflected_function_p->m_marked_for_delete_all = false; // Keep around
        return true; // Successfully updated
        }
      }
    }
  else
    {
    *out_reflected_class_pp = nullptr;
    }

  // No matching entry found at all
  *out_function_index_p = -1;
  return false;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::add_reflected_call(SkInvokableBase * sk_invokable_p)
  {
  // Check if this reflected call already exists, and if so, just update it
  ReflectedClass * reflected_class_p;
  int32_t function_index;
  if (try_update_reflected_function(sk_invokable_p, &reflected_class_p, &function_index))
    {
    return false; // Nothing changed
    }
  if (function_index >= 0)
    {
    delete_reflected_function(function_index);
    reflected_class_p->m_functions.remove(FunctionIndex(function_index));
    }

  // Parameters of the method we are creating
  const SkParameters & params = sk_invokable_p->get_params();
  const tSkParamList & param_list = params.get_param_list();
  uint32_t num_params = param_list.get_length();

  // Allocate reflected call
  ReflectedCall * reflected_call_p = new(FMemory::Malloc(sizeof(ReflectedCall) + num_params * sizeof(ReflectedCallParam))) ReflectedCall(sk_invokable_p, num_params, params.get_result_class());

  // Initialize parameters
  for (uint32_t i = 0; i < num_params; ++i)
    {
    const SkParameterBase * input_param = param_list[i];
    new (&reflected_call_p->get_param_array()[i]) ReflectedCallParam(input_param->get_name(), input_param->get_expected_type());
    }

  // Store reflected call in array
  store_reflected_function(reflected_call_p, reflected_class_p, function_index);

  // This entry changed
  return true;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::add_reflected_event(SkMethodBase * sk_method_p)
  {
  // Check if this reflected event already exists, and if so, just update it
  ReflectedClass * reflected_class_p;
  int32_t function_index;
  if (try_update_reflected_function(sk_method_p, &reflected_class_p, &function_index))
    {
    return false; // Nothing changed
    }
  if (function_index >= 0)
    {
    delete_reflected_function(function_index);
    reflected_class_p->m_functions.remove(FunctionIndex(function_index));
    }

  // Bind Sk method
  bind_event_method(sk_method_p);

  // Parameters of the method we are creating
  const SkParameters & params = sk_method_p->get_params();
  const tSkParamList & param_list = params.get_param_list();
  uint32_t num_params = param_list.get_length();

  // Allocate reflected event
  ReflectedEvent * reflected_event_p = new(FMemory::Malloc(sizeof(ReflectedEvent) + num_params * sizeof(ReflectedEventParam))) ReflectedEvent(sk_method_p, num_params);

  // Initialize parameters
  for (uint32_t i = 0; i < num_params; ++i)
    {
    const SkParameterBase * input_param_p = param_list[i];
    new (&reflected_event_p->get_param_array()[i]) ReflectedEventParam(input_param_p->get_name(), input_param_p->get_expected_type());
    }

  // Store reflected event in array
  store_reflected_function(reflected_event_p, reflected_class_p, function_index);

  // This entry changed
  return true;
  }

//---------------------------------------------------------------------------------------

SkUEReflectionManager::ReflectedDelegate * SkUEReflectionManager::add_reflected_delegate(const SkParameters * sk_params_p, UFunction * ue_function_p)
  {
  SK_ASSERTX(!m_reflected_delegates.get(sk_params_p), "Caller of add_reflected_delegate must ensure it does not already exist.");

  // Allocate reflected event
  const tSkParamList & param_list = sk_params_p->get_param_list();
  uint32_t num_params = param_list.get_length();
  ReflectedDelegate * reflected_delegate_p = new(FMemory::Malloc(sizeof(ReflectedEvent) + num_params * sizeof(ReflectedEventParam))) 
    ReflectedDelegate(sk_params_p, num_params, ue_function_p->HasAllFunctionFlags(FUNC_HasOutParms), ue_function_p->ParmsSize);

  // Allocate ReflectedPropertys to store temporary information
  ReflectedProperty * param_info_array_p = a_stack_allocate(num_params, ReflectedProperty);
  for (uint32_t i = 0; i < num_params; ++i)
    {
    new (param_info_array_p + i) ReflectedProperty();
    }

  // Reflect the parameters
  bool success = reflect_ue_params(*sk_params_p, ue_function_p, param_info_array_p);
  SK_ASSERTX(success, a_str_format("Failed to reflect delegate parameters for '%S'.", *ue_function_p->GetName()));

  // Initialize parameters
  for (uint32_t i = 0; i < num_params; ++i)
    {
    const SkParameterBase * input_param_p = param_list[i];

    ReflectedEventParam * reflected_param_p = new (&reflected_delegate_p->get_param_array()[i])
      ReflectedEventParam(input_param_p->get_name(), input_param_p->get_expected_type());

    const ReflectedProperty & param_info = param_info_array_p[i];
    reflected_param_p->set_byte_size(param_info.m_ue_property_p);
    reflected_param_p->m_outer_storer_p   = param_info.m_outer_p->m_sk_value_storer_p;
    reflected_param_p->m_inner_storer_p   = param_info.m_inner_p ? param_info.m_inner_p->m_sk_value_storer_p : nullptr;
    reflected_param_p->m_outer_assigner_p = param_info.m_ue_property_p->HasAllPropertyFlags(CPF_OutParm) ? param_info.m_outer_p->m_k2_value_assigner_p : nullptr;
    reflected_param_p->m_inner_fetcher_p  = param_info.m_inner_p ? param_info.m_inner_p->m_k2_value_fetcher_p : nullptr;
    reflected_param_p->m_offset           = param_info.m_ue_property_p->GetOffset_ForUFunction();
    }

  // And destroy the ReflectedPropertys
  for (uint32_t i = 0; i < num_params; ++i)
    {
    param_info_array_p[i].~ReflectedProperty();
    }

  // Store it
  m_reflected_delegates.append(*reflected_delegate_p);

  return reflected_delegate_p;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::expose_reflected_function(uint32_t function_index, tSkUEOnFunctionUpdatedFunc * on_function_updated_f, bool is_final)
  {
  bool anything_changed = false;

  ReflectedFunction * reflected_function_p = m_reflected_functions[function_index];
  if (reflected_function_p && reflected_function_p->m_sk_invokable_p)
    {
    // Only expose entries that have not already been exposed
    if (!reflected_function_p->m_ue_function_p.IsValid())
      {
      // Find reflected class belonging to this reflected function - must exist at this point
      ReflectedClass * reflected_class_p = m_reflected_classes.get(reflected_function_p->m_sk_invokable_p->get_scope()->get_name());
      // Get or look up UE4 equivalent of the class
      UClass * ue_static_class_p = reflected_class_p->m_ue_static_class_p.Get();
      if (!ue_static_class_p)
        {
        reflected_class_p->m_ue_static_class_p = ue_static_class_p = SkUEClassBindingHelper::get_static_ue_class_from_sk_class_super(reflected_function_p->m_sk_invokable_p->get_scope());
        }
      if (ue_static_class_p)
        {
        anything_changed = true;

        // Allocate ReflectedPropertys to store temporary information
        ReflectedProperty * param_info_array_p = a_stack_allocate(reflected_function_p->m_num_params + 1, ReflectedProperty);
        for (uint32_t i = 0; i < reflected_function_p->m_num_params + 1u; ++i)
          {
          new (param_info_array_p + i) ReflectedProperty();
          }

        // Now build or reflect UFunction
        UFunction * ue_function_p = nullptr;
        bool is_ue_function_built = !!(reflected_function_p->m_sk_invokable_p->get_annotation_flags() & SkAnnotation_ue4_blueprint);
        // Only for UClasses
        if (reflected_function_p->m_sk_invokable_p->get_scope()->is_entity_class())
          {
          if (is_ue_function_built)
            {
            // If function not there yet, build it
            ue_function_p = build_ue_function(ue_static_class_p, reflected_function_p->m_sk_invokable_p, reflected_function_p->m_type, function_index, param_info_array_p, is_final);
            }
          else
            {
            // It's a Blueprint function or a custom event, look it up
            ue_function_p = reflect_ue_function(reflected_function_p->m_sk_invokable_p, param_info_array_p);
            }
          }

        // Fill in the parameter information
        if (ue_function_p)
          {
          reflected_function_p->m_ue_function_p = ue_function_p;
          reflected_function_p->m_ue_params_size = ue_function_p->ParmsSize;
          reflected_function_p->m_has_out_params = ue_function_p->HasAllFunctionFlags(FUNC_HasOutParms);
          reflected_function_p->m_is_ue_function_built = is_ue_function_built;

          if (reflected_function_p->m_type == ReflectedFunctionType_call)
            {
            ReflectedCall * reflected_call_p = static_cast<ReflectedCall *>(reflected_function_p);

            // Initialize parameters
            for (uint32_t i = 0; i < reflected_function_p->m_num_params; ++i)
              {
              const ReflectedProperty & param_info = param_info_array_p[i];
              ReflectedCallParam & param_entry = reflected_call_p->get_param_array()[i];
              param_entry.set_byte_size(param_info.m_ue_property_p);
              param_entry.m_outer_fetcher_p = param_info.m_outer_p->m_k2_param_fetcher_p;
              param_entry.m_inner_fetcher_p = param_info.m_inner_p ? param_info.m_inner_p->m_k2_value_fetcher_p : nullptr;
              }

            // And return parameter
            const ReflectedProperty & return_info = param_info_array_p[reflected_function_p->m_num_params];
            reflected_call_p->m_result.set_byte_size(return_info.m_ue_property_p);
            reflected_call_p->m_result.m_outer_storer_p = return_info.m_outer_p ? return_info.m_outer_p->m_sk_value_storer_p : nullptr;
            reflected_call_p->m_result.m_inner_storer_p = return_info.m_inner_p ? return_info.m_inner_p->m_sk_value_storer_p : nullptr;
            }
          else
            {
            ReflectedEvent * reflected_event_p = static_cast<ReflectedEvent *>(reflected_function_p);

            // Initialize parameters
            for (uint32_t i = 0; i < reflected_function_p->m_num_params; ++i)
              {
              const ReflectedProperty & param_info = param_info_array_p[i];
              ReflectedEventParam & event_param = reflected_event_p->get_param_array()[i];
              event_param.set_byte_size(param_info.m_ue_property_p);
              event_param.m_outer_storer_p   = param_info.m_outer_p->m_sk_value_storer_p;
              event_param.m_inner_storer_p   = param_info.m_inner_p ? param_info.m_inner_p->m_sk_value_storer_p : nullptr;
              event_param.m_outer_assigner_p = param_info.m_ue_property_p->HasAllPropertyFlags(CPF_OutParm) ? param_info.m_outer_p->m_k2_value_assigner_p : nullptr;
              event_param.m_inner_fetcher_p  = param_info.m_inner_p ? param_info.m_inner_p->m_k2_value_fetcher_p : nullptr;
              event_param.m_offset           = param_info.m_ue_property_p->GetOffset_ForUFunction();
              }
            }

          // Clear parent class function cache if exists
          // as otherwise it might have cached a nullptr which might cause it to never find newly added functions
          #if WITH_EDITORONLY_DATA
            UClass * ue_class_p = SkUEClassBindingHelper::get_ue_class_from_sk_class(reflected_function_p->m_sk_invokable_p->get_scope());
            if (ue_class_p)
              {
              ue_class_p->ClearFunctionMapsCaches();
              }
          #endif

          // Invoke update callback if any
          if (on_function_updated_f && is_ue_function_built)
            {
            on_function_updated_f->invoke(ue_function_p, reflected_function_p->m_type == ReflectedFunctionType_event);
            }
          }

        // And destroy the ReflectedPropertys
        for (uint32_t i = 0; i < reflected_function_p->m_num_params + 1u; ++i)
          {
          param_info_array_p[i].~ReflectedProperty();
          }
        }
      }
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::sync_all_to_ue(tSkUEOnFunctionUpdatedFunc * on_function_updated_f, bool is_final)
  {
  bool anything_changed = false;

  // Loop through all the reflected classes and attach a USkookumScriptInstanceProperty as needed
  for (ReflectedClass * reflected_class_p : m_reflected_classes)
    {
    if (reflected_class_p->m_store_sk_instance)
      {
      SkClass * sk_class_p = SkBrain::get_class(reflected_class_p->get_name());
      if (sk_class_p && !sk_class_p->get_user_data_int())
        {
        UClass * ue_class_p = SkUEClassBindingHelper::get_ue_class_from_sk_class(sk_class_p);
        if (ue_class_p)
          {
          reflected_class_p->m_ue_static_class_p = SkUEClassBindingHelper::get_static_ue_class_from_sk_class_super(sk_class_p);
          anything_changed |= add_instance_property_to_class(ue_class_p, sk_class_p);
          }
        }
      }
    }

  // Loop through all bindings and generate their UFunctions
  for (uint32_t binding_index = 0; binding_index < m_reflected_functions.get_length(); ++binding_index)
    {
    anything_changed |= expose_reflected_function(binding_index, on_function_updated_f, is_final);
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::does_class_need_instance_property(SkClass * sk_class_p)
  {
  // Are we allowed to attach an instance property?
  if (sk_class_p->get_annotation_flags() & SkAnnotation_reflected_data)
    {
    // Does class have data members?
    if (sk_class_p->get_total_data_count()) return true;

    // Does class have a non-empty constructor?
    SkInvokableBase * ctor_p = sk_class_p->find_instance_method(ASymbolX_ctor);
    if (ctor_p && !ctor_p->is_empty()) return true;

    // Does class have a non-empty destructor?
    SkInvokableBase * dtor_p = sk_class_p->get_instance_destructor_inherited();
    if (dtor_p && !dtor_p->is_empty()) return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::add_instance_property_to_class(UClass * ue_class_p, SkClass * sk_class_p)
  {
  // Do this even in commandlet mode so that classes are serialized out with the USkookumScriptInstanceProperty included

  bool success = false;

  // Name it like the class for simplicity
  FName property_name = USkookumScriptInstanceProperty::StaticClass()->GetFName();

  // Is it already present (in this class or any of its superclasses) ?
  UProperty * property_p = ue_class_p->FindPropertyByName(property_name);
  if (!property_p)
    {
    // No objects of this class except the CDO must exist yet
    #if defined(MAD_CHECK) && (SKOOKUM & SK_DEBUG)
      TArray<UObject *> objects;
      GetObjectsOfClass(ue_class_p, objects);
      SK_ASSERTX(objects.Num() == 0, a_str_format("%d objects of class '%S' already exist when its USkookumScriptInstanceProperty is attached!", objects.Num(), *ue_class_p->GetName()));
    #endif

    // Attach new property
    property_p = NewObject<USkookumScriptInstanceProperty>(ue_class_p, property_name);
    // Note: The CDO was already created and _does not_ have this property!
    // So: Append to the end of the children's chain where it won't shift other properties around in memory
    // And, to prevent problems with the smaller CDO, all code in USkookumScriptInstanceProperty interacting with CDOs simply does nothing
    UField ** prev_pp = &ue_class_p->Children;
    while (*prev_pp) { prev_pp = &(*prev_pp)->Next; }
    *prev_pp = property_p;
    // Relink special pointers
    ue_class_p->StaticLink(true);

    #if WITH_EDITOR
      // Reinstance potentially existing objects of this class
      TMap<UClass*, UClass*> classes_to_reinstance;
      classes_to_reinstance.Add(ue_class_p, ue_class_p);
      TGuardValue<bool> hotreload_guard(GIsHotReload, true); // Pretend it's hot reload to allow replacing the class with itself
      TGuardValue<bool> reinstancing_guard(GIsReinstancing, true);
      FBlueprintCompileReinstancer::BatchReplaceInstancesOfClass(classes_to_reinstance);
    #endif

    // Something changed!
    success = true;
    }
  else
    {
    // ZB
    // If we found an existing instance property then we need to ensure that offset and size are properly calculated. I kept encountering
    // a crash here where the USkookumScriptInstanceProperty existed in the class if I walked down the LinkedProperty chain but the class 
    // was sized too small so that the USkookumScriptInstanceProperty offset existed beyond the bounds of the class size.
    // I discovered that this was happening in cases where a base class was constructing a USkookumScriptInstanceProperty and then a child was
    // inheriting it. Ultimately leading to an "instance offset out of range" crash. The odd thing is that the property is legitimately there
    // it's just that the class hasn't been fixed up to recalculate sizes etc. 
    // So here we force the class to perform static linking, the side-effect which will recalculate the size of the class as well as the
    // internal offsets for all linked UPROPERTYs.
    ue_class_p->StaticLink(true);
    }

  // Remember offset in the object where the SkInstance pointer is stored
  sk_class_p->set_user_data_int_recursively(property_p->GetOffset_ForInternal());

  return success;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::can_ue_property_be_reflected(UProperty * ue_property_p)
  {
  return reflect_ue_property(ue_property_p);
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::is_skookum_reflected_call(UFunction * function_p)
  {
  FNativeFuncPtr native_function_p = function_p->GetNativeFunc();
  return native_function_p == (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_class_method
      || native_function_p == (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_instance_method
      || native_function_p == (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_coroutine;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::is_skookum_reflected_event(UFunction * function_p)
  {
  return function_p->RPCId == EventMagicRepOffset;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::exec_sk_method(UObject* context_p, FFrame & stack, void * const result_p, SkClass * class_scope_p, SkInstance * this_p)
  {
  const ReflectedCall & reflected_call = static_cast<const ReflectedCall &>(*ms_singleton_p->m_reflected_functions[stack.CurrentNativeFunction->RPCId]);
  SK_ASSERTX(reflected_call.m_type == ReflectedFunctionType_call, "ReflectedFunction has bad type!");
  SK_ASSERTX(reflected_call.m_sk_invokable_p->get_invoke_type() == SkInvokable_method, "Must be a method at this point.");

  SkMethodBase * method_p = static_cast<SkMethodBase *>(reflected_call.m_sk_invokable_p);
  if (method_p->get_scope() != class_scope_p)
    {
    method_p = static_cast<SkMethodBase *>(class_scope_p->get_invokable_from_vtable(this_p ? SkScope_instance : SkScope_class, method_p->get_vtable_index()));
    #if SKOOKUM & SK_DEBUG
      // If not found, might be due to recent live update and the vtable not being updated yet - try finding it by name
      if (!method_p || method_p->get_name() != reflected_call.get_name())
        {
        method_p = this_p
          ? class_scope_p->find_instance_method_inherited(reflected_call.get_name())
          : class_scope_p->find_class_method_inherited(reflected_call.get_name());
        }
      // If still not found, that means the method placed in the graph is not in a parent class of class_scope_p
      if (!method_p)
        {
        // Just revert to original method and then, after processing the arguments on the stack, assert below
        method_p = static_cast<SkMethodBase *>(reflected_call.m_sk_invokable_p);
        }
    #endif
    }
  SkInvokedMethod imethod(nullptr, this_p ? this_p : &class_scope_p->get_metaclass(), method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  SKDEBUG_ICALL_SET_INTERNAL(&imethod);
  SKDEBUG_HOOK_SCRIPT_ENTRY(reflected_call.get_name());

  // Fill invoked method's argument list
  const ReflectedCallParam * call_params_p = reflected_call.get_param_array();
  SK_ASSERTX(imethod.get_data().get_size() >= reflected_call.m_num_params, a_str_format("Not enough space (%d) for %d arguments while invoking '%s@%s'!", imethod.get_data().get_size(), reflected_call.m_num_params, reflected_call.m_sk_invokable_p->get_scope()->get_name_cstr_dbg(), reflected_call.get_name_cstr_dbg()));
  for (uint32_t i = 0; i < reflected_call.m_num_params; ++i)
    {
    const ReflectedCallParam & call_param = call_params_p[i];
    imethod.data_append_arg((*call_param.m_outer_fetcher_p)(stack, call_param));
    }

  // Done with stack - now increment the code ptr unless it is null
  stack.Code += !!stack.Code;

  #if (SKOOKUM & SK_DEBUG)
    if (!class_scope_p->is_class(*reflected_call.m_sk_invokable_p->get_scope()))
      {
      SK_ERRORX(a_str_format("Attempted to invoke method '%s@%s' via a blueprint of type '%s'. You might have forgotten to specify the SkookumScript type of this blueprint as '%s' in its SkookumScriptClassDataComponent.", reflected_call.m_sk_invokable_p->get_scope()->get_name_cstr(), reflected_call.get_name_cstr(), class_scope_p->get_name_cstr(), reflected_call.m_sk_invokable_p->get_scope()->get_name_cstr()));
      }
    else
  #endif
      {
      // Call method
      SkInstance * result_instance_p = SkBrain::ms_nil_p;
      static_cast<SkMethod *>(method_p)->SkMethod::invoke(&imethod, nullptr, &result_instance_p); // We know it's a method so call directly
      // And pass back the result
      if (reflected_call.m_result.m_outer_storer_p)
        {
        (*reflected_call.m_result.m_outer_storer_p)(result_p, result_instance_p, reflected_call.m_result);
        }
      result_instance_p->dereference();
      }

  SKDEBUG_HOOK_SCRIPT_EXIT();
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::exec_sk_class_method(UObject* context_p, FFrame & stack, void * const result_p)
  {
  SkClass * class_scope_p = SkUEClassBindingHelper::get_object_class(context_p);
  exec_sk_method(context_p, stack, result_p, class_scope_p, nullptr);
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::exec_sk_instance_method(UObject* context_p, FFrame & stack, void * const result_p)
  {
  SkInstance * this_p = SkUEEntity::new_instance(context_p);
  exec_sk_method(context_p, stack, result_p, this_p->get_class(), this_p);
  this_p->dereference();
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::exec_sk_coroutine(UObject* context_p, FFrame & stack, void * const result_p)
  {
  const ReflectedCall & reflected_call = static_cast<const ReflectedCall &>(*ms_singleton_p->m_reflected_functions[stack.CurrentNativeFunction->RPCId]);
  SK_ASSERTX(reflected_call.m_type == ReflectedFunctionType_call, "ReflectedFunction has bad type!");
  SK_ASSERTX(reflected_call.m_sk_invokable_p->get_invoke_type() == SkInvokable_coroutine, "Must be a coroutine at this point.");

  // Get instance of this object
  SkInstance * this_p = SkUEEntity::new_instance(context_p);

  // Create invoked coroutine
  SkCoroutineBase * coro_p = static_cast<SkCoroutineBase *>(reflected_call.m_sk_invokable_p);
  SkClass * class_scope_p = this_p->get_class();
  if (coro_p->get_scope() != class_scope_p)
    {
    coro_p = static_cast<SkCoroutine *>(class_scope_p->get_invokable_from_vtable_i(coro_p->get_vtable_index()));
    #if SKOOKUM & SK_DEBUG
      // If not found, might be due to recent live update and the vtable not being updated yet - try finding it by name
      if (coro_p == nullptr || coro_p->get_name() != reflected_call.m_sk_invokable_p->get_name())
        {
        coro_p = class_scope_p->find_coroutine_inherited(reflected_call.m_sk_invokable_p->get_name());
        }
      // If still not found, that means the coroutine placed in the graph is not in a parent class of class_scope_p
      if (!coro_p)
        {
        // Just revert to original coroutine and then, after processing the arguments on the stack, assert below
        coro_p = static_cast<SkCoroutineBase *>(reflected_call.m_sk_invokable_p);
        }
    #endif
    }
  SkInvokedCoroutine * icoroutine_p = SkInvokedCoroutine::pool_new(coro_p);

  // Set parameters
  icoroutine_p->reset(SkCall_interval_always, nullptr, this_p, nullptr, nullptr);

  #if defined(SKDEBUG_COMMON)
    // Set with SKDEBUG_ICALL_STORE_GEXPR stored here before calls to argument expressions
    // overwrite it.
    const SkExpressionBase * call_expr_p = SkInvokedContextBase::ms_last_expr_p;
  #endif

  SKDEBUG_ICALL_SET_EXPR(icoroutine_p, call_expr_p);

  // Fill invoked coroutine's argument list
  const ReflectedCallParam * param_entry_array = reflected_call.get_param_array();
  icoroutine_p->data_ensure_size(reflected_call.m_num_params);
  for (uint32_t i = 0; i < reflected_call.m_num_params; ++i)
    {
    const ReflectedCallParam & call_param = param_entry_array[i];
    icoroutine_p->data_append_arg((*call_param.m_outer_fetcher_p)(stack, call_param));
    }

  // Done with stack - now increment the code ptr unless it is null
  stack.Code += !!stack.Code;

  SKDEBUG_HOOK_EXPR(call_expr_p, icoroutine_p, nullptr, nullptr, SkDebug::HookContext_peek);

  #if (SKOOKUM & SK_DEBUG)
    if (!this_p->get_class()->is_class(*reflected_call.m_sk_invokable_p->get_scope()))
      {
      SK_ERRORX(a_str_format("Attempted to invoke coroutine '%s@%s' via a blueprint of type '%s'. You might have forgotten to specify the SkookumScript type of this blueprint as '%s' in its SkookumScriptClassDataComponent.", reflected_call.m_sk_invokable_p->get_scope()->get_name_cstr(), reflected_call.get_name_cstr(), this_p->get_class()->get_name_cstr(), reflected_call.m_sk_invokable_p->get_scope()->get_name_cstr()));
      }
    else
  #endif
      {
      // Invoke the coroutine on this_p - might return immediately
      icoroutine_p->on_update();
      }

  // Free if not in use by our invoked coroutine
  this_p->dereference();
  }

//---------------------------------------------------------------------------------------
// Invoke a K2 (Blueprint) event
// Arg lambda_invoker - Lambda with the signature (ReflectedEvent & reflected_event, void * k2_params_p)
template<typename _EventType, typename _LambdaType>
A_FORCEINLINE_OPTIMIZED void SkUEReflectionManager::invoke_k2_event(_EventType * reflected_event_p, SkInvokedMethod * scope_p, SkInstance ** result_pp, _LambdaType && invoker)
  {
  // Create parameters on stack
  const ReflectedEventParam * event_params_p = reflected_event_p->get_param_array();
  uint8_t * k2_params_p = a_stack_allocate(reflected_event_p->m_ue_params_size, uint8_t);
  for (uint32_t i = 0; i < reflected_event_p->m_num_params; ++i)
    {
    const ReflectedEventParam & event_param = event_params_p[i];
    (*event_param.m_outer_storer_p)(k2_params_p + event_param.m_offset, scope_p->get_arg(i), event_param);
    }

  // Now perform the actual invocation
  invoker(k2_params_p);

  // Copy back any outgoing parameters
  if (reflected_event_p->m_has_out_params)
    {
    for (uint32_t i = 0; i < reflected_event_p->m_num_params; ++i)
      {
      const ReflectedEventParam & event_param = event_params_p[i];
      if (event_param.m_outer_assigner_p)
        {
        (*event_param.m_outer_assigner_p)(scope_p->get_arg(i), k2_params_p + event_param.m_offset, event_param);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Execute a blueprint event
void SkUEReflectionManager::mthd_invoke_k2_event(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  AActor * actor_p = scope_p->this_as<SkUEActor>();
  uint32_t function_index = scope_p->get_invokable()->get_user_data();
  ReflectedEvent * reflected_event_p = static_cast<ReflectedEvent *>(ms_singleton_p->m_reflected_functions[function_index]);
  SK_ASSERTX(reflected_event_p->m_type == ReflectedFunctionType_event, "ReflectedFunction has bad type!");

  // Perform invocation
  invoke_k2_event(reflected_event_p, scope_p, result_pp, [=](void * k2_params_p)
    {
    // Invoke K2 script event with parameters
    UFunction * ue_function_p = reflected_event_p->m_ue_function_to_invoke_p.Get();
    if (!ue_function_p)
      {
      // Find Kismet copy of our method to invoke
      ue_function_p = reflected_event_p->m_ue_function_p.Get(); // Invoke the first one

      // Just in time discovery of soft-referenced events
      if (!ue_function_p)
        {
        ue_function_p = find_ue_function(reflected_event_p->m_sk_invokable_p);
        reflected_event_p->m_ue_function_p = ue_function_p;
        SK_ASSERTX(ue_function_p, a_str_format("Cannot find UE counterpart of method %s@%s!", reflected_event_p->m_sk_invokable_p->get_scope()->get_name_cstr(), reflected_event_p->m_sk_invokable_p->get_name_cstr()));
        }

      ue_function_p = actor_p->FindFunctionChecked(*ue_function_p->GetName());
      reflected_event_p->m_ue_function_to_invoke_p = ue_function_p;
      reflected_event_p->m_ue_params_size = ue_function_p->ParmsSize;
      reflected_event_p->m_has_out_params = ue_function_p->HasAllFunctionFlags(FUNC_HasOutParms);
      }
    // Check if this event is actually present in any Blueprint graph
    SK_ASSERTX(ue_function_p->Script.Num() > 0, a_str_format("Warning: Call to '%S' on actor '%S' has no effect as no Blueprint event node named '%S' exists in any of its event graphs.", *ue_function_p->GetName(), *actor_p->GetName(), *ue_function_p->GetName()));
    
    // Call the event function
    actor_p->ProcessEvent(ue_function_p, k2_params_p);

    // Events don't have a return value, only out parameters
    });
  }

//---------------------------------------------------------------------------------------
// Generic constructor for user defined structs
void SkUEReflectionManager::mthd_struct_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  SkInstance * this_p = scope_p->get_this();
  SkClass * sk_class_p = this_p->get_class();
  UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 equivalent of struct '%s'.", sk_class_p->get_name_cstr()));
  if (ue_struct_p)
    {
    void * data_p = this_p->allocate_raw(ue_struct_p->PropertiesSize);
    ue_struct_p->InitializeStruct(data_p);
    }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::mthd_struct_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  SkInstance * this_p = scope_p->get_this();
  SkInstance * that_p = scope_p->get_arg(SkArg_1);
  SkClass * sk_class_p = this_p->get_class();
  SK_ASSERTX(sk_class_p == that_p->get_class(), a_str_format("Copy construction of '%s' with '%s' not possible. Types must be identical.", sk_class_p->get_name_cstr(), that_p->get_class()->get_name_cstr()));
  UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 equivalent of struct '%s'.", sk_class_p->get_name_cstr()));
  if (ue_struct_p)
    {
    void * this_data_p = this_p->allocate_raw(ue_struct_p->PropertiesSize);
    void * that_data_p = that_p->get_raw_pointer(ue_struct_p->PropertiesSize);
    ue_struct_p->CopyScriptStruct(this_data_p, that_data_p);
    }
  }

//---------------------------------------------------------------------------------------
// Generic destructor for user defined structs
void SkUEReflectionManager::mthd_struct_dtor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  SkInstance * this_p = scope_p->get_this();
  SkClass * sk_class_p = this_p->get_class();
  UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 equivalent of struct '%s'.", sk_class_p->get_name_cstr()));
  if (ue_struct_p)
    {
    void * data_p = this_p->get_raw_pointer(ue_struct_p->PropertiesSize);
    ue_struct_p->DestroyStruct(data_p);
    this_p->deallocate_raw(ue_struct_p->PropertiesSize);
    }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::mthd_struct_op_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  SkInstance * this_p = scope_p->get_this();
  SkInstance * that_p = scope_p->get_arg(SkArg_1);
  SkClass * sk_class_p = this_p->get_class();
  SK_ASSERTX(sk_class_p == that_p->get_class(), a_str_format("Assignment from type '%s' to '%s' not possible. Types must be identical.", that_p->get_class()->get_name_cstr(), sk_class_p->get_name_cstr()));
  UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 equivalent of struct '%s'.", sk_class_p->get_name_cstr()));
  if (ue_struct_p)
    {
    void * this_data_p = this_p->get_raw_pointer(ue_struct_p->PropertiesSize);
    void * that_data_p = that_p->get_raw_pointer(ue_struct_p->PropertiesSize);
    ue_struct_p->CopyScriptStruct(this_data_p, that_data_p);    
    }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::invoke_k2_delegate(const FScriptDelegate & script_delegate, const SkParameters * sk_params_p, SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  if (script_delegate.IsBound())
    {
    // Do we know this function signature already?
    ReflectedDelegate * reflected_delegate_p = m_reflected_delegates.get(sk_params_p);
    if (!reflected_delegate_p)
      {
      // No, find it and cache it
      reflected_delegate_p = add_reflected_delegate(sk_params_p, static_cast<const Hacked_FScriptDelegate &>(script_delegate).get_signature());
      }

    // Perform the actual invocation
    invoke_k2_event(reflected_delegate_p, scope_p, result_pp, [=](void * k2_params_p)
      {
      script_delegate.ProcessDelegate<UObject>(k2_params_p);
      });
    }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::invoke_k2_delegate(const FMulticastScriptDelegate & script_delegate, const SkParameters * sk_params_p, SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  if (script_delegate.IsBound())
    {
    // Do we know this function signature already?
    ReflectedDelegate * reflected_delegate_p = m_reflected_delegates.get(sk_params_p);
    if (!reflected_delegate_p)
      {
      // No, find it and cache it
      reflected_delegate_p = add_reflected_delegate(sk_params_p, static_cast<const Hacked_FMulticastScriptDelegate &>(script_delegate).get_signature());
      }

    // Perform the actual invocation
    invoke_k2_event(reflected_delegate_p, scope_p, result_pp, [=](void * k2_params_p)
      {
      script_delegate.ProcessMulticastDelegate<UObject>(k2_params_p);
      });
    }
  }

//---------------------------------------------------------------------------------------

template<class _TypedName>
bool SkUEReflectionManager::have_identical_signatures(const tSkParamList & param_list, const _TypedName * param_array_p)
  {
  for (uint32_t i = 0; i < param_list.get_length(); ++i)
    {
    const TypedName & typed_name = param_array_p[i];
    const SkParameterBase * param_p = param_list[i];
    if (typed_name.get_name() != param_p->get_name()
     || typed_name.m_sk_class_name != param_p->get_expected_type()->get_key_class()->get_name())
      {
      return false;
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Re-resolve class pointers for params
template<class _TypedName>
void SkUEReflectionManager::rebind_params_to_sk(const tSkParamList & param_list, _TypedName * param_array_p)
  {
  for (uint32_t i = 0; i < param_list.get_length(); ++i)
    {
    const SkParameterBase * param_p = param_list[i];
    TypedName & typed_name = param_array_p[i];
    SK_ASSERTX(typed_name.get_name() == param_p->get_name() && typed_name.m_sk_class_name == param_p->get_expected_type()->get_key_class()->get_name(), "Caller must ensure beforehand that signatures match.");
    typed_name.m_sk_class_p = param_p->get_expected_type()->get_key_class();
    }
  }

//---------------------------------------------------------------------------------------
// Store a given ReflectedFunction into the m_binding_entry_array
// If an index is given, use that, otherwise, find an empty slot to reuse, or if that fails, append a new entry
int32_t SkUEReflectionManager::store_reflected_function(ReflectedFunction * reflected_function_p, ReflectedClass * reflected_class_p, int32_t function_index_to_use)
  {
  // If no binding index known yet, look if there is an empty slot that we can reuse
  if (function_index_to_use < 0)
    {
    for (function_index_to_use = 0; function_index_to_use < (int32_t)m_reflected_functions.get_length(); ++function_index_to_use)
      {
      if (!m_reflected_functions[function_index_to_use]) break;
      }
    }
  if (function_index_to_use == m_reflected_functions.get_length())
    {
    m_reflected_functions.append(*reflected_function_p);
    }
  else
    {
    m_reflected_functions.set_at(function_index_to_use, reflected_function_p);
    }

  // Remember binding index to invoke Blueprint events
  reflected_function_p->m_sk_invokable_p->set_user_data(function_index_to_use);

  // Hook into class
  if (!reflected_class_p)
    {
    SK_ASSERTX(!m_reflected_classes.get(reflected_function_p->m_sk_invokable_p->get_scope()->get_name()), "Reflected class exists even though reflected_class_p was passed in as nullptr.");
    reflected_class_p = new ReflectedClass(reflected_function_p->m_sk_invokable_p->get_scope()->get_name());
    m_reflected_classes.append(*reflected_class_p);
    }
  reflected_class_p->m_functions.append(FunctionIndex(function_index_to_use));

  return function_index_to_use;
  }

//---------------------------------------------------------------------------------------
// Delete reflected function and set pointer to nullptr so it can be reused
// Note: Does _not_ remove this function's entry from its reflected class!
void SkUEReflectionManager::delete_reflected_function(uint32_t function_index)
  {
  ReflectedFunction * reflected_function_p = m_reflected_functions[function_index];
  if (reflected_function_p)
    {
    //SK_ASSERTX(reflected_function_p->m_ue_function_p.IsValid(), a_str_format("UFunction %s was deleted outside of SkUEReflectionManager and left dangling links behind in its owner UClass (%s).", reflected_function_p->get_name_cstr(), reflected_function_p->m_sk_invokable_p->get_scope()->get_name_cstr()));
    if (reflected_function_p->m_is_ue_function_built // Only destroy the UFunction if this plugin built it in the first place
     && reflected_function_p->m_ue_function_p.IsValid())
      {
      UFunction * ue_function_p = reflected_function_p->m_ue_function_p.Get();
      UClass * ue_class_p = ue_function_p->GetOwnerClass();
      // Unlink from its owner class
      ue_class_p->RemoveFunctionFromFunctionMap(ue_function_p);
      // Unlink from the Children list as well
      UField ** prev_field_pp = &ue_class_p->Children;
      for (UField * field_p = *prev_field_pp; field_p; prev_field_pp = &field_p->Next, field_p = *prev_field_pp)
        {
        if (field_p == ue_function_p)
          {
          *prev_field_pp = field_p->Next;
          break;
          }
        }

      // Destroy the function along with its attached properties
      // HACK remove from root if it's rooted - proper fix: Find out why it's rooted to begin with
      ue_function_p->RemoveFromRoot();
      ue_function_p->MarkPendingKill();
      }

    FMemory::Free(reflected_function_p);
    m_reflected_functions[function_index] = nullptr;
    }
  }

//---------------------------------------------------------------------------------------

UFunction * SkUEReflectionManager::find_ue_function(SkInvokableBase * sk_invokable_p)
  {
  UClass * ue_class_p = SkUEClassBindingHelper::get_ue_class_from_sk_class(sk_invokable_p->get_scope());
  if (!ue_class_p) return nullptr;

  FString ue_function_name;
  FString ue_function_name_skookified;
  AString sk_function_name = sk_invokable_p->get_name_str();
  for (TFieldIterator<UFunction> func_it(ue_class_p, EFieldIteratorFlags::ExcludeSuper); func_it; ++func_it)
    {
    UFunction * ue_function_p = *func_it;
    ue_function_p->GetName(ue_function_name);
    ue_function_name_skookified = FSkookumScriptGeneratorHelper::skookify_method_name(ue_function_name);
    int32 i;
    for (i = 0; i < ue_function_name_skookified.Len(); ++i)
      {
      if (ue_function_name_skookified[i] != sk_function_name[i]) goto NotFound;
      }
    if (sk_function_name[i] == 0) return ue_function_p;
  NotFound:;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------

UFunction * SkUEReflectionManager::reflect_ue_function(SkInvokableBase * sk_invokable_p, ReflectedProperty * out_param_info_array_p)
  {
  // Find the function
  UFunction * ue_function_p = find_ue_function(sk_invokable_p);
  if (ue_function_p)
    {
    // Build reflected parameter list
    if (reflect_ue_params(sk_invokable_p->get_params(), ue_function_p, out_param_info_array_p))
      {
      return ue_function_p;
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::reflect_ue_params(const SkParameters & sk_params, UFunction * ue_function_p, ReflectedProperty * out_param_info_array_p)
  {
  const tSkParamList & param_list = sk_params.get_param_list();
  uint32_t num_parameters = 0;
  for (TFieldIterator<UProperty> param_it(ue_function_p); param_it; ++param_it)
    {
    UProperty * param_p = *param_it;
    if ((param_p->GetPropertyFlags() & (CPF_ReturnParm|CPF_Parm)) == CPF_Parm)
      {
      // Too many parameters?
      if (num_parameters >= param_list.get_length())
        {
        return false;
        }

      // Reflect this parameter and check if successful
      ReflectedProperty * out_param_info_p = out_param_info_array_p + num_parameters;
      if (!reflect_ue_property(param_p, out_param_info_p)
       || out_param_info_p->get_name() != param_list[num_parameters]->get_name())
        {
        return false;
        }

      // Got one more parameter
      ++num_parameters;
      }
    }

  // Did we find the exact number of parameters that we need?
  return num_parameters == param_list.get_length();
  }

//---------------------------------------------------------------------------------------

bool SkUEReflectionManager::reflect_ue_property(UProperty * ue_property_p, ReflectedProperty * out_info_p)
  {
  // Based on Sk type, figure out the matching UProperty as well as fetcher and setter methods
  const ReflectedAccessors * container_accessors_p = nullptr;
  UProperty * ue_item_property_p = ue_property_p;

  // Is it an array? If so, remember that and look at item type instead
  if (ue_property_p->IsA<UArrayProperty>())
    {
    container_accessors_p = &ms_accessors_array;
    ue_item_property_p = static_cast<UArrayProperty *>(ue_property_p)->Inner;
    }

  const ReflectedAccessors * accessors_p = nullptr;
  if (ue_item_property_p->IsA<UBoolProperty>())
    {
    accessors_p = &ms_accessors_boolean;
    }
  else if (ue_item_property_p->IsA<UIntProperty>())
    {
    accessors_p = &ms_accessors_integer;
    }
  else if (ue_item_property_p->IsA<UFloatProperty>())
    {
    accessors_p = &ms_accessors_real;
    }
  else if (ue_item_property_p->IsA<UStrProperty>())
    {
    accessors_p = &ms_accessors_string;
    }
  else if (ue_item_property_p->IsA<UStructProperty>())
    {
    UScriptStruct * struct_p = static_cast<UStructProperty *>(ue_item_property_p)->Struct;
    if (struct_p->GetFName() == ms_struct_vector2_p->GetFName())
      {
      accessors_p = &ms_accessors_vector2;
      }
    else if (struct_p->GetFName() == ms_struct_vector3_p->GetFName())
      {
      accessors_p = &ms_accessors_vector3;
      }
    else if (struct_p->GetFName() == ms_struct_vector4_p->GetFName())
      {
      accessors_p = &ms_accessors_vector4;
      }
    else if (struct_p->GetFName() == ms_struct_rotation_angles_p->GetFName())
      {
      accessors_p = &ms_accessors_rotation_angles;
      }
    else if (struct_p->GetFName() == ms_struct_transform_p->GetFName())
      {
      accessors_p = &ms_accessors_transform;
      }
    else
      {
      if (SkInstance::is_data_stored_by_val(struct_p->GetStructureSize()))
        {
        accessors_p = &ms_accessors_struct_val;
        }
      else
        {
        accessors_p = &ms_accessors_struct_ref;
        }
      }
    }
  else if (ue_item_property_p->IsA<UByteProperty>() && static_cast<UByteProperty *>(ue_item_property_p)->Enum)
    {
    accessors_p = &ms_accessors_enum;
    }
  else if (ue_item_property_p->IsA<UObjectProperty>())
    {
    accessors_p = &ms_accessors_entity;
    }

  // Set result
  if (accessors_p && out_info_p)
    {
    FString var_name = FSkookumScriptGeneratorHelper::skookify_param_name(ue_property_p->GetName(), ue_property_p->IsA<UBoolProperty>());
    out_info_p->set_name(ASymbol::create(FStringToAString(var_name)));
    out_info_p->m_ue_property_p = ue_property_p;
    if (container_accessors_p)
      {
      out_info_p->m_outer_p = container_accessors_p;
      out_info_p->m_inner_p = accessors_p;
      }
    else
      {
      out_info_p->m_outer_p = accessors_p;
      out_info_p->m_inner_p = nullptr;
      }
    }

  return accessors_p != nullptr;
  }

//---------------------------------------------------------------------------------------
// Params:
//   out_param_info_array_p: Storage for info on each parameter, return value is stored behind the input parameters, and is zeroed if there is no return value
UFunction * SkUEReflectionManager::build_ue_function(UClass * ue_class_p, SkInvokableBase * sk_invokable_p, eReflectedFunctionType binding_type, uint32_t binding_index, ReflectedProperty * out_param_info_array_p, bool is_final)
  {
  // Build name of method including scope
  const char * invokable_name_p = sk_invokable_p->get_name_cstr();
  const char * class_name_p = sk_invokable_p->get_scope()->get_name_cstr();
  AString qualified_invokable_name;
  qualified_invokable_name.ensure_size_buffer(uint32_t(::strlen(invokable_name_p) + ::strlen(class_name_p) + 3u));
  qualified_invokable_name.append(class_name_p);
  qualified_invokable_name.append(" @ ", 3u);
  qualified_invokable_name.append(invokable_name_p);
  FName qualified_invokable_fname(qualified_invokable_name.as_cstr());

  // Must not be already present
  #if WITH_EDITORONLY_DATA
    UFunction * old_ue_function_p = ue_class_p->FindFunctionByName(qualified_invokable_fname);
    if (old_ue_function_p)
      {
      ue_class_p->ClearFunctionMapsCaches();
      old_ue_function_p = ue_class_p->FindFunctionByName(qualified_invokable_fname);
      SK_MAD_ASSERTX(!old_ue_function_p, a_str_format("Found reflected duplicate of function %S@%s!", *ue_class_p->GetName(), qualified_invokable_name.as_cstr()));
      }
  #endif

  // Make UFunction object
  UFunction * ue_function_p = NewObject<UFunction>(ue_class_p, qualified_invokable_fname, RF_Public);

  ue_function_p->FunctionFlags |= (FUNC_Public | FUNC_Const); // Pretend all functions are const since Sk cannot distinguish
  if (sk_invokable_p->is_class_member())
    {
    ue_function_p->FunctionFlags |= FUNC_Static;
    }

  if (binding_type == ReflectedFunctionType_call)
    {
    ue_function_p->FunctionFlags |= FUNC_BlueprintCallable | FUNC_Native;
    ue_function_p->SetNativeFunc(sk_invokable_p->get_invoke_type() == SkInvokable_coroutine 
      ? (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_coroutine
      : (sk_invokable_p->is_class_member() ? (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_class_method : (FNativeFuncPtr)&SkUEReflectionManager::exec_sk_instance_method));
    #if WITH_EDITOR
      ue_function_p->SetMetaData(TEXT("Tooltip"), *FString::Printf(TEXT("%S\n%S@%S()"), 
        sk_invokable_p->get_invoke_type() == SkInvokable_coroutine ? "Kick off SkookumScript coroutine" : "Call to SkookumScript method", 
        sk_invokable_p->get_scope()->get_name_cstr(), 
        sk_invokable_p->get_name_cstr()));
    #endif
    ue_function_p->RPCId = (uint16_t)binding_index; // Remember binding index here for later lookup
    }
  else // binding_type == BindingType_Event
    {
    ue_function_p->FunctionFlags |= FUNC_BlueprintEvent | FUNC_Event;
    ue_function_p->Bind(); // Bind to default Blueprint event mechanism
    #if WITH_EDITOR
      ue_function_p->SetMetaData(TEXT("Tooltip"), *FString::Printf(TEXT("Triggered by SkookumScript method\n%S@%S()"), sk_invokable_p->get_scope()->get_name_cstr(), sk_invokable_p->get_name_cstr()));
    #endif    
    ue_function_p->RPCId = EventMagicRepOffset; // So we can tell later this is an Sk event
    }

  #if WITH_EDITOR
    ue_function_p->SetMetaData(TEXT("Category"), TEXT("SkookumScript"));
  #endif

  // Parameters of the method we are creating
  const SkParameters & params = sk_invokable_p->get_params();
  const tSkParamList & param_list = params.get_param_list();
  uint32_t num_params = param_list.get_length();

  // Handle return value if any
  if (params.get_result_class() && params.get_result_class() != SkBrain::ms_object_class_p)
    {
    UProperty * result_param_p = build_ue_param(m_result_name, params.get_result_class(), ue_function_p, out_param_info_array_p ? out_param_info_array_p + num_params : nullptr, is_final);
    if (!result_param_p)
      {
      // If any parameters can not be mapped, skip building this entire function
      ue_function_p->MarkPendingKill();
      return nullptr;
      }

    result_param_p->PropertyFlags |= CPF_ReturnParm; // Flag as return value
    }

  // Handle input parameters (in reverse order so they get linked into the list in proper order)
  for (int32_t i = num_params - 1; i >= 0; --i)
    {
    const SkParameterBase * input_param_p = param_list[i];
    if (!build_ue_param(input_param_p->get_name(), input_param_p->get_expected_type(), ue_function_p, out_param_info_array_p ? out_param_info_array_p + i : nullptr, is_final))
      {
      // If any parameters can not be mapped, skip building this entire function
      ue_function_p->MarkPendingKill();
      return nullptr;
      }
    }

  // Make method known to its class
  ue_function_p->Next = ue_class_p->Children;
  ue_class_p->Children = ue_function_p;
  ue_class_p->AddFunctionToFunctionMap(ue_function_p, ue_function_p->GetFName());

  // Make sure parameter list is properly linked and offsets are set
  ue_function_p->StaticLink(true);

  // From 4.21 on, we need to add all functions to the root set. Failure to do this here will result
  // in sk functions getting GC'd in packaged builds.
  ue_function_p->AddToRoot(); 
  
  return ue_function_p;
  }

//---------------------------------------------------------------------------------------

UProperty * SkUEReflectionManager::build_ue_param(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UFunction * ue_function_p, ReflectedProperty * out_info_p, bool is_final)
  {
  // Build property
  UProperty * property_p = build_ue_property(sk_name, sk_type_p, ue_function_p, out_info_p, is_final);

  // Add flags and attach to function
  if (property_p)
    {
    property_p->PropertyFlags |= CPF_Parm;
    property_p->Next = ue_function_p->Children;
    ue_function_p->Children = property_p;
    }

  return property_p;
  }

//---------------------------------------------------------------------------------------

UProperty * SkUEReflectionManager::build_ue_property(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UObject * ue_outer_p, ReflectedProperty * out_info_p, bool is_final)
  {
  // Based on Sk type, figure out the matching UProperty as well as fetcher and setter methods
  UProperty * ue_property_p = nullptr;

  FName ue_name_outer(sk_name.as_cstr());

  eContainerType container_type = ContainerType_scalar;
  const ReflectedAccessors * container_accessors_p = nullptr;
  FName ue_name(ue_name_outer);

  // Is it a list? If so, remember that and store item type instead
  if (sk_type_p->get_key_class() == SkList::get_class())
    {
    container_type = ContainerType_array;
    container_accessors_p = &ms_accessors_array;
    sk_type_p = sk_type_p->get_item_type();
    ue_name = FName(*(FString(sk_name.as_cstr()) + "_item"));
    }

  const ReflectedAccessors * accessors_p = nullptr;
  if (sk_type_p == SkBoolean::get_class())
    {
    ue_property_p = NewObject<UBoolProperty>(ue_outer_p, ue_name, RF_Public);
    accessors_p = &ms_accessors_boolean;
    }
  else if (sk_type_p == SkInteger::get_class())
    {
    ue_property_p = NewObject<UIntProperty>(ue_outer_p, ue_name, RF_Public);
    accessors_p = &ms_accessors_integer;
    }
  else if (sk_type_p == SkReal::get_class())
    {
    ue_property_p = NewObject<UFloatProperty>(ue_outer_p, ue_name, RF_Public);
    accessors_p = &ms_accessors_real;
    }
  else if (sk_type_p == SkString::get_class())
    {
    ue_property_p = NewObject<UStrProperty>(ue_outer_p, ue_name, RF_Public);
    accessors_p = &ms_accessors_string;
    }
  else if (sk_type_p == SkVector2::get_class())
    {
    ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
    static_cast<UStructProperty *>(ue_property_p)->Struct = ms_struct_vector2_p;
    accessors_p = &ms_accessors_vector2;
    }
  else if (sk_type_p == SkVector3::get_class())
    {
    ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
    static_cast<UStructProperty *>(ue_property_p)->Struct = ms_struct_vector3_p;
    accessors_p = &ms_accessors_vector3;
    }
  else if (sk_type_p == SkVector4::get_class())
    {
    ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
    static_cast<UStructProperty *>(ue_property_p)->Struct = ms_struct_vector4_p;
    accessors_p = &ms_accessors_vector4;
    }
  else if (sk_type_p == SkRotationAngles::get_class())
    {
    ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
    static_cast<UStructProperty *>(ue_property_p)->Struct = ms_struct_rotation_angles_p;
    accessors_p = &ms_accessors_rotation_angles;
    }
  else if (sk_type_p == SkTransform::get_class())
    {
    ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
    static_cast<UStructProperty *>(ue_property_p)->Struct = ms_struct_transform_p;
    accessors_p = &ms_accessors_transform;
    }
  else if (sk_type_p->get_key_class()->is_class(*SkEnum::get_class()))
    {
    UEnum * ue_enum_p = FindObject<UEnum>(ANY_PACKAGE, *FString(sk_type_p->get_key_class_name().as_cstr()));
    if (ue_enum_p)
      {
      ue_property_p = NewObject<UByteProperty>(ue_outer_p, ue_name);
      static_cast<UByteProperty *>(ue_property_p)->Enum = ue_enum_p;
      accessors_p = &ms_accessors_enum;
      }
    else if (is_final)
      {
      on_unknown_type(sk_name, sk_type_p, ue_outer_p);
      }
    }
  else if (sk_type_p->get_key_class()->is_class(*SkUEEntity::get_class()))
    {
    UClass * ue_class_p = SkUEClassBindingHelper::get_ue_class_from_sk_class(sk_type_p->get_key_class());
    if (ue_class_p)
      {
      ue_property_p = NewObject<UObjectProperty>(ue_outer_p, ue_name, RF_Public);
      static_cast<UObjectProperty *>(ue_property_p)->PropertyClass = ue_class_p;
      accessors_p = &ms_accessors_entity;
      }
    else if (is_final)
      {
      on_unknown_type(sk_name, sk_type_p, ue_outer_p);
      }
    }
  else
    {
    UStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(sk_type_p->get_key_class());
    if (ue_struct_p)
      {
      ue_property_p = NewObject<UStructProperty>(ue_outer_p, ue_name);
      static_cast<UStructProperty *>(ue_property_p)->Struct = CastChecked<UScriptStruct>(ue_struct_p);
      if (SkInstance::is_data_stored_by_val(ue_struct_p->GetStructureSize()))
        {
        accessors_p = &ms_accessors_struct_val;
        }
      else
        {
        accessors_p = &ms_accessors_struct_ref;
        }
      }
    else if (is_final)
      {
      on_unknown_type(sk_name, sk_type_p, ue_outer_p);
      }
    }

  // Create container property if desired
  if (ue_property_p && container_type == ContainerType_array)
    {
    UArrayProperty * array_property_p = NewObject<UArrayProperty>(ue_outer_p, ue_name_outer);
    array_property_p->Inner = ue_property_p;
    ue_property_p = array_property_p;
    }

  // Set result
  out_info_p->set_name(sk_name);
  out_info_p->m_ue_property_p = ue_property_p;
  if (container_accessors_p)
    {
    out_info_p->m_outer_p = container_accessors_p;
    out_info_p->m_inner_p = accessors_p;
    }
  else
    {
    out_info_p->m_outer_p = accessors_p;
    out_info_p->m_inner_p = nullptr;
    }

  return ue_property_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::bind_event_method(SkMethodBase * sk_method_p)
  {
  SK_ASSERTX(!sk_method_p->is_bound() 
    || static_cast<SkMethodFunc *>(sk_method_p)->m_atomic_f == &mthd_invoke_k2_event
    || static_cast<SkMethodFunc *>(sk_method_p)->m_atomic_f == &mthd_struct_ctor
    || static_cast<SkMethodFunc *>(sk_method_p)->m_atomic_f == &mthd_struct_ctor_copy
    || static_cast<SkMethodFunc *>(sk_method_p)->m_atomic_f == &mthd_struct_op_assign
    || static_cast<SkMethodFunc *>(sk_method_p)->m_atomic_f == &mthd_struct_dtor,
    a_str_format("Trying to bind Blueprint event method '%s' but it is already bound to a different atomic implementation!", sk_method_p->get_name_cstr_dbg()));

  if (!sk_method_p->is_bound())
    {
    // Default is generic event method
    tSkMethodFunc func_p = &mthd_invoke_k2_event;
    // Check for special methods of user defined structs
    switch (sk_method_p->get_name_id())
      {
      case ASymbolId_ctor:      func_p = &mthd_struct_ctor;      break;
      case ASymbolId_ctor_copy: func_p = &mthd_struct_ctor_copy; break;
      case ASymbolId_assign:    func_p = &mthd_struct_op_assign; break;
      case ASymbolId_dtor:      func_p = &mthd_struct_dtor;      break;
      }
    // Bind it
    sk_method_p->get_scope()->register_method_func(sk_method_p->get_name(), func_p, sk_method_p->is_class_member() ? SkBindFlag_class_no_rebind : SkBindFlag_instance_no_rebind);
    }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::on_unknown_type(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UObject * ue_outer_p)
  {
  #if (SKOOKUM & SK_DEBUG)
    UFunction * ue_function_p = Cast<UFunction>(ue_outer_p);
    if (ue_function_p)
      {
      SK_ERRORX(a_cstr_format("Type '%s' of parameter '%s' of method '%S.%S' being reflected to Blueprints can not be mapped to a Blueprint-compatible type.", sk_type_p->get_key_class_name().as_cstr_dbg(), sk_name.as_cstr(), *ue_function_p->GetOwnerClass()->GetName(), *ue_function_p->GetName()));
      }
    UClass * ue_class_p = Cast<UClass>(ue_outer_p);
    if (ue_class_p)
      {
      SK_ERRORX(a_cstr_format("Type '%s' of data member '%s' of class '%S' being reflected to Blueprints can not be mapped to a Blueprint-compatible type.", sk_type_p->get_key_class_name().as_cstr_dbg(), sk_name.as_cstr(), *ue_class_p->GetName()));
      }
  #endif
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_boolean(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UBoolProperty::TCppType value = UBoolProperty::GetDefaultPropertyValue();
  stack.StepCompiledIn<UBoolProperty>(&value);
  return SkBoolean::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_integer(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UIntProperty::TCppType value = UIntProperty::GetDefaultPropertyValue();
  stack.StepCompiledIn<UIntProperty>(&value);
  return SkInteger::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_real(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UFloatProperty::TCppType value = UFloatProperty::GetDefaultPropertyValue();
  stack.StepCompiledIn<UFloatProperty>(&value);
  return SkReal::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_string(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UStrProperty::TCppType value = UStrProperty::GetDefaultPropertyValue();
  stack.StepCompiledIn<UStrProperty>(&value);
  return SkString::new_instance(FStringToAString(value));
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_vector2(FFrame & stack, const ReflectedCallParam & value_type)
  {
  FVector2D value(ForceInitToZero);
  stack.StepCompiledIn<UStructProperty>(&value);
  return SkVector2::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_vector3(FFrame & stack, const ReflectedCallParam & value_type)
  {
  FVector value(ForceInitToZero);
  stack.StepCompiledIn<UStructProperty>(&value);
  return SkVector3::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_vector4(FFrame & stack, const ReflectedCallParam & value_type)
  {
  FVector4 value(ForceInitToZero);
  stack.StepCompiledIn<UStructProperty>(&value);
  return SkVector4::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_rotation_angles(FFrame & stack, const ReflectedCallParam & value_type)
  {
  FRotator value(ForceInitToZero);
  stack.StepCompiledIn<UStructProperty>(&value);
  return SkRotationAngles::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_transform(FFrame & stack, const ReflectedCallParam & value_type)
  {
  FTransform value;
  stack.StepCompiledIn<UStructProperty>(&value);
  return SkTransform::new_instance(value);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_struct_val(FFrame & stack, const ReflectedCallParam & value_type)
  {
  void * dest_p;
  SkInstance * instance_p = SkInstance::new_instance_uninitialized_val(value_type.m_sk_class_p, value_type.m_byte_size, &dest_p);

  // First, initialize struct to default
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use Memzero and hope that works
      FMemory::Memzero(dest_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper initializer
      ue_struct_p->InitializeStruct(dest_p);
      }

  // Then, gather value from stack
  stack.StepCompiledIn<UStructProperty>(dest_p);
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_struct_ref(FFrame & stack, const ReflectedCallParam & value_type)
  {
  void * dest_p;
  SkInstance * instance_p = SkInstance::new_instance_uninitialized_ref(value_type.m_sk_class_p, value_type.m_byte_size, &dest_p);

  // First, initialize struct to default
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use Memzero and hope that works
      FMemory::Memzero(dest_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper initializer
      ue_struct_p->InitializeStruct(dest_p);
      }

  // Then, gather value from stack
  stack.StepCompiledIn<UStructProperty>(dest_p);
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_entity(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UObject * obj_p = nullptr;
  stack.StepCompiledIn<UObjectPropertyBase>(&obj_p);
  return SkUEEntity::new_instance(obj_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_param_enum(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UByteProperty::TCppType value = UByteProperty::GetDefaultPropertyValue();
  stack.StepCompiledIn<UByteProperty>(&value);
  SkInstance * instance_p = value_type.m_sk_class_p->new_instance();
  instance_p->construct<SkEnum>(tSkEnum(value));  
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// The parameter is an array of the type value_type
SkInstance * SkUEReflectionManager::fetch_k2_param_array(FFrame & stack, const ReflectedCallParam & value_type)
  {
  UArrayProperty::TCppType array; // = FScriptArray
  stack.StepCompiledIn<UArrayProperty>(&array);
  SkInstance * instance_p = SkList::new_instance(array.Num());
  SkInstanceList & list = instance_p->as<SkList>();
  APArray<SkInstance> & list_instances = list.get_instances();
  tK2ValueFetcher fetcher_p = value_type.m_inner_fetcher_p;
  const uint8_t * item_array_p = (const uint8_t *)array.GetData();
  size_t item_size = value_type.m_byte_size;
  for (uint32_t i = array.Num(); i; --i)
    {
    list_instances.append(*(*fetcher_p)(item_array_p, value_type));
    item_array_p += item_size;
    }
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_boolean(const void * value_p, const TypedName & value_type)
  {
  return SkBoolean::new_instance(*(const UBoolProperty::TCppType *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_integer(const void * value_p, const TypedName & value_type)
  {
  return SkInteger::new_instance(*(const UIntProperty::TCppType *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_real(const void * value_p, const TypedName & value_type)
  {
  return SkReal::new_instance(*(const UFloatProperty::TCppType *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_string(const void * value_p, const TypedName & value_type)
  {
  return SkString::new_instance(FStringToAString(*(const UStrProperty::TCppType *)value_p));
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_vector2(const void * value_p, const TypedName & value_type)
  {
  return SkVector2::new_instance(*(const FVector2D *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_vector3(const void * value_p, const TypedName & value_type)
  {
  return SkVector3::new_instance(*(const FVector *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_vector4(const void * value_p, const TypedName & value_type)
  {
  return SkVector4::new_instance(*(const FVector4 *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_rotation_angles(const void * value_p, const TypedName & value_type)
  {
  return SkRotationAngles::new_instance(*(const FRotator *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_transform(const void * value_p, const TypedName & value_type)
  {
  return SkTransform::new_instance(*(const FTransform *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_struct_val(const void * value_p, const TypedName & value_type)
  {
  void * dest_p;
  SkInstance * instance_p = SkInstance::new_instance_uninitialized_val(value_type.m_sk_class_p, value_type.m_byte_size, &dest_p);

  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p, value_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->InitializeStruct(dest_p);
      ue_struct_p->CopyScriptStruct(dest_p, value_p);
      }
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_struct_ref(const void * value_p, const TypedName & value_type)
  {
  void * dest_p;
  SkInstance * instance_p = SkInstance::new_instance_uninitialized_ref(value_type.m_sk_class_p, value_type.m_byte_size, &dest_p);

  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p, value_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->InitializeStruct(dest_p);
      ue_struct_p->CopyScriptStruct(dest_p, value_p);
      }
  return instance_p;
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_entity(const void * value_p, const TypedName & value_type)
  {
  return SkUEEntity::new_instance(*(UObject * const *)value_p);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkUEReflectionManager::fetch_k2_value_enum(const void * value_p, const TypedName & value_type)
  {
  SkInstance * instance_p = value_type.m_sk_class_p->new_instance();
  instance_p->construct<SkEnum>(tSkEnum(*(const UByteProperty::TCppType *)value_p));
  return instance_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_boolean(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkBoolean>() = *(const UBoolProperty::TCppType *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_integer(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkInteger>() = *(const UIntProperty::TCppType *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_real(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkReal>() = *(const UFloatProperty::TCppType *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_string(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkString>() = FStringToAString(*(const UStrProperty::TCppType *)value_p);
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_vector2(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkVector2>() = *(const FVector2D *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_vector3(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkVector3>() = *(const FVector *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_vector4(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkVector4>() = *(const FVector4 *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_rotation_angles(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkRotationAngles>() = *(const FRotator *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_transform(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkTransform>() = *(const FTransform *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_struct_val(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Unknown struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p->get_raw_pointer_val(), value_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->CopyScriptStruct(dest_p->get_raw_pointer_val(), value_p);
      }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_struct_ref(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p->get_raw_pointer_ref(), value_p, value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->CopyScriptStruct(dest_p->get_raw_pointer_ref(), value_p);
      }
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_entity(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkUEEntity>() = *(UObject * const *)value_p;
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_enum(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  dest_p->as<SkEnum>() = tSkEnum(*(const UByteProperty::TCppType *)value_p);
  }

//---------------------------------------------------------------------------------------

void SkUEReflectionManager::assign_k2_value_array(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type)
  {
  const UArrayProperty::TCppType * array_p = (const UArrayProperty::TCppType *)value_p; // = FScriptArray
  SkInstanceList & list = dest_p->as<SkList>();
  list.ensure_size_empty(array_p->Num()); // For now, delete and repopulate array
  APArray<SkInstance> & list_instances = list.get_instances();
  tK2ValueFetcher fetcher_p = value_type.m_inner_fetcher_p;
  const uint8_t * item_array_p = (const uint8_t *)array_p->GetData();
  size_t item_size = value_type.m_byte_size;
  for (uint32_t i = array_p->Num(); i; --i)
    {
    list_instances.append(*(*fetcher_p)(item_array_p, value_type));
    item_array_p += item_size;
    }
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_boolean(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  *((UBoolProperty::TCppType *)dest_p) = value_p->as<SkBoolean>();
  return sizeof(UBoolProperty::TCppType);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_integer(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  *((UIntProperty::TCppType *)dest_p) = value_p->as<SkInteger>();
  return sizeof(UIntProperty::TCppType);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_real(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  *((UFloatProperty::TCppType *)dest_p) = value_p->as<SkReal>();
  return sizeof(UFloatProperty::TCppType);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_string(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) UStrProperty::TCppType(value_p->as<SkString>().as_cstr());
  return sizeof(UStrProperty::TCppType);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_vector2(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) FVector2D(value_p->as<SkVector2>());
  return sizeof(FVector2D);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_vector3(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) FVector(value_p->as<SkVector3>());
  return sizeof(FVector);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_vector4(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) FVector4(value_p->as<SkVector4>());
  return sizeof(FVector4);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_rotation_angles(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) FRotator(value_p->as<SkRotationAngles>());
  return sizeof(FRotator);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_transform(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  new (dest_p) FTransform(value_p->as<SkTransform>());
  return sizeof(FTransform);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_struct_val(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p, value_p->get_raw_pointer_val(), value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->InitializeStruct(dest_p);
      ue_struct_p->CopyScriptStruct(dest_p, value_p->get_raw_pointer_val());
      }
  return value_type.m_byte_size;
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_struct_ref(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  const UScriptStruct * ue_struct_p = SkUEClassBindingHelper::get_ue_struct_from_sk_class(value_type.m_sk_class_p);
  SK_ASSERTX(ue_struct_p, a_str_format("Could not find UE4 struct for Sk class '%s'.", value_type.m_sk_class_name.as_cstr()));
  #ifdef SK_RUNTIME_RECOVER
    if (!ue_struct_p)
      {
      // Can't find struct - use memcpy and hope that works
      FMemory::Memcpy(dest_p, value_p->get_raw_pointer_ref(), value_type.m_byte_size);
      }
    else
  #endif
      {
      // Use proper copy that correctly copies arrays etc.
      ue_struct_p->InitializeStruct(dest_p);
      ue_struct_p->CopyScriptStruct(dest_p, value_p->get_raw_pointer_ref());
      }
  return value_type.m_byte_size;
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_entity(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  *((UObject **)dest_p) = value_p->as<SkUEEntity>();
  return sizeof(UObject *);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_enum(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  *((UByteProperty::TCppType *)dest_p) = (UByteProperty::TCppType)value_p->as<SkEnum>();
  return sizeof(UByteProperty::TCppType);
  }

//---------------------------------------------------------------------------------------

uint32_t SkUEReflectionManager::store_sk_value_array(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type)
  {
  SkInstanceList & list = value_p->as<SkList>();
  APArray<SkInstance> & list_instances = list.get_instances();
  uint32_t num_items = list_instances.get_length();
  uint32_t item_size = value_type.m_byte_size;
  UArrayProperty::TCppType * array_p = new (dest_p) UArrayProperty::TCppType; // = FScriptArray
  array_p->Add(num_items, item_size);
  uint8_t * item_array_p = (uint8_t *)array_p->GetData();
  tSkValueStorer storer_p = value_type.m_inner_storer_p;
  for (uint32_t i = 0; i < num_items; ++i)
    {
    (*storer_p)(item_array_p, list_instances[i], value_type);
    item_array_p += item_size;
    }
  return sizeof(UArrayProperty::TCppType);
  }

