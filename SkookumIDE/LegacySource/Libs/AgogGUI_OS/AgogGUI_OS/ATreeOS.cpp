// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATreeOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\ATreeOS.hpp>
#include <AgogIO\AApplication.hpp>
//#include <uxtheme.h>


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * ATreeOS::ms_default_class_p;

//=======================================================================================
// ATreeOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void ATreeOS::initialize()
  {
  INITCOMMONCONTROLSEX init_control_info = { sizeof(INITCOMMONCONTROLSEX), ICC_TREEVIEW_CLASSES };
  InitCommonControlsEx(&init_control_info);
  ms_default_class_p = new AMessageTargetClass(WC_TREEVIEW);
  }

//---------------------------------------------------------------------------------------

void ATreeOS::deinitialize()
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
ATreeOS::ATreeOS(
  AWindow *       parent_p,
  const AFont &   font,   // = AFont::ms_default
  const ARegion & region  // = AWindow::ms_region_def
  ) :
  AWindow(parent_p, ms_default_class_p)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    WS_CHILD | WS_TABSTOP   // Window Styles
      | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,  // ATreeOS Styles
    region.m_x, region.m_y, region.m_width, region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "ATreeOS()", ATreeOS);

  // $Revisit - CReis Looks cool with nice tilting triangle icons instead of plus and other
  // visual enhancements though it uses the theme grid line color so it limits the other
  // colors that can be used. If used must also link to UxTheme.lib in AgogGUI_OS.cpp.
  //::SetWindowTheme(m_os_handle, L"EXPLORER", nullptr);

  m_font = font;
  enable_subclass_messages();
  common_setup();


  // TVS_HASLINES      - The TVS_HASLINES style enhances the graphic representation of a tree-view control's hierarchy by drawing lines that link child items to their parent item. This style does not link items at the root of the hierarchy. To do so, you need to combine the TVS_HASLINES and TVS_LINESATROOT styles. 
  // TVS_HASBUTTONS    - The user can expand or collapse a parent item's list of child items by double-clicking the parent item. A tree-view control that has the TVS_HASBUTTONS style adds a button to the left side of each parent item. The user can click the button once instead of f64-clicking the parent item to expand or collapse the child. TVS_HASBUTTONS does not add buttons to items at the root of the hierarchy. To do so, you must combine TVS_HASLINES, TVS_LINESATROOT, and TVS_HASBUTTONS. 
  // TVS_EDITLABELS    - The TVS_EDITLABELS style makes it possible for the user to edit the labels of tree-view items. For more information about editing labels, see Tree-View Label Editing. 
  // TVS_SHOWSELALWAYS - The TVS_SHOWSELALWAYS style causes a selected item to remain selected when the tree-view control loses focus
  // TVS_INFOTIP       - Version 4.71. Obtains ToolTip information by sending the TVN_GETINFOTIP notification. 
  // TVS_SINGLEEXPAND  - Version 4.71. Causes the item being selected to expand and the item being unselected to collapse upon selection in the tree view. If the mouse is used to single-click the selected item and that item is closed, it will be expanded. If the user holds down the CTRL key while selecting an item, the item being unselected will not be collapsed. 
  //                     Version 5.80. Causes the item being selected to expand and the item being unselected to collapse upon selection in the tree view. If the user holds down the CTRL key while selecting an item, the item being unselected will not be collapsed.
  // TVS_TRACKSELECT   - Version 4.70. Enables hot tracking in a tree-view control. 
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
ATreeOS::~ATreeOS()
  {
  //remove_all();
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS * ATreeOS::get_by_pos(
  int x,
  int y
  ) const
  {
  TVHITTESTINFO hit_info;

  hit_info.pt.x = x;
  hit_info.pt.y = y;

  return handle2item(TreeView_HitTest(m_os_handle, &hit_info));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS * ATreeOS::get_named(
  const AString & item_title,
  uint             instance // = 1u
  ) const
  {
  ATreeItemOS * item_p = get_root();

  while (item_p != nullptr)
    {
    if (item_p->m_title == item_title)
      {
      if (instance <= 1u)
        {
        return item_p;
        }
      instance--;
      }
    item_p = get_next(*item_p);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS * ATreeOS::get_next(const ATreeItemOS & item) const
  {
  HTREEITEM next = get_next_handle(item.m_handle);

  if (next)
    {
    return handle2item(next);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATreeOS::append(
  ATreeItemOS *       item_p,
  const ATreeItemOS * parent_p, // = nullptr
  bool                sort_item // = false
  )
  {
  TVINSERTSTRUCT insert_info;

  insert_info.hParent         = parent_p ? parent_p->m_handle : nullptr;
  insert_info.hInsertAfter    = parent_p ? (sort_item ? TVI_SORT : TVI_LAST) : TVI_ROOT;
  insert_info.item.mask       = TVIF_PARAM | TVIF_TEXT;
  insert_info.item.lParam     = reinterpret_cast<LPARAM>(item_p);
  insert_info.item.pszText    = const_cast<char *>(item_p->m_title.as_cstr());
  insert_info.item.cchTextMax = item_p->m_title.get_length();

  if (item_p->m_bolded)
    {
    insert_info.item.mask      |= TVIF_STATE;
    insert_info.item.stateMask  = TVIS_BOLD;
    insert_info.item.state      = TVIS_BOLD;
    }

  item_p->m_handle = TreeView_InsertItem(m_os_handle, &insert_info);
     
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATreeOS::remove(const ATreeItemOS & item)
  {
  if (item.m_tree_p == this)
    {
    TreeView_DeleteItem(m_os_handle, item.m_handle);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATreeOS::expand_all()
  {
  HTREEITEM handle = TreeView_GetRoot(m_os_handle);

  while (handle)
    {
    TreeView_Expand(m_os_handle, handle, TVE_EXPAND);

    handle = get_next_handle(handle);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the default render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT ATreeOS::on_custom_draw(NMTVCUSTOMDRAW * info_p)
  {
  // uint32_t item_handle = info_p->nmcd.dwItemSpec;
  // uint32_t item_lparam = info_p->nmcd.lItemlParam;

  return CDRF_DODEFAULT;
  }

//---------------------------------------------------------------------------------------
// Signals that the selection is about to be changed from one item to another.
// Returns:    true if item selection is allowed and on_selected() should be called,
//             false if item selection should not change and on_selected() should not be
//             called.
// Arg         item_p - item about to be selected
// Arg         old_item_p - item currently selected
// Modifiers:   virtual
// Author(s):   Conan Reis
bool ATreeOS::on_selecting(
  ATreeItemOS * item_p,
  ATreeItemOS * old_item_p
  )
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Signals that the selection has changed from one item to another.
// Arg         item_p - item newly selected
// Arg         old_item_p - item previously selected
// Modifiers:   virtual
// Author(s):   Conan Reis
void ATreeOS::on_selected(
  ATreeItemOS * item_p,
  ATreeItemOS * old_item_p
  )
  {
  }

//---------------------------------------------------------------------------------------
// Called when an item is removed
// Arg         item_p - removed item
// Modifiers:   virtual
// Author(s):   Conan Reis
void ATreeOS::on_removed(ATreeItemOS * item_p)
  {
  item_p->attempt_self_destruct();
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
bool ATreeOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  NM_TREEVIEW * tree_info_p = reinterpret_cast<NM_TREEVIEW *>(info_p);

  switch (tree_info_p->hdr.code)
    {
    case NM_CUSTOMDRAW:
      *result_p = on_custom_draw(reinterpret_cast<NMTVCUSTOMDRAW *>(info_p));
      return false;

    case TVN_SELCHANGING:    // Signals that the selection is about to be changed from one item to another.  
      return on_selecting(reinterpret_cast<ATreeItemOS *>(tree_info_p->itemNew.lParam), reinterpret_cast<ATreeItemOS *>(tree_info_p->itemOld.lParam));

    case TVN_SELCHANGED:     // Signals that the selection has changed from one item to another.  
      on_selected(reinterpret_cast<ATreeItemOS *>(tree_info_p->itemNew.lParam), reinterpret_cast<ATreeItemOS *>(tree_info_p->itemOld.lParam));
      break;

    case TVN_DELETEITEM:     // Signals the deletion of a specific item. 
      on_removed(reinterpret_cast<ATreeItemOS *>(tree_info_p->itemOld.lParam));
      break;

    // It is often easiest to just sub-class since all the normal events are already handled in AWindow
    /*
    case TVN_SINGLEEXPAND   : A_DPRINT("TVN_SINGLEEXPAND\n"); break;
    case TVN_ITEMEXPANDING  : A_DPRINT("TVN_ITEMEXPANDING\n"); break;
    case TVN_ITEMEXPANDED   : A_DPRINT("TVN_ITEMEXPANDED\n"); break;
    case TVN_BEGINLABELEDIT : A_DPRINT("TVN_BEGINLABELEDIT\n"); break;
    case TVN_ENDLABELEDIT   : A_DPRINT("TVN_ENDLABELEDIT\n"); break;
    case TVN_BEGINDRAG      : A_DPRINT("TVN_BEGINDRAG\n"); break;
    case TVN_BEGINRDRAG     : A_DPRINT("TVN_BEGINRDRAG\n"); break;
    case TVN_GETINFOTIP     : A_DPRINT("TVN_GETINFOTIP\n"); break;
    case TVN_GETDISPINFO    : A_DPRINT("TVN_GETDISPINFO\n"); break;
    case TVN_SETDISPINFO    : A_DPRINT("TVN_SETDISPINFO\n"); break;
    case TVN_KEYDOWN        : A_DPRINT("TVN_KEYDOWN\n"); break;
    case NM_CLICK           : A_DPRINT("NM_CLICK\n"); break;
    case NM_CUSTOMDRAW      : A_DPRINT("NM_CUSTOMDRAW\n"); break;
    case NM_DBLCLK          : A_DPRINT("NM_DBLCLK\n"); break;
    case NM_KILLFOCUS       : A_DPRINT("NM_KILLFOCUS\n"); break;
    case NM_RCLICK          : A_DPRINT("NM_RCLICK\n"); break;
    case NM_RDBLCLK         : A_DPRINT("NM_RDBLCLK\n"); break;
    case NM_RETURN          : A_DPRINT("NM_RETURN\n"); break;
    case NM_SETFOCUS        : A_DPRINT("NM_SETFOCUS\n"); break;
    default: A_DPRINT("Control Code: 0x%x\n", tree_info_p->hdr.code);
    */

    // TVN_ASYNCDRAW    (Vista)
    // TVN_ITEMCHANGED  (Vista)
    // TVN_ITEMCHANGING (Vista)
    }

  return true;  // invoke default behaviour
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converts item handle to ATreeItemOS object
// Author(s):   Conan Reis
ATreeItemOS * ATreeOS::handle2item(HTREEITEM handle) const
  {
  if (handle == 0)
    {
    return nullptr;
    }

  TVITEMEX item_info;

  item_info.mask  = TVIF_PARAM;
  item_info.hItem = handle;

  BOOL success = TreeView_GetItem(m_os_handle, &item_info);

  A_ASSERT(success, "ATreeOS::handle2item - invalid item handle", AErrId_invalid_index, ATreeOS);

  return reinterpret_cast<ATreeItemOS *>(item_info.lParam);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
HTREEITEM ATreeOS::get_next_handle(HTREEITEM handle) const
  {
  HTREEITEM next = TreeView_GetChild(m_os_handle, handle);

  if (next)
    {
    return next;
    }

  next = TreeView_GetNextSibling(m_os_handle, handle);

  if (next)
    {
    return next;
    }

  next = TreeView_GetParent(m_os_handle, handle);

  while (next)
    {
    HTREEITEM sibling = TreeView_GetNextSibling(m_os_handle, next);

    if (sibling)
      {
      return sibling;
      }

    next = TreeView_GetParent(m_os_handle, next);
    }

  return nullptr;
  }


//=======================================================================================
// ATreeItemOS Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS::ATreeItemOS() :
  m_title(AString::ms_empty),
  m_tree_p(nullptr),
  m_handle(0),
  m_self_destruct(true),
  m_bolded(false)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS::ATreeItemOS(
  const AString & title,
  bool            bolded,       // = false
  bool            self_destruct // = true
  ) :
  m_title(title),
  m_tree_p(nullptr),
  m_handle(0),
  m_self_destruct(self_destruct),
  m_bolded(bolded)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ATreeItemOS::~ATreeItemOS()
  {
  }
