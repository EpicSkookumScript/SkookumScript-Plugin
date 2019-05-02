// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AColor structure declaration header
//=======================================================================================


#ifndef __ACOLOR_HPP
#define __ACOLOR_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\AMath.hpp>
#include <AgogIO\AgogIO.hpp>


//=======================================================================================
// Defines
//=======================================================================================

#define ACOLOR_RATIO2INT32(_ratio)  a_float2int((_ratio) * 255.0f)
#define ACOLOR_RATIO2UINT8(_ratio)  uint8_t(a_float2int((_ratio) * 255.0f))


//=======================================================================================
// Global Structures
//=======================================================================================

const f32 AColor_1_over_255 = 1.0f / 255.0f;

// Forward declaration so windows/GDI+ header does not need to be included
typedef DWORD COLORREF;
typedef DWORD ARGB;


//---------------------------------------------------------------------------------------
// *Note:  American 'color' chosen for type name rather than Canadian/English 'colour'
//         since 'color' is so prevalent in computer literature and as a reserved word in
//         computer languages.
struct AColor
  {
  // Data Members

    f32 m_red;
    f32 m_green;
    f32 m_blue;
    f32 m_alpha;

  // Class Data Members

    // Common Colours - See AColor::get_element_os() for user preferred system colours.
    // For the most part these are correct 'Pantone' colours.

    static AColor ms_void;            // Black with -1 alpha - good as a 'non-colour' / no colour change
    static AColor ms_default;         // Black with -2 alpha - good as a default (for example use the system or user preferred colour)

    // Greys - or US 'gray'
    static AColor ms_white;
    static AColor ms_light_grey;
    static AColor ms_grey;
    static AColor ms_dark_grey;
    static AColor ms_black;

    // Reds
    static AColor ms_red;
    static AColor ms_crimson;
    static AColor ms_fire_brick;
    static AColor ms_dark_red;
    static AColor ms_maroon;

    // Oranges
    static AColor ms_orange;
    static AColor ms_gold;
    static AColor ms_med_orange;
    static AColor ms_dark_orange;

    // Yellows
    static AColor ms_yellow;
    static AColor ms_wheat;
    static AColor ms_khaki;
    static AColor ms_tan;
    static AColor ms_olive;

    // Greens
    static AColor ms_green;           // more correctly called 'lime'
    static AColor ms_chartreuse;
    static AColor ms_spring_green;
    static AColor ms_yellow_green;
    static AColor ms_lime;            // more correctly called 'green'
    static AColor ms_dark_green;
    static AColor ms_olive_drab;
    static AColor ms_forest_green;
    static AColor ms_sea_green;

    // Cyans
    static AColor ms_cyan;            // or 'aqua'
    static AColor ms_pale_turquoise;
    static AColor ms_turquoise;
    static AColor ms_dark_turquoise;
    static AColor ms_cadet_blue;
    static AColor ms_teal;

    // Blues
    static AColor ms_blue;
    static AColor ms_deep_sky_blue;
    static AColor ms_sky_blue;
    static AColor ms_light_blue;      // or 'baby blue'
    static AColor ms_royal_blue;
    static AColor ms_navy_blue;
    static AColor ms_steel_blue;
    static AColor ms_slate_grey;
    static AColor ms_dark_blue;
    static AColor ms_navy;
    static AColor ms_midnight_blue;

    // Purples
    static AColor ms_magenta;         // or 'fuchsia'
    static AColor ms_violet;
    static AColor ms_lavender;
    static AColor ms_plum;
    static AColor ms_orchid;
    static AColor ms_blue_violet;
    static AColor ms_purple;
    static AColor ms_indigo;

    // Pinks
    static AColor ms_pink;
    static AColor ms_peach_puff;
    static AColor ms_hot_pink;
    static AColor ms_deep_pink;
    static AColor ms_coral;
    static AColor ms_tomato;
    static AColor ms_salmon;
    static AColor ms_indian_red;

    // Browns
    static AColor ms_brown;           // Actual 'brown' is quite reddish - 'saddle_brown' is what most people consider a good brown
    static AColor ms_sienna;
    static AColor ms_saddle_brown; 
    static AColor ms_dark_brown;

    // Conan's Faves - from his own colour theme
    static AColor ms_blue_grey;       // Good 3D object foreground colour
    static AColor ms_blue_slate;      // Good window dark background colour
    static AColor ms_blue_cloud;      // Good window light background colour
    static AColor ms_deep_blue;       // Good 3D editor background

  // Nested Structures

    enum eFormat
      {
      Format_decimal,  // Decimal range from 0.00 to 1.00 - d.dd, d.dd, d.dd (Common format)
      Format_hex,      // Hexadecimal range from 00 to ff - hhhhhhhh (Web format)
      Format_byte      // Byte range from 0 to 255        - d, d, d
      };

    enum eScheme
      {
      Scheme_default,  // Default colour scheme based on system user preferences
      Scheme_custom,   // User specified custom settings from configuration file or some other source
      Scheme_light,    // Built-in scheme with light background (& usually dark text)
      Scheme_dark      // Built-in scheme with dark background (& usually light text)
      };


  // Common Methods

    AColor()                                         : m_red(0.0f), m_green(0.0f), m_blue(0.0f), m_alpha(1.0f) {}
    AColor(f32 red, f32 green, f32 blue)             : m_red(red), m_green(green), m_blue(blue), m_alpha(1.0f) {}
    AColor(f32 red, f32 green, f32 blue, f32 alpha)  : m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) {}
    AColor(const AColor & color)                     : m_red(color.m_red), m_green(color.m_green), m_blue(color.m_blue), m_alpha(color.m_alpha) {}

    AColor & operator=(const AColor & color)  { m_red = color.m_red; m_green = color.m_green; m_blue = color.m_blue; m_alpha = color.m_alpha; return *this; }

  // Conversion Methods

    AColor(COLORREF rgb, f32 alpha = 1.0f);
    operator COLORREF () const;
    AColor & operator=(COLORREF rgb);
    ARGB as_argb() const;

    AString as_string(bool enclose_with_type = true, bool include_alpha = true, eFormat format = Format_decimal) const;

  // Comparison Methods

    bool operator==(const AColor & color) const  { return (m_red == color.m_red) && (m_green == color.m_green) && (m_blue == color.m_blue) && (m_alpha == color.m_alpha); }
    bool operator!=(const AColor & color) const  { return (m_red != color.m_red) || (m_green != color.m_green) || (m_blue != color.m_blue) || (m_alpha != color.m_alpha); }
    bool operator>(const AColor & color) const   { return (m_red > color.m_red) && (m_green > color.m_green) && (m_blue > color.m_blue) && (m_alpha > color.m_alpha); }
    bool operator<(const AColor & color) const   { return (m_red < color.m_red) && (m_green < color.m_green) && (m_blue < color.m_blue) && (m_alpha < color.m_alpha); }

  // Accessors

    bool  is_dark() const                                    { return get_luminance() < 0.5f; }
    bool  is_default() const                                 { return m_alpha == -2.0f; }
    bool  is_greyscale() const                               { return a_is_approx_equal(m_red, m_green) && a_is_approx_equal(m_red, m_blue); }
    bool  is_opaque() const                                  { return m_alpha == 1.0f; }
    bool  is_translucent() const                             { return m_alpha < 1.0f; }
    bool  is_transparent() const                             { return m_alpha == 0.0f; }
    bool  is_valid() const                                   { return m_alpha >= 0.0f; }
    bool  is_invalid() const                                 { return m_alpha < 0.0f; }
    bool  is_void() const                                    { return m_alpha == -1.0f; }
    float get_luminance() const;
    void  set_rgb(f32 red, f32 green, f32 blue)              { m_red = red; m_green = green; m_blue = blue; }
    void  set_rgba(f32 red, f32 green, f32 blue, f32 alpha)  { m_red = red; m_green = green; m_blue = blue; m_alpha = alpha; }

  // Methods

    AColor   as_greyscale() const;
    AColor   as_invert_luminance() const;
    AColor   as_luminance(f32 new_y) const;
    AColor   as_negate() const;
    AColor & clamp();
    AColor & greyscale();
    AColor & invert_luminance();
    AColor & set_luminance(f32 new_y);
    AColor & negate();
    AColor & normalize();
    AColor & scale(f32 envelope);
    AColor & scale_alpha(f32 ratio);
    AColor & scale_brightness(f32 ratio);

    // These operators affect all component values (red, green, blue, alpha)

    AColor   operator*(f32 ratio) const;
    AColor   operator/(f32 ratio) const;
    AColor   operator+(const AColor & color) const;
    AColor   operator-(const AColor & color) const;
    AColor   operator*(const AColor & color) const;
    AColor   operator/(const AColor & color) const;
    AColor & operator*=(f32 ratio);
    AColor & operator/=(f32 ratio);
    AColor & operator+=(const AColor & color);
    AColor & operator-=(const AColor & color);
    AColor & operator*=(const AColor & color);
    AColor & operator/=(const AColor & color);


  // Class Methods

    static AColor get_element_os(int element_id, f32 alpha = 1.0f);


  protected:

  // Internal Methods

    void vary_luminance(f32 current_y, f32 target_y);

  };


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor from a COLORREF
// Returns:    itself
// Arg         rgb - COLORREF to convert
// Arg         alpha - alpha (translucency/opacity) value.  (Default 1.0f)
// Author(s):   Conan Reis
inline AColor::AColor(
  COLORREF rgb,
  f32      alpha // = 1.0f
  ) :
  //m_red(uint8_t(rgb) * AColor_1_over_255),
  //m_green(uint8_t(uint16_t(rgb) >> 8) * AColor_1_over_255),
  //m_blue(uint8_t(rgb >> 16) * AColor_1_over_255),
  m_alpha(alpha)
  {
  m_red   = (rgb & 0xff) * AColor_1_over_255;
  m_green = ((rgb & 0xff00) >> 8) * AColor_1_over_255;
  m_blue  = uint8_t(rgb >> 16) * AColor_1_over_255;
  }

//---------------------------------------------------------------------------------------
// COLORREF converter
// Returns:    COLORREF version of itself
// Author(s):   Conan Reis
inline AColor::operator COLORREF () const
  {
  return COLORREF(
    (ACOLOR_RATIO2UINT8(m_red)
    | (uint16_t(ACOLOR_RATIO2UINT8(m_green)) << 8 ))
    | (DWORD(ACOLOR_RATIO2UINT8(m_blue)) << 16));
  }

//---------------------------------------------------------------------------------------
// Assignment operator from a COLORREF
// Returns:    itself
// Arg         rgb - COLORREF to assign
// Author(s):   Conan Reis
inline AColor & AColor::operator=(COLORREF rgb)
  {
  m_red   = uint8_t(rgb) * AColor_1_over_255;
  m_green = uint8_t(uint16_t(rgb) >> 8) * AColor_1_over_255;
  m_blue  = uint8_t(rgb >> 16) * AColor_1_over_255;

  return *this;
  }

//---------------------------------------------------------------------------------------
// ARGB converter
// Returns:    ARGB version of itself
// Author(s):   Conan Reis
inline ARGB AColor::as_argb() const
  {
  return ARGB(
    (ACOLOR_RATIO2INT32(m_alpha)   << 24)
    | (ACOLOR_RATIO2INT32(m_red)   << 16)
    | (ACOLOR_RATIO2INT32(m_green) <<  8)
    | ACOLOR_RATIO2INT32(m_blue));
  }

//---------------------------------------------------------------------------------------
// Greyscales the colour.
// Returns:    greyscaled copy of itself
// See:        greyscale(), as_negate(), negate()
// Author(s):   Conan Reis
inline AColor AColor::as_greyscale() const
  {
  f32 y = get_luminance();
  
  return AColor(y, y, y, m_alpha);
  }

//---------------------------------------------------------------------------------------
// Changes dark colours to light colours and light colours to dark colours.
// Returns:    itself with its luminance (Y) inverted
// Notes:      Luminance (Y) is essentially a quantifiable measure of brightness which is
//             otherwise a subjective measure.
// See:        as_luminance(), get_luminance(), invert_luminance(), is_dark(),
//             scale_brightness(), set_luminance()
// Author(s):   Conan Reis
inline AColor AColor::as_invert_luminance() const
  {
  AColor color(*this);
  f32    current_y = get_luminance();

  color.vary_luminance(current_y, 1.0f - current_y);

  return color;
  }

//---------------------------------------------------------------------------------------
// Sets the colour to a new luminance, but keeps the same hue.
// Returns:    itself with a new luminance
// Arg         new_y - new luminance value for the colour from 0.0f - 1.0f
// Notes:      Luminance (Y) is essentially a quantifiable measure of brightness which is
//             otherwise a subjective measure.
// See:        as_invert_luminance(), get_luminance(), invert_luminance(), is_dark(),
//             scale_brightness(), set_luminance()
// Author(s):   Conan Reis
inline AColor AColor::as_luminance(f32 new_y) const
  {
  AColor color(*this);

  color.vary_luminance(get_luminance(), new_y);

  return color;
  }

//---------------------------------------------------------------------------------------
// Negates the colour.
// Returns:    negated copy of itself
// See:        negate(), as_greyscale(), greyscale()
// Author(s):   Conan Reis
inline AColor AColor::as_negate() const
  {
  return AColor(1.0f - m_red, 1.0f - m_green, 1.0f - m_blue, m_alpha);
  }

//---------------------------------------------------------------------------------------
// Clamps the colour component values - ensuring that they are greater than
//             or equal to 0 and less than or equal to 1.
// Returns:    itself clamped
// See:        normalize()
// Author(s):   Conan Reis
inline AColor & AColor::clamp()
  {
  m_red   = a_clamp(m_red, 0.0f, 1.0f);
  m_green = a_clamp(m_green, 0.0f, 1.0f);
  m_blue  = a_clamp(m_blue, 0.0f, 1.0f);
  m_alpha = a_clamp(m_alpha, 0.0f, 1.0f);
  
  return *this;
  }

//---------------------------------------------------------------------------------------
// Gets the luminance (Y) of the colour.  Luminance is essentially a
//             quantifiable measure of brightness which is otherwise a subjective measure.
// Returns:    Luminance (Y) 
// Notes:      This computation assumes that the luminance spectral weighting can be
//             formed as a linear combination of the scanner curves, and assumes that
//             the component signals represent linear-light.  Either or both of these
//             conditions can be relaxed to some extent depending on the application.
//             Some computer systems have computed brightness using (R+G+B)/3 - however
//             this is at odds with the properties of human vision.
//
//             The specific coefficients used for the RGB values in this function are
//             derived from "ITU-R Recommendation BT.709, Basic Parameter Values for the
//             HDTV Standard for the Studio and for International Programme Exchange
//             1990" (or Rec. 709 for short) from the Commission Internationale de
//             L'Eclairage (CIE) which adopted standard curves for a hypothetical Standard
//             Human Observer.
// See:        as_invert_luminance(), as_luminance(), invert_luminance(), is_dark(),
//             scale_brightness(), set_luminance()
// Author(s):   Conan Reis
inline float AColor::get_luminance() const
  {
  return (m_red * 0.2126f) + (m_green * 0.7152f) + (m_blue * 0.0722f);
  }

//---------------------------------------------------------------------------------------
// Changes the colour to greyscale (based on luminance).
// Returns:    itself greyscaled
// See:        as_greyscale(), negate(), as_negate()
// Author(s):   Conan Reis
inline AColor & AColor::greyscale()
  {
  m_red = m_green = m_blue = get_luminance();

  return *this;
  }

//---------------------------------------------------------------------------------------
// Changes dark colours to light colours and light colours to dark colours.
// Returns:    itself with its luminance (Y) inverted
// Notes:      Luminance (Y) is essentially a quantifiable measure of brightness which is
//             otherwise a subjective measure.
// See:        as_invert_luminance(), as_luminance(), get_luminance(), is_dark(),
//             scale_brightness(), set_luminance()
// Author(s):   Conan Reis
inline AColor & AColor::invert_luminance()
  {
  f32 current_y = get_luminance();

  vary_luminance(current_y, 1.0f - current_y);

  return *this;
  }

//---------------------------------------------------------------------------------------
// Negates the colour - alpha stays the same.
// Returns:    itself negated
// See:        as_negate(), greyscale(), as_greyscale()
// Author(s):   Conan Reis
inline AColor & AColor::negate()
  {
  m_red   = 1.0f - m_red;
  m_green = 1.0f - m_green;
  m_blue  = 1.0f - m_blue;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Increases or decreases the alpha (translucency/opacity) of the colour.
// Arg         ratio - ratio (with value >= 0) to increase or decrease alpha by.
// Notes:      Alpha will change proportionately and stay between 0 and 1.
// See:        scale_brightness()
// Author(s):   Conan Reis
inline AColor & AColor::scale_alpha(float ratio)
  {
  m_alpha = a_min(m_alpha * ratio, 1.0f);

  return *this;
  }

//---------------------------------------------------------------------------------------
// Increases or decreases the brightness of the colour.
// Arg         ratio - ratio (with value >= 0) to increase or decrease brightness by.
//             This method is the same as operator*=.
// Notes:      Component values will change proportionately and stay between 0 and 1.
// See:        operator methods, as_invert_luminance(), as_luminance(), get_luminance(),
//             invert_luminance(), is_dark(), scale_alpha(), set_luminance()
// Author(s):   Conan Reis
inline AColor & AColor::scale_brightness(float ratio)
  {
  m_red   = a_min(m_red * ratio, 1.0f);
  m_green = a_min(m_green * ratio, 1.0f);
  m_blue  = a_min(m_red * ratio, 1.0f);

  return *this;
  }

//---------------------------------------------------------------------------------------
// Sets the colour to a new luminance, but keeps the same hue.
// Returns:    itself with a new luminance
// Notes:      Luminance (Y) is essentially a quantifiable measure of brightness which is
//             otherwise a subjective measure.
// See:        as_invert_luminance(), as_luminance(), get_luminance(), invert_luminance(),
//             is_dark(), scale_brightness()
// Author(s):   Conan Reis
inline AColor & AColor::set_luminance(f32 new_y)
  {
  vary_luminance(get_luminance(), new_y);

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator*(f32 ratio) const
  {
  return AColor(m_red * ratio, m_green * ratio, m_blue * ratio, m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator/(f32 ratio) const
  {
  f32 ratio_recip = a_reciprocal(ratio);

  return AColor(m_red * ratio_recip, m_green * ratio_recip, m_blue * ratio_recip, m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator+(const AColor & color) const
  {
  return AColor(m_red + color.m_red, m_green + color.m_green, m_blue + color.m_blue, m_alpha + color.m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator-(const AColor & color) const
  {
  return AColor(m_red - color.m_red, m_green - color.m_green, m_blue - color.m_blue, m_alpha - color.m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator*(const AColor & color) const
  {
  return AColor(m_red * color.m_red, m_green * color.m_green, m_blue * color.m_blue, m_alpha * color.m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor AColor::operator/(const AColor & color) const
  {
  return AColor(m_red / color.m_red, m_green / color.m_green, m_blue / color.m_blue, m_alpha / color.m_alpha);
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator*=(f32 ratio)
  {
  m_red   *= ratio;
  m_green *= ratio;
  m_blue  *= ratio;

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator/=(f32 ratio)
  {
  f32 ratio_recip = a_reciprocal(ratio);

  m_red   *= ratio_recip;
  m_green *= ratio_recip;
  m_blue  *= ratio_recip;

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator+=(const AColor & color)
  {
  m_red   += color.m_red;
  m_green += color.m_green;
  m_blue  += color.m_blue;
  m_alpha += color.m_alpha;

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator-=(const AColor & color)
  {
  m_red   -= color.m_red;
  m_green -= color.m_green;
  m_blue  -= color.m_blue;
  m_alpha -= color.m_alpha;

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator*=(const AColor & color)
  {
  m_red   *= color.m_red;
  m_green *= color.m_green;
  m_blue  *= color.m_blue;
  m_alpha *= color.m_alpha;

  return *this;
  }

//---------------------------------------------------------------------------------------
// See:        clamp(), normalize()
// Author(s):   Conan Reis
inline AColor & AColor::operator/=(const AColor & color)
  {
  m_red   /= color.m_red;
  m_green /= color.m_green;
  m_blue  /= color.m_blue;
  m_alpha /= color.m_alpha;

  return *this;
  }


#endif // __ACOLOR_HPP


