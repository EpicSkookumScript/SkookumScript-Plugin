// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for actor class descriptors and actor class objects
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkActorClass.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkActorClass.inl>
#endif

#if (SKOOKUM & SK_DEBUG)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkObjectId.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Class Data
//=======================================================================================


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Examples:   Called by system
// Modifiers:   virtual
// Author(s):   Conan Reis
SkActorClass::~SkActorClass()
  {
  // ~SkClass() also calls SkClass::clear_members()
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkActorClass::track_memory(
  AMemoryStats * mem_stats_p,
  bool           skip_demand_loaded
  ) const
  {
  bool demand_loaded = is_demand_loaded();

  if (!skip_demand_loaded || !demand_loaded)
    {
    mem_stats_p->track_memory(
      SKMEMORY_ARGS(SkActorClass, 0u),
      sizeof(void *) * (1u + m_subclasses.get_length() + m_data.get_length() + m_data_raw.get_length()
         + m_class_data.get_length() + m_class_data_values.get_length() + m_methods.get_length()
         + m_class_methods.get_length() + m_coroutines.get_length()
         + m_instances.get_length()),
      sizeof(void *) * (1u + m_subclasses.get_size() + m_data.get_size() + m_data_raw.get_size()
         + m_class_data.get_size() + m_class_data_values.get_size() + m_methods.get_size()
         + m_class_methods.get_size() + m_coroutines.get_size()
         + m_instances.get_size()));
      // Added 1 x pointer dynamic bytes for space in SkBrain::ms_classes

    uint32_t member_count = m_data.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedName, 0u), 0u, 0u, member_count);
      }

    member_count = m_data_raw.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedNameRaw, 0u), 0u, 0u, member_count);
      }

    member_count = m_class_data.get_length();

    if (member_count)
      {
      mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedName, 0u), 0u, 0u, member_count);
      }

    m_methods.track_memory(mem_stats_p);
    m_class_methods.track_memory(mem_stats_p);
    m_coroutines.track_memory(mem_stats_p);
    }
  else
    {
    mem_stats_p->track_memory(
      SKMEMORY_ARGS(SkActorClass, 0u),
      sizeof(void *) * (1u + m_subclasses.get_length()),
      sizeof(void *) * (1u + m_subclasses.get_size() + m_data.get_size() + m_data_raw.get_size()
         + m_class_data.get_size()+ m_class_data_values.get_size() + m_methods.get_size()
         + m_class_methods.get_size() + m_coroutines.get_size()
         + m_instances.get_size()));
         // Added 1 x pointer dynamic bytes for space in SkBrain::ms_classes
    }

  // Count demand loaded actor classes - though their size totals are captured by SkActorClass
  if (demand_loaded)
    {
    mem_stats_p->track_memory("SkActorClass.demand", 0u);
    }
  }


//---------------------------------------------------------------------------------------
// [Called during runtime when an object ID is used.] Determines the identified name
// object or returns nullptr if it cannot be found.
// 
// The supplied obj_id_p may be modified to cache the object so that it may be returned
// immediately if the same object ID is invoked repeatedly.  Any other aspects of the
// object ID such as its flags may also be modified in the process.
//
// #Modifiers virtual
// #See Also  object_id_validate()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns named object or nullptr if not found.
SkInstance * SkActorClass::object_id_lookup(
  const SkBindName & name,
  SkInvokedBase *    caller_p // = nullptr
  ) const
  {
  // Only used when is_builtin_actor_class() is true
  // If custom C++ lookup is present, use it
  if (m_object_id_lookup_f)
    {
    return (m_object_id_lookup_f)(name);
    }

  return m_instances.get(ASymbol::create_existing(name.as_string()));
  }


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// [Called during parsing whenever an object ID is used.] Determines if the supplied
// object ID may be a valid named object.  The supplied object ID may be modified by this
// method by changing flags, setting a more appropriate look-up class, etc.
//
// #Modifiers virtual
// #See Also  object_id_lookup()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns class type of named object [usually the same class as the look-up class in
  // obj_id_p - i.e. this class - though it could be a closer derived/sub-class] or nullptr
  // if name object not valid for this class.
//   SkClass *
// SkActorClass::object_id_validate(SkObjectID * obj_id_p) const
//   {
//   // Accept all named objects
//   return const_cast<SkActorClass *>(this);
//   }

#endif  // (SKOOKUM & SK_CODE_IN)


//---------------------------------------------------------------------------------------
// Recursively call the class destructor and clear out all members for this
//             class and all its subclasses.  [Already assumes that it is the demand load
//             group root class.]
// Returns:    true if unloaded immediately and false if unload is deferred, load is
//             locked or if it is already unloaded or not a demand loaded class.
// See:        invoke_class_dtor_recurse()
// Author(s):   Conan Reis
bool SkActorClass::demand_unload()
  {
  if (((m_flags & Flag__demand_loaded) == Flag__demand_loaded) && !is_load_locked())
    {
    if (m_instances.is_empty())
      {
      demand_unload_recurse();

      return true;
      }

    // Defer unload - see SkActorClass::remove_instance()
    m_flags |= Flag_demand_unload;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Returns true if it is an "Actor" class or a subclass of "Actor"
//             (SkActorClass) or false if not.
// Returns:    true if it is an actor class instance (SkActorClass)
// Notes:      Same as calling is_class(*SkBrain::ms_actor_class_p), but faster.
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkActorClass::is_builtin_actor_class() const
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Instantiates / creates a new instance of this class and adds any data
//             members and initializes them to nil.  Starts with one reference count.
// Returns:    an instance of this class
// Notes:      This will not work for the 'Boolean' class.
// Modifiers:   virtual - overridden from SkClass
// Author(s):   Conan Reis
SkInstance * SkActorClass::new_instance()
  {
  SkActor * actor_p = SK_NEW(SkActor)(ASymbolX_unnamed, this, false);

  actor_p->reference();

  return actor_p;
  }

