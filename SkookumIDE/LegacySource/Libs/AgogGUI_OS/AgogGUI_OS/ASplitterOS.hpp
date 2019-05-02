// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASplitterOS class declaration header
//=======================================================================================


#ifndef __ASPLITTEROS_HPP
#define __ASPLITTEROS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>
#include <AgogGUI_OS\APopMenuOS.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

const int ASplitterOS_bar_pixel_extra = 6;       // Extra space used to draw raised area in bar center
const int ASplitterOS_bar_pixel_def   = INT_MAX;
const f32 ASplitterOS_ratio_max       = 1.0f;
const f32 ASplitterOS_ratio_def       = 0.5f;

//---------------------------------------------------------------------------------------
// Notes      A ASplitterOS controls the ratio between two vertical or horizontal child
//            windows.
// Author(s)  Conan Reis
class ASplitterOS : public AWindow
  {
  public:
  // Nested Structures

    enum eOrient
      {
      // Orientation & Order States
      Orient_horiz_ab = 0x0, // A|B

                             //  A
      Orient_vert_ab  = 0x1, //  -
                             //  B

      Orient_horiz_ba = 0x2, // B|A

                             //  B
      Orient_vert_ba  = 0x3, //  -
                             //  A

      // Test Flags - used for masking and not for passing to methods
      Orient__horizontal = 0x0,
      Orient__vertical   = 0x1,
      Orient__a_b        = 0x0,
      Orient__b_a        = 0x2
      };

    enum eReorient
      {
      Reorient_disabled,
      Reorient_swap,
      Reorient_swap_rotate
      };

    enum eRatioUpdate
      {
      RatioChange_disabled,
      //RatioChange_delayed,
      RatioChange_live
      };


  // Common Methods

    static void initialize();
    static void deinitialize();

    ASplitterOS(AWindow * parent_p = nullptr, const ARegion & region = AWindow::ms_region_def);
    ~ASplitterOS();
    
  // Methods

    APopMenuOS & get_context_menu()     { return m_context_menu; }

    int  get_bar_pixels() const         { return m_bar_pixels; }
    void set_bar_pixels(int thickness = ASplitterOS_bar_pixel_def);

    void enable_auto_update(bool update_as_needed = true);
    void update(bool pane_a = true, bool pane_b = true);

    // Many Useful Methods inherited - see AWindow

  // Pane Methods

    const AWindow & get_pane_a() const             { return *m_pane_a_p; }
    void            set_pane_a(AWindow * win_p);

    const AWindow & get_pane_b() const             { return *m_pane_b_p; }
    void            set_pane_b(AWindow * win_p);

  // Orientation Methods

    bool      is_vertical() const            { return (m_orient & Orient__vertical) == Orient__vertical; }
    eOrient   get_orientation() const        { return m_orient; }
    void      set_orientation(eOrient orient);
    void      rotate_panes_cw();
    void      rotate_panes_ccw();
    void      swap_panes();

    eReorient get_user_reorient() const      { return m_user_reorient; }
    void      set_user_reorient(eReorient reorient);

  // Ratio Methods

    f32          get_ratio() const              { return m_ratio; }
    bool         set_ratio(f32 ratio);
    void         set_ratio_min_max(f32 ratio_min = 0.0f, f32 ratio_max = ASplitterOS_ratio_max);

    eRatioUpdate get_user_ratio_update() const  { return m_user_update; }
    void         set_user_ratio_update(eRatioUpdate update_mode);


  protected:
  // Event Methods

    virtual void on_ratio_change();
    virtual void on_ratio_changing();
    virtual void on_ratio_changed();
    virtual void on_reorient();

    // Many Useful Events inherited - see AWindow

    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_draw();
    virtual void on_menu_command(uint32_t item_id);
    virtual bool on_mouse_press(eAMouse button, eAMouse buttons, const AVec2i & client_pos, bool double_click);
    virtual void on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual void on_mouse_moving(const AVec2i & client_pos, eAMouse buttons);
    virtual bool on_set_cursor(HWND cursor_win, int hit_area, int mouse_msg);
    virtual void on_sizing();

  // Internal Methods

    f32 constrain_ratio(f32 desired_ratio);

  // Data Members

    AWindow *  m_pane_a_p;
    AWindow *  m_pane_b_p;
    APopMenuOS m_context_menu;

    eOrient   m_orient;
    eReorient m_user_reorient;

    int     m_bar_pixels;
    ARegion m_bar_region;
    int     m_bar_offset;

    f32          m_ratio;          // Ratio of pane A to pane B
    f32          m_ratio_min;
    f32          m_ratio_max;
    eRatioUpdate m_user_update;
    bool         m_ratio_changing_b;
    bool         m_ratio_changed_b;
    bool         m_auto_update_b;
    bool         m_showing_popup_b;

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // ASplitterOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Enables/disables auto update on changes.
// Arg         update_as_needed - if 'true' when any changes are made the pane positions
//             and areas are recalculated and the windows are redrawn.  Use 'false' if
//             a number of modifications are being made in succession on the splitter
//             then reenable auto update and call update() after all the modifications
//             are complete so that the calculations and redraws only occur once.
// Author(s):   Conan Reis
inline void ASplitterOS::enable_auto_update(
  bool update_as_needed // = true
  )
  {
  m_auto_update_b = true;
  }


#endif  // __ASPLITTEROS_HPP


