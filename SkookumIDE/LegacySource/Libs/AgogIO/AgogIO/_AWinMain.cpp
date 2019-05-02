// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// WinMain definition module
// Notes:          ##### This file should be included in every Agog application - it is not
//              compiled in the AgogIO library. #####
//
//              This has the standard Agog start up code in a custom WinMain() which calls
//              an application specific global Agog::entry_point()
//=======================================================================================


#ifndef AGOG_NO_WINMAIN


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO\AApplication.hpp>
#include <AgogIO\AgogIO.hpp>
#include <AgogGUI\AgogGUI.hpp>
#include <AgogGUI_OS\AgogGUI_OS.hpp>
#include <AgogGUI_OS\AErrorDialog.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if !defined(A_UNOPTIMIZED) && defined(A_EXTRA_CHECK)
  #include <crtdbg.h>
#endif


//=======================================================================================
// Function Prototypes
//=======================================================================================

namespace AgogIO
  {

  // This function should exist in an application's main.cpp or equivalent
  int entry_point(AApplication * app_p);

  };

//=======================================================================================
// WinMain implementation of AAppInfoCore
//=======================================================================================

class AAppInfoCoreWinMain : public AAppInfoCoreIO, public AAppInfoIO
  {

  virtual AErrorOutputBase * on_error_pre(bool nested) override;

  };

//---------------------------------------------------------------------------------------

AErrorOutputBase * AAppInfoCoreWinMain::on_error_pre(bool nested)
  {
  static AErrorDialog s_err_out;
  return &s_err_out;
  }

//=======================================================================================
// Global Function Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Windows' main entry point - custom for the Agog libraries.  It calls the
//             global Agog::entry_point()
// Returns:    application result value
// Arg         instance - 
// Arg         hinst_previous - 
// Arg         lpsz_cmd_line - 
// Arg         cmd_show
// Author(s):   Conan Reis
int PASCAL WinMain(
  HINSTANCE instance,
  HINSTANCE hinst_previous,
  LPSTR     lpsz_cmd_line,
  int       cmd_show
  )
  {
  AAppInfoCoreWinMain app_info;
  AgogCore::initialize(&app_info);
  AgogIO::initialize(&app_info);
  AgogGUI::initialize();
  AgogGUI_OS::initialize();

  int ret_val = 0;  // $Revisit - CReis Should this be an error value?  It is only used if Agog::entry_point() throws an exception.

    {
    eAErrAction  action  = AErrAction_ignore;
    AString      cmd_line(lpsz_cmd_line, ALength_calculate, false);
    AApplication app(instance, AApplication::cmd_show_to_show_state(cmd_show), cmd_line);

    #ifdef A_UNOPTIMIZED
      A_DPRINT("\n\nWinMain() - Starting application (thread 0x%X)\n\n", GetCurrentThreadId());
    #elif A_EXTRA_CHECK
      // For "Dev" builds:
      // The debug versions of some security-enhanced CRT functions fill the buffer passed
      // to them with a special character (0xFD) - this can be quite slow if there is no
      // other memory manager.  Calling _CrtSetDebugFillThreshold(0) will disable this
      // feature.
      _CrtSetDebugFillThreshold(0);
    #endif

    do
      {
      // This try-catch block is only for the code that is run in Agog::entry_point().
      // Agog::entry_point() in turn will call AApplication::main_loop() which has its own more
      // appropriate try-catch block.
        try
          {
          ret_val = AgogIO::entry_point(AApplication::ms_this_app_p);
          }
        catch (AExceptionBase & ex)
          {
          action = ex.resolve();
          }
        #ifdef AEX_CATCH_ALL  // See AEX_CATCH_ALL description at its definition in AgogCore/ADebug.hpp
          catch (...)
            {
            if (ADebug::resolve_error(AErrMsg(AErrMsg("An unexpected exception has occurred..."), nullptr, A_ERR_ARGS, AErrId_unknown), &action))
              {
              A_BREAK();
              }
            }
        #endif // AEX_CATCH_ALL
      }
    while (action == AErrAction_retry);

    // This dprint causes shutdown issues occasionally so commented out for now
    #if 0 //def A_UNOPTIMIZED
      A_DPRINT("\nWinMain() - Ending application (thread 0x%X)\n\n", ::GetCurrentThreadId());
    #endif
    }

  // Shut down Agog
  AgogGUI_OS::deinitialize();
  AgogGUI::deinitialize();
  AgogIO::deinitialize();
  AgogCore::deinitialize();

  return ret_val;
  }


#endif  // AGOG_NO_WINMAIN


