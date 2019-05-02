// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AWindow class definition module
// Notes:          Simple graphical window.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogCore\AMath.hpp>
#include <AgogIO\AIni.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>            // Uses: HDROP
#include <stdlib.h>              // Uses: _MAX_FILE
#include <commctrl.h>
#include <winuser.h>


#if defined(_MSC_VER)
  // Removes unnecessary warnings - often only part of a structure needs to be initialized
  #pragma warning( disable : 4701 ) // local variable may be used without having been initialized
#endif


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

#define AWIN_LO_INT16(_int32)       (int16_t(uint32_t(_int32) & 0xffff))
#define AWIN_HI_INT16(_int32)       (int16_t(uint32_t(_int32) >> 16))
#define AWIN_GET_X(_l_param)        (int(int16_t(LOWORD(_l_param))))
#define AWIN_GET_Y(_l_param)        (int(int16_t(HIWORD(_l_param))))
#define AWIN_GET_XBUTTON(_w_param)  ((HIWORD(_w_param) == XBUTTON1) ? AMouse_x1 : AMouse_x2)


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // AWindow enumerated constants
  enum
    {
    AWin_sys_command_code_mask  = 0xFFF0,
    AWin_command_code_menu      = 0,
    AWin_command_code_accel_key = 1,

    AWin_key_extended_flag      = 0x01000000,  // Specifies whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key keyboard
    AWin_key_repeat_flag        = 0x40000000
    };
  
  const uint32_t AWin_drag_query_file_count = 0xFFFFFFFF;

} // End unnamed namespace


//=======================================================================================
// Class Data Members
//=======================================================================================

const ARegion         AWindow::ms_region_def(AWin_def_x, AWin_def_y, AWin_def_width, AWin_def_height);
const ARegion         AWindow::ms_region_auto_def(0, 0, Size_auto, Size_auto);
const AWindow::View   AWindow::ms_view_def(ms_region_def);
const AWindow::View   AWindow::ms_view_hidden(ms_region_def, AShowState_hidden);
const AString         AWindow::ms_ini_view_strs[IniView__max];
AToolTipOS *        (*AWindow::ms_tip_create_f)(AWindow * parent_p, const AString & text, const AFont & font);

AMessageTargetClass * AWindow::ms_default_class_p;

//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AWindow::initialize()
  {
  ms_default_class_p = new AMessageTargetClass(AMessageTargetClass::Flags__default);

  // Initialize static variables
  const_cast<AString&>(ms_ini_view_strs[IniView_section])        = "View";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_x])          = "X";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_y])          = "Y";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_width])      = "Width";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_height])     = "Height";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_show_state]) = "ShowState";
  const_cast<AString&>(ms_ini_view_strs[IniView_key_display])    = "Display";
  }

//---------------------------------------------------------------------------------------

void AWindow::deinitialize()
  {
  const_cast<AString&>(ms_ini_view_strs[IniView_section])        = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_x])          = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_y])          = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_width])      = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_height])     = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_show_state]) = AString::ms_empty;
  const_cast<AString&>(ms_ini_view_strs[IniView_key_display])    = AString::ms_empty;

  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor - registers with OS
// Returns:    itself
// Arg         view - region (position and area) and initial show state of window.
// Arg         parent_p - a parent AWindow to this object or nullptr if this object
//             has no parent.  set_parent() may be used to modify the parent state of
//             this object.  (Default nullptr)
// Arg         class_p - pointer to a class defining a set of AWindow objects.
//             It must persist for the duration of this object or any other
//             AWindow object that uses it.  If nullptr is given, a default
//             graphical class is used.  (Default nullptr)
// See:        set_parent(), is_parent()
// Notes:      For each of the Win32 window styles there is either an associated method
//             or style is set according to the characteristics of the AWindow
// Author(s):   Conan Reis
AWindow::AWindow(
  const View &          view,             // = ms_view_def
  AWindow *             parent_p,         // = nullptr
  AMessageTargetClass * class_p           // = nullptr
  ) :
  AMessageTarget(
    parent_p,
    class_p ? class_p : ms_default_class_p,
    false),
  m_min_area(ADef_int, ADef_int),
  m_max_area(ADef_int, ADef_int),
  m_flags(Flags__default),
  m_color_bg(AColor::ms_default)
  {
  register_with_os(view, parent_p);
  }

//---------------------------------------------------------------------------------------
// Constructor - Registers with OS
//
//              This constructor allows creation of an WS_OVERLAPPED AWindow.
//
//              This variant of the AWindow constructor allows for creation of Window using
//              only the specified window and extended window styles. There is no assumption
//              of parent to child relationship i.e. no WS_CHILD, WS_POPUP or WS_CLIPCHILDREN.
//
//              The importance of this bland constructor is to allow capability to create an
//              AWindow establishing a parent-child relationship that is designating the parent
//              as an "owner" and the child window as not being contained or restricted by the
//              owner/parent.
//
//              This allows the "child" window to float above the "owner" without the child
//              rendering into the owner's client (or non-client) area but propagates/processes
//              message in a regular child-parent relationship. In other words, for example,
//              as the parent minimizes, sizes or closes, so does the child.
//
//              This constructor was specifically setup for implementing the SkIncrementalSearchEditbox.
//
//  See SkIncrementalSearchEditbox
//
//  Returns:        itself
//  Author(s):      John Stenersen
AWindow::AWindow(
  HWND      owner_hwnd,
  uint32_t  extended_window_style,
  uint32_t  window_style            // = WS_OVERLAP
  ) :
  AMessageTarget(
    nullptr,
    ms_default_class_p,
    false),
  m_min_area(ADef_int, ADef_int),
  m_max_area(ADef_int, ADef_int),
  m_flags(Flags__default),
  m_color_bg(AColor::ms_default)
  {
    m_os_handle = ::CreateWindowEx(
      extended_window_style,      // Extended Window Styles
      reinterpret_cast<const char *>(m_class_p->get_atom_id()),  // Using the name of the class is also valid, but using the atom id should be faster
      "",                         //  Title
      window_style,               //  Window Style
      0, 0, 0, 0,                 //  Position, Size
      owner_hwnd,                 //  Window Owner/Parent
      nullptr,                    //  Menu id
      AApplication::ms_instance,  //  Instance
      nullptr);                   //  l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "", AWindow);

  common_setup();
  }

//---------------------------------------------------------------------------------------
// Constructor - doesn't register with OS.  This permits the calling of the
//             CreateWindowEx() API function by a derived subclass constructor.
// Returns:    itself
// Arg         parent_p - a parent AWindow to this object or nullptr if this object
//             has no parent.  set_parent() may be used to modify the parent state of
//             this object.
// Arg         class_p - pointer to a class defining a set of AWindow objects.
//             It must persist for the duration of this object or any other
//             AWindow object that uses it.  If nullptr is given, a default
//             graphical class is used.
// See:        set_parent(), is_parent(), register_with_os()
// Notes:      For each of the Win32 window styles there is either an associated method
//             or style is set according to the characteristics of the AWindow
// Author(s):   Conan Reis
AWindow::AWindow(
  AWindow *             parent_p,  // = nullptr
  AMessageTargetClass * class_p
  ) :
  AMessageTarget(
    parent_p,
    class_p ? class_p : ms_default_class_p,
    false),
  m_min_area(ADef_int, ADef_int),
  m_max_area(ADef_int, ADef_int),
  m_flags(Flags__default),
  m_color_bg(AColor::ms_default)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor - wrapper for existing window handle created via some other
//             non-Agog library.
// Returns:    itself
// Arg         preexisting_handle - an already existing window handle for this object to
//             wrap around.
// Notes:      ##### Use this wrapper functionality with caution.  A number of methods
//             will not work - in particular the event methods.  It is most handy for
//             the get_*() and set_*() type of methods. #####
// Author(s):   Conan Reis
AWindow::AWindow(HWND preexisting_handle) :
  AMessageTarget(preexisting_handle),
  m_min_area(ADef_int, ADef_int),
  m_max_area(ADef_int, ADef_int),
  m_flags(Flags__default),
  m_color_bg(AColor::ms_default)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
// Notes:      Some windows/controls need to call destroy() in their own destructor
//             rather than letting the AMessageTarget destructor call it since destroy()
//             will end up sending windows messages and the windows/controls need to have
//             their virtual table still intact.
// Author(s):   Conan Reis
AWindow::~AWindow()
  {
  destroy();
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Misc. Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Enables or disables the acceptance of drag and drop files from the
//              MS Windows File Explorer on to this window.
// Arg          accept_files - specifies whether to accept (true) or reject (false) files
//              that are dragged and dropped on to this window.  (Default true)
// See:         on_drag_drop(), on_drag_drop_begin(), on_drag_drop_end()
// Notes:       Whenever a file (or files) is dropped on to this window when drag and
//              drop is accepted, position where the drop occurs on the window and the
//              name of the file are passed on a call to on_drag_drop().  If multiple
//              files are dropped on the window simultaneously, the on_drag_drop() event
//              method is called more than once in succession with one file name at a
//              time.
// Author(s):    Conan Reis
void AWindow::enable_drag_drop(
  bool accept_files // = true
  )
  {
  // $Revisit - CReis Determine behaviour of this setting with respect to parent and child windows.
  ::DragAcceptFiles(m_os_handle, accept_files);
  }

//---------------------------------------------------------------------------------------
// Enables or disables mouse and keyboard input to the specified window or
//             control. When input is disabled, the window does not receive input such
//             as mouse clicks and key presses.  When input is enabled, the window
//             receives all input. 
// Arg         input_accepted - specifies whether to accept input (true) or reject
//             input (false).  (Default true)
// See:        is_input_enabled(), enable_input_all()
// Author(s):   Conan Reis
void AWindow::enable_input(
  bool input_accepted // = true
  )
  {
  ::EnableWindow(m_os_handle, input_accepted);
  }

//---------------------------------------------------------------------------------------
//  Returns whether the window is in a hidden show state or not
// Returns:     true if hidden, false if visible (it still may be obscured)
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//              minimize(), restore(), set_show_state(), show()
// Author(s):    Conan Reis
bool AWindow::is_hidden() const
  {
  return (::IsWindowVisible(m_os_handle) == 0);
  }

//---------------------------------------------------------------------------------------
// Returns whether keyboard and mouse input can be received or not.
// Returns:    true if input is accepted, false if not.
// See:        enable_input(), is_focused()
// Notes:      A window must have its input enabled before it can be "activated".
// Author(s):   Conan Reis
bool AWindow::is_input_enabled() const
  {
  return (::IsWindowEnabled(m_os_handle) != 0);
  }

//---------------------------------------------------------------------------------------
//  Determines if the window has a menu associated with it or not
// Returns:     true if has menu, false if not
// Author(s):    Conan Reis
bool AWindow::is_menued() const
  {
  return ::GetMenu(m_os_handle) != nullptr;
  }

//---------------------------------------------------------------------------------------
//  Determines if user sizing by border is enabled.
// Returns:     true if border sizing enabled, false if not.
// See:         enable_sizing()
// Author(s):    Conan Reis
bool AWindow::is_sizable() const
  {
  return is_style(WS_THICKFRAME);
  }

//---------------------------------------------------------------------------------------
//  Determines if this window is above all non-topmost windows in the Z-order
// Returns:     true if topmost, false if not.
// See:         enable_topmost()
// Author(s):    Conan Reis
bool AWindow::is_topmost() const
  {
  return is_style(0, WS_EX_TOPMOST);
  }


//---------------------------------------------------------------------------------------
//  Sets the keyboard focus to this window
// Examples:    window.set_focus();
// Author(s):    Conan Reis
HWND AWindow::set_focus()
  {
  return ::SetFocus(m_os_handle);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position / Size Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converts client area to window area that would the same client area with
//             this window's current styles.
// Modifiers:   static
// Author(s):   Conan Reis
AVec2i AWindow::client2window_area(const AVec2i & client_area)
  {
  return client2window_area(
      uint(::GetWindowLongPtr(m_os_handle, GWL_STYLE)),
      uint(::GetWindowLongPtr(m_os_handle, GWL_EXSTYLE)),
      is_menued(),
      client_area);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AVec2i AWindow::get_area() const
  {
  RECT region;

  ::GetWindowRect(m_os_handle, &region);

  return AVec2i(region.right - region.left, region.bottom - region.top);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AVec2i AWindow::get_area_client() const
  {
  RECT region;

  ::GetClientRect(m_os_handle, &region);
  
  return AVec2i(region.right, region.bottom);
  }

//---------------------------------------------------------------------------------------
// Get aspect ratio of window - width / height
// Author(s):   Conan Reis
f32 AWindow::get_aspect() const
  {
  RECT region;

  ::GetWindowRect(m_os_handle, &region);

  return f32(region.right - region.left) / f32(region.bottom - region.top);
  }

//---------------------------------------------------------------------------------------
// Get aspect ratio of client area - width / height
// Author(s):   Conan Reis
f32 AWindow::get_aspect_client() const
  {
  RECT region;

  ::GetClientRect(m_os_handle, &region);

  return f32(region.right) / f32(region.bottom);
  }

//---------------------------------------------------------------------------------------
//  returns the height of the window, including non-client area
// Returns:     the height of the window
// Examples:    height = window.get_height()
int AWindow::get_height() const
  {
  RECT region;

  ::GetWindowRect(m_os_handle, &region);
  return region.bottom - region.top;
  }

//---------------------------------------------------------------------------------------
//  returns the height of the window's client area
// Returns:     the height of the window's client area
// Examples:    height = window.get_height_client()
int AWindow::get_height_client() const
  {
  RECT region;

  ::GetClientRect(m_os_handle, &region);
  return region.bottom;
  }

//---------------------------------------------------------------------------------------
AVec2i AWindow::get_position() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  
  return AVec2i(win_rect.left, win_rect.top);
  }

//---------------------------------------------------------------------------------------
AVec2i AWindow::get_position_rel() const
  {
  if (m_parent_handle)
    {
    POINT client_origin = {0, 0};

    ::ClientToScreen(m_parent_handle, &client_origin);

    RECT client_rect;

    ::GetWindowRect(m_os_handle, &client_rect); // Get client's screen coords
    
    return AVec2i(client_rect.left - client_origin.x, client_rect.top - client_origin.y);
    }
  else
    {
    RECT win_rect;

    ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.

    return AVec2i(win_rect.left, win_rect.top);
    }
  }

//---------------------------------------------------------------------------------------
//  Returns the current region occupied by this window. The region dimensions
//              are relative to the upper-left corner of the screen.
// Returns:     current region occupied by this window
// Examples:    ARegion dimensions = window.get_region()
// See:         get_region_normal()
// Notes:       If this window is in a show state other than normal (i.e. minimized or
//              maximized) the region is smaller or larger than normal accordingly.
//              If the region occupied by the window regardless of its current show state
//              is desired call the get_region_normal() method.
// Author(s):    Conan Reis
ARegion AWindow::get_region() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  return ARegion(win_rect.left, win_rect.top, win_rect.right - win_rect.left, win_rect.bottom - win_rect.top);      
  }

//---------------------------------------------------------------------------------------
//  Returns the client region of the window. The position will always be 0,0
// Returns:     current client region of this window
// Examples:    ARegion client_region = window.get_region_client()
// Author(s):    Conan Reis
ARegion AWindow::get_region_client() const
  {
  RECT win_rect;

  ::GetClientRect(m_os_handle, &win_rect); // Get screen coords.
  return ARegion(0, 0, win_rect.right, win_rect.bottom);      
  }

//---------------------------------------------------------------------------------------
//  Returns the region occupied by this window while in the normal/restored
//              show state (i.e. AShowState_normal or AShowState_normal_focus) regardless
//              of what show state the window is currently in.  The region dimensions are
//              relative to the upper-left corner of the screen.
// Returns:     region occupied in the normal/restored show state
// Examples:    ARegion dimensions = window.get_region()
// See:         get_region()
// Notes:       If the current region is desired (i.e. normal, minimized, maximized),
//              call the get_region() method.
// Author(s):    Conan Reis
ARegion AWindow::get_region_normal() const
  {
  WINDOWPLACEMENT win_placement;

  win_placement.length = sizeof(WINDOWPLACEMENT);
  win_placement.flags  = 0;

  // $Note - CReis This is a trick to get GetWindowPlacement() return screen co-ordinates
  // rather than workspace co-ordinates.  This is done by temporarily giving the window
  // the WS_EX_TOOLWINDOW style.  If this was not done, the window would "creep" up or
  // left of its previous screen position if the taskbar was at the top or left of the
  // screen and not hidden.  Note that ClientToScreen() will not convert the points
  // properly.
  LONG_PTR style = ::GetWindowLongPtr(m_os_handle, GWL_EXSTYLE);           // Get current extended styles

  ::SetWindowLongPtr(m_os_handle, GWL_EXSTYLE, style | WS_EX_TOOLWINDOW);  // Add WS_EX_TOOLWINDOW temporarily to extended styles
  ::GetWindowPlacement(m_os_handle, &win_placement);                       // Get all screen coords.
  ::SetWindowLongPtr(m_os_handle, GWL_EXSTYLE, style);                     // Put extended styles back the way they were

  return ARegion(
    win_placement.rcNormalPosition.left,
    win_placement.rcNormalPosition.top,
    win_placement.rcNormalPosition.right - win_placement.rcNormalPosition.left,
    win_placement.rcNormalPosition.bottom - win_placement.rcNormalPosition.top);      
  }

//---------------------------------------------------------------------------------------
//  returns the width of the window, including non-client area
// Returns:     the width of the window
// Examples:    int height = window.get_width()
int AWindow::get_width() const
  {
  RECT region;

  ::GetWindowRect(m_os_handle, &region);
  return region.right - region.left;
  }

//---------------------------------------------------------------------------------------
//  returns the width of the window's client area
// Returns:     the width of the window's client area
// Examples:    int height = window.get_width_client()
int AWindow::get_width_client() const
  {
  RECT region;

  ::GetClientRect(m_os_handle, &region);
  return region.right;
  }

//---------------------------------------------------------------------------------------
//  Returns the x coord of the window's left side relative to the screen
// Returns:     The x coord of the window's left side as an int.
// Examples:    int left = window.get_x()
int AWindow::get_x() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  return (win_rect.left);      
  }

//---------------------------------------------------------------------------------------
// Returns the x coord of the window's right side relative to the screen
// Returns:    The x coord of the window's right side
// Notes:      This method is handy for positioning graphical elements relative to this
//             window
// Author(s):   Conan Reis
int AWindow::get_right() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  return win_rect.right;      
  }

//---------------------------------------------------------------------------------------
// Returns the x value of the window's left side relative to it's parent if
//             if there is one or the x screen value if it has no parent
int AWindow::get_x_rel() const
  {
  RECT client_rect;

  ::GetWindowRect(m_os_handle, &client_rect);           // Get client's screen coords

  if (m_parent_handle)
    {
    POINT client_origin = {0, 0};

    ::ClientToScreen(m_parent_handle, &client_origin);  // Get the client origin in screen coords
    return (client_rect.left - client_origin.x);        // return the difference
    }
  return client_rect.left;
  }

//---------------------------------------------------------------------------------------
// Returns the x value of the window's right side relative to it's parent if
//             if there is one or the x screen value if it has no parent
int AWindow::get_right_rel() const
  {
  RECT client_rect;

  ::GetWindowRect(m_os_handle, &client_rect);           // Get client's screen coords

  if (m_parent_handle)
    {
    POINT client_origin = {0, 0};

    ::ClientToScreen(m_parent_handle, &client_origin);  // Get the client origin in screen coords
    return (client_rect.right - client_origin.x);       // return the difference
    }
  return client_rect.right;
  }

//---------------------------------------------------------------------------------------
//  Returns the x coord of the window's top side relative to the screen
// Returns:     The x coord of the window's top side as an int.
// Examples:    int top = window.get_y()
int AWindow::get_y() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  return (win_rect.top);      
  }

//---------------------------------------------------------------------------------------
// Returns the x coord of the window's bottom side relative to the screen
// Returns:    The x coord of the window's bottom side
// Notes:      This method is handy for positioning graphical elements relative to this
//             window
// Author(s):   Conan Reis
int AWindow::get_bottom() const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect); // Get screen coords.
  return win_rect.bottom;      
  }

//---------------------------------------------------------------------------------------
// Returns the y value of the window's top side relative to it's parent if
//             if there is one or the y screen value if it has no parent
int AWindow::get_y_rel() const
  {
  RECT client_rect;

  ::GetWindowRect(m_os_handle, &client_rect);           // Get client's screen coords

  if (m_parent_handle)
    {
    POINT client_origin = {0, 0};

    ::ClientToScreen(m_parent_handle, &client_origin);  // Get the client origin in screen coords
    return (client_rect.top - client_origin.y);         // return the difference
    }
  return client_rect.top;
  }

//---------------------------------------------------------------------------------------
// Returns the y value of the window's bottom side relative to it's parent if
//             if there is one or the y screen value if it has no parent
int AWindow::get_bottom_rel() const
  {
  RECT client_rect;

  ::GetWindowRect(m_os_handle, &client_rect);           // Get client's screen coords

  if (m_parent_handle)
    {
    POINT client_origin = {0, 0};

    ::ClientToScreen(m_parent_handle, &client_origin);  // Get the client origin in screen coords
    return (client_rect.bottom - client_origin.y);      // return the difference
    }
  return client_rect.bottom;
  }

//---------------------------------------------------------------------------------------
// Determine if given screen co-ordinates are enclosed by the window
// Returns:    true if the co-ordinates are surrounded by the window, false
//             if not.
// Arg         pos - screen co-ordinate
// Examples:   bool enclosed = window.is_in(pos);
// Author(s):   Conan Reis
bool AWindow::is_in(const AVec2i & pos) const
  {
  RECT win_rect;

  ::GetWindowRect(m_os_handle, &win_rect);  // Get screen co-ords
  if ((win_rect.left <= pos.m_x) && (win_rect.right >= pos.m_x) && (win_rect.top <= pos.m_y) && (win_rect.bottom >= pos.m_y))
    {
    return true;
    }
  return false;
  }

//---------------------------------------------------------------------------------------
// Loads previously stored region from the supplied configuration/.ini file,
//             or if the values do not yet exist in the file, the default values will be
//             written to the file and returned.  If the values stored in the
//             configuration file are not appropriate (i.e. cannot grab a window with
//             the mouse) for the current screen dimensions, the window is shifted
//             towards the center of the screen and new values are written to the
//             configuration file.
// Returns:    region stored in ini file or specified default if it did not yet exist in
//             the file
// Arg         section_cstr_p - unique section for the window so that its values may be
//             distinguished from the values for other windows.
// Arg         default_region - Default region for the window to occupy if the
//             configuration file does not yet have any values stored in it.
// Arg         pos_only - if true only load (and save) position, if false also load
//             width and height
// Arg         config_file_p - Pointer to a configuration/.ini file to use.
// See:        ini_load_show_state(), ini_restore_region(), ini_restore_show_state(),
//             ini_restore_view(), ini_save_view()
// Modifiers:   static
// Author(s):   Conan Reis
ARegion AWindow::ini_load_region(
  const char *    section_cstr_p,
  const ARegion & default_region, // = ARegion(0, 0, 800, 600)
  bool            pos_only,       // = false
  AIni *          config_file_p   // = AApplication::ms_ini_file_p
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load (& Save default) Info From Ini
  ARegion region(default_region);
  
  region.m_x = config_file_p->get_value_int_default(default_region.m_x, ms_ini_view_strs[IniView_key_x], section_cstr_p);
  region.m_y = config_file_p->get_value_int_default(default_region.m_y, ms_ini_view_strs[IniView_key_y], section_cstr_p);

  if (!pos_only)
    {
    region.m_width  = config_file_p->get_value_int_default(default_region.m_width, ms_ini_view_strs[IniView_key_width], section_cstr_p);
    region.m_height = config_file_p->get_value_int_default(default_region.m_height, ms_ini_view_strs[IniView_key_height], section_cstr_p);
    }

  const ADisplay * display_p = ADisplay::find_device_num(config_file_p->get_value_int_default(ADisplay::get_info().m_device_num, ms_ini_view_strs[IniView_key_display], section_cstr_p), true);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Adjust Window Region

  // Adjust window position from being relative to its display to virtual screen co-ordinates.
  // This ensures that the placement of the window is still correct even if the display
  // has changed its position on the desktop.
  region.m_x += display_p->m_region.m_x;
  region.m_y += display_p->m_region.m_y;

  // Ensure that width is not too small and no more than the total width available
  const ARegion & desktop = ADisplay::get_virtual_region();

  region.constrain_area(IniView_selection_min, desktop.m_width, IniView_selection_min, desktop.m_height);

  // Adjust window if it is not visible so that it can be dragged and resized
  if (!ADisplay::is_visible(region))
    {
    ADisplay::hit_display(region)->m_region_work.snap_enclose(&region);
    }

  return region;
  }

//---------------------------------------------------------------------------------------
// Loads previously stored show state from the supplied configuration/.ini
//             file, or if the value does not yet exist in the file, the default value
//             will be written to the file and returned.
// Returns:    show state stored in ini file or specified default if it did not yet
//             exist in the file.
// Arg         section_cstr_p - unique section for the window so that its values may be
//             distinguished from the values for other windows.
// Arg         default_state - Default show state for the window if the configuration
//             file does not yet have any values stored in it.
// Arg         use_app_showstate - if true, the show state specified by the
//             run/execution starting values as indicated by WinMain() will override the
//             value in the configuration file - if overridden the window will generally
//             minimized or maximized.
// Arg         config_file_p - Pointer to a configuration/.ini file to use.
// See:        ini_load_region(), ini_restore_region(), ini_restore_show_state(),
//             ini_restore_view(), ini_save_view()
// Modifiers:   static
// Author(s):   Conan Reis
eAShowState AWindow::ini_load_show_state(
  const char * section_cstr_p,
  eAShowState  default_state,     // = AShowState_normal_focus
  bool         use_app_showstate, // = true
  AIni *       config_file_p      // = AApplication::ms_ini_file_p
  )
  {
  eAShowState show_state = AShowState_normal_focus;

  if (use_app_showstate)
    {
    // Allow window show state to be overridden by starting parameters - generally
    // minimized or maximized
    show_state = AApplication::ms_this_app_p->get_requested_show_state();
    }

  // If normal show state, load previous settings from configuration file.
  if (show_state == AShowState_normal_focus)
    {
    show_state = static_cast<eAShowState>(config_file_p->get_value_int_default(default_state, ms_ini_view_strs[IniView_key_show_state], section_cstr_p));
    }

  return show_state;
  }

//---------------------------------------------------------------------------------------
// Saves the window's view settings to a configuration/.ini file.  This
//             includes the region the window occupies in normal/restored view mode
//             and whether the window is hidden/minimized/normal/maximized and whether
//             the window is active or inactive.
// Arg         section_cstr_p - unique section for the window so that its values may be
//             distinguished from the values for other windows.
// Arg         pos_only - if true only save position, if false also save width and height
// Arg         config_file_p - Pointer to a configuration/.ini file to use.
//             (Default &AIni() - application filename with a .ini extension)
// Examples:   This should generally be called just prior to closing the window - i.e.
//             invoke it in the window's destructor method or somewhere similar.
// See:        ini_load_region(), ini_load_show_state(), ini_restore_region(),
//             ini_restore_show_state(), ini_restore_view()
// Author(s):   Conan Reis
void AWindow::ini_save_view(
  const char * section_cstr_p,
  bool         pos_only,     // = false
  AIni *       config_file_p // = &AIni()
  ) const
  {
  if (config_file_p->ensure_writable_query())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save the current window position
    ARegion          normal_region = get_region_normal();
    const ADisplay * display_p     = ADisplay::hit_display(*this);

    // Make the window region relative to the display that it is on.  This ensures that the
    // placement of the window is still correct even if the display changes its position on
    // the desktop.
    normal_region.m_x -= display_p->m_region.m_x;
    normal_region.m_y -= display_p->m_region.m_y;

    config_file_p->set_value(AString::ctor_int(normal_region.m_x), ms_ini_view_strs[IniView_key_x], section_cstr_p);
    config_file_p->set_value(AString::ctor_int(normal_region.m_y), ms_ini_view_strs[IniView_key_y], section_cstr_p);

    if (!pos_only)
      {
      config_file_p->set_value(AString::ctor_int(normal_region.m_width), ms_ini_view_strs[IniView_key_width], section_cstr_p);
      config_file_p->set_value(AString::ctor_int(normal_region.m_height), ms_ini_view_strs[IniView_key_height], section_cstr_p);
      }

    // Save the display device number that the window is on
    config_file_p->set_value(AString::ctor_int(display_p->m_device_num), ms_ini_view_strs[IniView_key_display], section_cstr_p);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save the current window show state
    config_file_p->set_value(AString::ctor_int(get_show_state()), ms_ini_view_strs[IniView_key_show_state], section_cstr_p);
    }
  }

//---------------------------------------------------------------------------------------
// sets the window to (x0, y0), (x1, y1) on the screen
//             Additionally lets user specify the z-order and special flags
// Arg         The window to insert after in the z-order
// Arg         region - position (relative to parent if it has one) and area of the window
// Arg         Additional flags (see notes)
// Examples:   window.set_order(nullptr, x0, y0, x1, y1);
// Notes:      If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
//
//             $Revisit - CReis The flags argument should be rewritten so that it does not use Win32
//             constants.
//             The coords are relative to the screen
//             The flags field has the following values:
//               Value              Meaning
//               SWP_ASYNCWINDOWPOS - If the calling thread and the thread that owns the window are attached to different input queues, the system posts the request to the thread that owns the window. This prevents the calling thread from blocking its execution while other threads process the request. 
//               SWP_DEFERERASE     - Prevents generation of the WM_SYNCPAINT message. 
//               SWP_DRAWFRAME      - Draws a frame (defined in the window's class description) around the window.
//               SWP_FRAMECHANGED   - Applies new frame styles set using the SetWindowLong function. Sends a WM_NCCALCSIZE message to the window, even if the window's size is not being changed. If this flag is not specified, WM_NCCALCSIZE is sent only when the window's size is being changed.
//               SWP_HIDEWINDOW     - Hides the window.
//               SWP_NOACTIVATE     - Does not activate the window. If this flag is not set, the window is activated and moved to the top of either the topmost or non-topmost group (depending on the setting of the hWndInsertAfter parameter).
//               SWP_NOCOPYBITS     - Discards the entire contents of the client area. If this flag is not specified, the valid contents of the client area are saved and copied back into the client area after the window is sized or repositioned.
//               SWP_NOMOVE         - Retains the current position (ignores X and Y parameters).
//               SWP_NOOWNERZORDER  - Does not change the owner window's position in the Z order.
//               SWP_NOREDRAW       - Does not redraw changes. If this flag is set, no repainting of any kind occurs. This applies to the client area, the nonclient area (including the title bar and scroll bars), and any part of the parent window uncovered as a result of the window being moved. When this flag is set, the application must explicitly invalidate or redraw any parts of the window and parent window that need redrawing.
//               SWP_NOSENDCHANGING - Prevents the window from receiving the WM_WINDOWPOSCHANGING message.
//               SWP_NOSIZE         - Retains the current size (ignores the cx and cy parameters).
//               SWP_NOZORDER       - Retains the current Z order (ignores the hWndInsertAfter parameter).
//               SWP_SHOWWINDOW     - Displays the window.
// Author(s):   Conan Reis
void AWindow::set_order(
  AWindow *       insert_after_p,
  const ARegion & region,
  uint32_t        flags
  )
  {
  // SetWindowPos sends WM_WINDOWPOSCHANGED -> WM_SIZE & WM_MOVE
  A_VERIFY_OS(
    ::SetWindowPos(
      m_os_handle,
      insert_after_p ? insert_after_p->get_os_handle() : nullptr,
      region.m_x, region.m_y, region.m_width, region.m_height,
      flags),
    "",
    AWindow);

  // MoveWindow sends WM_WINDOWPOSCHANGING -> [WM_GETMINMAXINFO] & WM_WINDOWPOSCHANGED -> WM_SIZE & WM_MOVE
  //::MoveWindow(m_os_handle, left, top, w, h, repaint_it);
  }

//---------------------------------------------------------------------------------------
// Sets the window to the specified height.
// Arg         height - height in pixels
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Examples:   window.set_height(200);
// Author(s):   Conan Reis
void AWindow::set_height(int height)
  {
  set_order(nullptr, ARegion(0, 0, get_width(), height), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
  }

//---------------------------------------------------------------------------------------
// Set position (relative to parent if it has one) of window.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
// Author(s):   Conan Reis
void AWindow::set_position(int x, int y)
  {
  set_order(nullptr, ARegion(x, y, 0, 0), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
  }

//---------------------------------------------------------------------------------------
// Set position (relative to parent if it has one) of window.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
// Author(s):   Conan Reis
void AWindow::set_position(const AVec2i & pos)
  {
  set_order(nullptr, ARegion(pos.m_x, pos.m_y, 0, 0), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
  }

//---------------------------------------------------------------------------------------
// Sets the position (relative to parent if it has one) and area of the window.
// Arg         x - x co-ordinate in pixels
// Arg         y - y co-ordinate in pixels
// Arg         width - width in pixels
// Arg         height - height in pixels
// Examples:   window.set_region(x, y, width, height);
// See:        set_region(region)
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Author(s):   Conan Reis
void AWindow::set_region(
  int x,
  int y,
  int width,
  int height
  )
  {
  set_region(ARegion(x, y, width, height));
  }

//---------------------------------------------------------------------------------------
// Sets the position (relative to parent if it has one) and area of the window.
// Arg         x - x co-ordinate in pixels
// Arg         y - y co-ordinate in pixels
// Arg         width - width in pixels
// Arg         height - height in pixels
// Examples:   window.set_region(region);
// See:        set_region(region)
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Author(s):   Conan Reis
void AWindow::set_region(const ARegion & region)
  {
  ARegion old_rgn(get_region());

  if (region != old_rgn)
    {
    if (region.get_area() != old_rgn.get_area())
      {
      if (region.get_pos() != old_rgn.get_pos())
        {
        set_order(nullptr, region, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
        }
      else
        {
        set_order(nullptr, region, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
        }
      }
    else
      {
      set_order(nullptr, region, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Sets the area of the window.
// Arg         area - New area in pixels.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Author(s):   Conan Reis
void AWindow::set_area(
  const AVec2i & area
  )
  {
  set_order(nullptr, area, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
  }

//---------------------------------------------------------------------------------------
// Sets the client area of the window.
// Arg         area - New area in pixels.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Author(s):   Conan Reis
void AWindow::set_area_client(
  const AVec2i & area
  )
  {
  set_order(
    nullptr,
    client2window_area(area),
    SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
  }

//---------------------------------------------------------------------------------------
// Sets the window to the specified width.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
//
//             If the size is changed, invalidate() may need to be called so that the
//             window is redrawn.
// Author(s):   Conan Reis
void AWindow::set_width(int width)
  {
  set_order(nullptr, ARegion(0, 0, width, get_height()), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_DRAWFRAME);
  }

//---------------------------------------------------------------------------------------
// Sets the position (relative to parent if it has one) of the left side of
//             the window.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
// Author(s):   Conan Reis
void AWindow::set_x(int x)
  {
  set_order(nullptr, ARegion(x, get_y_rel(), 0, 0), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
  }

//---------------------------------------------------------------------------------------
// Sets the position (relative to parent if it has one) of the top side of
//             the window.
// Notes:      The z-order relative to other windows, the activation state, and the
//             show/hide state of the window is not changed.
// Author(s):   Conan Reis
void AWindow::set_y(int y)
  {
  set_order(nullptr, ARegion(get_x_rel(), y, 0, 0), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
  }

//---------------------------------------------------------------------------------------
// Converts this window's specified client co-ordinates to screen co-ordinates
// Returns:    screen coordinate
// Arg         client_pos - client co-ordinate to convert to screen co-ordinate
// Examples:   AVec2i screen_pos(window.xy_client2screen(client_pos));
// Author(s):   Conan Reis
AVec2i AWindow::xy_client2screen(const AVec2i & client_pos) const
  {
  AVec2i screen_pos(client_pos);

  ::ClientToScreen(m_os_handle, (POINT *)&screen_pos);

  return screen_pos;
  }

//---------------------------------------------------------------------------------------
// Converts screen co-ordinates ot this window's client co-ordinates 
// Returns:    client coordinate
// Arg         screen_pos - screen co-ordinate to convert to client co-ordinate
// Examples:   AVec2i client_pos(window.xy_screen2client(screen_pos));
// Author(s):   Conan Reis
AVec2i AWindow::xy_screen2client(const AVec2i & screen_pos) const
  {
  AVec2i client_pos(screen_pos);

  ::ScreenToClient(m_os_handle, (POINT *)&client_pos);

  return client_pos;
  }

/*
//---------------------------------------------------------------------------------------
//  Returns the windows bounding coords relative to the client window
// Returns:     The bounding coords of the window as an ARegion
// Examples:    ARegion left = window.get_region()
ARegion AWindow::get_rel_region() const
  {
  RECT  parent_rect, client_rect;
  POINT top_left;

  if (m_parent_handle == nullptr)
    {
    return get_region();
    }
  ::GetClientRect(m_parent_handle, &parent_rect); // Get parent's screen coords.
  top_left.x = parent_rect.left;
  top_left.y = parent_rect.top;
  ::ClientToScreen(m_parent_handle, &top_left);
  ::GetWindowRect(m_os_handle, &client_rect);     // Get client's screen coords
  client_rect.top -= top_left.y;
  client_rect.left -= top_left.x;
  client_rect.bottom -= top_left.y;
  client_rect.right -= top_left.x;
  return (ARegion(client_rect));
  }

//---------------------------------------------------------------------------------------
//  Adjusts the windows bounding coords relative to the screen
// Arg          Amount to adjust by.
// Returns:     The bounding coords of the window as an ARegion
// Examples:    window.adjust(10)   // Increase the window region by 10 pixels
void AWindow::adjust_region(int amount)
  {
  ARegion rect = get_region();

  rect.adjust(amount);
  set_region(rect);
  }

//---------------------------------------------------------------------------------------
//  Adjusts the windows bounding coords relative to the screen
// Arg          Left side adjustment
// Arg          Top side adjustment
// Arg          Right side adjustment
// Arg          Bottom side adjustment
// Examples:    window.adjust(-10, 20, -10, 20)   // Make the window taller and skinnier
void AWindow::adjust_region(int left, int top, int right, int bottom)
  {
  ARegion rect = get_region();

  rect.adjust(left, top, right, bottom);
  set_region(rect);
  }
*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Capability Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Enable/disable calling of on_control_event_standard() event
// Arg         call_events - true to enable false to disable
// See:        on_* methods
// Author(s):   Conan Reis
void AWindow::enable_control_event(
  bool call_events // = true
  )
  {
  if (call_events)
    {
    m_flags |= Flag_control_events;
    }
  else
    {
    m_flags &= ~Flag_control_events;
    }
  }

//---------------------------------------------------------------------------------------
//  Enables or disables user sizing by border for this window.
// Arg          user_sizable - indicating whether user sizing should be allowed (true)
//              or disallowed (false).  (Default true)
// Examples:    window.enable_sizing();
// See:         is_sizable(), set_border()
// Notes:       This function will also automatically add a raised border (in addition to
//              any current border) around the window.
// Author(s):    Conan Reis
void AWindow::enable_sizing(
  bool user_sizable // = true
  )
  {
  RECT rect;

  if (m_flags & Flag_keep_client_area)
    {
    ::GetClientRect(m_os_handle, &rect);
    }

  if (user_sizable)
    {
    append_style(WS_THICKFRAME, ADef_no_change, false);
    }
  else
    {
    remove_style(WS_THICKFRAME, ADef_no_change, false);
    }

  if (m_flags & Flag_keep_client_area)
    {
    set_area_client(AVec2i(rect.right, rect.bottom));
    }
  else
    {
    refresh();
    }
  }

//---------------------------------------------------------------------------------------
//  Places the window above all non-topmost windows in the Z-order.  The
//              window maintains its topmost position even when it is deactivated.
// Arg          stay_on_top - indicates whether this window should stay on top of other
//              windows (true) or not (false).  (Default true)
// Examples:    window.enable_topmost();
// See:         is_topmost()
// Notes:       This is appropriate for windows that are used to 'alert' the user.
//              Fullscreen windows should also be topmost.
// Author(s):    Conan Reis
void AWindow::enable_topmost(
  bool stay_on_top // = true
  )
  {
  // Note that SetWindowPos() is used rather than calling SetWindowLongPtr() with WS_EX_TOPMOST
  if (stay_on_top)
    {
    ::SetWindowPos(m_os_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
  else
    {
    ::SetWindowPos(m_os_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
  }

//---------------------------------------------------------------------------------------
//  Returns the handle of the cursor associated with the class of the window.
// Returns:     the handle of the cursor associated with the class of the window.
// Examples:    HCURSOR cursor = window.get_class_cursor()
// Author(s):    Conan Reis
HCURSOR AWindow::get_class_cursor() const
  {
  // $Revisit - CReis Could check GetLastError
  return HCURSOR(::GetClassLongPtr(m_os_handle, GCLP_HCURSOR));
  }

//---------------------------------------------------------------------------------------
//  Sets the type of border the window uses.
// Arg          border - Border type to use (Default Border_raised)
// Examples:    window.set_border(Border_no_border);
// See:         enable_sizing()
// Notes:       If window sizing is enabled, a raised border (in addition to the
//              specified border) will be placed around the window.
void AWindow::set_border(
  eBorder border // = Border_raised
  )
  {
  RECT rect;

  if (m_flags & Flag_keep_client_area)
    {
    ::GetClientRect(m_os_handle, &rect);
    }

  // $Revisit CReis - Is there a reason why WS_EX_WINDOWEDGE is not used?
  switch (border)
    {
    case Border_no_border:
      remove_style(WS_BORDER, WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME | WS_EX_STATICEDGE, false);
      break;

    case Border_line:
      modify_style(WS_BORDER, ADef_no_change, ADef_no_change, WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME | WS_EX_STATICEDGE, false);
      break;

    case Border_thin_sunken:  // Non-interactable client
      modify_style(ADef_no_change, WS_BORDER, WS_EX_STATICEDGE, WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, false);
      break;

    case Border_thin_sunken_line:
      modify_style(WS_BORDER, ADef_no_change, WS_EX_STATICEDGE, WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, false);
      break;

    case Border_sunken:       // Interactable client
      modify_style(ADef_no_change, WS_BORDER, WS_EX_CLIENTEDGE, WS_EX_DLGMODALFRAME | WS_EX_STATICEDGE, false);
      break;

    case Border_sunken_line:
      modify_style(WS_BORDER, ADef_no_change, WS_EX_CLIENTEDGE, WS_EX_DLGMODALFRAME | WS_EX_STATICEDGE, false);
      break;

    case Border_thick_sunken:
      modify_style(ADef_no_change, WS_BORDER, WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, WS_EX_DLGMODALFRAME, false);
      break;

    case Border_raised:       // Parent windows
      modify_style(ADef_no_change, WS_BORDER, WS_EX_DLGMODALFRAME, WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, false);
      break;

    case Border_rounded:
      modify_style(ADef_no_change, WS_BORDER, WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME, WS_EX_STATICEDGE, false);
    }

  if (m_flags & Flag_keep_client_area)
    {
    set_area_client(AVec2i(rect.right, rect.bottom));
    }
  else
    {
    refresh();
    }
  }

//---------------------------------------------------------------------------------------
// Arg          HCURSOR structure containing the cursor handle
//  Replaces the handle of the cursor associated with the class of the window.
void AWindow::set_class_cursor(HCURSOR cursor_handle)
  {
  ::SetClassLongPtr(m_os_handle, GCLP_HCURSOR, LONG_PTR(cursor_handle));
  }

//---------------------------------------------------------------------------------------
//  Sets the parent of the window
// Arg          parent_p - Pointer to the window's new parent window or nullptr
// Arg          screen_relative - if true, its position stays/becomes relative
//              to the screen, if false, its x and y position stays/becomes
//              relative to the client window.  (Default true)
// See:         set_parent_handle()
// Author(s):    Conan Reis
void AWindow::set_parent(
  AWindow * parent_p,
  bool      screen_relative // = true
  )
  {
  if (m_parent_p != parent_p)
    {
    int x;
    int y;

    if (screen_relative)
      {
      RECT rect;

      // Get screen coords.
      ::GetWindowRect(m_os_handle, &rect);

      x = rect.left;
      y = rect.top;
      }

    if (parent_p)  // Make into child
      {
      m_parent_handle = parent_p->m_os_handle;
      ::SetParent(m_os_handle, m_parent_handle);

      if (screen_relative)
        {
        ::SetWindowLongPtr(m_os_handle, GWL_STYLE, (~WS_POPUP & ::GetWindowLongPtr(m_os_handle, GWL_STYLE)) | WS_CHILD);

        // Have window's position stay relative to the screen
        POINT point = {x, y};

        ::ScreenToClient(m_parent_handle, &point);
        ::SetWindowPos(m_os_handle, nullptr, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_DRAWFRAME);
        }
      else
        {
        modify_style(WS_CHILD, WS_POPUP);
        }
      }
    else  // Make into pop-up
      {
      m_parent_handle = nullptr;
      ::SetParent(m_os_handle, nullptr);
      if (screen_relative)
        {
        ::SetWindowLongPtr(m_os_handle, GWL_STYLE, (~WS_POPUP & ::GetWindowLongPtr(m_os_handle, GWL_STYLE)) | WS_CHILD);
        ::SetWindowPos(m_os_handle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREDRAW);
        }
      modify_style(WS_POPUP, WS_CHILD);
      }
    m_parent_p = parent_p;
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the parent of the window
// Arg          parent_handle - handle to the window's new parent window or nullptr
// Arg          screen_relative - if true, its position stays/becomes relative
//              to the screen, if false, its x and y position stays/becomes
//              relative to the client window.  (Default true)
// See:         set_parent()
// Author(s):    Conan Reis
void AWindow::set_parent_handle(
  HWND parent_handle,
  bool screen_relative // = true
  )
  {
  if (m_parent_handle != parent_handle)
    {
    int x;
    int y;

    // Get screen coords.
    if (screen_relative)
      {
      RECT rect;

      ::GetWindowRect(m_os_handle, &rect);

      x = rect.left;
      y = rect.top;
      }

    m_parent_handle = parent_handle;
    ::SetParent(m_os_handle, parent_handle);

    if (parent_handle)  // Make into child
      {
      m_parent_p = get_target_from_handle(parent_handle);
      if (screen_relative)
        {
        ::SetWindowLongPtr(m_os_handle, GWL_STYLE, (~WS_POPUP & ::GetWindowLongPtr(m_os_handle, GWL_STYLE)) | WS_CHILD);

        // Have window's position stay relative to the screen
        POINT point = {x, y};

        ::ScreenToClient(parent_handle, &point);
        ::SetWindowPos(m_os_handle, nullptr, point.x, point.y, 0, 0, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
      else
        {
        modify_style(WS_CHILD, WS_POPUP);
        }
      }
    else  // Make into pop-up
      {
      m_parent_p = nullptr;
      if (screen_relative)
        {
        ::SetWindowPos(m_os_handle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);
        }
      modify_style(WS_POPUP, WS_CHILD);
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Recalculates & redraws aspects of the window - such as styles
// Examples:    target.refresh()
// See:         invalidate(), append_style(), get_style(), get_style_extended(),
//              is_style(), modify_style(), remove_style(), set_style()
// Author(s):    Conan Reis
void AWindow::refresh()
  {
  // $Revisit - CReis Test if SWP_NOCOPYBITS is required
  ::SetWindowPos(
    m_os_handle, nullptr, 0, 0, 0, 0,
    ((m_flags & Flag_keep_client_area) ? SWP_NOSENDCHANGING : 0u)
      | SWP_FRAMECHANGED | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Input Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Ensures that mouse input is received when the cursor is over the capturing
//             window like normal *and* when a mouse button was pressed while the cursor
//             was over the window and the button is still down.  Only one window at a
//             time can 'capture' the mouse.
// Arg         msgs_outside_win - receive mouse messages from outside the window once the
//             mouse is captured.
// Author(s):   Conan Reis
void AWindow::enable_mouse_capture(
  bool msgs_outside_win // = true
  )
  {
  if (msgs_outside_win)
    {
    ::SetCapture(m_os_handle);
    }
  else
    {
    ::ReleaseCapture();
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Font Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Sets the default font for this window.
// 
// Examples:   `window.set_font(AFont("Arial", 11.0f));`
// Modifiers:   Virtual - override for custom behaviour
// See:         constructor, get_font(), reset_font()
// Author(s):   Conan Reis
void AWindow::set_font(const AFont & font)
  {
  if (font.m_sys_font_p != m_font.m_sys_font_p)
    {
    A_ASSERTX(font.get_type() == AFontType_true_type, "Tried to associate a non-TrueType font!");

    m_font = font;

    ::SendMessage(m_os_handle, WM_SETFONT, WPARAM(((ATrueTypeFont *)font.m_sys_font_p)->m_font_handle_p), MAKELPARAM(TRUE, 0));
    //if (!ASurfaceBase::is_fullscreen_mode())
    //  {
      invalidate();
    //  }
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that the font associated with this window is being used by it.
// Examples:   window.set_font();
// Notes:      Used primarily in a window's initialization
void AWindow::reset_font(
  bool redraw // = true
  )
  {
  ::SendMessage(m_os_handle, WM_SETFONT, WPARAM(((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p), MAKELPARAM(TRUE, 0));
  if (redraw)
    {
    invalidate();
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Display State Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets the show state (normal, minimized, maximized, hidden,
//              active/inactive, etc.)
// Returns:     the current show state of the window
// Examples:    eAShowState show_state = window.get_show_state()
// See:         hide(), is_maximized(), is_minimized(), maximize(), minimize(),
//              restore(), set_show_state(), show(), is_hidden()
// Notes:       See AgogIO\AFile.hpp for valid values for eAShowState
// Author(s):    Conan Reis
eAShowState AWindow::get_show_state() const
  {
  // $Revisit - CReis GetWindowPlacement() returns a showCmd member in the WINDOWPLACEMENT
  // structure, but according to the function description not all show states are
  // reported (only min, max, or normal) - this should be checked out.

  if (!::IsWindowVisible(m_os_handle))
    {
    return AShowState_hidden;
    }

  if (::IsZoomed(m_os_handle))
    {
    return AShowState_maximized;
    }

  bool active_b = ::IsWindowEnabled(m_os_handle);

  if (::IsIconic(m_os_handle))
    {
    return active_b ? AShowState_min_focus : AShowState_minimized;
    }

  return active_b ? AShowState_normal_focus : AShowState_normal;
  }

//---------------------------------------------------------------------------------------
// Hides the window and deactivates it.
// 
// #Examples
//   window.hide()
// 
// #See Also
//   get_show_state(), is_maximized(), is_minimized(), maximize(), minimize(), restore(),
//   set_show_state(), show(), is_hidden()
//   
// #Author(s) Conan Reis
void AWindow::hide()
  {
  ::ShowWindow(m_os_handle, SW_HIDE);
  }

//---------------------------------------------------------------------------------------
//  Determines whether the window is currently maximized or not
// Returns:     true if maximized, false if not
// See:         get_show_state(), hide(), is_minimized(), maximize(), minimize(),
//              restore(), set_show_state(), show(), is_hidden()
// Author(s):    Conan Reis
bool AWindow::is_maximized() const
  {
  return (::IsZoomed(m_os_handle) == TRUE);
  }

//---------------------------------------------------------------------------------------
// Determines if this window is minimized - as a top-level window or within its parent.
//
// Returns:   `true` if minimized `false` if not
// 
// Notes:  
//   If this is a child window, its top-level parent window could be minimized and this
//   could still return that it is not minimized.
//   
// See:  
//   get_show_state(), hide(), is_maximized(), maximize(), minimize(), restore(),
//   set_show_state(), show(), is_hidden()
//   
// Author(s):    Conan Reis
bool AWindow::is_minimized() const
  {
  return (::IsIconic(m_os_handle) == TRUE);
  }

//---------------------------------------------------------------------------------------
//  Shows the window as taking up the whole screen and activates it if activate
//              == true
// Examples:    window.restore()
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), minimize(),
//              restore(), set_show_state(), show(), is_hidden()
// Author(s):    Conan Reis
void AWindow::maximize()
  {
  // There is no version of maximize that does not also activate the window
  ::ShowWindow(m_os_handle, SW_SHOWMAXIMIZED);
  }

//---------------------------------------------------------------------------------------
//  Iconizes the window and activates it if activate == true
// Arg          activates the window if true
// Examples:    window.restore()
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//              restore(), set_show_state(), show(), is_hidden()
// Author(s):    Conan Reis
void AWindow::minimize(bool activate)
  {
  ::ShowWindow(m_os_handle, activate ? SW_SHOWMINIMIZED : SW_MINIMIZE);
  }

//---------------------------------------------------------------------------------------
//  Shows the window in its normal size and position and activates it if
//              activate == true
// Arg          activates the window if true
// Examples:    window.restore()
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//              minimize(), set_show_state(), show(), is_hidden()
// Author(s):    Conan Reis
void AWindow::restore(bool activate)
  {
  ::ShowWindow(m_os_handle, activate ? SW_RESTORE : SW_SHOWNOACTIVATE);
  }

//---------------------------------------------------------------------------------------
//  Sets the show state (normal, minimized, maximized, hidden,
//              active/inactive, etc.)
// Examples:    window.set_show_state(AShowState_minimized)
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//              minimize(), restore(), show(), is_hidden()
// Notes:       See AgogIO\AFile.hpp for valid values for eAShowState
// Author(s):    Conan Reis
void AWindow::set_show_state(
  eAShowState show_state // = AShowState_normal_focus
  )
  {
  // Specialized methods called since they may be overridden.
  switch (show_state)
    {
    case AShowState_hidden:
      hide();
      return;

    case AShowState_minimized:
    case AShowState_min_focus:
      minimize((show_state | AShowState__focus) != 0u);
      return;

    case AShowState_maximized:
      maximize();
      return;
    }

  // AShowState_normal or AShowState_normal_focus
  restore((show_state | AShowState__focus) != 0u);
  }

//---------------------------------------------------------------------------------------
//  Shows the window in its current state and activates if specified.
// Arg          activate - 
// Examples:    window.show()
// See:         get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//              minimize(), restore(), set_show_state(), is_hidden()
// Author(s):    Conan Reis
void AWindow::show(bool activate)
  {
  if (is_minimized())
    {
    // $Revisit - CReis Should remember last show state prior to being minimized (if there was one)
    restore(activate);
    }
  else
    {
    ::ShowWindow(m_os_handle, activate ? SW_SHOW : SW_SHOWNA);
    }
  }

//---------------------------------------------------------------------------------------
// Brings this window to the foreground
// 
// Notes:
//   If a different process/app needs to relinquish being in the foreground it may need
//   to call AllowSetForegroundWindow() or this method will just cause the app icon to
//   flash in the Start Menu.
// 
// See:
//   show(), get_show_state(), hide(), is_maximized(), is_minimized(), maximize(),
//   minimize(), restore(), set_show_state(), is_hidden()
//   
// Author(s): Conan Reis
void AWindow::make_foreground()
  {
  ::SetForegroundWindow(m_os_handle);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Scroll Bar Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Enables/disables (shows/hides) scroll bars
// Arg          bar_flags - See AWindow::eScrollbar
// Author(s):    Conan Reis
void AWindow::enable_scrollbars(
  eScrollbar bar_flags // = Scrollbar_both
  )
  {
  // $Revisit - CReis Should EnableScrollBar() be called?  Also make sure it works before the window is created

  uint32_t append_styles = 0u;
  uint32_t remove_styles = 0u;

  if (bar_flags & Scrollbar_vert)
    {
    append_styles = WS_VSCROLL;
    }
  else
    {
    remove_styles = WS_VSCROLL;
    }
  if (bar_flags & Scrollbar_horiz)
    {
    append_styles |= WS_HSCROLL;
    }
  else
    {
    remove_styles |= WS_HSCROLL;
    }

  RECT rect;

  if (m_flags & Flag_keep_client_area)
    {
    ::GetClientRect(m_os_handle, &rect);
    }

  modify_style(append_styles, remove_styles, ADef_no_change, ADef_no_change, false);

  if (m_flags & Flag_keep_client_area)
    {
    set_area_client(AVec2i(rect.right, rect.bottom));
    }
  else
    {
    refresh();
    }
  }

//---------------------------------------------------------------------------------------
// Determines whether either of the scrollbars is enabled
// Returns:    Scrollbar flags - see AWindow::eScrollbar
// Author(s):   Conan Reis
AWindow::eScrollbar AWindow::get_scrollbars() const
  {
  uint32_t styles = get_style();
  uint32_t flags  = 0u;

  if (styles & WS_VSCROLL)
    {
    flags |= Scrollbar_vert;
    }

  if (styles & WS_HSCROLL)
    {
    flags |= Scrollbar_horiz;
    }

  return eScrollbar(flags);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Title Bar Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Enables (shows) / disables (hides) title bar
// Arg          title_bar_b - if true the window uses a tile bar, if false it does not
// Author(s):    Conan Reis
void AWindow::enable_title_bar(
  bool title_bar_b // = true
  )
  {
  RECT rect;
  bool keep_area_b = (m_flags & Flag_keep_client_area) != 0;

  if (keep_area_b)
    {
    ::GetClientRect(m_os_handle, &rect);
    }

  // $Revisit - CReis For some reason WS_EX_TOOLWINDOW does not seem to work.  Perhaps some
  // other style is interfering - investigate.

  if (title_bar_b)
    {
    append_style(WS_CAPTION, ADef_no_change, !keep_area_b);
    }
  else
    {
    remove_style(WS_CAPTION, ADef_no_change, !keep_area_b);
    }

  if (keep_area_b)
    {
    set_area_client(AVec2i(rect.right, rect.bottom));
    }
  }

//---------------------------------------------------------------------------------------
// Determines if title_bar is enabled (shown).
// Returns:    true if title_bar enabled, false if not.
// See:        enable_title_bar()
// Author(s):   Conan Reis
bool AWindow::is_title_bar_enabled() const
  {
  return is_style(WS_CAPTION);
  }

//---------------------------------------------------------------------------------------
// Flashes the window title and taskbar icon
// Arg         flash_count - number of times to flash
// Arg         interval_secs - time interval between flashes in seconds
// Author(s):   Conan Reis
void AWindow::flash_title(
  uint flash_count,  // = 4u
  f32 interval_secs // = 0.1f
  )
  {
  FLASHWINFO flash_info = {sizeof(FLASHWINFO), m_os_handle, FLASHW_ALL, flash_count, DWORD(interval_secs * 1000.0f)};

  ::FlashWindowEx(&flash_info);
  }

//---------------------------------------------------------------------------------------
// Returns the HICON associated with the window
// Author(s):   Conan Reis
HICON AWindow::get_icon() const
  {
  // $Revisit - CReis Could check GetLastError
  return HICON(::GetClassLongPtr(m_os_handle, GCLP_HICON));
  }

//---------------------------------------------------------------------------------------
//  Returns the text of the window's title bar. If the specified
//              window is a control, the text of the control is copied.
// Returns:     string containing title
// Author(s):    Conan Reis
AString AWindow::get_title() const
  {
  uint32_t length = ::GetWindowTextLength(m_os_handle);  // +1 for nullptr char
  AString  str(nullptr, length + 1u, 0u);

  ::GetWindowText(m_os_handle, str.as_cstr_writable(), length + 1u);
  str.set_length(length);
  return str;
  }

//---------------------------------------------------------------------------------------
//  Changes the text of the window's title bar (if it has one) to str. If the
//              specified window is a control, the text of the control is changed.
// Arg          Reference to a string containing the title text
// Author(s):    Conan Reis
void AWindow::set_title(const char * cstr_p)
  {
  A_VERIFY_OS(cstr_p && ::SetWindowText(m_os_handle, cstr_p), "set_title()", AWindow);
  }

//---------------------------------------------------------------------------------------
//  Sets the type of buttons to put on the title bar
// Arg          button_type - Type of buttons to use
// Author(s):    Conan Reis
void AWindow::set_title_buttons(eTitleButton button_type)
  {
  switch (button_type)
    {
    case TitleButton_none:     // No buttons on title bar
      remove_style(WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, WS_EX_CONTEXTHELP);
      break;
    case TitleButton_close:    // Icon(system menu) Close
      modify_style(WS_SYSMENU, WS_MAXIMIZEBOX | WS_MINIMIZEBOX, ADef_no_change, WS_EX_CONTEXTHELP);
      break;
    case TitleButton_help:     // Icon(system menu) Help, Close
      modify_style(WS_SYSMENU, WS_MAXIMIZEBOX | WS_MINIMIZEBOX, WS_EX_CONTEXTHELP);
      break;
    case TitleButton_minimize: // Icon(system menu) Minimize, Maximize(greyed), Close
      modify_style(WS_SYSMENU | WS_MINIMIZEBOX, WS_MAXIMIZEBOX, ADef_no_change, WS_EX_CONTEXTHELP);
      break;
    case TitleButton_maximize: // Icon(system menu) Minimize(greyed), Maximize, Close
      modify_style(WS_SYSMENU | WS_MAXIMIZEBOX, WS_MINIMIZEBOX, ADef_no_change, WS_EX_CONTEXTHELP);
      break;
    case TitleButton_min_max:  // Icon(system menu) Minimize, Maximize, Close
      modify_style(WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, ADef_no_change, ADef_no_change, WS_EX_CONTEXTHELP);
    }
  }

//---------------------------------------------------------------------------------------
// Replaces the icon resource associated with the window's class
//             (i.e. AMessageTargetClass).
// Arg         icon_resource_id - resource id of icon to load from executable.
//             The following id values are the predefined icons:
//               IDI_APPLICATION  Default application icon. 
//               IDI_ERROR        Hand-shaped icon. 
//               IDI_QUESTION     Question mark icon. 
//               IDI_WARNING      Exclamation point icon. 
//               IDI_INFORMATION  Asterisk icon. 
//               IDI_WINLOGO      Windows logo icon. 
// See:        get_icon()
// Author(s):   Conan Reis
void AWindow::set_icon(LONG_PTR icon_resource_id)
  {
  // $Revisit - CReis Stability:  The current implementation precludes valid resource ids
  // with the same value as the predefined ids.
  // $Revisit - CReis Clearness:  Since this method alters the state of the window class
  // (AMessageTargetClass) and therefore any window that uses that class, it may be more
  // appropriate to put this method in AgogIO\AMessageTargetClass.

  bool  use_predefined = a_is_ordered(LONG_PTR(IDI_APPLICATION), icon_resource_id, LONG_PTR(IDI_SHIELD));
  HICON icon_handle    = ::LoadIcon(use_predefined ? nullptr : AApplication::ms_res_instance, MAKEINTRESOURCE(icon_resource_id));

  A_VERIFY_OS(icon_handle != nullptr, "set_icon()", AWindow);
  ::SetClassLongPtr(m_os_handle, GCLP_HICON, LONG_PTR(icon_handle));
  }

//---------------------------------------------------------------------------------------
// Replaces the icon resource associated with the window's class
//             (i.e. AMessageTargetClass).
// Arg         icon_file_str_p - Icon file string
// Arg         pixel_size - desired pixel size.  If there are multiple icons in the icon
//             file, this will help to determine which icon size to use.  If pixel_size
//             is 0 then the default size is used (usually 32x32 pixels).
// See:        get_icon()
// Author(s):   Conan Reis
void AWindow::set_icon_file(
  const char * icon_file_str_p,
  int          pixel_size //= 16
  )
  {
  // $Revisit - CReis Clearness:  Since this method alters the state of the window class
  // (AMessageTargetClass) and therefore any window that uses that class, it may be more
  // appropriate to put this method in AgogIO\AMessageTargetClass.

  HANDLE icon_handle = ::LoadImage(nullptr, icon_file_str_p, IMAGE_ICON, pixel_size, pixel_size, LR_LOADFROMFILE | ((pixel_size <= 0) ? LR_DEFAULTSIZE : 0));

  A_VERIFY_OS(icon_handle != nullptr, "set_icon_file()", AWindow);
  ::SetClassLongPtr(m_os_handle, GCLP_HICON, LONG_PTR(icon_handle));
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OS Specific Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Invalidate a region for redraw - indicating that on_draw() should be
//             called. 
// Arg         region - region to invalidate
// Arg         erase_bg - erases client background if true (Default true)
// Arg         draw_now - redraw immediately - only use as necessary (Default false)
// Examples:   window.invalidate(ARegion(120, 250 550, 1230))
// See:        refresh() - always redraws
// Author(s):   Conan Reis
void AWindow::invalidate(
  const ARegion & region,
  bool            erase_bg, // = true
  bool            draw_now  // = false
  )
  {
  RECT rect = {region.m_x, region.m_y, region.m_x + region.m_width, region.m_y + region.m_height};

  A_VERIFY_OS(::InvalidateRect(m_os_handle, &rect, erase_bg), "", AWindow);

  if (draw_now)
    {
    ::UpdateWindow(m_os_handle);
    }
  }

//---------------------------------------------------------------------------------------
// Invalidate the whole client area for redraw - indicating that on_draw() should be
//             called. 
// Arg         erase_bg - erases client background if true (Default true)
// Arg         draw_now - redraw immediately - only use as necessary (Default false)
// Examples:   window.invalidate()
// See:        refresh() - always redraws
// Author(s):   Conan Reis
void AWindow::invalidate(
  bool erase_bg, // = true
  bool draw_now  // = false
  )
  {
  A_VERIFY_OS(::InvalidateRect(m_os_handle, nullptr, erase_bg), "", AWindow);

  if (draw_now)
    {
    ::UpdateWindow(m_os_handle);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Registers the window with the operating system
// Arg         view - region (position and area) and initial show state of window.
// Arg         parent_p - a parent AWindow to this object or nullptr if this object has no
//             parent.  The set_parent() method may be used to modify the parent state of
//             this object.
// Author(s):   Conan Reis
void AWindow::register_with_os(const View & view, AWindow * parent_p)
  { 
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects
  // $Revisit - CReis Look into WS_EX_TRANSPARENT, WS_EX_MDICHILD, WS_EX_TOOLWINDOW, WS_EX_CONTROLPARENT, WS_EX_APPWINDOW
  // $Revisit - CReis Look into WinXP specific: WS_EX_COMPOSITED
  // $Revisit - CReis Look into Win2000/XP specific: WS_EX_LAYERED, WS_EX_NOACTIVATE, WS_EX_NOINHERITLAYOUT

  m_os_handle = ::CreateWindowEx(
    0,           // Extended Window Styles
    reinterpret_cast<const char *>(m_class_p->get_atom_id()),  // Using the name of the class is also valid, but using the atom id should be faster
    "",          // title
    (parent_p ? WS_CHILD : WS_POPUP) | WS_CLIPCHILDREN, // Window Styles
    view.m_region.m_x, view.m_region.m_y, view.m_region.m_width, view.m_region.m_height,
    m_parent_handle, 
    nullptr,        // Menu id,  [See AgogGUI_OS/AMenuOS and AgogGUI_OS/APopMenuOS]
    AApplication::ms_instance,
    nullptr);       // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "", AWindow);

  common_setup();

  // Add to child list of Window objects
  //((Window *)m_parent_p)->append(this);

  if (view.m_show_state != AShowState_hidden)
    {
    set_show_state(view.m_show_state);
    }
  }

//---------------------------------------------------------------------------------------
// Performs common window initialization
// Author(s):   Conan Reis
void AWindow::common_setup()
  {
  // Store pointer in MS Windows user data area for quick handle->AMessageTarget retrieval
  ::SetWindowLongPtr(m_os_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  reset_font(false);
  }

//---------------------------------------------------------------------------------------
// Destroys OS aspect of this message target
// Notes:      Some windows/controls need to call destroy() in their own destructor
//             rather than letting the AMessageTarget destructor call it since destroy()
//             will end up sending windows messages and the windows/controls need to have
//             their virtual table still intact.
// Author(s):   Conan Reis
void AWindow::destroy()
  {
  if (m_os_handle && m_class_p)
    {
    hide();
    AMessageTarget::destroy();
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Handle mouse input events
// Author(s):   Conan Reis
bool AWindow::parse_mouse(
  uint32_t   msg,
  WPARAM w_param,
  LPARAM l_param,
  bool * handled_p
  )
  {
  AVec2i  client_pos(AWIN_GET_X(l_param), AWIN_GET_Y(l_param));
  eAMouse buttons = eAMouse(w_param & AMouse_l_r_m_x1_x2);

  *handled_p = true;

  switch (msg)
    {
    case WM_MOUSEMOVE:
      on_mouse_moving(client_pos, buttons);
      return true;

    case WM_LBUTTONDOWN:
      return on_mouse_press(AMouse_left, buttons, client_pos, false);

    case WM_LBUTTONUP:
      on_mouse_release(AMouse_left, buttons, client_pos);
      return true;

    case WM_LBUTTONDBLCLK:
      return on_mouse_press(AMouse_left, buttons, client_pos, true);

    case WM_RBUTTONDOWN:
      return on_mouse_press(AMouse_right, buttons, client_pos, false);

    case WM_RBUTTONUP:
      on_mouse_release(AMouse_right, buttons, client_pos);
      // So that WM_CONTEXTMENU message is sent, but only if no modifier key is pressed
      return (AKeyboard::get_mod_keys() == AKeyMod_none);

    case WM_RBUTTONDBLCLK:
      return on_mouse_press(AMouse_right, buttons, client_pos, true);

    case WM_MBUTTONDOWN:
      return on_mouse_press(AMouse_middle, buttons, client_pos, false);

    case WM_MBUTTONUP:
      on_mouse_release(AMouse_middle, buttons, client_pos);
      return true;

    case WM_MBUTTONDBLCLK:
      return on_mouse_press(AMouse_middle, buttons, client_pos, true);

    case WM_XBUTTONDOWN:
      return on_mouse_press(AWIN_GET_XBUTTON(w_param), buttons, client_pos, false);

    case WM_XBUTTONUP:
      on_mouse_release(AWIN_GET_XBUTTON(w_param), buttons, client_pos);
      return true;

    case WM_XBUTTONDBLCLK:
      return on_mouse_press(AWIN_GET_XBUTTON(w_param), buttons, client_pos, true);

    case WM_MOUSEWHEEL:
      on_mouse_spinning(f32(GET_WHEEL_DELTA_WPARAM(w_param)) / f32(WHEEL_DELTA), buttons, client_pos);
      return true;
    }

  *handled_p = false;

  return true;
  }

//---------------------------------------------------------------------------------------
// Handle keyboard input events
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Author(s):   Conan Reis
bool AWindow::parse_keyboard(
  uint32_t   msg,
  WPARAM w_param,
  LPARAM l_param,
  bool * handled_p
  )
  {
  *handled_p = true;

  switch (msg)
    {
    case WM_CHAR:
      // Ignore "non-printable" characters: BackSpace, Tab,
      // Carriage Return - Enter, Escape - use on_key_press() instead
      if ((w_param >= 32u) && (w_param < 127))
        {
        return on_character(char(w_param), bool(l_param & AWin_key_repeat_flag));
        }
      return true;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:  // Key pressed while Alt held
      {
      eAKey key = eAKey(w_param);

      // Differentiate AKey_return and AKey_num_enter
      if ((key == AKey_return) && (l_param & AWin_key_extended_flag))
        {
        key = AKey_num_enter;
        }

      if (!on_key_press(key, bool(l_param & AWin_key_repeat_flag)))
        {
        // Remove character messages that were added during the TranslateMessage() in the
        // main loop.
        MSG  char_msg;
        uint32_t key_msg = (msg == WM_KEYDOWN) ? WM_CHAR : WM_SYSCHAR;

        ::PeekMessage(&char_msg, m_os_handle, key_msg, key_msg, PM_REMOVE);

        //  Need to re-issue the the shutdown message if received here.
        if (char_msg.message == WM_QUIT)
          {
          ::PostQuitMessage(0);
          }

        return false;
        }

      return true;
      }

    case WM_KEYUP:
    case WM_SYSKEYUP:
      {
      eAKey key = eAKey(w_param);

      // Differentiate AKey_return and AKey_num_enter
      if ((key == AKey_return) && (l_param & AWin_key_extended_flag))
        {
        key = AKey_num_enter;
        }

      return on_key_release(key);
      }

    // $Revisit - CReis Todo: International keys, accented, etc.
    //case WM_UNICHAR:     // Unicode version of WM_CHAR
    //case WM_SYSCHAR:     // Like WM_CHAR but Alt is pressed
    //case WM_DEADCHAR:    // Like WM_CHAR for international characters - accented, umlaut, etc.  
    //case WM_SYSDEADCHAR: // Like WM_CHAR for international characters but Alt is pressed
    }

  *handled_p = false;

  return true;
  }

//---------------------------------------------------------------------------------------
// Handles whenever the user chooses a command from the Window menu.
// Author(s):   Conan Reis
bool AWindow::parse_sys_command(
  uint    cmd,
  LPARAM l_param
  )
  {
  bool default_process = false;

  switch (cmd)
    {
    case SC_CLOSE:  // Attempt to closes the window
      {
      // A close "request" from the user is more appropriate than just immediately
      // sending WM_CLOSE (which is what the default procedure would do) - code should
      // do the actual close.  If it were left up to the user, the code could try to
      // use the AWindow wrapper, but the m_os_handle would be invalid.
      if (on_close_attempt())
        {
        close_default();
        }
      }
      break;

    case SC_MINIMIZE:  // Minimizes the window. 
      default_process = on_show_zoom_attempt(ShowZoom_minimize);
      break;

    case SC_MAXIMIZE:  // Maximizes the window. 
      default_process = on_show_zoom_attempt(ShowZoom_maximize);
      break;

    case SC_RESTORE:   // Restores the window to its normal position and size. 
      default_process = on_show_zoom_attempt(ShowZoom_restore);
      break;

    case SC_KEYMENU:  // Retrieves the window menu as a result of a keystroke.
      // $HACK - CReis This stops the 'beep' when an unassociated key is pressed.
      // It should be stopped with WM_MENUCHAR, but that does not seem to work.  This
      // could prevent "good" actions if they are not accounted for like Alt-Space
      // opening up the sys menu.
      if (l_param == AKey_space)
        {
        default_process = true;
        }
      break;

    // Extra cases
    //case SC_CONTEXTHELP:  - Changes the cursor to a question mark with a pointer. If the user then clicks a control in the dialog box, the control receives a WM_HELP message.
    //case SC_MOVE:         - Moves the window. 
    //case SC_SIZE:         - Sizes the window. 
    //case SC_MOUSEMENU:    - Retrieves the window menu as a result of a mouse click. 
    // $Note - CReis I've never witnessed these codes actually occurring
    //case SC_DEFAULT:      - Selects the default item; the user double-clicked the window menu. 
    //case SC_HOTKEY:       - Activates the window associated with the application-specified hot key. The low-order word of lParam identifies the window to activate. 
    //case SC_HSCROLL:      - Scrolls horizontally. 
    //case SC_VSCROLL:      - Scrolls vertically. 
    //case SC_NEXTWINDOW:   - Moves to the next window. 
    //case SC_PREVWINDOW:   - Moves to the previous window. 
    //case SC_SCREENSAVE:   - Executes the screen saver application specified in the [boot] section of the SYSTEM.INI file. 
    //case SC_MONITORPOWER: - Sets the state of the display. This command supports devices that have power-saving features, such as a battery-powered personal computer.  lParam can have the following values: 1 means the display is going to low power. 2 means the display is being shut off.
    //case SC_TASKLIST:     - Activates the Start menu. 
    default:
      //A_DPRINT(A_SOURCE_STR " Uncaptured WM_SYSCOMMAND - 0x%x\n", cmd);
      default_process = true;
    }

  return default_process;
  }

//---------------------------------------------------------------------------------------
// Handles common messages.   
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Arg         msg - id of message
// Arg         w_param - extra message info
// Arg         l_param - extra message info
// Arg         result_p - pointer to store return info for message
// Arg         handled_p - address to store Boolean indicating if message was parsed and
//             handled (true) or not (false).
// Examples:   Usually called by on_message().
// See:        All the on_* events.
// Notes:      All common window messages should be processed by this function.
//             This method should *not* be virtual
bool AWindow::parse_common_message(
  uint32_t   msg,
  WPARAM w_param,
  LPARAM l_param,
  LRESULT * result_p,
  bool * handled_p
  )
  {
  *handled_p = true;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Mouse Events
  if ((msg >= WM_MOUSEFIRST) && (msg <= WM_MOUSELAST))
    {
    return parse_mouse(msg, w_param, l_param, handled_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Keyboard Events
  if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST))
    {
    return parse_keyboard(msg, w_param, l_param, handled_p);
    }


  switch (msg)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Moving/Sizing Events

    case WM_ENTERSIZEMOVE:
      m_flags |= Flag_in_size_move_loop;
      return true;

    case WM_SIZE:
      // $Revisit - CReis The client width and height should be passed on as values.
      // client width, height = int32_t(LOWORD(l_param)), int32_t(HIWORD(l_param))

      // The status information is not sent - just because of simplicity.  If there is
      // ever a good reason to have such information, it could easily be added.
      // flags = uint32_t(w_param)
      //   SIZE_MAXHIDE   Message is sent to all pop-up windows when some other window is maximized. 
      //   SIZE_MAXIMIZED The window has been maximized. 
      //   SIZE_MAXSHOW   Message is sent to all pop-up windows when some other window has been restored to its former size. 
      //   SIZE_MINIMIZED The window has been minimized. 
      //   SIZE_RESTORED  The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies. 

      if (m_flags & Flag_in_size_move_loop)
        {
        if (!(m_flags & Flag_sizing))
          {
          m_flags |= Flag_sizing;
          on_size();
          }
        on_sizing();
        }
      else
        {
        on_sizing();
        on_sized();
        }
      return true;

    case WM_GETMINMAXINFO:
      {
      // Sent to a window when the size or position of the window is about to change. An
      // application can use this message to override the window's default maximized size
      // and position, or its default minimum or maximum tracking size - also see
      // WM_WINDOWPOSCHANGING.
      MINMAXINFO * minmax_p = reinterpret_cast<MINMAXINFO *>(l_param);

      //A_DPRINT(A_SOURCE_STR " min/max w:%i/%i, min/max h:%i/%i - 0x%p\n", minmax_p->ptMinTrackSize.x, minmax_p->ptMaxTrackSize.x, minmax_p->ptMinTrackSize.y, minmax_p->ptMaxTrackSize.y, this);

      if (m_min_area.m_x != ADef_int)
        {
        minmax_p->ptMinTrackSize.x = m_min_area.m_x;
        }

      if (m_min_area.m_y != ADef_int)
        {
        minmax_p->ptMinTrackSize.y = m_min_area.m_y;
        }

      if (m_max_area.m_x != ADef_int)
        {
        minmax_p->ptMaxTrackSize.x = m_max_area.m_x;
        }

      if (m_max_area.m_y != ADef_int)
        {
        minmax_p->ptMaxTrackSize.y = m_max_area.m_y;
        }

      return true;
      }

    //case WM_WINDOWPOSCHANGING:
    //  // *This may be useful for setting a minimum or maximum size - sent to a window
    //  // whose size, position, or place in the Z order is about to change as a result of
    //  // a call to the SetWindowPos function or another window-management function.
    //  {
    //  WINDOWPOS * winpos_p = reinterpret_cast<WINDOWPOS *>(l_param);
    //  if ((winpos_p->flags & SWP_NOSIZE) == 0)
    //    {
    //    A_DPRINT(A_SOURCE_STR " w:%i, h:%i - 0x%p\n", winpos_p->cx, winpos_p->cy, this);
    //    }
    //  }

    case WM_MOVE:
      // The client x and y are not passed on as values since they could always be
      // queried from the window.  Besides, what is more appropriate, client position or
      // window position, relative co-ordinates or screen co-ordinates?
      // client x, y = int32_t(AWIN_LO_INT16(l_param)), int32_t(AWIN_HI_INT16(l_param))

      if (m_flags & Flag_in_size_move_loop)
        {
        if (!(m_flags & Flag_moving))
          {
          m_flags |= Flag_moving;
          on_move();
          }
        on_moving(Space_client);
        }
      else
        {
        on_moving(Space_client);
        on_moved();
        }
      return true;

    case WM_EXITSIZEMOVE:
      m_flags &= ~Flag_in_size_move_loop;
      if (m_flags & Flag_sizing)
        {
        on_sized();
        m_flags &= ~Flag_sizing;
        }
      if (m_flags & Flag_moving)
        {
        on_moved();
        m_flags &= ~Flag_moving;
        }
      return true;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Scrollbar Events

    case WM_HSCROLL:
      return on_scrollbar_horiz();

    case WM_VSCROLL:
//       A_DPRINT(A_SOURCE_STR "WM_HSCROLL\n");
      return on_scrollbar_vert();


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Command Events

    case WM_SYSCOMMAND:
      // User title bar & sizing border commands
      // There is mouse horizontal and vertical position information contained in l_param,
      // but it does not seem to be useful.
      return parse_sys_command(w_param & AWin_sys_command_code_mask, l_param);


    // User generated an application command event, for example, by clicking an
    // application command button on the mouse, keyboard or other device.
    case WM_APPCOMMAND:
      {
      uint cmd = GET_APPCOMMAND_LPARAM(l_param);

      // Additional info available, but not used:
      //uint device = GET_DEVICE_LPARAM(l_param);
      //uint keys   = GET_KEYSTATE_LPARAM(l_param);

      eAppCmd app_cmd = (cmd <= AppCmd__last)
        ? eAppCmd(cmd)
        : AppCmd__unknown;

      bool call_default = on_app_command(app_cmd);

      if (!call_default)
        {
        *result_p = TRUE;
        }

      return call_default;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Menu Events

    case WM_CONTEXTMENU:
      // The w_param contains the handle of the window that actually received the click,
      // but since it did not intercept this message, it is assumed that it is no longer
      // relevant.
      // The default process sends a WM_CONTEXTMENU message to this window's parent if
      // it has one.
      //A_DPRINT(A_SOURCE_STR " on_context_menu() orig win:0x%p  this win:0x%p\n", w_param, m_os_handle);
      return on_context_menu(AVec2i(AWIN_LO_INT16(l_param), AWIN_HI_INT16(l_param)));

    case WM_ENTERMENULOOP:
      on_menu_modal(true);
      // $Revisit - CReis Should default proc be called?
      return false;

    case WM_EXITMENULOOP:
      on_menu_modal(false);
      // $Revisit - CReis Should default proc be called?
      return false;

    case WM_INITMENU:
      on_menu_init((HMENU)w_param);
      return false;

    case WM_INITMENUPOPUP:
      on_submenu_init((HMENU)w_param);
      return false;

    case WM_UPDATEUISTATE:
      return false;

    case WM_DRAWITEM:
      return false;

    case WM_COMMAND:
      return parse_command(HIWORD(w_param), LOWORD(w_param), (HWND)l_param);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Drawing/Rendering Events

    case WM_PAINT:  // Sent when the system or another application makes a request to paint a portion of an application's window.
      // The DefWindowProc function validates the update region and may also send the
      // WM_NCPAINT message to the window procedure if the window frame must be painted
      // and send the WM_ERASEBKGND message if the window background must be erased.
      return on_draw();

    case WM_ERASEBKGND:  // Sent when the window background must be erased (for example, when a window is resized).
      return on_draw_background(HDC(w_param));

    case WM_DISPLAYCHANGE:
      ADisplay::make_stale();  // This only needs to be called once and so it may be called redundantly from multiple windows.
      on_display_change();
      return false;

    case WM_SETCURSOR:
      return on_set_cursor(HWND(w_param), HIWORD(l_param), LOWORD(l_param));

    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Focus/activation Events

    case WM_SETFOCUS:
      {
      // If return true, continue with default processing (set the window focus)
      if (on_focus())
        {
        m_flags |= Flag_focused;

        return true;
        }

      return false;
      }

    case WM_KILLFOCUS:
      m_flags &= ~Flag_focused;
      on_focus_lost((HWND)w_param);
      return true;

    case WM_ACTIVATE:
      // l_param is the handle of the other window that is being either activated or deactivated
      if (AWIN_LO_INT16(w_param) == WA_INACTIVE)
        {
        // Deactivating
        m_flags &= ~Flag_activated;
        }
      else
        {
        // Activating
        m_flags |= Flag_activated;
        }

      return true;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Drag & Drop Events

    case WM_DROPFILES:  
      // files have been dropped
      parse_drag_drop((HDROP)w_param);
      return false;
    }

  // Try common messages from AMessageTarget
  return AMessageTarget::parse_common_message(msg, w_param, l_param, result_p, handled_p);
  }

//---------------------------------------------------------------------------------------
// Message target message event handler
// 
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         msg - id of message
// Arg         w_param - extra message info
// Arg         l_param - extra message info
// Arg         result_p - pointer to store return info for message
// Examples:   Called by the system.
// See:        All the on_* events.
// Notes:      All messages for this window come through this function
bool AWindow::on_message(
  uint32_t  msg,
  WPARAM    w_param,
  LPARAM    l_param,
  LRESULT * result_p
  )
  {
  // $Revisit - CReis Possible To Do Notification Messages:
  //
  // Activation/Deactivation Messages:
  //   WM_ACTIVATEAPP   - sent when a window belonging to a different application than the active window is about to be activated. The message is sent to the application whose window is being activated and to the application whose window is being deactivated.
  //   WM_CANCELMODE    - sent to cancel certain modes, such as mouse capture. For example, the system sends this message to the active window when a dialog box or message box is displayed. Certain functions also send this message explicitly to the specified window regardless of whether it is the active window. For example, the EnableWindow function sends this message when disabling the specified window.
  //   WM_CHILDACTIVATE - sent to a child window when the user clicks the window's title bar or when the window is activated, moved, or sized.
  //   WM_ENABLE        - sent when an application changes the enabled state of a window. It is sent to the window whose enabled state is changing. This message is sent before the EnableWindow function returns, but after the enabled state (WS_DISABLED style bit) of the window has changed. 
  //   WM_NCACTIVATE    - sent to a window when its nonclient area needs to be changed to indicate an active or inactive state.
  //   WM_NCCREATE      - sent prior to the WM_CREATE message when a window is first created.
  //   WM_NCDESTROY     - informs a window that its nonclient area is being destroyed. The DestroyWindow function sends the WM_NCDESTROY message to the window following the WM_DESTROY message. WM_DESTROY is used to free the allocated memory object associated with the window.  The WM_NCDESTROY message is sent after the child windows have been destroyed. In contrast, WM_DESTROY is sent before the child windows are destroyed.
  //
  // Pos & Size Messages:
  //   WM_HSCROLL           - sent to a window when a scroll event occurs in the window's standard horizontal scroll bar. This message is also sent to the owner of a horizontal scroll bar control when a scroll event occurs in the control.
  //   WM_VSCROLL           - sent to a window when a scroll event occurs in the window's standard vertical scroll bar. This message is also sent to the owner of a vertical scroll bar control when a scroll event occurs in the control.
  //   WM_NCCALCSIZE        - sent when the size and position of a window's client area must be calculated. By processing this message, an application can control the content of the window's client area when the size or position of the window changes.
  //   WM_QUERYOPEN         - sent to an icon when the user requests that the window be restored to its previous size and position.
  //
  // Drawing Messages:
  //   WM_DRAWITEM       - sent to the owner window of an owner-drawn button, combo box, list box, or menu when a visual aspect of the button, combo box, list box, or menu has changed
  //   WM_GETICON        - sent to a window to retrieve a handle to the large or small icon associated with a window. The system displays the large icon in the ALT+TAB dialog, and the small icon in the window caption. 
  //   WM_NCPAINT        - sent to a window when its frame must be painted. 
  //   WM_PALETTECHANGED - sent to all top-level and overlapped windows after the window with the keyboard focus has realized its logical palette, thereby changing the system palette. This message enables a window that uses a color palette but does not have the keyboard focus to realize its logical palette and update its client area.
  //   WM_QUERYDRAGICON  - sent to a minimized (iconic) window. The window is about to be dragged by the user but does not have an icon defined for its class. An application can return a handle to an icon or cursor. The system displays this cursor or icon while the user drags the icon.
  //   WM_STYLECHANGED   - sent to a window after the SetWindowLong function has changed one or more of the window's styles.
  //   WM_STYLECHANGING  - sent to a window when the SetWindowLong function is about to change one or more of the window's styles.
  //   WM_THEMECHANGED   - broadcast to every window following a theme change event. Examples of theme change events are the activation of a theme, the deactivation of a theme, or a transition from one theme to another.
  //
  // Misc Messages:
  //   WM_INPUTLANGCHANGE        - sent to the topmost affected window after an application's input language has been changed. You should make any application-specific settings and pass the message to the DefWindowProc function, which passes the message to all first-level child windows. These child windows can pass the message to DefWindowProc to have it pass the message to their child windows, and so on. 
  //   WM_INPUTLANGCHANGEREQUEST - posted to the window with the focus when the user chooses a new input language, either with the hotkey (specified in the Keyboard control panel application) or from the indicator on the system taskbar. An application can accept the change by passing the message to the DefWindowProc function or reject the change (and prevent it from taking place) by returning immediately. 
  //   WM_USERCHANGED            - sent to all windows after the user has logged on or off.  When the user logs on or off, the system updates the user-specific settings.  The system sends this message immediately after updating the settings.
  //
  // Menu messages:
  //   WM_INITMENU        - sent when a menu is about to become active. It occurs when the user clicks an item on the menu bar or presses a menu key. This allows the application to modify the menu before it is displayed.
  //   WM_INITMENUPOPUP   - sent when a drop-down menu or submenu is about to become active. This allows an application to modify the menu before it is displayed, without changing the entire menu
  //   WM_MENUSELECT      - sent to a menu's owner window when the user selects a menu item
  //   WM_MENUCHAR        - sent when a menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key.
  //   WM_ENTERIDLE       - sent to the owner window of a modal dialog box or menu that is entering an idle state. A modal dialog box or menu enters an idle state when no messages are waiting in its queue after it has processed one or more previous messages.
  //   WM_ENTERMENULOOP   - menu modal loop has been entered. 
  //   WM_EXITMENULOOP    - menu modal loop has been exited. 
  //   WM_MENUCOMMAND     - sent when the user makes a selection from a menu. 
  //   WM_MENUDRAG        - sent to the owner of a drag-and-drop menu when the user drags a menu item. 
  //   WM_MENUGETOBJECT   - sent to the owner of a drag-and-drop menu when the mouse cursor enters a menu item or moves from the center of the item to the top or bottom of the item. 
  //   WM_MENURBUTTONUP   - sent when the user releases the right mouse button while the cursor is on a menu item. 
  //   WM_NEXTMENU        - sent to an application when the right or left arrow key is used to switch between the menu bar and the system menu. 
  //   WM_UNINITMENUPOPUP - sent when a drop-down menu or submenu has been destroyed. 
  //
  // Useless (Probably) Messages: - more appropriate behaviour is achieved via a combination of other messages.
  //   WM_CREATE       - (Just use constructor?) sent when an application requests that a window be created by calling the CreateWindowEx or CreateWindow function. (The message is sent before the function returns.) The window procedure of the new window receives this message after the window is created, but before the window becomes visible. 
  //   WM_DESTROY      - (Just use destructor?) sent when a window is being destroyed. It is sent to the window procedure of the window being destroyed after the window is removed from the screen.  This message is sent first to the window being destroyed and then to the child windows (if any) as they are destroyed. During the processing of the message, it can be assumed that all child windows still exist.
  //   WM_PARENTNOTIFY - Doesn't seem useful since all instances have a constructor and a destructor.  Sent to the parent of a child window when the child window is created or destroyed, or when the user clicks a mouse button while the cursor is over the child window.
  //   WM_WINDOWPOSCHANGED
  //   WM_SIZING
  //   WM_MOVING

  bool handled;
  bool default_process = parse_common_message(msg, w_param, l_param, result_p, &handled);

  if (!handled)
    {
    default_process = false;

    switch (msg)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Misc. Events

      case WM_SHOWWINDOW:
        // The status information is not sent - just because of simplicity.  If there is
        // ever a good reason to have such information, it could easily be added.
        // status = l_param
        //   SW_OTHERUNZOOM   - The window is being uncovered because a maximize window was restored or minimized. 
        //   SW_OTHERZOOM     - The window is being covered by another window that has been maximized. 
        //   SW_PARENTCLOSING - The window's owner window is being minimized. 
        //   SW_PARENTOPENING - The window's owner window is being restored. 
        default_process = (w_param) ? on_show(bool(l_param == SW_PARENTOPENING)) : on_hide(bool(l_param == SW_PARENTCLOSING));
        break;

      /*
      // Testing setting colours for edit boxes
      case WM_CTLCOLOREDIT:
        {
        AWindow * control_p = static_cast<AWindow *>(get_target_from_handle(HWND(l_param)));

        if ((control_p == nullptr) || control_p->m_color_bg.is_default())
          {
          break;
          }

        HDC hdc = HDC(w_param);

        // Set focused color
        ::SetTextColor(hdc, AColor::ms_white);
        ::SetBkColor(hdc, control_p->m_color_bg);

        // Set unfocused color
        *result_p = LRESULT(::GetStockObject(DKGRAY_BRUSH)); // NULL_BRUSH

        default_process = false;
        break;
        }
        
      // WM_CTLCOLORSTATIC
      */

      case WM_NOTIFY:
        // Child control notification
        default_process = parse_control(reinterpret_cast<NMHDR *>(l_param), result_p);
        break;

      default:
        default_process = AMessageTarget::on_message(msg, w_param, l_param, result_p);
      }
    }

  return default_process;
  }

//---------------------------------------------------------------------------------------
// Called when the close button on the title bar is pressed (or any
//             equivalent mechanism).
// Returns:    true if the close was accepted and the default closing behaviour should
//             be taken, false if the close was not accepted, or if custom closing
//             behaviour is performed in the on_close_attempt().
// Notes:      The default behaviour on a true return is to call attempt_self_destruct()
//             for self-destructing objects, and for non self-destructing objects it
//             hides the window.  Whether it was self destructing or not, if it was the
//             last visible window it calls AApplication::shut_down().
//             If the default close behaviour is appropriate, but it would be handy to
//             perform some additional behaviour prior to it, just override this method
//             and execute the extra behaviour, but still return true.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_close_attempt()
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Called when the minimize or maximize/restore button on the title bar is
//             pressed (or any equivalent mechanism).
// Returns:    true if the request should be accepted and the default behaviour should be
//             taken, false if not accepted, or if custom behaviour is performed.
// Notes:      If the default behaviour is appropriate, but it would be handy to perform
//             some additional behaviour prior to it, just override this method, execute
//             the extra behaviour and return true.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_show_zoom_attempt(eShowZoom type)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " zoom:%u - 0x%p\n", type, this);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever the right mouse button is released or when the user types
//             Shift+F10, or presses and releases the context menu key (which usually
//             looks like a small menu with a mouse pointer on it, it is usually between
//             the right hand Start menu key and the right hand Control key - its scan
//             scan code is VK_APPS).
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
//             The default behaviour is to call this window's parent's on_contect_menu().
// Arg         screen_pos - screen co-ordinates of the mouse cursor when the right button
//             is released or (-1, -1) if this event is caused by a keystroke.
// Examples:   called by the system
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_context_menu(const AVec2i & screen_pos)
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_context_menu() - 0x%p\n", this);
  // Call parent's on_context_menu()
  return true;
  }

//---------------------------------------------------------------------------------------
// Called when a modal menu loop has been entered or exited.
// Arg         enter_b - true if entered, false if exited
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void AWindow::on_menu_modal(bool enter_b)
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_menu(%s) - 0x%p\n", enter_b ? "entered" : "exited", this);
  }

//---------------------------------------------------------------------------------------
//  Called when menu is about to become active.
//  Arg           Menu handle/ID
//  Modifiers:    virtual - override for custom behaviour
//  Author(s):    John Stenersen
bool AWindow::on_menu_init(HMENU menu_handle)
  {
  return false;
  }

//---------------------------------------------------------------------------------------
//  Called when a submenu/popup is about to become active.
//  Arg           Menu handle/ID
//  Modifiers:    virtual - override for custom behaviour
//  Author(s):    John Stenersen
bool AWindow::on_submenu_init(HMENU submenu_handle)
  {
  return false;
  }

//---------------------------------------------------------------------------------------
// Called when the user makes a selection from associated menu bar submenu
//             or associated pop-up / context menu.
// Arg         item_id - id of item selected
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void AWindow::on_menu_command(uint32_t item_id)
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_menu_command(item_id:%u) - 0x%p\n", item_id, this);
  }

//---------------------------------------------------------------------------------------
// Called whenever display settings have changed.
// See:        ADisplay
// Examples:   called by the system
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_display_change()
  {
  }

//---------------------------------------------------------------------------------------
// Called whenever a file (or files) is dropped on to this window and drag
//             and drop is enabled.
// Arg         const AString & file_name - the fully qualified path name of the file /
//             folder / etc.  Note that directory paths do *not* end with a trailing
//             backslash '\'.
// Arg         pos - position where the drop occurs on the window
// Examples:   called by system
// See:        enable_drag_drop(), on_drag_drop(), on_drag_drop_end()
// Notes:      Use ADirectory::is_existing_path() to determine if file_name is a file
//             or a directory.
//
//             If multiple files are dropped on the window simultaneously, this method
//             is called successively with one file name at a time - i.e. if 42 files
//             are dropped onto this window, this method will be called 42 times.
//
//             Directories are not recursed automatically.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_drag_drop(
  const AString & file_name,
  const AVec2i &  pos
  )
  {
  }

//---------------------------------------------------------------------------------------
// Called at the beginning of a drag and drop sequence prior to the first
//             path is sent to on_drag_drop().
// See:        on_drag_drop(), on_drag_drop_end(), enable_drag_drop()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_drag_drop_begin()
  {
  }

//---------------------------------------------------------------------------------------
// Called at the end of a drag and drop sequence following the last path
//             sent to on_drag_drop().
// See:        on_drag_drop(), on_drag_drop_begin(), enable_drag_drop()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_drag_drop_end()
  {
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_draw()
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_draw() - 0x%p\n", this);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called when the window client area background is to be drawn.
// Returns:    true if default background colour should fill client region, false if
//             background was drawn.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_draw_background(HDC dc)
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_draw_background() - 0x%p\n", this);

  // Use default background?
  if (m_color_bg.is_default())
    {
    return true;
    }

  // Draw no background - true if m_color_bg set to AColor::ms_void.
  if (m_color_bg.is_invalid())
    {
    return false;
    }

  // Draw custom solid background
  RECT region;
  ::GetClientRect(m_os_handle, &region);
  // Note that ExtTextOut() is faster drawing a solid colour than PatBlt() or FillRect() which use brushes.
  COLORREF old_color = ::SetBkColor(dc, m_color_bg); 
  ::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &region, nullptr, 0, nullptr); 
  ::SetBkColor(dc, old_color); 

  return false;
  }

//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is gained.
// 
// Returns:  
//   In non-OS windows returning true indicates that changing the focus to this window is
//   allowed.  In OS windows (AEditOS, AListOS, ATreeOS, etc.) the return value is
//   ignored.
//   
// See:         on_focus_lost()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_focus()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p\n", this);
  return true;  // Allow focus
  }

//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is lost.
// 
// See:        on_focus()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_focus_lost(HWND focus_window)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p\n", this);
  }

//---------------------------------------------------------------------------------------
// Called when the window is about to be hidden.  If state_changed == true
//             then hide is due to window state change (i.e. from restored to minimized)
//             else then hide is due to being covered by another window.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_hide(bool state_changed)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p\n", this);
  return true;  // Call DefWindowProc
  }

//---------------------------------------------------------------------------------------
// Notes:
//   `on_move()` is called first, followed with repeated calles to `on_moving()` and then
//   one final call to `on_moved()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
// 
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_move()
  {
  }

//---------------------------------------------------------------------------------------
// Called whenever the window moves in its client space.
// Can also be *custom* called by a parent when it moves in screen space.
// 
// Params:
//   space: Space_client or Space_screen
//
// Notes:
//   `on_move()` is called first, followed with repeated calles to `on_moving()` and then
//   one final call to `on_moved()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
// 
// See:       on_move(), on_moved()
// Modifiers: virtual - Override for custom behaviour.
// Author(s): Conan Reis
void AWindow::on_moving(eSpace space)
  {
  }

//---------------------------------------------------------------------------------------
//
// Notes:
//   `on_move()` is called first, followed with repeated calles to `on_moving()` and then
//   one final call to `on_moved()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
// 
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_moved()
  {
  }

//---------------------------------------------------------------------------------------
// Called when the mouse cursor is over the window and needs to be set.
// Arg         cursor_win - window that the cursor is above.  This might be a child
//             window rather than this window.
// Arg         hit_area - Hit code of the mouse event - for example HTCLIENT
// Arg         mouse_msg - Mouse message that caused need for cursor to be set - for
//             example WM_MOUSEMOVE.
// Notes:      Call AMouse::get_position() and xy_screen2client() to get the
//             cursor position.
//
//             To set cursor call AMouse::set_current_cursor_id(), ensure that the
//             MessageTargetClass of this window is not drawing the cursor, and return
//             false to indicate that the cursor has been set.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_set_cursor(
  HWND cursor_win,
  int  hit_area,
  int  mouse_msg
  )
  {
  // $Revisit - CReis hit_area should probably be an enum.
  //A_DPRINT(A_SOURCE_STR " AWindow::on_set_cursor(hit_code: %i, msg_id: %i, cursor_win: %p(h)) - %p(h)\n", hit_code, mouse_msg, cursor_win, m_os_handle);
  return true;  // Call DefWindowProc
  }

//---------------------------------------------------------------------------------------
// Called when the window is about to be shown.  If state_changed == true
//             then show is due to window state change (i.e. from minimized to restored)
//             else then show is due to being uncovered by another window.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_show(bool state_changed)
  {
  return true;  // Call DefWindowProc
  }

//---------------------------------------------------------------------------------------
// Called once whenever a window's client size begins to change.  Usually
//             this is associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_sizing(), on_sized()
// 
// Notes:
//   `on_size()` is called first, followed with repeated calles to `on_sizing()` and then
//   one final call to `on_sized()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
//   
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_size()
  {
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is associated with
// a user dragging a window's sizing border.
// 
// Examples: called by the system
// 
// Notes:
//   `on_size()` is called first, followed with repeated calles to `on_sizing()` and then
//   one final call to `on_sized()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
//   
// See:       on_size(), on_sized()
// Modifiers: virtual - Override for custom behaviour.
// Author(s): Conan Reis
void AWindow::on_sizing()
  {
  }

//---------------------------------------------------------------------------------------
// Called once whenever a window's client size has finished changing.
//             Usually this is associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sizing()
// 
// Notes:
//   `on_size()` is called first, followed with repeated calles to `on_sizing()` and then
//   one final call to `on_sized()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
//   
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_sized()
  {
  }

//---------------------------------------------------------------------------------------
// Called once whenever a window's horizontal scrollbar has event.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   John Stenersen
bool AWindow::on_scrollbar_horiz()
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Called once whenever a window's vertical scrollbar has event.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   John Stenersen
bool AWindow::on_scrollbar_vert()
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Called when link is selected on associated tool tip and it does not have an URL
// already specified.
// 
// See:         AToolTipOS
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_tooltip_link()
  {
  A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p\n", this);
  }

//---------------------------------------------------------------------------------------
// Called when a user generates an application command event, for example, by
//             clicking an application command button on the mouse, keyboard or other
//             device.
// Returns:    Boolean indicating if app command should be passed to default windows
//             procedure 9true) or not (false).  If true and this is a child the event is
//             sent to its parent.  If true and this is a top level window the event is
//             passed to the OS shell.
// Arg         command - app command event - see eAppCmd
// See:        key events, mouse events
// Notes:      It is likely that only a subset of app commands (or even none at all) can
//             be sent on a particular system depending on the devices available and
//             their drivers.
//             Note that unlike buttons that have two states (pressed/released), app
//             commands are 1-shot events with no state.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_app_command(eAppCmd command)
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Called when a printable key is pressed and translated.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Arg         ch - ANSI character code corresponding to a translated character based on
//             the keys pressed and toggled on the keyboard.  If Shift-'2' is pressed '@'
//             is sent.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_key_press(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
//             This method is called just after an associated on_key_press() is called
//             and before an associated on_key_release() is called.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_character(
  char ch,
  bool repeated
  )
  {
  // Call default procedure
  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// 
// #Notes
//   Call AKeyboard::get_mod_keys() to determine if any modifier keys are in effect.
//   
// #Modifiers virtual - Override for custom behaviour.
// #See Also  on_character(), on_key_release()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if default behaviour should be called or false if should consider key handled. 
  bool
AWindow::on_key_press(
  // key code corresponding to a physical key on the keyboard.
  // If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.  Defines
  // for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
  // AKey_0 to AKey_9 are the same as ANSI '0' to '9' (0x30 - 0x39)
  // AKey_A to AKey_Z are the same as ANSI 'A' to 'Z' (0x41 - 0x5A)
  // Special characters like AKey_control are also possible.
  eAKey key,
  // true if this is a repeated send (holding down the key), false if this is the first
  // time key has been pressed.
  bool repeated
  )
  {
  // $Revisit - CReis Should move this to parse_keyboard() so that it still works even if
  // overridden and will also be more efficient by not doing recursion.
  if (m_parent_p)
    {
    // Parents of AWindow objects should only be other AWindow objects.
    return static_cast<AWindow *>(m_parent_p)->on_key_press(key, repeated);
    }

  // Call default procedure
  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is released.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO\AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// See:        on_character(), on_key_press()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
//             This method is called after an associated on_key_press() is called
//             and after an associated on_character() (if appropriate - i.e. it
//             is a printable character) is called.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_key_release(eAKey key)
  {
  // Call default procedure
  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a mouse button is pressed in the client region of this
//             window (or anywhere if the mouse is 'captured').
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Arg         button - the button that was just pressed - see eAMouse
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// Arg         client_pos - client position of mouse cursor
// Arg         double_click - true if this was a double click
// See:        on_mouse_release(), on_mouse_moving(), on_mouse_spinning(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_mouse_press(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos,
  bool           double_click
  )
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a mouse button is released in the client region of this
//             window (or anywhere if the mouse is 'captured').
// Arg         button - the button that was just released - see eAMouse
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// Arg         client_pos - client position of mouse cursor
// See:        on_mouse_press(), on_mouse_moving(), on_mouse_spinning(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  }

//---------------------------------------------------------------------------------------
// Called whenever the mouse cursor moves in the client region of this window
//             (or anywhere if the mouse is 'captured').
// Arg         client_pos - client position of mouse cursor
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// See:        on_mouse_press(), on_mouse_release(), on_mouse_spinning(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_mouse_moving(
  const AVec2i & client_pos,
  eAMouse        buttons
  )
  {
  }

//---------------------------------------------------------------------------------------
// Called whenever a mouse wheel is spun in the client region of this window
//             (or anywhere if the mouse is 'captured').
// Arg         wheel_delta - rotates positive clockwise and negative counter-clockwise in
//             with each full increment/decrement a multiple of 1.  Values > 1 or < 1 are
//             possible if the wheel is spun quickly.
//             This is usually an integer (i.e. no fractional part), but wheels with
//             finer resolution are possible.
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// Arg         client_pos - client position of mouse cursor
// See:        on_mouse_press(), on_mouse_release(), on_mouse_moving(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AWindow::on_mouse_spinning(
  f32            wheel_delta,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_STR " AWindow::on_mouse_spinning(delta: %f) - 0x%p\n", wheel_delta, this);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OS Graphical Control Events
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever an OS common control sends a notification message.
// 
// Returns:  
//   a boolean indicating whether the default MS Windows API process with respect to the
//   given message should be called `true` or not `false`.
//   
// Params:    
//   code:     message code to parse
//   result_p: pointer to store return info for message
//   
// Notes:  
//   This method should be overridden by OS common control objects and then parse out any
//   appropriate messages. For a list of OS standard /system controls see the "Notes"
//   section of the `AMessageTargetClass(os_class_name)` constructor.
//   
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  return true;  // invoke default behaviour
  }

//---------------------------------------------------------------------------------------
// Called whenever an OS standard control sends a notification message.
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         uint32_t code - message code to parse
// Notes:      This method should be overridden by OS standard / system control objects
//             and then parse out any appropriate messages.
//             For a list of OS standard /system controls see the "Notes" section of the
//             AMessageTargetClass(os_class_name) constructor.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool AWindow::on_control_event_standard(uint32_t code)
  {
  return true;  // invoke default behaviour
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converts client area to window area that would the same client area with
//             the specified styles.
// Modifiers:   static
// Author(s):   Conan Reis
AVec2i AWindow::client2window_area(
  uint            styles,
  uint            ex_styles,
  bool           menu_b,
  const AVec2i & client_area
  )
  {
  RECT rect = {0, 0, client_area.m_x, client_area.m_y};

  ::AdjustWindowRectEx(&rect, styles, menu_b, ex_styles);

  // AdjustWindowRectEx() does not take scrollbars into effect
  if (styles & WS_VSCROLL)
    {
    rect.bottom += ::GetSystemMetrics(SM_CXVSCROLL);
    }

  if (styles & WS_HSCROLL)
    {
    rect.right += ::GetSystemMetrics(SM_CXHSCROLL);
    }
  
  return AVec2i(rect.right - rect.left, rect.bottom - rect.top);
  }

//---------------------------------------------------------------------------------------
// Enables / disables the receiving of input for all windows that share the
//             current thread.
// Arg         input_accepted - specifies whether to accept input (true) or reject
//             input (false).
// Arg         skip_win - optional handle of window to skip.  Usually a window that needs
//             exclusive input - for example a modal dialog.
// See:        enable_input(), is_input_enabled()
// Modifiers:   static
// Author(s):   Conan Reis
void AWindow::enable_input_all(
  bool input_accepted,  // = true
  HWND skip_win // = nullptr
  )
  {
  struct EnableInfo
    {
    bool m_enable;
    HWND m_skip_win;

    static BOOL CALLBACK enable_proc(HWND win_handle, LPARAM l_param)
      {
      EnableInfo * info_p = reinterpret_cast<EnableInfo *>(l_param);

      if (win_handle != info_p->m_skip_win)
        {
        ::EnableWindow(win_handle, info_p->m_enable);
        }

      // Continue enumerating
      return TRUE;
      }

    };


  EnableInfo enable_info = {input_accepted, skip_win};

  ::EnumThreadWindows(::GetCurrentThreadId(), EnableInfo::enable_proc, reinterpret_cast<LPARAM>(&enable_info));
  }

//---------------------------------------------------------------------------------------
//  Counts the number of visible pop-up (top level) windows in this
//              application.
// Returns:     number of pop-up windows
// Notes:       Handy for determining when there are no futher visible windows available
//              and hence the application should probably shutdown since there can be
//              no user interaction.
//              This method assumes that all the windows are located in the same thread.
// Modifiers:    static
// Author(s):    Conan Reis
uint AWindow::get_popup_count()
  {
  struct Nested
    {

    static BOOL CALLBACK count_popups_callback(HWND win_handle, LPARAM user_data)
      {
      if (::IsWindowVisible(win_handle))
        {
        (*reinterpret_cast<uint *>(user_data))++;
        }
      return TRUE;
      }

    };


  uint count = 0u;

  ::EnumThreadWindows(::GetCurrentThreadId(), Nested::count_popups_callback, reinterpret_cast<LPARAM>(&count));
  return count;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default user close behaviour
// See:         on_close_attempt()
// Author(s):    Conan Reis
void AWindow::close_default()
  {
  hide();
  attempt_self_destruct();

  if (get_popup_count() == 0u)
    {
    AApplication::shut_down();
    }
  }

//---------------------------------------------------------------------------------------
//  Parses a WM_COMMAND message - sent when the user selects a command item
//              from a menu [though AMenuOS objects send WM_MENUCOMMAND instead], when a
//              control sends a notification message to its parent window, or when an
//              accelerator keystroke is translated.
// Returns:     a boolean indicating whether the default MS Windows API process with
//              respect to the given message should be called (true) or not (false).
// Arg          code - code of control, 1 if it is an accelerator key, or 0 if it is
//              a menu command
// Arg          id - id of the control, accelerator key, or menu
// Arg          l_param - control handle or nullptr
// Examples:    called by on_message()
// See:         on_command_control()
// Notes:       If it is a graphical common control command, it sends the command to the
//              appropriate control via on_control_event_standard() so that it can parse
//              the code itself rather than doing the crazy MS Windows system of the
//              parent doing all the work.  Even MFC does this now - a sure sign that
//              redirecting messages to a parent is a bad design idea.  MFC calls a
//              control handling its own messages "Message Reflection" - which is sure to
//              confuse developers since a control redirects to its parent and then the
//              parent redirects *back* to the control.
//              For a list of OS standard /system controls see the "Notes" section of the
//              AMessageTargetClass(os_class_name) constructor.
// Author(s):    Conan Reis
bool AWindow::parse_command(
  uint32_t code,
  uint32_t id,
  HWND control_handle
  )
  {
  if (control_handle)
    {
    AWindow * control_p = static_cast<AWindow *>(get_target_from_handle(control_handle));

    if (control_p && (control_p->m_flags & Flag_control_events))
      {
      return control_p->on_control_event_standard(code);
      }
    }
  else
    {
    if (code == AWin_command_code_menu)  // Menu Command
      {
      // Menu commands are incremented by 1 internally
      on_menu_command(id - 1u);

      return false;
      }

    //if (code == AWin_command_code_accel_key)  // Accelerator key
    //  {
    //  A_DPRINT(A_SOURCE_STR " - unprocessed accelerator key\n");
    //  }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
//  Dispatches a message to one of the more sophisticated OS graphical
//              controls.
// Returns:     a boolean indicating whether the default MS Windows API process with
//              respect to the given message should be called (true) or not (false).
// Arg          info_p - pointer to the notification message header.
// Arg          result_p - pointer to store return info for message
// Examples:    called by on_message()
// Notes:       Sends the control event to the appropriate control via on_control_event()
//              so that it can parse the code itself rather than doing the crazy MS
//              Windows system of the parent doing all the work.  Even MFC does this
//              now - a sure sign that redirecting messages to a parent is a bad design
//              idea.  MFC calls a control handling its own messages "Message Reflection"
//              - which is sure to  confuse developers since a control redirects to its
//              parent and then the parent redirects *back* to the control.
//              For a list of OS common controls see the "Notes" section of the
//              AMessageTargetClass(os_class_name) constructor.
// Author(s):    Conan Reis
bool AWindow::parse_control(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  AWindow * control_p = static_cast<AWindow *>(get_target_from_handle(info_p->hwndFrom));

  if (control_p)
    {
    switch (info_p->code)
      {
      case NM_SETFOCUS:
        {
        if (control_p->on_focus())
          {
          m_flags |= Flag_focused;

          return true;
          }

        return false;
        }

      case NM_KILLFOCUS:
        control_p->on_focus_lost(nullptr);
        m_flags &= ~Flag_focused;
        break;

      default:  // All other notifications are fairly control specific
        return control_p->on_control_event(info_p, result_p);
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
//  Parses file drag and drop events and calls on_drag_drop() as appropriate.
// Arg          HDROP drag_handle - handle for the drag and drop file information.
// Examples:    called by on_message() when a WM_DROPFILES message is sent.
// See:         on_drag_drop()
// Notes:       This method calls on_drag_drop() for each file that is dropped onto the
//              window.
// Author(s):    Conan Reis
void AWindow::parse_drag_drop(HDROP drag_handle)
  {
  // $Revisit - CReis [Completeness] The 'in_client_area' boolean is not used - is there some use for it?

  AVec2i  drop_point;
  uint32_t    name_length;
  AString file_name(nullptr, _MAX_PATH + 1u, 0u);

  ::DragQueryPoint(drag_handle, (POINT *)&drop_point);

  //bool   in_client_area = (::DragQueryPoint(drag_handle, &drop_point) != 0);
  uint32_t file_count = ::DragQueryFile(drag_handle, AWin_drag_query_file_count, nullptr, 0u);
  uint32_t pos        = 0u;

  on_drag_drop_begin();

  // Iterate through each dropped file
  for (; pos < file_count; pos++)
    {
    // Ensure that there is enough space available for the file name
    name_length = ::DragQueryFile(drag_handle, pos, nullptr, 0);

    // Retrieve the name of the file at pos
    ::DragQueryFile(drag_handle, pos, file_name.as_cstr_writable(), name_length + 1u);
    file_name.set_length(name_length);

    // Call the event
    on_drag_drop(file_name, drop_point);
    }
  ::DragFinish(drag_handle);
  on_drag_drop_end();
  }

