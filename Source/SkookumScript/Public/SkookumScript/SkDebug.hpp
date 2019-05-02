// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Debugging and error handling classes
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFreePtr.hpp>
#include <AgogCore/AFunctionBase.hpp>
#include <AgogCore/AIdPtr.hpp>
#include <AgogCore/AList.hpp>
#include <AgogCore/APArray.hpp>
#include <AgogCore/APSorted.hpp>
#include <SkookumScript/SkMemberInfo.hpp>
#include <SkookumScript/SkRemoteBase.hpp>
#include <SkookumScript/SkParser.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#if (SKOOKUM & SK_DEBUG)

  #define SK_DPRINTF(_msg, ...)                                     ADebug::print_format(_msg, __VA_ARGS__)

  #define SK_ASSERTX(_boolean_exp, _ex_desc)                        A_VERIFY(_boolean_exp, _ex_desc, AErrId_generic, SkDebug)
  #define SK_ASSERTX_NO_THROW(_boolean_exp, _ex_desc)               A_VERIFY_NO_THROW(_boolean_exp, _ex_desc, AErrId_generic, SkDebug)
  #define SK_ASSERT(_boolean_exp, _ex_desc, _ExClass)               A_VERIFY(_boolean_exp, _ex_desc, AErrId_generic, _ExClass)
  #define SK_ASSERT_INVOKED(_boolean_exp, _ex_desc)                 A_VERIFY(_boolean_exp, (SkDebugInfoSetter(), _ex_desc), AErrId_generic, SkDebug)
  #define SK_ASSERT_INFO(_boolean_exp, _ex_desc, _info)             A_VERIFY(_boolean_exp, (SkDebugInfoSetter(_info), _ex_desc), AErrId_generic, SkDebug)
  #define SK_ASSERT_MEMORY(_ex_desc, _ExClass)                      A_VERIFY_MEMORY(_ex_desc, _ExClass)
  #define SK_ASSERT_ID(_boolean_exp, _ex_desc, _err_id, _ExClass)   A_VERIFY(_boolean_exp, _ex_desc, _err_id, _ExClass)
  #define SK_ERRORX(_ex_desc)                                       A_ERROR(_ex_desc, AErrId_generic, SkDebug)
  #define SK_ERROR(_ex_desc, _ExClass)                              A_ERROR(_ex_desc, AErrId_generic, _ExClass)
  #define SK_ERROR_INVOKED(_ex_desc)                                A_ERROR((SkDebugInfoSetter(), _ex_desc), AErrId_generic, SkDebug)
  #define SK_ERROR_INFO(_ex_desc, _info)                            A_ERROR((SkDebugInfoSetter(_info), _ex_desc), AErrId_generic, SkDebug)
  #define SK_ERROR_ID(_ex_desc, _err_id, _ExClass)                  A_ERROR(_ex_desc, _err_id, _ExClass)

  // Mad asserts are for extra checking for coders & advanced developers
  // Any condition asserted by mad asserts MUST NOT BE FATAL and MUST BE 100% RECOVERABLE
  #ifdef A_MAD_CHECK
    #define SK_MAD_ASSERTX(_boolean_exp, _ex_desc)                  A_VERIFY(_boolean_exp, _ex_desc, AErrId_generic, SkDebug)
    #define SK_MAD_ASSERTX_NO_THROW(_boolean_exp, _ex_desc)         A_VERIFY_NO_THROW(_boolean_exp, _ex_desc, AErrId_generic, SkDebug)
    #define SK_MAD_ERRORX(_ex_desc)                                 A_ERROR(_ex_desc, AErrId_generic, SkDebug)
  #else
    #define SK_MAD_ASSERTX(_boolean_exp, _ex_desc)                  (void(0))
    #define SK_MAD_ASSERTX_NO_THROW(_boolean_exp, _ex_desc)         (void(0))
    #define SK_MAD_ERRORX(_ex_desc)                                 (void(0))
  #endif

  // Store current call so rest of engine can know if in the middle of a script call or not.
  #define SKDEBUG_STORE_CALL(_scope_p)                              SkInvokedContextBase * _old_call_p = SkDebug::ms_current_call_p; SkDebug::ms_current_call_p = _scope_p;
  #define SKDEBUG_RESTORE_CALL()                                    SkDebug::ms_current_call_p = _old_call_p;

  // Setting debug info for invoked method/coroutine.  Called internally so there is no info.
  #define SKDEBUG_ICALL_SET_INTERNAL(_icontext_p)                   (_icontext_p)->m_source_idx = 0u; (_icontext_p)->m_debug_info = uint16_t(SkDebugInfo::Flag__default);

  // Setting debug info for invoked method/coroutine.  Copy from previously stored global expression values.  Done via globals rather than passing expression as an argument so the arguments are consistent across build targets.
  #define SKDEBUG_ICALL_SET_GEXPR(_icontext_p)                      (_icontext_p)->m_source_idx = SkInvokedContextBase::ms_last_expr_p->m_source_idx; (_icontext_p)->m_debug_info = SkInvokedContextBase::ms_last_expr_p->m_debug_info;

  #define SKDEBUG_ICALL_SET_EXPR(_icontext_p, _expr_p)              (_icontext_p)->m_source_idx = _expr_p->m_source_idx; (_icontext_p)->m_debug_info = _expr_p->m_debug_info;

  // Store expression debug info for invoked methods/coroutines.  Done via globals rather than passing expression as an argument so the arguments are consistent across build targets.
  #define SKMEMORY_ARGS(_Class, _debug_bytes)                       #_Class, sizeof(_Class) - (_debug_bytes), _debug_bytes

#else  // SK_DEBUG not defined

  // Note, using (void(0)) avoids warning that ; is used without an expression.

  #define SK_DPRINTF(_msg, ...)                                     (void(0))

  #define SK_ASSERTX(_boolean_exp, _ex_desc)                        (void(0))
  #define SK_ASSERTX_NO_THROW(_boolean_exp, _ex_desc)               (void(0))
  #define SK_ASSERT(_boolean_exp, _ex_desc, _ExClass)               (void(0))
  #define SK_ASSERT_INVOKED(_boolean_exp, _ex_desc, _info)          (void(0))
  #define SK_ASSERT_INFO(_boolean_exp, _ex_desc, _info)             (void(0))
  #define SK_ASSERT_MEMORY(_ex_desc, _ExClass)                      (void(0))
  #define SK_ASSERT_ID(_boolean_exp, _ex_desc, _err_id, _ExClass)   (void(0))
  #define SK_ERRORX(_ex_desc)                                       (void(0))
  #define SK_ERROR(_ex_desc, _ExClass)                              (void(0))
  #define SK_ERROR_INVOKED(_ex_desc)                                (void(0))
  #define SK_ERROR_INFO(_ex_desc, _info)                            (void(0))
  #define SK_ERROR_ID(_ex_desc, _err_id, _ExClass)                  (void(0))

  #define SK_MAD_ASSERTX(_boolean_exp, _ex_desc)                    (void(0))
  #define SK_MAD_ASSERTX_NO_THROW(_boolean_exp, _ex_desc)           (void(0))
  #define SK_MAD_ERRORX(_ex_desc)                                   (void(0))

  #define SKDEBUG_STORE_CALL(_scope_p)                              (void(0))
  #define SKDEBUG_RESTORE_CALL()                                    (void(0))

  #define SKDEBUG_ICALL_SET_INTERNAL(_icontext_p)                   (void(0))
  #define SKDEBUG_ICALL_SET_GEXPR(_icontext_p)                      (void(0))
  #define SKDEBUG_ICALL_SET_EXPR(_icontext_p, _expr_p)              (void(0))

  #define SKMEMORY_ARGS(_Class, _debug_bytes)                       #_Class, sizeof(_Class), _debug_bytes

#endif  // (SKOOKUM & SK_DEBUG)


#if defined(SKDEBUG_COMMON)
  // Store expression debug info for next invoked method/coroutine.  Done via globals rather than passing expression as an argument so the arguments are consistent across build targets.
  #define SKDEBUG_ICALL_STORE_GEXPR(_expr_p)                        SkInvokedContextBase::ms_last_expr_p = _expr_p;
#else
  #define SKDEBUG_ICALL_STORE_GEXPR(_expr_p)                        (void(0))
#endif


//---------------------------------------------------------------------------------------
// Debug hooks for notifying when scripts start/stop various tasks so that things like
// tracing, profiling, breakpoints, etc. can be added.
#if defined(SKDEBUG_HOOKS)  // Normally defined in Sk.hpp

  // Called whenever a method is about to be invoked - see SkDebug::append_hook()
  #define SKDEBUG_HOOK_METHOD(_imethod_p)                           SkDebug::hook_method(_imethod_p)

  // Called whenever a coroutine is about to be invoked - see SkDebug::append_hook()
  #define SKDEBUG_HOOK_COROUTINE(_icoro_p)                           SkDebug::hook_coroutine(_icoro_p)

  // Called whenever the Skookum scripting system is about to be entered to execute some script code - see SkDebug::append_hook()
  #define SKDEBUG_HOOK_SCRIPT_ENTRY(_origin_id)                     SkDebug::hook_script_origin_push(_origin_id)

  // Called whenever the Skookum scripting system is about to be exited after having executed some script code - see SkDebug::append_hook()
  #define SKDEBUG_HOOK_SCRIPT_EXIT()                                SkDebug::hook_script_origin_pop()

#else  // SKDEBUG_HOOKS not defined

  // Note, using (void(0)) avoids warning that ; is used without an expression.

  #define SKDEBUG_HOOK_METHOD(_imethod_p)                           (void(0))
  #define SKDEBUG_HOOK_COROUTINE(_icoro_p)                          (void(0))
  #define SKDEBUG_HOOK_SCRIPT_ENTRY(_origin_id)                     (void(0))
  #define SKDEBUG_HOOK_SCRIPT_EXIT()                                (void(0))

#endif  // SKDEBUG_HOOKS


//---------------------------------------------------------------------------------------
// SKDEBUG_HOOK_EXPR() - Called whenever expression is about to be invoked and it has a
// debug action (like a breakpoint) - see SkDebug::hook_expression(),
// Needed for stepwise debugging so defined even when just `SK_DEBUG` is set and
// `SKDEBUG_HOOKS` is not defined.
#if (SKOOKUM & SK_DEBUG)
  
    #define SKDEBUG_HOOK_EXPR(_expr_p, _scope_p, _caller_p, _caller_caller_p, _hook_context) \
      if (SkDebug::ms_expr_hook_flag | ((_expr_p)->m_debug_info & SkDebugInfo::Flag_debug_enabled)) \
        { SkDebug::hook_expression((SkExpressionBase *)(_expr_p), (_scope_p), (_caller_p), (_caller_caller_p), (_hook_context)); } \
      (void(0))

#elif defined(SKDEBUG_HOOKS)

  #define SKDEBUG_HOOK_EXPR(_expr_p, _scope_p, _caller_p, _caller_caller_p, _hook_context) \
    if (SkDebug::ms_expr_hook_flag) \
      { SkDebug::hook_expression((SkExpressionBase *)(_expr_p), (_scope_p), (_caller_p), (_caller_caller_p), (_hook_context)); } \
    (void(0))

#else  // Neither (SKOOKUM & SK_DEBUG) nor SKDEBUG_HOOKS defined

  #define SKDEBUG_HOOK_EXPR(_expr_p, _scope_p, _caller_p, _caller_caller_p, _hook_context) \
    (void(0))

#endif


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkExpressionBase;
class SkInstance;
class SkInvokedBase;
class SkInvokedContextBase;
class SkInvokedMethod;
class SkInvokedCoroutine;
class SkObjectBase;
class SkQualifier;


//---------------------------------------------------------------------------------------
// Used when generating information for callstacks/local variables/etc.
enum eSkInvokeInfo
  {
  SkInvokeInfo_none           = 0x0,     // No extra info
  SkInvokeInfo_indent         = 1 << 0,  // Indent lines when building multi-line string
  SkInvokeInfo_skip_this      = 1 << 1,  // Skip the most recent/current call when string
  SkInvokeInfo_peek           = 1 << 2,  // The most current call is just a peek

  SkInvokeInfo_scope          = 1 << 3,  // Include the identifier scope
  SkInvokeInfo_args           = 1 << 4,  // Include argument values
  SkInvokeInfo_this           = 1 << 5,  // Include this/receiver value

  // Locals only info
  SkInvokeInfo_temporaries    = 1 << 6,  // Include local/temp variables
  SkInvokeInfo_captured       = 1 << 7,  // Include captured variables from closures
  SkInvokeInfo_instance_data  = 1 << 8,  // Include instance data members
  SkInvokeInfo_class_data     = 1 << 9,  // Include class data members

  SkInvokeInfo__locals_def    = SkInvokeInfo_scope | SkInvokeInfo_temporaries | SkInvokeInfo_captured | SkInvokeInfo_args | SkInvokeInfo_this | SkInvokeInfo_instance_data | SkInvokeInfo_class_data,

  // Callstack only info
  SkInvokeInfo_ignore_absent  = 1 << 10,  // If no call stack available do not print any info
  SkInvokeInfo_index          = 1 << 11, // Include caller source character index
  SkInvokeInfo_updater        = 1 << 12, // Include the updater for invoked coroutines
  SkInvokeInfo_depth          = 1 << 13, // Include the call depth
  // Update count for coroutines?
  // Pending count?
  // Expressions?
  
  SkInvokeInfo__callstack_def = SkInvokeInfo_scope | SkInvokeInfo_args | SkInvokeInfo_this | SkInvokeInfo_index | SkInvokeInfo_updater
  };


//---------------------------------------------------------------------------------------
// Debug/logging print "decoration" type.  Used by SkDebug:print() and SkConsole::log_append()
enum eSkDPrintType
  {
  SkDPrintType_standard,  // Skookum (white)
  SkDPrintType_title,     // Title / link (yellow)
  SkDPrintType_note,      // Notable action (green)
  SkDPrintType_system,    // C++ (electric blue)
  SkDPrintType_error,     // (red)
  SkDPrintType_warning,   // (orange)
  SkDPrintType_result,    // (light yellow)
  SkDPrintType_trace,     // (lavender)

  // Or-ed in flag that indicates that print comes from remote IDE - italics or slightly
  // different background.  If not present then locale is considered to be embedded/runtime.
  SkDPrintType_flag_remote   = 1 << 7,

  SkDPrintType_non_flag_mask = 0x7F

  // $Note - CReis Currently assuming this will fit in 1 byte.
  };


//---------------------------------------------------------------------------------------
// Type of script code/classes to include for memory statistics
enum eSkCodeSerialize
  {
  SkCodeSerialize_static,        // Only always loaded code
  SkCodeSerialize_static_demand  // Always loaded code + demand loaded code
  };


//---------------------------------------------------------------------------------------
struct SkPrintInfo
  {
  // Public Data Members

    AString  m_str;
    uint32_t m_type;  // See eSkDPrintType

  // Common Methods

    SkPrintInfo(const AString & str, uint type = SkDPrintType_system)
      : m_str(str), m_type(type)
      {}

  };

typedef AFunctionArgBase<const SkPrintInfo &> tSkPrintFunc;


const uint16_t SkExpr_char_pos_invalid = UINT16_MAX - 1u;

//---------------------------------------------------------------------------------------
struct SK_API SkDebugInfo
  {
  // Nested Structures

    // Describes how the bits of SkExpressionBase::m_debug_info are utilized.
    enum eFlag
      {
      // Debug index (11 bits) - Index into debug action table (0-2047).  Flag_debug_idx__none indicates no debug action set.
      Flag_debug_idx__mask   = (1 << 11) - 1,
      Flag_debug_idx__none   = Flag_debug_idx__mask,

      // Code Origin (4 bits) - Indicates the location where the code for this expression came from
      Flag_origin_internal   = 0x0,      // Code was hooked up internally in C++ or was generated
      Flag_origin_source     = 1 << 12,  // Code came from a standard source file (* default args may need more info)
      Flag_origin_alias,                 // Code came from an alias - origin info specifies alias
      Flag_origin_custom1,               // Code came from a custom location #1 such as a tool
      Flag_origin_custom2,               // Code came from a custom location #2 such as a tool
      Flag_origin_custom3,               // Code came from a custom location #3 such as a tool
      Flag_origin_custom4,               // Code came from a custom location #4 such as a tool
      Flag_origin_custom5,               // Code came from a custom location #5 such as a tool
      Flag_origin_custom6,               // Code came from a custom location #6 such as a tool
      Flag_origin_custom7,               // Code came from a custom location #7 such as a tool
      Flag_origin_custom8,               // Code came from a custom location #8 such as a tool
      Flag_origin_custom9,               // Code came from a custom location #9 such as a tool
      Flag_origin_custom10,              // Code came from a custom location #10 such as a tool
      Flag_origin_custom11,              // Code came from a custom location #11 such as a tool
      Flag_origin_custom12,              // Code came from a custom location #12 such as a tool
      Flag_origin_custom13,              // Code came from a custom location #13 such as a tool
      Flag_origin__mask      = ((1 << 15) - 1) - Flag_debug_idx__mask,

      // Debug enable (1 bit)  - Indicates a debug action to take when this expression is about to be evaluated
      Flag_debug_disabled    = 0x0,      // No special action to take
      Flag_debug_enabled     = 1 << 15,  // Examine debug index to determine action

      Flag__default        = Flag_debug_idx__none | Flag_origin_internal | Flag_debug_disabled,
      Flag__default_source = Flag_debug_idx__none | Flag_origin_source | Flag_debug_disabled
      };

  // Public Data

    // Source string character index position where this expression starts in the original
    // parsed code file/string.  m_debug_flags describes the source code origin.
    uint16_t m_source_idx; 

    // Debug flags & misc. info - such as breakpoint set flag - see eFlag
    uint16_t m_debug_flags;

    // Used for top level commands during creation of callstack
    static const SkExpressionBase * ms_expr_default_p;
    static const SkExpressionBase * ms_expr_p;

  // Methods

    void set_info(uint16_t source_idx, uint16_t debug_flags)  { m_source_idx = source_idx; m_debug_flags = debug_flags; }
    void set_internal()                                       { m_source_idx = 0u; m_debug_flags = Flag__default; }

    bool is_origin_source() const                             { return (m_debug_flags & Flag_origin__mask) == Flag_origin_source; }
    bool is_origin_internal() const                           { return (m_debug_flags & Flag_origin__mask) == Flag_origin_internal; }
    bool is_valid_origin_source() const                       { return ((m_debug_flags & Flag_origin__mask) == Flag_origin_source) && (m_source_idx != SkExpr_char_pos_invalid); }

  // Class Methods

    static SkExpressionBase * get_expr_default();
  };

// Dummy structure used to calculate size used by SkDebugInfo with respect to alignment
// and pointer/virtual table use, etc.
struct SkDebugInfoPtrDummyStruct
  {
  void * m_ptr_p;
  SkDebugInfo m_info;
  };

// Actual amount of bytes used by SkDebugInfo in structures like SkExpressionBase for use
// in methods like track_memory()
const size_t SkDebugInfo_size_used = sizeof(SkDebugInfoPtrDummyStruct) - sizeof(void *);


//---------------------------------------------------------------------------------------
// Used to provide extra debug info in SK_ASSERT_INFO() when the currently executing
// expression is known.
struct SK_API SkDebugInfoSetter
  {
  // Methods
    SkDebugInfoSetter();
    SkDebugInfoSetter(const SkExpressionBase & expr);
    SkDebugInfoSetter(SkInvokedBase * caller_p);
    ~SkDebugInfoSetter();
  };


//---------------------------------------------------------------------------------------
// Notes      Member Expression identifying information
// Author(s)  Conan Reis
class SK_API SkMemberExpression : public SkMemberInfo
  {
  public:

    // Common Methods

      SkMemberExpression()                                : m_expr_p(nullptr), m_source_idx(0u) {}
      SkMemberExpression(const SkMemberInfo & member_info, SkExpressionBase * expr_p = nullptr);
      SkMemberExpression(const SkMemberExpression & info) : SkMemberInfo(info), m_expr_p(info.m_expr_p), m_source_idx(info.m_source_idx) {}
      virtual ~SkMemberExpression() {}

    #if (SKOOKUM & SK_DEBUG)
      SkMemberExpression(SkObjectBase * scope_p, SkInvokedBase * caller_p, SkExpressionBase * expr_p) {}
    #endif

    // Comparison Methods

      bool operator==(const SkMemberExpression & expr_info) const;
      bool operator<(const SkMemberExpression & expr_info) const;

    // Accessor methods

      virtual SkExpressionBase * get_expr() const;
      virtual void               release_expr();

      uint32_t get_source_idx();
      bool     is_origin_source() const;

      #if (SKOOKUM & SK_DEBUG)
        using SkMemberInfo::set_context;
        void set_context(SkObjectBase * scope_p, SkInvokedBase * caller_p, SkExpressionBase * expr_p);
      #endif

    // Binary Stream Methods

      SkMemberExpression(const void ** binary_pp);

      #if (SKOOKUM & SK_COMPILED_OUT)
        void     as_binary(void ** binary_pp) const;
        uint32_t as_binary_length() const             { return SkMemberInfo::as_binary_length() + 2u; }
      #endif

  //protected: // Public for now

    // Data Members

      // Cached info - may go stale - use get_expr() instead of using directly
      mutable SkExpressionBase * m_expr_p;

      // Expression character index position - may go stale - use get_source_idx()
      // cached from m_expr_p and only valid if SK_DEBUG flag is set in the SKOOKUM define.
      // $Revisit - CReis Consider using line and row instead of index to decrease shifting
      // when the code is edited.
      mutable uint32_t m_source_idx;

      // $Revisit - CReis Might need this if different source origins are used
      //SkDebugInfo::eFlag m_code_origin;

  };


#if 0 // Currently unused

//---------------------------------------------------------------------------------------
// Invoked object descriptor - SkInvokedContext(SkInvokedMethod or SkInvokedCoroutine) or
// SkInvokedExpression/SkExpression "leaf".
class SK_API SkInvokedInfo
  : public SkMemberInfo,
  AListNode<SkInvokedInfo>  // Stored in the m_calls of its caller
  {
  public:

  // Nested Structures

    enum eFlag
      {
      Flag_expression   = 1 << 0,  // Represents a "leaf" expression.
      };


  // Common Methods

    operator const uint32_t & () const                  { return m_unique_id; }
    bool operator==(const SkInvokedInfo & iinfo) const  { return m_unique_id == iinfo.m_unique_id; }
    bool operator<(const SkInvokedInfo & iinfo) const   { return m_unique_id < iinfo.m_unique_id; }

  protected:

    friend AListNode<SkInvokedInfo>;

  // Data Members

    uint32_t m_flags;

    // Unique instance id for this invoked object "thread"
    uint32_t m_unique_id;

    // Expression index position of calling expression if this represents an invoked
    // context (method/coroutine) or the current/leaf expression index if it is a invoked
    // expression.
    uint32_t m_expr_idx;

    // String representation of arguments in name->value form.
    // Symbols translated as needed and escape characters converted to backslash equivalents.
    AString m_args_str;

    // Call Hierarchy
      
      SkInvokedInfo *      m_caller_p;  // Invoked context that called this
      AList<SkInvokedInfo> m_calls;     // List of any invoked objects called by this invoked object

    // Scope/receiver/this Info
    
      uint32_t  m_scope_id;       // Unique id of scope
      uint32_t  m_scope_addr;     // Scope memory address
      SkClass * m_scope_class_p;  // Scope class
      AString   m_scope_str;      // String representation of scope (translated & breaks converted)

    // Updater info

      ASymbol   m_updater_name;
      SkClass * m_updater_class_p;

    // Invoked Coroutine Info

      uint32_t m_update_count;
      f32      m_update_interval;
      f64      m_update_next;

  };  // SkInvokedInfo


//---------------------------------------------------------------------------------------
// Since Skookum runs scripts concurrently a "call tree" is a more correct and useful
// representation than a "call stack".  Each actor object may have zero to many call tree
// "leaves" that it is updating and there can be many call trees distributed amongst all
// the actors in the runtime.
class SK_API SkCallTree
  {
  public:

  // Common Methods

  protected:

  // Data Members

    // Leaf point of current call stack being examined
    SkInvokedInfo * m_current_path_p;

    // Leaf point of current break stack
    SkInvokedInfo * m_break_path_p;

    // Ancestor/originating root of entire call tree
    SkInvokedInfo * m_call_root_p;

  };  // SkCallTree

#endif

#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Notes      Debug Breakpoint
// Author(s)  Conan Reis
class SK_API SkBreakPoint : public SkMemberExpression
  {
  public:

  // Nested Structures

    // Hit type
    enum eHit
      {
      Hit_always,
      Hit_equal,
      Hit_greater_equal,
      Hit_less_equal,
      Hit_multiple
      };

    enum eConstrain
      {
      Constrain_ignored,
      Constrain_populate
      };

    enum eBinaryFlag
      {
      BinaryFlag__none        = 0x0,
      BinaryFlag_class_scope  = 1 << 0,
      BinaryFlag_enabled      = 1 << 1
      };

    enum eUpdate
      {
      Update_enable,
      Update_disable,
      Update_remove
      };


  // Common Methods

    SK_NEW_OPERATORS(SkBreakPoint);

    SkBreakPoint(const SkMemberInfo & member_info, SkExpressionBase * break_expr_p = nullptr);
    SkBreakPoint(const SkMemberExpression & info, uint32_t table_idx, bool enabled = true);
    virtual ~SkBreakPoint();

    // Accessor methods

      virtual SkExpressionBase * get_expr() const override;
      virtual void               release_expr() override;
      static void                release_expr(SkExpressionBase * expr_p);
      void                       acquire_expr()  { get_expr(); }

  // Methods

    void enable(bool set_break = true);
    void enable_set()                            { enable(); }
    void enable_clear()                          { enable(false); }
    void enable_toggle()                         { enable(!m_enabled); }
    bool is_enabled() const                      { return m_enabled; }
    void remove();

  //protected: // Public for now

    // Internal Methods

      void reaquire_expr(bool force = false) const;

    // Data Members

      // Index position in SkDebug::ms_breakpoint_table - SkDebugInfo::Flag_debug_idx__none
      // reserved for expressions without a breakpoint.
      uint32_t m_table_idx;

      // Indicates whether breakpoint is active or not
      bool m_enabled;


      // Future conditions:
      //
      //   // Only perform break action when all other conditions met and m_hit_count is:
      //   //  Hit_always        - any value
      //   //  Hit_equal         - equal to m_hit_count_test
      //   //  Hit_greater_equal - greater than or equal to m_hit_count_test
      //   //  Hit_greater_equal - less than or equal to m_hit_count_test
      //   //  Hit_multiple      - a multiple of m_hit_count_test
      //   eHit m_hit_type;
      //   uint m_hit_count_test;
      // 
      //   // Increment when all other conditions met
      //   uint m_hit_count;
      // 
      //   // Only perform break action when this context is in the call stack
      //   AIdPtr<SkInvokedContextBase> m_invoked_caller_p;
      // 
      //   // Only perform break action when being updated by a class (or subclass if m_updater_subclass_check is set) of this type.
      //   SkClass * m_updater_class_p;
      //   bool      m_updater_subclass_check;
      // 
      //   // Only perform break action when scope/receiver is an actor with the specified name
      //   ASymbol m_scope_actor_name;
      //
      //   // Test to determine if break action should occur once other conditions met [May want to keep original code]
      //   SkExpressionBase * m_test_expr_p;
      // 
      //   // Other Conditions:
      //   //
      //   //  Level/update count
      //   //  Time-based
      //   //  Updater mind
      //   //  Scope/receiver class
      //   //  Scope/receiver instance
      //   //  Method call
      //   //  Coroutine call
      //   //  Method caller
      //   //  Coroutine caller
      //   //  Invoked Coroutine invoke count

  };  // SkBreakPoint


typedef APSortedLogical<SkBreakPoint, SkMemberExpression> tSkBreakPoints;

//---------------------------------------------------------------------------------------
// A watch variable
struct SkWatch
  {
  enum Kind
    {
    Kind_instance_data,
    Kind_class_data,
    Kind_this,
    Kind_parameter,
    Kind_return_parameter,
    Kind_temporary,
    Kind_captured
    };

  SkWatch(Kind kind, const ASymbol & var_name, const ASymbol & scope_name, const SkInstance * var_p, uint64_t var_guid = 0);

  #if (SKOOKUM & SK_COMPILED_IN)             
             SkWatch(const void ** binary_pp) { assign_binary(binary_pp); }
    void     assign_binary(const void ** binary_pp);
  #endif

  #if (SKOOKUM & SK_COMPILED_OUT)
    void     as_binary(void ** binary_pp) const;
    uint32_t as_binary_length() const;
  #endif

  Kind      m_kind;
  ASymbol   m_var_name;
  ASymbol   m_type_name;
  ASymbol   m_scope_name;
  AString   m_value;
  uint64_t  m_var_guid;
  uint32_t  m_ref_count;
  uint32_t  m_ptr_id;
  uint64_t  m_obj_address;

  // The data members and/or list items of this variable
  APArrayFree<SkWatch> m_children;
  };

//---------------------------------------------------------------------------------------
// Stores a SkookumScript call stack for printing and debugging
// Derived from ARefCountMix so it can be optionally stashed away during processing calls
struct SK_API SkCallStack : public ARefCountMix<SkCallStack>
  {

  SkCallStack() : m_current_level_idx(0) {}

  // Conversion Methods

  #if (SKOOKUM & SK_COMPILED_IN)
          SkCallStack(const void ** binary_pp) { assign_binary(binary_pp); }
    void  assign_binary(const void ** binary_pp);
  #endif

  #if (SKOOKUM & SK_COMPILED_OUT)
    void     as_binary(void ** binary_pp) const;
    uint32_t as_binary_length() const;  
  #endif

  // Data

  // Stores a stack frame
  struct Level : SkMemberExpression
    {
    Level(const SkMemberExpression & member_expr, const AString & label, bool is_context) : SkMemberExpression(member_expr), m_label(label), m_is_context(is_context) {}

    #if (SKOOKUM & SK_COMPILED_IN)             
      Level(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const;
    #endif

    // Label string to display for this callstack level
    AString m_label;

    // Is this an invoked routine?
    bool m_is_context;

    // The locals known on this level of the callstack
    APArrayFree<SkWatch> m_locals;
    };

  // The stack level to show by default (currently, either 0 or 1)
  uint16_t m_current_level_idx;

  // The stack of callers ordered from inner to outer
  APArrayFree<Level> m_stack;
  };

#endif  // (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------

typedef void (* tSkMethodHook)(SkInvokedMethod * imethod_p);
typedef void (* tSkCoroutineHook)(SkInvokedCoroutine * icoro_p);
typedef void (* tSkScriptSystemHook)(const ASymbol & origin_id);

//---------------------------------------------------------------------------------------
// Notes      SkookumScript Debug
// Author(s)  Conan Reis
class SK_API SkDebug
  {
  public:

  // Public Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Runtime Debug State
    enum eState
      {
      State__flag_suspended      = 1 << 0,
      State__flag_member_info    = 1 << 1,
      State__flag_expr_info      = 1 << 2,

      State_running              = 0x0,
      State_suspended            = State__flag_suspended,
      State_suspended_member     = State__flag_suspended | State__flag_member_info,
      State_suspended_expr       = State__flag_suspended | State__flag_member_info | State__flag_expr_info
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Stepwise Debugging Type/Flags
    enum eStep
      {
      // Used to indicate not stepping
      Step__none = 0x0,

      // Stop at next expression regardless of the context
      Step_next  = 1,

      // Stop on the next expression that has the same invoked context (method/coroutine)
      // unless it is stale in which case ignore it.
      Step_into  = 2,

      // Stop on the expression with the same invoked context (method/coroutine) unless it
      // is stale in which case ignore it after the current expression has completed.
      Step_over  = 3,

      // Stop on the next expression after the current invoked context (method/coroutine)
      // has completed (it should be on the same thread).
      Step_out   = 4
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Debugging Flags - used by SkDebug::ms_flags
    enum eFlag
      {
      Flag_hook_expression    = 1 << 0,
      Flag_stepping           = 1 << 1,

      Flag__none                  = 0x0,
      Flag__test_expression_mask  = Flag_hook_expression | Flag_stepping,
      Flag__default               = Flag__none
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Debugging preferences flags - used by SkDebug::ms_pref_flags
    enum ePrefFlag
      {
      // Automatically print out callstack when a Skookum expression break occurs
      PrefFlag_break_print_callstack    = 1 << 0,

      // Automatically print out local variables when a Skookum expression break occurs
      PrefFlag_break_print_locals       = 1 << 1,

      PrefFlag__none              = 0x0,
      PrefFlag__break_print_mask  = PrefFlag_break_print_callstack | PrefFlag_break_print_locals,
      PrefFlag__default           = PrefFlag_break_print_callstack | PrefFlag_break_print_locals
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Which context the debug hook gets invoked in
    enum eHookContext
      {
      HookContext_current,  // Same context that is currently being debugged
      HookContext_peek,     // Context of routine that is about to be invoked but hasn't been invoked yet
      };

    #if defined(SKDEBUG_HOOKS)

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Hook group used to describe a set of script execution callbacks to enable/disable
      // all at once.
      struct Hook : public
        ANamed,
        AListNode<Hook, tSkMethodHook>,
        AListNode<Hook, tSkCoroutineHook>,
        AListNode<Hook, tSkScriptSystemHook>
        {
        friend class SkDebug;

        // Nested Structures
       
          // Used by SkDebug::Hook::m_flags
          enum eFlag
            {
            Flag_enabled                = 1 << 0,

            Flag_only_updater_class     = 1 << 4,
            Flag_only_scope_name        = 1 << 6,
            Flag_only_invoked_caller    = 1 << 7,

            Flag__none = 0x0,
            Flag__condition_mask = Flag_only_updater_class | Flag_only_scope_name | Flag_only_invoked_caller
            };

        // Data Members

          // Short name of hook - used for UI
          AString m_hook_name;

          // More detailed description of hook - used for UI
          AString m_hook_desc;

          // Callback/hook for methods - ignored if nullptr
          tSkMethodHook m_hook_method_f;

          // Callback/hook for coroutines - ignored if nullptr
          tSkCoroutineHook m_hook_coroutine_f;

          // Callback/hook for entering the Skookum system prior to executing scripts.
          // [Either both m_hook_script_entry_f and m_hook_script_exit_f must be set to a
          // valid function or both must be set to nullptr - in which case they are ignored.]
          tSkScriptSystemHook m_hook_script_entry_f;

          // Callback/hook for exiting the Skookum system after executing scripts.
          // [Either both m_hook_script_entry_f and m_hook_script_exit_f must be set to a
          // valid function or both must be set to nullptr - in which case they are ignored.]
          tSkScriptSystemHook m_hook_script_exit_f;

          // Conditions / Constraints for method/coroutine hooks

            // Only run hooks when being updated by a class (or subclass if
            // m_updater_subclass_check is set) of this type.
            SkClass * m_updater_class_p;
            bool      m_updater_subclass_check;  // $Revisit - CReis Could store this in m_flags

            // Only run hooks when scope/receiver is an actor with the specified name
            ASymbol m_scope_actor_name;

            // Only run hooks when this context is in the call stack
            AIdPtr<SkInvokedContextBase> m_invoked_caller_p;

            // [Conditions that could be added in the future]
            // 
            // Note - all these conditions (and more) can be tested in the hook/ functions
            // themselves - it just might not be as efficient as "built-in" tests.
            // 
            //   Level/update count
            //   Time-based
            //   Updater mind
            //   Scope/receiver class
            //   Scope/receiver instance
            //   Method call
            //   Coroutine call
            //   Method caller
            //   Coroutine caller
            //   Invoked Coroutine invoke count

        // Methods

          Hook(const AString & hook_name);
          Hook(const Hook & hook);

          Hook & operator=(const Hook & hook);

          AString       get_name_str() const        { return m_hook_name; }
          const char *  get_name_cstr() const       { return m_hook_name.as_cstr(); }
          AString       get_name_str_dbg() const    { return m_hook_name; }
          const char *  get_name_cstr_dbg() const   { return m_hook_name.as_cstr(); }
          void          set_name(const AString & name);

          bool is_enabled() const                   { return (m_flags & Flag_enabled) !=0; }
          bool is_conditions_met(SkInvokedContextBase * icontext_p) const;

        protected:

          // Internal methods

            void update_flags();

          // Protected Data Members

            // Current hook state - see SkDebug::Hook::eFlag
            uint32_t m_flags;

        };  // SkDebug::Hook

      #endif // SKDEBUG_HOOKS


  // Public Class Data Members - for quick access

    static bool ms_no_step_default_hack;

    // Number of spaces to indent - used in situations like when converting in-memory
    // structures to code scripts like most `as_string_debug()` methods.
    // This is independent of tab stop size.
    static uint32_t ms_indent_size;

    // Tab stop size in non-proportional space characters. Used wherever tab `\t`
    // characters are displayed in text - used to determine visual columns a section of
    // code uses when creating debug / error messages like
    // `SkParser::get_result_context_string()`.
    // This is independent of indent size.
    static uint32_t ms_tab_stops;

    #if (SKOOKUM & SK_DEBUG)
      // Currently executing method or coroutine - tracked here for debugging purposes.
      // Enables rest of engine to know if in the middle of a script call or not.
      // Also see ms_next_expr & ms_next_invokable_p.
      static AIdPtr<SkInvokedContextBase> ms_current_call_p;
    #endif

    #if defined(SKDEBUG_COMMON)
      // If set to SkDebugInfo::Flag_debug_enabled then all expressions have their execution
      // trapped/tested otherwise only expressions with breakpoints are tested.
      static uint32_t ms_expr_hook_flag;
    #endif

  // Common Methods

      static void initialize();
      static void enable_engine_present(bool engine_present_b = true)           { ms_engine_present_b = engine_present_b; }
      static bool is_engine_present()                                           { return ms_engine_present_b; }
      static void register_bindings();
      static void deinitialize();

    // Debug Output

      static void    print_info();
      static void    print_callstack(SkInvokedBase * invoked_p = nullptr, uint32_t stack_flags = SkInvokeInfo__callstack_def);
      static void    append_callstack_string(AString * str_p, SkInvokedBase * invoked_p = nullptr, uint32_t stack_flags = SkInvokeInfo__callstack_def);
      static void    append_locals_string(AString * str_p, SkInvokedBase * caller_p, SkInvokedBase * invoked_p = nullptr, uint32_t flags = SkInvokeInfo__locals_def);
      static AString get_context_string(const AString & description, SkObjectBase * call_scope_p, SkObjectBase * alt_scope_p = nullptr, uint32_t stack_flags = SkInvokeInfo__callstack_def);
      static void    print(const AString & str, eSkLocale locale = SkLocale_all, uint32_t type = SkDPrintType_system);
      static bool    print_ide(const AString & str, eSkLocale locale = SkLocale_all, uint32_t type = SkDPrintType_system);
      static void    print_ide_all(const AString & str)                { print_ide(str); }
      static void    print_agog(const AString & str, eSkLocale locale = SkLocale_all, uint32_t type = SkDPrintType_system);
      static void    print_script_context(const AString & msg, const SkInvokableBase * invokable_p = nullptr, SkExpressionBase * expr_p = nullptr, uint32_t type = SkDPrintType_system);
      static void    print_error(const AString & err_msg, eAErrLevel level = AErrLevel_error);
      static void    set_print_func(tSkPrintFunc * log_func_p);
      static void    register_print_with_agog();
      static void    suppress_prints(bool suppress)                             { ms_suppress_prints = suppress; }

      #if (SKOOKUM & SK_CODE_IN)
        static void  print_parse_error(SkParser::eResult result, const AString & path = AString::ms_empty, const AString * code_p = nullptr, uint32_t result_pos = 0u, uint32_t result_start = ADef_uint32, uint32_t start_pos = 0u);
        static void  print_parse_error(const SkParser::Args & args, const AString & path = AString::ms_empty, const AString * code_p = nullptr, uint32_t start_pos = 0u)  { print_parse_error(args.m_result, path, code_p, args.m_end_pos, args.m_start_pos, start_pos); }
      #endif

      #if (SKOOKUM & SK_DEBUG)
        static void context_append(AString * str_p);
      #endif

    // Debug Memory

      static void     print_memory(eSkCodeSerialize type = SkCodeSerialize_static_demand);
      static uint32_t print_memory_runtime();
      static uint32_t print_memory_code(eSkCodeSerialize type = SkCodeSerialize_static_demand, SkClass * from_class_p = nullptr, eAHierarchy iterate = AHierarchy__all);
      static void     set_print_memory_ext_func(AFunctionBase * mem_func_p)         { ms_print_mem_ext_p = mem_func_p; }


    // Debugging

      #if defined(SKDEBUG_COMMON)
        static void set_flag(eFlag debug_flag, bool enable = true);
      #endif

      #if (SKOOKUM & SK_DEBUG)
  
        static eState               get_execution_state()                       { return ms_exec_state; }
        static SkMemberExpression & get_next_expression()                       { return ms_next_expr; }
        static SkInvokedBase *      get_next_invokable();
        static SkDebugInfo          get_next_debug_info();
        static uint32_t             get_preferences()                           { return ms_pref_flags; }

        static SkCallStack *        get_callstack(const SkInvokedBase * invoked_p, const SkMemberExpression * initial_member_expr_p, uint32_t stack_flags = SkInvokeInfo__callstack_def);
        static SkCallStack *        get_callstack(const SkInvokedBase * invoked_p, const SkInvokedBase * invoked_caller_p, const SkMemberExpression * initial_member_expr_p, uint32_t stack_flags = SkInvokeInfo__callstack_def);
        static void                 append_watch_locals(APArray<SkWatch> * m_locals_p, const SkInvokedBase * invoked_p);
        static void                 append_watch_members(APArray<SkWatch> * m_members_p, SkInstance * obj_p);

        static void break_expression(SkObjectBase * scope_p, SkInvokedBase * caller_p, SkExpressionBase * expr_p);
        static void break_invokable(SkInvokedBase * invoked_p);
        static void enable_preference(ePrefFlag preference, bool enable = true);
        static void invalidate_next_expression();
        static bool is_active_member(const SkMemberInfo & member) { return member.is_valid() && ms_next_expr.is_valid() && (member == ms_next_expr); }
        static bool is_preference(ePrefFlag preference) { return (ms_pref_flags & preference) != 0u; }
        static void set_execution_state(eState state) { ms_exec_state = state; }
        static void set_next_expression(const SkMemberExpression & expr_info);
        static void set_next_expression(SkObjectBase * scope_p, SkInvokedBase * caller_p, SkExpressionBase * expr_p);
        static void set_preferences(uint32_t pref_flags) { ms_pref_flags = pref_flags; }
        static void step(eStep step_type);
      #endif  // (SKOOKUM & SK_DEBUG)

    // Breakpoints

      #if (SKOOKUM & SK_DEBUG)

        static SkBreakPoint * breakpoint_append(const SkMemberExpression & bp_info, uint32_t table_idx, bool enabled = true);
        static SkBreakPoint * breakpoint_append_absent(const SkMemberInfo & member_info, uint source_idx, bool * appended_p = nullptr);
        static SkBreakPoint * breakpoint_append_absent(const SkMemberInfo & member_info, SkExpressionBase * break_expr_p, bool * appended_p = nullptr);
        static uint32_t       breakpoint_get_count()                            { return ms_breakpoints.get_length(); }
        static bool           breakpoint_remove(const SkBreakPoint & bp);
        static bool           breakpoint_remove_by_expr(const SkExpressionBase & expr);
        static void           breakpoint_enable_all();
        static void           breakpoint_disable_all();
        static void           breakpoint_remove_all();
        static void           breakpoint_release_all();
        static void           breakpoint_acquire_all();
        static void           breakpoint_list_all();

        static bool breakpoint_is_on_class(const SkClass & ssclass);
        static bool breakpoint_is_on_member(const SkMemberInfo & member_info);
        static bool breakpoint_is_on_expr(const SkExpressionBase & expr);

        static void                   breakpoint_get_all_by_member(tSkBreakPoints * bps_p, const SkMemberInfo & member_info);
        static SkBreakPoint *         breakpoint_get_by_expr(const SkExpressionBase & expr);
        static SkBreakPoint *         breakpoint_get_at_idx(uint32_t table_idx);

        static const APSortedLogicalFree<SkBreakPoint, SkMemberExpression> & breakpoints_get_all()  { return ms_breakpoints; }

      #endif  // (SKOOKUM & SK_DEBUG)


    // Breakpoints - Scripted

      static void set_scripted_break(void (*scripted_break_f)(const AString & message, SkInvokedMethod * scope_p))    { ms_scripted_break_f = scripted_break_f; }

    // Execution Hooks

      #if defined(SKDEBUG_COMMON)
        static void            hook_expression(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p, SkInvokedBase * _caller_caller_p, eHookContext hook_context);
      #endif

      #if defined(SKDEBUG_HOOKS)

        static void            append_hook(const Hook & hook, bool auto_enable = true);
        static const Hook *    copy_hook(const ASymbol & hook_name, const AString & new_name);
        static void            enable_hook(const ASymbol & hook_name, bool enable = true);
        static void            enable_hook(Hook * hook_p, bool enable);
        static void            hook_condition_scope_actor(const ASymbol & hook_name, const ASymbol & actor_name);
        static void            hook_condition_invoked_caller(const ASymbol & hook_name, SkInvokedContextBase * caller_p);
        static void            hook_condition_updater_class(const ASymbol & hook_name, SkClass * class_p, bool subclass_check);
        static void            remove_hook(const ASymbol & hook_name);
        static bool            is_hook_enabled(const ASymbol & hook_name);
        static const Hook *    get_hook(const ASymbol & hook_name)            { return ms_hooks.get(hook_name); }
        static const ASymbol & get_hook_script_origin()                       { return ms_hook_script_origin_stack[ms_hook_script_origin_idx]; }
        static void            set_hook_expr(void (*expr_dbg_f)(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p) = nullptr);

        static const APSortedLogicalFree<Hook, ASymbol> & get_hooks()         { return ms_hooks; }

        // Example/built-in Hooks

          static void hook_examples();
          static void hook_trace_method(SkInvokedMethod * imethod_p);
          static void hook_trace_coroutine(SkInvokedCoroutine * icoro_p);
          static void hook_trace_script_entry(const ASymbol & origin_id);
          static void hook_trace_script_exit(const ASymbol & origin_id);

        // Called by system - do not call manually.  [Public by necessity - see SKDEBUG_HOOK_*() macros.]

          static void hook_method(SkInvokedMethod * imethod_p);
          static void hook_coroutine(SkInvokedCoroutine * icoro_p);
          static void hook_script_origin_push(const ASymbol & origin_id);
          static void hook_script_origin_pop();

      #endif // SKDEBUG_HOOKS

  protected:

  // Internal Class Methods

    static void breakpoint_hit_embedded_def(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p);

  // SkookumScript Atomic Methods

    static void mthdc_break(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_callstack(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_callstack_str(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_copy_hook(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_enable_hook(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_engine_presentQ(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_hook_names(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_hook_condition_scope_actor(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_hook_condition_invoked_caller(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_hook_condition_updater_class(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_hook_enabledQ(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_print(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_println(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthdc_sym_to_str(SkInvokedMethod * scope_p, SkInstance ** result_pp);

  // Class Data Members

    // If true, the engine/game is present/hooked-in and if false it is not present.
    // This value is tested by Debug@is_engine_present()
    static bool ms_engine_present_b;

    // Printing

      static bool                    ms_suppress_prints;
      static tSkPrintFunc *          ms_log_func_p;
      static AFreePtr<AFunctionBase> ms_print_mem_ext_p;

    #if (SKOOKUM & SK_DEBUG)

    // Preferences

      // See ePrefFlag
      static uint32_t ms_pref_flags;

    // Execution Info

      // Runtime Execution Debug state
      static eState ms_exec_state;
      
      // Next member expression to execute - generally the expression that the runtime
      // is suspended/broken on.  Also see ms_current_call_p
      static SkMemberExpression ms_next_expr;

      // Current invokable that is sacrosanct to step-wise debugging - ms_current_call_p can change or be lost
      static AIdPtr<SkInvokedBase> ms_next_invokable_p;

      // See eStep
      static eStep ms_step_type;

      static AIdPtr<SkInvokedContextBase> ms_step_icontext_p;
      static AIdPtr<SkInvokedBase>        ms_step_topmost_caller_p;
      static SkExpressionBase *           ms_step_expr_p;

    // Breakpoints
    
      // Breakpoints stored by index position - may be sparse with nullptr at unused indexes.
      // Index position is cached in breakpoint objects and used as a breakpoint "id".
      static APArray<SkBreakPoint> ms_breakpoint_table;

      // Breakpoints sorted by member expression info
      static APSortedLogicalFree<SkBreakPoint, SkMemberExpression> ms_breakpoints;

    #endif  // (SKOOKUM & SK_DEBUG)

      static void (* ms_scripted_break_f)(const AString & message, SkInvokedMethod * scope_p);

    // Execution Hooks
    #if defined(SKDEBUG_HOOKS)
      static ASymbol  ms_hook_script_origin_stack[8u];  // Shouldn't need to be too deep
      static uint32_t ms_hook_script_origin_idx;

      // Expression Debugger Function pointer
      static void (* ms_hook_expr_f)(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p);

      static APSortedLogicalFree<Hook, ASymbol> ms_hooks;
      static AList<Hook, tSkMethodHook>         ms_hook_methods;
      static AList<Hook, tSkCoroutineHook>      ms_hook_coroutines;
      static AList<Hook, tSkScriptSystemHook>   ms_hook_origins;
    #endif

    #if defined(SKDEBUG_COMMON)
      // Debugging flags - see SkDebug::eFlag
      static uint32_t ms_flags;
    #endif

  };  // SkDebug


#if (SKOOKUM & SK_DEBUG)

//=======================================================================================
// SkBreakPoint Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void SkBreakPoint::remove()
  {
  SkDebug::breakpoint_remove(*this);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline SkBreakPoint::~SkBreakPoint()
  {
  }

#endif  // (SKOOKUM & SK_DEBUG)


//=======================================================================================
// SkDebug Inline Methods
//=======================================================================================

#ifdef SKDEBUG_COMMON

//---------------------------------------------------------------------------------------
// Sets debug flag - see SkDebug::eFlag
// Modifiers:   static
// Author(s):   Conan Reis
inline void SkDebug::set_flag(
  eFlag debug_flag,
  bool  enable // = true
  )
  {
  ms_flags = enable ? (ms_flags | debug_flag) : (ms_flags & ~debug_flag);

  // Test all expressions?
  ms_expr_hook_flag = ((ms_flags & Flag__test_expression_mask) != 0u)
    ? SkDebugInfo::Flag_debug_enabled
    : SkDebugInfo::Flag_debug_disabled;
  }

#endif // SKDEBUG_COMMON


#ifdef SKDEBUG_HOOKS

//---------------------------------------------------------------------------------------
// Sets the expression debugger hook function.  It is called within each
//             expression's invoke() method just prior to actually invoking the
//             expression.
// Arg         expr_dbg_f - expression debugger function to set
// See:        breakpoint_hit_embedded_def() - explains the arguments
// Notes:      In addition to any custom tests, an expression's debug data can be tested
//             for interesting values.  For example ExpressionBase::m_debug_info can be
//             tested to see if its SkDebugInfo::Flag_debug_enabled flag is set.
// Author(s):   Conan Reis
inline void SkDebug::set_hook_expr(
  void (*expr_dbg_f)(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p) // = nullptr
  )
  {
  ms_hook_expr_f = expr_dbg_f;
  set_flag(Flag_hook_expression, expr_dbg_f != nullptr);
  }

#endif  // SKDEBUG_HOOKS
