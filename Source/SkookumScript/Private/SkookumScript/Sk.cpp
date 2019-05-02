// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript common definitions.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <AgogCore/AObjReusePool.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkActorClass.hpp>
#include <SkookumScript/SkParser.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// Common/Local Data
//=======================================================================================

// The UE4 build system requires to link with a function named like this, so give it one
void EmptyLinkFunctionForStaticInitializationSkookumScript(void) {}

//=======================================================================================
// SkookumScript Class Data Members
//=======================================================================================

SkAppInfo * SkookumScript::ms_app_info_p = nullptr;

SkClass *               SkookumScript::ms_startup_class_p         = nullptr;
SkMind *                SkookumScript::ms_master_mind_p           = nullptr;
SkProgramUpdateRecord * SkookumScript::ms_program_update_record_p = nullptr;

void (* SkookumScript::ms_on_initialization_level_changed_f)(SkookumScript::eInitializationLevel, SkookumScript::eInitializationLevel) = nullptr;
void (* SkookumScript::ms_on_update_request_f)(bool update_req_b) = nullptr;
void (* SkookumScript::ms_update_time_f)() = nullptr;
void (* SkookumScript::ms_on_script_linear_bytes_f)(uint32_t bytes_needed) = nullptr;

SkookumScript::eInitializationLevel SkookumScript::ms_initialization_level  = SkookumScript::InitializationLevel_none;
uint32_t                            SkookumScript::ms_flags                 = Flag__none;

uint64_t SkookumScript::ms_sim_ticks = UINT64_C(0);
f64      SkookumScript::ms_sim_time  = 0.0;
f32      SkookumScript::ms_sim_delta = 0.0f;


//=======================================================================================
// SkookumScript Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Updates script time
// Arg         sim_ticks - Simulation ticks (in milliseconds) since game / level start.
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update_time_ticks(uint64_t sim_ticks)
  {
  ms_sim_delta = f32(sim_ticks - ms_sim_ticks) * 0.001f;
  ms_sim_time  = f64(sim_ticks) * 0.001;
  ms_sim_ticks = sim_ticks;
  }


//---------------------------------------------------------------------------------------
// Gets the master mind, or if not present, its meta class
SkInstance * SkookumScript::get_master_mind_or_meta_class()
  {
  // Do we have a master mind?
  if (ms_master_mind_p)
    {
    // Yes, then that's what we want
    return ms_master_mind_p;
    }

  // Nope, then return its meta class instead
  SK_ASSERTX(ms_startup_class_p, "Startup class not found!");
  return &ms_startup_class_p->get_metaclass();
  }

//---------------------------------------------------------------------------------------
// Enables or disables a specific scripting state.
// Arg         flag - flag to change the state of
// Arg         enable_b - indicates whether the showing of status information should be
//             enabled (true) or disabled (false).
// See:        is_flag_set()
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::enable_flag(
  eFlag flag,
  bool  enable_b //= true
  )
  {
  // Ignore if flag was already set in the manner desired
  if (((ms_flags & flag) != 0u) != enable_b)
    {
    ms_flags += enable_b ? flag : -flag;

    // Special stuff for Flag_evaluate
    if ((flag == Flag_paused) && ms_on_update_request_f && (ms_flags & Flag_need_update))
      {
      ms_on_update_request_f(!enable_b);
      }

    #if (SKOOKUM & SK_DEBUG)
      switch (flag)
        {
        case Flag_paused:  // Toggle updating of scripts
          SkDebug::print(a_str_format("Script Evaluation %s.\n", (enable_b ? "disabled" : "enabled")));
          break;
        case Flag_trace:     // Trace Scripts
          SkDebug::print(a_str_format("Script Tracing %s.\n", (enable_b ? "enabled" : "disabled")));
          break;
        default: // Make Clang happy
          break;
        }
    #endif
    }
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
//             [This update most accurately reflects the game time since all the time
//             values are provided.]
// Arg         sim_ticks - Simulation elapsed time (in milliseconds) since game / level start.
// Arg         sim_time  - Simulation elapsed time (in seconds) since game / level start.
// Arg         sim_delta - Simulation delta time (in seconds) since last update.
// See:        update_ticks(), update_delta()
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update(
  uint64_t sim_ticks,
  f64  sim_time,
  f32  sim_delta
  )
  {
  ms_sim_ticks = sim_ticks;
  ms_sim_time  = sim_time;

  // $Note - CReis If the scripts are not being evaluated for a while and then script
  // evaluation is re-enabled the script sim delta time just uses the most recent delta.
  // Alternatively the script sim delta time could accumulate until script evaluation is
  // re-enabled however this could cause unpredictable jumps.
  ms_sim_delta = sim_delta;

  if (ms_initialization_level >= InitializationLevel_sim && is_flags_set(Flag_need_update) && !is_flags_set(Flag_paused))
    {
    // Update all actors
    SkMind::update_all();
    }
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
//             [This update mirrors game time fairly accurately with no between update
//             accumulation errors.]
// Arg         sim_ticks - Simulation elapsed time (in milliseconds) since game / level start.
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update_ticks(uint64_t sim_ticks)
  {
  // See Note [1] about script deltas in SkookumScript::update()
  update_time_ticks(sim_ticks);

  if (ms_initialization_level >= InitializationLevel_sim && is_flags_set(Flag_need_update) && !is_flags_set(Flag_paused))
    {
    // Update all actors
    SkMind::update_all();
    }
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
// Arg         sim_delta - Simulation delta time (in seconds) since last update.
//             [This update provides the least accurate mirror of game time since between
//             update accumulation errors can occur.]
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update_delta(f32 sim_delta)
  {
  // See Note [1] about script deltas in SkookumScript::update()
  ms_sim_delta  = sim_delta;
  ms_sim_time  += sim_delta;
  ms_sim_ticks  = uint64_t((ms_sim_time * 1000.0) + 0.5);

  if (ms_initialization_level >= InitializationLevel_sim && is_flags_set(Flag_need_update) && !is_flags_set(Flag_paused))
    {
    // Update all actors
    SkMind::update_all();
    }
  }

//---------------------------------------------------------------------------------------
// External time update
// See:        register_update_time_func()
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update_time()
  {
  if (ms_update_time_f)
    {
    ms_update_time_f();
    }
  }

//---------------------------------------------------------------------------------------
// Change status of whether scripts need updating and if scripts are being
//             updated, notify interested parties.
// Arg         update_req_b - true if scripts need updating, false if not
// See:        register_update_request_func()
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::update_request(
  bool update_req_b // = true
  )
  {
  if (update_req_b)
    {
    ms_flags |= Flag_need_update;
    }
  else
    {
    ms_flags &= ~Flag_need_update;
    }

  if (ms_update_time_f)
    {
    ms_update_time_f();
    }

  if (ms_on_update_request_f && ms_initialization_level >= InitializationLevel_sim)
    {
    ms_on_update_request_f(update_req_b);
    }
  }

//---------------------------------------------------------------------------------------

void SkookumScript::set_app_info(SkAppInfo * app_p)
  {
  ms_app_info_p = app_p;
  }

//---------------------------------------------------------------------------------------

SkAppInfo * SkookumScript::get_app_info()
  {
  A_ASSERTX(ms_app_info_p, "SkookumScript app interface not set! SkookumScript::set_app() must be called by the enclosing app before this point in code is reached.");
  return ms_app_info_p;
  }

//---------------------------------------------------------------------------------------
// Registers a function that binds methods, coroutines, etc. that
//             were previously declared (via SkookumScript source code prototype files) to
//             their atomic C++ counterparts.
// Arg         bind_atomics_f - function pointer to register
// Examples:   SkookumScript::register_bind_atomics_func(SomeLibClass::bind_atomics);
// See:        SkBrain::register_bind_atomics_func() - same function, but it requires
//             more header files to be included.
// Notes:      This should be called before SkookumScript::initialize_program() (probably
//             in main() or its equivalent) for any external library that needs to bind
//             C++ atomics.
//
//             Any registered functions will be called during SkookumScript::initialize_program().
//
//             It would be nice if this class could be used as a global whose constructor
//             had the side effect of registering the bind atomics function, but (after
//             much much research) it seems that any libraries that used this technique
//             could optimize out the global and its constructor since it would not be
//             referenced in the main module.  A call to SkookumScript::register_bindings_func()
//             should be used instead.
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::register_bind_atomics_func(void (*bind_atomics_f)())
  {
  SkBrain::register_bind_atomics_func(bind_atomics_f);
  }

//---------------------------------------------------------------------------------------
// Ensures that all pooled data structures are loaded in memory
// Modifiers:   static
// Author(s):   Conan Reis
void SkookumScript::pools_reserve()
  {
  SkInstance::get_pool().reset(get_app_info()->get_pool_init_instance(), get_app_info()->get_pool_incr_instance());
  SkDataInstance::get_pool().reset(get_app_info()->get_pool_init_data_instance(), get_app_info()->get_pool_incr_data_instance(), false);
  SkInvokedExpression::get_pool().reset(get_app_info()->get_pool_init_iexpr(), get_app_info()->get_pool_incr_iexpr());
  SkInvokedCoroutine::get_pool().reset(get_app_info()->get_pool_init_icoroutine(), get_app_info()->get_pool_incr_icoroutine());
  }

//---------------------------------------------------------------------------------------

void SkookumScript::pools_empty()
  {
  SkInstance::get_pool().empty();
  SkDataInstance::get_pool().empty();
  SkInvokedExpression::get_pool().empty();
  SkInvokedCoroutine::get_pool().empty();
  }

//---------------------------------------------------------------------------------------

void SkookumScript::set_initialization_level(eInitializationLevel level)
  {
  eInitializationLevel prev_level = ms_initialization_level;
  ms_initialization_level = level;
  if (ms_on_initialization_level_changed_f)
    {
    (*ms_on_initialization_level_changed_f)(prev_level, level);
    }
  }

//---------------------------------------------------------------------------------------
// SkookumScript main initialization sequence
// Initialize from InitializationLevel_none to InitializationLevel_minimum
void SkookumScript::initialize()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_none, "Unexpected initialization level.");

  // Assign values to the static symbols
  SkSymbolDefs::initialize();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that pooled data structures are allocated to ensure AI memory uses contiguous
  // blocks with few if any dynamic allocations during the game update loop.
  SkookumScript::pools_reserve();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SkDebug::initialize();
  SkClass::initialize();
  SkBrain::initialize();

  #if (SKOOKUM & SK_CODE_IN)
    SkParser::initialize();
  #endif

  // Set new initialization level
  set_initialization_level(InitializationLevel_minimum);
  }

//---------------------------------------------------------------------------------------
// SkookumScript main initialization sequence
// Initialize from InitializationLevel_minimum to InitializationLevel_program
// Registers/connects atomic classes, coroutines, etc. with the previously
// loaded parameters.
void SkookumScript::initialize_program()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_minimum, "Unexpected initialization level.");

  SkBrain::initialize_post_load();

  // Set new initialization level
  set_initialization_level(InitializationLevel_program);
  }

//---------------------------------------------------------------------------------------
// SkookumScript main initialization sequence
// Initialize from InitializationLevel_program to InitializationLevel_sim
void SkookumScript::initialize_sim()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_program, "Unexpected initialization level.");

  SkDebug::print_agog("\nSkookumScript initializing session...\n", SkLocale_all, SkDPrintType_title);

  SK_ASSERTX(SkBrain::ms_object_class_p, "Binaries must have been loaded when SkookumScript::initialize_sim() is called!");
  if (SkBrain::ms_object_class_p) // Prevent crash if the above test failed
    {
    SK_ASSERTX(!ms_master_mind_p, "Master Mind must not have been constructed yet!");

  #ifdef SKOOKUM_REMOTE
    // Conditionally call class constructors if Remote IDE
    if (!SkRemoteBase::ms_default_p || SkRemoteBase::ms_default_p->should_class_ctors_be_called())
  #endif
      {
      // Call any class constructors
      SkBrain::initialize_classes();
      }
    }

  // Create the update record (for now, persistent during the entire sim phase)
  if (!ms_program_update_record_p)
    {
    ms_program_update_record_p = new SkProgramUpdateRecord;
    }

  // Set new initialization level
  set_initialization_level(InitializationLevel_sim);

  // Legacy - serve ms_on_update_request_f
  if (ms_on_update_request_f && (ms_flags & Flag_need_update))
    {
    ms_on_update_request_f(true);
    }
  }

//---------------------------------------------------------------------------------------
// SkookumScript main initialization sequence
// Initialize from InitializationLevel_sim to InitializationLevel_gameplay
// Create master mind etc.
void SkookumScript::initialize_gameplay(bool create_master_mind /*= true*/)
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_sim, "Unexpected initialization level.");

  SK_ASSERTX(SkBrain::ms_object_class_p, "Binaries must have been loaded when SkookumScript::initialize_sim() is called!");
  if (SkBrain::ms_object_class_p) // Prevent crash if the above test failed
    {
    if (create_master_mind)
      {
      SkDebug::print(
        a_str_format(
          "\n\nCreating startup master mind object using class '%s'\n\n", ms_startup_class_p->get_name_cstr_dbg()),
        SkLocale_all,
        SkDPrintType_note);

      SK_ASSERTX(!ms_master_mind_p, "Tried to initialize Master Mind twice in a row!");
      ms_master_mind_p = SK_NEW(SkMind)(ms_startup_class_p);
      ms_master_mind_p->reference();
      ms_master_mind_p->call_default_constructor();
      }
    }

  SkookumScript::reset_time();

  // Set new initialization level
  set_initialization_level(InitializationLevel_gameplay);
  }

//---------------------------------------------------------------------------------------
// SkookumScript main deinitialization sequence
// Deinitialize from InitializationLevel_gameplay to InitializationLevel_sim
void SkookumScript::deinitialize_gameplay()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_gameplay, "Unexpected initialization level.");

  // Clean up master mind
  if (ms_master_mind_p)
    {
    ms_master_mind_p->abort_coroutines();
    SK_ASSERTX(ms_master_mind_p->get_references() == 1, "Master Mind has stray references.");
    ms_master_mind_p->on_no_references();
    ms_master_mind_p = nullptr;
    }

  // Make sure there are no minds or coroutines lingering around
  SkMind::abort_all_coroutines();

  // Clear out actor stragglers
  if (SkookumScript::get_app_info()->use_builtin_actor())
    {
    SkActorClass * actor_class_p = static_cast<SkActorClass *>(SkBrain::ms_actor_class_p);
    SkActor * straggler_p;
    while ((straggler_p = actor_class_p->get_instances().get_last()) != nullptr)
      {
      straggler_p->on_no_references();
      }
    }

  // While all coroutines have been terminated, purge and re-create the update record to recycle its memory
  if (ms_program_update_record_p)
    {
    delete ms_program_update_record_p;
    ms_program_update_record_p = new SkProgramUpdateRecord;
    }

  // Set new initialization level
  set_initialization_level(InitializationLevel_sim);
  }

//---------------------------------------------------------------------------------------
// SkookumScript main deinitialization sequence
// Deinitialize from InitializationLevel_sim to InitializationLevel_program
void SkookumScript::deinitialize_sim()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_sim, "Unexpected initialization level.");

  SK_MAD_ASSERTX(!ms_master_mind_p, "Master Mind not deinitialized before calling SkookumScript::deinitialize_sim()! Call SkookumScript::deinitialize_gameplay() beforehand!");

  // Deinitialize SkookumScript classes
  SkBrain::deinitialize_classes();

  // Recycle program update record memory at this point
  if (ms_program_update_record_p)
    {
    delete ms_program_update_record_p;
    ms_program_update_record_p = nullptr;
    }

  // Set new initialization level
  set_initialization_level(InitializationLevel_program);

  // Legacy - serve ms_on_update_request_f
  if (ms_on_update_request_f && (ms_flags & Flag_need_update))
    {
    ms_on_update_request_f(false);
    }
  }

//---------------------------------------------------------------------------------------
// SkookumScript main deinitialization sequence
// Deinitialize from InitializationLevel_program to InitializationLevel_minimum
// Deletes the program from SkBrain memory
void SkookumScript::deinitialize_program()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_program, "Unexpected initialization level.");

  // Clear brain
  SkBrain::deinitialize_program();

  // Set new initialization level
  set_initialization_level(InitializationLevel_minimum);
  }

//---------------------------------------------------------------------------------------
// SkookumScript main deinitialization sequence
// Deinitialize from InitializationLevel_minimum to InitializationLevel_none
// Unloads SkookumScript and cleans-up.
void SkookumScript::deinitialize()
  {
  SK_MAD_ASSERTX(ms_initialization_level == InitializationLevel_minimum, "Unexpected initialization level.");

  #if (SKOOKUM & SK_CODE_IN)
    SkParser::deinitialize();
  #endif

  ms_startup_class_p = nullptr;
  SkBrain::deinitialize();
  SkClass::deinitialize();

  // Clear up debug stuff
  SkDebug::deinitialize();

  // Free up pooled data structures
  SkookumScript::pools_empty();

  // Clean up static symbols
  SkSymbolDefs::deinitialize();

  // Set new initialization level
  set_initialization_level(InitializationLevel_none);
  }

//---------------------------------------------------------------------------------------

void SkookumScript::register_on_initialization_level_changed_func(void (*on_initialization_level_changed_f)(eInitializationLevel, eInitializationLevel))
  {
  SK_ASSERTX(!ms_on_initialization_level_changed_f, "`SkookumScript::register_on_initialization_level_changed_func()` invoked twice in a row. Only one callback is currently supported.");
  ms_on_initialization_level_changed_f = on_initialization_level_changed_f;
  }

