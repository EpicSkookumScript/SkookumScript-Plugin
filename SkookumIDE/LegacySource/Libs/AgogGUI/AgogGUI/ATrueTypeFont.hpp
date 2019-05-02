// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATrueTypeFont class declaration header
//=======================================================================================


#ifndef __ATRUETYPEFONT_HPP
#define __ATRUETYPEFONT_HPP
#pragma once


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AFontSystemBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Describes a system specific font type.
// Subclasses 
// See Also   AFont
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, gdi32.lib
// Inlibs     AgogGUI\AgogGUI.lib
// Examples:      
// Author(s)  Conan Reis
class ATrueTypeFont : public AFontSystemBase
  {
  public:
  // Nested Classes

  // Common Methods

    ATrueTypeFont(const AString & name, int pixel_height, uint32_t weight, bool italicised, bool underlined, int spacing, bool antialias = true);
    ~ATrueTypeFont();

  // Non-Modifying Methods

    virtual eAFontType get_type() const;
    virtual int        get_width(const AString & text, uint32_t start_pos = 0, uint32_t char_count = ALength_remainder) const;
    virtual bool       is_hit_char(const AString & text, int x_pos, uint32_t * char_pos_p = nullptr, uint32_t start_pos = 0) const;
    virtual void       word_wrap(AString * text_p, int line_width) const;

  // Modifying Methods

    virtual void free();
    virtual void set_char_spacing(int extra_pixels);

  // Class Methods

    static void * get_info_dc();  // Cast to HDC
    static void   release_info_dc(void * info_dc_p);

  public:  // Internal class so make it a little faster
  // Data Members

    void * m_font_handle_p;  // Handle to font.

  };  // ATrueTypeFont


//=======================================================================================
// Inline Functions
//=======================================================================================


#endif // __ATRUETYPEFONT_HPP


