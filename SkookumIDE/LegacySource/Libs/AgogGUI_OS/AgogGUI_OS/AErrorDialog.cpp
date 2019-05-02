// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AErrorDialog class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AErrorDialog.hpp>
#include <AgogGUI_OS\AEditOS.hpp>
#include <AgogGUI_OS\AButtonOS.hpp>
#include <AgogGUI_OS\ACheckBoxOS.hpp>
#include <AgogGUI\ADisplay.hpp>
#include <AgogCore\AMethod.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

// Using unnamed namespace to prevent pollution of global namespace
namespace 
{
    // Enumerated constants
    enum
      {
      AErrDlg__none      = 0x00,
      AErrDlg_debug_ui   = 0x01,
      AErrDlg_ignore_ui  = 0x02,
      AErrDlg_recover_ui = 0x04,
      AErrDlg_quit_ui    = 0x08,

      AErrDlg_no_choice        = 64, 
      AErrDlg_print_char_extra = 2048
      };

    const int AErrDlg_start_y        = 20;
    const int AErrDlg_alert_width    = 750;
    const int AErrDlg_alert_height   = 400;
    const f32 AErrDlg_button_stretch = 1.5f;
    const f32 AErrDlg_button_spacing = 1.5f;


    class AErrorWin : public AWindow
      {
      public:

      // Methods
      
        AErrorWin(const AString & desc, const char * ignore_text_p, const AFont & font, uint32_t ui_flags);

        bool determine_choice(eAErrAction * action_p);

        void on_btn_continue()    { m_choice = AErrAction_continue; }
        void on_btn_retry()       { m_choice = AErrAction_retry; }
        void on_btn_quit()        { m_choice = AErrAction_quit; }
        void on_btn_ignore()      { m_choice = AErrAction_ignore; }
        void on_btn_ignore_all()  { m_choice = AErrAction_ignore_all; }
        void on_btn_break_quit()  { m_choice = AErrAction_quit; m_btn_break.enable_toggle(); }

        virtual void on_sizing();

      // Data Members

        uint32_t    m_ui_flags;
        int         m_choice;
        AEditOS     m_desc;
        AButtonOS   m_btn_continue;
        AButtonOS   m_btn_retry;
        AButtonOS   m_btn_quit;
        AButtonOS   m_btn_ignore;
        AButtonOS   m_btn_ignore_all;
        AButtonOS   m_btn_break_quit;
        ACheckBoxOS m_btn_break;

      };

}  // End unnamed namespace

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AErrorWin::AErrorWin(
  const AString & desc,
  const char *    ignore_text_p,
  const AFont &   font,
  uint32_t        ui_flags
  ) :
  m_ui_flags(ui_flags),
  m_choice(AErrDlg_no_choice),
  m_desc(this, desc, true, AFont("Arial", 10.0f)),
  m_btn_continue(this, "Continue", font),
  m_btn_retry(this, "Retry", font),
  m_btn_quit(this, "Quit", font),
  m_btn_ignore(this, ignore_text_p, font),
  m_btn_ignore_all(this, "Ignore All", font),
  m_btn_break_quit(this, "Break && Quit", font),
  m_btn_break(this, "Break After", ACheckType_2_state, font)
  {
  // $Revisit - CReis The components of this dialog should be tabbable and allow enter as a default (WS_EX_CONTROLPARENT)

  // Setup main error window
  set_font(font);
  enable_sizing();
  enable_title_bar();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int spacing       = get_spacing();
  int button_width  = m_btn_break_quit.get_width() + spacing;  // Largest button text plus a bit more
  int button_height = m_btn_break_quit.get_height() + spacing;

  // Setup description edit box
  m_desc.set_border(Border_thin_sunken);
  m_desc.enable_read_only();
  m_desc.set_position(spacing, spacing);
  m_desc.show();
    
  // Show appropriate buttons
  bool recover_ui = false;

  if (ui_flags & AErrDlg_recover_ui)
    {
    recover_ui = true;
    m_btn_continue.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_continue));
    m_btn_continue.enable_default_border();
    m_btn_continue.set_area(button_width, button_height);
    m_btn_continue.show();

    m_btn_retry.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_retry));
    m_btn_retry.set_area(button_width, button_height);
    m_btn_retry.show();
    }

  if (ui_flags & AErrDlg_quit_ui)
    {
    m_btn_quit.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_quit));
    m_btn_quit.enable_default_border(!recover_ui);
    m_btn_quit.set_area(button_width, button_height);
    m_btn_quit.show();
    }

  if (ui_flags & AErrDlg_ignore_ui)
    {
    m_btn_ignore.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_ignore));
    m_btn_ignore.set_area(button_width, button_height);
    m_btn_ignore.show();

    m_btn_ignore_all.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_ignore_all));
    m_btn_ignore_all.set_area(button_width, button_height);
    m_btn_ignore_all.show();
    }

  m_btn_break_quit.set_area(button_width, button_height);

  if (ui_flags & AErrDlg_debug_ui)
    {
    //m_btn_break_quit.set_on_pressed_func(new AMethod<AErrorWin>(this, &AErrorWin::on_btn_break_quit));
    //m_btn_break_quit.show();

    m_btn_break.set_area(button_width, button_height);
    m_btn_break.show();
    }

  set_region(ADisplay::center(AErrDlg_alert_width, AErrDlg_alert_height));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool AErrorWin::determine_choice(eAErrAction * action_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Disable all other windows on the same thread, show the alert window, give
  // alert beep, and flash the alert window title.
  //enable_input_all(false, m_os_handle);
  show();
  make_foreground();
  flash_title();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wait until choice is made
  while (m_choice == AErrDlg_no_choice)
    {
    // Check for messages
    // $Revisit - CReis Change to *wait* for messages
    AMessageTarget::process_messages(AAsyncFilter__none);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean-up
  //enable_input_all(true, m_os_handle);
  hide();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return choice

  *action_p = eAErrAction(m_choice);

  return (m_ui_flags & AErrDlg_debug_ui) && m_btn_break.is_toggled();
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is
//             associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sized()
// Notes:      This applies to the window's client area size changing and not
//             necessarily the outer edge of the window.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void AErrorWin::on_sizing()
  {
  AVec2i carea(get_area_client());
  int    spacing       = get_spacing();
  int    small_spacing = get_spacing() / 3;
  int    button_width  = m_btn_break_quit.get_width();
  int    button_height = m_btn_break_quit.get_height();
  int    x_pos         = carea.m_x - (button_width + spacing);
  int    y_pos         = spacing;

  m_desc.set_area(x_pos - (2 * spacing), carea.m_y - (2 * spacing));
    
  bool recover_ui = false;

  if (m_ui_flags & AErrDlg_recover_ui)
    {
    recover_ui = true;
    m_btn_continue.set_position(x_pos, y_pos);
    y_pos += button_height + small_spacing;

    m_btn_retry.set_position(x_pos, y_pos);
    y_pos += button_height + small_spacing;
    }

  if (m_ui_flags & AErrDlg_quit_ui)
    {
    m_btn_quit.set_position(x_pos, y_pos);
    y_pos += button_height + small_spacing;
    }

  if (m_ui_flags & AErrDlg_ignore_ui)
    {
    if (y_pos > spacing)
      {
      y_pos += spacing;
      }

    m_btn_ignore.set_position(x_pos, y_pos);
    y_pos += button_height + small_spacing;

    m_btn_ignore_all.set_position(x_pos, y_pos);
    y_pos += button_height + small_spacing;
    }

  if (m_ui_flags & AErrDlg_debug_ui)
    {
    //m_btn_break_quit.set_position(x_pos, y_pos);
    //y_pos += button_height + small_spacing;

    y_pos += spacing * 2;
    m_btn_break.set_position(x_pos, y_pos);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that controls are redrawn
  refresh();
  }


//=======================================================================================
// Method Definitions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Determines which choice to take by prompting the user.  It also writes
//             out information to the IDE Output Window.
//             $Revisit - CReis It should probably also log to a file
// Returns:    true if a user break should be made in the debugger, false if not
// Arg         msg - See the definition of AErrMsg in ADebug.h for more information.
// Arg         action_p - address to store chosen course of action to take to resolve error
// Modifiers:   virtual from AErrorOutputBase
// Author(s):   Conan Reis
bool AErrorDialog::determine_choice(
  const AErrMsg & msg,
  eAErrAction *   action_p
  )
  {
  const char * title_p;
  const char * ignore_text_p = "Ignore";
  int          warn_level    = MB_ICONERROR;
  int32_t      icon_id       = int32_t(intptr_t(IDI_WARNING));  // Exclamation point icon
  // $Revisit - CReis Always add extra debug info for now.
  uint32_t     ui_flags      = AErrDlg_debug_ui;  //ADebug::is_debugging() ? AErrDlg_debug_ui : AErrDlg__none;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set pop-up attributes and default values
  switch (msg.m_err_level)
    {
    case AErrLevel_internal:
      title_p = (msg.m_title_p ? msg.m_title_p : "Internal Exception");
      icon_id = int32_t(intptr_t(IDI_INFORMATION));  // Asterisk icon
      break;

    case AErrLevel_notify:
      title_p        = (msg.m_title_p ? msg.m_title_p : "Notification / Warning");
      ignore_text_p  = "Ok / Ignore";
      warn_level     = MB_ICONWARNING;
      ui_flags      |= (AErrDlg_ignore_ui | AErrDlg_quit_ui);
      break;

    case AErrLevel_warning:
      title_p     = (msg.m_title_p ? msg.m_title_p : "Warning");
      warn_level  = MB_ICONWARNING;
      #if defined(A_THROW_EXCEPTION)
        ui_flags |= (AErrDlg_recover_ui | AErrDlg_quit_ui);
      #else
        ui_flags |= (AErrDlg_ignore_ui | AErrDlg_quit_ui);
      #endif 
      break;

    case AErrLevel_error:
      title_p     = (msg.m_title_p ? msg.m_title_p : "Error");
      warn_level  = MB_ICONWARNING;
      #if defined(A_THROW_EXCEPTION)
        ui_flags |= (AErrDlg_ignore_ui | AErrDlg_recover_ui | AErrDlg_quit_ui);
      #else
        ui_flags |= (AErrDlg_ignore_ui | AErrDlg_quit_ui);
      #endif 
      break;

    default:  // AErrLevel_fatal
      title_p     = (msg.m_title_p ? msg.m_title_p : "Fatal Error");
      ui_flags   |= AErrDlg_quit_ui;
      warn_level  = MB_ICONERROR;
      icon_id     = int32_t(intptr_t(IDI_ERROR));  // Hand-shaped icon
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Format description AString
  const char * desc_high_p = msg.m_desc_high_p ? msg.m_desc_high_p : "An error has occurred.";
  const char * desc_low_p  = msg.m_desc_low_p ? msg.m_desc_low_p : "";
  const char * func_name_p = msg.m_func_name_p ? msg.m_func_name_p : "";
  uint32_t     desc_length = uint32_t(::strlen(desc_high_p));
  char *       desc_p      = a_stack_allocate(desc_length + AErrDlg_print_char_extra, char);
  AString      desc(desc_p, AErrDlg_print_char_extra + desc_length, 0u, false);

  desc.append(desc_high_p, desc_length);
  desc.append('\n');

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add any extra debugging context
  ADebug::context_append(&desc);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Internal info
  if (msg.m_source_path_p)
    {
    desc.append_format("\n  C++ Internal Info:\n    %s\n    %s(%u) :\n    %s\n", func_name_p, msg.m_source_path_p, msg.m_source_line, desc_low_p);
    }
  else
    {
    desc.append_format("\n  C++ Internal Info:\n    %s\n    %s\n", func_name_p, desc_low_p);
    }

  if (msg.m_err_id > AErrId_generic)
    {
    desc.append_format("    Exception Id: %u\n", msg.m_err_id);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Print to the IDE Output window.
  // *Note that this is done before displaying an alert window in case there are any
  // problems displaying the window.
  if (ui_flags & AErrDlg_debug_ui)
    {
    // $Revisit - CReis This print should indicate that it is an error message somehow so
    // that it can be grouped/colour coded accordingly.
    ADebug::print_format("\n###%s : ", title_p);
    ADebug::print(desc_p);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prompt user (if necessary)
  bool debug_break = false;

  *action_p = AErrAction_ignore;

  if (msg.m_err_level > AErrLevel_internal)
    {
    if ((ui_flags & (AErrDlg_recover_ui | AErrDlg_ignore_ui)) == (AErrDlg_recover_ui | AErrDlg_ignore_ui))
      {
      desc.append("\n['Continue' is usually more stable than 'Ignore']");
      }

    // Create Alert Window
    AErrorWin alert_win(desc, ignore_text_p, AFont("Arial", 11.0f), ui_flags);

    // $Revisit - CReis This sets the icon for all windows of this window class - create a unique error window class
    //alert_win.set_icon(icon_id);
    alert_win.set_title(title_p);

    ::MessageBeep(warn_level);

    debug_break = alert_win.determine_choice(action_p);
    }

  return debug_break;
  }

