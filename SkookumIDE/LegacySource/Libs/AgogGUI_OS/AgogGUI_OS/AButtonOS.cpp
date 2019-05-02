// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AButtonOS class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AButtonOS.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================


//=======================================================================================
// Local Global Structures
//=======================================================================================

// AButtonOS enumerated constants
enum
  {
  AButton_width_multiple = 3,
  AButton_width_extra    = 8,
  AButton_height_extra   = 4
  };

//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * AButtonOS::ms_default_class_p;

//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AButtonOS::initialize()
  {
  ms_default_class_p = new AMessageTargetClass("button");
  }

//---------------------------------------------------------------------------------------

void AButtonOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
//  Text Faced Constructor
// Returns:     itself
// Arg          parent_p - a pointer parent AWindow to this object.  This *MUST* be
//              given and cannot be nullptr.  The set_parent() method does not work properly
//              with OS graphical controls.
// Arg          text - text to place on button face (Default "OK")
// Arg          font - font to use for text (Default AFont::ms_default - Arial point 10)
// Arg          region - position and area of the button.  If width set to Size_auto,
//              it uses the width of the supplied text using this button's initial font.
//              If height set to Size_auto, it uses the height of this button's initial
//              font.  (Default Region(0, 0, Size_auto, Size_auto))
// Notes:       Initial settings (modified with appropriately matching methods):
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
AButtonOS::AButtonOS(
  AWindow *       parent_p,
  const AString & text,  // = AString(ABUTTON_TEXT_DEFAULT)
  const AFont &   font,  // = AFont::ms_default
  const ARegion & region // = AWindow::ms_region_auto_def
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_pressed_func_p(nullptr)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    text.as_cstr(),         // title
    WS_CHILD | WS_TABSTOP | BS_CENTER | BS_VCENTER, // Window & AButtonOS Styles
    region.m_x,
    region.m_y,
    (region.m_width == Size_auto) ? m_font.get_width(text) + AButton_width_extra : region.m_width,
    (region.m_height == Size_auto) ? m_font.get_height() + AButton_height_extra : region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "AButtonOS()", AButtonOS);

  m_font = font;
  enable_subclass_messages();
  common_setup();
  }

//---------------------------------------------------------------------------------------
//  Constructor
// Returns:     itself
// Arg          parent_p - a pointer parent AWindow to this object.  This *MUST* be
//              given and cannot be nullptr.  The set_parent() method does not work properly
//              with OS graphical controls.
// Arg          region - position and area of the button.  If width set to Size_auto,
//              it uses the height of this button's initial font * 3 for the width.  If
//              height set to Size_auto, it uses the height of this button's initial font.
//              (Default Region(0, 0, Size_auto, Size_auto))
// Notes:       Initial settings (modified with appropriately matching methods):
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
AButtonOS::AButtonOS(
  AWindow *       parent_p,
  const ARegion & region // = AWindow::ms_region_auto_def
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_pressed_func_p(nullptr)
  {
  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    0,                      // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "",                     // title
    WS_CHILD | WS_TABSTOP | BS_CENTER | BS_VCENTER,  // Window & AButtonOS Styles
    region.m_x,
    region.m_y,
    (region.m_width == Size_auto) ? m_font.get_height() * AButton_width_multiple : region.m_width,
    (region.m_height == Size_auto) ? m_font.get_height() + AButton_height_extra : region.m_height,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "AButtonOS()", AButtonOS);

  enable_subclass_messages();
  common_setup();
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Examples:    called by system
// Author(s):    Conan Reis
AButtonOS::~AButtonOS()
  {
  delete m_pressed_func_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// General Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Enables or disables 3D shading of the button face.
// Arg          shade_3d - if true, the button is shaded to give it a 3D appearance, if
//              false, it has a 'flat' look.  (Default true)
// See:         enable_default_border()
// Author(s):    Conan Reis
void AButtonOS::enable_3d_shading(
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
//  Enables or disables a heavy black border around the button. The user can
//              select this button by pressing the ENTER key.  This style enables the user
//              to quickly select the most likely option (the default option).
// Arg          shade_3d - if true, the button is shaded to give it a 3D appearance, if
//              false, it has a 'flat' look.  (Default true)
// See:         enable_3d_shading()
// Author(s):    Conan Reis
void AButtonOS::enable_default_border(
  bool default_button // = true
  )
  {
  if (default_button)
    {
    append_style(BS_DEFPUSHBUTTON);
    }
  else
    {
    remove_style(BS_DEFPUSHBUTTON);
    }
  }

//---------------------------------------------------------------------------------------
//  Determines if the button is currently pressed.
// Returns:     true if button is currently pressed, false if not
// See:         press(), on_pressed()
// Author(s):    Conan Reis
bool AButtonOS::is_pressed() const
  {
  return ((::SendMessage(m_os_handle, BM_GETSTATE, 0u, 0u) & BST_PUSHED) != 0u);
  }

//---------------------------------------------------------------------------------------
//  Simulates a button press.
// See:         is_pressed(), on_pressed()
// Author(s):    Conan Reis
void AButtonOS::press()
  {
  set_focus();
  ::SendMessage(m_os_handle, BM_CLICK, 0u, 0u);
  }

//---------------------------------------------------------------------------------------
//  Resizes the button to fit the current text displayed on the button face.
// See:         set_text(), set_font(), set_title()
// Author(s):    Conan Reis
void AButtonOS::resize()
  {
  set_area(AVec2i(
    m_font.get_width(get_title()) + AButton_width_extra,
    m_font.get_height() + AButton_height_extra));
  }

//---------------------------------------------------------------------------------------
//  Sets the on_pressed event function
// Arg          pressed_func_p -  dynamically allocated on_pressed event function object
//              to store.  It will be deleted when this object is destructed or when
//              another on_pressed event function object is set.
// Author(s):    Conan Reis
void AButtonOS::set_on_pressed_func(AFunctionBase * pressed_func_p)
  {
  if (pressed_func_p != m_pressed_func_p)
    {
    delete m_pressed_func_p ;
    m_pressed_func_p = pressed_func_p;
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
//              for a AgogGUI_OS\AButtonOS.  (Default AHorizAlign_centered)
// Arg          vertical - the vertical alignment of the text.  One of AVertAlign_top, AVertAlign_bottom,
//              or AVertAlign_centered.  (Default AVertAlign_centered)
// Arg          word_wrap - specifies whether the text should be word wrapped to the
//              current size of the AgogGUI_OS\AButtonOS.  This value is ignored if resize_to_text
//              is set to true.  (Default false)
// See:         resize(), set_title(), set_font(), 
// Notes:       If the button is already in display mode set_title() will change the text
//              and use all of the current text settings.
// Author(s):    Conan Reis
void AButtonOS::set_text(
  const AString & text,
  bool            resize_to_text, // = false
  eAHorizAlign    horizontal,     // = AHorizAlign_centered
  eAVertAlign     vertical,       // = AVertAlign_centered
  bool            word_wrap       // = false
  )
  {
  if (!text.is_empty())
    {
    uint32_t style_bits = 0u;

    style_bits |= (horizontal == AHorizAlign_left) ? BS_LEFT : ((horizontal == AHorizAlign_right) ? BS_RIGHT : BS_CENTER);
    style_bits |= (vertical == AVertAlign_centered) ? BS_VCENTER : ((vertical == AVertAlign_top) ? BS_TOP : BS_BOTTOM);
    if (word_wrap)
      {
      style_bits |= BS_MULTILINE;
      }

    if (resize_to_text)
      {
      set_area(AVec2i(
        m_font.get_width(text) + AButton_width_extra,
        m_font.get_height() + AButton_height_extra));
      }
    modify_style(style_bits);
    set_title(text);
    }
  else
    {
    // BM_SETSTYLE
    set_title(text);
    }
  }

// --------------------------------------------------------------------------------------
// Load the BMP and set it as the button's appearance
// Arg         filename - BMP file to load
// Arg         autoSizeRegion - if true resizes the button to fit the specified image
// Author(s):   Ken Kavanagh
void AButtonOS::set_image(const AString & filename, bool autoSizeRegion)
  {
  HANDLE bmp_handle = ::LoadImage(nullptr, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

  if (bmp_handle)
    {
    if (autoSizeRegion)
      {
      // fix the btn's region to match the images dimensions
      BITMAP bmp;

      if (::GetObject(bmp_handle, sizeof(BITMAP), (LPSTR)&bmp)) 
        {
        //add to the width and height - it's a magic number based on what it winds up looking like. no logic here at all
        AVec2i area(bmp.bmWidth + 4, bmp.bmHeight + 4);

        set_region(area);
        }
      }

    // send the magic message that applies the loaded bmp to the button
    ::SendMessage(m_os_handle, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp_handle);
    append_style(BS_BITMAP);
    }
  else
    {
    A_ERROR_OS(a_str_format("LoadImage(\"%s\") failed!", filename.as_cstr()), AButtonOS);
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Event Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Called whenever the Edit standard control sends a notification message.
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
// Arg         uint32_t code - message code to parse
// Author(s):   Conan Reis
bool AButtonOS::on_control_event_standard(uint32_t code)
  {
  if (code == BN_CLICKED)
    {
    on_pressed();
    }
  /*
  switch (code)
    {
    case BN_DOUBLECLICKED:
    case BN_CLICKED:
      on_pressed();
      break;

    // These are only sent if the BS_NOTIFY style is set
    case BN_SETFOCUS:
      on_focus();
      break;

    case BN_KILLFOCUS:
      on_focus_lost();
      break;

    default:
      ; //A_DPRINT("%AButtonOS::on_control_event_standard(0x%x)\n", A_SOURCE_STR, code);
    }
  */
  return true;  // invoke default behaviour
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AButtonOS::on_pressed()
  {
  if (m_pressed_func_p)
    {
    m_pressed_func_p->invoke();
    }
  }

//---------------------------------------------------------------------------------------
// Notes:       The sequence of behaviour to trigger this event is as follows:
//                1. left mouse button down
//                2. left mouse button up                 - on_pressed()
//                3. left mouse button down (soon after)  - on_pressed_double()
//
//              Note that a final mouse button release is not utilized.
// Author(s):    Conan Reis
//void AButtonOS::on_pressed_double()
//  {
//  }
