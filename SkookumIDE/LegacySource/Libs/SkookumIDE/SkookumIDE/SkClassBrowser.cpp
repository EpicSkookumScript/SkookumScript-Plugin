// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Browser & supporting classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkClassBrowser.hpp>
#include <SkookumIDE/SkConsole.hpp>
#include <stdio.h>
#include <AgogCore/ACompareMethod.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogIO/AClipboard.hpp>
#include <AgogIO/AIni.hpp>
#include <AgogGUI/ATrueTypeFont.hpp>
#include <AgogGUI_OS/ADialogOS.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumIDE/SkookumIDE_Res.h>
#include <objidl.h>

#pragma warning( push )
 #pragma warning( disable : 4458 ) // hidden class member
 #include <gdiplus.h>
#pragma warning( pop )


//=======================================================================================
// Local Global Structures
//=======================================================================================

typedef AMethodArg<SkDataList, tSkSubText *>          tSkDataTextCall;
typedef ACompareMethod<SkDataList, SkMemberReference> tSkDataCompareCall;

typedef AMethodArg<SkCodeList, tSkSubText *>          tSkCodeTextCall;
typedef ACompareMethod<SkCodeList, SkMemberReference> tSkCodeCompareCall;

eSkMemberImage g_member_type_to_image[SkMember__length] =
  {
  SkMemberImage_method,
  SkMemberImage_method_atomic,
  SkMemberImage_method_atomic,
  SkMemberImage_coroutine,
  SkMemberImage_coroutine_atomic,
  SkMemberImage_coroutine_atomic,
  SkMemberImage_data,
  SkMemberImage_class
  };

// Always show error types as custom rather than non-atomic
eSkMemberImage g_member_type_to_error_image[SkMember__length] =
  {
  SkMemberImage_method,
  SkMemberImage_method,
  SkMemberImage_method,
  SkMemberImage_coroutine,
  SkMemberImage_coroutine,
  SkMemberImage_coroutine,
  SkMemberImage_data,
  SkMemberImage_class
  };

namespace
{

  const bool SkMemberView_show_instance_def   = true;
  const bool SkMemberView_show_class_def      = true;
  const bool SkMemberView_show_inherited_def  = true;
  const bool SkMemberView_show_custom_def     = true;
  const bool SkMemberView_show_atomic_def     = true;
  const bool SkMemberView_show_methods_def    = true;
  const bool SkMemberView_show_coroutines_def = true;

  const bool SkEditor_auto_parse_def          = true;

  // Enumerated constants
  enum
    {
    // $Revisit - CReis Many of these initial values should be provided by application defined functions.

    SkContextInfo_data_size_init    = 64,
    SkContextInfo_code_size_init    = 256,
    SkContextInfo_population_init   = 512,  // Initial simultaneous member info structures in member lists
    SkContextInfo_population_expand = 128,  // Member info structure grow amount - used in member lists

    SkMemberView_gui_inset           = 4,
    SkMemberView_history_max         = 100,

    SkClassBrowser_status_inset     = 2,
    SkClassBrowser_status_inset2    = SkClassBrowser_status_inset * 2,
    SkClassBrowser_status_offset    = 2,
    SkClassBrowser_log_str_length   = 600
    };

  enum eSkBrowseMenu
    {
    SkBrowseMenu_unimplemented            = 1,

      // Format
      SkBrowseMenu_edit_indent            = 2100,
      SkBrowseMenu_edit_unindent,
      SkBrowseMenu_edit_tabs_to_spaces,
      SkBrowseMenu_edit_trim_trailing_spaces,
      SkBrowseMenu_edit_lowercase,
      SkBrowseMenu_edit_uppercase,
      SkBrowseMenu_edit_capitalize,
      SkBrowseMenu_edit_sort,

      SkBrowseMenu_edit_comment_block,
      SkBrowseMenu_edit_comment_lines,
      SkBrowseMenu_edit_block_wrap,

/*
      // Version Control
      SkBrowseMenu_edit_p4_toggle         = 2200,
      SkBrowseMenu_edit_p4_checkout,
      SkBrowseMenu_edit_p4_revert,
      SkBrowseMenu_edit_p4_diff,
      SkBrowseMenu_edit_p4_history,
      SkBrowseMenu_edit_p4_timelapse,
      SkBrowseMenu_edit_p4_properties
*/
    };

  // $Revisit - CReis If these were AString objects it would be a little faster

  // *IDE* config (.ini) file
  const char * g_ini_section_browser_p            = "Script Browser";
  const char * g_ini_key_instance_p               = "Instance";
  const char * g_ini_key_class_p                  = "Class";
  const char * g_ini_key_inherited_p              = "Inherited";
  const char * g_ini_key_custom_p                 = "Custom";
  const char * g_ini_key_atomic_p                 = "Atomic";
  const char * g_ini_key_methods_p                = "Methods";
  const char * g_ini_key_coroutines_p             = "Coroutines";
  const char * g_ini_key_last_class_p             = "LastClass";
  const char * g_ini_key_last_member_p            = "LastMember";
  const char * g_ini_key_user_name_p              = "UserName";
  const char * g_ini_key_split_members_ratio_p    = "SplitMembersRatio";
  const char * g_ini_key_split_members_orient_p   = "SplitMembersOrient";
  const char * g_ini_key_split_tree_ratio_p       = "SplitTreeRatio";
  const char * g_ini_key_split_tree_orient_p      = "SplitTreeOrient";
  const char * g_ini_key_split_data_code_ratio_p  = "SplitDataCodeRatio";
  const char * g_ini_key_split_data_code_orient_p = "SplitDataCodeOrient";

  const char * g_ini_section_debugger_p           = "Debugger";
  const char * g_ini_key_break_callstack_p        = "CallstackOnBreak";
  const char * g_ini_key_break_locals_p           = "LocalsOnBreak";

  static const AColor g_colour_okay(        0.7f,  1.0f,  0.25f);     // Light Green
  static const AColor g_colour_okay_dark(   0.52f, 0.75f, 0.15f);     // Med Green
  static const AColor g_colour_unbound(     1.0f,  0.65f, 0.0f);      // Light orange
  static const AColor g_colour_unbound_dark(0.8f,  0.52f, 0.0f);      // Med orange
  static const AColor g_color_error(        0.86f, 0.08f, 0.24f);     // Crimson
  static const AColor g_color_bg(           0.18f, 0.18f, 0.22f);     // Pro
  static const AColor g_color_text_bg(      0.15f, 0.15f, 0.19f);     // Pro Edit partial
  static const AColor g_color_text_edit_bg( 0.13f, 0.13f, 0.16f);     // Pro Edit
  static const AColor g_colour_expr_guide(  0.0f,  0.5f,  1.0f, 0.6); // Expression guide marks

} // End unnamed namespace


//=======================================================================================
// SkMemberReference Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Retrieves a SkMemberReference object from the dynamic pool and initializes
//              it for use.  This should be used instead of 'new' because it prevents
//              unnecessary allocations by reusing previously allocated objects.
// Returns:     a dynamic SkMemberReference
// See:         pool_delete()
// Notes:       To 'deallocate' an object that was retrieved with this method, use
//              'pool_delete()' rather than 'delete'.
// Modifiers:    static
// Author(s):    Conan Reis
SkMemberReference * SkMemberReference::pool_new(
  const SkContextInfo & member_info,
  eStatus               status
  )
  {
  SkMemberReference * info_p = get_pool().allocate();

  info_p->init(member_info, status);

  return info_p;
  }

//---------------------------------------------------------------------------------------
// Returns dynamic reference pool. Pool created first call and reused on successive calls.
// 
// #Notes
//   Uses Scott Meyers' tip "Make sure that objects are initialized before they're used"
//   from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd Edition]
//   This is instead of using a non-local static object for a singleton.
//   
// #Modifiers  static
// #Author(s)  Conan Reis
AObjReusePool<SkMemberReference> & SkMemberReference::get_pool()
  {
  static AObjReusePool<SkMemberReference> s_pool(SkContextInfo_population_init, SkContextInfo_population_expand);

  return s_pool;
  }


//=======================================================================================
// SkDataList Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkDataList::SkDataList(
  AWindow *      parent_p,
  SkMemberView * member_view_p
  ) :
  AListOS<SkMemberReference>(parent_p, SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_member_view_p(member_view_p)
  {
  ListView_SetTextBkColor(m_os_handle, g_color_text_bg);  // BG with text
  ListView_SetBkColor(m_os_handle, g_color_text_bg);      // BG without text
  enable_remove_events();
  enable_image_sharing();
  set_image_list(SkConsole::ms_console_p->get_member_images());
  enable_gridlines();
  ensure_size(SkContextInfo_data_size_init);
  set_border(Border_sunken);

  m_col_instance_p = new AColumnOS<SkMemberReference>("i/c", nullptr, new tSkDataCompareCall(this, &SkDataList::on_compare_instance), false);
  column_append(m_col_instance_p);

  m_col_scope_p = new AColumnOS<SkMemberReference>(
    "Scope",
    new tSkDataTextCall(this, &SkDataList::on_text_scope),
    new tSkDataCompareCall(this, &SkDataList::on_compare_scope));
  column_append(m_col_scope_p);

  m_col_name_p = new AColumnOS<SkMemberReference>(
    "Name",
    new tSkDataTextCall(this, &SkDataList::on_text_name),
    new tSkDataCompareCall(this, &SkDataList::on_compare_name),
    false);
  column_append(m_col_name_p);

  m_col_type_p = new AColumnOS<SkMemberReference>(
    "Type",
    new tSkDataTextCall(this, &SkDataList::on_text_type),
    new tSkDataCompareCall(this, &SkDataList::on_compare_type));
  column_append(m_col_type_p);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkDataList::~SkDataList()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::append_member(bool class_scope, SkClass * class_p, const ASymbol & name)
  {
  SkContextInfo member_info(SkQualifier(name, class_p), SkMember_data, class_scope);

  append(
    *m_member_view_p->pool_new_info(member_info),
    class_scope ? SkMemberImage_scope_class : SkMemberImage_scope_instance);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::set_class(SkClass * class_p)
  {
  remove_all();

  if (class_p)
    {
    // Populate list
    bool show_instance   = m_member_view_p->m_toggle_instance.is_toggled();
    bool show_class      = m_member_view_p->m_toggle_class.is_toggled();
    bool show_inherited  = m_member_view_p->m_toggle_inherited.is_toggled();

    SkClass * add_class_p = class_p;

    while (add_class_p)
      {
      // Add Data Members
      if (show_instance)
        {
        SkTypedName ** data_pp     = add_class_p->get_instance_data().get_array();
        SkTypedName ** data_end_pp = data_pp + add_class_p->get_instance_data().get_length();

        for (; data_pp < data_end_pp; data_pp++)
          {
          append_member(false, add_class_p, (*data_pp)->get_name());
          }

        SkTypedNameRaw ** data_raw_pp     = add_class_p->get_instance_data_raw().get_array();
        SkTypedNameRaw ** data_raw_end_pp = data_raw_pp + add_class_p->get_instance_data_raw().get_length();

        for (; data_raw_pp < data_raw_end_pp; data_raw_pp++)
          {
          append_member(false, add_class_p, (*data_raw_pp)->get_name());
          }
        }

      if (show_class)
        {
        SkTypedName ** data_pp     = add_class_p->get_class_data().get_array();
        SkTypedName ** data_end_pp = data_pp + add_class_p->get_class_data().get_length();

        for (; data_pp < data_end_pp; data_pp++)
          {
          append_member(true, add_class_p, (*data_pp)->get_name());
          }
        }

      // Add inherited members
      add_class_p = show_inherited ? add_class_p->get_superclass() : nullptr;
      }

    // Default sort by type and name
    m_col_name_p->sort();
    }
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
void SkDataList::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_member_view_p->on_mouse_release(button, buttons, client_pos);
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
bool SkDataList::on_context_menu(const AVec2i & screen_pos)
  {
  enum eDataPop
    {
    DataPop_rename,
    DataPop_remove,
    DataPop_duplicate,
    DataPop_new,

    // Version Control
      //SkBrowseMenu_edit_p4_checkout,
      //SkBrowseMenu_edit_p4_revert,
      //SkBrowseMenu_edit_p4_diff,
      //SkBrowseMenu_edit_p4_history,
      //SkBrowseMenu_edit_p4_timelapse,
      //SkBrowseMenu_edit_p4_properties,

    DataPop_goto,

    DataPop_arrange_panes
    };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  APopMenuOS pop_menu;

  pop_menu.append_item("&New subclass / member...\t[Ctrl+N]",       DataPop_new);
  //pop_menu.append_item(a_cstr_format("Rename '%s'...", "m_member"),    DataPop_rename, false);
  //pop_menu.append_item(a_cstr_format("Remove '%s'...", "m_member"),    DataPop_remove, false);
  //pop_menu.append_item(a_cstr_format("Duplicate '%s'...", "m_member"), DataPop_duplicate, false);

  //--------------------
  pop_menu.append_separator();
  pop_menu.append_item("Goto Data/Context...\t[Alt+Shift+G or Ctrl+G]", DataPop_goto);

  //--------------------
  pop_menu.append_separator();
  pop_menu.append_item("Arrange panes...",             DataPop_arrange_panes);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool call_parent = false;
  uint32_t pop_id      = 0u;

  if (pop_menu.show(screen_pos, *this, &pop_id))
    {
    switch (eDataPop(pop_id))
      {
      case DataPop_new:
        SkClassBrowser::ms_browser_p->get_navigation_view().show_create_popup();
        break;

      case DataPop_goto:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_data);
        break;

      case DataPop_arrange_panes:
        call_parent = true;
        break;
      }
    }

  // Call parent's on_context_menu()?
  return call_parent;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkDataList::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  switch (key)
    {
    case 'G':
      if (AKeyboard::get_mod_keys() == AKeyMod_ctrl)
        {
        // Ignore repeated keys
        if (!repeated)
          {
          SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_data);
          }

        return false;
        }
    }

  return m_member_view_p->on_key_press(key, repeated);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::on_item_focused(SkMemberReference * item_p, uint row)
  {
  m_member_view_p->on_member_selected(item_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::on_item_removed_row(uint row, uintptr_t item_info)
  {
  SkMemberReference * item_p = (SkMemberReference *)item_info;

  m_member_view_p->pool_delete_info(item_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkDataList::on_compare_instance(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  if (lhs.m_class_scope == rhs.m_class_scope)
    {
    return on_compare_name(lhs, rhs);
    }

  if (lhs.m_class_scope)
    {
    return m_col_instance_p->m_sort_ascending ? AEquate_greater : AEquate_less;
    }

  return m_col_instance_p->m_sort_ascending ? AEquate_less : AEquate_greater;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkDataList::on_compare_name(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  eAEquate result = lhs.m_member_id.get_name_str().compare(rhs.m_member_id.get_name_str());

  if (result == AEquate_equal)
    {
    result = lhs.get_class()->compare(*rhs.get_class());
    return m_col_scope_p->m_sort_ascending ? result : eAEquate(-result);
    }

  return m_col_name_p->m_sort_ascending ? result : eAEquate(-result);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkDataList::on_compare_scope(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  eAEquate result = lhs.get_class()->compare(*rhs.get_class());

  return (result != AEquate_equal)
    ? result
    : on_compare_name(lhs, rhs);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkDataList::on_compare_type(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  eAEquate result = lhs.m_member_id.get_data_type()->compare(*rhs.m_member_id.get_data_type());

  return (result != AEquate_equal)
    ? result
    : on_compare_name(lhs, rhs);
  }

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the list render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT SkDataList::on_custom_draw(NMLVCUSTOMDRAW * info_p)
  {
  switch (info_p->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
      // Ask for subitem (row) notifications
      return CDRF_NOTIFYSUBITEMDRAW;

    case CDDS_ITEMPREPAINT:
      info_p->clrText = AColor::ms_white;  //AColor::get_element_os(COLOR_WINDOWTEXT);
      return CDRF_NEWFONT;
    }

  return CDRF_DODEFAULT;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::on_text_name(tSkSubText * info_p)
  {
  info_p->m_subitem_str_p->append(info_p->m_item_p->m_member_id.get_name_str());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::on_text_scope(tSkSubText * info_p)
  {
  info_p->m_subitem_str_p->append(info_p->m_item_p->get_class()->get_name_str());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDataList::on_text_type(tSkSubText * info_p)
  {
  info_p->m_subitem_str_p->append(info_p->m_item_p->m_member_id.get_data_type()->as_code());
  }

//---------------------------------------------------------------------------------------
//  # Modifiers: virtual - Override for custom behaviour.
//  # Author(s): John Stenersen
bool SkDataList::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkClassBrowser::ms_browser_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()));
  SkMainWindowBase::on_focus(this, FocusType_data_list);

  return true;
  }


//=======================================================================================
// SkCodeList Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkCodeList::SkCodeList(
  AWindow *      parent_p,
  SkMemberView * member_view_p
  ) :
  AListOS<SkMemberReference>(parent_p, SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_member_view_p(member_view_p)
  {
  ListView_SetTextBkColor(m_os_handle, g_color_text_bg);  // BG with text
  ListView_SetBkColor(m_os_handle, g_color_text_bg);      // BG without text
  enable_remove_events();
  enable_image_sharing();
  set_image_list(SkConsole::ms_console_p->get_member_images());
  enable_gridlines();  // $Revisit - CReis Draw custom grid lines so that a specific colour can be used.
  ensure_size(SkContextInfo_code_size_init);
  set_border(Border_sunken);

  // $Revisit - CReis [Incomplete] Indicate members that have debug hooks (breakpoints, etc.) set.

  // $Note - CReis Whenever columns or added or removed, ensure that eColumn is in synch.
  //column_append(new AColumnOS<SkMemberReference>("Dbg"));

  m_col_instance_p = new AColumnOS<SkMemberReference>("i/c", nullptr, new tSkCodeCompareCall(this, &SkCodeList::on_compare_instance), false);
  column_append(m_col_instance_p);

  m_col_scope_p = new AColumnOS<SkMemberReference>(
    "Scope",
    new tSkCodeTextCall(this, &SkCodeList::on_text_scope),
    new tSkCodeCompareCall(this, &SkCodeList::on_compare_scope),
    false);
  column_append(m_col_scope_p);

  m_col_name_p = new AColumnOS<SkMemberReference>(
    "Name",
    new tSkCodeTextCall(this, &SkCodeList::on_text_name),
    new tSkCodeCompareCall(this, &SkCodeList::on_compare_name),
    false);
  column_append(m_col_name_p);

  column_append(new AColumnOS<SkMemberReference>(
    "Parameters",
    new tSkCodeTextCall(this, &SkCodeList::on_text_parameters)));
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkCodeList::~SkCodeList()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::append_member(
  const SkContextInfo &      info,
  SkMemberReference::eStatus status // = SkMemberReference::Status_valid
  )
  {
  uint row = append(*m_member_view_p->pool_new_info(info, status));

  set_image_index(
    info.m_class_scope
      ? SkMemberImage_scope_class
      : SkMemberImage_scope_instance,
    row,
    m_col_instance_p->get_rank());

  set_image_index(
    (status != SkMemberReference::Status_error)
      ? g_member_type_to_image[info.m_type]
      : g_member_type_to_error_image[info.m_type],
    row,
    m_col_name_p->get_rank());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::append_invokables(
  SkInvokableBase ** members_pp,
  uint                length,
  eSkInvokable       custom_type,
  bool               show_custom,
  bool               show_atomic,
  bool               class_scope
  )
  {
  SkMemberReference::eStatus status;
  bool               placeholder_b;
  eSkInvokable       type;
  SkInvokableBase ** members_end_pp = members_pp + length;
  SkContextInfo      member_info;

  // $Revisit - CReis Ignore status of uninitialized members when in Remote IDE mode
  bool remote_ide = SkRemoteBase::ms_default_p->is_remote_ide();

  member_info.m_class_scope = class_scope;

  for (; members_pp < members_end_pp; members_pp++)
    {
    type          = (*members_pp)->get_invoke_type();
    placeholder_b = (*members_pp)->is_placeholder();

    // Only show custom or atomic members as desired, but always show placeholder members
    // since they could be either atomic or custom
    if (placeholder_b || (show_custom && (type == custom_type)) || (show_atomic && (type != custom_type)))
      {
      member_info.m_type      = eSkMember(type);
      member_info.m_member_id = **members_pp;

      status = placeholder_b
        ? SkMemberReference::Status_error
        : (remote_ide || ((*members_pp)->is_bound())
          ? SkMemberReference::Status_valid
          : SkMemberReference::Status_uninitialized);

      append_member(member_info, status);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::set_class(SkClass * class_p)
  {
  remove_all();

  if (class_p)
    {
    // populate list
    bool show_instance   = m_member_view_p->m_toggle_instance.is_toggled();
    bool show_class      = m_member_view_p->m_toggle_class.is_toggled();
    bool show_inherited  = m_member_view_p->m_toggle_inherited.is_toggled();
    bool show_custom     = m_member_view_p->m_toggle_script.is_toggled();
    bool show_atomic     = m_member_view_p->m_toggle_cpp.is_toggled();
    bool show_methods    = m_member_view_p->m_toggle_methods.is_toggled();
    bool show_coroutines = m_member_view_p->m_toggle_coroutines.is_toggled();

    SkClass * add_class_p = class_p;

    while (add_class_p)
      {
      if (show_methods)
        {
        // Add Methods
        if (show_instance)
          {
          append_invokables(
            (SkInvokableBase **)add_class_p->get_instance_methods().get_array(),
            add_class_p->get_instance_methods().get_length(),
            SkInvokable_method, show_custom, show_atomic, false);
          }

        if (show_class)
          {
          append_invokables(
            (SkInvokableBase **)add_class_p->get_class_methods().get_array(),
            add_class_p->get_class_methods().get_length(),
            SkInvokable_method, show_custom, show_atomic, true);
          }
        }

      // Add coroutines - Note that there are no 'class' coroutines
      if (show_coroutines && show_instance)
        {
        append_invokables(
          (SkInvokableBase **)add_class_p->get_coroutines().get_array(),
          add_class_p->get_coroutines().get_length(),
          SkInvokable_coroutine, show_custom, show_atomic, false);
        }

      // Add inherited members
      add_class_p = show_inherited ? add_class_p->get_superclass() : nullptr;
      }

    // Default sort by type and name
    m_col_name_p->sort();
    }
  }

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the list render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT SkCodeList::on_custom_draw(NMLVCUSTOMDRAW * info_p)
  {
  switch (info_p->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
      // Ask for subitem (row) notifications
      return CDRF_NOTIFYSUBITEMDRAW;

    case CDDS_ITEMPREPAINT:
      // Ask for subitem (cell) notifications
      return CDRF_NOTIFYSUBITEMDRAW;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
      // uintptr_t item_info = info_p->nmcd.lItemlParam;
      // uint       row       = info_p->nmcd.dwItemSpec;
      // uint       rank      = info_p->iSubItem;
      // 
      SkMemberReference * item_p = reinterpret_cast<SkMemberReference *>(info_p->nmcd.lItemlParam);

      if (item_p->m_status != SkMemberReference::Status_valid)
        {
        uint rank = info_p->iSubItem;

        info_p->clrText = ((rank == Column_name) || (rank == Column_params))
          ? ((item_p->m_status == SkMemberReference::Status_error)
            ? g_color_error
            : g_colour_unbound)  // SkMemberReference::Status_uninitialized
          : AColor::ms_white;
        }
      else
        {
        info_p->clrText = AColor::ms_white;  //AColor::get_element_os(COLOR_WINDOWTEXT);
        }

      return CDRF_NEWFONT;
      }
    }

  return CDRF_DODEFAULT;
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
void SkCodeList::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_member_view_p->on_mouse_release(button, buttons, client_pos);
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
bool SkCodeList::on_context_menu(const AVec2i & screen_pos)
  {
  enum eCodePop
    {
    //SkBrowseMenu_edit_copy,
    //SkBrowseMenu_edit_copy_path,

    CodePop_rename,
    CodePop_remove,
    CodePop_duplicate,

    CodePop_new,

    // Version Control
      //SkBrowseMenu_edit_p4_checkout,
      //SkBrowseMenu_edit_p4_revert,
      //SkBrowseMenu_edit_p4_diff,
      //SkBrowseMenu_edit_p4_history,
      //SkBrowseMenu_edit_p4_timelapse,
      //SkBrowseMenu_edit_p4_properties,

    CodePop_goto,

    //SkBrowseMenu_file_open_associated,
    //SkBrowseMenu_file_open_explorer,

    CodePop_arrange_panes
    };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  APopMenuOS pop_menu;

  // If member selected
  //pop_menu.append_item(a_cstr_format("Rename '%s'...", "member()"),    CodePop_rename, false);
  //pop_menu.append_item(a_cstr_format("Remove '%s'...", "member()"),    CodePop_remove, false);
  // $Revisit - CReis Move / override / copy member to current class/overlay - prompt if class/overlay is different
  // $Revisit - CReis Toggle method between instance/class

  //--------------------
  //pop_menu.append_separator();
  pop_menu.append_item("&New subclass / member...\t[Ctrl+N]",       CodePop_new);

  //--------------------
  pop_menu.append_separator();
  pop_menu.append_item("Goto Code/Context...\t[Alt+Ctrl+G or Ctrl+G]", CodePop_goto);

  //--------------------
  pop_menu.append_separator();
  pop_menu.append_item("Arrange panes...",             CodePop_arrange_panes);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool call_parent = false;
  uint32_t pop_id      = 0u;

  if (pop_menu.show(screen_pos, *this, &pop_id))
    {
    switch (eCodePop(pop_id))
      {
      case CodePop_new:
        SkClassBrowser::ms_browser_p->get_navigation_view().show_create_popup();
        break;

      case CodePop_goto:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_routines);
        break;

      case CodePop_arrange_panes:
        call_parent = true;
        break;
      }
    }

  // Call parent's on_context_menu()?
  return call_parent;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkCodeList::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  switch (key)
    {
    case 'G':
      if (AKeyboard::get_mod_keys() == AKeyMod_ctrl)
        {
        // Ignore repeated keys
        if (!repeated)
          {
          SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_routines);
          }

        return false;
        }
    }

  return m_member_view_p->on_key_press(key, repeated);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::on_item_focused(SkMemberReference * item_p, uint row)
  {
  SkClassBrowser::ms_browser_p->set_focus_splitter(&this->m_member_view_p->get_splitter());

  if (item_p && !SkClassBrowser::ms_browser_p->get_member_info().compare_files(*item_p))
    {
    m_member_view_p->on_member_selected(item_p);
    }
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkCodeList::on_subitem_clicked(
  uint     row,
  uint     rank,
  eAMouse button,
  bool    double_click
  )
  {
  SkMemberReference * item_p = row2item(row);

  if (item_p && !SkClassBrowser::ms_browser_p->get_member_info().compare_files(*item_p))
    {
    m_member_view_p->on_member_selected(item_p);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::on_item_removed_row(uint row, uintptr_t item_info)
  {
  m_member_view_p->pool_delete_info((SkMemberReference *)item_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkCodeList::on_compare_instance(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  if (lhs.m_class_scope == rhs.m_class_scope)
    {
    return on_compare_name(lhs, rhs);
    }

  if (lhs.m_class_scope)
    {
    return m_col_instance_p->m_sort_ascending ? AEquate_greater : AEquate_less;
    }

  return m_col_instance_p->m_sort_ascending ? AEquate_less : AEquate_greater;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::on_text_name(tSkSubText * info_p)
  {
  const ASymbol & member_name = info_p->m_item_p->m_member_id.get_name();
  const ASymbol & op_name     = SkParser::method_to_operator(member_name);

  if (op_name.is_null())
    {
    info_p->m_subitem_str_p->append(member_name.as_str_dbg());
    }
  else
    {
    // Show operator symbol[s] if it is also an operator alias

    char    cstr_p[ASymbol_length_max + 1u];
    AString str(cstr_p, ASymbol_length_max + 1u, 0u);

    str.format("%s  [%s]", member_name.as_cstr_dbg(), op_name.as_cstr_dbg());
    info_p->m_subitem_str_p->append(str);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkCodeList::on_compare_name(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  eSkMember ltype = unify_type(lhs.m_type);
  eSkMember rtype = unify_type(rhs.m_type);

  if (ltype == rtype)
    {
    eAEquate result = lhs.m_member_id.get_name_str().compare(rhs.m_member_id.get_name_str());

    if (result == AEquate_equal)
      {
      result = lhs.get_class()->compare(*rhs.get_class());
      return m_col_scope_p->m_sort_ascending ? result : eAEquate(-result);
      }

    return m_col_name_p->m_sort_ascending ? result : eAEquate(-result);
    }

  // Put coroutines first & methods last
  if (ltype > rtype)
    {
    return AEquate_less;
    }

  return AEquate_greater;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::on_text_scope(tSkSubText * info_p)
  {
  info_p->m_subitem_str_p->append(info_p->m_item_p->get_class()->get_name_str());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkCodeList::on_compare_scope(const SkMemberReference & lhs, const SkMemberReference & rhs)
  {
  eAEquate result = lhs.get_class()->compare(*rhs.get_class());

  if (result == AEquate_equal)
    {
    return on_compare_name(lhs, rhs);
    }

  return m_col_scope_p->m_sort_ascending ? result : eAEquate(-result);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCodeList::on_text_parameters(tSkSubText * info_p)
  {
  AString params(info_p->m_item_p->as_invokable()->as_code_params());
  uint     size = info_p->m_subitem_str_p->get_size();

  info_p->m_save_text = false;

  if (params.get_length() >= (size - 1u))
    {
    info_p->m_subitem_str_p->append(params.as_cstr(), size - 6u);
    info_p->m_subitem_str_p->append(" ...", 4u);
    }
  else
    {
    info_p->m_subitem_str_p->append(params);
    }
  }


//---------------------------------------------------------------------------------------
//  # Modifiers: virtual - Override for custom behaviour.
//  # Author(s): John Stenersen
bool SkCodeList::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkClassBrowser::ms_browser_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()));
  SkMainWindowBase::on_focus(this, FocusType_code_list);

  return true;
  }


//=======================================================================================
// SkMemberView Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkMemberView::SkMemberView(
  AWindow *        parent_p,
  SkClassBrowser * browser_p
  ) :
  AWindow(AWindow::ms_region_def, parent_p),
  m_browser_p(browser_p),
  m_class_p(nullptr),
  m_on_toggle_member_kind(this, &SkMemberView::on_toggle_member_kind),
  m_toggle_instance(  this, "Instance",   ACheckType_2_state, m_font),
  m_toggle_class(     this, "Class",      ACheckType_2_state, m_font),
  m_toggle_inherited( this, "Inherited",  ACheckType_2_state, m_font),
  m_toggle_script(    this, "Script",     ACheckType_2_state, m_font),
  m_toggle_cpp(       this, "C++",        ACheckType_2_state, m_font),
  m_toggle_methods(   this, "Methods",    ACheckType_2_state, m_font),
  m_toggle_coroutines(this, "Coroutines", ACheckType_2_state, m_font),
  m_splitter(this, ARegion(m_toggle_instance.get_width() * 2, 0)),
  m_data_list(&m_splitter, this),
  m_code_list(&m_splitter, this)
  {
  //set_color_background(g_color_bg);

  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
  ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
  ::SetBkMode(hdc, OPAQUE);  // TRANSPARENT

  ::ReleaseDC(m_os_handle, hdc);


  // Load ini values for check boxes
  load_settings();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Filter setup
  int spacing  = get_spacing();
  int spacing2 = spacing * 2;
  int x;

  m_filter_rect.top   = spacing;
  m_filter_rect.left  = spacing;
  m_filter_rect.right = m_splitter.get_x_rel() - spacing;

  x = m_filter_rect.left + spacing;

  m_toggle_instance.set_position( x, m_filter_rect.top + spacing2);
  m_toggle_instance.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_instance.show();

  m_toggle_class.set_position(    x, m_toggle_instance.get_bottom_rel());
  m_toggle_class.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_class.show();

  m_toggle_inherited.set_position(x, m_toggle_class.get_bottom_rel());
  m_toggle_inherited.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_inherited.show();

  m_toggle_script.set_position(   x, m_toggle_inherited.get_bottom_rel() + spacing);
  m_toggle_script.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_script.show();
  
  m_toggle_cpp.set_position(      x, m_toggle_script.get_bottom_rel());
  m_toggle_cpp.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_cpp.show();
  
  m_toggle_methods.set_position(  x, m_toggle_cpp.get_bottom_rel() + spacing);
  m_toggle_methods.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_methods.show();
  
  m_toggle_coroutines.set_position(x, m_toggle_methods.get_bottom_rel());
  m_toggle_coroutines.set_on_toggled_func(&m_on_toggle_member_kind, false);
  m_toggle_coroutines.show();
  
  m_filter_rect.bottom = m_toggle_coroutines.get_bottom_rel() + spacing;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Code/Data Splitter setup
  m_data_list.show();
  m_code_list.show();

  m_splitter.enable_auto_update(false);
  //m_splitter.set_color_background(g_color_bg);
  m_splitter.set_orientation(ASplitterOS::eOrient(SkConsole::ms_console_p->get_ini_ide().get_value_int_default(
    ASplitterOS::Orient_horiz_ab,
    g_ini_key_split_data_code_orient_p,
    g_ini_section_browser_p)));
  m_splitter.set_ratio(SkConsole::ms_console_p->get_ini_ide().get_value_default(
    "0.25",
    g_ini_key_split_data_code_ratio_p,
    g_ini_section_browser_p).as_float32());
  m_splitter.set_pane_a(&m_data_list);
  m_splitter.set_pane_b(&m_code_list);
  m_splitter.enable_auto_update();
  m_splitter.show();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkMemberView::~SkMemberView()
  {
  m_members.remove_all();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMemberView::ensure_members_listed(const SkContextInfo & member_info)
  {
  bool set_ssclass  = member_info.m_class_scope;
  bool set_instance = !member_info.m_class_scope;

  bool set_custom     = false;
  bool set_atomic     = false;
  bool set_methods    = false;
  bool set_coroutines = false;
 
  switch (member_info.m_type)
    {
    case SkMember_method:
      set_custom  = true;
      set_methods = true;
      break;

    case SkMember_method_func:
    case SkMember_method_mthd:
      set_atomic  = true;
      set_methods = true;
      break;

    case SkMember_coroutine:
      set_custom    = true;
      set_coroutines = true;
      break;

    case SkMember_coroutine_mthd:
    case SkMember_coroutine_func:
      set_atomic    = true;
      set_coroutines = true;
      break;
    }

  bool update_lists = false;

  if (set_instance)
    {
    update_lists = m_toggle_instance.enable_toggle() || update_lists;
    }

  if (set_ssclass)
    {
    update_lists = m_toggle_class.enable_toggle() || update_lists;
    }

  if (set_custom)
    {
    update_lists = m_toggle_script.enable_toggle() || update_lists;
    }

  if (set_atomic)
    {
    update_lists = m_toggle_cpp.enable_toggle() || update_lists;
    }

  if (set_methods)
    {
    update_lists = m_toggle_methods.enable_toggle() || update_lists;
    }

  if (set_coroutines)
    {
    update_lists = m_toggle_coroutines.enable_toggle() || update_lists;
    }

  if (update_lists)
    {
    m_class_p = nullptr;
    }

  set_class(member_info.get_class());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMemberView::set_class(SkClass * class_p)
  {
  if (m_class_p != class_p)
    {
    m_class_p = class_p;

    refresh_members();
    }
  }

//---------------------------------------------------------------------------------------
// Focus on specified member
// Author(s):   Conan Reis
void SkMemberView::select_member(const SkContextInfo & member_info)
  {
  SkMemberReference * info_p = m_members.get(member_info.m_member_id);

  // $Revisit - CReis Ignore if already selected

  if (info_p)
    {
    if (member_info.m_type == SkMember_data)
      {
      m_data_list.select(*info_p);
      m_data_list.ensure_visible(*info_p);
      }
    else
      {
      m_code_list.select(*info_p);
      m_code_list.ensure_visible(*info_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Focus on specified member - refreshing list as needed
// Author(s):   Conan Reis
void SkMemberView::set_member(const SkContextInfo & member_info)
  {
  ensure_members_listed(member_info);

  select_member(member_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMemberView::refresh_members()
  {
  // $Revisit - CReis Should remember currently focused items and refocus them after refresh.
  m_data_list.set_class(m_class_p);
  m_code_list.set_class(m_class_p);
  m_code_list.columns_set_width();
  m_data_list.columns_set_width();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMemberView::load_settings()
  {
  AIni & ide_ini = SkConsole::ms_console_p->get_ini_ide();
  
  m_toggle_instance.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_instance_def,
      g_ini_key_instance_p,
      g_ini_section_browser_p));

  m_toggle_class.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_class_def,
      g_ini_key_class_p,
      g_ini_section_browser_p));

  m_toggle_inherited.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_inherited_def,
      g_ini_key_inherited_p,
      g_ini_section_browser_p));

  m_toggle_script.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_custom_def,
      g_ini_key_custom_p,
      g_ini_section_browser_p));

  m_toggle_cpp.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_atomic_def,
      g_ini_key_atomic_p,
      g_ini_section_browser_p));

  m_toggle_methods.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_methods_def,
      g_ini_key_methods_p,
      g_ini_section_browser_p));

  m_toggle_coroutines.enable_toggle(
    ide_ini.get_value_bool_default(
      SkMemberView_show_coroutines_def,
      g_ini_key_coroutines_p,
      g_ini_section_browser_p));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMemberView::save_settings()
  {
  AIni & ide_ini = SkConsole::ms_console_p->get_ini_ide();
  
  ide_ini.set_value_bool(
    m_toggle_instance.is_toggled(),
    g_ini_key_instance_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_class.is_toggled(),
    g_ini_key_class_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_inherited.is_toggled(),
    g_ini_key_inherited_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_script.is_toggled(),
    g_ini_key_custom_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_cpp.is_toggled(),
    g_ini_key_atomic_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_methods.is_toggled(),
    g_ini_key_methods_p,
    g_ini_section_browser_p);

  ide_ini.set_value_bool(
    m_toggle_coroutines.is_toggled(),
    g_ini_key_coroutines_p,
    g_ini_section_browser_p);

  ide_ini.set_value(
    AString::ctor_int(m_splitter.get_orientation()),
    g_ini_key_split_data_code_orient_p,
    g_ini_section_browser_p);

  ide_ini.set_value(
    AString::ctor_float(m_splitter.get_ratio()),
    g_ini_key_split_data_code_ratio_p,
    g_ini_section_browser_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkMemberReference * SkMemberView::pool_new_info(
  const SkContextInfo &      member_info,
  SkMemberReference::eStatus status // = SkMemberReference::Status_valid
  )
  {
  SkMemberReference * info_p = SkMemberReference::pool_new(member_info, status);
  
  m_members.append(*info_p);
  
  return info_p;
  }

//---------------------------------------------------------------------------------------
// Called when one of the member lists focuses on an item
// Author(s):   Conan Reis
void SkMemberView::on_member_selected(SkMemberReference * member_p)
  {
  m_browser_p->on_member_selected(member_p);
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
void SkMemberView::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_browser_p->on_mouse_release(button, buttons, client_pos);
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
// Modifiers:   virtual
// Examples:   called by the system
// Author(s):   Conan Reis
bool SkMemberView::on_context_menu(const AVec2i & screen_pos)
  {
  // For some reason the child lists send a WM_CONTEXT message as soon as the right mouse
  // button is pressed rather than just when the right mouse button is released.
  if ((screen_pos.m_x != -1) && (screen_pos.m_y != -1) && !AKeyboard::is_pressed(AKey_btn_right))
    {
    if (m_data_list.is_in(screen_pos))
      {
      //ADebug::print("In data list\n");
      return false;
      }

    if (m_code_list.is_in(screen_pos))
      {
      //ADebug::print("In code list\n");
      return false;
      }
    }

  //ADebug::print("In member view\n");
  // Call parent's on_context_menu()
  return true;
  }

//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkMemberView::on_sizing()
  {
  AVec2i carea(get_area_client());

  m_splitter.set_area(AVec2i(carea.m_x - m_splitter.get_x_rel() - SkMemberView_gui_inset, carea.m_y));

  refresh();
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// 
// #Modifiers   virtual - Overridden from AgogGUI\AWindow.
// #Author(s)   Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Boolean indicating whether the default Windows procedure with respect to the given
  // message should be called (true) or not (false)
  bool
SkMemberView::on_draw()
  {
  PAINTSTRUCT ps;
  HDC         hdc     = ::BeginPaint(m_os_handle, &ps);
  int         spacing = get_spacing();
  int         y;

  // Set header1 font
  ::SelectObject(hdc, ((ATrueTypeFont *)AFont::ms_header1_p->m_sys_font_p)->m_font_handle_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Filter Settings
  y = m_filter_rect.top - spacing;
  ::DrawEdge(hdc, &m_filter_rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, m_filter_rect.left + spacing, y, 0u, nullptr, "Filters", 7, nullptr);

  // Put default font back
  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a member toggle button is changed
// Author(s):   Conan Reis
void SkMemberView::on_toggle_member_kind(eAFlag state)
  {
  refresh_members();
  }


//=======================================================================================
// SkEditor Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkEditor::SkEditor(
  SkEditView *    edit_view_p,
  const AString & initial // = AString::ms_empty
  ) :
  SkEditSyntax(Type_editor, edit_view_p, SkIncrementalSearchEditBox::ParentContext_editor, initial, SkConsole::ms_console_p->get_ini_font()),
  m_source_available(false),
  m_edit_view_p(edit_view_p),
  m_auto_complete(this)
  {
  enable_read_only();
  set_border(Border_line);
  }

//---------------------------------------------------------------------------------------
// If the source file is available and read-only, the user is asked if they
//             want to remove the read-only file attribute.
// Returns:    true if source file is available and can be written to, false if not
// Author(s):   Conan Reis
bool SkEditor::ensure_writable(
  bool query_user // = true
  )
  {
  if (is_read_only())
    {
    if (m_source_available)
      {
      if (m_source_file.ensure_writable_query())
        {
        enable_read_only(false);
        SkDebug::print(
          a_str_format(
            "\n'%s' %s\n",
            m_source_file.get_file_str().as_cstr(),
            (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
              ? "checked out for edit by Perforce."
              : "made writable."),
          SkLocale_local);

        return true;
        }
      }

    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditor::set_source(
  const AString & str,
  bool            read_only_b // = true
  )
  {
  SkEditSyntax::set_source(str, read_only_b);

  m_edit_view_p->set_modified(true);
  on_modified();
  m_edit_view_p->set_modified(false);

  SkClassBrowser::ms_browser_p->status_update();
  }

//---------------------------------------------------------------------------------------
// Arg         index_start - edit selection start
// Arg         index_end - edit selection end
// Author(s):   Conan Reis
void SkEditor::set_source_file(
  AFile * source_p // = nullptr
  )
  {
  save_settings();

  m_source_available = source_p != nullptr;

  if (m_source_available)
    {
    m_source_file = *source_p;
    AString source(m_source_file.read_text());

    source.line_break_dos2unix();
    
    set_source(source, source_p->is_read_only());
    }
  else
    {
    m_source_file.empty_file_str();
    enable_read_only();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditor::save_settings(
  eSkLog log // = SkLog_ide_print
  )
  {
  if (m_source_available && m_edit_view_p->is_modified())
    {
    if (log == SkLog_ide_print)
      {
      SkDebug::print(a_str_format("\nFile: %s\n  Saving changes... ", m_source_file.as_cstr()), SkLocale_local);
      }

    set_break_conversion(ALineBreak_dos);
    AString code(get_text());
    set_break_conversion(ALineBreak_unix);
    
    m_source_file.write_text(code);
    m_edit_view_p->set_modified(false);
    SkClassBrowser::ms_browser_p->status_update();

    if (log == SkLog_ide_print)
      {
      SkDebug::print(" saved.\n", SkLocale_local);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Draw expression span range/selection
void SkEditor::draw_expr_idx_span(
  uint32_t            idx,
  const AColor &      color,
  Gdiplus::Graphics & graphics
  )
  {
  uint32_t idx_begin;
  uint32_t idx_end;

  idx_to_expr_span(idx, &idx_begin, &idx_end);
  draw_span(idx_begin, idx_end, color, graphics);
  }

//---------------------------------------------------------------------------------------
// Draw expression span range/selection
void SkEditor::draw_expr_mark(
  SkExpressionBase &  expr,
  const AColor &      color,
  Gdiplus::Graphics & graphics
  )
  {
  draw_mark(file_to_index(expr.m_source_idx), color, graphics);
  }

//---------------------------------------------------------------------------------------
// Draw expression span range/selection
void SkEditor::draw_expr_span(
  SkInvokableBase &   invokable,
  SkExpressionBase &  expr,
  const AColor &      color,
  Gdiplus::Graphics & graphics
  )
  {
  uint32_t idx_begin;
  uint32_t idx_end;

  invokable.get_expr_span(expr, &idx_begin, &idx_end);
  idx_begin = file_to_index(idx_begin);
  idx_end   = file_to_index(idx_end);

  uint32_t idx_pivot = file_to_index(expr.m_source_idx);

  draw_span_pivot(idx_begin, idx_end, idx_pivot, color, graphics);
  }

//---------------------------------------------------------------------------------------
// Draw aspects specific to this type of window.
void SkEditor::on_draw_subpart(Gdiplus::Graphics & graphics)
  {
  // As much work as possible here should be cached rather than done each redraw.
  bool draw_breaks     = false;
  bool draw_dbg_pos    = false;
  bool draw_expr_guide = false;

  SkMemberExpression * dbg_expr_p = nullptr;

  if (m_source_available)
    {
    draw_breaks     = m_edit_view_p->m_bps_on_member.get_length();
    dbg_expr_p      = &SkDebug::get_next_expression();
    draw_dbg_pos    = dbg_expr_p->is_valid() && dbg_expr_p->get_expr() && SkDebug::is_active_member(m_edit_view_p->get_member_info());
    draw_expr_guide = SkConsole::ms_console_p->is_expression_guide();
    }

  SkInvokableBase * invokable_p = nullptr;

  if (draw_breaks || draw_dbg_pos)
    {
    invokable_p = m_edit_view_p->get_member_info().as_invokable();
    }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Draw breakpoints
  if (draw_breaks)
    {
    AColor          colour_enabled(1.0f, 0.0f, 0.0f, 0.8f);
    AColor          colour_disabled(0.8f, 0.4f, 0.4f, 0.6f);
    SkBreakPoint ** bps_pp     = m_edit_view_p->m_bps_on_member.get_array();
    SkBreakPoint ** bps_end_pp = bps_pp + m_edit_view_p->m_bps_on_member.get_length();

    for (; bps_pp < bps_end_pp; bps_pp++)
      {
      draw_expr_mark(*(*bps_pp)->get_expr(), (*bps_pp)->m_enabled ? colour_enabled : colour_disabled, graphics);
      //draw_expr_span(*invokable_p, *(*bps_pp)->get_expr(), (*bps_pp)->m_enabled ? colour_enabled : colour_disabled, graphics);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Draw current debug position
  if (draw_dbg_pos)
    {
    draw_expr_mark(*dbg_expr_p->get_expr(), AColor::ms_orange, graphics);
    //draw_expr_span(*invokable_p, *dbg_expr_p->get_expr(), AColor::ms_orange, graphics);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Draw current expression
  if (draw_expr_guide)
    {
    // $TODO - CReis Test Parse to index probe
    //AColor  colour_parse_span(0.0f, 0.7f, 0.0f, 0.4f);
    //draw_expr_idx_span(get_caret_index(), colour_parse_span, graphics);

    SkExpressionBase * caret_expr_p = get_caret_expr();

    if (caret_expr_p)
      {
      draw_expr_mark(*caret_expr_p, g_colour_expr_guide, graphics);
      //draw_expr_span(*invokable_p, *caret_expr_p, g_colour_expr_guide, graphics);
      }
    }
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
void SkEditor::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_auto_complete.on_parent_mouse_release(button, buttons, client_pos);
  m_edit_view_p->on_mouse_release(button, buttons, client_pos);
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkEditor::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  bool     process_key_b = true;
  eAKeyMod mod_keys      = AKeyboard::get_mod_keys();

  if (is_read_only() && m_source_available && AKeyboard::is_edit_key(key, mod_keys))
    {
    process_key_b = ensure_writable();

    if (process_key_b)
      {
      SkClassBrowser::ms_browser_p->status_update();
      }

    set_focus();
    }

  //if (process_key_b)
  //  {
  //  switch (key)
  //    {
  //    case AKey_f9:  // Toggle breakpoint
  //      if (mod_keys == AKeyMod_none)
  //        {
  //        // Ignore repeated keys
  //        if (!repeated)
  //          {
  //          m_edit_view_p->toggle_breakpoint();
  //          }
  //        return false;
  //        }
  //      break;
  //    }
  //  }

  if (!m_auto_complete.on_parent_key_press(key, repeated, AKeyboard::get_mod_keys()))
    {
    return false;
    }

  return SkEditSyntax::on_key_press(key, repeated);
  }

//---------------------------------------------------------------------------------------
// The user has taken an action that may have altered text in an edit control.
//             Unlike the on_predraw() event, this event is sent after the system updates
//             the screen.
// See:        enable_on_modified, on_predraw()
// Notes:      This event is not called when it is a multi-line control and the text is
//             set via WM_SETTEXT
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkEditor::on_modified()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  //***************************
  //  Auto-parse the text.
  SkContextInfo * info_p = &m_edit_view_p->m_member_info;

  if (SkConsole::ms_console_p->is_auto_parse()
    && info_p->is_valid()
    && (get_selection_length() <= 0)
    && (SkCompiler::ms_compiler_p->get_phase() == SkCompiler::Phase_idle))
    {
    SkParser::Args args;
    SkParser       parser(get_text());
    ASymbolTable   sym_tab;

    uint32_t sym_tab_len_before = ASymbolTable::ms_main_p->get_length();

    sym_tab.track_auto_parse_init();

    //args.m_flags = SkParser::ArgFlag__default_no_struct;
    switch (info_p->m_type)
      {
      case SkMember_method :
      case SkMember_method_func :
      case SkMember_method_mthd :
        {
        SkMethodBase * method = parser.parse_method_source(ASymbol_auto_parse_, info_p->get_class_scope(), args, false);

        if (method)
          {
          delete method;
          }
        }
        break;

      case SkMember_coroutine :
      case SkMember_coroutine_func :
      case SkMember_coroutine_mthd :
        {
        SkCoroutineBase * coroutine_p = parser.parse_coroutine_source(ASymbol_auto_parse_, info_p->get_class_scope(), args, false);

        if (coroutine_p)
          {
          delete coroutine_p;
          }
        }
        break;

      case SkMember_data :
        parser.parse_data_members_source(info_p->get_class_scope(), args, false);
        break;

      case SkMember_class_meta :
        parser.parse_class_meta_source(info_p->get_class(), args, false);
        break;

      default:
        //A_DPRINT(A_SOURCE_STR "Unknown m_type = %ld\n", info_p->m_type);
        args.m_result = SkParser::Result_ok;
        break;
      }

    if (args.m_result != SkParser::Result_ok)
      {
      m_auto_parse_ok     = false;
      m_auto_parse_start  = args.m_start_pos;
      m_auto_parse_end    = args.m_end_pos;
      //A_DPRINT(A_SOURCE_STR "Auto-Parse: Error\n");
      }
    else
      {
      m_auto_parse_ok     = true;
      m_auto_parse_start  = 0;
      m_auto_parse_end    = 0;
      //A_DPRINT(A_SOURCE_STR "Auto-Parse: Ok\n");
      }

    sym_tab.track_auto_parse_term();

    uint32_t sym_tab_len_after = ASymbolTable::ms_main_p->get_length();
    uint32_t sym_tab_len_delta = sym_tab_len_after - sym_tab_len_before;

    if (sym_tab_len_delta != 0)
      {
      A_DPRINT(A_SOURCE_STR "Symbol table delta = %ld, before = %ld, after = %ld\n", sym_tab_len_delta, sym_tab_len_before, sym_tab_len_after);
      }
    }

  if (!m_edit_view_p->is_modified())
    {
    m_edit_view_p->set_modified(true);
    SkClassBrowser::ms_browser_p->status_update();
    }

    SkEditSyntax::on_modified();
   
    //  It's important to have the auto-parse complete before syntax_highlight() is called because syntax_highlight()
    //  makes a SendMessage() call which allows Windows to process a CALLBACK function which calls the on_draw() routine
    //  before the auto-parse completes. Its Windows "CALLBACK message pump pot pouri" otherwise.

    if (SkConsole::ms_console_p->is_syntax_highlight())
      {
      syntax_highlight(Coverage_visible);
      }
  } //  SkEditor::on_modified()


//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is attained.
// # See:      on_focus()
// # Modifiers: virtual
// # Author(s): John Stenersen
bool SkEditor::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkClassBrowser::ms_browser_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()->get_parent()));
  SkMainWindowBase::on_focus(this, FocusType_editor);

  return true;
  }


//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is lost. Settings are saved and the auto-complete
// is canceled.
// 
// See:        on_focus()
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkEditor::on_focus_lost(HWND focus_window)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if ((focus_window != m_auto_complete.get_os_handle()) && (focus_window != get_os_handle()))
    {
    m_auto_complete.cancel();
    }

  save_settings();
  }


//---------------------------------------------------------------------------------------
//  Passes any printable character to the auto-complete first and, if not "consumed" then
//  the character is processed by the default proc.
//  
//  Author(s):  John Stenersen
bool SkEditor::on_character(char ch, bool repeated)
  {
  if (!m_auto_complete.on_parent_character(ch, repeated, AKeyboard::get_mod_keys()))
    {
    return false;
    }

  return true;  //  Not processed, so pass to default proc.
  }


//---------------------------------------------------------------------------------------
// Get expression relative to caret position - preferring one on same row
// Returns:    Expression that caret is on/near
// Author(s):   Conan Reis
SkExpressionBase * SkEditor::get_caret_expr() const
  {
  uint32_t           caret_idx = caret_index_to_file();
  SkExpressionBase * caret_expr_p
    = m_edit_view_p->m_member_info.find_expr_on_pos(caret_idx, SkExprFind_interesting);

  if (caret_expr_p)
    {
    // Prefer expr starting on caret row
    uint32_t caret_row = get_row_caret();

    if (get_row_from_index(file_to_index(caret_expr_p->m_source_idx)) < caret_row)
      {
      SkExpressionBase * next_expr_p
        = m_edit_view_p->m_member_info.find_expr_by_pos(caret_idx, SkExprFind_interesting);

      if (next_expr_p
        && (get_row_from_index(file_to_index(next_expr_p->find_expr_by_pos_first()->m_source_idx)) == caret_row))
        {
        caret_expr_p = next_expr_p;
        }
      }
    }

  return caret_expr_p;
  }


//=======================================================================================
// SkHistoryPicker Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Arg         region - if height is not supplied, it will be auto determined
// Author(s):   Conan Reis
SkHistoryPicker::SkHistoryPicker(
  AWindow *     parent_p,
  const AFont & font
  ) :
  AComboBoxOS(parent_p, AComboOSFlags__edit_no_sort, font)
  {
  set_border(Border_thin_sunken);
  //set_color_background(g_color_bg);
  }

//---------------------------------------------------------------------------------------
// Called whenever a new selection is accepted.
// Notes:      Call row2info() or get_selected_info() to get the the row's item info.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void SkHistoryPicker::on_selected(uint row, uint row_prev)
  {
  SkEditInfo * info_p = row2obj<SkEditInfo>(row);

  if (info_p)
    {
    SkClassBrowser::ms_browser_p->get_edit_view().history_jump(info_p);
    }
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
void SkHistoryPicker::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  SkClassBrowser::ms_browser_p->get_edit_view().on_mouse_release(button, buttons, client_pos);
  }


//=======================================================================================
// SkEditView Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkEditView::SkEditView(
  AWindow *        parent_p,
  SkClassBrowser * browser_p
  ) :
  AWindow(AWindow::ms_region_def, parent_p),
  m_browser_p(browser_p),
  m_current_info_p(nullptr),
  m_history_count(0u),
  m_btn_sync(this, "<<"),
  m_history_combo(this, SkConsole::ms_console_p->get_ini_font()),
  m_btn_goto(this, "Go To..."),
  m_modified(false),
  m_edit(this, "Code Editor")
  {
  int spacing = get_spacing();
  int height  = a_max(m_history_combo.get_height(), m_btn_goto.get_height());  

  //set_color_background(g_color_bg);

  m_btn_sync.set_on_pressed_func(new AMethod<SkEditView>(this, &SkEditView::sync_browser));
  m_btn_sync.set_position(0u, spacing);
  m_btn_sync.set_area(m_btn_sync.get_width() + spacing, height);
  m_btn_sync.show();

  m_history_combo.set_position(m_btn_sync.get_right_rel_spaced(), spacing);
  m_history_combo.show();

  m_btn_goto.set_on_pressed_func(new AMethod<SkClassBrowser>(SkClassBrowser::ms_browser_p, &SkClassBrowser::on_goto));
  m_btn_goto.set_area(m_btn_goto.get_width() + (spacing * 2), height);
  m_btn_goto.show();

  // $Revisit - CReis Allow mechanism to view/edit overridden lower sequence overlays.
  
  m_edit.set_position(0, a_max(m_history_combo.get_bottom_rel(), m_btn_goto.get_bottom_rel()) + spacing);
  m_edit.show();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkEditView::~SkEditView()
  {
  history_clear();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditView::set_member(
  const SkEditInfo & info,
  bool               select_scroll // = true
  )
  {
  m_edit.save_settings();

  m_member_info = info;
  set_modified(false);

  A_ASSERTX(info.get_class(), "No class scope set!");

  refresh_member();

  if (select_scroll && m_edit.on_draw())
    {
    m_edit.select(info.m_sel_idx_begin, info.m_sel_idx_end);

    if (info.m_scroll_pos != AVec2i(-1, -1))
      {
      m_edit.set_scroll_pos(info.m_scroll_pos);
      }
    else
      {
      m_edit.ensure_visible_caret();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Arg         member_info - script/member to browse
// Arg         index_start - source file selection start
// Arg         index_end - source file selection end
// Author(s):   Conan Reis
void SkEditView::set_member(
  const SkContextInfo & member_info,
  uint                  index_start, // = 0u
  uint                  index_end    // = ADef_uint32
  )
  {
  bool converted = false;

  if (!m_member_info.compare_files(member_info))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Update member history

    // Free any "next" history
    if (m_current_info_p)
      {
      SkEditInfo * stale_p = m_history.get_last();

      while (stale_p != m_current_info_p)
        {
        m_history_combo.remove(*stale_p);
        delete stale_p;
        m_history_count--;
        stale_p = m_history.get_last();
        }

      // Save current position before switch
      m_edit.get_selection(
        &m_current_info_p->m_sel_idx_begin,
        &m_current_info_p->m_sel_idx_end);
      m_current_info_p->m_scroll_pos = m_edit.get_scroll_pos();
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Remove if already in history so now it will be at top
    SkEditInfo * info_p     = new SkEditInfo(member_info, index_start, index_end);
    SkEditInfo * old_info_p = m_history.find_key(*info_p);

    if (old_info_p)
      {
      m_history_combo.remove(*old_info_p);
      m_history.remove(old_info_p);
      m_history_count--;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Limit amount of history rather than be infinite
    while (m_history_count >= SkMemberView_history_max)
      {
      m_history_combo.remove(*m_history.get_first());
      m_history.free_first();
      m_history_count--;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Add new source info
    m_history.append(info_p);
    m_history_combo.insert(info_p->as_file_title(), *info_p);
    m_history_count++;
    m_current_info_p = info_p;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    set_member(*m_current_info_p, false);

    // Adjust index from DOS (/r/n) to Unix/RichEdit (/n)
    index_start = m_edit.file_to_index(index_start);
    index_end   = (index_end == ADef_uint32)
      ? index_start
      : m_edit.file_to_index(index_end);

    converted = true;

    m_current_info_p->m_sel_idx_begin = index_start;
    m_current_info_p->m_sel_idx_end   = index_end;
    }

  if (m_edit.is_source_available())
    {
    if (!converted)
      {
      // Adjust index from DOS (/r/n) to Unix/RichEdit (/n)
      index_start = m_edit.file_to_index(index_start);
      index_end   = (index_end == ADef_uint32)
        ? index_start
        : m_edit.file_to_index(index_end);
      }

    m_edit.select(index_start, index_end);
    m_edit.ensure_visible_caret();
    }
  }

//---------------------------------------------------------------------------------------
// Go to next member in history if there is one.
// See:        history_prev(), history_jump(), history_clear(), set_member()
// Author(s):   Conan Reis
void SkEditView::history_next()
  {
  if (m_current_info_p && m_history.get_next_null(m_current_info_p))
    {
    // Save current position before switch
    m_edit.get_selection(
      &m_current_info_p->m_sel_idx_begin,
      &m_current_info_p->m_sel_idx_end);
    m_current_info_p->m_scroll_pos = m_edit.get_scroll_pos();

    m_current_info_p = m_history.get_next_null(m_current_info_p);

    set_member(*m_current_info_p);
    }
  }

//---------------------------------------------------------------------------------------
// Go to previous member in history if there is one.
// See:        history_next(), history_jump(), history_clear(), set_member()
// Author(s):   Conan Reis
void SkEditView::history_prev()
  {
  if (m_current_info_p && m_history.get_prev_null(m_current_info_p))
    {
    // Save current position before switch
    m_edit.get_selection(
      &m_current_info_p->m_sel_idx_begin,
      &m_current_info_p->m_sel_idx_end);
    m_current_info_p->m_scroll_pos = m_edit.get_scroll_pos();

    m_current_info_p = m_history.get_prev_null(m_current_info_p);

    set_member(*m_current_info_p);
    }
  }

//---------------------------------------------------------------------------------------
// Jumps to specific point in existing history - called by history combo.
// See:        history_next(), history_prev(), history_clear(), set_member()
// Author(s):   Conan Reis
void SkEditView::history_jump(SkEditInfo * info_p)
  {
  if (m_current_info_p && (m_current_info_p != info_p))
    {
    // Save current position before switch
    m_edit.get_selection(
      &m_current_info_p->m_sel_idx_begin,
      &m_current_info_p->m_sel_idx_end);
    m_current_info_p->m_scroll_pos = m_edit.get_scroll_pos();

    m_current_info_p = info_p;

    set_member(*m_current_info_p);
    }
  }

//---------------------------------------------------------------------------------------
// Clear out the member history if any.
// See:        history_next(), history_prev(), history_jump(), set_member()
// Author(s):   Conan Reis
void SkEditView::history_clear()
  {
  if (m_history_count)
    {
    m_history_combo.remove_all();
    m_history.free_all();
    m_history_count  = 0u;
    m_current_info_p = nullptr;
    }
  }

//---------------------------------------------------------------------------------------
// Opens, selects and makes visible the class in class tree and member in
//             member view that is currently open in the editor.
// Author(s):   Conan Reis
void SkEditView::sync_browser()
  {
  if (m_member_info.is_valid())
    {
    m_browser_p->get_navigation_view().set_class(m_member_info.m_member_id.get_scope(), AFlag_off);
    m_browser_p->get_member_view().set_member(m_member_info);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditView::member_invalidate()
  {
  if (m_member_info.is_valid())
    {
    m_member_info.invalidate();
    set_modified(false);

    m_edit.save_settings();

    refresh_member();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AString SkEditView::get_member_name() const
  {
  return m_member_info.as_file_title();
  }

//---------------------------------------------------------------------------------------
// Toggles breakpoint at current position in editor.
// Author(s):   Conan Reis
void SkEditView::toggle_breakpoint()
  {
  if (m_member_info.is_valid())
    {
    bool           appended;
    SkBreakPoint * bp_p = SkDebug::breakpoint_append_absent(
      m_member_info,
      m_edit.get_caret_expr(),
      &appended);

    if (bp_p)
      {
      if (appended)
        {
        m_bps_on_member.append(*bp_p);

        SkRemoteIDE * remote_p = SkConsole::ms_console_p->get_remote_ide();

        if (remote_p->is_authenticated())
          {
          remote_p->cmd_breakpoint_create(*bp_p);
          }
        }
      else
        {
        SkConsole::ms_console_p->get_remote_ide()->cmd_breakpoint_update(bp_p, SkBreakPoint::Update_remove);

        m_bps_on_member.remove(*bp_p);
        bp_p->remove();
        }

      m_edit.invalidate();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Toggles breakpoint at current position in editor.
// Author(s):   Conan Reis
void SkEditView::toggle_breakpoint_enable()
  {
  if (m_member_info.is_valid())
    {
    SkExpressionBase * caret_expr_p = m_edit.get_caret_expr();
    SkBreakPoint *     bp_p         = caret_expr_p ? SkDebug::breakpoint_get_by_expr(*caret_expr_p) : nullptr;

    if (bp_p)
      {
      bp_p->enable_toggle();

      SkConsole::ms_console_p->get_remote_ide()->cmd_breakpoint_update(bp_p, bp_p->is_enabled() ? SkBreakPoint::Update_enable : SkBreakPoint::Update_disable);

      refresh_annotations();
      }
    else
      {
      SkDebug::print("No breakpoint at current position to toggle.\n", SkLocale_local);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Update markings/annotations on editor like breakpoints and bookmarks.
// Author(s):   Conan Reis
void SkEditView::refresh_annotations(
  bool visual_refresh // = true
  )
  {
  m_bps_on_member.remove_all();

  if (m_edit.is_source_available() && m_member_info.is_valid())
    {
    SkDebug::breakpoint_get_all_by_member(&m_bps_on_member, m_member_info);
    }

  if (visual_refresh)
    {
    m_edit.refresh();
    }
  }

//---------------------------------------------------------------------------------------
// Arg         index_start - edit selection start
// Arg         index_end - edit selection end
// Author(s):   Conan Reis
void SkEditView::refresh_member()
  {
  m_bps_on_member.remove_all();

  if (m_member_info.is_valid())
    {
    m_history_combo.set_text(m_member_info.as_file_title());

    if ((m_member_info.m_type <= SkMember__disassembly) && SkConsole::ms_console_p->is_disassembly())
      {
      // Show disassembly version
      m_edit.set_source_file();
      m_edit.set_source(m_member_info.as_invokable()->as_code(), false);
      }
    else
      {
      // Show file text
      AFile       source_file;
      SkOverlay * overlay_p = SkCompiler::ms_compiler_p->find_member_file(m_member_info, &source_file);

      if (overlay_p)
        {
        m_browser_p->set_overlay_current(overlay_p->m_name);
        m_edit.set_source_file(&source_file);
        SkDebug::breakpoint_get_all_by_member(&m_bps_on_member, m_member_info);
        }
      else
        {
        // Source file not found
        m_browser_p->set_overlay_current("???");
        m_edit.set_source_file();

        AString separator('#', 88u);
        AString str(512u,
          "//%s\n"
          "// Could not find source file:\n"
          "//   %s\n"
          "//\n"
          "// [The info/code may have been generated/inferred and there would be no file to view.]\n",
          separator.as_cstr(),
          source_file.as_cstr());

        if (m_member_info.m_type != SkMember_class_meta)
          {
          str.append(
            "//\n"
            "// Here is the disassembly of the code:\n");
          }

        str.append("//");
        str.append(separator);
        str.append("\n\n");

        if (m_member_info.m_type <= SkMember__disassembly)
          {
          SkInvokableBase * invokable_p = m_member_info.as_invokable();

          if (invokable_p)
            {
            str.append(invokable_p->as_code());
            }
          else
            {
            str.append(
              "//Could not find code to disassemble!\n"
              "//[Probably running a script string directly.]\n");
            }
          }

        m_edit.set_source(str, false);
        }
      }
    }
  else
    {
    m_edit.set_source_file();
    m_history_combo.set_text("");
    m_browser_p->set_overlay_current(AString::ms_empty);
    m_edit.empty();
    }
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
void SkEditView::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_browser_p->on_mouse_release(button, buttons, client_pos);
  }

//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkEditView::on_sizing()
  {
  int    spacing = get_spacing();
  AVec2i carea(get_area_client());

  //SendMessage(m_os_handle, WM_SETREDRAW, FALSE, 0);

  int x = carea.m_x - spacing;

  x -= spacing + m_btn_goto.get_width();
  m_btn_goto.set_position(x, spacing);

  x -= (2 * spacing) + m_btn_sync.get_width();
  m_history_combo.set_area(x, a_max(m_history_combo.get_height(), m_btn_goto.get_height()));

  m_edit.set_area(carea.m_x, carea.m_y - m_edit.get_y_rel());

  //int ed_width;
  //int ed_height;

  //m_edit.get_size(&ed_width, &ed_height);
  //carea.m_y -= m_edit.get_y_rel();

  //bool ed_redraw = ((carea.m_x > ed_width) || (carea.m_y > ed_height));
  //m_edit.set_order(nullptr, 0, 0, carea.m_x, carea.m_y, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | (ed_redraw ? 0 : SWP_NOREDRAW));

  //SendMessage(m_os_handle, WM_SETREDRAW, TRUE, 0);

  //::RedrawWindow(m_history_combo.get_os_handle(), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_ALLCHILDREN);
  }


//=======================================================================================
// SkClassBrowser Class Data
//=======================================================================================

SkClassBrowser * SkClassBrowser::ms_browser_p = nullptr;


//=======================================================================================
// SkClassBrowser Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Author(s):   Conan Reis
SkClassBrowser::SkClassBrowser(SkConsole * console_p) :
  SkMainWindowBase(MainWindowBaseType_browser),
  m_singleton_setter(this),
  m_console_p(console_p),
  m_status(this, "",  AFont("Arial Narrow", 12.0f), 0, 0, Size_auto, false, false),
  m_split_main(this),
  m_split_secondary(&m_split_main),
  m_member_debug_tabs(&m_split_main, ARectEdge_right, *AFont::ms_header2_p),
  m_member_view(&m_member_debug_tabs, this),
  //m_call_stack(get_view_def(), &m_member_debug_tabs),
  m_navigation_view(&m_split_secondary, this),
  m_edit_view(&m_split_secondary, this)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Status bar setup
  m_status.set_border(Border_thin_sunken);
  //m_status.set_color_background(g_color_text_bg);
  m_status.enable_read_only();
  m_status.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Navigation View setup
  m_navigation_view.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Editor setup
  m_edit_view.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ClassTree-Editor Secondary Splitter setup
  m_split_secondary.enable_auto_update(false);
  m_split_secondary.set_orientation(ASplitterOS::eOrient(SkConsole::ms_console_p->get_ini_ide().get_value_int_default(
    ASplitterOS::Orient_horiz_ab,
    g_ini_key_split_tree_orient_p,
    g_ini_section_browser_p)));
  m_split_secondary.set_ratio(SkConsole::ms_console_p->get_ini_ide().get_value_default(
    "0.25",
    g_ini_key_split_tree_ratio_p,
    g_ini_section_browser_p).as_float32());
  m_split_secondary.set_pane_a(&m_navigation_view);
  m_split_secondary.set_pane_b(&m_edit_view);
  m_split_secondary.set_user_reorient(ASplitterOS::Reorient_swap_rotate);

  //m_split_secondary.set_color_background(g_color_bg);
  m_split_secondary.enable_auto_update();
  m_split_secondary.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  m_member_debug_tabs.append_win("Members", &m_member_view);
  //m_member_debug_tabs.append_win("Call Stack", &m_call_stack);
  m_member_debug_tabs.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Member List setup
  //m_member_view.set_border(Border_thin_sunken);

  // Secondary Splitter-Member List Main Splitter setup
  m_split_main.enable_auto_update(false);
  m_split_main.set_orientation(ASplitterOS::eOrient(SkConsole::ms_console_p->get_ini_ide().get_value_int_default(
    ASplitterOS::Orient_vert_ab,
    g_ini_key_split_members_orient_p,
    g_ini_section_browser_p)));
  m_split_main.set_ratio(SkConsole::ms_console_p->get_ini_ide().get_value_default(
    "0.66",
    g_ini_key_split_members_ratio_p,
    g_ini_section_browser_p).as_float32());
  m_split_main.set_pane_a(&m_split_secondary);
  m_split_main.set_pane_b(&m_member_debug_tabs);
  m_split_main.enable_auto_update();
  //m_split_main.set_color_background(g_color_bg);
  m_split_main.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Settings needed before menu is created
  SkDebug::enable_preference(
    SkDebug::PrefFlag_break_print_callstack,
    SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(
      SkDebug::is_preference(SkDebug::PrefFlag_break_print_callstack),
      g_ini_key_break_callstack_p,
      g_ini_section_debugger_p));

  SkDebug::enable_preference(
    SkDebug::PrefFlag_break_print_locals,
    SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(
      SkDebug::is_preference(SkDebug::PrefFlag_break_print_locals),
      g_ini_key_break_locals_p,
      g_ini_section_debugger_p));

  //  Setup the common menubar.
  setup_menubar();


/*
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Find Commands
    // APopMenuOS find_popup;
    // 
    //   //--------------------
    //   edit_popup.append_separator();
    //   edit_popup.append_submenu(&find_popup, "Find/Replace");
    //   find_popup.append_item("Find...\t[Ctrl+F]",                             SkBrowseMenu_edit_find, false);
    //   find_popup.append_item("Find Next\t[F3]",                               SkBrowseMenu_edit_find_next, false);
    //   find_popup.append_item("Find Previous\t[Shift+F3]",                     SkBrowseMenu_edit_find_prev, false);
    //   find_popup.append_item("Find Selected Next\t[Ctrl+F3]",                 SkBrowseMenu_edit_find_next_populate, false);
    //   find_popup.append_item("Find Selected Previous\t[Ctrl+Shift+F3]",       SkBrowseMenu_edit_find_prev_populate, false);
    //   find_popup.append_item("Find Next Incrementally\t[Ctrl+I]",             SkBrowseMenu_edit_find_next, false);
    //   find_popup.append_item("Find Previous Incrementally\t[Ctrl+Shift+I]",   SkBrowseMenu_edit_find_next, false);
    //   find_popup.append_item("Find Previous\t[Shift+F3]",                     SkBrowseMenu_edit_find_prev, false);
    //   find_popup.append_item("Replace...\t[Ctrl+R or Ctrl+H]",                SkBrowseMenu_edit_replace, false);
    // 
    //   //--------------------
    //   find_popup.append_separator();
    //   find_popup.append_item("Find in Files...\t[Ctrl+Shift+F]",         SkBrowseMenu_edit_find_files, false);
    //   find_popup.append_item("Replace in Files...\t[Ctrl+Shift+H]",      SkBrowseMenu_edit_replace_files, false);
    // 
    //   //--------------------
    //   find_popup.append_separator();
    //   find_popup.append_item("Match Case",                 SkBrowseMenu_edit_match_case, false, false);
    //   find_popup.append_item("Match Wold Word",            SkBrowseMenu_edit_match_word, false, false);
    //   find_popup.append_item("Match Regular Expressions",  SkBrowseMenu_edit_match_reg_exprs, false, false);
    // 
    //   //--------------------
    //   find_popup.append_separator();
    //   find_popup.append_item("Search Selection",           SkBrowseMenu_edit_search_selection, false, false);
    //   find_popup.append_item("Incremental Search",         SkBrowseMenu_edit_search_incremental, false, false);
    //   find_popup.append_item("Highlight Matches",          SkBrowseMenu_edit_search_highlight, false, false);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Format Commands
    APopMenuOS format_menu;

      //--------------------
      edit_menu.append_separator();
      edit_menu.append_submenu(&format_menu, "Formatting");
      format_menu.append_item("Indent lines\t[Tab]",          SkBrowseMenu_edit_indent);
      format_menu.append_item("Unindent lines\t[Shift+Tab]",  SkBrowseMenu_edit_unindent);

      // //--------------------
      // format_menu.append_separator();
      // format_menu.append_item("Convert tabs to spaces",       SkBrowseMenu_edit_tabs_to_spaces, false);
      // format_menu.append_item("Trim trailing spaces",         SkBrowseMenu_edit_trim_trailing_spaces, false);
      // format_menu.append_item("lowercase",                    SkBrowseMenu_edit_lowercase, false);
      // format_menu.append_item("UPPERCASE\t[Ctrl+Shift+A]",    SkBrowseMenu_edit_uppercase, false);
      // format_menu.append_item("Capitalize",                   SkBrowseMenu_edit_capitalize, false);
      // 
      // //--------------------
      // format_menu.append_separator();
      // format_menu.append_item("Sort lines",                   SkBrowseMenu_edit_sort, false);
      // 
      // //--------------------
      // // Code Formatting
      // format_menu.append_separator();
      // format_menu.append_item("Toggle Comment \/* *\/",         SkBrowseMenu_edit_comment_block, false);
      // format_menu.append_item("Toggle Comment Lines //",      SkBrowseMenu_edit_comment_lines, false);
      // format_menu.append_item("Wrap in code block []",        SkBrowseMenu_edit_block_wrap, false);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Version Control Commands
    APopMenuOS p4_menu;
    bool       p4_enabled = SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4;

      //--------------------
      edit_menu.append_separator();
      edit_menu.append_submenu(&p4_menu, "Version Control");
      p4_menu.append_item("Perforce - Checkout\t[Alt+P]",          SkBrowseMenu_edit_p4_checkout, p4_enabled);
      // p4_menu.append_item("Perforce - Revert...",                  SkBrowseMenu_edit_p4_revert, false);
      // p4_menu.append_item("Perforce - Diff Previous...\t[Ctrl+P]", SkBrowseMenu_edit_p4_diff, false);
      // p4_menu.append_item("Perforce - History...\t[Ctrl+Shift+P]", SkBrowseMenu_edit_p4_history, false);
      // p4_menu.append_item("Perforce - Timelapse View...",          SkBrowseMenu_edit_p4_timelapse, false);
      // p4_menu.append_item("Perforce - Properties...",              SkBrowseMenu_edit_p4_properties, false);
      p4_menu.append_item("Use Perforce",                          SkBrowseMenu_edit_p4_toggle, true, p4_enabled);
*/

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup main window
  //set_color_background(g_color_bg);
  enable_sizing();
  enable_title_bar();
  //set_icon(IDI_SKOOKUM);
  set_title_buttons(TitleButton_min_max);
  update_title();

  load_settings();

  m_member_view.get_splitter().refresh();

  status_update();
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
SkClassBrowser::~SkClassBrowser()
  {
  }


//---------------------------------------------------------------------------------------
//  Called when a submenu is about to become active/displayed.
//  Arg           Menu handle/ID
//  Modifiers:    virtual - override for custom behaviour
//  Author(s):    John Stenersen
bool SkClassBrowser::on_submenu_init(HMENU submenu)
  {
  return refresh_menubar(submenu, get_focused_browser(), get_focused_browser_type());
  } //  SkClassBrowser::on_submenu_init()


//---------------------------------------------------------------------------------------
// Enables/disables viewing expression span guide in editor
// Author(s):   Conan Reis
void SkClassBrowser::enable_debug_preference(
  SkDebug::ePrefFlag perf_flag,
  bool               enable // = true
  )
  {
  bool prev_pref = SkDebug::is_preference(perf_flag);

  if (prev_pref != enable)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SkDebug::enable_preference(perf_flag, enable);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Update menu
    const char *  ini_key_p = nullptr;
    eSkMenu       choice    = SkMenu_unimplemented;
    AMenuOS       menu      = AMenuOS::get_menu_bar(*this);

    switch (perf_flag)
      {
      case SkDebug::PrefFlag_break_print_callstack:
        choice    = SkMenu_debug_break_callstack;
        ini_key_p = g_ini_key_break_callstack_p;
        break;

      case SkDebug::PrefFlag_break_print_locals:
        choice    = SkMenu_debug_break_locals;
        ini_key_p = g_ini_key_break_locals_p;
        break;
      }
      
    menu.check_item(choice, enable);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Update preferences file
    SkConsole::ms_console_p->get_ini_ide().set_value_bool(
      enable,
      ini_key_p,
      g_ini_section_debugger_p);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Update runtime
    m_console_p->get_remote_ide()->cmd_debug_preferences();
    }
  }

//---------------------------------------------------------------------------------------
void SkClassBrowser::update_title()
  {
  AString title("SkookumIDE: Browser (");
  title += SkCompiler::ms_compiler_p->get_ini_project_name();
  title += ")";
  set_title(title);
  }

//---------------------------------------------------------------------------------------
//  Update the status bar
// Author(s):    Conan Reis
void SkClassBrowser::status_update()
  {
  SkEditor &   editor     = m_edit_view.get_editor();
  uint32_t     idx_caret  = editor.get_caret_index();
  uint32_t     idx_begin;
  uint32_t     idx_end;
  uint32_t     row        = editor.get_row_from_index(idx_caret);
  const char * mod_cstr_p = editor.is_source_available()
      ? (editor.is_read_only()
        ?   "[Read Only]"
        : (m_edit_view.is_modified()
          ? "[Modified] "
          : "                 "))
      :     "[No Source]";

  editor.get_selection(&idx_begin, &idx_end);

  // $Note - CReis Assume /r endings for editor and /r/n for file
  if (idx_begin != idx_end)
    {
    m_status.set_text(a_str_format(
      "Overlay: %s\t%s\tLine %-4u\tColumn %-4u\tIndex %-7u\tSelection %7u-%-8u\t%s",
      m_overlay_current_name.as_cstr(),
      mod_cstr_p,
      row + 1u,
      editor.get_column_from_index(idx_caret) + 1u,
      idx_caret + row,
      idx_begin + editor.get_row_from_index(idx_begin),
      idx_end + editor.get_row_from_index(idx_end),
      editor.is_insert_mode() ? "INS" : "OVR"));
    }
  else
    {
    m_status.set_text(a_str_format(
      "Overlay: %s\t%s\tLine %-4u\tColumn %-4u\tIndex %-7u\t\t\t\t%s",
      m_overlay_current_name.as_cstr(),
      mod_cstr_p,
      row + 1u,
      editor.get_column_from_index(idx_caret) + 1u,
      idx_caret + row,
      editor.is_insert_mode() ? "INS" : "OVR"));
    }
  }

//---------------------------------------------------------------------------------------
// Arg         info - script/member to browse
// Arg         index_start - source file selection start
// Arg         index_end - source file selection end
// Author(s):   Conan Reis
void SkClassBrowser::set_member(
  const SkContextInfo & info,
  uint                   index_start, // = 0u
  uint                   index_end    // = ADef_uint32
  )
  {
  m_navigation_view.set_class(info.get_class(), (info.m_type == SkMember_class_meta) ? AFlag_on : AFlag_off);
  m_edit_view.set_member(info, index_start, index_end);
  m_member_view.set_member(info);
  }

//---------------------------------------------------------------------------------------
// Unhooks browser from class hierarchy
// 
// Author(s):   Conan Reis
void SkClassBrowser::unhook()
  {
  m_edit_view.member_invalidate();
  m_edit_view.history_clear();
  m_member_view.set_class(nullptr);
  m_navigation_view.set_class(nullptr);
  }

//---------------------------------------------------------------------------------------
// Rehooks browser to class hierarchy
// Author(s):   Conan Reis
void SkClassBrowser::rehook()
  {
  m_navigation_view.rehook();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkClassBrowser::load_settings(eLoadView load_view)
  {
  AIni & ide_ini = SkConsole::ms_console_p->get_ini_ide();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Bring window up using previous settings if available.
  // $Revisit - CReis These appear to be magical numbers - use values from ADisplay instead
  // $Revisit - CReis Many launching apps seem to give an incorrect starting show state, so ignore it
  if (load_view == LoadView_restore)
    {
    ini_restore_view(g_ini_section_browser_p, ARegion(34, 34, 1240, 1000), AShowState_normal_focus, false, false, &ide_ini);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set Class Tree to last class selected
  SkClass * last_class_p = SkBrain::get_class(ASymbol::create(
    ide_ini.get_value_default(
      SkBrain::ms_object_class_p->get_name_str(),
      g_ini_key_last_class_p,
      g_ini_section_browser_p),
    ATerm_long));

  if (last_class_p == nullptr)
    {
    last_class_p = SkBrain::ms_object_class_p;
    }

  m_navigation_view.set_class(last_class_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get last member edited
  SkContextInfo member_info;
  SkParser      member_name(ide_ini.get_value_default(
    AString::ms_empty,
    g_ini_key_last_member_p,
    g_ini_section_browser_p));

  member_info.m_type = member_name.identify_member_filename(&member_info.m_member_id, &member_info.m_class_scope);

  // $Revisit - CReis Quick fix for case lost when saving to ini file.
  if (member_info.is_valid() && member_info.get_class())
    {
    SkQualifier * member_p = nullptr;
    ASymbol       name     = member_info.m_member_id.get_name();

    switch (member_info.m_type)
      {
      case SkMember_method:
        member_p = member_info.m_class_scope
          ? member_info.get_class()->get_class_methods().get(name)
          : member_info.get_class()->get_instance_methods().get(name);
        break;

      case SkMember_coroutine:
        member_p = member_info.get_class()->get_coroutines().get(name);
        break;

      case SkMember_data:
        {
        // A somewhat hokey way to get a qualifier, but it should work
        SkClass * class_p = member_info.get_class();

        if (member_info.m_class_scope
          ? class_p->get_class_data().is_filled()
          : class_p->get_instance_data().is_filled())
          {
          static SkQualifier s_member;

          s_member.set_scope(class_p);
          s_member.set_name(member_info.m_class_scope
            ? class_p->get_class_data().get_first()->get_name()
            : *class_p->get_instance_data().get_first());
          member_p = &s_member;
          }
        }
        break;
      }

    if (member_p)
      {
      m_edit_view.set_member(member_info);

      if (m_member_view.is_inherited_shown()
        ? last_class_p->is_class(*member_info.get_class())
        : (last_class_p == member_info.get_class()))
        {
        m_member_view.select_member(member_info);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Saves the browser settings
// 
// See:         load_settings()
// Author(s):   Conan Reis
void SkClassBrowser::save_settings()
  {
  AIni & ide_ini = SkConsole::ms_console_p->get_ini_ide();

  m_edit_view.get_editor().save_settings();

  if (ide_ini.ensure_writable_query())
    {
    ini_save_view(g_ini_section_browser_p, false, &ide_ini);
    m_member_view.save_settings();

    SkClass * last_class_p = m_navigation_view.get_class_tree().get_selected_class();

    if (last_class_p == nullptr)
      {
      last_class_p = SkBrain::ms_object_class_p;
      }

    ide_ini.set_value(
      last_class_p->get_name_str(),
      g_ini_key_last_class_p,
      g_ini_section_browser_p);

    ide_ini.set_value(
      m_edit_view.get_member_name(),
      g_ini_key_last_member_p,
      g_ini_section_browser_p);

    //  Save Splitter info
    ide_ini.set_value(
      AString::ctor_int(m_split_secondary.get_orientation()),
      g_ini_key_split_tree_orient_p,
      g_ini_section_browser_p);

    ide_ini.set_value(
      AString::ctor_float(m_split_secondary.get_ratio()),
      g_ini_key_split_tree_ratio_p,
      g_ini_section_browser_p);

    ide_ini.set_value(
      AString::ctor_int(m_split_main.get_orientation()),
      g_ini_key_split_members_orient_p,
      g_ini_section_browser_p);

    ide_ini.set_value(
      AString::ctor_float(m_split_main.get_ratio()),
      g_ini_key_split_members_ratio_p,
      g_ini_section_browser_p);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkClassBrowser::on_class_selected(
  SkClass * class_p,
  bool      show_meta // = true
  )
  {
  m_navigation_view.set_class(class_p);
  m_member_view.set_class(class_p);

  if (show_meta)
    {
    SkContextInfo info(SkQualifier(ASymbol::ms_null, class_p), SkMember_class_meta, false);

    m_edit_view.set_member(info);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkClassBrowser::on_member_selected(SkMemberReference * member_p)
  {
  m_edit_view.set_member(*member_p);
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
void SkClassBrowser::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  switch (button)
    {
    case AMouse_x1:
      m_edit_view.history_prev();
      break;

    case AMouse_x2:
      m_edit_view.history_next();
      break;
    }
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
SkClassBrowser::on_key_press(
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
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  eAKeyMod mod_keys = AKeyboard::get_mod_keys();

  switch (key)
    {
    case AKey_f4:
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!repeated)
          {
          on_close_attempt();
          }
        return false;
        }
      break;

    case AKey_f9:  // Toggle breakpoint
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          m_edit_view.toggle_breakpoint();
          }
        return false;
        }
      break;

    case AKey_f12:
      if (mod_keys == AKeyMod_none)
        {
        // Also see Alt+G & Ctrl+,
        if (!repeated)
          {
          m_console_p->display_goto_context_editor();
          }
        return false;
        }
      break;

    case AKey_tab:
      if (mod_keys == AKeyMod_ctrl_shift)
        {
        if (!repeated)
          {
          SkConsole::ms_console_p->show();
          SkConsole::ms_console_p->make_foreground();
          }

        return false;
        }
      break;

    case AKey_escape:
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          if (m_navigation_view.is_create_popup())
            {
            m_navigation_view.toggle_create_popup();
            }
          }
        return false;
        }
      break;

    case AKey_left:
      if (mod_keys == AKeyMod_alt)
        {
        m_edit_view.history_prev();
        return false;
        }
      break;

    case AKey_browser_back:
      m_edit_view.history_prev();
      return false;

    case AKey_right:
      if (mod_keys == AKeyMod_alt)
        {
        m_edit_view.history_next();
        return false;
        }
      break;

    case AKey_browser_forward:
      m_edit_view.history_next();
      return false;

    case AKey_insert:
      switch(mod_keys)
        {
        case AKeyMod_shift:
          m_edit_view.get_editor().clipboard_paste_plain();
          return false;

        case AKeyMod_ctrl:
          if (!repeated)
            {
            m_edit_view.get_editor().clipboard_copy_plain_sel_or_row();
            }
          return false;

        case AKeyMod_alt:
          m_edit_view.get_editor().replace_selection(SkEditBox::get_result_string(), true);
          return false;

        case AKeyMod_alt_ctrl:
          {
          AClipboard clip(SkConsole::ms_console_p);

          clip.set_text(SkEditBox::get_result_string());
          break;
          }
        }
      break;

    case AKey_comma:
      if (mod_keys == AKeyMod_ctrl)
        {
        // Also see Alt+G, F12, Ctrl+,
        if (!repeated)
          {
          m_console_p->display_goto_context_editor();
          }
        return false;
        }
      break;

    case AKey_tilde:
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!repeated)
          {
          m_console_p->toggle_ide();
          }
        return false;
        }
      break;

    case 'E':
      switch (mod_keys)
        {
        case AKeyMod_ctrl:
          {
          if (!repeated)
            {
            m_edit_view.edit_externally();
            }
          return false;
          }

        case AKeyMod_ctrl_shift:
          {
          if (!repeated)
            {
            const SkContextInfo & context = get_member_info();

            if (context.is_valid())
              {
              m_edit_view.save_changes();
              context.action_goto_file_explorer();
              }
            }

          return false;
          }
        }
      break;

    case 'G':
      if (!repeated)
        {
        switch (mod_keys)
          {
          case AKeyMod_alt:
            // Also see Alt+G, F12, Ctrl+,
            m_console_p->display_goto_context_editor();
            return false;

          case AKeyMod_alt_shift:
            m_console_p->display_goto_dialog(SkMatchKind_data);
            return false;

          case AKeyMod_alt_ctrl:
            m_console_p->display_goto_dialog(SkMatchKind_routines);
            return false;

          case AKeyMod_ctrl_shift:
            m_console_p->display_goto_context_focus();
            return false;

          case AKeyMod_alt_ctrl_shift:
            m_console_p->display_goto_dialog(SkMatchKind_all);
            return false;
          }
        }
      break;

    case 'N':
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!repeated)
          {
          m_navigation_view.show_create_popup();
          }

        return false;
        }
      break;

    case 'P':
      if ((mod_keys == AKeyMod_alt)
        && (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4))
        {
        if (!repeated)
          {
          get_member_info().action_p4_checkout();
          }

        return false;
        }
      break;

    case 'S':  // Save file
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!repeated)
          {
          m_edit_view.save_changes();
          }

        return false;
        }
      break;
    }

  return m_console_p->on_key_press(key, repeated);
  } //  SkClassBrowser::on_key_press()


//---------------------------------------------------------------------------------------
// Called when the user makes a selection from associated menu bar submenu
//             or associated pop-up / context menu.
// Arg         item_id - id of item selected
// Modifiers:   virtual - overridden from AWindow
// Author(s):   Conan Reis
void SkClassBrowser::on_menu_command(uint32_t item_id)
  {
    if (on_menubar(item_id))
    {
    return;   //  No further processing required.
    }

  switch(item_id)
    {
    // $Vital - CReis Ensure that the edit commands ensure that the source is editable.

    case SkBrowseMenu_edit_indent:
      m_edit_view.get_editor().indent_selection(SkDebug::ms_indent_size);
      break;

    case SkBrowseMenu_edit_unindent:
      m_edit_view.get_editor().unindent_selection(
        SkDebug::ms_indent_size, SkDebug::ms_tab_stops);
      break;

    default:
      //   // Find Commands
      //   SkBrowseMenu_edit_find,
      //   SkBrowseMenu_edit_find_next,
      //   SkBrowseMenu_edit_find_prev,
      //   SkBrowseMenu_edit_replace,
      //   SkBrowseMenu_edit_find_files,
      //   SkBrowseMenu_edit_replace_files,
      // 
      //   // Format Commands
      //   SkBrowseMenu_edit_tabs_to_spaces,
      //   SkBrowseMenu_edit_trim_trailing_spaces,
      //   SkBrowseMenu_edit_lowercase,
      //   SkBrowseMenu_edit_uppercase,
      //   SkBrowseMenu_edit_capitalize,
      //   SkBrowseMenu_edit_sort,
      // 
      //   SkBrowseMenu_edit_comment_block,
      //   SkBrowseMenu_edit_comment_lines,
      //   SkBrowseMenu_edit_block_wrap,
      // 
      //   // Version Control Commands
      //   SkBrowseMenu_edit_p4_revert,
      //   SkBrowseMenu_edit_p4_diff,
      //   SkBrowseMenu_edit_p4_history,
      //   SkBrowseMenu_edit_p4_timelapse,
      //   SkBrowseMenu_edit_p4_properties,

      SkDebug::print(a_str_format(A_SOURCE_STR " [Menu incomplete - item: %u]\n", item_id), SkLocale_local);
    }
  }

//---------------------------------------------------------------------------------------
// Called when the close button on the title bar is pressed.
// Returns:    true and the default closing behaviour is taken
// Author(s):   Conan Reis
bool SkClassBrowser::on_close_attempt()
  {
  save_settings();
  hide();

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
// Author(s):   Conan Reis
bool SkClassBrowser::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  // If no browser related window has the focus, set it to the editor.
  if (!SkMainWindowBase::get_focused_browser())
    {
    get_edit_view().get_editor().set_focus();
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called when the menubar exits.
// 
// # Author(s): John Stenersen
void SkClassBrowser::on_menu_modal(bool enter_b)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if (enter_b)
    {
    return;   //  Don't care about the entry.
    }

  // Return to the window that last had focus in the browser.
  if (!SkMainWindowBase::get_focused_browser())
    {
    get_edit_view().get_editor().set_focus();
    }
  else
    {
    SkMainWindowBase::get_focused_browser()->set_focus();
    }

  return;
  }


//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkClassBrowser::on_sizing()
  {
  AVec2i carea(get_area_client());
  int    status_height = m_status.get_font().get_height() + SkClassBrowser_status_offset + SkClassBrowser_status_inset2;

  m_split_main.set_area(AVec2i(carea.m_x, carea.m_y - status_height - SkClassBrowser_status_inset2));

  m_status.set_region(
    SkClassBrowser_status_inset,
    carea.m_y - status_height + SkClassBrowser_status_inset,
    carea.m_x - SkClassBrowser_status_inset2,
    status_height - SkClassBrowser_status_inset2);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets the name of the user from the ini file.
// 
// Returns:     Name of the user
// Modifiers:   static
// Author(s):   Conan Reis
AString SkClassBrowser::get_ini_user_name()
  {
  // $Revisit - CReis Should prompts the user for name if one does not exist in ini.
  AString user_name = SkConsole::ms_console_p->get_ini_ide().get_value(g_ini_key_user_name_p, g_ini_section_browser_p);

  if (user_name == AIni::ms_not_present_str)
    {
    // Prompt user for name
    // $Revisit - CReis [Incomplete] Prompt for user name.
    user_name = "Scripter Dude/Dudette";

    SkConsole::ms_console_p->get_ini_ide().set_value(g_ini_key_user_name_p, user_name, g_ini_section_browser_p);
    }

  return user_name;
  }


//  End of SkClassBrowser.cpp
