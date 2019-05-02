// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFontSystemBase class declaration header
//=======================================================================================


#ifndef __AFONTSYSTEMBASE_HPP
#define __AFONTSYSTEMBASE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\AString.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#define AFONT_FACE_DEF        "Tahoma"
#define AFONT_FACE_FIXED_DEF  "Lucida Console"

const float AFont_point_def = 11.0f;


//=======================================================================================
// Global Structures
//=======================================================================================

enum eAFontType
  {
  AFontType_true_type,
  AFontType_d3dx        // TrueType font optimized for Direct3D
  };

// Font Weight - note that these numbers correspond to the Windows' constants
enum eAFontWeight
  {
  AFontWeight_thin       = 100,
  AFontWeight_extralight = 200,
  AFontWeight_light      = 300,
  AFontWeight_normal     = 400,
  AFontWeight_medium     = 500,
  AFontWeight_semibold   = 600,
  AFontWeight_bold       = 700,
  AFontWeight_extrabold  = 800,
  AFontWeight_heavy      = 900
  };

// Font Family - note that these numbers correspond to the Windows' constants
enum eAFontFamily
  {
  AFontFamily_dont_care  = 0 << 4,
  AFontFamily_roman      = 1 << 4,
  AFontFamily_swiss      = 2 << 4,
  AFontFamily_modern     = 3 << 4,
  AFontFamily_script     = 4 << 4,
  AFontFamily_decorative = 5 << 4,

  AFontFamily__mask      = 0xf0
  };

//---------------------------------------------------------------------------------------
// Notes      Describes a system specific font type.
// Subclasses ATrueTypeFont
// See Also   ATrueTypeFont, AFont
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib
// Inlibs     AgogGUI\AgogGUI.lib
// Examples:      
// Author(s)  Conan Reis
class AFontSystemBase
  {
  public:
  // Nested Classes

  // Common Methods

    AFontSystemBase(const AString & name);

  // Non-Modifying Methods

    virtual eAFontType get_type() const = 0;
    virtual int        get_width(const AString & text, uint32_t start_pos = 0u, uint32_t char_count = ALength_remainder) const = 0;
    virtual bool       is_hit_char(const AString & text, int x_pos, uint32_t * char_pos_p = nullptr, uint32_t start_pos = 0u) const = 0;
    virtual void       word_wrap(AString * text_p, int line_width) const = 0;

  // Modifying Methods

    void         reference();
    void         dereference();
    virtual void free() = 0;
    virtual void set_char_spacing(int extra_pixels) = 0;


  // Class Methods

  protected:

  public:  // For quick access
  // Data Members

    AString      m_name;            // identifying name of the font
    int          m_height;          // height of font in pixels (ascent + descent and not including the leading value) of characters
    f32          m_point_size;      // height based on pixels per inch or resolution pixels = point size * (vertical pixels per inch / 72)
    uint32_t     m_avg_width;
    uint32_t     m_weight;
    int          m_ascent;          // number of pixels that the font extends up from the font's baseline for chanacters like:  bdfhijkltABCDEFGHIJKLMNOPQ...
    int          m_descent;         // [height - ascent] number of pixels below the font's base line for characters like:  jpqyQ.
    int          m_leading;         // specifies the number of vertical pixels of space that the normally required between rows
    int          m_spacing;         // Specifies the number of extra horizontal pixels of space to use between characters above and beyond the recommended size.  This value can be negative.
    eAFontFamily m_font_family;
    bool         m_italicised;      // Boolean indicating whether font is italicised or not
    bool         m_proportional;    // (pitch) whether the font has characters of a fixed (monospace) - false or proportional - true width
    bool         m_underlined;
    bool         m_antialiased;

    uint     m_reference_count;

    // Future data
    // m_rotation - clockwise rotation of text from un-rotated top leftmost point in tenths of a degree with three o'clock at 0.  For example, 450 will have the end of a text draw point down 45degrees and to the right.  This may never be implemented.  Drawing to a temporary bitmap and rotating that may be used instead.

  // Class Data Members
  };  // AFontSystemBase


//=======================================================================================
// Inline Functions
//=======================================================================================


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Increments the reference count of this dependant font
// Returns:     itself
// Author(s):    Conan Reis
inline AFontSystemBase::AFontSystemBase(const AString & name) :
  m_name(name),
  m_reference_count(0)
  {
  // Derived classes should initialize data members
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Decrements the reference count of this dependant font.  If the
//              reference count then zero, it calls free().
// Author(s):    Conan Reis
inline void AFontSystemBase::dereference()
  {
  // Coded like this to avoid potential load-hit-store penalty
  uint reference_count = m_reference_count;
  m_reference_count = --reference_count;
  if (!reference_count)
    {
    free();
    }
  }

//---------------------------------------------------------------------------------------
//  Increments the reference count of this dependant font
// Author(s):    Conan Reis
inline void AFontSystemBase::reference()
  {
  m_reference_count++;
  }


#endif // __AFONTSYSTEMBASE_HPP


