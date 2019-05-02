// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADisplay class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\ADisplay.hpp>
#include <AgogGUI\AWindow.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

namespace
{

  struct AMonitorEnumInfo
    {
    uint        m_count;
    uint        m_primary_id;
    ADisplay * m_displays_a;
    };

  HMONITOR g_monitor_handles_a[ADisplay::COUNT_max];  // This has paired info in ADisplay::ms_displays_a - this is so that windows.h does not need to be included in the header
  uint      g_display_count = ADisplay::COUNT_stale;   // Copy of ADisplay::ms_display_count for easy access

  //---------------------------------------------------------------------------------------
  // Called by ADisplay::refresh_info()
  // Author(s):   Conan Reis
  BOOL CALLBACK monitor_enum_proc(
    HMONITOR handle,
    HDC      device_context,
    RECT *   rect_p,
    LPARAM   user_info
    )
    {
    MONITORINFOEX info;
    
    info.cbSize = sizeof(MONITORINFOEX);

    ::GetMonitorInfo(handle, &info);

    DISPLAY_DEVICE dev_info;

    dev_info.cb = sizeof(DISPLAY_DEVICE);
    ::EnumDisplayDevices(info.szDevice, 0, &dev_info, 0u);

    AMonitorEnumInfo * enum_p    = reinterpret_cast<AMonitorEnumInfo *>(user_info);
    uint                id        = enum_p->m_count;
    ADisplay *         display_p = &enum_p->m_displays_a[id];

    g_monitor_handles_a[id] = handle;

    display_p->m_name.set_cstr(dev_info.DeviceString, ALength_calculate, false);
    display_p->m_device_name.set_cstr(info.szDevice, ALength_calculate, false);  // $Revist - CReis This name may need conversion.
    display_p->m_device_num  = uint(info.szDevice[display_p->m_device_name.get_length() - 1u] - '0');  // $Revisit - CReis This will only work for the first 10 devices
    display_p->m_region      = info.rcMonitor;
    display_p->m_region_work = info.rcWork;
    display_p->m_id          = id;

    if (info.dwFlags & MONITORINFOF_PRIMARY)
      {
      display_p->m_primary = true;

      enum_p->m_primary_id = id;
      }

    enum_p->m_count++;

    // TRUE = Continue enumeration
    return TRUE;
    }

  //---------------------------------------------------------------------------------------
  // Converts a monitor handle to an id.
  //             [It assumes that the display info is not stale.]
  // Author(s):   Conan Reis
  uint monitor_handle_to_id(HMONITOR handle)
    {
    uint id = 0u;

    while (id < g_display_count)
      {
      if (g_monitor_handles_a[id] == handle)
        {
        return id;
        }
      id++;
      }

    A_ERRORX(AErrMsg("Could not find display id!", AErrLevel_notify));

    return ADisplay::get_primary_id();
    }

} // End unnamed namespace


//=======================================================================================
// Class Data Members
//=======================================================================================

uint      ADisplay::ms_display_count = ADisplay::COUNT_stale;
uint      ADisplay::ms_primary_id    = 0u;
ARegion   ADisplay::ms_virtual_region;
ADisplay  ADisplay::ms_displays_a[ADisplay::COUNT_max];  // This has paired info in g_monitor_handles_a


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converts the virtual display id (which could be one of the ID_* constants)
//             to a non-virtual id.
// Returns:    non virtual id
// Arg         id - virtual display id (which could be one of the ID_* constants)
// Notes:      [It assumes that the display info is not stale.]
// Modifiers:   static
// Author(s):   Conan Reis
uint ADisplay::normalize_id(uint id)
  {
  switch (id)
    {
    case ID_primary:
      return ms_primary_id;

    case ID_mouse_display:
      return get_mouse_id();

    default:
      if (id > ms_display_count)
        {
        A_ERRORX(AErrMsg("Tried to get infomration for a non-existent display!", AErrLevel_notify));

        return ms_primary_id;
        }
    }

    return id;
  }

//---------------------------------------------------------------------------------------
// Get the region of the virtual display (or virtual desktop).
// Returns:    virutal display region
// Notes:      The virtual display is the bounding rectangle enclosing all the displays.
//             The primary display top left corner is always (0,0) and because of this,
//             the virtual display may have an offset(potentially even negative values)
//             depending on how the other displays are arranged.
//
//                Virtual Display
//                         \
//                ...................+--------+
//                :                  |        |
//                +------+           | Disp3  |
//                |      +-----------+        |
//                |Disp2 |0,0        +--------+
//                +------+           |        :
//                :      |   Disp1   |        :
//                :      | [Primary] |        :
//                :......+-----------+........:
//
// Modifiers:   static
// Author(s):   Conan Reis
const ARegion & ADisplay::get_virtual_region()
  {
  if (AApplication::is_os_version(AOSVersion_win_98, AOSFamily_nt_or_win))
    {
    ms_virtual_region.m_x      = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    ms_virtual_region.m_y      = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    ms_virtual_region.m_width  = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    ms_virtual_region.m_height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
  else
    {
    ms_virtual_region.m_x      = 0;
    ms_virtual_region.m_y      = 0;
    ms_virtual_region.m_width  = ::GetSystemMetrics(SM_CXSCREEN);
    ms_virtual_region.m_height = ::GetSystemMetrics(SM_CYSCREEN);
    }

  return ms_virtual_region;
  }

//---------------------------------------------------------------------------------------
// Determines id of display that contains or is nearest to the mouse cursor.
// Returns:    id of display that contains or is nearest to the mouse cursor.
// See:        get_info(), center()
// Modifiers:   static
// Author(s):   Conan Reis
uint ADisplay::get_mouse_id()
  {
  refresh_if_stale();

  // This code assumes that the mouse cursor will be contained by at least one display.
  // It defaults to the primary display id otherwise.

  uint    id = 0u;
  AVec2i pos(AMouse::get_position());

  while (id < ms_display_count)
    {
    if (ms_displays_a[id].m_region.is_in(pos))
      {
      return id;
      }
    id++;
    }

  return ms_primary_id;
  }

//---------------------------------------------------------------------------------------

void ADisplay::destroy()
  {
  m_name        = AString::ms_empty;
  m_device_name = AString::ms_empty;
  }

//---------------------------------------------------------------------------------------

void ADisplay::initialize()
  {
  }

//---------------------------------------------------------------------------------------

void ADisplay::deinitialize()
  {
  // Free anything potentially allocated
  for (uint32_t i = 0; i < A_COUNT_OF(ms_displays_a); ++i)
    {
    ms_displays_a[i].destroy();
    }
  }

//---------------------------------------------------------------------------------------
// Gets info on specified display.
// Returns:    pointer to display or nullptr if device number not found
// Arg         device_num - Device number of display
// Arg         primary_if_not_found - if the device number is not found and this is true
//             then the primary display info is returned, if it is false then nullptr is
//             returned.  (Default false)
// Modifiers:   static
// Author(s):   Conan Reis
const ADisplay * ADisplay::find_device_num(
  uint  device_num,
  bool primary_if_not_found // = false
  )
  {
  refresh_if_stale();

  uint id = 0u;

  while (id < ms_display_count)
    {
    if (ms_displays_a[id].m_device_num == device_num)
      {
      return &ms_displays_a[id];
      }
    id++;
    }

  return primary_if_not_found
    ? &ms_displays_a[ms_primary_id]
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines the display that the point intersects if any.
// Returns:    A pointer to a ADisplay structure or nullptr.
// Arg         x - x screen position to test
// Arg         y - y screen position to test
// Arg         miss_return - Action to take if a display is not intersected - one of:
//               OnMiss_null    - return nullptr
//               OnMiss_nearest - return the nearest display (Default)
//               OnMiss_primary - return the nearest display
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
const ADisplay * ADisplay::hit_display(
  int     x,
  int     y,
  eOnMiss miss_return // = OnMiss_nearest
  )
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  refresh_if_stale();

  POINT    pos    = {x, y};
  HMONITOR handle = ::MonitorFromPoint(pos, DWORD(miss_return));

  if (handle)
    {
    return &ms_displays_a[monitor_handle_to_id(handle)];
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines the display that the region intersects if any.
// Returns:    A pointer to a ADisplay structure or nullptr.
// Arg         region - screen region to test
// Arg         miss_return - Action to take if a display is not intersected - one of:
//               OnMiss_null    - return nullptr
//               OnMiss_nearest - return the nearest display (Default)
//               OnMiss_primary - return the nearest display
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
const ADisplay * ADisplay::hit_display(
  const ARegion & region,
  eOnMiss         miss_return // = OnMiss_nearest
  )
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  refresh_if_stale();

  RECT rect(region);

  HMONITOR handle = ::MonitorFromRect(&rect, DWORD(miss_return));

  if (handle)
    {
    return &ms_displays_a[monitor_handle_to_id(handle)];
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines the display that the window intersects if any.
// Returns:    A pointer to a ADisplay structure or nullptr.
// Arg         win - window to test
// Arg         miss_return - Action to take if a display is not intersected - one of:
//               OnMiss_null    - return nullptr
//               OnMiss_nearest - return the nearest display (Default)
//               OnMiss_primary - return the nearest display
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
const ADisplay * ADisplay::hit_display(
  const AWindow & win,
  eOnMiss         miss_return // = OnMiss_nearest
  )
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  refresh_if_stale();

  HMONITOR handle = ::MonitorFromWindow(win.get_os_handle(), DWORD(miss_return));

  if (handle)
    {
    return &ms_displays_a[monitor_handle_to_id(handle)];
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the point is visble on any display.
// Returns:    true - it is visible, false - it is not visible.
// Arg         x - x screen position to test
// Arg         y - y screen position to test
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
bool ADisplay::is_visible(
  int x,
  int y
  )
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  POINT pos = {x, y};

  return (::MonitorFromPoint(pos, MONITOR_DEFAULTTONULL) != nullptr);
  }

//---------------------------------------------------------------------------------------
// Determines if any part of the region is visble on any display.
// Returns:    true - it is visible, false - it is not visible.
// Arg         region - screen region to test
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
bool ADisplay::is_visible(const ARegion & region)
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  RECT rect(region);

  return (::MonitorFromRect(&rect, MONITOR_DEFAULTTONULL) != nullptr);
  }

//---------------------------------------------------------------------------------------
// Determines if any part of the window is visble on any display.
// Returns:    true - it is visible, false - it is not visible.
// Arg         win - window to test
// Notes:      Win98 and up only.
// Modifiers:   static
// Author(s):   Conan Reis
bool ADisplay::is_visible(const AWindow & win)
  {
  // $Revisit - CReis This could be modified so that it works for Win95 too.

  return (::MonitorFromWindow(win.get_os_handle(), MONITOR_DEFAULTTONULL) != nullptr);
  }

//---------------------------------------------------------------------------------------
// Refreshes display info.
// Modifiers:   static
// Author(s):   Conan Reis
void ADisplay::refresh_info()
  {
  // $Vital - CReis This should be reinvoked whenever WM_DISPLAYCHANGE is sent (and possibly with other user changes).

  ms_display_count = 0u;
  if (AApplication::is_os_version(AOSVersion_win_98, AOSFamily_nt_or_win))
    {
    AMonitorEnumInfo enum_info = {0u, 0u, ms_displays_a};

    ::EnumDisplayMonitors(nullptr, nullptr, monitor_enum_proc, reinterpret_cast<LPARAM>(&enum_info));

    ms_primary_id    = enum_info.m_primary_id;
    ms_display_count = enum_info.m_count;
    }
  else  // Windows 95
    {
    RECT       rect;
    ADisplay & display = ms_displays_a[0];

    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

    display.m_name            = "Monitor";
    display.m_device_name     = "\\\\.\\DISPLAY1";
    display.m_region.m_x      = 0;
    display.m_region.m_y      = 0;
    display.m_region.m_width  = ::GetSystemMetrics(SM_CXSCREEN);
    display.m_region.m_height = ::GetSystemMetrics(SM_CYSCREEN);
    display.m_region_work     = rect;
    display.m_id              = 0u;
    display.m_primary         = true;

    ms_primary_id    = 0u;
    ms_display_count = 1;
    }
  g_display_count = ms_display_count;
  }

//---------------------------------------------------------------------------------------
// Takes normal "landscape" values like 800x600 and returns display area
//             based on whether the display is rotated or not - i.e. 800x600 for landscape
//             or 600x800 if the display is in portrait (pivot/rotated) mode.
// Returns:    Appropriately pivoted area
// Arg         side_long - long side of area
// Arg         si_side_short - short side of area
// Arg         id - virtual display id to pivot area to (which could be one of the ID_*
//             constants)
// Modifiers:   static
// Author(s):   Conan Reis
AVec2i ADisplay::pivot_area(
  const AVec2i & landscape_area,  // = AVec2i(800, 600)
  uint            id               // = ID_primary
  )
  {
  if (get_info(id).is_landscape())
    {
    return landscape_area;
    }

  return AVec2i(landscape_area.m_y, landscape_area.m_x);
  }

//---------------------------------------------------------------------------------------
// Takes normal "landscape" values like 800x600 and returns display area
//             based on whether the display is rotated or not - i.e. 800x600 for landscape
//             or 600x800 if the display is in portrait (pivot/rotated) mode.
// Returns:    Appropriately pivoted region
// Arg         side_long - long side of area
// Arg         si_side_short - short side of area
// Modifiers:   static
// Author(s):   Conan Reis
ARegion ADisplay::pivot_region(
  const ARegion & landscape_region  // = ARegion(0, 0, 800, 600)
  )
  {
  // The lengths are halved to approximate the common area whether pivoted or not.
  const ADisplay * disp_p = hit_display(
    ARegion(landscape_region.m_x, landscape_region.m_y, landscape_region.m_height / 2, landscape_region.m_width / 2));

  if (disp_p->is_landscape())
    {
    return landscape_region;
    }

  return ARegion(landscape_region.m_x, landscape_region.m_y, landscape_region.m_height, landscape_region.m_width);
  }
