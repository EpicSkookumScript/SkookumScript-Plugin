// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript common declarations.  [Included in all SkookumScript files]
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>
#include <AgogCore/AString.hpp>
#include <AgogCore/ASymbol.hpp>
#include <AgogCore/AVCompactArray.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#ifndef SKOOKUM
  #error The 'SKOOKUM' define must be present and be set to an appropriate value
#endif

// These are the possible flags that may be set for the SKOOKUM define:

// Include extra SkookumScript debugging code.
#define SK_DEBUG     (1 << 0)  // 1

// Include ability to parse SkookumScript code string/text into internal data structures.
#define SK_CODE_IN   (1 << 1)  // 2

// Include ability to generate SkookumScript code string/text from internal data structures.
#define SK_CODE_OUT  (1 << 2)  // 4

// Include ability to read in SkookumScript compiled binary code and convert it to internal data structures.
#define SK_COMPILED_IN  (1 << 3)  // 8

// Include ability to generate SkookumScript compiled binary code from internal data structures.
#define SK_COMPILED_OUT (1 << 4)  // 16

// Here are the normal configuration values for the SKOOKUM define:
//   SkookumScript IDE      = 31  [SK_DEBUG | SK_CODE_IN | SK_CODE_OUT | SK_COMPILED_IN | SK_COMPILED_OUT]
//   Game (Debug & Dev)     = 15  [SK_DEBUG | SK_CODE_IN | SK_CODE_OUT | SK_COMPILED_IN]
//   Game (Release / Final) = 8   [SK_COMPILED_IN]


// If this is defined then check for conditions that would normally be caught and fixed at
// compile-time, but that may slip through to runtime.  For example a bad cast can fool
// the compiler into thinking that an object is of a type that it ends up not being at 
// runtime.  Assert and describe the error if (SKOOKUM & SK_DEBUG) is defined.  Then
// recover from the error state as best as possible.  These checks should be redundant
// if all the errors were caught when (SKOOKUM & SK_DEBUG) is defined, but in big complex
// games lots of things can't be tested rigorously enough to warrant removing the extra
// checks.
// $Revisit - CReis It might be a good idea to have this as another flag in the SKOOKUM
// define.
#define SK_RUNTIME_RECOVER

//---------------------------------------------------------------------------------------
// Debug hooks for notifying when scripts start/stop various tasks so that things like
// tracing, profiling, breakpoints, etc. can be added.
// See SkDebug.hpp - put this here due to problems with order of includes
//#if (SKOOKUM & SK_DEBUG) && !defined(SKDEBUG_HOOKS_DISABLE)
//  #define SKDEBUG_HOOKS
//#endif


//---------------------------------------------------------------------------------------
// Debug stuff common to both regular debugging and execution hooks
#if (SKOOKUM & SK_DEBUG) || defined(SKDEBUG_HOOKS)
  #define SKDEBUG_COMMON
#endif


//---------------------------------------------------------------------------------------
// When present, methods used to visualize/convert data-structures to code scripts will be
// present in the compile.
#if (SKOOKUM & SK_DEBUG) || (SKOOKUM & SK_CODE_OUT) || defined(SKDEBUG_HOOKS)
  #define SK_AS_STRINGS
#endif


//---------------------------------------------------------------------------------------
// When present, data structures and routines to visualize/convert data-structures to
// code scripts or to read in code scripts will be present in the compile.
#if defined(SK_AS_STRINGS) || (SKOOKUM & SK_CODE_IN)
  #define SK_CODE
#endif


//---------------------------------------------------------------------------------------
#if (SKOOKUM & SK_DEBUG) && !defined(SK_IGNORE_DPRINT)
  #define SK_KEEP_DPRINT
#endif

//---------------------------------------------------------------------------------------
// DLL API
#ifdef SKOOKUMSCRIPT_API // Use SKOOKUMSCRIPT_API passed down from UE4 build system if present
// SKOOKUMSCRIPT_API is either DLLIMPORT, DLLEXPORT or empty - map to AgogCore macros
#ifndef DLLIMPORT
  #define DLLIMPORT A_DLLIMPORT
  #define DLLEXPORT A_DLLEXPORT
#endif
#define SK_API SKOOKUMSCRIPT_API
#elif defined(SK_IS_DLL) // otherwise, DLL linkage control via SK_IS_DLL & SK_IS_BUILDING_SELF
  // Further down, we'll set A_DLLIMPORT and A_DLLEXPORT to platform-specific values
  #ifdef SK_IS_BUILDING_SELF
    #define SK_API A_DLLEXPORT
  #else
    #define SK_API A_DLLIMPORT
  #endif
#else // SkookumScript is a static library
  #define SK_API
#endif

class SkInstance;
class SkClass;
struct SkBindName;

//---------------------------------------------------------------------------------------
// Interface for SkookumScript to interact with its app
class SkAppInfo
  {
  public:

    //---------------------------------------------------------------------------------------
    // If to use the built-in actor class. Otherwise you have to implement your own.
    virtual bool use_builtin_actor() const { return true; }

    //---------------------------------------------------------------------------------------
    // If you do not use the built-in actor class, here is where you specify the name of yours.
    // Or set to ASymbol::ms_null if you do not want an actor class at all.
    virtual ASymbol get_custom_actor_class_name() const { return ASymbol::ms_null; }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for SkInstance objects  
    virtual uint32_t get_pool_init_instance() const { return 3000; }
    virtual uint32_t get_pool_incr_instance() const { return 256; }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for SkDataInstance objects
    virtual uint32_t get_pool_init_data_instance() const { return 64; }
    virtual uint32_t get_pool_incr_data_instance() const { return 64; }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for SkInvokedExpression objects
    virtual uint32_t get_pool_init_iexpr() const { return 1152; }
    virtual uint32_t get_pool_incr_iexpr() const { return 128; }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for SkInvokedCoroutine objects
    virtual uint32_t get_pool_init_icoroutine() const { return 896; }
    virtual uint32_t get_pool_incr_icoroutine() const { return 128; }

    //---------------------------------------------------------------------------------------
    // Handling of custom bind names
    // SkBindName is a placeholder structure used by the app to hold a name symbol in its native format
    virtual void          bind_name_construct(SkBindName * bind_name_p, const AString & value) const = 0;
    virtual void          bind_name_destruct(SkBindName * bind_name_p) const = 0;
    virtual void          bind_name_assign(SkBindName * bind_name_p, const AString & value) const = 0;
    virtual AString       bind_name_as_string(const SkBindName & bind_name) const = 0;
    virtual SkInstance *  bind_name_new_instance(const SkBindName & bind_name) const = 0;
    virtual SkClass *     bind_name_class() const = 0;

  };

//=======================================================================================
// Global Types
//=======================================================================================

typedef int32_t   tSkInteger;
typedef f32       tSkReal;
typedef bool      tSkBoolean;
typedef uint32_t  tSkEnum;

//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class  ASymbol;
class  SkClassDescBase;
class  SkClass;
class  SkMind;
class  SkInstance;
struct SkProgramUpdateRecord;

// Constant indicating that the interval should be as often as possible - i.e. every call
// of SkMind::on_update().
const f32 SkCall_interval_always = 0.0f;

// Constant indicating that the interval should be as small as possible - i.e. never call
const f32 SkCall_interval_never = FLT_MAX;

//---------------------------------------------------------------------------------------
// Indicates whether an invocation/call should return immediately - like methods, after a
// duration (including immediately) - like coroutines or either.
enum eSkInvokeTime
  {
  SkInvokeTime_immediate  = 1 << 0,
  SkInvokeTime_durational = 1 << 1,
  SkInvokeTime_any        = SkInvokeTime_immediate | SkInvokeTime_durational
  };

//---------------------------------------------------------------------------------------
// Invocation type - it is stored in the compiled binary code so modify with care
enum eSkInvokeType
  {
  SkInvokeType__invalid                 = 0,
  SkInvokeType_coroutine                = 1,
  SkInvokeType_method_on_instance       = 2,  // Invoke an instance method on an instance
  SkInvokeType_method_on_class          = 3,  // Invoke a class method on a class
  SkInvokeType_method_on_instance_class = 4,  // Invoke a class method on an instance
  SkInvokeType_method_on_class_instance = 5,  // Invoke an instance method on a class instance (= meta class)
  SkInvokeType_method_boolean_and       = 6,  // Invoke the instance method `and`  on an instance
  SkInvokeType_method_boolean_or        = 7,  // Invoke the instance method `or`   on an instance
  SkInvokeType_method_boolean_nand      = 8,  // Invoke the instance method `nand` on an instance
  SkInvokeType_method_boolean_nor       = 9,  // Invoke the instance method `nor`  on an instance
  SkInvokeType_method_assert            = 10, // Special invocation for asserts so we can examine the given expression
  SkInvokeType_method_assert_no_leak    = 11, // Same plus also allow for examination of memory usage before and after
  };

//---------------------------------------------------------------------------------------
// This is used to differentiate between different types of invokables when it is only
// known that an instance is of type SkInvokableBase, but not the specific subclass.
// It is returned by the method SkInvokableBase::get_invoke_type();
enum eSkInvokable
  {
  SkInvokable_method         = 0,  // Custom method
  SkInvokable_method_func    = 1,  // Atomic method (C++ function)
  SkInvokable_method_mthd    = 2,  // Atomic method (C++ method)
  SkInvokable_coroutine      = 3,  // Custom coroutine
  SkInvokable_coroutine_func = 4,  // Atomic coroutine (C++ function)
  SkInvokable_coroutine_mthd = 5   // Atomic coroutine (C++ method)
  };

//---------------------------------------------------------------------------------------
enum eSkMember
  {
  SkMember_method         = SkInvokable_method,          // Custom method
  SkMember_method_func    = SkInvokable_method_func,     // Atomic method (C++ function)
  SkMember_method_mthd    = SkInvokable_method_mthd,     // Atomic method (C++ method)
  SkMember_coroutine      = SkInvokable_coroutine,       // Custom coroutine
  SkMember_coroutine_func = SkInvokable_coroutine_func,  // Atomic coroutine (C++ function)
  SkMember_coroutine_mthd = SkInvokable_coroutine_mthd,  // Atomic coroutine (C++ method)

  // Members <= this can show disassembly
  SkMember__disassembly = SkMember_coroutine_mthd,  

  SkMember_data,
  SkMember_class_meta,        // Class meta information
  SkMember_object_ids,        // Class object ID validation list
  SkMember_object_ids_defer,  // Class object ID validation list (deferred)

  SkMember__length,     // The position of these enumerations are important
  SkMember__invalid,    // Not a valid member and can be ignored
  SkMember__error       // Not a valid member, but considered an error rather than ignored
  };


//---------------------------------------------------------------------------------------
enum eSkScope
  {
  SkScope__none,        // Variable does not exist in current scope
  SkScope_class,        // Class members
  SkScope_instance,     // Instance members
  SkScope_instance_raw, // Raw instance members (data members only)
  SkScope_local         // Temporary variables, arguments [& perhaps variables higher on call stack?]
  };

//---------------------------------------------------------------------------------------
// Valid annotations (hardcoded here for now) 
// HACK $Revisit MBreyer make data driven
enum eSkAnnotation
  {
  // Annotations just used by the parser
  SkAnnotation_raw              = 1 << 0, // Used to denote a raw data member

  // Annotations used at runtime
  SkAnnotation_reflected_cpp    = 1 << 1, // This class was auto-generated via reflection from C++ code
  SkAnnotation_reflected_data   = 1 << 2, // This class was auto-generated from data (e.g. a Blueprint class)
  SkAnnotation_invokable        = 1 << 3, // This class represents an invokable instance and must be qualified with a signature
  SkAnnotation_ue4_blueprint    = 1 << 6, // $UE4-specific Expose this method to the UE4 Blueprint system

  SkAnnotation__none    = 0,
  SkAnnotation__default = SkAnnotation__none
  };

// What we are applying annotations to
enum eSkAnnotationTarget
  {
  SkAnnotationTarget__any,
  SkAnnotationTarget_class,
  SkAnnotationTarget_invokable,
  SkAnnotationTarget_instance_data,
  };

//---------------------------------------------------------------------------------------
// List of a.k.a. (also-known-as) names for invokables
typedef AVCompactArray<AString> tSkAkas;

//---------------------------------------------------------------------------------------
// A POD struct the same size as a given class T
template<class T>
struct SkPOD
  {
  uint8_t m_pod[sizeof(T)];
  };

template<class T>
struct SkPOD32
  {
  uint32_t m_pod[sizeof(T)/4];
  };

//---------------------------------------------------------------------------------------
// Notes      SkookumScript main class
// Author(s)  Conan Reis
class SK_API SkookumScript
  {
  friend class SkActor;
  friend class SkBrain;

  public:

  enum eFlag
    {
    Flag_need_update = 1 << 0,  // There are scripts that need updating (set by system)
    Flag_updating    = 1 << 1,  // In the middle of an update (set by system)
    Flag_trace       = 1 << 2,  // Trace scripts
    Flag_paused      = 1 << 3,  // Pause scripts - for debugging

    // Masks and combinations
    Flag__none       = 0,
    //Flag__eval_update = Flag_evaluate | Flag_need_update  // Can evaluate and an update is needed
    };

  enum eInitializationLevel
    {
    InitializationLevel_none,       // Uninitialized
    InitializationLevel_minimum,    // Basic initialization, no classes exist yet
    InitializationLevel_program,    // Program/classes/routines exist, atomics are bound, program not running yet
    InitializationLevel_sim,        // Class constructors have been called and program is running, no mind instances created yet
    InitializationLevel_gameplay,   // Game is running and minds have been created
    };

  // Class Methods

    static void         set_app_info(SkAppInfo * app_info_p);
    static SkAppInfo *  get_app_info();

    static void register_bind_atomics_func(void (*bind_atomics_f)());

    static void initialize();
    static void initialize_program();
    static void initialize_sim();
    static void initialize_gameplay(bool create_master_mind = true);

    static void deinitialize_gameplay();
    static void deinitialize_sim();
    static void deinitialize_program();
    static void deinitialize();

    static void register_on_initialization_level_changed_func(void (*on_initialization_level_changed_f)(eInitializationLevel, eInitializationLevel));

    static eInitializationLevel get_initialization_level()        { return ms_initialization_level; }
    static void                 enable_flag(eFlag flag, bool enable_b = true);
    static bool                 is_flag_set(eFlag flag);
    static bool                 is_flags_set(uint32_t flag);

    static SkMind *             get_master_mind() { return ms_master_mind_p; }
    static SkInstance *         get_master_mind_or_meta_class();
    static SkClass *            get_startup_class() { return ms_startup_class_p; }

    static SkProgramUpdateRecord * get_program_update_record()    { return ms_program_update_record_p; }

    // Update methods (in order of preference) - just use most convenient version once an update

      static void update(uint64_t sim_ticks, f64 sim_time, f32 sim_delta);
      static void update_ticks(uint64_t sim_ticks);
      static void update_delta(f32 sim_delta);

      static void register_update_request_func(void (*on_update_request_f)(bool update_req_b))   { ms_on_update_request_f = on_update_request_f; }
      static void update_request(bool update_req_b = true);

    // Time Methods

      static uint64_t get_sim_ticks()                                     { return ms_sim_ticks; }
      static f64      get_sim_time()                                      { return ms_sim_time; }
      static f32      get_sim_delta()                                     { return ms_sim_delta; }
      static void     register_update_time_func(void (*update_time_f)())  { ms_update_time_f = update_time_f; }
      static void     reset_time();
      static void     update_time();
      static void     update_time_ticks(uint64_t sim_ticks);

    // Memory Methods

      static void register_script_linear_bytes_func(void (*on_script_linear_bytes_f)(uint32_t bytes_needed))   { ms_on_script_linear_bytes_f = on_script_linear_bytes_f; }
      static void notify_script_linear_bytes(uint32_t bytes_needed)       { if (ms_on_script_linear_bytes_f) { ms_on_script_linear_bytes_f(bytes_needed); } }

  protected:

  // Class Methods

    static void pools_reserve();
    static void pools_empty();

    static void set_initialization_level(eInitializationLevel new_level);

  // Class Data Members

    static SkAppInfo * ms_app_info_p;

    static SkClass *                ms_startup_class_p;
    static SkMind *                 ms_master_mind_p;   // created in `SkookumScript::initialize_session()`
    static SkProgramUpdateRecord *  ms_program_update_record_p;

    static eInitializationLevel ms_initialization_level;
    static uint32_t             ms_flags;

    static void (* ms_on_initialization_level_changed_f)(eInitializationLevel, eInitializationLevel);
    static void (* ms_on_update_request_f)(bool update_req_b);
    static void (* ms_update_time_f)();
    static void (* ms_on_script_linear_bytes_f)(uint32_t bytes_needed);


    // Simulation Time Data Members - The simulation time should only be updated when the
    // game is being simulated - i.e. it should not change when the game is paused or
    // potentially when in the game's front end.

      // Simulation time (in seconds) since game / level start.  It may have been scaled
      // to be faster or slower (even paused) compared to real time.  Most people find it
      // more intuitive and less error prone to think in terms of seconds rather than
      // milliseconds or hundredths of seconds.  A 64-bit float is used rather than a
      // 32-bit float for greater accuracy - it will only have millisecond rounding
      // errors after several tens of thousands of years, whereas a 32-bit float can
      // begin to have rounding errors after only about 3 hours have passed and after
      // about 9 hours (specifically 2^15 = 32768 seconds) the precision will have
      // deteriorated to the point that adding a millisecond would be ignored.
      static f64 ms_sim_time;

      // Simulation ticks (in milliseconds) since game / level start.  It may have been
      // scaled to be faster or slower (even paused) compared to real time.  If time is
      // being represented to users then ms_sim_time might be more appropriate since most
      // people find it more intuitive and less error prone to think in seconds rather
      // than milliseconds.  A 64-bit integer is used rather than a 32-bit integer for
      // greater accuracy - it will only overflow every 585 million years, whereas a
      // 32-bit integer will overflow every 49.71 days.
      static uint64_t ms_sim_ticks;

      // Amount of time that has passed (in seconds) since the last update / frame of the
      // simulation.  It may be scaled to be faster or slower (even paused) compared to
      // real time.  ms_sim_delta should *only* be used for calculations that take place
      // in a single update iteration - an event that occurs in the future should use
      // ms_sim_time or ms_sim_ticks to trigger it rather than accumulating or decrementing
      // ms_sim_delta since it will accumulate millisecond errors over time.
      static f32 ms_sim_delta;

  };  // SkookumScript

//---------------------------------------------------------------------------------------
// User data to store an engine-specific name (e.g. UE4 FName)
struct SkBindName
  {
  #if (SKOOKUM & SK_DEBUG)
    uint64_t  m_data[2]; // 16 bytes in debug builds
  #else
    uint64_t  m_data;    // 8 bytes in release builds
  #endif

  SkBindName(const AString & name) { SkookumScript::get_app_info()->bind_name_construct(this, name); }
  ~SkBindName()                    { SkookumScript::get_app_info()->bind_name_destruct(this); }

  void         operator = (const AString & name)  { SkookumScript::get_app_info()->bind_name_assign(this, name); }
  AString      as_string() const                  { return SkookumScript::get_app_info()->bind_name_as_string(*this); }
  SkInstance * new_instance() const               { return SkookumScript::get_app_info()->bind_name_new_instance(*this); }

  static SkClass * get_class()                    { return SkookumScript::get_app_info()->bind_name_class(); }

  #if (SKOOKUM & SK_COMPILED_IN)
    SkBindName(const void ** binary_pp);
    void assign_binary(const void ** binary_pp);
  #endif

  #if (SKOOKUM & SK_COMPILED_OUT)
    void     as_binary(void ** binary_pp) const;
    uint32_t as_binary_length() const;
  #endif

  // Copy constructing and assignment to other bind name unsupported for now
  SkBindName(const SkBindName & other) = delete;
  void operator = (const SkBindName & other) = delete;
  };

//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Determines if particular flag is enabled or not.
// Returns:    true if flag enabled, false if not
// See:        enable_flag()
// Modifiers:   static
// Author(s):   Conan Reis
inline bool SkookumScript::is_flag_set(eFlag flag)
  {
  return (ms_flags & flag) != 0u;
  }

//---------------------------------------------------------------------------------------
// Determines if particular flags are *all* enabled or not.
// Returns:    true if flags enabled, false if not
// See:        enable_flag()
// Modifiers:   static
// Author(s):   Conan Reis
inline bool SkookumScript::is_flags_set(uint32_t flags)
  {
  return (ms_flags & flags) == flags;
  }

//---------------------------------------------------------------------------------------
//  Reset the time variables
// Modifiers:    static
// Author(s):    Conan Reis
inline void SkookumScript::reset_time()
  {
  ms_sim_ticks = UINT64_C(0);
  ms_sim_time  = 0.0;
  ms_sim_delta = 0.0f;
  }

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Construct a bind name from binary
inline SkBindName::SkBindName(const void ** binary_pp)
  { 
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);
  AString string(*(const char **)binary_pp, length, true);
  (*(uint8_t **)binary_pp) += length + 1u;
  SkookumScript::get_app_info()->bind_name_construct(this, string); 
  }

//---------------------------------------------------------------------------------------
// Assign binary data to a bind name
inline void SkBindName::assign_binary(const void ** binary_pp) 
  { 
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);
  AString string(*(const char **)binary_pp, length, true);
  (*(uint8_t **)binary_pp) += length + 1u;
  SkookumScript::get_app_info()->bind_name_assign(this, string); 
  }

#endif

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Store bind name as binary data
inline void SkBindName::as_binary(void ** binary_pp) const 
  { 
  AString string = SkookumScript::get_app_info()->bind_name_as_string(*this);
  uint16_t length = (uint16_t)string.get_length();
  A_BYTE_STREAM_OUT16(binary_pp, &length);
  ::memcpy(*binary_pp, string.as_cstr(), length + 1u); // Serialize including the terminating 0
  (*(uint8_t **)binary_pp) += length + 1u;  
  }

//---------------------------------------------------------------------------------------
// Get length of bind name binary data
inline uint32_t SkBindName::as_binary_length() const 
  { 
  return SkookumScript::get_app_info()->bind_name_as_string(*this).get_length() + 3u;
  }

#endif
