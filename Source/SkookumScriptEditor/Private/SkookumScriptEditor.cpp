// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#include "ISkookumScriptEditor.h"
#include "Slate/SlateGameResources.h" 
#include "Interfaces/IMainFrameModule.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Kismet2/EnumEditorUtils.h"
#include "ISkookumScriptRuntime.h"
#include "AssetRegistryModule.h"
#include "BlueprintActionDatabase.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/UserDefinedEnum.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"

#include "Modules/ModuleManager.h" // For IMPLEMENT_MODULE

#include "GraphEditor.h"

#include "../../SkookumScriptGenerator/Private/SkookumScriptGeneratorBase.inl"

DEFINE_LOG_CATEGORY_STATIC(LogSkookumScriptEditor, Log, All);

//---------------------------------------------------------------------------------------

class FSkookumScriptEditor 
  : public ISkookumScriptEditor
  , public ISkookumScriptRuntimeEditorInterface
  , public FStructureEditorUtils::FStructEditorManager::ListenerType
  , public FEnumEditorUtils::FEnumEditorManager::ListenerType
{
public:

protected:

  //---------------------------------------------------------------------------------------
  // IModuleInterface implementation

  virtual void  StartupModule() override;
  virtual void  ShutdownModule() override;

  //---------------------------------------------------------------------------------------
  // ISkookumScriptRuntimeEditorInterface implementation

  virtual void  on_class_updated(UClass * ue_class_p) override;
  virtual void  on_function_updated(UFunction * ue_function_p, bool is_event) override;
  virtual void  on_function_removed_from_class(UClass * ue_class_p) override;

  //---------------------------------------------------------------------------------------
  // FStructureEditorManager::ListenerType implementation

  virtual void PreChange(const UUserDefinedStruct * struct_p, FStructureEditorUtils::EStructureEditorChangeInfo change_type) override;
  virtual void PostChange(const UUserDefinedStruct * struct_p, FStructureEditorUtils::EStructureEditorChangeInfo change_type) override;

  //---------------------------------------------------------------------------------------
  // FEnumEditorManager::ListenerType implementation

  virtual void PreChange(const UUserDefinedEnum * enum_p, FEnumEditorUtils::EEnumEditorChangeInfo change_type) override;
  virtual void PostChange(const UUserDefinedEnum * enum_p, FEnumEditorUtils::EEnumEditorChangeInfo change_type) override;

  //---------------------------------------------------------------------------------------
  // Local implementation

  void  on_main_frame_loaded(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow);
  void  on_object_modified(UObject * obj_p);
  void  on_new_asset_created(UFactory * factory_p);
  void  on_assets_deleted(const TArray<UClass*> & deleted_asset_classes);
  void  on_asset_post_import(UFactory * factory_p, UObject * obj_p);
  void  on_asset_added(const FAssetData & asset_data);
  void  on_asset_renamed(const FAssetData & asset_data, const FString & old_object_path);
  void  on_in_memory_asset_created(UObject * obj_p);
  void  on_in_memory_asset_deleted(UObject * obj_p);
  void  on_blueprint_compiled(UBlueprint * blueprint_p);
  void  on_map_opened(const FString & file_name, bool as_template);

  // Data members

  ISkookumScriptRuntime *       m_runtime_p;
  bool                          m_is_skookum_disabled;

  FDelegateHandle               m_on_object_modified_handle;
  FDelegateHandle               m_on_map_opened_handle;
  FDelegateHandle               m_on_new_asset_created_handle;
  FDelegateHandle               m_on_assets_deleted_handle;
  FDelegateHandle               m_on_asset_post_import_handle;
  FDelegateHandle               m_on_asset_added_handle;
  FDelegateHandle               m_on_asset_renamed_handle;
  FDelegateHandle               m_on_in_memory_asset_created_handle;
  FDelegateHandle               m_on_in_memory_asset_deleted_handle;

  };

IMPLEMENT_MODULE(FSkookumScriptEditor, SkookumScriptEditor)

//=======================================================================================
// IModuleInterface implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::StartupModule()
  {
  // Get pointer to runtime module
  m_runtime_p = static_cast<ISkookumScriptRuntime *>(FModuleManager::Get().GetModule("SkookumScriptRuntime"));

  // Is SkookumScript active?
  m_is_skookum_disabled = m_runtime_p->is_skookum_disabled();

  // Don't do anything if SkookumScript is not active
  if (m_is_skookum_disabled)
    {
    return;
    }

  // Tell runtime that editor is present (needed even in commandlet mode as we might have to demand-load blueprints)
  m_runtime_p->set_editor_interface(this);

  if (!IsRunningCommandlet())
    {
    // Hook up delegates
    //m_on_asset_loaded_handle          = FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &FSkookumScriptEditor::on_asset_loaded); // Handled in SkookumScriptRuntime module
    m_on_object_modified_handle       = FCoreUObjectDelegates::OnObjectModified.AddRaw(this, &FSkookumScriptEditor::on_object_modified);
    m_on_map_opened_handle            = FEditorDelegates::OnMapOpened.AddRaw(this, &FSkookumScriptEditor::on_map_opened);
    m_on_new_asset_created_handle     = FEditorDelegates::OnNewAssetCreated.AddRaw(this, &FSkookumScriptEditor::on_new_asset_created);
    m_on_assets_deleted_handle        = FEditorDelegates::OnAssetsDeleted.AddRaw(this, &FSkookumScriptEditor::on_assets_deleted);

    FAssetRegistryModule & asset_registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
    m_on_asset_added_handle             = asset_registry.Get().OnAssetAdded().AddRaw(this, &FSkookumScriptEditor::on_asset_added);
    m_on_asset_renamed_handle           = asset_registry.Get().OnAssetRenamed().AddRaw(this, &FSkookumScriptEditor::on_asset_renamed);
    m_on_in_memory_asset_created_handle = asset_registry.Get().OnInMemoryAssetCreated().AddRaw(this, &FSkookumScriptEditor::on_in_memory_asset_created);
    m_on_in_memory_asset_deleted_handle = asset_registry.Get().OnInMemoryAssetDeleted().AddRaw(this, &FSkookumScriptEditor::on_in_memory_asset_deleted);

	  // When loading a new engine version for the first time, the instrumentation of existing blueprints/structs/enums will call through to the source control
	  // provider code to delete or add files. Unfortunately doing so causes a UI status bar to show up, this is a problem when the engine isn't up yet and will
	  // results in a crash. So we delay this final instrumentation until after the main editor frame creation is done.
	  IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	  MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FSkookumScriptEditor::on_main_frame_loaded);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::ShutdownModule()
  {
  // Don't do anything if SkookumScript is not active
  if (m_is_skookum_disabled)
    {
    return;
    }

  m_runtime_p = nullptr;

  if (!IsRunningCommandlet())
    {
    // Remove delegates
    //FCoreUObjectDelegates::OnAssetLoaded.Remove(m_on_asset_loaded_handle); // Handled in SkookumScriptRuntime module
    FCoreUObjectDelegates::OnObjectModified.Remove(m_on_object_modified_handle);
    FEditorDelegates::OnMapOpened.Remove(m_on_map_opened_handle);
    FEditorDelegates::OnNewAssetCreated.Remove(m_on_new_asset_created_handle);
    FEditorDelegates::OnAssetsDeleted.Remove(m_on_assets_deleted_handle);

    if (GEditor)
      {
      GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.Remove(m_on_asset_post_import_handle);
      }
    

    FAssetRegistryModule * asset_registry_p = FModuleManager::GetModulePtr<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
    if (asset_registry_p)
      {
      asset_registry_p->Get().OnAssetAdded().Remove(m_on_asset_added_handle);
      asset_registry_p->Get().OnAssetRenamed().Remove(m_on_asset_renamed_handle);
      asset_registry_p->Get().OnInMemoryAssetCreated().Remove(m_on_in_memory_asset_created_handle);
      asset_registry_p->Get().OnInMemoryAssetDeleted().Remove(m_on_in_memory_asset_deleted_handle);
      }
    }
  }

//=======================================================================================
// ISkookumScriptRuntimeEditorInterface implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::on_class_updated(UClass * ue_class_p)
  {
  // Nothing to do if no engine
  if (!GEngine) return;

  // 1) Refresh actions (in Blueprint editor drop down menu)
  FBlueprintActionDatabase::Get().RefreshClassActions(ue_class_p);

  // Remember affected Blueprints here
  TArray<UBlueprint *> affected_blueprints;

  // Temporarily suspend the undo buffer - we don't need to remember the reconstructed nodes
  ITransaction * undo_p = GUndo;
  GUndo = nullptr;
  // Pretend we are loading a package so that node reconstruction does not cause the Blueprint to be marked dirty
  bool is_loading = GIsEditorLoadingPackage;
  GIsEditorLoadingPackage = true;

  // 2) Refresh node display of all SkookumScript function call nodes
  for (TObjectIterator<UK2Node_CallFunction> call_it; call_it; ++call_it)
    {
    UFunction * target_function_p = call_it->GetTargetFunction();
    // Also refresh all nodes with no target function as it is probably a Sk function that was deleted
    //if (!target_function_p || m_runtime_p->is_skookum_reflected_call(target_function_p))
    if (target_function_p 
     && m_runtime_p->is_skookum_reflected_call(target_function_p)
     && ue_class_p->IsChildOf(target_function_p->GetOwnerClass()))
      {
      call_it->ReconstructNode();
      affected_blueprints.AddUnique(FBlueprintEditorUtils::FindBlueprintForNode(*call_it));
      }
    }

  // 3) Refresh node display of all SkookumScript event nodes
  for (TObjectIterator<UK2Node_Event> event_it; event_it; ++event_it)
    {
    UFunction * event_function_p = event_it->FindEventSignatureFunction();
    if (event_function_p 
     && m_runtime_p->is_skookum_reflected_event(event_function_p)
     && ue_class_p->IsChildOf(event_function_p->GetOwnerClass()))
      {
      event_it->ReconstructNode();
      affected_blueprints.AddUnique(FBlueprintEditorUtils::FindBlueprintForNode(*event_it));
      }
    }

  // 4) Try recompiling any affected Blueprint that previously had errors
  for (UBlueprint * blueprint_p : affected_blueprints)
    {
    if (blueprint_p && blueprint_p->Status == BS_Error)
      {
      FKismetEditorUtilities::CompileBlueprint(blueprint_p);
      }
    }

  // Restore the undo buffer
  GUndo = undo_p;
  // Restore the loading flag
  GIsEditorLoadingPackage = is_loading;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::on_function_updated(UFunction * ue_function_p, bool is_event)
  {
  // Nothing to do if no engine
  if (!GEngine) return;

  // 1) Refresh actions (in Blueprint editor drop down menu)
  FBlueprintActionDatabase::Get().RefreshClassActions(ue_function_p->GetOwnerClass());

  // Remember affected Blueprints here
  TArray<UBlueprint *> affected_blueprints;

  // Temporarily suspend the undo buffer - we don't need to remember the reconstructed nodes
  ITransaction * undo_p = GUndo;
  GUndo = nullptr;
  // Pretend we are loading a package so that node reconstruction does not cause the Blueprint to be marked dirty
  bool is_loading = GIsEditorLoadingPackage;
  GIsEditorLoadingPackage = true;

  // Lambda to run on an event node or call function node
  auto check_node = [ue_function_p, &affected_blueprints](UK2Node * node_p, const FMemberReference & function_ref)
    {
    if (function_ref.GetMemberName() == ue_function_p->GetFName())
      {
      UBlueprint * blueprint_p = FBlueprintEditorUtils::FindBlueprintForNode(node_p);
      UClass * function_class_p = function_ref.GetMemberParentClass();
      bool is_owner_matching;
      if (function_class_p)
        {
        is_owner_matching = ue_function_p->GetOwnerClass()->IsChildOf(function_class_p);
        }
      else
        {
        function_class_p = blueprint_p->GeneratedClass;
        is_owner_matching = function_class_p ? function_class_p->IsChildOf(ue_function_p->GetOwnerClass()) : false;
        }
      if (is_owner_matching)
        {
        node_p->ReconstructNode();
        affected_blueprints.AddUnique(blueprint_p);
        }
      }
    };

  // 2) Refresh node display of all nodes using this function
  if (is_event)
    {
    for (TObjectIterator<UK2Node_Event> event_it; event_it; ++event_it)
      {
      check_node(*event_it, event_it->EventReference);
      }
    }
  else
    {
	  for (TObjectIterator<UK2Node_CallFunction> call_it; call_it; ++call_it)
      {
      check_node(*call_it, call_it->FunctionReference);
      }
    }

  // 3) Try recompiling any affected Blueprint that previously had errors
  if (!IsLoading()) // Cannot compile while loading
    {
    for (UBlueprint * blueprint_p : affected_blueprints)
      {
      if (blueprint_p && blueprint_p->Status == BS_Error)
        {
        FKismetEditorUtilities::CompileBlueprint(blueprint_p);
        }
      }
    }

  // Restore the undo buffer
  GUndo = undo_p;
  // Restore the loading flag
  GIsEditorLoadingPackage = is_loading;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::on_function_removed_from_class(UClass * ue_class_p)
  {
  // Refresh actions (in Blueprint editor drop down menu)
  FBlueprintActionDatabase::Get().RefreshClassActions(ue_class_p);
  }

//=======================================================================================
// FStructureEditorManager::ListenerType implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::PreChange(const UUserDefinedStruct * struct_p, FStructureEditorUtils::EStructureEditorChangeInfo change_type)
  {
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::PostChange(const UUserDefinedStruct * struct_p, FStructureEditorUtils::EStructureEditorChangeInfo change_type)
  {
  on_object_modified((UObject *)struct_p);
  }

//=======================================================================================
// FEnumEditorManager::ListenerType implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::PreChange(const UUserDefinedEnum * enum_p, FEnumEditorUtils::EEnumEditorChangeInfo change_type)
  {
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::PostChange(const UUserDefinedEnum * enum_p, FEnumEditorUtils::EEnumEditorChangeInfo change_type)
  {
  on_object_modified((UObject *)enum_p);
  }

//=======================================================================================
// FSkookumScriptEditor implementation
//=======================================================================================

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_main_frame_loaded(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow)
  {
  // GEditor not valid until main frame loaded
  m_on_asset_post_import_handle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.AddRaw(this, &FSkookumScriptEditor::on_asset_post_import);

  // Instrument all already existing blueprints
  for (TObjectIterator<UBlueprint> blueprint_it; blueprint_it; ++blueprint_it)
    {
    m_runtime_p->on_new_asset(*blueprint_it);
    }

  // Same for user defined structs
  for (TObjectIterator<UUserDefinedStruct> struct_it; struct_it; ++struct_it)
    {
    m_runtime_p->on_new_asset(*struct_it);
    }

  // Same for user defined enums
  for (TObjectIterator<UUserDefinedEnum> enum_it; enum_it; ++enum_it)
    {
    m_runtime_p->on_new_asset(*enum_it);
    }
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_object_modified(UObject * obj_p)
  {
  // Is this a blueprint?
  UBlueprint * blueprint_p = Cast<UBlueprint>(obj_p);
  if (blueprint_p)
    {    
    m_runtime_p->on_class_added_or_modified(blueprint_p);
    }

  // Is this a struct?
  UUserDefinedStruct * struct_p = Cast<UUserDefinedStruct>(obj_p);
  if (struct_p)
    {
    m_runtime_p->on_struct_added_or_modified(struct_p);
    }

  // Is this an enum?
  UUserDefinedEnum * enum_p = Cast<UUserDefinedEnum>(obj_p);
  if (enum_p)
    {
    m_runtime_p->on_enum_added_or_modified(enum_p);
    }
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_new_asset_created(UFactory * factory_p)
  {
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_assets_deleted(const TArray<UClass*> & deleted_asset_classes)
  {
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_asset_post_import(UFactory * factory_p, UObject * obj_p)
  {
  on_object_modified(obj_p);
  }

//---------------------------------------------------------------------------------------
// An asset is being added to the asset registry - during load, or otherwise
void FSkookumScriptEditor::on_asset_added(const FAssetData & asset_data)
  {
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_asset_renamed(const FAssetData & asset_data, const FString & old_object_path)
  {
  static FName s_blueprint_class_name(TEXT("Blueprint"));
  static FName s_struct_class_name(TEXT("UserDefinedStruct"));
  static FName s_enum_class_name(TEXT("UserDefinedEnum"));

  if (asset_data.AssetClass == s_blueprint_class_name)
    {
    UBlueprint * blueprint_p = FindObjectChecked<UBlueprint>(ANY_PACKAGE, *asset_data.AssetName.ToString());
    if (blueprint_p)
      {
      m_runtime_p->on_class_renamed(blueprint_p, FPaths::GetBaseFilename(old_object_path));
      }
    }
  else if (asset_data.AssetClass == s_struct_class_name)
    {
    UUserDefinedStruct * struct_p = FindObjectChecked<UUserDefinedStruct>(ANY_PACKAGE, *asset_data.AssetName.ToString());
    if (struct_p)
      {
      m_runtime_p->on_struct_renamed(struct_p, FPaths::GetBaseFilename(old_object_path));
      }
    }
  else if (asset_data.AssetClass == s_enum_class_name)
    {
    UUserDefinedEnum * enum_p = FindObjectChecked<UUserDefinedEnum>(ANY_PACKAGE, *asset_data.AssetName.ToString());
    if (enum_p)
      {
      m_runtime_p->on_enum_renamed(enum_p, FPaths::GetBaseFilename(old_object_path));
      }
    }
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_in_memory_asset_created(UObject * obj_p)
  {
  m_runtime_p->on_new_asset(obj_p);
  }

//---------------------------------------------------------------------------------------
//
void FSkookumScriptEditor::on_in_memory_asset_deleted(UObject * obj_p)
  {
  UBlueprint * blueprint_p = Cast<UBlueprint>(obj_p);
  if (blueprint_p)
    {
    m_runtime_p->on_class_deleted(blueprint_p);
    }

  UUserDefinedStruct * struct_p = Cast<UUserDefinedStruct>(obj_p);
  if (struct_p)
    {
    m_runtime_p->on_struct_deleted(struct_p);
    }

  UUserDefinedEnum * enum_p = Cast<UUserDefinedEnum>(obj_p);
  if (enum_p)
    {
    m_runtime_p->on_enum_deleted(enum_p);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptEditor::on_blueprint_compiled(UBlueprint * blueprint_p)
  {
  m_runtime_p->on_class_added_or_modified(blueprint_p);
  }

//---------------------------------------------------------------------------------------
// Called when the map is done loading (load progress reaches 100%)
void FSkookumScriptEditor::on_map_opened(const FString & file_name, bool as_template)
  {
  // Let runtime know we are done opening a new map
  m_runtime_p->on_editor_map_opened();
  }

