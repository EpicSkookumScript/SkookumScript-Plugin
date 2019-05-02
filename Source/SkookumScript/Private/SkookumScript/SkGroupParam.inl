// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Group parameter class - variable length parameter that has matching
//             arguments grouped into a single List argument
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkBrain.hpp>


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - name of the parameter (Default ASymbol::ms_null)
// Author(s):   Conan Reis
A_INLINE SkGroupParam::SkGroupParam(
  const ASymbol & name // = ASymbol::ms_null
  ) :
  SkParameterBase(name)
  {
  }

//---------------------------------------------------------------------------------------
// Transfer copy constructor - takes over internal contents of supplied
//             group parameter and sets it to default values.
// Returns:    itself
// Arg         group_p - group parameter to take over contents of
// Notes:      This method is useful when the contents of local stack allocated group
//             parameters need to be promoted to dynamic heap allocated objects.
// Author(s):   Conan Reis
A_INLINE SkGroupParam::SkGroupParam(SkGroupParam * group_p) :
  SkParameterBase(group_p->m_name),
  m_class_pattern(&group_p->m_class_pattern)
  {
  }

//---------------------------------------------------------------------------------------
// Retrieves the class pattern.
// Returns:    class pattern
// See:        append_class()
// Author(s):   Conan Reis
A_INLINE const APArray<SkClassDescBase> & SkGroupParam::get_pattern() const
  {
  return m_class_pattern;
  }

//---------------------------------------------------------------------------------------
// Gets the number of classes in the group pattern
// Returns:    number of classes in pattern
// Author(s):   Conan Reis
A_INLINE uint32_t SkGroupParam::get_pattern_length() const
  {
  uint32_t length = m_class_pattern.get_length();

  // Note that a zero length array is the same as {Object}
  return length ? length : 1u;
  }

//---------------------------------------------------------------------------------------
// Retrieves the type for an argument occurring at the specified index.
// Returns:    type for specified argument index
// Author(s):   Conan Reis
A_INLINE SkClassDescBase * SkGroupParam::get_pattern_type(uint32_t index) const
  {
  uint32_t length = m_class_pattern.get_length();

  // Note that a zero length array is the same as {Object}
  return length
    ? m_class_pattern.get_array()[index % length]
    : SkBrain::ms_object_class_p;
  }

//---------------------------------------------------------------------------------------
// Determines if this group parameter will match anything - i.e. {} or {Object}
//
// #Author(s) Conan Reis
A_INLINE bool SkGroupParam::is_match_all() const
  {
  uint32_t length = m_class_pattern.get_length();

  return ((length == 0u)
    || ((length == 1) && ((*m_class_pattern.get_array()) == SkBrain::ms_object_class_p)));
  }
