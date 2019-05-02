    
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE main window components shared by the Console and Browser.
// # Notes:        
//=======================================================================================

#ifndef __SKMAINWINDOWBASE_HPP
#define __SKMAINWINDOWBASE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI/AWindow.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class  SkClassBrowser;
class  SkConsole;
class  SkEditSyntax;
class  SkEditBox;


//------------------------------// Common Menu Options - shared between Console and Browser
enum eSkMenu
  {
  SkMenu_unimplemented            = 1,

  //  Common: File
  SkMenu_file_load_project          = 10000,
  SkMenu_file_load_project_default,
  SkMenu_file_new,
  SkMenu_file_save,
  SkMenu_file_checkout,
  SkMenu_file_open_external_editor,
  SkMenu_file_open_explorer,
  SkMenu_file_copy_pathname,
  SkMenu_file_open_close_browser,
  SkMenu_file_exit_console,

  // Common: Edit
  SkMenu_edit_undo                = 10100,
  SkMenu_edit_redo,
  SkMenu_edit_cut,
  SkMenu_edit_copy,
  SkMenu_edit_copy_with_format,
  SkMenu_edit_copy_result,
  SkMenu_edit_paste,
  SkMenu_edit_paste_result,
  SkMenu_edit_delete,
  SkMenu_edit_clear_all,
  SkMenu_edit_select_all,

  // Common: View
  SkMenu_view                     = 10200,
  SkMenu_view_console,
  SkMenu_view_browser,
  SkMenu_view_disassembly,
  SkMenu_view_expression_guide,
  SkMenu_view_auto_parse,
  SkMenu_view_auto_parse_sel,
  SkMenu_view_auto_complete,
  SkMenu_view_syntax_highlight,
  SkMenu_view_current_line_highlight,
  SkMenu_view_swap,
  SkMenu_view_rotate_cw,
  SkMenu_view_rotate_ccw,

  // Common: Goto
  SkMenu_goto_context             = 10300,
  SkMenu_goto_class,
  SkMenu_goto_routine,
  SkMenu_goto_data,
  SkMenu_goto_local,
  SkMenu_goto_all,
  SkMenu_goto_find,
  SkMenu_goto_find_next,
  SkMenu_goto_find_prev,
  SkMenu_goto_history_next,
  SkMenu_goto_history_prev,

  // Common: Compile
  SkMenu_compile_project_stale    = 10400,
  SkMenu_compile_project,
  SkMenu_compile_incremental,
  SkMenu_compile_class,
  SkMenu_compile_classes,
  SkMenu_compile_member,
  SkMenu_compile_snippet,
  SkMenu_compile_snippet_ide,
  SkMenu_compile_update_remote,
  SkMenu_compile_strict,
  SkMenu_compile_error_dialog,

  // Common: Debug
  SkMenu_debug_break_all          = 10500,
  SkMenu_debug_continue,
  SkMenu_debug_step_next,
  SkMenu_debug_step_over,
  SkMenu_debug_step_into,
  SkMenu_debug_step_out,
  SkMenu_debug_run_to,
  SkMenu_debug_show_next_expr,
  SkMenu_debug_set_next_expr,
  SkMenu_debug_print_callstack,
  SkMenu_debug_break_callstack,
  SkMenu_debug_print_locals,
  SkMenu_debug_break_locals,
  SBMenu_debug_custom_breakpoint,
  SkMenu_debug_toggle_breakpoint,
  SkMenu_debug_toggle_breakpoint_enable,
  SkMenu_debug_disable_breakpoints,
  SkMenu_debug_clear_breakpoints,
  SkMenu_debug_view_breakpoints,

  //  Common: Memory
  SkMenu_memory_memory_runtime    = 10600,
  SkMenu_memory_memory,
  SkMenu_memory_memory_demand,

  //  Common: Settings
  SkMenu_settings_classes         = 10700,
  SkMenu_settings_layout,
  SkMenu_settings_right_alt,
  SkMenu_settings_prefs,
  SkMenu_settings_remote_runtime,
  SkMenu_settings_perforce,

  //  Common: Help
  SkMenu_help_online_docs         = 10800,
  SkMenu_help_online_syntax,
  SkMenu_help_syntax,            
  SkMenu_help_syntax_proposed,
  SkMenu_help_online_forum,
  SkMenu_help_license,
  SkMenu_help_about
  };


//---------------------------------------------------------------------------------------
// Shared features amongst top-level windows
class SkMainWindowBase : public AWindow
  {
  public:

    enum eSkMainWindowBaseType
      {
      MainWindowBaseType_console,
      MainWindowBaseType_browser
      };

    enum eLoadView
      {
      LoadView_ignore,
      LoadView_restore,
      };

  // Public Class Data Members

  // Methods

    SkMainWindowBase(eSkMainWindowBaseType type) { m_type = type; }

    virtual void save_settings() = 0;

    //  Menubar event methods
    static void on_focus(AWindow * focus_p, eFocusType focus_type);
           bool on_menubar(uint32_t item_id);

    //  Menubar setup/update routines
    void setup_menubar();
    bool set_focus_splitter(ASplitterOS * splitter_p);
    bool refresh_menubar(HMENU submenu, AWindow * focus_p, eFocusType focus_type);
    static void append_menubar_edit(AMenuOS menu_p, SkEditBox * editbox_p, eFocusType focus_type, bool context_menu = true);

    eSkMainWindowBaseType get_type()                  { return m_type;                  }

    static SkEditSyntax * get_focused_console()       { return ms_focused_console_p;    }
    static SkEditSyntax * get_focused_browser()       { return ms_focused_browser_p;    }
    static SkEditSyntax * get_focused_last()          { return ms_focused_last_p;       }
    static eFocusType     get_focused_console_type()  { return ms_focused_console_type; }
    static eFocusType     get_focused_browser_type()  { return ms_focused_browser_type; }
    static eFocusType     get_focused_last_type()     { return ms_focused_last_type;    }

  //  Members
  protected:
    void append_menubar_file(     AMenuOS menu);
    void append_menubar_view(     AMenuOS menu);
    void append_menubar_goto(     AMenuOS menu);
    void append_menubar_compile(  AMenuOS menu);
    void append_menubar_debug(    AMenuOS menu);
    void append_menubar_memory(   AMenuOS menu);
    void append_menubar_settings( AMenuOS menu);
    void append_menubar_help(     AMenuOS menu);

    void refresh_menubar_file(    AMenuOS menu);
    void refresh_menubar_edit(    AMenuOS menu);
    void refresh_menubar_view(    AMenuOS menu);
    void refresh_menubar_goto(    AMenuOS menu);
    //  Note: No Compile menu refreshing is needed.
    void refresh_menubar_debug(   AMenuOS menu);

    bool on_menubar_file(    uint32_t item_id);
    bool on_menubar_edit(    uint32_t item_id);
    bool on_menubar_view(    uint32_t item_id);
    bool on_menubar_goto(    uint32_t item_id);
    bool on_menubar_compile( uint32_t item_id);
    bool on_menubar_debug(   uint32_t item_id);
    bool on_menubar_memory( uint32_t item_id);
    bool on_menubar_settings(uint32_t item_id);
    bool on_menubar_help(    uint32_t item_id);

    eSkMainWindowBaseType   m_type;                       //  Menubar is called from a context e.g. Log, Editor's search box, Workspace, etc.
    ASplitterOS           * m_focus_splitter_p = nullptr; //  Menubar is called from a context - this member is set to that pane's parent splitter.

    //  Handles to menubar dropdown submenus.
    HMENU m_menu_file;
    HMENU m_menu_edit;
    HMENU m_menu_view;
    HMENU m_menu_goto;
    HMENU m_menu_compile;
    HMENU m_menu_debug;
    HMENU m_menu_memory;
    HMENU m_menu_settings;
    HMENU m_menu_help;

  //  Static members

    //  Focus context
    static SkEditSyntax * ms_focus_p;     //  The context (pane) set just before menu is displayed.
    static eFocusType     ms_focus_type;

    //  Tracks input focus. Needed for updating menus before displaying and performing selection.
    static SkEditSyntax * ms_focused_console_p;    //  One of Log or Workspace (or incremental search windows)
    static SkEditSyntax * ms_focused_browser_p;    //  One of Editor, ClassSettings, NewClass, DataList, CodeList (or incremental search windows)
    static SkEditSyntax * ms_focused_last_p;       //  Last window focused, either m_focused_browser or m_focused_console
    static eFocusType     ms_focused_console_type;
    static eFocusType     ms_focused_browser_type;
    static eFocusType     ms_focused_last_type;

  };  // SkMainWindowBase

                 
#endif  //  __SKMAINWINDOWBASE
