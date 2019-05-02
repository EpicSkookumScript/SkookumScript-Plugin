// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Browser & supporting classes
//=======================================================================================


#ifndef __SKCLASSBROWSER_HPP
#define __SKCLASSBROWSER_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO/AKeyboard.hpp>
#include <AgogCore/AMethodArg.hpp>
#include <AgogCore/AObjReusePool.hpp>
#include <AgogGUI_OS/AComboBoxOS.hpp>
#include <AgogGUI_OS/AListOS.hpp>
#include <AgogGUI_OS/ATabViewOS.hpp>
#include <AgogGUI_OS/ATreeOS.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumIDE/SkConsole.hpp>
#include <SkookumIDE/SkNavigationView.hpp>
#include <SkookumIDE/SkSearchDialog.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
class SkConsole;
class SkMemberView;
class SkEditView;
class SkClassBrowser;

namespace Gdiplus
  {
  class Graphics;
  }

// These are indexes into the member images (their order is important)
enum eSkMemberImage
  {
  SkMemberImage_none,
  SkMemberImage_debug_break,
  SkMemberImage_debug_break_disabled,
  SkMemberImage_debug_break_child,
  SkMemberImage_debug_child,
  SkMemberImage_debug_child_disabled,
  SkMemberImage_scope_instance,
  SkMemberImage_scope_class,
  SkMemberImage_method,
  SkMemberImage_method_atomic,
  SkMemberImage_coroutine,
  SkMemberImage_coroutine_atomic,
  SkMemberImage_data,
  SkMemberImage_class
  };

extern eSkMemberImage g_member_type_to_image[SkMember__length];

// Always show error types as custom rather than non-atomic
extern eSkMemberImage g_member_type_to_error_image[SkMember__length];


//---------------------------------------------------------------------------------------
struct SkMemberReference : public SkContextInfo
  {
  // Nested Data-structures

    enum eStatus
      {
      Status_valid,
      Status_error,
      Status_uninitialized
      };

  // Public Data

    eStatus m_status;

  // Methods

    SkMemberReference() : m_status(Status_uninitialized)            {}

    SkMemberReference & operator=(const SkMemberReference & ref)    { SkContextInfo::operator=(ref); m_status = ref.m_status; return *this; }
    void init(const SkContextInfo & member_info, eStatus status)    { SkContextInfo::operator=(member_info); m_status = status; }



  // Class Methods

    static void                               pool_delete(SkMemberReference * info_p);
    static AObjReusePool<SkMemberReference> & get_pool();
    static SkMemberReference *                pool_new(const SkContextInfo & member_info, eStatus status);
    SkMemberReference **                      get_pool_unused_next() { return (SkMemberReference **)m_member_id.get_pool_unused_next(); } // Area in this class where to store the pointer to the next unused object when not in use

  };  // SkMemberReference


typedef AColumnOS<SkMemberReference>::SubItemText tSkSubText;


//---------------------------------------------------------------------------------------
class SkDataList : public AListOS<SkMemberReference>
  {
  public:
  // Methods

    SkDataList(AWindow * parent_p, SkMemberView * member_view_p);
    virtual ~SkDataList();

    void set_class(SkClass * class_p);

  protected:

  // Internal Methods

    void append_member(bool class_scope, SkClass * class_p, const ASymbol & name);

  // Event Methods

    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_key_press(eAKey key, bool repeated);
    virtual void on_item_focused(SkMemberReference * item_p, uint row);
    virtual void on_item_removed_row(uint row, uintptr_t item_info);
    virtual bool on_focus();

    virtual LRESULT on_custom_draw(NMLVCUSTOMDRAW * info_p);
    void            on_text_name(tSkSubText * info_p);
    void            on_text_scope(tSkSubText * info_p);
    void            on_text_type(tSkSubText * info_p);
    eAEquate        on_compare_instance(const SkMemberReference & lhs, const SkMemberReference & rhs);
    eAEquate        on_compare_name(const SkMemberReference & lhs, const SkMemberReference & rhs);
    eAEquate        on_compare_scope(const SkMemberReference & lhs, const SkMemberReference & rhs);
    eAEquate        on_compare_type(const SkMemberReference & lhs, const SkMemberReference & rhs);

  // Data Members

    SkMemberView * m_member_view_p;

    AColumnOS<SkMemberReference> * m_col_instance_p;
    AColumnOS<SkMemberReference> * m_col_scope_p;
    AColumnOS<SkMemberReference> * m_col_name_p;
    AColumnOS<SkMemberReference> * m_col_type_p;

  };  // SkDataList


//---------------------------------------------------------------------------------------
class SkCodeList : public AListOS<SkMemberReference>
  {
  public:

  // Nested Data-structures

    enum eColumn
      {
      Column_scope,  // class instance or class scope (i/c)
      Column_class,  // class scope belonged to (Scope)
      Column_name,   // member name
      Column_params  // parameters / interface
      };

  // Methods

    SkCodeList(AWindow * parent_p, SkMemberView * member_view_p);
    ~SkCodeList();

    void append_member(const SkContextInfo & info, SkMemberReference::eStatus status = SkMemberReference::Status_valid);
    void set_class(SkClass * class_p);
    void sort()                                             { m_col_name_p->sort(); }

  protected:

  // Internal Methods

    void append_invokables(SkInvokableBase ** members_pp, uint length, eSkInvokable custom_type, bool show_custom, bool show_atomic, bool class_scope);

  // Event Methods

    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_key_press(eAKey key, bool repeated);
    virtual void on_subitem_clicked(uint row, uint rank, eAMouse button, bool double_click);
    virtual void on_item_focused(SkMemberReference * item_p, uint row);
    virtual void on_item_removed_row(uint row, uintptr_t item_info);
    virtual bool on_focus();

    virtual LRESULT on_custom_draw(NMLVCUSTOMDRAW * info_p);
    void            on_text_name(tSkSubText * info_p);
    void            on_text_scope(tSkSubText * info_p);
    void            on_text_parameters(tSkSubText * info_p);
    eAEquate        on_compare_instance(const SkMemberReference & lhs, const SkMemberReference & rhs);
    eAEquate        on_compare_name(const SkMemberReference & lhs, const SkMemberReference & rhs);
    eAEquate        on_compare_scope(const SkMemberReference & lhs, const SkMemberReference & rhs);

  // Internal Class Methods

    static eSkMember unify_type(eSkMember type);

  // Data Members

    SkMemberView * m_member_view_p;

    AColumnOS<SkMemberReference> * m_col_instance_p;
    AColumnOS<SkMemberReference> * m_col_name_p;
    AColumnOS<SkMemberReference> * m_col_scope_p;

  };  // SkCodeList


//---------------------------------------------------------------------------------------
class SkMemberView : public AWindow
  {
  public:

    friend class SkDataList;
    friend class SkCodeList;

  // Methods

    SkMemberView(AWindow * parent_p, SkClassBrowser * browser_p);
    ~SkMemberView();

    void ensure_members_listed(const SkContextInfo & member_info);
    void set_class(SkClass * class_p);
    void set_member(const SkContextInfo & member_info);
    void select_member(const SkContextInfo & member_info);
    void refresh_members();

    bool is_inherited_shown()  { return m_toggle_inherited.is_toggled(); }

    void load_settings();
    void save_settings();

    SkCodeList &         get_code_list()            { return m_code_list; }
    SkDataList &         get_data_list()            { return m_data_list; }
    ASplitterOS &        get_splitter()             { return m_splitter; }

    void                pool_delete_info(SkMemberReference * info_p)  { m_members.remove(*info_p); SkMemberReference::pool_delete(info_p); }
    SkMemberReference * pool_new_info(const SkContextInfo & member_info, SkMemberReference::eStatus status = SkMemberReference::Status_valid);

  // Public Events

    void on_member_selected(SkMemberReference * member_p);

  protected:

  // Event Methods

    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_draw();
    virtual void on_sizing();

    void on_toggle_member_kind(eAFlag state);

  // Data Members

    SkClassBrowser * m_browser_p;
    SkClass *        m_class_p;

    AMethodArg<SkMemberView, eAFlag> m_on_toggle_member_kind;

    // Filter UI Objects

      RECT m_filter_rect;

      // Scope Checkboxes: instance, class, or inherited
      ACheckBoxOS m_toggle_instance;
      ACheckBoxOS m_toggle_class;
      ACheckBoxOS m_toggle_inherited;

      // Code Type Checkboxes
      ACheckBoxOS m_toggle_script;
      ACheckBoxOS m_toggle_cpp;

      // Member Type Checkboxes
      ACheckBoxOS m_toggle_methods;
      ACheckBoxOS m_toggle_coroutines;

    // Data List / Code List
    ASplitterOS m_splitter;

    // Data member list
    SkDataList m_data_list;

    // Routine list (methods & coroutines)
    SkCodeList m_code_list;

    // Used for SkQualifier to SkMemberReference look-up
    APSortedLogical<SkMemberReference, SkQualifier> m_members;

  };  // SkMemberView


//---------------------------------------------------------------------------------------
class SkEditor : public SkEditSyntax
  {
  public:

    SkEditor(SkEditView * edit_view_p, const AString & initial = AString::ms_empty);

    bool ensure_writable(bool query_user = true);
    bool is_source_available() const                { return m_source_available; }
    uint caret_index_to_file() const                { return get_caret_index() + get_row_caret(); }

    SkExpressionBase * get_caret_expr() const;

    virtual const AFile & get_source_file() const   { return m_source_file; }
    virtual void          set_source(const AString & str, bool read_only_b = false);
    void                  set_source_file(AFile * source_p = nullptr);
    virtual void          save_settings(eSkLog log = SkLog_ide_print);

    void draw_expr_idx_span(uint32_t idx, const AColor & color, Gdiplus::Graphics & graphics);
    void draw_expr_mark(SkExpressionBase & expr, const AColor & color, Gdiplus::Graphics & graphics);
    void draw_expr_span(SkInvokableBase & invokable, SkExpressionBase & expr, const AColor & color, Gdiplus::Graphics & graphics);

  protected:

  // Event Methods

    // Inherited events from SkEditSyntax

      virtual void on_draw_subpart(Gdiplus::Graphics & graphics);

    // Inherited events from ARichEditOS

      virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
      virtual bool on_key_press(eAKey key, bool repeated);

    // Inherited events from AEditLineOS

      virtual void on_modified();

    // Inherited events from AWindow

      virtual bool on_focus() override;
      virtual void on_focus_lost(HWND focus_window) override;
      virtual bool on_character(char ch, bool repeated) override;

  // Data Members

    SkEditView *    m_edit_view_p;
    SkAutoComplete  m_auto_complete;

    AFile m_source_file;
    bool  m_source_available;
  };  // SkEditor


//---------------------------------------------------------------------------------------
struct SkEditInfo : public SkContextInfo, public AListNode<SkEditInfo>
  {
  // Methods

    SkEditInfo(const SkContextInfo & info, uint32_t idx_begin, uint32_t idx_end)   : SkContextInfo(info), m_sel_idx_begin(idx_begin), m_sel_idx_end(idx_end), m_scroll_pos(-1, -1) {}
    SkEditInfo(const SkEditInfo & info)                                    : SkContextInfo(info), m_sel_idx_begin(info.m_sel_idx_begin), m_sel_idx_end(info.m_sel_idx_end), m_scroll_pos(info.m_scroll_pos) {}
    SkEditInfo & operator=(const SkEditInfo & info)                        { SkContextInfo::operator=(info); m_sel_idx_begin = info.m_sel_idx_begin; m_sel_idx_end = info.m_sel_idx_end; m_scroll_pos = info.m_scroll_pos; return *this; }

    bool operator==(const SkEditInfo & info) const                         { return compare_files(info); }

  // Data Members

    // Uses native edit indexes rather than file indexes
    uint32_t m_sel_idx_begin;
    uint32_t m_sel_idx_end;
    AVec2i   m_scroll_pos;

  };  // SkMatchReference


//---------------------------------------------------------------------------------------
class SkHistoryPicker : public AComboBoxOS
  {
  public:

  // Methods

    SkHistoryPicker(AWindow * parent_p, const AFont & font);

  // Events

    virtual void on_selected(uint row, uint row_prev);
    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);

  };


//---------------------------------------------------------------------------------------
class SkEditView : public AWindow
  {
  public:

    friend class SkEditor;

  // Methods

    SkEditView(AWindow * parent_p, SkClassBrowser * browser_p);
    ~SkEditView();

    bool          is_modified() const          { return m_modified; }
    void          set_modified(bool modified)  { m_modified = modified; }
    void          focus_editor()               { m_edit.set_focus(); }
    SkEditor &    get_editor()                 { return m_edit; }
    const AFile & get_source_file() const      { return m_edit.get_source_file(); }
    void          edit_externally()            { m_edit.action_edit_externally(); }
    void          save_changes()               { m_edit.save_settings(); }
    void          toggle_breakpoint();
    void          toggle_breakpoint_enable();
    
  // Member Methods
      const SkContextInfo & get_member_info() const      { return m_member_info; }
      AString               get_member_name() const;
      void                  set_member(const SkEditInfo & info, bool select_scroll = true);
      void                  set_member(const SkContextInfo & member_info, uint index_start = 0u, uint index_end = ADef_uint32);
      void                  member_invalidate();
      void                  refresh_member();
      void                  refresh_annotations(bool visual_refresh = true);
      void                  sync_browser();

      void                  history_next();
      void                  history_prev();
      bool                  is_history_next() { return m_current_info_p && m_history.get_next_null(m_current_info_p); }
      bool                  is_history_prev() { return m_current_info_p && m_history.get_prev_null(m_current_info_p); }
      void                  history_jump(SkEditInfo * info_p);
      void                  history_clear();

  // Event Methods

    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual void on_sizing();
    virtual void on_moving(eSpace space)      { m_edit.on_moving(Space_screen); }
    virtual bool on_hide(bool state_changed)  { return m_edit.on_hide(state_changed); }

    //void on_toggle_auto_parse(eAFlag state);

    tSkBreakPoints  m_bps_on_member;

  protected:

  // Data Members

    SkClassBrowser * m_browser_p;

    SkContextInfo     m_member_info;
    SkEditInfo *      m_current_info_p;
    AList<SkEditInfo> m_history;
    uint32_t          m_history_count;

    AButtonOS       m_btn_sync;
    SkHistoryPicker m_history_combo;
    AButtonOS       m_btn_goto;
    SkEditor        m_edit;

    bool            m_modified;

  };  // SkEditView


//---------------------------------------------------------------------------------------
class SkClassBrowser : public SkMainWindowBase
  {
    friend class SkMainWindowBase;

  public:

  // Public Class Data Members

    static SkClassBrowser * ms_browser_p;

  // Types

    struct SingletonSetter
      {
      SingletonSetter(SkClassBrowser * browser_p)                   { SkClassBrowser::ms_browser_p = browser_p; }
      ~SingletonSetter()                                            { SkClassBrowser::ms_browser_p = nullptr; }
      };

  // Methods

    SkClassBrowser(SkConsole * console_p);
    ~SkClassBrowser();

    const SkContextInfo & get_member_info() const                   { return m_edit_view.get_member_info(); }
    const AFile &         get_member_file() const                   { return m_edit_view.get_source_file(); }
    SkEditView &          get_edit_view()                           { return m_edit_view; } 
    SkMemberView &        get_member_view()                         { return m_member_view; }
    SkCodeList &          get_routine_list()                        { return m_member_view.get_code_list(); }
    SkDataList &          get_data_list()                           { return m_member_view.get_data_list(); }
    SkClassTree &         get_class_tree()                          { return m_navigation_view.get_class_tree(); }
    SkNavigationView &    get_navigation_view()                     { return m_navigation_view; }

    void enable_debug_preference(SkDebug::ePrefFlag perf_flag, bool enable = true);
    void refresh_member()                                           { m_edit_view.refresh_member(); }
    void set_member(const SkContextInfo & member_info, uint index_start = 0u, uint index_end = ADef_uint32);
    void focus_editor()                                             { m_edit_view.focus_editor(); }
    void set_class(SkClass * class_p)                               { m_navigation_view.set_class(class_p); m_member_view.set_class(class_p); }

    void update_title();
    void status_update();
    void status_set_text(const AString & text)                      { m_status.set_text(text); }
    void set_overlay_current(const AString & overlay_str)           { m_overlay_current_name = overlay_str; }

    void unhook();
    void rehook();

    void         load_settings(eLoadView load_view = LoadView_restore);
    virtual void save_settings() override;

  // Public Events

    void         on_goto()                                          { m_console_p->display_goto_dialog(); }

    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual bool on_key_press(eAKey key, bool repeated);
    virtual void on_menu_command(uint32_t item_id);
    virtual bool on_submenu_init(HMENU menu_handle);
    void         on_class_selected(SkClass * class_p, bool show_meta = true);
    void         on_member_selected(SkMemberReference * member_p);

  // Class Methods

    static AString get_ini_user_name();

  protected:

  // Event Methods

    // Inherited events from AWindow

    virtual bool on_close_attempt();
    virtual bool on_focus() override;
    virtual void on_menu_modal(bool enter_b);
    virtual void on_sizing();
    virtual void on_moving(eSpace space)      { m_edit_view.on_moving(Space_screen); }
    virtual bool on_hide(bool state_changed)  { return m_edit_view.on_hide(state_changed); }

  // Data Members

    SingletonSetter  m_singleton_setter;  // Ensures ms_default_p is set is soon as possible.
    AString          m_overlay_current_name;
    SkConsole *      m_console_p;
    AEditLineOS      m_status;            // $Revisit - Change to ARichEdit with colours
    ASplitterOS      m_split_main;        // Secondary splitter / Member View
    ASplitterOS      m_split_secondary;   // Class Tree / Edit View
    ATabViewOS       m_member_debug_tabs;
    SkMemberView     m_member_view;
    SkNavigationView m_navigation_view;
    SkEditView       m_edit_view;
    //AWindow          m_call_stack;

  };  // SkClassBrowser


//=======================================================================================
// SkMemberReference Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Frees up a object and puts it into the dynamic pool ready for its next
//              use.  This should be used instead of 'delete' because it  prevents
//              unnecessary deallocations by saving previously allocated objects.
// See:         pool_new()
// Notes:       To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:    static
// Author(s):    Conan Reis
inline void SkMemberReference::pool_delete(SkMemberReference * info_p)
  {
  get_pool().recycle(info_p);
  }


//=======================================================================================
// SkCodeList Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline eSkMember SkCodeList::unify_type(eSkMember type)
  {
  switch (type)
    {
    case SkMember_method_func:
    case SkMember_method_mthd:   return SkMember_method;
    case SkMember_coroutine_func:
    case SkMember_coroutine_mthd: return SkMember_coroutine;
    default:                     return type;
    }
  }


#endif  // __SKCLASSBROWSER_HPP

