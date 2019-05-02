// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// The "Brain" class - holds class hierarchy and other misc. objects that do
//             not have an obvious home elsewhere.
// # Notes:        
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkBrain.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkBrain.inl>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AFunction.hpp>
#include <AgogCore/AFunctionArg.hpp>
#include <AgogCore/AString.hpp>
#include <SkookumScript/SkActorClass.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkEnum.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkNone.hpp>
#include <SkookumScript/SkObject.hpp>
#include <SkookumScript/SkRandom.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkRemoteRuntimeBase.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkTypedClass.hpp>


//=======================================================================================
// Global Variables
//=======================================================================================

namespace
{

  // Looks like "CBC0DE" in file - i.e. compiled binary code
  const uint32_t SkBrain_bin_code_id_type = 0x00DEC0CB;
  // Version between 0 and 255 (should be safe to cycle back to 0 once 255 is passed)
  const uint32_t SkBrain_bin_code_id_version = 61u;

  // Bits to shift for version code id
  const uint32_t SkBrain_bin_code_id_version_shift = 24u;

  // Id code for [C]ompiled [B]inary [Code] version [NN]
  // Looks like "CBC0DENN" in file
  const uint32_t SkBrain_bin_code_id = (SkBrain_bin_code_id_version << SkBrain_bin_code_id_version_shift) + SkBrain_bin_code_id_type;

} // End unnamed namespace


//=======================================================================================
// Local Global Function Definitions
//=======================================================================================


//=======================================================================================
// SkBrain Class Data Members
//=======================================================================================


tSkSessionGUID SkBrain::ms_session_guid = 0;  // No session specified yet
tSkRevision    SkBrain::ms_revision = 0;      // Revision 0 corresponds to an empty program

AString        SkBrain::ms_project_name;         // Name of project this program was generated from
AString        SkBrain::ms_project_path;         // Path to project ini file this program was generated from
AString        SkBrain::ms_default_project_path; // Path to default project ini file this program was generated from

SkClass *      SkBrain::ms_auto_class_p                  = nullptr;  // Auto_
SkClass *      SkBrain::ms_boolean_class_p               = nullptr;
SkClass *      SkBrain::ms_class_class_p                 = nullptr;
SkClass *      SkBrain::ms_closure_class_p               = nullptr;
SkClass *      SkBrain::ms_debug_class_p                 = nullptr;
SkClass *      SkBrain::ms_integer_class_p               = nullptr;
SkClass *      SkBrain::ms_invoked_base_class_p          = nullptr;
SkClass *      SkBrain::ms_invoked_context_base_class_p  = nullptr;
SkClass *      SkBrain::ms_invoked_coroutine_class_p     = nullptr;
SkClass *      SkBrain::ms_invoked_method_class_p        = nullptr;
SkClass *      SkBrain::ms_item_class_p                  = nullptr;  // ItemClass_
SkClass *      SkBrain::ms_list_class_p                  = nullptr;
SkClass *      SkBrain::ms_object_class_p                = nullptr;
SkClass *      SkBrain::ms_real_class_p                  = nullptr;
SkClass *      SkBrain::ms_string_class_p                = nullptr;
SkClass *      SkBrain::ms_symbol_class_p                = nullptr;
SkClass *      SkBrain::ms_this_class_p                  = nullptr;  // ThisClass_
SkClass *      SkBrain::ms_mind_class_p                  = nullptr;
SkClass *      SkBrain::ms_master_class_p                = nullptr;
SkClass *      SkBrain::ms_actor_class_p                 = nullptr;

SkInstance *   SkBrain::ms_nil_p                         = nullptr;

ASymbol        SkBrain::ms_actor_class_name;
ASymbol        SkBrain::ms_entity_class_name;
ASymbol        SkBrain::ms_component_class_name;
SkClass *      SkBrain::ms_engine_actor_class_p          = nullptr;

uint32_t       SkBrain::ms_checksum_folders              = 0u;
uint32_t       SkBrain::ms_checksum_files                = 0u;

tSkClasses     SkBrain::ms_classes;

bool           SkBrain::ms_builtin_bindings_registered = false;

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Retrieves the requested class or nullptr if the class does not yet exist.
// # Returns:  The requested class or nullptr if the class does not yet exist
// Arg         name - Name of the class.  It should follow the form of "ClassName"
//             uppercase {alphanumeric}
// # See:      is_class_present(), create_class()
// # Modifiers: static
// # Author(s): Conan Reis
SkClass * SkBrain::get_class(const ASymbol & class_name)
  {
  return ms_classes.get(class_name);
  }

//---------------------------------------------------------------------------------------
// Retrieves the requested class or nullptr if the class does not yet exist.
// # Returns:  The requested class or nullptr if the class does not yet exist
// Arg         name - Name of the class (it is converted to a ASymbol which can be
//             expensive so if this method is called repeatedly with the same name a symbol
//             should be created from the name and the symbol version of this method
//             should be called instead.
//             It should follow the form of "ClassName" uppercase {alphanumeric}
// # See:      is_class_present(), create_class()
// # Modifiers: static
// # Author(s): Conan Reis
SkClass * SkBrain::get_class(const char * class_name_p)
  {
  return ms_classes.get(ASymbol::create(class_name_p, ALength_calculate, ATerm_short));
  }

//---------------------------------------------------------------------------------------
// Retrieves the engine-specific actor class using name stored in `ms_actor_class_name`
// which is usually `Actor`.
// 
// # Returns:   engine-specific actor class
// # See:       get_class()
// # Modifiers: static
// # Author(s): Conan Reis
SkClass * SkBrain::get_class_actor()
  {
  if (ms_engine_actor_class_p == nullptr)
    {
    ms_engine_actor_class_p = get_class(ms_actor_class_name);
    }

  return ms_engine_actor_class_p;
  }

//---------------------------------------------------------------------------------------
// Determines if a class with the specified name exists or not.
// # Returns:  true if class exists, false if not
// Arg         name - Name of the class.  It should follow the form of "ClassName"
//             uppercase {alphanumeric}
// # See:      get_class()
// # Modifiers: static
// # Author(s): Conan Reis
bool SkBrain::is_class_present(const ASymbol & class_name)
  {
  return ms_classes.find(class_name);
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to recreate the
// SkookumScript class hierarchy and increments the memory address to just past the last
// byte written.
// 
// ##binary_pp
//   Pointer to address to fill and increment.  Its size *must* be large enough to fit all
//   the binary data.  Use the get_binary_length() method to determine the size needed
//   prior to passing binary_pp to this method.
//   
// #Notes
//   Used in combination with as_binary_length().
//
//   Binary composition:
//     4 bytes - SkookumScript compiled binary code id
//     4 bytes - folder checksum from source scripts
//     4 bytes - file checksum from source scripts
//     4 bytes - bytes needed for linear allocation of hierarchy
//     4 bytes - optional additional debug bytes needed for linear allocation of hierarchy
//     4 bytes - number of classes (including demand loaded)
//     n bytes - class hierarchy placeholders   }- Repeating & Recursive
//     4 bytes - startup mind class id
//     4 bytes - number of typed classes
//     4 bytes - number of invokable classes
//     4 bytes - number of class unions
//     n bytes - class union binary             }- Repeating
//     n bytes - typed class binary             }- Repeating
//     n bytes - invokable class binary         }- Repeating
//     4 bytes - number of classes (excluding demand loaded)
//     n bytes - SkClass or SkActorClass binary }- Repeating
//
//   $Note - CReis This could be packed more
//   
// #See Also:  as_binary_length(), assign_binary(binary_pp)
// #Author(s): Conan Reis
void SkBrain::as_binary(void ** binary_pp)
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkBrain::as_binary_length());

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Memory

  // Determine how much memory is needed for reload
  AMemoryStats mem_stats(AMemoryStats::Track_needed);

  // Skip demand loaded classes by passing "true"
  bool skip_demand_loaded = true;

  ms_object_class_p->track_memory_recursive(&mem_stats, skip_demand_loaded);

  // $Vital - CReis Some of the parameter interfaces may be shared with demand loaded classes
  SkParameters::shared_track_memory(&mem_stats);
  SkInvokableClass::shared_track_memory(&mem_stats);
  SkClassUnion::shared_track_memory(&mem_stats);
  SkTypedClass::shared_track_memory(&mem_stats);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 4 bytes - SkookumScript compiled binary code id
  uint32_t value = SkBrain_bin_code_id;
  A_BYTE_STREAM_OUT32(binary_pp, &value);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Zero out checksums - write them out last (helps identify aborted binary writes)
  // 4 bytes - folder checksum from source scripts
  value = 0u;
  uint32_t * folder_checksum_p = (uint32_t *)*binary_pp;
  A_BYTE_STREAM_OUT32(binary_pp, &value);

  // 4 bytes - file checksum from source scripts
  uint32_t * file_checksum_p = (uint32_t *)*binary_pp;
  A_BYTE_STREAM_OUT32(binary_pp, &value);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 12 bytes - session id and revision
  static_assert(sizeof(ms_session_guid) == sizeof(uint64_t) && sizeof(ms_revision) == sizeof(uint32_t), "Sizes must match.");
  A_BYTE_STREAM_OUT64(binary_pp, &ms_session_guid);
  A_BYTE_STREAM_OUT32(binary_pp, &ms_revision);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // n bytes - project debug info
  ms_project_name.as_binary(binary_pp);
  ms_project_path.as_binary(binary_pp);
  ms_default_project_path.as_binary(binary_pp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 4 bytes - bytes needed for linear allocation of hierarchy
  uint32_t length = mem_stats.m_size_needed;
  A_BYTE_STREAM_OUT32(binary_pp, &length);

  // 4 bytes - optional additional debug bytes needed for linear allocation of hierarchy
  length = mem_stats.m_size_needed_debug;
  A_BYTE_STREAM_OUT32(binary_pp, &length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #1 - The class hierarchy is written first so that calls to get_class() will
  // work during the construction of class unions, methods, coroutines, etc.

  // 4 bytes - number of classes (including demand loaded)
  length = ms_classes.get_length();
  A_BYTE_STREAM_OUT32(binary_pp, &length);

  // n bytes - class hierarchy placeholder }- Repeating & Recursive
  ms_object_class_p->as_binary_placeholder_recurse(binary_pp);

  // 4 bytes - startup mind class id
  SkookumScript::ms_startup_class_p->get_name().as_binary(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #2a - The number of common/shared type classes are written next so that array
  // buffer size will be the correct, etc.  Note that typed classes may reference each other.
  // 4 bytes - number of typed classes
  length = SkTypedClass::ms_typed_classes.get_length();
  A_BYTE_STREAM_OUT32(binary_pp, &length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #3a - The number of common/shared invokable classes are written next so that
  // references (which are an index into this common list) will be able to resolve.  Note
  // that even invokable classes may reference each other.
  // 4 bytes - number of invokable classes
  length = SkInvokableClass::ms_shared_classes.get_length();
  A_BYTE_STREAM_OUT32(binary_pp, &length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #4 - The common/shared class unions are written next so that calls to
  // SkClassDescBase::from_binary_ref() will work during the construction of methods,
  // coroutines, etc.
  // 4 bytes - number of class unions
  // n bytes - class union binary }- Repeating
  SkClassUnion::ms_shared_unions.as_binary(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #2b - The contents of common/shared type classes are now written.
  // n bytes - typed class binary }- Repeating
  SkTypedClass::ms_typed_classes.as_binary_elems(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #3b - The contents of common/shared invokable classes are now written.
  // n bytes - invokable class binary }- Repeating
  SkInvokableClass::ms_shared_classes.as_binary_elems(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #5 - The class members are written last.
  // 4 bytes - number of classes (excluding demand loaded)
  // n bytes - SkClass or SkActorClass binary }- Repeating
  ms_object_class_p->as_binary_group(binary_pp, skip_demand_loaded);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write out checksums
  // - done last (helps identify partially written / aborted binary writes)
  *folder_checksum_p = ms_checksum_folders;
  *file_checksum_p   = ms_checksum_files;
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// 
// Returns: length of binary version of itself in bytes
// 
// Notes:
//   Binary composition:
//     4 bytes - SkookumScript compiled binary code id
//     4 bytes - folder checksum from source scripts
//     4 bytes - file checksum from source scripts
//     4 bytes - bytes needed for linear allocation of hierarchy
//     4 bytes - optional additional debug bytes needed for linear allocation of hierarchy
//     4 bytes - number of classes (including demand loaded)
//     n bytes - class hierarchy placeholders   }- Repeating & Recursive
//     4 bytes - startup mind class id
//     4 bytes - number of typed classes
//     4 bytes - number of invokable classes
//     4 bytes - number of class unions
//     n bytes - class union binary             }- Repeating
//     n bytes - typed class binary             }- Repeating
//     n bytes - invokable class binary         }- Repeating
//     4 bytes - number of classes (excluding demand loaded)
//     n bytes - SkClass/SkActorClass members   }- Repeating
//
//   Little error checking on binary as it is assumed to have been validated when saved.
// 
// See: as_binary(), assign_binary()
uint32_t SkBrain::as_binary_length()
  {
  // id(4) + checksums(8) + session_guid(8) + revision(4) + linear size(4) + debug linear size(4) + startup mind class id(4)
  // + number of classes(4) + number of typed classes(4) + number of invokable classes(4)
  uint32_t binary_length = 48u;

  // n bytes - project debug info
  binary_length += ms_project_name.as_binary_length();
  binary_length += ms_project_path.as_binary_length();
  binary_length += ms_default_project_path.as_binary_length();

  // n bytes - class hierarchy placeholders   }- Repeating & Recursive
  binary_length += ms_object_class_p->as_binary_placeholder_recurse_length();

  // 4 bytes - number of class unions
  // n bytes - class union binary }- Repeating
  binary_length += SkClassUnion::ms_shared_unions.as_binary_length();

  // n bytes - typed class binary }- Repeating
  binary_length += SkTypedClass::ms_typed_classes.as_binary_elems_length();

  // n bytes - invokable class binary }- Repeating
  binary_length += SkInvokableClass::ms_shared_classes.as_binary_elems_length();

  // 4 bytes - number of classes (excluding demand loaded)
  // n bytes - SkClass or SkActorClass binary }- Repeating
  // Skip demand loaded classes indicated by passing "true"
  binary_length += ms_object_class_p->as_binary_group_length(true);

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Recreate class hierarchy and associated info from supplied binary.
// 
// Params:
//   binary_pp:
//     Pointer to address to read binary serialization info from and to increment
//     - previously filled using as_binary() or a similar mechanism.
//   
// Notes:
//   Binary composition:
//     4 bytes - SkookumScript compiled binary code id
//     4 bytes - folder checksum from source scripts
//     4 bytes - file checksum from source scripts
//     4 bytes - bytes needed for linear allocation of hierarchy
//     4 bytes - optional additional debug bytes needed for linear allocation of hierarchy
//     4 bytes - number of classes (including demand loaded)
//     n bytes - class hierarchy placeholders   }- Repeating & Recursive
//     4 bytes - startup mind class id
//     4 bytes - number of typed classes
//     4 bytes - number of invokable classes
//     4 bytes - number of class unions
//     n bytes - class union binary             }- Repeating
//     n bytes - typed class binary             }- Repeating
//     n bytes - invokable class binary         }- Repeating
//     4 bytes - number of classes (excluding demand loaded)
//     n bytes - SkClass/SkActorClass members   }- Repeating
//
//   Little error checking on binary as it is assumed to have been validated when saved.
//   
// See:       as_binary(), as_binary_length()
// Modifiers: static
void SkBrain::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - SkookumScript compiled binary code id
  uint32_t id = A_BYTE_STREAM_UI32_INC(binary_pp);

  #if (SKOOKUM & SK_DEBUG)
    SK_ASSERT_ID(
      id == SkBrain_bin_code_id,
      AString(
        512u,
        "Invalid SkookumScript class hierarchy compiled binary code version.\n"
        "Engine wanted version '%u', but binary version was '%u'\n\n"
        "The engine and compiled binary code versions are probably out of synch.\n"
        "%s\n",
        SkBrain_bin_code_id_version,
        id >> SkBrain_bin_code_id_version_shift,
        (((id >> SkBrain_bin_code_id_version_shift) > SkBrain_bin_code_id_version)
          ? "Get a newer engine or older SkookumScript compiled binary code files."
          : "Get an older engine or newer SkookumScript compiled binary code files.")),
      AErrId_invalid_datum_id,
      SkBrain);
  #else
    if (id != SkBrain_bin_code_id)
      {
      A_DPRINT("Invalid SkookumScript class hierarchy compiled binary code version!\n");
      }
  #endif

  // 4 bytes - folder checksum from source scripts
  ms_checksum_folders = A_BYTE_STREAM_UI32_INC(binary_pp);

  // 4 bytes - file checksum from source scripts
  ms_checksum_files = A_BYTE_STREAM_UI32_INC(binary_pp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 12 bytes - session id and revision
  static_assert(sizeof(ms_session_guid) == sizeof(uint64_t) && sizeof(ms_revision) == sizeof(uint32_t), "Sizes must match.");
  ms_session_guid = A_BYTE_STREAM_UI64_INC(binary_pp);
  ms_revision = A_BYTE_STREAM_UI32_INC(binary_pp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // n bytes - project debug info
  ms_project_name.assign_binary(binary_pp);
  ms_project_path.assign_binary(binary_pp);
  ms_default_project_path.assign_binary(binary_pp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine how much memory is needed for reload

  // 4 bytes - bytes needed for linear allocation of hierarchy
  uint32_t bytes_needed = A_BYTE_STREAM_UI32_INC(binary_pp);

  // 4 bytes - optional additional debug bytes needed for linear allocation of hierarchy
  #if (SKOOKUM & SK_DEBUG)
    bytes_needed += A_BYTE_STREAM_UI32_INC(binary_pp);
  #else
    // Not Debug so just skip
    A_BYTE_STREAM_UI32_INC(binary_pp);
  #endif

  SkookumScript::notify_script_linear_bytes(bytes_needed);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #1 - Class Hierarchy

  // 4 bytes - number of classes (including demand loaded)
  uint32_t class_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  ms_classes.ensure_size(class_count);

  // n bytes - class hierarchy placeholders }- Repeating & Recursive
  assign_binary_class_hier(binary_pp, nullptr);

  // Make initial built-in classes
  initialize_core_classes(class_count);

  // 4 bytes - startup mind class id
  ASymbol startup_class_name = ASymbol::create_from_binary(binary_pp);

  initialize_after_classes_known(startup_class_name);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #2a - Typed Classes (count)

  // 4 bytes - number of typed classes
  uint32_t tclass_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  SkTypedClass::ms_typed_classes.set_size(tclass_count, tclass_count);

  SkTypedClass ** tclass_pp;
  SkTypedClass ** tclass_end_pp = nullptr;

  if (tclass_count)
    {
    tclass_pp     = SkTypedClass::ms_typed_classes.get_array();
    tclass_end_pp = tclass_pp + tclass_count;

    // Create empty typed classes - they can now be referenced/indexed but their data is
    // yet to be loaded.
    for (; tclass_pp < tclass_end_pp ; tclass_pp++)
      {
      SkTypedClass * tclass_p = SK_NEW(SkTypedClass)();
      // All SkTypedClasses stored in ms_typed_classes start out with 1 refcount
      // so that ARefPtr can never free them
      // They only ever get deleted in SkTypedClass::shared_ensure_references()
      tclass_p->reference();
      *tclass_pp = tclass_p;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #3a - Invokable Classes (count)

  // 4 bytes - number of invokable classes
  uint32_t iclass_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  SkInvokableClass::ms_shared_classes.set_size(iclass_count, iclass_count);

  SkInvokableClass ** iclass_pp;
  SkInvokableClass ** iclass_end_pp = nullptr;

  if (iclass_count)
    {
    iclass_pp     = SkInvokableClass::ms_shared_classes.get_array();
    iclass_end_pp = iclass_pp + iclass_count;

    // Create empty invokable classes - they can now be referenced/indexed but their data is
    // yet to be loaded.
    for (; iclass_pp < iclass_end_pp ; iclass_pp++)
      {
      *iclass_pp = SK_NEW(SkInvokableClass)();
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #4 - Class Unions

  // 4 bytes - number of union classes
  uint32_t union_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  SkClassUnion::ms_shared_unions.set_size(union_count, union_count);

  if (union_count)
    {
    // n bytes - class union binary }- Repeating
    SkClassUnion ** union_pp = SkClassUnion::ms_shared_unions.get_array();

    for (; union_count > 0u ; union_count--)
      {
      // The union classes should already be sorted so just add them incrementally
      SkClassUnion * union_p = SK_NEW(SkClassUnion)(binary_pp);
      // All SkClassUnions stored in ms_shared_unions start out with 1 refcount
      // so that ARefPtr can never free them
      // They only ever get deleted in SkClassUnion::shared_ensure_references()
      union_p->reference();
      *union_pp++ = union_p;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #2b - Typed Classes (info)
  if (tclass_count)
    {
    tclass_pp = SkTypedClass::ms_typed_classes.get_array();

    // Load info for pre-existing (#2a) typed classes - it is assumed that they were stored
    // in sorted order.  Some typed classes may already have been referenced/indexed by
    // class unions.
    for (; tclass_pp < tclass_end_pp ; tclass_pp++)
      {
      (*tclass_pp)->assign_binary(binary_pp);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #3b - Invokable Classes (info)
  if (iclass_count)
    {
    iclass_pp = SkInvokableClass::ms_shared_classes.get_array();

    // Load info for pre-existing (#3a) invokable classes - it is assumed that they were
    // stored in sorted order.  Some invokable classes may already have been referenced/
    // indexed by class unions or typed classes.
    for (; iclass_pp < iclass_end_pp ; iclass_pp++)
      {
      (*iclass_pp)->assign_binary(binary_pp);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Step #5 - Class Members

  // 4 bytes - number of classes (excluding demand loaded)
  // n bytes - SkClass or SkActorClass binary }- Repeating
  SkClass::from_binary_group(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SkookumScript::notify_script_linear_bytes(0u);
  }

//---------------------------------------------------------------------------------------
// Assign binary info to this object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// # See:      as_binary(), SkClass::as_binary_placeholder_recurse()
// # Notes:    Binary composition:
//               4 bytes - class name id
//               4 bytes - class flags
//               4 bytes - annotation flags
//               n bytes - bind name
//               2 bytes - number of subclasses
//               n bytes - class ancestry binary }-  Repeating & Recursive
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// # Modifiers: static
// # Author(s): Conan Reis
void SkBrain::assign_binary_class_hier(const void ** binary_pp, SkClass * superclass_p)
  {
  // 4 bytes - class name id
  ASymbol class_name = ASymbol::create_from_binary(binary_pp);

  // 4 bytes - class flags
  uint32_t flags = A_BYTE_STREAM_UI32_INC(binary_pp);

  // 4 bytes - annotation flags
  uint32_t annotation_flags = A_BYTE_STREAM_UI32_INC(binary_pp);

  // Create the class
  SkClass * class_p = create_class(class_name, superclass_p, flags, annotation_flags);

  // n bytes - bind name
  class_p->set_bind_name(SkBindName(binary_pp).as_string());

  // 2 bytes - number of subclasses
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);

  class_p->get_subclasses().ensure_size(length);


  // n bytes - class ancestry binary }- Repeating & Recursive
  for (; length > 0u ; length--)
    {
    assign_binary_class_hier(binary_pp, class_p);
    }
  }

//---------------------------------------------------------------------------------------
// Determines if the supplied compiled binary code id is valid.
// # Returns:  one of:
//               AEquate_equal   - Id checks out
//               AEquate_greater - Id of compiled binary code is newer so engine is stale
//               AEquate_less    = Id of engine is newer so compiled binary code is stale
// Arg         bin_id - compiled binary code id to validate
// # See:      is_binary_valid()
// # Modifiers: static
// # Author(s): Conan Reis
eAEquate SkBrain::is_binary_id_valid(uint32_t bin_id)
  {
  return (bin_id == SkBrain_bin_code_id)
    ? AEquate_equal      // Id checks out
    : (((bin_id >> SkBrain_bin_code_id_version_shift) > SkBrain_bin_code_id_version)
      ? AEquate_greater  // Id of compiled binary code is newer so engine is stale
      : AEquate_less);   // Id of engine is newer so compiled binary code is stale
  }

//---------------------------------------------------------------------------------------
// Determines if the supplied class hierarchy binary has a valid id.
// # Returns:  one of:
//               AEquate_equal   - Id checks out
//               AEquate_greater - Id of compiled binary code is newer so engine is stale
//               AEquate_less    = Id of engine is newer so compiled binary code is stale
// Arg         binary_p - binary to validate
// # See:      is_binary_id_valid(), assign_datum()
// # Modifiers: static
// # Author(s): Conan Reis
eAEquate SkBrain::is_binary_valid(const void * binary_p)
  {
  // compiled binary code verification Id
  uint32_t id = A_BYTE_STREAM_UI32(binary_p);

  return is_binary_id_valid(id);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//---------------------------------------------------------------------------------------
//  Retrieves the specified class if it already is in the system, or creates
//              and returns one if not.
// # Returns:   a pointer to the requested class
// Arg          name - Name of the class.  It should follow the form of "ClassName"
//              uppercase {alphanumeric}
// Arg          superclass_p - pointer to superclass of this class.  Only "Object" should
//              have no superclass.  (Default ms_object_class_p "Object")
// # See:       get_class()
// # Notes:     If the requested class already exists, this method ensures that the
//              existing superclass and the requested superclass are the same.
// # Modifiers:  static
// # Author(s):  Conan Reis
SkClass * SkBrain::create_class(
  const ASymbol & class_name,
  SkClass *       superclass_p,     // = ms_object_class_p
  uint32_t        flags,            // = ADef_uint32
  uint32_t        annotation_flags  // = 0
  )
  {
  uint32_t  insert_pos;
  SkClass * class_p = ms_classes.get(class_name, AMatch_first_found, &insert_pos);

  if (class_p == nullptr)
    {
    // Determine if actor class
    bool actor_b =
      (class_name == ms_actor_class_name) || (superclass_p && superclass_p->is_actor_class());

    if (flags == ADef_uint32)
      {
      flags = actor_b ? SkClass::Flag__default_actor : SkClass::Flag__default;
      }

    if (actor_b && SkookumScript::get_app_info()->use_builtin_actor())
      {
      class_p = SK_NEW(SkActorClass)(class_name, superclass_p, flags, annotation_flags);
      }
    else
      {
      class_p = SK_NEW(SkClass)(class_name, superclass_p, flags, annotation_flags);
      }

    SK_ASSERT_MEMORY(class_p, SkBrain);

    ms_classes.insert(*class_p, insert_pos);
    }
  else
    {
    // Ensure that the existing superclass and the requested superclass are the same.
    // This may change if multiple inheritance is added.
    SK_ASSERT_ID(!class_p->m_superclass_p || superclass_p == class_p->m_superclass_p, 
      a_cstr_format(
        "Class '%s' already exists and is derived from superclass '%s'\n"
        "and not the requested superclass '%s'!\n"
        "[Erase any old/stale script files and folders?]",
        class_name.as_cstr_dbg(),
        class_p->m_superclass_p ? class_p->m_superclass_p->m_name.as_cstr_dbg() : "[N/A]",
        superclass_p ? superclass_p->m_name.as_cstr_dbg() : "[N/A]"),
      ErrId_superclass_mismatch,
      SkBrain);

    // If unlinked, link it in now
    if (superclass_p && !class_p->m_superclass_p)
      {
      superclass_p->append_subclass(class_p);
      }
    }

  return class_p;
  }

//---------------------------------------------------------------------------------------
//  Retrieves the specified class if it already is in the system, or creates
//              and returns a one if not.
// # Returns:   a pointer to the requested class
// Arg          name - Name of the class.  It should follow the form of "ClassName"
//              uppercase {alphanumeric}
// Arg          superclass_name - name of the superclass for this class.  Only "Object"
//              should have no superclass.
// # See:       get_class()
// # Notes:     If the requested class already exists, this method ensures that the
//              existing superclass and the requested superclass are the same.
// # Modifiers:  static
// # Author(s):  Conan Reis
SkClass * SkBrain::create_class(
  const ASymbol & class_name,
  const ASymbol & superclass_name,
  uint32_t        flags,              // = ADef_uint32
  uint32_t        annotation_flags    // = 0
  )
  {
  SkClass * superclass_p = ms_classes.get(superclass_name);

  SK_ASSERT_ID(superclass_p, 
    a_cstr_format(
      "Tried to create class '%s', but requested superclass '%s'\n"
      "does not [yet] exist!",
      class_name.as_cstr_dbg(),
      superclass_name.as_cstr_dbg()),
    ErrId_no_superclass,
    SkBrain);

  return create_class(class_name, superclass_p, flags, annotation_flags);
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Ensures that both atomic invokables and raw data members are properly registered and accessible
void SkBrain::ensure_atomics_registered(SkClass ** ignore_classes_pp /*= nullptr*/, uint32_t ignore_count /*= 0u*/)
  {
  ensure_invokables_registered(ignore_classes_pp, ignore_count);
  ensure_raw_accessors_registered(ignore_classes_pp, ignore_count);
  }

//---------------------------------------------------------------------------------------
// Ensures that all atomic invokables (methods, coroutines, etc.) have been properly
//             registered and associated for all the classes.
// # Modifiers: static
// # Author(s): Conan Reis
void SkBrain::ensure_invokables_registered(
  SkClass ** ignore_classes_pp, // = nullptr
  uint32_t   ignore_count       // = 0u
  )
  {
  APArray<SkInvokableBase> atomics;
  APArray<SkClass>         ignore_list;

  SkClass ** classes_pp     = ms_classes.get_array();
  SkClass ** classes_end_pp = classes_pp + ms_classes.get_length();

  // The "Boolean" class does not register the methods that use short-circuit Boolean
  // evaluation - they are processed internally rather than being actual methods.
  // $Revisit - CReis [Correctness] Only the 4 short-circuit methods should be skipped
  ignore_list.ensure_size_empty(ignore_count + 1u);
  ignore_list.append_all(ignore_classes_pp, ignore_count);
  ignore_list.append(*SkBrain::ms_boolean_class_p);   // Boolean short-circuit evaluation
  ignore_list.append(*SkBrain::ms_debug_class_p);     // assert(), assert_no_leak()

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    // Only check class if it is not on the ignore list.
    if (!ignore_list.find_equiv(**classes_pp))
	    {
      (*classes_pp)->find_unregistered_atomics(&atomics);
	    }
    }

  uint32_t length = atomics.get_length();

  if (length)
    {
    AString error_str(AString::ctor_uint(length));
    
    error_str.append(
      " atomic routines have been declared via script files without being associated "
      "with C++ functions.\n\n"
	  "[Description/fixing instructions follows list.]\n\n");
    
    SkInvokableBase ** invoke_pp     = atomics.get_array();
    SkInvokableBase ** invoke_end_pp = invoke_pp + atomics.get_length();

    for (; invoke_pp < invoke_end_pp; invoke_pp++)
      {
      error_str.append("  ", 2u);
      error_str.append((*invoke_pp)->as_string_name());
      error_str.append('\n');
      }

    error_str.append(
	  "\n"
      "The most common cause for this error/warning is running the SkookumScript IDE as "
      "a stand-alone application - i.e. not running it via the game engine.  In which "
      "case this is a warning to not *run* any game script commands.  Scripts can still "
      "be written and compiled and non-engine scripts can still be run.  [Just press "
      "'Ignore All' and continue.]\n\n"
      "A more troublesome cause for this error - especially when running the game engine "
      "- is when the game engine and scripts are not synchronized.  Usually the game "
      "engine is not built/up-to-date or a programmer who added or removed script "
      "commands forgot to update the C++ or script files in version control.\n\n"
      "[This warning *can* be ignored and the scripts may still run, but if a script "
      "calls one of these methods or coroutines, the game will crash.  These methods and "
      "coroutines will be colour coded in the Browser to make them stand out from ones"
      "that are properly bound to custom script code or C++ functions.]\n\n");

    SK_ERROR(AErrMsg(error_str, AErrLevel_notify, "SkookumScript code/binary not in synch with C++ Engine"), SkBrain);
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that all existing raw data members are accessible
void SkBrain::ensure_raw_accessors_registered(SkClass ** ignore_classes_pp /*= nullptr*/, uint32_t ignore_count /*= 0u*/)
  {
  APArray<SkRawMemberRecord> raw_members;
  APArray<SkClass>           ignore_list;

  SkClass ** classes_pp = ms_classes.get_array();
  SkClass ** classes_end_pp = classes_pp + ms_classes.get_length();

  // The "Boolean" class does not register the methods that use short-circuit Boolean
  // evaluation - they are processed internally rather than being actual methods.
  // $Revisit - CReis [Correctness] Only the 4 short-circuit methods should be skipped
  ignore_list.ensure_size_empty(ignore_count + 1u);
  ignore_list.append_all(ignore_classes_pp, ignore_count);
  ignore_list.append(*SkBrain::ms_boolean_class_p);   // Boolean short-circuit evaluation
  ignore_list.append(*SkBrain::ms_debug_class_p);     // assert(), assert_no_leak()

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    // Only check class if it is not on the ignore list.
    if (!ignore_list.find_equiv(**classes_pp))
      {
      (*classes_pp)->find_inaccessible_raw_members(&raw_members);
      }
    }

  uint32_t length = raw_members.get_length();

  if (length)
    {
    AString error_str(AString::ctor_uint(length));

    error_str.append(
      " raw members have been declared via script files but are inaccessible since they have no registered accessor callbacks.\n\n"
      "[Description/fixing instructions follows list.]\n\n");

    for (auto record_p : raw_members)
      {
      error_str.append("  ", 2u);
      error_str.append(record_p->m_class_p->get_name_cstr_dbg());
      error_str.append(record_p->m_raw_member_p ? record_p->m_raw_member_p->get_name_cstr_dbg() : "@<all raw data members>");
      error_str.append('\n');
      }

    SK_ERROR(AErrMsg(error_str, AErrLevel_notify, "Inaccessible raw data members present"), SkBrain);
    }
  }

#endif // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Retrieves array of registered functions that bind atomic C++ methods,
//             coroutines, etc.
// # Returns:  array of atomic binding/registering functions
// # See:      register_bind_atomics_func()
// # Modifiers: static
// # Author(s): Conan Reis
tSkBindFuncs * SkBrain::get_bind_funcs()
  {
  // Being static data ensures it is available - unlike it being a global or class data member.
  static tSkBindFuncs s_bind_funcs;

  return &s_bind_funcs;
  }

//---------------------------------------------------------------------------------------
// Registers a function that binds methods, coroutines, etc. that
//             were previously declared (via SkookumScript source code prototype files) to
//             their atomic C++ counterparts.
// Arg         bind_atomics_f - function pointer to register
// # Examples: SkBrain::register_bind_atomics_func(SomeLibClass::bind_atomics);
// # See:      SkookumScript::register_bind_atomics_func() - same function, but it requires
//             less header files to be included.
// # Notes:    This should be called before SkookumScript::initialize_post_load() (probably
//             in main() or its equivalent) for any external library that needs to bind
//             C++ atomics.
//
//             Any registered functions will be called during SkookumScript::initialize_post_load().
//
//             It would be nice if this class could be used as a global whose constructor
//             had the side effect of registering the bind atomics function, but (after
//             much much research) it seems that any libraries that used this technique
//             could optimize out the global and its constructor since it would not be
//             referenced in the main module.  A call to SkookumScript::register_bindings_func()
//             should be used instead.
// # Modifiers: static
// # Author(s): Conan Reis
void SkBrain::register_bind_atomics_func(void (*bind_atomics_f)())
  {
  get_bind_funcs()->append(*SK_NEW(AFunction)(bind_atomics_f));
  }

//---------------------------------------------------------------------------------------
// Unregisters a function that was registered above
void SkBrain::unregister_bind_atomics_func(void (*bind_atomics_f)())
  {
  tSkBindFuncs * bind_funcs_p = get_bind_funcs();
  for (uint32_t i = 0; i < bind_funcs_p->get_length(); ++i)
    {
    if (*(*bind_funcs_p)[i] == bind_atomics_f)
      {
      bind_funcs_p->remove(i);
      return;
      }
    }
  }

//---------------------------------------------------------------------------------------

void SkBrain::unregister_all_bind_atomics_funcs()
  {
  get_bind_funcs()->empty_compact();
  }

//---------------------------------------------------------------------------------------
// Basic initialization before compiled binaries are looked at
void SkBrain::initialize()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Figure name of actor class
  ms_actor_class_name = (SkookumScript::get_app_info()->use_builtin_actor() || SkookumScript::get_app_info()->get_custom_actor_class_name().is_null())
    ? ASymbol_Actor
    : SkookumScript::get_app_info()->get_custom_actor_class_name();

  // Forget about the session/revision
  ms_session_guid = 0;
  ms_revision = 0;

  // Reset project debug info
  ms_project_name = AString::ms_empty;
  ms_project_path = AString::ms_empty;
  ms_default_project_path = AString::ms_empty;

  // Not registered yet
  ms_builtin_bindings_registered = false;
  }

//---------------------------------------------------------------------------------------
// Registers/connects atomic classes, coroutines, etc. that do not
// require the code or compiled binary code to be already loaded.
void SkBrain::initialize_core_classes(uint32_t ensure_class_count)
  {
  //SkDebug::print_agog("\nInitializing core classes...\n", SkLocale_ide, SkDPrintType_title);

  ms_classes.ensure_size(ensure_class_count);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Already created? Then nothing to do
  if (ms_object_class_p)
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create Base Classes
  ms_object_class_p = create_class(ASymbol_Object, nullptr);

  ms_class_class_p  = create_class(ASymbol_Class);
  ms_mind_class_p   = create_class(ASymbol_Mind);

  if (SkookumScript::get_app_info()->use_builtin_actor())
    {
    ms_actor_class_p = create_class(ASymbol_Actor, ms_object_class_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create Generic/Reflective/Runtime Classes

  create_class(ASymbol_None);
  ms_auto_class_p                 = create_class(ASymbol_Auto_, ms_class_class_p);
  ms_this_class_p                 = create_class(ASymbol_ThisClass_, ms_class_class_p);
  ms_item_class_p                 = create_class(ASymbol_ItemClass_, ms_class_class_p);
  ms_invoked_base_class_p         = create_class(ASymbol_InvokedBase);
  ms_invoked_context_base_class_p = create_class(ASymbol_InvokedContextBase, ms_invoked_base_class_p);
  ms_invoked_method_class_p       = create_class(ASymbol_InvokedMethod, ms_invoked_context_base_class_p);
  ms_invoked_coroutine_class_p    = create_class(ASymbol_InvokedCoroutine, ms_invoked_context_base_class_p);
  ms_closure_class_p              = create_class(ASymbol_Closure);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Correct metaclass class pointers
  SkClass ** classes_pp     = ms_classes.get_array();
  SkClass ** classes_end_pp = classes_pp + ms_classes.get_length();

  while (classes_pp < classes_end_pp)
    {
    (*classes_pp)->m_metaclass.m_class_p = ms_class_class_p;
    classes_pp++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create Common Classes
  ms_boolean_class_p     = create_class(ASymbol_Boolean);
  ms_debug_class_p       = create_class(ASymbol_Debug);
  ms_integer_class_p     = create_class(ASymbol_Integer);
  ms_list_class_p        = create_class(ASymbol_List);
  ms_real_class_p        = create_class(ASymbol_Real);
  ms_string_class_p      = create_class(ASymbol_String);
  ms_symbol_class_p      = create_class(ASymbol_Symbol);
  ms_master_class_p      = create_class(ASymbol_Master, ms_mind_class_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create the single instance of the None object
  SkNone::initialize();

  static SkNone s_nil;
  s_nil.m_class_p = SkNone::get_class();
  ms_nil_p = &s_nil;
  }

//---------------------------------------------------------------------------------------
// Registers classes that require classes to have been already parsed
void SkBrain::initialize_after_classes_known(const ASymbol & startup_class_name)
  {
  SkookumScript::ms_startup_class_p = get_class(startup_class_name);

  if (SkookumScript::ms_startup_class_p == nullptr)
    {
    SkDebug::print_agog(
      a_cstr_format("\nStartup Mind class '%s' does not exist - defaulting to 'Master'!\n", startup_class_name.as_cstr_dbg()),
      SkLocale_ide,
      SkDPrintType_error);

    SkookumScript::ms_startup_class_p = ms_master_class_p;
    }

  if (!SkookumScript::get_app_info()->use_builtin_actor() && !SkookumScript::get_app_info()->get_custom_actor_class_name().is_null())
    {
    ms_actor_class_p = get_class(SkookumScript::get_app_info()->get_custom_actor_class_name());
    }
  }

//---------------------------------------------------------------------------------------
// Registers/connects atomic classes, methods, coroutines, etc. with
// the parameters previously loaded during the preload phase.
void SkBrain::initialize_post_load()
  {
  // Register builtin bindings if not already
  if (!ms_builtin_bindings_registered)
    {
    register_builtin_bindings();
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call any additional registered atomic binding functions.
  get_bind_funcs()->apply_method(&AFunctionBase::invoke);

  // Create vtables for all classes
  SkObject::get_class()->build_vtables_recurse(false);

  // Resolve raw data for all classes if a callback function is given
  SkBrain::ms_object_class_p->resolve_raw_data_recurse();

  SkInstance::initialize_post_load();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  #if (SKOOKUM & SK_DEBUG)
    // If remote runtime client is nullptr then this is a script running on the IDE
    if (SkRemoteRuntimeBase::ms_client_p && SkRemoteRuntimeBase::ms_client_p->is_connected())
      {
      // Have runtime communicate to IDE that it is at a break
      SkRemoteRuntimeBase::ms_client_p->cmd_ready_to_debug();
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Invokes class constructors recursively
void SkBrain::initialize_classes()
  {
  // Initialize all the classes
  ms_object_class_p->invoke_class_ctor_recurse();
  }

//---------------------------------------------------------------------------------------
// Invokes class destructors recursively
void SkBrain::deinitialize_classes()
  {
  if (ms_object_class_p)
    {
    ms_object_class_p->invoke_class_dtor_recurse();
    }
  }

//---------------------------------------------------------------------------------------
// Delete the program = remove all classes, data and routines
void SkBrain::deinitialize_program()
  {
  // Empty classes before they are freed so that other class destructors can still refer
  // to them.
  ms_classes.apply_method(&SkClass::clear_members);

  SkInvokableClass::shared_pre_empty();
  SkParameters::shared_empty();      // Must be after SkInvokableClass::shared_pre_empty()
  SkTypedClass::shared_pre_empty();
  SkClassUnion::shared_empty();      // Must be after SkTypedClass::shared_pre_empty()
  SkTypedClass::shared_empty();
  SkInvokableClass::shared_empty();  // Must be after all other component classes

  compact(false);

  ms_classes.free_all();
  ms_classes.compact();

  ms_auto_class_p = nullptr;  // Auto_
  ms_boolean_class_p = nullptr;
  ms_class_class_p = nullptr;
  ms_closure_class_p = nullptr;
  ms_integer_class_p = nullptr;
  ms_invoked_base_class_p = nullptr;
  ms_invoked_context_base_class_p = nullptr;
  ms_invoked_coroutine_class_p = nullptr;
  ms_invoked_method_class_p = nullptr;
  ms_item_class_p = nullptr;  // ItemClass_
  ms_list_class_p = nullptr;
  ms_object_class_p = nullptr;
  ms_real_class_p = nullptr;
  ms_string_class_p = nullptr;
  ms_symbol_class_p = nullptr;
  ms_this_class_p = nullptr;  // ThisClass_
  ms_mind_class_p = nullptr;
  ms_master_class_p = nullptr;
  ms_actor_class_p = nullptr;

  ms_engine_actor_class_p = nullptr;

  // Forget about the session/revision
  ms_session_guid = 0;
  ms_revision = 0;

  // Reset project debug info
  ms_project_name = AString::ms_empty;
  ms_project_path = AString::ms_empty;
  ms_default_project_path = AString::ms_empty;

  // Built-in bindings no longer registered now
  ms_builtin_bindings_registered = false;
  }

//---------------------------------------------------------------------------------------
// Frees up any resources used by SkBrain
void SkBrain::deinitialize()
  {
  // Just to be sure, call it again
  deinitialize_program();
  }

//---------------------------------------------------------------------------------------
// Remove unreferenced data
void SkBrain::compact(bool purge_deleted_members)
  {
  // If requested, find deleted classes and clear their members
  if (purge_deleted_members)
    {
    for (SkClass * class_p : ms_classes)
      {
      if (class_p->is_deleted())
        {
        class_p->clear_members();
        }
      }
    }

  // Rinse & repeat until all cross references are resolved
  bool anything_changed;
  do
    {
    anything_changed  = SkInvokableClass::shared_ensure_references();
    anything_changed |= SkParameters::shared_ensure_references();
    anything_changed |= SkTypedClass::shared_ensure_references();
    anything_changed |= SkClassUnion::shared_ensure_references();
    } while (anything_changed);
  }

//---------------------------------------------------------------------------------------

void SkBrain::register_builtin_bindings()
  {
  SK_ASSERTX(!ms_builtin_bindings_registered, "register_builtin_bindings() called twice in a row!");

  SkObject::register_bindings();
  SkInvokedBase::register_bindings();
  SkInvokedMethod::register_bindings();
  SkInvokedCoroutine::register_bindings();
  SkClosure::register_bindings();

  SkBoolean::register_bindings();
  SkInteger::register_bindings();
  SkEnum::register_bindings();
  SkReal::register_bindings();
  SkString::register_bindings();
  SkSymbol::register_bindings();
  SkRandom::register_bindings();
  SkList::register_bindings();
  SkDebug::register_bindings();
  SkActor::register_bindings();
  SkMind::register_bindings();

  ms_builtin_bindings_registered = true;
  }

