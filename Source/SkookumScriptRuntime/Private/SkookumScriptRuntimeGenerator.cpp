// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Unreal Engine Runtime Script Generator
//=======================================================================================

#include "SkookumScriptRuntimeGenerator.h"
#include "../../SkookumScriptGenerator/Private/SkookumScriptGeneratorBase.inl" // Need this even in cooked builds

#if WITH_EDITORONLY_DATA

#include "Settings/ProjectPackagingSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/UserDefinedStruct.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "Interfaces/IMainFrameModule.h"
#include "SourceControlHelpers.h"
#include "SourceControlOperations.h"

#include "Bindings/SkUEReflectionManager.hpp"
#include "Bindings/SkUEUtils.hpp"

#include <AgogCore/ASymbol.hpp>
#include <SkookumScript/SkDebug.hpp>

//=======================================================================================

//---------------------------------------------------------------------------------------

FSkookumScriptRuntimeGenerator::FSkookumScriptRuntimeGenerator(ISkookumScriptRuntimeInterface * runtime_interface_p)
  : m_runtime_interface_p(runtime_interface_p)
  , m_project_mode(SkProjectMode_read_only)
  {
  // Reset super classes
  m_used_classes.Empty();

  // Set up project and overlay paths
  initialize_paths();

  // LEGACY: Update old projects to new format
  if (have_project() && m_overlay_path_depth != PathDepth_archived_per_class)
    {
    // Update project ini file to new format
    FString project_file_body;
    load_text_file(m_project_file_path, project_file_body);
    const TCHAR * const overlay_names_p[2] = { ms_overlay_name_bp_p, ms_overlay_name_bp_old_p };
    const TCHAR * const end_chars_p[3] = { TEXT(""), TEXT("\\"), TEXT("/") };
    int32 replace_count = 0;
    for (int32 oldnew_idx = 0; oldnew_idx < A_COUNT_OF(overlay_names_p) && !replace_count; ++oldnew_idx)
      {
      const TCHAR * overlay_name_p = overlay_names_p[oldnew_idx];
      for (int32 path_end_idx = 0; path_end_idx < A_COUNT_OF(end_chars_p) && !replace_count; ++path_end_idx)
        {
        FString search_string_base = FString(overlay_name_p) + end_chars_p[path_end_idx];
        replace_count += project_file_body.ReplaceInline(*(search_string_base + TEXT("|1")), *(search_string_base + TEXT("|C")), ESearchCase::CaseSensitive);
        }
      }
    if (replace_count)
      {
      save_text_file(m_project_file_path, project_file_body);
      source_control_checkout_or_add(m_project_file_path);
      m_overlay_path_depth = PathDepth_archived_per_class;
      // Delete old files
      delete_all_class_script_files();
      // Let user know
      FText title = FText::FromString(TEXT("Project file updated"));
      FMessageDialog::Open(
        EAppMsgType::Ok,
        FText::Format(FText::FromString(
          TEXT("Beginning with release 3.0.5279, the overlay '{0}' is stored in a new optimized file format.\n")
          TEXT("Your project file '{1}' was automatically migrated to the new format. All old files in the overlay '{0}' were deleted.\n")
          TEXT("As a result, you may temporarily experience script compile errors, and the plugin may temporarily be unable to load the compiled binaries.\n")
          TEXT("If you get a message from UE4Editor that the compiled binaries could not be generated, simply select 'Continue' to continue loading the project. ")
          TEXT("The contents of the overlay '{0}' will eventually get regenerated and the compile errors will resolve automatically.")),
          FText::FromString(replace_count ? ms_overlay_name_bp_p : ms_overlay_name_bp_old_p), FText::FromString(m_project_file_path)),
        &title);
      }
    }

  // Initialize GenerationTargetBases
  initialize_generation_targets();

  // Do an initial sync from disk
  sync_all_class_script_files_from_disk();
  }

//---------------------------------------------------------------------------------------

FSkookumScriptRuntimeGenerator::~FSkookumScriptRuntimeGenerator()
  {
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::have_project() const
  {
  return FApp::GetGameName() && FApp::GetGameName()[0] && FPlatformString::Strcmp(FApp::GetGameName(), TEXT("None")) != 0;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptRuntimeGenerator::get_project_file_path()
  {
  return m_project_file_path;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptRuntimeGenerator::get_default_project_file_path()
  {
  return m_default_project_file_path;
  }

//---------------------------------------------------------------------------------------

int32 FSkookumScriptRuntimeGenerator::get_overlay_path_depth() const
  {
  return m_overlay_path_depth;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptRuntimeGenerator::make_project_editable()
  {
  FString error_msg;

  if (!have_project())
    {
    error_msg = TEXT("Tried to make project editable but engine has no project loaded!");
    }
  else
    {
    // Check if maybe already editable - if so, silently do nothing
    FString editable_scripts_path = FPaths::GameDir() / TEXT("Scripts");
    FString editable_project_file_path(editable_scripts_path / TEXT("Skookum-project.ini"));
    if (FPaths::FileExists(editable_project_file_path))
      {
      // Found editable project - make sure mode is set properly
      m_project_mode = SkProjectMode_editable;
      }
    else
      {
      // No editable project found - check temporary location (in `Intermediate` folder)
      // We don't use FPaths::GameIntermediateDir() here as that might point to the %APPDATA% folder
      FString temp_root_path(FPaths::GameDir() / TEXT("Intermediate/SkookumScript"));
      FString temp_scripts_path(temp_root_path / TEXT("Scripts"));
      FString temp_project_file_path = temp_scripts_path / TEXT("Skookum-project.ini");
      if (!FPaths::FileExists(temp_project_file_path))
        {
        error_msg = TEXT("Tried to make project editable but neither an editable nor a non-editable project was found!");
        }
      else
        {
        if (!IFileManager::Get().Move(*editable_scripts_path, *temp_scripts_path, true, true))
          {
          error_msg = TEXT("Failed moving project information from temporary to editable location!");
          }
        else
          {
          // Move compiled binaries for convenience
          // We don't care if this succeeds
          FString temp_binary_folder_path = temp_root_path / TEXT("Content/SkookumScript");
          FString editable_binary_folder_path = FPaths::GameDir() / TEXT("Content/SkookumScript");
          IFileManager::Get().Move(*editable_binary_folder_path, *temp_binary_folder_path, true, true);

          // Change project packaging settings to include Sk binaries
          UProjectPackagingSettings * packaging_settings_p = Cast<UProjectPackagingSettings>(UProjectPackagingSettings::StaticClass()->GetDefaultObject());
          const TCHAR * binary_path_name_p = TEXT("SkookumScript");
          for (TArray<FDirectoryPath>::TConstIterator dir_path(packaging_settings_p->DirectoriesToAlwaysStageAsUFS); dir_path; ++dir_path)
            {
            if (dir_path->Path == binary_path_name_p)
              {
              binary_path_name_p = nullptr;
              break;
              }
            }
          if (binary_path_name_p)
            {
            FDirectoryPath binary_path;
            binary_path.Path = binary_path_name_p;
            packaging_settings_p->DirectoriesToAlwaysStageAsUFS.Add(binary_path);
            FString config_file_path = FPaths::GameConfigDir() / TEXT("DefaultGame.ini");
            source_control_checkout_or_add(config_file_path);
            packaging_settings_p->SaveConfig(CPF_Config, *config_file_path);
            }

          // Create Project overlay folder
          IFileManager::Get().MakeDirectory(*(editable_scripts_path / TEXT("Project/Object")), true);

          // Change project to be editable
          FString proj_ini;
          if (load_text_file(editable_project_file_path, proj_ini))
            {
            proj_ini = proj_ini.Replace(ms_editable_ini_settings_p, TEXT("")); // Remove editable settings
            proj_ini += TEXT("Overlay9=Project|Project\n"); // Create Project overlay definition
            save_text_file(editable_project_file_path, proj_ini);
            }

          // Now in new mode
          m_project_mode = SkProjectMode_editable;
          // Remember new project path
          m_project_file_path = FPaths::ConvertRelativePathToFull(editable_project_file_path);
          // Also update overlay path and depth
          set_overlay_path();

          // Add project file and class archive files to source control
          source_control_checkout_or_add(m_project_file_path);
          TArray<FString> all_class_file_names;
          all_class_file_names.Reserve(512);
          IFileManager::Get().FindFiles(all_class_file_names, *(m_overlay_path / TEXT("*.sk")), true, false);
          for (const FString & class_file_name : all_class_file_names)
            {
            source_control_checkout_or_add(m_overlay_path / class_file_name);
            }

          // Update the class file cache
          m_class_files.empty();
          sync_all_class_script_files_from_disk();
          }
        }
      }
    }

  return error_msg;
  }

//---------------------------------------------------------------------------------------
// Attempt to load blueprint with given qualified class path
UBlueprint * FSkookumScriptRuntimeGenerator::load_blueprint_asset(const FString & class_path, bool * sk_class_deleted_p)
  {
  // Try to extract asset path from meta file of Sk class
  FString full_class_path = m_overlay_path / class_path;
  FString meta_file_path = full_class_path / TEXT("!Class.sk-meta");
  FString meta_file_text;
  *sk_class_deleted_p = false;
  if (load_text_file(meta_file_path, meta_file_text))
    {
    // Found meta file - try to extract asset path contained in it
    int32 package_path_begin_pos = meta_file_text.Find(ms_package_name_key);

    // Temporary clean-up hack (2016-06-19): We only support Game assets now, so if not a game asset, it's an old script file lingering around
    if (package_path_begin_pos < 0 || meta_file_text.Mid(package_path_begin_pos + ms_package_name_key.Len(), 6) != TEXT("/Game/"))
      {
      // If it has a path and it's not "/Game/" then delete it and pretend it never existed
      if (package_path_begin_pos >= 0)
        {
        IFileManager::Get().DeleteDirectory(*full_class_path, false, true);
        }
      return nullptr;
      }

    if (package_path_begin_pos >= 0)
      {
      package_path_begin_pos += ms_package_name_key.Len();
      int32 package_path_end_pos = meta_file_text.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, package_path_begin_pos);
      if (package_path_end_pos > package_path_begin_pos)
        {
        // Successfully got the path of the package, so assemble with asset name and load the asset
        FString package_path = meta_file_text.Mid(package_path_begin_pos, package_path_end_pos - package_path_begin_pos);

        // Get Sk class name
        FString class_name = FPaths::GetCleanFilename(class_path);
        // If there's a dot in the class name, use portion right of it
        int dot_pos = -1;
        if (class_name.FindChar(TCHAR('.'), dot_pos))
          {
          class_name = class_name.Mid(dot_pos + 1);
          }
        // Default asset name is the Sk class name
        FString asset_name = class_name;
        // Now try to extract exact asset name from comment, as it may differ in punctuation
        int32 asset_name_begin_pos = meta_file_text.Find(ms_asset_name_key);
        if (asset_name_begin_pos >= 0)
          {
          asset_name_begin_pos += ms_asset_name_key.Len();
          int32 asset_name_end_pos = meta_file_text.Find(TEXT("\r\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, asset_name_begin_pos);
          if (asset_name_end_pos <= asset_name_begin_pos)
            {
            asset_name_end_pos = meta_file_text.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, asset_name_begin_pos);
            }
          if (asset_name_end_pos > asset_name_begin_pos)
            {
            asset_name = meta_file_text.Mid(asset_name_begin_pos, asset_name_end_pos - asset_name_begin_pos);
            }
          }

        // Now try to find the Blueprint
        FString asset_path = package_path + TEXT(".") + asset_name;
        UBlueprint * blueprint_p = LoadObject<UBlueprint>(nullptr, *asset_path);
        if (!blueprint_p)
          {
          // Asset not found, ask the user what to do
          FText title = FText::Format(FText::FromString(TEXT("Asset Not Found For {0}")), FText::FromString(class_name));
          if (FMessageDialog::Open(
            EAppMsgType::YesNo,
            FText::Format(FText::FromString(
              TEXT("Cannot find Blueprint asset belonging to SkookumScript class '{0}'. ")
              TEXT("It was originally generated from the asset '{1}' but this asset appears to no longer exist. ")
              TEXT("Maybe it was deleted or renamed. ")
              TEXT("If you no longer need the SkookumScript class '{0}', you can fix this issue by deleting the class. ")
              TEXT("Would you like to delete the SkookumScript class '{0}'?")), FText::FromString(class_name), FText::FromString(asset_path)),
            &title) == EAppReturnType::Yes)
            {
            // User requested deletion, so nuke it
            IFileManager::Get().DeleteDirectory(*full_class_path, false, true);
            *sk_class_deleted_p = true;
            m_runtime_interface_p->on_class_scripts_changed_by_generator(class_name, ISkookumScriptRuntimeInterface::ChangeType_deleted);
            }
          }
        return blueprint_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::can_export_property(UProperty * var_p, int32 include_priority, uint32 referenced_flags)
  {
  if (!is_property_type_supported(var_p))
    {
    return false;
    }

  // Accept all known static object types plus all generated by Blueprints
  UObjectPropertyBase * object_var_p = Cast<UObjectPropertyBase>(var_p);
  if (object_var_p && (!object_var_p->PropertyClass || (!m_runtime_interface_p->is_static_class_known_to_skookum(object_var_p->PropertyClass) && !UBlueprint::GetBlueprintFromClass(object_var_p->PropertyClass))))
    {
    return false;
    }

  // Accept all known static struct types plus all user generated structs
  UStructProperty * struct_var_p = Cast<UStructProperty>(var_p);
  if (struct_var_p && (!struct_var_p->Struct || (get_skookum_struct_type(struct_var_p->Struct) == SkTypeID_UStruct
   && !m_runtime_interface_p->is_static_struct_known_to_skookum(struct_var_p->Struct) && !struct_var_p->Struct->IsA<UUserDefinedStruct>())))
    {
    return false;
    }

  // Accept all known static enum types plus all UUserDefinedEnums
  UEnum * enum_p = get_enum(var_p);
  if (enum_p && !m_runtime_interface_p->is_static_enum_known_to_skookum(enum_p) && !enum_p->IsA<UUserDefinedEnum>())
    {
    return false;
    }

  // Accept all arrays of known types
  UArrayProperty * array_var_p = Cast<UArrayProperty>(var_p);
  if (array_var_p && (!array_var_p->Inner || !can_export_property(array_var_p->Inner, include_priority + 1, referenced_flags)))
    {
    return false;
    }

  // Accept delegates as long as their signatures have acceptable parameters
  UFunction * signature_function_p = nullptr;
  UDelegateProperty * delegate_p = Cast<UDelegateProperty>(var_p);
  if (delegate_p)
    {
    signature_function_p = delegate_p->SignatureFunction;
    }
  UMulticastDelegateProperty * multicast_delegate_p = Cast<UMulticastDelegateProperty>(var_p);
  if (multicast_delegate_p)
    {
    signature_function_p = multicast_delegate_p->SignatureFunction;
    }
  if (signature_function_p)
    {
    // Reject if any of the parameter types is unsupported 
    for (TFieldIterator<UProperty> param_it(signature_function_p); param_it; ++param_it)
      {
      if (!can_export_property(*param_it, include_priority + 1, referenced_flags))
        {
        return false;
        }
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::on_type_referenced(UField * type_p, int32 include_priority, uint32 referenced_flags)
  {
  // In this use case this callback should never be called for anything but structs/classes
  m_used_classes.Add(CastChecked<UStruct>(type_p));
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::report_error(const FString & message) const
  {
  FText title = FText::FromString(TEXT("Error during SkookumScript script file generation!"));
  FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(message), &title);
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::source_control_checkout_or_add(const FString & file_path) const
  {
  // In read-only mode, do nothing
  if (m_project_mode == SkProjectMode_read_only)
    {
    return true;
    }

  // Hack for 4.18 - the source control module will crash if the main frame is not initialized yet
  IMainFrameModule * main_frame_module_p = FModuleManager::GetModulePtr<IMainFrameModule>("MainFrame");
  if (main_frame_module_p && main_frame_module_p->IsWindowInitialized())
    {
    // Call UE4 implementation to do the work
    if (USourceControlHelpers::CheckOutFile(file_path))
      {
      // If successful, also check out all the queued file paths
      for (const FString & queued_file_path : m_queued_files_to_checkout)
        {
        USourceControlHelpers::CheckOutFile(queued_file_path);
        }
      m_queued_files_to_checkout.Empty();
      return true;
      }
    }

  // Cannot check out now - remember for later checkout
  m_queued_files_to_checkout.AddUnique(file_path);
  return false;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::source_control_delete(const FString & file_path) const
  {
  // Deal with source control only in editable mode 
  if (m_project_mode == SkProjectMode_editable)
    {
    // There seems to be no UE4 implementation of this that is ready to use, so we implement it here
    ISourceControlProvider & provider = ISourceControlModule::Get().GetProvider();

    // first check for source control check out
    if (ISourceControlModule::Get().IsEnabled())
      {
      FSourceControlStatePtr source_control_state_p = provider.GetState(file_path, EStateCacheUsage::ForceUpdate);
      if (source_control_state_p.IsValid())
        {
        if (source_control_state_p->IsSourceControlled())
          {
          if (source_control_state_p->CanCheckout())
            {
            ECommandResult::Type result = provider.Execute(ISourceControlOperation::Create<FDelete>(), file_path);
            if (result != ECommandResult::Succeeded)
              {
              report_error(FString::Printf(TEXT("Could not mark file `%s` for delete.\n"), *file_path));
              }
            }
          else if (source_control_state_p->IsAdded())
            {
            ECommandResult::Type result = provider.Execute(ISourceControlOperation::Create<FRevert>(), file_path);
            if (result != ECommandResult::Succeeded)
              {
              report_error(FString::Printf(TEXT("Could not revert file `%s`.\n"), *file_path));
              }
            }
          else if (source_control_state_p->IsCheckedOut())
            {
            ECommandResult::Type result = provider.Execute(ISourceControlOperation::Create<FRevert>(), file_path);
            if (result != ECommandResult::Succeeded)
              {
              report_error(FString::Printf(TEXT("Could not revert file `%s`.\n"), *file_path));
              }
            else
              {
              result = provider.Execute(ISourceControlOperation::Create<FDelete>(), file_path);
              if (result != ECommandResult::Succeeded)
                {
                report_error(FString::Printf(TEXT("Could not mark file `%s` for delete.\n"), *file_path));
                }
              }
            }
          }
        }
      }
    }

  // Now delete file (if still exists)
  return IFileManager::Get().Delete(*file_path, false, true);
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::reload_skookumscript_ini()
  {
  return initialize_generation_targets();
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::sync_all_class_script_files_from_disk()
  {
  // Bail if nothing to do
  if (m_overlay_path.IsEmpty()) return;

  // Find all files and update cache
  TArray<FString> all_class_file_names;
  all_class_file_names.Reserve(512);
  IFileManager::Get().FindFiles(all_class_file_names, *(m_overlay_path / TEXT("*.sk")), true, false);
  for (const FString & class_file_name : all_class_file_names)
    {
    // Parse file name and get time stamp of file
    FString class_file_path = m_overlay_path / class_file_name;
    CachedClassFile file(class_file_path, false);

    // See if we already got a cached entry
    uint32_t insert_pos;
    CachedClassFile * cached_file_p = m_class_files.get(file.get_name(), AMatch_first_found, &insert_pos);
    if (cached_file_p)
      {
      // Touch it
      cached_file_p->m_was_synced = true;

      // Did parent change?
      if (file.m_sk_super_name != cached_file_p->m_sk_super_name)
        {
        // Yes, then consider the file on disk authoritative and delete other existing file
        FString old_file_path = m_overlay_path / cached_file_p->get_file_name();
        source_control_delete(old_file_path);
        // Change superclass to new one
        cached_file_p->m_sk_super_name = file.m_sk_super_name;
        // And load (potentially) different file body
        if (!cached_file_p->load(class_file_path, &file.m_file_time_stamp))
          {
          report_error(FString::Printf(TEXT("Couldn't load class file '%s'"), *class_file_path));
          }
        }
      // Did file change on disk?
      else if (file.m_file_time_stamp > cached_file_p->m_file_time_stamp)
        {
        // Yes, the consider the file on disk authoritative and load it
        if (!cached_file_p->load(class_file_path, &file.m_file_time_stamp))
          {
          report_error(FString::Printf(TEXT("Couldn't load class file '%s'"), *class_file_path));
          }
        }
      }
    else
      {
      // Not cached yet, load and insert it now
      if (!file.load(class_file_path, &file.m_file_time_stamp))
        {
        report_error(FString::Printf(TEXT("Couldn't load class file '%s'"), *class_file_path));
        }
      file.m_was_synced = true;
      m_class_files.insert(file, insert_pos);
      }
    }

  // Remove all cached files that were not found on disk
  for (uint32_t idx = 0; idx < m_class_files.get_length(); ++idx)
    {
    CachedClassFile & cached_file = m_class_files[idx];
    if (cached_file.m_was_synced)
      {
      cached_file.m_was_synced = false;
      }
    else
      {
      m_class_files.remove(idx--);
      }
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::delete_all_class_script_files()
  {
  // Bail if nothing to do
  if (m_overlay_path.IsEmpty()) return;

  // Clear contents of overlay scripts folder
  TArray<FString> all_class_file_names;
  all_class_file_names.Reserve(512);
  IFileManager::Get().FindFiles(all_class_file_names, *(m_overlay_path / TEXT("*.sk")), true, false);
  for (const FString & class_file_name : all_class_file_names)
    {
    source_control_delete(m_overlay_path / class_file_name);
    }

  // Clear cache
  m_class_files.empty();

  // Reset used classes
  m_used_classes.Empty();

  // LEGACY: Also remove all files generated by previous versions of the plugin
  all_class_file_names.Empty();
  IFileManager::Get().FindFilesRecursive(all_class_file_names, *(m_overlay_path / TEXT("Object")), TEXT("*.*"), true, false);
  for (const FString & class_file_path : all_class_file_names)
    {
    source_control_delete(class_file_path);
    }
  }

//---------------------------------------------------------------------------------------
// Generate SkookumScript class script files for all known blueprint assets
void FSkookumScriptRuntimeGenerator::update_all_class_script_files(bool allow_members)
  {
  // Bail if nothing to do
  if (m_overlay_path.IsEmpty()) return;

  // Re-generate classes for all Blueprints
  for (TObjectIterator<UBlueprint> blueprint_it; blueprint_it; ++blueprint_it)
    {
    UClass * ue_class_p = blueprint_it->GeneratedClass;
    if (ue_class_p)
      {
      update_class_script_file(ue_class_p, true, allow_members);
      }
    }

  // Re-generate classes for all user defined structs
  for (TObjectIterator<UUserDefinedStruct> struct_it; struct_it; ++struct_it)
    {
    update_class_script_file(*struct_it, true, allow_members);
    }

  // Re-generate classes for all user defined enums
  for (TObjectIterator<UUserDefinedEnum> enum_it; enum_it; ++enum_it)
    {
    update_class_script_file(*enum_it, true, allow_members);
    }

  // Re-generate the ECollisionChannel enum class in the Project-generated overlay as it gets customized by the game project
  //UEnum * ecc_enum_p = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECollisionChannel"), true);
  //generate_class_script_files(ecc_enum_p, true, false, false);

  // Also generate any dependent classes
  update_used_class_script_files(allow_members);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::update_class_script_file(UField * type_p, bool allow_non_game_classes, bool allow_members)
  {
  SK_ASSERTX(!m_overlay_path.IsEmpty(), "Overlay path for script generation not set!");

  // Do not generate any script files in commandlet mode
  if (IsRunningCommandlet())
    {
    return;
    }

  // Only generate script files for game assets if requested
  UPackage * package_p = Cast<UPackage>(type_p->GetOutermost());
  if (!allow_non_game_classes && (!package_p || (!package_p->FileName.ToString().StartsWith(TEXT("/Game/")) && !package_p->GetName().StartsWith(TEXT("/Game/")))))
    {
    return;
    }

  // Do not generate anything if class is temporary, deleted or CDO
  if (type_p->GetName().StartsWith(TEXT("REINST_"))
   || type_p->GetName().StartsWith(TEXT("SKEL_"))
   || type_p->GetName().StartsWith(TEXT("TRASH_"))
   || type_p->GetName().StartsWith(TEXT("GC_"))
   || type_p->GetName().StartsWith(TEXT("Default__")))
    {
    return;
    }

#if !PLATFORM_EXCEPTIONS_DISABLED
  try
#endif
    {
    // Determine Sk class and superclass name
    FString sk_class_name = get_skookum_class_name(type_p);
    FString sk_super_name = get_skookum_parent_name(type_p, 0, 0);

    // Don't process any classes that are marked as skipped or any classes with a superclass marked as skip
    if (m_targets[ClassScope_project].is_type_skipped(type_p->GetFName())
      || m_targets[ClassScope_engine].is_type_skipped(type_p->GetFName()))
    {
      return;
    }

    // Create body of class archive file
    // First, meta body
    FString body = generate_class_meta_file_body(type_p) + TEXT("\n");

    // Then, members, but only for BP types
    bool is_bp_type = type_p->IsA<UBlueprintGeneratedClass>() || type_p->IsA<UUserDefinedStruct>() || type_p->IsA<UUserDefinedEnum>();
    if (is_bp_type && allow_members )
      {
      // Is it a class or struct?
      UStruct * struct_or_class_p = Cast<UStruct>(type_p);
      if (struct_or_class_p)
        {
        // Generate instance data member declarations
        body += TEXT("$$ @\n");
        body += generate_class_instance_data_file_body(struct_or_class_p, 0, Referenced_by_game_module) + TEXT("\n");

        // Generate ctor/dtor/assignment method scripts for user defined structs
        if (struct_or_class_p->IsA<UUserDefinedStruct>())
          {
          body += TEXT("$$ @!\n");
          body += FString::Printf(TEXT("() %s\n\n"), *sk_class_name);
          body += TEXT("$$ @!copy\n");
          body += FString::Printf(TEXT("(%s other) %s\n\n"), *sk_class_name, *sk_class_name);
          body += TEXT("$$ @assign\n");
          body += FString::Printf(TEXT("(%s other) %s\n\n"), *sk_class_name, *sk_class_name);
          body += TEXT("$$ @!!\n");
          body += FString::Printf(TEXT("() %s\n\n"), *sk_class_name);
          }

        // Generate methods for custom events and Blueprint functions
        if (struct_or_class_p->IsA<UBlueprintGeneratedClass>())
          {
          // Write out the methods
          TSet<FString> method_file_names;
          for (TFieldIterator<UFunction> func_it(struct_or_class_p, EFieldIteratorFlags::ExcludeSuper); func_it; ++func_it)
            {
            UFunction * function_p = *func_it;
            if (can_export_blueprint_function(function_p))
              {
              FString script_method_name = skookify_method_name(function_p->GetName());
              body += FString::Printf(TEXT("$$ %s%s\n"), function_p->HasAnyFunctionFlags(FUNC_Static) ? TEXT("@@") : TEXT("@"), *script_method_name);
              body += generate_method_script_file_body(function_p, script_method_name) + TEXT("\n");
              }
            }
          }
        }

      // Is it an enum?
      UEnum * enum_p = Cast<UEnum>(type_p);
      if (enum_p)
        {
        // Generate class data member declarations
        body += TEXT("$$ @@\n");
        body += generate_enum_class_data_body(enum_p) + TEXT("\n");

        // Generate class constructor to initialize the data members
        body += TEXT("$$ @@!\n");
        body += generate_enum_class_constructor_body(enum_p) + TEXT("\n");
        }
      }

    body += TEXT("$$ .\n");

    // Now get or create cached file entry
    CachedClassFile * cached_file_p = get_or_create_cached_class_file(ASymbol::create(FStringToAString(sk_class_name)));
    bool is_existing = (cached_file_p->m_file_time_stamp.GetTicks() > 0u);
    ASymbol super_name = ASymbol::create(FStringToAString(sk_super_name));

    // Check if superclass changed
    bool anything_changed = false;
    if (cached_file_p->m_sk_super_name != super_name)
      {
      // Delete old file
      if (!cached_file_p->m_sk_super_name.is_null())
        {
        FString old_file_path = m_overlay_path / cached_file_p->get_file_name();
        source_control_delete(old_file_path);
        }

      // Write new file and update time stamp
      cached_file_p->m_sk_super_name = super_name;
      cached_file_p->m_body = body;
      FString new_file_path = m_overlay_path / cached_file_p->get_file_name();
      if (cached_file_p->save(new_file_path))
        {
        source_control_checkout_or_add(new_file_path);
        }
      else
        {
        report_error(FString::Printf(TEXT("Couldn't save class file '%s'"), *new_file_path));
        }

      anything_changed = true;
      }
    else if (body != cached_file_p->m_body)
      {
      // Write file and update time stamp
      cached_file_p->m_body = body;
      FString new_file_path = m_overlay_path / cached_file_p->get_file_name();
      if (cached_file_p->save(new_file_path))
        {
        source_control_checkout_or_add(new_file_path);
        }
      else
        {
        report_error(FString::Printf(TEXT("Couldn't save class file '%s'"), *new_file_path));
        }

      anything_changed = true;
      }

    if (anything_changed)
      {
      ISkookumScriptRuntimeInterface::eChangeType change_type = is_existing ? ISkookumScriptRuntimeInterface::ChangeType_modified : ISkookumScriptRuntimeInterface::ChangeType_created;
      m_runtime_interface_p->on_class_scripts_changed_by_generator(sk_class_name, change_type);
      }
    }
#if !PLATFORM_EXCEPTIONS_DISABLED
  catch (const TCHAR * error_msg_p)
    {
    checkf(false, TEXT("%s"), error_msg_p);
    }
#endif
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::update_used_class_script_files(bool allow_members)
{
  // Loop through all previously used classes and create stubs for them
  tUsedClasses used_classes_copy;
  while (m_used_classes.Num() > 0)
    {
    used_classes_copy = MoveTemp(m_used_classes);
    for (tUsedClasses::TConstIterator iter(used_classes_copy); iter; ++iter)
      {
      update_class_script_file(*iter, true, allow_members);
      }
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::create_root_class_script_file(const TCHAR * sk_class_name_p)
  {
  // Get or create cached file entry
  CachedClassFile * cached_file_p = get_or_create_cached_class_file(ASymbol::create(AString(sk_class_name_p)));
  bool is_existing = (cached_file_p->m_file_time_stamp.GetTicks() > 0u);
  bool anything_changed = false;
  FString body = FString::Printf(TEXT("// %s\n$$ .\n"), sk_class_name_p);
  if (body != cached_file_p->m_body)
    {
    // Write file and update time stamp
    cached_file_p->m_body = body;
    cached_file_p->m_sk_super_name = ASymbol::create("Object"); // Don't use ASymbol_Object here as it might not be initialized yet
    FString new_file_path = m_overlay_path / cached_file_p->get_file_name();
    if (cached_file_p->save(new_file_path))
      {
      source_control_checkout_or_add(new_file_path);
      }
    else
      {
      report_error(FString::Printf(TEXT("Couldn't save class file '%s'"), *new_file_path));
      }

    anything_changed = true;
    }

  if (anything_changed)
    {
    ISkookumScriptRuntimeInterface::eChangeType change_type = is_existing ? ISkookumScriptRuntimeInterface::ChangeType_modified : ISkookumScriptRuntimeInterface::ChangeType_created;
    m_runtime_interface_p->on_class_scripts_changed_by_generator(sk_class_name_p, change_type);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::rename_class_script_file(UObject * type_p, const FString & old_ue_class_name)
  {
  // Determine class and superclass names
  FString new_sk_class_name = get_skookum_class_name(type_p);
  FName old_ue_name = FName(*old_ue_class_name);
  FName package_name = type_p->GetOutermost()->GetFName(); // Assume it stays in the same package
  FString old_sk_class_name = type_p->IsA<UEnum>() ? skookify_enum_name(old_ue_name, package_name) : skookify_class_name(old_ue_name, package_name);

  ASymbol old_sk_name = ASymbol::create(FStringToAString(old_sk_class_name));

  // 1) Find cached class and update it
  uint32_t idx;
  const CachedClassFile * cached_file_p = m_class_files.get(old_sk_name, AMatch_first_found, &idx);
  if (cached_file_p)
    {
    // Rename the file
    const FString old_file_name = cached_file_p->get_file_name();
    const FString new_file_name = old_file_name.Replace(*(old_sk_class_name + TEXT(".")), *(new_sk_class_name + TEXT(".")));
    move_script_file(old_file_name, new_file_name);
    
    // And forget its cached entry
    m_class_files.remove(idx);

    // Even if the filenames were the same above and a file move did not occur, we still need to update the script file 
    // since it is likely that the package path has changed. So regenerate the script file if possible.
    if (UField * field_p = get_field(type_p))
      {
      update_class_script_file(field_p, true, true);
      }
    }

  // 2) Find all cached classes that have this class as a superclass and update them
  for (const CachedClassFile & cached_file : m_class_files)
    {
    if (cached_file.m_sk_super_name == old_sk_name)
      {
      // Rename the file - no need to regenerate
      const FString old_file_name = cached_file_p->get_file_name();
      const FString new_file_name = old_file_name.Replace(*(TEXT(".") + old_sk_class_name), *(TEXT(".") + new_sk_class_name));
      move_script_file(old_file_name, new_file_name);
      }
    }

  // Inform the runtime module of the change
  m_runtime_interface_p->on_class_scripts_changed_by_generator(old_sk_class_name, ISkookumScriptRuntimeInterface::ChangeType_deleted);
  m_runtime_interface_p->on_class_scripts_changed_by_generator(new_sk_class_name, ISkookumScriptRuntimeInterface::ChangeType_created);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::delete_class_script_file(UObject * type_p)
  {
  // Determine sk class name
  FString sk_class_name = get_skookum_class_name(type_p);

  // Get cached file entry - assume file does not exist if not found
  uint32_t idx;
  CachedClassFile * cached_file_p = m_class_files.get(ASymbol::create(FStringToAString(sk_class_name)), AMatch_first_found, &idx);
  if (cached_file_p)
    {
    // Delete the file
    FString class_file_path = m_overlay_path / cached_file_p->get_file_name();
    if (source_control_delete(class_file_path))
      {
      m_runtime_interface_p->on_class_scripts_changed_by_generator(sk_class_name, ISkookumScriptRuntimeInterface::ChangeType_deleted);
      m_class_files.remove(idx);
      }
    else
      {
      report_error(FString::Printf(TEXT("Couldn't delete class file '%s'"), *class_file_path));
      }
    }
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::move_script_file(const FString & old_file_name, const FString & new_file_name)
  {
  const FString old_file_path = m_overlay_path / old_file_name;
  const FString new_file_path = m_overlay_path / new_file_name;

  // We are about to rename a file in Project-Generated-BP. When moving blueprints around to different folders
  // the filename in Project-Generated-BP remains the same. Let's only proceed with the move operation if
  // source and destination paths are different.
  const bool is_new_path = old_file_path.Compare(new_file_path, ESearchCase::IgnoreCase) != 0;

  if (is_new_path)
    {
    if (IFileManager::Get().Move(*new_file_path, *old_file_path, true, true))
      {
      if (new_file_name != old_file_name)
        {
        source_control_delete(old_file_path);
        }
      source_control_checkout_or_add(new_file_path);
      return true;
      }
    else
      {
      report_error(FString::Printf(TEXT("Couldn't rename script file from '%s' to '%s'"), *old_file_name, *new_file_name));
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntimeGenerator::initialize_paths()
  {
  // Look for default SkookumScript project file in engine folder.
  FString plugin_root_path(IPluginManager::Get().FindPlugin(TEXT("SkookumScript"))->GetBaseDir());
  FString default_project_file_path(plugin_root_path / TEXT("Scripts/Skookum-project-default.ini"));
  checkf(FPaths::FileExists(default_project_file_path), TEXT("Cannot find default project settings file '%s'!"), *default_project_file_path);
  m_default_project_file_path = FPaths::ConvertRelativePathToFull(default_project_file_path);
  m_project_file_path.Empty();

  // Look for specific SkookumScript project in game/project folder.
  FString project_file_path;
  if (have_project())
    {
    project_file_path = get_or_create_project_file(FPaths::GameDir(), FApp::GetGameName(), &m_project_mode);
    }
  if (!project_file_path.IsEmpty())
    {
    // Qualify and store for later reference
    m_project_file_path = FPaths::ConvertRelativePathToFull(project_file_path);
    // Set overlay path and depth
    set_overlay_path();
    }
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::initialize_generation_targets()
  {
  GenerationTargetBase::eState target_state = m_targets[ClassScope_engine].initialize(IPluginManager::Get().FindPlugin(TEXT("SkookumScript"))->GetBaseDir(), TEXT("UE4"));
  m_current_target_p = &m_targets[ClassScope_engine];
  if (have_project())
    {
    GenerationTargetBase::eState project_target_state = m_targets[ClassScope_project].initialize(FPaths::GameDir(), FApp::GetGameName(), &m_targets[ClassScope_engine]);
    m_current_target_p = &m_targets[ClassScope_project];
    if (project_target_state > target_state)
      {
      target_state = project_target_state;
      }
    }

  return target_state == GenerationTargetBase::State_valid_changed;
  }

//---------------------------------------------------------------------------------------
// Set overlay path and depth
void FSkookumScriptRuntimeGenerator::set_overlay_path()
  {
  SK_ASSERTX(!m_project_file_path.IsEmpty(), "Project file path not set!");
  FString scripts_path = FPaths::GetPath(m_project_file_path);
  const TCHAR * overlay_name_bp_p = ms_overlay_name_bp_p;
  m_overlay_path = FPaths::ConvertRelativePathToFull(scripts_path / overlay_name_bp_p);
  if (!FPaths::DirectoryExists(m_overlay_path))
    {
    overlay_name_bp_p = ms_overlay_name_bp_old_p;
    m_overlay_path = FPaths::ConvertRelativePathToFull(scripts_path / overlay_name_bp_p);
    }
  compute_scripts_path_depth(m_project_file_path, overlay_name_bp_p);
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::can_export_blueprint_function(UFunction * function_p)
  {
  static FName s_user_construction_script_name(TEXT("UserConstructionScript"));

  // Only consider non-native (=script) functions that can be called from Blueprint event graphs
  if (function_p->GetFName() == s_user_construction_script_name
   || function_p->HasAnyFunctionFlags(FUNC_Native|FUNC_Private|FUNC_Protected|FUNC_Delegate)
   || !function_p->HasAllFunctionFlags(FUNC_BlueprintCallable|FUNC_BlueprintEvent))
    {
    return false;
    }

  // Check that all parameters can be exported
  for (TFieldIterator<UProperty> param_it(function_p); param_it; ++param_it)
    {
    UProperty * param_p = *param_it;
    if (param_p->HasAllPropertyFlags(CPF_Parm)
     && (!SkUEReflectionManager::can_ue_property_be_reflected(param_p)
      || !can_export_property(param_p, 0, 0)))
      {
      return false;
      }
    }

  // Passed all tests, so can export!
  return true;
  }

//---------------------------------------------------------------------------------------

FSkookumScriptRuntimeGenerator::CachedClassFile * FSkookumScriptRuntimeGenerator::get_or_create_cached_class_file(ASymbol sk_class_name)
  {
  // Get or create cached file entry for this class
  CachedClassFile * cached_file_p = m_class_files.get(sk_class_name);
  if (!cached_file_p)
    {
    uint32_t pos;
    m_class_files.append(CachedClassFile(sk_class_name), &pos);
    cached_file_p = &m_class_files[pos];
    }
  return cached_file_p;
  }

//---------------------------------------------------------------------------------------

FSkookumScriptRuntimeGenerator::CachedClassFile::CachedClassFile(const FString & class_file_path, bool load_body)
  : m_was_synced(false)
  {
  // Figure out sk class name and superclass name
  FString base_file_name = FPaths::GetBaseFilename(class_file_path); // Just file name, without extension
  int32 split_pos;
  if (base_file_name.FindChar('.', split_pos))
    {
    m_sk_super_name = ASymbol::create(AString(*base_file_name + split_pos + 1, base_file_name.Len() - split_pos - 1));
    }
  else
    {
    split_pos = base_file_name.Len();
    }
  set_name(ASymbol::create(AString(*base_file_name, split_pos)));

  // Load body and set time stamp
  if (load_body)
    {
    load(class_file_path);
    }
  }

//---------------------------------------------------------------------------------------

FSkookumScriptRuntimeGenerator::CachedClassFile::CachedClassFile(ASymbol sk_class_name) 
  : ANamed(sk_class_name)
  , m_file_time_stamp(0)
  , m_was_synced(false)
  {

  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptRuntimeGenerator::CachedClassFile::get_file_name() const
  {
  SK_ASSERTX(!m_sk_super_name.is_null(), a_str_format("m_sk_super_name of %s is null!", get_name().as_cstr()));

  if (m_sk_super_name.is_null())
    {
    return FString(get_name().as_cstr()) + TEXT(".sk");
    }
  return FString(get_name().as_cstr()) + TEXT(".") + FString(m_sk_super_name.as_cstr()) + TEXT(".sk");
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::CachedClassFile::load(const FString & class_file_path, const FDateTime * file_time_stamp_if_known_p)
  {
  // Load the file body
  if (!FFileHelper::LoadFileToString(m_body, *class_file_path))
    {
    return false;
    }

  // Remove any CRs from the loaded file
  m_body.ReplaceInline(TEXT("\r"), TEXT(""), ESearchCase::CaseSensitive);

  // Set time stamp
  if (file_time_stamp_if_known_p)
    {
    m_file_time_stamp = *file_time_stamp_if_known_p;
    }
  else
    {
    m_file_time_stamp = IFileManager::Get().GetTimeStamp(*class_file_path);
    }

  return true;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntimeGenerator::CachedClassFile::save(const FString & class_file_path)
  {
  // On Windows, insert CRs before LFs
  #if PLATFORM_WINDOWS
    FString platform_body = m_body.Replace(TEXT("\n"), TEXT("\r\n"));
  #else
    const FString & platform_body = m_body;
  #endif

  // Save the file body
  if (!FFileHelper::SaveStringToFile(platform_body, *class_file_path, ms_script_file_encoding, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly))
    {
    return false;
    }

  // Set time stamp
  m_file_time_stamp = IFileManager::Get().GetTimeStamp(*class_file_path);

  return true;
  }

#endif // WITH_EDITORONLY_DATA
