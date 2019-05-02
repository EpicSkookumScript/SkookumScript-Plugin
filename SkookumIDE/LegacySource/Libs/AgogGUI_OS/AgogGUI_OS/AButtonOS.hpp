// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AButtonOS class declaration header
//=======================================================================================


#ifndef __ABUTTONOS_HPP
#define __ABUTTONOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AgogGUI_OS.hpp>
#include <AgogGUI\AWindow.hpp>
#include <AgogCore\AFunctionBase.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#define ABUTTON_TEXT_DEFAULT "OK"


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class ACheckBoxOS;

//---------------------------------------------------------------------------------------
// A AButtonOS is a built-in button control from the operating system.
class AButtonOS : public AWindow
  {
  friend class ACheckBoxOS;  // So that ACheckBoxOS can access the default AMessageTargetClass

  public:

  // Common Methods

    static void initialize();
    static void deinitialize();

    AButtonOS(AWindow * parent_p, const AString & text = AString(ABUTTON_TEXT_DEFAULT), const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_auto_def);
    AButtonOS(AWindow * parent_p, const ARegion & region = AWindow::ms_region_auto_def);
    ~AButtonOS();
    
  // Methods
    
    void enable_3d_shading(bool shade_3d = true);
    void enable_default_border(bool default_button = true);
    bool is_pressed() const;
    void press();
    void resize();
    void set_on_pressed_func(AFunctionBase * pressed_func_p);
    void set_text(const AString & text, bool resize_to_text = false, eAHorizAlign horizontal = AHorizAlign_centered, eAVertAlign vertical = AVertAlign_centered, bool word_wrap = false);
    void set_image(const AString & filename, bool autoSizeRegion );
    // ?    get_image() const;

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

  protected:
  // Event Methods

    virtual void on_pressed();
    //virtual void on_pressed_double();  BS_NOTIFY
    //#virtual void on_press();
    //#virtual void on_pressing();

    // Inherited events from AWindow

    virtual bool on_control_event_standard(uint32_t code);
    //virtual bool on_focus();
    //virtual bool on_focus_lost();

  // Data Members

    AFunctionBase * m_pressed_func_p;

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // AButtonOS


//=======================================================================================
// Inline Functions
//=======================================================================================


#endif  // __ABUTTONOS_HPP


