// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AEditOS class declaration header
//=======================================================================================


#ifndef __AEDITOS_HPP
#define __AEDITOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AEditLineOS.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      An AEditOS is a built-in edit control from the operating system.  It is a
//            rectangular control window typically used in a dialog box to permit the
//            user to enter and edit multiple lines of text by typing on the keyboard.
// Subclasses 
// See Also   AEditLineOS
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, User32.lib
// Inlibs     AgogGUI_OS\AgogGUI_OS.lib
// Author(s)  Conan Reis
class AEditOS : public AEditLineOS
  {
  public:

  // Unhiding Inherited Methods

    // Methods in this class with the same name as methods in superclasses are 'hidden'
    // (even if they do not have the same interface), this makes them visible again.
    // These using directives must precede the new methods in this class.
    using AEditLineOS::get_text;

  // Common Methods

    AEditOS(AWindow * parent_p, const AString & initial = AString::ms_empty, bool word_wrap = true, const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_auto_def, bool select_always = true, bool register_os = true);
    
  // Methods

    // General Setting Methods

    void enable_culling(bool truncate_old = true);
    void enable_vertical_scroll(bool no_fixed_height = true);
    void set_break_conversion(eALineBreak expected);
    void set_tabs_fixed_spaces(uint32_t space_count);

    // Inherited from AEditLineOS

      virtual void set_char_limit(uint32_t max_chars);
      //void         enable_read_only(bool read_only = true);
      //void         enable_horizontal_scroll(bool virtual_width_or_disable_word_wrapping = true);
      //bool         is_modified() const;
      //bool         is_read_only() const;
      //int          get_char_limit() const;
      //void         set_alignment(eEditAlignment type);
      //void         set_conversion(eAEditConvert type);
      //void         set_char_limit(uint32_t length);
      //void         set_modified(bool modified = true);

      // Yet to be implemented for AEditLineOS

      //void   enable_auto_resize(bool auto_resize = true);
      //void   enable_background(bool draw = true);
      //void   enable_revert_mode(bool escape_revert = true);
      //bool   is_auto_resizing() const;
      //bool   is_revert_mode() const;
      //Colour get_colour() const;
      //void   set_colour(Colour & colour);
      //void   set_virtual_region(const ARegion & region);
      //??? get & set margins

    // Selection Methods

    void    get_selection_or_row(AString * str_p) const;
    AString get_selection_or_row() const                     { AString str; get_selection_or_row(&str); return str; }
    void    select_row(uint32_t row);
    void    select_rows(uint32_t from_row, uint32_t to_row);

    // Inherited from AEditLineOS

      void         get_selection_rows(uint32_t * row_begin_p, uint32_t * row_end_p) const;
      virtual void replace_selection(const AString & replace_str, bool undoable = false);
      //void       deselect();
      //void       enable_selection_hiding(bool focus_lost_hide = true);
      //void       extend_selection(uint32_t index);
      //void       get_selection(uint32_t * start_p, uint32_t * end_p) const;
      //AString    get_selection() const;
      //void       get_selection(AString * str_p) const;
      //bool       is_selected() const;
      //AString    pop_selection(bool undoable = false);
      //void       pop_selection(AString * str_p, bool undoable = false);
      //void       remove_selection(bool undoable = false);
      //void       select(uint32_t start, uint32_t end);
      //void       select_all();

    // Text Methods

    virtual void convert_from_edit_linebreaks(AString * str_p) const;
    virtual void convert_to_edit_linebreaks(AString * str_p) const;

    void         cull_char_rows(uint32_t min_chars, bool undoable = false);
    AString      get_row(uint32_t row) const;
    void         get_row(uint32_t row, AString * str_p) const;
    void         remove_row(uint32_t row, bool undoable = false);
    void         remove_rows(uint32_t from_row, uint32_t to_row, bool undoable = false);

      // Inherited from AEditLineOS

      virtual void get_text(AString * str_p) const;
      virtual void get_text(uint32_t start, uint32_t end, AString * str_p) const;
      virtual void set_text(const AString & str, bool undoable = false);

      //void    append(const AString & str, bool undoable = false);
      //void    empty(bool undoable = false);
      //AString get_text() const;
      //AString get_text(uint32_t start, uint32_t end) const;
      //bool    is_empty() const;

    // Positioning & Hit Testing Methods

    uint32_t         get_index_visible_first() const;
    uint32_t         get_index_visible_last() const;
    virtual uint32_t get_index_from_pos(const AVec2i & client_pos) const;
    uint32_t         get_index_from_row(uint32_t row) const;
    uint32_t         get_index_home_nonspace(uint32_t index) const;
    void             set_index_first(uint32_t index);

    uint32_t         get_row_count() const;
    uint32_t         get_row_caret() const;
    uint32_t         get_row_visible_first() const;
    uint32_t         get_row_char_count(uint32_t row) const;
    virtual bool     get_row_from_pos(const AVec2i & client_pos, uint32_t * row_p = nullptr) const;
    virtual uint32_t get_row_from_index(uint32_t index) const;
    //uint32_t       get_row_visible_count() const;
    //void           set_row_first(uint32_t row);

    uint         get_row_index_from_index(uint idx) const;
    uint         get_column_from_index(uint idx, uint tab_stops = AString_tab_stop_def) const;
    void         set_column_first(uint32_t column);
    uint32_t     toggle_caret_home_nonspace();
    //uint32_t     get_column_first() const;

    //void set_word_break_func(AFunctionArgBase<WordBreakInfo *> * word_break_func_p);

      // Inherited from AEditLineOS
      //uint32_t get_caret_index() const;
      //void get_index_position(uint32_t index, int * x_p, int * y_p) const;
      //uint32_t get_length() const;
      //void ensure_visible_caret();
      //bool get_position_index(int x_pos, int y_pos, uint32_t * index_p = nullptr) const;
      //void set_caret_index(uint32_t index);

      // Yet to be implemented for AEditLineOS
      //uint32_t get_first_index() const;
      //void ensure_visible(uint32_t index);
      //void identify_word(uint32_t index, uint32_t * start_p, uint32_t * end_p) const;
      //void resize();

    // Clipboard Methods

      void clipboard_copy_plain_sel_or_row();

    // Undo and Redo Methods

      // Inherited from AEditLineOS

      //void undo();
      //void clear_undo();

      // Yet to be implemented for AEditLineOS
      //void redo();
      //uint32_t get_current_undo_depth() const;
      //int  get_undo_depth() const;
      //void set_undo_depth(int depth = -1);
      //void revert();

    // Methods Nice to have, but hard (if at all possible) to implement
    //#void         enable_insert_mode(bool insert = true);
    //#bool         is_insert_mode() const;
    //#bool         is_whitespace_enabled() const;
    //#virtual void on_abort();
    //#virtual void on_accept();

  protected:
  // Event Methods

    virtual void on_vertical_scroll();

      // Inherited from AEditLineOS
      //virtual void on_exceeded_char_limit();
      //virtual void on_horizontal_scroll();
      //virtual void on_insufficient_memory();
      //virtual void on_modified();
      //virtual void on_predraw();

      // Inherited events from AWindow

      virtual bool on_control_event_standard(uint32_t code);
      //virtual bool on_focus();
      //virtual bool on_focus_lost();

  // Data Members

    // Indicates whether the line break convention used by the control should be converted
    // (as needed) to some other specified line break convention.  In other words all
    // text strings passed to this control should be in this form and all text strings
    // returned from this control will be in this form.
    // If it is set to ALineBreak__default then the text passed in and returned will be
    // the native convention used by this control - /r/n for AEditOS and /r for ARichEdit
    // controls respectively.
    // The initial value is ALineBreak_unix which is the standard for the Agog library.
    eALineBreak m_breaks_expected;

    bool m_culling;
    uint32_t m_char_limit;

  };  // AEditOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Gets the string of characters from the specified line (not including any
//             newline or carriage return)
// Arg         row - row to retrieve
// See:        get_row_char_count()
// Author(s):   Conan Reis
inline AString AEditOS::get_row(uint32_t row) const
  {
  AString str;

  get_row(row, &str);

  return str;
  }


#endif  // __AEDITOS_HPP


