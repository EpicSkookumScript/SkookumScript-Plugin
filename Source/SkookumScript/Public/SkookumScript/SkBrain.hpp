// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// The "Brain" class - holds class hierarchy and other misc. objects that do
//             not have an obvious home elsewhere.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APSorted.hpp>
#include <AgogCore/ASymbol.hpp>
#include <SkookumScript/Sk.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class AFunctionBase;
class SkObjectBase;
class SkInstance;

#ifdef A_PLAT_PC
  template<class _ElementType, class _KeyType = _ElementType, class _CompareClass = ACompareAddress<_KeyType> > class APArrayFree;
#else
  #include <AgogCore/APArray.hpp>
#endif

// Shorthand
typedef APSortedLogical<SkClass, ASymbol> tSkClasses;
typedef APArrayFree<AFunctionBase>        tSkBindFuncs;

// For revisioning the program in memory
typedef uint64_t tSkSessionGUID;
typedef uint32_t tSkRevision;

//---------------------------------------------------------------------------------------
// Notes      The "Brain" class - holds class hierarchy and other misc. objects that do
//            not have an obvious home elsewhere.
// See Also   SkClass (and other classes)
// Author(s)  Conan Reis
class SK_API SkBrain
  {
  public:
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Nested Structures

    // AEx<SkBrain> exception id values
    enum
      {
      ErrId_superclass_mismatch = AErrId_last,  // Tried to create same Class with different superclasses
      ErrId_no_superclass                       // Requested superclass does not exist
      };

    // Header for compiled binary code
    struct BinHeader
      {
      // Compiled binary code id
      uint32_t m_id;

      // Folder checksum from source scripts
      uint32_t m_checksum_folders;

      // File checksum from source scripts
      uint32_t m_checksum_files;
      };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Public Class Data Members (for quick access)

    // Revision of this program

      static tSkSessionGUID ms_session_guid;  // GUID of the compiler session that this program was compiled with
      static tSkRevision    ms_revision;      // Revision of this program associated with the above session GUID

    // Extra debug info

      static AString ms_project_name;         // Name of project this program was generated from
      static AString ms_project_path;         // Path to project ini file this program was generated from
      static AString ms_default_project_path; // Path to default project ini file this program was generated from

    // Common Classes

      static SkClass * ms_boolean_class_p;
      static SkClass * ms_class_class_p;  // In practice, the metaclass "<Object>" is used instead of "Class"
      static SkClass * ms_closure_class_p;
      static SkClass * ms_debug_class_p;
      static SkClass * ms_integer_class_p;
      static SkClass * ms_invoked_base_class_p;
      static SkClass * ms_invoked_context_base_class_p;
      static SkClass * ms_invoked_method_class_p;
      static SkClass * ms_invoked_coroutine_class_p;
      static SkClass * ms_list_class_p;
      static SkClass * ms_object_class_p;
      static SkClass * ms_real_class_p;
      static SkClass * ms_string_class_p;
      static SkClass * ms_symbol_class_p;
      static SkClass * ms_mind_class_p;
      static SkClass * ms_master_class_p;
      static SkClass * ms_actor_class_p;

    // Generic/Reflective Classes

      static SkClass * ms_auto_class_p;  // Auto_
      static SkClass * ms_this_class_p;  // ThisClass_
      static SkClass * ms_item_class_p;  // ItemClass_

    // Global Instances

      static SkInstance * ms_nil_p;  // The single instance of the None object

    // Names of special classes

      static ASymbol   ms_actor_class_name;
      static ASymbol   ms_entity_class_name;
      static ASymbol   ms_component_class_name;
      static SkClass * ms_engine_actor_class_p;

    // File management members

      // Checksums for tracking changes on the source folders and files that define the
      // class hierarchy.  Used for quick dependency checking when determining whether the
      // compiled code binary is up-to-date with the source scripts.
      static uint32_t ms_checksum_folders;
      static uint32_t ms_checksum_files;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Conversion Methods

    #if (SKOOKUM & SK_COMPILED_OUT)

      static void     as_binary(void ** binary_pp);
      static uint32_t as_binary_length();

    #endif // (SKOOKUM & SK_COMPILED_OUT)


    #if (SKOOKUM & SK_COMPILED_IN)

      static void      assign_binary(const void ** binary_pp);
      static void      assign_binary_class_hier(const void ** binary_pp, SkClass * superclass_p);
      static eAEquate  is_binary_valid(const void * binary_p);
      static eAEquate  is_binary_id_valid(uint32_t bin_id);

    #endif // (SKOOKUM & SK_COMPILED_IN)

  // Class Methods

    static SkClass * create_class(const ASymbol & class_name, SkClass * superclass_p = ms_object_class_p, uint32_t flags = ADef_uint32, uint32_t annotation_flags = 0);
    static SkClass * create_class(const ASymbol & class_name, const ASymbol & superclass_name, uint32_t flags = ADef_uint32, uint32_t annotation_flags = 0);
    static SkClass * get_class(const ASymbol & class_name);
    static SkClass * get_class(const char * class_name_p);
    static SkClass * get_class_actor();
    static bool      is_class_present(const ASymbol & class_name);

    static const tSkClasses & get_classes()  { return ms_classes; }


    // Initialization

    static void register_bind_atomics_func(void (*bind_atomics_f)());
    static void unregister_bind_atomics_func(void(*bind_atomics_f)());
    static void unregister_all_bind_atomics_funcs();
    static void initialize();
    static void initialize_core_classes(uint32_t ensure_class_count);
    static void initialize_after_classes_known(const ASymbol & startup_class_name);
    static void initialize_post_load();
    static void initialize_classes();
    static void deinitialize_classes();
    static void deinitialize_program();
    static void deinitialize();
    static void compact(bool purge_deleted_members);

    static void register_builtin_bindings(); // Call if built-in bindings need to be registered prior to initialize_post_load begin called

    #if (SKOOKUM & SK_DEBUG)

      static void ensure_atomics_registered(SkClass ** ignore_classes_pp = nullptr, uint32_t ignore_count = 0u);

    #endif // (SKOOKUM & SK_DEBUG)

  protected:

  // Internal Class Methods

    static tSkBindFuncs * get_bind_funcs();

    #if (SKOOKUM & SK_DEBUG)

      static void ensure_invokables_registered(SkClass ** ignore_classes_pp = nullptr, uint32_t ignore_count = 0u);
      static void ensure_raw_accessors_registered(SkClass ** ignore_classes_pp = nullptr, uint32_t ignore_count = 0u);

    #endif

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Class Data Members

    static tSkClasses ms_classes;

    static bool ms_builtin_bindings_registered;

  };  // SkBrain


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkBrain.inl>
#endif
