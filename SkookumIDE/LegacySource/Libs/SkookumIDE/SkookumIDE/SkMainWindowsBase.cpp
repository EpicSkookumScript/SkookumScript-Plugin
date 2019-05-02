// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Console (Workbench window) & supporting classes
// # Notes:        
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumIDE/SkConsole.hpp"
#include "SkookumIDE/SkClassBrowser.hpp"
#include "SkookumIDE/SkMainWindowBase.hpp"
#include "AgogIO/AClipboard.hpp"
#include <shellapi.h>  // Uses ShellExecute()

//=======================================================================================
// SkMainWindowBase Class Data
//=======================================================================================

SkEditSyntax *      SkMainWindowBase::ms_focus_p              = nullptr;
AWindow::eFocusType SkMainWindowBase::ms_focus_type           = FocusType_unknown;

SkEditSyntax *      SkMainWindowBase::ms_focused_console_p    = nullptr;    //////
SkEditSyntax *      SkMainWindowBase::ms_focused_browser_p    = nullptr;    //////  
SkEditSyntax *      SkMainWindowBase::ms_focused_last_p       = nullptr;    //////
AWindow::eFocusType SkMainWindowBase::ms_focused_console_type = FocusType_unknown;
AWindow::eFocusType SkMainWindowBase::ms_focused_browser_type = FocusType_unknown;
AWindow::eFocusType SkMainWindowBase::ms_focused_last_type    = FocusType_unknown;


//=======================================================================================
//  File menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard File Menu items whose state will be updated before being displayed.
//   
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_file(AMenuOS menu)
  {
  menu.append_item("Load project...",                      SkMenu_file_load_project);
  menu.append_item("Load default project",                 SkMenu_file_load_project_default);
  menu.append_separator();
  menu.append_item("&New Class / Member...\tCtrl+N",       SkMenu_file_new);
  menu.append_separator();
  menu.append_item("&Save Files\tCtrl+S",                  SkMenu_file_save);
  menu.append_item("&Checkout File\tAlt+P",                SkMenu_file_checkout);
  menu.append_separator();
  menu.append_item("Open in External &Editor\tCtrl+E",     SkMenu_file_open_external_editor);
  menu.append_item("Open in &File Explorer\tCtrl+Shift+E", SkMenu_file_open_explorer);
  menu.append_item("Copy File &Pathname",                  SkMenu_file_copy_pathname);
  menu.append_separator();
  menu.append_item("Close &Browser\tCtrl+F4",              SkMenu_file_open_close_browser);
  menu.append_item("E&xit IDE\tAlt+F4",                    SkMenu_file_exit_console);
  } //  SkMainWindowBase::append_menubar_file()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receiving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Author(s): John Stenersen
void SkMainWindowBase::refresh_menubar_file(AMenuOS menu)
  {
  SkClassBrowser * browser_p = SkClassBrowser::ms_browser_p;
  SkConsole      * console_p = SkConsole::ms_console_p;

  bool existing     = false;
  AString filename  = "File";
  if (browser_p)
    {
    const AFile & source_file = browser_p->get_edit_view().get_source_file();
    existing = source_file.is_titled();
    if (existing)
      {
      filename = "'" + source_file.get_name() + "'";
      }
    }

  if (m_type == MainWindowBaseType_console)
    {
    switch (ms_focus_type & ~FocusType_isearch)
      {
      case FocusType_workspace:
      case FocusType_log:
        menu.set_item_text(SkMenu_file_save,                 "&Save Workbench File\tCtrl+S");
        menu.set_item_text(SkMenu_file_checkout,             "&Checkout " + filename + "\tAlt+P");
        menu.set_item_text(SkMenu_file_open_external_editor, "Open Workbench in &External Editor\tCtrl+S");
        menu.set_item_text(SkMenu_file_open_explorer,        "Open Workbench in &File Explorer\tCtrl+Shift+E");
        menu.set_item_text(SkMenu_file_copy_pathname,        "Copy Workbench &Pathname");
        existing = true;
        break;

      default:
        menu.set_item_text(SkMenu_file_save,                 "&Save Files\tCtrl+S");
        menu.set_item_text(SkMenu_file_checkout,             "&Checkout File\tAlt+P");
        menu.set_item_text(SkMenu_file_open_external_editor, "Open in External &Editor\tCtrl+E");
        menu.set_item_text(SkMenu_file_open_explorer,        "Open in &File Explorer\tCtrl+Shift+E");
        menu.set_item_text(SkMenu_file_copy_pathname,        "Copy File &Pathname");
      }
    }
  else  //  MainWindowBaseType_browser
    {
    menu.set_item_text(SkMenu_file_save,                 "&Save " + filename + "\tCtrl+S");
    menu.set_item_text(SkMenu_file_checkout,             "&Checkout " + filename + "\tAlt+P");
    menu.set_item_text(SkMenu_file_open_external_editor, "Open " + filename + " in &External Editor\tCtrl+E");
    menu.set_item_text(SkMenu_file_open_explorer,        "Open " + filename + " in &File Explorer\tCtrl+Shift+E");
    menu.set_item_text(SkMenu_file_copy_pathname,        "Copy " + filename + " &Pathname");
    }

  menu.enable_item(SkMenu_file_checkout,             existing && (console_p->get_version_control_system() == SkVersionControl_p4));
  menu.enable_item(SkMenu_file_open_external_editor, existing);
  menu.enable_item(SkMenu_file_open_explorer,        existing);
  menu.enable_item(SkMenu_file_copy_pathname,        existing);

  if (browser_p && !browser_p->is_hidden())
    {
    menu.set_item_text(SkMenu_file_open_close_browser, "Close &Browser\tCtrl+F4");
    }
  else
    {
    menu.set_item_text(SkMenu_file_open_close_browser, "Open &Browser\tCtrl+Shift+Tab");
    }
  } //  SkMainWindowBase::refresh_menubar_file()


//---------------------------------------------------------------------------------------
//  Processes menu items from the File menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_file(uint32_t item_id)
  {
  SkClassBrowser  * browser_p = SkClassBrowser::ms_browser_p;
  SkConsole       * console_p = SkConsole::ms_console_p;

  //  Process File menu item selections common to both the console and the browser.
  switch (eSkMenu(item_id))
    {
    case SkMenu_file_load_project:
      console_p->load_project_query();
      return true;

    case SkMenu_file_load_project_default:
      console_p->load_project_default_query();
      return true;

    case SkMenu_file_new:
      if (!browser_p)
        {
          browser_p = console_p->display_browser();
        }
      if (browser_p)
        {
        browser_p->show();
        browser_p->m_navigation_view.show_create_popup();
        }
      return true;

    case SkMenu_file_open_close_browser:
      if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        browser_p->show();
        browser_p->make_foreground();
        }
      else
        {
        browser_p->on_close_attempt();
        }
      return true;

    case SkMenu_file_exit_console:
      console_p->on_close_attempt();
      return true;
    }
    
    //  Process File menu items selections of the console.
    if (m_type == MainWindowBaseType_console)
      {
      SkWorkspace * workspace_p = &console_p->get_workspace();

      switch (eSkMenu(item_id))
        {
        case SkMenu_file_save:
          workspace_p->save_settings();
          return true;

        case SkMenu_file_open_external_editor:
          workspace_p->action_edit_externally();
          return true;

        case SkMenu_file_open_explorer:
          workspace_p->action_goto_file_explorer();
          return true;

        case SkMenu_file_copy_pathname:
          workspace_p->action_copy_path();
          return true;
        }
      }

    //  Process File menu items selections of the console.
    else  //  MenuWindowBaseTypes_browser
      {
      const SkContextInfo & context = browser_p->get_member_info();

      switch (eSkMenu(item_id))
        {
        case SkMenu_file_save:
          browser_p->m_edit_view.save_changes();
          return true;

        case SkMenu_file_checkout:
          context.action_p4_checkout();
          return true;

        case SkMenu_file_open_external_editor:
          context.action_edit_external();
          return true;

        case SkMenu_file_open_explorer:
          context.action_goto_file_explorer();
          return true;

        case SkMenu_file_copy_pathname:
          context.action_copy_path();
        }
      }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_file()


//=======================================================================================
//  Edit menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard list of Edit Menu items properly grayed based on the editbox context.
//
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_edit(AMenuOS menu, SkEditBox * editbox_p, eFocusType focus_type, bool context_menu)
  {
  bool read_only                  = false;
  bool allow_read_only_clear_all  = false;

  if (!(focus_type & FocusType_isearch))
    {
    switch (focus_type)
      {
      case FocusType_log:
        read_only                 = true;
        allow_read_only_clear_all = true;
        break;

      case FocusType_workspace:
        read_only  = editbox_p->is_read_only();
        break;

      case FocusType_editor:
        read_only  = editbox_p->is_read_only();
        break;

      case FocusType_data_list:
      case FocusType_code_list:
      case FocusType_class_tree:
        editbox_p = &SkClassBrowser::ms_browser_p->get_edit_view().get_editor();
        read_only = editbox_p->is_read_only();
        break;

      case FocusType_editsyntax:
        break;

      default:
        A_DPRINT(A_SOURCE_STR "Unexpected focus_type = %ld\n", focus_type);
        return;
      }
    }
  else  //  Incremental search focuses
    {
    switch (focus_type & ~FocusType_isearch)
      {
      case FocusType_log:
        if (!context_menu)
          {
          editbox_p                 = &SkConsole::ms_console_p->get_log();
          read_only                 = true;
          allow_read_only_clear_all = true;
          }
        break;

      case FocusType_workspace:
        if (!context_menu)
          {
          editbox_p = &SkConsole::ms_console_p->get_workspace();
          read_only = editbox_p->is_read_only();
          }
        break;

      case FocusType_editor:
        if (!context_menu)
          {
          editbox_p = &SkClassBrowser::ms_browser_p->get_edit_view().get_editor();
          read_only = editbox_p->is_read_only();
          }
        break;

      case FocusType_editsyntax:
        break;

      default:
        A_DPRINT(A_SOURCE_STR "Unexpected focus_type = %ld\n", focus_type);
        return;
      }
    }

  bool selected   = editbox_p->is_selected();
  bool empty      = editbox_p->get_length() <= 0;

  menu.append_item("&Undo\tCtrl+Z | Alt+BkSp",        SkMenu_edit_undo, editbox_p->is_undo());
  menu.append_item("&Redo\tCtrl+Y | Ctrl+Shift+Z",    SkMenu_edit_redo, editbox_p->is_redo() && !read_only);
  menu.append_separator();
  menu.append_item( selected || empty ? "Cu&t\tCtrl+X | Shift+Del" : "Cu&t Current Line\tCtrl+X | Shift+Del",  SkMenu_edit_cut,  !empty && !read_only);
  menu.append_item( selected || empty ? "&Copy\tCtrl+C | Ctrl+Ins" : "&Copy Current Line\tCtrl+C | Ctrl+Ins",  SkMenu_edit_copy, !empty);
  menu.append_item("Copy with &Formatting",           SkMenu_edit_copy_with_format, !empty);
  menu.append_item("Copy Result\tAlt+Ctrl+Ins",       SkMenu_edit_copy_result);
  menu.append_item("&Paste\tCtrl+V | Shift+Ins",      SkMenu_edit_paste, !read_only);
  menu.append_item("Paste Re&sult\tAlt+Ins",          SkMenu_edit_paste_result, !read_only);
  menu.append_item("&Delete Selection\tDel",          SkMenu_edit_delete, selected && !read_only);
  menu.append_item("Clear All",                       SkMenu_edit_clear_all, !empty && (!read_only || (read_only && allow_read_only_clear_all)));
  menu.append_item("Select &All\tCtrl+A",             SkMenu_edit_select_all, !empty && editbox_p->get_selection_length() <= editbox_p->get_length());

  //  Save the focus for use when processing the menu item.
  ms_focus_p      = reinterpret_cast<SkEditSyntax *>(editbox_p);
  ms_focus_type   = focus_type;

  } //  SkMainWindowBase::append_menubar_edit()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receieving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Author(s): John Stenersen
void SkMainWindowBase::refresh_menubar_edit(AMenuOS menu)
  {
  //  Remove all the previous edit entries.
  while( ::GetMenuItemCount(menu.get_handle()) > 0 )
    {
    ::DeleteMenu(menu.get_handle(), 0, MF_BYPOSITION);
    }

  //  Append new Edit items based on which selection, read-only state, etc.
  append_menubar_edit(menu, ms_focus_p, ms_focus_type, false);

  } //  SkMainWindowBase::refresh_menubar_edit()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Edit menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_edit(uint32_t item_id)
  {
  bool selected   = ms_focus_p->is_selected();
  bool read_only  = false;
  if (FocusType_file & ms_focus_type)
    {
    read_only  = ms_focus_p->is_read_only();
    }

  //  Process File menu item selections common to both the console and the browser.
  switch (eSkMenu(item_id))
    {
    case SkMenu_edit_undo:
      if (read_only)
        {
        //  Note: This may seem unusual. No other editing command other than "Clear All" can change a
        //        read-only editbox. So, for convenience, the "Clear All" can be undone restoring the
        //        editbox to its previous state, still read-only.
        ms_focus_p->enable_read_only(false);
        ms_focus_p->undo();
        ms_focus_p->enable_read_only(true);
        }
      else
        {
        ms_focus_p->undo();
        }
      return true;

    case SkMenu_edit_redo:
      ms_focus_p->redo();
      return true;

    case SkMenu_edit_cut:
      ms_focus_p->clipboard_copy_plain_sel_or_row();
      if (selected)
        {
        //  Cut the selection
        ms_focus_p->remove_selection(true);
        }
      else
        {
        //  Cut a single line
        ms_focus_p->remove_row(ms_focus_p->get_row_caret(), true);
        }
      return true;

    case SkMenu_edit_copy:
      ms_focus_p->clipboard_copy_plain_sel_or_row();
      return true;

    case SkMenu_edit_copy_with_format:
      ms_focus_p->clipboard_copy_selected();
      return true;

    case SkMenu_edit_paste:
      ms_focus_p->clipboard_paste_plain();
      return true;

    case SkMenu_edit_paste_result:
      ms_focus_p->replace_selection(ms_focus_p->get_result_string(), true);
      return true;

    case SkMenu_edit_delete:
      ms_focus_p->remove_selection(true);
      return true;

    case SkMenu_edit_copy_result:
      {
      AClipboard clip(SkConsole::ms_console_p);
      clip.set_text(ms_focus_p->get_result_string());
      return true;
      }

    case SkMenu_edit_clear_all:
      ms_focus_p->select_all();
      ms_focus_p->remove_selection(true);
      return true;

    case SkMenu_edit_select_all:
      ms_focus_p->select_all();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_edit()


//=======================================================================================
//  View menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard list of View Menu items properly grayed based on the editbox context.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_view(AMenuOS menu)
  {
  if (get_type() == MainWindowBaseType_browser)
    {
    menu.append_item("Switch to &Console...\tCtrl+Shift+Tab",   SkMenu_view_console);
    }
  else
    {
    menu.append_item("Switch to &Browser...\tCtrl+Shift+Tab",   SkMenu_view_browser);
    }
  menu.append_separator();
  menu.append_item("Toggle Disassembly\tCtrl+F11",  SkMenu_view_disassembly,            true, false);
  menu.append_item("Toggle Expression Guide",       SkMenu_view_expression_guide,       true, false);
  menu.append_item("Toggle Auto-Parse Editor",      SkMenu_view_auto_parse,             true, false);
  menu.append_item("Toggle Auto-Parse Selections",  SkMenu_view_auto_parse_sel,         true, false);
  menu.append_item("Toggle Auto-Complete",          SkMenu_view_auto_complete,          true, false);
  menu.append_item("Toggle Syntax Highlight",       SkMenu_view_syntax_highlight,       true, false);
  menu.append_item("Toggle Current Line Highlight", SkMenu_view_current_line_highlight, true, false);
  menu.append_separator();
  menu.append_item("&Swap Panes", SkMenu_view_swap);
  menu.set_item_bitmap(SkMenu_view_swap, HBMMENU_MBAR_RESTORE);
  menu.append_item("&Rotate Clockwise", SkMenu_view_rotate_cw);
  menu.append_item("Rotate &Counter-Clockwise", SkMenu_view_rotate_ccw);
  } //  SkMainWindowBase::append_menubar_view()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receiving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Author(s): John Stenersen
void SkMainWindowBase::refresh_menubar_view(AMenuOS menu)
  {
  SkConsole * console_p = SkConsole::ms_console_p;

  menu.check_item(SkMenu_view_disassembly,            console_p->is_disassembly());
  menu.check_item(SkMenu_view_expression_guide,       console_p->is_expression_guide());
  menu.check_item(SkMenu_view_auto_parse,             console_p->is_auto_parse());
  menu.check_item(SkMenu_view_auto_parse_sel,         console_p->is_auto_parse_sel());
  menu.check_item(SkMenu_view_auto_complete,          SkAutoComplete::is_active());
  menu.check_item(SkMenu_view_syntax_highlight,       console_p->is_syntax_highlight());
  menu.check_item(SkMenu_view_current_line_highlight, console_p->is_current_line_highlight());

  //  Find the appropriate splitter based on the last focus.
  menu.enable_item(SkMenu_view_swap,       m_focus_splitter_p && m_focus_splitter_p->get_user_reorient() != ASplitterOS::Reorient_disabled);
  menu.enable_item(SkMenu_view_rotate_cw,  m_focus_splitter_p && m_focus_splitter_p->get_user_reorient() >= ASplitterOS::Reorient_swap_rotate);
  menu.enable_item(SkMenu_view_rotate_ccw, m_focus_splitter_p && m_focus_splitter_p->get_user_reorient() >= ASplitterOS::Reorient_swap_rotate);

  } //  SkMainWindowBase::refresh_menubar_view()


//---------------------------------------------------------------------------------------
//  Processes menu items from the View menu.
// 
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_view(uint32_t item_id)
  {
  SkClassBrowser  * browser_p   = SkClassBrowser::ms_browser_p;
  SkConsole       * console_p   = SkConsole::ms_console_p;
  ASplitterOS     * splitter_p  = m_focus_splitter_p;

  switch (eSkMenu(item_id))
    {
    case SkMenu_view_browser:
      {
      if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        }
        
      if (browser_p)
        {
        browser_p->show();
        browser_p->make_foreground();
        ms_focused_browser_p->set_focus();
        }
      
      return true;
      }

    case SkMenu_view_console:
      {
      console_p->show();
      console_p->make_foreground();
      ms_focused_console_p->set_focus();
      return true;
      }

    case SkMenu_view_disassembly:
      if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        }
        
      if (browser_p)
        {
        browser_p->show();
        browser_p->make_foreground();
        ms_focused_browser_p->set_focus();
        console_p->toggle_disassembly();
        }
      return true;

    case SkMenu_view_expression_guide:
      if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        }
        
      if (browser_p)
        {
        browser_p->show();
        browser_p->make_foreground();
        ms_focused_browser_p->set_focus();
        console_p->toggle_expression_guide();
        }
      return true;

    case SkMenu_view_auto_parse:      //  Editor/browser only
      if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        }
        
      if (browser_p)
        {
        browser_p->show();
        browser_p->make_foreground();
        ms_focused_browser_p->set_focus();
        console_p->toggle_auto_parse();
        }
      return true;

    case SkMenu_view_auto_parse_sel:  //  Workspace/console only
      if (console_p->get_workspace().get_selection_length() > 0)
        {
        console_p->show();
        console_p->make_foreground();
        ms_focused_console_p->set_focus();
        }
      console_p->toggle_auto_parse_sel();
      return true;

    case SkMenu_view_auto_complete:
      SkAutoComplete::toggle_active();          //  Note: No need to explicit hide the auto-complete listbox because that's done when focus is lost to display the menu.
      return true;

    case SkMenu_view_syntax_highlight:
      console_p->toggle_syntax_highlight();
      return true;

    case SkMenu_view_current_line_highlight:    //  Workspace/editor/log
      console_p->toggle_current_line_highlight();
      return true;

    case SkMenu_view_swap:
      if (splitter_p)
        {
        splitter_p->swap_panes();
        }
      return true;

    case SkMenu_view_rotate_cw:
      if (splitter_p)
        {
        splitter_p->rotate_panes_cw();
        }
      return true;

    case SkMenu_view_rotate_ccw:
      if (splitter_p)
        {
        splitter_p->rotate_panes_ccw();
        }
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_view()


//=======================================================================================
//  Goto menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard Goto/Find Menu items whose state will be updated before being displayed.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_goto(AMenuOS menu)
  {
    menu.append_item("Goto Context...\tAlt+G | F12 | Ctrl+,",   SkMenu_goto_context);
    menu.append_item("Goto Class...\tAlt+C",                    SkMenu_goto_class);
    menu.append_item("Goto Routine...\tCtrl+Alt+G",             SkMenu_goto_routine);
    menu.append_item("Goto Data...\tShift+Alt+G",               SkMenu_goto_data);
    menu.append_item("Goto Focused Kind...\tCtrl+Shift+G",      SkMenu_goto_local);
    menu.append_item("Goto...\tCtrl+Shift+Alt+G",               SkMenu_goto_all);
    menu.append_separator();
    menu.append_item("Find\tCtrl+F | Ctrl+I",                   SkMenu_goto_find);
    menu.append_item("Find Next\tCtrl+F | F3",                  SkMenu_goto_find_next);
    menu.append_item("Find Previous\tCtrl+Shift+F | Shift+F3",  SkMenu_goto_find_prev);
    menu.append_separator();
    menu.append_item("History Next\tAlt+-> | MouseNext",        SkMenu_goto_history_next);
    menu.append_item("History Previous\tAlt+<- | MousePrev",    SkMenu_goto_history_prev);
  } //  SkMainWindowBase::append_menubar_goto()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receieving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Author(s): John Stenersen
void SkMainWindowBase::refresh_menubar_goto(AMenuOS menu)
  {
  SkClassBrowser * browser_p = SkClassBrowser::ms_browser_p;

  //  Update the History items.
  bool    enabled_prev = false;
  bool    enabled_next = false;
  SkEditView * edit_view_p;
  if (browser_p)
    {
    edit_view_p   = &browser_p->get_edit_view();
    enabled_next  = edit_view_p && edit_view_p->is_history_next();
    enabled_prev  = edit_view_p && edit_view_p->is_history_prev();
    }

  menu.enable_item(SkMenu_goto_history_next, enabled_next);
  menu.enable_item(SkMenu_goto_history_prev, enabled_prev);

  SkIncrementalSearchEditBox * search_p = nullptr;
  AString location;
  if (m_type == MainWindowBaseType_console)
    {
    if ((ms_focused_console_type & ~FocusType_isearch) == FocusType_log)
      {
      search_p = SkConsole::ms_console_p->get_log().get_incremental_search();
      location = " in IDE Log";
      }
    else
      {
      search_p = SkConsole::ms_console_p->get_workspace().get_incremental_search();
      location = " in IDE Workbench";
      }
    }
  else  //  MainWindowBaseType_browser
    {
    search_p = SkClassBrowser::ms_browser_p->get_edit_view().get_editor().get_incremental_search();
    AString filename  = " in IDE Editor";
    if (browser_p)
      {
      const AFile & source_file = browser_p->get_edit_view().get_source_file();
      if (source_file.is_titled())
        {
        location = " in " + source_file.get_name();
        }
      }
    }

  //  Update the Find items.
  menu.set_item_text(SkMenu_goto_find,      "Find" + location + "\tCtrl+F | Ctrl+I");
  menu.set_item_text(SkMenu_goto_find_next, "Find Next" + location + "\tCtrl+F | F3");
  menu.set_item_text(SkMenu_goto_find_prev, "Find Previous" + location +"\tCtrl+Shift+F | Shift+F3");

  menu.enable_item(SkMenu_goto_find,      search_p);
  menu.enable_item(SkMenu_goto_find_next, search_p && (search_p->get_match_count() > 0));
  menu.enable_item(SkMenu_goto_find_prev, search_p && (search_p->get_match_count() > 0));
  } //  SkMainWindowBase::refresh_menubar_goto()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Goto menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_goto(uint32_t item_id)
  {
  SkClassBrowser  * browser_p = SkClassBrowser::ms_browser_p;
  SkConsole       * console_p = SkConsole::ms_console_p;

  SkIncrementalSearchEditBox * search_p = nullptr;
  if (m_type == MainWindowBaseType_console)
    {
    if ((ms_focused_console_type & ~FocusType_isearch) == FocusType_log)
      {
      search_p = SkConsole::ms_console_p->get_log().get_incremental_search();
      }
    else
      {
      search_p = SkConsole::ms_console_p->get_workspace().get_incremental_search();
      }
    }
  else  //  MainWindowBaseType_browser
    {
    search_p = SkClassBrowser::ms_browser_p->get_edit_view().get_editor().get_incremental_search();
    }

  switch(eSkMenu(item_id))
    {
    case SkMenu_goto_history_next:
    case SkMenu_goto_history_prev:
        if (!browser_p || browser_p->is_hidden())
        {
        browser_p = console_p->display_browser();
        }
        
      if (browser_p)
        {
        browser_p->show();
        browser_p->make_foreground();
        browser_p->set_focus();
        }
    }

  switch (eSkMenu(item_id))
    {
    case SkMenu_goto_local:
      console_p->display_goto_context_focus();
      return true;

    case SkMenu_goto_context:
      console_p->display_goto_context_editor();
      return true;

    case SkMenu_goto_class:
      console_p->display_goto_dialog(SkMatchKind_classes);
      return true;

    case SkMenu_goto_routine:
      console_p->display_goto_dialog(SkMatchKind_routines);
      return true;

    case SkMenu_goto_data:
      console_p->display_goto_dialog(SkMatchKind_data);
      return true;

    case SkMenu_goto_all:
      console_p->display_goto_dialog(SkMatchKind_all);
      return true;

    case SkMenu_goto_find:
      if (search_p)
        {
        search_p->on_key_press_bidirectional(AKey_f, false, AKeyMod_ctrl, true);
        }
      return true;

    case SkMenu_goto_find_next:
      if (search_p)
        {
        search_p->on_key_press_bidirectional(AKey_f3, false, AKeyMod_none, true);
        }
      return true;

    case SkMenu_goto_find_prev:
      if (search_p)
        {
        search_p->on_key_press_bidirectional(AKey_f3, false, AKeyMod_shift, true);
        }
      return true;

    case SkMenu_goto_history_next:
      browser_p->get_edit_view().history_next();
      return true;

    case SkMenu_goto_history_prev:
      browser_p->get_edit_view().history_prev();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_goto()


//=======================================================================================
//  Compile menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard Compile Menu items whose state will be updated before being displayed.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_compile(AMenuOS menu)
  {
  menu.append_item("&Compile Project (if stale)\tF7",                 SkMenu_compile_project_stale);
  menu.append_item("Compile Project (force)\tCtrl+Alt+F7",            SkMenu_compile_project);
  //memu.append_item("&Incremental Recompile Project",                  SkMenu_compile_incremental, false);
  menu.append_item("Recompile Current Class",                         SkMenu_compile_class);
  menu.append_item("Recompile Current Class (&& subclasses)\tAlt+F7", SkMenu_compile_classes);
  menu.append_item("Recompile Current Routine\tCtrl+F7",              SkMenu_compile_member);
  menu.append_item("&Execute Workbench Snippet\tF4",                  SkMenu_compile_snippet);
  menu.append_item("&Execute Workbench Snippet (on IDE)\tShift+F4",   SkMenu_compile_snippet_ide);
  menu.append_separator();
  menu.append_item("Show Dialog on Recompile Errors",                 SkMenu_compile_error_dialog, true, SkConsole::ms_console_p->is_show_error_dialog());
  menu.append_item("Live Update Runtime with Changes",                SkMenu_compile_update_remote, true, (SkConsole::ms_console_p->get_prefernce_flags() & SkConsole::Preference_update_remote) != 0u);
  menu.append_item("Strict Compile",                                  SkMenu_compile_strict, true, SkParser::is_strict());
  } //  SkMainWindowBase::append_menubar_compile()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Compile menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_compile(uint32_t item_id)
  {
  SkConsole * console_p = SkConsole::ms_console_p;

  switch (eSkMenu(item_id))
    {
    case SkMenu_compile_project_stale:
      console_p->compile_project_stale();
      return true;

    case SkMenu_compile_project:
      console_p->compile_project();
      return true;

    case SkMenu_compile_member:
      console_p->compile_member_browser();
      return true;

    case SkMenu_compile_class:
      console_p->compile_class_browser(false);
      return true;

    case SkMenu_compile_classes:
      console_p->compile_class_browser(true);
      return true;

    case SkMenu_compile_snippet:
      console_p->get_workspace().action_evaluate_selected();
      return true;

    case SkMenu_compile_snippet_ide:
      console_p->get_workspace().action_evaluate_selected(SkLocale_ide);
      return true;

    case SkMenu_compile_error_dialog:
      console_p->enable_error_dialog(!console_p->is_show_error_dialog());
      return true;

    case SkMenu_compile_update_remote:
      console_p->enable_remote_update(!console_p->is_remote_update_enabled());
      return true;

    case SkMenu_compile_strict:
      console_p->enable_strict_compile(!SkParser::is_strict());
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_compile()


//=======================================================================================
//  Debug menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard Debug Menu items whose state will be updated before being displayed.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_debug(AMenuOS menu)
  {
  //menu.append_item("Break all\tShift+F10",                  SkMenu_debug_break_all, false);
  menu.append_item("Continue\tF5",                            SkMenu_debug_continue, false);
  menu.append_item("Step Next (unlocked)\tShift+F10",         SkMenu_debug_step_next, false);
  menu.append_item("Step Into (thread)\tF11",                 SkMenu_debug_step_over, false);
  menu.append_item("Step Over (thread)\tF10",                 SkMenu_debug_step_into, false);
  menu.append_item("Step Out (thread)\tShift+F11",            SkMenu_debug_step_out, false);
  //menu.append_item("Run to Cursor\tCtrl+F10",                 SkMenu_debug_run_to, false);
  menu.append_separator();
  menu.append_item("Show Next Statement\tAlt+Num*",           SkMenu_debug_show_next_expr, false);
  //menu.append_item("Set Next Statement\tCtrl+Shift+F10",      SkMenu_debug_set_next_expr, false);
  menu.append_separator();
  menu.append_item("Print Callstack...",                      SkMenu_debug_print_callstack, false);
  menu.append_item("Callstack on break",                      SkMenu_debug_break_callstack, true, SkDebug::is_preference(SkDebug::PrefFlag_break_print_callstack));
  menu.append_item("Print Locals...",                         SkMenu_debug_print_locals, false);
  menu.append_item("Locals on break",                         SkMenu_debug_break_locals, true, SkDebug::is_preference(SkDebug::PrefFlag_break_print_locals));
  menu.append_separator();
  //menu.append_item("Custom Breakpoint...\tCtrl+B",            SBenu_debug_custom_breakpoint, false);
  menu.append_item("Add/Remove Breakpoint\tF9",               SkMenu_debug_toggle_breakpoint);
  menu.append_item("Enable/Disable Breakpoint\tCtrl+F9",      SkMenu_debug_toggle_breakpoint_enable);
  menu.append_item("Disable All Breakpoints\tAlt+Ctrl+F9",    SkMenu_debug_disable_breakpoints);
  menu.append_item("Remove All Breakpoints\tCtrl+Shift+F9",   SkMenu_debug_clear_breakpoints);
  menu.append_item("View Breakpoints...\tAlt+F9",             SkMenu_debug_view_breakpoints);
  } //  SkMainWindowBase::append_menubar_debug()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receieving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Author(s): John Stenersen
void SkMainWindowBase::refresh_menubar_debug(AMenuOS menu)
  {
  switch (SkDebug::get_execution_state())
    {
    case SkDebug::State_running:
      //menu.enable_item(SkMenu_debug_break_all,       true);
      menu.enable_item(SkMenu_debug_continue,        false);
      menu.enable_item(SkMenu_debug_step_next,       false);
      menu.enable_item(SkMenu_debug_step_over,       false);
      menu.enable_item(SkMenu_debug_step_into,       false);
      menu.enable_item(SkMenu_debug_step_out,        false);
      menu.enable_item(SkMenu_debug_show_next_expr,  false);
      //menu.enable_item(SkMenu_debug_set_next_expr,   false);
      menu.enable_item(SkMenu_debug_print_callstack, false);
      menu.enable_item(SkMenu_debug_print_locals,    false);
      break;

    case SkDebug::State_suspended:
      //menu.enable_item(SkMenu_debug_break_all,      false);
      menu.enable_item(SkMenu_debug_continue,         true);
      menu.enable_item(SkMenu_debug_step_next,        false);
      menu.enable_item(SkMenu_debug_step_over,        false);
      menu.enable_item(SkMenu_debug_step_into,        false);
      menu.enable_item(SkMenu_debug_step_out,         false);
      menu.enable_item(SkMenu_debug_show_next_expr,   false);
      //menu.enable_item(SkMenu_debug_set_next_expr,    false);
      menu.enable_item(SkMenu_debug_print_callstack,  false);
      menu.enable_item(SkMenu_debug_print_locals,     false);
      break;

    case SkDebug::State_suspended_member:
      //menu.enable_item(SkMenu_debug_break_all,        false);
      menu.enable_item(SkMenu_debug_continue,         true);
      menu.enable_item(SkMenu_debug_step_next,        false);
      menu.enable_item(SkMenu_debug_step_over,        false);
      menu.enable_item(SkMenu_debug_step_into,        false);
      menu.enable_item(SkMenu_debug_step_out,         true);
      menu.enable_item(SkMenu_debug_show_next_expr,   true);
      //menu.enable_item(SkMenu_debug_set_next_expr,    false);
      menu.enable_item(SkMenu_debug_print_callstack,  true);
      menu.enable_item(SkMenu_debug_print_locals,     true);
      break;

    case SkDebug::State_suspended_expr:
      //menu.enable_item(SkMenu_debug_break_all,        false);
      menu.enable_item(SkMenu_debug_continue,         true);
      menu.enable_item(SkMenu_debug_step_next,        true);
      menu.enable_item(SkMenu_debug_step_over,        true);
      menu.enable_item(SkMenu_debug_step_into,        true);
      menu.enable_item(SkMenu_debug_step_out,         true);
      menu.enable_item(SkMenu_debug_show_next_expr,   true);
      //menu.enable_item(SkMenu_debug_set_next_expr,    true);
      menu.enable_item(SkMenu_debug_print_callstack,  true);
      menu.enable_item(SkMenu_debug_print_locals,     true);
      break;
    }
  } //  SkMainWindowBase::refresh_menubar_debug()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Debug menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_debug(uint32_t item_id)
  {
  SkClassBrowser  * browser_p = SkClassBrowser::ms_browser_p;
  SkConsole       * console_p = SkConsole::ms_console_p;

  switch (eSkMenu(item_id))
    {
    case SkMenu_debug_toggle_breakpoint:
      if (browser_p)
        {
        browser_p->get_edit_view().toggle_breakpoint();
        }
      return true;

    case SkMenu_debug_continue:
      console_p->debug_continue();
      return true;

    case SkMenu_debug_step_next:
      console_p->debug_step(SkDebug::Step_next);
      return true;

    case SkMenu_debug_step_into:
      console_p->debug_step(SkDebug::Step_into);
      return true;

    case SkMenu_debug_step_over:
      console_p->debug_step(SkDebug::Step_over);
      return true;

    case SkMenu_debug_step_out:
      console_p->debug_step(SkDebug::Step_out);
      return true;

    case SkMenu_debug_show_next_expr:
      console_p->show_debug_expr();
      return true;

    case SkMenu_debug_print_callstack:
      console_p->get_remote_ide()->cmd_print_callstack();
      return true;

    case SkMenu_debug_break_callstack:
      if (browser_p)
        {
        browser_p->enable_debug_preference(SkDebug::PrefFlag_break_print_callstack, !SkDebug::is_preference(SkDebug::PrefFlag_break_print_callstack));
        }
      return true;

    case SkMenu_debug_print_locals:
      console_p->get_remote_ide()->cmd_print_locals();
      return true;

    case SkMenu_debug_break_locals:
      if (browser_p)
        {
        browser_p->enable_debug_preference(SkDebug::PrefFlag_break_print_locals, !SkDebug::is_preference(SkDebug::PrefFlag_break_print_locals));
        }
      return true;

    case SkMenu_debug_toggle_breakpoint_enable:
      if (browser_p)
        {
        browser_p->get_edit_view().toggle_breakpoint_enable();
        }
      return true;

    case SkMenu_debug_disable_breakpoints:
      console_p->breakpoints_disable_all();
      return true;

    case SkMenu_debug_clear_breakpoints:
      console_p->breakpoints_remove_all();
      return true;

    case SkMenu_debug_view_breakpoints:
      console_p->breakpoints_list_all();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_debug()


//=======================================================================================
//  Memory menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard list of Memory Menu items properly grayed based on the editbox context.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_memory(AMenuOS menu)
  {
  //  Memory statistics submenu
  menu.append_item("Runtime",                          SkMenu_memory_memory_runtime);
  menu.append_item("Runtime + Static Scripts",         SkMenu_memory_memory);
  menu.append_item("Runtime + Static + Demand Loaded", SkMenu_memory_memory_demand);
  } //  SkMainWindowBase::append_menubar_memory()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Memory menu.
// 
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_memory(uint32_t item_id)
  {
  SkConsole * console_p = SkConsole::ms_console_p;

  console_p->show();
  console_p->make_foreground();

  switch (eSkMenu(item_id))
    {
    case SkMenu_memory_memory_runtime:
      SkDebug::print_memory_runtime();
      ms_focused_last_p->set_focus();
      return true;

    case SkMenu_memory_memory:
      SkDebug::print_memory(SkCodeSerialize_static);
      ms_focused_last_p->set_focus();
      return true;

    case SkMenu_memory_memory_demand:
      SkDebug::print_memory(SkCodeSerialize_static_demand);
      ms_focused_last_p->set_focus();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_memory()


//=======================================================================================
//  Settings menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard Settings Menu items whose state will be updated before being displayed.
//  # Author(s): John Stenersen
void SkMainWindowBase::append_menubar_settings(AMenuOS menu)
  {
  menu.append_item("&Project Settings...",                 SkMenu_settings_classes);
  menu.append_item("&Save Layout",                         SkMenu_settings_layout);
  menu.append_item("&Right Alt Keyboard Characters",       SkMenu_settings_right_alt, true, AKeyboard::is_locale_alt());
  menu.append_item("&User Preferences...",                 SkMenu_settings_prefs);
  menu.append_separator();
  menu.append_item("Remote runtimes (mobile and console)", SkMenu_settings_remote_runtime, true, SkConsole::ms_console_p->get_remote_ide()->is_server_remote_enabled());
  menu.append_item("Use Perforce Version Control",         SkMenu_settings_perforce, true, SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4);
  } //  SkMainWindowBase::append_menubar_settings()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Settings menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_settings(uint32_t item_id)
  {
  SkConsole * console_p = SkConsole::ms_console_p;

  switch (eSkMenu(item_id))
    {
    case SkMenu_settings_classes:
      console_p->display_class_settings();
      return true;

    case SkMenu_settings_layout:
      SkDebug::print("\nSaving layout and settings...\n", SkLocale_local);
      console_p->save_settings();
      SkDebug::print("  ...saved.\n", SkLocale_local);
      return true;

    case SkMenu_settings_right_alt:
      console_p->enable_locale_alt(!AKeyboard::is_locale_alt());
      return true;

    case SkMenu_settings_prefs:
      SkDebug::print(
        "\n[" A_SOURCE_STR " Preferences dialog incomplete - try the .ini file.\n"
        "Any changes may not take effect until IDE is restarted.]\n",
        SkLocale_local,
        SkDPrintType_warning);
      console_p->get_ini_ide().get_file().execute();
      return true;

    case SkMenu_settings_remote_runtime:
      console_p->toggle_remote_runtime();
      return true;

    case SkMenu_settings_perforce:
      console_p->toggle_version_control();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_settings()


//=======================================================================================
//  Help menu methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Appends the standard Help Menu items whose state will be updated before being displayed.
//  # Author(s): John Stenersen
//---------------------------------------------------------------------------------------
void SkMainWindowBase::append_menubar_help(AMenuOS menu)
  {
  menu.append_item("Documentation (online)...",        SkMenu_help_online_docs);
  menu.append_item("Syntax (online)...",               SkMenu_help_online_syntax);
  menu.append_item("&Syntax (PDF)...",                 SkMenu_help_syntax);
  menu.append_item("Syntax &Proposed (PDF)...",        SkMenu_help_syntax_proposed);
  menu.append_item("SkookumScript forum (online)...",  SkMenu_help_online_forum);
  menu.append_item("End user license (online)...",     SkMenu_help_license);
  menu.append_separator();
  menu.append_item("&About SkookumScript...",          SkMenu_help_about);
  } //  SkMainWindowBase::append_menubar_help()


//---------------------------------------------------------------------------------------
//  Processes menu items from the Help menu.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::on_menubar_help(uint32_t item_id)
  {
  SkConsole * console_p = SkConsole::ms_console_p;

  switch (eSkMenu(item_id))
    {
    case SkMenu_help_online_docs:
      ::ShellExecute(nullptr, "open", "http://www.skookumscript.com/docs/", nullptr, nullptr, SW_SHOWNORMAL);
      return true;

    case SkMenu_help_online_syntax:
      ::ShellExecute(nullptr, "open", "http://www.skookumscript.com/docs/v3.0/lang/syntax/#skookumscript-language-specification", nullptr, nullptr, SW_SHOWNORMAL);
      return true;

    case SkMenu_help_syntax:
      {
      // $Revisit - CReis Hacky method to get some simple docs in for now.
      AFile doc(AApplication::ms_this_app_p->get_directory().get_path() + "Docs\\SkookumScript - Syntax.pdf");

      doc.qualify();
      doc.execute();
      return true;
      }

    case SkMenu_help_syntax_proposed:
      {
      // $Revisit - CReis Hacky method to get some simple docs in for now.
      AFile doc(AApplication::ms_this_app_p->get_directory().get_path() + "Docs\\SkookumScript - Syntax(new).pdf");

      doc.qualify();
      doc.execute();
      return true;
      }

    case SkMenu_help_online_forum:
      ::ShellExecute(nullptr, "open", "http://forum.skookumscript.com/", nullptr, nullptr, SW_SHOWNORMAL);
      return true;

    case SkMenu_help_license:
      ::ShellExecute(nullptr, "open", "http://forum.skookumscript.com/t/skookumscript-licensing-conditions/629/1", nullptr, nullptr, SW_SHOWNORMAL);
      return true;

    case SkMenu_help_about:
      console_p->display_about();
      return true;
    }

    return false;   //  Menu item was not processed here.
  } //  SkMainWindowBase::on_menubar_help()


//---------------------------------------------------------------------------------------
//  This routine sets up the menubar for either the Console or Browser windows.
//  Most of them are submenus are placeholder whose items are populated or the state updated
//  either "early" or "late". Early updates items, for example, when focus is attained by a window.
//  Late updates items just before the submenu is displayed.
//   
//  # Author(s): John Stenersen
void SkMainWindowBase::setup_menubar()
  {
  // Create Console's menubar
  AMenuOS menu;

  // Create File submenu
  APopMenuOS menu_file;
  append_menubar_file(menu_file);
  menu.append_submenu(&menu_file, "&File");   //  Populated here, item states are updated "late" just before the submenu is displayed.
  m_menu_file = menu_file.get_handle();

  // Create Edit submenu
  APopMenuOS menu_edit;
  menu.append_submenu(&menu_edit, "&Edit");   //  Make an empty Edit menu for the menubar, populated "late" with states depending on read-only and selection state.
  m_menu_edit = menu_edit.get_handle();

  //  Create View submenu
  APopMenuOS menu_view;
  append_menubar_view(menu_view);
  menu.append_submenu(&menu_view, "&View");   //  Populate items, though most of the states are updated "early".
  m_menu_view = menu_view.get_handle();

  //  Create the Goto submenu
  APopMenuOS menu_goto;
  append_menubar_goto(menu_goto);
  menu.append_submenu(&menu_goto, "&Goto");   //  Make an empty Edit menu for the menubar, populated later.
  m_menu_goto = menu_goto.get_handle();

  //  Create the Compile submenu
  APopMenuOS menu_compile;
  append_menubar_compile(menu_compile);
  menu.append_submenu(&menu_compile, "&Compile"); //  Make an empty Edit menu for the menubar, populated later.
  m_menu_compile = menu_compile.get_handle();

  //  Create the Debug submenu
  APopMenuOS menu_debug;
  append_menubar_debug(menu_debug);
  menu.append_submenu(&menu_debug, "&Debug");       //  Make an empty Edit menu for the menubar, populated later.
  m_menu_debug = menu_debug.get_handle();           //  Save the menu handle so the items can be added just before being displayed by the menubar.

  //  Create the Settings submenu
  APopMenuOS menu_settings;
  menu.append_submenu(&menu_settings, "&Settings"); //  Make an empty Edit menu for the menubar, populated later.
  m_menu_settings = menu_settings.get_handle();     //  Save the menu handle so the items can be added just before being displayed by the menubar.
  append_menubar_settings(menu_settings);

  //  Create the Memory submenu
  APopMenuOS menu_analyze;
  menu.append_submenu(&menu_analyze, "&Memory");    //  Make an empty Edit menu for the menubar, populated later.
  m_menu_memory = menu_analyze.get_handle();        //  Save the menu handle so the items can be added just before being displayed by the menubar.
  append_menubar_memory(menu_analyze);

  //  Create the Help submenu
  APopMenuOS menu_help;
  menu.append_submenu(&menu_help, "&Help");         //  Make an empty Edit menu for the menubar, populated later.
  m_menu_help = menu_help.get_handle();             //  Save the menu handle so the items can be added just before being displayed by the menubar.
  append_menubar_help(menu_help);

  //  Attach the menubar to the Console window.
  menu.set_menu_bar(this);
  } //  SkMainWindowBase::setup_menubar()


/*
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


//---------------------------------------------------------------------------------------
//  This routine is called when an editbox attains focus so the parent splitter is preserved
//  and used to update the menubar.
// 
//  # Params:
//    splitter_p: Pointer to the pane's parent splitter so the View menu can be updated for proper Swap/Rotate options.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::set_focus_splitter(ASplitterOS * splitter_p)
  {
  m_focus_splitter_p = splitter_p;
  return false; //  For now...
  } //  SkMainWindowBase::set_focus_splitter()


//---------------------------------------------------------------------------------------
//  This routine is called just before the submenu is displayed (upon receieving a WM_INITMENU or perhaps WM_INIMENUPOPUP message).
// 
//  # Params:
//    editbox_p:  Pointer to the incremental search EditSyntax box if calling from that context, otherwise nullptr.
//    submenu:    The submenu selected from either of the Console or Browser menubars.
//    allow_read_only_clear_all:  Only true for the Log pane, other read-only panes cannot be cleared i.e. edited.
//   
//  # Author(s): John Stenersen
bool SkMainWindowBase::refresh_menubar(HMENU submenu, AWindow * focus_p, eFocusType focus_type)
  {
  if (!focus_p)
    {
    A_DPRINT(A_SOURCE_FUNC_STR "focus_p is nullptr\n");
    return false;
    }

  //  Preserve the editbox focus and type. They're used to setup the menubar item states just before its rendered.
  //  They'll be needed once an item is selected so the correct editbox is acted upon.
  ms_focus_p    = reinterpret_cast<SkEditSyntax *>(focus_p);
  ms_focus_type = focus_type;

  //  Populate the File menu based on the display state of the Browser.
  if (submenu == m_menu_file)
    {
    refresh_menubar_file(AMenuOS(submenu));
    return true;
    }

  //  Populate the Edit menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_edit)
    {
    refresh_menubar_edit(AMenuOS(submenu));
    return true;
    }

  //  Refresh the View menu based on which window last had focus i.e. need to determine which splitter to operate on.
  if (submenu == m_menu_view)
    {
    refresh_menubar_view(AMenuOS(submenu));
    return true;
    }

  //  The Memory Statistics submenu has no items to update.
  if (submenu == m_menu_memory)
    {
    return true;
    }

  //  Populate the Goto menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_goto)
    {
    refresh_menubar_goto(AMenuOS(submenu));
    return true;
    }

  //  Populate the Compile menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_compile)
    {
    //  Nothing to refresh.
    return true;
    }

  //  Populate the Debug menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_debug)
    {
    refresh_menubar_debug(AMenuOS(submenu));
    return true;
    }

  //  Populate the Memory menu.
  if (submenu == m_menu_memory)
    {
    //  Nothing to refresh.
    return true;
    }

  //  Populate the Settings menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_settings)
    {
    return true;
    }

  //  Populate the Help menu from the current state of either the Log, Workbench or Edit panes.
  if (submenu == m_menu_help)
    {
    return true;
    }

  A_DPRINT(A_SOURCE_STR "Unknown submenu.\n");
  return false;
  } //  SkMainWindowBase::refresh_menubar()

  
//---------------------------------------------------------------------------------------
//  Called when the IDE focus window changes. The SkEditSyntax window that has attained is preserved (one for
//  each of the console and browser windows.
//
//  Args: One of focused_console and focused_browser will be non-null, which one indicates which of the
//        two main windows has gained focus.
//   
//  # Author(s): John Stenersen
void SkMainWindowBase::on_focus(AWindow * focus_p, eFocusType focus_type)
  {
  if (focus_type == FocusType_unknown)
    {
    A_DPRINT(A_SOURCE_FUNC_STR "FocusType_unknown\n");
    return;
    }

  //  Set the last
  ms_focused_last_p       = reinterpret_cast<SkEditSyntax *>(focus_p);
  ms_focused_last_type    = focus_type;

  switch (focus_type & ~FocusType_isearch)
    {
    case FocusType_log:
    case FocusType_workspace:
      ms_focused_console_p    = reinterpret_cast<SkEditSyntax *>(focus_p);
      ms_focused_console_type = focus_type;
      break;

    default:
      ms_focused_browser_p    = reinterpret_cast<SkEditSyntax *>(focus_p);
      ms_focused_browser_type = focus_type;
    }
  } //  SkMainWindowBase::on_focus()


//---------------------------------------------------------------------------------------
// Common menu actions for main/top-level windows - called when the user makes a
// selection from associated menu bar or associated pop-up / context menu.
// 
// # Params:
//   item_id: id of item selected
//   menu_win_p: window the menu action originated from
//   
// # Modifiers: virtual - overridden from AWindow
// # Author(s): Conan Reis
bool SkMainWindowBase::on_menubar(uint32_t item_id)
  {
  if (
    on_menubar_file(item_id)    ||
    on_menubar_edit(item_id)    ||
    on_menubar_view(item_id)    ||
    on_menubar_goto(item_id)    ||
    on_menubar_compile(item_id) ||
    on_menubar_debug(item_id)   ||
    on_menubar_memory(item_id)  ||
    on_menubar_settings(item_id)||
    on_menubar_help(item_id)
    )
    {
    return true;
    }

  A_DPRINT(A_SOURCE_STR "Unexpected menu item_id = %d\n", item_id);
  return false;
  } //  SkMainWindowBase::on_menubar()
