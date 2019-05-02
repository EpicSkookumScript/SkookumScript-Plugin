// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// SkookumScript C++ library.
// All rights reserved.
//
// Class descriptor for invokable/callable objects with parameters.
//             [$Revisit - CReis Should only be needed by parser.]
// Author(s):   Conan Reis
// Notes:          
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>


//=======================================================================================
// SkInvokableClass Inline Methods
//=======================================================================================

// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//             Binary composition:
//               4 bytes - class name id
//               n bytes - parameters
//               1 byte  - invoke type
// Author(s):   Conan Reis
A_INLINE uint32_t SkInvokableClass::as_binary_length() const
  {
  return 4u + 1u + m_params_p->as_binary_length();
  }

//---------------------------------------------------------------------------------------
// Binary reference length in bytes.
// Modifiers:   virtual
// Author(s):   Conan Reis
A_INLINE uint32_t SkInvokableClass::as_binary_ref_typed_length() const
  {
  return ms_compounds_use_ref ? Binary_ref_size_typed : (1u + as_binary_length());
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


//---------------------------------------------------------------------------------------
A_INLINE eAEquate SkInvokableClass::compare(const SkInvokableClass & iclass) const
  {
  if (m_class_p != iclass.m_class_p)
    {
    return m_class_p->compare_ids(*iclass.m_class_p);
    }

  if (m_invoke_type != iclass.m_invoke_type)
    {
    return (m_invoke_type < iclass.m_invoke_type) ? AEquate_less : AEquate_greater;
    }

  if (m_params_p != iclass.m_params_p)
    {
    return (*m_params_p < *iclass.m_params_p) ? AEquate_less : AEquate_greater;
    }

  return AEquate_equal;
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator==(const SkInvokableClass & iclass) const
  {
  return (m_class_p == iclass.m_class_p)
    && (m_invoke_type == iclass.m_invoke_type)
    && (m_params_p == iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator!=(const SkInvokableClass & iclass) const
  {
  return (m_class_p != iclass.m_class_p)
    || (m_invoke_type != iclass.m_invoke_type)
    || (m_params_p != iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator<(const SkInvokableClass & iclass) const
  {
  return (m_class_p != iclass.m_class_p)
    ? (m_class_p->compare_ids(*iclass.m_class_p) == AEquate_less)
    : (m_invoke_type != iclass.m_invoke_type)
      ? (m_invoke_type < iclass.m_invoke_type)
      : (m_params_p != iclass.m_params_p) && (*m_params_p < *iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator<=(const SkInvokableClass & iclass) const
  {
  return (m_class_p != iclass.m_class_p)
    ? (m_class_p->compare_ids(*iclass.m_class_p) == AEquate_less)
    : (m_invoke_type != iclass.m_invoke_type)
      ? (m_invoke_type < iclass.m_invoke_type)
      : (m_params_p == iclass.m_params_p) || (*m_params_p < *iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator>(const SkInvokableClass & iclass) const
  {
  return (m_class_p != iclass.m_class_p)
    ? (m_class_p->compare_ids(*iclass.m_class_p) == AEquate_greater)
    : (m_invoke_type != iclass.m_invoke_type)
      ? (m_invoke_type > iclass.m_invoke_type)
      : (m_params_p != iclass.m_params_p) && !(*m_params_p < *iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkInvokableClass::operator>=(const SkInvokableClass & iclass) const
  {
  return (m_class_p != iclass.m_class_p)
    ? (m_class_p->compare_ids(*iclass.m_class_p) == AEquate_greater)
    : (m_invoke_type != iclass.m_invoke_type)
      ? (m_invoke_type > iclass.m_invoke_type)
      : (m_params_p == iclass.m_params_p) || !(*m_params_p < *iclass.m_params_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE uint32_t SkInvokableClass::generate_crc32() const
  {
  // Simple XOR shall suffice for combining the two CRCs here since order is fixed anyway
  uint32_t crc = m_class_p->get_name_id() ^ m_params_p->generate_crc32();
  return AChecksum::generate_crc32_uint8((uint8_t)m_invoke_type, crc);
  }

//---------------------------------------------------------------------------------------
// Clears all internal references to other class types
// Author(s):   Conan Reis
A_INLINE void SkInvokableClass::clear()
  {
  m_params_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Called when the number of references to this object reaches zero - by
//             default it deletes this object and removes it from the list of available
//             typed classes.
// See:        dereference(), ensure_reference()
// Notes:      called by dereference() and ensure_reference()
// Author(s):   Conan Reis
A_INLINE void SkInvokableClass::on_no_references()
  {
  ms_shared_classes.free(*this);
  }

//---------------------------------------------------------------------------------------
// Returns class type
// Returns:    class type
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE eSkClassType SkInvokableClass::get_class_type() const
  {
  return SkClassType_invokable_class;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference
//
// #Modifiers static
// #Author(s) Conan Reis
A_INLINE SkInvokableClass * SkInvokableClass::get_or_create(const SkInvokableClass & iclass)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for existing matching class
  uint32_t           find_pos;
  SkInvokableClass * iclass_p = ms_shared_classes.get(iclass, AMatch_first_found, &find_pos);

  if (iclass_p)
    {
    // Found it!
    return iclass_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a new dynamic class to share
  iclass_p = SK_NEW(SkInvokableClass)(iclass);

  // Give it extra reference for being stored in shared list.
  iclass_p->reference();
  ms_shared_classes.insert(*iclass_p, find_pos);

  return iclass_p;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference
//
// #Modifiers static
// #Author(s) Conan Reis
A_INLINE SkInvokableClass * SkInvokableClass::get_or_create(
  SkClass *      class_p,
  SkParameters * params_p,
  eSkInvokeTime  invoke_type
  )
  {
  SkInvokableClass iclass(class_p, params_p, invoke_type);

  return get_or_create(iclass);
  }


