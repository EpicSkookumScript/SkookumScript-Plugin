// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AColor structure definition file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AColor.hpp>
#include <AgogCore\AString.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Class Data
//=======================================================================================

// For the most part these are correct 'Pantone' colours.

AColor AColor::ms_void(          0.0f,  0.0f,  0.0f,  -1.0f);  // Black with -1 alpha - good as a 'non-colour' / no colour change
AColor AColor::ms_default(       0.0f,  0.0f,  0.0f,  -2.0f);  // Black with -2 alpha - good as a default (for example use the system or user preferred colour)

// Greys - or US 'gray'
AColor AColor::ms_white(         1.0f,  1.0f,  1.0f,  1.0f);
AColor AColor::ms_light_grey(    0.75f, 0.75f, 0.75f, 1.0f);
AColor AColor::ms_grey(          0.5f,  0.5f,  0.5f,  1.0f);
AColor AColor::ms_dark_grey(     0.25f, 0.25f, 0.25f, 1.0f);
AColor AColor::ms_black(         0.0f,  0.0f,  0.0f,  1.0f);

// Reds
AColor AColor::ms_red(           1.0f,  0.0f,  0.0f,  1.0f);
AColor AColor::ms_crimson(       0.86f, 0.08f, 0.24f, 1.0f);
AColor AColor::ms_fire_brick(    0.7f,  0.13f, 0.13f, 1.0f);
AColor AColor::ms_dark_red(      0.55f, 0.0f,  0.0f,  1.0f);
AColor AColor::ms_maroon(        0.5f,  0.0f,  0.0f,  1.0f);

// Oranges
AColor AColor::ms_orange(        1.0f,  0.65f, 0.0f,  1.0f);
AColor AColor::ms_gold(          1.0f,  0.84f, 0.0f,  1.0f);
AColor AColor::ms_med_orange(    1.0f,  0.5f,  0.0f,  1.0f);
AColor AColor::ms_dark_orange(   0.5f,  0.25f, 0.0f,  1.0f);

// Yellows
AColor AColor::ms_yellow(        1.0f,  1.0f,  0.0f,  1.0f);
AColor AColor::ms_wheat(         0.96f, 0.87f, 0.7f,  1.0f);
AColor AColor::ms_khaki(         0.94f, 0.9f,  0.55f, 1.0f);
AColor AColor::ms_tan(           0.82f, 0.71f, 0.55f, 1.0f);
AColor AColor::ms_olive(         0.5f,  0.5f,  0.0f,  1.0f);

// Greens
AColor AColor::ms_green(         0.0f,  1.0f,  0.0f,  1.0f);  // more correctly called 'lime'
AColor AColor::ms_chartreuse(    0.5f,  1.0f,  0.0f,  1.0f);
AColor AColor::ms_spring_green(  0.0f,  1.0f,  0.5f,  1.0f);
AColor AColor::ms_yellow_green(  0.55f, 0.8f,  0.2f,  1.0f);
AColor AColor::ms_lime(          0.0f,  0.55f, 0.0f,  1.0f);  // more correctly called 'green'
AColor AColor::ms_dark_green(    0.0f,  0.39f, 0.0f,  1.0f);
AColor AColor::ms_olive_drab(    0.42f, 0.56f, 0.14f, 1.0f);
AColor AColor::ms_forest_green(  0.13f, 0.55f, 0.13f, 1.0f);
AColor AColor::ms_sea_green(     0.18f, 0.55f, 0.34f, 1.0f);

// Cyans
AColor AColor::ms_cyan(          0.0f,  1.0f,  1.0f,  1.0f);  // or 'aqua'
AColor AColor::ms_pale_turquoise(0.69f, 0.93f, 0.93f, 1.0f);
AColor AColor::ms_turquoise(     0.25f, 0.88f, 0.82f, 1.0f);
AColor AColor::ms_dark_turquoise(0.0f,  0.81f, 0.82f, 1.0f);
AColor AColor::ms_cadet_blue(    0.37f, 0.62f, 0.63f, 1.0f);
AColor AColor::ms_teal(          0.0f,  0.5f,  0.5f,  1.0f);

// Blue
AColor AColor::ms_blue(          0.0f,  0.0f,  1.0f,  1.0f);
AColor AColor::ms_deep_sky_blue( 0.0f,  0.75f, 1.0f,  1.0f);
AColor AColor::ms_sky_blue(      0.53f, 0.81f, 0.92f, 1.0f);
AColor AColor::ms_light_blue(    0.68f, 0.85f, 0.9f,  1.0f);  // or 'baby blue'
AColor AColor::ms_royal_blue(    0.25f, 0.41f, 0.88f, 1.0f);
AColor AColor::ms_navy_blue(     0.62f, 0.69f, 0.87f, 1.0f);
AColor AColor::ms_steel_blue(    0.27f, 0.51f, 0.71f, 1.0f);
AColor AColor::ms_slate_grey(    0.44f, 0.5f,  0.56f, 1.0f);
AColor AColor::ms_dark_blue(     0.0f,  0.0f,  0.55f, 1.0f);
AColor AColor::ms_navy(          0.0f,  0.0f,  0.5f,  1.0f);
AColor AColor::ms_midnight_blue( 0.1f,  0.1f,  0.44f, 1.0f);

// Purples
AColor AColor::ms_magenta(       1.0f,  0.0f,  1.0f,  1.0f);  // or 'fuchsia'
AColor AColor::ms_violet(        0.93f, 0.51f, 0.93f, 1.0f);
AColor AColor::ms_lavender(      0.9f,  0.9f,  0.98f, 1.0f);
AColor AColor::ms_plum(          0.87f, 0.63f, 0.87f, 1.0f);
AColor AColor::ms_orchid(        0.85f, 0.44f, 0.84f, 1.0f);
AColor AColor::ms_blue_violet(   0.54f, 0.17f, 0.89f, 1.0f);
AColor AColor::ms_purple(        0.5f,  0.0f,  0.5f,  1.0f);
AColor AColor::ms_indigo(        0.29f, 0.0f,  0.51f, 1.0f);

// Pinks
AColor AColor::ms_pink(          1.0f,  0.75f, 0.8f,  1.0f);
AColor AColor::ms_peach_puff(    1.0f,  0.85f, 0.73f, 1.0f);
AColor AColor::ms_hot_pink(      1.0f,  0.41f, 0.71f, 1.0f);
AColor AColor::ms_deep_pink(     1.0f,  0.08f, 0.58f, 1.0f);
AColor AColor::ms_coral(         1.0f,  0.5f,  0.31f, 1.0f);
AColor AColor::ms_tomato(        1.0f,  0.39f, 0.28f, 1.0f);
AColor AColor::ms_salmon(        0.98f, 0.5f,  0.45f, 1.0f);
AColor AColor::ms_indian_red(    0.8f,  0.36f, 0.36f, 1.0f);

// Browns
AColor AColor::ms_brown(         0.65f, 0.17f, 0.17f, 1.0f);  // Actual 'brown' is quite reddish - 'saddle_brown' is what most people consider a good brown
AColor AColor::ms_sienna(        0.63f, 0.32f, 0.18f, 1.0f);
AColor AColor::ms_saddle_brown(  0.55f, 0.27f, 0.07f, 1.0f); 
AColor AColor::ms_dark_brown(    0.31f, 0.20f, 0.05f, 1.0f);

// Conan's Faves - from his own colour theme
AColor AColor::ms_blue_grey(     0.39f, 0.39f, 0.47f, 1.0f);  // Good 3D object foreground colour
AColor AColor::ms_blue_slate(    0.19f, 0.22f, 0.28f, 1.0f);  // Good window dark background colour
AColor AColor::ms_blue_cloud(    0.63f, 0.67f, 0.75f, 1.0f);  // Good window light background colour
AColor AColor::ms_deep_blue(     0.09f, 0.11f, 0.15f, 1.0f);  // Good 3D editor background


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Converts itself to a string.
// Returns:    String version of itself
// Arg         enclose_with_type - enclose colour components with type name or not.
//             As in: Color(xxx) or just xxx.
// Arg         include_alpha - include alpha component of colour with string or not.
// Arg         format - Format_decimal (default), Format_hex (web), or Format_byte
//             - see AColor::eFormat.
// Author(s):   Conan Reis
AString AColor::as_string(
  bool    enclose_with_type, // = true
  bool    include_alpha,     // = true
  eFormat format             // = Format_decimal
  ) const
  {
  AString str(nullptr, 32u, 0u);

  if (enclose_with_type)
    {
    str.append("Color(", 6u);
    }

  switch (format)
    {
    case Format_decimal:
      if (include_alpha)
        {
        str.append_format("%.2f, %.2f, %.2f, %.2f", m_red, m_green, m_blue, m_alpha);
        }
      else
        {
        str.append_format("%.2f, %.2f, %.2f", m_red, m_green, m_blue);
        }
      break;

    case Format_hex:
      if (include_alpha)
        {
        str.append_format("%02x, %02x, %02x, %02x", ACOLOR_RATIO2INT32(m_red), ACOLOR_RATIO2INT32(m_green), ACOLOR_RATIO2INT32(m_blue), ACOLOR_RATIO2INT32(m_alpha));
        }
      else
        {
        str.append_format("%02x, %02x, %02x", ACOLOR_RATIO2INT32(m_red), ACOLOR_RATIO2INT32(m_green), ACOLOR_RATIO2INT32(m_blue));
        }
      break;

    case Format_byte:
      if (include_alpha)
        {
        str.append_format("%i, %i, %i, %i", ACOLOR_RATIO2INT32(m_red), ACOLOR_RATIO2INT32(m_green), ACOLOR_RATIO2INT32(m_blue), ACOLOR_RATIO2INT32(m_alpha));
        }
      else
        {
        str.append_format("%i, %i, %i", ACOLOR_RATIO2INT32(m_red), ACOLOR_RATIO2INT32(m_green), ACOLOR_RATIO2INT32(m_blue));
        }
      break;
    }

  if (enclose_with_type)
    {
    str.append(')');
    }

  return str;
  }

//---------------------------------------------------------------------------------------
// Normalizes the colour - i.e. adjusts the compontent values
//             proportionately until they are all between 0 and 1.
// Returns:    itself normalized
// See:        clamp()
// Notes:      The alpha is clamped between 0 and 1.
// Author(s):   Conan Reis
AColor & AColor::normalize()
  {
  // Ensure the RGB components are greater than 0
  f32 value = a_min(a_min(m_red, m_green), m_blue);

  if (value < 0.0f)
    {
    m_red   -= value;
    m_green -= value;
    m_blue  -= value;
    }

  // Shrink RGB components if one or more is greater than 1
  value = a_max(a_max(m_red, m_green), m_blue);

  if (value > 1.0f)
    {
    value = a_reciprocal(value);

    m_red   *= value;
    m_green *= value;
    m_blue  *= value;
    }

  // Clamp alpha
  m_alpha = a_clamp(m_alpha, 0.0f, 1.0f);

  return *this;
  }

//---------------------------------------------------------------------------------------
// Scales the colour - i.e. adjusts the compontent values proportionately
//             until they are all between 0 and envelope and ensuring that the largest
//             will be equal to envelope.
// Arg         envelope - positive value to scale largest component value to.
// Returns:    itself scaled to the specified envelope
// See:        clamp()
// Notes:      The alpha is clamped between 0 and 1.
// Author(s):   Conan Reis
AColor & AColor::scale(f32 envelope)
  {
  // Ensure the RGB components are greater than 0
  f32 value = a_min(a_min(m_red, m_green), m_blue);

  if (value < 0.0f)
    {
    m_red   -= value;
    m_green -= value;
    m_blue  -= value;
    }

  // Scale RGB components with largest = envelope
  value = a_reciprocal(a_max(a_max(m_red, m_green), m_blue)) * envelope;

  m_red   *= value;
  m_green *= value;
  m_blue  *= value;

  // Clamp alpha
  m_alpha = a_clamp(m_alpha, 0.0f, 1.0f);

  return *this;
  }


//=======================================================================================
// Class Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Get system element color as set by user preferences.
// Returns:    Colour of system element.
// Arg         element_id - one of:
//
//               COLOR_3DDKSHADOW          - Dark shadow for three-dimensional display elements. 
//               COLOR_3DFACE              - (or COLOR_BTNFACE) Face color for three-dimensional display elements and for dialog box backgrounds. 
//               COLOR_3DHILIGHT           - (or COLOR_3DHIGHLIGHT, COLOR_BTNHILIGHT, COLOR_BTNHIGHLIGHT) Highlight color for three-dimensional display elements (for edges facing the light source.) 
//               COLOR_3DLIGHT             - Light color for three-dimensional display elements (for edges facing the light source.) 
//               COLOR_3DSHADOW            - (or COLOR_BTNSHADOW) Shadow color for three-dimensional display elements (for edges facing away from the light source). 
//               COLOR_ACTIVEBORDER        - Active window border. 
//               COLOR_APPWORKSPACE        - Background color of multiple document interface (MDI) applications. 
//               COLOR_BACKGROUND          - (or COLOR_DESKTOP) Desktop. 
//               COLOR_BTNTEXT             - Text on push buttons. 
//               COLOR_CAPTIONTEXT         - Text in caption, size box, and scroll bar arrow box. 
//               COLOR_GRAYTEXT            - Greyed (disabled) text. This color is set to 0 if the current display driver does not support a solid grey color. 
//               COLOR_HIGHLIGHT           - Item(s) selected in a control. 
//               COLOR_HIGHLIGHTTEXT       - Text of item(s) selected in a control. 
//               COLOR_INACTIVEBORDER      - Inactive window border. 
//               COLOR_INACTIVECAPTIONTEXT - Color of text in an inactive caption. 
//               COLOR_INFOBK              - Background color for tooltip controls. 
//               COLOR_INFOTEXT            - Text color for tooltip controls. 
//               COLOR_MENU                - Menu background. 
//               COLOR_MENUTEXT            - Text in menus. 
//               COLOR_SCROLLBAR           - Scroll bar grey area. 
//               COLOR_WINDOW              - Window background. 
//               COLOR_WINDOWFRAME         - Window frame. 
//               COLOR_WINDOWTEXT          - Text in windows. 
//
//               ---- Win98 & Above ----
//               COLOR_ACTIVECAPTION           - Active window title bar.  Specifies the left side color in the color gradient of an active window's title bar if the gradient effect is enabled.  [Windows NT, Windows 95:  This remark does not apply.]
//               COLOR_GRADIENTACTIVECAPTION   - Right side color in the color gradient of an active window's title bar. COLOR_ACTIVECAPTION specifies the left side color. Use SPI_GETGRADIENTCAPTIONS with the SystemParametersInfo function to determine whether the gradient effect is enabled.  [Windows NT, Windows 95:  This value is not supported.]
//               COLOR_GRADIENTINACTIVECAPTION - Right side color in the color gradient of an inactive window's title bar. COLOR_INACTIVECAPTION specifies the left side color.  [Windows NT, Windows 95:  This value is not supported.]
//               COLOR_HOTLIGHT                - Color for a hot-tracked item. Single clicking a hot-tracked item executes the item.  [Windows NT, Windows 95:  This value is not supported.]
//               COLOR_INACTIVECAPTION         - Inactive window caption.  Specifies the left side color in the color gradient of an inactive window's title bar if the gradient effect is enabled.  [Windows NT, Windows 95:  This remark does not apply.]
//
//               ---- Win2000/XP & Above ----
//               COLOR_MENUHILIGHT - The color used to highlight menu items when the menu appears as a flat menu (see SystemParametersInfo). The highlighted menu item is outlined with COLOR_HIGHLIGHT.  [Windows 2000/NT, Windows Me/98/95:  This value is not supported.]
//               COLOR_MENUBAR     - The background color for the menu bar when menus appear as flat menus (see SystemParametersInfo). However, COLOR_MENU continues to specify the background color of the menu popup.  [Windows 2000/NT, Windows Me/98/95:  This value is not supported.]
//
// Arg         alpha - alpha (translucency/opacity) value.  (Default 1.0f)
// Notes:      The windows header file may need to be included to get the element id
//             value defines.  If useful, the element ids could be made into an
//             enumerated type and placed in the header file.
//             This method is not inlined so that the windows header does not need to be
//             included in AgogGUI\AColor.hpp
// Modifiers:   static
// Author(s):   Conan Reis
AColor AColor::get_element_os(
  int element_id,
  f32 alpha // = 1.0f
  )
  {
  return AColor(::GetSysColor(element_id), alpha);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void AColor::vary_luminance(f32 current_y, f32 target_y)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Note - CReis This catches values less than 0 which are technically illegal.
  if (target_y <= 0.0f)
    {
    m_red   = 0.0f;
    m_green = 0.0f;
    m_blue  = 0.0f;

    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Note - CReis This catches values greater than 1 which are technically illegal.
  if (target_y >= 1.0f)
    {
    m_red   = 1.0f;
    m_green = 1.0f;
    m_blue  = 1.0f;

    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for scale of black/grey/white 
  if (is_greyscale())
    {
    m_red   = target_y;
    m_green = target_y;
    m_blue  = target_y;

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calc new luminance values for non-greyscale colour that has a non-black/white target
  f32 ratio = target_y / current_y;

  if (ratio <= 1.0f)
    {
    m_red   *= ratio;
    m_green *= ratio;
    m_blue  *= ratio;

    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for overflow and raise up other colour channels if overflow occurs
  // $Revisit - CReis This algorithm is not very optimal
  bool overflow_b = false;

  while (!a_is_approx_equal(current_y, target_y))
    {
    ratio = target_y / current_y;

    m_red   *= ratio;
    m_green *= ratio;
    m_blue  *= ratio;

    // Check for overflow and raise up other colour channels if overflow occurs
    overflow_b = false;

    if (m_red > 1.0f)
      {
      m_red      = 1.0f;
      overflow_b = true;
      }

    if (m_green > 1.0f)
      {
      m_green    = 1.0f;
      overflow_b = true;
      }

    if (m_blue > 1.0f)
      {
      m_blue     = 1.0f;
      overflow_b = true;
      }

    if (overflow_b)
      {
      if (m_red < AColor_1_over_255)   m_red   = AColor_1_over_255;
      if (m_green < AColor_1_over_255) m_green = AColor_1_over_255;
      if (m_blue < AColor_1_over_255)  m_blue  = AColor_1_over_255;
      }

    current_y = get_luminance();
    }
  }
