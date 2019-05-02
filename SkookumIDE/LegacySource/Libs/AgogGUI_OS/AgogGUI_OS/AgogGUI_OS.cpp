// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Components common to the AgogGUI_OS library - including all class data
//              members that require a specific initialization sequence.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AgogGUI_OS.hpp>
#include <AgogGUI_OS\ADialogOS.hpp>
#include <AgogGUI_OS\AButtonOS.hpp>
#include <AgogGUI_OS\ASplitterOS.hpp>
#include <AgogGUI_OS\AToolTipOS.hpp>
#include <AgogGUI_OS\ATreeOS.hpp>
#include <AgogGUI_OS\ATabViewOS.hpp>
#include <AgogGUI_OS\AListOS.hpp>
#include <AgogGUI_OS\AComboBoxOS.hpp>
#include <AgogIO\AgogIO.hpp>
#include <AgogCore\AConstructDestruct.hpp>
#include <objbase.h>
#include <shlwapi.h>


//---------------------------------------------------------------------------------------
// Link required libraries
// $Note - CReis These will only link if this module (cpp file) contains other used code.
#if defined(_MSC_VER)

  // Ensure that version 6 of common controls is used.
  // This could be put in a manifest XML, but embedding it in code is less error prone.
  // Note that the /MANIFEST compiler option must still be used to generate the manifest
  // as either a stand-alone file or embedded in the assembly.
  #pragma comment(linker, \
    "\"/manifestdependency:type='win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

  // Uncomment to display common control version when AgogGUI_OS::initialize() is called.
  //#define A_PRINT_OS_CONTROL_VER

  // Shell32.lib - (linked by AgogIO\AgogIO.lib) Used by ADialogOS
  // User32.lib  - (linked by AgogIO\AgogIO.lib) Used by everything in AgogGUI_OS
  // GDI32.lib   - (linked by AgogGUI\AgogGUI.lib)    Used by ASplitter
  #pragma comment(lib, "ComCtl32.lib")    // Used by AImageListOS, AListOS, ATreeOS
  #pragma comment(lib, "ComDlg32.lib")    // Used by ADialogOS
  #pragma comment(lib, "OLE32.lib")       // Used by AgogGUI_OS\AgogGUI_OS.cpp
  //#pragma comment(lib, "UxTheme.lib")     // Used by ATreeOS.cpp

#endif


//=======================================================================================
// Common Class Data Members
//=======================================================================================

//---------------------------------------------------------------------------------------
// Ensures that this translation unit is initialized before all others except for
// compiler translation units
// ***Note: This may not work with all compilers (Borland for example).
//#if defined(_MSC_VER)
//  pragma warning( disable :  4073 )  // Disable warning message for the next line
//  #pragma init_seg(lib)
//#endif


//=======================================================================================
// AgogGUI_OS Common
//=======================================================================================

namespace AgogGUI_OS
  {

  #ifdef A_PRINT_OS_CONTROL_VER

    #define PACKVERSION(major,minor) MAKELONG(minor,major)

    //---------------------------------------------------------------------------------------
    ULONGLONG get_dll_version(const char * dll_name_cstr_p)
      {
      HINSTANCE hinstDll;
      ULONGLONG dll_version = 0u;

      // For security purposes, LoadLibrary should be provided with a fully-qualified path
      // to the DLL. The dll_name_cstr_p variable should be tested to ensure that it is a
      // fully qualified path before it is used.
      hinstDll = LoadLibrary(dll_name_cstr_p);

      if (hinstDll)
        {
        DLLGETVERSIONPROC dll_get_version_p = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        // Because some DLLs might not implement this function, it must be tested for
        // explicitly.  Depending on the particular DLL, the lack of a DllGetVersion
        // function can be a useful indicator of the version.

        if (dll_get_version_p)
          {
          DLLVERSIONINFO2 dvi;

          ZeroMemory(&dvi, sizeof(dvi));
          dvi.cbSize = sizeof(dvi);

          HRESULT hr = (*dll_get_version_p)(&dvi);

          if (SUCCEEDED(hr))
            {
            //dll_version = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            dll_version = dvi.ullVersion;
            }
          }

        FreeLibrary(hinstDll);
        }

      return dll_version;
      }

  #endif  // A_PRINT_OS_CONTROL_VER

  //---------------------------------------------------------------------------------------
  // Initializes Agog library systems and globals.  Used with apps that do not
  //             use the Agog libraries for the main func.  It does not need to be called
  //             in apps that do use the Agog libraries as the main func.
  // Author(s):   Conan Reis
  void initialize_non_agog(
    HINSTANCE    app_instance,
    const char * cmd_line_p,
    HINSTANCE    res_instance // = 0u
    )
    {
    // Initialize Agog stuff
    AgogIO::initialize_non_agog(app_instance, cmd_line_p, res_instance);
    initialize();
    }

  //---------------------------------------------------------------------------------------
  // Initializes the AgogGUI_OS library and integrates it into subordinate Agog
  //             libraries.
  // Modifiers:   static
  // Author(s):   Conan Reis
  void initialize()
    {
    AWindow::set_tool_tip_create_func(AToolTipOS::create_attached);
    ADialogOS::register_writable_dialog();

    AButtonOS::initialize();
    ASplitterOS::initialize();
    AToolTipOS::initialize();
    ATreeOS::initialize();
    ATabGroupOS::initialize();
    AListIdxOS::initialize();
    AComboBoxOS::initialize();
    ARichEditOS::initialize();
    AEditLineOS::initialize();

    #ifdef A_PRINT_OS_CONTROL_VER
      ULONGLONG comctrl_ver = get_dll_version(TEXT("comctl32.dll"));  
      ADebug::print_format("\nComCtrl32.dll version: 0x%llx\n", comctrl_ver);
    #endif
    }

  //---------------------------------------------------------------------------------------
  // Deinitializes the AgogGUI_OS library
  void deinitialize()
    {
    AButtonOS::deinitialize();
    ASplitterOS::deinitialize();
    AToolTipOS::deinitialize();
    ATreeOS::deinitialize();
    ATabGroupOS::deinitialize();
    AListIdxOS::deinitialize();
    AComboBoxOS::deinitialize();
    ARichEditOS::deinitialize();
    AEditLineOS::deinitialize();

    // Deinitialize COM library if it was initialized
    deinitialize_com();
    }

  static bool g_com_inited = false;

  //---------------------------------------------------------------------------------------
  // Initialized the Component Object Model (COM) library for use by the calling
  //             thread, sets the thread's concurrency model, and creates a new apartment
  //             for the thread if one is required.
  // Modifiers:   static
  // Author(s):   Conan Reis
  void initialize_com()
    {
    if (!g_com_inited)
      {
      ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
      g_com_inited = true;
      }
    }

  //---------------------------------------------------------------------------------------
  // Deinitialize COM library if it was initialized.
  // Examples:   Called by system.
  // Modifiers:   static
  // Author(s):   Conan Reis
  void deinitialize_com()
    {
    if (g_com_inited)
      {
      ::CoUninitialize();
      g_com_inited = false;
      }
    }

  } // namespace AgogGUI_OS
