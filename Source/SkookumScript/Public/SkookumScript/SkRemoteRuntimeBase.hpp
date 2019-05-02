// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Remote Runtime
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkRemoteBase.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkBrain.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

#ifdef SKOOKUM_REMOTE

//---------------------------------------------------------------------------------------
// Skookum remote runtime communication commands that are specific to the client runtime.
class SK_API SkRemoteRuntimeBase : public SkRemoteBase
  {
  friend class SkDebug;

  public:

  // Public Class Data

    static SkRemoteRuntimeBase * ms_client_p;

  // Common Methods

    SkRemoteRuntimeBase();
    ~SkRemoteRuntimeBase();

    //---------------------------------------------------------------------------------------
    // Determines amount of time elapsed time in seconds (from some consistent start time at
    // or before its first call like: start of system, start of app launch, etc.)
    //
    // Notes:
    //   Used to time system / debug wait intervals.
    //   Should only be used as a difference between an earlier call.
    //   The double has sufficient precision such that it can be used to represent elapsed
    //   milliseconds without error until 285,421 years have passed..
    virtual double get_elapsed_seconds() = 0;

    //---------------------------------------------------------------------------------------
    // Supply information about current project
    // 
    // See: SkRemoteRuntimeBase::cmd_compiled_state()
    virtual void get_project_info(SkProjectInfo * out_project_info_p) = 0;

    //---------------------------------------------------------------------------------------
    // Brute force spawn of remote IDE. Called when connect attempts don't work and it is
    // assumed that the remote IDE is not running.
    // 
    // Returns:  true if remote IDE started false if not (due to some error, etc.)
    virtual bool spawn_remote_ide() = 0;

    //---------------------------------------------------------------------------------------
    // Blocking 1/10 second (100 millisecond) wait for current thread/message handle as
    // appropriate for current platform and project.
    //
    // Notes:
    //   Should also update SkRemoteRuntimeBase derived objects if they aren't updated on
    //   one or more separate threads. This can be done by calling concurrent process updates
    //   like message handlers or by calling a custom SkRemoteRuntimeBase object's update
    //   method directly.
    virtual void wait_for_update() = 0;

    bool      attempt_connect(double connect_time_out_seconds, bool authenticate = true, bool spawn_ide = true);
    bool      ensure_connected(double connect_time_out_seconds);
    eAConfirm ensure_compiled();
    bool      is_suspended() const;
    void      suspend();
    void      resume();

  // Commands

    // *Note all these commands do byte swapping as necessary since they could be run on
    // the client of any platform.

    void cmd_version_reply(uint8_t server_version, uint32_t authenticate_seed);
    void cmd_compiled_state(bool freshen);
    void cmd_show(eAFlag show_flag, ASymbol focus_class_name = ASymbol::ms_null, ASymbol focus_member_name = ASymbol::ms_null, bool is_data_member = false, bool focus_member_class_scope = false);
    void cmd_make_editable_reply(const AString & error_msg, const SkProjectInfo & project_info);
    void cmd_incremental_update_reply(bool is_success, tSkSessionGUID session_guid, tSkRevision revision);
    bool cmd_recompile_classes(SkClass * class_p, bool recurse, bool wait_reply = true);
    void cmd_ready_to_debug();
    void cmd_breakpoint_hit(const SkBreakPoint & bp, const SkCallStack * callstack_p);
    void cmd_break_expression(const SkMemberExpression & expr_info, const SkCallStack * callstack_p);

  protected:

  // Internal Methods

    bool wait_connect(double time_out_seconds);
    bool wait_authenticate();

  // Events

    void         on_cmd_authenticate();
    virtual void on_cmd_make_editable();
    void         on_cmd_debug_preferences(const void ** binary_pp);
    virtual void on_cmd_freshen_compiled_reply(eCompiledState state);
    void         on_cmd_incremental_update(const void ** binary_pp, uint32_t data_length);
    void         on_cmd_hierarchy_update(const void ** binary_pp);
    void         on_cmd_class_update(const void ** binary_pp);
    void         on_cmd_recompile_classes_reply(const void ** binary_pp);

    void         on_cmd_breakpoint_update(SkBreakPoint * bp_p, SkBreakPoint::eUpdate action);
    void         on_cmd_break_continue();
    void         on_cmd_break_step(SkDebug::eStep step_type);
    void         on_cmd_break_print_callstack();
    void         on_cmd_break_print_locals();

    virtual bool on_cmd_recv(eCommand cmd, const uint8_t * data_p, uint32_t data_length) override;

    virtual void on_breakpoint_hit(const SkBreakPoint & bp, const SkCallStack * callstack_p, SkObjectBase * scope_p, SkInvokedBase * caller_p);
    virtual void on_break_expression(const SkMemberExpression & expr_info, const SkCallStack * callstack_p);

    virtual void on_connect_change(eConnectState old_state) override;

  // Data Members

    eCompiledState m_remote_binaries;

    // Indicates whether an attempt was made to spawn the remote IDE
    bool m_spawning_ide;

    // Indicates whether runtime execution is supposed to be suspended or not.  A count is
    // used rather than a Boolean since suspend calls could overlap or be nested.  Do not
    // test this value directly - call is_suspended().
    mutable uint32_t m_suspend_count;

    // Name of pending remote command
    AString m_pending_name;

    // Number of remote sub-commands/actions expected before an encapsulating/higher-order
    // command is complete.
    uint32_t m_pending_count;

    // Number of pieces/classes/members etc. involved in some commands
    uint32_t m_part_count;

    // Number of errors that occurred in some commands
    uint32_t m_error_count;

  };  // SkRemoteRuntimeBase

#endif  // SKOOKUM_REMOTE


//=======================================================================================
// Inline Methods
//=======================================================================================
