// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#pragma once

#include "Misc/FileHelper.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealType.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"

//---------------------------------------------------------------------------------------
// Mode the plugin is currently in 
enum eSkProjectMode
  {
  SkProjectMode_read_only, // IDE is used as a REPL only, no scripts exist in project itself
  SkProjectMode_editable   // Project depends on Sk scripts and plugin features
  };

//---------------------------------------------------------------------------------------
// Contains all types and methods used even in shipping builds
class FSkookumScriptGeneratorHelper
  {
  public:

    //---------------------------------------------------------------------------------------
    // Types

    enum eSkTypeID
      {
      SkTypeID_none = 0,
      SkTypeID_Integer,
      SkTypeID_Real,
      SkTypeID_Boolean,
      SkTypeID_String,
      SkTypeID_Vector2,
      SkTypeID_Vector3,
      SkTypeID_Vector4,
      SkTypeID_Rotation,
      SkTypeID_RotationAngles,
      SkTypeID_Transform,
      SkTypeID_Color,
      SkTypeID_Name,
      SkTypeID_Enum,
      SkTypeID_UStruct,
      SkTypeID_UClass,
      SkTypeID_UObject,
      SkTypeID_UObjectWeakPtr,
      SkTypeID_Delegate,
      SkTypeID_MulticastDelegate,
      SkTypeID_List,

      SkTypeID__count
      };

    // What kind of variable we are dealing with
    enum eDataScope
      {
      DataScope_instance,
      DataScope_class
      };

    //---------------------------------------------------------------------------------------
    // Methods

    static eSkTypeID      get_skookum_property_type(FProperty * property_p, bool allow_all);
    static eSkTypeID      get_skookum_struct_type(UStruct * struct_p);
    static bool           is_property_type_supported(FProperty * property_p);
    static bool           is_struct_type_supported(UStruct * struct_p);
    static bool           is_pod(UStruct * struct_p);
    static bool           does_class_have_static_class(UClass * class_p);
    static UEnum *        get_enum(FField * field_p); // Returns the Enum if it is an enum, nullptr otherwise

    static FString        skookify_param_name(const FString & name, bool append_question_mark);
    static FString        skookify_method_name(const FString & name, FProperty * return_property_p = nullptr);
    static FString        skookify_data_name_basic(const FString & name, bool append_question_mark, eDataScope scope);
    static FString        skookify_class_name_basic(const FString & name);
    static bool           compare_var_name_skookified(const TCHAR * ue_var_name_p, const ANSICHAR * sk_var_name_p);
    static bool           is_skookum_reserved_word(const FString & name);

    //---------------------------------------------------------------------------------------
    // Data

    static const FString  ms_sk_type_id_names[SkTypeID__count]; // Names belonging to the ids above
    static const FString  ms_reserved_keywords[];               // = Forbidden variable names

  #if WITH_EDITOR || HACK_HEADER_GENERATOR
    static const FName    ms_meta_data_key_blueprint_type;
  #endif

    static const FName    ms_skookum_script_instance_property_name;
  };

#if WITH_EDITOR || HACK_HEADER_GENERATOR

//---------------------------------------------------------------------------------------
// This class provides functionality for processing UE4 runtime type information
// and for generating Sk script files

class FSkookumScriptGeneratorBase : public FSkookumScriptGeneratorHelper
  {
  public:

    // Indicates where a class is being declared
    enum eClassScope
      {
      ClassScope_engine,    // Class is declared in engine
      ClassScope_project,   // Class is declared in project
      };

    // Information for engine/project code generation
    struct GenerationTargetBase
      {
      // Initialization result, in order of precedence
      enum eState
        {
        State_invalid,
        State_valid_unchanged,
        State_valid_changed
        };

      struct ClassRenameEntry
        {
        FName     m_package_name; // Full name of package (or NONE to match any package)
        FString   m_replacement;  // New SkookumScript name
        };

      struct DataRenameEntry
        {
        FName     m_package_name; // Full name of package (or NONE to match any package)
        FName     m_owner_name;   // Name of owner (or NONE to match any owner)
        FString   m_replacement;  // New SkookumScript name
        };

      typedef TMultiMap<FName, ClassRenameEntry>  tClassRenameMap; // Maps name of class/struct/enum to rename to replacement
      typedef TMultiMap<FName, DataRenameEntry>   tDataRenameMap;  // Maps name of property to rename to replacement

      FString         m_root_directory_path;  // Root of the runtime plugin or project we're generating the code for
      FString         m_project_name;         // Name of this project
      TSet<FName>     m_skip_classes;         // All classes set to skip in SkookumScript.ini
      tClassRenameMap m_class_rename_map;     // Classes/structs/enums to rename  
      tDataRenameMap  m_data_rename_map;      // Properties to rename  
      
      FDateTime       m_ini_file_stamp;

                   GenerationTargetBase() : m_ini_file_stamp(1) {} // Set to 1 tick to mark dirty

      eState       initialize(const FString & root_directory_path, const FString & project_name, const GenerationTargetBase * inherit_from_p = nullptr);
      bool         is_valid() const;
      FString      get_ini_file_path() const;
      bool         is_type_skipped(FName type_name) const;
      FString      find_class_rename_replacement(FName name, FName package_name) const;
      FString      find_data_rename_replacement(FName name, FName owner_name, FName package_name) const;
      };

    // Helper struct for building path to SkookumScript class
    struct SuperClassEntry
      {
      FString   m_name;
      UStruct * m_class_or_struct_p;

      SuperClassEntry(const FString & name, UStruct * class_or_struct_p) : m_name(name), m_class_or_struct_p(class_or_struct_p) {}
      };

    // (keep these in sync with the same enum in SkTextProgram.hpp)
    enum ePathDepth
      {
      PathDepth_any                = -1, // No limit to path depth
      PathDepth_archived           = -2, // All chunks stored in a single archive file
      PathDepth_archived_per_class = -3  // Chunks stored in an archive file per class
      };

    //---------------------------------------------------------------------------------------
    // Interface

                           FSkookumScriptGeneratorBase() : m_overlay_path_depth(0), m_current_target_p(nullptr) {}
    virtual               ~FSkookumScriptGeneratorBase() {}

    enum eReferenced
      {
      Referenced_by_engine_module = 1 << 0,
      Referenced_by_game_module   = 1 << 1,
      Referenced_as_binding_class = 1 << 2,
      };

    virtual bool          can_export_property(FProperty * property_p, int32 include_priority, uint32 referenced_flags) = 0;
    virtual void          on_type_referenced(UField * type_p, int32 include_priority, uint32 referenced_flags) = 0;
    virtual void          report_error(const FString & message) const = 0;
    virtual bool          source_control_checkout_or_add(const FString & file_path) const { return true; }
    virtual bool          source_control_delete(const FString & file_path)          const { return true; }

    //---------------------------------------------------------------------------------------
    // Methods

    FString               get_or_create_project_file(const FString & ue_project_directory_path, const TCHAR * project_name_p, eSkProjectMode * project_mode_p = nullptr, bool * created_p = nullptr);
    bool                  compute_scripts_path_depth(FString project_ini_file_path, const FString & overlay_name);
    bool                  load_text_file(const FString & file_path, FString & out_contents) const;
    bool                  save_text_file(const FString & file_path, const FString & contents) const;
    bool                  save_text_file_if_changed(const FString & file_path, const FString & new_file_contents); // Helper to change a file only if needed
    void                  flush_saved_text_files(); // Puts generated files into place after all code generation is done

    FString               skookify_class_name(FName name, FName package_name);
    FString               skookify_enum_name(FName name, FName package_name);
    FString               skookify_data_name(FName name, FName owner_name, FName package_name, bool append_question_mark, eDataScope scope);

    FString               get_skookified_enum_val_name_by_index(UEnum * enum_p, int32 index);
    FString               get_skookified_default_enum_val_name_by_id(UEnum * enum_p, const FString & id);

    FString               get_skookum_class_name(FField * type_p);
    FString               get_skookum_class_name(UObject * type_p);
    FString               get_skookum_data_name(FProperty * property_p);
    FString               get_skookum_parent_name(FField * type_p, int32 include_priority, uint32 referenced_flags, UStruct ** out_parent_pp = nullptr);
    FString               get_skookum_parent_name(UField * type_p, int32 include_priority, uint32 referenced_flags, UStruct ** out_parent_pp = nullptr);
    FString               get_skookum_class_path(UObject * type_p, int32 include_priority, uint32 referenced_flags, FString * out_class_name_p = nullptr);
    FString               get_skookum_method_file_name(const FString & script_function_name, bool is_static);
    FString               get_skookum_property_type_name(FProperty * property_p, bool include_invokable_signature = false, int32 * out_first_line_length_p = nullptr, int32 * out_max_line_length_p = nullptr);
    FString               get_skookum_default_initializer(UFunction * function_p, FProperty * param_p);
    static uint32         get_skookum_symbol_id(const FString & string);
    static FString        get_comment_block(UObject * type_p);
    static FString        get_comment_block(FField * type_p);
    static void           build_comment_block(FString& comment_block, const FString& ue_name_comment);

    static UField *       get_field(UObject * type_p); // Find its UField if type is not a UField

    static FString        multiline_right_pad(const FString & text, int32 desired_width);

    FString               generate_routine_script_parameters(UFunction * function_p, int32 indent_spaces, int32 * out_num_inputs_p = nullptr, FString * out_return_type_name_p = nullptr, int32 * out_max_line_length_p = nullptr); // Generate script code for a routine's parameter list (without parentheses)

    FString               generate_class_meta_file_body(UObject * type_p);
    FString               generate_class_instance_data_file_body(UStruct * class_or_struct_p, int32 include_priority, uint32 referenced_flags);
    FString               generate_enum_class_data_body(UEnum * enum_p);
    FString               generate_enum_class_constructor_body(UEnum * enum_p);
    FString               generate_method_script_file_body(UFunction * function_p, const FString & script_function_name); // Generate script file for a method

    void                  generate_class_meta_file(UField * type_p, const FString & class_path, const FString & skookum_class_name);
    void                  generate_root_meta_file(const TCHAR * root_class_name_p); // Generate meta file for a root class like Enum or UStruct

    //---------------------------------------------------------------------------------------
    // Data

    static const FFileHelper::EEncodingOptions  ms_script_file_encoding;

    static const FName           ms_meta_data_key_function_category;
    static const FName           ms_meta_data_key_display_name;

    static const FString         ms_asset_name_key; // Label used to extract asset name from Sk class meta file
    static const FString         ms_package_name_key; // Label used to extract package name from Sk class meta file
    //static const FString         ms_package_path_key; // Label used to extract package path from Sk class meta file
    static TCHAR const * const   ms_editable_ini_settings_p; // ini file settings to describe that a project is not editable
    static TCHAR const * const   ms_overlay_name_bp_p; // Name of overlay used for Sk classes generated from Blueprints
    static TCHAR const * const   ms_overlay_name_bp_old_p; // Legacy 2016-07-18 - remove after some time
    static TCHAR const * const   ms_overlay_name_cpp_p; // Name of overlay used for Sk classes generated from C++ reflection macros

    static const FName ms_name_object;
    static const FName ms_name_class;
    static const FName ms_name_entity;
    static const FName ms_name_vector;
    static const FName ms_name_enum;

    static const FName ms_name_data_table_function_library;
    static const FName ms_name_gameplay_statics;
    static const FName ms_name_head_mounted_display_function_library;
    static const FName ms_name_kismet_array_library;
    static const FName ms_name_kismet_guid_library;
    static const FName ms_name_kismet_input_library;
    static const FName ms_name_kismet_material_library;
    static const FName ms_name_kismet_math_library;
    static const FName ms_name_kismet_node_helper_library;
    static const FName ms_name_kismet_string_library;
    static const FName ms_name_kismet_system_library;
    static const FName ms_name_kismet_text_library;
    static const FName ms_name_visual_logger_kismet_library;

    FString                      m_overlay_path;       // Folder where to place generated script files
    int32                        m_overlay_path_depth; // Amount of super classes until we start flattening the script file hierarchy due to the evil reign of Windows MAX_PATH. 1 = everything is right under 'Object', 0 is not allowed, -1 means "no limit" and -2 means single archive file

    const GenerationTargetBase * m_current_target_p;   // Currently active generation target

    TArray<FString>              m_temp_file_paths;    // Keep track of temp files generated by save_files_if_changed()
  };

#endif // WITH_EDITOR || HACK_HEADER_GENERATOR
