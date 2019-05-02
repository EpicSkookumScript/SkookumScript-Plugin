
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Console (Workspace window) & supporting classes
//=======================================================================================

#ifndef __SKCONSOLE_HPP
#define __SKCONSOLE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO/AKeyboard.hpp>
#include <AgogGUI_OS/ACheckBoxOS.hpp>
#include <AgogGUI_OS/AButtonOS.hpp>
#include <AgogGUI_OS/AListOS.hpp>
#include <AgogGUI_OS/ARichEditOS.hpp>
#include <AgogGUI_OS/ASplitterOS.hpp>
#include <AgogGUI_OS/AToolTipOS.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumIDE/SkAutoComplete.hpp>
#include <SkookumIDE/SkCompiler.hpp>
#include <SkookumIDE/SkMainWindowBase.hpp>
#include <SkookumIDE/SkRemoteIDE.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class  ASymbol;
struct ADirEntry;
class  SkClassBrowser;
class  SkClassSettings;
class  SkConsole;
class  SkEditSyntax;
class  SkIncrementalSearchEditBox;
struct SkMatchCriteria;
class  SkSearchDialog;
struct SkScriptEntry;  // Declared in the cpp

namespace Gdiplus
  {
  class Graphics;
  }

enum eSkLog
  {
  SkLog_silent,
  SkLog_ide_print
  };


enum eSkMatchKind
  {
  SkMatchKind_classes  = 1 << 0,
  SkMatchKind_routines = 1 << 1,
  SkMatchKind_data     = 1 << 2,
  SkMatchKind_members  = SkMatchKind_data | SkMatchKind_routines,
  SkMatchKind_all      = SkMatchKind_classes | SkMatchKind_data | SkMatchKind_routines,

  SkMatchKind__invalid = 0x0
  };

//---------------------------------------------------------------------------------------
enum eSkVersionControl
  {
  SkVersionControl_none,  // No version control system set
  SkVersionControl_p4     // Perforce
  };


//---------------------------------------------------------------------------------------
// Adds IDE related methods to SkMemberInfo.
// $Revisit - CReis Might be useful to derive from SkMemberExpression instead.
struct SkContextInfo : public SkMemberInfo
  {
  // Methods

    SkContextInfo()                                                                {}
    SkContextInfo(const SkQualifier & member_id, eSkMember type, bool class_scope) : SkMemberInfo(member_id, type, class_scope)  {}
    SkContextInfo(const SkContextInfo & info)                                      : SkMemberInfo(info)  {}
    SkContextInfo(const SkMemberInfo & info)                                       : SkMemberInfo(info)  {}

    bool  compare_files(const SkMemberInfo & info) const;

    AFile as_file_existing() const;
    AFile as_file_create(const SkOverlay & overlay) const;
    void  action_goto_browser() const;
    void  action_goto_file_explorer() const;
    void  action_copy_name() const;
    void  action_copy_path() const;
    void  action_insert_name_editor() const;
    void  action_insert_name_workspace() const;
    void  action_edit_external() const;
    void  action_p4_checkout() const;
    void  action_p4_revert() const;
    void  action_p4_diff_prev() const;
    void  action_p4_properties() const;
    void  action_p4_timelapse() const;
    void  action_p4_history() const;
  };


//---------------------------------------------------------------------------------------
// Subclasses: SkEditSyntax(SkEditor, SkWorkspace), SkLog
class SkEditBox : public ARichEditOS
  {
  public:

  // Methods

    static void initialize();
    static void deinitialize();

    SkEditBox(AWindow * parent_p, const AString & initial = AString::ms_empty, const AFont & font = *AFont::ms_fixed_p, uint32_t flags = RichFlag_show_selection_always);

    eSkMatchKind caret_context(SkMatchCriteria * match_info_p = nullptr, uint32_t * begin_idx_p = nullptr, uint32_t * end_idx_p = nullptr) const;
    virtual void save_settings(eSkLog log = SkLog_ide_print)                 {}
    virtual void set_font(const AFont & font_p) override;
//    bool on_context_menu_edit(uint32_t item_id);

    // User Actions

      void  action_evaluate_selected(eSkLocale locale = SkLocale_runtime);

    // Class Methods

      static AString & get_result_string()                                   { return ms_result; }
      static void      set_result_string(AString & result_str)               { ms_result = result_str; }

  protected:

  // Internal Class Methods

    static int CALLBACK word_break_callback(LPTSTR lpch, int ichCurrent, int cch, int code);

  // Class Data

    static AString ms_result;

  };  // SkEditBox


//---------------------------------------------------------------------------------------
class SkIncrementalSearchEditBox : public AWindow
  {
  friend class SkEditSyntax;
  friend class SkLog;

  public:
    // Nested Types

    enum eParentContext
      {
      ParentContext_other,
      ParentContext_log,
      ParentContext_workspace,
      ParentContext_editor,
      ParentContext_create_new,
      ParentContext_search_dialog,
      ParentContext_class_settings
      };

    SkIncrementalSearchEditBox(SkEditBox * parent_p, eParentContext parent_context, int width = 300);

    virtual bool    on_key_press_bidirectional(eAKey key, bool repeated, eAKeyMod mod_keys, bool from_parent);                                       //  Handles key presses from both parent editbox or child (search key) editbox.
    int             get_match_count() { return m_match_count; }
    SkEditBox     * get_search_key()  { return & m_search_key; }

  protected:

    // Methods

    void show();
    void hide() override;
    void reposition();
    void matches_highlight();
    void matches_unhighlight();

    // Event Methods
    virtual bool on_key_press(eAKey key, bool repeated) { return on_key_press_bidirectional(key, repeated, AKeyboard::get_mod_keys(), false); }   //  Is called from the child (search key) editbox, pass it down to process incremental search.
    virtual void on_sizing();
    virtual void on_search_key_modified();
    virtual void on_parent_modified();
    virtual bool on_hide(bool state_changed) { hide(); return true; }
    virtual void on_toggle_case_sensitive(eAFlag);
    virtual void on_toggle_fuzzy(eAFlag);
    virtual bool on_focus();

    bool search_forward(uint32_t search_start = 0, uint32_t search_end = ALength_remainder, bool wrap_flag = true);
    bool search_reverse(uint32_t search_start = 0, uint32_t search_end = ALength_remainder, bool wrap_flag = true);
    void set_text_style_fuzzy(uint32_t fuzzy_start, uint32_t fuzzy_end);

    // Internal Methods

    //  Data Members
    //  UI Elements
    SkEditBox       * m_parent_p;
    eParentContext    m_parent_context  = ParentContext_other;
    SkEditBox         m_search_key;
    ACheckBoxOS       m_case_sensitive_tgl;
    ACheckBoxOS       m_fuzzy_tgl;
    //AToolTipOS      m_tool_tip;

    int               m_initial_start   = 0;
    int               m_initial_end     = 0;
    bool              m_accepted        = false;
    int               m_width;
    int               m_height          = 30;     //  Will be calculated relative to the font height.
    int               m_match_count     = 0;
    AString           m_search_key_last = "";

    // Inherited from AWindow

  };  // SkIncrementalSearchEditbox


//---------------------------------------------------------------------------------------
// Subclasses: SkEditor, SkWorkspace
class SkEditSyntax : public SkEditBox
  {
  public:

  // Nested Structures

    // For tracking open/close group pairs like () {} []
    enum eGroup
      {
      Group_1,
      Group_2,
      Group_3
      };

    enum eCoverage
      {
      Coverage_none,
      Coverage_all,
      Coverage_visible
      };

    enum eType
      {
      Type_editor,
      Type_workspace,
      Type_log,
      Type_single_line
      };

  // Common Methods

    SkEditSyntax(eType type, AWindow * parent_p, SkIncrementalSearchEditBox::eParentContext  parent_context, const AString & initial = AString::ms_empty, const AFont & font = *AFont::ms_fixed_p);

    virtual const AFile & get_source_file() const;
    virtual void          set_source(const AString & str, bool read_only_b = false);
    void                  syntax_highlight(eCoverage coverage = Coverage_all);
    void                  set_identify_flags(uint32_t flags) { m_identify_flags = flags; }
    eType                 get_type() { return m_type; }

    void  action_edit_externally();
    void  action_goto_file_explorer();
    void  action_copy_path() const;

  // Event Methods

    virtual bool on_context_menu(const AVec2i & screen_pos) override;
    virtual bool on_key_press(eAKey key, bool repeated) override;
    virtual void on_mouse_spinning(f32 wheel_delta, eAMouse buttons, const AVec2i & client_pos) override;
    virtual bool on_focus()              override;
    virtual bool on_scrollbar_horiz()    override;
    virtual bool on_scrollbar_vert()     override;
    virtual void on_sizing()             override;
    virtual void on_moving(eSpace space) override;
    virtual void on_modified()           override      { m_incremental_search_editbox.on_parent_modified(); SkEditBox::on_modified(); }
    virtual bool on_hide(bool state_changed) override  { m_incremental_search_editbox.hide(); return SkEditBox::on_hide(state_changed); }
    virtual bool on_draw()               override;
    virtual void on_draw_subpart(Gdiplus::Graphics & graphics) {}
    virtual void on_selecting(uint32_t start, uint32_t end) override;

    SkIncrementalSearchEditBox * get_incremental_search() { return & m_incremental_search_editbox; }

    uint file_to_index(uint file_index) const;
    void reset_syntax_style(AColor::eScheme scheme = AColor::Scheme_default);
    void get_syntax_style(ATextStyle * style_p, SkParser::eIdentify element);
    AColor::eScheme get_scheme() {return m_scheme; }

  protected:

  // Internal Methods


    // Expression index methods

    uint32_t idx_to_expr_idx(uint32_t idx);
    void     idx_to_expr_span(uint32_t idx, uint32_t * idx_begin_p, uint32_t * idx_end_p);

    void draw_mark(uint32_t idx, const AColor & color, Gdiplus::Graphics & graphics);
    void draw_span(uint32_t idx_begin, uint32_t idx_end, const AColor & color, Gdiplus::Graphics & graphics);
    void draw_span_pivot(uint32_t idx_begin, uint32_t idx_end, uint32_t idx_pivot, const AColor & color, Gdiplus::Graphics & graphics);

  // Data Members

    eType           m_type;
    uint32_t        m_identify_flags;  // See SkParser::eIdentifyFlag
    AColor::eScheme m_scheme;
    uint32_t        m_start_idx_prev;
    eGroup          m_start_group_prev;
    eGroup          m_group;
    AColor          m_default_colour;
    AColor          m_color_font;
    bool            m_invert_b;  // Invert luminance?

    bool            m_auto_parse_ok     = true;
    uint32_t        m_auto_parse_start  = 0;
    uint32_t        m_auto_parse_end    = 0;
    uint32_t        m_caret_index_last  = ALength_remainder;

    SkIncrementalSearchEditBox  m_incremental_search_editbox;

  };  // SkEditSyntax


//---------------------------------------------------------------------------------------
// Author(s): Conan Reis
class SkWorkspace : public SkEditSyntax
  {
  public:
  // Common Methods

    SkWorkspace(AWindow * parent_p, SkConsole * console_p);

    void         load_settings();
    virtual void save_settings(eSkLog log = SkLog_ide_print) override;

  // Event Methods

    virtual void on_modified();

  // Class Methods

    virtual const AFile & get_source_file() const { return get_ini_workspace_file(); }
    const AFile &         get_ini_workspace_file() const;

  protected:

    virtual bool on_focus() override;
    virtual void on_focus_lost(HWND focus_window) override;
    virtual bool on_character(char ch, bool repeated) override;
    virtual bool on_key_press(eAKey key, bool repeated) override;
    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos) override {m_auto_complete.on_parent_mouse_release(button, buttons, client_pos); }

  // Data Members

    AToolTipOS      m_tooltip;
    SkConsole *     m_console_p;
    SkAutoComplete  m_auto_complete;
    AFile           m_workspace_file;

  };  // SkWorkspace


//---------------------------------------------------------------------------------------
class SkLog : public SkEditSyntax
  {
  public:
  // Common Methods

    SkLog(AWindow * parent_p, SkConsole * console_p);

    virtual void save_settings(eSkLog log = SkLog_ide_print) override;

  // Event Methods

    virtual bool on_mouse_press(eAMouse button, eAMouse buttons, const AVec2i & client_pos, bool double_click);
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_key_press(eAKey key, bool repeated);
    virtual bool on_hide(bool state_changed) { m_incremental_search_editbox.on_hide(state_changed); return SkEditBox::on_hide(state_changed); }
    virtual bool on_focus();

  protected:

  // Data Members

    AToolTipOS  m_tooltip;
    SkConsole * m_console_p;

  };  // SkLog


typedef AColumnOS<SkOverlay>::SubItemText tSkOverlaySubText;

//---------------------------------------------------------------------------------------
class SkOverlayList : public AListOS<SkOverlay>
  {
  public:
  // Nested Data-structures

    enum eColumn
      {
      Column_sequence,
      Column_name,
      Column_directory
      };

  // Methods

    SkOverlayList(SkClassSettings * settings_p, SkConsole * console_p);
    virtual ~SkOverlayList();

  protected:

  // Event Methods

    virtual void on_subitem_activated(uint row, uint rank);
    virtual void on_item_focused(SkOverlay * item_p, uint row);
    virtual void on_item_selected(SkOverlay * item_p, uint row, bool selected);

    void on_text_sequence(tSkOverlaySubText * info_p);
    void on_text_name(tSkOverlaySubText * info_p);
    void on_text_dir(tSkOverlaySubText * info_p);

  // Data Members

    SkClassSettings * m_settings_p;
    SkConsole *       m_console_p;

  };  // SkDataList


//---------------------------------------------------------------------------------------
class SkClassSettings : public AWindow
  {
  friend class SkOverlayList;

  public:
  // Common Methods

    SkClassSettings(SkConsole * console_p);

    void update_title();
    void apply_changes();
    void display();
    void set_overlays_changed();

  // Event Methods

    void on_startup_mind_modified();
    void on_overlay_focus(uint row);
    void on_toggle_load(eAFlag new_state);

    virtual bool on_draw();
    virtual bool on_focus();
    virtual void on_sizing();

    // Button Events

    void on_overlay_add();
    void on_overlay_remove();
    void on_overlay_toggle();
    void on_overlay_remap();
    void on_overlay_up();
    void on_overlay_down();
    void on_compiled_remap();
    void on_ok();

  protected:

  // Data Members

    SkConsole * m_console_p;

    RECT          m_overlay_rect;
    AButtonOS     m_overlay_add_btn;
    AButtonOS     m_overlay_remove_btn;
    AButtonOS     m_overlay_toggle_btn;
    AButtonOS     m_overlay_remap_btn;
    AButtonOS     m_overlay_up_btn;
    AButtonOS     m_overlay_down_btn;
    SkOverlayList m_overlay_list;
    bool          m_overlays_changed_b;

    RECT        m_compiled_rect;
    ACheckBoxOS m_compiled_load_tgl;
    ACheckBoxOS m_compiled_save_tgl;
    AEditLineOS m_compiled_path;  
    AButtonOS   m_compiled_remap_btn;
    int         m_compiled_path_text_y;

    RECT         m_eval_rect;
    ACheckBoxOS  m_eval_atomics_tgl;
    SkEditSyntax m_eval_mind_class;
    AButtonOS    m_eval_mind_btn;
    int          m_eval_mind_text_y;

    AButtonOS   m_ok_btn;
    AButtonOS   m_cancel_btn;
    AButtonOS   m_apply_btn;

  };  // SkClassSettings


//---------------------------------------------------------------------------------------
class SkAbout : public AWindow
  {
  public:
  // Common Methods

    SkAbout();

  // Event Methods

    virtual bool on_draw();
    virtual bool on_key_press(eAKey key, bool repeated);

  protected:

  // Data Members

    AButtonOS m_ok_btn;

  };  // SkAbout


//---------------------------------------------------------------------------------------
// Dialog shown when compiler errors occur
// 
// $Revisit - CReis Should probably rewrite this as a generic class.
class SkErrorDialog : public AWindow
  {
  public:
  // Common Methods

    SkErrorDialog();

    void set_message(const AString & msg);

  // Event Methods

    void on_btn_continue();
    void on_btn_recompile();

    virtual bool on_draw();
    virtual bool on_key_press(eAKey key, bool repeated);

  protected:

  // Data Members

    AString m_text;

    AButtonOS m_ok_btn;
    AButtonOS m_continue_btn;
    AButtonOS m_recompile_btn;

  };  // SkErrorDialog

//---------------------------------------------------------------------------------------
struct SkProjectInfoConsole : SkProjectInfo
  {
  eAProgess  m_load_state;  
  uint32_t   m_compiled_flags;

  SkProjectInfoConsole() :
    m_load_state(AProgess_skip),
    m_compiled_flags(SkRemoteIDE::CompiledFlag_ignore)
    {}

  // Assign project information
  void set(const SkProjectInfo & project_info) { *static_cast<SkProjectInfo *>(this) = project_info; }
  };

//---------------------------------------------------------------------------------------
// The console is a combination of the output window and the workspace window with a
// splitter bar in between them.
class SkConsole : public SkMainWindowBase, public SkConsoleBase
  {
  friend class SkClassSettings;

  public:

  // Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Used by m_pref_flags
    enum ePreference
      {
      Preference_none           = 0x0,
      Preference_update_remote  = 1 << 0,   // Send changes to remote side
      Preference_online_menu    = 1 << 1,
      Preference_error_dialog   = 1 << 2,

      Preference__default = Preference_none
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eCloseAction
      {
      CloseAction_shutdown,
      CloseAction_close,
      CloseAction_hide
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eSound
      {
      Sound_open,
      Sound_close,
      Sound_error,
      Sound_breakpoint,

      Sound__count  // Number of sounds
      };


  // Public Class Data Members

    static SkConsole * ms_console_p;

  // Common Methods

    static void initialize();
    static void deinitialize();
    static bool is_initialized();

    SkConsole(SkCompiler::eInit init_type = SkCompiler::Init_phased, eCloseAction close_action = CloseAction_hide);
    ~SkConsole();

  // Methods

    void set_close_action(eCloseAction action)                        { m_close_action = action; }
    void close(bool shutdown_b = false);

    void compile_project();
    void compile_project_stale();
    void compile_class(SkClass * class_p, bool subclasses);
    void compile_class_browser(bool subclasses);
    void compile_member(const SkMemberInfo & member);
    void compile_member_browser();
    void enable_strict_compile(bool strict = true);
    void enable_locale_alt(bool uses_alt_chars = true);

    //---------------------------------------------------------------------------------------
    // Set info when connected to a new runtime
    bool connect_new_runtime(const SkProjectInfo & project_info);

    //---------------------------------------------------------------------------------------
    // Loads project from specified project file (.ini) path.
    // 
    // Note:
    //   It does not check if the file first exists or not - that should be checked at the
    //   point where the path is retrieved.
    // 
    // Params:
    //   project_path:
    //     project file path to load. If it isn't titled (i.e. `is_titled()` test fails on it
    //     then the default project is loaded instead.
    bool load_project(const SkProjectInfo & project_info);

    bool      load_project_query();
    bool      load_project_default_query();
    eAProgess get_load_project_state()                                      { return m_project_info.m_load_state; }
    void      notify_on_load_project_deferred(bool freshen)                 { m_project_info.m_compiled_flags = SkRemoteIDE::CompiledFlag_notify | (freshen ? SkRemoteIDE::CompiledFlag_freshen : 0u); }
    bool      verify_project_editable();

    virtual AFile get_ini_file_proj_startup() override;
    void          set_last_project(const AFile & project_file, const AFile & default_project_file);
    void          update_title();

    void          refresh_status_remote();
    virtual void  status_update();
    virtual bool  browse_folder(AString * path_p, const char * message_p, const char * path_start_p);
    virtual void  log_append(const SkPrintInfo & info);
    void          play_sound(eSound sound);
    virtual void  progress_dot(bool completed = false);

    void              toggle_ide();
    void              display_ide(eAFlag show_flag = AFlag_on);
    void              display_ide(eAFlag show_flag, ASymbol focus_class_name, ASymbol focus_member_name, bool is_data_member, bool focus_member_class_scope);
    void              display_about();
    void              display_class_settings();
    void              display_error(const AString & msg);
    void              hide_error();

    SkClassBrowser *  display_browser(bool show_b = true);
    void              toggle_browser();
    void              browse_member(const SkContextInfo & member_info, uint index_start = 0u, uint index_end = ADef_uint32);
    SkClassBrowser *  get_browser() const                             { return m_browser_p; }
    SkWorkspace &     get_workspace()                                 { return m_workspace; }
    SkLog &           get_log()                                       { return m_log; }
    ASplitterOS *     get_split_text()                                { return & m_split_text; }

    const AImageListOS & get_member_images();

    void              display_goto_dialog(eSkMatchKind kind = SkMatchKind_all, const AString & match_text = AString::ms_empty, const SkMatchCriteria * match_info_p = nullptr);
    void              display_goto_context(const SkEditBox & editor);
    void              display_goto_context_focus();
    void              display_goto_context_editor();
    void              display_goto_context_workspace();

    static void       cmd_args_execute(const AString & cmd_str);
    bool              cmd_args_execute_auto(const AString & simple_str);

  // Settings Methods

    void           load_settings();
    virtual void   save_settings() override;
    virtual AFile  get_ini_compiled_file_query(bool loading_b = true) override;
    AIni &         get_ini_ide()                                      { return m_ini_ide; }
    uint32_t       get_prefernce_flags() const                        { return m_pref_flags; }
    static AFile   make_qualified(const AFile & rel_file)             { return ms_console_p->m_ini_ide.make_qualified(rel_file); }

    AFont              get_ini_font();
    AFont              get_ini_font_code_narrow();
    eSkLocale          get_ini_online_mode();
    const ADirectory & get_script_template_dir()                      { return m_template_dir; }
    void               set_online_settings(eSkLocale locale);

    void               toggle_remote_runtime();


  // Debugging

    void breakpoints_disable_all();
    void breakpoints_remove_all(bool query = true);
    void breakpoints_list_all(bool focus_log = true);
    void debug_expr(SkExpressionBase * expr_p, const SkContextInfo & member_info);
    void debug_continue();
    void debug_reset();
    void debug_step(SkDebug::eStep step_type);
    void refresh_debug_ui();
    void show_debug_expr();

    #ifdef SKOOKUM_IDE_EMBEDDED
      void        enable_debugging_embedded(bool enable_b = true);
      static void breakpoint_hit_embedded(SkBreakPoint * bp_p, SkObjectBase * scope_p, SkInvokedBase * caller_p);
    #endif

    static void debug_scripted_break(const AString & message, SkInvokedMethod * scope_p);

  // Version Control Methods

    eSkVersionControl get_version_control_system() const  { return m_version_control; }
    void              set_version_control(eSkVersionControl system);
    void              toggle_version_control();


  // Remote IDE Methods

    bool          is_remote_ide() const                   { return m_remote.get_mode() == SkLocale_ide; }
    bool          is_remote_update_enabled() const        { return is_remote_ide() && ((m_pref_flags & Preference_update_remote) != 0u); }
    bool          is_show_error_dialog() const            { return (m_pref_flags & Preference_error_dialog) != 0u; }
    void          enable_error_dialog(bool error_dialog = true);
    void          enable_remote_update(bool update_remote = true);
    SkRemoteIDE * get_remote_ide()                        { return &m_remote; }
    void          update_online_menu(eSkLocale locale);

  //  View Settings
    void          enable_disassembly(             bool show = true);
    void          enable_expression_guide(        bool show = true);
    void          enable_auto_parse(              bool show = true);
    void          enable_auto_parse_sel(          bool show = true);
    void          enable_syntax_highlight(        bool show = true);
    void          enable_current_line_highlight(  bool show = true);

    bool          is_disassembly() const       { return m_disassembly; }
    bool          is_expression_guide() const  { return m_expression_guide; }
    bool          is_auto_parse() const        { return m_auto_parse; }
    bool          is_auto_parse_sel() const    { return m_auto_parse_sel; }
    bool          is_syntax_highlight() const  { return m_syntax_highlight; }
    bool          is_current_line_highlight() const { return m_current_line_highlight; }

    void          toggle_disassembly()            { enable_disassembly(!m_disassembly); }
    void          toggle_expression_guide()       { enable_expression_guide(!m_expression_guide); }
    void          toggle_auto_parse()             { enable_auto_parse(!m_auto_parse); }
    void          toggle_auto_parse_sel()         { enable_auto_parse_sel(!m_auto_parse_sel); }
    void          toggle_syntax_highlight()       { enable_syntax_highlight(!m_syntax_highlight); }
    void          toggle_current_line_highlight() { enable_current_line_highlight(!m_current_line_highlight); }


  // Event Methods

    void on_load_project_deferred();
    void on_press_browse();
    void on_toggle_evaluate(eAFlag new_state);
    void on_toggle_sound(eAFlag new_state);

    // Inherited events from SkConsoleBase

      virtual void on_error(uint error_count = 1u);
      virtual void on_compile_complete();
      virtual void on_reparse(SkClass * class_p = nullptr);
      virtual void on_overlays_changed();
  
    // Inherited events from AWindow

      virtual bool on_close_attempt();
      virtual bool on_show_zoom_attempt(eShowZoom type);
      virtual void on_drag_drop(const AString & file_name, const AVec2i & pos);
      virtual void on_drag_drop_begin();
      virtual bool on_focus() override;
      virtual void on_menu_modal(bool enter_b);
      virtual bool on_key_press(eAKey key, bool repeated);
      virtual void on_menu_command(uint32_t item_id);
      virtual bool on_submenu_init(HMENU submenu);
      virtual void on_sizing();
      virtual void on_moving(eSpace space)      { m_log.on_moving(Space_screen); m_workspace.on_moving(Space_screen); }
      virtual bool on_draw();
      virtual bool on_hide(bool state_changed)  { m_log.on_hide(state_changed); return m_workspace.on_hide(state_changed); }

  protected:

  // Internal Methods

    static bool    cmd_arg_parse(const AString & cmd_str, uint32_t idx_begin, uint32_t * idx_end_p, uint32_t * flags_p);
    static bool    cmd_arg_parse_file(const AString & file_str, eSkLocale exec_locale);
    static bool    cmd_arg_parse_ident(const AString & ident_str, SkMemberInfo * info_p, SkOverlay ** overlay_pp = nullptr, bool * topmost_p = nullptr);
    static AString cmd_arg_parse_unquote(const AString & cmd_str, uint32_t idx_begin);
    static void    cmd_args_help();

    static bool init_class_members(SkConsole * console_p)   { ms_console_p = console_p; return true; }

  // Data Members

    bool          m_class_members_inited;
    SkCompiler *  m_compiler_p;
    AIni          m_ini_ide;
    bool          m_show_browser;
    bool          m_play_sounds;
    AString       m_sound_file_strs[Sound__count];
    ADirectory    m_template_dir;
    uint32_t      m_pref_flags;
    uint32_t      m_dot_count;
    int           m_online_txt_width;
    AImageListOS  m_member_images;
    AButtonOS     m_browse;
    ASplitterOS   m_split_text;
    SkLog         m_log;
    SkWorkspace   m_workspace;
    AEditLineOS   m_status;  // $Revisit - Change to ARichEdit with colours
    eCloseAction  m_close_action;
    SkRemoteIDE   m_remote;
    AString       m_remote_status;
    //ACheckBoxOS   m_toggle_evaluate;

    //  View Settings
    bool          m_disassembly;                // true - show disassembly, false - file view
    bool          m_expression_guide;
    bool          m_auto_parse;
    bool          m_auto_parse_sel;
    bool          m_syntax_highlight;
    bool          m_current_line_highlight;

    eSkVersionControl m_version_control;

    SkClassBrowser *          m_browser_p;
    SkSearchDialog *          m_goto_view_p;
    AFreePtr<SkClassSettings> m_classes_dlg_p;
    AFreePtr<SkAbout>         m_about_dlg_p;
    AFreePtr<SkErrorDialog>   m_error_dlg_p;

    AMethodArg<SkConsole, const SkPrintInfo &> m_print_func;

    SkProjectInfoConsole     m_project_info;

  };  // SkConsole


//=======================================================================================
// Inline Methods
//=======================================================================================



#endif  // __SKCONSOLE_HPP

