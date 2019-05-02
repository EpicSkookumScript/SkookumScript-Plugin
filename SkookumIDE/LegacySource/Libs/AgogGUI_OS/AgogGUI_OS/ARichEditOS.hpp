// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARichEditOS class declaration header
//=======================================================================================


#ifndef __ARICHEDITOS_HPP
#define __ARICHEDITOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AEditOS.hpp>
#include <AgogGUI\AColor.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Underline types for text style
enum eAUnderline
  {
  AUnderline__unchanged,  // Leave current underline type unchanged
  AUnderline_none,        // No underline
  AUnderline_solid,
  AUnderline_thick,       // Thick solid underline
  AUnderline_wave,        // Wavy line - like Word uses to mark spelling mistakes
  AUnderline_dotted,
  AUnderline_dash,        // Long dash - it will typically cover 1.5 to 2 letters at once
  AUnderline_dash_dot,
  AUnderline_dash_dot_dot
  };

// Text Effects
enum eAText
  {
  AText_italics     = 1 << 0,
  AText_bold        = 1 << 1,  // [Note that this setting will increase the width of monospace fonts.]
  AText_strikeout   = 1 << 2,
  AText_disabled    = 1 << 3,  // Greys out text and gives it a dropshadow.  [If the text is any colour other than the user default, this setting will be ignored.]
  AText_superscript = 1 << 4,  // Only one of subscript or super script may be set at one time
  AText_subscript   = 1 << 5,  // [Note that both superscript and subscript will decrease the width of monospace fonts.]

  // Masks
  AText__none = 0x0,
  AText__all  = AText_italics | AText_bold | AText_strikeout | AText_disabled | AText_superscript | AText_subscript
  };

// Text Style info structure
struct ATextStyle
  {
  // Nested Structures

  // Public Data

    const AColor * m_font_color_p;
    const AColor * m_background_color_p;
    eAUnderline    m_underline;
    uint32_t       m_effect_mask;
    uint32_t       m_effect_flags;
    //AString        m_font_name;
    //uint32_t       m_font_height;  // Character height, in twips (1/1440 of an inch or 1/20 of a printer's point).

  // Methods

    ATextStyle();
    ATextStyle(const AColor & font_color, const AColor & background_color = AColor::ms_void, eAUnderline underline = AUnderline__unchanged, uint32_t effect_mask = AText__none, uint32_t effect_flags = AText__none);
    ATextStyle(const AColor & font_color, uint32_t effect_mask, uint32_t effect_flags = AText__none);
    ATextStyle(eAUnderline underline, uint32_t effect_mask = AText__none, uint32_t effect_flags = AText__none);
    ATextStyle(uint32_t effect_mask, uint32_t effect_flags);

    ATextStyle(const ATextStyle & style);
    ATextStyle & operator=(const ATextStyle & style)  { A_ERRORX("Cannot assign ATextStyle objects to each other."); return *this; }
  };

//---------------------------------------------------------------------------------------
// Notes      An ARichEditOS is a built-in edit control from the operating system.  It is
//            a rectangular control window typically used in a dialog box to permit the
//            user to enter and edit multiple lines of text by typing on the keyboard.
// See Also   AEditLineOS
// Author(s)  Conan Reis
class ARichEditOS : public AEditOS
  {
  public:

  // Nested Structures

    // Used by m_rich_flags
    enum eRichFlag
      {
      RichFlag_none                     = 0x0,
      RichFlag_word_wrap                = 1 << 0,  // Specifies whether word wrapping should be enabled or not. Note word wrapping is incompatible with enabling the horizontal scrollbar.
      RichFlag_show_selection_always    = 1 << 1,  // If set to true, whenever there is a selection, it is shown regardless of whether this is the active window or not. If set to false, the selection is shown only when this is the active window.
      RichFlag_single_line              = 1 << 2,  // Single line edit box line AEditline though with all the features of ARichEdit
      RichFlag_ignore_control_event     = 1 << 3,  // If set events that are enabled via enable_events() are temporarily ignored.  Has no effect if this control is subclassed.
      RichFlag_ignore_selection_change  = 1 << 4,  // If set on_selecting() will not be called

      RichFlag__default = RichFlag_word_wrap | RichFlag_show_selection_always
      };


  // Reveal Hidden Inherited Methods

    // Methods in this class with the same name as methods in superclasses are 'hidden'
    // (even if they do not have the same interface), this makes them visible again.
    // These using directives must precede the new methods in this class.
    using AEditOS::get_selection;
    using AEditOS::get_text;


  // Common Methods

    static void initialize();
    static void deinitialize();

    ARichEditOS(AWindow * parent_p, const AString & initial = AString::ms_empty, uint32_t flags = RichFlag__default, const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_auto_def);
    ~ARichEditOS();

  // Methods

    // Text Style Methods

      void set_text_background(const AColor & color = AColor::ms_default);
      void set_text_style(const ATextStyle & style);
      void set_text_style(uint32_t start, uint32_t end, const ATextStyle & style, bool preserve_selection = true);
      void set_text_style_selection(const ATextStyle & style);

    // Drawing Methods

      uint freeze();
      uint unfreeze(bool keep_vscroll = true);

    // General Setting Methods

      void enable_events(bool key_events = true, bool mouse_events = false);
      void enable_on_modified(bool enable = true);
      bool is_insert_mode() const;
      void clear_flag(eRichFlag flag)                         { m_rich_flags &= ~flag; }
      void set_flag(eRichFlag flag)                           { m_rich_flags |= flag; }

      // Inherited from AEditOS

        virtual void set_char_limit(uint32_t max_chars);

      //void         enable_culling(bool truncate_old = true);
      //void         enable_vertical_scroll(bool no_fixed_height = true);
      //void         set_break_conversion(eALineBreak type);

      // Inherited from AEditLineOS

      //void enable_read_only(bool read_only = true);
      //void enable_horizontal_scroll(bool virtual_width_or_disable_word_wrapping = true);
      //bool is_modified() const;
      //bool is_read_only() const;
      //int  get_char_limit() const;
      //void set_alignment(eEditAlignment type);
      //void set_conversion(eAEditConvert type);
      //void set_modified(bool modified = true);

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

      void indent_selection(uint space_count = AString_indent_spaces_def);
      void unindent_selection(uint space_count = AString_indent_spaces_def, uint tab_stops = AString_tab_stop_def);

      // Inherited from AEditOS

        //void select_row(uint32_t row);
        //void select_rows(uint32_t from_row, uint32_t to_row);

      // Inherited from AEditLineOS

        virtual void get_selection(uint32_t * start_p, uint32_t * end_p) const;
        virtual void select(uint32_t start, uint32_t end);

        //void    deselect();
        //void    enable_selection_hiding(bool focus_lost_hide = true);
        //void    extend_selection(uint32_t index);
        //AString get_selection() const;
        //void    get_selection(AString * str_p) const;
        //bool    is_selected() const;
        //AString pop_selection(bool undoable = false);
        //void    pop_selection(AString * str_p, bool undoable = false);
        //void    remove_selection(bool undoable = false);
        //void    select_all();

    // Text Methods

      void append_style(const AString & str, const ATextStyle & style, bool undoable = false);

      // Inherited from AEditOS

        virtual void convert_from_edit_linebreaks(AString * str_p) const;
        virtual void convert_to_edit_linebreaks(AString * str_p) const;

        //void    cull_char_rows(uint32_t min_chars, bool undoable = false);
        //AString get_row(uint32_t row) const;
        //void    get_row(uint32_t row, AString * str_p) const;
        //void    remove_row(uint32_t row, bool undoable = false);
        //void    remove_rows(uint32_t from_row, uint32_t to_row, bool undoable = false);

      // Inherited from AEditLineOS

        virtual uint32_t get_length() const;
        virtual void get_text(AString * str_p) const;
        virtual void get_text(uint32_t start, uint32_t end, AString * str_p) const;
        virtual void set_text(const AString & str, bool undoable = false);

        //void         append(const AString & str, bool undoable = false);
        //void         empty(bool undoable = false);
        //AString      get_text() const;
        //AString      get_text(uint32_t start, uint32_t end) const;                 
        //bool         is_empty() const;

    // Positioning & Hit Testing Methods

        AVec2i get_scroll_pos() const;
        void   set_scroll_pos(const AVec2i & virtual_pos);

      // Inherited from AEditOS

        virtual uint32_t get_index_from_pos(const AVec2i & client_pos) const;
        virtual bool get_row_from_pos(const AVec2i & client_pos, uint32_t * row_p = nullptr) const;
        //void set_word_break_func(AFunctionArgBase<WordBreakInfo *> * word_break_func_p);

        // uint32_t     get_index_visible_first() const;
        // uint32_t     get_index_visible_last() const;
        // virtual uint32_t get_index_from_pos(const AVec2i & client_pos) const;
        // uint32_t     get_index_from_row(uint32_t row) const;
        // void         set_index_first(uint32_t index);
        // 
        // uint32_t     get_row_count() const;
        // uint32_t     get_row_caret() const;
        // uint32_t     get_row_visible_first() const;
        // uint32_t     get_row_char_count(uint32_t row) const;
        // virtual bool get_row_from_pos(const AVec2i & client_pos, uint32_t * row_p = nullptr) const;
        // virtual uint32_t get_row_from_index(uint32_t index) const;
        //
        // uint32_t     get_column_from_index(uint32_t index) const;
        // void         set_column_first(uint32_t column);

      // Inherited from AEditLineOS

        virtual uint32_t get_caret_index() const;
        virtual void get_index_position(uint32_t index, int * x_p, int * y_p) const;
        virtual bool get_position_index(const AVec2i & pos, uint32_t * index_p = nullptr) const;
    
        //void ensure_visible_caret();
        //void set_caret_index(uint32_t index);

        // Yet to be implemented for AEditLineOS
        //uint32_t get_first_index() const;
        //void ensure_visible(uint32_t index);
        //void identify_word(uint32_t index, uint32_t * start_p, uint32_t * end_p) const;
        //void resize();

    // Clipboard Methods

      void clipboard_paste_plain();

    // Undo and Redo Methods

      void enable_undo(bool accumulate_undo_info = true);
      bool is_redo() const;
      void redo();

      // Inherited from AEditLineOS

        //bool is_undo() const;
        //void clear_undo();
        //void undo();

        // Yet to be implemented for AEditLineOS
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
      
    virtual void on_selecting(uint32_t start, uint32_t end);

    // Inherited from AEditOS

      //virtual void on_vertical_scroll();

    // Inherited from AEditLineOS

      //virtual void on_exceeded_char_limit();
      //virtual void on_horizontal_scroll();
      //virtual void on_insufficient_memory();
      //virtual void on_modified();
      //virtual void on_predraw();

    // Inherited events from AWindow

      virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);

      //virtual bool on_focus();
      //virtual bool on_focus_lost();

      // These only work if enabled with enable_events() - key events are enabled by default
      // [or enabled with enable_subclass_messages() which does *all* normal window events
      // including the events that would otherwise be ignored below.]
      // When these events are processed [without sub-classing] if RichFlag_ignore_control_event 
      // is set, the event is ignored and the standard window procedure is not called.

        //virtual void on_character(char ch, bool repeated);
        //virtual void on_key_press(eAKey key, bool repeated);
        //virtual void on_key_release(eAKey key);


  // Data Members

    uint32_t m_rich_flags;  // See eRichFlags
    uint     m_ignore_on_modified_count;  // Call on_modified() when 0, otherwise do not call
    void *   m_ole_p;       // A pointer to a IRichEditOle interface
    void *   m_tom_p;       // A pointer to a ITextDocument interface

  // Class Data

    static AMessageTargetClass *  ms_default_class_p;
    static void *                 ms_lib_handle;

  };  // ARichEditOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default Constructor for Rich Edit style info
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle() :
  m_font_color_p(nullptr),
  m_background_color_p(nullptr),
  m_underline(AUnderline__unchanged),
  m_effect_mask(AText__none),
  m_effect_flags(AText__none)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor for Rich Edit style info
// Arg         font_color - font colour to use.  If AColor::ms_void is used, the existing
//             colour is unchanged.  If AColor::ms_default is used, the user preference
//             for text is loaded from the OS.
// Arg         background_color - window background colour to use.  If AColor::ms_void is
//             used, the existing colour is unchanged.  If AColor::ms_default is used, the
//             user preference for a window background colour is loaded from the OS.
// Arg         underline - Underline type to use - see eAUnderline.
// Arg         effect_mask - A combination of eText flags that are to be modified using
//             the values specified in "effect_flags".  Only the flags that are set may
//             be modified.
// Arg         effect_flags - combination of eAText flags to enable/disable depending on
//             "effect_mask" - see eAText.
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle(
  const AColor & font_color,
  const AColor & background_color, // = AColor::ms_void
  eAUnderline    underline,        // = AUnderline__unchanged
  uint32_t       effect_mask,      // = AText__none
  uint32_t       effect_flags      // = AText__none
  ) :
  m_font_color_p((&font_color && !font_color.is_void()) ? &font_color : nullptr),
  m_background_color_p((&background_color && !background_color.is_void()) ? &background_color : nullptr),
  m_underline(underline),
  m_effect_mask(effect_mask),
  m_effect_flags(effect_flags)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor for Rich Edit style info
// Arg         font_color - font colour to use.  If AColor::ms_void is used, the existing
//             colour is unchanged.  If AColor::ms_default is used, the user preference
//             for text is loaded from the OS.
// Arg         effect_mask - A combination of eAText flags that are to be modified using
//             the values specified in "effect_flags".  Only the flags that are set may
//             be modified.
// Arg         effect_flags - combination of eAText flags to enable/disable depending on
//             "effect_mask" - see eAText.
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle(
  const AColor & font_color,
  uint32_t       effect_mask,
  uint32_t       effect_flags // = AText__none
  ) :
  m_font_color_p((&font_color && !font_color.is_void()) ? &font_color : nullptr),
  m_background_color_p(nullptr),
  m_underline(AUnderline__unchanged),
  m_effect_mask(effect_mask),
  m_effect_flags(effect_flags)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor for Rich Edit style info
// Arg         underline - Underline type to use - see eAUnderline.
// Arg         effect_mask - A combination of eAText flags that are to be modified using
//             the values specified in "effect_flags".  Only the flags that are set may
//             be modified.
// Arg         effect_flags - combination of eAText flags to enable/disable depending on
//             "effect_mask" - see eAText.
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle(
  eAUnderline underline,
  uint32_t    effect_mask, // = AText__none
  uint32_t    effect_flags // = AText__none
  ) :
  m_font_color_p(nullptr),
  m_background_color_p(nullptr),
  m_underline(underline),
  m_effect_mask(effect_mask),
  m_effect_flags(effect_flags)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor for Rich Edit style info
// Arg         effect_mask - A combination of eAText flags that are to be modified using
//             the values specified in "effect_flags".  Only the flags that are set may
//             be modified.
// Arg         effect_flags - combination of eAText flags to enable/disable depending on
//             "effect_mask" - see eAText.
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle(uint32_t effect_mask, uint32_t effect_flags) :
  m_font_color_p(nullptr),
  m_background_color_p(nullptr),
  m_underline(AUnderline__unchanged),
  m_effect_mask(effect_mask),
  m_effect_flags(effect_flags)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor for Rich Edit style info
// Arg         style - style info to copy
// See:        other constructors
// Notes:      The values are used immediately and do not need to be maintained.
// Author(s):   Conan Reis
inline ATextStyle::ATextStyle(const ATextStyle & style) :
  m_font_color_p(style.m_font_color_p),
  m_background_color_p(style.m_background_color_p),
  m_underline(style.m_underline),
  m_effect_mask(style.m_effect_mask),
  m_effect_flags(style.m_effect_flags)
  {
  }


#endif  // __ARICHEDITOS_HPP


