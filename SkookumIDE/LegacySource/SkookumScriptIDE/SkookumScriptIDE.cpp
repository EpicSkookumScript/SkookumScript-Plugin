// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Main & global function hooks
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\AFunction.hpp>
#include <AgogCore\AFunctionArg.hpp>
#include <AgogIO\AgogIO.hpp>
#include <AgogIO\AApplication.hpp>
#include <AgogIO\ATimer.hpp>
#include <AgogGUI_OS\ADialogOS.hpp>
#include <AgogGUI_OS\AErrorDialog.hpp>
#include <AgogGUI_OS\AgogGUI_OS.hpp>
#include <SkookumIDE\SkookumIDE.hpp>
#include <SkookumIDE\SkConsole.hpp>
#include <SkookumIDE\SkSearchDialog.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objidl.h>

#pragma warning( push )
  #pragma warning( disable : 4458 ) // hidden class member
  #include <gdiplus.h>
#pragma warning( pop )


//---------------------------------------------------------------------------------------
// Link required libraries
// $Note - CReis These will only link if this module (cpp file) contains other used code.
#if defined(_MSC_VER)
  #pragma comment(lib, "GDIPlus.lib")
#endif


//=======================================================================================
// Local Global Structures
//=======================================================================================

using namespace Gdiplus;

namespace
{

} // End unnamed namespace


//---------------------------------------------------------------------------------------
// Wrapper for SkookumIDE and updating/iterating code for it
class SkookumIDEUpdater
  {
  public:

  // Public Data Members

    SkookumIDE m_ide;
    ATimer     m_timer;

  // Public Class Data Members

    static bool                ms_need_update_b;
    static uint32_t            ms_session_start;
    static SkookumIDEUpdater * ms_updater_p;


  // Methods

    SkookumIDEUpdater(SkConsole::eCloseAction close_action);

    void shutdown();

  // Class Methods

    static void on_idle();
    static void on_update_request(bool update_req_b);
    static void on_update_time();

  };  // SkookumIDEUpdater


bool                SkookumIDEUpdater::ms_need_update_b = false;
uint32_t            SkookumIDEUpdater::ms_session_start = 0u;
SkookumIDEUpdater * SkookumIDEUpdater::ms_updater_p     = nullptr;


//---------------------------------------------------------------------------------------
// Constructor
// # Author(s): Conan Reis
SkookumIDEUpdater::SkookumIDEUpdater(SkConsole::eCloseAction close_action) :
  m_ide(close_action)
  {
  ms_updater_p = this;

  SkookumScript::register_update_time_func(on_update_time);
  SkookumScript::register_update_request_func(on_update_request);

  m_timer.set_idle_func_p(new AFunction(SkookumIDEUpdater::on_idle));
  //m_timer.set_tick_func_p(new AFunction(SkookumIDEUpdater::on_idle));
  //m_timer.set_tick_interval(120u, 20u);
  }

//---------------------------------------------------------------------------------------
void SkookumIDEUpdater::shutdown()
  {
  m_ide.shutdown();
  m_timer.enable_idle_processing(false);
  ms_updater_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Called every idle loop - i.e. whenever the CPU isn't doing anything.
// # Notes:    When this is running, CPU usage will be at 100% since all idle cycles will
//             be used.
// # Modifiers: static
// # Author(s): Conan Reis
void SkookumIDEUpdater::on_idle()
  {
  // $Note - CReis This will overflow approximately every 50 days.
  // The AMetrics class should be used in game code instead.
  SkookumScript::update_ticks(ATimer::get_elapsed_ms() - ms_session_start);
  }

//---------------------------------------------------------------------------------------
// Called every idle loop - i.e. whenever the CPU isn't doing anything.
// # Notes:    When this is running, CPU usage will be at 100% since all idle cycles will
//             be used.
// # Modifiers: static
// # Author(s): Conan Reis
void SkookumIDEUpdater::on_update_time()
  {
  // $Note - CReis This will overflow approximately every 50 days.
  // The AMetrics class should be used in game code instead.
  if (ms_session_start == 0u)
    {
    ms_session_start = ATimer::get_elapsed_ms();
    }

  SkookumScript::update_time_ticks(ATimer::get_elapsed_ms() - ms_session_start);
  }

//---------------------------------------------------------------------------------------
// Called whenever the need to call SkookumScript::update*() changes.
// Arg         update_req_b - if true then SkookumScript needs to be updated, if false
//             then SkookumScript does not need to be updated.
// # Modifiers: static
// # Author(s): Conan Reis
void SkookumIDEUpdater::on_update_request(bool update_req_b)
  {
  if (ms_need_update_b != update_req_b)
    {
    //A_DPRINT("Scripts evaluated during idle processing: %s\n", update_req_b ? "on": "off");

    if (ms_session_start == 0u)
      {
      ms_session_start = ATimer::get_elapsed_ms();
      }

    ms_need_update_b = update_req_b;
    ms_updater_p->m_timer.enable_idle_processing(update_req_b);
    }
  }


//=======================================================================================
// SkookumIDE implementation of AAppInfoCore
//=======================================================================================

//---------------------------------------------------------------------------------------
class AAppInfoCoreSkIDE : public AAppInfoCoreIO
  {

  virtual AErrorOutputBase * on_error_pre(bool nested) override;

  };

//---------------------------------------------------------------------------------------
AErrorOutputBase * AAppInfoCoreSkIDE::on_error_pre(bool nested)
  {
  static AErrorDialog s_err_out;
  // Only give error dialog if console is still available
  return SkConsole::ms_console_p ? &s_err_out : nullptr;
  }


//=======================================================================================
// App Info
//=======================================================================================

//---------------------------------------------------------------------------------------
// IDE implementation of SkAppInfo
class SkIDEAppInfo : public SkAppInfo
  {

  virtual bool         use_builtin_actor() const override;
  virtual ASymbol      get_custom_actor_class_name() const override;
  
  virtual void         bind_name_construct(SkBindName * bind_name_p, const AString & value) const override;
  virtual void         bind_name_destruct(SkBindName * bind_name_p) const override;
  virtual void         bind_name_assign(SkBindName * bind_name_p, const AString & value) const override;
  virtual AString      bind_name_as_string(const SkBindName & bind_name) const override;
  virtual SkInstance * bind_name_new_instance(const SkBindName & bind_name) const override;
  virtual SkClass *    bind_name_class() const override;

  };

//---------------------------------------------------------------------------------------

bool SkIDEAppInfo::use_builtin_actor() const
  {
  return SkCompiler::ms_compiler_p->use_builtin_actor();
  }

//---------------------------------------------------------------------------------------

ASymbol SkIDEAppInfo::get_custom_actor_class_name() const
  {
  return SkCompiler::ms_compiler_p->get_custom_actor_class_name();
  }

//---------------------------------------------------------------------------------------

void SkIDEAppInfo::bind_name_construct(SkBindName * bind_name_p, const AString & value) const
  {
  static_assert(sizeof(AString) <= sizeof(SkBindName), "FName must fit into SkBindName.");
  new (bind_name_p) AString(value.as_cstr());
  }

//---------------------------------------------------------------------------------------

void SkIDEAppInfo::bind_name_destruct(SkBindName * bind_name_p) const
  {
  reinterpret_cast<AString *>(bind_name_p)->~AString();
  }

//---------------------------------------------------------------------------------------

void SkIDEAppInfo::bind_name_assign(SkBindName * bind_name_p, const AString & value) const
  {
  *reinterpret_cast<AString *>(bind_name_p) = value;
  }

//---------------------------------------------------------------------------------------

AString SkIDEAppInfo::bind_name_as_string(const SkBindName & bind_name) const
  {
  return reinterpret_cast<const AString &>(bind_name);
  }

//---------------------------------------------------------------------------------------

SkInstance * SkIDEAppInfo::bind_name_new_instance(const SkBindName & bind_name) const
  {
  SK_ERRORX("SkAppInfo::bind_name_new_instance() not implemented in IDE since the IDE does not have a native Name class.");
  return nullptr;
  }

//---------------------------------------------------------------------------------------

SkClass * SkIDEAppInfo::bind_name_class() const
  {
  return SkCompiler::ms_compiler_p->get_bind_name_class();
  }


//=======================================================================================
// Local Global Function Definitions
//=======================================================================================

namespace AgogIO
  {

  //---------------------------------------------------------------------------------------
  // This is the application starting point and it is called by WinMain().
  // # Author(s): Conan Reis
  int entry_point(AApplication * app_p)
    {
    // Set up Agog and SkookumScript
    AAppInfoCoreSkIDE core_info;
    SkIDEAppInfo app_info;
    // Set up IDE
    SkCompiler::initialize();
    SkConsole::initialize();

    AAppInfoCore * app_info_p = AgogCore::get_app_info();

    AgogCore::set_app_info(&core_info);
    SkookumScript::set_app_info(&app_info);

    // Ensure that there is only one instance of this app and if it is a duplicate instance
    // then pass on any command-line arguments to the first instance and shutdown.
    if (!app_p->ensure_single_instance(
      new AFunctionArg<const AString &>(&SkConsole::cmd_args_execute),
      "SkookumScriptIDE"))
      {
      return AExitCode_ok;
      }

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
   
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    SkSearchDialog::initialize();

    SkookumIDEUpdater ide_updater(SkConsole::CloseAction_shutdown);

    // the message loop
    int result = AApplication::main_loop();


    // Shut down IDE

    SkSearchDialog::deinitialize();
    SkConsole::deinitialize();
    SkCompiler::deinitialize();

    ide_updater.shutdown();

    GdiplusShutdown(gdiplusToken);

    // Put old app info back in place
    AgogCore::set_app_info(app_info_p);

    return result;
    }

  }  // End Agog namespace
