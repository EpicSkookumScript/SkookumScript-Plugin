// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AWindow class declaration header
// Notes:          Simple graphical window.
//=======================================================================================


#ifndef __AWINDOW_HPP
#define __AWINDOW_HPP
#pragma once


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO\AApplication.hpp>
#include <AgogIO\AMessageTarget.hpp>
#include <AgogGUI\AColor.hpp>
#include <AgogGUI\ADisplay.hpp>
#include <AgogGUI\AFont.hpp>
#include <AgogGUI\AMouse.hpp>
#include <AgogIO\AKeyboard.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class AToolTipOS;  // Declared in AgogGUI_OS/AToolTipOS.hpp
struct tagNMHDR;
typedef tagNMHDR NMHDR;
PREDECLARE_HANDLE(HDROP);
PREDECLARE_HANDLE(HMENU);

// Enumerated constants
enum
  {
  Size_auto = 0  // Used by *some* windows for auto width/height
  };

// AWindow Enumerated constants
enum
  {
  AWin_gui_spacing_divisor         = 2,  // Font size relative spacing divisor
  AWin_dialog_units_per_avg_char_x = 4,  // Number dialog units for average character width in current font
  AWin_dialog_units_per_avg_char_y = 8,  // Number dialog units for average character height in current font
  AWin_def_x                       = 0,
  AWin_def_y                       = 0,
  AWin_def_width                   = 800,
  AWin_def_height                  = 600
  };

//---------------------------------------------------------------------------------------
// Notes      Simple graphical window class.
// Subclasses OS controls
// UsesLibs   User32.lib, Shell32.lib   
// Author     Conan Reis
class AWindow : public AMessageTarget
  {
  public:

  // Nested Structures

    //---------------------------------------------------------------------------------------
    enum eBorder
      {                         //                       With sizing border looks like:
      Border_no_border,         //                         raised
      Border_line,              //                         raised
      Border_thin_sunken,       // Non-interactable area   thin_sunken
      Border_thin_sunken_line,  //  ^ with inner line      thin_rounded with inner line
      Border_sunken,            // Interactable area       thick_sunken
      Border_sunken_line,       //  ^ with inner line      thick rounded
      Border_thick_sunken,      //                         thick sunken
      Border_raised,            // Parent windows          raised
      Border_rounded            //                         thicker rounded
      };

    //---------------------------------------------------------------------------------------
    // Possible title bar button states
    enum eTitleButton
      {
      TitleButton_none,     // No buttons on title bar
      TitleButton_close,    // Icon(system menu) Close
      TitleButton_min_max,  // Icon(system menu) Minimize, Maximize, Close
      TitleButton_help,     // Icon(system menu) Help, Close
      TitleButton_minimize, // Icon(system menu) Minimize, Maximize(greyed), Close
      TitleButton_maximize  // Icon(system menu) Minimize(greyed), Maximize, Close
      };

    //---------------------------------------------------------------------------------------
    // Used by on_show_zoom_attempt()
    enum eShowZoom
      {
      ShowZoom_minimize,
      ShowZoom_maximize,
      ShowZoom_restore
      };

    enum eSpace
      {
      Space_client,
      Space_screen
      };

    //---------------------------------------------------------------------------------------
    // AWindow Configuration file enumerated constants
    enum eIniView
      {
      IniView_section         = 0,
      IniView_key_x           = 1,
      IniView_key_y           = 2,
      IniView_key_width       = 3,
      IniView_key_height      = 4,
      IniView_key_show_state  = 5,
      IniView_key_display     = 6,
      IniView__max,
      IniView_selection_min   = 16
      };

    //---------------------------------------------------------------------------------------
    enum eFlag
      {
      Flag_in_size_move_loop = 1 << 0,
      Flag_sizing            = 1 << 1,
      Flag_moving            = 1 << 2,
      Flag_activated         = 1 << 3,  // The active window is the top-level window the user is currently working with.  To help the user identify the active window, the system places it at the top of the Z order and highlights its title bar (if it has one) and border.  Also see the related "focused" flag.
      Flag_focused           = 1 << 4,  // A window that has input focus is the primary window receiving input - it is either the "active" window or a child of the active window.  Also see the related "activated" flag.
      Flag_keep_client_area  = 1 << 5,  // Indicates that the current client area should be retained and the window area increased to accommodate state changes rather than the other way around (which is default)
      Flag_control_events    = 1 << 6,  // If set then on_control_event_standard() is called (set by default) and modified with enable_control_event()

      Flags__default         = Flag_control_events
      };

    //---------------------------------------------------------------------------------------
    enum eScrollbar
      {
      Scrollbar_none  = 0x0,
      Scrollbar_vert  = 0x1,
      Scrollbar_horiz = 0x2,
      Scrollbar_both  = Scrollbar_vert | Scrollbar_horiz
      };

    //---------------------------------------------------------------------------------------
    // User generated application command event, for example, by clicking an application
    // command button on the mouse, keyboard or other device.  It is passed to the event
    // method on_app_command().  It is likely that only a subset of these commands (or even
    // none at all) can be sent on a particular system depending on the devices available and
    // their drivers.
    //
    // Their values are special - they match the values for the WM_APPCOMMAND command values
    // from APPCOMMAND_BROWSER_BACKWARD to APPCOMMAND_MEDIA_CHANNEL_DOWN.
    enum eAppCmd
      {
      // Browser related:
      AppCmd_browser_prev       = 1,
      AppCmd_browser_next       = 2,
      AppCmd_browser_refresh    = 3,
      AppCmd_browser_stop       = 4,
      AppCmd_browser_search     = 5,
      AppCmd_browser_favorites  = 6,
      AppCmd_browser_home       = 7,

      // Sound related
      AppCmd_volume_mute        = 8,
      AppCmd_volume_down        = 9,
      AppCmd_volume_up          = 10,
      AppCmd_bass_down          = 19,
      AppCmd_bass_boost         = 20,  // Bass boost toggle
      AppCmd_bass_up            = 21,
      AppCmd_treble_down        = 22,
      AppCmd_treble_up          = 23,
      AppCmd_mic_volume_mute    = 24,
      AppCmd_mic_volume_down    = 25,
      AppCmd_mic_volume_up      = 26,
      AppCmd_mic_toggle         = 44,  // Toggle Microphone on/off
      AppCmd_dictate_toggle     = 43,  // Toggles between two modes of speech input: dictation and command/control (giving commands to an application or accessing menus). 
      AppCmd_dictate_correct    = 45,  // Brings up the correction list when a word is incorrectly identified during speech input.

      // Media related:
      AppCmd_media_track_next   = 11,
      AppCmd_media_track_prev   = 12,
      AppCmd_media_stop         = 13,
      AppCmd_media_play_pause   = 14,  // Play or pause playback toggle.  If there are discrete Play and Pause buttons, applications should take action on this command as well as AppCmd_media_play and AppCmd_media_pause.
      AppCmd_media_select       = 16,
      AppCmd_media_play         = 46,  // Begin playing at the current position. If paused, resume. This is a direct PLAY command that has no state. If there are discrete Play and Pause buttons, applications should take action on this command as well as AppCmd_media_play_pause.
      AppCmd_media_pause        = 47,  // Pause. If already paused, take no further action.  This is a direct PAUSE command that has no state.  If there are discrete Play and Pause buttons, applications should take action on this command as well as AppCmd_media_play_pause. 
      AppCmd_media_record       = 48,
      AppCmd_media_fast_forward = 49,  // Increase playback speed. This can be implemented in many ways, for example, using a fixed speed or toggling through a series of increasing speeds. 
      AppCmd_media_rewind       = 50,  // Decrease playback speed. This can be implemented in many ways, for example, using a fixed speed or toggling through a series of decreasing speeds. 
      AppCmd_media_channel_next = 51,
      AppCmd_media_channel_prev = 52,

      // Editor related:
      AppCmd_help               = 27,  // Open the Help dialog. [F1]
      AppCmd_find               = 28,  // Open the Find dialog.
      AppCmd_new                = 29,  // Create a new window.
      AppCmd_open               = 30,  // Open a window.
      AppCmd_close              = 31,  // Close the window (not the application). [Ctrl+F4]
      AppCmd_save               = 32,  // Save current document.
      AppCmd_print              = 33,  // Print current document.
      AppCmd_undo               = 34,  // Undo last action.  [Alt+Backspace, Ctrl+Z]
      AppCmd_redo               = 35,  // Redo last action.  [Ctrl+Y]
      AppCmd_copy               = 36,  // Copy the selection. [Ctrl+Insert, Ctrl+C]
      AppCmd_cut                = 37,  // Cut the selection.  [Shift+Delete, Ctrl+X]
      AppCmd_paste              = 38,  // Paste  [Shift+Insert, Ctrl+V]
      AppCmd_spell_check        = 42,  // Initiate a spell check.

      // Mail related:
      AppCmd_mail               = 15,  // Launch mail
      AppCmd_mail_reply         = 39,
      AppCmd_mail_forward       = 40,
      AppCmd_mail_send          = 41,

      // App related:
      AppCmd_app1               = 17,  // Launch App1
      AppCmd_app2               = 18,  // Launch App2


      // Special Values:
      AppCmd__unknown           = 0,
      AppCmd__first             = 1,
      AppCmd__last              = 52
      };

    enum eFocusType
      {
      FocusType_unknown               = 0 << 0,

      FocusType_log                   = 1 << 1,     //  These can be combined with FocusType_isearch.
      FocusType_workspace             = 1 << 2,
      FocusType_editor                = 1 << 3,

      FocusType_class_tree            = 1 << 4,     //  These focus' do not have incremental search editboxes.
      FocusType_data_list             = 1 << 5,
      FocusType_code_list             = 1 << 6,

      FocusType_editsyntax            = 1 << 7,     //  General edit syntax editbox.

      FocusType_file                  = FocusType_workspace | FocusType_editor,
      FocusType_isearch               = 1 << 16,
      };

    struct View
      {
      // Public Data Members

        // Window region or 0,0,0,0 (ARegion::ms_zero) if not set.
        ARegion m_region;

        // Visibility state of a window
        eAShowState m_show_state;

      // Methods

        View(eAShowState show_state = AShowState_normal_focus)                           : m_region(AWindow::ms_region_def), m_show_state(show_state) {}
        View(const ARegion & region, eAShowState show_state = AShowState_normal_focus)   : m_region(region), m_show_state(show_state) {}
        View(const View & view)                                                          : m_region(view.m_region), m_show_state(view.m_show_state) {}

        bool is_valid() const  { return m_region.is_zero(); }

      };


  // Public Class Data

    static const ARegion ms_region_def;
    static const ARegion ms_region_auto_def;  // Only usable by *some* window types
    static const View    ms_view_def;         // ms_region_def, ShowState=AShowState_hidden
    static const View    ms_view_hidden;      // Region(0,0,0,0) ShowState=AShowState_hidden

  // Common Methods

    static void initialize();
    static void deinitialize();

    AWindow(const View & view = get_view_def(), AWindow * parent_p = nullptr, AMessageTargetClass * class_p = nullptr);
    AWindow(AWindow * parent_p, AMessageTargetClass * class_p = nullptr);
    AWindow(HWND owner_hwnd, uint32_t extended_window_style, uint32_t window_style);
    AWindow(HWND preexisting_handle);
    virtual ~AWindow();

  // Methods

    // Position / Area Methods

      // Methods that have '_rel' are relative to their parent if they have one.

      AVec2i  get_area() const;
      AVec2i  get_area_client() const;
      f32     get_aspect() const;
      f32     get_aspect_client() const;
      int     get_bottom() const;
      int     get_bottom_rel() const;
      int     get_bottom_rel_spaced() const     { return get_bottom_rel() + get_spacing(); }
      int     get_height() const;
      int     get_height_client() const;
      AVec2i  get_position() const;
      AVec2i  get_position_rel() const;
      ARegion get_region() const;
      ARegion get_region_client() const;
      ARegion get_region_normal() const;
      int     get_spacing() const               { return (m_font.get_height() / AWin_gui_spacing_divisor); }
      int     get_right() const;
      int     get_right_rel() const;
      int     get_right_rel_spaced() const      { return get_right_rel() + get_spacing(); }
      int     get_width() const;
      int     get_width_client() const;
      int     get_x() const;
      int     get_x_rel() const;
      int     get_x_rel_spaced() const          { return get_x_rel() - get_spacing(); }
      int     get_y() const;
      int     get_y_rel() const;
      bool    is_in(const AVec2i & pos) const;
      void    set_area(const AVec2i & area);
      void    set_area(int width, int height)     { set_area(AVec2i(width, height)); }
      void    set_area_client(const AVec2i & area);
      void    set_height(int height);
      void    set_order(AWindow * insert_after_p, const ARegion & region, uint32_t flags = 0u);
      void    set_position(int x, int y);
      void    set_position(const AVec2i & pos);
      void    set_pos_centered(AWindow & win)   { set_position(get_region_normal().center_on(win.get_region()).get_pos()); }
      void    set_pos_centered_display(uint id = ADisplay::ID_mouse_display);
      void    set_region(int x, int y, int width, int height);
      void    set_region(const ARegion & region);
      void    set_width(int width);
      void    set_x(int x);
      void    set_y(int y);

      void    set_area_min(const AVec2i & area = AVec2i(ADef_int, ADef_int));
      void    set_area_max(const AVec2i & area = AVec2i(ADef_int, ADef_int));

      AVec2i        xy_client2screen(const AVec2i & client_pos) const;
      AVec2i        xy_screen2client(const AVec2i & client_pos) const;
      AVec2i        client2window_area(const AVec2i & client_area);
      static AVec2i client2window_area(uint styles, uint ex_styles, bool menu_b, const AVec2i & client_area);

      static ARegion     ini_load_region(const char * section_cstr_p, const ARegion & default_region = ms_region_def, bool pos_only = false, AIni * config_file_p = AApplication::ms_ini_file_p);
      static eAShowState ini_load_show_state(const char * section_cstr_p, eAShowState default_state = AShowState_normal_focus, bool use_app_showstate = true, AIni * config_file_p = AApplication::ms_ini_file_p);
      static View        ini_load_view(const char * section_cstr_p, const View & default_view = get_view_def(), bool use_app_showstate_b = true, bool pos_only = false, AIni * config_file_p = AApplication::ms_ini_file_p);
      void               ini_restore_region(const char * section_cstr_p, const ARegion & default_region = ms_region_def, bool pos_only = false, AIni * config_file_p = AApplication::ms_ini_file_p)                    { set_region(ini_load_region(section_cstr_p, default_region, pos_only, config_file_p)); }
      void               ini_restore_show_state(const char * section_cstr_p, eAShowState default_state = AShowState_normal_focus, bool use_app_showstate = true, AIni * config_file_p = AApplication::ms_ini_file_p)  { set_show_state(ini_load_show_state(section_cstr_p, default_state, use_app_showstate, config_file_p)); }
      void               ini_restore_view(const char * section_cstr_p, const ARegion & default_region = ms_region_def, eAShowState default_state = AShowState_normal_focus, bool use_app_showstate = true, bool pos_only = false, AIni * config_file_p = AApplication::ms_ini_file_p);
      void               ini_save_view(const char * section_cstr_p, bool pos_only = false, AIni * config_file_p = AApplication::ms_ini_file_p) const;

      // Future Methods?
        //void      adjust_region(int amount);
        //void      adjust_region(int left, int top, int right, int bottom);                         
        //Rectangle get_pos_constrain() const;
        //ARegion   get_region_rel() const;      
        //void      get_region_rel(int * x_p, int * y_p, int * width_p, int * height_p) const;
        //void      set_maximized_region(const ARegion & region = [default]);
        //void      set_pos_constrain(const Rectangle & bounding = [default], bool local_xy = true);
        //void      set_area_constrain(uint32_t min_width = ADef_uint32, uint32_t min_height = ADef_uint32, uint32_t max_width = ADef_uint32, uint32_t max_height = ADef_uint32, bool local_xy = true);

    // Capability Methods

      void    enable_drag_drop(bool accept_files = true);
      void    enable_client_area_retain(bool keep_area = true);
      void    enable_control_event(bool call_events = true);
      void    enable_sizing(bool user_sizable = true);
      void    enable_topmost(bool stay_on_top = true);
      HCURSOR get_class_cursor() const;
      bool    is_sizable() const;
      bool    is_topmost() const;
      void    set_border(eBorder border = Border_raised);
      void    set_class_cursor(HCURSOR cursor_handle);
      void    set_parent(AWindow * parent_p, bool screen_relative = true);
      //bool    is_drag_drop_accepted() const;

    // Input Methods

      void enable_input(bool input_accepted = true);
      void enable_mouse_capture(bool msgs_outside_win = true);
      bool is_activated() const;
      bool is_focused() const;
      bool is_input_enabled() const;
      HWND set_focus();
      //void set_hot_key();       // RegisterHotKey
      //void set_activate_key();  // WM_SETHOTKEY

      static void enable_input_all(bool input_accepted = true, HWND skip_win = nullptr);

    // Color Methods

      void set_color_background(const AColor & color);

    // Font Methods

      virtual void  set_font(const AFont & font_p);
      const AFont & get_font() const;
      void          reset_font(bool redraw = true);

    // Display State Methods

      bool         is_hidden() const;
      bool         is_maximized() const;
      bool         is_minimized() const;
      void         show(bool activate = true);
      virtual void hide();
      void         maximize();
      void         minimize(bool activate = false);
      void         restore(bool activate = true);
      void         make_foreground();
      void         set_show_state(eAShowState show_state = AShowState_normal_focus);
      eAShowState  get_show_state() const;
      static View  get_view_def();

    // Scroll bar Messages

      void       enable_scrollbars(eScrollbar bar_flags = Scrollbar_both);
      eScrollbar get_scrollbars() const;

    // Title bar Messages

      void    enable_title_bar(bool title_bar_b = true);
      bool    is_title_bar_enabled() const;
      void    flash_title(uint flash_count = 4u, f32 interval_secs = 0.1f);
      HICON   get_icon() const;
      AString get_title() const;
      void    set_icon(LONG_PTR icon_resource_id);
      void    set_icon_file(const char * icon_file_str_p, int pixel_size = 16);
      void    set_title(const char * cstr_p);
      void    set_title_buttons(eTitleButton button_type);

    // Menu Methods

      bool is_menued() const;

    // Tool Tip Methods

      AToolTipOS * set_tool_tip(const AString & tip_text, const AFont & font = *AFont::ms_narrow_p);
      AToolTipOS * set_tool_tip(AWindow * tool_tip_p)  { m_tip_p = tool_tip_p; return (AToolTipOS *)tool_tip_p; }
      AToolTipOS * get_tool_tip() const                { return (AToolTipOS *)m_tip_p.get_obj(); }

      static void  set_tool_tip_create_func(AToolTipOS * (*tip_create_f)(AWindow * parent_p, const AString & text, const AFont & font)) { ms_tip_create_f = tip_create_f; }

    // Enumeration Methods

      static uint get_popup_count();

    //  FocusType Methods

      void        set_focus_type(eFocusType focus_type) { m_focus_type = focus_type; }
      eFocusType  get_focus_type()                      { return m_focus_type;       }

    // OS Specific Messages

      void         invalidate(const ARegion & region, bool erase_bg = true, bool draw_now = false);
      void         invalidate(bool erase_bg = true, bool draw_now = false);
      void         set_parent_handle(HWND parent_handle, bool screen_relative = true);
      virtual void refresh();


  // Event Methods
    // Each on_message method receives any needed information based from l_param and
    // w_param.  Often, the event methods must return a bool set to true if
    // DefWindowProc() should be called or false if not.
    // Many OS controls handle events themselves so some event calls will not be made in
    // these cases unless the control is sub-classed (and still calling the default
    // procedure handling if necessary) - see AMessageTarget::enable_subclass_messages()

    virtual bool on_close_attempt();
    virtual bool on_show_zoom_attempt(eShowZoom type);
    virtual void on_display_change();
    virtual void on_drag_drop(const AString & file_name, const AVec2i & pos);
    virtual void on_drag_drop_begin();
    virtual void on_drag_drop_end();
    virtual bool on_draw();
    virtual bool on_draw_background(HDC dc);
    virtual bool on_focus();
    virtual void on_focus_lost(HWND focus_window);
    virtual bool on_hide(bool state_changed);
    virtual void on_move();
    virtual void on_moving(eSpace space);
    virtual void on_moved();
    virtual bool on_set_cursor(HWND cursor_win, int hit_area, int mouse_msg);
    virtual bool on_scrollbar_horiz();
    virtual bool on_scrollbar_vert();
    virtual bool on_show(bool state_changed);
    virtual void on_size();
    virtual void on_sizing();
    virtual void on_sized();
    virtual void on_tooltip_link();

    // Menu methods - see AMenuOS and APopMenuOS
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual void on_menu_modal(bool enter_b);
    virtual void on_menu_command(uint32_t item_id);
    virtual bool on_menu_init(HMENU menu);
    virtual bool on_submenu_init(HMENU submenu);

    // User Application Command Event - by clicking an application command button on the
    // mouse, keyboard or other device. (see eAppCmd)

      virtual bool on_app_command(eAppCmd command);

    // Keyboard Events - these might not work with some controls

      virtual bool on_character(char ch, bool repeated);
      virtual bool on_key_press(eAKey key, bool repeated);
      virtual bool on_key_release(eAKey key);

    // Mouse Events - these might not work with some controls

      virtual bool on_mouse_press(eAMouse button, eAMouse buttons, const AVec2i & client_pos, bool double_click);
      virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
      virtual void on_mouse_moving(const AVec2i & client_pos, eAMouse buttons);
      virtual void on_mouse_spinning(f32 wheel_delta, eAMouse buttons, const AVec2i & client_pos);

    // OS Graphical Control Events

      virtual bool on_control_event_standard(uint32_t code);
      virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);

    // Inherited events from AMessageTarget

      virtual bool on_message(uint32_t msg, WPARAM w_param, LPARAM l_param, LRESULT * result_p);


  protected:

  // Internal Methods

    void         register_with_os(const View & view, AWindow * parent_p);
    void         common_setup();
    virtual void destroy();
    void         close_default();
    bool         parse_command(uint32_t code, uint32_t id, HWND control_handle); 
    virtual bool parse_common_message(uint32_t msg, WPARAM w_param, LPARAM l_param, LRESULT * result_p, bool * handled_p);
    bool         parse_control(NMHDR * info_p, LRESULT * result_p);
    void         parse_drag_drop(HDROP drag_handle);
    bool         parse_keyboard(uint32_t msg, WPARAM w_param, LPARAM l_param, bool * handled_p);
    bool         parse_mouse(uint32_t msg, WPARAM w_param, LPARAM l_param, bool * handled_p);
    bool         parse_sys_command(uint cmd, LPARAM l_param);

  // Data members

    AVec2i m_min_area;
    AVec2i m_max_area;
    uint   m_flags;       //  eFlag
    AFont  m_font;
    AColor m_color_bg;

    eFocusType  m_focus_type = FocusType_unknown;

    AFreePtr<AWindow> m_tip_p;

  // Class Data Members

    static const AString ms_ini_view_strs[IniView__max];
    static AToolTipOS * (*ms_tip_create_f)(AWindow * parent_p, const AString & text, const AFont & font);

    static AMessageTargetClass * ms_default_class_p;

  };  // AWindow


//=======================================================================================
// Inline Functions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Ensures that this window cannot be made any smaller (by resizing/etc.)
//             than is specified.
// Arg         area - Minimum width and height that the window can be set to.  If an axis
//             is ADef_int then the OS default is used for that axis.
// See:        set_area_max()
// Author(s):   Conan Reis
inline void AWindow::set_area_min(
  const AVec2i & area // = AVec2i(ADef_int, ADef_int)
  )
  {
  m_min_area = area;
  }

//---------------------------------------------------------------------------------------
// Ensures that this window cannot be made any larger (by resizing/etc.)
//             than is specified - including maximizing the window.
// Arg         area - Maximum width and height that the window can be set to.  If an axis
//             is ADef_int then the OS default is used for that axis.
// See:        set_area_min()
// Author(s):   Conan Reis
inline void AWindow::set_area_max(
  const AVec2i & area // = AVec2i(ADef_int, ADef_int)
  )
  {
  m_max_area = area;
  }

//---------------------------------------------------------------------------------------
// Enables/disables whether the client area is retained after a window state
//             change - enable_sizing(), enable_scrollbars(), enable_title_bar(), and
//             set_border().
// Arg         keep_area - if true, the current client area is retained and the window
//             area is increased to accomodate state change.  If false, the current
//             window area is retained and the client area is increased to accomodate
//             state change (this is normally the default state).
// Author(s):   Conan Reis
inline void AWindow::enable_client_area_retain(
  bool keep_area // = true
  )
  {
  if (keep_area)
    {
    m_flags |= Flag_keep_client_area;
    }
  else
    {
    m_flags &= ~Flag_keep_client_area;
    }
  }

//---------------------------------------------------------------------------------------
// Loads previously stored view (region & show state) from the supplied
//             configuration/.ini file, or if the values do not yet exist in the file,
//             the default values will be written to the file and returned.  If the
//             values stored in the configuration file are not appropriate (i.e. cannot
//             grab a window with the mouse) for the current screen dimensions, the
//             window is shifted towards the center of the screen and new values are
//             written to the configuration file.
// Returns:    region stored in ini file or specified default if it did not yet exist in
//             the file
// Arg         section_cstr_p - unique section for the window so that its values may be
//             distinguished from the values for other windows.
// Arg         default_view - Default view (region & show state) for the window if the
//             configuration file does not yet have any values stored in it.
// Arg         pos_only_b - if true only load (and save) position, if false also load
//             width and height
// Arg         config_file_p - Pointer to a configuration/.ini file to use.
// See:        ini_load_show_state(), ini_restore_region(), ini_restore_show_state(),
//             ini_restore_view(), ini_save_view()
// Modifiers:   static
// Author(s):   Conan Reis
inline AWindow::View AWindow::ini_load_view(
  const char * section_cstr_p,
  const View & default_view,        // = View()
  bool         pos_only_b,          // = false
  bool         use_app_showstate_b, // = true
  AIni *       config_file_p        // = AApplication::ms_ini_file_p
  )
  {
  return View(
    ini_load_region(section_cstr_p, default_view.m_region, pos_only_b, config_file_p),
    ini_load_show_state(section_cstr_p, default_view.m_show_state, use_app_showstate_b, config_file_p));
  }

//---------------------------------------------------------------------------------------
// Loads previously stored region from the supplied configuration/.ini file,
//             or if the values do not yet exist in the file, the default values will be
//             written to the file and returned.  If the values stored in the
//             configuration file are not appropriate (i.e. cannot grab a window with
//             the mouse) for the current screen dimensions, the window is shifted
//             towards the center of the screen and new values are written to the
//             configuration file.
// Returns:    region stored in ini file or specified default if it did not yet exist in
//             the file
// Arg         section_cstr_p - unique section for the window so that its values may be
//             distinguished from the values for other windows.
// Arg         default_region - Default region for the window to occupy if the
//             configuration file does not yet have any values stored in it.
// Arg         default_state - Default show state for the window if the configuration
//             file does not yet have any values stored in it.
// Arg         use_app_showstate - if true, the show state specified by the
//             run/execution starting values as indicated by WinMain() will override the
//             value in the configuration file - if overridden the window will generally
//             minimized or maximized.
// Arg         pos_only - if true only load (and save) position, if false also load
//             width and height
// Arg         config_file_p - Pointer to a configuration/.ini file to use.
// See:        ini_load_region(), ini_load_show_state(), ini_restore_region(),
//             ini_restore_show_state(), ini_save_view()
inline void AWindow::ini_restore_view(
  const char *    section_cstr_p,
  const ARegion & default_region,    // = ARegion(0, 0, 800, 600)
  eAShowState     default_state,     // = AShowState_normal_focus
  bool            use_app_showstate, // = true
  bool            pos_only,          // = false
  AIni *          config_file_p      // = &AIni()
  )
  {
  set_region(ini_load_region(section_cstr_p, default_region, pos_only, config_file_p));
  set_show_state(ini_load_show_state(section_cstr_p, default_state, use_app_showstate, config_file_p));
  }

//---------------------------------------------------------------------------------------
// Returns whether the window is the "active" window or not.  The active
//             window is the top-level window the user is currently working with.  To
//             help the user identify the active window, the system places it at the top
//             of the Z order and highlights its title bar (if it has one) and border.
// Returns:    true if focused, false if not
// See:        is_focused(), is_input_enabled()
// Author(s):   Conan Reis
inline bool AWindow::is_activated() const
  {
  return (m_flags & Flag_activated) == Flag_activated;
  }

//---------------------------------------------------------------------------------------
// Returns whether the window is focused or not.  A window that has focus is
//             the primary window receiving input (usually most specifically the keyboard)
//             - it is either the "active" window or a child of the active window.
// Returns:    true if focused, false if not
// See:        is_activated(), is_input_enabled()
// Author(s):   Conan Reis
inline bool AWindow::is_focused() const
  {
  return (m_flags & Flag_focused) == Flag_focused;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline const AFont & AWindow::get_font() const
  {
  return m_font;
  }

//---------------------------------------------------------------------------------------
// Returns a default view based on the specified display
// Returns:    View
// Modifiers:   static
// Author(s):   Conan Reis
inline AWindow::View AWindow::get_view_def()
  {
  return View(ADisplay::pivot_region(ms_region_def), AShowState_hidden);
  }

//---------------------------------------------------------------------------------------
// Centers the window on the specified display
// Arg         id - Id of display to center this window on.  ID_primary & ID_mouse_display
//             are also valid ids.
// Author(s):   Conan Reis
inline void AWindow::set_pos_centered_display(
  uint id // = ADisplay::ID_mouse_display
  )
  {
  ARegion region(get_region_normal());

  ADisplay::center(&region, id);

  set_position(region.get_pos());
  }

//---------------------------------------------------------------------------------------
// Sets the background color for the window.
// 
// #See Also  on_draw_background()
// #Author(s) Conan Reis
inline void AWindow::set_color_background(
  // Solid color to use when redrawing the window's background.
  // Uses default color when set to AColor::ms_default.
  // Background not redrawn when set to AColor::ms_void.
  const AColor & color
  )
  {
  m_color_bg = color;
  }


#endif // __AWINDOW_HPP


