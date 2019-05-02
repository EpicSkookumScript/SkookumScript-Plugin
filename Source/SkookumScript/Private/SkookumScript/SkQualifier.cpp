// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Qualifier class - full qualification consisting of member name and owner class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkQualifier.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkQualifier.inl>
#endif

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkTyped.hpp>


//=======================================================================================
// SkQualifier Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this typed name and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               6 bytes - SkNamedIndexed
//               4 bytes - scope
//
// Author(s):   Conan Reis
void SkQualifier::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkQualifier::as_binary_length());

  SkNamedIndexed::as_binary(binary_pp);
  (m_scope_p ? m_scope_p->get_name() : ASymbol::get_null()).as_binary(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               6 bytes - SkNamedIndexed
//               4 bytes - scope
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
SkQualifier::SkQualifier(const void ** binary_pp) :
  SkNamedIndexed(binary_pp),
  m_scope_p(SkClass::from_binary_ref(binary_pp))
  {
  }

//---------------------------------------------------------------------------------------
// Assignment from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               6 bytes - SkNamedIndexed
//               4 bytes - scope
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkQualifier::assign_binary(const void ** binary_pp)
  {
  SkNamedIndexed::assign_binary(binary_pp);
  m_scope_p = SkClass::from_binary_ref(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Less than comparison function
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         qual - other qualifier to compare against
// Notes:      This assumes that 'm_scope_p' for this instance is set and 'm_scope_p'
//             for 'qual' may or may not be set.  If 'm_scope_p' for 'qual' is not set,
//             only the name is compared.
// Author(s):   Conan Reis
bool SkQualifier::operator<(const SkQualifier & qual) const
  {
  // $Revisit - CReis this should be inlined
  return (m_name < qual.m_name)  // Name is less than (true)
    || ((m_name == qual.m_name)    // Name is equal to
      && qual.m_scope_p && m_scope_p->is_subclass(*qual.m_scope_p));  // less than (true) if subclass, not less than (false) if not
  }

//---------------------------------------------------------------------------------------
// Less than comparison function - first compare scope then name
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         qual - other qualifier to compare against
// Notes:      This assumes that 'm_scope_p' for this instance is set and 'm_scope_p'
//             for 'qual' may or may not be set.  If 'm_scope_p' for 'qual' is not set,
//             only the name is compared.
// Author(s):   Conan Reis
bool SkQualifier::less_ids_scope_name(const SkQualifier & qual) const
  {
  // $Revisit - CReis this should be inlined
  return (m_scope_p == qual.m_scope_p)
    ? (!qual.m_name.is_null() && (m_name < qual.m_name))
    : (m_scope_p->get_name() < qual.m_scope_p->get_name());
  }

//---------------------------------------------------------------------------------------
// Less than comparison function - first compare name then scope
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         qual - other qualifier to compare against
// Notes:      This assumes that 'm_scope_p' for this instance is set and 'm_scope_p'
//             for 'qual' may or may not be set.  If 'm_scope_p' for 'qual' is not set,
//             only the name is compared.
// Author(s):   Conan Reis
bool SkQualifier::less_ids_name_scope(const SkQualifier & qual) const
  {
  // $Revisit - CReis this should be inlined
  return (m_name == qual.m_name)
    ? (*m_scope_p < *qual.m_scope_p)
    : (m_name < qual.m_name);
  }

//---------------------------------------------------------------------------------------
// Gets the class scope
// Returns:    Pointer to class scope or nullptr
// Notes:      Only valid if this represents a data member.
// Author(s):   Conan Reis
SkClassDescBase * SkQualifier::get_data_type() const
  {
  // $Revisit - CReis this should be inlined
  return m_scope_p->get_data_type(m_name)->m_type_p;
  }


//---------------------------------------------------------------------------------------
// Returns 0 if equal, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
// Returns:    0, < 0 or > 0 
// See:        APSorted, etc.
// Modifiers:   static
// Author(s):   Conan Reis
ptrdiff_t SkQualifierCompareName::comparison(const SkQualifier & lhs, const SkQualifier & rhs)
  {
  // $Revisit - CReis this should be inlined
  // $Note - CReis Cannot simply subtract name ids since they are unsigned and can be so big
  return (lhs.m_name != rhs.m_name)
    ? (lhs.m_name < rhs.m_name ? -1 : 1)
    : ((lhs.m_scope_p != rhs.m_scope_p)
      ? ((lhs.m_scope_p->get_name_id() < rhs.m_scope_p->get_name_id()) ? -1 : 1)
      : 0);
  }
