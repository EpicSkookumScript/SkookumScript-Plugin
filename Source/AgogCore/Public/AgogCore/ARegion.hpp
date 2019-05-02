// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARegion structure declaration header
// Notes:          Simple region structure.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AVec2i.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

#if defined(A_PLAT_PC)
  struct  tagRECT;
  typedef tagRECT RECT;    // Forward declaration so windows header does not need to be included
#endif


//---------------------------------------------------------------------------------------
// Different than a rectangle since a rectangle has (left, top, right, bottom) and a
// region has (x, y, width, height)
struct A_API ARegion
  {
  // Data Members

    // Any modifications or additions to these data members may require that the
    // Agog3D\AViewport class also be changed.
    // $Revisit - CReis Consider changing these values to 2 AVec2i structures.

    int m_x;
    int m_y;
    int m_width;
    int m_height;

  // Class Data Members

    static ARegion ms_zero;

  // Common Methods

    ARegion(const AVec2i & area, const AVec2i & offset = AVec2i::ms_zero) : m_x(offset.m_x), m_y(offset.m_y), m_width(area.m_x), m_height(area.m_y) {}
    ARegion(int x = 0, int y = 0, int width = 0, int height = 0)             : m_x(x), m_y(y), m_width(width), m_height(height) {}
    ARegion(const ARegion & region)                                      : m_x(region.m_x), m_y(region.m_y), m_width(region.m_width), m_height(region.m_height) {}

    ARegion & operator=(const ARegion & region)  { m_x = region.m_x; m_y = region.m_y; m_width = region.m_width; m_height = region.m_height; return *this; }

  // Conversion Methods

    #if defined(A_PLAT_PC)

      ARegion(const RECT & rect);

      ARegion & operator=(const RECT & rect);

      operator RECT () const;
      RECT & fill_rect(RECT * rect_p) const;

    #endif

  // Comparison Methods

    bool operator==(const ARegion & region) const  { return (m_x == region.m_x) && (m_y == region.m_y) && (m_width == region.m_width) && (m_height == region.m_height); }
    bool operator!=(const ARegion & region) const  { return (m_x != region.m_x) || (m_y != region.m_y) || (m_width != region.m_width) || (m_height != region.m_height); }

  // Accessor Methods

    AVec2i    get_area() const                         { return AVec2i(m_width, m_height); }
    f32       get_aspect() const                       { return f32(m_width) / f32(m_height); }
    AVec2i    get_center() const                       { return AVec2i(m_x + a_round(f32(m_width) * 0.5f), m_y + a_round(f32(m_height) * 0.5f)); }
    AVec2i    get_pos() const                          { return AVec2i(m_x, m_y); }
    bool      is_landscape() const                     { return m_width >= m_height; }
    bool      is_zero() const                          { return ((m_width + m_height) == 0) && (m_x == 0) && (m_y == 0); }
    ARegion & set_area(int width, int height)          { m_width = width; m_height = height; return *this; }
    ARegion & set_area(const AVec2i & area)            { m_width = area.m_x; m_height = area.m_y; return *this; }
    ARegion & set_pos(int x, int y)                    { m_x = x; m_y = y; return *this; }
    ARegion & set_pos(const AVec2i & pos)              { m_x = pos.m_x; m_y = pos.m_y; return *this; }
    ARegion & zero_pos()                               { m_x = 0; m_y = 0; return *this; }

  // Methods

    void      center(ARegion * region_p) const;
    ARegion & center_on(const ARegion & region)        { region.center(this); return *this; }
    void      constrain_area(int width_min, int width_max, int height_min, int height_max);
    void      contract(int inset)                      { m_x += inset; m_y += inset; m_width -= inset * 2; m_height -= inset * 2; }
    bool      is_hit(const ARegion & region) const;
    bool      is_in(const AVec2i & pos) const          { return (pos.m_x >= m_x) && (pos.m_x < (m_x + m_width)) && (pos.m_y >= m_y) && (pos.m_y < (m_y + m_height)); }
    void      snap_enclose(ARegion * region_p) const;
  };


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Centers supplied region on this region.
// Arg         region_p - region to center.  'm_width' and 'm_height' must be valid.
//             'm_x' and 'm_y' are set to centered offset position.  The original values
//             of 'm_x' and 'm_y' are ignored.
// Author(s):   Conan Reis
inline void ARegion::center(ARegion * region_p) const
  {
  region_p->m_x = m_x + a_round(f32(m_width - region_p->m_width) * 0.5f); 
  region_p->m_y = m_y + a_round(f32(m_height - region_p->m_height) * 0.5f); 
  }

//---------------------------------------------------------------------------------------
// Determines if this region intersects with the specified region.
// Returns:    true if they intersect, false if not
// Arg         region - region to test against
// Author(s):   Conan Reis
inline bool ARegion::is_hit(const ARegion & region) const
  {
  return ((m_x <= (region.m_x + region.m_width)) &&
    (region.m_x <= (m_x + m_width)) && 
    ((m_y + m_height) >= region.m_y) && 
    ((region.m_y + region.m_height) >= m_y)); 
  }
