// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Unreal Engine Runtime Script Generator
//=======================================================================================

#pragma once

#include "../../SkookumScriptGenerator/Private/SkookumScriptGeneratorBase.h" // Need this even in cooked builds

#if WITH_EDITORONLY_DATA

#include <AgogCore/ANamed.hpp>
#include <AgogCore/AVSorted.hpp>

class UBlueprint;

//---------------------------------------------------------------------------------------

class ISkookumScriptRuntimeInterface
  {
  public:

    // Types

    enum eChangeType
      {
      ChangeType_created,
      ChangeType_modified,
      ChangeType_deleted
      };

    // Methods

    virtual bool  is_static_class_known_to_skookum(UClass * class_p) const = 0;
    virtual bool  is_static_struct_known_to_skookum(UStruct * struct_p) const = 0;
    virtual bool  is_static_enum_known_to_skookum(UEnum * enum_p) const = 0;

    virtual void  on_class_scripts_changed_by_generator(const FString & class_name, eChangeType change_type) = 0;

  };

//---------------------------------------------------------------------------------------

class FSkookumScriptRuntimeGenerator : public FSkookumScriptGeneratorBase
  {
  public:

    // Methods

                   FSkookumScriptRuntimeGenerator(ISkookumScriptRuntimeInterface * runtime_interface_p);
    virtual       ~FSkookumScriptRuntimeGenerator();

    bool           have_project() const;
    eSkProjectMode get_project_mode() const { return m_project_mode; }
    FString        get_project_file_path();
    FString        get_default_project_file_path();
    int32          get_overlay_path_depth() const;
    FString        make_project_editable();
    UBlueprint *   load_blueprint_asset(const FString & class_path, bool * sk_class_deleted_p);

    bool           reload_skookumscript_ini();
    void           sync_all_class_script_files_from_disk();
    void           delete_all_class_script_files();
    void           update_all_class_script_files(bool allow_members);

    GenerationTargetBase* get_target(eClassScope scope) { return &m_targets[scope]; }

    void           update_class_script_file(UField * type_p, bool allow_non_game_classes, bool allow_members);
    void           rename_class_script_file(UObject * type_p, const FString & old_ue_class_name);
    void           delete_class_script_file(UObject * type_p);
    void           update_used_class_script_files(bool allow_members);
    void           create_root_class_script_file(const TCHAR * sk_class_name_p);

    bool           move_script_file(const FString & old_file_name, const FString & new_file_name);

    // FSkookumScriptGeneratorBase interface implementation

    virtual bool   can_export_property(FProperty * property_p, int32 include_priority, uint32 referenced_flags) override final;
    virtual void   on_type_referenced(UField * type_p, int32 include_priority, uint32 referenced_flags) override final;
    virtual void   report_error(const FString & message) const override final;
    virtual bool   source_control_checkout_or_add(const FString & file_path) const override final;
    virtual bool   source_control_delete(const FString & file_path) const override final;

  protected:

    // Types

    typedef TSet<UStruct *> tUsedClasses;

    struct CachedClassFile : ANamed // Sk class name
      {
      ASymbol                 m_sk_super_name; // Sk superclass name
      FString                 m_body;
      FDateTime               m_file_time_stamp;
      bool                    m_was_synced;

      CachedClassFile(ASymbol sk_class_name);
      CachedClassFile(const FString & class_file_path, bool load_body);

      FString get_file_name() const;
      bool    load(const FString & class_file_path, const FDateTime * file_time_stamp_if_known_p = nullptr);
      bool    save(const FString & class_file_path);
      };

    void              initialize_paths();
    bool              initialize_generation_targets();
    void              set_overlay_path();
    bool              can_export_blueprint_function(UFunction * function_p);
    CachedClassFile * get_or_create_cached_class_file(ASymbol sk_class_name);

    // Data members

    ISkookumScriptRuntimeInterface * m_runtime_interface_p;

    eSkProjectMode  m_project_mode;

    FString         m_project_file_path;
    FString         m_default_project_file_path;

    GenerationTargetBase m_targets[2]; // Indexed by eClassScope (engine or project)

    AVSorted<CachedClassFile, ASymbol> m_class_files; // Key is Sk name of class

    tUsedClasses    m_used_classes;       // All classes used as types (by parameters, properties etc.)

    mutable TArray<FString> m_queued_files_to_checkout; // When source control is not available, we queue files to check out here

  };

#endif // WITH_EDITORONLY_DATA