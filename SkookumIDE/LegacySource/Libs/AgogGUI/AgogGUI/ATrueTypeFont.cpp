// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATrueTypeFont class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogIO\ADebugOS.hpp>
//#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#if defined(_MSC_VER)
  // Removes unnecessary warnings - often only part of a structure needs to be initialized
  #pragma warning( disable : 4701 ) // local variable may be used without having been initialized
#endif


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Constructor for a MS Windows GDI TrueType font
// Returns:     itself
// Arg          name - identifying name of the font
// Arg          pixel_height - height of font in pixels (ascent + descent and not
//              including the leading value) of characters
// Arg          weight - common values are AFontWeight_normal and AFontWeight_bold.  See eAFontWeight in
//              AgogGUI\AFontSystemBase.hpp
// Arg          italicised - boolean indicating whether font is italicised or not
// Arg          underlined - boolean indicating whether font is underlined or not
// Arg          spacing - positive or negative extra pixel spacing between
//              characters used when drawing text
// Arg          antialias - specifies whether the font should use antialiasing
// Examples:    ATrueTypeFont font(Arial", 20);
// Notes:       The returned AFontSystemBase has automatically been referenced.  It should
//              be dereferenced to free it.
// Author(s):    Conan Reis
ATrueTypeFont::ATrueTypeFont(
  const AString & name,
  int             pixel_height,
  uint32_t        weight,
  bool            italicised,
  bool            underlined,
  int             spacing,
  bool            antialias
  ) :
  AFontSystemBase(name),
  m_font_handle_p(nullptr)
  {
  m_font_handle_p = ::CreateFont(
    -pixel_height,                                   // Height
    0,                                               // Average width
    0,                                               // Escapement
    0,                                               // Orientation
    weight,                                          // Weight
    italicised,                                      // Italic
    underlined,                                      // Underline
    false,                                           // StrikeOut
    ANSI_CHARSET,                                    // CharSet
    OUT_TT_PRECIS,                                   // OutPrecision
    CLIP_DEFAULT_PRECIS,                             // ClipPrecision
    antialias ? CLEARTYPE_QUALITY : DRAFT_QUALITY,   // Quality  DRAFT_QUALITY is used rather than PROOF_QUALITY because it allows scaling of bitmap fonts
    DEFAULT_PITCH | AFontFamily_dont_care,           // PitchAndFamily
    name);                                           // FaceName
  A_VERIFY_OS(m_font_handle_p != nullptr, "", ATrueTypeFont);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the rest of the metrics

  TEXTMETRIC text_metrics;
  HDC        info_dc = HDC(get_info_dc());
  HGDIOBJ    old_obj = ::SelectObject(info_dc, m_font_handle_p);

  char chosen_name_p[LF_FACESIZE + 1];
  ::GetTextFace(info_dc, LF_FACESIZE, chosen_name_p);

  if (m_name != chosen_name_p)
    {
    m_name = chosen_name_p;
    }

  A_VERIFY_OS(::GetTextMetrics(info_dc, &text_metrics), "", ATrueTypeFont);

  f32 vert_pixels_per_inch = f32(::GetDeviceCaps(info_dc, LOGPIXELSY));

  if (old_obj)
    {
    ::SelectObject(info_dc, old_obj);
    }

  release_info_dc(info_dc);

  // Store actual metrics as opposed to requested metrics
  m_height       = text_metrics.tmHeight;
  m_point_size   = f32(pixel_height) * 72.0f / vert_pixels_per_inch;  // point size = (pixel size * 72) / vertical pixels per inch
  m_avg_width    = text_metrics.tmAveCharWidth;
  m_weight       = text_metrics.tmWeight;
  m_ascent       = text_metrics.tmAscent;
  m_descent      = text_metrics.tmDescent;
  m_leading      = text_metrics.tmExternalLeading;
  m_spacing      = spacing;
  m_font_family  = eAFontFamily(text_metrics.tmPitchAndFamily & AFontFamily__mask);
  m_italicised   = text_metrics.tmItalic != 0;
  m_proportional = (text_metrics.tmPitchAndFamily & TMPF_FIXED_PITCH) == TMPF_FIXED_PITCH; // Note that the define seems opposite to its meaning
  m_underlined   = text_metrics.tmUnderlined != 0;
  m_antialiased  = antialias;

  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
ATrueTypeFont::~ATrueTypeFont()
  {
  ::DeleteObject(m_font_handle_p);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATrueTypeFont::free()
  {
  delete this;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void ATrueTypeFont::set_char_spacing(int extra_pixels)
  {
  m_spacing = extra_pixels;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets the font type
// Returns:    AFontType_true_type
// Author(s):   Conan Reis
eAFontType ATrueTypeFont::get_type() const
  {
  return AFontType_true_type;
  }

//---------------------------------------------------------------------------------------
//  Determines the width that the specified AString and range of text
//              would be when drawing with this font.
// Returns:     width of given range of text
// Arg          text - text to determine width of
// Arg          start_pos - character index position in text where the 0 x pixel
//              position begins, i.e. the text is clipped. (Default 0)
// Arg          char_count - number of characters to get width of starting at
//              start_pos.  If it is set to ALength_remainder, it is set to the length
//              of the text minus the start_pos.  (Default ALength_remainder)
// Notes:       All control characters including tab, new-line, and carriage
//              return are normaly represented by a outlined square, so the width
//              of such strings will be inaccurate.
// Author(s):    Conan Reis
int ATrueTypeFont::get_width(
  const AString & text,
  uint32_t        start_pos, // = 0
  uint32_t        char_count  // = ALength_remainder
  ) const
  {
  SIZE    size;
  HDC     info_dc     = HDC(get_info_dc());
  HGDIOBJ old_obj     = ::SelectObject(info_dc, m_font_handle_p);
  int     old_spacing = ::SetTextCharacterExtra(info_dc, m_spacing);

  if (char_count == ALength_remainder)
    {
    char_count = text.get_length() - start_pos;
    }
  A_VERIFY_OS(::GetTextExtentPoint32(info_dc, text.as_cstr() + start_pos, char_count, &size), "", ATrueTypeFont);

  ::SelectObject(info_dc, old_obj);
  ::SetTextCharacterExtra(info_dc, old_spacing);
  release_info_dc(info_dc);

  return size.cx;
  }

//---------------------------------------------------------------------------------------
//  Determines if and where there is a character hit at the given x
//              pixel position when the drawing specified AString with this font.
// Returns:     true if a character is hit, false if the x 
// Arg          text - text to determine which character is hit
// Arg          x_pos - x pixel position to do hit test with
// Arg          char_pos_p - address to store index location of hit character.
//              It is undefined if no character is hit.  It is ignored if it is
//              nullptr.  (Default nullptr)
// Arg          start_pos - character index position in text where the 0 x pixel
//              position begins, i.e. the text is clipped. (Default 0)
// Notes:       All control characters including tab, new-line, and carriage
//              return are normaly represented by a outlined square, so the width
//              of such strings will be inaccurate.
//              The start_pos argument is used for efficiency so that a substring
//              AString does not need to be created along with its memory
//              allocation overhead etc.
// Author(s):    Conan Reis
bool ATrueTypeFont::is_hit_char(
  const AString & text,
  int             x_pos,
  uint32_t *          char_pos_p, // = nullptr
  uint32_t        start_pos   // = 0
  ) const
  {
  bool found = false;

  if (x_pos >= 0)
    {
    SIZE    size;
    int     pos;
    HDC     info_dc     = HDC(get_info_dc());
    HGDIOBJ old_font    = ::SelectObject(info_dc, m_font_handle_p);  // Save old font
    int     length      = text.get_length();
    int     old_spacing = ::SetTextCharacterExtra(info_dc, m_spacing);

    // Find character hit position
    A_VERIFY_OS(::GetTextExtentExPoint(info_dc, &(text.as_cstr()[start_pos]), length, x_pos, &pos, nullptr, &size), "", ATrueTypeFont);
    pos += start_pos;

    // replace old settings
    ::SelectObject(info_dc, old_font);
    ::SetTextCharacterExtra(info_dc, old_spacing);

    release_info_dc(info_dc);

    if (pos < length)
      {
      found = true;
      if (char_pos_p)
        {
        *char_pos_p = pos;
        }
      }
    }
  return found;
  }

//---------------------------------------------------------------------------------------
//  Puts newlines in given AString were appropriate to create a series
//              of lines of text that fit to the specified line pixel width.
// Arg          text_p - text add newlines to
// Arg          line_width - pixel width of lines to create
// Notes:       All control characters including tab, new-line, and carriage
//              return are normaly represented by a outlined square, so the width
//              of such strings will be inaccurate.
//              *** This method does not currently account for newlines already
//              existing in the AString - such will be an eventual enhancement.
//              This method does not currently deal with leading or trailing spaces.
//              Also there must be enough spaces to allow for breaks.
// Author(s):    Conan Reis
void ATrueTypeFont::word_wrap(
  AString * text_p,
  int       line_width
  ) const
  {
  uint32_t length = text_p->get_length();

  if (length)
    {
    SIZE    size;
    uint32_t    break_pos;
    int     hit_pos;
    uint32_t    pos         = 0;
    HDC     info_dc     = HDC(get_info_dc());
    HGDIOBJ old_font    = ::SelectObject(info_dc, m_font_handle_p);  // Save old font
    char *  cstr_p      = (char *)text_p->as_cstr();
    int     old_spacing = ::SetTextCharacterExtra(info_dc, m_spacing);

    do
      {
      // Find character hit position
      A_VERIFY_OS(::GetTextExtentExPoint(info_dc, &cstr_p[pos], length - pos, line_width, &hit_pos, nullptr, &size), "", ATrueTypeFont);
      pos += hit_pos;
      if (pos < length)
        {
        // $Revisit - CReis Makes the assumption that some whitespace will be found
        text_p->find_reverse(ACharMatch_white_space, 1, &break_pos, 0, pos);
        cstr_p[break_pos] = '\n';
        pos = break_pos;
        }
      pos++;
      }
    while (pos < length);

    // replace old settings
    ::SelectObject(info_dc, old_font);
    ::SetTextCharacterExtra(info_dc, old_spacing);

    release_info_dc(info_dc);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets a device context to use for informational purposes with
//              Win32 GDI functions.
// Returns:     A device context to use for informational purposes (cast to HDC)
// See:         release_info_dc()
// Notes:       The device context may be based on the current surface, or it may
//              be just an informational device context based on the current
//              graphics mode.
//              It should be released (release_info_dc()) when it is finished.
//
//              $Revisit - CReis This is to be used temporarily until the ASurfaceBase class
//              is created.
// Author(s):    Conan Reis
void * ATrueTypeFont::get_info_dc()
  {
  void * info_dc_p = ::CreateIC("DISPLAY", "", nullptr, nullptr);

  A_VERIFY_OS(info_dc_p != nullptr, "", ATrueTypeFont);
  return info_dc_p;
  }

//---------------------------------------------------------------------------------------
//  Frees info device context previously created with get_info_dc()
// Arg          info_dc_p - handle to an info device context
// See:         get_info_dc()
// Author(s):    Conan Reis
void ATrueTypeFont::release_info_dc(void * info_dc_p)
  {
  DeleteDC(HDC(info_dc_p));
  }

