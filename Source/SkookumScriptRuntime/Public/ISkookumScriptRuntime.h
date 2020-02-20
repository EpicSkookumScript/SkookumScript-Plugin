// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Main entry point for the SkookumScript runtime plugin
//=======================================================================================

#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

class SkClass;
class UClass;
class UBlueprint;
class UBlueprintGeneratedClass;
class UUserDefinedStruct;
class UUserDefinedEnum;
class SkUEBindingsInterface;
class FSkookumScriptRuntimeGenerator;

SKOOKUMSCRIPTRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogSkookum, Log, All);

//---------------------------------------------------------------------------------------
// Interface class for the runtime plugin to call the editor plugin
class ISkookumScriptRuntimeEditorInterface
  {
  public:

  #if WITH_EDITOR

    virtual void  on_class_updated(UClass * ue_class_p) = 0;
    virtual void  on_function_updated(UFunction * ue_function_p, bool is_event) = 0;
    virtual void  on_function_removed_from_class(UClass * ue_class_p) = 0;

  #endif

  };


//---------------------------------------------------------------------------------------
// The public interface to this module
class ISkookumScriptRuntime : public IModuleInterface
  {
  public:

    // Methods

    virtual void  set_project_generated_bindings(SkUEBindingsInterface * project_generated_bindings_p) = 0;

    virtual bool  is_skookum_disabled() const = 0;
    virtual bool  is_freshen_binaries_pending() const = 0;

    #if WITH_EDITOR

      virtual void  set_editor_interface(ISkookumScriptRuntimeEditorInterface * editor_interface_p) = 0;

      virtual bool  is_connected_to_ide() const = 0;
      virtual void  on_application_focus_changed(bool is_active) = 0;
      virtual void  on_editor_map_opened() = 0;
      virtual void  show_ide(const FString & focus_ue_class_name, const FString & focus_ue_member_name, bool is_data_member, bool is_class_member) = 0;
      virtual void  freshen_compiled_binaries_if_have_errors() = 0;

      virtual bool  has_skookum_default_constructor(UClass * class_p) const = 0;
      virtual bool  has_skookum_destructor(UClass * class_p) const = 0;
      virtual bool  is_skookum_class_data_component_class(UClass * class_p) const = 0;
      virtual bool  is_skookum_behavior_component_class(UClass * class_p) const = 0;
      virtual bool  is_skookum_reflected_call(UFunction * function_p) const = 0;
      virtual bool  is_skookum_reflected_event(UFunction * function_p) const = 0;

      virtual void  on_class_added_or_modified(UBlueprint * blueprint_p) = 0;
      virtual void  on_class_renamed(UBlueprint * blueprint_p, const FString & old_ue_class_name) = 0;
      virtual void  on_class_deleted(UBlueprint * blueprint_p) = 0;

      virtual void  on_struct_added_or_modified(UUserDefinedStruct * ue_struct_p) = 0;
      virtual void  on_struct_renamed(UUserDefinedStruct * ue_struct_p, const FString & old_class_name) = 0;
      virtual void  on_struct_deleted(UUserDefinedStruct * ue_struct_p) = 0;

      virtual void  on_enum_added_or_modified(UUserDefinedEnum * ue_enum_p) = 0;
      virtual void  on_enum_renamed(UUserDefinedEnum * ue_enum_p, const FString & old_enum_name) = 0;
      virtual void  on_enum_deleted(UUserDefinedEnum * ue_enum_p) = 0;

      virtual void  on_new_asset(UObject * obj_p) = 0;

      virtual FSkookumScriptRuntimeGenerator* get_runtime_generator() = 0;

    #endif

  };

