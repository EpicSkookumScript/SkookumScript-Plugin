// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFont class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AFont.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogGUI\AFontSystemBase.hpp>
#include <AgogCore\AMath.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Class Data Members
//=======================================================================================

// Defined in AgogGUI\AgogGUI.cpp


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Initialize static variables
void AFont::initialize()
  {
  ms_default_p = new AFont(AFONT_FACE_DEF);
  ms_narrow_p  = new AFont("Trebuchet MS", AFont_point_def - 1.0f);
  ms_header1_p = new AFont("Trebuchet MS", AFont_point_def + 2.0f, AFontWeight_bold);
  ms_header2_p = new AFont("Trebuchet MS", AFont_point_def, AFontWeight_bold);
  ms_fixed_p   = new AFont(AFONT_FACE_FIXED_DEF);
  }

//---------------------------------------------------------------------------------------
// Deinitialize static variables
void AFont::deinitialize()
  {
  delete ms_default_p;
  delete ms_narrow_p;
  delete ms_header1_p;
  delete ms_header2_p;
  delete ms_fixed_p;
  }

//---------------------------------------------------------------------------------------
// Constructor.  Creates a device / driver / graphics
//             library independent font.
// Returns:    itself
// Arg         name - identifying name of the font
// Arg         point_size - device independent point size of font
// Arg         weight - levels of "boldness" including being lighter than default.
// Arg         italicised - boolean indicating whether font is italicised or not
// Arg         char_extr - number of extra horizontal pixels of space to use
//             between characters above and beyond the recommended size.  This
//             value can be negative.
// Arg         leading_extra - number of extra vertical pixels of space to use
//             between rows above and beyond the recommended size.  This value
//             can be negative.
// Author(s):   Conan Reis
AFont::AFont(
  const AString & face_name,     // = AFONT_FACE_DEF
  f32             point_size,    // = AFont_point_def
  uint32_t        weight,        // = AFontWeight_normal
  bool            italicised,    // = false
  bool            underlined,    // = false
  int             char_extra,    // = 0
  int             leading_extra, // = 0
  bool            antialiased    // = true
  ) :
  m_leading_extra(leading_extra),
  m_sys_font_p(new ATrueTypeFont(face_name, point_to_pixel(point_size), weight, italicised, underlined, char_extra, antialiased))
  {
  m_sys_font_p->reference();
  }

//---------------------------------------------------------------------------------------
// Constructor.  Creates a device / driver / graphics library
//             independent font.
// Returns:    itself
// Arg         name - identifying name of the font
// Arg         pixel_height - pixel height of the highest character in font
// Arg         bolded - boolean indicating whether font is bolded or not.  This
//             metric could be replaced with "weight" which would give levels of
//             "boldness" including being lighter than default.
// Arg         italicised - boolean indicating whether font is italicised or not
// Arg         char_extra - number of extra horizontal pixels of space to use
//             between characters above and beyond the recommended size.  This
//             value can be negative.
// Arg         leading_extra - number of extra vertical pixels of space to use
//             between rows above and beyond the recommended size.  This value
//             can be negative (Default 0).
// Author(s):   Conan Reis
AFont::AFont(
  const AString & face_name,
  int             pixel_height,
  uint32_t        weight,        // = AFontWeight_normal
  bool            italicised,    // = false
  bool            underlined,    // = false
  int             char_extra,    // = 0
  int             leading_extra, // = 0
  bool            antialiased    // = false
  ) :
  m_leading_extra(leading_extra),
  m_sys_font_p(new ATrueTypeFont(face_name, pixel_height, weight, italicised, underlined, char_extra, antialiased))
  {
  m_sys_font_p->reference();
  }

//---------------------------------------------------------------------------------------
// Constructor.  Creates a device / driver / graphics library
//             independent font.
// Returns:    itself
// Arg         dfont_p - pointer to a dynamically allocated device dependant
//             font.  It will automatically be deleted when no AFont is using
//             it anymore.  It should not be nullptr.
// Arg         leading_extra - number of extra vertical pixels of space to use
//             between rows above and beyond the recommended size.  This value
//             can be negative (Default 0).
// Author(s):   Conan Reis
AFont::AFont(
  AFontSystemBase * dfont_p,
  int               leading_extra // = 0
  ) :
  m_leading_extra(leading_extra),
  m_sys_font_p(dfont_p)
  {
  dfont_p->reference();
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
  // width of given range of text in pixels
  int
AFont::get_width(
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
  return m_sys_font_p->get_width(text, start_pos, char_count);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Converts from a point size to a pixel size, thus allowing fonts
//              to be the same size to the user irrespective of screen resoloution
//              or size of monitor.
// Returns:     size in pixels
// Arg          point_size - device independent point size of font
// Modifiers:    static
// Examples:    int pixel_size = AFont::point_to_pixel(14.0f); // Get 14 pointsize
// Author(s):    Conan Reis
uint32_t AFont::point_to_pixel(f32 point_size)
  {
  HDC  info_dc          = (HDC)ATrueTypeFont::get_info_dc();
  f32  vpixels_per_inch = f32(::GetDeviceCaps(info_dc, LOGPIXELSY));

  ATrueTypeFont::release_info_dc(info_dc);

  // Actual height of font
  // pixels = point size * (vertical pixels per inch / 72)
  return a_round(point_size * vpixels_per_inch / 72.0f);
  }

//---------------------------------------------------------------------------------------
//  Converts from a pixel size to a point size.
// Returns:     device independent point size of font
// Arg          point_size - size in pixels
// Modifiers:    static
// Examples:    f32 point_size = AFont::pixel_to_point(25);
// Author(s):    Conan Reis
f32 AFont::pixel_to_point(uint32_t pixel_size)
  {
  f32 point_size;
  HDC info_dc = (HDC)ATrueTypeFont::get_info_dc();

  // Logical height of font
  // point size = (pixel size * 72) / vertical pixels per inch
  point_size = (f32(pixel_size) * 72.0f) / f32(::GetDeviceCaps(info_dc, LOGPIXELSY));
  ATrueTypeFont::release_info_dc(info_dc);

  return point_size;
  }

