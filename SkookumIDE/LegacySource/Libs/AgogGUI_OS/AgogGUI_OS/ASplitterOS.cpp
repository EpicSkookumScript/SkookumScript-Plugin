// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASplitterOS class definition header
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\ASplitterOS.hpp>
#include <AgogGUI\AMouse.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  enum eSplitPop
    {
    SplitPop_swap,
    SplitPop_rotate_cw,
    SplitPop_rotate_ccw
    };

  f32 ASplitterOS_spin_increment = 0.04f;

} // End unnamed namespace


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * ASplitterOS::ms_default_class_p;

//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void ASplitterOS::initialize()
  {
  ms_default_class_p = new AMessageTargetClass(AMessageTargetClass::Flag_load_icon);
  }

//---------------------------------------------------------------------------------------

void ASplitterOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a parent AWindow to this object or nullptr if this object
//             has no parent.  set_parent() may be used to modify the parent state of
//             this object.  (Default nullptr)
// Arg         region - position and area of window.  (Default Region(0, 0, 320, 200))
// Notes:      The context menu is modeless by default, if modal is desired call
//             get_context_menu().enable_modeless(false).
// Author(s):   Conan Reis
ASplitterOS::ASplitterOS(
  AWindow *       parent_p, // = nullptr
  const ARegion & region    // = AWindow::ms_region_def
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_pane_a_p(nullptr),
  m_pane_b_p(nullptr),
  m_context_menu(true),
  m_orient(Orient_horiz_ab),
  m_user_reorient(Reorient_swap_rotate),
  m_ratio(ASplitterOS_ratio_def),
  m_ratio_min(0.0f),
  m_ratio_max(ASplitterOS_ratio_max),
  m_user_update(RatioChange_live),
  m_bar_pixels(0u),
  m_ratio_changing_b(false),
  m_ratio_changed_b(false),
  m_auto_update_b(false),
  m_showing_popup_b(false)
  {
  set_bar_pixels(ASplitterOS_bar_pixel_def);

  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects
  m_os_handle = ::CreateWindowEx(
    0,           // Extended Window Styles
    reinterpret_cast<const char *>(m_class_p->get_atom_id()),  // Using the name of the class is also valid, but using the atom id should be faster
    "",          // title
    (parent_p ? WS_CHILD : WS_POPUP), // Window Styles - note that the children are not clipped
    region.m_x, region.m_y, region.m_width, region.m_height,
    m_parent_handle, 
    nullptr,        // Menu id,
    AApplication::ms_instance,
    nullptr);       // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "", AWindow);

  common_setup();

  m_auto_update_b = true;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Build pop-up menu
  m_context_menu.append_item("&Swap Panes", SplitPop_swap, m_user_reorient != Reorient_disabled);
  m_context_menu.set_item_bitmap(SplitPop_swap, HBMMENU_MBAR_RESTORE);
  m_context_menu.append_item("&Rotate Clockwise", SplitPop_rotate_cw, m_user_reorient >= Reorient_swap_rotate);
  m_context_menu.append_item("Rotate &Counter-Clockwise", SplitPop_rotate_ccw, m_user_reorient >= Reorient_swap_rotate);

  // $Revisit - CReis Add bitmaps to pop-up choices

  //m_context_menu.set_item_bitmap(2, HBMMENU_MBAR_CLOSE);

  //HBITMAP bmp1 = HBITMAP(::LoadImage(AApplication::ms_res_instance, MAKEINTRESOURCE(IDB_WIN_SWAP_HORIZ), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT)); // LR_LOADMAP3DCOLORS
  //HBITMAP bmp2 = ::LoadBitmap(AApplication::ms_res_instance, MAKEINTRESOURCE(IDB_MEMBERS));
  //HBITMAP bmp3 = ::LoadBitmap(nullptr, LPCTSTR(32738)); // OBM_COMBO
  //m_context_menu.set_item_bitmap(1, bmp1);
  //m_context_menu.set_item_bitmap(2, bmp2);
  //m_context_menu.set_item_bitmap(3, bmp3);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Examples:   called by system
// Author(s):   Conan Reis
ASplitterOS::~ASplitterOS()
  {
  //::DeleteObject(bmp1);
  //::DeleteObject(bmp2);
  //::DeleteObject(bmp3);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Set pane A window without altering the constrains on it.
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::set_pane_a(AWindow * win_p)
  {
  if (m_pane_a_p != win_p)
    {
    if (m_pane_a_p)
      {
      m_pane_a_p->set_parent(nullptr);
      }

    m_pane_a_p = win_p;

    if (win_p)
      {
      win_p->enable_sizing(false);
      
      if (m_auto_update_b)
        {
        update(true, false);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Set pane B window without altering the constrains on it.
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::set_pane_b(AWindow * win_p)
  {
  if (m_pane_b_p != win_p)
    {
    if (m_pane_b_p)
      {
      m_pane_b_p->set_parent(nullptr);
      }

    m_pane_b_p = win_p;

    if (win_p)
      {
      win_p->enable_sizing(false);
      
      if (m_auto_update_b)
        {
        update(false, true);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Set current orientation of splitter.
// See:        rotate_panes_cw(), rotate_panes_ccw(), swap_panes(), get_orientation(),
//             set_user_reorient(), get_user_reorient()
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::set_orientation(eOrient orient)
  {
  if (m_orient != orient)
    {
    m_orient = orient;

    if (m_auto_update_b)
      {
      update();
      }

    on_reorient();
    }
  }

//---------------------------------------------------------------------------------------
// Rotate panes clockwise.
// See:        rotate_panes_ccw(), swap_panes(), set_orientation(), get_orientation(),
//             set_user_reorient(), get_user_reorient()
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::rotate_panes_cw()
  {
  set_orientation((m_orient != Orient_vert_ba)
    ? eOrient(m_orient + 1)
    : Orient_horiz_ab);
  }

//---------------------------------------------------------------------------------------
// Rotate panes counter-clockwise.
// See:        rotate_panes_cw(), swap_panes(), set_orientation(), get_orientation(),
//             set_user_reorient(), get_user_reorient()
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::rotate_panes_ccw()
  {
  set_orientation((m_orient != Orient_horiz_ab)
    ? eOrient(m_orient - 1)
    : Orient_vert_ba);
  }

//---------------------------------------------------------------------------------------
// Swap panes - keeping either vertical or horizontal orientation.  Note that
//             the pane A/B designation remains the same - they only change visually.
// See:        rotate_panes_cw(), rotate_panes_ccw(), set_orientation(), get_orientation(),
//             set_user_reorient(), get_user_reorient()
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::swap_panes()
  {
  set_orientation((m_orient & Orient__b_a)
    ? eOrient(m_orient - Orient__b_a)
    : eOrient(m_orient + Orient__b_a));
  }

//---------------------------------------------------------------------------------------
// Set the permissions for a user to change the orientation of the panes.
// See:        rotate_panes_cw(), rotate_panes_ccw(), swap_panes(), set_orientation(),
//             get_orientation(), get_user_reorient()
// Author(s):   Conan Reis
void ASplitterOS::set_user_reorient(eReorient reorient)
  {
  m_user_reorient = reorient;

  // Update Context Menu
  m_context_menu.enable_item(SplitPop_swap, m_user_reorient != Reorient_disabled);
  m_context_menu.enable_item(SplitPop_rotate_cw, m_user_reorient >= Reorient_swap_rotate);
  m_context_menu.enable_item(SplitPop_rotate_ccw, m_user_reorient >= Reorient_swap_rotate);
  }

//---------------------------------------------------------------------------------------
// Set the ratio from pane A to pane B.
// Returns:    true if ratio changed, false if not
// Arg         ratio - ratio of pane A to pane B.  If set to 0.75, pane A will take up
//             75% of the available space and pane B will take up the remaining 25%.
//             The ratio will be modified as needed to fit the current ratio min/max and
//             pane min/max area constraints if any exist.
// Notes:      The event methods on_ratio_change(), on_ratio_changing(), and
//             on_ratio_changed() are not called when this method is called.
//             Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
bool ASplitterOS::set_ratio(f32 ratio)
  {
  A_ASSERTX(
    (ratio >= 0.0f) && (ratio <= 1.0f),
    a_cstr_format("Invalid splitter ratio: %g.  Must be between 0 and 1.", ratio));

  ratio = constrain_ratio(ratio);

  if (m_ratio != ratio)
    {
    m_ratio = ratio;

    if (m_auto_update_b)
      {
      update();
      }

    return true;
    }
  return false;
  }

//---------------------------------------------------------------------------------------
// Set constraints on the ratio between pane a & b.
// Notes:      If the current ratio is not within the new constraints it is modified.
//             The event methods on_ratio_change(), on_ratio_changing(), and
//             on_ratio_changed() are not called when this method is called.
// Author(s):   Conan Reis
void ASplitterOS::set_ratio_min_max(
  f32 ratio_min, // = 0.0f
  f32 ratio_max  // = ASplitterOS_ratio_max
  )
  {
  A_ASSERTX(
    (ratio_min >= 0.0f) && (ratio_min <= 1.0f) && (ratio_min <= ratio_max) && (ratio_max >= 0.0f) && (ratio_max <= 1.0f),
    a_cstr_format("Invalid splitter ratio(s) - min:%g, max:%g", ratio_min, ratio_max));

  m_ratio_min = ratio_min;
  m_ratio_max = ratio_max;

  set_ratio(constrain_ratio(m_ratio));
  }

//---------------------------------------------------------------------------------------
// Set the manner in which a user can alter the ratio between the panes.
// Author(s):   Conan Reis
void ASplitterOS::set_user_ratio_update(eRatioUpdate update_mode)
  {
  m_user_update = update_mode;
  }

//---------------------------------------------------------------------------------------
// Set the thickness of the ratio bar between the panes.
// Arg         thickness - number of pixels to use for the ratio bar.  If it is set to
//             It will use ASplitterOS_bar_pixel_extra + the current OS sizing border
//             thickness.  (Default ASplitterOS_bar_pixel_def)
// Notes:      Auto updates if auto update is needed and enabled - see enable_auto_update().
// Author(s):   Conan Reis
void ASplitterOS::set_bar_pixels(
  int thickness // = ASplitterOS_bar_pixel_def
  )
  {
  if (thickness == ASplitterOS_bar_pixel_def)
    {
    // $Revisit - CReis For correctness whenever a splitter uses the OS sizing border
    // thickness the splitter bar should be resized whenever the user changes the OS
    // settings and not just once the splitter is created.
    ::SystemParametersInfo(SPI_GETBORDER, 0, &thickness, 0);
    thickness += ASplitterOS_bar_pixel_extra;
    }

  if (m_bar_pixels != thickness)
    {
    m_bar_pixels = thickness;

    if (m_auto_update_b)
      {
      update();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Recalculate the pane positions and areas with respect to the current ratio
//             and orientation and redraw the windows as needed.
// Arg         pane_a - indicates whether pane A should be adjusted and redrawn.
//             (Default true)
// Arg         pane_b - indicates whether pane B should be adjusted and redrawn.
//             (Default true)
// Author(s):   Conan Reis
void ASplitterOS::update(
  bool pane_a, // = true
  bool pane_b  // = true
  )
  {
  int     usable_pixels;
  ARegion region_a;
  ARegion region_b;
  AVec2i  carea(get_area_client());

  if (m_orient & Orient__vertical)
    {
    m_bar_region.m_x      = 0;
    m_bar_region.m_width  = carea.m_x;
    m_bar_region.m_height = m_bar_pixels;
    usable_pixels         = carea.m_y - m_bar_pixels;
    region_a.m_height     = int(f32(usable_pixels) * m_ratio);
    region_b.m_height     = usable_pixels - region_a.m_height;
    region_a.m_width      = carea.m_x;
    region_b.m_width      = carea.m_x;
    }
  else
    {
    m_bar_region.m_y      = 0;
    m_bar_region.m_height = carea.m_y;
    m_bar_region.m_width  = m_bar_pixels;
    usable_pixels         = carea.m_x - m_bar_pixels;
    region_a.m_width      = int(f32(usable_pixels) * m_ratio);
    region_b.m_width      = usable_pixels - region_a.m_width;
    region_a.m_height     = carea.m_y;
    region_b.m_height     = carea.m_y;
    }
  
  switch (m_orient)
    {
    case Orient_horiz_ab: // A|B
      region_b.m_x     = region_a.m_width + m_bar_pixels;
      m_bar_region.m_x = region_a.m_width;
      break;

    case Orient_vert_ab:  // A/B
      region_b.m_y     = region_a.m_height + m_bar_pixels;
      m_bar_region.m_y = region_a.m_height;
      break;

    case Orient_horiz_ba: // B|A
      region_a.m_x     = region_b.m_width + m_bar_pixels;
      m_bar_region.m_x = region_b.m_width;
      break;

    case Orient_vert_ba:  // B/A
      region_a.m_y     = region_b.m_height + m_bar_pixels;
      m_bar_region.m_y = region_b.m_height;
    }

  if (pane_a && m_pane_a_p)
    {
    m_pane_a_p->set_region(region_a);
    m_pane_a_p->invalidate();

    // $Note - CReis invalidate() may not fully update some types of windows.  Have such
    // windows call refresh() in there on_sizing() or on_sized().  This method could be
    // rewritten to call refresh itself, but it makes some windows flicker unnecessarily.
    //m_pane_a_p->refresh();
    }

  if (pane_b && m_pane_b_p)
    {
    m_pane_b_p->set_region(region_b);
    m_pane_b_p->invalidate();
    //m_pane_b_p->refresh();
    }

  invalidate(m_bar_region);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever ratio first begins to change via the user
// See:        on_ratio_changing(), on_ratio_changed()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void ASplitterOS::on_ratio_change()
  {
  //A_DPRINT("ASplitterOS::on_ratio_change()\n");
  }

//---------------------------------------------------------------------------------------
// Called whenever ratio changes via the user
// See:        on_ratio_change(), on_ratio_changed()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void ASplitterOS::on_ratio_changing()
  {
  //A_DPRINT("ASplitterOS::on_ratio_changing()\n");
  }

//---------------------------------------------------------------------------------------
// Called whenever ratio finished changing via the user
// See:        on_ratio_change(), on_ratio_changing()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void ASplitterOS::on_ratio_changed()
  {
  //A_DPRINT("ASplitterOS::on_ratio_changed()\n");
  }

//---------------------------------------------------------------------------------------
// Called whenever the orientation of the panes is changed via the user
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void ASplitterOS::on_reorient()
  {
  //A_DPRINT("ASplitterOS::on_reorient()\n");
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
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool ASplitterOS::on_context_menu(const AVec2i & screen_pos)
  {
  // Turn off sizing cursor
  m_showing_popup_b = true;

  uint32_t pop_id   = 0u;
  bool selected = m_context_menu.show(screen_pos, *this, &pop_id);

  // Turn sizing cursor back on
  m_showing_popup_b = false;

  if (selected)
    {
    switch (eSplitPop(pop_id))
      {
      case SplitPop_swap:
        swap_panes();
        break;

      case SplitPop_rotate_cw:
        rotate_panes_cw();
        break;

      case SplitPop_rotate_ccw:
        rotate_panes_ccw();
      }
    }

  // Don't call parent's on_context_menu()
  return false;
  }

//---------------------------------------------------------------------------------------
// Called when the user makes a selection from associated menu bar submenu
//             or associated pop-up / context menu.
// Arg         item_id - id of item selected
// Modifiers:   virtual - overridden from AWindow
// Author(s):   Conan Reis
void ASplitterOS::on_menu_command(uint32_t item_id)
  {
  switch (eSplitPop(item_id))
    {
    case SplitPop_swap:
      swap_panes();
      break;

    case SplitPop_rotate_cw:
      rotate_panes_cw();
      break;

    case SplitPop_rotate_ccw:
      rotate_panes_ccw();
    }
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool ASplitterOS::on_draw()
  {
  //A_DPRINT("Redrawing bar (%i, %i)...\n", m_bar_region.m_x, m_bar_region.m_y);

  PAINTSTRUCT ps;
  HDC         hdc = ::BeginPaint(m_os_handle, &ps);

  // Draw Background
  ::SelectObject(hdc, ::GetSysColorBrush(COLOR_3DFACE));
  ::PatBlt(hdc, m_bar_region.m_x, m_bar_region.m_y, m_bar_region.m_width, m_bar_region.m_height, PATCOPY);

  // $Vital - CReis Draw background for any empty panes.
  
  if (m_user_update != RatioChange_disabled)
    {
    // Draw splitter line
    ::SelectObject(hdc, ::GetSysColorBrush(COLOR_ACTIVEBORDER));
    ::PatBlt(hdc, m_bar_region.m_x + 2, m_bar_region.m_y + 2, m_bar_region.m_width - 4, m_bar_region.m_height - 4, PATCOPY);
    }

  // Draw raised edge around splitter bar
  RECT rect = {m_bar_region.m_x + 1, m_bar_region.m_y + 1, m_bar_region.m_x + m_bar_region.m_width - 1, m_bar_region.m_y + m_bar_region.m_height - 1};

  ::DrawEdge(hdc, &rect, BDR_RAISEDOUTER, BF_RECT);

  // $Revisit - CReis Draw special left click area for pop-up

  ::EndPaint(m_os_handle, &ps);

  return false;
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
bool ASplitterOS::on_mouse_press(
  eAMouse button,
  eAMouse buttons,
  const AVec2i &  client_pos,
  bool            double_click
  )
  {
  if ((m_user_update != RatioChange_disabled)
    && (buttons == AMouse_left) && (AKeyboard::get_mod_keys() == AKeyMod_none))
    {
    m_bar_offset = (m_orient & Orient__vertical)
      ? client_pos.m_y - m_bar_region.m_y
      : client_pos.m_x - m_bar_region.m_x;
    enable_mouse_capture();
    m_ratio_changing_b = true;
    m_ratio_changed_b  = false;
    }

  return false;
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
void ASplitterOS::on_mouse_release(
  eAMouse button,
  eAMouse buttons,
  const AVec2i &  client_pos
  )
  {
  if (button == AMouse_left)
    {
    if (m_ratio_changing_b)
      {
      enable_mouse_capture(false);
      m_ratio_changing_b = false;

      if (m_ratio_changed_b)
        {
        on_ratio_changed();
        m_ratio_changed_b = false;
        }
      }

    // Bring up context menu when user reratio is disabled and left mouse is released
    if ((m_user_update == RatioChange_disabled) && (AKeyboard::get_mod_keys() == AKeyMod_none))
      {
      on_context_menu(xy_client2screen(client_pos));
      }
    }
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
void ASplitterOS::on_mouse_moving(
  const AVec2i &  client_pos,
  eAMouse buttons
  )
  {
  if (m_ratio_changing_b)
    {
    int useable_pixels;
    int desired_pixels;

    if (m_orient & Orient__vertical)
      {
      useable_pixels = get_height_client() - m_bar_pixels;
      desired_pixels = client_pos.m_y - m_bar_offset;
      }
    else
      {
      useable_pixels = get_width_client() - m_bar_pixels;
      desired_pixels = client_pos.m_x - m_bar_offset;
      }

    if (desired_pixels < 0)
      {
      desired_pixels = 0;
      }

    if (desired_pixels > useable_pixels)
      {
      desired_pixels = useable_pixels;
      }

    f32 new_ratio = (m_orient & Orient__b_a)
      ? f32(useable_pixels - desired_pixels) / f32(useable_pixels)
      : f32(desired_pixels) / f32(useable_pixels);

    if (set_ratio(new_ratio))
      {
      //invalidate(false, true);

      if (!m_ratio_changed_b)
        {
        on_ratio_change();
        m_ratio_changed_b = true;
        }

      on_ratio_changing();
      }
    }
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
bool ASplitterOS::on_set_cursor(
  HWND cursor_win,
  int  hit_area,
  int  mouse_msg
  )
  {
  if (cursor_win == m_os_handle)
    {
    if (m_bar_region.is_in(xy_screen2client(AMouse::get_position())))
      {
      AMouse::set_cursor_id(
        m_showing_popup_b
          ? ACursorOS_arrow
          : ((m_user_update != RatioChange_disabled)
            ? (is_vertical() ? ACursorOS_size_ns : ACursorOS_size_we)
            : ACursorOS_up_arrow));

      return false;
      }
    }

  return true;  // Call DefWindowProc
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is
//             associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sized()
// Notes:      This applies to the window's client area size changing and not
//             necessarily the outer edge of the window.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void ASplitterOS::on_sizing()
  {
  update();
  //invalidate();
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Returns a ratio based off of 'desired_ratio' that is no less than the
//             ratio minimum, no more than the ratio maximum and appropriate for the
//             min/max areas of the pane windows.
// Author(s):   Conan Reis
f32 ASplitterOS::constrain_ratio(f32 desired_ratio)
  {
  if (desired_ratio < m_ratio_min)
    {
    return m_ratio_min;
    }

  if (desired_ratio > m_ratio_max)
    {
    return m_ratio_max;
    }

  // $Revisit - CReis Constrain to the min/max area of the panes if set.

  return desired_ratio;
  }
