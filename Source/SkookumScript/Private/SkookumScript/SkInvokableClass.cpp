// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class descriptor for invokable/callable objects with parameters.
//             [$Revisit - CReis Should only be needed by parser.]
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvokableClass.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvokableClass.inl>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// SkInvokableClass Class Data Members
//=======================================================================================

// Typed class objects that are shared amongst various data-structures
APSortedLogicalFree<SkInvokableClass> SkInvokableClass::ms_shared_classes;


//=======================================================================================
// SkInvokableClass Method Definitions
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
//               n bytes - parameters
//               1 byte  - invoke type
// Author(s):   Conan Reis
void SkInvokableClass::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvokableClass::as_binary_length());

  // 4 bytes - class name id
  m_class_p->as_binary_ref(binary_pp);

  // n bytes - parameter list
  m_params_p->as_binary(binary_pp);

  uint8_t invoke_type = uint8_t(m_invoke_type);
  A_BYTE_STREAM_OUT8(binary_pp, &invoke_type);
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create a reference
// to this SkInvokableClass and increments the memory address to just past the last byte
// written.
//
// #Notes
//   Binary composition:
//     4 bytes - typed class id [index in global typed class list]
//               [2 bytes would undoubtedly be sufficient, but the other class types use
//               4 bytes for a reference so 4 bytes is used for consistency.]
//     (or)          
//     4 bytes - class name id
//     n bytes - parameters
//     1 byte  - invoke type
//
// #Modifiers virtual - overridden from SkClassDescBase
// #Author(s) Conan Reis
void SkInvokableClass::as_binary_ref(
  // Pointer to address to fill and increment.  Its size *must* be large enough to fit
  // 4 bytes of binary data.
  void ** binary_pp
  ) const
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis This method assumes that the number and order of shared classes will
    // be the same when the reference is loaded.

    uint32_t index = 0u;

    ms_shared_classes.find(*this, AMatch_first_found, &index);

    // 4 bytes - typed class id [index in global invokable class list]
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
//               n bytes - parameters
//               1 byte  - invoke type
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkInvokableClass::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - class name id
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // n bytes - parameter list
  m_params_p = SkParameters::get_or_create(binary_pp);

  // 1 byte - invoke type
  m_invoke_type = eSkInvokeTime(A_BYTE_STREAM_UI8_INC(binary_pp));

  // Give it extra reference for being stored in shared list.
  m_ref_count++;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference based on the binary
//   reference info
//
// #Notes
//   Binary composition:
//     4 bytes - class union id [index in global class union list]
//     (or)
//     4 bytes - class name id
//     n bytes - parameters
//     1 byte  - invoke type
//     
//   Little error checking is done on the binary info as it assumed that it was previously
//   validated upon input.   
//   
// #Modifiers static
// #Author(s) Conan Reis
SkInvokableClass * SkInvokableClass::from_binary_ref(
  // Pointer to address to read binary serialization info from and to increment
  // - previously filled using as_binary() or a similar mechanism.
  const void ** binary_pp
  )
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis Assumes that the order and number of typed classes is the same now as
    // when the reference was serialized.
    return ms_shared_classes.get_at(A_BYTE_STREAM_UI32_INC(binary_pp));
    }
  else
    {
    SkInvokableClass iclass(binary_pp);

    return get_or_create(iclass);
    }
  }

#endif  // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this class into its source code string equivalent.  This is essentially a
// disassembly of the internal script data-structures into source code.
//
// #Notes
//   The code generated may not look exactly like the original source - for example any
//   comments will not be retained, but it should parse equivalently.
//
//   closure-class  = ['_' | '|'] parameters
//   parameters     = parameter-list [ws class-desc]
//   parameter-list = '(' ws [send-params ws] [';' ws return-params ws] ')'
//
// #Modifiers virtual (overriding pure method from SkClassDescBase) 
// #See Also  as_binary()
// #Author(s) Conan Reis
//------------------------------------------
  // Source code string version of itself
  AString
SkInvokableClass::as_code() const
  {
  char first_ch = ' ';
  const AString & params_desc = m_params_p->as_code();

  switch (m_invoke_type)
    {
    case SkInvokeTime_immediate:
      return params_desc;

    case SkInvokeTime_durational:
      first_ch = '_';
      break;

    case SkInvokeTime_any:
      first_ch = '+';
      break;
    }

  AString str(nullptr, 1u + params_desc.get_length(), 0u);

  str.append(first_ch);

  //m_class_p->m_name.as_str_dbg()

  str.append(params_desc);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "_(ThisClass_ arg)" with "String" as a scope type becomes "_(String arg)"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkInvokableClass::is_generic() const
  {
  return m_params_p->is_generic();
  }

//---------------------------------------------------------------------------------------
// If this is a generic/reflective class, it will be replaced with its
//             finalized/specific class using scope_type as its scope
//             For example: "ThisClass_" could become "String"
// Returns:    Finalized non-generic class
// Arg         scope_type - current scope class type
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkClassDescBase * SkInvokableClass::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  SkParameters * params_p       = m_params_p;
  SkParameters * final_params_p = params_p->as_finalized_generic(scope_type);

  if (final_params_p != params_p)
    {
    return get_or_create(m_class_p, final_params_p, m_invoke_type);
    }

  return const_cast<SkInvokableClass *>(this);
  }

//---------------------------------------------------------------------------------------
// Determines the closest superclass that this class and cls share.
//
// #Modifiers virtual
// #Author(s) Conan Reis
SkClassUnaryBase * SkInvokableClass::find_common_type(const SkClassDescBase & cls) const
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
      class_p = cls.get_key_class();
      break;

    case SkClassType_invokable_class:
      {
      const SkInvokableClass * iclass_p = static_cast<const SkInvokableClass *>(&cls);

      SkClass * shared_class_p = m_class_p->find_common_class(*iclass_p->m_class_p);

      // $Revisit - CReis Could try to create a parameter list that could be acceptable
      // to both input classes though for now just accept the exact same parameter list.
      uint32_t common_time = m_invoke_type & iclass_p->m_invoke_type;

      if ((common_time != 0u)
        && (*m_params_p == *iclass_p->m_params_p))
        {
        return get_or_create(shared_class_p, m_params_p, eSkInvokeTime(common_time));
        }

      // Params did not match (or too complex to build common parameter list) so just
      // return common class.
      return shared_class_p;
      }

    case SkClassType_class_union:
      return find_common_type(*cls.as_unary_class());
    }

  return m_class_p->find_common_class(*class_p);
  }

//---------------------------------------------------------------------------------------
// Determines if this class is compatible with the specified class type
//             - i.e. can this class be passed as an argument to type_p.
// Returns:    true if compatible, false if not
// Arg         type_p - type to test compatibility against
// See:        is_builtin_actor_class(), is_metaclass(), is_class_type(), get_class_type()
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
bool SkInvokableClass::is_class_type(const SkClassDescBase * type_p) const
  {
  switch (type_p->get_class_type())
    {
    case SkClassType_invokable_class:
      {
      const SkInvokableClass * iclass_p = static_cast<const SkInvokableClass *>(type_p);

      return (this == iclass_p)
        || (((m_invoke_type & iclass_p->m_invoke_type) != 0u)
          && m_params_p->is_valid_arg_to(*iclass_p->m_params_p));
      }

    case SkClassType_class:
      return (type_p == SkBrain::ms_object_class_p)
        || m_class_p->is_class(*static_cast<const SkClass *>(type_p));

    case SkClassType_class_union:
      return static_cast<const SkClassUnion *>(type_p)->is_valid_param_for(this);
     
    default:
      // SkClassType_metaclass
      // SkClassType_typed_class
      return false;
    }
  }

//---------------------------------------------------------------------------------------
// Removes any references to shared parameter structures so they can be deleted before
// calling shared_empty().
//
// #Modifiers static
// #See Also  shared_empty(), shared_ensure_references()
// #Author(s) Conan Reis
void SkInvokableClass::shared_pre_empty()
  {
  SkInvokableClass ** iclass_pp     = ms_shared_classes.get_array();
  SkInvokableClass ** iclass_end_pp = iclass_pp + ms_shared_classes.get_length();

  while (iclass_pp < iclass_end_pp)
    {
    (*iclass_pp)->m_params_p.null_delay();

    iclass_pp++;
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that all the shared invokable classes are referenced.
// Parsing may create temporary invokable classes - this method frees them from memory.
//
// #Modifiers static
// #See Also  shared_pre_empty(), shared_empty()
// #Author(s) Conan Reis
bool SkInvokableClass::shared_ensure_references()
  {
  bool anything_changed = false;

  SkInvokableClass ** iclass_pp     = ms_shared_classes.get_array();
  SkInvokableClass ** iclass_end_pp = iclass_pp + ms_shared_classes.get_length();

  while (iclass_pp < iclass_end_pp)
    {
    iclass_end_pp--;

    if ((*iclass_end_pp)->m_ref_count == 1u)
      {
      ms_shared_classes.free(uint32_t(iclass_end_pp - iclass_pp));
      anything_changed = true;
      }
    }

  // If empty, get rid of memory
  if (ms_shared_classes.is_empty())
    {
    ms_shared_classes.compact();
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this class of object
// See:        SkDebug, AMemoryStats
// Modifiers:   static
// Author(s):   Conan Reis
void SkInvokableClass::shared_track_memory(AMemoryStats * mem_stats_p)
  {
  // Note that the array buffer is added as dynamic memory
  ms_shared_classes.track_memory_and_array(mem_stats_p, "SkInvokableClass");
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkInvokableClass::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkInvokableClass, 0u));

  const SkParameters * params_p = m_params_p;

  if (!params_p->is_sharable())
    {
    params_p->track_memory(mem_stats_p);
    }
  }

