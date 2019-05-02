// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AListIdxOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AListOS.hpp>
#include <AgogCore\AStringRef.hpp>
#include <AgogIO\AApplication.hpp>
#include <StdIO.h>


//=======================================================================================
// Globals, Constants, & Defines
//=======================================================================================

#define ALISTOS_STATE_IMAGE_FLAGS_TO_INDEX(_flags) ((_flags) >> 12)

namespace
{

  // AListOS Enumerated Constants
  enum
    {
    AListOS_new_index = INT32_MAX
    };

} // End unnamed namespace


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * AListIdxOS::ms_default_class_p;

//=======================================================================================
// AListIdxOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AListIdxOS::initialize()
  {
  INITCOMMONCONTROLSEX init_control_info = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
  InitCommonControlsEx(&init_control_info);
    
  // $Revisit - CReis Window class subclassing should be looked into to provide greater
  // and more consistent functionality.
  ms_default_class_p = new AMessageTargetClass(WC_LISTVIEW);
  }

//---------------------------------------------------------------------------------------

void AListIdxOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
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
AListIdxOS::AListIdxOS(
  AWindow *       parent_p,
  const AFont &   font,   // = AFont::ms_default
  const ARegion & region  // = AWindow::ms_region_def
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_remove_events(false)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    WS_CHILD | WS_TABSTOP   // Window Styles
      | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,  // AListIdxOS Styles
    region.m_x, region.m_y, region.m_width, region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "AListIdxOS()", AListIdxOS);

  // LVS_EX_DOUBLEBUFFER
  ListView_SetExtendedListViewStyle(m_os_handle, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_LABELTIP);

  m_font = font;
  enable_subclass_messages();
  common_setup();

  // Other List View styles of interest:

  // LVS_NOCOLUMNHEADER  - Column headers are not displayed in report view. By default, columns have headers in report view. 
  // LVS_NOSORTHEADER    - Column headers do not work like buttons. This style can be used if clicking a column header in report view does not carry out an action, such as sorting. 

  // LVS_EDITLABELS      - Item text can be edited in place. The parent window must process the LVN_ENDLABELEDIT notification message. 

  // LVS_SHAREIMAGELISTS - The image list will not be deleted when the control is destroyed. This style enables the use of the same image lists with multiple list-view controls. 
  // LVS_OWNERDRAWFIXED  - The owner window can paint items in report view. The list-view control sends a WM_DRAWITEM message to paint each item; it does not send separate messages for each subitem. The itemData member of the DRAWITEMSTRUCT structure contains the item data for the specified list-view item. 

  // LVS_OWNERDATA       - This style specifies a virtual list-view control. For more information about this list control style, see List-View Controls. 
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
AListIdxOS::~AListIdxOS()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Enable / disable ability to change the order of columns by dragging them
//             to new positions.
// Arg         allow_drag - if true allow reordering by drag and drop, if false do not
//             allow reordering.
// Author(s):   Conan Reis
void AListIdxOS::enable_header_swapping(
  bool allow_drag // = true
  )
  {
  if (allow_drag)
    {
    ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
    }
  else
    {
    ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_HEADERDRAGDROP, 0u);
    }
  }

//---------------------------------------------------------------------------------------
// Enable / disable the drawing of row and column grid lines.
// Arg         draw_grid - if true lines are drawn to delineate rows and columns, if
//             false no lines are drawn.
// Author(s):   Conan Reis
void AListIdxOS::enable_gridlines(
  bool draw_grid // = true
  )
  {
  if (draw_grid)
    {
    ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
    }
  else
    {
    ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_GRIDLINES, 0u);
    }
  }

//---------------------------------------------------------------------------------------
// Enable / disable the editing of labels in place.
// Author(s):   Conan Reis
void AListIdxOS::enable_user_label_edit(
  bool allow_edit // = true
  )
  {
  if (allow_edit)
    {
	append_style(LVS_EDITLABELS);
    }
  else
    {
	remove_style(LVS_EDITLABELS);
    }
  }

//---------------------------------------------------------------------------------------
// Enable / disable multiple selection.
// Author(s):   Conan Reis
void AListIdxOS::enable_multi_select(
  bool multi_select // = true
  )
  {
  if (multi_select)
    {
	remove_style(LVS_SINGLESEL);
    }
  else
    {
	append_style(LVS_SINGLESEL);
    }
  }

//---------------------------------------------------------------------------------------
// Set hover mode.
// Arg         mode - one of:
//               AListHover_off
//               AListHover_track_auto_select  - Track mouse over subitems and auto select if mouse rests over subitem
//               AListHover_track_activate2    - Track mouse over subitems and auto activate if a selected subitem is clicked a second time
//               AListHover_track_activate1    - Track mouse over subitems and auto activate if a subitem is clicked
// Author(s):   Conan Reis
void AListIdxOS::set_hover(eAListHover mode)
  {
  switch (mode)
    {
    case AListHover_off:
      ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE | LVS_EX_ONECLICKACTIVATE, 0u);
      break;

    case AListHover_track_auto_select:  // Track mouse over subitems and auto select if mouse rests over subitem
      ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE | LVS_EX_ONECLICKACTIVATE, LVS_EX_TRACKSELECT);
      break;

    case AListHover_track_activate2:    // Track mouse over subitems and auto activate if a selected subitem is clicked a second time
      ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE | LVS_EX_ONECLICKACTIVATE, LVS_EX_TWOCLICKACTIVATE);
      break;

    case AListHover_track_activate1:    // Track mouse over subitems and auto activate if a subitem is clicked
      ListView_SetExtendedListViewStyleEx(m_os_handle, LVS_EX_TRACKSELECT | LVS_EX_TWOCLICKACTIVATE | LVS_EX_ONECLICKACTIVATE, LVS_EX_ONECLICKACTIVATE);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
uint AListIdxOS::pos2row(
  const AVec2i & client_pos,
  uint *           rank_p // = nullptr
  ) const
  {
  LVHITTESTINFO hit_info;

  hit_info.pt.x = client_pos.m_x;
  hit_info.pt.y = client_pos.m_y;

  if (rank_p)
    {
    *rank_p = ListView_SubItemHitTest(m_os_handle, &hit_info);

    return hit_info.iItem;
    }
  else
    {
    return ListView_HitTest(m_os_handle, &hit_info);
    }
  }

//---------------------------------------------------------------------------------------
// Gets the currently selected row indexes
// Returns:    The number of selected rows
// Arg         rows_p[] - array address to store selected row indexes.  Must be at least
//             large enough for get_selected_count() index elements.
// See:        get_selected_count()
// Author(s):   Conan Reis
uint AListIdxOS::get_selected_rows(uint rows_p[]) const
  {
  uint selected_count = 0u;
  int next_item      = -1;

  do
    {
    next_item = ListView_GetNextItem(m_os_handle, next_item, LVNI_SELECTED);

    if (next_item != -1)
      {
      rows_p[selected_count] = next_item;
      selected_count++;
      }
    }
  while (next_item != -1);

  return selected_count;
  }

//---------------------------------------------------------------------------------------
// Starts user label edit for specified row
// Notes:      enable_user_label_edit() should already be called with true in order to allow
//             label editing.
// Author(s):   Conan Reis
void AListIdxOS::label_user_edit(uint row)
  {
  set_focus();
  ListView_EditLabel(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Cancels any current user label edit
// Notes:      enable_user_label_edit() should already be called with true in order to allow
//             label editing.
// Author(s):   Conan Reis
void AListIdxOS::label_user_edit_cancel()
  {
  set_focus();
  ListView_EditLabel(m_os_handle, -1);
  }

//---------------------------------------------------------------------------------------
// Converts item info to item row
// Author(s):   Conan Reis
uint AListIdxOS::info2row(uintptr_t info) const
  {
  LVFINDINFO find_info;

  find_info.flags  = LVFI_PARAM;
  find_info.lParam = info;

  return ListView_FindItem(m_os_handle, -1, &find_info);
  }

//---------------------------------------------------------------------------------------
// Converts item row to item info
// Author(s):   Conan Reis
uintptr_t AListIdxOS::row2info(uint row) const
  {
  A_ASSERT(row != AListOS_invalid_index, "AListIdxOS::handle2item - invalid item handle", AErrId_invalid_index, AListIdxOS);

  LVITEM item_info;

  item_info.mask     = LVIF_PARAM;
  item_info.iItem    = row;
  item_info.iSubItem = 0;

  BOOL success = ListView_GetItem(m_os_handle, &item_info);

  A_ASSERT(success, "AListIdxOS::handle2item - invalid item handle", AErrId_invalid_index, AListIdxOS);

  return item_info.lParam;
  }

//---------------------------------------------------------------------------------------
// Returns:    Row index of appended info.
// Author(s):   Conan Reis
uint AListIdxOS::append(
  uintptr_t info,
  uint       icon_image, // = AListOS_image_default
  uint       state_image // = AListOS_image_default
  )
  {
  LVITEM item_info;

  item_info.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
  item_info.iItem     = AListOS_new_index;
  item_info.iSubItem  = 0;
  item_info.pszText   = LPSTR_TEXTCALLBACK;
  item_info.iImage    = icon_image;
  item_info.lParam    = info;
  item_info.stateMask = LVIS_STATEIMAGEMASK;
  item_info.state     = INDEXTOSTATEIMAGEMASK(state_image);

  // WM_SETREDRAW Can be used to disable redrawing for faster adding

  return ListView_InsertItem(m_os_handle, &item_info);
  }


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
uint AListIdxOS::column_append(
  const AString & title,
  int             width, // = AListOS_column_width_title
  eAColumnAlign   align  // = AColumnAlign_left
  )
  {
  LVCOLUMN col_info;

  // $Note - CReis The iSubItem is only 32-bits in both 32-bit and 64-bit builds so it may
  // not store a pointer address.
  col_info.mask     = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  col_info.fmt      = align;
  col_info.pszText  = const_cast<char *>(title.as_cstr());
  col_info.cx       = (width < 0)
    ? (ListView_GetStringWidth(m_os_handle, title.as_cstr()) + AListOS_column_width_padding)
    : width;

  return ListView_InsertColumn(m_os_handle, AListOS_new_index, &col_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAColumnAlign AListIdxOS::column_get_align_rank(uint rank) const
  {
  LVCOLUMN column_info;

  column_info.mask = LVCF_FMT;

  ListView_GetColumn(m_os_handle, rank, &column_info);

  return eAColumnAlign(column_info.fmt & LVCFMT_JUSTIFYMASK);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AListIdxOS::column_set_align_rank(
  uint           rank,
  eAColumnAlign align
  )
  {
  LVCOLUMN column_info;

  column_info.mask = LVCF_FMT;

  ListView_GetColumn(m_os_handle, rank, &column_info);

  int new_fmt = align | (column_info.fmt & ~LVCFMT_JUSTIFYMASK);

  if (new_fmt != column_info.fmt)
    {
    column_info.fmt = new_fmt;
    ListView_SetColumn(m_os_handle, rank, &column_info);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AListIdxOS::column_set_title_rank(
  uint             rank,
  const AString & title
  )
  {
  LVCOLUMN column_info;

  column_info.mask    = LVCF_TEXT;
  column_info.pszText = const_cast<char *>(title.as_cstr());

  ListView_GetColumn(m_os_handle, rank, &column_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AListIdxOS::set_image_list(
  const AImageListOS & images,
  eAListImage          type // = AListImage_small_icon
  )
  {
  ListView_SetImageList(m_os_handle, images.get_handle(), type);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AImageListOS AListIdxOS::get_image_list(
  eAListImage type // = AListImage_small_icon
  ) const
  {
  return ListView_GetImageList(m_os_handle, type);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AListIdxOS::set_image_index(
  uint         image_idx,
  uint         row,
  uint         rank, // = 0u
  eAListImage type  // = AListImage_icon
  )
  {
  LVITEM item_info;

  item_info.iItem    = row;
  item_info.iSubItem = rank;

  if (type == AListImage_state)
    {
    item_info.mask      = LVIF_STATE;
    item_info.stateMask = LVIS_STATEIMAGEMASK;
    item_info.state     = INDEXTOSTATEIMAGEMASK(image_idx);
    }
  else
    {
    item_info.mask   = LVIF_IMAGE;
    item_info.iImage = image_idx;
    }

  ListView_SetItem(m_os_handle, &item_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
uint AListIdxOS::get_image_index(
  uint         row,
  uint         rank, // = 0u
  eAListImage type  // = AListImage_icon
  ) const
  {
  LVITEM item_info;

  item_info.iItem    = row;
  item_info.iSubItem = rank;

  if (type == AListImage_state)
    {
    item_info.mask      = LVIF_STATE;
    item_info.stateMask = LVIS_STATEIMAGEMASK;
    }
  else
    {
    item_info.mask = LVIF_IMAGE;
    }

  ListView_SetItem(m_os_handle, &item_info);

  return (type == AListImage_state)
    ? ALISTOS_STATE_IMAGE_FLAGS_TO_INDEX(item_info.state)
    : item_info.iImage;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called by on_control_event()
// Notes:      This method is intentionally inline *and* in the cpp file
// Author(s):   Conan Reis
inline void AListIdxOS::parse_attempt_click(
  NMITEMACTIVATE * info_p,
  eAMouse          button,
  bool             double_click
  )
  {
  if (info_p->iItem != AListOS_invalid_index)
    {
    // iItem, iSubItem, and ptAction are valid
    on_subitem_clicked(info_p->iItem, info_p->iSubItem, button, double_click);
    }
  }

//---------------------------------------------------------------------------------------
// Called by on_control_event()
// Author(s):   Conan Reis
void AListIdxOS::parse_subitem_get_text(NMLVDISPINFO * info_p)
  {
  if ((info_p->item.mask & LVIF_TEXT) == 0u)
    {
    // Not looking for text info
    return;
    }

  uint32_t item_text_size = info_p->item.cchTextMax;

  //A_ASSERTX(
  //  ((info_p->item.mask == LVIF_TEXT) || (info_p->item.mask == (LVIF_TEXT | LVIF_IMAGE)))
  //    && (item_text_size > 0),
  //  "Unexpected list view 'get text' info structure!");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // pszText and cchTextMax members specify the address and size of a buffer. You can
  // either copy text to the buffer or assign the address of a AString to the pszText
  // member. In the latter case, you must not change or delete the AString until the
  // corresponding item text is deleted or two additional LVN_GETDISPINFO messages
  // have been sent.

  bool    store_text = true;
  AString str(info_p->item.pszText, item_text_size, 0u);

  on_subitem_get_text(info_p->item.iItem, info_p->item.iSubItem, &str, &store_text);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // New string buffer?
  char * cstr_p = const_cast<char *>(str.as_cstr());

  if (cstr_p != info_p->item.pszText)
    {
    // Make a copy if storing the text or if set to deallocate
    if (store_text || str.get_str_ref()->m_deallocate)
      {
      AString trunc_str(info_p->item.pszText, item_text_size, 0u);
      uint     str_length = str.get_length();

      trunc_str.empty();

      if (str_length >= (item_text_size - 1u))
        {
        if (item_text_size >= 6u)
          {
          trunc_str.append(cstr_p, item_text_size - 6u);
          trunc_str.append(" ...", 4u);
          }
        else
          {
          // Not enough space in free buffer
          info_p->item.pszText = "...";
          }
        }
      else
        {
        trunc_str.append(str);
        }
      }
    else
      {
      info_p->item.pszText = cstr_p;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the LVIF_DI_SETITEM flag in the mask member of the LVITEM structure if the
  // list-view control should store the requested information and not ask for it again
  if (store_text)
    {
    info_p->item.mask |= LVIF_DI_SETITEM;
    }
  }

//---------------------------------------------------------------------------------------
// Called by on_control_event()
// Author(s):   Conan Reis
void AListIdxOS::parse_item_changed(NMLISTVIEW * info_p)
  {
  // $Revisit - CReis I am ignoring changes that apply to all items, but something useful
  // might be missed - look into further.
  if ((info_p->iItem != AListOS_invalid_index) && (info_p->uChanged & LVIF_STATE))
    {
    // iItem and lParam are valid
    if ((info_p->uNewState & LVIS_FOCUSED) && ((info_p->uOldState & LVIS_FOCUSED) == 0u))
      {
      on_item_focused_row(info_p->iItem, info_p->lParam);
      }

	bool new_select = (info_p->uNewState & LVIS_SELECTED) != 0u;
	bool old_select = (info_p->uOldState & LVIS_SELECTED) != 0u;

    if ((new_select) && (!old_select))
      {
      // Newly selected
      on_item_selected_row(info_p->iItem, info_p->lParam, true);
      }

    if ((!new_select) && (old_select))
      {
      // Newly deselected
      on_item_selected_row(info_p->iItem, info_p->lParam, false);
      }
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the list render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT AListIdxOS::on_custom_draw(NMLVCUSTOMDRAW * info_p)
  {
  // uint32_t item_info = info_p->nmcd.lItemlParam;
  // uint  row       = info_p->nmcd.dwItemSpec;
  // uint  rank      = info_p->iSubItem;

  return CDRF_DODEFAULT;
  }

//---------------------------------------------------------------------------------------
// Called just prior to starting a user label edit.
// Returns:    true if the edit is to be allowed, false if it is to be disallowed.
// Arg         row - index of row - can call row2info to get associated info
// Arg         item_info - user info associated with item
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool AListIdxOS::on_label_user_edit(
  uint       row,
  uintptr_t item_info
  )
  {
  // Allow edit
  return true;
  }

//---------------------------------------------------------------------------------------
// Called just after a user label edit.
// Returns:    true if the edit is to be accepted, false if it is not to be accepted.
// Arg         label_p - new label or nullptr if edit was aborted/canceled
// Arg         row - index of row - can call row2info to get associated info
// Arg         item_info - user info associated with item
// See:        on_edit_lable(), label_user_edit(), label_user_edit_cancel(), enable_user_label_edit()
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool AListIdxOS::on_label_user_edited(
  uint       row,
  uintptr_t item_info,
  AString * label_p
  )
  {
  // Allow label change
  return true;
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Arg         item_info - user info associated with item
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void AListIdxOS::on_item_focused_row(
  uint        row,
  uintptr_t item_info
  )
  {
  }

//---------------------------------------------------------------------------------------
// Called on initial selection or deselection of a row.
// Arg         row - index of row - can call row2info to get associated info
// Arg         item_info - user info associated with item
// Arg         selected - true if selected, false if deselected
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void AListIdxOS::on_item_selected_row(
  uint       row,
  uintptr_t item_info,
  bool      selected
  )
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Arg         item_info - 
// See:        enable_remove_events()
// Notes:      This method is only called if enable_remove_events() is called with true.
//             Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_item_removed_row(
  uint       row,
  uintptr_t item_info
  )
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_column_clicked_rank(uint rank)
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Arg         rank -
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_subitem_hovered(
  uint row,
  uint rank
  )
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_subitem_clicked(
  uint     row,
  uint     rank,
  eAMouse button,
  bool    double_click
  )
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_subitem_activated(
  uint row,
  uint rank
  )
  {
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void AListIdxOS::on_subitem_get_text(
  uint       row,
  uint       rank,
  AString * subitem_str_p,
  bool *    save_text_p
  )
  {
  //subitem_str_p->format("Row: %i(%u),  Col: %i",
  //  row, row2info(row),
  //  rank);
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
// Modifiers:   virtual (from AWindow)
// Author(s):   Conan Reis
bool AListIdxOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  switch (info_p->code)
    {
    // Handled by AWindow::parse_control()
    // NM_SETFOCUS
    // NM_KILLFOCUS

    case NM_CUSTOMDRAW:
      *result_p = on_custom_draw(reinterpret_cast<NMLVCUSTOMDRAW *>(info_p));
      return false;

    case LVN_ITEMCHANGED:
      parse_item_changed(reinterpret_cast<NMLISTVIEW *>(info_p));
      break;

    case LVN_HOTTRACK:
      {
      NM_LISTVIEW * list_info_p = reinterpret_cast<NMLISTVIEW *>(info_p);

      // iItem, iSubItem, and ptAction are valid
      on_subitem_hovered(list_info_p->iItem, list_info_p->iSubItem);
      }
      break;

    case NM_CLICK:
      parse_attempt_click(reinterpret_cast<NMITEMACTIVATE *>(info_p), AMouse_left, false);
      break;

    case NM_DBLCLK:
      parse_attempt_click(reinterpret_cast<NMITEMACTIVATE *>(info_p), AMouse_left, true);
      break;

    case NM_RCLICK:
      parse_attempt_click(reinterpret_cast<NMITEMACTIVATE *>(info_p), AMouse_right, false);
      break;

    case NM_RDBLCLK:
      parse_attempt_click(reinterpret_cast<NMITEMACTIVATE *>(info_p), AMouse_right, true);
      break;

    case LVN_ITEMACTIVATE:
      {
      NMITEMACTIVATE * activate_info_p = reinterpret_cast<NMITEMACTIVATE *>(info_p);

      // iItem, iSubItem, and ptAction are valid
      on_subitem_activated(activate_info_p->iItem, activate_info_p->iSubItem);
      }
      break;

    case LVN_GETDISPINFO:
      parse_subitem_get_text(reinterpret_cast<NMLVDISPINFO *>(info_p));
      break;

    case LVN_COLUMNCLICK:
      on_column_clicked_rank(reinterpret_cast<NM_LISTVIEW *>(info_p)->iSubItem);
      break;

    case LVN_DELETEALLITEMS:
      return !m_remove_events;

    case LVN_DELETEITEM:
      if (m_remove_events)
        {
        NM_LISTVIEW * list_info_p = reinterpret_cast<NMLISTVIEW *>(info_p);

        // iItem and lParam are valid
        on_item_removed_row(list_info_p->iItem, list_info_p->lParam);
        }
      break;

    case LVN_BEGINLABELEDIT:
      {
      LVITEM * item_p = &reinterpret_cast<NMLVDISPINFO *>(info_p)->item;

      // Allow label edit?  Note that Boolean value is negated.
      *result_p = on_label_user_edit(item_p->iItem, item_p->lParam) ? FALSE : TRUE;

      break;
      }

    case LVN_ENDLABELEDIT:
      {
      LVITEM *  item_p = &reinterpret_cast<NMLVDISPINFO *>(info_p)->item;
      AString   label;
	  AString * label_p = nullptr;
	  
      if (item_p->pszText)
        {
        label.set_buffer(item_p->pszText, item_p->cchTextMax, ALength_calculate, false);
        label_p = &label;
        }

      // Allow label change?
      *result_p =
        on_label_user_edited(item_p->iItem, item_p->lParam, label_p)
          ? TRUE
          : FALSE;

      break;
      }

    // Other potentially interesting notifications:

    // LVN_ITEMCHANGING 
    // LVN_ITEMCHANGED 

    // LVN_BEGINDRAG 
    // LVN_BEGINRDRAG

    // LVN_INSERTITEM 

    // LVN_SETDISPINFO 
    // LVN_ODCACHEHINT 
    // LVN_ODFINDITEM 
    // LVN_ODSTATECHANGED 

    // NM_HOVER      // NMHDR
    // LVN_KEYDOWN 
    // LVN_MARQUEEBEGIN 
    // NM_RELEASEDCAPTURE
    // NM_RETURN
    }

  return true;  // invoke default behaviour
  }

