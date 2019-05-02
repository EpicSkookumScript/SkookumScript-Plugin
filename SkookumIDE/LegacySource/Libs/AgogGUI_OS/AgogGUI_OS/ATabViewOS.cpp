// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATabViewOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS/ATabViewOS.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * ATabGroupOS::ms_default_class_p;

//=======================================================================================
// ATabGroupOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Interesting Macros that may still need wrappers:

//  ~TabCtrl_InsertItem 
//  ~TabCtrl_GetItem 
//   TabCtrl_SetItem 
//   TabCtrl_SetItemExtra 

//   TabCtrl_GetRowCount 
//   TabCtrl_GetItemRect 
//   TabCtrl_HitTest 
//   TabCtrl_SetItemSize 
//   TabCtrl_SetMinTabWidth 

//   TabCtrl_HighlightItem 
   
//   TabCtrl_GetImageList
//   TabCtrl_RemoveImage 
//   TabCtrl_SetImageList 
//   TabCtrl_SetPadding 

//   TabCtrl_GetToolTips 
//   TabCtrl_SetToolTips 

//   TabCtrl_GetUnicodeFormat 
//   TabCtrl_SetUnicodeFormat 
//   TabCtrl_DeselectAll 
//   TabCtrl_GetExtendedStyle 
//   TabCtrl_SetExtendedStyle 


//---------------------------------------------------------------------------------------

void ATabGroupOS::initialize()
  {
  INITCOMMONCONTROLSEX init_control_info = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
  InitCommonControlsEx(&init_control_info);
  ms_default_class_p = new AMessageTargetClass(WC_TABCONTROL);
  }

//---------------------------------------------------------------------------------------

void ATabGroupOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
// Arg         flags - see eAComboOSFlag
// Arg         font - font to use for text (Default AFont::ms_default - Arial point 10)
// Arg         region - position and area of window
// Notes:      Initial settings (modified with appropriately matching methods):
//               has lines
//               has expand buttons
//               always shows selection
//
//             Initial relevant AWindow inherited settings:
//               show state     - AShowState_hidden, The button is not visible, this is since some settings that are modified after this constructor will not redraw until a redraw or show command is given
//               input enabled  - true, it is enabled rather than disabled (and greyed out)
//               keyboard focus - not focused
//               mouse cursor   - default cursor
// Author(s):   Conan Reis
ATabGroupOS::ATabGroupOS(
  AWindow *       parent_p,
  eARectEdge      placement, // = ARectEdge_top
  const AFont &   font,      // = AFont::ms_default
  const ARegion & region     // = AWindow::ms_region_def
  ) :
  AWindow(parent_p, ms_default_class_p)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects
  uint32_t style_flags = WS_CHILD | WS_TABSTOP | TCS_MULTILINE;

  switch (placement)
    {
    case ARectEdge_left:
      style_flags |= TCS_VERTICAL;
      break;

    case ARectEdge_bottom:
      style_flags |= TCS_BOTTOM;
      break;

    case ARectEdge_right:
      style_flags |= TCS_VERTICAL | TCS_RIGHT;
      break;

    // ARectEdge_top
    }

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    style_flags,
    region.m_x, region.m_y, region.m_width, region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "ATabGroupOS()", ATabGroupOS);

  m_font = font;
  enable_subclass_messages();
  common_setup();

  // Other styles of interest:
  //
  // TCS_TOOLTIPS [Should try] The tab control has a tooltip control associated with it. 
  // TCS_TABS Tabs appear as tabs, and a border is drawn around the display area. This style is the default.
  // TCS_HOTTRACK [Meh] Version 4.70. Items under the pointer are automatically highlighted. You can check whether hot tracking is enabled by calling SystemParametersInfo. 
  // 
  // TCS_FIXEDWIDTH All tabs are the same width. This style cannot be combined with the TCS_RIGHTJUSTIFY style.
  // TCS_FORCEICONLEFT Icons are aligned with the left edge of each fixed-width tab. This style can only be used with the TCS_FIXEDWIDTH style.
  // TCS_FORCELABELLEFT Labels are aligned with the left edge of each fixed-width tab; that is, the label is displayed immediately to the right of the icon instead of being centered. This style can only be used with the TCS_FIXEDWIDTH style, and it implies the TCS_FORCEICONLEFT style.
  // TCS_OWNERDRAWFIXED The parent window is responsible for drawing tabs.
  // 
  // TCS_RIGHTJUSTIFY The width of each tab is increased, if necessary, so that each row of tabs fills the entire width of the tab control. This window style is ignored unless the TCS_MULTILINE style is also specified.
  // TCS_RAGGEDRIGHT Rows of tabs will not be stretched to fill the entire width of the control. This style is the default.
  // TCS_SCROLLOPPOSITE Version 4.70. Unneeded tabs scroll to the opposite side of the control when a tab is selected.
  // 
  // TCS_FOCUSNEVER The tab control does not receive the input focus when clicked.
  // TCS_FOCUSONBUTTONDOWN The tab control receives the input focus when clicked.

  // Visually unappealing:
  //  
  // TCS_BUTTONS Tabs appear as buttons, and no border is drawn around the display area.
  // TCS_FLATBUTTONS Version 4.71. Selected tabs appear as being indented into the background while other tabs appear as being on the same plane as the background. This style only affects tab controls with the TCS_BUTTONS style.
  // TCS_MULTISELECT Version 4.70. Multiple tabs can be selected by holding down the CTRL key when clicking. This style must be used with the TCS_BUTTONS style.

  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
ATabGroupOS::~ATabGroupOS()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Set current focus.
// Author(s):   Conan Reis
void ATabGroupOS::focus_idx(uint idx)
  {
  TabCtrl_SetCurFocus(m_os_handle, idx);
  }

//---------------------------------------------------------------------------------------
// Get the number of tabs.
// Author(s):   Conan Reis
uint ATabGroupOS::get_count() const
  {
  return TabCtrl_GetItemCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Returns focus index or ATabViewOS_invalid_index if no tab is selected.
// Author(s):   Conan Reis
uint ATabGroupOS::get_focus_idx() const
  {
  return TabCtrl_GetCurFocus(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Returns selected index or ATabViewOS_invalid_index if no tab is selected.
// Author(s):   Conan Reis
uint ATabGroupOS::get_selected_idx() const
  {
  return TabCtrl_GetCurSel(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Get user info stored at currently selected tab or return ATabViewOS_error
//             if no tab is selected.
// Author(s):   Conan Reis
uintptr_t ATabGroupOS::get_selected_info() const
  {
  uint idx = get_selected_idx();

  return (idx != ATabViewOS_invalid_index)
    ? idx2info(idx)
    : ATabViewOS_error;
  }

//---------------------------------------------------------------------------------------
// Get the tab "working client region" - i.e. the area in the client co-ords
//             that is clear from any of the tabs graphical elements.
// Returns:    "working client region" in client co-ordinates
// See:        get_tab_window_region(), set_tab_region()
// Author(s):   Conan Reis
ARegion ATabGroupOS::get_tab_region() const
  {
  RECT work_rect;

  ::GetClientRect(m_os_handle, &work_rect);

  // $Note - CReis The MSDN Docs claim that this macro will only work if the tabs are at
  // the top of the window yet it seems to also work with tabs at the bottom, right or
  // left - at least with the styles used here.
  TabCtrl_AdjustRect(m_os_handle, FALSE, &work_rect);

  return work_rect;
  }

//---------------------------------------------------------------------------------------
// Gets the region needed to make a window around this tab group with it's
//             current settings with the specified "working client region".
// Returns:    window region in screen co-ordinates
// Arg         tab_region - "working client region" in client co-ordinates
// See:        get_tab_region(), set_tab_region()
// Author(s):   Conan Reis
ARegion ATabGroupOS::get_tab_window_region(const ARegion & tab_region) const
  {
  RECT win_rect(tab_region);

  // $Note - CReis The MSDN Docs claim that this macro will only work if the tabs are at
  // the top of the window yet it seems to also work with tabs at the bottom, right or
  // left - at least with the styles used here.
  TabCtrl_AdjustRect(m_os_handle, TRUE, &win_rect);

  return win_rect;
  }

//---------------------------------------------------------------------------------------
// Adjusts the window region of this tab group so that it will have the same
//             "working client region" as specified.
// Arg         tab_region - "working client region" in client co-ordinates
// See:        get_tab_region(), get_tab_window_region()
// Author(s):   Conan Reis
void ATabGroupOS::set_tab_region(const ARegion & tab_region)
  {
  set_region(get_tab_window_region(tab_region));
  }

//---------------------------------------------------------------------------------------
// Converts item info to item tab index
// Author(s):   Conan Reis
uint ATabGroupOS::info2idx(uintptr_t info) const
  {
  uint count = get_count();
  uint idx   = 0u;

  while (idx < count)
    {
    if (idx2info(idx) == info)
      {
      return idx;
      }

    idx++;
    }

  return uint(ATabViewOS_error);
  }

//---------------------------------------------------------------------------------------
// Converts tab index to item info
// Author(s):   Conan Reis
uintptr_t ATabGroupOS::idx2info(uint idx) const
  {
  A_ASSERT(idx != ATabViewOS_invalid_index, "Invalid tab index", AErrId_invalid_index, ATabGroupOS);

  TCITEM tab_info;

  tab_info.mask = TCIF_PARAM;

  if (TabCtrl_GetItem(m_os_handle, idx, &tab_info) == TRUE)
    {
    return tab_info.lParam;
    }

  return uint32_t(ATabViewOS_error);
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    tab index position of string in list
// Arg         tab_cstr_p - C-string to add to list
// Author(s):   Conan Reis
uint ATabGroupOS::insert_tab(
  const char * tab_cstr_p,
  uintptr_t    info,
  uint          idx // = 0u
  )
  {
  A_ASSERT(idx != ATabViewOS_invalid_index, "Invalid tab index", AErrId_invalid_index, ATabGroupOS);

  TCITEM tab_info;

  tab_info.mask    = TCIF_TEXT | TCIF_PARAM;
  tab_info.pszText = const_cast<char *>(tab_cstr_p);
  tab_info.lParam  = info;

  return TabCtrl_InsertItem(m_os_handle, idx, &tab_info);
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    tab index position of string in list
// Arg         tab_cstr_p - C-string to add to list
// Author(s):   Conan Reis
uint ATabGroupOS::append_tab(
  const char * tab_cstr_p,
  uintptr_t    info
  )
  {
  return insert_tab(tab_cstr_p, info, get_count());
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to select_idx() on the same tab will not call
//             on_item_selected()
// Author(s):   Conan Reis
void ATabGroupOS::select_idx(uint idx)
  {
  TabCtrl_SetCurSel(m_os_handle, idx);
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to select_info() on the same tab will not call on_item_selected()
// Returns:    tab index where found
// Author(s):   Conan Reis
uint ATabGroupOS::select_info(uintptr_t info)
  {
  uint idx = info2idx(info);

  if (idx == ATabViewOS_invalid_index)
    {
    return uint(ATabViewOS_error);
    }

  select_idx(idx);

  return idx;
  }

//---------------------------------------------------------------------------------------
// Notes:      Can call on_item_selected() if tab was the current selection
// Author(s):   Conan Reis
void ATabGroupOS::remove_idx(uint idx)
  {
  TabCtrl_DeleteItem(m_os_handle, idx);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATabGroupOS::remove_all()
  {
  TabCtrl_DeleteAllItems(m_os_handle);
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever the currently focused tab changes.
// Notes:      Call idx2info() to get the the tab's item info.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void ATabGroupOS::on_focused_idx(uint idx)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - %u(%u)\n", idx, idx2info(idx));
  }

//---------------------------------------------------------------------------------------
// Called whenever a new selection is accepted.
// Arg         new_idx - index of the newly selected tab
// Arg         old_idx - index of the previously selected tab
// Notes:      Call idx2info() or get_selected_info() to get the the tab's item info.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void ATabGroupOS::on_selected_idx(uint new_idx, uint old_idx)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - new: %u(%u), old: %u(%u)\n", new_idx, idx2info(new_idx), old_idx, idx2info(old_idx));
  }

//---------------------------------------------------------------------------------------
// Called whenever an OS common control sends a notification message.
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         uint32_t code - message code to parse
// Arg         result_p - pointer to store return info for message
// Notes:      This method should be overridden by OS common control objects and then
//             parse out any appropriate messages.
//             For a list of OS standard /system controls see the "Notes" section of the
//             AMessageTargetClass(os_class_name) constructor.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool ATabGroupOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  switch (info_p->code)
    {
    // Handled by AWindow::parse_control()
    //   NM_SETFOCUS
    //   NM_KILLFOCUS

    // Probably easiest to get these via sub-classing
    //   NM_CLICK
    //   NM_DBLCLK
    //   NM_RCLICK
    //   NM_RDBLCLK
    //   TCN_KEYDOWN

    // NM_RELEASEDCAPTURE
    // TCN_GETOBJECT

    case TCN_FOCUSCHANGE:
      {
      uint idx = get_focus_idx();
      //A_DPRINT(A_SOURCE_FUNC_STR " - TCN_FOCUSCHANGE: %u\n", idx);
      on_focused_idx(idx);
      break;
      }

    case TCN_SELCHANGING:
      {
      m_prev_idx = get_selected_idx();
      //A_DPRINT(A_SOURCE_FUNC_STR " - TCN_SELCHANGING: %u\n", m_prev_idx);

      // Can allow or disallow selection to occur.
      break;
      }

    case TCN_SELCHANGE:
      {
      uint idx = get_selected_idx();
      //A_DPRINT(A_SOURCE_FUNC_STR " - TCN_SELCHANGE: %u\n", idx);
      on_selected_idx(idx, m_prev_idx);
      break;
      }

    }

  return true;  // invoke default behaviour
  }


//=======================================================================================
// ATabViewOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
// Arg         flags - see eAComboOSFlag
// Arg         font - font to use for text (Default AFont::ms_default - Arial point 10)
// Arg         region - position and area of window
// Notes:      Initial settings (modified with appropriately matching methods):
//               has lines
//               has expand buttons
//               always shows selection
//
//             Initial relevant AWindow inherited settings:
//               show state     - AShowState_hidden, The button is not visible, this is since some settings that are modified after this constructor will not redraw until a redraw or show command is given
//               input enabled  - true, it is enabled rather than disabled (and greyed out)
//               keyboard focus - not focused
//               mouse cursor   - default cursor
// Author(s):   Conan Reis
ATabViewOS::ATabViewOS(
  AWindow *       parent_p,
  eARectEdge      placement, // = ARectEdge_top
  const AFont &   font,      // = AFont::ms_default
  const ARegion & region     // = AWindow::ms_region_def
  ) :
  ATabGroupOS(parent_p, placement, font, region)
  {
  enable_subclass_messages();
  }

//---------------------------------------------------------------------------------------
// Appends window with tab at end of tab group
// Returns:    Tab index associated with window.
// Arg         tab_cstr_p - title name for tab
// Arg         win_p - window that is being added as a child
// See:        append_win(), remove_win()
// Author(s):   Conan Reis
uint ATabViewOS::append_win(
  const char * tab_cstr_p,
  AWindow * win_p)
  {
  uint idx = get_count();

  insert_win(tab_cstr_p, win_p, idx);

  return idx;
  }

//---------------------------------------------------------------------------------------
// Inserts window with tab at specified index
// Arg         tab_cstr_p - title name for tab
// Arg         win_p - window that is being added as a child
// Arg         idx - tab index position to insert at
// See:        append_win(), remove_win()
// Author(s):   Conan Reis
void ATabViewOS::insert_win(
  const char * tab_cstr_p,
  AWindow *    win_p,
  uint          idx // = 0u
  )
  {
  AWindow * old_win_p = get_selected_win();

  insert_tab(tab_cstr_p, reinterpret_cast<uintptr_t>(win_p), idx);

  win_p->set_parent(this, false);
  win_p->set_region(get_tab_region());

  if (is_selected_idx(idx))
    {
    if (old_win_p)
      {
      old_win_p->hide();
      }

    win_p->show();
    }
  else
    {
    win_p->hide();
    }
  }

//---------------------------------------------------------------------------------------
// Removes specified window. 
// Arg         win_p - child window to remove
// Author(s):   Conan Reis
void ATabViewOS::remove_win(AWindow * win_p)
  {
  AWindow * sel_win_p = get_selected_win();

  remove_info(reinterpret_cast<uintptr_t>(win_p));

  win_p->set_parent(nullptr);

  // If this was the selected window - select next window
  if (sel_win_p == win_p)
    {
    win_p->hide();

    // Select next
    win_p = get_selected_win();

    if (win_p)
      {
      win_p->show();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to select_idx() on the same tab will not call on_item_selected()
// Modifiers:   virtual - Overridden from ATabGroupOS
// Author(s):   Conan Reis
void ATabViewOS::select_idx(uint idx)
  {
  if (is_selected_idx(idx))
    {
    return;
    }

  AWindow * win_p = get_selected_win();

  if (win_p)
    {
    win_p->hide();
    }

  TabCtrl_SetCurSel(m_os_handle, idx);

  win_p = get_selected_win();

  if (win_p)
    {
    win_p->show();
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever an OS common control sends a notification message.
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         code - message code to parse
// Arg         result_p - pointer to store return info for message
// Notes:      This method should be overridden by OS common control objects and then
//             parse out any appropriate messages.
//             For a list of OS standard /system controls see the "Notes" section of the
//             AMessageTargetClass(os_class_name) constructor.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool ATabViewOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  if (info_p->code == TCN_SELCHANGE)
    {
    uint idx = get_selected_idx();

    //A_DPRINT(A_SOURCE_FUNC_STR " - TCN_SELCHANGE: %u\n", idx);

    AWindow * old_win_p = idx2win(m_prev_idx);

    if (old_win_p)
      {
      old_win_p->hide();
      }

    AWindow * new_win_p = idx2win(idx);

    if (new_win_p)
      {
      new_win_p->show();
      }

    on_selected_idx(idx, m_prev_idx);
    on_selected_win(new_win_p, idx, old_win_p, m_prev_idx);

    return true;
    }
  
  return ATabGroupOS::on_control_event(info_p, result_p);
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is
//             associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sized()
// Notes:      This applies to the window's client area size changing and not
//             necessarily the outer edge of the window.
// Modifiers:   virtual - Overridden from AWindow
// Author(s):   Conan Reis
void ATabViewOS::on_sizing()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  // Only real-time resize the selected window.  Resize the other windows when the resize
  // is complete.
  AWindow * win_p = get_selected_win();

  if (win_p)
    {
    win_p->set_region(get_tab_region());
    }
  }

//---------------------------------------------------------------------------------------
// Called once whenever a window's client size has finished changing.
//             Usually this is associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sizing()
// Notes:      This applies to the window's client area size changing and not necessarily
//             the outer edge of the window.
// Modifiers:   virtual - Overridden from AWindow
// Author(s):   Conan Reis
void ATabViewOS::on_sized()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  uint32_t tab_count = get_count();

  if (tab_count)
    {
    AWindow * win_p;
    uint       sel_idx = get_selected_idx();
    ARegion   work_region(get_tab_region());

    // Resize all windows except the selected window since it was already resized by the
    // on_sizing() method.
    while (tab_count)
      {
      tab_count--;

      if (tab_count != sel_idx)
        {
        win_p = idx2win(tab_count);

        if (win_p)
          {
          win_p->set_region(work_region);
          }
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever a tab is selected.
// Arg         new_win_p - window associated with selected tab
// Arg         new_idx - index of the newly selected tab
// Arg         old_win_p - window associated with previously selected tab
// Arg         old_idx - index of the previously selected tab
// See:        on_selected_idx()
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void ATabViewOS::on_selected_win(AWindow * new_win_p, uint new_idx, AWindow * old_win_p, uint old_idx)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - new: %u(%u), old: %u(%u)\n", new_idx, new_win_p, old_idx, old_win_p);
  }

