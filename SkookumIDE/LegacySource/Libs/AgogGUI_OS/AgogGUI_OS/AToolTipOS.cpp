// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AToolTipOS class declaration header
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AToolTipOS.hpp>
#include <AgogGUI_OS\ARichEditOS.hpp>
#include <AgogGUI\ATrueTypeFont.hpp>
#include <AgogIO\AApplication.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>  // Uses ShellExecute()
#include <Commctrl.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // Enumerated constants
  enum
    {
    AToolTip_initial_word_break_avg_char = 60,
    AToolTip_milliseconds_per_char_avg   = 70
    };

} // End unnamed namespace

//=======================================================================================
// Class Data
//=======================================================================================

AMessageTargetClass * AToolTipOS::ms_default_class_p;

//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------

void AToolTipOS::initialize()
  {
  ms_default_class_p = new AMessageTargetClass(TOOLTIPS_CLASS);
  }

//---------------------------------------------------------------------------------------

void AToolTipOS::deinitialize()
  {
  delete ms_default_class_p;
  }

//---------------------------------------------------------------------------------------
// Constructor
// 
// Params:  
//   parent_p:
//     The window to supply a tool tip.  This *MUST* be given and cannot be nullptr.
//   text: Text to place in the tip
//   font: font to use for text
//
// Author(s):    Conan Reis
AToolTipOS::AToolTipOS(
  AWindow *       parent_p,
  const AString & text,
  bool            enabled,            //  = true
  int             delay_reshow,       //  = -1;
  const AFont   & font,               //  = AFont::ms_narrow
  ARichEditOS   * parent_rich_edit_p  //  = nullptr
  ) :
  AWindow(parent_p, ms_default_class_p),
  m_delay_reshow(delay_reshow),
  m_parent_rich_edit_p(parent_rich_edit_p),
  m_enabled(enabled)
  {
  ZeroMemory(m_link_func, sizeof(m_link_func));

  // Note, for each of the styles there is either an associated method or style is set
  // according to the characteristics of the AMessageTarget
  // Also note that styles do not apply to non-graphical AMessageTarget objects

  m_os_handle = ::CreateWindowEx(
    WS_EX_TOPMOST,          // Extended Window Styles
    m_class_p->get_name(),  // Must use  the name of the class for predefined classes
    "Tip Title",            // title
    WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,  // TTS_USEVISUALSTYLE TTS_CLOSE
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    m_parent_handle, 
    nullptr,                   // Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                  // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "AToolTipOS()", AToolTipOS);

  m_font = font;
  enable_subclass_messages();
  common_setup();

  ::SetWindowPos(m_os_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  TOOLINFO info = { 0 };
  info.cbSize   = sizeof(info);
  info.hwnd     = m_parent_handle;
  info.uFlags   = TTF_IDISHWND | TTF_PARSELINKS | TTF_SUBCLASS | TTF_TRANSPARENT;
  info.uId      = (UINT_PTR)m_parent_handle;
  info.lpszText = (LPSTR)text.as_cstr();
  ::SendMessage(m_os_handle, TTM_ADDTOOL, 0, (LPARAM)&info);

  set_width_word_break(AToolTip_initial_word_break_avg_char * m_font.get_avg_width());

  // Adjust delay time based on length of text
  ::SendMessage(m_os_handle, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM(text.get_length() * AToolTip_milliseconds_per_char_avg, 0u));
  ::SendMessage(m_os_handle, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELPARAM(m_delay_reshow, 0u));

  enable_activate(m_enabled);
  }

//---------------------------------------------------------------------------------------
// Destructor
// 
// Examples:    called by system
// Author(s):   Conan Reis
AToolTipOS::~AToolTipOS()
  {
  }

//---------------------------------------------------------------------------------------
// Enables or disables tip activation. If disabled, tip is never shown - even with a call
// to `show*()`.
// 
// Notes:       Tips are activated by default.
// Author(s):   Conan Reis
void AToolTipOS::enable_activate(
  bool activate // = true
  )
  {
  ::SendMessage(m_os_handle, TTM_ACTIVATE, WPARAM(activate), 0u);
  }

//---------------------------------------------------------------------------------------
// Remove tip pop-up from view.
// 
// See:         show_at_mouse(), show(), pop()
// Author(s):   Conan Reis
void AToolTipOS::pop()
  {
  ::SendMessage(m_os_handle, TTM_POP, 0u, 0u);
  }

//---------------------------------------------------------------------------------------
// Sets the header/title of the tip including optional icon.
// 
// Params:  
//   header:
//     Text to use for header/title. It must not exceed 100 characters.
//     No header is displayed if string is empty.
//   icon:
//     Icon to use - either one of AToolTipOS::eIcon such as AToolTipOS::Icon_info or
//     IDI_APPLICATION ... IDI_WINLOGO or specify an icon resource id.
// 
// Example:  
//   tip.set_header("Header text", IDI_SKOOKUM);
// 
// See:         set_text()
// Author(s):   Conan Reis
void AToolTipOS::set_header(
  const AString & header,
  LONG_PTR        icon_resource_id // = Icon_none
  )
  {
  HICON icon_handle    = HICON(icon_resource_id);
  bool  use_predefined = true;
  
  if (icon_resource_id >= Icon__last)
    {
    use_predefined = a_is_ordered(LONG_PTR(IDI_APPLICATION), icon_resource_id, LONG_PTR(IDI_SHIELD));
    icon_handle    = ::LoadIcon(
      use_predefined ? nullptr : AApplication::ms_res_instance,
      MAKEINTRESOURCE(icon_resource_id));
    A_VERIFY_OS(icon_handle != nullptr, "set_header()", AToolTipOS);
    }

  ::SendMessage(m_os_handle, TTM_SETTITLE, WPARAM(icon_handle), LPARAM(header.as_cstr()));

  if (!use_predefined)
    {
    ::DestroyIcon(icon_handle);
    }
  }

//---------------------------------------------------------------------------------------
// Sets the link URL/file to open if a link in the tip is clicked. If it is empty then
// the associated parent window's on_tip_link() will be called.
// 
// Params:  
//   text: URL/file to open or empty if parent window's on_tip_link() should be called.
// 
// See:         on_tip_link()
// Author(s):   Conan Reis
void AToolTipOS::set_link(const AString & link_url)
  {
  m_link_url = link_url;
  }

//---------------------------------------------------------------------------------------
// Sets the text of the tip.
// 
// Params:  
//   text: text to use for tip
// 
// See:         set_width_word_break()
// Author(s):   Conan Reis
void AToolTipOS::set_text(const AString & text)
  {
  // $Revisit - CReis Could use empty text to indicate that 
  TOOLINFO info = { 0 };
  info.cbSize   = sizeof(info);
  info.hwnd     = m_parent_handle;
  info.uId      = (UINT_PTR)m_parent_handle;
  info.lpszText = (LPSTR)text.as_cstr();

  ::SendMessage(m_os_handle, TTM_UPDATETIPTEXT, 0, (LPARAM)&info);

  // Adjust delay time based on length of text
  ::SendMessage(m_os_handle, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM(text.get_length() * AToolTip_milliseconds_per_char_avg, 0u));
  }

//---------------------------------------------------------------------------------------
// Sets the width where to break the text into multiple lines, using spaces to determine
// line breaks. If the text cannot be segmented into multiple lines, it is displayed on a
// single line, which may exceed the maximum tooltip width.
// 
// Params:  
//   width: word break width or -1 to allow any width
// 
// Notes:       Tips word break at `AToolTip_initial_word_break_avg_char` by default.
// Author(s):   Conan Reis
void AToolTipOS::set_width_word_break(int width)
  {
  ::SendMessage(m_os_handle, TTM_SETMAXTIPWIDTH, 0u, LPARAM(width));
  }

//---------------------------------------------------------------------------------------
// Shows the tip at the last mouse position.
// 
// Notes:  
//   The tip is normally automatically shown by the system on the first mouse hover on the
//   associated window.
//   `show()` will also show the tip but at the last tip position.
//   
// See:         show(), pop()
// Author(s):   Conan Reis
void AToolTipOS::show_at_mouse()
  {
  ::SendMessage(m_os_handle, TTM_POPUP, 0u, 0u);
  }

//---------------------------------------------------------------------------------------
// Creates a dynamically allocated tool tip attached to parent_p. Used to indirectly
// create tool tips with `AWindow::set_tool_tip_create_func()` since `AWindow` is in the
// AgogGUI library and `AToolTipOS` is in the AgogGUI_OS library.
// 
// Params:      same as constructor
// Author(s):   Conan Reis
AToolTipOS * AToolTipOS::create_attached(
  AWindow *       parent_p,
  const AString & text,
  const AFont &   font
  )
  {
  AToolTipOS * tip_p = new AToolTipOS(parent_p, text, true, -1, font);
  
  parent_p->set_tool_tip(tip_p);

  return tip_p;
  }


//---------------------------------------------------------------------------------------
//  Sets or clears a link function.
// 
//  Returns:      -1 on an error, otherwise the number of link functions defined.
//  See:          AToolTipOS::on_tip_link()
//  Author(s):    John Stenersen
int AToolTipOS::set_link_funct(uint link_func_index, void (*link_func)(AToolTipOS * tooltip_p))
  {
  if (link_func_index >= ms_link_func_max) return -1;
  if (m_link_func[link_func_index]) m_link_func_count --;
  if (link_func) m_link_func_count ++;
  m_link_func[link_func_index] = link_func;

  return m_link_func_count;

  } //  AToolTipOS::set_link_funct()


//---------------------------------------------------------------------------------------
//  Outputs a meesage to the Log that looks like a remotely generated message.
// 
//  Author(s):    John Stenersen
void AToolTipOS::on_link_disable(AToolTipOS * tooltip_p)
  {
  tooltip_p->m_enabled = false;
  tooltip_p->enable_activate(false);

  if (tooltip_p->m_parent_rich_edit_p)
    {
    tooltip_p->m_parent_rich_edit_p->set_focus();
    }

  //  TBD: Set the ini file.
  }   //  AToolTipOS::on_link_disable()


//---------------------------------------------------------------------------------------
//  Outputs a meesage to the Log that looks like a remotely generated message.
// 
//  Author(s):    John Stenersen
void AToolTipOS::on_link_remote_message_example(AToolTipOS * tooltip_p)
  {
  if (tooltip_p->m_parent_rich_edit_p)
    {
    AString message("This is an example of a output message from the remote Application runtime.\n");
    ATextStyle  style;
    AColor      color( 0.53f, 0.73f, 1.0f,  1.0f);  //  electric blue

    style.m_effect_mask  = AText__all;
    style.m_font_color_p = & color;

    tooltip_p->m_parent_rich_edit_p->append_style(message, style);
    }
  } //  AToolTipOS::on_link_remote_message_example()


//---------------------------------------------------------------------------------------
//  Outputs a meesage to the Log that looks like a locally generated message.
// 
//  Author(s):    John Stenersen
void AToolTipOS::on_link_local_message_example(AToolTipOS * tooltip_p)
  {
  if (tooltip_p->m_parent_rich_edit_p)
    {
    AString message("This is an example of a output message from the local IDE.\n");
    ATextStyle  style;
    AColor      color( 0.53f, 0.73f, 1.0f,  1.0f);  //  electric blue

    style.m_effect_mask  = AText__all;
    style.m_font_color_p = & color;
    style.m_effect_flags = AText_italics;

    tooltip_p->m_parent_rich_edit_p->append_style(message, style);
    }
  } //  AToolTipOS::on_link_local_message_example()


//---------------------------------------------------------------------------------------
// Called when link is selected.
// 
// See:         set_link(), AWindow::on_tooltip_link()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AToolTipOS::on_tooltip_link()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " Link clicked - 0x%p\n", this);

  POINT mouse_pos;
  ::GetCursorPos(&mouse_pos);
  ::ScreenToClient(this->m_os_handle, &mouse_pos);
//  A_DPRINT("mouse_pos = (%ld, %ld), font height = %ld\n", mouse_pos.x, mouse_pos.y, m_font.get_height());

  int func_index = ( mouse_pos.y - ms_link_y_offset ) / m_font.get_height();

  if (m_link_func_count && (func_index >= 0) && (func_index < ms_link_func_max) && m_link_func[ func_index ])
      {
      (* m_link_func[ func_index ])(this);
      //  Note: Up to the funct to close the tip or not.
      return;
      }

  if (m_link_url.is_filled())
    {
    ::ShellExecute(nullptr, "open", m_link_url.as_cstr(), nullptr, nullptr, SW_SHOWNORMAL);
    }
  else
    {
    static_cast<AWindow *>(m_parent_p)->on_tooltip_link();
    }

  // Close tip
  pop();
  } //  AToolTipOS::on_tip_link()


//---------------------------------------------------------------------------------------
// Called whenever an OS common control sends a notification message.
// 
// Returns:  
//   a boolean indicating whether the default MS Windows API process with respect to the
//   given message should be called `true` or not `false`.
//   
// Params:    
//   code:     message code to parse
//   result_p: pointer to store return info for message
//   
// Notes:  
//   This method should be overridden by OS common control objects and then parse out any
//   appropriate messages. For a list of OS standard /system controls see the "Notes"
//   section of the `AMessageTargetClass(os_class_name)` constructor.
//   
// Modifiers:   virtual - Overridden for custom behaviour.
// Author(s):   Conan Reis
bool AToolTipOS::on_control_event(
  NMHDR *   info_p,
  LRESULT * result_p
  )
  {
  switch (info_p->code)
    {
    case TTN_SHOW:
      //A_DPRINT(A_SOURCE_STR " Tip pre-show - 0x%p\n", this);
      // To display tip in its default location, return zero. To customize the tip
      // position, reposition the tip window with SetWindowPos() and return TRUE. 
      *result_p = 0u;
      break;

    case TTN_LINKCLICK:
      on_tooltip_link();
      break;
    }

  return true;  // invoke default behaviour
  }

