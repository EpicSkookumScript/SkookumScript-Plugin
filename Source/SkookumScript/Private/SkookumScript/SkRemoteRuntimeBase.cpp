// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Remote Runtime
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

//#pragma optimize("", off)


#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkRemoteRuntimeBase.hpp>

#ifdef SKOOKUM_REMOTE

#include <AgogCore/ADatum.hpp>
#include <AgogCore/ARandom.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>

#ifdef A_PLAT_PC
  #define WIN32_LEAN_AND_MEAN // Keep this define out of public header files
  #include "Windows/AllowWindowsPlatformTypes.h"
  #include "Windows/PreWindowsApi.h"
    #include <windows.h> // AllowSetForegroundWindow
  #include "Windows/PostWindowsApi.h"
  #include "Windows/HideWindowsPlatformTypes.h"
#endif

//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // Max amount of time to wait (in 1/10 second) / repeats to attempt
  const double g_wait_connect_attempt_seconds =  5.0;
  const double g_wait_connect_seconds         =  6.0;
  const double g_wait_authenticate_seconds    = 20.0;  // Could be doing initial compile
  const double g_wait_compile_seconds         = 60.0;

} // End unnamed namespace


//=======================================================================================
// SkRemoteRuntimeBase Methods
//=======================================================================================

SkRemoteRuntimeBase * SkRemoteRuntimeBase::ms_client_p = nullptr;


//=======================================================================================
// SkRemoteRuntimeBase Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkRemoteRuntimeBase::SkRemoteRuntimeBase() :
  m_remote_binaries(CompiledState_unknown),
  m_spawning_ide(false),
  m_suspend_count(0u),
  m_pending_count(0u),
  m_part_count(0u),
  m_error_count(0u)
  {
  ms_client_p = this;
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkRemoteRuntimeBase::~SkRemoteRuntimeBase()
  {
  set_mode(SkLocale_embedded);
  ms_client_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Blocking wait until connected.
// Returns:    true if authenticated false if failed
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::wait_connect(double time_out_seconds)
  {
  double timeout = get_elapsed_seconds() + time_out_seconds;

  // Wait until connected or timeout
  A_LOOP_INFINITE
    {
    if (m_connect_state != ConnectState_connecting)
      {
      if (m_spawning_ide)
        {
        // Reset spawn flag
        m_spawning_ide = m_connect_state >= ConnectState_authenticated;
        }

      return (m_connect_state <= ConnectState_authenticated);
      }

    if (timeout <= get_elapsed_seconds())
      {
      break;
      }

    wait_for_update();
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Blocking wait until authenticated.
// Returns:    true if authenticated false if failed
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::wait_authenticate()
  {
  double timeout = get_elapsed_seconds() + g_wait_authenticate_seconds;

  // Wait until authenticated or timeout
  A_LOOP_INFINITE
    {
    if (is_authenticated())
      {
      return true;
      }

    if (m_connect_state >= ConnectState_disconnecting)
      {
      return false;
      }

    if (timeout <= get_elapsed_seconds())
      {
      break;
      }

    wait_for_update();
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Attempts to connect *once* and if it fails it assumes that the remote IDE
//             is not running so it begins the process of starting up the remote IDE so
//             that future connect attempts - such as with ensure_connected() - may succeed.
// Arg         authenticate - if true and the remote IDE is already connected then wait
//             until also fully authenticated before returning.
// Returns:    true if connected and false if not
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::attempt_connect(
  double connect_time_out_seconds,
  bool authenticate, // = true
  bool spawn_ide // = true
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Attempt to connect
  set_mode(SkLocale_runtime);

  if (wait_connect(connect_time_out_seconds))
    {
    if (authenticate)
      {
      return wait_authenticate();
      }

    return true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Execute Skookum Remote IDE if failed to connect
  if (spawn_ide && !m_spawning_ide)
    {
    m_spawning_ide = spawn_remote_ide();
    }

  // Did not immediately connect to Remote IDE - though it may now be starting up
  return false;
  }

//---------------------------------------------------------------------------------------
// If not already connected it tries to connect / reconnect
// Returns:    true if connected and false if not
bool SkRemoteRuntimeBase::ensure_connected(double connect_time_out_seconds)
  {
  // If we not currently connected make sure we are truly disconnected first
  if (!is_authenticated())
    {
    disconnect();
    }

  double timeout = get_elapsed_seconds() + g_wait_connect_attempt_seconds;

  A_LOOP_INFINITE
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Attempt to connect
    set_mode(SkLocale_runtime);

    // Wait until connected and authenticated
    if (wait_connect(connect_time_out_seconds))
      {
      return wait_authenticate();
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Execute Skookum Remote IDE if failed to connect and try to reconnect
    if (!m_spawning_ide)
      {
      m_spawning_ide = spawn_remote_ide();

      if (!m_spawning_ide)
        {
        timeout = 0.0;
        }
      }

    if (timeout <= get_elapsed_seconds())
      {
      break;
      }

    // Wait for IDE to start-up
    wait_for_update();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Assume spawn failed

  // Reset spawn flag
  m_spawning_ide = false;

  SkDebug::print_agog("\nSkookum RT: Could not start/connect to IDE!\n", SkLocale_local, SkDPrintType_error);

  return false;
  }

//---------------------------------------------------------------------------------------
// Send command to remote IDE to ensure that the compiled binaries are up-to-
//             date and do a blocking wait until confirmation is received.
// Returns:    true if remote IDE says that compiled binaries are up-to-date and false if
//             they are stale or have errors.
// Author(s):   Conan Reis
eAConfirm SkRemoteRuntimeBase::ensure_compiled()
  {
  if (!is_authenticated())
    {
    return AConfirm_abort;
    }

  cmd_compiled_state(true);

  double timeout = get_elapsed_seconds() + g_wait_compile_seconds;

  // Wait until get compiled reply or timeout
  while ((m_remote_binaries < CompiledState_stale)
    && (timeout > get_elapsed_seconds()))
    {
    wait_for_update();
    }

  if ((m_remote_binaries < CompiledState_stale) || !is_authenticated())
    {
    return AConfirm_abort;
    }

  return (m_remote_binaries == CompiledState_fresh)
    ? AConfirm_yes
    : AConfirm_no;
  }

//---------------------------------------------------------------------------------------
// Determines if the execution state is supposed to be suspended or not.
// See:        suspend(), resume() 
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::is_suspended() const
  {
  if (m_suspend_count)
    {
    // Resume if authentication is lost.
    // $Revisit - CReis It might be preferable to stay suspended and to allow IDE to
    // reconnect in a suspended state.
    if (!is_authenticated())
      {
      m_suspend_count = 0u;
      }
    }

  return m_suspend_count != 0u;
  }

//---------------------------------------------------------------------------------------
// Increments the runtime suspension counter
// See:        resume(), is_suspended()
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::suspend()
  {
  m_suspend_count++;
  }

//---------------------------------------------------------------------------------------
// Decrements the runtime suspension counter
// See:        suspend(), is_suspended()
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::resume()
  {
  if (m_suspend_count)
    {
    m_suspend_count--;
    }
  }

//---------------------------------------------------------------------------------------
// Send version info reply
// Arg         server_version - version of the server
// Arg         authenticate_seed - authentication id seed value
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::cmd_version_reply(uint8_t server_version, uint32_t authenticate_seed)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check version
  SkDebug::print("Skookum RT: Authenticating version...\n", SkLocale_local);

  if (server_version != ms_version)
    {
    SkDebug::print("Skookum RT: Incorrect version!\n", SkLocale_local);
    disconnect();
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Generate authentication hash id
  ARandom auth_rand(authenticate_seed);

  auth_rand.uniform_ui();
  auth_rand.uniform_ui();
  
  uint32_t auth_id = auth_rand.uniform_ui();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Send version reply
  SkProjectInfo project_info;
  SkRemoteRuntimeBase::ms_client_p->get_project_info(&project_info);
  uint32_t data_length =
      SkRemote_version_reply_byte_size
    + project_info.as_binary_length();
  ADatum   datum(data_length);
  void *   data_p  = datum.get_data_writable();
  void **  data_pp = &data_p;

  uint32_t cmd = Command_version_reply;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  A_BYTE_STREAM_OUT8(data_pp, &SkRemoteBase::ms_version);

  A_BYTE_STREAM_OUT32(data_pp, &auth_id);

  uint32_t client_flags = SkRemoteFlag_none;

  #ifdef A_SYMBOL_STR_DB
    //#pragma A_LOG("Symbols converted to strings by client.")
    client_flags |= SkRemoteFlag_symbol_db;
  //#else
    //#pragma A_LOG("Symbols *not* converted to strings by client!")
  #endif

  A_BYTE_STREAM_OUT32(data_pp, &client_flags);

  // Transmit project info
  project_info.as_binary((void**)data_pp);

  SkDebug::print("Skookum RT: Sending authentication back.\n", SkLocale_local);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Determine/ensure the project (specific script overlays, etc.) used by this runtime is
// loaded into the IDE and the compiled binary is up-to-date and optionally freshen it
// (recompile) as needed.
// 
// Params:
//   freshen:
//     recompile if true and out-of-date or get current state of compiled binary if false
void SkRemoteRuntimeBase::cmd_compiled_state(bool freshen)
  {
  if ((m_remote_binaries == CompiledState_determining) || (m_remote_binaries == CompiledState_compiling))
    {
    // Already working on it.
    return;
    }

  m_remote_binaries = CompiledState_determining;

  // Binary composition:
  //   4 bytes - command id
  //   1 byte  - freshen bool
  //   n bytes - project info
  SkProjectInfo project_info;
  SkRemoteRuntimeBase::ms_client_p->get_project_info(&project_info);
  ADatum  datum(uint32_t(sizeof(uint32_t) + sizeof(uint8_t) + project_info.as_binary_length()));
  void *  data_p  = datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_freshen_compiled;
  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 1 byte  - freshen bool
  uint8_t value8 = uint8_t(freshen);
  A_BYTE_STREAM_OUT8(data_pp, &value8);

  // n bytes - project info
  project_info.as_binary(data_pp);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Show/hide/toggle SkookumIDE
// Arg         show - if to show or hide IDE
// Arg         focus_class_name - optional class to switch focus to
// Arg         focus_member_name - optional member of that class (method/coroutine/data member) to switch focus to
// Arg         is_data_member - true if data member, false if invokable (method, coroutine etc.)
// Arg         focus_member_class_scope - true if class member, false if instance member
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::cmd_show(
  eAFlag show_flag,
  ASymbol focus_class_name,
  ASymbol focus_member_name,
  bool is_data_member,
  bool focus_member_class_scope
  )
  {
  bool reconnected = false;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Abort if not connected
  if (!is_authenticated())
    {
    reconnected = true;

    if (!ensure_connected(g_wait_connect_seconds))
      {
      return;
      }

    show_flag = AFlag_on;
    }

  #ifdef A_PLAT_PC
    // Allow other apps to use SetForegroundWindow() 
    ::AllowSetForegroundWindow(uint32_t(-1)); // ASFW_ANY
  #endif

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ADatum     datum(static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint8_t) + 2 * sizeof(uint32_t) + 2 * sizeof(uint8_t)));
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_show;
  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // $Revisit - CReis Should use a SkMemberInfo structure instead

  uint8_t show_flag8 = (uint8_t)show_flag;
  A_BYTE_STREAM_OUT8(data_pp, &show_flag8);

  uint32_t focus_class_name_id = focus_class_name.get_id();
  A_BYTE_STREAM_OUT32(data_pp, &focus_class_name_id);

  uint32_t focus_member_name_id = focus_member_name.get_id();
  A_BYTE_STREAM_OUT32(data_pp, &focus_member_name_id);

  uint8_t is_data_member8 = (uint8_t)is_data_member;
  A_BYTE_STREAM_OUT8(data_pp, &is_data_member8);

  uint8_t focus_member_class_scope8 = (uint8_t)focus_member_class_scope;
  A_BYTE_STREAM_OUT8(data_pp, &focus_member_class_scope8);

  on_cmd_send(datum);

  if (reconnected)
    {
    if (m_remote_binaries == CompiledState_fresh)
      {
      cmd_ready_to_debug();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tell remote IDE about success of making the project editable
void SkRemoteRuntimeBase::cmd_make_editable_reply(const AString & error_msg, const SkProjectInfo & project_info)
  {
  uint32_t data_length = 4u + error_msg.as_binary_length() + project_info.as_binary_length();

  ADatum   datum(data_length);
  void *   data_p = datum.get_data_writable();
  void **  data_pp = &data_p;

  uint32_t cmd = Command_make_editable_reply;
  A_BYTE_STREAM_OUT32(data_pp, &cmd);
  error_msg.as_binary(data_pp);
  project_info.as_binary(data_pp);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Reply to IDE if the incremental update was successful
// Also send current revision, 0 if session GUID didn't match
void SkRemoteRuntimeBase::cmd_incremental_update_reply(bool is_success, tSkSessionGUID session_guid, tSkRevision revision)
  {
  uint32_t data_length = 4u + 1u + sizeof(tSkSessionGUID) + sizeof(tSkRevision);

  ADatum   datum(data_length);
  void *   data_p = datum.get_data_writable();
  void **  data_pp = &data_p;

  uint32_t cmd = Command_incremental_update_reply;
  A_BYTE_STREAM_OUT32(data_pp, &cmd);
  A_BYTE_STREAM_OUT8(data_pp, &is_success);
  A_BYTE_STREAM_OUT64(data_pp, &session_guid);
  A_BYTE_STREAM_OUT32(data_pp, &revision);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Tell remote IDE to recompile the specified class (and optional subclasses)
//             and optionally wait (pausing execution) until the results of the recompile
//             are available.
// Returns:    true if successfully recompiled, false if not
// Arg         class_p - class to recompile (also indicates superclass if recurse set)
// Arg         recurse - if true the subclasses of class_p will also be recompiled
// Arg         wait_reply - if true execution is paused until the recompile results are
//             received.
// See:        SkRemoteIDE::on_cmd_recompile_classes()
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::cmd_recompile_classes(
  SkClass * class_p,
  bool      recurse,
  bool      wait_reply // = true
  )
  {
  m_pending_name.empty();
  m_pending_count = 0u;
  m_part_count    = 0u;
  m_error_count   = 0u;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure connected and abort if cannot connect.
  if (!is_authenticated())
    {
    if (!ensure_connected(g_wait_connect_seconds))
      {
      return false;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about wait
  if (wait_reply) 
    {
    SkDebug::print_agog(
      "\nSkookum RT: Suspending execution.\n"
      "  Waiting for recompile results from IDE...\n",
      SkLocale_all,
      SkDPrintType_warning);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   4 bytes - name of class to recompile
  //   1 byte  - recurse subclasses (true) or not (false)
  
  ADatum    datum(9u);
  uint8_t * data_p  = datum.get_data_writable();
  void **   data_pp = (void **)&data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_recompile_classes;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 4 bytes - name of class recompiled
  class_p->get_name().as_binary(data_pp);

  // 1 byte - recurse subclasses (true) or not (false)
  A_BYTE_STREAM_OUT8(data_pp, &recurse);

  on_cmd_send(datum);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wait for results?
  if (!wait_reply) 
    {
    // Don't wait - assume all good
    return true;
    }

  suspend();

  do 
    {
    while (m_pending_name.is_empty() && is_suspended())
      {
      wait_for_update();
      }

    if (!is_suspended())
      {
      SkDebug::print_agog(
        "Skookum RT: Recompile aborted!\n",
        SkLocale_all,
        SkDPrintType_warning);

      return false;
      }

    if (m_error_count)
      {
      // Must have been errors
      m_pending_name.empty();
      m_pending_count = 0u;
      m_part_count    = 0u;
      m_error_count   = 0u;
      }
    }
  while (m_part_count == 0u);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Waiting for compiled binaries from Remote IDE
  SkDebug::print_agog(
    "\nSkookum RT: Recompile successful!\n"
    "  Waiting for recompiled binaries from IDE...\n");

  while (m_pending_count && is_suspended())
    {
    wait_for_update();
    }

  if (m_pending_count && !is_suspended())
    {
    SkDebug::print_agog(
      a_str_format("Skookum RT: Still to receive %u binaries and resuming execution!\n", m_pending_count),
      SkLocale_all,
      SkDPrintType_warning);

    return false;
    }

  SkDebug::print_agog(
    "\nSkookum RT: Recompiled class binaries received.\n"
    "  Resuming execution.\n");

  resume();

  return true;
  }

//---------------------------------------------------------------------------------------
// Called to notify the IDE that the runtime has loaded the compiled binaries and is
// ready to receive debugging information such as breakpoints.
// 
// Author(s): Conan Reis
void SkRemoteRuntimeBase::cmd_ready_to_debug()
  {
  SkDebug::print_agog("Skookum RT: Ready for remote debugging.\n");

  cmd_simple(Command_ready_to_debug);
  }

//---------------------------------------------------------------------------------------
// Called when a breakpoint is hit.  Notifies the remote IDE that a
//             breakpoint has been hit.
// Arg         bp - breakpoint that has been hit
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::cmd_breakpoint_hit(const SkBreakPoint & bp, const SkCallStack * callstack_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   2 bytes - breakpoint table index

  ADatum  datum(4u + 2u + callstack_p->as_binary_length());
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_breakpoint_hit;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 2 bytes - table index
  uint16_t bp_idx = uint16_t(bp.m_table_idx);

  A_BYTE_STREAM_OUT16(data_pp, &bp_idx);

  // n bytes - callstack
  callstack_p->as_binary(data_pp);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Called when a expression break is hit and notifies the remote IDE.
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::cmd_break_expression(const SkMemberExpression & expr_info, const SkCallStack * callstack_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //    4 bytes - command id
  //   11 bytes - member expression info

  ADatum  datum(4u + expr_info.as_binary_length() + callstack_p->as_binary_length());
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  uint32_t cmd = Command_break_expression;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 11 bytes - member expression info
  expr_info.as_binary(data_pp);

  // n bytes - callstack
  callstack_p->as_binary(data_pp);

  on_cmd_send(datum);
  }

void SkRemoteRuntimeBase::cmd_project()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get update project info
  SkProjectInfo project_info;
  SkRemoteRuntimeBase::ms_client_p->get_project_info(&project_info);

  // Command + project info
  uint32_t data_length = 4u + project_info.as_binary_length();
  ADatum   datum(data_length);
  void *   data_p = datum.get_data_writable();
  void **  data_pp = &data_p;

  uint32_t cmd = Command_project;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // Transmit project info
  project_info.as_binary((void**)data_pp);

  SkDebug::print("Skookum RT: Sending project info to IDE.\n", SkLocale_local);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
void SkRemoteRuntimeBase::on_cmd_authenticate()
  {
  // Client is now authenticated.  Note: If server ever rejects the socket will be
  // disconnected and auto-unauthenticate.
  set_connect_state(ConnectState_authenticated);
  }

//---------------------------------------------------------------------------------------
// Default implementation
void SkRemoteRuntimeBase::on_cmd_make_editable()
  {
  SK_ERRORX("Runtime asked by iDE to make project editable but this function is not implemented!");
  }

//---------------------------------------------------------------------------------------
void SkRemoteRuntimeBase::on_cmd_debug_preferences(const void ** binary_pp)
  {
  // Binary composition:
  //   4 bytes - Debug preference flags - see SkDebug::ePrefFlag
  //   1 byte  - indent size
  //   1 byte  - tab stop size
  
  // 4 bytes - preference flags see SkDebug::ePrefFlag
  SkDebug::set_preferences(A_BYTE_STREAM_UI32_INC(binary_pp));

  // 1 byte - indent size
  SkDebug::ms_indent_size = A_BYTE_STREAM_UI8_INC(binary_pp);

  // 1 byte - tab stop size
  SkDebug::ms_tab_stops = A_BYTE_STREAM_UI8_INC(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Called whenever the state of the compiled binary is received.
// Arg         state - see eCompiledState
// See:        cmd_compiled_state()
// Modifiers:   virtual - overridden from SkRemoteBase
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_freshen_compiled_reply(eCompiledState state)
  {
  m_remote_binaries = state;
  }

//---------------------------------------------------------------------------------------
// Called when the IDE sends a patch to update the program
void SkRemoteRuntimeBase::on_cmd_incremental_update(const void ** binary_pp, uint32_t data_length)
  {
  // Bail if no binaries loaded yet
  if (SkookumScript::get_initialization_level() < SkookumScript::InitializationLevel_program)
    {
    // Request load of binaries at this point as they have obviously successfully compiled
    m_remote_binaries = CompiledState_fresh;

    // And notify IDE that we couldn't process this update
    cmd_incremental_update_reply(false, 0, 0);

    return;
    }

  // Get the patch information
  tSkSessionGUID  session_guid  = A_BYTE_STREAM_UI64_INC(binary_pp);
  tSkRevision     from_revision = A_BYTE_STREAM_UI32_INC(binary_pp);
  tSkRevision     to_revision   = A_BYTE_STREAM_UI32_INC(binary_pp);

  // Check if session GUID matches
  if (session_guid != SkBrain::ms_session_guid)
    {
    // If no match and non-zero revision, request a new patch
    if (from_revision != 0)
      {
      cmd_incremental_update_reply(false, SkBrain::ms_session_guid, SkBrain::ms_revision);
      return;
      }

    // If revision is 0, i.e. a full force update, 
    // force a new session guid and revision onto the program
    SkBrain::ms_session_guid = session_guid;
    SkBrain::ms_revision = 0;
    }

  A_SCOPED_BINARY_SIZE_SANITY_CHECK((void**)binary_pp, data_length - 16u);

  // Receive symbol table
  ASymbol::table_from_binary(binary_pp);

  // Apply patch
  APSorted<SkClass> updated_classes;
  uint32_t length;
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Receive added/modified classes
  length = A_BYTE_STREAM_UI32_INC(binary_pp);
  for (; length; --length)
    {
    SkClass * superclass_p = SkClass::from_binary_ref(binary_pp);
    ASymbol name = ASymbol::create_from_binary(binary_pp);
    SkClass * class_p = SkBrain::get_class(name);
    if (!class_p)
      {
      class_p = SkBrain::create_class(name, superclass_p);
      }
    else if (class_p->get_superclass() != superclass_p)
      {
      SK_ERRORX(a_str_format("Class '%s' was reparented in the IDE, but the incremental update algorithm currently does not support this. Therefore, the runtime will not reflect the change until the next time the compiled binaries are reloaded.", name.as_cstr()));
      }

    // Assign binary in reparse mode
    class_p->reparse_begin(false);
    class_p->assign_binary(binary_pp, false);
    class_p->reparse_end();

    // Re-resolve raw data
    class_p->resolve_raw_data();

  #if 0
    // $Vital - CReis If the class is an actor/mind probably need to either:
    //   1) get rid of all instances of this class
    //   2) get rid of all running coroutines for this class
    //   3) go through all invoked objects and reset their cached pointers

    // If last class in set call their class constructors
    if (m_pending_count <= 1u)
      {
      class_p = SkBrain::get_class(ASymbol::create(m_pending_name, ATerm_short));

      if (m_part_count <= 1)
        {
        SkDebug::print_agog(a_str_format(
          "\n\nCalling class constructor (if it exists) for recompiled '%s'...\n",
          m_pending_name.as_cstr()));

        class_p->invoke_class_ctor();
        }
      else
        {
        SkDebug::print_agog(a_str_format(
          "\n\nCalling class constructors (if any exist) for recompiled '%s'\n"
          "and its %u subclass%s...\n",
          m_pending_name.as_cstr(),
          m_part_count - 1u,
          (m_part_count > 2u) ? "es" : ""));

        class_p->invoke_class_ctor_recurse();
        }

      SkDebug::print_agog("  ...done!\n");
      }
  #endif

    // Remember that this class was updated
    updated_classes.append(*class_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Receive deleted classes
  length = A_BYTE_STREAM_UI32_INC(binary_pp);
  for (; length; --length)
    {
    ASymbol name = ASymbol::create_from_binary(binary_pp);
    SkClass * class_p = SkBrain::get_class(name);
    SK_ASSERTX(class_p, a_str_format("Class `%s` requested to delete does not exist.", name.as_cstr_dbg()));
    // TODO - actually delete it!
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Receive added/modified routines
  SkRoutineUpdateRecord * routine_update_record_p = nullptr;
  SkClassDescBase::enable_compound_refs(false);
  length = A_BYTE_STREAM_UI32_INC(binary_pp);
  for (; length; --length)
    {
    ASymbol class_name = ASymbol::create_from_binary(binary_pp);
    SkClass * class_p = SkBrain::get_class(class_name);
    SK_ASSERTX(class_p, a_str_format("Request recieved to add a routine to class `%s` which does not exist.", class_name.as_cstr_dbg()));

    // Keep record of changes to existing routines
    if (!routine_update_record_p)
      {
      routine_update_record_p = new SkRoutineUpdateRecord;
      }

    bool is_class_member = !!A_BYTE_STREAM_UI8_INC(binary_pp);
    // Peek ahead at the invoke type which comes first in the routine binary
    eSkInvokable invoke_type = eSkInvokable(**((const uint8_t **)binary_pp));
    switch (invoke_type)
      {
        case SkInvokable_method:
        case SkInvokable_method_func:
        case SkInvokable_method_mthd:
          if (is_class_member)
            {
            class_p->append_class_method(binary_pp, routine_update_record_p);
            }
          else
            {
            class_p->append_instance_method(binary_pp, routine_update_record_p);
            }
          break;

        case SkInvokable_coroutine:
        case SkInvokable_coroutine_func:
        case SkInvokable_coroutine_mthd:
          SK_ASSERTX(!is_class_member, "Coroutines cannot be class members.");
          class_p->append_coroutine(binary_pp, routine_update_record_p);
          break;

        default:
          SK_ASSERTX(false, "Received routine with illegal invoke type.");
          break;
      }

    // Was an existing routine updated?
    if (routine_update_record_p->m_routine_p)
      {
      SkClassUpdateRecord * class_update_record_p = SkookumScript::get_program_update_record()->get_or_create_class_update_record(class_name);
      class_update_record_p->m_updated_routines.append(*routine_update_record_p);
      routine_update_record_p = nullptr;
      }

    // Remember that a member of this class was added or updated
    updated_classes.append_absent(*class_p);
    }
  SkClassDescBase::enable_compound_refs();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Receive deleted routines
  length = A_BYTE_STREAM_UI32_INC(binary_pp);
  for (; length; --length)
    {
    SkMemberInfo member_info(binary_pp);

    // Keep record of routine deletions
    if (!routine_update_record_p)
      {
      routine_update_record_p = new SkRoutineUpdateRecord;
      }

    switch (member_info.m_type)
      {
        case SkInvokable_method:
        case SkInvokable_method_func:
        case SkInvokable_method_mthd:
          if (member_info.m_class_scope)
            {
            routine_update_record_p->m_previous_routine_p = member_info.get_class()->find_class_method(member_info.m_member_id.get_name());
            member_info.get_class()->unlink_class_method(member_info.m_member_id.get_name());
            }
          else
            {
            routine_update_record_p->m_previous_routine_p = member_info.get_class()->find_instance_method(member_info.m_member_id.get_name());
            member_info.get_class()->unlink_instance_method(member_info.m_member_id.get_name());
            }
          break;

        case SkInvokable_coroutine:
        case SkInvokable_coroutine_func:
        case SkInvokable_coroutine_mthd:
          SK_ASSERTX(!member_info.m_class_scope, "Coroutines cannot be class members.");
          routine_update_record_p->m_previous_routine_p = member_info.get_class()->find_coroutine(member_info.m_member_id.get_name());
          member_info.get_class()->unlink_coroutine(member_info.m_member_id.get_name());
          break;

        default:
          SK_ASSERTX(false, "Received routine with illegal invoke type.");
          break;
      }

    // Was a routine deleted?
    if (routine_update_record_p->m_previous_routine_p)
      {
      SkClassUpdateRecord * class_update_record_p = SkookumScript::get_program_update_record()->get_or_create_class_update_record(member_info.get_class()->get_name());
      class_update_record_p->m_updated_routines.append(*routine_update_record_p);
      routine_update_record_p = nullptr;
      }

    // Remember that a member of this class was removed
    updated_classes.append_absent(*member_info.get_class());
    }

  // Clean up
  if (routine_update_record_p)
    {
    delete routine_update_record_p;
    routine_update_record_p = nullptr;
    }

  // After update, set new revision
  SkBrain::ms_revision = to_revision;

  // Report back to IDE that we were successful
  cmd_incremental_update_reply(true, session_guid, to_revision);

  // Announce to the runtime which classes where updated
  for (SkClass * class_p : updated_classes)
    {
    on_class_updated(class_p);
    }
  }

//---------------------------------------------------------------------------------------
// Update class hierarchy
// Arg         binary_pp - byte stream to parse for class hierarchy info
// See:        cmd_hierarchy_update()
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_hierarchy_update(const void ** binary_pp)
  {
  // Binary composition:
  //   4 bytes - #classes
  //   4 bytes - name of class       \_ repeating
  //   4 bytes - name of superclass  /

  // 4 bytes - #classes
  uint32_t class_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  while (class_count)
    {
    // 4 bytes - name of class
    ASymbol class_name(ASymbol::create_from_binary(binary_pp));

    // 4 bytes - name of superclass
    ASymbol superclass_name(ASymbol::create_from_binary(binary_pp));

    if (class_name != ASymbol_Object && SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_program)
      {
      SkBrain::create_class(class_name, superclass_name, ADef_uint32);
      }
 
    class_count--;
    }
  }

//---------------------------------------------------------------------------------------
// Receive update of class (and optionally any subclasses).
// This could be sent directly from IDE or as a reply to earlier freshen compiled request.
// 
// Params:
//   binary_pp: byte stream to parse for class hierarchy info
//   
// See:        cmd_hierarchy_update()
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_class_update(const void ** binary_pp)
  {
  // Bail if no binaries loaded yet
  if (SkookumScript::get_initialization_level() < SkookumScript::InitializationLevel_program)
    {
    SK_ERRORX("Cannot remote update class when no binaries are loaded yet.");
    return;
    }

  SkClass * class_p = SkClass::from_binary_ref(binary_pp);

  ADebug::print_format("Skookum RT: Received update for class '%s'\n", class_p->get_name_cstr_dbg());

  // Assign binary in reparse mode
  class_p->reparse_begin(true);
  class_p->assign_binary(binary_pp, false);
  class_p->reparse_end();

  // Re-resolve raw data
  class_p->resolve_raw_data();

  // $Vital - CReis If the class is an actor/mind probably need to either:
  //   1) get rid of all instances of this class
  //   2) get rid of all running coroutines for this class
  //   3) go through all invoked objects and reset their cached pointers

  // If last class in set call their class constructors
  if (m_pending_count <= 1u)
    {
    class_p = SkBrain::get_class(ASymbol::create(m_pending_name, ATerm_short));

    if (m_part_count <= 1)
      {
      SkDebug::print_agog(a_str_format(
        "\n\nCalling class constructor (if it exists) for recompiled '%s'...\n",
        m_pending_name.as_cstr()));

      class_p->invoke_class_ctor();
      }
    else
      {
      SkDebug::print_agog(a_str_format(
        "\n\nCalling class constructors (if any exist) for recompiled '%s'\n"
        "and its %u subclass%s...\n",
        m_pending_name.as_cstr(),
        m_part_count - 1u,
        (m_part_count > 2u) ? "es" : ""));

      class_p->invoke_class_ctor_recurse();
      }

    SkDebug::print_agog("  ...done!\n");
    }

  // Announce that this class was updated
  on_class_updated(class_p);

  // Decrement done after calling class constructors since other actions may be waiting to
  // trigger on m_pending_count == 0.
  if (m_pending_count)
    {
    m_pending_count--;
    }

  if (m_pending_count == 0)
    {
    cmd_ready_to_debug();
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever the remote IDE has recompiled one or more classes and may be sending
// class updates to this runtime.
void SkRemoteRuntimeBase::on_cmd_recompile_classes_reply(const void ** binary_pp)
  {
  // name of class recompiled - not using a symbol since it might not exist yet
  // 4 bytes - string length
  // n bytes - string
  m_pending_name.assign_binary(binary_pp);

  // 1 byte  - successful recompile (true) or not (false)
  bool compile_success = bool(A_BYTE_STREAM_UI8_INC(binary_pp) > 0u);

  // 4 bytes - number of classes to receive or # of errors
  uint32_t count = A_BYTE_STREAM_UI32_INC(binary_pp);

  if (compile_success)
    {
    // Detach existing breakpoints
    SkDebug::breakpoint_release_all();

    m_pending_count = count;
    m_part_count    = count;
    m_error_count   = 0u;
    }
  else
    {
    m_pending_count = 0u;
    m_part_count    = 0u;
    m_error_count   = count;
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever the remote IDE has changed the state of an existing debug breakpoint.
// 
// Params:
//   bp_p:   address of breakpoint to update or if nullptr update all breakpoints
//   action: update action to perform
// Author(s):  Conan Reis
void SkRemoteRuntimeBase::on_cmd_breakpoint_update(
  SkBreakPoint *        bp_p,
  SkBreakPoint::eUpdate action
  )
  {
  switch (action)
    {
    case SkBreakPoint::Update_enable:
      if (bp_p)
        {
        bp_p->enable();
        }
      else
        {
        SkDebug::breakpoint_enable_all();
        }
      break;

    case SkBreakPoint::Update_disable:
      if (bp_p)
        {
        bp_p->enable(false);
        }
      else
        {
        SkDebug::breakpoint_disable_all();
        }
      break;

    case SkBreakPoint::Update_remove:
      if (bp_p)
        {
        bp_p->remove();
        }
      else
        {
        SkDebug::breakpoint_remove_all();
        }
      break;
    }
  }

//---------------------------------------------------------------------------------------
// Continue from a break
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_break_continue()
  {
  SkDebug::print_agog(
    "Skookum RT: Execution resumed.\n",
    SkLocale_all,
    SkDPrintType_trace);

  SkDebug::invalidate_next_expression();
  resume();
  }

//---------------------------------------------------------------------------------------
// Continue until the next step point
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_break_step(SkDebug::eStep step_type)
  {
  SkDebug::step(step_type);
  resume();
  }

//---------------------------------------------------------------------------------------
// Prints out callstack for current break location.
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_break_print_callstack()
  {
  AString str('\n');

  SkDebug::append_callstack_string(&str, SkDebug::get_next_invokable());
  str.append('\n');
  SkDebug::print_agog(str, SkLocale_all, SkDPrintType_trace);
  }

//---------------------------------------------------------------------------------------
// Prints out local variables for current break location.
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_cmd_break_print_locals()
  {
  AString str('\n');

  SkDebug::append_locals_string(&str, SkDebug::get_next_invokable(), SkDebug::get_next_invokable());
  SkDebug::print_agog(str, SkLocale_all, SkDPrintType_trace);
  }

//---------------------------------------------------------------------------------------
// Called when an expression breakpoint is hit - communicates to remote IDE
//             that this Runtime has hit a breakpoint.
// Arg         bp_p - breakpoint that was just hit
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_breakpoint_hit(const SkBreakPoint & bp, const SkCallStack * callstack_p, SkObjectBase * scope_p, SkInvokedBase * caller_p)
  {
  // Suspend execution - called before cmd_breakpoint_hit() in case the call is blocking
  // and the result is returned immediately.
  suspend();

  // Inform IDE
  cmd_breakpoint_hit(bp, callstack_p);

  int counter = 0;

  // Wait until resumed
  while (is_suspended())
    {
    wait_for_update();

    if ((++counter & 0x1f) == 0)
      {
      if (cmd_ping_test() != SendResponse_OK)
        {
        resume();
        break;
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Called when an expression breakpoint is hit - communicates to remote IDE
//             that this Runtime has hit a breakpoint.
// Arg         bp_p - breakpoint that was just hit
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_break_expression(const SkMemberExpression & expr_info, const SkCallStack * callstack_p)
  {
  // Suspend execution - called before cmd_breakpoint_hit() in case the call is blocking
  // and the result is returned immediately.
  suspend();

  // Inform IDE
  cmd_break_expression(expr_info, callstack_p);

  int counter = 0;

  // Wait until resumed
  while (is_suspended())
    {
    wait_for_update();

    if ((++counter & 0x1f) == 0)
      {
      if (cmd_ping_test() != SendResponse_OK)
        {
        resume();
        break;
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Receive and parse/execute a command from the remote side
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkRemoteRuntimeBase::on_cmd_recv(eCommand cmd, const uint8_t * data_p, uint32_t data_length)
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
      case Command_version:
        {
        if (data_length != SkRemote_version_byte_size - sizeof(uint32_t))
          {
          disconnect();
          break;
          }

        uint8_t version = A_BYTE_STREAM_UI8_INC(data_pp);

        cmd_version_reply(version, A_BYTE_STREAM_UI32_INC(data_pp));
        }
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_authenticate:
        on_cmd_authenticate();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_disconnect:
        // Ensure no breakpoints when IDE is not connected
        SkDebug::breakpoint_remove_all();
        disconnect();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_make_editable:
        on_cmd_make_editable();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_preferences:
        on_cmd_debug_preferences(data_pp);
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_freshen_compiled_reply:
        on_cmd_freshen_compiled_reply(eCompiledState(A_BYTE_STREAM_UI8_INC(data_pp)));
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_incremental_update:
        on_cmd_incremental_update(data_pp, data_length);
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_class_hierarchy_update:
        on_cmd_hierarchy_update(data_pp);
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_class_update:
        on_cmd_class_update(data_pp);
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_recompile_classes_reply:
        on_cmd_recompile_classes_reply(data_pp);
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_breakpoint_create:
        {
        // Binary composition:
        //   11 bytes - member expression
        //    1 byte  - enabled
        //    2 bytes - table index

        // 11 bytes - member expression
        SkMemberExpression bp_info(data_pp);

        // 1 byte - enabled
        bool enabled = A_BYTE_STREAM_UI8_INC(data_pp) != 0u;

        // 2 bytes - table index
        uint32_t table_idx = A_BYTE_STREAM_UI16_INC(data_pp);

        // Add the breakpoint
        SkDebug::breakpoint_append(bp_info, table_idx, enabled);

        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_breakpoint_update:
        {
        // Binary composition:
        //   2 bytes - breakpoint table index or SkDebugInfo::Flag_debug_idx__none to indicate all
        //   1 byte  - update action

        // 2 bytes - breakpoint table index or SkDebugInfo::Flag_debug_idx__none to indicate all
        SkBreakPoint * bp_p = SkDebug::breakpoint_get_at_idx(A_BYTE_STREAM_UI16_INC(data_pp));

        // 1 byte - update action
        on_cmd_breakpoint_update(bp_p, SkBreakPoint::eUpdate(A_BYTE_STREAM_UI8_INC(data_pp)));

        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_break_continue:
        on_cmd_break_continue();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_break_step:
        on_cmd_break_step(SkDebug::eStep(A_BYTE_STREAM_UI8_INC(data_pp)));
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_break_print_callstack:
        on_cmd_break_print_callstack();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      case Command_break_print_locals:
        on_cmd_break_print_locals();
        break;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      default:
        // Send error to both sides
        if (cmd < Command__last)
          {
          SkDebug::print_agog(
            a_str_format(
              "Skookum RT: Command from IDE not implemented on runtime!  [Cmd Id: %u]\n", cmd),
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
    SkDebug::print_agog(
      a_str_format(
        "Skookum RT: Unknown command sent from IDE to runtime!  [Cmd Id: %u]\n", cmd),
      SkLocale_all,
      SkDPrintType_error);
    }

  return cmd_processed;
  }

//---------------------------------------------------------------------------------------
// Called when connection state changes.  Useful as a hook for UI notification.
// Arg         old_state - previous connection state (use m_connect_state for current)
// See:        set_connect_state()
// Notes:      Called by set_connect_state() - do not call directly.
// Modifiers:   virtual - overridden from SkRemoteBase
// Author(s):   Conan Reis
void SkRemoteRuntimeBase::on_connect_change(eConnectState old_state)
  {
  //ADebug::print_format("Skookum RT: on_connect_change(%u->%u)\n", old_state, m_connect_state);

  #if (SKOOKUM & SK_DEBUG)
    if ((old_state == ConnectState_authenticated)
      && (m_connect_state >= ConnectState_disconnecting))
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Breakpoints sent on reconnect
      // Remove breakpoints on disconnect
      SkDebug::breakpoint_remove_all();
      }
  #endif
  }

#endif  // SKOOKUM_REMOTE


//#pragma optimize("g", on)

