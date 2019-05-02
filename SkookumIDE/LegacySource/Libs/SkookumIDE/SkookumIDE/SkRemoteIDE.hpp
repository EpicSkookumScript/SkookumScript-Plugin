// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript Remote IDE
//=======================================================================================


#ifndef __SKREMOTEIDE_HPP
#define __SKREMOTEIDE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO/ASocketServer.hpp>
#include <SkookumScript/SkRemoteBase.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Forward declaration
class SkIDEServer;

//---------------------------------------------------------------------------------------
// Skookum remote IDE communication commands that are specific to the server IDE.
class SkRemoteIDE : public SkRemoteBase
  {
  friend class SkIDEServer;

  public:
  // Nested Structures

    enum eCompiledFlag
      {
      CompiledFlag_notify   = 1 << 0,
      CompiledFlag_freshen  = 1 << 1,

      CompiledFlag_ignore   = 0x0
      };


  // Common Methods

    SkRemoteIDE();
    ~SkRemoteIDE();

    AIPAddress   get_ini_ip_address();
    void         set_flags(uint32_t flags)                        { m_compiled_flags = flags; }

    virtual void set_mode(eSkLocale mode);
    virtual bool is_connected() const                             { return this && (m_mode != SkLocale_embedded) && m_socket_p && m_socket_p->is_connected(); }
    bool         is_remote_compile_req()                          { return m_compiled_flags != CompiledFlag_ignore; }
    virtual void disconnect();

    virtual bool should_class_ctors_be_called() const;

    void server_remote_enable(bool remote = true);
    bool is_server_remote_enabled();

  // Commands

    // *Note all these commands do not bother with byte swapping since they are run on the
    // PC IDE server.
    // 
    void     cmd_version();
    void     cmd_authenticate();
    void     cmd_debug_preferences();
    void     cmd_make_editable();
    void     cmd_compiled_state_reply(eCompiledState state);
    void     cmd_hierarchy_update(SkClass * class_p, bool recurse = true);
    void     cmd_class_update(SkClass * class_p, bool recurse = true);
    uint32_t cmd_recompile_classes_reply(SkClass * class_p, bool recurse, uint32_t error_count = 0u);

    void     cmd_breakpoint_create(const SkBreakPoint & bp);
    void     cmd_breakpoint_update(SkBreakPoint * bp_p, SkBreakPoint::eUpdate action);
    void     cmd_breakpoint_update_all();
    void     cmd_break_continue();
    void     cmd_break_step(SkDebug::eStep step_type);
    void     cmd_print_callstack();
    void     cmd_print_locals();

  protected:

    void init_socket(ASocket * socket_p);
    void close_socket();
    void server_connect();
    void server_display();

  // Events

    void on_authenticate_client(uint8_t client_version, uint32_t auth_id, uint32_t client_flags, const SkProjectInfo & proj_info);
    void on_cmd_freshen_compiled(const SkProjectInfo & project_info, bool freshen);
    void on_cmd_make_editable_reply(const AString & error_msg, const SkProjectInfo & project_info);
    void on_cmd_recompile_classes(const ASymbol & class_name, bool recurse);
    void on_cmd_breakpoint_hit(SkBreakPoint * bp_p);
    void on_cmd_break_expression(const SkMemberExpression & expr_info);

    virtual void          on_cmd_invoke_result(AString & result_str) override;
    virtual eSendResponse on_cmd_send(const ADatum & datum);
    virtual bool          on_cmd_recv(eCommand cmd, const uint8_t * data_p, uint32_t data_length);

    virtual void on_connect_change(eConnectState old_state);

    void on_connect(ASocket * socket_p);
    void on_disconnect(ASocket * socket_p);
    void on_incoming(ADatum * datum_p);
    void on_error(ASocketError * error_p);

  // Telemetry

    void transmit_telemetry_connect(const AString & engine_id_string, const AString & platform_id_string);
    void on_telemetry_connect(ASocket * socket_p);
    void on_telemetry_disconnect(ASocket * socket_p);
    void on_telemetry_error(ASocketError * error_p);

  // Data Members

    // Server listening on localhost (127.0.0.1)
    SkIDEServer * m_local_server_p;

    // Server listening on specific IP address for remote connections
    SkIDEServer * m_remote_server_p;

    // True if there is a remote server listening
    bool m_remote_server;

    ASocket * m_socket_p;
    ASocket * m_socket_disconnecting_p;
    uint32_t  m_auth_id;  // Authentication id

    ASocket * m_telemetry_socket_p;
    AString   m_telemetry_request;

    // See eCompiledFlag 
    uint32_t m_compiled_flags;

  };  // SkRemoteIDE


//=======================================================================================
// Inline Methods
//=======================================================================================


#endif  // __SKREMOTEIDE_HPP

