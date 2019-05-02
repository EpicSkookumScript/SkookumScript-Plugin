// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AEditLineOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AEditLineOS.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <AgogCore\AFunctionBase.hpp>


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * AEditLineOS::ms_default_class_p;

//=======================================================================================
// Global Variables
//=======================================================================================

const uint AEdit_deselect = ADef_uint32;


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AEditLineOS::initialize()
  {
  ms_default_class_p = new AMessageTargetClass("edit");
  }

//---------------------------------------------------------------------------------------

void AEditLineOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
// Arg         initial_cstr_p - initial AgogCore\AString.  (Default "")
// Arg         font - AFont to use (Default AFont::ms_default)
// Arg         x - x position of the edit box for lefmost co-ordinate (Default 0)
// Arg         y - y position of edit box for topmost co-ordinate (Default 0)
// Arg         width - width of edit box.  If set to Size_auto, it will be
//             the client width of the parent minus its x position.
//             (Default Size_auto)
// Arg         virtual_width - if true, scroll horizontally to the right by 10 characters
//             when the user types a character at the visible end of the line, if false
//             limit maximum character length to visible area (Default true)
// Arg         select_always - if set to true, whenever there is a selection, it is
//             shown regardless of whether this AEditLineOS is the active window or not.
//             If set to false, the selection is shown only when this AEditLineOS is the
//             active window.  (Default true)
// Arg         register_os - specifies whether or not this AEditLineOS should be
//             registered with the operating system.  If true this object is registered
//             and will begin receiving messages and be showable.
//             Low Level Note:  setting this to false will permit the calling of the
//             CreateWindowEx() API function by a derived subclass constructor
//             *** be careful when doing this *** - the position and size variables
//             will be ignored in this case (they will have to be passed into the
//             ::CreateWindowEx() function directly).
//             (Default true)
// Notes:      Initial settings (modified with appropriately matching methods):
//               auto resize       - false, When reaching the right hand side of the edit box, do not resize to contain more text
//               height            - height of current font
//               insert mode       - true, Insert text rather than overwrite
//               read only         - false, Text can be read and written
//               revert mode       - false, Escape does not revert to initial text
//               selection hiding  - false, Any selected text is still visible even when the edit box does not have keyboard focus
//               horizontal scroll - true, Automatically scrolls text to the right by 10 characters when the user types a character at the end of the visible portion of the AEditLineOS and the edit box can be scrolled horizontally to view text that is not within the view window.  If false, the maximum amount of text that may be entered is limited by the visible area of the edit box.
//               alignment         - EA_left, Left aligned text
//               conversion        - AEditConvert_normal, No text conversion is performed.
//               character limit   - 32 kilobytes (32768 characters).
//
//             Initial relevant AWindow inherited settings:
//               show state       - AShowState_hidden, The edit box is not visible, this is since some settings that are modified after this constructor will not redraw until a redraw or show command is given
//               scroll bars      - No vertical or horizontal scroll bar.  It would probably be a good idea to show a horizontal scrollbar if the actual number of columns / characters (get_length()) is less than the visible number of characters (which can be measured with AFont::get_width()).
//               font             - Default proportional font, (Arial point 10).
//               input enabled    - true, it is enabled rather than disabled (and greyed out)
//               keyboard focus   - not focused
//               user sizing      - false, It cannot be resized by the user via a sizing border.
//               border           - Border_no_border
//               mouse cursor     - default I bar
//
//             The maximum number of characters / columns is 1024. 
//             The maximum width of a row is 30,000 pixels. 
// Author(s):   Conan Reis
AEditLineOS::AEditLineOS(
  AWindow *      parent_p,
  const char *   initial_cstr_p,  // = ""
  const AFont &  font,            // = AFont::ms_default
  int            x,               // = 0
  int            y,               // = 0
  int            width,           // = Size_auto
  bool           virtual_width,   // = true
  bool           select_always,   // = true
  bool           register_os      // = true
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_conversion_type(AEditConvert_normal),
  m_modified_func_p(nullptr)
  {
  m_font = font;
  
  if (register_os)
    {
    // Note, for each of the styles there is either an associated method or style is set
    // according to the characteristics of the AMessageTarget
    // Also note that styles do not apply to non-graphical AMessageTarget objects

    // Note that the ES_NOHIDESEL and ES_AUTOHSCROLL styles may only be set on the
    // creation of the edit control and may not be added or removed afterward using the
    // append_style() or the remove_style() methods.
    m_os_handle = ::CreateWindowEx(
      0,               // Extended Window Styles
      m_class_p->get_name(),  // Must use  the name of the class for predefined classes
      initial_cstr_p,  // title
      WS_CHILD | WS_TABSTOP | (select_always ? ES_NOHIDESEL : 0u) | (virtual_width ? ES_AUTOHSCROLL : 0u), // Window & Edit Styles
      x,
      y,
      (width == Size_auto) ? parent_p->get_width_client() - x : width,
      font.get_height(),
      m_parent_handle, 
      nullptr,            // Menu id - This object handles its own messages so an id is not necessary
      AApplication::ms_instance,
      nullptr);           // l_param argument to the WM_CREATE message

    A_VERIFY_OS(m_os_handle != nullptr, "AEditLineOS()", AEditLineOS);

    common_setup();
    }
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
AEditLineOS::~AEditLineOS()
  {
  delete m_modified_func_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Settings Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//void AEditLineOS::enable_auto_resize(bool auto_resize = true)
//void AEditLineOS::enable_background(bool draw = true)

//---------------------------------------------------------------------------------------
//  Sets whether the edit box is read only or editable - it is always
//              readable and selectable.
// Arg          read_only - Specifies whether the edit box should be read only (true) or
//              not (false).  (Default true)
// See:         is_read_only()
// Author(s):    Conan Reis
void AEditLineOS::enable_read_only(
  bool read_only // = true
  )
  {
  A_VERIFY_OS(::SendMessage(m_os_handle, EM_SETREADONLY, read_only, 0u), "enable_read_only()", AEditLineOS);
  }

//void AEditLineOS::enable_revert_mode(bool escape_revert = true)

//---------------------------------------------------------------------------------------
//  If true, enables horizontal scrolling.  If true, automatically scrolls
//              text to the right by 10 characters when the user types a character at the
//              end of the visible text area.  If false, the maximum amount of text that
//              may be entered is limited by the visible area of the edit box.
// Arg          virtual_width_or_disable_word_wrapping - if true, scroll horizontally when
//              appropriate, if false fix maximum text length to visible area (or word
//              wrap if multi-line edit control - AEditOS - hence the really long name). 
//              (Default true)
// See:         AWindow::enable_scrollbars(), enable_vertical_scroll()
// Notes:       !!!!!!!!!!!!!!! This method does not currently function !!!!!!!!!!!!!!!
//              This style cannot be changed once the control has been created.  Use the
//              virtual_width argument in the constructor.  For AEditOS objects, use the
//              word_wrap argument in the AEditOS constructor.
//
//              For multi-line edit controls (AEditOS) this method could have focused on
//              word wrapping and been alternatively named enable_word_wrapping() with
//              the boolean value negated.  In explaination, multi-line edits controls
//              with horizontal scrolling enabled do not break lines of text at the
//              last visible column of text (it goes of the screen into a virual area)
//              and likewise if horizontal scrolling is not enabled for multi-line edit
//              controls then each line is word wrapped at the edge of the visible text
//              area.
//
//              If horizontal scrolling is enabled, the edit box probably should also
//              have a horizontal scroll bar attached via the enable_scrollbars() method.
// Author(s):    Conan Reis
void AEditLineOS::enable_horizontal_scroll(
  bool virtual_width_or_disable_word_wrapping // = true
  )
  {
  if (virtual_width_or_disable_word_wrapping)
    {
    append_style(ES_AUTOHSCROLL);
    }
  else
    {
    remove_style(ES_AUTOHSCROLL);
    }
  }

//bool AEditLineOS::is_auto_resizing() const

//---------------------------------------------------------------------------------------
//  Determines if the edit box has modified since last set_modified(false).
// Returns:     true if it has been modified, false if not
// See:         set_modified()
// Author(s):    Conan Reis
bool AEditLineOS::is_modified() const
  {
  return (::SendMessage(m_os_handle, EM_GETMODIFY, 0u, 0u) != 0u);
  }

//---------------------------------------------------------------------------------------
//  Determines whether the edit box is read only or editable - it is always
//              readable and selectable.
// Returns:     true if read only or false if editable 
// See:         enable_read_only()
// Author(s):    Conan Reis
bool AEditLineOS::is_read_only() const
  {
  return is_style(ES_READONLY);
  }

//bool AEditLineOS::is_revert_mode() const
//Colour AEditLineOS::get_colour() const

//---------------------------------------------------------------------------------------
//  Returns the maximum number of characters that the user is allowed to
//              enter.  [The upper limit for Windows 95-ME is 64KB (65536 characters) and
//              for Windows NT/2000+ it is 4GB.]
// Returns:     maximum allowed number of characters
// See:         set_char_limit()
// Author(s):    Conan Reis
uint32_t AEditLineOS::get_char_limit() const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_GETLIMITTEXT, 0u, 0u));
  }

//---------------------------------------------------------------------------------------
//  Sets the text alignment of the edit box.
// Arg          type - One of:
//                AHorizAlign_left,
//                AHorizAlign_centered,  Only works with AEditLineOS if Win98+ or Win2000+
//                AHorizAlign_right,     Only works with AEditLineOS if Win98+ or Win2000+
//                Note that AHorizAlign_justified is not available with an AEditLineOS nor a AEditOS
// Author(s):    Conan Reis
void AEditLineOS::set_alignment(eAHorizAlign type)
  {
  // $Revisit - CReis This may not work after the edit control is created - TEST!
  switch (type)
    {
    case AHorizAlign_left:
      remove_style(ES_RIGHT | ES_CENTER);
      break;

    case AHorizAlign_centered:
      modify_style(ES_CENTER, ES_RIGHT);
      break;

    case AHorizAlign_right:
      modify_style(ES_RIGHT, ES_CENTER);
      break;

    default:  // AHorizAlign_justified is not available in an AEditLineOS nor an AEditOS
      ;
    }
  }

//---------------------------------------------------------------------------------------
//  Specifies how characters should be converted and displayed as they are
//              added to the edit box.
// Arg          type - One of:
//                AEditConvert_normal,        No text conversion is performed
//                AEditConvert_password,      Displays an asterisk (*) for each character displayed (Only supported by AEditLineOS)
//                AEditConvert_as_lowercase,  Converts text to lowercase as it is typed (not supported by RichEdit)
//                AEditConvert_as_uppercase,  Converts text to uppercase as it is typed (not supported by RichEdit)
//                AEditConvert_digits_only    Permits only digits to be entered
// Author(s):    Conan Reis
void AEditLineOS::set_conversion(eAEditConvert type)
  {
  // $Revisit - CReis Determine what happens to existing text and the difference between
  // user and programmatic entered text.
  // Also RichEdit can do uppercase and lowercase conversion with EM_SETEDITSTYLE - make an override of this method for AgogGUI_OS\ARichEditOS.

  if (m_conversion_type != type)
    {
    uint32_t remove_style = 0u;
    uint32_t append_style = 0u;

    switch (type)
      {
      case AEditConvert_as_lowercase:  // Converts text to lowercase as it is typed
        append_style = ES_LOWERCASE;   // Not supported by RichEdit
        break;

      case AEditConvert_as_uppercase:  // Converts text to uppercase as it is typed
        append_style = ES_UPPERCASE;   // Not supported by RichEdit
        break;

      case AEditConvert_digits_only:   // Permits only digits to be entered
        append_style = ES_NUMBER;
        break;

      case AEditConvert_password:      // Displays an asterisk (*) for each character displayed
        remove_style = ES_PASSWORD;    // Only supported by AEditLineOS
        break;

      default:                         // AEditConvert_normal - no settings to add
        ;
      }

    // Remove previous settings
    switch (m_conversion_type)
      {
      case AEditConvert_as_lowercase:
        remove_style = ES_LOWERCASE;  // Not supported by RichEdit
        break;

      case AEditConvert_as_uppercase:
        remove_style = ES_UPPERCASE;  // Not supported by RichEdit
        break;

      case AEditConvert_digits_only:
        remove_style = ES_NUMBER;
        break;

      case AEditConvert_password:     // Only supported by AEditLineOS
        remove_style = ES_PASSWORD;
        break;

      default:                        // AEditConvert_normal - no settings to remove
        ;
      }

    modify_style(append_style, remove_style);

    m_conversion_type = type;
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the maximum number of characters (not counting the null terminator)
//              that the user is allowed to enter.
// Arg          max_chars - maximum characters allowed.  If set to AEdit_max_characters, it
//              will allow the maximum number of characters possible.  For Windows 95-ME
//              this is 64KB (65536 characters).  For Windows NT/2000 this is 4GB.
// Author(s):    Conan Reis
void AEditLineOS::set_char_limit(uint32_t max_chars)
  {
  ::SendMessage(m_os_handle, EM_LIMITTEXT, max_chars ? max_chars : 1u, 0u);
  }

//---------------------------------------------------------------------------------------
// Gets the height of the edit box that fits around one line of text using the current font.
// 
// #Author(s) Conan Reis
int AEditLineOS::get_height_line_auto() const
  {
  return get_font().get_height() + (get_height() - get_height_client());
  }

//---------------------------------------------------------------------------------------
// Sets the height of the edit box to fit around one line of text using the current font.
// 
// #Author(s) Conan Reis
void AEditLineOS::set_height_line_auto()
  {
  set_height(get_height_line_auto());
  }

//---------------------------------------------------------------------------------------
//  Sets or clears the modification flag for an edit control.
// Arg          modified - true if the flag should be set, false if it should be cleared.
//              (Default true)
// Notes:       The modification flag indicates whether the text within the edit control
//              has been modified. 
// Author(s):    Conan Reis
void AEditLineOS::set_modified(
  bool modified // = true
  )
  {
  ::SendMessage(m_os_handle, EM_SETMODIFY, modified, 0u);
  }

//void AEditLineOS::set_virtual_region(const ARegion & region)
// EM_SETRECTNP - not supported by RichEdit
// EM_SETRECT


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Selection  Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Deselects any selected text.
// See:         extend_selection(), get_selection(), is_selected(), replace_selection(),
//              remove_selection(), pop_selection(), select(), select_all()
// Author(s):    Conan Reis
void AEditLineOS::deselect()
  {
  select(AEdit_deselect, 0u);
  }

//---------------------------------------------------------------------------------------
//  Extend the current selection from the anchor point to the specified index.
// Arg          uint32_t index - position to extend selection to (first index beyond
//              the selection).  If this value is set to AEdit_last_index, all the text
//              following the anchor index will be selected.
// Author(s):    Conan Reis
void AEditLineOS::extend_selection(uint32_t index)
  {
  uint32_t start;

  get_selection(&start, nullptr);
  select(start, index);
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
AString AEditLineOS::get_selection() const
  {
  uint32_t start;
  uint32_t end;

  get_selection(&start, &end);

  return get_text(start, end);
  }

//---------------------------------------------------------------------------------------
// Arg          str_p - 
// Author(s):    Conan Reis
void AEditLineOS::get_selection(AString * str_p) const
  {
  uint32_t start;
  uint32_t end;

  get_selection(&start, &end);
  get_text(start, end, str_p);
  }

//---------------------------------------------------------------------------------------
//  Determines the number of characters currently selected.
// Returns:     number of characters currently selected.
// Author(s):    Conan Reis
uint32_t AEditLineOS::get_selection_length() const
  {
  uint32_t start;
  uint32_t end;

  get_selection(&start, &end);
  return end - start;
  }

//---------------------------------------------------------------------------------------
// Determines the starting and ending character positions of the current
//             selection (if any) in an edit control.
// Arg         start_p - address to store the starting index of the selection.  This
//             parameter can be nullptr.
// Arg         end_p - address to store the position of the first non-selected index
//             after the end of the selection.  This parameter can be nullptr. 
// See:        deselect(), extend_selection(), get_selection(), is_selected(),
//             replace_selection(), remove_selection(), pop_selection(), select(),
//             select_all()
// Notes:      If there is no text currently selected, the start and end positions will
//             both equal the current position of the caret.
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::get_selection(
  uint32_t * start_p,
  uint32_t * end_p
  ) const
  {
  // $Note - CReis The selection has an "anchor" index and an "active" index that contains
  // the caret.  This method does not differentiate between the two though it is more
  // likely that the active index will be the end index of the selection.
  // The anchor/active index *can* be determined via TOM in ARichEditOS objects.
  ::SendMessage(m_os_handle, EM_GETSEL, reinterpret_cast<WPARAM>(start_p), reinterpret_cast<LPARAM>(end_p));
  }

//---------------------------------------------------------------------------------------
//  Determines whether any of the text in the edit box is currently selected.
// Returns:     true if text is selected, false if not.
// See:         deselect(), extend_selection(), get_selection(), replace_selection(),
//              remove_selection(), pop_selection(), select(), select_all()
// Author(s):    Conan Reis
bool AEditLineOS::is_selected() const
  {
  uint32_t start;
  uint32_t end;

  get_selection(&start, &end);
  return (start != end);
  }

//---------------------------------------------------------------------------------------
//  Replaces the current selection with the specified AgogCore\AString.
// Arg          replace_str - text to replace selection with.
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Notes:       If there is no current selection, the replace_str is inserted at the
//              current caret position.
// Author(s):    Conan Reis
void AEditLineOS::replace_selection(
  const AString & replace_str,
  bool            undoable // = false
  )
  {
  ::SendMessage(m_os_handle, EM_REPLACESEL, undoable, reinterpret_cast<LPARAM>(replace_str.as_cstr()));
  }

//---------------------------------------------------------------------------------------
//  Deletes the current selection.
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Notes:       If there is no current selection, no action is performed.
// Author(s):    Conan Reis
void AEditLineOS::remove_selection(
  bool undoable // = false
  )
  {
  replace_selection(AString::ms_empty, undoable);
  }

//---------------------------------------------------------------------------------------
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
AString AEditLineOS::pop_selection(
  bool undoable // = false
  )
  {
  uint32_t start;
  uint32_t end;

  get_selection(&start, &end);

  AString str;

  get_text(start, end, &str);
  remove_selection(undoable);
  return str;
  }

//---------------------------------------------------------------------------------------
// Arg          str_p - 
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
void AEditLineOS::pop_selection(
  AString * str_p,
  bool      undoable // = false
  )
  {
  get_selection(str_p);
  remove_selection(undoable);
  }

//---------------------------------------------------------------------------------------
// Selects the specified text index range.
//             [*** There is a bug in the Win32 message - the anchor is always the
//             minimum value rather than 'start'. ***]
// Arg         uint32_t start - the starting index (anchor) of the text selection.
// Arg         uint32_t end - ending index of the selection (first index beyond
//             the selection).  If this value is set to AEdit_last_index, all the text
//             following the start index will be selected.
// See:        deselect(), extend_selection(), get_selection(), is_selected(),
//             replace_selection(), remove_selection(), pop_selection(), select_all()
// Notes:      The start value can be greater than the end value.  The lower of the two
//             values specifies the character position of the first character in the
//             selection.  The higher value specifies the position of the first
//             character beyond the selection. 
//             The start value is the anchor point of the selection, and the end value
//             is the active end.  If the user uses the SHIFT key to adjust the size of
//             the selection, the active end can move but the anchor point remains the
//             same.
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::select(
  uint32_t start,
  uint32_t end
  )
  {
  ::SendMessage(m_os_handle, EM_SETSEL, start, end);
  }

//---------------------------------------------------------------------------------------
//  Selects all the text in the edit box.
// See:         deselect(), extend_selection(), get_selection(), is_selected(),
//              replace_selection(), remove_selection(), pop_selection(), select()
// Author(s):    Conan Reis
void AEditLineOS::select_all()
  {
  select(0u, AEdit_last_index);
  }

//---------------------------------------------------------------------------------------
//  Deselects text and places the caret for the insertion point at the end of
//              the edit box.
// See:         deselect(), extend_selection(), get_selection(), is_selected(),
//              replace_selection(), remove_selection(), pop_selection(), select(),
//              select_all()
// Author(s):    Conan Reis
void AEditLineOS::select_end()
  {
  select(AEdit_last_index, AEdit_last_index);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Text Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Arg          str - 
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
void AEditLineOS::append(
  const AString & str,
  bool            undoable // = false
  )
  {
  select_end();
  replace_selection(str, undoable);
  }

//---------------------------------------------------------------------------------------
//  Deletes all the text.
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
void AEditLineOS::empty(
  bool undoable // = false
  )
  {
  select_all();
  remove_selection(undoable);
  }

//---------------------------------------------------------------------------------------
// Gets entire text string
// Author(s):   Conan Reis
AString AEditLineOS::get_text() const
  {
  AString str;

  get_text(&str);
  return str;
  }

//---------------------------------------------------------------------------------------
// Gets entire text string
// Arg         str_p - address of string to store text
// Author(s):   Conan Reis
void AEditLineOS::get_text(AString * str_p) const
  {
  uint32_t length = get_length();

  // $Revisit - CReis EM_GETHANDLE is more optimal when it is available, but it does not
  // work for Win95/98/Me or RichEdit.
  str_p->ensure_size_empty(length);
  ::GetWindowText(m_os_handle, str_p->as_cstr_writable(), length + 1u);  // +1 for nullptr char
  str_p->set_length(length);
  }

//---------------------------------------------------------------------------------------
// Gets text sub-string
// Arg         start - character index start
// Arg         end - character index end
// Author(s):   Conan Reis
AString AEditLineOS::get_text(
  uint32_t start,
  uint32_t end
  ) const
  {
  AString str;

  get_text(start, end, &str);
  return str;
  }

//---------------------------------------------------------------------------------------
// Gets text sub-string
// Arg         start - character index start
// Arg         end - character index end
// Arg         str_p - address of string to store text
// Author(s):   Conan Reis
void AEditLineOS::get_text(
  uint32_t  start,
  uint32_t  end,
  AString * str_p
  ) const
  {
  uint32_t sub_length = end - start;

  if (sub_length)
    {
    // Note, though it would be handy, EM_GETHANDLE, does not work for Win95/98/Me.
    // Therefore unfortunately, an offset into the text cannot be specified, so if start
    // is not 0, the initial text prior to start will have to be copied and then removed.
    // AEditOS and ARichEditOS override this method with a more efficient look-up

    // $Revisit - CReis It might be more efficient to accumulate the rows spanned between start and
    // end using get_row() and then crop() the unnecessary bits since get_row() can
    // effectively set an offset.
    uint32_t length = get_length();

    str_p->ensure_size_empty(length);
    length = ::GetWindowText(m_os_handle, str_p->as_cstr_writable(), length + 1u);  // +1 for nullptr char
    str_p->set_length(end);
    str_p->remove_all(0u, start);
    }
  else
    {
    str_p->empty();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
bool AEditLineOS::is_empty() const
  {
  return (get_length() == 0u);
  }

//---------------------------------------------------------------------------------------
// Sets entire text string
// Arg         str - string to set
// Arg         undoable - indicates whether this action should be undoable (true) or
//             not (false).
// See:        set_title()
// Author(s):   Conan Reis
void AEditLineOS::set_text(
  const AString & str,
  bool            undoable // = false
  )
  {
  if (undoable)
    {
    // $Revisit - CReis This causes the edit box to flash because of the selection, the
    // caret ends up at the end of the edit box, and the edit will scroll to ensure that
    // the bottom of the edit is visible
    select_all();
    replace_selection(str, undoable);
    // Here is a mechanism to put caret at start of edit and to have the top in view
    //select(0u, 0u);
    //ensure_visible_caret();
    }
  else
    {
    ::SetWindowText(m_os_handle, str.as_cstr());
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position & Hit Test Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AEditLineOS::ensure_visible_caret()
  {
  ::SendMessage(m_os_handle, EM_SCROLLCARET, 0u, 0u);
  }

//void AEditLineOS::ensure_visible(uint32_t index)

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
uint32_t AEditLineOS::get_caret_index() const
  {
  uint32_t idx;

  // $Note - CReis The selection has an "anchor" index and an "active" index that contains
  // the caret.  This method does not differentiate between the two though it is more
  // likely that the active index will be the end index of the selection.
  // The anchor/active index *can* be determined via TOM in ARichEditOS objects.
  get_selection(nullptr, &idx);

  return idx;
  }

//uint32_t AEditLineOS::get_first_index() const

//---------------------------------------------------------------------------------------
// Retrieves the client area coordinates of the specified character index.
// See:        get_position_index()
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::get_index_position(
  uint32_t index,
  int * x_p,
  int * y_p
  ) const
  {
  LRESULT coords = ::SendMessage(m_os_handle, EM_POSFROMCHAR, index, 0u);

  *x_p = LOWORD(coords);
  *y_p = HIWORD(coords);
  }

//---------------------------------------------------------------------------------------
// Gets the number of characters in the edit control
// Returns:    number of characters in the edit control
// Modifiers:   virtual
// Author(s):   Conan Reis
uint32_t AEditLineOS::get_length() const
  {
  return ::GetWindowTextLength(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Determines the character index closest to the specified client area point.
// Returns:     
// Arg         pos - 
// Arg         index_p - (Default nullptr)
// See:        get_index_position()
// Modifiers:   virtual
// Author(s):   Conan Reis
bool AEditLineOS::get_position_index(
  const AVec2i & pos,
  uint32_t *         index_p // = nullptr
  ) const
  {
  // $Revisit - CReis is it possible to determine if virtual space was hit rather than a valid character?
  if (index_p)
    {
    *index_p = LOWORD(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, MAKELONG(pos.m_x, pos.m_y)));
    }
  return true;
  }

//void AEditLineOS::identify_word(uint32_t index, uint32_t * start_p, uint32_t * end_p) const

//void AEditLineOS::resize()

//---------------------------------------------------------------------------------------
// Arg         index - new caret character index. 
// Author(s):   Conan Reis
void AEditLineOS::set_caret_index(uint32_t index)
  {
  // $Note - Theoretically the caret index could be at the beginning or end of a selection
  // range though this method removes any selection.  Setting the caret index with respect
  // to a selection may be possible to implement via the seemingly hacky method of
  // simulating user key presses.
  select(index, index);
  }

//void AEditLineOS::set_index_first(uint32_t index)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Clipboard Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Copies current selection to clipboard. Copies as much formatting info as is available.
// 
// Author(s):   Conan Reis
void AEditLineOS::clipboard_copy_selected()
  {
  ::SendMessage(m_os_handle, WM_COPY, 0u, 0u);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Undo and Redo Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AEditLineOS::clear_undo()
  {
  ::SendMessage(m_os_handle, EM_EMPTYUNDOBUFFER, 0u, 0u);
  }

//---------------------------------------------------------------------------------------
// Determines if a redo can be performed
// See:        redo(), undo()
// Author(s):   Conan Reis
bool AEditLineOS::is_undo() const
  {
  return ::SendMessage(m_os_handle, EM_CANUNDO, 0u, 0u) != 0u;
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AEditLineOS::undo()
  {
  ::SendMessage(m_os_handle, EM_UNDO, 0u, 0u);
  }

//void AEditLineOS::redo()

//uint32_t AEditLineOS::get_current_undo_depth() const
//uint32_t AEditLineOS::get_undo_depth() const
// EM_CANUNDO

//void AEditLineOS::set_undo_depth(int depth = -1)
//void AEditLineOS::revert()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Called whenever the Edit standard control sends a notification message.
// Returns:     a boolean indicating whether the default MS Windows API process with
//              respect to the given message should be called (true) or not (false).
// Arg          uint32_t code - message code to parse
// Author(s):    Conan Reis
bool AEditLineOS::on_control_event_standard(uint32_t code)
  {
  switch (code)
    {
    case EN_HSCROLL:   // The user has clicked the edit control's horizontal scroll bar. The system sends this message before updating the screen. 
      on_horizontal_scroll();
      break;

    case EN_MAXTEXT:   // While inserting text, the user has exceeded the specified number of characters for the edit control. Insertion has been truncated. This message is also sent either when an edit control does not have the ES_AUTOHSCROLL style and the number of characters to be inserted exceeds the width of the edit control or when an edit control does not have the ES_AUTOVSCROLL style and the total number of lines to be inserted exceeds the height of the edit control. 
      on_exceeded_char_limit();
      break;

    case EN_UPDATE:    // The system is about to draw the text in the edit box. The system sends this message after formatting the text, but before displaying it, so that the application can resize the edit control window. 
      on_predraw();
      break;

    case EN_CHANGE:    // The user has taken an action that may have altered text in an edit control. Unlike the EN_UPDATE notification message, this notification message is sent after the system updates the screen
      on_modified();
      break;

    case EN_SETFOCUS:  // The user has selected this edit control.
      if (!is_subclassed())
        {
        // If return true, continue with default processing (set the window focus)
        if (on_focus())
          {
          m_flags |= Flag_focused;

          return true;
          }

        return false;
        }
      break;

    case EN_KILLFOCUS: // The user has selected another control.
      if (!is_subclassed())
        {
        m_flags &= ~Flag_focused;
        on_focus_lost(0);
        }
      break;

    //case EN_ERRSPACE:  // The edit control cannot allocate enough memory to meet a specific request. 
    default:
      //A_DPRINT("%s AEditLineOS::on_control_event_standard(0x%x)\n", A_SOURCE_STR, code);
      ;
    }

  // Call default behaviour
  return true;
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AEditLineOS::on_exceeded_char_limit()
  {
  //A_DPRINT("AEditLineOS::on_exceeded_char_limit()\n");
  }

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::on_horizontal_scroll()
  {
  }

//---------------------------------------------------------------------------------------
// The user has taken an action that may have altered text in an edit control.
//             Unlike the on_predraw() event, this event is sent after the system updates
//             the screen.
// See:        ARichEditOS::enable_on_modified, on_predraw()
// Notes:      This event is not called when it is a multiline control and the text is
//             set via WM_SETTEXT
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::on_modified()
  {
  if (m_modified_func_p)
    {
    m_modified_func_p->invoke();
    }
  }

//---------------------------------------------------------------------------------------
// The system is about to draw the text (and any selection range) in the edit
//             box.  This event method is called after formatting the text, but before
//             displaying it, so that the edit control can be resized etc. 
// See:        on_modified()
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditLineOS::on_predraw()
  {
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Sets the on_pressed event function
// Arg          pressed_func_p -  dynamically allocated on_pressed event function object
//              to store.  It will be deleted when this object is destructed or when
//              another on_pressed event function object is set.
// Author(s):    Conan Reis
void AEditLineOS::set_on_modified_func(AFunctionBase * modified_func_p)
  {
  if (modified_func_p != m_modified_func_p)
    {
    delete m_modified_func_p;
    m_modified_func_p = modified_func_p;
    }
  }
