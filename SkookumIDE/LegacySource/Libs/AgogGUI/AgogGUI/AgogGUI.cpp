// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Components common to the AgogGUI library - including all class data
//              members that require a specific initialization sequence.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AFont.hpp>
#include <AgogGUI\AWindow.hpp>


//---------------------------------------------------------------------------------------
// Link required libraries
// $Note - CReis These will only link if this module (cpp file) contains other used code.
#if defined(_MSC_VER)
  // Shell32.lib - (linked by AgogIO\AgogIO.lib) Used by AWindow (drag & drop)
  // User32.lib  - (linked by AgogIO\AgogIO.lib) Used by AWindow, ADisplay, AMouse
  #pragma comment(lib, "GDI32.lib")       // Used by AFont, ATrueTypeFont
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
// AgogGUI
//=======================================================================================

namespace AgogGUI
  {

  //---------------------------------------------------------------------------------------

  void initialize()
    {
    AFont::initialize();
    AWindow::initialize();
    ADisplay::initialize();
    }

  //---------------------------------------------------------------------------------------

  void deinitialize()
    {
    ADisplay::deinitialize();
    AWindow::deinitialize();
    AFont::deinitialize();
    }

  }

//=======================================================================================
// AFont Class Data Members
//=======================================================================================

AFont * AFont::ms_default_p;
AFont * AFont::ms_narrow_p;
AFont * AFont::ms_header1_p;
AFont * AFont::ms_header2_p;
AFont * AFont::ms_fixed_p;