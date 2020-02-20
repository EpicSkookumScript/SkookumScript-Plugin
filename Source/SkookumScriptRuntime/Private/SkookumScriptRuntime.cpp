// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Main entry point for the SkookumScript runtime plugin
//=======================================================================================

#include "SkookumScriptRuntime.h"

#include "SkookumScriptBehaviorComponent.h"
#include "SkookumScriptClassDataComponent.h"
#include "SkookumScriptMindComponent.h"
#include "SkookumScriptInstanceProperty.h"

#include "Modules/ModuleManager.h" // For IMPLEMENT_MODULE

#if WITH_EDITOR
#include "Misc/OutputDeviceConsole.h"
#endif

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

#include <AgogCore/AMethodArg.hpp>

// For profiling SkookumScript performance
DECLARE_CYCLE_STAT(TEXT("SkookumScript Time"), STAT_SkookumScriptTime, STATGROUP_Game);

//---------------------------------------------------------------------------------------

TCHAR const * const FSkookumScriptRuntime::ms_ini_section_name_p = TEXT("SkookumScriptRuntime");
TCHAR const * const FSkookumScriptRuntime::ms_ini_key_last_connected_to_ide_p = TEXT("LastConnectedToIDE");

//---------------------------------------------------------------------------------------
// Simple error dialog until more sophisticated one in place.
// Could communicate remotely with SkookumIDE and have it bring up message window.
class ASimpleErrorOutput : public AErrorOutputBase
  {
    public:

    virtual bool determine_choice(const AErrMsg & info, eAErrAction * action_p) override;

  };

//---------------------------------------------------------------------------------------
// Determines which error choice to take by prompting the user.  It also writes out
// information to the default output window(s).
// 
// # Returns:  true if a user break should be made in the debugger, false if not
// 
// # Params:
//   msg:      See the definition of `AErrMsg` in ADebug.hpp for more information.
//   action_p: address to store chosen course of action to take to resolve error
//   
// # Author(s): Conan Reis
bool ASimpleErrorOutput::determine_choice(
  const AErrMsg & msg,
  eAErrAction *   action_p
  )
  {
  const char * title_p;
  const char * choice_p    = NULL;
  bool         dbg_present = FPlatformMisc::IsDebuggerPresent();

  // Set pop-up attributes and default values
  switch (msg.m_err_level)
    {
    case AErrLevel_internal:
      title_p = (msg.m_title_p ? msg.m_title_p : "Internal recoverable exception");
      break;

    default:
      title_p  = (msg.m_title_p ? msg.m_title_p : "Error");
      choice_p =
        "\nChoose:\n"
        "  'Abort'  - break into C++ & get callstack [then ignore on continue]\n"
        "  'Retry'  - attempt recovery/ignore [still tests this assert in future]\n"
        "  'Ignore' - recover/ignore always [auto-ignores this assert in future]";
    }

  // Format description
  char         desc_p[2048];
  AString      desc(desc_p, 2048, 0u, false);
  const char * high_desc_p = msg.m_desc_high_p ? msg.m_desc_high_p : "An error has occurred.";
  const char * low_desc_p  = msg.m_desc_low_p  ? msg.m_desc_low_p  : "";
  const char * func_desc_p = msg.m_func_name_p ? msg.m_func_name_p : "";

  desc.insert(high_desc_p);
  desc.append(ADebug::context_string());

  // Ensure there is some space
  desc.ensure_size_extra(512u);

  if (msg.m_source_path_p)
    {
    desc.append_format("\n\n  C++ Internal Info:\n    %s\n    %s(%u) :\n    %s\n", func_desc_p, msg.m_source_path_p, msg.m_source_line, low_desc_p, msg.m_err_id);
    }
  else
    {
    desc.append_format("\n\n  C++ Internal Info:\n    %s\n    %s\n", func_desc_p, low_desc_p, msg.m_err_id);
    }

  // Print out to debug system first
  ADebug::print_format("\n###%s : ", title_p);
  ADebug::print(desc);

  desc.append(choice_p);

  // Prompt user (if necessary)
  eAErrAction action     = AErrAction_ignore;
  bool        user_break = false;

  if (choice_p)
    {
    #if defined(A_PLAT_PC)

      int result = ::MessageBoxA(
        NULL, desc, title_p, MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE | MB_DEFBUTTON1 | MB_SETFOREGROUND | MB_APPLMODAL);

      switch (result)
        {
        case IDABORT:    // Abort button was selected.
          user_break = true;
          action     = AErrAction_ignore;
          break;

        case IDRETRY:    // Retry button was selected.
          action = AErrAction_ignore;
          break;

        case IDIGNORE:   // Ignore button was selected.
          action = AErrAction_ignore_all;
          break;
        }
    #else
      user_break = dbg_present;
    #endif
    }

  *action_p = action;

  return user_break;
  }


//=======================================================================================
// Global Function Definitions
//=======================================================================================

IMPLEMENT_MODULE(FSkookumScriptRuntime, SkookumScriptRuntime)
DEFINE_LOG_CATEGORY(LogSkookum);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FAppInterface implementation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

FAppInfo::FAppInfo()
  {
  AgogCore::initialize(this);
  SkookumScript::set_app_info(this);
  SkUESymbol::initialize();
  }

//---------------------------------------------------------------------------------------

FAppInfo::~FAppInfo()
  {
  SkUESymbol::deinitialize();
  SkookumScript::set_app_info(nullptr);
  AgogCore::deinitialize();
  }

//---------------------------------------------------------------------------------------

void * FAppInfo::malloc(size_t size, const char * debug_name_p)
  {
  return size ? FMemory::Malloc(size, 16) : nullptr; // $Revisit - MBreyer Make alignment controllable by caller
  }

//---------------------------------------------------------------------------------------

void FAppInfo::free(void * mem_p)
  {
  if (mem_p) FMemory::Free(mem_p); // $Revisit - MBreyer Make alignment controllable by caller
  }

//---------------------------------------------------------------------------------------

uint32_t FAppInfo::request_byte_size(uint32_t size_requested)
  {
  // Since we call the 16-byte aligned allocator
  return a_align_up(size_requested, 16);
  }

//---------------------------------------------------------------------------------------

bool FAppInfo::is_using_fixed_size_pools()
  {
  return false;
  }

//---------------------------------------------------------------------------------------

void FAppInfo::debug_print(const char * cstr_p)
  {
  /*
  // Strip LF from start and end to prevent unnecessary gaps in log
  FString msg(cstr_p);
  msg.RemoveFromEnd(TEXT("\n"));
  msg.RemoveFromEnd(TEXT("\n"));
  msg.RemoveFromStart(TEXT("\n"));
  msg.RemoveFromStart(TEXT("\n"));
  UE_LOG(LogSkookum, Log, TEXT("%s"), *msg);
  */

#if WITH_EDITOR
  if (GLogConsole && IsRunningCommandlet())
    {
    FString message(cstr_p);
    GLogConsole->Serialize(*message, ELogVerbosity::Display, FName(TEXT("SkookumScript")));
    }
  else
#endif
    {
    ADebug::print_std(cstr_p);
    }
  }

//---------------------------------------------------------------------------------------

AErrorOutputBase * FAppInfo::on_error_pre(bool nested)
  {
  static ASimpleErrorOutput s_simple_err_out;
  return &s_simple_err_out;
  }

//---------------------------------------------------------------------------------------

void FAppInfo::on_error_post(eAErrAction action)
  {
  // Depending on action could switch back to fullscreen
  }

//---------------------------------------------------------------------------------------

void FAppInfo::on_error_quit()
  {
  exit(EXIT_FAILURE);
  }

//---------------------------------------------------------------------------------------

bool FAppInfo::use_builtin_actor() const
  {
  return false;
  }

//---------------------------------------------------------------------------------------

ASymbol FAppInfo::get_custom_actor_class_name() const
  {
  return ASymbol_Actor;
  }

//---------------------------------------------------------------------------------------

void FAppInfo::bind_name_construct(SkBindName * bind_name_p, const AString & value) const
  {
  static_assert(sizeof(FName) <= sizeof(SkBindName), "FName must fit into SkBindName.");
  new (bind_name_p) FName(value.as_cstr());
  }

//---------------------------------------------------------------------------------------

void FAppInfo::bind_name_destruct(SkBindName * bind_name_p) const
  {
  reinterpret_cast<FName *>(bind_name_p)->~FName();
  }

//---------------------------------------------------------------------------------------

void FAppInfo::bind_name_assign(SkBindName * bind_name_p, const AString & value) const
  {
  *reinterpret_cast<FName *>(bind_name_p) = FName(value.as_cstr());
  }

//---------------------------------------------------------------------------------------

AString FAppInfo::bind_name_as_string(const SkBindName & bind_name) const
  {
  const FName & name = reinterpret_cast<const FName &>(bind_name);
  ANSICHAR ansi_string[NAME_SIZE];
  name.GetPlainANSIString(ansi_string);
  const ANSICHAR * plain_string_p = ansi_string;

  // If no number, quickly make a string from the plain text
  if (name.GetNumber() == NAME_NO_NUMBER_INTERNAL)
    {
    // Make sure we allocate memory for the string
    return AString(plain_string_p, false);
    }

  // Has a number, append it separated by _
  AString bind_name_string;
  bind_name_string.ensure_size(FCStringAnsi::Strlen(plain_string_p) + 11);
  bind_name_string.append_format("%s_%d", plain_string_p, NAME_INTERNAL_TO_EXTERNAL(name.GetNumber()));
  return bind_name_string;
  }

//---------------------------------------------------------------------------------------

SkInstance * FAppInfo::bind_name_new_instance(const SkBindName & bind_name) const
  {
  return SkUEName::new_instance(reinterpret_cast<const FName &>(bind_name));
  }

//---------------------------------------------------------------------------------------

SkClass * FAppInfo::bind_name_class() const
  {
  return SkUEName::get_class();
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FSkookumScriptRuntime
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
FSkookumScriptRuntime::FSkookumScriptRuntime()
  : m_is_skookum_disabled(false)
#if WITH_EDITORONLY_DATA
  , m_generator(this)
#endif
#ifdef SKOOKUM_REMOTE_UNREAL
#if WITH_EDITORONLY_DATA 
  , m_remote_client(&m_generator)
  , m_freshen_binaries_requested(true)
#else
  , m_remote_client(nullptr)
  , m_freshen_binaries_requested(false) // With cooked data, load binaries immediately and do not freshen
#endif
#endif
  , m_game_world_p(nullptr)
  , m_editor_world_p(nullptr)
  , m_num_game_worlds(0)
  {
  }

//---------------------------------------------------------------------------------------

FSkookumScriptRuntime::~FSkookumScriptRuntime()
  {
  }

//---------------------------------------------------------------------------------------
// This code will execute after your module is loaded into memory (but after global
// variables are initialized, of course.)
void FSkookumScriptRuntime::StartupModule()
  {
  #if WITH_EDITORONLY_DATA
    // In editor builds, don't activate SkookumScript if there's no project (project wizard mode)
    if (!m_generator.have_project())
      {
      m_is_skookum_disabled = true;
      return;
      }
  #else
    // In cooked builds, stay inert when there's no compiled binaries
    if (!m_runtime.is_binary_hierarchy_existing())
      {
      m_is_skookum_disabled = true;
      return;
      }
  #endif

  A_DPRINT("Starting up SkookumScript plug-in modules\n");

  load_ini_settings();

  // Note that FWorldDelegates::OnPostWorldCreation has world_p->WorldType set to None
  // Note that FWorldDelegates::OnPreWorldFinishDestroy has world_p->GetName() set to "None"
  m_on_world_init_pre_handle    = FWorldDelegates::OnPreWorldInitialization.AddRaw(this, &FSkookumScriptRuntime::on_world_init_pre);
  m_on_world_init_post_handle   = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FSkookumScriptRuntime::on_world_init_post);
  m_on_world_cleanup_handle     = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FSkookumScriptRuntime::on_world_cleanup);

  #if WITH_EDITORONLY_DATA
    // Install this class as a "compiler" so we know when a Blueprint is about to be compiled
    IKismetCompilerInterface & kismet_compiler = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>(KISMET_COMPILER_MODULENAME);
    kismet_compiler.GetCompilers().Add(this);

    m_on_pre_compile_handle  = FKismetCompilerContext::OnPreCompile.AddRaw(this, &FSkookumScriptRuntime::OnPreCompile);
    m_on_post_compile_handle = FKismetCompilerContext::OnPostCompile.AddRaw(this, &FSkookumScriptRuntime::OnPostCompile);
    m_on_asset_loaded_handle = FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &FSkookumScriptRuntime::on_new_asset);
  #endif

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Start up SkookumScript
  // Originally, the compiled binaries were loaded with a delay when in UE4Editor to provide the user with a smoother startup sequence
  // However this caused issues with the proper initialization of Skookum Blueprint nodes
  // So to avoid glitches, SkookumScript is always initialized right away right here
  ensure_runtime_initialized();
  }

//---------------------------------------------------------------------------------------
// Called after the module has been reloaded
/*
void FSkookumScriptRuntime::PostLoadCallback()
  {
  A_DPRINT(A_SOURCE_STR " SkookumScript - loaded.\n");
  }
*/

//---------------------------------------------------------------------------------------
void FSkookumScriptRuntime::on_world_init_pre(UWorld * world_p, const UWorld::InitializationValues init_vals)
  {
  //A_DPRINT("on_world_init_pre: %S %p\n", *world_p->GetName(), world_p);

  if (world_p->IsGameWorld())
    {
    // Keep track of how many game worlds we got
    ++m_num_game_worlds;

    if (!m_game_world_p)
      {
      m_game_world_p = world_p;

      // Make sure all UE4 classes that have been loaded along with this world are bound
      m_runtime.sync_all_reflected_to_ue(true);

      // When the first game world is initialized, do some last minute binding
      if (m_runtime.is_compiled_scripts_loaded() && !m_runtime.is_compiled_scripts_bound())
        {
        // Finish binding atomics now
        m_runtime.bind_compiled_scripts();
        }

      if (is_skookum_initialized())
        {
        SkUEClassBindingHelper::set_world(world_p);
        SkookumScript::initialize_gameplay();
        }
      m_game_tick_handle = world_p->OnTickDispatch().AddRaw(this, &FSkookumScriptRuntime::tick_game);
      }
    }
#if WITH_EDITOR
  else if (world_p->WorldType == EWorldType::Editor || world_p->WorldType == EWorldType::Inactive)
    {
    // Only one editor world does ever get ticked, so add tick handlers to all editor worlds we encounter
    if (!m_editor_tick_handles.Find(world_p))
      {
      m_editor_tick_handles.Emplace(world_p) = world_p->OnTickDispatch().AddRaw(this, &FSkookumScriptRuntime::tick_editor);
      }
    }
#endif
  }

//---------------------------------------------------------------------------------------
void FSkookumScriptRuntime::on_world_init_post(UWorld * world_p, const UWorld::InitializationValues init_vals)
  {
  //A_DPRINT("on_world_init_post: %S %p\n", *world_p->GetName(), world_p);

  #if !WITH_EDITORONLY_DATA
    // Resolve raw data for all classes if a callback function is given
    // $Revisit MBreyer this gets called several times (so in cooked builds everything gets resolved) - fix so it's called only once
    SkBrain::ms_object_class_p->resolve_raw_data_recurse();
  #endif

  #ifdef SKOOKUM_REMOTE_UNREAL
    if (world_p->IsGameWorld() && !IsRunningCommandlet() && allow_auto_connect_to_ide())
      {
      SkUERemote::ms_client_p->ensure_connected(5.0);
      }
  #endif
  }

//---------------------------------------------------------------------------------------
void FSkookumScriptRuntime::on_world_cleanup(UWorld * world_p, bool session_ended_b, bool cleanup_resources_b)
  {
  //A_DPRINT("on_world_cleanup: %S %p\n", *world_p->GetName(), world_p);

  if (world_p->IsGameWorld())
    {
    // Set world pointer to null if it was pointing to us
    if (m_game_world_p == world_p)
      {
      m_game_world_p->OnTickDispatch().Remove(m_game_tick_handle);
      m_game_world_p = nullptr;
      SkUEClassBindingHelper::set_world(nullptr);
      }

    // We shouldn't get here if there are no game worlds left but things happen
    if (m_num_game_worlds > 0)
      {
      // Keep track of how many game worlds we got
      --m_num_game_worlds;

      // Restart SkookumScript if initialized
      if (m_num_game_worlds == 0 && is_skookum_initialized())
        {
        // Simple shutdown
        //SkookumScript::get_world()->clear_coroutines();
        A_DPRINT(
          "SkookumScript resetting session...\n"
          "  cleaning up...\n");
        SkookumScript::deinitialize_gameplay();
        SkookumScript::deinitialize_sim();
        SkookumScript::initialize_sim();
        A_DPRINT("  ...done!\n\n");
        }
      }
    }
#if WITH_EDITOR
  else if (world_p->WorldType == EWorldType::Editor)
    {
    // Find and remove tick handler from editor world
    FDelegateHandle * handle_p = m_editor_tick_handles.Find(world_p);
    if (handle_p)
      {
      world_p->OnTickDispatch().Remove(*handle_p);
      m_editor_tick_handles.Remove(world_p);
      }
    }
#endif
  }

//---------------------------------------------------------------------------------------
// Called before the module has been unloaded
/*
void FSkookumScriptRuntime::PreUnloadCallback()
  {
  A_DPRINT(A_SOURCE_STR " SkookumScript - about to unload.\n");
  }
*/

//---------------------------------------------------------------------------------------
// This function may be called during shutdown to clean up your module.  For modules that
// support dynamic reloading, we call this function before unloading the module.
void FSkookumScriptRuntime::ShutdownModule()
  {
  // Don't do anything if SkookumScript is not active
  if (m_is_skookum_disabled)
    {
    return;
    }

  // Printing during shutdown will re-launch IDE in case it has been closed prior to UE4
  // So quick fix is to just not print during shutdown
  //A_DPRINT(A_SOURCE_STR " Shutting down SkookumScript plug-in modules\n");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean up SkookumScript
  m_runtime.shutdown();

  #ifdef SKOOKUM_REMOTE_UNREAL
    // Remote communication to and from SkookumScript IDE
    m_remote_client.disconnect();
  #endif

  // Clear out our registered delegates
  FWorldDelegates::OnPreWorldInitialization.Remove(m_on_world_init_pre_handle);
  FWorldDelegates::OnPostWorldInitialization.Remove(m_on_world_init_post_handle);
  FWorldDelegates::OnWorldCleanup.Remove(m_on_world_cleanup_handle);

  #if WITH_EDITORONLY_DATA
    IKismetCompilerInterface * kismet_compiler_p = FModuleManager::GetModulePtr<IKismetCompilerInterface>(KISMET_COMPILER_MODULENAME);
    if (kismet_compiler_p)
      {
      kismet_compiler_p->GetCompilers().Remove(this);
      }

    FKismetCompilerContext::OnPreCompile.Remove(m_on_pre_compile_handle);
    FKismetCompilerContext::OnPostCompile.Remove(m_on_post_compile_handle);
    FCoreUObjectDelegates::OnAssetLoaded.Remove(m_on_asset_loaded_handle);
  #endif
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::set_project_generated_bindings(SkUEBindingsInterface * project_generated_bindings_p)
  {
  // If we got bindings, make sure things are initialized
  if (project_generated_bindings_p)
    {
    // Have we had any game bindings before?
    if (m_runtime.have_game_module())
      {
      // Yes, this is a hot reload of the game DLL which just has been recompiled, and scripts have been regenerated by UHT
      // so recompile and reload the binaries
      compile_and_load_binaries();
      }
    else
      {
      // No, this is the first time bindings are set
      // Make sure things are properly initialized
      ensure_runtime_initialized();
      }
    }

  // Now that binaries are loaded, point to the bindings to use
  m_runtime.set_project_generated_bindings(project_generated_bindings_p);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::load_ini_settings()
  {
  const FString & ini_file_path = get_ini_file_path();

  #ifdef SKOOKUM_REMOTE_UNREAL
    FString last_connected_to_ide(TEXT("0"));
    GConfig->GetString(ms_ini_section_name_p, ms_ini_key_last_connected_to_ide_p, last_connected_to_ide, ini_file_path);
    m_remote_client.set_last_connected_to_ide(!last_connected_to_ide.IsEmpty() && last_connected_to_ide[0] == '1');
  #endif
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::save_ini_settings()
  {
  if (!IsRunningCommandlet())
    {
    const FString & ini_file_path = get_ini_file_path();

    #ifdef SKOOKUM_REMOTE_UNREAL
      GConfig->SetString(ms_ini_section_name_p, ms_ini_key_last_connected_to_ide_p, m_remote_client.get_last_connected_to_ide() ? TEXT("1") : TEXT("0"), ini_file_path);
    #endif
    }
  }

//---------------------------------------------------------------------------------------

const FString & FSkookumScriptRuntime::get_ini_file_path() const
  {
  static FString _ini_file_path;
  if (_ini_file_path.IsEmpty())
    {
    const FString ini_file_dir = FPaths::ProjectSavedDir() + TEXT("Config/");
    FConfigCacheIni::LoadGlobalIniFile(_ini_file_path, TEXT("SkookumScriptRuntime"), NULL, false, false, true, *ini_file_dir);
    }
  return _ini_file_path;
  }

//---------------------------------------------------------------------------------------

eSkProjectMode FSkookumScriptRuntime::get_project_mode() const
  {
  #if WITH_EDITORONLY_DATA
    // If we have a runtime generator, ask it for the mode
    return m_generator.get_project_mode();
  #else
    // This is a cooked build: If we get here, the compiled binaries exist in their "editable" location, 
    // otherwise m_is_skookum_disabled would be set and the entire plugin disabled
    return SkProjectMode_editable;
  #endif
  }

//---------------------------------------------------------------------------------------
// Plugin is dormant when in read-only (REPL) mode and not connected to IDE
bool FSkookumScriptRuntime::is_dormant() const
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    return (get_project_mode() == SkProjectMode_read_only && !m_remote_client.is_authenticated());
  #else
    return false;
  #endif
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntime::allow_auto_connect_to_ide() const
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    return !is_dormant() || m_remote_client.get_last_connected_to_ide();
  #else
    return false; // There is no way to connect to the IDE in this case
  #endif
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntime::is_skookum_initialized() const
  {
  return (SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_program);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::ensure_runtime_initialized()
  {
  if (!m_runtime.is_initialized())
    {
    m_runtime.startup();
    compile_and_load_binaries();
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::compile_and_load_binaries()
  {
  #if defined(SKOOKUM_REMOTE_UNREAL) && WITH_EDITOR
    // Tell IDE to compile the binaries, then load them
    if (!IsRunningCommandlet() && allow_auto_connect_to_ide())
      {
      // At this point, wait if necessary to make sure we are connected
      m_remote_client.ensure_connected(0.0);

      // Alert user in case we are still not connected - and allow for corrective measures
      bool load_binaries = true;
      while (!m_remote_client.is_authenticated())
        {
        FText title = FText::FromString(TEXT("SkookumScript UE4 Plugin cannot connect to the SkookumIDE!"));
        EAppReturnType::Type decision = FMessageDialog::Open(
          EAppMsgType::CancelRetryContinue,
          FText::Format(FText::FromString(TEXT(
            "The SkookumScript UE4 Plugin cannot connect to the SkookumIDE. A connection to the SkookumIDE is required to properly work with SkookumScript.\n\n"
            "The connection problem could be caused by any of the following situations:\n"
            "- The SkookumIDE application is not running. If this is the case, your security software (Virus checker, VPN, Firewall) may have blocked or even removed it. If so, allow SkookumIDE.exe to run, then click 'Retry'. "
            "You can also try to launch the IDE manually. It should be located at the following path: {0}. Once running, click 'Retry'.\n"
            "- The SkookumIDE application is running, but stuck on an error. If so, try to resolve the error, and when the SkookumIDE is back up, click 'Retry'.\n"
            "- The SkookumIDE application is running and seems to be working fine. "
            "If so, the IP and port that the SkookumScript UE4 Plugin is trying to connect to ({1}) might be different from the IP and port that the SkookumIDE is listening to (see SkookumIDE log window), or blocked by a firewall. "
            "These problems could be due to your networking environment, such as a custom firewall, virtualization software such as VirtualBox, or multiple network adapters.\n\n"
            "For additional information including how to specify the SkookumIDE address for the runtime, please see http://skookumscript.com/docs/v3.0/ide/ip-addresses/ and ensure 'Settings'->'Remote runtimes' on the SkookumIDE is set properly.\n\n"
            "If you are having difficulties resolving this issue, please don't hesitate to ask us for help at the SkookumScript Forum (http://forum.skookumscript.com). We are here to make your experience skookum!\n")), 
            FText::FromString(FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("SkookumScript"))->GetBaseDir() / TEXT("SkookumIDE") / TEXT("SkookumIDE.exe"))),
            FText::FromString(m_remote_client.get_ip_address_ide()->ToString(true))),
          &title);
        if (decision != EAppReturnType::Retry)
          {
          load_binaries = (decision == EAppReturnType::Continue);
          break;
          }
        m_remote_client.ensure_connected(10.0);
        }

      if (load_binaries && m_remote_client.is_authenticated())
        {
      RetryCompilation:
        // Block while binaries are being recompiled
        m_remote_client.cmd_compiled_state(true);
        m_freshen_binaries_requested = false; // Request satisfied
        while (!m_remote_client.is_load_compiled_binaries_requested()
            && !m_remote_client.is_compiled_binaries_have_errors())
          {
          m_remote_client.wait_for_update();
          }
        if (m_remote_client.is_compiled_binaries_have_errors())
          {
          FText title = FText::FromString(TEXT("Compilation errors!"));
          EAppReturnType::Type decision = FMessageDialog::Open(
            EAppMsgType::CancelRetryContinue,
            FText::FromString(TEXT(
              "The SkookumScript compiled binaries could not be generated because errors were found in the script files.\n\n"
              "Check the IDE if the errors are in your project code and can be easily fixed. If so, fix them then hit 'Retry'.\n\n"
              "If the errors are in an overlay named 'Project-Generated' or 'Project-Generated-BP', the scripts in that overlay might have to be regenerated. "
              "To do this click 'Continue'. UE4 will continue loading, regenerate the script code and all should be good.\n\n"
              "If the above did not help, (and the errors are in an overlay named 'Project-Generated' or 'Project-Generated-BP'), deleting the folder these files are in might help. "
              "In the IDE, when displaying the error, right-click on the script that has the error and choose 'Show in Explorer'. "
              "Make sure the folder is inside 'Project-Generated' or 'Project-Generated-BP'. If so, delete the folder you opened up, and recompile in the IDE."
            )),
            &title);
          if (decision == EAppReturnType::Retry)
            {
            m_remote_client.clear_load_compiled_binaries_requested();
            goto RetryCompilation;
            }
          load_binaries = (decision == EAppReturnType::Continue);
          }
        m_remote_client.clear_load_compiled_binaries_requested();
        }

      if (load_binaries)
        {
        // Attempt to load binaries at this point
        bool success_b = m_runtime.load_compiled_scripts();
        if (success_b)
          {
          // Inform the IDE about the version we got
          m_remote_client.cmd_incremental_update_reply(true, SkBrain::ms_session_guid, SkBrain::ms_revision);
          }
        else
          {
          // Something went wrong - let the user know
          FText title = FText::FromString(TEXT("Unable to load SkookumScript compiled binaries!"));
          FMessageDialog::Open(
            EAppMsgType::Ok,
            FText::FromString(TEXT(
              "Unable to load the compiled binaries. This is most likely caused by errors in the script files which prevented a successful compilation. The project will continue to load with SkookumScript temporarily disabled.")),
            &title);
          }
        }
      }
    else if (!is_dormant()) // Don't load binaries if dormant since the binaries might have issues
  #endif
      {
      // If no remote connection, or commandlet mode, or cooked build, load binaries at this point
      bool success_b = m_runtime.load_compiled_scripts();
      SK_ASSERTX(success_b, AErrMsg("Unable to load SkookumScript compiled binaries!", AErrLevel_notify));
      }
  }

//---------------------------------------------------------------------------------------
// Update SkookumScript in game
//
// #Params:
//   deltaTime: Game time passed since the last call.
void FSkookumScriptRuntime::tick_game(float deltaTime)
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    tick_remote();
  #endif

  // When paused, set deltaTime to 0.0
  #if WITH_EDITOR
    if (!m_game_world_p->IsPaused())
  #endif
      {
      SCOPE_CYCLE_COUNTER(STAT_SkookumScriptTime);
      m_runtime.update(deltaTime);
      }
  }

//---------------------------------------------------------------------------------------
// Update SkookumScript in editor
//
// #Params:
//   deltaTime: Game time passed since the last call.
void FSkookumScriptRuntime::tick_editor(float deltaTime)
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    if (!m_game_world_p)
      {
      tick_remote();
      }
  #endif
  }

#ifdef SKOOKUM_REMOTE_UNREAL

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::tick_remote()
  {
  if (!IsRunningCommandlet())
    {
    if (m_remote_client.is_authenticated())
      {
      // Remember connection status
      if (!m_remote_client.get_last_connected_to_ide())
        {
        m_remote_client.set_last_connected_to_ide(true);
        save_ini_settings();
        }

      // Request recompilation of binaries if script files changed
      if (m_freshen_binaries_requested)
        {
        m_remote_client.cmd_compiled_state(true);
        m_freshen_binaries_requested = false;
        }

      // Remote communication to and from SkookumScript IDE.
      // Needs to be called whether in editor or game and whether paused or not
      // $Revisit - CReis This is probably a hack. The remote client update should probably
      // live somewhere other than a tick method such as its own thread.
      m_remote_client.process_incoming();

      // Re-load compiled binaries?
      // If the game is currently running, delay until it's not
      if (m_remote_client.is_load_compiled_binaries_requested() 
       && SkookumScript::get_initialization_level() < SkookumScript::InitializationLevel_gameplay)
        {
        // Load the Skookum class hierarchy scripts in compiled binary form
        bool is_first_time = !is_skookum_initialized();

        bool success_b = m_runtime.load_and_bind_compiled_scripts(true);
        SK_ASSERTX(success_b, AErrMsg("Unable to load SkookumScript compiled binaries!", AErrLevel_notify));
        m_remote_client.clear_load_compiled_binaries_requested();

        // If class data needs to be generated, generate it now
        #if WITH_EDITORONLY_DATA
          if (m_remote_client.does_class_data_need_to_be_regenerated())
            {
            m_generator.update_all_class_script_files(true);
            m_remote_client.clear_class_data_need_to_be_regenerated();
            }
        #endif

        // Inform the IDE about the version we got
        if (success_b)
          {
          m_remote_client.cmd_incremental_update_reply(true, SkBrain::ms_session_guid, SkBrain::ms_revision);
          }
        }
      }
    else
      {
      // Remember connection status
      if (m_remote_client.get_last_connected_to_ide())
        {
        m_remote_client.set_last_connected_to_ide(false);
        save_ini_settings();
        }
      }
    }
  }

#endif

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntime::is_skookum_disabled() const
  {
  return m_is_skookum_disabled;
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_freshen_binaries_pending() const
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    return m_freshen_binaries_requested;
  #else
    return false;
  #endif
  }

#if WITH_EDITOR

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::set_editor_interface(ISkookumScriptRuntimeEditorInterface * editor_interface_p)
  {
  m_runtime.set_editor_interface(editor_interface_p);
  #ifdef SKOOKUM_REMOTE_UNREAL
    m_remote_client.set_editor_interface(editor_interface_p);
  #endif
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptRuntime::is_connected_to_ide() const
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    return m_remote_client.is_authenticated();
  #else
    return false;
  #endif
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_application_focus_changed(bool is_active)
  {
  if (is_active && !is_dormant())
    {
    m_generator.sync_all_class_script_files_from_disk();

    if (m_generator.reload_skookumscript_ini())
      {
      m_generator.update_all_class_script_files(true);
      }
    }
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::on_editor_map_opened()
  {
  if (!is_dormant())
    {
    // Regenerate them all just to be sure
    m_generator.update_all_class_script_files(true);

    #ifdef SKOOKUM_REMOTE_UNREAL
      m_remote_client.clear_class_data_need_to_be_regenerated();
    #endif
    }
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::show_ide(const FString & focus_ue_class_name, const FString & focus_ue_member_name, bool is_data_member, bool is_class_member)
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    // Remove qualifier from member name if present
    FString focus_sk_class_name;
    FString focus_sk_member_name;
    int32 at_pos = 0;
    if (focus_ue_member_name.FindChar('@', at_pos))
      {
      focus_sk_class_name = focus_ue_member_name.Left(at_pos).TrimTrailing();
      focus_sk_member_name = focus_ue_member_name.Mid(at_pos + 1).Trim();
      }
    else
      {
      focus_sk_class_name = FSkookumScriptGeneratorHelper::skookify_class_name_basic(focus_ue_class_name);
      focus_sk_member_name = is_data_member
        ? FSkookumScriptGeneratorHelper::skookify_data_name_basic(focus_ue_member_name, false, is_class_member ? FSkookumScriptGeneratorHelper::DataScope_class : FSkookumScriptGeneratorHelper::DataScope_instance)
        : FSkookumScriptGeneratorHelper::skookify_method_name(focus_ue_member_name);
      }

    // Convert to symbols and send off
    ASymbol focus_class_name_sym(ASymbol::create_existing(FStringToAString(focus_sk_class_name)));
    ASymbol focus_member_name_sym(ASymbol::create_existing(FStringToAString(focus_sk_member_name)));
    m_remote_client.cmd_show(AFlag_on, focus_class_name_sym, focus_member_name_sym, is_data_member, is_class_member);
  #endif
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::freshen_compiled_binaries_if_have_errors()
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    if (m_remote_client.is_compiled_binaries_have_errors())
      {
      m_freshen_binaries_requested = true;
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::has_skookum_default_constructor(UClass * class_p) const
  {
  SK_ASSERTX(m_runtime.is_initialized(), "Runtime must be initialized for this code to work.");

  SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(class_p);
  if (sk_class_p)
    {
    return (sk_class_p->find_instance_method_inherited(ASymbolX_ctor) != nullptr);
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::has_skookum_destructor(UClass * class_p) const
  {
  SK_ASSERTX(m_runtime.is_initialized(), "Runtime must be initialized for this code to work.");

  SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(class_p);
  if (sk_class_p)
    {
    return (sk_class_p->find_instance_method_inherited(ASymbolX_dtor) != nullptr);
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_skookum_class_data_component_class(UClass * class_p) const
  {
  return class_p->IsChildOf(USkookumScriptClassDataComponent::StaticClass());
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_skookum_behavior_component_class(UClass * class_p) const
  {
  return class_p->IsChildOf(USkookumScriptBehaviorComponent::StaticClass());
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_skookum_reflected_call(UFunction * function_p) const
  {
  return m_runtime.get_reflection_manager()->is_skookum_reflected_call(function_p);
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_skookum_reflected_event(UFunction * function_p) const
  {
  return m_runtime.get_reflection_manager()->is_skookum_reflected_event(function_p);
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::on_class_added_or_modified(UBlueprint * blueprint_p)
  {
  // Only do something if the blueprint has a class
  UClass * ue_class_p = blueprint_p->GeneratedClass;
  if (ue_class_p && !is_dormant())
    {
    // Generate script files for the new/changed class
    m_generator.update_class_script_file(ue_class_p, true, true);
    m_generator.update_used_class_script_files(true);

    // Find associated SkClass if any
    SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_class(ue_class_p);
    if (sk_class_p)
      {
      // Attach USkookumScriptInstanceProperty to Blueprint generated classes
      if (SkUEReflectionManager::does_class_need_instance_property(sk_class_p))
        {
        SkUEReflectionManager::add_instance_property_to_class(ue_class_p, sk_class_p);
        }

      // Re-expose all functions as some might have been regenerated
      m_runtime.sync_all_reflected_to_ue(true);

      // Re-resolve the raw data if applicable
      if (SkUEClassBindingHelper::resolve_raw_data_funcs(sk_class_p, ue_class_p))
        {
        SkUEClassBindingHelper::resolve_raw_data(sk_class_p, ue_class_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::on_class_renamed(UBlueprint * blueprint_p, const FString & old_ue_class_name)
  {
  if (!is_dormant())
    {
    m_generator.rename_class_script_file(blueprint_p, old_ue_class_name);
    }
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::on_class_deleted(UBlueprint * blueprint_p)
  {
  if (!is_dormant())
    {
    m_generator.delete_class_script_file(blueprint_p);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_struct_added_or_modified(UUserDefinedStruct * ue_struct_p)
  {
  if (!is_dormant())
    {
    // Generate script files for the new/changed class
    m_generator.update_class_script_file(ue_struct_p, false, true);
    // Also generate parent class "UStruct"
    m_generator.create_root_class_script_file(TEXT("UStruct"));

    // Find associated SkClass if any
    SkClass * sk_class_p = SkUEClassBindingHelper::get_sk_class_from_ue_struct(ue_struct_p);
    if (sk_class_p)
      {
      // Re-resolve the raw data if applicable
      if (SkUEClassBindingHelper::resolve_raw_data_funcs(sk_class_p, ue_struct_p))
        {
        SkUEClassBindingHelper::resolve_raw_data(sk_class_p, ue_struct_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_struct_renamed(UUserDefinedStruct * ue_struct_p, const FString & old_ue_struct_name)
  {
  if (!is_dormant())
    {
    m_generator.rename_class_script_file(ue_struct_p, old_ue_struct_name);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_struct_deleted(UUserDefinedStruct * ue_struct_p)
  {
  if (!is_dormant())
    {
    m_generator.delete_class_script_file(ue_struct_p);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_enum_added_or_modified(UUserDefinedEnum * ue_enum_p)
  {
  if (!is_dormant())
    {
    // Generate script files for the new/changed enum
    m_generator.update_class_script_file(ue_enum_p, false, true);
    // Also generate parent class "Enum"
    m_generator.create_root_class_script_file(TEXT("Enum"));
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_enum_renamed(UUserDefinedEnum * ue_enum_p, const FString & old_ue_enum_name)
  {
  if (!is_dormant())
    {
    m_generator.rename_class_script_file(ue_enum_p, old_ue_enum_name);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_enum_deleted(UUserDefinedEnum * ue_enum_p)
  {
  if (!is_dormant())
    {
    m_generator.delete_class_script_file(ue_enum_p);
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_new_asset(UObject * obj_p)
  {
  UBlueprint * blueprint_p = Cast<UBlueprint>(obj_p);
  if (blueprint_p)
    {
    // Install callback so we know when it was compiled, make sure that only a 
    // single callback is installed per blueprint.
    if (!blueprint_p->OnCompiled().IsBoundToObject(this))
      {
      blueprint_p->OnCompiled().AddRaw(this, &FSkookumScriptRuntime::on_blueprint_compiled);

      // If the callback is already installed then on_blueprint_compiled will call the below
      on_class_added_or_modified(blueprint_p);
      }
    }

  UUserDefinedStruct * struct_p = Cast<UUserDefinedStruct>(obj_p);
  if (struct_p)
    {
    on_struct_added_or_modified(struct_p);
    }

  UUserDefinedEnum * enum_p = Cast<UUserDefinedEnum>(obj_p);
  if (enum_p)
    {
    on_enum_added_or_modified(enum_p);
    }
  }

#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::OnPreCompile()
  {
  // Last minute attempt to resolve yet unresolved bindings
  m_runtime.sync_all_reflected_to_ue(true);

  // Make sure all Blueprints are instrumented at this point
  //if (!GIsEditor) // Only necessary in standalone mode
    {
    for (TObjectIterator<UBlueprint> blueprint_it; blueprint_it; ++blueprint_it)
      {
      if (!blueprint_it->OnCompiled().IsBoundToObject(this))
        {
        on_new_asset(*blueprint_it);
        }
      // In the case of multiple class inheritance we can often get into a situation where we're
      // adding a BP variable to a Parent class followed by adding a BP variable to a Child class.
      // In these cases, the child blueprint will crash during BP compilation (in serialization).
      // Debugging this for quite a long time, my conclusion was that raw data needed to be resolved
      // prior to compiling. Raw data resolution will occur in on_new_asset above, but only if we 
      // haven't yet bound to OnCompiled. So below we force raw data resolution if a blueprint is
      // about to be or is being compiled.
      else if((*blueprint_it)->bBeingCompiled || (*blueprint_it)->bQueuedForCompilation)
        {
        on_class_added_or_modified(*blueprint_it);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::OnPostCompile()
  {
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::on_blueprint_compiled(UBlueprint * blueprint_p)
  {
  on_class_added_or_modified(blueprint_p);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::PreCompile(UBlueprint * blueprint_p)
  {
  // At this point, all bindings must be resolved
  m_runtime.sync_all_reflected_to_ue(true);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptRuntime::PostCompile(UBlueprint * blueprint_p)
  {
  // Slap a USkookumScriptInstanceProperty onto the newly generated class
  on_class_added_or_modified(blueprint_p);
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_static_class_known_to_skookum(UClass * class_p) const
  {
  m_runtime.ensure_static_ue_types_registered();
  return SkUEClassBindingHelper::is_static_class_registered(class_p);
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_static_struct_known_to_skookum(UStruct * struct_p) const
  {
  m_runtime.ensure_static_ue_types_registered();
  return SkUEClassBindingHelper::is_static_struct_registered(struct_p);
  }

//---------------------------------------------------------------------------------------
// 
bool FSkookumScriptRuntime::is_static_enum_known_to_skookum(UEnum * enum_p) const
  {
  m_runtime.ensure_static_ue_types_registered();
  return SkUEClassBindingHelper::is_static_enum_registered(enum_p);
  }

//---------------------------------------------------------------------------------------
// 
void FSkookumScriptRuntime::on_class_scripts_changed_by_generator(const FString & class_name, eChangeType change_type)
  {
  #ifdef SKOOKUM_REMOTE_UNREAL
    m_freshen_binaries_requested = true;
  #endif
  }

#endif // WITH_EDITORONLY_DATA

//#pragma optimize("g", on)
