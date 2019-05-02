// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structures for class descriptors and class objects
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkTypedClass.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkTypedClass.inl>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// SkTypedClass Class Data Members
//=======================================================================================

// Typed class objects that are shared amongst various data-structures
APSortedLogicalFree<SkTypedClass> SkTypedClass::ms_typed_classes;


//=======================================================================================
// SkTypedClass Method Definitions
//=======================================================================================

// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this typed class and increments the memory address to just past
//             the last byte written.
//             binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Binary composition:
//               4 bytes - class name id
//               5*bytes - typed class reference
// Author(s):   Conan Reis
void SkTypedClass::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkTypedClass::as_binary_length());

  // 4 bytes - class name id
  m_class_p->as_binary_ref(binary_pp);

  // 5*bytes - typed class reference
  m_item_type_p->as_binary_ref_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create
//             a reference to this SkTypedClass and increments the memory address  to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit 4 bytes of binary data.
// Notes:      Binary composition:
//               4 bytes - typed class id [index in global typed class list]
//
//               [2 bytes would undoubtedly be sufficient, but the other class types use
//               4 bytes for a reference so 4 bytes is used for consistency.]
//
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
void SkTypedClass::as_binary_ref(void ** binary_pp) const
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis This method assumes that the number and order of shared/ typed classes
    // will be the same when the reference is loaded.

    uint32_t index = 0u;

    ms_typed_classes.find(*this, AMatch_first_found, &index);

    // 4 bytes - typed class id [index in global typed class list]
    **(uint32_t **)binary_pp = index;
    (*(uint32_t **)binary_pp)++;
    }
  else
    {
    as_binary(binary_pp);
    }
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from compiled binary code to data structures
#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Assign binary info to this object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - class name id
//               5*bytes - typed class reference
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkTypedClass::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - class name id
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // 5*bytes - typed class reference
  SkClassDescBase * class_p = from_binary_ref_typed(binary_pp);

  class_p->reference();

  if (m_item_type_p)
    {
    m_item_type_p->dereference_delay();
    }

  m_item_type_p = class_p;
  }

#endif  // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this nil union class into its source code string equivalent.
//             This is essentially a disassembly of the internal script data-structures
//             into source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
//
//               list-class = List ['{' ws [class-desc ws] '}']
//
// Modifiers:   virtual (overriding pure method from SkClassDescBase) 
// Author(s):   Conan Reis
AString SkTypedClass::as_code() const
  {
  AString item_desc(m_item_type_p->as_code());
  AString str(m_class_p->m_name.as_str_dbg(), item_desc.get_length() + 3u);

  str.append('{');
  str.append(item_desc);
  str.append('}');

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "List{ThisClass_}" with "String" as a scope type becomes "List{String}"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkTypedClass::is_generic() const
  {
  return m_item_type_p->is_generic();
  }

//---------------------------------------------------------------------------------------
// If this is a generic/reflective class, it will be replaced with its
//             finalized/specific class using scope_type as its scope
//             For example: "ThisClass_" could become "String"
// Returns:    Finalized non-generic class
// Arg         scope_type - current scope class type
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkClassDescBase * SkTypedClass::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  SkClassDescBase * item_type_p       = m_item_type_p;
  SkClassDescBase * final_item_type_p = item_type_p->as_finalized_generic(scope_type);

  if (final_item_type_p != item_type_p)
    {
    return get_or_create(m_class_p, final_item_type_p);
    }

  return const_cast<SkTypedClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
//
// #Modifiers virtual
// #Author(s) Conan Reis
SkClassUnaryBase * SkTypedClass::find_common_type(const SkClassDescBase & cls) const
  {
  const SkClass * class_p = nullptr;

  switch (cls.get_class_type())
    {
    case SkClassType_class:
      class_p = static_cast<const SkClass *>(&cls);
      break;

    case SkClassType_metaclass:
      return static_cast<const SkMetaClass *>(&cls)->find_common_class(*m_class_p);

    case SkClassType_typed_class:
      {
      const SkTypedClass * tclass_p = static_cast<const SkTypedClass *>(&cls);

      return get_or_create(
        m_class_p->find_common_class(*tclass_p->m_class_p),
        m_item_type_p->find_common_type(*tclass_p->m_item_type_p));
      }

    case SkClassType_invokable_class:
      class_p = cls.get_key_class();
      break;

    case SkClassType_class_union:
      return find_common_type(*cls.as_unary_class());
    }

  return m_class_p->find_common_class(*class_p);
  }

//---------------------------------------------------------------------------------------
// Removes any references to shared parameter structures so they can be deleted before
// calling shared_empty().
//
// #Modifiers static
// #See Also  shared_empty(), shared_ensure_references()
// #Author(s) Conan Reis
void SkTypedClass::shared_pre_empty()
  {
  SkTypedClass ** tclass_pp     = ms_typed_classes.get_array();
  SkTypedClass ** tclass_end_pp = tclass_pp + ms_typed_classes.get_length();

  while (tclass_pp < tclass_end_pp)
    {
    (*tclass_pp)->clear();
    tclass_pp++;
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that all the globally available class unions are referenced.
//             Parsing may create some temporary class unions - this method frees them
//             from memory.
// Modifiers:   static
// Author(s):   Conan Reis
bool SkTypedClass::shared_ensure_references()
  {
  bool anything_changed = false;

  SkTypedClass ** tclass_pp     = ms_typed_classes.get_array();
  SkTypedClass ** tclass_end_pp = tclass_pp + ms_typed_classes.get_length();

  while (tclass_pp < tclass_end_pp)
    {
    tclass_end_pp--;

    // All SkTypedClasses stored in ms_typed_classes start out with 1 refcount
    // so that ARefPtr can never free them
    // So if the refcount is 1 (or less) it means they are not actually referenced
    // and should be removed
    if ((*tclass_end_pp)->m_ref_count <= 1u)
      {
      ms_typed_classes.free(uint32_t(tclass_end_pp - tclass_pp));
      anything_changed = true;
      }
    }

  // If empty, get rid of memory
  if (ms_typed_classes.is_empty())
    {
    ms_typed_classes.compact();
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------
// Determines if this metaclass is compatible with the specified class type
//             - i.e. can this class be passed as an argument to type_p.
// Returns:    true if compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkTypedClass::is_class_type(const SkClassDescBase * type_p) const
  {
  switch (type_p->get_class_type())
    {
    case SkClassType_class:
      return (type_p == SkBrain::ms_object_class_p)
        || m_class_p->is_class(*static_cast<const SkClass *>(type_p));

    case SkClassType_typed_class:
      return m_class_p->is_class(*static_cast<const SkTypedClass *>(type_p)->m_class_p)
        && m_item_type_p->is_class_type(static_cast<const SkTypedClass *>(type_p)->m_item_type_p);

    case SkClassType_class_union:
      return static_cast<const SkClassUnion *>(type_p)->is_valid_param_for(this);

    // SkClassType_metaclass
    // SkClassType_invokable_class
    default:
      return false;
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this class of object
// See:        SkDebug, AMemoryStats
// Modifiers:   static
// Author(s):   Conan Reis
void SkTypedClass::shared_track_memory(AMemoryStats * mem_stats_p)
  {
  // Note that the SkTypedClass array buffer is added as dynamic memory
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkTypedClass, 0u),
    ms_typed_classes.get_length() * sizeof(void *),
    ms_typed_classes.get_size() * sizeof(void *),
    ms_typed_classes.get_length());
  }

