// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AComboBoxOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS/AComboBoxOS.hpp>
#include <AgogCore/AStringRef.hpp>
#include <AgogIO/AApplication.hpp>
#include <StdIO.h>


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * AComboBoxOS::ms_default_class_p;

//=======================================================================================
// AComboBoxOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AComboBoxOS::initialize()
  {
  ms_default_class_p = new AMessageTargetClass(WC_COMBOBOX);
  }

//---------------------------------------------------------------------------------------

void AComboBoxOS::deinitialize()
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
AComboBoxOS::AComboBoxOS(
  AWindow *       parent_p,
  uint32_t        flags,     // = AComboOSFlags__static_no_sort
  const AFont &   font,      // = AFont::ms_default
  const ARegion & region     // = AWindow::ms_region_def
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_selected_prev(uint(AComboBoxOS_invalid_index)),
  m_select_okay(false)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    WS_CHILD | WS_TABSTOP 
	  | (( flags & AComboOSFlag_no_vscroll)
	    ? 0 // no vscroll for me!
		: WS_VSCROLL) // Window Styles
      | CBS_DISABLENOSCROLL  // AComboBoxOS Styles
      | ((flags & AComboOSFlag_edit)
        ? CBS_DROPDOWN | CBS_AUTOHSCROLL
        : CBS_DROPDOWNLIST)
      | ((flags & AComboOSFlag_sort)
        ? CBS_SORT
        : 0u),
    region.m_x, region.m_y, region.m_width, region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "AComboBoxOS()", AComboBoxOS);

  m_font = font;
  enable_subclass_messages();
  common_setup();
  }

// Other misc commands that don't currently seem useful:
//   ComboBox_AddItemData
//   ComboBox_Dir
//   ComboBox_GetDroppedControlRect
//   ComboBox_GetExtendedUI
//   ComboBox_SetExtendedUI


//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
AComboBoxOS::~AComboBoxOS()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Converts item info to item row
// Author(s):   Conan Reis
uint AComboBoxOS::info2row(uintptr_t info) const
  {
  // $Revisit - CReis This does not seem to be working for some inexplicable reason
  //return ComboBox_FindItemData(m_os_handle, -1, info);

  uint count = get_length();
  uint row   = 0u;

  while (row < count)
    {
    if (row2info(row) == info)
      {
      return row;
      }

    row++;
    }

  return uint(AComboBoxOS_error);
  }

//---------------------------------------------------------------------------------------
// Converts item row to item info
// Author(s):   Conan Reis
uintptr_t AComboBoxOS::row2info(uint row) const
  {
  A_ASSERT(row != AComboBoxOS_invalid_index, "Invalid row index", AErrId_invalid_index, AComboBoxOS);

  return ComboBox_GetItemData(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    row index position of string in list
// Arg         item_cstr_p - C-string to add to list
// Author(s):   Conan Reis
uint AComboBoxOS::append_text(const char * item_cstr_p)
  {
  return ComboBox_AddString(m_os_handle, item_cstr_p);
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    row index position of string in list
// Arg         item_cstr_p - C-string to add to list
// Author(s):   Conan Reis
uint AComboBoxOS::append_info(
  const char * item_cstr_p,
  uintptr_t    info
  )
  {
  uint row = ComboBox_AddString(m_os_handle, item_cstr_p);

  ComboBox_SetItemData(m_os_handle, row, info);

  return row;
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    row index position of string in list
// Arg         item_cstr_p - C-string to add to list
// Arg         row - 0-based index to insert the string, or ADef_uint32 to add to end of list.
// Author(s):   Conan Reis
uint AComboBoxOS::insert_text(
  const char * item_cstr_p,
  uint32_t     row // = 0u
  )
  {
  return ComboBox_InsertString(m_os_handle, row, item_cstr_p);
  }

//---------------------------------------------------------------------------------------
// Append string to combo list
// Returns:    row index position of string in list
// Arg         item_cstr_p - C-string to add to list
// Author(s):   Conan Reis
uint AComboBoxOS::insert_info(
  const char * item_cstr_p,
  uintptr_t    info,
  uint32_t     row // = 0u
  )
  {
  // $Revisit - CReis ComboBox_InsertItemData?
  uint true_row = ComboBox_InsertString(m_os_handle, row, item_cstr_p);

  ComboBox_SetItemData(m_os_handle, true_row, info);

  return true_row;
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to select_info() on the same row will not call on_item_selected()
// Returns:    row index where found
// Author(s):   Conan Reis
uint AComboBoxOS::select_info(uintptr_t info)
  {
  // $Revisit - CReis This does not seem to be working for some inexplicable reason
  //return ComboBox_SelectItemData(m_os_handle, -1, info);

  uint row = info2row(info);

  if (row == AComboBoxOS_error)
    {
    return uint(AComboBoxOS_error);
    }

  select_row(row);

  return row;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever a new selection is accepted.
// Notes:      Call row2info() or get_selected_info() to get the the row's item info.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void AComboBoxOS::on_selected(uint row, uint row_prev)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "New: %u(%u)  Prev: %u(%u)\n", row, row2info(row), row_prev, row2info(row_prev));
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
// Modifiers:   virtual from AWindow
// Author(s):   Conan Reis
bool AComboBoxOS::on_control_event_standard(uint32_t code)
  {
  switch (code)
    {
    // Handled by AWindow::parse_control()
    // NM_SETFOCUS
    // NM_KILLFOCUS

    // CBN_DBLCLK
    // CBN_ERRSPACE
    // WM_COMPAREITEM
    // WM_DELETEITEM
    // WM_DRAWITEM
    // WM_MEASUREITEM

    case CBN_SETFOCUS:
      {
      m_select_okay = false;

      if (on_focus())
        {
        m_flags |= Flag_focused;

        return true;
        }

      return false;
      }

    case CBN_KILLFOCUS:
      on_focus_lost(0);
      m_flags &= ~Flag_focused;
      break;


    case CBN_CLOSEUP:
      if (!m_select_okay && (get_selected_row() != m_selected_prev))
        {
        select_row(m_selected_prev);
        }
      break;

    case CBN_DROPDOWN:
      m_select_okay   = false;
      m_selected_prev = get_selected_row();
      break;


    //case CBN_EDITCHANGE:
    //  A_DPRINT(A_SOURCE_FUNC_STR " - CBN_EDITCHANGE   Idx: %u,  Id: %u\n", get_selected_row(), get_selected_info());
    //  break;

    //case CBN_EDITUPDATE:
    //  A_DPRINT(A_SOURCE_FUNC_STR " - CBN_EDITUPDATE   Idx: %u,  Id: %u\n", get_selected_row(), get_selected_info());
    //  break;


    //case CBN_SELCHANGE:
    //  A_DPRINT(A_SOURCE_FUNC_STR " - CBN_SELCHANGE    Idx: %u,  Id: %u\n", get_selected_row(), get_selected_info());
    //  break;

    case CBN_SELENDCANCEL:
      m_select_okay = false;
      break;

    case CBN_SELENDOK:
      {
      //A_DPRINT(A_SOURCE_FUNC_STR " - CBN_SELENDOK     Idx: %u,  Id: %u\n", get_selected_row(), get_selected_info());

      uint row = get_selected_row();

      m_select_okay = true;

      if (row != m_selected_prev)
        {
        on_selected(row, m_selected_prev);
        }

      break;
      }
    }

  return true;  // invoke default behaviour
  }
