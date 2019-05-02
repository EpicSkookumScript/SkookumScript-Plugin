// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AEditLineOS class declaration header
//=======================================================================================


#ifndef __AEDITLINEOS_HPP
#define __AEDITLINEOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>

// fwd decls
class AFunctionBase;

//=======================================================================================
// Global Structures
//=======================================================================================

enum eAEditConvert
  {
  AEditConvert_normal,        // No text conversion is performed
  AEditConvert_password,      // Displays asterisks instead of normal text - Not valid with an AEditLineOS
  AEditConvert_as_lowercase,  // Converts text to lowercase as it is typed - Not valid with ARichEditOS
  AEditConvert_as_uppercase,  // Converts text to uppercase as it is typed - Not valid with ARichEditOS
  AEditConvert_digits_only    // Permits only digits to be entered
  };

// AEditLineOS constants
const uint AEdit_max_characters = ADef_uint;
const uint AEdit_last_index     = ADef_uint;


//---------------------------------------------------------------------------------------
// Notes      An AEditLineOS is a built-in edit control from the operating system.  It is
//            a rectangular control window typically used in a dialog box to permit the
//            user to enter and edit a single line of text by typing on the keyboard.
// Subclasses AEditOS(ARichEditOS)
// Author(s)  Conan Reis
class AEditLineOS : public AWindow
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    AEditLineOS(AWindow * parent_p, const char * initial_cstr_p = "", const AFont & font = *AFont::ms_default_p, int x = 0, int y = 0, int width = Size_auto, bool virtual_width = true, bool select_always = true, bool register_os = true);
    virtual ~AEditLineOS();
    
  // Methods

    // General Setting Methods

      void         enable_read_only(bool read_only = true);
      void         enable_horizontal_scroll(bool virtual_width_or_disable_word_wrapping = true);
      bool         is_modified() const;
      bool         is_read_only() const;
      uint32_t     get_char_limit() const;
      void         set_alignment(eAHorizAlign type);
      void         set_conversion(eAEditConvert type);
      virtual void set_char_limit(uint32_t max_chars);
      int          get_height_line_auto() const;
      void         set_height_line_auto();
      void         set_modified(bool modified = true);
      void         set_on_modified_func(AFunctionBase * modified_func_p);

      //void         enable_auto_resize(bool auto_resize = true);
      //void         enable_background(bool draw = true);
      //void         enable_revert_mode(bool escape_revert = true);
      //bool         is_auto_resizing() const;
      //bool         is_revert_mode() const;
      //Colour       get_colour() const;
      //void         set_colour(Colour & colour);
      //void         set_virtual_region(const ARegion & region);
      //??? get & set margins
      //??? get & set tab stops

    // Selection Methods

      void         deselect();
      void         extend_selection(uint32_t index);
      virtual void get_selection(uint32_t * start_p, uint32_t * end_p) const;
      AString      get_selection() const;
      void         get_selection(AString * str_p) const;
      uint32_t     get_selection_length() const;
      bool         is_selected() const;
      AString      pop_selection(bool undoable = false);
      void         pop_selection(AString * str_p, bool undoable = false);
      void         remove_selection(bool undoable = false);
      virtual void replace_selection(const AString & replace_str, bool undoable = false);
      virtual void select(uint32_t start, uint32_t end);
      void         select_all();
      void         select_end();

    // Text Methods

      void         append(const AString & str, bool undoable = false);
      void         empty(bool undoable = false);
      AString      get_text() const;
      virtual void get_text(AString * str_p) const;
      AString      get_text(uint32_t start, uint32_t end) const;
      virtual void get_text(uint32_t start, uint32_t end, AString * str_p) const;
      bool         is_empty() const;
      virtual void set_text(const AString & str, bool undoable = false);

    // Positioning & Hit Testing Methods

      void             ensure_visible_caret();
      virtual uint32_t get_caret_index() const;
      virtual void     get_index_position(uint32_t index, int * x_p, int * y_p) const;
      virtual uint32_t get_length() const;
      virtual bool     get_position_index(const AVec2i & pos, uint32_t * index_p = nullptr) const;
      void             set_caret_index(uint32_t index);
      //virtual  void set_index_first(uint32_t index);
      //void     ensure_visible(uint32_t index);
      //uint32_t get_first_index() const;
      //void     identify_word(uint32_t index, uint32_t * start_p, uint32_t * end_p) const;
      //void     resize();

    // Clipboard Methods

      void clipboard_copy_selected();

    // Undo and Redo Methods

      bool is_undo() const;
      void clear_undo();
      void undo();

      //void     redo();
      //uint32_t get_current_undo_depth() const;
      //int      get_undo_depth() const;
      //void     set_undo_depth(int depth = -1);
      //void     revert();

    // Useful Methods inherited from AWindow

      // Position / Size Methods - see AWindow
      // Display State Methods - see AWindow
      // void          enable_input(bool input_accepted = true);
      //?void          enable_sizing(bool user_sizable = true);
      //?HCURSOR       get_cursor() const;
      // const AFont & get_font() const;
      //?bool          is_input_enabled() const;
      //?bool          is_sizable() const;
      // void          set_border(eBorder border = Border_raised);
      //?void          set_cursor(HCURSOR cursor_handle);
      // void          set_focus();
      // void          set_font(const AFont & font_p);

    // Methods Nice to have, but difficult (if at all possible) to implement

      //#void         enable_insert_mode(bool insert = true);
      //#bool         is_insert_mode() const;
      //#bool         is_whitespace_enabled() const;
      //#virtual void on_abort();
      //#virtual void on_accept();

  protected:
  // Event Methods

    virtual void on_exceeded_char_limit();
    virtual void on_horizontal_scroll();
    virtual void on_modified();
    virtual void on_predraw();

    // Inherited events from AWindow

      virtual bool on_control_event_standard(uint32_t code);
      //virtual bool on_focus();
      //virtual bool on_focus_lost();

  // Data Members

    eAEditConvert m_conversion_type;
    
    // Called prior to any call to on_modifed()
	  AFunctionBase * m_modified_func_p;

  // Class Data

    static AMessageTargetClass *  ms_default_class_p;

  };  // AEditLineOS


//=======================================================================================
// Inline Functions
//=======================================================================================


#endif  // __AEDITLINEOS_HPP


