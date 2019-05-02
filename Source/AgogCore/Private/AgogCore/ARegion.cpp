// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARegion class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/ARegion.hpp>
#if defined(A_PLAT_PC)
  #define WIN32_LEAN_AND_MEAN // Keep this define out of public header files
  #include <windows.h>  // For definition of RECT
#endif


//=======================================================================================
// Class Data Members
//=======================================================================================

ARegion ARegion::ms_zero;


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if defined(A_PLAT_PC)

//---------------------------------------------------------------------------------------
// Constructor from a RECT
// Returns:    itself
// Arg         rect - rectangle to construct from
// See:        operator RECT(), =(rect), fill_rect()
// Notes:      This is not inlined so that the windows header does not need to be included.
// Author(s):   Conan Reis
ARegion::ARegion(const RECT & rect) :
  m_x(rect.left),
  m_y(rect.top),
  m_width(rect.right - rect.left),
  m_height(rect.bottom - rect.top)
  {
  }

//---------------------------------------------------------------------------------------
// Assignment from a RECT
// Returns:    itself
// Arg         rect - rectangle to get info from
// See:        operator RECT(), fill_rect()
// Notes:      This is not inlined so that the windows header does not need to be included.
// Author(s):   Conan Reis
ARegion & ARegion::operator=(const RECT & rect)
  {
  m_x      = rect.left;
  m_y      = rect.top;
  m_width  = rect.right - rect.left;
  m_height = rect.bottom - rect.top;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Converter to a RECT
// Returns:    a RECT version of itself
// See:        Region(rect), =(rect), fill_rect()
// Notes:      This is not inlined so that the windows header does not need to be included.
// Author(s):   Conan Reis
ARegion::operator RECT () const
  {
  RECT rect = {m_x, m_y, m_x + m_width, m_y + m_height};

  return rect;
  }

//---------------------------------------------------------------------------------------
// Converter to a RECT
// Returns:    a reference to the rectangle that was passed as an argument
// Arg         rect_p - rectangle to fill
// See:        Region(rect), =(rect), operator RECT()
// Notes:      This is not inlined so that the windows header does not need to be included.
// Author(s):   Conan Reis
RECT & ARegion::fill_rect(RECT * rect_p) const
  {
  rect_p->left   = m_x;
  rect_p->top    = m_y;
  rect_p->right  = m_x + m_width;
  rect_p->bottom = m_y + m_height;

  return *rect_p;
  }

#endif  // A_PLAT_PC


//---------------------------------------------------------------------------------------
// Ensures that the area of the region is constrained to the minimums and
//             maximums specified.
// Arg         width_min - minimum width allowed
// Arg         width_max - maximum width allowed
// Arg         height_min - minimum height allowed
// Arg         height_max - maximum height allowed
// Author(s):   Conan Reis
void ARegion::constrain_area(
  int width_min,
  int width_max,
  int height_min,
  int height_max
  )
  {
  if (m_width < width_min)
    {
    m_width = width_min;
    }
  else
    {
    if (m_width > width_max)
      {
      m_width = width_max;
      }
    }

  if (m_height < height_min)
    {
    m_height = height_min;
    }
  else
    {
    if (m_height > height_max)
      {
      m_height = height_max;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that the supplied region is fully visible within this region while
//             attempting to retain as much of its original area and offset as possible.
//             A higher priority is given to the area over the offset - i.e. if the
//             region can keep its original width or height by changing its offset then
//             it will.  The area is only modified if it is larger than this region.
// Arg         region_p - region to snap to this region
// Author(s):   Conan Reis
void ARegion::snap_enclose(ARegion * region_p) const
  {
  if (region_p->m_width > m_width)
    {
    region_p->m_width = m_width;
    region_p->m_x     = m_x;
    }
  else
    {
    if (region_p->m_x < m_x)
      {
      region_p->m_x = m_x;
      }
    else
      {
      if ((region_p->m_x + region_p->m_width) > (m_x + m_width))
        {
        region_p->m_x = m_x + m_width - region_p->m_width;
        }
      }
    }

  if (region_p->m_height > m_height)
    {
    region_p->m_height = m_height;
    region_p->m_y      = m_y;
    }
  else
    {
    if (region_p->m_y < m_y)
      {
      region_p->m_y = m_y;
      }
    else
      {
      if ((region_p->m_y + region_p->m_height) > (m_y + m_height))
        {
        region_p->m_y = m_y + m_height - region_p->m_height;
        }
      }
    }
  }

