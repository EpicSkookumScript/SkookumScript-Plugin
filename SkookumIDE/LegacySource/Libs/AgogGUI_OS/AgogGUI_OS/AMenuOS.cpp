// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AMenuOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AMenuOS.hpp>
#include <AgogGUI_OS\APopMenuOS.hpp>
#include <AgogGUI\AColor.hpp>
#include <AgogGUI\AWindow.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#if defined(_MSC_VER)
  // Removes unnecessary warnings - often only part of a structure needs to be initialized
  #pragma warning( disable : 4701 ) // local variable may be used without having been initialized
#endif


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         modeless_b - if true the menu can operate at same the time (same message
//             loop) as other message targets (like other windows).  If false the menu is
//             modal and suspends other message targets including idle processing - it
//             does this by creating its own message loop.
// Author(s):   Conan Reis
AMenuOS::AMenuOS(
  bool modeless_b // = true
  ) :
  m_menu_handle(::CreateMenu()),
  m_owned_b(false),
  m_break_next(Break_none)
  {
  A_VERIFY_OS(m_menu_handle != nullptr, "Failed to create", AMenuOS);

  // MNS_NOTIFYBYPOS is interesting in that it also sends the menu handle, but it is
  // incompatible with modal pop-up menus so it is not used.

  if (modeless_b)
    {
    MENUINFO menu_info;

    menu_info.cbSize  = sizeof(MENUINFO);
    menu_info.fMask   = MIM_STYLE;
    menu_info.dwStyle = MNS_MODELESS;

    A_VERIFY_OS(::SetMenuInfo(m_menu_handle, &menu_info), "", AMenuOS);
    }
  }

//---------------------------------------------------------------------------------------
// Destructor
// Examples:   called by system
// Author(s):   Conan Reis
AMenuOS::~AMenuOS()
  {
  if (m_menu_handle && !m_owned_b)
    {
    A_VERIFY_OS_NO_THROW(::DestroyMenu(m_menu_handle), "", AMenuOS);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Breaks menu into additional columns with optional dividing line.
// See:        append_item(), append_submenu(), show()
// Author(s):   Conan Reis
void AMenuOS::append_column_break(
  eBreak break_type // = Break_colum_bar
  )
  {
  m_break_next = break_type;
  }


//---------------------------------------------------------------------------------------
// Appends a new item to the pop-up menu.
// Arg         item_cstr_p - string describing the item.  Hint: if you would like to
//             indicate a hot key for this item, insert a tab character (\t) prior to
//             the string denoting the key.  All substrings following tabs will be
//             aligned after the longest item string.
// Arg         id - id to identify the new item - enumerated types are recommended.
// Arg         enable - Specifies whether this element should be enabled and selectable
//             (true) or greyed and not selectable (false).  See the notes section.
// Arg         checked - true if item should start with a check mark, false if the item
//             should be unchecked.  Toggling the display or lack of a check mark must
//             be done manually via check_item().  (Default false)
// Arg         default_on_abort - This specifies whether this item should be returned as
//             a result from show() when the pop-up menu is aborted.  Only one item may
//             be set as default for a pop-up menu and it will be in a bold font to set
//             it apart from the other choices.
// See:        append_submenu(), append_separator(), show()
// Notes:      As a user-interface design standard: any potential pop-up menu item
//             choices should always be visually present whether they are currently 
//             selectable or not (i.e. show them as greyed out) - this gives the user
//             information that the option is potentially available, just not at present.
// Author(s):   Conan Reis
void AMenuOS::append_item(
  const char * item_cstr_p,
  uint32_t     id,
  bool         enable,          // = true
  bool         checked,         // = false
  bool         default_on_abort // = false
  )
  {
  MENUITEMINFO item_info;

  item_info.cbSize     = sizeof(MENUITEMINFO);
  item_info.fMask      = MIIM_STATE | MIIM_STRING | MIIM_ID;
  item_info.fState     = (checked ? MFS_CHECKED : MFS_UNCHECKED) | (enable ? MFS_ENABLED : MFS_DISABLED) | (default_on_abort ? MFS_DEFAULT : 0u);
  item_info.dwTypeData = const_cast<char *>(item_cstr_p);
  item_info.cch        = (UINT) strlen(item_cstr_p) + 1;
  item_info.wID        = id + 1u;  // Ids are incremented by 1 internally.

  // Insert column break if one was requested for the next appended item.
  if (m_break_next != Break_none)
    {
    item_info.fMask |= MIIM_FTYPE;
    item_info.fType = (m_break_next == Break_column) ? MFT_MENUBREAK : MFT_MENUBARBREAK;
    m_break_next = Break_none;
    }

  A_VERIFY_OS(::InsertMenuItem(m_menu_handle, UINT_MAX, TRUE, &item_info), "", AMenuOS);
  }


//---------------------------------------------------------------------------------------
// Appends a horizontal dividing line to separate items in the pop-up menu.
// See:        append_item(), append_submenu(), show()
// Author(s):   Conan Reis
void AMenuOS::append_separator()
  {
  A_VERIFY_OS(::AppendMenu(m_menu_handle, MF_SEPARATOR, 0u, nullptr), "", AMenuOS);
  }


//---------------------------------------------------------------------------------------
// Appends a submenu
// Arg         submenu_p - pointer to a submenu to append - this must not be nullptr
// Arg         submenu_cstr_p - string describing the submenu
// Arg         enable - Specifies whether this submenu should be enabled and selectable
//             (true) or greyed and not selectable (false).  See the notes section.
//             (Default true)
// See:        append_item(), append_separator(), show()
// Notes:      The same submenu may only be used once.  Once a submenu has been appended
//             to some other pop-up menu, the AMenuOS object defining the submenu
//             does not need to stay persistent - this parent pop-up menu will retain
//             all the submenus info.
//
//             Note that the ids returned via show() of items chosen from a submenu are
//             not given any special differentiation than items of the 'parent' pop-up
//             menu other than the id values that they are originally given.  So ensure
//             that the ids do not conflict, or are acceptable if ids have duplicates.
//
//             As a user-interface design standard: any potential pop-up menu item
//             choices should always be visually present whether they are currently 
//             selectable or not (i.e. show them as greyed out) - this gives the user
//             information that the option is potentially available, just not at present.
// Author(s):   Conan Reis
void AMenuOS::append_submenu(
  APopMenuOS * submenu_p,
  const char * submenu_cstr_p,
  bool         enable, // = true
  bool         checked // = false
  )
  {
  uint32_t flags = MF_POPUP | MF_STRING | (enable ? MF_ENABLED : MF_GRAYED) | (checked ? MF_CHECKED : MF_UNCHECKED);
    
  // Insert column break if one was requested for the next appended item.
  if (m_break_next != Break_none)
    {
    flags |= (m_break_next == Break_column) ? MF_MENUBREAK : MF_MENUBARBREAK;
    m_break_next = Break_none;
    }

  A_VERIFY_OS(
    ::AppendMenu(m_menu_handle, flags, uintptr_t(submenu_p->m_menu_handle), submenu_cstr_p),
    "append_submenu()",
    AMenuOS);

  submenu_p->m_owned_b = true;
  }

//---------------------------------------------------------------------------------------
// Enables or disables and greys-out the specified item.
// Arg         id - identifier of the item to change the enable state
// Arg         enabled - true if item should be enabled, false if the item should be
//             disabled and greyed-out.  (Default true)
// See:        is_item_enabled()
// Author(s):   Conan Reis
void AMenuOS::enable_item(
  uint32_t id,
  bool enable // = true
  )
  {
  // Ids are incremented by 1 internally.
  A_VERIFY((::EnableMenuItem(m_menu_handle, id + 1u, (enable ? MF_ENABLED : MF_GRAYED)) != -1), "Menu item did not exist", AErrId_invalid_index, AMenuOS);
  }

//---------------------------------------------------------------------------------------
// Removes an item from a menu.
// Arg         id - identifier of the item to remove.
// # Author(s): John Stenersen
void AMenuOS::remove_item(uint32_t item_id)
  {
  // Ids are incremented by 1 internally.
  ::DeleteMenu(m_menu_handle, item_id + 1, MF_BYCOMMAND);
  }

//---------------------------------------------------------------------------------------
// Enables or disables modeless capability - i.e. the menu can operate at the
//             same time (same message loop) as other message targets (like other windows).
//             If disabled the menu is modal and suspends other message targets including
//             idle processing - it does this by creating its own message loop.
// Arg         modeless_b - true if menu should be modeless, false if the menu should be
//             modal.
// See:        is_item_enabled()
// Author(s):   Conan Reis
void AMenuOS::enable_modeless(
  bool modeless_b // = true
  )
  {
  MENUINFO menu_info;

  menu_info.cbSize = sizeof(MENUINFO);
  menu_info.fMask  = MIM_STYLE;

  A_VERIFY_OS(::GetMenuInfo(m_menu_handle, &menu_info), "", AMenuOS);

  if (((menu_info.dwStyle & MNS_MODELESS) != 0u) != modeless_b)
    {
    if (modeless_b)
      {
      menu_info.dwStyle |= MNS_MODELESS;
      }
    else
      {
      menu_info.dwStyle &= ~MNS_MODELESS;
      }

    A_VERIFY_OS(::SetMenuInfo(m_menu_handle, &menu_info), "", AMenuOS);
    }
  }

//---------------------------------------------------------------------------------------
// Returns the number of items in the pop-up menu
// Returns:    number of items in the pop-up menu
// Notes:      Note that submenus and separators count as items.
// Author(s):   Conan Reis
uint32_t AMenuOS::get_length() const
  {
  int32_t length = ::GetMenuItemCount(m_menu_handle);

  A_VERIFY_OS(length != -1, "", AMenuOS);

  return uint32_t(length);
  }

//---------------------------------------------------------------------------------------
// Determines if the specified item is enabled or disabled and geyed-out.
// Returns:    true if enabled, false if disabled and geyed-out
// Arg         id - identifier of the item to determine the enabled state
// See:        enable_item()
// Author(s):   Conan Reis
bool AMenuOS::is_item_enabled(uint32_t id) const
  {
  MENUITEMINFO item_info;

  item_info.cbSize = sizeof(MENUITEMINFO);
  item_info.fMask  = MIIM_STATE;

  // Ids are incremented by 1 internally.
  A_VERIFY_OS(::GetMenuItemInfo(m_menu_handle, id + 1u, FALSE, &item_info), "", AMenuOS);

  return ((item_info.fState & MF_ENABLED) == MF_ENABLED);
  }

//---------------------------------------------------------------------------------------
// Determines if the menu is modeless - i.e. the menu can operate at the same
//             time (same message loop) as other message targets (like other windows).
//             If false, the menu is modal and suspends other message targets including
//             idle processing - it does this by creating its own message loop.
// Returns:    true if modeless, false if modal
// Author(s):   Conan Reis
bool AMenuOS::is_modeless() const
  {
  MENUINFO menu_info;

  menu_info.cbSize = sizeof(MENUINFO);
  menu_info.fMask  = MIM_STYLE;

  A_VERIFY_OS(::GetMenuInfo(m_menu_handle, &menu_info), "", AMenuOS);

  return (menu_info.dwStyle & MNS_MODELESS) != 0u;
  }

//---------------------------------------------------------------------------------------
// 
// Author(s):   Conan Reis
void AMenuOS::set_item_bitmap(
  uint     id,
  HBITMAP bitmap_handle
  )
  {
  MENUITEMINFO item_info;

  item_info.cbSize   = sizeof(MENUITEMINFO);
  item_info.fMask    = MIIM_BITMAP;
  item_info.hbmpItem = bitmap_handle;

  // Ids are incremented by 1 internally.
  A_VERIFY_OS(::SetMenuItemInfo(m_menu_handle, id + 1u, FALSE, &item_info), "", AMenuOS);
  }


//---------------------------------------------------------------------------------------
// 
// Author(s):   John Stenersen
void AMenuOS::set_item_text(
  uint    item_id,
  AString item_text
  )
  {
  MENUITEMINFO item_info;

  item_info.cbSize      = sizeof(MENUITEMINFO);
  item_info.fMask       = MIIM_STRING;
  item_info.dwTypeData  = (LPSTR) item_text.as_cstr();
//  item_info.cch         = item_text.get_length() + 1;

  // Ids are incremented by 1 internally.
  A_VERIFY_OS(::SetMenuItemInfo(m_menu_handle, item_id + 1u, false, &item_info), "", AMenuOS);
  }


//---------------------------------------------------------------------------------------
// Places or removes a check mark preceding the specified item.
// Arg         id - identifier of the item to change the check state
// Arg         checked - true if item should be checked, false if the item should be
//             unchecked.  (Default true)
// See:        check_item_toggle(), is_item_checked()
// Author(s):   Conan Reis
void AMenuOS::check_item(
  uint32_t id,
  bool checked // = true
  )
  {
  // Ids are incremented by 1 internally.
  A_VERIFY((::CheckMenuItem(m_menu_handle, id + 1u, (checked ? MF_CHECKED : MF_UNCHECKED)) != -1), "Specified menu id does not exist", AErrId_invalid_index, AMenuOS);
  }

//---------------------------------------------------------------------------------------
// Toggles check mark preceding the specified item.
// Arg         id - identifier of the item to change the check state
// See:        check_item(), is_item_checked()
// Author(s):   Conan Reis
void AMenuOS::check_item_toggle(uint32_t id)
  {
  // Ids are incremented by 1 internally.
  check_item(id + 1u, !is_item_checked(id + 1u));
  }

//---------------------------------------------------------------------------------------
// Determines if the specified item is checked or not
// Returns:    true if checked, false if not
// Arg         id - identifier of the item to determine the check state
// See:        check_item(), check_item_toggle()
// Author(s):   Conan Reis
bool AMenuOS::is_item_checked(uint32_t id) const
  {
  MENUITEMINFO item_info;

  item_info.cbSize = sizeof(MENUITEMINFO);
  item_info.fMask  = MIIM_STATE;

  // Ids are incremented by 1 internally.
  A_VERIFY_OS(::GetMenuItemInfo(m_menu_handle, id + 1u, FALSE, &item_info), "", AMenuOS);

  return ((item_info.fState & MFS_CHECKED) == MFS_CHECKED);
  }

//---------------------------------------------------------------------------------------
// Assign this menu to the supplied window - destroying any menu previously
//             assigned to the window.
// Arg         win_p - window to assign this menu to
// See:        get_from()
// Author(s):   Conan Reis
void AMenuOS::set_menu_bar(AWindow * win_p)
  {
  HWND  win_handle  = win_p->get_os_handle();
  HMENU menu_handle = ::GetMenu(win_handle);

  A_VERIFY_OS(::SetMenu(win_handle, m_menu_handle), "", AMenuOS);
  m_owned_b = true;
  
  if (menu_handle)
    {
    A_VERIFY_OS(::DestroyMenu(menu_handle), "", AMenuOS);
    }
  }

//---------------------------------------------------------------------------------------
// Returns a wrapper for the menu currently associated with the supplied
//             window.
// Returns:    AMenuOS wrapper
// Arg         win - window to retrieve menu from
// See:        set_menu_bar(), redraw_menu_bar(), APopMenu::get_win_menu()
// Notes:      Use is_valid() to determine if the menu returned is valid - the window
//             might not have a menubar assigned to it.
// Modifiers:   static
// Author(s):   Conan Reis
AMenuOS AMenuOS::get_menu_bar(const AWindow & win)
  {
  return ::GetMenu(win);
  }

//---------------------------------------------------------------------------------------
// Redraws the menu bar of the specified window.
// Arg         win - window to have its menu bar redrawn
// Modifiers:   static
// Author(s):   Conan Reis
void AMenuOS::redraw_menu_bar(const AWindow & win)
  {
  A_VERIFY_OS(::DrawMenuBar(win), "", AMenuOS);
  }
