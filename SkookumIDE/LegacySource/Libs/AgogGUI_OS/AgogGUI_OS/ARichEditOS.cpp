// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARichEditOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\ARichEditOS.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogIO\AApplication.hpp>
#include <AgogIO\AKeyboard.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <RichEdit.h>
#include <OLEidl.h>
#include <RichOLE.h>
#include <TOM.h>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

#if (_RICHEDIT_VER >= 0x0200)
  #define ARICH_EDIT_DLL_NAME "RICHED20.DLL"  // RichEdit 2.0 or 3.0
#else
  #define ARICH_EDIT_DLL_NAME "RICHED32.DLL"  // RichEdit 1.0
#endif

#define A_DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  EXTERN_C const GUID CDECL name = {l, w1, w2, {b1, b2,  b3,  b4,  b5,  b6,  b7,  b8}}

A_DEFINE_GUIDXXX(IID_ITextDocument, 0x8CC497C0, 0xA1DF, 0x11CE, 0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D);


namespace
{

  enum
    {
    ARichEditOS_key_extended_flag = 0x01000000,  // Specifies whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key keyboard
    ARichEditOS_key_repeat_flag   = 0x40000000
    };


  //---------------------------------------------------------------------------------------
  // Fills a CHARFORMAT2 from a ATextStyle
  // Author(s):   Conan Reis
  void a_richeditos_prep_charformat(
    const ATextStyle & style,
    CHARFORMAT2 *      cfmt_p
    )
    {
    cfmt_p->cbSize    = sizeof(CHARFORMAT2);
    cfmt_p->dwMask    = 0u;
    cfmt_p->dwEffects = 0u;

    // Is font colour specified?
    if (style.m_font_color_p)
      {
      cfmt_p->dwMask |= CFM_COLOR;

      if (style.m_font_color_p->is_default())
        {
        cfmt_p->dwEffects |= CFE_AUTOCOLOR;
        }
      else
        {
        cfmt_p->crTextColor = *style.m_font_color_p; 
        }
      }

    // Is background colour specified?
    if (style.m_background_color_p)
      {
      cfmt_p->dwMask |= CFM_BACKCOLOR;

      if (style.m_background_color_p->is_default())
        {
        cfmt_p->dwEffects |= CFE_AUTOBACKCOLOR;
        }
      else
        {
        cfmt_p->crBackColor = *style.m_background_color_p; 
        }
      }

    // Is an underline type specified?
    if (style.m_underline != AUnderline__unchanged)
      {
      switch (style.m_underline)
        {
        case AUnderline_solid:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINE;
          break;

        case AUnderline_thick:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINETHICK;
          break;

        case AUnderline_wave:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINEWAVE;
          break;

        case AUnderline_dotted:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINEDOTTED;
          break;

        case AUnderline_dash:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINEDASH;
          break;

        case AUnderline_dash_dot:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINEDASHDOT;
          break;

        case AUnderline_dash_dot_dot:
          cfmt_p->dwMask         |= CFM_UNDERLINETYPE;
          cfmt_p->bUnderlineType  = CFU_UNDERLINEDASHDOTDOT;
          break;

        default: // AUnderline_none - No underline
          cfmt_p->dwMask |= CFM_UNDERLINE;
          break;
        }
      // The following underline types do not display using the method that they imply so
      // they are not used:
      //   CFU_UNDERLINEDOUBLE          - displayed as single solid
      //   CFU_UNDERLINEWORD            - displayed as single solid
      //   CFU_CF1UNDERLINE             - displayed as single solid
      //   CFU_INVERT                   - displayed as single solid
      //   CFU_UNDERLINEHAIRLINE        - displayed as single solid
      //   CFU_UNDERLINEHEAVYWAVE       - displayed as as wave
      //   CFU_UNDERLINEDOUBLEWAVE      - displayed as as wave
      //   CFU_UNDERLINETHICKDOTTED     - displayed as dot
      //   CFU_UNDERLINETHICKDASH       - displayed as dash
      //   CFU_UNDERLINELONGDASH        - displayed as dash
      //   CFU_UNDERLINETHICKLONGDASH   - displayed as dash
      //   CFU_UNDERLINETHICKDASHDOT    - displayed as dash dot
      //   CFU_UNDERLINETHICKDASHDOTDOT - displayed as dash dot dot
      }

    // Are any effects specified?
    if (style.m_effect_mask)
      {
      if (style.m_effect_mask & AText_italics)
        {
        cfmt_p->dwMask |= CFM_ITALIC;

        if (style.m_effect_flags & AText_italics)
          {
          cfmt_p->dwEffects |= CFE_ITALIC;
          }
        }

      if (style.m_effect_mask & AText_bold)
        {
        cfmt_p->dwMask |= CFM_BOLD;

        if (style.m_effect_flags & AText_bold)
          {
          cfmt_p->dwEffects |= CFE_BOLD;
          }
        }

      if (style.m_effect_mask & AText_strikeout)
        {
        cfmt_p->dwMask |= CFM_STRIKEOUT;

        if (style.m_effect_flags & AText_strikeout)
          {
          cfmt_p->dwEffects |= CFE_STRIKEOUT;
          }
        }

      if (style.m_effect_mask & AText_disabled)
        {
        cfmt_p->dwMask |= CFM_DISABLED;

        if (style.m_effect_flags & AText_disabled)
          {
          cfmt_p->dwEffects |= CFE_DISABLED;
          }
        }

      if (style.m_effect_mask & AText_superscript)
        {
        cfmt_p->dwMask |= CFM_SUPERSCRIPT;

        if (style.m_effect_flags & AText_superscript)
          {
          cfmt_p->dwEffects |= CFE_SUPERSCRIPT;
          }
        }
      else
        {
        if (style.m_effect_mask & AText_subscript)
          {
          cfmt_p->dwMask |= CFM_SUBSCRIPT;

          if (style.m_effect_flags & AText_subscript)
            {
            cfmt_p->dwEffects |= CFE_SUBSCRIPT;
            }
          }
        }
      }
    }

} // End unnamed namespace


//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * ARichEditOS::ms_default_class_p;
void *                ARichEditOS::ms_lib_handle;

//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void ARichEditOS::initialize()
  {
  ms_lib_handle = ::LoadLibrary(ARICH_EDIT_DLL_NAME);
  A_VERIFY_OS(ms_lib_handle, "ARichEditOS::ms_lib_handle", ARichEditOS);
  ms_default_class_p = new AMessageTargetClass(RICHEDIT_CLASS);
  }

//---------------------------------------------------------------------------------------

void ARichEditOS::deinitialize()
  {
  delete ms_default_class_p;
  ::FreeLibrary((HMODULE)ms_lib_handle);
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         parent_p - a pointer parent AWindow to this object.  This *MUST* be
//             given and cannot be nullptr.  The set_parent() method does not work properly
//             with OS graphical controls.
// Arg         initial - initial AgogCore\AString.  (Default AString::ms_empty)
// Arg         flags - see eRichFlag
// Arg         font - AFont to use (Default AFont::ms_default)
// Arg         region - position and area of the edit box.  If width set to
//             Size_auto, the width will be the client width of the
//             parent minus the x position.  If height set to Size_auto,
//             the height will be the client height of the parent minus the y position.
//             (Default ARegion(0, 0, Size_auto, Size_auto))
// Notes:      Initial settings (modified with appropriately matching methods):
//               key events        - true - on_character(), on_key_press(), and on_key_release() are called
//               mouse events      - false - on_mouse_press(), on_mouse_release(), on_mouse_moving(), and on_mouse_spinning() are not called
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
// Author(s):   Conan Reis
ARichEditOS::ARichEditOS(
  AWindow *       parent_p,
  const AString & initial,      // = AString::ms_empty
  uint32_t        flags,        // = RichFlag__default
  const AFont &   font,         // = AFont::ms_default
  const ARegion & region        // = AWindow::ms_region_auto_def
  ) :
  AEditOS(parent_p, "", (flags & RichFlag_word_wrap) != 0u, font, region, (flags & RichFlag_show_selection_always) != 0u, false),
  m_rich_flags(RichFlag__default),
  m_ignore_on_modified_count(0u),
  m_ole_p(nullptr),
  m_tom_p(nullptr)
  {
  m_class_p = ms_default_class_p;

  AString edit_str(initial);

  convert_to_edit_linebreaks(&edit_str);

  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Only the styles and the initial string / title are different from AWindow
  DWORD styles = WS_CHILD | WS_TABSTOP
    | ((flags & RichFlag_show_selection_always) ? ES_NOHIDESEL : 0u);

  if (flags & RichFlag_single_line)
    {
    styles |= ES_AUTOHSCROLL;
    }
  else
    {
    styles |= ES_MULTILINE | ES_WANTRETURN | ES_DISABLENOSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL
      | ((flags & RichFlag_word_wrap) ? 0u : WS_HSCROLL);
    }

  // Currently unused interesting styles:
  //   ES_CENTER - Centers text in a single-line or multiline edit control.
  //   ES_LEFT   - Left aligns text.
  //   ES_RIGHT  - Right aligns text in a single-line or multiline edit control.

  // Note that the ES_NOHIDESEL and ES_AUTOHSCROLL styles may only be set on the
  // creation of the edit control and may not be added or removed afterward using the
  // append_style() or the remove_style() methods.
  m_os_handle = ::CreateWindowEx(
    0,           // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    edit_str,   // title
    styles,
    region.m_x,
    region.m_y,
    (region.m_width == Size_auto) ? parent_p->get_width_client() - region.m_x : region.m_width,
    (region.m_height == Size_auto) ? parent_p->get_height_client() - region.m_y : region.m_height,
    m_parent_handle, 
    nullptr,        // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);       // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "ARichEditOS()", ARichEditOS);

  // Get IRichEditOle interface
  ::SendMessage(m_os_handle, EM_GETOLEINTERFACE , 0u, reinterpret_cast<LPARAM>(&m_ole_p));

  if (m_ole_p)
    {
    reinterpret_cast<IRichEditOle *>(m_ole_p)->QueryInterface(IID_ITextDocument, &m_tom_p);
    }
  
  common_setup();

  // Set text mode
  ::SendMessage(m_os_handle, EM_SETTEXTMODE, TM_RICHTEXT | TM_MULTILEVELUNDO | TM_SINGLECODEPAGE, 0u);

  enable_events();
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
ARichEditOS::~ARichEditOS()
  {
  if (m_ole_p)
    {
    reinterpret_cast<IRichEditOle *>(m_ole_p)->Release();
    }
  }


// $Revisit - CReis Here are some useful RichEdit messages that have yet to be implemented.
//   More undo/redo actions
//   EM_GETCHARFORMAT
//   EM_SETFONTSIZE
//   EM_SETPARAFORMAT
//   EM_EXLINEFROMCHAR   - longer lengths for get_row_from_index()
//
//   EM_AUTOURLDETECT    EM_GETAUTOURLDETECT
//   EM_SETSCROLLPOS     EM_GETSCROLLPOS     EM_SHOWSCROLLBAR
//   EM_SETEDITSTYLE     EM_GETEDITSTYLE
//   EM_SETOPTIONS       EM_GETOPTIONS
//   EM_SETWORDWRAPMODE
//   EM_GETSELTEXT       - EM_GETTEXTRANGE works better in combination with get_selection(start_p, end_p)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Text Style Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Set text default background colour.
// Arg         color - If the colour is AColor::ms_default, then the system background
//             colour is used.
// Author(s):   Conan Reis
void ARichEditOS::set_text_background(
  const AColor & color // = AColor::ms_default
  )
  {
  // Note that using TRUE and having the system pick the window background colour doesn't seem to work.
  ::SendMessage(m_os_handle, EM_SETBKGNDCOLOR, FALSE, color.is_valid() ? COLORREF(color) : AColor::get_element_os(COLOR_WINDOW));
  }

//---------------------------------------------------------------------------------------
// Sets the style / character formatting for the entire text.
// Arg         style - see ATextStyle
// See:        ATextStyle
// Author(s):   Conan Reis
void ARichEditOS::set_text_style(const ATextStyle & style)
  {
  CHARFORMAT2 cfmt;

  a_richeditos_prep_charformat(style, &cfmt);

  enable_undo(false);

  ::SendMessage(m_os_handle, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&cfmt));

  enable_undo();
  }

//---------------------------------------------------------------------------------------
// Sets the style / character formatting for the specified range of text.
// Arg         start - starting character index position
// Arg         end - ending character index position
// Arg         style - see ATextStyle
// Arg         preserve_selection - if true, keep the previous selection as it was prior
//             to this call, else change the selection to the indicated range.
// See:        ATextStyle
// Author(s):   Conan Reis
void ARichEditOS::set_text_style(
  uint32_t           start,
  uint32_t           end,
  const ATextStyle & style,
  bool               preserve_selection // = true
  )
  {
  uint32_t old_start = 0u;
  uint32_t old_end   = 0u;

  set_flag(RichFlag_ignore_selection_change);

  // Preserve previous selection on request
  if (preserve_selection)
    {
    get_selection(&old_start, &old_end);
    }

  select(start, end);
  set_text_style_selection(style);

  if (preserve_selection)
    {
    select(old_start, old_end);
    }

  clear_flag(RichFlag_ignore_selection_change);
  }

//---------------------------------------------------------------------------------------
// Sets the style / character formatting for the currently selected text.
// Arg         style - see ATextStyle
// See:        ATextStyle
// Author(s):   Conan Reis
void ARichEditOS::set_text_style_selection(const ATextStyle & style)
  {
  CHARFORMAT2 cfmt;

  a_richeditos_prep_charformat(style, &cfmt);

  enable_undo(false);

  ::SendMessage(m_os_handle, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&cfmt));

  enable_undo();
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Drawing Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Increments the freeze count.  If the freeze count is nonzero, screen
//             updating is disabled for this rich edit.  This allows a sequence of
//             operations to be performed without the performance loss and flicker of
//             screen updating.  To decrement the freeze count, call the unfreeze()
//             method.
// Author(s):   Conan Reis
uint ARichEditOS::freeze()
  {
  long freeze_count;

  reinterpret_cast<ITextDocument *>(m_tom_p)->Freeze(&freeze_count);

  return freeze_count;
  }

//---------------------------------------------------------------------------------------
// Decrements the freeze count.  If the freeze count is nonzero, screen
//             updating is disabled for this rich edit.  This allows a sequence of
//             operations to be performed without the performance loss and flicker of
//             screen updating.  To increment the freeze count, call the freeze() method.
// Author(s):   Conan Reis
uint ARichEditOS::unfreeze(
  bool keep_vscroll // = true
  )
  {
  long freeze_count;

  int32_t row = 0;
  
  if (keep_vscroll)
    {
    row = get_row_visible_first();
    }

  reinterpret_cast<ITextDocument *>(m_tom_p)->Unfreeze(&freeze_count);

  if (keep_vscroll)
    {
    row -= get_row_visible_first();

    if (row != 0)
      {
      ::SendMessage(m_os_handle, EM_LINESCROLL, 0u, row);
      }
    }

  return freeze_count;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Settings Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Enables/disables misc. events for rich edit control.
// Arg         key_events - enables/disables on_character(), on_key_press(),
//             on_key_release()  (Default true)
// Arg         mouse_events - enables/disables on_mouse_press(), on_mouse_release(),
//             on_mouse_moving(), on_mouse_spinning()  (Default false)
// Notes:      All these rich edit events can set the RichFlag_ignore_control_event flag if
//             a particular event should be ignored.
// Author(s):   Conan Reis
void ARichEditOS::enable_events(
  bool key_events,  // = true
  bool mouse_events // = false
  )
  {
  // Event Masks
    //ENM_CHANGE          - Sends EN_CHANGE notifications.
    //ENM_UPDATE          - Sends EN_UPDATE notifications. 
    //ENM_SELCHANGE       - Sends EN_SELCHANGE notifications.
    //ENM_SCROLL          - Sends EN_HSCROLL and EN_VSCROLL notifications.
    //ENM_KEYEVENTS       - Sends EN_MSGFILTER notifications for keyboard events.
    //ENM_MOUSEEVENTS     - Sends EN_MSGFILTER notifications for mouse events.
    //ENM_SCROLLEVENTS    - Sends EN_MSGFILTER notifications for mouse wheel events.

    //ENM_LINK            - Rich Edit 2.0 and later: Sends EN_LINK notifications when the mouse pointer is over text that has the CFE_LINK and one of several mouse actions is performed.
    //ENM_CORRECTTEXT     - Sends EN_CORRECTTEXT notifications.
    //ENM_DRAGDROPDONE    - Sends EN_DRAGDROPDONE notifications.
    //ENM_DROPFILES       - Sends EN_DROPFILES notifications.

    //ENM_REQUESTRESIZE   - Sends EN_REQUESTRESIZE notifications.  Notifies parent that the edit's contents are either smaller or larger than its window size.
    //ENM_OBJECTPOSITIONS - Sends EN_OBJECTPOSITIONS notifications.
    //ENM_PROTECTED       - Sends EN_PROTECTED notifications.

  uint event_flags = ENM_UPDATE | ENM_CHANGE | ENM_SELCHANGE | ENM_SCROLL;

  if (key_events)
    {
    event_flags |= ENM_KEYEVENTS;
    }

  if (mouse_events)
    {
    event_flags |= ENM_MOUSEEVENTS | ENM_SCROLLEVENTS;
    }

  ::SendMessage(m_os_handle, EM_SETEVENTMASK, 0u, event_flags);
  }

//---------------------------------------------------------------------------------------
// Enables or disables the calling of the event method on_modified().  This
//             is done using a counter so that calls to enable and disable the event can
//             be nested - each call to enable the event decrements the count and each
//             call to disable the event increments the count.  If the count is zero
//             on_modified() is called and if the count is non-zero it is not called.
// Arg         enable - true if on_modified() should be called, false if not
// See:        on_modified()
// Author(s):   Conan Reis
void ARichEditOS::enable_on_modified(
  bool enable // = true
  )
  {
  if (enable)
    {
    if (m_ignore_on_modified_count == 1u)
      {
      // Enable on_modified event
      ::SendMessage(m_os_handle, EM_SETEVENTMASK, 0u, ::SendMessage(m_os_handle, EM_GETEVENTMASK, 0u, 0u) | ENM_CHANGE);
      }

    if (m_ignore_on_modified_count > 0u)
      {
      // Decrement ignore count
      m_ignore_on_modified_count--;
      }
    }
  else
    {
    // Increment ignore count
    m_ignore_on_modified_count++;

    if (m_ignore_on_modified_count == 1u)
      {
      // Disable on_modified event
      ::SendMessage(m_os_handle, EM_SETEVENTMASK, 0u, ::SendMessage(m_os_handle, EM_GETEVENTMASK, 0u, 0u) & ~ENM_CHANGE);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Determines if in insert mode (true) or overwrite mode (false)
// Returns:    insert mode (true) or overwrite mode (false)
// Author(s):   Conan Reis
bool ARichEditOS::is_insert_mode() const
  {
  ITextSelection * itxt_sel_p;

  reinterpret_cast<ITextDocument *>(m_tom_p)->GetSelection(&itxt_sel_p);

  long flags;

  itxt_sel_p->GetFlags(&flags);

  return (flags & tomSelOvertype) == 0u;
  }

//---------------------------------------------------------------------------------------
// Sets the maximum number of characters (not counting the null terminator) that the user
// is allowed to enter/paste.
//
// #Notes
//   The default maximum characters prior to this method being called is 32,767. When
//   calculating, newline characters ('\n') count as 2 characters since the carriage
//   return character ('\r') is included with it in line breaks ("\r\n").
//
// #Author(s) Conan Reis
void ARichEditOS::set_char_limit(
  // Maximum characters allowed.  If set to AEdit_max_characters, it will allow the
  // maximum number of characters possible.  For Windows 95-ME this is 64KB (65536).
  // For Windows NT/2000 this is 4GB.
  uint32_t max_chars
  )
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

  ::SendMessage(m_os_handle, EM_EXLIMITTEXT, max_chars ? max_chars : 1u, 0u);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Selection Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Determines character index position nearest to pos
// Returns:    character index position nearest to pos
// Arg         client_pos - 
// See:        get_row_from_pos()
// Modifiers:   virtual
// Author(s):   Conan Reis
uint32_t ARichEditOS::get_index_from_pos(const AVec2i & client_pos) const
  {
  return uint32_t(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, reinterpret_cast<LPARAM>(&client_pos)));
  }

//---------------------------------------------------------------------------------------
// Arg          pos - 
// Arg          row_p - (Default nullptr)
// Author(s):    Conan Reis
bool ARichEditOS::get_row_from_pos(
  const AVec2i & client_pos,
  uint32_t *         row_p // = nullptr
  ) const
  {
  if (row_p)
    {
    *row_p = uint32_t(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, reinterpret_cast<LPARAM>(&client_pos)));
    }

  return true;  // $Revisit - CReis is it possible to determine if virtual space was hit rather than a valid character?
  }

//---------------------------------------------------------------------------------------
// Indents selected rows/lines by specified `space_count` spaces - then selects indented
// lines.
//
// Params:  
//   space_count: number of space characters to indent
//   
// Notes:  
//   - Spaces inserted just prior to first non-space (space or tab) character on each row.
//   - Rows with no non-space characters are not indented.
//   - If the range ends just after a line break, the following row is not indented - at
//     least one character on a row must be included for it to be indented
//   - Mimics indent behaviour of Visual Studio editor - though not selection.
// 
// See:        unindent_selection(), AString::line_unindent()
// Author(s):   Conan Reis
void ARichEditOS::indent_selection(
  uint space_count // = AString_indent_spaces_def
  )
  {
  uint32_t row_begin;
  uint32_t row_end;

  get_selection_rows(&row_begin, &row_end);

  // This command holds back updating the appearance of the edit box.
  // It also makes this command RichEdit specific.
  freeze();

  select_rows(row_begin, row_end);

  AString str(get_selection());

  str.line_indent(space_count);
  replace_selection(str, true);

  // Unfreeze must occur before last select so that it will be visible
  unfreeze();

  select_rows(row_begin, row_end);
  }

//---------------------------------------------------------------------------------------
// Remove one level of indentation on selected lines.
// See:        indent_selection()
// Author(s):   Conan Reis
void ARichEditOS::unindent_selection(
  uint space_count, // = AString_indent_spaces_def
  uint tab_stops    // = AString_tab_stop_def
  )
  {
  uint32_t row_begin;
  uint32_t row_end;

  get_selection_rows(&row_begin, &row_end);

  // This command holds back updating the appearance of the edit box.
  // It also makes this command RichEdit specific.
  freeze();

  select_rows(row_begin, row_end);

  AString str(get_selection());

  str.line_unindent(space_count, tab_stops);
  replace_selection(str, true);

  // Unfreeze must occur before last select so that it will be visible
  unfreeze();

  select_rows(row_begin, row_end);
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
//             both equal the current position of the caret.  If start is 0 and end is
//             UINT32_MAX then the range includes everything.
// Modifiers:   virtual from AEditLineOS
// Author(s):   Conan Reis
void ARichEditOS::get_selection(
  uint32_t * start_p,
  uint32_t * end_p
  ) const
  {
  CHARRANGE range;

  ::SendMessage(m_os_handle, EM_EXGETSEL, 0u, reinterpret_cast<LPARAM>(&range));

  if (start_p)
    {
    *start_p = range.cpMin;
    }

  if (end_p)
    {
    *end_p = range.cpMax;
    }
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
// Modifiers:   virtual from AEditLineOS
// Author(s):   Conan Reis
void ARichEditOS::select(
  uint32_t start,
  uint32_t end
  )
  {
  CHARRANGE range = {(LONG)start, (LONG)end};
  // EM_EXSETSEL used instead of EM_SETSEL since it is not limited to 64K.
  ::SendMessage(m_os_handle, EM_EXSETSEL, 0u, reinterpret_cast<LPARAM>(&range));
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Text Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Appends specified text with the supplied style to the end of the edit.
// Arg         str - text to append
// Arg         undoable - indicates whether this action should be undoable (true) or
//             not (false).
// Author(s):   Conan Reis
void ARichEditOS::append_style(
  const AString &    str,
  const ATextStyle & style,
  bool  undoable // = false
  )
  {
  select_end();
  set_text_style_selection(style);
  replace_selection(str, undoable);
  }

//---------------------------------------------------------------------------------------
// Converts string from this edit box's native line break convention to the
//             "external" line break convention.
// Arg         str_p - string to convert
// See:        convert_to_edit_linebreaks(), set_break_conversion(), m_breaks_expected
// Modifiers:   virtual
// Author(s):   Conan Reis
void ARichEditOS::convert_from_edit_linebreaks(AString * str_p) const
  {
  switch (m_breaks_expected)
    {
    case ALineBreak_unix:  // \n    [Standard] C++, Unix, Mac-OSX
      str_p->line_break_rich2unix();
      break;

    case ALineBreak_dos:   // \r\n  PC files, edit boxes
      str_p->line_break_rich2dos();
      break;

    //case ALineBreak_rich:  // \r    RichEdit boxes
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
void ARichEditOS::convert_to_edit_linebreaks(AString * str_p) const
  {
  switch (m_breaks_expected)
    {
    case ALineBreak_unix:  // \n    [Standard] C++, Unix, Mac-OSX
      str_p->line_break_unix2rich();
      break;

    case ALineBreak_dos:   // \r\n  PC files, edit boxes
      str_p->line_break_dos2rich();
      break;

    //case ALineBreak_rich:  // \r    RichEdit boxes
    //case ALineBreak__default:
      // Already in desired convention - do nothing
    }
  }

//---------------------------------------------------------------------------------------
// Gets the number of characters in the edit control.
// ***Note*** Just to keep you on your toes, the number of characters returned by this
// method may slightly differ from the actual number of characters returned when calling
// `get_text()`.
// 
// Returns:    number of characters in the edit control
// Modifiers:   virtual
// Author(s):   Conan Reis
uint32_t ARichEditOS::get_length() const
  {
  // Note that including the GTL_PRECISE flag does not seem to increase the accuracy for
  // one off errors.
  GETTEXTLENGTHEX length_ex = {GTL_NUMCHARS, CP_ACP};

  return uint32_t(::SendMessage(m_os_handle, EM_GETTEXTLENGTHEX, reinterpret_cast<WPARAM>(&length_ex), 0));
  }

//---------------------------------------------------------------------------------------
// Arg         str_p - 
// Modifiers: virtual from AEditLineOS
// Author(s): Conan Reis
void ARichEditOS::get_text(AString * str_p) const
  {
  uint32_t length = get_length();

  if (length == 0u)
    {
    str_p->empty();

    return;
    }

  str_p->ensure_size_empty(length);

  // $Revisit - CReis Should move from ANSI (CP_ACP) to UTF-8 (CP_UTF8)
  const CHAR * def_char_p    = "~";  // Ensuring a non-common ASCII character to substitute.
  BOOL         used_def_char = FALSE;
  GETTEXTEX    str_info      = {str_p->get_size(), GT_DEFAULT, CP_ACP, def_char_p, &used_def_char};

  //or EM_STREAMOUT
  // $Note - CReis The EM_GETTEXTEX documentation says that the null terminator is
  // included in the returned length though it is *NOT* included.
  uint32_t actual_length = uint32_t(::SendMessage(m_os_handle, EM_GETTEXTEX, reinterpret_cast<WPARAM>(&str_info), reinterpret_cast<LPARAM>(str_p->as_cstr_writable())));

  if (actual_length == 0u)
    {
    // $Revisit - CReis For some inexplicable reason EM_GETTEXTEX can fail occasionally
    // when the system is set to certain locales though getting a text range seems to
    // succeed in these cases.

    // Try again
    get_text(0, length, str_p);
    return;
    }

  str_p->set_length(actual_length);
  convert_from_edit_linebreaks(str_p);
  }

//---------------------------------------------------------------------------------------
// Arg         start - 
// Arg         end - 
// Arg         str_p - 
// Modifiers: virtual from AEditLineOS
// Author(s): Conan Reis
void ARichEditOS::get_text(
  uint32_t  start,
  uint32_t  end,
  AString * str_p
  ) const
  {
  uint32_t length = (start <= end) ? end - start : 0u;

  if (length)
    {
    // Add space for null character and a little extra for insurance
    str_p->ensure_size_empty(length + 2u);

    TEXTRANGE text_range = {{(LONG)start, (LONG)end}, str_p->as_cstr_writable()};

    length = uint32_t(::SendMessage(m_os_handle, EM_GETTEXTRANGE, 0u, reinterpret_cast<LPARAM>(&text_range)));
    str_p->set_length(length);

    convert_from_edit_linebreaks(str_p);
    }
  else
    {
    str_p->empty();
    }
  }

//---------------------------------------------------------------------------------------
// Sets entire text string
// Arg         str - string to set
// Arg         undoable - indicates whether this action should be undoable (true) or
//             not (false).
// See:        set_title()
// Author(s):   Conan Reis
void ARichEditOS::set_text(
  const AString & str,
  bool            undoable // = false
  )
  {
  SETTEXTEX set_txt = {undoable ? (DWORD)ST_KEEPUNDO : (DWORD)ST_DEFAULT, ADef_uint32};
  AString   edit_str(str);

  convert_to_edit_linebreaks(&edit_str);
  ::SendMessage(m_os_handle, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&set_txt), reinterpret_cast<LPARAM>(edit_str.as_cstr()));
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Position & Hit Test Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets visible upper-left corner point in virtual text space pixels.
// Returns:    upper-left corner virtual point
// See:        set_scroll_pos()
// Author(s):   Conan Reis
AVec2i ARichEditOS::get_scroll_pos() const
  {
  AVec2i vpos;

  ::SendMessage(m_os_handle, EM_GETSCROLLPOS, 0u, reinterpret_cast<LPARAM>(&vpos));

  return vpos;
  }

//---------------------------------------------------------------------------------------
// Sets visible upper-left corner point in virtual text space pixels.
// Arg         virtual_pos - upper-left corner virtual point
// See:        get_scroll_pos()
// Author(s):   Conan Reis
void ARichEditOS::set_scroll_pos(const AVec2i & virtual_pos)
  {
  ::SendMessage(m_os_handle, EM_SETSCROLLPOS, 0u, reinterpret_cast<LPARAM>(&virtual_pos));
  }

//---------------------------------------------------------------------------------------
// Gets the caret character index - i.e. the "active" index of the selection.
// Author(s):   Conan Reis
uint32_t ARichEditOS::get_caret_index() const
  {
  uint32_t idx_begin;
  uint32_t idx_end;

  // Get uncategorized selection "anchor" and "active" indexes
  get_selection(&idx_begin, &idx_end);

  // If no selection it does not matter which index is active.
  if (idx_begin == idx_end)
    {
    return idx_end;
    }

  ITextSelection * itxt_sel_p;

  reinterpret_cast<ITextDocument *>(m_tom_p)->GetSelection(&itxt_sel_p);

  long flags;

  itxt_sel_p->GetFlags(&flags);

  return (flags & tomSelStartActive)
    ? idx_begin
    : idx_end;
  }

//---------------------------------------------------------------------------------------
// Retrieves the client area coordinates of the specified character index.
// Arg         index - 
// Arg         x_p - 
// Arg         y_p - 
// See:        get_position_index()
// Modifiers:   virtual
// Author(s):   Conan Reis
void ARichEditOS::get_index_position(
  uint32_t index,
  int * x_p,
  int * y_p
  ) const
  {
  POINTL pos;

  ::SendMessage(m_os_handle, EM_POSFROMCHAR, WPARAM(&pos), index);

  *x_p = pos.x;
  *y_p = pos.y;
  }

//---------------------------------------------------------------------------------------
// Determines the character index closest to the specified client area point.
// Returns:     
// Arg         pos - 
// Arg         index_p - (Default nullptr)
// See:        get_index_position()
// Modifiers:   virtual
// Author(s):   Conan Reis
bool ARichEditOS::get_position_index(
  const AVec2i & pos,
  uint32_t *         index_p // = nullptr
  ) const
  {
  // $Revisit - CReis is it possible to determine if virtual space was hit rather than a valid character?
  if (index_p)
    {
    *index_p = uint32_t(::SendMessage(m_os_handle, EM_CHARFROMPOS, 0u, LPARAM(&pos)));
    }
  return true;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Clipboard Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Pastes plain text info from clipboard into current selection.
// 
// See:  
//   AEditLineOS::clipboard_copy_selected(), AEditOS::clipboard_copy_plain_sel_or_row()
//   
// Author(s):   Conan Reis
void ARichEditOS::clipboard_paste_plain()
  {
  ::SendMessage(m_os_handle, EM_PASTESPECIAL, CF_UNICODETEXT, NULL);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Undo Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// 
// Arg         accumulate_undo_info - true if accumulating undo information should be
//             resumed
// See:        on_modified()
// Author(s):   Conan Reis
void ARichEditOS::enable_undo(
  bool accumulate_undo_info // = true
  )
  {
  if (accumulate_undo_info)
    {
    reinterpret_cast<ITextDocument *>(m_tom_p)->Undo(tomResume, nullptr);
    }
  else
    {
    reinterpret_cast<ITextDocument *>(m_tom_p)->Undo(tomSuspend, nullptr);
    }
  }

//---------------------------------------------------------------------------------------
// Determines if a redo can be performed
// See:        redo(), undo()
// Author(s):   Conan Reis
bool ARichEditOS::is_redo() const
  {
  return ::SendMessage(m_os_handle, EM_CANREDO, 0u, 0u) != 0u;
  }

//---------------------------------------------------------------------------------------
// Redoes last undone action
// See:        is_redo(), undo()
// Author(s):   Conan Reis
void ARichEditOS::redo()
  {
  ::SendMessage(m_os_handle, EM_REDO, 0u, 0u);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever an OS common control sends a notification message.
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         uint32_t code - message code to parse
// Arg         result_p - pointer to store return info for message
// Notes:      This method should be overridden by OS common control objects and then
//             parse out any appropriate messages.
//             For a list of OS standard /system controls see the "Notes" section of the
//             AMessageTargetClass(os_class_name) constructor.
// Modifiers:   virtual (from AWindow)
// Author(s):   Conan Reis
bool ARichEditOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  switch (info_p->code)
    {
    case EN_MSGFILTER:
      {
      if (!is_subclassed())
        {
        MSGFILTER * filter_p     = reinterpret_cast<MSGFILTER *>(info_p);
        uint         msg          = filter_p->msg;
        bool        call_default = true;
        bool        handled;

        clear_flag(RichFlag_ignore_control_event);

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Keyboard Events
        if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST))
          {
          call_default = parse_keyboard(msg, filter_p->wParam, filter_p->lParam, &handled);
          }
        else
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Mouse Events
          if ((msg >= WM_MOUSEFIRST) && (msg <= WM_MOUSELAST))
            {
            call_default = parse_mouse(msg, filter_p->wParam, filter_p->lParam, &handled);
            }
          }

        if (!call_default || (m_rich_flags & RichFlag_ignore_control_event))
          {
          *result_p = TRUE;

          return false;
          }
        }
      }
      break;

    case EN_SELCHANGE:
      // Current selection has changed - including any empty selection (i.e. caret move).
      if ((m_rich_flags & RichFlag_ignore_selection_change) == 0u)
        {
        SELCHANGE * selchange_p = reinterpret_cast<SELCHANGE *>(info_p);

        on_selecting(selchange_p->chrg.cpMin, selchange_p->chrg.cpMax);
        }
      break;
    }

  // EN_CORRECTTEXT  - The EN_CORRECTTEXT message notifies a rich edit control parent window that a SYV_CORRECT gesture occurred, giving the parent window a chance to cancel correcting the text.
  // EN_DRAGDROPDONE - The EN_DRAGDROPDONE notification message notifies a rich edit control's parent window that the drag-and-drop operation has completed.
  // EN_DROPFILES    - The EN_DROPFILES message notifies a rich edit control parent window that the user is attempting to drop files into the control. A rich edit control sends this notification message in the form of a WM_NOTIFY message when it receives the WM_DROPFILES message.
  // EN_LINK         - A rich edit control sends EN_LINK messages when it receives various messages, for example, when the user clicks the mouse or when the mouse pointer is over text that has the CFE_LINK effect.

  return true;
  }

//---------------------------------------------------------------------------------------
// Current selection has changed - including any empty selection (i.e. caret
//             move).
// Arg         start - starting index of the selection.
// Arg         end - position of the first nonselected index after the end of the
//             selection.
// Notes:      If there is no text currently selected, the start and end positions will
//             both equal the current position of the caret.  If start is 0 and end is
//             UINT32_MAX then the range includes everything.
//             This event will not be called if RichFlag_ignore_selection_change is set.
//             Override this method for custom behaviour.
// Modifiers:   virtual
// Author(s):   Conan Reis
void ARichEditOS::on_selecting(
  uint32_t start,
  uint32_t end
  )
  {
  //A_DPRINT("%s ARichEditOS::on_selecting(%u, %u) - 0x%p\n", A_SOURCE_STR, start, end, this);
  }


