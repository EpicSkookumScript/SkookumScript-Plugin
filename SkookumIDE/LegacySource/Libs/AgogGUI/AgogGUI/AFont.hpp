 //=======================================================================================
// Agog Labs C++ library.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
//
//  AFont class declaration header
// Author(s):    Conan Reis
// Create Date:   2000-07-11
// Notes:          
//=======================================================================================


#ifndef __AFONT_HPP
#define __AFONT_HPP
#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI/AFontSystemBase.hpp>
#include <AgogCore/AVec2i.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      This defines the device / driver / graphics library independent font class.
// Subclasses 
// See Also   ATrueTypeFont
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, gdi32.lib
// Examples:      
// Author(s)  Conan Reis
class AFont
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    AFont();
    explicit AFont(const AString & face_name, f32 point_size = AFont_point_def, uint32_t weight = AFontWeight_normal, bool italicised = false, bool underlined = false, int char_extra = 0, int leading_extra = 0, bool antialiased = true);
    AFont(const AString & face_name, int pixel_height, uint32_t weight = AFontWeight_normal, bool italicised = false, bool underlined = false, int char_extra = 0, int leading_extra = 0, bool antialiased = true);
    AFont(AFontSystemBase * dfont_p, int leading_extra = 0);
    AFont(const AFont & font);
    ~AFont();
    AFont &operator=(const AFont & font);

  // Accessor Methods

    eAFontType get_type() const  { return m_sys_font_p->get_type(); }

    AString  get_name() const;
    f32      get_point_size() const;
    int      get_height() const;
    int      get_avg_width() const;
    int      get_ascent() const;
    int      get_descent() const;
    int      get_leading() const;
    int      get_leading_extra() const;
    int      get_character_spacing() const;
    uint32_t get_weight() const;
    bool     is_italicised() const;
    bool     is_proportional() const;
    bool     is_underlined() const;
    bool     is_antialiased() const;

  // Modifying Methods

    void set_leading_extra(int extra_pixels);
    void set_char_spacing(int extra_pixels);

  // Non-Modifying Methods

    AFont  as_variant(f32 point_size = -1.0f, uint32_t weight = ADef_no_change, eAConfirm italicised = AConfirm_abort, eAConfirm underlined = AConfirm_abort) const;
    AVec2i get_area(const AString & text, uint32_t start_pos = 0, uint32_t char_count = ALength_remainder) const;
    int    get_width(const AString & text, uint32_t start_pos = 0, uint32_t char_count = ALength_remainder) const;
    bool   is_hit_char(const AString & text, int x_pos, uint32_t * char_pos_p = nullptr, uint32_t start_pos = 0) const;
    void   word_wrap(AString * text_p, int line_width) const;

  // Class Methods

    static uint32_t point_to_pixel(f32 point_size);
    static f32      pixel_to_point(uint32_t pixel_height);

  protected:
  // Data Members

    int m_leading_extra;

  public:  // For quick access

    AFontSystemBase * m_sys_font_p;

  // Class Data Members

    static AFont * ms_default_p;   // Default proportionally spaced font
    static AFont * ms_narrow_p;   // Default narrow/small proportionally spaced font
    static AFont * ms_header1_p;  // Default header font
    static AFont * ms_header2_p;  // Default sub-header font
    static AFont * ms_fixed_p;    // Default fixed space (monospace) font
  };  // AFont


//=======================================================================================
// Inline Functions
//=======================================================================================


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Author(s):    Conan Reis
inline AFont::AFont() :
  m_sys_font_p(ms_default_p->m_sys_font_p),
  m_leading_extra(ms_default_p->m_leading_extra)
  {
  m_sys_font_p->reference();
  }

  //---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Author(s):    Conan Reis
inline AFont::AFont(const AFont & font) :
  m_sys_font_p(font.m_sys_font_p),
  m_leading_extra(font.m_leading_extra)
  {
  m_sys_font_p->reference();
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
inline AFont::~AFont()
  {
  if (m_sys_font_p)
    {
    m_sys_font_p->dereference();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
inline AFont & AFont::operator=(const AFont & font)
  {
  m_leading_extra = font.m_leading_extra;
  font.m_sys_font_p->reference();
  m_sys_font_p->dereference();
  m_sys_font_p = font.m_sys_font_p;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Retrieves the face name of the font.
// Returns:     face name of the font.
// Author(s):    Conan Reis
inline AString AFont::get_name() const
  {
  return m_sys_font_p->m_name;
  }

//---------------------------------------------------------------------------------------
//  height based on pixels per inch or resolution
//              point size = (pixels * 72) / vertical pixels per inch
// Returns:     point size
// Author(s):    Conan Reis
inline f32 AFont::get_point_size() const
  {
  return m_sys_font_p->m_point_size;
  }

//---------------------------------------------------------------------------------------
//  height of font in pixels (ascent + descent and not including the
//              leading value) of characters
// Returns:     height of font in pixels
// Author(s):    Conan Reis
inline int AFont::get_height() const
  {
  return m_sys_font_p->m_height;
  }

//---------------------------------------------------------------------------------------
// Specifies the average width of characters in the font (generally defined
//             as the width of the letter 'x'). This value does not include the overhang
//             required for bold or italic characters
// Returns:    average width of characters in pixels
// Author(s):   Conan Reis
inline int AFont::get_avg_width() const
  {
  return m_sys_font_p->m_avg_width;
  }

//---------------------------------------------------------------------------------------
//  number of pixels that the font extends up from the font's
//              baseline for characters like:  bdfhijkltABCDEFGHIJKLMNOPQ...
// Returns:     pixels that the font extends up from the font's baseline
// Author(s):    Conan Reis
inline int AFont::get_ascent() const
  {
  return m_sys_font_p->m_ascent;
  }

//---------------------------------------------------------------------------------------
//  number of pixels below the font's base line for characters
//              like:  jpqyQ.
// Returns:     pixels below the font's base line
// Author(s):    Conan Reis
inline int AFont::get_descent() const
  {
  return m_sys_font_p->m_descent;
  }

//---------------------------------------------------------------------------------------
//  Specifies the number of vertical pixels of space to use between
//              rows.
// Notes:       This value is the recommended leading value modified by the
//              leading extra value.
// Author(s):    Conan Reis
inline int AFont::get_leading() const
  {
  return m_sys_font_p->m_leading + m_leading_extra;
  }

//---------------------------------------------------------------------------------------
//  Specifies the number of extra vertical pixels of space to use
//              between rows above and beyond the recommended size.  This value
//              can be negative.
// Returns:     extra vertical pixels of space to use between rows above and
//              beyond the recommended size
// Author(s):    Conan Reis
inline int AFont::get_leading_extra() const
  {
  return m_leading_extra;
  }

//---------------------------------------------------------------------------------------
//  Specifies the number of extra horizontal pixels of space to use
//              between characters above and beyond the recommended size.  This
//              value can be negative.
// Returns:     extra vertical pixels of space to use between rows above and
//              beyond the recommended size
// Author(s):    Conan Reis
inline int AFont::get_character_spacing() const
  {
  return m_sys_font_p->m_spacing;
  }

//---------------------------------------------------------------------------------------
//  Determines whether this font is bold or not
// Returns:     true if bold, false if not
// Author(s):    Conan Reis
inline uint32_t AFont::get_weight() const
  {
  return m_sys_font_p->m_weight;
  }

//---------------------------------------------------------------------------------------
//  Determines whether this font is italicised or not
// Returns:     true if italicised, false if not
// Author(s):    Conan Reis
inline bool AFont::is_italicised() const
  {
  return m_sys_font_p->m_italicised;
  }

//---------------------------------------------------------------------------------------
//  Determines whether this font is antialiased or not
// Returns:     true if antialiased, false if not
// Author(s):    Conan Reis
inline bool AFont::is_antialiased() const
  {
  return m_sys_font_p->m_antialiased;
  }

//---------------------------------------------------------------------------------------
//  Determines whether this font is underlined or not
// Returns:     true if underlined, false if not
// Author(s):    Conan Reis
inline bool AFont::is_underlined() const
  {
  return m_sys_font_p->m_underlined;
  }

//---------------------------------------------------------------------------------------
//  Determines whether this font uses a varying width for its
//              characters or not
// Returns:     true if proportional, false if fixed (monospace)
// Author(s):    Conan Reis
inline bool AFont::is_proportional() const
  {
  return m_sys_font_p->m_proportional;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Sets the number of extra vertical pixels of space to use
//              between rows above and beyond the recommended size.  This value
//              can be negative.
// Arg          extra_pixels - extra space size
// Author(s):    Conan Reis
inline void AFont::set_leading_extra(int extra_pixels)
  {
  m_leading_extra = extra_pixels;
  }

//---------------------------------------------------------------------------------------
//  Sets the number of extra vertical pixels of space to use
//              between rows above and beyond the recommended size.  This value
//              can be negative.
// Arg          extra_pixels - extra space size
// Author(s):    Conan Reis
inline void AFont::set_char_spacing(int extra_pixels)
  {
  m_sys_font_p->set_char_spacing(extra_pixels);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Creates a new font based on this font with some optional differences.
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline
  // New font based on this font
  AFont
AFont::as_variant(
  // If negative, keep same - otherwise change to specified value
  f32 point_size, // = -1.0f
  // If ADef_no_change, keep same - otherwise change to specified value
  uint32_t weight, // = ADef_no_change
  // If AConfirm_abort, keep same - otherwise change to specified value
  eAConfirm italicised, // = AConfirm_abort
  // If AConfirm_abort, keep same - otherwise change to specified value
  eAConfirm underlined // = AConfirm_abort
  ) const
  {
  return AFont(
    get_name(),
    (point_size > 0.0f) ? point_size : get_point_size(),
    (weight != ADef_no_change) ? weight : get_weight(),
    (italicised != AConfirm_abort) ? bool(italicised) : is_italicised(),
    (underlined != AConfirm_abort) ? bool(underlined) : is_underlined(),
    get_character_spacing(),
    get_leading_extra(),
    is_antialiased());
  }

//---------------------------------------------------------------------------------------
// Determines the width that the specified AString and range of text would be when
// drawing with this font.
//
// #Notes
//   All control characters including tab, new-line, and carriage return are normally
//   represented by a outlined square, so the width of such strings will be inaccurate.
//   This start_pos and char_count arguments are used for efficiency so that a substring
//   AString does not need to be created along with its memory allocation overhead etc.
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline
  // Area of given range of text in pixels
  AVec2i
AFont::get_area(
  // text to determine width of
  const AString & text,
  // character index position in text where the 0 x pixel position begins, i.e. the text
  // is clipped.
  uint32_t start_pos, // = 0
  // number of characters to get width of starting at start_pos.  If it is set to 
  // ALength_remainder, it is set to the length of the text minus the start_pos.
  uint32_t char_count  // = ALength_remainder
  ) const
  {
  return AVec2i(
    get_width(text, start_pos, char_count),
    get_height());
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
//              return are normally represented by a outlined square, so the width
//              of such strings will be inaccurate.
//              The start_pos argument is used for efficiency so that a substring
//              AString does not need to be created along with its memory
//              allocation overhead etc.
// Author(s):    Conan Reis
inline bool AFont::is_hit_char(
  const AString & text,
  int             x_pos,
  uint32_t *          char_pos_p, // = nullptr
  uint32_t        start_pos   // = 0
  ) const
  {
  return m_sys_font_p->is_hit_char(text, x_pos, char_pos_p, start_pos);
  }

//---------------------------------------------------------------------------------------
//  Puts newlines in given AString were appropriate to create a series
//              of lines of text that fit to the specified line pixel width.
// Arg          text_p - text add newlines to
// Arg          line_width - pixel width of lines to create
// Notes:       All control characters including tab, new-line, and carriage
//              return are normally represented by a outlined square, so the width
//              of such strings will be inaccurate.
//              *** This method does not currently account for newlines already
//              existing in the AString - such will be an eventual enhancement.
//              This method does not current deal with leading or trailing spaces.
//              Also there must be enough spaces to allow for breaks.
// Author(s):    Conan Reis
inline void AFont::word_wrap(
  AString * text_p,
  int       line_width
  ) const
  {
  m_sys_font_p->word_wrap(text_p, line_width);
  }


#endif // __AFONT_HPP


