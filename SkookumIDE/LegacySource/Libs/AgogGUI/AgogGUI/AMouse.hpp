// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//=======================================================================================


#ifndef __AMOUSE_HPP
#define __AMOUSE_HPP
#pragma once


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO\AInputTypes.hpp>
#include <AgogCore\ARegion.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Note:  These values must match the ones in winuser.h from IDC_ARROW - IDC_HELP.
//        They are the same values, but MAKEINTRESOURCE has not yet converted them.
// ***    Care must be taken in getting MS Win32 code updates and porting so that
//        these values are meaningful.
enum eACursorOS
  {
  ACursorOS_none            = 0,      // No system cursor
  ACursorOS__first          = 32512,
  ACursorOS_arrow           = 32512,  // Standard arrow 
  ACursorOS_i_beam          = 32513,  // I-beam 
  ACursorOS_busy            = 32514,  // Hourglass 
  ACursorOS_cross           = 32515,  // Crosshair 
  ACursorOS_up_arrow        = 32516,  // Vertical arrow 
  ACursorOS_size_nwse       = 32642,  // Double-pointed arrow pointing northwest and southeast 
  ACursorOS_size_nesw       = 32643,  // Double-pointed arrow pointing northeast and southwest 
  ACursorOS_size_we         = 32644,  // Double-pointed arrow pointing west and east 
  ACursorOS_size_ns         = 32645,  // Double-pointed arrow pointing north and south 
  ACursorOS_size_all        = 32646,  // Four-pointed arrow pointing north, south, east, and west 
  ACursorOS_no              = 32648,  // Slashed circle 
  ACursorOS_hand            = 32649,  // Windows NT 5.0 and later: Hand 
  ACursorOS_busy_background = 32650,  // Standard arrow and small hourglass 
  ACursorOS_help            = 32651,  // Arrow and question mark 
  ACursorOS__last           = 32651,
  ACursorOS_unmodified      = ADef_int // Leaves the cursor as is - no change
  };


//---------------------------------------------------------------------------------------
// Notes    AMouse input object
//          AWindow in AgogGUI has mouse input event methods that can be overridden.
// Author   Conan Reis
class AMouse
  {
  public:

  // Class Methods

    // Capabilities

      static uint    get_button_count();
      static eAMouse get_button_flags();
      static bool    is_wheel_present();

    // State

      static eAMouse get_buttons();
      static AVec2i  get_position();
      static void    set_position(const AVec2i & pos);
      static void    get_constrain(ARegion * screen_region_p);
      static void    set_constrain(const ARegion & screen_region);
      static void    remove_constrain();

    // Cursor Methods

      static void     enable_cursor(bool show = true);
      static bool     is_cursor_enabled();
      static uint32_t get_cursor_id();
      static void     set_cursor_id(uint32_t resource_id = ACursorOS_arrow, bool jiggle = false);

  protected:

  // Class Data Members

    static bool ms_cursor_visible;

  };  // AMouse


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Determines whether the mouse cursor is being drawn or not.
// Returns:    true if drawn, false if not
// Author(s):   Conan Reis
inline bool AMouse::is_cursor_enabled()
  {
  return ms_cursor_visible;
  }


#endif  // __AMOUSE_HPP


