// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Remote IDE Commands
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

//#pragma optimize("", off)

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkRemoteBase.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/ADatum.hpp>
#include <AgogCore/AFunctionArg.hpp>
#include <AgogCore/ASymbolTable.hpp>


//=======================================================================================
// Global Variables
//=======================================================================================

namespace
{

} // End unnamed namespace


//=======================================================================================
// SkRemoteBase Class Data
//=======================================================================================

SkRemoteBase * SkRemoteBase::ms_default_p = nullptr;

// Remote communication protocol version.
// Value between 0 and 255 and it should be safe to cycle back to 0 once 255 is passed.
const uint8_t SkRemoteBase::ms_version = 8u;


//=======================================================================================
// SkRemoteBase Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkRemoteBase::SkRemoteBase() :
  m_mode(SkLocale_embedded),
  m_connect_state(ConnectState_disconnected),
  m_remote_flags(SkRemoteFlag_none)
  {
  if (ms_default_p == nullptr)
    {
    ms_default_p = this;
    }
  }

//---------------------------------------------------------------------------------------
// Set remote connection mode.
// Author(s):   Conan Reis
void SkRemoteBase::set_mode(eSkLocale mode)
  {
  if (m_mode != mode)
    {
    A_ASSERTX((mode <= SkLocale_ide) || (mode == SkLocale_all), "Invalid Skookum Remote mode!");

    // $Revisit - CReis Call class ctors or dtors as needed when switching from and to remote IDE mode

    m_remote_flags = SkRemoteFlag_none;

    m_mode = mode;
    }
  }

//---------------------------------------------------------------------------------------
// Disconnect client runtime
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkRemoteBase::disconnect()
  {
  if (m_connect_state < ConnectState_disconnecting)
    {
    set_connect_state(ConnectState_disconnecting);

    if (m_mode == SkLocale_runtime)
      {
      set_mode(SkLocale_embedded);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Indicates whether class constructors should be called at the start of the
//             Skookum session or not.
//             [The IDE may want to skip calling class constructors since they may contain
//             engine/runtime calls which may be unavailable to the remote IDE.]
// Returns:    true if class constructors should be called or false if not.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkRemoteBase::should_class_ctors_be_called() const
  {
  return true;
  }


#ifdef SKOOKUM_REMOTE

//---------------------------------------------------------------------------------------
// Notifies the remote side of a simple command that has no associated data.
//             Use this rather than making a bunch of commands that are otherwise identical.
// Arg         cmd - remote command type
// Author(s):   Conan Reis
void SkRemoteBase::cmd_simple(eCommand cmd)
  {
  if (!is_authenticated())
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id

  ADatum  datum(4u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Notifies the remote side of a simple command that has a uint32_t as associated
//             data.
//             Use this rather than making a bunch of commands that are otherwise identical.
// Arg         cmd - remote command type
// Arg         value - 4 bytes of data associated with command
// Author(s):   Conan Reis
void SkRemoteBase::cmd_simple_uint32(eCommand cmd, uint32_t value)
  {
  if (!is_authenticated())
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Binary composition:
  //   4 bytes - command id
  //   4 bytes - value

  ADatum  datum(4u + 4u);
  void *  data_p  = (void *)datum.get_data_writable();
  void ** data_pp = &data_p;

  // 4 bytes - command id
  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // 4 bytes - value
  A_BYTE_STREAM_OUT32(data_pp, &value);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Send a ping command with no associated data.  This is just a test to see if
//  the connection is still open.
// 
// #Author(s): Stephen Johnson
SkRemoteBase::eSendResponse SkRemoteBase::cmd_ping_test()
  {
  ADatum     datum(static_cast<uint32_t>(sizeof(uint32_t)));
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_ping_test;
  
  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  return on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Print string on remote client
// Arg         str - string to print.
// Arg         type - see eSkDPrintType
// Author(s):   Conan Reis
void SkRemoteBase::cmd_print(const AString & str, uint32_t type)
  {
  uint32_t   str_len = str.get_length();
  ADatum     datum(static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint8_t) + str_len));
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_print;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  // eSkDPrintType - should only be 1 byte
  uint8_t type_byte = uint8_t(type);

  A_BYTE_STREAM_OUT8(data_pp, &type_byte);

  ::memcpy(data_p, str.as_cstr(), str_len);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Invoke code on remote side
// Arg         code - script code to invoke.
// Author(s):   Conan Reis
void SkRemoteBase::cmd_invoke(const AString & code)
  {
  uint32_t   str_len = code.get_length();
  ADatum     datum(static_cast<uint32_t>(sizeof(uint32_t)) + str_len);
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_invoke;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  ::memcpy(data_p, code.as_cstr(), str_len);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Return result of invoke in string form.
// 
// Params: 
//   str: string result
//   
// Author(s):  Conan Reis
void SkRemoteBase::cmd_invoke_result(const AString & str)
  {
  uint32_t   str_len = str.get_length();
  ADatum     datum(static_cast<uint32_t>(sizeof(uint32_t)) + str_len);
  uint8_t *  data_p  = datum.get_data_writable();
  uint8_t ** data_pp = &data_p;

  uint32_t cmd = Command_invoke_result;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  ::memcpy(data_p, str.as_cstr(), str_len);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
// Send local symbols to remote side to merge with existing remote symbols
// Arg         syms - symbols to merge into remote side
// Author(s):   Conan Reis
void SkRemoteBase::cmd_symbols_update(const ASymbolTable & syms)
  {
  uint32_t  syms_len = syms.as_binary_length();
  ADatum    datum(static_cast<uint32_t>(sizeof(uint32_t)) + syms_len);
  uint8_t * data_p  = datum.get_data_writable();
  void ** data_pp = (void **)&data_p;

  uint32_t cmd = Command_symbol_update;

  A_BYTE_STREAM_OUT32(data_pp, &cmd);

  syms.as_binary(data_pp);

  on_cmd_send(datum);
  }

//---------------------------------------------------------------------------------------
void SkRemoteBase::on_cmd_invoke_result(AString & result_str)
  {
  AString log_str(nullptr, result_str.get_length() + 3u, 0u);

  log_str.append('\n');
  log_str.append(result_str);
  log_str.append('\n');
  SkDebug::print(log_str, SkLocale_local, SkDPrintType_result | SkDPrintType_flag_remote);
  }

//---------------------------------------------------------------------------------------
// Remote print
// Arg         str - string to print
// Arg         type_flags - see eSkDPrintType
// See:        cmd_print()
// Author(s):   Conan Reis
void SkRemoteBase::on_cmd_print(AString & str, uint32_t type_flags)
  {
  #if defined(A_SYMBOL_STR_DB_AGOG)
    // $Revisit - CReis This could be made ASymbolTable agnostic
    //          - also unless symbols are in sync might be good to always translate

    // Translate symbol hashes as needed.
    if (!is_symbol_db_remote())
      {
      // Convert [known] symbol ids if any found
      ASymbolTable::ms_main_p->translate_ids(&str);
      }

  #endif

  SkDebug::print(str, SkLocale_local, type_flags | SkDPrintType_flag_remote);
  }

//---------------------------------------------------------------------------------------
// Process received command.
// Author(s):   Conan Reis
bool SkRemoteBase::on_cmd_recv(eCommand cmd, const uint8_t * data_p, uint32_t data_length)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for and reject unauthenticated commands
  if (!is_authenticated() && (cmd > Command__auth_limit))
    {
    // Reject unauthenticated command
    ADebug::print_format("SkookumScript: Remote side sent command before authentication - ignored!  [Cmd Id: %u]\n", cmd);
    return true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse common commands
  const void ** data_pp = (const void **)&data_p;

  switch (cmd)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Command_ping_test:
      {
      // no response required for the ping test - it's the sender's connection test
      }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Command_print:
      {
      uint32_t type = uint32_t(A_BYTE_STREAM_UI8_INC(data_pp));

      AString str((const char *)data_p, data_length - sizeof(uint8_t), false);

      on_cmd_print(str, type);
      }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Command_invoke:
      // Ignore if parser is not available
      #if (SKOOKUM & SK_CODE_IN)
        {
        AString code((const char *)data_p, data_length, false);
        AString result_str;

        SkParser::invoke_script(code, &result_str);
        cmd_invoke_result(result_str);
        }
      #endif
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Command_invoke_result:
      {
      AString result_str((const char *)data_p, data_length, false);
      on_cmd_invoke_result(result_str);
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Command_symbol_update:
      ASymbol::table_from_binary(data_pp);
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    default:
      return false;
    }

  return true;
  }

#endif  // SKOOKUM_REMOTE


//---------------------------------------------------------------------------------------
// Translate actual locale codes (embedded/runtime/IDE) to virtual locale
//             codes (local/remote).
// Returns:    virtual locale code (local/remote)
// Arg         locale - if it is an actual code it is converted to its appropriate virtual
//             code.
// See:        locale_as_actual()
// Modifiers:   static
// Author(s):   Conan Reis
eSkLocale SkRemoteBase::locale_as_virtual(eSkLocale locale)
  {
  return (locale <= SkLocale_ide)
    ? ((ms_default_p) && (ms_default_p->get_mode() == SkLocale_ide)
      ?  ((locale == SkLocale_runtime) ? SkLocale_remote : SkLocale_local)  // Is IDE
      :  ((locale == SkLocale_ide) ? SkLocale_remote : SkLocale_local))     // Is Runtime
    : locale;  // Does not need conversion
  }

//---------------------------------------------------------------------------------------
// Translate virtual locale codes (local/remote) to actual locale codes
//             (embedded/runtime/IDE).
// Returns:    actual locale code (embedded/runtime/IDE)
// Arg         locale - if it is a virtual code it is converted to its appropriate actual
//             code.
// See:        locale_as_virtual()
// Modifiers:   static
// Author(s):   Conan Reis
eSkLocale SkRemoteBase::locale_as_actual(eSkLocale locale)
  {
  if ((locale >= SkLocale_local) && (locale != SkLocale_all))
    {
    eSkLocale mode_current  = ms_default_p ? ms_default_p->get_mode() : SkLocale_embedded;
    eSkLocale mode_opposite = (mode_current != SkLocale_embedded)
      ? ((mode_current == SkLocale_runtime) ? SkLocale_ide : SkLocale_runtime)
      : SkLocale_embedded;

    return (locale == SkLocale_local) ? mode_current : mode_opposite;
    }

  // Does not need conversion
  return locale;
  }

//---------------------------------------------------------------------------------------
// Sets the effective state of the runtime - IDE connection
// Arg         state - new connection state
// See:        on_connect_change()
// Author(s):   Conan Reis
void SkRemoteBase::set_connect_state(eConnectState state)
  {
  eConnectState old_state = m_connect_state;

  if (old_state != state)
    {
    m_connect_state = state;
    on_connect_change(old_state);
    }
  }

//---------------------------------------------------------------------------------------
// Called when connection state changes.  Useful as a hook for UI notification.
// Arg         old_state - previous connection state (use m_connect_state for current)
// See:        set_connect_state()
// Notes:      Called by set_connect_state() - do not call directly.
// Modifiers:   virtual - override for custom behaviour.
// Author(s):   Conan Reis
void SkRemoteBase::on_connect_change(eConnectState old_state)
  {
  //ADebug::print_format("SkookumScript: on_connect_change(%u->%u)\n", old_state, m_connect_state);
  }

//---------------------------------------------------------------------------------------
// Called whenever a class is live updated
void SkRemoteBase::on_class_updated(SkClass * class_p)
  {
  //ADebug::print_format("SkookumScript: on_class_updated(%s)\n", class_p->get_name_cstr_dbg());

  struct Nested
    {
    static void clear_object_id_lookup_cache(SkClass * cls_p)
      {
      cls_p->object_id_lookup_clear_cache();
      }
    };

  AFunctionArg<SkClass *> class_func(&Nested::clear_object_id_lookup_cache);

  // Methods may have been changed, added or removed so get rid of any cached methods
  class_p->iterate_recurse(&class_func);
  }

//#pragma optimize("g", on)

//---------------------------------------------------------------------------------------

uint32_t SkProjectInfo::as_binary_length() const
  {
  return m_engine_id.as_binary_length()
       + m_platform_id.as_binary_length()
       + m_project_name.as_binary_length()
       + m_project_path.as_binary_length()
       + m_default_project_path.as_binary_length();
  }

//---------------------------------------------------------------------------------------

void SkProjectInfo::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkProjectInfo::as_binary_length());

  m_engine_id.as_binary(binary_pp);
  m_platform_id.as_binary(binary_pp);
  m_project_name.as_binary(binary_pp);
  m_project_path.as_binary(binary_pp);
  m_default_project_path.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------

void SkProjectInfo::assign_binary(const void ** binary_pp)
  {
  m_engine_id.assign_binary(binary_pp);
  m_platform_id.assign_binary(binary_pp);
  m_project_name.assign_binary(binary_pp);
  m_project_path.assign_binary(binary_pp);
  m_default_project_path.assign_binary(binary_pp);
  }

