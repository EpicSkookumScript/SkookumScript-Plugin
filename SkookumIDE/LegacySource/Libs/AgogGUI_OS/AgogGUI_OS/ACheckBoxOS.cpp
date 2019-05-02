// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ACheckBoxOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\ACheckBoxOS.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

// ACheckBoxOS enumerated constants
enum
  {
  ACheckBox_width_multiple = 3,
  ACheckBox_height_extra   = 4,
  ACheckBox_width_extra    = 24  // $Revisit - CReis This seems too much like a magic number...
  };


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Text Faced Constructor
// Returns:     itself
// Arg          parent_p - a pointer parent AWindow to this object.  This *MUST* be
//              given and cannot be nullptr.  The set_parent() method does not work properly
//              with OS graphical controls.
// Arg          text - text to place on button face
// Arg          states - number of states allowed by the check box - ACheckType_2_state (with
//              cleared and checked states) or ACheckType_3_state (with cleared, checked, and
//              indeterminate states).
// Arg          font - font to use for text
// Arg          region - position and area of the button.  If width set to Size_auto,
//              it uses the width of the supplied text using this button's initial font.
//              If height set to Size_auto, it uses the height of this button's initial
//              font.
// Notes:       Initial settings (modified with appropriately matching methods):
//                check state    - cleared
//                3D appearance  - true, the button is shaded to give it a 3D appearance.  If this were false, it would have a 'flat' look.
//                display text   - true, the button does shows the given text on its face
//                  horizontal alignment - centered
//                  vertical alignment   - centered
//                  word wrapping        - false
//
//              Initial relevant AWindow inherited settings:
//                show state     - AShowState_hidden, The button is not visible, this is since some settings that are modified after this constructor will not redraw until a redraw or show command is given
//                input enabled  - true, it is enabled rather than disabled (and greyed out)
//                keyboard focus - not focused
//                mouse cursor   - default cursor
// Author(s):    Conan Reis
ACheckBoxOS::ACheckBoxOS(
  AWindow *       parent_p,
  const AString & text,   // = AString(CHECKBOX_TEXT_DEFAULT)
  eACheckType     states, // = ACheckType_2_state
  const AFont &   font,   // = AFont::ms_default (Arial point 10)
  const ARegion & region  // = AWindow::ms_region_auto_def (0, 0, Size_auto, Size_auto)
  ) :
  AWindow(parent_p, AButtonOS::ms_default_class_p),
  m_free_toggle_func(false),
  m_toggled_func_p(nullptr)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_font = font;
  m_os_handle = ::CreateWindowEx(
    0u,                     // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    text.as_cstr(),         // title
    WS_CHILD | WS_TABSTOP | BS_LEFT | BS_VCENTER | ((states == ACheckType_2_state) ? BS_AUTOCHECKBOX : BS_AUTO3STATE), // Window & ACheckBoxOS Styles
    region.m_x,
    region.m_y,
    (region.m_width == Size_auto) ? font.get_width(text) + ACheckBox_width_extra : region.m_width,
    (region.m_height == Size_auto) ? font.get_height() + ACheckBox_height_extra : region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "ACheckBoxOS()", ACheckBoxOS);

  enable_subclass_messages();
  common_setup();
  }

//---------------------------------------------------------------------------------------
//  Constructor
// Returns:     itself
// Arg          parent_p - a pointer parent AWindow to this object.  This *MUST* be
//              given and cannot be nullptr.  The set_parent() method does not work properly
//              with OS graphical controls.
// Arg          states - number of states allowed by the check box - ACheckType_2_state (with
//              cleared and checked states) or ACheckType_3_state (with cleared, checked, and
//              indeterminate states).  (Defatult ACheckType_2_state)
// Arg          region - position and area of the button.  If width set to Size_auto,
//              it uses the height of this button's initial font * 3 for the width.  If
//              height set to Size_auto, it uses the height of this button's initial font.
//              (Default Region(0, 0, Size_auto, Size_auto))
// Notes:       Initial settings (modified with appropriately matching methods):
//                check state    - cleared
//                3D appearance  - true, the button is shaded to give it a 3D appearance.  If this were false, it would have a 'flat' look.
//                display text   - false, the button does not show any text
//                  horizontal alignment - ignored
//                  vertical alignment   - ignored
//                  word wrapping        - ignored
//
//              Initial relevant AWindow inherited settings:
//                show state     - AShowState_normal, The button is visible, though its parent must also be visible too.
//                font           - Default proportional font, (Arial point 10) - but ignored since there is no text.
//                input enabled  - true, it is enabled rather than disabled (and greyed out)
//                keyboard focus - not focused
//                mouse cursor   - default cursor
// Author(s):    Conan Reis
ACheckBoxOS::ACheckBoxOS(
  AWindow *       parent_p,
  eACheckType     states, // = ACheckType_2_state
  const ARegion & region // = AWindow::ms_region_auto_def
  ) :
  AWindow(parent_p, AButtonOS::ms_default_class_p),
  m_free_toggle_func(false),
  m_toggled_func_p(nullptr)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0u,                     // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    WS_CHILD | WS_TABSTOP | ((states == ACheckType_2_state) ? BS_AUTOCHECKBOX : BS_AUTO3STATE),  // Window & ACheckBoxOS Styles
    region.m_x,
    region.m_y,
    (region.m_width == Size_auto) ? m_font.get_height() * ACheckBox_width_multiple : region.m_width,
    (region.m_height == Size_auto) ? m_font.get_height() + ACheckBox_height_extra : region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "ACheckBoxOS()", ACheckBoxOS);

  enable_subclass_messages();
  common_setup();
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Examples:    called by system
// Author(s):    Conan Reis
ACheckBoxOS::~ACheckBoxOS()
  {
  if (m_free_toggle_func)
    {
    delete m_toggled_func_p;
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Enables or disables 3D shading of the button face.
// Arg          shade_3d - if true, the button is shaded to give it a 3D appearance, if
//              false, it has a 'flat' look.  (Default true)
// Author(s):    Conan Reis
void ACheckBoxOS::enable_3d_shading(
  bool shade_3d // = true
  )
  {
  if (shade_3d)
    {
    remove_style(BS_FLAT);
    }
  else
    {
    append_style(BS_FLAT);
    }
  }

//---------------------------------------------------------------------------------------
//  Determines if the button is currently toggled.
// Returns:     true if button is currently toggled, false if not
// See:         toggle(), on_toggled()
// Author(s):    Conan Reis
eAFlag ACheckBoxOS::get_toggle_state() const
  {
  LRESULT state_bits = ::SendMessage(m_os_handle, BM_GETCHECK, 0u, 0u);

  return (state_bits == BST_CHECKED) ? AFlag_on : ((state_bits == BST_INDETERMINATE) ? AFlag__toggle : AFlag_off);
  }

//---------------------------------------------------------------------------------------
//  Resizes the button to fit the current text displayed on the button face.
// See:         set_text(), set_font(), set_title()
// Author(s):    Conan Reis
void ACheckBoxOS::resize()
  {
  set_area(AVec2i(
    m_font.get_width(get_title()) + ACheckBox_width_extra,
    m_font.get_height() + ACheckBox_height_extra));
  }

//---------------------------------------------------------------------------------------
//  Sets the on_toggled event function
// Arg          toggled_func_p -  dynamically allocated on_toggled event function object
//              to store.  It will be deleted when this object is destructed or when
//              another on_toggled event function object is set.
// Author(s):    Conan Reis
void ACheckBoxOS::set_on_toggled_func(
  AFunctionArgBase<eAFlag> * toggled_func_p,
  bool                              free_toggle_func // = true
  )
  {
  if (toggled_func_p != m_toggled_func_p)
    {
    if (m_free_toggle_func)
      {
      delete m_toggled_func_p ;
      }
    m_toggled_func_p   = toggled_func_p;
    m_free_toggle_func = free_toggle_func;
    }
  }

//---------------------------------------------------------------------------------------
//  Enables the showing of text on the button face (if it is not already
//              doing so), sets the text to display, specifies the text alignment and
//              whether the text should be word wrapped according to the current size of
//              the button - AND it optionally resizes the button to fit the given text.
// Arg          text - text to display.  If this is an empty AString, text displaying is
//              disabled and the remaining arguments are ignored.
// Arg          resize_to_text - if true resizes the button to fit the specified text
//              Note that word wrap is ignored if this is set to true.  (Default false)
// Arg          horizontal - the horizontal alignment of the text.  One of AHorizAlign_left,
//              AHorizAlign_right, or AHorizAlign_centered.  Note that AHorizAlign_justified is not a valid alignment
//              for a AgogGUI_OS\ACheckBoxOS.  (Default AHorizAlign_centered)
// Arg          vertical - the vertical alignment of the text.  One of AVertAlign_top, AVertAlign_bottom,
//              or AVertAlign_centered.  (Default AVertAlign_centered)
// Arg          word_wrap - specifies whether the text should be word wrapped to the
//              current size of the AgogGUI_OS\ACheckBoxOS.  This value is ignored if resize_to_text
//              is set to true.  (Default false)
// See:         resize(), set_title(), set_font(), 
// Notes:       If the button is already in display mode set_title() will change the text
//              and use all of the current text settings.
// Author(s):    Conan Reis
void ACheckBoxOS::set_text(
  const AString & text,
  bool            resize_to_text, // = false
  eAHorizAlign    horizontal,     // = AHorizAlign_centered
  eAVertAlign     vertical,       // = AVertAlign_centered
  bool            word_wrap       // = false
  )
  {
  if (!text.is_empty())
    {
    uint32_t style_bits = BS_TEXT;

    style_bits |= (horizontal == AHorizAlign_left) ? BS_LEFT : ((horizontal == AHorizAlign_right) ? BS_RIGHT : BS_CENTER);
    style_bits |= (vertical == AVertAlign_centered) ? BS_VCENTER : ((vertical == AVertAlign_top) ? BS_TOP : BS_BOTTOM);
    if (word_wrap)
      {
      style_bits |= BS_MULTILINE;
      }

    if (resize_to_text)
      {
      set_area(AVec2i(
        m_font.get_width(text) + ACheckBox_width_extra,
        m_font.get_height() + ACheckBox_height_extra));
      }
    modify_style(style_bits);
    set_title(text);
    }
  else
    {
    // BM_SETSTYLE
    remove_style(BS_TEXT);
    set_title(text);
    }
  }

//---------------------------------------------------------------------------------------
//  Determines if the button is currently toggled.
// Returns:     true if button state is changed, false if button was already in the state
//              requested
// See:         toggle(), on_toggled()
// Author(s):    Conan Reis
bool ACheckBoxOS::set_toggle_state(
  eAFlag new_state // = AFlag_on
  )
  {
  if (get_toggle_state() != new_state)
    {
    ::SendMessage(m_os_handle, BM_SETCHECK, (new_state == AFlag_on) ? BST_CHECKED : ((new_state == AFlag_off) ? BST_UNCHECKED : BST_INDETERMINATE), 0u);

    return true;
    }
  return false;
  }

//---------------------------------------------------------------------------------------
//  Simulates a toggle.  Cycles through the possible toggle states.
// See:         get_toggle_state(), set_toggle_state(), on_toggled()
// Author(s):    Conan Reis
void ACheckBoxOS::toggle()
  {
  set_focus();
  ::SendMessage(m_os_handle, BM_CLICK, 0u, 0u);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Called whenever the Edit standard control sends a notification message.
// Returns:     a boolean indicating whether the default MS Windows API process with
//              respect to the given message should be called (true) or not (false).
// Arg          uint32_t code - message code to parse
// Author(s):    Conan Reis
bool ACheckBoxOS::on_control_event_standard(uint32_t code)
  {
  if (code == BN_CLICKED)
    {
    on_toggled(get_toggle_state());
    }
  //else
  //  {
  //  A_DPRINT("%ACheckBoxOS::on_control_event_standard(0x%x)\n", A_SOURCE_STR, code);
  //  }

  /*
  switch (code)
    {
    case BN_DOUBLECLICKED:
    case BN_CLICKED:
      on_toggled(get_toggle_state());
      break;

    // These are only sent if the BS_NOTIFY style is set
    case BN_SETFOCUS:
      on_focus();
      break;

    case BN_KILLFOCUS:
      on_focus_lost();
      break;

    default:
      ; //A_DPRINT("%ACheckBoxOS::on_control_event_standard(0x%x)\n", A_SOURCE_STR, code);
    }
  */
  return true;  // invoke default behaviour
  }

//---------------------------------------------------------------------------------------
//  Called whenever the toggle state of this check box is changed
// See:         set_on_toggled_func(), set_toggle_state(), toggle(), get_toggle_state()
// Notes:       Override this event method for custom behavior.
// Modifiers:    virtual
// Author(s):    Conan Reis
void ACheckBoxOS::on_toggled(eAFlag new_state)
  {
  //A_DPRINT("ACheckBoxOS::on_toggled(%i)\n", new_state);
  if (m_toggled_func_p)
    {
    m_toggled_func_p->invoke(new_state);
    }
  }

