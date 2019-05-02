// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AEditOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS/AEditOS.hpp>
#include <AgogCore/AMath.hpp>
#include <AgogIO/AApplication.hpp>
#include <AgogIO/AClipboard.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
// Arg         initial - initial AgogCore\AString.  (Default AString::ms_empty)
// Arg         word_wrap - specifies whether word wrapping should be enabled or not.
//             Note word wrapping is incompatible with enabling the horizontal scrollbar.
//             (Default true)
// Arg         font - AFont to use (Default AFont::ms_default)
// Arg         region - position and area of the edit box.  If width set to
//             Size_auto, the width will be the client width of the
//             parent minus the x position.  If height set to Size_auto,
//             the height will be the client height of the parent minus the y position.
//             (Default ARegion(0, 0, Size_auto, Size_auto))
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
//               auto resize       - false, When reaching the bottom of the edit box, do not resize to contain more text
//               break conversion  - true, Native C/Unix/Mac style line breaks with just newlines (\n) are input and output.  If false, DOS style line breaks with both carriage returns and newlines (\r\n) are input and output.
//               culling           - false, When the text buffer is full disallow extra text.  If true, removes as many lines from the top of the edit box until there is enough space to allow for the extra characters.
//               insert mode       - true, Insert text rather than overwrite
//               read only         - false, Text can be read and written
//               revert mode       - false, Escape does not revert to initial text
//               selection hiding  - false, Any selected text is still visible even when the edit box does not have keyboard focus
//               vertical scroll   - true, Text scrolls up 10 lines to allow for more text when the bottom of the edit box is reached.  If it were false, additional text would be disallowed past the bottom.
//               horizontal scroll - negation of word_wrap, Text wraps to the next line if the right hand side is reached.  If true, the edit box can be scrolled horizontally to view text that is not within the view window.
//               alignment         - EA_left, Left aligned text
//               conversion        - AEditConvert_normal, No text conversion is performed.
//               character limit   - 32 kilobytes (32768 characters).
//
//             Initial relevant AWindow inherited settings:
//               show state       - AShowState_hidden, The edit box is not visible, this is since some settings that are modified after this constructor will not redraw until a redraw or show command is given
//               scroll bars      - Vertical bar present, horizontal scroll bar if not word_wrap.
//               font             - Default proportional font, (Arial point 10).
//               input enabled    - true, it is enabled rather than disabled (and greyed out)
//               keyboard focus   - not focused
//               user sizing      - false, It cannot be resized by the user via a sizing border.
//               border           - Border_no_border
//               mouse cursor     - default I bar
//
//             The maximum number of characters / columns in a single row is 1024. 
//             The maximum width of a row is 30,000 pixels. 
//             The maximum number of rows is approximately 16,350. 
// Author(s):    Conan Reis
AEditOS::AEditOS(
  AWindow *       parent_p,
  const AString & initial,       // = AString::ms_empty
  bool            word_wrap,     // = true
  const AFont &   font,          // = AFont::ms_default
  const ARegion & region,        // = AWindow::ms_region_auto_def
  bool            select_always, // = true
  bool            register_os    // = true
  ) :
  AEditLineOS(parent_p, "", font, 0, 0, 0, !word_wrap, select_always, false),
  m_breaks_expected(ALineBreak_unix),  // Assume Unix convention for Agog libraries
  m_culling(false),
  m_char_limit(0u)
  {
  if (register_os)
    {
    AString edit_str(initial);

    convert_to_edit_linebreaks(&edit_str);

    // Note, for each of the styles there is either an associated method or style is set
    // according to the characteristics of the AMessageTarget
    // Only the styles and the initial AString / title are different from AWindow

    // Note that the ES_NOHIDESEL and ES_AUTOHSCROLL styles may only be set on the
    // creation of the edit control and may not be added or removed afterward using the
    // append_style() or the remove_style() methods.
    m_os_handle = ::CreateWindowEx(
      0,           // Extended Window Styles
      m_class_p->get_name(),  // Must use  the name of the class for predefined classes
      edit_str,    // title
      WS_CHILD | WS_TABSTOP | ES_MULTILINE | (select_always ? ES_NOHIDESEL : 0u) | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL | (word_wrap ? 0u : WS_HSCROLL | ES_AUTOHSCROLL), // Window & Edit Styles
      region.m_x,
      region.m_y,
      (region.m_width == Size_auto) ? parent_p->get_width_client() - region.m_x : region.m_width,
      (region.m_height == Size_auto) ? parent_p->get_height_client() - region.m_y : region.m_height,
      m_parent_handle, 
      nullptr,        // Menu id - This object handles its own messages so an id is not necessary
      AApplication::ms_instance,
      nullptr);       // l_param argument to the WM_CREATE message

    A_VERIFY_OS(m_os_handle != nullptr, "AEditOS()", AEditOS);

    common_setup();
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Settings Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Specifies whether the line break convention used by the control should be
//              converted/ (as needed) to some other specified line break convention.  In
//              other words all text strings passed to this control should be in this form
//              and all text strings returned from this control will be in this form.
//              If it is set to ALineBreak__default then the text passed in and returned
//              will be the native convention used by this control - /r/n for AEditOS and
//              /r for ARichEdit controls respectively.
//              The initial value is ALineBreak_unix which is the standard for the Agog
//              library.
// Arg          expected - See eALineBreak
// See:         every AString related method of AEditOS
// Author(s):    Conan Reis
void AEditOS::set_break_conversion(eALineBreak expected)
  {
  m_breaks_expected = expected;
  }

//---------------------------------------------------------------------------------------
// Set tabs in fixed intervals using the specified number of space characters using the
// current font.
//
// # Notes
//   Good for fixed width fonts.
//   Will not auto-draw pre-existing text - if needed call: `refresh()` or `invalidate()`.
//
// # Author(s) Conan Reis
void AEditOS::set_tabs_fixed_spaces(uint32_t space_count)
  {
  UINT dlg_units = AWin_dialog_units_per_avg_char_x * space_count;
  
  ::SendMessage(m_os_handle, EM_SETTABSTOPS, 1u, LPARAM(&dlg_units));
  }

//---------------------------------------------------------------------------------------
// Arg          truncate_old - (Default true)
// Author(s):    Conan Reis
void AEditOS::enable_culling(
  bool truncate_old // = true
  )
  {
  m_culling    = true;
  m_char_limit = get_char_limit();
  }

//---------------------------------------------------------------------------------------
//  If true, indicates that the edit control should scroll the text
//              vertically when the user enters more text than can be displayed within
//              the edit control.  If false, the edit control will not accept the input
//              when more text is entered than can be displayed.
// Arg          no_fixed_height - (Default true)
// See:         AWindow::enable_scrollbars(), enable_horizontal_scroll()
// Notes:       If vertical scrolling is enabled, the edit box probably should also have
//              a vertical scroll bar attached via the enable_scrollbars() method.
// Author(s):    Conan Reis
void AEditOS::enable_vertical_scroll(
  bool no_fixed_height // = true
  )
  {
  if (no_fixed_height)
    {
    append_style(ES_AUTOVSCROLL);
    }
  else
    {
    remove_style(ES_AUTOVSCROLL);
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the maximum number of characters (not counting the null terminator)
//              that the user is allowed to enter.
// Arg          max_chars - maximum characters allowed.  If set to AEdit_max_characters, it
//              will allow the maximum number of characters possible.  For Windows 95-ME
//              this is 64KB (65536 characters).  For Windows NT/2000 this is 4GB.
// Notes:       When calculating, newline characters ('\n') count as 2 characters since
//              the carriage return character ('\r') is included with it in line breaks
//              ("\r\n").
// Author(s):    Conan Reis
void AEditOS::set_char_limit(uint32_t max_chars)
  {
  if (m_culling)
    {
    uint32_t length = get_length();

    m_char_limit = max_chars;

    if (length > max_chars)
      {
      cull_char_rows(length - max_chars);
      }
    }

  ::SendMessage(m_os_handle, EM_LIMITTEXT, max_chars ? max_chars : 1u, 0u);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Selection  Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets the current selection and if there is no selection gets the current
//             row/line that the caret is on.
// Arg         str_p - address to store selection or line.
// Author(s):   Conan Reis
void AEditOS::get_selection_or_row(AString * str_p) const
  {
  if (is_selected())
    {
    get_selection(str_p);
    }
  else
    {
    get_row(get_row_caret(), str_p);
    }
  }

//---------------------------------------------------------------------------------------
// Determine beginning and ending row (0-based) of current selection.
//
// See:        get_selection(), get_row_from_index(), get_index_from_row()
// Author(s):   Conan Reis
void AEditOS::get_selection_rows(
  uint32_t * row_begin_p,
  uint32_t * row_end_p
  ) const
  {
  uint32_t sel_begin;
  uint32_t sel_end;

  get_selection(&sel_begin, &sel_end);

  // Determine selection beginning row
  *row_begin_p = get_row_from_index(sel_begin);

  // Determine selection ending row
  uint32_t row_end = get_row_from_index(sel_end);

  // If on first column assume previous row
  *row_end_p = (sel_end == get_index_from_row(row_end))
    ? row_end - 1
    : row_end;
  }

//---------------------------------------------------------------------------------------
//  Replaces the current selection with the specified AgogCore\AString.
// Arg          replace_str - text to replace selection with.
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Notes:       If there is no current selection, the replace_str is inserted at the
//              current caret position.
// Author(s):    Conan Reis
void AEditOS::replace_selection(
  const AString & replace_str,
  bool            undoable // = false
  )
  {
  AString edit_str(replace_str);

  convert_to_edit_linebreaks(&edit_str);

  // $Revisit - CReis The replacement string could be more than the character limit all by itself!
  if (m_culling)
    {
    uint32_t sel_start;
    uint32_t sel_end;
    int  min_remove;
    int  old_length = get_length();

    // Save previous selection
    get_selection(&sel_start, &sel_end);

    // Determine minimum amount of characters to remove
    min_remove = (old_length - (sel_end - sel_start) + edit_str.get_length()) - m_char_limit;

    if (min_remove > 0)
      {
      int difference;

      // Remove the minimum number of rows that contain at least min_remove characters.
      cull_char_rows(min_remove);

      // Reset selection - if it wasn't just removed
      difference = get_length() - old_length;
      sel_start  = a_max(difference + int(sel_start), 0);
      sel_end    = a_max(difference + int(sel_end), 0);
      select(sel_start, sel_end);
      }
    }

  ::SendMessage(m_os_handle, EM_REPLACESEL, undoable, reinterpret_cast<LPARAM>(edit_str.as_cstr()));
  }

//---------------------------------------------------------------------------------------
// Arg          row - 
// Notes:       The row selection will include the carriage return and newline characters.
// Author(s):    Conan Reis
void AEditOS::select_row(uint32_t row)
  {
  uint32_t start_index = get_index_from_row(row);
  uint32_t end_index   = get_index_from_row(row + 1);

  select(start_index, (end_index == -1) ? get_length() : end_index);
  }

//---------------------------------------------------------------------------------------
// Arg          from_row - 
// Arg          to_row -
// Notes:       The row selection will include the carriage return and newline characters.
// Author(s):    Conan Reis
void AEditOS::select_rows(
  uint32_t from_row,
  uint32_t to_row
  )
  {
  uint32_t start_index = get_index_from_row(from_row);
  uint32_t end_index   = (get_row_count() > (to_row + 1)) ? get_index_from_row(to_row + 1) : get_length();

  select(start_index, end_index);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Text Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converts string from this edit box's native line break convention to the
//             "external" line break convention.
// Arg         str_p - string to convert
// See:        convert_to_edit_linebreaks(), set_break_conversion(), m_breaks_expected
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditOS::convert_from_edit_linebreaks(AString * str_p) const
  {
  switch (m_breaks_expected)
    {
    case ALineBreak_unix:  // \n    [Standard] C++, Unix, Mac-OSX
      str_p->line_break_dos2unix();
      break;

    case ALineBreak_rich:  // \r    RichEdit boxes
      str_p->line_break_dos2rich();
      break;

    //case ALineBreak_dos:   // \r\n  PC files, edit boxes
    //case ALineBreak__default:
      // Already in desired convention - do nothing
    }
  }

//---------------------------------------------------------------------------------------
// Converts string from the "external" line break convention to this edit
//             box's native line break convention.
// Arg         str_p - string to convert
// See:        convert_from_edit_linebreaks(), set_break_conversion(), m_breaks_expected
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditOS::convert_to_edit_linebreaks(AString * str_p) const
  {
  switch (m_breaks_expected)
    {
    case ALineBreak_unix:  // \n    [Standard] C++, Unix, Mac-OSX
      str_p->line_break_unix2dos();
      break;

    case ALineBreak_rich:  // \r    RichEdit boxes
      str_p->line_break_rich2dos();
      break;

    //case ALineBreak_dos:   // \r\n  PC files, edit boxes
    //case ALineBreak__default:
      // Already in desired convention - do nothing
    }
  }

//---------------------------------------------------------------------------------------
// Removes the minimum number of rows that contain at least min_chars
//             characters from the beginning of the AgogGUI_OS\AEditOS.
// Arg         min_chars - minimum number of characters that must be removed
// Arg         undoable - indicates whether this action should be undoable (true) or
//             not (false).  (Default false)
// See:        enable_culling()
// Author(s):   Conan Reis
void AEditOS::cull_char_rows(
  uint32_t min_chars,
  bool undoable // = false
  )
  {
  remove_rows(0u, get_row_from_index(min_chars), undoable);
  }

//---------------------------------------------------------------------------------------
// Stores the string of characters from the specified line (not including any
//             newline or carriage return) in str_p
// Arg         row - row to retrieve
// Arg         str_p - 
// See:        get_row_char_count()
// Author(s):   Conan Reis
void AEditOS::get_row(
  uint32_t  row,
  AString * str_p
  ) const
  {
  uint32_t length = get_row_char_count(row);

  if (length)
    {
    // Store buffer size in first WORD of buffer.
    // Ensure size is large enough
    str_p->ensure_size_buffer(a_max(length, uint32_t(sizeof(WORD))));

    char * cstr_p = str_p->as_cstr_writable();

    *reinterpret_cast<WORD *>(cstr_p) = WORD(str_p->get_size());

    // The length returned by EM_GETLINE with RichEdit controls is sometimes 1 greater than
    // the actual size so using length from get_row_count() above instead.
    ::SendMessage(m_os_handle, EM_GETLINE, row, reinterpret_cast<LPARAM>(cstr_p));

    str_p->set_length(length);

    convert_from_edit_linebreaks(str_p);
    }
  else
    {
    str_p->set_length(length);
    }
  }

//---------------------------------------------------------------------------------------
// Gets entire text string
// Arg         str_p - address of string to store text
// Author(s):   Conan Reis
void AEditOS::get_text(AString * str_p) const
  {
  AEditLineOS::get_text(str_p);
  convert_from_edit_linebreaks(str_p);
  }

//---------------------------------------------------------------------------------------
// Gets text sub-string
// Arg         start - character index start
// Arg         end - character index end
// Arg         str_p - address of string to store text
// Author(s):   Conan Reis
void AEditOS::get_text(
  uint32_t  start,
  uint32_t  end,
  AString * str_p
  ) const
  {
  AEditLineOS::get_text(start, end, str_p);
  convert_from_edit_linebreaks(str_p);
  }

//---------------------------------------------------------------------------------------
// Arg          row - 
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
void AEditOS::remove_row(
  uint32_t row,
  bool undoable // = false
  )
  {
  select_row(row);
  remove_selection(undoable);
  }

//---------------------------------------------------------------------------------------
// Arg          from_row - 
// Arg          to_row - 
// Arg          undoable - indicates whether this action should be undoable (true) or
//              not (false).  (Default false)
// Author(s):    Conan Reis
void AEditOS::remove_rows(
  uint32_t from_row,
  uint32_t to_row,
  bool undoable // = false
  )
  {
  select_rows(from_row, to_row);
  remove_selection(undoable);
  }

//---------------------------------------------------------------------------------------
// Sets entire text string
// Arg         str - string to set
// Arg         undoable - indicates whether this action should be undoable (true) or
//             not (false).
// See:        set_title()
// Author(s):   Conan Reis
void AEditOS::set_text(
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
    AString edit_str(str);

    convert_to_edit_linebreaks(&edit_str);
    ::SetWindowText(m_os_handle, edit_str.as_cstr());
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position & Hit Test Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
uint32_t AEditOS::get_row_caret() const
  {
  // $Note - CReis -1 can be used with get_row_from_index() for the "current" row, but
  // get_caret_index() is virtual and gives a more true caret position for ARichEditOS
  // controls.
  return get_row_from_index(get_caret_index());
  }

//uint32_t AEditOS::get_first_column() const

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
uint32_t AEditOS::get_row_visible_first() const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_GETFIRSTVISIBLELINE, 0u, 0u));
  }

//---------------------------------------------------------------------------------------
// Get the index relative to the start of the row - like a column though using character
// count without taking tabs into account.
// 
// See:         get_column_from_index()
// Author(s):   Conan Reis
uint32_t AEditOS::get_row_index_from_index(uint32_t idx) const
  {
  return idx - get_index_from_row(get_row_from_index(idx));
  }

//---------------------------------------------------------------------------------------
// Get the 0-based display column (taking tabs `\t` into account and assuming a
// monospace/fixed-width font) for the specified index.
// 
// See:         get_row_index_from_index()
// Author(s):   Conan Reis
uint AEditOS::get_column_from_index(
  uint idx,
  uint tab_stops // = AString_tab_stop_def
  ) const
  {
  uint row        = get_row_from_index(idx);
  uint idx_on_row = idx - get_index_from_row(row);

  AString row_str(get_row(row));

  return AString::count_columns(row_str.as_cstr(), row_str.as_cstr() + idx_on_row, tab_stops);
  }

//---------------------------------------------------------------------------------------
// Arg          index - 
// Author(s):    Conan Reis
uint32_t AEditOS::get_row_from_index(uint32_t index) const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_LINEFROMCHAR, index, 0u));
  }

//---------------------------------------------------------------------------------------
// Determines index of first visible character
// See:        get_index_visible_last()
// Author(s):   Conan Reis
uint32_t AEditOS::get_index_visible_first() const
  {
  return get_index_from_row(get_row_visible_first());
  }

//---------------------------------------------------------------------------------------
// Determines index of last visible character
// See:        get_index_visible_last()
// Author(s):   Conan Reis
uint32_t AEditOS::get_index_visible_last() const
  {
  return get_index_from_pos(get_area_client());
  }

//---------------------------------------------------------------------------------------
// Gets the number of characters on the specified row not including any
//             newline or carriage return.
// Arg         row - row to count characters on
// See:        get_row()
// Author(s):   Conan Reis
uint32_t AEditOS::get_row_char_count(uint32_t row) const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_LINELENGTH, get_index_from_row(row), 0u));
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
uint32_t AEditOS::get_row_count() const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_GETLINECOUNT, 0u, 0u));
  }

//uint32_t AEditOS::get_row_visible_count() const
// EM_GETTHUMB?

//---------------------------------------------------------------------------------------
// Arg          row - 
// Author(s):    Conan Reis
uint32_t AEditOS::get_index_from_row(uint32_t row) const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_LINEINDEX, row, 0u));
  }

//---------------------------------------------------------------------------------------
// Arg          pos - 
// Arg          row_p - (Default nullptr)
// Author(s):    Conan Reis
bool AEditOS::get_row_from_pos(
  const AVec2i & client_pos,
  uint32_t *         row_p // = nullptr
  ) const
  {
  if (row_p)
    {
    *row_p = HIWORD(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, MAKELONG(client_pos.m_x, client_pos.m_y)));
    }

  return true;  // $Revisit - CReis is it possible to determine if virtual space was hit rather than a valid character?
  }

//---------------------------------------------------------------------------------------
// Determines character index position nearest to pos
// Returns:    character index position nearest to pos
// Arg         client_pos - 
// See:        get_row_from_pos()
// Modifiers:   virtual
// Author(s):   Conan Reis
uint32_t AEditOS::get_index_from_pos(const AVec2i & client_pos) const
  {
  return HIWORD(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, MAKELONG(client_pos.m_x, client_pos.m_y)));
  }

//---------------------------------------------------------------------------------------
// Arg          column - 
// Author(s):    Conan Reis
void AEditOS::set_column_first(uint32_t column)
  {
  // $Revisit - CReis These may be relative to the current position.
  ::SendMessage(m_os_handle, EM_LINESCROLL, column, get_row_visible_first());
  }

//---------------------------------------------------------------------------------------
// Arg          index - 
// Author(s):    Conan Reis
void AEditOS::set_index_first(uint32_t index)
  {
  uint32_t row    = get_row_from_index(index);
  uint32_t column = index - get_index_from_row(row);

  // $Revisit - CReis These may be relative to the current position.
  ::SendMessage(m_os_handle, EM_LINESCROLL, column, row);
  }

//---------------------------------------------------------------------------------------
// Get the home index position based on the supplied index - toggles on the row of the
// supplied index between first non-space column and the first column.
//
// #Notes
//   Useful for the 'Home' key behaviour especially for code editors.
//
// #See Also  toggle_caret_home_nonspace()
// #Author(s) Conan Reis
uint32_t AEditOS::get_index_home_nonspace(
  // index to toggle between first non-space column and first column
  uint32_t index
  ) const
  {
  uint32_t caret_idx = get_caret_index();
  uint32_t caret_row = get_row_from_index(caret_idx);
  uint32_t row_idx   = get_index_from_row(caret_row);

  // Determine first non-space index on row
  AString row_str;
  uint32_t    nspace_column = 0u;
  get_row(caret_row, & row_str);
  row_str.find(ACharMatch_not_white_space, 1u, &nspace_column);

  uint32_t nspace_idx = row_idx + nspace_column;

  return (caret_idx == nspace_idx)
    ? row_idx
    : nspace_idx;
  }

//---------------------------------------------------------------------------------------
// Toggle caret position on its current row between the first non-space column and the
// first column.
//
// #Notes
//   Useful for the 'Home' key behaviour especially for code editors.
//
// #See Also  get_index_home_nonspace()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // new caret index
  uint32_t
AEditOS::toggle_caret_home_nonspace()
  {
  uint32_t new_idx = get_index_home_nonspace(get_caret_index());

  set_caret_index(new_idx);

  return new_idx;
  }


//void AEditOS::set_first_row(uint32_t row)
// EM_LINESCROLL EM_SCROLL

//void AEditOS::set_word_break_func(AFunctionArgBase<WordBreakInfo *> * word_break_func_p)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Clipboard Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Copies current selection (or line if no selection) in plain text format to clipboard.
// 
// See:         AEditLineOS::clipboard_copy_selected()
// Author(s):   Conan Reis
void AEditOS::clipboard_copy_plain_sel_or_row()
  {
  AClipboard clip(this);

  clip.set_text(get_selection_or_row());
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Called whenever the Edit standard control sends a notification message.
// Returns:     a boolean indicating whether the default MS Windows API process with
//              respect to the given message should be called (true) or not (false).
// Arg          uint32_t code - message code to parse
// Modifiers:    virtual
// Author(s):    Conan Reis
bool AEditOS::on_control_event_standard(uint32_t code)
  {
  if (code == EN_VSCROLL)
    {
    // The user has clicked the edit control's vertical scroll bar or has scrolled the
    // mouse wheel over the edit control. The system sends this message before updating
    // the screen.  
    on_vertical_scroll();
    return true;  // invoke default behaviour
    }

  return AEditLineOS::on_control_event_standard(code);
  }

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
void AEditOS::on_vertical_scroll()
  {
  }
