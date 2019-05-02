// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AMouse
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AMouse.hpp>
#include <AgogCore\AMath.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

namespace
{

  enum
    {
    AMouse_key_test_shift   = 15,
    AMouse_key_test_bit     = 1 << AMouse_key_test_shift,
    AMouse_btn_left_shift   = AMouse_key_test_shift - 0,  // These bits correspond to the values in eAMouse
    AMouse_btn_right_shift  = AMouse_key_test_shift - 1,
    AMouse_btn_middle_shift = AMouse_key_test_shift - 4,
    AMouse_btn_x1_shift     = AMouse_key_test_shift - 5,
    AMouse_btn_x2_shift     = AMouse_key_test_shift - 6
    };

} // End unnamed namespace


//=======================================================================================
// Class Data Members
//=======================================================================================

bool AMouse::ms_cursor_visible = true;


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Shows/hides the mouse cursor.
// Arg         show - if true the mouse cursor is shown, if false it is hidden.  (Default true)
// Author(s):   Conan Reis
void AMouse::enable_cursor(
  bool show // = true
  )
  {
  ms_cursor_visible = show;

  // Some mouse drivers do not show or hide on the first attempt.
  if (show)
    {
    while(::ShowCursor(true) < 0)
      {
      }
    }
  else
    {
    while(::ShowCursor(false) >= 0)
      {
      }
    }
  }

//---------------------------------------------------------------------------------------
// Gets number of mouse buttons.
// Returns:    0 - No mouse installed
//             1 - Left mouse button present
//             2 - Left & Right mouse buttons present
//             3 - Left, Right, & Middle mouse buttons present
//             4 - Left, Right, Middle, & X1 mouse buttons present
//             5 - Left, Right, Middle, X1, & X2 mouse buttons present
// See:        get_button_flags(), is_wheel_present()
// Modifiers:   static
// Author(s):   Conan Reis
uint AMouse::get_button_count()
  {
  // $Note - CReis This does not seem to be entirely accurrate.  My Logitech Cordless Mouseman
  // Optical reports having 5 buttons when it only has 4.
  return ::GetSystemMetrics(SM_CMOUSEBUTTONS);
  }

//---------------------------------------------------------------------------------------
// Gets mouse button capability flags.
// Returns:    Flags as specified by eAgogGUI\AMouse.  If it returns AMouse__none then no
//             mouse is installed.
// See:        get_button_count(), is_wheel_present()
// Modifiers:   static
// Author(s):   Conan Reis
eAMouse AMouse::get_button_flags()
  {
  eAMouse buttons[5]   = {AMouse__none, AMouse_left, AMouse_l_r, AMouse_l_r_m, AMouse_l_r_m_x1};
  int      button_count = ::GetSystemMetrics(SM_CMOUSEBUTTONS);

  // $Note - CReis This does not seem to be entirely accurrate.  My Logitech Cordless Mouseman
  // Optical reports having 5 buttons when it only has 4.

  return (button_count < 5)
    ? buttons[button_count]
    : AMouse_l_r_m_x1_x2;
  }

//---------------------------------------------------------------------------------------
// Gets the pressed state of all the mouse buttons.
// Returns:    eAMouse flags
// Modifiers:   static
// Author(s):   Conan Reis
eAMouse AMouse::get_buttons()
  {
  return eAMouse(
    ((::GetKeyState(VK_LBUTTON)  & AMouse_key_test_bit) >> AMouse_btn_left_shift) |
    ((::GetKeyState(VK_RBUTTON)  & AMouse_key_test_bit) >> AMouse_btn_right_shift) |
    ((::GetKeyState(VK_MBUTTON)  & AMouse_key_test_bit) >> AMouse_btn_middle_shift) |
    ((::GetKeyState(VK_XBUTTON1) & AMouse_key_test_bit) >> AMouse_btn_x1_shift) |
    ((::GetKeyState(VK_XBUTTON2) & AMouse_key_test_bit) >> AMouse_btn_x2_shift));
  }

//---------------------------------------------------------------------------------------
// Gets the screen region that the mouse movement is confined confined to.
//             If the cursor is not constrained, screen_region_p will be set to the
//             dimensions of the screen.
// Arg         screen_region_p - pointer to region to store constraints
// Modifiers:   static
// Author(s):   Conan Reis
void AMouse::get_constrain(ARegion * screen_region_p)
  {
  RECT rect;

  ::GetClipCursor(&rect);
  screen_region_p->m_x      = rect.left;
  screen_region_p->m_y      = rect.top;
  screen_region_p->m_width  = rect.right - rect.left;
  screen_region_p->m_height = rect.bottom - rect.top;
  }

//---------------------------------------------------------------------------------------
// Gets x and y screen co-ordinates of mouse
// Returns:    screen co-ordinate of mouse
// Examples:   AVec2i pos(AMouse::get_position());
// Modifiers:   static
// Author(s):   Conan Reis
AVec2i AMouse::get_position()
  {
  AVec2i pos;

  ::GetCursorPos(reinterpret_cast<POINT *>(&pos));
  
  return pos;
  }

//---------------------------------------------------------------------------------------
// Determines whether or not the mouse has a wheel.  If it has a wheel then
//             the AWindow::on_mouse_spinning() event will be called when it is spun.
// Returns:    bool
// See:        get_button_count(), get_button_flags()
// Modifiers:   static
// Author(s):   Conan Reis
bool AMouse::is_wheel_present()
  {
  return (::GetSystemMetrics(SM_MOUSEWHEELPRESENT) != 0);
  }

//---------------------------------------------------------------------------------------
// Removes constraints (if any present) from mouse cursor movement.
// Modifiers:   static
// Author(s):   Conan Reis
void AMouse::remove_constrain()
  {
  ::ClipCursor(nullptr);
  }

//---------------------------------------------------------------------------------------
// Sets the screen region to confine mouse movement to.  If a subsequent
//             cursor position (via set_position() or user input) lies outside the region,
//             the cursor position will be automatically adjusted to keep it inside the
//             restricted region.
// Arg         screen_region - region to restrict movement to
// Modifiers:   static
// Author(s):   Conan Reis
void AMouse::set_constrain(const ARegion & screen_region)
  {
  RECT rect = {screen_region.m_x, screen_region.m_y, screen_region.m_x + screen_region.m_width, screen_region.m_y + screen_region.m_height};

  ::ClipCursor(&rect);
  }

//---------------------------------------------------------------------------------------
// Sets the system mouse cursor to the specified cursor resource id.
// Arg         resource_id - resource id from a Win32 resource.  See eACursorOS for
//             standard cursors.  A value of ACursorOS_none (0) means no system mouse
//             cursor should be drawn.  A value of ACursorOS_unmodified leaves the system
//             cursor as it is.
//             Do not use MAKEINTRESOURCE, it is used automatically inside this method.
//             (Default ACursorOS_arrow)
// Arg         jiggle - set this to true if the cursor change does not seem to take place
//             immediately.  The cursor is sometimes not redrawn unless it moves - this
//             sets the cursor to its current position and 'wakes' it up.  (Default false)
//             [Warning: Using the 'jiggle' setting may resend the message that made this
//             call to set_cursor_id() which could cause a redundant loop.]
// Examples:   AMouse::set_cursor_id(ACursorOS_i_beam);
// Author(s):   Conan Reis
void AMouse::set_cursor_id(
  uint32_t resource_id, // = ACursorOS_arrow
  bool jiggle       // = false
  )
  {
  if (resource_id != ACursorOS_unmodified)
    {
    HINSTANCE app_instance = a_is_ordered(ACursorOS__first, static_cast<eACursorOS>(resource_id), ACursorOS__last) ? nullptr : AApplication::ms_res_instance;
    HCURSOR   cursor       = resource_id ? LoadCursor(app_instance, MAKEINTRESOURCE(resource_id)) : nullptr;

    // This sets the cursor immediately (no need to wait for event loop).
    ::SetCursor(cursor);  

    if (jiggle)
      {
      // This may make the cursor change happen immediately if the system is waiting for
      // a mouse move before the cursor is redrawn.

      POINT point;
      ::GetCursorPos(&point);
      ::SetCursorPos(point.x, point.y);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Sets x and y screen co-ordinates of mouse
// Arg         pos - screen co-ordinates
// Notes:      If the new coordinates are not within the screen region set by the most
//             recent set_constrain() function, the position will be automatically
//             adjusted to keep the cursor inside it. 
// Modifiers:   static
// Author(s):   Conan Reis
void AMouse::set_position(const AVec2i & pos)
  {
  ::SetCursorPos(pos.m_x, pos.m_y);
  }


