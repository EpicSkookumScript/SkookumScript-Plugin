// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// SkookumScript C++ library.
// Data structures for class descriptors and class objects
// Author(s):   Conan Reis
// Notes:          
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>


//=======================================================================================
// SkTypedClass Inline Methods
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
//               5*bytes - typed class reference
// Author(s):   Conan Reis
A_INLINE uint32_t SkTypedClass::as_binary_length() const
  {
  return SkClass::Binary_ref_size + m_item_type_p->as_binary_ref_typed_length();
  }

//---------------------------------------------------------------------------------------
// Binary reference length in bytes.
// Modifiers:   virtual
// Author(s):   Conan Reis
A_INLINE uint32_t SkTypedClass::as_binary_ref_typed_length() const
  {
  return ms_compounds_use_ref ? Binary_ref_size_typed : (1u + as_binary_length());
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from compiled binary code to data structures
#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Returns pointer to class based on the binary reference info or nullptr if not
//             found
// Returns:    pointer to class or nullptr
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_ref() or a similar
//             mechanism.
// See:        as_binary_ref_typed(), as_binary_ref_typed_length(), from_binary_ref_typed()
// Notes:      Binary composition:
//               4 bytes - class union id [index in global class union list]
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkTypedClass * SkTypedClass::from_binary_ref(const void ** binary_pp)
  {
  if (ms_compounds_use_ref)
    {
    // $Note - CReis Assumes that the order and number of typed classes is the same now as
    // when the reference was serialized.
    return ms_typed_classes.get_at(A_BYTE_STREAM_UI32_INC(binary_pp));
    }
  else
    {
    return get_or_create(SkTypedClass(binary_pp));
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
A_INLINE eAEquate SkTypedClass::compare(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p)
    ? m_class_p->compare_ids(*tclass.m_class_p)
    : m_item_type_p->compare(*tclass.m_item_type_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE uint32_t SkTypedClass::generate_crc32() const
  {
  // Simple XOR shall suffice for combining the two CRCs here since order is fixed anyway
  return m_class_p->get_name_id() ^ m_item_type_p->generate_crc32();
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator==(const SkTypedClass & tclass) const
  {
  return (m_class_p == tclass.m_class_p) && (m_item_type_p == tclass.m_item_type_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator!=(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p) || (m_item_type_p != tclass.m_item_type_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator<(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p)
    ? m_class_p->compare_ids(*tclass.m_class_p) == AEquate_less
    : m_item_type_p->compare(*tclass.m_item_type_p) == AEquate_less;
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator<=(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p)
    ? m_class_p->compare_ids(*tclass.m_class_p) == AEquate_less
    : m_item_type_p->compare(*tclass.m_item_type_p) != AEquate_greater;
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator>(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p)
    ? m_class_p->compare_ids(*tclass.m_class_p) == AEquate_greater
    : m_item_type_p->compare(*tclass.m_item_type_p) == AEquate_greater;
  }

//---------------------------------------------------------------------------------------
A_INLINE bool SkTypedClass::operator>=(const SkTypedClass & tclass) const
  {
  return (m_class_p != tclass.m_class_p)
    ? m_class_p->compare_ids(*tclass.m_class_p) == AEquate_greater
    : m_item_type_p->compare(*tclass.m_item_type_p) != AEquate_less;
  }

//---------------------------------------------------------------------------------------
// Clears all internal references to other class types
// Author(s):   Conan Reis
A_INLINE void SkTypedClass::clear()
  {
  if (m_item_type_p)
    {
    m_item_type_p->dereference_delay();
    m_item_type_p = nullptr;
    }
  }

//---------------------------------------------------------------------------------------
// Called when the number of references to this object reaches zero - by
//             default it deletes this object and removes it from the list of available
//             typed classes.
// See:        dereference(), ensure_reference()
// Notes:      called by dereference() and ensure_reference()
// Author(s):   Conan Reis
A_INLINE void SkTypedClass::on_no_references()
  {
  ms_typed_classes.free(*this);
  }

//---------------------------------------------------------------------------------------
// Returns a matching typed class from the shared global list of typed classes.
//             If a matching typed class does not already exist, it creates one.
// Returns:    global class union
// Arg         tclass - typed class to find match for
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkTypedClass * SkTypedClass::get_or_create(const SkTypedClass & tclass)
  {
  uint32_t       find_pos;
  SkTypedClass * tclass_p = ms_typed_classes.get(tclass, AMatch_first_found, &find_pos);

  if (tclass_p == nullptr)
    {
    tclass_p = SK_NEW(SkTypedClass)(tclass);
    // All SkTypedClasses stored in ms_typed_classes start out with 1 refcount
    // so that ARefPtr can never free them
    // They only ever get deleted in SkTypedClass::shared_ensure_references()
    tclass_p->reference();
    ms_typed_classes.insert(*tclass_p, find_pos);
    }

  return tclass_p;
  }

//---------------------------------------------------------------------------------------
// Returns a matching typed class from the shared global list of typed classes.
//             If a matching typed class does not already exist, it creates one.
// Returns:    global class union
// Arg         class_p - primary class
// Arg         item_type_p - element class type
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkTypedClass * SkTypedClass::get_or_create(
  SkClass *         class_p,
  SkClassDescBase * item_type_p
  )
  {
  // Ensure that if item_type_p has no reference prior to this call that it is not garbage collected.
  item_type_p->reference();

  SkTypedClass * tclass_p = get_or_create(SkTypedClass(class_p, item_type_p));

  item_type_p->dereference_delay();

  return tclass_p;
  }

//---------------------------------------------------------------------------------------
// Returns class type
// Returns:    class type
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
A_INLINE eSkClassType SkTypedClass::get_class_type() const
  {
  return SkClassType_typed_class;
  }

