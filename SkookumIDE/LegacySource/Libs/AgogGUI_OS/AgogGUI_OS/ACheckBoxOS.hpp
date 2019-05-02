// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ACheckBoxOS class declaration header
//=======================================================================================


#ifndef __ACHECKBOXOS_HPP
#define __ACHECKBOXOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AButtonOS.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#define ACHECKBOX_TEXT_DEFAULT "Toggle"


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
enum eACheckType
  {
  ACheckType_2_state,
  ACheckType_3_state
  };


//---------------------------------------------------------------------------------------
// A ACheckBoxOS is a built-in check box / toggle button control from the operating system.
// 
// Author(s)  Conan Reis
class ACheckBoxOS : public AWindow
  {
  public:
  // Common Methods

    ACheckBoxOS(AWindow * parent_p, const AString & text = AString(ACHECKBOX_TEXT_DEFAULT), eACheckType states = ACheckType_2_state, const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_auto_def);
    ACheckBoxOS(AWindow * parent_p, eACheckType states = ACheckType_2_state, const ARegion & region = AWindow::ms_region_auto_def);
    ~ACheckBoxOS();
    
  // Methods
    
    void enable_3d_shading(bool shade_3d = true);
    void resize();
    void set_text(const AString & text, bool resize_to_text = false, eAHorizAlign horizontal = AHorizAlign_centered, eAVertAlign vertical = AVertAlign_centered, bool word_wrap = false);

    bool   is_toggled() const                    { return (get_toggle_state() != AFlag_off); }
    bool   enable_toggle(bool new_state = true)  { return set_toggle_state(eAFlag(new_state)); }
    void   toggle();
    eAFlag get_toggle_state() const;
    bool   set_toggle_state(eAFlag new_state = AFlag_on);
    void   set_on_toggled_func(AFunctionArgBase<eAFlag> * toggled_func_p, bool free_toggle_func = true);

    // ?    get_image() const;
    // void set_image(?);

    // Useful Methods inherited from AWindow

    // Position / Size Methods - see AWindow
    // Display State Methods - see AWindow
    //?void          enable_input(bool input_accepted = true);
    //?void          enable_sizing(bool user_sizable = true);
    //?HCURSOR       get_cursor() const;
    // const AFont & get_font() const;
    //?HICON         get_icon() const;
    // AString       get_title() const;
    //?bool          is_input_enabled() const;
    //?bool          is_sizable() const;
    // void          set_border(eBorder border = Border_raised);
    //?void          set_cursor(HCURSOR cursor_handle);
    // void          set_focus();
    // void          set_font(const AFont & font_p);
    //?void          set_icon(HICON icon_handle);

  // Class Methods
    static eAFlag group_flag2toggle(eAGroupFlag group_flag);

  protected:
  // Event Methods

    virtual void on_toggled(eAFlag new_state);

    // Inherited events from AWindow

    virtual bool on_control_event_standard(uint32_t code);
    //virtual bool on_focus();
    //virtual bool on_focus_lost();

  // Data Members

    bool m_free_toggle_func;

    AFunctionArgBase<eAFlag> * m_toggled_func_p;

  };  // ACheckBoxOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Converts a eAGroupFlag to a eAFlag type
// Author(s):   Conan Reis
inline eAFlag ACheckBoxOS::group_flag2toggle(eAGroupFlag group_flag)
  {
  switch (group_flag)
    {
    case AGroupFlag_present:
	  return AFlag_on;

    case AGroupFlag_mixed:
	  return AFlag__toggle;

    default:  // AGroupFlag_absent, AGroupFlag_uninitialized
	  return AFlag_off;
    }
  }


#endif  // __ACHECKBOXOS_HPP


