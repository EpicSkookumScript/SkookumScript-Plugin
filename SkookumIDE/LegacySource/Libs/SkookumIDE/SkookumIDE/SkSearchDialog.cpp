// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Symbol Search Dialog
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkSearchDialog.hpp>
#include <SkookumIDE/SkClassBrowser.hpp>
#ifndef SK_NO_RESOURCES
  #include <SkookumIDE/SkookumIDE_Res.h>
#endif
#include <AgogCore/ACompareMethod.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <AgogGUI/ATrueTypeFont.hpp>
#include <AgogGUI_OS/ADialogOS.hpp>
#include <SkookumScript/SkBrain.hpp>


//=======================================================================================
// Local Global Structures
//=======================================================================================

typedef AMethodArg<SkMatchList, tSkMatchText *>       tSkMatchTextCall;
typedef ACompareMethod<SkMatchList, SkMatchReference> tSkMatchCompareCall;

namespace
{

  enum 
    {
    SkMatchInfo_population_init   = 512,  // Initial simultaneous member info structures in member lists
    SkMatchInfo_population_expand = 128,  // Member info structure grow amount - used in member lists
    };

  AString  g_search_prompt;

} // End unnamed namespace


//=======================================================================================
// SkMatchReference Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Retrieves a SkMatchReference object from the dynamic pool and initializes
//              it for use.  This should be used instead of 'new' because it prevents
//              unnecessary allocations by reusing previously allocated objects.
// Returns:     a dynamic SkMatchReference
// See:         pool_delete()
// Notes:       To 'deallocate' an object that was retrieved with this method, use
//              'pool_delete()' rather than 'delete'.
// Modifiers:    static
// Author(s):    Conan Reis
SkMatchReference * SkMatchReference::pool_new(const SkContextInfo & member_info)
  {
  SkMatchReference * info_p = get_pool().allocate();

  info_p->init(member_info);

  return info_p;
  }

//---------------------------------------------------------------------------------------
// Returns dynamic reference pool. Pool created first call and reused on successive calls.
// 
// #Notes
//   Uses Scott Meyers' tip "Make sure that objects are initialized before they're used "
//   from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd Edition]
//   This is instead of using a non-local static object for a singleton.
//   
// #Modifiers  static
// #Author(s)  Conan Reis
AObjReusePool<SkMatchReference> & SkMatchReference::get_pool()
  {
  static AObjReusePool<SkMatchReference> s_pool(SkMatchInfo_population_init, SkMatchInfo_population_expand);

  return s_pool;
  }


//=======================================================================================
// SkMatchList Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkMatchList::SkMatchList(
  SkSearchDialog * search_dlg_p
  ) :
  AListOS<SkMatchReference>(search_dlg_p, search_dlg_p->get_font()),
  m_search_dlg_p(search_dlg_p)
  {
  enable_remove_events();
  enable_image_sharing();
  set_image_list(SkConsole::ms_console_p->get_member_images());
  enable_gridlines();
  //ensure_size(SkContextInfo_code_size_init);

  m_col_name_p = new AColumnOS<SkMatchReference>(
    "Name",
    new tSkMatchTextCall(this, &SkMatchList::on_text_name),
    new tSkMatchCompareCall(this, &SkMatchList::on_compare_name),
    false);
  column_append(m_col_name_p);

  m_col_instance_p = new AColumnOS<SkMatchReference>(
    "i/c", nullptr, new tSkMatchCompareCall(this, &SkMatchList::on_compare_instance), false);
  column_append(m_col_instance_p);

  m_col_scope_p = new AColumnOS<SkMatchReference>(
    "Scope",
    new tSkMatchTextCall(this, &SkMatchList::on_text_scope),
    new tSkMatchCompareCall(this, &SkMatchList::on_compare_scope),
    false);
  column_append(m_col_scope_p);

  column_append(new AColumnOS<SkMatchReference>(
    "Info",
    new tSkMatchTextCall(this, &SkMatchList::on_text_info)));
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkMatchList::~SkMatchList()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMatchList::append_member(const SkContextInfo & info)
  {
  uint row = append(*m_search_dlg_p->pool_new_info(info));

  set_image_index(
    info.m_class_scope
      ? SkMemberImage_scope_class
      : SkMemberImage_scope_instance,
    row,
    m_col_instance_p->get_rank());

  set_image_index(
    g_member_type_to_image[info.m_type],
    row,
    m_col_name_p->get_rank());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMatchList::append_invokables(
  SkInvokableBase ** members_pp,
  uint                length,
  eSkInvokable       custom_type,
  bool               show_custom,
  bool               show_atomic,
  bool               class_scope
  )
  {
  bool               placeholder_b;
  eSkInvokable       type;
  SkInvokableBase ** members_end_pp = members_pp + length;
  SkContextInfo      member_info;

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

      append_member(member_info);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the list render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT SkMatchList::on_custom_draw(NMLVCUSTOMDRAW * info_p)
  {
  // uint32_t item_info = info_p->nmcd.lItemlParam;
  // uint  row       = info_p->nmcd.dwItemSpec;
  // uint  rank      = info_p->iSubItem;

  return CDRF_DODEFAULT;
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
bool SkMatchList::on_mouse_press(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos,
  bool           double_click
  )
  {
  if (buttons == AMouse_left)
    {
    SkMatchReference * match_p;
    eAKeyMod           mod_keys = AKeyboard::get_mod_keys();

    if (double_click)
      {
      match_p = get_selected_first();

      switch (mod_keys)
        {
        case AKeyMod_alt:
          match_p->action_insert_name_editor();
          m_search_dlg_p->close_default();
          return false;

        case AKeyMod_shift:
          match_p->action_copy_name();
          m_search_dlg_p->close_default();
          return false;
        }
      }
    else
      {
      if (mod_keys == AKeyMod_ctrl)
        {
        uint32_t row;

        match_p = pos2item(client_pos, &row);

        if (match_p)
          {
          select_row(row);
          //update_row(row);
          process_messages(AAsyncFilter__none);
          match_p->action_goto_browser();
          }

        return false;
        }
      }
    }

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
bool SkMatchList::on_context_menu(const AVec2i & screen_pos)
  {
  SkMatchReference * match_p = get_selected_first();

  if (match_p == nullptr)
    {
    return true;
    }


  enum eMatchPop
    {
    MatchPop_browse,              // Open in Browser     [Ctrl + click]
    MatchPop_browse_close,        // Open in Browser & Close   [Double click]
    MatchPop_copy,                // Copy to clipboard         [Ctrl + C, Ctrl + Insert]
    MatchPop_copy_close,          // Copy to clipboard & Close [Shift + Double click]
    MatchPop_insert_editor,       // Paste in editor & Close   [Alt + Double click]
    MatchPop_insert_workspace,    // Paste in workspace & Close

    MatchPop_copy_path,           // Copy path to clipboard
    MatchPop_open_explorer,       // Open in file explorer     [Ctrl + F]
    MatchPop_open_associated,     // Open in external editor   [Ctrl + E]

    // Version Control
      MatchPop_p4_checkout,       // Perforce - checkout       [Alt + P]
      MatchPop_p4_revert,         // Perforce - revert & confirm
      MatchPop_p4_diff,           // Perforce - diff prev      [Ctrl + P]
      MatchPop_p4_history,        // Perforce - history        [Ctrl + Shift + P]
      MatchPop_p4_timelapse,      // Perforce - timelapse view
      MatchPop_p4_properties      // Perforce - properties
    };

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  APopMenuOS pop_menu;

  pop_menu.append_item("Go to Browser\t[Ctrl+Click]",              MatchPop_browse);
  pop_menu.append_item("Go to Browser && Close\t[Double Click]",   MatchPop_browse_close);
  pop_menu.append_item("Copy Name\t[Ctrl+C or Ctrl+Ins]",          MatchPop_copy);
  pop_menu.append_item("Copy Name && Close\t[Shift+DClick]",       MatchPop_copy_close);
  pop_menu.append_item("Insert in editor && Close\t[Alt+DClick]",  MatchPop_insert_editor);
  pop_menu.append_item("Insert in Workbench && Close",             MatchPop_insert_workspace);

  pop_menu.append_separator();
  pop_menu.append_item("Copy File Path",                           MatchPop_copy_path);
  pop_menu.append_item("Open in external editor...\t[Ctrl+E]",     MatchPop_open_associated);
  pop_menu.append_item("Open in file explorer...\t[Ctrl+Shift+E]", MatchPop_open_explorer);

  if (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
    {
    APopMenuOS p4_menu;
    pop_menu.append_separator();
    pop_menu.append_submenu(&p4_menu, "Version Control");
    p4_menu.append_item("Perforce - Checkout\t[Alt+P]",       MatchPop_p4_checkout);
    //p4_menu.append_item("Perforce - Revert...",               MatchPop_p4_revert, false);
    //p4_menu.append_item("Perforce - Diff Previous\t[Ctrl+P]", MatchPop_p4_diff, false);
    //p4_menu.append_item("Perforce - History\t[Ctrl+Shift+P]", MatchPop_p4_history, false);
    //p4_menu.append_item("Perforce - Timelapse View",          MatchPop_p4_timelapse, false);
    //p4_menu.append_item("Perforce - Properties",              MatchPop_p4_properties, false);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool call_parent = false;
  uint32_t pop_id      = 0u;

  if (pop_menu.show(screen_pos, *this, &pop_id))
    {
    switch (eMatchPop(pop_id))
      {
      case MatchPop_browse:
        match_p->action_goto_browser();
        break;

      case MatchPop_browse_close:
        match_p->action_goto_browser();
        m_search_dlg_p->close_default();
        break;

      case MatchPop_copy:
        match_p->action_copy_name();
        break;

      case MatchPop_copy_close:
        match_p->action_copy_name();
        m_search_dlg_p->close_default();
        break;

      case MatchPop_insert_editor:
        match_p->action_insert_name_editor();
        m_search_dlg_p->close_default();
        break;

      case MatchPop_insert_workspace:
        match_p->action_insert_name_workspace();
        m_search_dlg_p->close_default();
        break;

      case MatchPop_copy_path:
        match_p->action_copy_path();
        break;

      case MatchPop_open_explorer:
        match_p->action_goto_file_explorer();
        break;

      case MatchPop_open_associated:
        match_p->action_edit_external();
        break;

      case MatchPop_p4_checkout:
        match_p->action_p4_checkout();
        break;

      case MatchPop_p4_revert:
        match_p->action_p4_revert();
        break;

      case MatchPop_p4_diff:
        match_p->action_p4_diff_prev();
        break;

      case MatchPop_p4_properties:
        match_p->action_p4_properties();
        break;

      case MatchPop_p4_timelapse:
        match_p->action_p4_timelapse();
        break;

      case MatchPop_p4_history:
        match_p->action_p4_history();
        break;
      }
    }

  // Call parent's on_context_menu()?
  return call_parent;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
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
bool SkMatchList::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  if (!repeated)
    {
    eAKeyMod mod_keys = AKeyboard::get_mod_keys();

    switch (key)
      {
      case AKey_insert:
      case 'C':
        if (mod_keys == AKeyMod_ctrl)
          {
          SkMatchReference * match_p = get_selected_first();

          if (match_p)
            {
            match_p->action_copy_name();
            }

          return false;
          }
        break;
      }
    }

  return m_search_dlg_p->on_key_press(key, repeated);
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkMatchList::on_subitem_activated(
  uint row,
  uint rank
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "(row: %u, column: %u) 0x%p\n", row, rank, this);

  m_search_dlg_p->on_ok();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMatchList::on_item_focused(SkMatchReference * item_p, uint row)
  {
  //m_search_dlg_p->on_member_selected(item_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMatchList::on_item_removed_row(uint row, uintptr_t item_info)
  {
  m_search_dlg_p->pool_delete_info((SkMatchReference *)item_info);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkMatchList::on_compare_instance(const SkMatchReference & lhs, const SkMatchReference & rhs)
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
void SkMatchList::on_text_name(tSkMatchText * info_p)
  {
  AString *       item_str_p  = info_p->m_subitem_str_p;
  const ASymbol & member_name = info_p->m_item_p->m_member_id.get_name();

  switch (info_p->m_item_p->m_type)
    {
    case SkMember_method:
    case SkMember_method_func:
    case SkMember_method_mthd:
      {
      // Determine if it is an operator name
      const ASymbol & op_name = SkParser::method_to_operator(member_name);

      if (!op_name.is_null())
        {
        // Show operator symbol[s] if it is also an operator alias
        item_str_p->append(member_name.as_string());
        item_str_p->append("  [", 3u);
        item_str_p->append(op_name.as_string());
        item_str_p->append(']');
        break;
        }
      }

    default:  // Coroutines, data, classes
      item_str_p->append(member_name.as_string());
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkMatchList::on_compare_name(const SkMatchReference & lhs, const SkMatchReference & rhs)
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
void SkMatchList::on_text_scope(tSkMatchText * info_p)
  {
  SkClass * scope_p = info_p->m_item_p->get_class();
  
  if (info_p->m_item_p->m_type == SkMember_class_meta)
    {
    scope_p = scope_p->get_superclass();

    if (scope_p == nullptr)
      {
      info_p->m_subitem_str_p->append("--", 2u);

      return;
      }
    }

  info_p->m_subitem_str_p->append(scope_p->get_name_str());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
eAEquate SkMatchList::on_compare_scope(const SkMatchReference & lhs, const SkMatchReference & rhs)
  {
  SkClass * lscope_p = lhs.get_class();
  SkClass * rscope_p = rhs.get_class();
  
  if (lhs.m_type == SkMember_class_meta)
    {
    lscope_p = lscope_p->get_superclass();

    if (lscope_p == nullptr)
      {
      lscope_p = lhs.get_class();
      }
    }

  if (rhs.m_type == SkMember_class_meta)
    {
    rscope_p = rscope_p->get_superclass();

    if (rscope_p == nullptr)
      {
      rscope_p = rhs.get_class();
      }
    }

  eAEquate result = lscope_p->get_name_str().compare(rscope_p->get_name_str());

  if (result == AEquate_equal)
    {
    // Should sort by type next
    return on_compare_name(lhs, rhs);
    }

  return m_col_scope_p->m_sort_ascending ? result : eAEquate(-result);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkMatchList::on_text_info(tSkMatchText * info_p)
  {
  AString * item_str_p = info_p->m_subitem_str_p;
  eSkMember type       = unify_type(info_p->m_item_p->m_type);

  switch (type)
    {
    case SkMember_method:
    case SkMember_coroutine:
      {
      AString params(info_p->m_item_p->as_invokable()->as_code_params());
      uint     size = item_str_p->get_size();

      info_p->m_save_text = false;

      if (params.get_length() >= (size - 1u))
        {
        item_str_p->append(params.as_cstr(), size - 6u);
        item_str_p->append(" ...", 4u);
        }
      else
        {
        item_str_p->append(params);
        }

      break;
      }

    //case SkMember_class_meta:
    //case SkMember_data:
    //default:
    //  item_str_p->append("--", 2u);
    }
  }


//=======================================================================================
// SkMatchCriteria Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Assignment operator (allows stringization c = c = c)
// Returns:    itself
// Arg         criteria - other criteria info to copy
// Author(s):   Conan Reis
SkMatchCriteria & SkMatchCriteria::operator=(const SkMatchCriteria & criteria)
  {
  m_class_match_type  = criteria.m_class_match_type;
  m_class_match_str   = criteria.m_class_match_str;
  m_member_match_type = criteria.m_member_match_type;
  m_member_match_str  = criteria.m_member_match_str;

  return *this;
  }

//---------------------------------------------------------------------------------------
// String converter
// Returns:    String version of itself
// Author(s):   Conan Reis
AString SkMatchCriteria::as_string() const
  {
  bool class_b  = m_class_match_str.is_filled();
  bool member_b = m_member_match_str.is_filled();
  uint32_t str_size =
    (class_b ? m_class_match_str.get_length() + 2u : 0u)
    + 1u  // For scope delimiter
    + (member_b ? m_member_match_str.get_length() + 2u : 0u);
  AString str(nullptr, str_size, 0u);

  if (class_b)
    {
    if (m_class_match_type == AStrMatch_suffix)
      {
      str.append('*');
      }

    if (m_class_match_type == AStrMatch_exact)
      {
      str.append('"');
      }

    str.append(m_class_match_str);

    if (m_class_match_type == AStrMatch_prefix)
      {
      str.append('*');
      }

    if (m_class_match_type == AStrMatch_exact)
      {
      str.append('"');
      }
    }

  if (member_b)
    {
    if (class_b)
      {
      str.append('@');
      }

    if (m_member_match_type == AStrMatch_suffix)
      {
      str.append('*');
      }

    if (m_member_match_type == AStrMatch_exact)
      {
      str.append('"');
      }

    str.append(m_member_match_str);

    if (m_member_match_type == AStrMatch_prefix)
      {
      str.append('*');
      }

    if (m_member_match_type == AStrMatch_exact)
      {
      str.append('"');
      }
    }

  return str;
  }


//=======================================================================================
// SkSearchDialog Methods
//=======================================================================================

//---------------------------------------------------------------------------------------

void SkSearchDialog::initialize()
  {
  // Initialize static variables
  const_cast<AString&>(g_search_prompt) = "Search for:";
  }

//---------------------------------------------------------------------------------------

void SkSearchDialog::deinitialize()
  {
  const_cast<AString&>(g_search_prompt) = AString::ms_empty;
  }

//---------------------------------------------------------------------------------------

bool SkSearchDialog::is_initialized()
  {
  return !g_search_prompt.is_empty();
  }

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkSearchDialog::SkSearchDialog(
  eSkMatchKind kind // = SkMatchKind_all
  ) :
  m_kind(SkMatchKind__invalid),
  m_match_list(this),
  m_on_toggle_method(this, &SkSearchDialog::on_toggle_filter),
  m_toggle_classes(this, "Classes", ACheckType_2_state, m_font),
  m_toggle_superclasses(this, "Superclasses/Inherited", ACheckType_2_state, m_font),
  m_toggle_subclasses(this, "Subclasses/Derived", ACheckType_2_state, m_font),
  m_toggle_data_instance(this, "Instance Data", ACheckType_2_state, m_font),
  m_toggle_data_class(this, "Class Data", ACheckType_2_state, m_font),
  m_toggle_coroutines(this, "Coroutines", ACheckType_2_state, m_font),
  m_toggle_methods_instance(this, "Instance Methods", ACheckType_2_state, m_font),
  m_toggle_methods_class(this, "Class Methods", ACheckType_2_state, m_font),
  m_toggle_script(this, "Script", ACheckType_2_state, m_font),
  m_toggle_cpp(this, "C++", ACheckType_2_state, m_font),
  m_search_text_y(0),
  m_search_text(SkEditSyntax::Type_single_line, this, SkIncrementalSearchEditBox::ParentContext_search_dialog, "", SkConsole::ms_console_p->get_ini_font()),
  m_ok_btn(this, "OK"),
  m_cancel_btn(this, "Cancel")
  {
  SK_ASSERTX(is_initialized(), "SkSearchDialog must be initialized before use!");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Class Settings Window
  enable_sizing();
  enable_title_bar();
  #ifndef SK_NO_RESOURCES
    set_icon(IDI_SKOOKUM);
  #else
    set_icon_file(SkConsole::make_qualified(AFile("Media\\SkookumScript.ico")).get_file_str().as_cstr());
  #endif
  set_title_buttons(TitleButton_close);

  m_toggle_superclasses.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_subclasses.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_classes.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_coroutines.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_methods_instance.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_methods_class.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_script.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_cpp.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_data_instance.set_on_toggled_func(&m_on_toggle_method, false);
  m_toggle_data_class.set_on_toggled_func(&m_on_toggle_method, false);


  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
  ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
  ::SetBkMode(hdc, OPAQUE);  // TRANSPARENT

  ::ReleaseDC(m_os_handle, hdc);

  int spacing = get_spacing();

  m_match_list.set_position(spacing, spacing);
  m_match_list.set_border(Border_thick_sunken);
  m_match_list.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup OK & Cancel buttons [UI built from bottom to top]
  AVec2i area(m_cancel_btn.get_area());

  area *= 1.15f;

  m_cancel_btn.set_region(area);
  m_cancel_btn.set_on_pressed_func(new AMethod<SkSearchDialog>(this, &SkSearchDialog::close_default));
  m_cancel_btn.show();

  m_ok_btn.set_region(area);
  m_ok_btn.enable_default_border();
  m_ok_btn.set_on_pressed_func(new AMethod<SkSearchDialog>(this, &SkSearchDialog::on_ok));
  m_ok_btn.show();

  m_search_text.set_on_modified_func(new AMethod<SkSearchDialog>(this, &SkSearchDialog::on_search_text_modified));
  m_search_text.set_identify_flags(SkParser::IdentifyFlag_break_strings);
  m_search_text.set_border(Border_sunken);
  m_search_text.set_height_line_auto();
  m_search_text.syntax_highlight();
  m_search_text.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  m_timer.set_idle_func_p(new AMethod<SkSearchDialog>(this, &SkSearchDialog::on_idle));
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkSearchDialog::~SkSearchDialog()
  {
  empty();
  }

//---------------------------------------------------------------------------------------
// Clear previous search
// Author(s):   Conan Reis
void SkSearchDialog::empty()
  {
  m_match_list.remove_all();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkSearchDialog::load_settings(eSkMatchKind kind)
  {
  // Reload settings on new kind of search
  if (m_kind == kind)
    {
    return;
    }

  m_kind = kind;

  set_title_kind();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Filters

  int spacing  = get_spacing();
  int spacing2 = spacing * 2;

  // Always show class filters
  m_rgn_class.m_width = m_toggle_superclasses.get_width() + spacing2;

  int width = m_rgn_class.m_width + (spacing * 4);

  m_toggle_superclasses.enable_toggle(false);
  m_toggle_subclasses.enable_toggle(false);

  // Note that this is only on by default for a class search - when using "search all" it
  // is off by default.
  m_toggle_classes.enable_toggle((kind == SkMatchKind_classes) != 0u);

  m_toggle_superclasses.show();
  m_toggle_subclasses.show();

  // Only show class type filter if all types are available
  if (kind == SkMatchKind_all)
    {
    m_toggle_classes.show();
    m_rgn_class.m_height = spacing * 11;
    }
  else
    {
    m_toggle_classes.hide();
    m_rgn_class.m_height = spacing * 8;
    }

  if (kind & SkMatchKind_routines)
    {
    m_rgn_routine.m_width  = m_toggle_methods_instance.get_width() + m_toggle_script.get_width() + (spacing * 3);
    m_rgn_routine.m_height = spacing * 11;

    width += m_rgn_routine.m_width + spacing;

    m_toggle_coroutines.enable_toggle();
    m_toggle_methods_instance.enable_toggle();
    m_toggle_methods_class.enable_toggle();
    m_toggle_script.enable_toggle();
    m_toggle_cpp.enable_toggle();

    m_toggle_coroutines.show();
    m_toggle_methods_instance.show();
    m_toggle_methods_class.show();
    m_toggle_script.show();
    m_toggle_cpp.show();
    }
  else
    {
    m_toggle_coroutines.enable_toggle(false);
    m_toggle_methods_instance.enable_toggle(false);
    m_toggle_methods_class.enable_toggle(false);

    m_toggle_coroutines.hide();
    m_toggle_methods_instance.hide();
    m_toggle_methods_class.hide();
    m_toggle_script.hide();
    m_toggle_cpp.hide();
    }

  if (kind & SkMatchKind_data)
    {
    m_rgn_data.m_width  = m_toggle_data_instance.get_width() + spacing2;
    m_rgn_data.m_height = spacing * 8;

    width += m_rgn_data.m_width + spacing;

    m_toggle_data_instance.enable_toggle();
    m_toggle_data_class.enable_toggle();

    m_toggle_data_instance.show();
    m_toggle_data_class.show();
    }
  else
    {
    m_toggle_data_instance.enable_toggle(false);
    m_toggle_data_class.enable_toggle(false);

    m_toggle_data_instance.hide();
    m_toggle_data_class.hide();
    }

  int height = spacing * 70;

  width = a_max(width, spacing * 45);

  set_area(width, height);
  set_area_min(AVec2i(width, spacing * 40));


  // $Revisit - Load previous search text


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Display dialog
  set_pos_centered_display();
  on_sizing();
  }

//---------------------------------------------------------------------------------------
// Sets title with type of search and number of matches
// Author(s):   Conan Reis
void SkSearchDialog::set_title_kind(
  bool searching // = false
  )
  {
  const char * title_cstr_p = "Go To";

  switch (m_kind)
    {
    case SkMatchKind_classes:
      title_cstr_p = "Go To Class";
      break;

    case SkMatchKind_data:
      title_cstr_p = "Go To Data Member";
      break;

    case SkMatchKind_routines:
      title_cstr_p = "Go To Code Member";
      break;
    }

  if (searching)
    {
    set_title(a_cstr_format("SkookumIDE: %s  [Searching...]", title_cstr_p));
    }
  else
    {
    set_title(a_cstr_format("SkookumIDE: %s  [%u matches]", title_cstr_p, m_match_list.get_length()));
    }
  }

//---------------------------------------------------------------------------------------
// Loads class hierarchy settings and displays dialog.
// Author(s):   Conan Reis
void SkSearchDialog::display(
  eSkMatchKind            kind,        // = SkMatchKind_all
  const AString &         match_text,  // = AString::ms_empty
  const SkMatchCriteria * match_info_p // = nullptr
  )
  {
  if (match_text.is_filled())
    {
    m_search_text.enable_control_event(false);
    m_search_text.set_text(match_text);
    m_search_text.syntax_highlight();
    m_search_text.select_all();
    m_search_text.enable_control_event();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load existing settings
  load_settings(kind);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find Matches
  find_matches(match_info_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Auto-pick if only 1 match on first display
  if (match_info_p && (m_matches.get_count() == 1u))
    {
    on_ok();

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Bring window up using previous settings if available.
  //ini_restore_view(g_ini_section_browser_p, ARegion(34, 34, 1240, 1000), AShowState_normal_focus, false, &ide_ini);
  show();

  make_foreground();
  m_search_text.set_focus();
  m_search_text.select_all();
  }

//---------------------------------------------------------------------------------------
// Find matches in a data table
void SkSearchDialog::find_data_matches(const tSkTypedNameArray * data_table_p, SkClass * class_p, SkMatchCriteria::eExpected expected_members, SkMatchCriteria &criteria, SkContextInfo &info)
  {
  uint32_t data_count = data_table_p->get_length();
  if (data_count)
    {
    SkTypedName * data_p;

    if (expected_members == SkMatchCriteria::Expected_one)
      {
      data_p = data_table_p->get(ASymbol::create(criteria.m_member_match_str));

      if (data_p)
        {
        info.m_type = SkMember_data;
        info.m_member_id.set_name(data_p->get_name());
        info.m_member_id.set_scope(class_p);
        m_match_list.append_member(info);
        }
      }
    else
      {
      SkTypedName ** data_pp = data_table_p->get_array();
      SkTypedName ** data_end_pp = data_pp + data_count;

      while (data_pp < data_end_pp)
        {
        data_p = *data_pp;

        if (criteria.is_member_match(*data_p))
          {
          info.m_type = SkMember_data;
          info.m_member_id.set_name(data_p->get_name());
          info.m_member_id.set_scope(class_p);
          m_match_list.append_member(info);
          }

        data_pp++;
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Parses out a part (class or member identifier) from a match string.
// Author(s):   Conan Reis
eAStrMatch SkSearchDialog::parse_match_part(
  const AString & match_str,
  uint32_t        start_idx,
  uint32_t *          end_idx_p,
  AString *       match_part_p
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for empty string
  uint32_t str_length = match_str.get_length();

  if (start_idx >= str_length)
    {
    *end_idx_p = start_idx;
    match_part_p->empty();

    return AStrMatch_subpart;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find start type if any
  eAStrMatch   match_type   = AStrMatch_subpart;
  const char * match_cstr_p = match_str.as_cstr();

  switch (match_cstr_p[start_idx])
    {
    case '*':
      match_type = AStrMatch_suffix;
      start_idx++;
      break;

    case '"':
      match_type = AStrMatch_exact;
      start_idx++;
      break;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Store part match string
  uint32_t find_idx  = str_length;
  uint32_t ident_idx = (match_cstr_p[start_idx] != '!') ? start_idx : (start_idx + 1u);  // Skip !

  if (ident_idx < find_idx)
    {
    match_str.find(ACharMatch_not_identifier, 1u, &find_idx, ident_idx);
    }

  match_str.get(match_part_p, start_idx, find_idx - start_idx);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find end type if any
  switch (match_type)
    {
    case AStrMatch_subpart:
      if (match_cstr_p[find_idx] == '*')
        {
        match_type = AStrMatch_prefix;
        find_idx++;
        }
      break;

    case AStrMatch_suffix:
      if (match_cstr_p[find_idx] == '*')
        {
        // Two asterisks = search for sub-string
        match_type = AStrMatch_subpart;
        find_idx++;
        }
      break;

    case AStrMatch_exact:
      if (match_cstr_p[find_idx] == '"')
        {
        // Skip ending double quote "
        match_type = AStrMatch_exact;
        find_idx++;
        }
      break;
    }

  *end_idx_p = find_idx;

  return match_type;
  }

//---------------------------------------------------------------------------------------
// Analyzes match text string for match criteria
// Returns:    true if there might be a match false if match_text is empty
// Arg         match_text - match string to parse
// Arg         criteria_p - match criteria determined via parse
// Author(s):   Conan Reis
bool SkSearchDialog::parse_match_text(const AString & match_text, SkMatchCriteria * criteria_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove leading and trailing whitespace.
  AString match_str(match_text);

  match_str.crop();

  if (match_str.is_empty())
    {
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine first part of match criteria
  AString    match_part;
  uint32_t   start_idx  = 0u;
  uint32_t   end_idx    = 0u;
  eAStrMatch match_type = parse_match_part(match_str, start_idx, &end_idx, &match_part);

  if (m_kind == SkMatchKind_classes)
    {
    // Only classes are being matched
    criteria_p->m_class_match_str  = match_part;
    criteria_p->m_class_match_type = match_type;

    return true;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if first part of match criteria is for classes or for members
  char delimiter_ch = match_str.as_cstr()[end_idx];
  bool class_part   = false;

  if (AString::ms_is_uppercase[uint8_t(match_part.get_first())])
    {
    class_part = true;
    }

  if ((delimiter_ch == '.') || (delimiter_ch == '@'))
    {
    end_idx++;
    class_part = true;
    }
  
  if (!class_part)
    {
    // Only members are being matched
    criteria_p->m_class_match_str.empty();
    criteria_p->m_class_match_type = AStrMatch_subpart;

    criteria_p->m_member_match_str  = match_part;
    criteria_p->m_member_match_type = match_type;

    return true;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Both classes and members are being matched - determine second part (member) of match criteria
  criteria_p->m_class_match_str  = match_part;
  criteria_p->m_class_match_type = match_type;

  match_type = parse_match_part(match_str, end_idx, &end_idx, &match_part);

  criteria_p->m_member_match_str  = match_part;
  criteria_p->m_member_match_type = match_type;

  return true;
  }

//---------------------------------------------------------------------------------------
// Finds matches based on match kind, search text and filters and populates
//             match list.
// Author(s):   Conan Reis
void SkSearchDialog::find_matches(
  const SkMatchCriteria * match_info_p // = nullptr
  )
  {
  set_title_kind(true);

  // Stop any countdown.
  m_timer.enable_idle_processing(false);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - CReis Only clear list if changes warrant it.

  m_match_list.remove_all();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Analyze the match text
  SkMatchCriteria criteria;
  bool match_expected;

  if (match_info_p)
    {
    criteria = *match_info_p;
    match_expected = criteria.get_match_expected() != SkMatchCriteria::Expected_none;
    }
  else
    {
    match_expected = parse_match_text(m_search_text.get_text(), &criteria);
    }

  if (!match_expected)
    {
    // No match text
    set_title_kind();
    return;
    }

  SkMatchCriteria::eExpected expected_classes = criteria.get_class_expected();

  if (expected_classes == SkMatchCriteria::Expected_none)
    {
    // No classes expected to match
    set_title_kind();
    return;
    }

  bool match_classes = (m_kind & SkMatchKind_classes) && m_toggle_classes.is_toggled();

  SkMatchCriteria::eExpected expected_members = criteria.get_member_expected();

  if ((expected_members == SkMatchCriteria::Expected_none) && !match_classes)
    {
    // No members expected to match
    set_title_kind();
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine classes to search

  SkClass *          class_p;
  SkClass **         classes_pp;
  SkClass **         classes_end_pp;
  const tSkClasses & classes     = SkBrain::get_classes();
  uint32_t           class_count = classes.get_length();

  // Note - uses local stack memory for array buffer.
  const SkClass ** search_classes_pp = a_stack_allocate(class_count, const SkClass *);
  tSkClasses       search_classes(search_classes_pp, 0u, class_count, true);

  switch (expected_classes)
    {
    case SkMatchCriteria::Expected_one:
      class_p = classes.get(ASymbol::create(criteria.m_class_match_str));

      if (class_p)
        {
        search_classes.append(*class_p);
        }
      break;

    case SkMatchCriteria::Expected_some:
      {
      // Search for sub-strings
      classes_pp     = classes.get_array();
      classes_end_pp = classes_pp + class_count;

      while (classes_pp < classes_end_pp)
        {
        class_p = *classes_pp;

        if (criteria.is_class_match(*class_p))
          {
          search_classes.append(*class_p);
          }

        classes_pp++;
        }
      break;
      }

    case SkMatchCriteria::Expected_all:
      search_classes = classes;
      break;
    }

  uint32_t search_class_count = search_classes.get_length();

  if (search_class_count == 0u)
    {
    // No matches if no search classes matched
    // Ensure that local stack memory is not freed.
    search_classes.empty_null_buffer();
    set_title_kind();
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine related classes to search
  if (search_class_count < class_count)
    {
    bool search_supers = m_toggle_superclasses.is_toggled();
    bool search_subs   = m_toggle_subclasses.is_toggled();

    if (search_supers || search_subs)
      {
      // Note - uses local stack memory for array buffer.
      SkClass ** original_classes_pp = a_stack_allocate(search_class_count, SkClass *);
      tSkClasses original_classes((const SkClass **)original_classes_pp, 0u, search_class_count, true);

      original_classes = search_classes;
      classes_end_pp   = original_classes_pp + search_class_count;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Add all ancestors
      if (search_supers)
        {
        classes_pp = original_classes_pp;

        while (classes_pp < classes_end_pp)
          {
          (*classes_pp)->get_superclasses_all(&search_classes);
          classes_pp++;
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Add all descendants
      if (search_subs)
        {
        classes_pp = original_classes_pp;

        while (classes_pp < classes_end_pp)
          {
          (*classes_pp)->get_subclasses_all(&search_classes);
          classes_pp++;
          }
        }

      search_class_count = search_classes.get_length();

      // Ensure that local stack memory is not freed.
      original_classes.empty_null_buffer();
      }
    }


  SkContextInfo info;

  classes_pp     = search_classes.get_array();
  classes_end_pp = classes_pp + search_class_count;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine matching classes
  if (match_classes)
    {
    info.m_type        = SkMember_class_meta;
    info.m_class_scope = true;

    while (classes_pp < classes_end_pp)
      {
      class_p = *classes_pp;

      info.m_member_id.set_name(class_p->get_name());
      info.m_member_id.set_scope(class_p);
      m_match_list.append_member(info);

      classes_pp++;
      }
    }


  if (expected_members != SkMatchCriteria::Expected_none)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine matching routine members
    if (m_kind & SkMatchKind_routines)
      {
      bool search_scripts    = m_toggle_script.is_toggled();
      bool search_cpp        = m_toggle_cpp.is_toggled();
      bool filter_script_cpp = search_scripts != search_cpp;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Search for coroutines
      if (m_toggle_coroutines.is_toggled())
        {
        info.m_class_scope = false;

        classes_pp = search_classes.get_array();

        while (classes_pp < classes_end_pp)
          {
          class_p = *classes_pp;

          const tSkCoroutines & coroutines = class_p->get_coroutines();
          uint32_t coro_count = coroutines.get_length();

          if (coro_count)
            {
            SkCoroutineBase * coro_p;

            if (expected_members == SkMatchCriteria::Expected_one)
              {
              coro_p = coroutines.get(ASymbol::create(criteria.m_member_match_str));

              if (coro_p)
                {
                info.m_type = coro_p->get_member_type();

                if (!filter_script_cpp || (search_scripts == (info.m_type == SkMember_coroutine)))
                  {
                  info.m_member_id = *coro_p;
                  m_match_list.append_member(info);
                  }
                }
              }
            else
              {
              SkCoroutineBase ** coro_pp     = coroutines.get_array();
              SkCoroutineBase ** coro_end_pp = coro_pp + coro_count;

              while (coro_pp < coro_end_pp)
                {
                coro_p      = *coro_pp;
                info.m_type = coro_p->get_member_type();

                if ((!filter_script_cpp || (search_scripts == (info.m_type == SkMember_coroutine)))
                  && criteria.is_member_match(*coro_p))
                  {
                  info.m_member_id = *coro_p;
                  m_match_list.append_member(info);
                  }

                coro_pp++;
                }
              }
            }

          classes_pp++;
          }
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Search for instance methods
      if (m_toggle_methods_instance.is_toggled())
        {
        info.m_class_scope = false;

        classes_pp = search_classes.get_array();

        while (classes_pp < classes_end_pp)
          {
          class_p = *classes_pp;

          const tSkMethodTable * methods_p    = &class_p->get_instance_methods();
          uint32_t               method_count = methods_p->get_length();

          if (method_count)
            {
            SkMethodBase * method_p;

            if (expected_members == SkMatchCriteria::Expected_one)
              {
              method_p = methods_p->get(ASymbol::create(criteria.m_member_match_str));

              if (method_p)
                {
                info.m_type = method_p->get_member_type();

                if (!filter_script_cpp || (search_scripts == (info.m_type == SkMember_method)))
                  {
                  info.m_member_id = *method_p;
                  m_match_list.append_member(info);
                  }
                }
              }
            else
              {
              SkMethodBase ** method_pp     = methods_p->get_array();
              SkMethodBase ** method_end_pp = method_pp + method_count;

              while (method_pp < method_end_pp)
                {
                method_p      = *method_pp;
                info.m_type = method_p->get_member_type();

                if ((!filter_script_cpp || (search_scripts == (info.m_type == SkMember_method)))
                  && criteria.is_member_match(*method_p))
                  {
                  info.m_member_id = *method_p;
                  m_match_list.append_member(info);
                  }

                method_pp++;
                }
              }
            }

          classes_pp++;
          }
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Search for class methods
      if (m_toggle_methods_class.is_toggled())
        {
        info.m_class_scope = true;

        classes_pp = search_classes.get_array();

        while (classes_pp < classes_end_pp)
          {
          class_p = *classes_pp;

          const tSkMethodTable * methods_p    = &class_p->get_class_methods();
          uint32_t               method_count = methods_p->get_length();

          if (method_count)
            {
            SkMethodBase * method_p;

            if (expected_members == SkMatchCriteria::Expected_one)
              {
              method_p = methods_p->get(ASymbol::create(criteria.m_member_match_str));

              if (method_p)
                {
                info.m_type = method_p->get_member_type();

                if (!filter_script_cpp || (search_scripts == (info.m_type == SkMember_method)))
                  {
                  info.m_member_id = *method_p;
                  m_match_list.append_member(info);
                  }
                }
              }
            else
              {
              SkMethodBase ** method_pp     = methods_p->get_array();
              SkMethodBase ** method_end_pp = method_pp + method_count;

              while (method_pp < method_end_pp)
                {
                method_p      = *method_pp;
                info.m_type = method_p->get_member_type();

                if ((!filter_script_cpp || (search_scripts == (info.m_type == SkMember_method)))
                  && criteria.is_member_match(*method_p))
                  {
                  info.m_member_id = *method_p;
                  m_match_list.append_member(info);
                  }

                method_pp++;
                }
              }
            }

          classes_pp++;
          }
        }
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine matching data members
    if (m_kind & SkMatchKind_data)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Search for instance data
      if (m_toggle_data_instance.is_toggled())
        {
        info.m_class_scope = false;

        classes_pp = search_classes.get_array();

        while (classes_pp < classes_end_pp)
          {
          class_p = *classes_pp++;
          find_data_matches(&class_p->get_instance_data(), class_p, expected_members, criteria, info);
          // Note HACK this relies on the fact that SkTypedNameRaw is derived from SkTypedName
          find_data_matches(reinterpret_cast<const tSkTypedNameArray *>(&class_p->get_instance_data_raw()), class_p, expected_members, criteria, info);
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Search for class data
      if (m_toggle_data_class.is_toggled())
        {
        info.m_class_scope = true;

        classes_pp = search_classes.get_array();

        while (classes_pp < classes_end_pp)
          {
          class_p = *classes_pp++;
          find_data_matches(&class_p->get_class_data(), class_p, expected_members, criteria, info);
          }
        }
      }
    }

  // Ensure that local stack memory is not freed.
  search_classes.empty_null_buffer();


  uint32_t match_count = m_match_list.get_length();

  if (match_count)
    {
    // Default sort by type and name
    m_match_list.m_col_name_p->sort();

    m_match_list.select_row(0u);
    }

  m_match_list.columns_set_width();
  set_title_kind();
  }

//---------------------------------------------------------------------------------------
// Calls find_matches() after an elapsed time - allows a few keys to be 
//             pressed before doing a search so the first letter doesn't necessarily have
//             a big stall.
// Author(s):   Conan Reis
void SkSearchDialog::find_matches_countdown()
  {
  m_countdown_start = ATimer::get_elapsed_ms();
  m_timer.enable_idle_processing();
  }

//---------------------------------------------------------------------------------------
// Called whenever any filter toggle changes state
// Author(s):   Conan Reis
void SkSearchDialog::on_toggle_filter(eAFlag new_state)
  {
  find_matches_countdown();
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
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
bool SkSearchDialog::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  if (!repeated)
    {
    eAKeyMod mod_keys = AKeyboard::get_mod_keys();

    switch (key)
      {
      case AKey_escape:
        if (mod_keys == AKeyMod_none)
          {
          close_default();

          return false;
          }
        break;

      case AKey_return:
      case AKey_num_enter:
        if (mod_keys == AKeyMod_none)
          {
          on_ok();

          return false;
          }
        break;

      case 'E':
        switch (mod_keys)
          {
          case AKeyMod_ctrl:
            {
            SkMatchReference * match_p = m_match_list.get_selected_first();

            if (match_p)
              {
              match_p->action_edit_external();
              }

            return false;
            }

          case AKeyMod_ctrl_shift:
            {
            SkMatchReference * match_p = m_match_list.get_selected_first();

            if (match_p)
              {
              match_p->action_goto_file_explorer();
              }

            return false;
            }
          }
        break;

      case 'P':
        if ((mod_keys == AKeyMod_alt) 
          && (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4))
          {
          SkMatchReference * match_p = m_match_list.get_selected_first();

          if (match_p)
            {
            match_p->action_p4_checkout();
            }

          return false;
          }
        break;
      }
    }

  return SkConsole::ms_console_p->on_key_press(key, repeated);
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Overridden from AgogGUI\AWindow.
// Author(s):   Conan Reis
bool SkSearchDialog::on_draw()
  {
  PAINTSTRUCT ps;
  HDC         hdc     = ::BeginPaint(m_os_handle, &ps);

  // Set header1 font
  ::SelectObject(hdc, ((ATrueTypeFont *)AFont::ms_header1_p->m_sys_font_p)->m_font_handle_p);

  int spacing = get_spacing();

  ::ExtTextOut(hdc, spacing, m_search_text_y, 0u, nullptr, g_search_prompt.as_cstr(), g_search_prompt.get_length(), nullptr);

  RECT rect;
  
  rect = m_rgn_class;
  ::DrawEdge(hdc, &rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, rect.left + spacing, rect.top - spacing, 0u, nullptr, "Class/Scope", 11, nullptr);

  if (m_kind & SkMatchKind_routines)
    {
    rect = m_rgn_routine;
    ::DrawEdge(hdc, &rect, EDGE_ETCHED, BF_RECT);
    ::ExtTextOut(hdc, rect.left + spacing, rect.top - spacing, 0u, nullptr, "Routines", 8, nullptr);
    }

  if (m_kind & SkMatchKind_data)
    {
    rect = m_rgn_data;
    ::DrawEdge(hdc, &rect, EDGE_ETCHED, BF_RECT);
    ::ExtTextOut(hdc, rect.left + spacing, rect.top - spacing, 0u, nullptr, "Data", 4, nullptr);
    }

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is gained.
// Returns:    In non-OS windows returning true indicates that changing the focus to this
//             window is allowed.  In OS windows (AEditOS, AListOS, ATreeOS, etc.) the
//             return value is ignored.
// See:        on_focus_lost()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkSearchDialog::on_focus()
  {
  ADialogOS::set_common_parent(this);

  return true;  // Allow focus
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
void SkSearchDialog::on_sizing()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup OK & Cancel buttons [UI built from bottom to top]
  AVec2i cancel_area(m_cancel_btn.get_area());
  AVec2i client_area(get_area_client());
  int    spacing  = get_spacing();
  int    spacing2 = spacing * 2;
  int    x        = client_area.m_x - cancel_area.m_x - spacing;
  int    y        = client_area.m_y - cancel_area.m_y - spacing;

  m_cancel_btn.set_position(x, y);
  x -= cancel_area.m_x + spacing;
  m_ok_btn.set_position(x, y);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Search Text
  int old_x = x;
  int search_height      = m_search_text.get_height();
  int search_y_diff_half = a_round((search_height - m_search_text.get_font().get_height()) * 0.5f);

  x = spacing2 + AFont::ms_header1_p->get_width(g_search_prompt);
  m_search_text_y = y + a_round(search_height * 0.5f) - spacing;

  m_search_text.set_region(x, m_search_text_y + search_y_diff_half, old_x - spacing - x, search_height);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Filters

  // Class Filters
  x = spacing;
  y = client_area.m_y - cancel_area.m_y - (spacing * 3) - ((m_kind & SkMatchKind_routines)
    ? m_rgn_routine.m_height
    : m_rgn_class.m_height);

  m_rgn_class.m_x = x;
  m_rgn_class.m_y = y;

  x += spacing;
  y += spacing2;

  if (m_kind == SkMatchKind_all)
    {
    m_toggle_classes.set_position(x, y);
    y += spacing * 3;
    }

  m_toggle_superclasses.set_position(x, y);
  y += spacing * 3;
  m_toggle_subclasses.set_position(x, y);
  x += m_rgn_class.m_width;

  // Invokable Filters
  if (m_kind & SkMatchKind_routines)
    {
    y = client_area.m_y - cancel_area.m_y - m_rgn_routine.m_height - (spacing * 3);

    m_rgn_routine.m_x = x;
    m_rgn_routine.m_y = y;

    x += spacing;
    y += spacing2;

    m_toggle_coroutines.set_position(x, y);
    y += spacing * 3;
    m_toggle_methods_instance.set_position(x, y);
    y += spacing * 3;
    m_toggle_methods_class.set_position(x, y);

    x += m_toggle_methods_instance.get_width() + spacing;
    y  = m_rgn_routine.m_y + spacing;
    m_toggle_script.set_position(x, y);
    y += spacing * 3;
    m_toggle_cpp.set_position(x, y);
    x = m_rgn_routine.m_x + m_rgn_routine.m_width + spacing;
    }

  // Data Filters
  if (m_kind & SkMatchKind_data)
    {
    y = m_rgn_class.m_y;

    m_rgn_data.m_x = x;
    m_rgn_data.m_y = y;

    x += spacing;
    y += spacing2;

    m_toggle_data_instance.set_position(x, y);
    y += spacing * 3;
    m_toggle_data_class.set_position(x, y);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Match List
  m_match_list.set_area(client_area.m_x - spacing2, m_rgn_class.m_y - spacing2);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that controls are redrawn
  refresh();
  }

//---------------------------------------------------------------------------------------
// Called when "OK" button is pressed
// Author(s):   Conan Reis
void SkSearchDialog::on_ok()
  {
  if (m_timer.is_idle_processing())
    {
    // If waiting to find matched stop waiting and find immediately
    find_matches();
    }

  SkMatchReference * match_p = m_match_list.get_selected_first();

  if (match_p)
    {
    match_p->action_goto_browser();
    }

  hide();
  }

//---------------------------------------------------------------------------------------
// Called whenever the search field text changes
// 
// #Author(s) Conan Reis
void SkSearchDialog::on_search_text_modified()
  {
  m_search_text.syntax_highlight();
  find_matches_countdown();
  }

//---------------------------------------------------------------------------------------
// Called every idle loop - i.e. whenever the CPU isn't doing anything.
// Notes:      When this is running, CPU usage will be at 100% since all idle cycles will
//             be used.
// Author(s):   Conan Reis
void SkSearchDialog::on_idle()
  {
  // Determine amount of time to wait
  uint32_t wait_ms = 400;

  switch (m_search_text.get_length())
    {
    case 1: wait_ms = 1000; break;
    case 2: wait_ms =  800; break;
    case 3: wait_ms =  600; break;
    }

  if ((ATimer::get_elapsed_ms() - m_countdown_start) > wait_ms)
    {
    find_matches();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkMatchReference * SkSearchDialog::pool_new_info(const SkContextInfo & member_info)
  {
  SkMatchReference * info_p = SkMatchReference::pool_new(member_info);
  
  m_matches.append(info_p);
  
  return info_p;
  }


