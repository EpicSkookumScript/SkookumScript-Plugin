// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// APopMenuOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\APopMenuOS.hpp>
#include <AgogGUI\AWindow.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#if defined(_MSC_VER)
  // Removes unnecessary warnings - often only part of a structure needs to be initialized
  #pragma warning( disable : 4701 ) // local variable may be used without having been initialized
#endif


//=======================================================================================
// Global Variables
//=======================================================================================

const uint32_t APopMenuOS_aborted = 0u;


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
//             does this by creating its own message loop.  [Note that this also changes
//             how the pop-up menu is shown - see the show() method.
// Arg         auto_abort_b - if true, menu automatically aborts when mouse is outside the
//             menu for approximately 10 seconds.  If false, it stays up until a choice
//             is made or the pop-up menu is aborted.  (Default false)
// See:        AWindow::on_context_menu()
// Notes:      It is better to make a pop-up menu once and then to reuse it via show()
//             than to remake a pop-up menu every AWindow::on_context_menu() for
//             example.
// Author(s):   Conan Reis
APopMenuOS::APopMenuOS(
  bool modeless_b,  // = false
  bool auto_abort_b // = false
  ) :
  AMenuOS(::CreatePopupMenu(), false)
  {
  A_VERIFY_OS(m_menu_handle != nullptr, "Pop-up failed to create", APopMenuOS);

  // MNS_NOTIFYBYPOS is interesting in that it also sends the menu handle, but it is
  // incompatible with modal pop-up menus so it is not used.

  MENUINFO menu_info;

  menu_info.cbSize  = sizeof(MENUINFO);
  menu_info.fMask   = MIM_STYLE;
  menu_info.dwStyle = MNS_CHECKORBMP | (modeless_b ? MNS_MODELESS : 0u) | (auto_abort_b ? MNS_AUTODISMISS : 0u);

  A_VERIFY_OS(::SetMenuInfo(m_menu_handle, &menu_info), "", APopMenuOS);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets the id of the default item if the pop-up menu has a default otherwise
//             it returns APopMenuOS_no_id
// Returns:    default item id or APopMenuOS_no_id
// Author(s):   Conan Reis
uint32_t APopMenuOS::get_default_item() const
  {
  uint32_t item_id = ::GetMenuDefaultItem(m_menu_handle, FALSE, 0u);

  // Ids are incremented by 1 internally.
  return (item_id != APopMenuOS_no_id) ? (item_id - 1u) : APopMenuOS_no_id;
  }

//---------------------------------------------------------------------------------------
// Gets the id of the item at the specified index position.  If there is no
//             item at that index or if that item opens a submenu, APopMenuOS_no_id is
//             returned.
// Returns:    item id or APopMenuOS_no_id
// Author(s):   Conan Reis
uint32_t APopMenuOS::get_item_id(uint32_t item_index) const
  {
  uint32_t item_id = ::GetMenuItemID(m_menu_handle, item_index);

  // Ids are incremented by 1 internally.
  return (item_id != APopMenuOS_no_id) ? (item_id - 1u) : APopMenuOS_no_id;
  }

//---------------------------------------------------------------------------------------
// Displays the pop-up menu and awaits a user choice or a user abort.
//             If it is modal, it returns immediately with false and item_id_p is ignored
//             - instead AWindow::on_menu_command() is called if a item is selected.
// Returns:    true if choice selected or defaulted, false if aborted and no default
//             item is set.
// Arg         screen_pos - screen position to display the pop-up menu
// Arg         owner_handle - Handle of window that owns the pop-up menu while it is being
//             shown - it can be any window in the application.
// Arg         item_p - if a choice is made or defaulted, the id and user data pointer
//             of the selected item are filled.  If the pop-up menu is aborted and no
//             default item is set, item_p is unmodified.  nullptr is not an acceptable
//             value for this parameter.
// Author(s):   Conan Reis
bool APopMenuOS::show(
  const AVec2i & screen_pos,
  HWND           owner_handle,
  uint32_t *         item_id_p
  ) const
  {
  if (is_modeless())
    {
    // Modeless - call AWindow::on_menu_command()
    ::TrackPopupMenuEx(m_menu_handle, TPM_RIGHTBUTTON, screen_pos.m_x, screen_pos.m_y, owner_handle, nullptr);
    }
  else
    {
    // Modal - make custom message loop and return after selection or abort
    uint32_t item_id = ::TrackPopupMenuEx(m_menu_handle, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, screen_pos.m_x, screen_pos.m_y, owner_handle, nullptr);

    if (item_id == APopMenuOS_aborted)
      {
      uint32_t default_id = get_default_item();

      if (default_id != APopMenuOS_no_id)
        {
        item_id = default_id;
        }
      }

    if (item_id != APopMenuOS_aborted)
      {
      if (item_id_p)
        {
        // Decremented since ids are incremented by 1 internally.
        *item_id_p = item_id - 1u;
        }

      return true;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Displays the pop-up menu and awaits a user choice or a user abort.
//             If it is modal, it returns immediately with false and item_id_p is ignored
//             - instead AWindow::on_menu_command() is called if a item is selected.
// Returns:    true if choice selected or defaulted, false if aborted and no default
//             item is set.
// Arg         screen_pos - screen position to display the pop-up menu
// Arg         owner - Window that owns the pop-up menu while it is being shown
//             [it can be any window in the application].
// Arg         item_p - if a choice is made or defaulted, the id and user data pointer
//             of the selected item are filled.  If the pop-up menu is aborted and no
//             default item is set, item_p is unmodified.  nullptr is not an acceptable
//             value for this parameter.
// Author(s):   Conan Reis
bool APopMenuOS::show(
  const AVec2i &  screen_pos,
  const AWindow & owner,
  uint32_t *          item_id_p
  ) const
  {
  return show(screen_pos, owner.get_os_handle(), item_id_p);
  }

//---------------------------------------------------------------------------------------
// Returns a wrapper for the currently associated window menu (also known as
//             the system menu or the control menu) if there is one so that it can be
//             copied or modified.
// Returns:    APopMenuOS wrapper
// Arg         win - window to retrieve menu from
// See:        AMenu::get_menu_bar()
// Notes:      Use is_valid() to determine if the menu returned is valid - the window
//             might not have a window menu assigned to it.
APopMenuOS APopMenuOS::get_win_menu(const AWindow & win)
  {
  return ::GetSystemMenu(win, FALSE);
  }
