// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADisplay class definition module
//=======================================================================================


#ifndef _ADISPLAY_HPP
#define _ADISPLAY_HPP
#pragma once


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\ARegion.hpp>
#include <AgogCore\AString.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

class AWindow;  // Forward declaration

//---------------------------------------------------------------------------------------
// Notes    ADisplay info object
//
//          The primary display is not necessarily the zeroeth display id.
//          Changing the primary display does not change the order of the display ids.
//          Adding / activating a new display usually adds the display after the already
//          active displays, but after a reboot the order of displays may change.
// UsesLibs AgogCore\AgogCore.lib AgogIO\AgogIO.lib   
// Inlibs   AgogGUI\AgogGUI.lib
// Author   Conan Reis
class ADisplay
  {
  public:

  // Nested Structures

    enum
      {
      COUNT_stale = 0,
      COUNT_max   = 16,  // CReis I know I'm just asking for trouble here with a magical number, but I don't expect more than 16 displays on one system anytime soon.  [The maximum as of WindowsXP is 10.]

      ID_primary,        // Substitute with the id of the primary display.
      ID_mouse_display   // Substitute with the id of the display that the mouse cursor is on or nearest.
      };

    enum eOnMiss
      {
      OnMiss_null    = 0,  // These match MS API constants MONITOR_DEFAULTTONULL etc.
      OnMiss_primary = 1,
      OnMiss_nearest = 2
      };

  // Methods

    void destroy();
    bool is_landscape() const  { return m_region.is_landscape(); }

  // Class Methods

    static void             initialize();
    static void             deinitialize();

    static void             center(ARegion * region_p, uint id = ID_mouse_display);
    static ARegion          center(int width, int height, uint id = ID_mouse_display);
    static const ADisplay * find_device_num(uint device_num, bool primary_if_not_found = false);
    static uint             get_count();
    static uint             get_primary_id()    { refresh_if_stale(); return ms_primary_id; }
    static uint             get_mouse_id();
    static const ARegion &  get_virtual_region();
    static const ADisplay & get_info(uint id = ID_primary);
    static const ADisplay * hit_display(int x, int y, eOnMiss miss_return = OnMiss_nearest);
    static const ADisplay * hit_display(const ARegion & region, eOnMiss miss_return = OnMiss_nearest);
    static const ADisplay * hit_display(const AWindow & win, eOnMiss miss_return = OnMiss_nearest);
    static bool             is_multi_display()  { return get_count() > 1u; }
    static bool             is_visible(int x, int y);
    static bool             is_visible(const ARegion & region);
    static bool             is_visible(const AWindow & win);
    static AVec2i           pivot_area(const AVec2i & lanscape_area = AVec2i(800, 600), uint id = ID_primary);
    static AVec2i           pivot_area(int side_long, int side_short, uint id = ID_primary)  { return pivot_area(AVec2i(side_long, side_short), id); }
    static ARegion          pivot_region(const ARegion & landscape_region = ARegion(0, 0, 800, 600));

    // Called internally as needed, but exposed here just in case...

      static void make_stale()         { ms_display_count = COUNT_stale; }
      static uint normalize_id(uint id);
      static void refresh_info();
      static void refresh_if_stale()   { if (ms_display_count == COUNT_stale) { refresh_info(); } }

  // Data Members - public for quick access

      uint    m_id;           // Id of display - may not be persistent after reboot but has quick access
      uint    m_device_num;   // Device number of display - should be relatively persistent - same as last digit in device_name
      AString m_name;         // Human name of display - usually the name of the Monitor ex. "ViewSonic PS790"
      AString m_device_name;  // Name of display device (the most static identifier) - usually in the form "//./DISPLAY#"
      bool    m_primary;      // true if primary display
      ARegion m_region;       // Display area and offset in virtual display
      ARegion m_region_work;  // Working area - takes into account items like toolbars and 'Start' menu

  protected:

  // Class Data Members

    static uint     ms_display_count;
    static uint     ms_primary_id;
    static ARegion  ms_virtual_region;
    static ADisplay ms_displays_a[COUNT_max];

  };  // ADisplay


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Gets the number of active displays on the system.
// Returns:    number of active displays on the system
// See:        
// Modifiers:   static
// Author(s):   Conan Reis
inline uint ADisplay::get_count()
  {
  refresh_if_stale();

  return ms_display_count;

  // Old Code:

  // Note that GetSystemMetrics(SM_CMONITORS) counts only display monitors. This is
  // different from EnumDisplayMonitors, which enumerates display monitors and also
  // non-display pseudo-monitors.
  //return AApplication::is_os_version(AOSVersion_win_98, AOSFamily_nt_or_win)
  //  ? ::GetSystemMetrics(SM_CMONITORS)
  //  : 1u;
  }

//---------------------------------------------------------------------------------------
// Gets info on specified display.
// Returns:    a display
// Arg         id - Id of display to get.  ID_primary and ID_mouse_display are
//             also valid ids.  (Default ID_primary)
// Modifiers:   static
// Author(s):   Conan Reis
inline const ADisplay & ADisplay::get_info(
  uint id // = ID_primary
  )
  {
  refresh_if_stale();

  return ms_displays_a[normalize_id(id)];
  }

//---------------------------------------------------------------------------------------
// Centers the supplied region on the work region of the specified display
//             in virtual screen co-ordinates.
// Arg         region_p - region to center.  'm_width' and 'm_height' must be valid.
//             'm_x' and 'm_y' are set to centered offset position.
// Arg         id - Id of display to center region on.  ID_primary and ID_mouse_display
//             are also valid ids.  (Default ID_mouse_display)
// Modifiers:   static
// Author(s):   Conan Reis
inline void ADisplay::center(
  ARegion * region_p,
  uint       id // = ID_mouse_display
  )
  {
  get_info(id).m_region_work.center(region_p);
  }

//---------------------------------------------------------------------------------------
// Centers the supplied area on the work region of the specified display
//             in virtual screen co-ordinates.
// Arg         width - width of area to center
// Arg         height - height of area to center
// Arg         id - Id of display to center region on.  ID_primary and ID_mouse_display
//             are also valid ids.  (Default ID_mouse_display)
// Modifiers:   static
// Author(s):   Conan Reis
inline ARegion ADisplay::center(
  int width,
  int height,
  uint id // = ID_mouse_display
  )
  {
  ARegion region(0, 0, width, height);

  get_info(id).m_region_work.center(&region);

  return region;
  }

#endif  // _ADISPLAY_HPP


