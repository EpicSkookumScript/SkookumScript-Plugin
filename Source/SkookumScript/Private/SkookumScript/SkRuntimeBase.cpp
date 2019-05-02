// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// The "Brain" class - holds class hierarchy and other misc. objects that do
//             not have an obvious home elsewhere.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkRuntimeBase.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// SkRuntimeBase Class Data Members
//=======================================================================================

SkRuntimeBase * SkRuntimeBase::ms_singleton_p = nullptr;


//=======================================================================================
// SkRuntimeBase Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor for wrapper abstract base object for system / platform specific
//             / IO-based functions.
// Author(s):   Conan Reis
SkRuntimeBase::SkRuntimeBase()
  {
  A_ASSERTX(ms_singleton_p == nullptr, "More than 1 SkRuntimeBase has been instantiated\nand there should only ever be 1.");

  ms_singleton_p = this;
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkRuntimeBase::~SkRuntimeBase()
  {
  ms_singleton_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Load the Skookum class hierarchy scripts in compiled binary form.
// 
// See Also:   load_compiled_class_group()
// Modifiers:  static
// Author(s):  Conan Reis
eSkLoadStatus SkRuntimeBase::load_compiled_hierarchy()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure compiled binary file exists
  // - This check is done before the symbol file check since the symbol file is only
  //   needed when debugging and giving an error about missing the compiled binary is
  //   more intuitive to the end user than a missing symbol file.
  if (!is_binary_hierarchy_existing())
    {
    return SkLoadStatus_not_found;
    }


  #if defined(A_SYMBOL_STR_DB_AGOG)

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1) Load SkookumScript Symbol Table
    SkBinaryHandle * sym_handle_p = get_binary_symbol_table();

    if (sym_handle_p == nullptr)
      {
      return SkLoadStatus_debug_info_not_found;
      }

    const void * sym_binary_p = sym_handle_p->m_binary_p;

    ASymbolTable::ms_main_p->merge_binary(&sym_binary_p);
    SK_ASSERTX((const char *)sym_binary_p == (const char *)sym_handle_p->m_binary_p + sym_handle_p->m_size, "Binary symbol file does not have correct size!");

    release_binary(sym_handle_p);

  #endif


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2) Load SkookumScript compiled binary
  SkBinaryHandle * hierarchy_handle_p  = get_binary_hierarchy();
  const void **    hierarchy_binary_pp = (const void **)&hierarchy_handle_p->m_binary_p;
  eAEquate         version = SkBrain::is_binary_valid(*hierarchy_binary_pp);

  if (version != AEquate_equal)
    {
    SkDebug::print_agog(a_str_format(
      "  Compiled binary file is %s!\n\n",
      (version == AEquate_less)
        ? "stale, engine is newer"
        : "newer, engine is older"));

    release_binary(hierarchy_handle_p);

    return (version == AEquate_less)
        ? SkLoadStatus_stale
        : SkLoadStatus_runtime_old;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean out anything remaining from a previous session
  if (SkookumScript::get_initialization_level() > SkookumScript::InitializationLevel_none)
    {
    // Sim might not be running
    if (SkookumScript::get_initialization_level() >= SkookumScript::InitializationLevel_sim)
      {
      SkookumScript::deinitialize_sim();
      SkookumScript::deinitialize_program();
      }
    SkookumScript::deinitialize();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 3) Starts up SkookumScript - Registers/connects atomic classes, coroutines, etc. that
  // do not require the code or compiled binary to be already loaded.
  SkookumScript::initialize();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 4) Convert compiled binary to data structures
  #if (SKOOKUM & SK_DEBUG)
    const void * binary_mem_p = *hierarchy_binary_pp;
  #endif
  SkBrain::assign_binary(hierarchy_binary_pp);
  SK_ASSERTX((uint8_t *)*hierarchy_binary_pp - (uint8_t *)binary_mem_p == (ptrdiff_t)hierarchy_handle_p->m_size, a_str_format("Inconsistent binary length of loaded compiled binary (expected: %d, actual: %d)!", hierarchy_handle_p->m_size, (uint8_t *)*hierarchy_binary_pp - (uint8_t *)binary_mem_p));
  release_binary(hierarchy_handle_p);

  // Note: Do _not_ initialize program and sim here - will be done later in bind_compiled_scripts()

  return SkLoadStatus_ok;
  }

//---------------------------------------------------------------------------------------
// Loads group of classes the specified class belongs to if not already loaded.
// This can be used as a mechanism to "demand load" scripts.
// 
// See:        load_compiled_scripts()
// Modifiers:   virtual - overridden from SkRuntimeBase
// Author(s):   Conan Reis
void SkRuntimeBase::load_compiled_class_group(SkClass * class_p)
  {
  SkClass * root_p = class_p->get_demand_loaded_root();

  if (root_p == nullptr)
    {
    return;
    }

  if (!root_p->is_loaded())
    {
    SkBinaryHandle * bin_handle_p = get_binary_class_group(*root_p);

    // $Revisit - CReis Handle binary not available.

    void * binary_p = bin_handle_p->m_binary_p;

    // 4 bytes - number of classes (excluding demand loaded)
    // n bytes - SkClass or SkActorClass binary }- Repeating
    SkClass::from_binary_group((const void **)&binary_p);

    release_binary(bin_handle_p);

    // $Revisit - CReis Should call optional "register bindings" callback for class group

    // Call class constructors as requested
    if (SkRemoteBase::ms_default_p->should_class_ctors_be_called())
      {
      // initialize classes
      root_p->invoke_class_ctor_recurse();
      }
    }

  // Also turns off deferred unload if it is set
  root_p->set_loaded();
  }

//---------------------------------------------------------------------------------------
// Loads all demand load classes as needed.
// Author(s):   Conan Reis
void SkRuntimeBase::load_compiled_class_group_all()
  {
  uint32_t length = SkBrain::get_classes().get_length();

  if (length)
    {
    SkClass *  class_p;
    SkClass ** classes_pp     = SkBrain::get_classes().get_array();
    SkClass ** classes_end_pp = classes_pp + length;

    for (; classes_pp < classes_end_pp; classes_pp++)
      {
      class_p = *classes_pp;

      if (!class_p->is_loaded())
        {
        //ADebug::print_format("  %s\n", class_p->get_name_cstr());
        load_compiled_class_group(class_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Override to add bindings to any custom C++ routines (methods & coroutines).
//
// See Also   SkBrain: :register_bind_atomics_func()
// #Modifiers  virtual
// #Author(s)  Conan Reis
void SkRuntimeBase::on_bind_routines()
  {
  }

//---------------------------------------------------------------------------------------

void SkRuntimeBase::on_initialization_level_changed(SkookumScript::eInitializationLevel from_level, SkookumScript::eInitializationLevel to_level)
  {
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
// 
// [1st most preferred - This update most accurately reflects the game time since all the
// time values are provided.]
// 
// Notes:  Only one of the update() routines needs to be called each frame.
// 
// Params: 
//   sim_ticks: Simulation elapsed time (in milliseconds) since game / level start.
//   sim_time:  Simulation elapsed time (in seconds) since game / level start.
//   sim_delta: Simulation delta time (in seconds) since last update.
//   
// See:        other versions of update()
// Modifiers:  static
// Author(s):  Conan Reis
void SkRuntimeBase::update(
  uint64_t sim_ticks,
  f64  sim_time,
  f32  sim_delta
  )
  {
  SkookumScript::update(sim_ticks, sim_time, sim_delta);
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
// 
// [2nd most preferred - This update provides a good reflection of game time with no
// between update accumulation of errors.]
// 
// Notes:  Only one of the update() routines needs to be called each frame.
// 
// Params: 
//   sim_ticks: Simulation elapsed time (in milliseconds) since game / level start.
//   
// See:        other versions of update()
// Modifiers:  static
// Author(s):  Conan Reis
void SkRuntimeBase::update(uint64_t sim_ticks)
  {
  SkookumScript::update_ticks(sim_ticks);
  }

//---------------------------------------------------------------------------------------
// Updates all time dependent script objects (generally coroutines)
// 
// [3rd most preferred - This update provides the least accurate reflection of the game
// time since between update accumulation errors can occur.]
// 
// Notes:  Only one of the update() routines needs to be called each frame.
// 
// Params: 
//   sim_delta: Simulation delta time (in seconds) since last update.
//
// See:        other versions of update()
// Modifiers:  static
// Author(s):  Conan Reis
void SkRuntimeBase::update(f32 sim_delta)
  {
  SkookumScript::update_delta(sim_delta);
  }

//---------------------------------------------------------------------------------------
// Modifiers:   static
// Author(s):   Conan Reis
void SkRuntimeBase::bind_routines()
  {
  ms_singleton_p->on_bind_routines();
  }

//---------------------------------------------------------------------------------------

void SkRuntimeBase::initialization_level_changed(SkookumScript::eInitializationLevel from_level, SkookumScript::eInitializationLevel to_level)
  {
  ms_singleton_p->on_initialization_level_changed(from_level, to_level);
  }

