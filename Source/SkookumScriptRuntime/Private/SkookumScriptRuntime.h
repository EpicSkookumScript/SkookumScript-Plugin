#include "ISkookumScriptRuntime.h"
#include "SkookumScriptRuntimeGenerator.h"

#include "Bindings/SkUEBindings.hpp"
#include "Bindings/SkUEClassBinding.hpp"
#include "Bindings/SkUERuntime.hpp"
#include "Bindings/SkUERemote.hpp"
#include "Bindings/SkUEReflectionManager.hpp"
#include "Bindings/SkUESymbol.hpp"
#include "Bindings/SkUEUtils.hpp"
#include "Bindings/Engine/SkUEName.hpp"

#include "Runtime/Launch/Resources/Version.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"
#include "Stats/Stats.h"

#if WITH_EDITORONLY_DATA
#include "KismetCompiler.h"
#include "KismetCompilerModule.h"
#include "Engine/UserDefinedStruct.h"
#include "Engine/UserDefinedEnum.h"
#endif

#include <SkookumScript/SkSymbolDefs.hpp>

//---------------------------------------------------------------------------------------
// UE4 implementation of AAppInfoCore
class FAppInfo : public AAppInfoCore, public SkAppInfo
{
public:

  FAppInfo();
  virtual ~FAppInfo();

protected:

  // AAppInfoCore implementation

  virtual void *             malloc(size_t size, const char * debug_name_p) override;
  virtual void               free(void * mem_p) override;
  virtual uint32_t           request_byte_size(uint32_t size_requested) override;
  virtual bool               is_using_fixed_size_pools() override;
  virtual void               debug_print(const char * cstr_p) override;
  virtual AErrorOutputBase * on_error_pre(bool nested) override;
  virtual void               on_error_post(eAErrAction action) override;
  virtual void               on_error_quit() override;

  // SkAppInfo implementation

  virtual bool               use_builtin_actor() const override;
  virtual ASymbol            get_custom_actor_class_name() const override;
  virtual void               bind_name_construct(SkBindName * bind_name_p, const AString & value) const override;
  virtual void               bind_name_destruct(SkBindName * bind_name_p) const override;
  virtual void               bind_name_assign(SkBindName * bind_name_p, const AString & value) const override;
  virtual AString            bind_name_as_string(const SkBindName & bind_name) const override;
  virtual SkInstance *       bind_name_new_instance(const SkBindName & bind_name) const override;
  virtual SkClass *          bind_name_class() const override;

};

class FSkookumScriptRuntime : public ISkookumScriptRuntime
#if WITH_EDITORONLY_DATA
  , public ISkookumScriptRuntimeInterface
#endif
#if WITH_EDITOR
  , public IBlueprintCompiler
#endif
{
public:

  FSkookumScriptRuntime();
  ~FSkookumScriptRuntime();

#if WITH_EDITORONLY_DATA
  virtual FSkookumScriptRuntimeGenerator* get_runtime_generator();
#endif

protected:

  // Methods

    // Overridden from IModuleInterface

  virtual void  StartupModule() override;
  //virtual void  PostLoadCallback() override;
  //virtual void  PreUnloadCallback() override;
  virtual void  ShutdownModule() override;

  // Overridden from ISkookumScriptRuntime

  virtual void  set_project_generated_bindings(SkUEBindingsInterface * project_generated_bindings_p) override;
  virtual bool  is_skookum_disabled() const override;
  virtual bool  is_freshen_binaries_pending() const override;

#if WITH_EDITOR

  virtual void  set_editor_interface(ISkookumScriptRuntimeEditorInterface * editor_interface_p) override;

  virtual bool  is_connected_to_ide() const override;
  virtual void  on_application_focus_changed(bool is_active) override;
  virtual void  on_editor_map_opened() override;
  virtual void  show_ide(const FString & focus_ue_class_name, const FString & focus_ue_member_name, bool is_data_member, bool is_class_member) override;
  virtual void  freshen_compiled_binaries_if_have_errors() override;

  virtual bool  has_skookum_default_constructor(UClass * class_p) const override;
  virtual bool  has_skookum_destructor(UClass * class_p) const override;
  virtual bool  is_skookum_class_data_component_class(UClass * class_p) const override;
  virtual bool  is_skookum_behavior_component_class(UClass * class_p) const override;
  virtual bool  is_skookum_reflected_call(UFunction * function_p) const override;
  virtual bool  is_skookum_reflected_event(UFunction * function_p) const override;

  virtual void  on_class_added_or_modified(UBlueprint * blueprint_p) override;
  virtual void  on_class_renamed(UBlueprint * blueprint_p, const FString & old_ue_class_name) override;
  virtual void  on_class_deleted(UBlueprint * blueprint_p) override;

  virtual void  on_struct_added_or_modified(UUserDefinedStruct * ue_struct_p) override;
  virtual void  on_struct_renamed(UUserDefinedStruct * ue_struct_p, const FString & old_ue_struct_name) override;
  virtual void  on_struct_deleted(UUserDefinedStruct * ue_struct_p) override;

  virtual void  on_enum_added_or_modified(UUserDefinedEnum * ue_enum_p) override;
  virtual void  on_enum_renamed(UUserDefinedEnum * ue_enum_p, const FString & old_ue_enum_name) override;
  virtual void  on_enum_deleted(UUserDefinedEnum * ue_enum_p) override;

  // UObject Callbacks
  virtual void  on_new_asset(UObject * obj_p) override;

#endif

#if WITH_EDITORONLY_DATA

  void OnPreCompile();
  void OnPostCompile();

  // This callback is installed for each blueprint to be called when they are compiled
  void on_blueprint_compiled(UBlueprint * blueprint_p);

  // Overridden from IBlueprintCompiler
  virtual bool  CanCompile(const UBlueprint* Blueprint) override { return false; }
  virtual void  Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results) override {}
  virtual void  PreCompile(UBlueprint* Blueprint) override;
  virtual void  PostCompile(UBlueprint* Blueprint) override;


  // Overridden from ISkookumScriptRuntimeGeneratorInterface

  virtual bool  is_static_class_known_to_skookum(UClass * class_p) const override;
  virtual bool  is_static_struct_known_to_skookum(UStruct * struct_p) const override;
  virtual bool  is_static_enum_known_to_skookum(UEnum * enum_p) const override;

  virtual void  on_class_scripts_changed_by_generator(const FString & class_name, eChangeType change_type) override;

#endif

  // Local methods

  void            load_ini_settings();
  void            save_ini_settings();
  const FString & get_ini_file_path() const;

  eSkProjectMode  get_project_mode() const;
  bool            is_dormant() const;
  bool            allow_auto_connect_to_ide() const;
  bool            is_skookum_initialized() const;

  void            ensure_runtime_initialized();
  void            compile_and_load_binaries();

  void            tick_game(float deltaTime);
  void            tick_editor(float deltaTime);

#ifdef SKOOKUM_REMOTE_UNREAL
  void          tick_remote();
#endif

  void            on_world_init_pre(UWorld * world_p, const UWorld::InitializationValues init_vals);
  void            on_world_init_post(UWorld * world_p, const UWorld::InitializationValues init_vals);
  void            on_world_cleanup(UWorld * world_p, bool session_ended_b, bool cleanup_resources_b);

  // Data Members

  bool                    m_is_skookum_disabled;

  FAppInfo                m_app_info;

  mutable SkUERuntime     m_runtime;

#if WITH_EDITORONLY_DATA
  FSkookumScriptRuntimeGenerator  m_generator;
#endif

#ifdef SKOOKUM_REMOTE_UNREAL
  SkUERemote            m_remote_client;
  bool                  m_freshen_binaries_requested;
  bool                  m_allow_remote_ide;
#endif

  UWorld *                m_game_world_p;
  UWorld *                m_editor_world_p;

  uint32                  m_num_game_worlds;

  FDelegateHandle         m_on_world_init_pre_handle;
  FDelegateHandle         m_on_world_init_post_handle;
  FDelegateHandle         m_on_world_cleanup_handle;

#if WITH_EDITORONLY_DATA
  FDelegateHandle       m_on_pre_compile_handle;
  FDelegateHandle       m_on_post_compile_handle;
  FDelegateHandle       m_on_asset_loaded_handle;
#endif

  FDelegateHandle                 m_game_tick_handle;
  TMap<UWorld *, FDelegateHandle> m_editor_tick_handles;

  // Settings

  static TCHAR const * const ms_ini_section_name_p;
  static TCHAR const * const ms_ini_key_last_connected_to_ide_p;

};