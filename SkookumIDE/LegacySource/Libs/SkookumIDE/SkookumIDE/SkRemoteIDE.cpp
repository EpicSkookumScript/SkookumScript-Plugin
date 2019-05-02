// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript Remote IDE
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkRemoteIDE.hpp>
#include <SkookumIDE/SkConsole.hpp>
#include <SkookumIDE/SkClassBrowser.hpp>
#include <SkookumIDE/SkVersionText.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogCore/AMethodArg.hpp>
#include <AgogCore/ARandom.hpp>
#include <AgogCore/ASymbol.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkParser.hpp>
#include <AgogIO/AApplication.hpp>
#include <AgogIO/AIni.hpp>
#include <AgogIO/AWeb.hpp>
#include <AgogGUI_OS/ADialogOS.hpp>
#include <ws2tcpip.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
class SkIDEServer : public ASocketServer
  {
  public:

  // Common Methods
    
    SkIDEServer(SkRemoteIDE * remote_ide_p);

  // Server Events

    virtual ASocket * on_client_connect(const uint8_t remote_ipv4_p[4], uint16_t remote_port) override;
    virtual void      on_client_disconnect(ASocket * client_p) override;
    virtual bool      on_server_error(uint error_id) override;

  // Data Members

    SkRemoteIDE * m_remote_ide_p;

  };  // SkIDEServer


namespace
{

  const char * g_ini_section_remote_hosts_p   = "Remote IDE Hosts";

  // *Main* config (.ini) file
  const char * g_ini_section_settings_p       = "Settings";
  const char * g_ini_key_origin_p             = "Origin";    // A string describing where the user got this IDE from

  // *User/IDE* config (.ini) file
  const char * g_ini_section_console_p        = "Script Console";
  const char * g_ini_key_analytics_p          = "Analytics"; // Non-zero or undefined = analytics is enabled

} // End unnamed namespace


//=======================================================================================
// SkRemoteIDE Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkRemoteIDE::SkRemoteIDE() :
  m_local_server_p(nullptr),
  m_remote_server_p(nullptr),
  m_remote_server(false),
  m_socket_p(nullptr),
  m_socket_disconnecting_p(nullptr),
  m_auth_id(0u),
  m_telemetry_socket_p(nullptr),
  m_compiled_flags(CompiledFlag_ignore)
  {
  m_local_server_p  = new SkIDEServer(this);
  m_local_server_p->set_local_address(AIPAddress());

  m_remote_server_p = new SkIDEServer(this);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkRemoteIDE::~SkRemoteIDE()
  {
  close_socket();
  set_mode(SkLocale_embedded);
  delete m_local_server_p;
  m_local_server_p  = nullptr;
  delete m_remote_server_p;
  m_remote_server_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Set remote connection mode.
// Author(s):   Conan Reis
void SkRemoteIDE::set_mode(eSkLocale mode)
  {
  if (m_mode != mode)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Stop old mode
    m_compiled_flags = CompiledFlag_ignore;

    close_socket();

    switch (m_mode)
      {
      case SkLocale_embedded:
        break;

      case SkLocale_ide:
        if (m_local_server_p->is_connected())
          {
          m_local_server_p->disconnect();

          if (m_remote_server)
            {
            m_remote_server_p->disconnect();
            }
          }
        break;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Start new mode
    SkRemoteBase::set_mode(mode);

    if (SkConsole::ms_console_p)
      {
      SkConsole::ms_console_p->update_online_menu(mode);
      SkConsole::ms_console_p->refresh();
      }

    switch (m_mode)
      {
      case SkLocale_embedded:
        //SkDebug::print("\nSkookumIDE: Off-line mode\n\n", SkLocale_local);
        break;

      case SkLocale_ide:
        server_connect();
        break;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Initialize specified socket
// Author(s):   Conan Reis
void SkRemoteIDE::init_socket(ASocket * socket_p)
  {
  SK_ASSERTX(!m_socket_p, "Initializing socket when a connection already exists.");

  m_socket_p = socket_p;

  socket_p->enable_self_destruct();
  socket_p->set_connect_func(new AMethodArg<SkRemoteIDE, ASocket *>(this, &SkRemoteIDE::on_connect));
  socket_p->set_disconnect_func(new AMethodArg<SkRemoteIDE, ASocket *>(this, &SkRemoteIDE::on_disconnect));
  socket_p->set_incoming_func(new AMethodArg<SkRemoteIDE, ADatum *>(this, &SkRemoteIDE::on_incoming));
  socket_p->set_error_func(new AMethodArg<SkRemoteIDE, ASocketError *>(this, &SkRemoteIDE::on_error));
  }

//---------------------------------------------------------------------------------------
// Disconnect from currently active socket
void SkRemoteIDE::close_socket()
  {
  if (m_socket_p)
    {
    SK_ASSERTX(!m_socket_disconnecting_p, "Trying to disconnect when already disconnecting.");

    SkDebug::print(a_str_format("SkookumIDE: Disconnecting... %s\n", m_socket_p->as_string(false).as_cstr()), SkLocale_local);

    // Explicitly tell runtime that IDE is disconnecting.
    // Some socket implementations cannot detect or do not have an event for disconnection of
    // a socket.
    cmd_simple(Command_disconnect);

    // Do not disconnect here - wait for the remote runtime to close the connection when it receives Command_disconnect
    m_socket_p->disconnect();
    m_socket_disconnecting_p = m_socket_p;
    m_socket_p = nullptr;

    // Set after Command_disconnect is sent - no longer authenticated now
    set_connect_state(ConnectState_disconnecting);
    }
  }

//---------------------------------------------------------------------------------------
// Gets local host IP address using ini file as a guide
// Author(s):   Conan Reis
AIPAddress SkRemoteIDE::get_ini_ip_address()
  {
  // Only increments reference to socket system.  System only goes online if not already online.
  ASocketSystem::go_online();

  AString host_name(AIPAddress::get_local_host_name());

  // Read from ini file
  AString  host_addr_str = SkCompiler::ms_compiler_p->get_ini_ide().get_value(host_name, g_ini_section_remote_hosts_p);

  // If not present or starts with a dash it is disabled
  if ((host_addr_str == AIni::ms_not_present_str)
    || (host_addr_str.get_first() == '-'))
    {
    return AIPAddress(AIPAddress::ms_invalid_ipv4);
    }

  uint32_t host_addr_ip = AIPAddress::ms_invalid_ip;

  AIPAddress::str_to_ip_v_4(host_addr_str, &host_addr_ip);

  // $Revisit - CReis Need to determine if address already in use
  AIPAddress host_addr(AIPAddress::get_host_address(host_name, host_addr_ip));

  // Only decrements reference to socket system.  System only goes off-line as needed.
  ASocketSystem::go_offline();

  if (host_addr.get_ip4() != host_addr_ip)
    {
    // Save to ini
    SkCompiler::ms_compiler_p->get_ini_ide().set_value(host_addr.as_string(AIPFormat_number, false), host_name, g_ini_section_remote_hosts_p);
    }

  // Reject a localhost address since the local server is already using it.
  if (host_addr.get_ip4() == *reinterpret_cast<const uint32_t *>(AIPAddress::ms_local_host_ipv4))
    {
    return AIPAddress(AIPAddress::ms_invalid_ipv4);
    }

  return host_addr;
  }

//---------------------------------------------------------------------------------------
// Enabled or disable remote server listening
// 
// Author(s): Conan Reis
void SkRemoteIDE::server_remote_enable(
  bool remote //= true
  )
  {
  // Read from ini file
  AString host_name(AIPAddress::get_local_host_name());
  AString host_addr_str = SkCompiler::ms_compiler_p->get_ini_ide().get_value(
    host_name, g_ini_section_remote_hosts_p);

  if (host_addr_str == AIni::ms_not_present_str)
    {
    host_addr_str = '-';
    }

  // If it starts with a dash it is disabled
  bool current = (host_addr_str.get_first() != '-');

  if (remote != current)
    {
    if (remote)
      {
      host_addr_str.remove_all(0u, 1u);
      }
    else
      {
      host_addr_str.insert('-');
      }

    SkCompiler::ms_compiler_p->get_ini_ide().set_value(
      host_addr_str, host_name, g_ini_section_remote_hosts_p);

    if (remote)
      {
      if (!m_remote_server)
        {
        AIPAddress remote_addr(get_ini_ip_address());

        m_remote_server = remote_addr.is_valid();

        if (m_remote_server)
          {
          m_remote_server_p->set_local_address(remote_addr);
          m_remote_server_p->connect();
          }
        else
          {
          SkDebug::print(a_str_format(
              "SkookumIDE: Could not listen for remote socket connections!\n"
              "Check user.ini [%s] to ensure IP address is valid.\n\n",
              g_ini_section_remote_hosts_p),
            SkLocale_local,
            SkDPrintType_error);
          }
        }
      }
    else
      {
      if (m_remote_server)
        {
        m_remote_server = false;
        m_remote_server_p->disconnect();
        }
      }

    server_display();
    }
  }

//---------------------------------------------------------------------------------------
// Determines if the remote runtime server is listening or not.
// 
// Author(s): Conan Reis
bool SkRemoteIDE::is_server_remote_enabled()
  {
  AString host_addr_str = SkCompiler::ms_compiler_p->get_ini_ide().get_value(
    AIPAddress::get_local_host_name(), g_ini_section_remote_hosts_p);

  return (host_addr_str != AIni::ms_not_present_str) && (host_addr_str.get_first() != '-');
  }

//---------------------------------------------------------------------------------------
// Connect the server and listen for new client connections
// 
// Author(s): Conan Reis
void SkRemoteIDE::server_connect()
  {
  SkDebug::print("\nSkookumIDE: Prints in italics.\n", SkLocale_local);

  if (!m_local_server_p->is_connected())
    {
    // Only increments reference to socket system.  System only goes online if not already online.
    ASocketSystem::go_online();

    m_local_server_p->connect();

    // Determine if remote server should listen too
    AIPAddress remote_addr(get_ini_ip_address());

    m_remote_server = remote_addr.is_valid();

    if (m_remote_server)
      {
      m_remote_server_p->set_local_address(remote_addr);
      m_remote_server_p->connect();
      }

    // Only decrements reference to socket system.  System only goes off-line as needed.
    ASocketSystem::go_offline();
    }

  server_display();
  }

//---------------------------------------------------------------------------------------
// Display IP addresses and ports that server(s) are listening to.
// 
// Author(s): Conan Reis
void SkRemoteIDE::server_display()
  {
  if (m_remote_server)
    {
    SkDebug::print(
      a_str_format(
        "SkookumIDE: Listening for runtime connection on\n"
        "  localhost[%s] and\n"
        "  %s...\n",
        m_local_server_p->get_local_id().as_string(AIPFormat_number, AIPFormat_number, false).as_cstr(),
        m_remote_server_p->get_local_id().as_string(AIPFormat_resolved, AIPFormat_resolved, false).as_cstr()
        ),
      SkLocale_local);
    }
  else
    {
    SkDebug::print(
      a_str_format(
        "SkookumIDE: Listening for runtime connection on\n"
        "  localhost[%s]\n",
        m_local_server_p->get_local_id().as_string(AIPFormat_number, AIPFormat_number, false).as_cstr()
        ),
      SkLocale_local);
    }
  }

//---------------------------------------------------------------------------------------
// Disconnect remote IDE
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkRemoteIDE::disconnect()
  {
  if (m_connect_state < ConnectState_disconnecting)
    {
    switch (m_mode)
      {
      case SkLocale_runtime:
        set_mode(SkLocale_embedded);
        break;

      case SkLocale_ide:
        close_socket();
        break;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Indicates whether class constructors should be called at the start of the
//             Skookum session or not.
//             [The remote IDE may want to skip calling class constructors since they may
//             contain engine/runtime calls which may be unavailable to the IDE.]
// Returns:    true if class constructors should be called or false if not.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkRemoteIDE::should_class_ctors_be_called() const
  {
  if (!is_remote_ide())
    {
    return true;
    }

  // $Revisit - CReis Should cache this and might make more sense as a project-wide setting
  return SkCompiler::ms_compiler_p->get_ini_ide().get_value_bool_default(
    true, "RemoteIDEClassCtors", "Script Console");
  }

//---------------------------------------------------------------------------------------
// Have server authenticate the client via the version info the client sent
// Arg         client_version - 
// Arg         auth_id - authentication id from client
// Arg         client_flags - flags from client: see eSkRemoteFlag
// Author(s):   Conan Reis
void SkRemoteIDE::on_authenticate_client(
  uint8_t client_version,
  uint32_t auth_id,
  uint32_t client_flags,
  const SkProjectInfo & project_info
  )
  {
  if (auth_id != m_auth_id)
    {
    SkDebug::print("SkookumIDE: Runtime failed authorization!\n", SkLocale_local, SkDPrintType_error);
    disconnect();

    return;
    }

  if (client_version != ms_version)
    {
    SkDebug::print(
      a_str_format(
        "SkookumIDE: Runtime version is %s than the IDE version!\n",
        (client_version < ms_version) ? "less" : "greater"),
      SkLocale_all,
      SkDPrintType_warning);
    disconnect();

    return;
    }

  m_remote_flags |= client_flags;

  // Translate symbol ids as needed.
  if (!is_symbol_db_remote())
    {
    SkDebug::print("SkookumIDE: Will attempt to translate unconverted symbol ids from runtime.\n", SkLocale_local);
    }

  // Assign friendly name to socket
  m_socket_p->set_friendly_name(project_info.m_project_name);

  cmd_authenticate();

  // Send telemetry info to our telemetry server
  transmit_telemetry_connect(project_info.m_engine_id, project_info.m_platform_id);

  // Send over debug preferences
  cmd_debug_preferences();

  SkConsole::ms_console_p->connect_new_runtime(project_info);
  }

//---------------------------------------------------------------------------------------
// Called on request to determine/ensure the project (specific script overlays, etc.)
// used by the runtime is loaded into the IDE and the compiled binary is up-to-date and
// optionally freshen it (recompile) as needed.
// 
// Params:
//   project_path: project settings file to use
//   freshen:
//     recompile if true and out-of-date or get current state of compiled binary if false
//
// See: cmd_freshen_compiled()
void SkRemoteIDE::on_cmd_freshen_compiled(
  const SkProjectInfo & project_info,
  bool freshen
  )
  {
  SkDebug::print(
    a_str_format("SkookumIDE: Runtime requested to %s if compiled binaries are up-to-date.\n", freshen ? "ensure" : "determine"),
    SkLocale_local);

  uint32_t compiled_flags = SkRemoteIDE::CompiledFlag_notify | (freshen ? SkRemoteIDE::CompiledFlag_freshen : 0u);

  switch (SkConsole::ms_console_p->get_load_project_state())
    {
    case AProgess_queued:
      SkConsole::ms_console_p->notify_on_load_project_deferred(freshen);
      return;

    case AProgess_processing:
      m_compiled_flags = compiled_flags;
      return;
    }

  // $Revisit - CReis could change project while connected
  //SkConsole::ms_console_p->load_project(project_info);

  m_compiled_flags = compiled_flags;

  if (SkCompiler::ms_compiler_p->get_phase() > SkCompiler::Phase_parse)
    {
    // If in the middle of a recompile ignore reparse/check
    if (freshen)
      {
      SkCompiler::ms_compiler_p->reparse_if_stale();
      }
    else
      {
      SkCompiler::ms_compiler_p->is_compiled_fresh();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Runtime made the project editable and transmitted back the new project information
void SkRemoteIDE::on_cmd_make_editable_reply(const AString & error_msg, const SkProjectInfo & project_info)
  {
  // Were we successful?
  if (error_msg.is_empty())
    {
    // Yes, reload project
    if (SkConsole::ms_console_p->load_project(project_info))
      {
      // And make sure binaries are fresh
      SkCompiler::ms_compiler_p->is_compiled_fresh();
      }
    }
  else
    {
    // No, display error message
    ADialogOS::info(
      error_msg,
      "Error while trying to make project editable!",
      ADialogOS::Flag_none,
      ADialogOS::Icon_warning);
    }
  }

//---------------------------------------------------------------------------------------
// Called when the runtime requests IDE to recompile a class (and optional
//             subclasses).
// Arg         class_name - name of class to recompile (also indicates superclass if
//             recurse set)
// Arg         recurse - if true the subclasses of class_name should also be recompiled
// See:        SkRemoteBase::cmd_recompile_classes()
// Author(s):   Conan Reis
void SkRemoteIDE::on_cmd_recompile_classes(
  const ASymbol & class_name,
  bool            recurse
  )
  {
  SkClass * class_p = SkBrain::get_class(class_name);

  SkDebug::set_execution_state(SkDebug::State_suspended);
  SkConsole::ms_console_p->refresh_debug_ui();

  SkConsole::ms_console_p->compile_class(class_p, recurse);
  }

//---------------------------------------------------------------------------------------
// Called whenever the remote runtime has hit a breakpoint.
// Arg         bp_p - address of breakpoint hit
// Author(s):   Conan Reis
void SkRemoteIDE::on_cmd_breakpoint_hit(SkBreakPoint * bp_p)
  {
  SkConsole::ms_console_p->play_sound(SkConsole::Sound_breakpoint);
  SkDebug::set_next_expression(*bp_p);
  SkDebug::set_execution_state(SkDebug::State_suspended_expr);

  // Browse to member and expression position and enable step-wise debugging commands.
  SkContextInfo member_info(*bp_p);

  SkConsole::ms_console_p->debug_expr(bp_p->get_expr(), member_info);
  }

//---------------------------------------------------------------------------------------
// Called whenever the remote runtime has hit a breakpoint.
// Arg         bp_p - address of breakpoint hit
// Author(s):   Conan Reis
void SkRemoteIDE::on_cmd_break_expression(const SkMemberExpression & expr_info)
  {
  // Note that a sound is not played for simple expression breaks
  SkDebug::set_next_expression(expr_info);
  SkDebug::set_execution_state(SkDebug::State_suspended_expr);

  // Browse to member and expression position and enable step-wise debugging commands.
  SkContextInfo member_info(expr_info);

  SkConsole::ms_console_p->debug_expr(expr_info.get_expr(), member_info);
  }

//---------------------------------------------------------------------------------------
void SkRemoteIDE::on_cmd_invoke_result(AString & result_str)
  {
  SkRemoteBase::on_cmd_invoke_result(result_str);
  SkEditBox::set_result_string(result_str);
  }

//---------------------------------------------------------------------------------------
// Sends a command in datum form to the remote side
// Arg         datum - command in datum form from a cmd_*() method
// Modifiers:   virtual
// Author(s):   Conan Reis
SkRemoteBase::eSendResponse SkRemoteIDE::on_cmd_send(const ADatum & datum)
  {
  if (m_socket_p)
    {
    m_socket_p->send(datum);
    }
  else
    {
    SkDebug::print(
      a_str_format(
        "SkookumIDE: %s side is not connected - command ignored!\n"
        "[Connect runtime to IDE (there is usually a \"toggle SkookumIDE\" option)"
        "or you can run the command locally on the IDE with Shift+F4 - though only"
        "if the command is available on the IDE and not just in the runtime.]\n",
        is_remote_ide() ? "Client runtime" : "Remote IDE"),
      SkLocale_local,
      SkDPrintType_warning);

    return SkRemoteBase::SendResponse_Not_Connected;
    }

  return SkRemoteBase::SendResponse_OK;
  }

//---------------------------------------------------------------------------------------
// Send version and authentication info to client and request client version in return.
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_version()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Generate desired authentication id result
  uint32_t    auth_seed = ARandom::ms_gen.uniform_ui();
  ARandom auth_rand(auth_seed);

  auth_rand.uniform_ui();
  auth_rand.uniform_ui();
  
  m_auth_id = auth_rand.uniform_ui();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Send version info
  ADatum datum(SkRemote_version_byte_size);
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_version;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  **data_pp = SkRemoteBase::ms_version;
  (*data_pp)++;

  A_BYTE_STREAM_OUT32(data_pp, &auth_seed);

  SkDebug::print("SkookumIDE: Authenticating version...\n", SkLocale_local);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Inform the runtime that it has been authenticated.
void SkRemoteIDE::cmd_authenticate()
  {
  SkDebug::print(a_str_format("SkookumIDE: Runtime%s authenticated!\n",
    m_socket_p->get_friendly_name().is_empty() ? "" : (" \"" + m_socket_p->get_friendly_name() + "\"").as_cstr()),
    SkLocale_local);

  set_connect_state(ConnectState_authenticated);
  cmd_simple(Command_authenticate);
  }

//---------------------------------------------------------------------------------------
// Send debug preferences to runtime.
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_debug_preferences()
  {
  if (!is_authenticated())
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   4 bytes - Debug preference flags - see SkDebug::ePrefFlag
  //   1 byte  - indent size
  //   1 byte  - tab stop size

  ADatum  datum(10u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t value32 = Command_preferences;
  A_BYTE_STREAM_OUT32(data_pp, &value32);

  // 4 bytes - Debug preference flags
  value32 = SkDebug::get_preferences();
  A_BYTE_STREAM_OUT32(data_pp, &value32);

  // 1 byte - indent size
  uint8_t value8 = uint8_t(SkDebug::ms_indent_size);
  A_BYTE_STREAM_OUT8(data_pp, &value8);

  // 1 byte - tab stop size
  value8 = uint8_t(SkDebug::ms_tab_stops);
  A_BYTE_STREAM_OUT8(data_pp, &value8);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Tell the runtime to make the current project editable
void SkRemoteIDE::cmd_make_editable()
  {
  cmd_simple(Command_make_editable);
  }

//---------------------------------------------------------------------------------------
// Send reply to earlier freshen compiled request
// Arg         state - see eCompiledState
// See:        on_freshen_compiled()
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_compiled_state_reply(eCompiledState state)
  {
  if ((m_compiled_flags == CompiledFlag_ignore) || !is_connected())
    {
    m_compiled_flags = CompiledFlag_ignore;
    return;
    }

  if ((state == CompiledState_stale) && (m_compiled_flags & CompiledFlag_freshen))
    {
    // Ignore stale reply - about to start compiling
    m_compiled_flags &= ~CompiledFlag_freshen;
    return;
    }

  const char * state_str_p = "script binaries have an unknown state";

  switch (state)
    {
    case CompiledState_determining:
      state_str_p = "script binaries are being determined if stale/fresh";
      break;

    case CompiledState_compiling:
      state_str_p = "scripts are compiling";
      break;

    case CompiledState_stale:
      state_str_p = "script binaries are stale";
      break;

    case CompiledState_errors:
      state_str_p = "scripts have errors";
      break;

    case CompiledState_fresh:
      state_str_p = "script binaries are fresh";
      break;
    }

  SkDebug::print(a_str_format("\nSkookumIDE: Notifying runtime that %s\n", state_str_p), SkLocale_local, SkDPrintType_trace);

  // Remove notification if end state
  if (state > CompiledState_compiling)
    {
    m_compiled_flags = CompiledFlag_ignore;
    }

  // Binary composition:
  //   4 bytes - command id
  //   1 byte  - compiled state
  
  ADatum datum(static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint8_t)));
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_freshen_compiled_reply;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  *data_p = uint8_t(state);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Send portion of class hierarchy superclass-subclass relationships to ensure
//             that the classes exist on the remote runtime.  [Usually called prior to
//             some calls that depend on the classes already being present.]
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_hierarchy_update(
  SkClass * class_p,
  bool      recurse // = true
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Track symbols if not already tracked
  ASymbolTable used_syms;
  bool         track_syms = !ASymbol::is_tracking_serialized() && is_symbol_db_remote();

  if (track_syms)
    {
    // Track all symbols that are saved in the compiled binary.
    ASymbol::track_serialized(&used_syms);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about update
  uint32_t class_count = 1;
  
  if (recurse)
    {
    class_count = class_p->get_class_recurse_count(false);

    ADebug::print_format(
      "SkookumIDE: Ensuring the class '%s' and its %u subclass%s exist on runtime client.\n",
      class_p->get_name_cstr_dbg(),
      class_count - 1u,
      (class_count != 2u) ? "es" : "");
    }
  else
    {
    ADebug::print_format(
      "SkookumIDE: Ensuring the class '%s' exists on runtime client.\n",
      class_p->get_name_cstr_dbg(),
      class_count - 1u);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   4 bytes - #classes
  //   4 bytes - name of class       \_ repeating
  //   4 bytes - name of superclass  /

  ADatum datum(static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint32_t) + (sizeof(uint32_t) * 2 * class_count)));

  uint8_t *   data_p  = datum.get_data_writable();
  void ** data_pp = (void **)&data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_class_hierarchy_update;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 4 bytes - #classes
  A_BYTE_STREAM_OUT32(data_pp, &class_count);

  SkClass * current_p = class_p;

  do 
    {
    // 4 bytes - name of class
    class_p->get_name().as_binary(data_pp);

    // 4 bytes - name of superclass
    if (class_p->get_superclass())
      {
      class_p->get_superclass()->get_name().as_binary(data_pp);
      }
    else
      {
      ASymbol::ms_null.as_binary(data_pp);
      }

    current_p = recurse ? current_p->next_class(class_p) : nullptr;
    }
  while (current_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Stop tracking symbols
  if (track_syms)
    {
    ASymbol::track_serialized(nullptr);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Send out the command
  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Notify runtime about result of compiling class(es)
// This could be sent directly from IDE or as a reply to earlier freshen compiled request.
//
// Returns: number of classes affected
// Params:
//   class_p: class (& optional subclasses) being updated
//   recurse: true if subclasses are also being updated, false if not
//   
// Author(s): Conan Reis
uint32_t SkRemoteIDE::cmd_recompile_classes_reply(
  SkClass * class_p,
  bool      recurse,
  uint32_t  error_count // = 0u
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Send recompile results to runtime
  // Binary composition:
  //   4 bytes - command id
  //   4 bytes - string length
  //   n bytes - string name of class recompiled
  //   1 byte  - successful recompile (true) or not (false)
  //   4 bytes - number of classes to receive or # of errors

  AString   class_name(class_p->get_name_str());
  ADatum    datum(9u + class_name.as_binary_length());
  uint8_t * data_p  = datum.get_data_writable();
  void **   data_pp = (void **)&data_p;


  // 4 bytes - command id
  uint32_t cmd = Command_recompile_classes_reply;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);


  // name of class recompiled - not using a symbol since it might not exist yet
  // 4 bytes - string length
  // n bytes - string
  class_p->get_name_str().as_binary(data_pp);


  // 1 byte  - successful recompile (true) or not (false)
  bool success = (error_count == 0u);

  A_BYTE_STREAM_OUT8(data_pp, &success);


  // 4 bytes - number of classes to receive or # of errors
  uint32_t class_count = recurse ? class_p->get_class_recurse_count(false) : 1u;

  if (success)
    {
    A_BYTE_STREAM_OUT32(data_pp, &class_count);

    // Indicate that runtime is running
    SkDebug::set_execution_state(SkDebug::State_running);
    SkConsole::ms_console_p->refresh_debug_ui();
    }
  else
    {
    A_BYTE_STREAM_OUT32(data_pp, &error_count);
    }

  on_cmd_send(datum);

  return class_count;
  }

//---------------------------------------------------------------------------------------
// Send update of class (and optionally any subclasses).
// This could be sent directly from IDE or as a reply to earlier freshen compiled request.
// 
// Params:
//   class_p: class (& optional subclasses) being updated
//   recurse: true if subclasses are also being updated, false if not
//   
// Author(s): Conan Reis
void SkRemoteIDE::cmd_class_update(
  SkClass * class_p,
  bool      recurse // = true
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about update
  uint32_t class_count = cmd_recompile_classes_reply(class_p, recurse);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Track symbols if not already tracked
  ASymbolTable used_syms;
  bool         track_syms = !ASymbol::is_tracking_serialized() && is_symbol_db_remote();

  if (track_syms)
    {
    // Track all symbols that are saved in the compiled binary.
    ASymbol::track_serialized(&used_syms);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Build up class update packets (done first to build-up used symbol table)
  uint8_t *       data_p;
  void **         data_pp;
  ADatum *        class_datum_p;
  APArray<ADatum> class_datums;
  SkClass *       current_p = class_p;
  uint32_t        cmd       = Command_class_update;

  // Change class unions and typed classes to full binary desc instead of reference index
  SkClassDescBase::enable_compound_refs(false);

  class_datums.ensure_size_empty(class_count);

  do 
    {
    //ADebug::print_format(
    //  "  Updating '%s'...\n",
    //  current_p->get_name_cstr_dbg());
      
    class_datum_p = SK_NEW(ADatum)(static_cast<uint32_t>(sizeof(uint32_t)) + current_p->as_binary_length());
    class_datums.append(*class_datum_p);

    data_p  = class_datum_p->get_data_writable();
    data_pp = (void **)&data_p;

    // 4 bytes - command id
    A_BYTE_STREAM_OUT32(data_pp, &cmd);

    // n bytes - class binary
    current_p->as_binary(data_pp);

    // $Revisit - CReis Could add a CRC32 to help ensure binary integrity

    current_p = recurse ? current_p->next_class(class_p) : nullptr;
    }
  while (current_p);

  // Change back class unions and typed classes to reference index for binaries
  SkClassDescBase::enable_compound_refs();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Stop tracking symbols - and send symbol update if needed
  if (track_syms)
    {
    ASymbol::track_serialized(nullptr);

    // Update remote symbols - must be 1st in chain of commands
    ADebug::print("SkookumIDE: Sending symbol DB update.\n");
    cmd_symbols_update(used_syms);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure classes exist on remote runtime - 2nd in chain of commands
  cmd_hierarchy_update(class_p, recurse);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Send out the class updates - each class as a separate update
  ADatum ** datums_pp     = class_datums.get_array();
  ADatum ** datums_end_pp = datums_pp + class_count;

  for (; datums_pp < datums_end_pp; datums_pp++)
    {
    on_cmd_send(**datums_pp);
    }

  // Update breakpoints
  cmd_breakpoint_update_all();
  }

//---------------------------------------------------------------------------------------
// Create a breakpoint on the runtime side
// 
// Params:
//   bp: breakpoint to create
// 
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_breakpoint_create(const SkBreakPoint & bp)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //    4 bytes - command id
  //   11 bytes - member expression
  //    1 byte  - enabled
  //    2 bytes - table index

  ADatum datum(4u + bp.as_binary_length() + 1u + 2u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_breakpoint_create;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 11 bytes - member expression
  bp.as_binary(data_pp);

  // 1 byte - enabled
  A_BYTE_STREAM_OUT8(data_pp, &bp.m_enabled);

  // 2 bytes - table index
  A_BYTE_STREAM_OUT16(data_pp, &bp.m_table_idx);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Update one or more breakpoints on the runtime side
// 
// Params:
//   bp_p:
//     address of breakpoint to update or `nullptr` if update action should apply to all
//     breakpoints.
//   
// Author(s): Conan Reis
void SkRemoteIDE::cmd_breakpoint_update(
  SkBreakPoint *        bp_p,
  SkBreakPoint::eUpdate action
  )
  {
  if (!is_authenticated())
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   2 bytes - breakpoint table index or SkDebugInfo::Flag_debug_idx__none to indicate all
  //   1 byte  - update action

  ADatum datum(4u + 2u + 1u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_breakpoint_update;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 2 bytes - table index
  uint16_t table_idx = bp_p ? uint16_t(bp_p->m_table_idx) : SkDebugInfo::Flag_debug_idx__none;

  A_BYTE_STREAM_OUT16(data_pp, &table_idx);

  // 1 byte - member type
  A_BYTE_STREAM_OUT8(data_pp, &action);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Update all the breakpoints on the runtime side.
// Usually called after a recompile, etc.
// 
// Author(s): Conan Reis
void SkRemoteIDE::cmd_breakpoint_update_all()
  {
  if (!is_authenticated())
    {
    return;
    }

  // Reset and resend all breakpoints to the runtime.
  
  // Remove any existing breakpoints
  cmd_breakpoint_update(nullptr, SkBreakPoint::Update_remove);

  const APSortedLogicalFree<SkBreakPoint, SkMemberExpression> & breakpoints = SkDebug::breakpoints_get_all();

  uint32_t bp_count = breakpoints.get_length();

  if (bp_count)
    {
    SkBreakPoint ** bp_p     = breakpoints.get_array();
    SkBreakPoint ** bp_end_p = bp_p + bp_count;

    do 
      {
      cmd_breakpoint_create(**bp_p);
      bp_p++;
      }
    while (bp_p < bp_end_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called to notify the remote runtime that the execution can be resumed.
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_break_continue()
  {
  if (!is_authenticated())
    {
    return;
    }

  SkDebug::print(
    "\nSkookumIDE: Resuming runtime execution...\n",
    SkLocale_local,
    SkDPrintType_trace);

  cmd_simple(Command_break_continue);
  }

//---------------------------------------------------------------------------------------
// Called to notify the remote runtime that the execution can be resumed
//             until the specified step point.
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_break_step(SkDebug::eStep step_type)
  {
  if (!is_authenticated())
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   1 byte  - step type

  ADatum datum(4u + 1u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_break_step;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 1 byte - step type
  A_BYTE_STREAM_OUT8(data_pp, &step_type);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Request runtime to print out callstack
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_print_callstack()
  {
  if (!is_authenticated() || (SkDebug::get_execution_state() == SkDebug::State_running))
    {
    SkDebug::print(
      "\nSkookum runtime is not at a break - call stack not available.\n",
      SkLocale_local,
      SkDPrintType_warning);

    return;
    }

  cmd_simple(Command_break_print_callstack);
  }

//---------------------------------------------------------------------------------------
// Request runtime to print out callstack
// Author(s):   Conan Reis
void SkRemoteIDE::cmd_print_locals()
  {
  if (!is_authenticated() || (SkDebug::get_execution_state() == SkDebug::State_running))
    {
    SkDebug::print(
      "\nSkookum runtime is not at a break - locals not available.\n",
      SkLocale_local,
      SkDPrintType_warning);

    return;
    }

  cmd_simple(Command_break_print_locals);
  }

//---------------------------------------------------------------------------------------
// Receive and parse/execute a command from the remote side
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkRemoteIDE::on_cmd_recv(eCommand cmd, const uint8_t * data_p, uint32_t data_length)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If default commands need to be overridden then this would be called last rather than first.
  bool cmd_processed = SkRemoteBase::on_cmd_recv(cmd, data_p, data_length);


  if (!cmd_processed)
    {
    const void ** data_pp = (const void **)&data_p;

    cmd_processed = true;

    switch (cmd)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_version_reply:
        {
        if (data_length < SkRemote_version_reply_byte_size - sizeof(uint32_t))
          {
          disconnect();
          break;
          }

        uint8_t  version = A_BYTE_STREAM_UI8_INC(data_pp);
        uint32_t auth_id = A_BYTE_STREAM_UI32_INC(data_pp);
        uint32_t flags   = A_BYTE_STREAM_UI32_INC(data_pp);

        SkProjectInfo project_info(data_pp);

        on_authenticate_client(version, auth_id, flags, project_info);

        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_freshen_compiled:
        {
        // 1 byte  - freshen bool
        bool freshen = bool(A_BYTE_STREAM_UI8_INC(data_pp));

        // n bytes - project info
        SkProjectInfo project_info(data_pp);

        on_cmd_freshen_compiled(project_info, freshen);
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Ignore advanced command not implemented in this IDE
      case Command_incremental_update_reply:
        *((uint8_t **)data_pp) += 13; // Skip the command body
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_show:
        {
        eAFlag show(eAFlag(A_BYTE_STREAM_UI8_INC(data_pp)));
        ASymbol focus_class_name(ASymbol::create_from_binary(data_pp, false));
        ASymbol focus_member_name(ASymbol::create_from_binary(data_pp, false));
        bool is_data_member(A_BYTE_STREAM_UI8_INC(data_pp));
        bool focus_member_class_scope(A_BYTE_STREAM_UI8_INC(data_pp));
        SkConsole::ms_console_p->display_ide(show, focus_class_name, focus_member_name, is_data_member, focus_member_class_scope);
        }
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_make_editable_reply:
        {
        AString error_msg(data_pp);
        SkProjectInfo project_info(data_pp);
        on_cmd_make_editable_reply(error_msg, project_info);
        }
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_recompile_classes:
        {
        // 4 bytes - name of class recompiled
        ASymbol class_name(ASymbol::create_from_binary(data_pp));

        // 1 byte  - successful recompile (true) or not (false)
        bool recurse = bool(A_BYTE_STREAM_UI8_INC(data_pp));

        on_cmd_recompile_classes(class_name, recurse);

        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_ready_to_debug:
        cmd_breakpoint_update_all();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_breakpoint_hit:
        {
        // Binary composition:
        //   2 bytes - breakpoint table index

        // 2 bytes - breakpoint table index or SkDebugInfo::Flag_debug_idx__none to indicate all
        SkBreakPoint * bp_p = SkDebug::breakpoint_get_at_idx(A_BYTE_STREAM_UI16_INC(data_pp));
        SK_ASSERTX(bp_p, "Cannot locate breakpoint!");
        if (bp_p)
          {
          on_cmd_breakpoint_hit(bp_p);
          }
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_break_expression:
        {
        SkMemberExpression expr_info(data_pp);

        on_cmd_break_expression(expr_info);
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      default:
        if (cmd < Command__last)
          {
          SkDebug::print(
            a_str_format(
              "SkookumIDE: Command from runtime not implemented on IDE!  [Cmd Id: %u]\n", cmd),
            SkLocale_all,
            SkDPrintType_error);
          }
        else
          {
          cmd_processed = false;
          }
      }
    }

  if (!cmd_processed)
    {
    // Send error to both sides
    SkDebug::print(
      a_str_format(
        "SkookumIDE: Unknown command sent from runtime to IDE!  [Cmd Id: %u]\n", cmd),
      SkLocale_all,
      SkDPrintType_error);
    }

  return cmd_processed;
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_connect()
// Author(s):   Conan Reis
void SkRemoteIDE::on_connect(ASocket * socket_p)
  {
  SK_ASSERTX(socket_p == m_socket_p, "Connecting to unknown socket.");

  SkDebug::print(a_str_format("SkookumIDE: Connected %s\n", m_socket_p->as_string(false).as_cstr()), SkLocale_local);
  set_connect_state(ConnectState_authenticating);

  if (is_remote_ide())
    {
    // Send version and authentication info
    cmd_version();
    }
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_disconnect()
// Author(s):   Conan Reis
void SkRemoteIDE::on_disconnect(ASocket * socket_p)
  {
  // Allow disconnection when both m_socket_p and m_socket_disconnecting_p are null as this might be after an error
  SK_ASSERTX((!m_socket_p && !m_socket_disconnecting_p) || socket_p == m_socket_p || socket_p == m_socket_disconnecting_p, AErrMsg("Disconnecting from unknown socket.", AErrLevel_notify));

  SkDebug::print(a_str_format(
      "SkookumIDE: Disconnected %s\n",
      socket_p->get_local_id().as_string(AIPFormat_resolved, AIPFormat_resolved, false).as_cstr()),
    SkLocale_local,
    SkDPrintType_warning);

  if (socket_p == m_socket_disconnecting_p)
    {
    m_socket_disconnecting_p = nullptr;
    }
  else if (socket_p == m_socket_p)
    {
    m_socket_p = nullptr;

    // Since we are now left without a connection, set our state accordingly
    set_connect_state(ConnectState_disconnected);
    if (m_mode == SkLocale_runtime)
      {
      set_mode(SkLocale_embedded);
      }
    }
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_incoming()
// Author(s):   Conan Reis
void SkRemoteIDE::on_incoming(ADatum * datum_p)
  {
  uint32_t data_length = datum_p->get_data_length();

  if (data_length == 0u)
    {
    return;
    }

  const uint8_t * data_p = datum_p->get_data();
  eCommand    cmd;

  A_BYTE_STREAM_IN32(&cmd, &data_p);

  on_cmd_recv(cmd, data_p, data_length - 4u);
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_error() & ASocketError
// Author(s):   Conan Reis
void SkRemoteIDE::on_error(ASocketError * error_p)
  {
  ASocket * socket_p = error_p->m_socket_p;

  if (socket_p && (socket_p->get_state() == ASocket::State_connecting))
    {
    // Prevent default assert from occurring
    error_p->m_default_action = false;

    SkDebug::print("\nSkookumIDE: Could not connect with remote side!\n\n", SkLocale_local, SkDPrintType_error);
    }
  else
    {
    if (socket_p)
      {
      if ((error_p->m_error_id == WSAECONNRESET)
        || (error_p->m_error_id == ERROR_NETNAME_DELETED))
        {
        SkDebug::print("\nSkookumIDE: connection reset from runtime!\n\n", SkLocale_local, SkDPrintType_warning);

        // Prevent default assert from occurring
        error_p->m_default_action = false;
        }
      }
    }

  if (socket_p == m_socket_p)
    {
    m_socket_p = nullptr;

    if (m_mode == SkLocale_runtime)
      {
      set_mode(SkLocale_embedded);
      }
    }
  else if (socket_p == m_socket_disconnecting_p)
    {
    m_socket_disconnecting_p = nullptr;
    }
  }

//---------------------------------------------------------------------------------------
// Called when connection state changes.  Useful as a hook for UI notification.
// Arg         old_state - previous connection state (use m_connect_state for current)
// See:        set_connect_state()
// Notes:      Called by set_connect_state() - do not call directly.
// Modifiers:   virtual - overridden from SkRemoteBase
// Author(s):   Conan Reis
void SkRemoteIDE::on_connect_change(eConnectState old_state)
  {
  //ADebug::print_format("SkookumIDE: on_connect_change(%u->%u)\n", old_state, m_connect_state);

  if (SkConsole::ms_console_p)
    {    
    if ((old_state == ConnectState_authenticated) || (m_connect_state == ConnectState_authenticated))
      {
      SkConsole::ms_console_p->debug_reset();
      }

    SkConsole::ms_console_p->refresh();
    }
  }

//---------------------------------------------------------------------------------------
// Transmits telemetry hit to our analytics server
void SkRemoteIDE::transmit_telemetry_connect(const AString & engine_id_string, const AString & platform_id_string)
  {
  // Is telemetry desired?
  if (SkCompiler::ms_compiler_p->get_ini_ide().get_value_int(g_ini_key_analytics_p, g_ini_section_console_p))
    {
    // Determine some information about this computer, this app, the remote client etc.
    const char * host_name_p = "ide.skookumscript.com";
    AString ide_version(SK_VERSION_TEXT);
    uint32_t machine_id_crc32 = AChecksum::generate_crc32(AApplication::get_machine_id());
    WCHAR locale_name_buffer[32];
    AString locale_name(locale_name_buffer, GetUserDefaultLocaleName(locale_name_buffer, sizeof(locale_name_buffer)) - 1);

    // Construct request
    m_telemetry_request.empty();
    m_telemetry_request.ensure_size(2048);
    m_telemetry_request.format("GET /s?cid=%u&av=%s&cd=c%s%%2F%s&ul=%s",
      machine_id_crc32,
      AWeb::url_encode(ide_version).as_cstr(),
      AWeb::url_encode(engine_id_string).as_cstr(),
      AWeb::url_encode(platform_id_string).as_cstr(),
      AWeb::url_encode(locale_name).as_cstr());

    // Determine source if given
    AString source = SkCompiler::ms_compiler_p->get_ini_main().get_value(g_ini_key_origin_p, g_ini_section_settings_p);
    source.crop();
    if (!source.is_empty() && source != AIni::ms_not_present_str)
      {
      m_telemetry_request.append_format("&aiid=%s", AWeb::url_encode(source).as_cstr());
      }

    // Append size of class browser window as "viewport"
    const ADisplay * display_p = nullptr;
    if (SkClassBrowser::ms_browser_p)
      {
      m_telemetry_request.append_format("&vp=%dx%d", SkClassBrowser::ms_browser_p->get_width(), SkClassBrowser::ms_browser_p->get_height());
      display_p = ADisplay::hit_display(*SkClassBrowser::ms_browser_p);
      }
    else if (SkConsole::ms_console_p)
      {
      display_p = ADisplay::hit_display(*SkConsole::ms_console_p);
      }
    // Append size of display where window is located as "screen resolution"
    if (display_p)
      {
      m_telemetry_request.append_format("&sr=%dx%d", display_p->m_region.m_width, display_p->m_region.m_height);
      }

    // Finalize request
    m_telemetry_request.append_format(" HTTP/1.1\r\nHost: %s\r\n\r\n", host_name_p);

    // Now trigger a hit with our analytics server
    AIPAddress ip_address = AIPAddress::get_host_address(host_name_p, nullptr, true);
    if (ip_address.is_valid())
      {
      m_telemetry_socket_p = new ASocket(ip_address, AIPPort_http, ASocket::Format_raw);
      m_telemetry_socket_p->enable_self_destruct();
      m_telemetry_socket_p->set_connect_func(new AMethodArg<SkRemoteIDE, ASocket *>(this, &SkRemoteIDE::on_telemetry_connect));
      m_telemetry_socket_p->set_disconnect_func(new AMethodArg<SkRemoteIDE, ASocket *>(this, &SkRemoteIDE::on_telemetry_disconnect));
      m_telemetry_socket_p->set_error_func(new AMethodArg<SkRemoteIDE, ASocketError *>(this, &SkRemoteIDE::on_telemetry_error));
      m_telemetry_socket_p->connect();
      }
    }
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_telemetry_connect()
void SkRemoteIDE::on_telemetry_connect(ASocket * socket_p)
  {
  SK_ASSERTX(socket_p == m_telemetry_socket_p, "Connected to unknown telemetry socket.");

  m_telemetry_socket_p->send(m_telemetry_request.as_cstr(), m_telemetry_request.get_length());
  m_telemetry_socket_p->disconnect();
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_telemetry_disconnect()
void SkRemoteIDE::on_telemetry_disconnect(ASocket * socket_p)
  {
  SK_ASSERTX(socket_p == m_telemetry_socket_p, "Disconnected from unknown telemetry socket.");

  m_telemetry_socket_p = nullptr;
  m_telemetry_request.empty();
  }

//---------------------------------------------------------------------------------------
// See ASocket::on_error() & ASocketError
void SkRemoteIDE::on_telemetry_error(ASocketError * error_p)
  {
  m_telemetry_socket_p = nullptr;
  m_telemetry_request.empty();
  }


//=======================================================================================
// SkRemoteIDE Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
SkIDEServer::SkIDEServer(SkRemoteIDE * remote_ide_p) :
  ASocketServer(AIPPort_skookum_ide),
  m_remote_ide_p(remote_ide_p)
  {
  }

//---------------------------------------------------------------------------------------
// This event is called whenever a client socket is "heard" while this
//             server socket is listening (i.e. connected).
//             This event should be overridden to handle incoming client socket
//             connections or a valid function callback object should be registered with
//             set_accept_client_func().
// Returns:    If it accepts the client connection based on the connecting
//             remote client socket's information, it creates a server client socket (an
//             instance of the ASocket class) using the ASocket server client class
//             constructor and returns it.  If it does not accept the client connection,
//             a client socket object is not created and nullptr is returned - indicating
//             that the connecting client socket is rejected.
// Arg         remote_ipv4_p - IP Address of the client attempting to connect in a four
//             byte array form.  Note, this type is used rather than AIPAddress for
//             reasons of efficiency - a more sophisticated AIPAddress object may not
//             need to be instantiated if the client socket is rejected.
// Arg         remote_port - IP Port of the client attempting to connect in 16 bit form.
//             Note, this type is used rather than AIPPort for reasons of efficiency - a
//             more sophisticated AIPPort object may not need to be instantiated if the
//             client socket is rejected.
// See:        set_accept_client_func(), get_clients()
//             on_client_disconnect(), set_client_disconnect_func()
// Notes:      If a server client socket is created and returned, it will be added to the
//             server's client list and it will be immediately connected and begin to
//             service any data coming from or going to the remote client.
//
//             The order of connect and disconnect event methods is:
//               EVENT METHOD                          SERVER CLIENT LIST
//               ASocketServer::on_client_connect()     not in list
//               ASocket::on_connect()                  in list
//               ASocket::on_disconnect()               in list
//               ASocketServer::on_client_disconnect()  not in list
//
//             Most sockets will be created dynamically on the heap and look after
//             themselves via a call to enable_self_destruct() - if so a matching
//             attempt_self_destruct() is called following the call to the
//             on_client_disconnect() method.  
//             ASocket objects may still be created on the stack, but they must be fully
//             disconnected before they go out of scope.
// Author(s):   Conan Reis
ASocket * SkIDEServer::on_client_connect(
  const uint8_t remote_ipv4_p[4],
  uint16_t      remote_port
  )
  {
  if (m_remote_ide_p->m_socket_p)
    {
    SkDebug::print(
      "\nSkookumIDE: Connect attempt made to this IDE, but a different runtime is already connected!\n",
      SkLocale_local,
      SkDPrintType_warning);

    if (!ADialogOS::confirm(
      "A new runtime is attempting to connect while a different runtime is already connected!\n\n"
      "The SkookumIDE can only be connected to one runtime at once though you can switch back and fourth.\n\n"
      "Disconnect current runtime and connect to the new runtime?",
      "SkookumIDE: Replace runtime connection?",
      ADialogOS::Flag_disable_win,
      ADialogOS::Icon_warning))
      {
      // Ignore new runtime
      SkDebug::print(
        "  ...new runtime connect attempt ignored.\n\n",
        SkLocale_local,
        SkDPrintType_warning);
      return nullptr;
      }

    // Replace previous runtime with new runtime
    SkDebug::print(
      "  ...disconnecting current runtime and connecting to new runtime.\n\n",
      SkLocale_local,
      SkDPrintType_warning);

    m_remote_ide_p->disconnect();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SkDebug::print("SkookumIDE: Connecting to runtime.\n", SkLocale_local);
  m_remote_ide_p->set_connect_state(SkRemoteIDE::ConnectState_connecting);
  m_remote_ide_p->init_socket(new ASocket(this, remote_ipv4_p, remote_port));

  return m_remote_ide_p->m_socket_p;
  }

//---------------------------------------------------------------------------------------
// See ASocketServer::on_client_disconnect()
// Author(s):   Conan Reis
void SkIDEServer::on_client_disconnect(ASocket * client_p)
  {
  SkDebug::print(a_str_format("SkookumIDE: Disconnected from runtime%s.\n",
    client_p->get_friendly_name().is_empty() ? "" : (" \"" + client_p->get_friendly_name() + "\"").as_cstr()),
    SkLocale_local);

  // Did we disconnect from all clients?
  if (!m_remote_ide_p->m_socket_p)
    {
    if (m_remote_ide_p->m_mode == SkLocale_ide)
      {
      m_remote_ide_p->server_display();
      }

    m_remote_ide_p->set_connect_state(SkRemoteIDE::ConnectState_disconnected);
    }
  }

//---------------------------------------------------------------------------------------
// Event called when the *server* socket has an error to resolve.
// Returns:    true if default handling of error should be performed (assert and
//             disconnect socket) or false if error is handled in some custom way.
// See:        set_error_func(), asocket_resolve_error(), ASocket::on_error() for client
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkIDEServer::on_server_error(uint error_id)
  {
  #ifdef ASOCKET_TRACE
    ADebug::print(A_SOURCE_FUNC_STR "\n", false);
  #endif

  if (error_id == WSAEADDRINUSE)
    {
    SkDebug::print("\nSkookumIDE: TCP/IP port already in use!\n\n", SkLocale_local, SkDPrintType_error);

    // $Revisit - CReis Might want to do something more than just shutdown immediately
    //SkConsole::ms_console_p->on_close_attempt();
    }

  m_remote_ide_p->set_connect_state(SkRemoteIDE::ConnectState_disconnected);
  m_remote_ide_p->set_mode(SkLocale_embedded);

  return false;
  }
