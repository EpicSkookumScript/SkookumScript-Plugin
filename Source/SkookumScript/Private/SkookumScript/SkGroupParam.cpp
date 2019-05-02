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

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkGroupParam.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkGroupParam.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkTypedClass.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkGroupParam::~SkGroupParam()
  {
  // Dereference classes
  m_class_pattern.apply_method(&SkClassDescBase::dereference);
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         class_count - number of classes in the class pattern between 0 and 63.
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - parameter name id
//               5*bytes - class type }- repeating
//
//             Note that the first byte created with SkGroupParam::as_binary() should
//             have already been parsed by SkParameterBase::from_binary_new().
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkGroupParam::SkGroupParam(
  uint32_t      class_count,
  const void ** binary_pp
  ) :
  // 4 bytes - parameter name id
  SkParameterBase(ASymbol::create_from_binary(binary_pp)),
  m_class_pattern(nullptr, 0u, class_count)
  {
  // 5*bytes - class type }- repeating
  SkClassDescBase * type_p;

  for (; class_count > 0u ; class_count--)
    {
    type_p = SkClassDescBase::from_binary_ref_typed(binary_pp);
    type_p->reference();
    m_class_pattern.append(*type_p);
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//---------------------------------------------------------------------------------------
// Logical equals 
//
// #Author(s) Conan Reis
bool SkGroupParam::compare_equal(const SkParameterBase & param) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare name first then kind, class type.
  // [Comparing by kind/type is probably more relevant than by name though comparing by
  // name is the fastest.]
  if ((m_name != param.get_name())
    || (param.get_kind() != SkParameter_group))
    {
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare class pattern lengths
  uint32_t class_count  = m_class_pattern.get_length();
  uint32_t pclass_count = static_cast<const SkGroupParam *>(&param)->m_class_pattern.get_length();

  if (class_count != pclass_count)
    {
    return false;
    }

  // Compare class pattern
  SkClassDescBase ** pclass_pp    = static_cast<const SkGroupParam *>(&param)->m_class_pattern.get_array();
  SkClassDescBase ** class_pp     = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + m_class_pattern.get_length();

  while (class_pp < class_end_pp)
    {
    // Class types are unique so just compare addresses.
    if (*class_pp != *pclass_pp)
      {
      return false;
      }

    pclass_pp++;
    class_pp++;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Logical less than 
//
// #Author(s) Conan Reis
bool SkGroupParam::compare_less(const SkParameterBase & param) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare name first then kind, class type.
  // [Comparing by kind/type is probably more relevant than by name though comparing by
  // name is the fastest.]
  if (m_name < param.get_name())
    {
    return true;
    }

  if (m_name > param.get_name() || (param.get_kind() != SkParameter_group))
    {
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare class pattern lengths
  uint32_t class_count  = m_class_pattern.get_length();
  uint32_t pclass_count = static_cast<const SkGroupParam *>(&param)->m_class_pattern.get_length();

  if (class_count != pclass_count)
    {
    return class_count < pclass_count;
    }

  // Compare class pattern
  SkClassDescBase ** pclass_pp    = static_cast<const SkGroupParam *>(&param)->m_class_pattern.get_array();
  SkClassDescBase ** class_pp     = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + m_class_pattern.get_length();

  while (class_pp < class_end_pp)
    {
    switch ((*class_pp)->compare(**pclass_pp))
      {
      case AEquate_less:
        return true;

      case AEquate_greater:
        return false;
          
      default:
        pclass_pp++;
        class_pp++;
        break;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
uint32_t SkGroupParam::generate_crc32() const
  {
  uint32_t crc = get_name_id();

  SkClassDescBase ** class_pp = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + m_class_pattern.get_length();
  for (; class_pp < class_end_pp; ++class_pp)
    {
    crc = AChecksum::generate_crc32_uint32((*class_pp)->generate_crc32(), crc);
    }

  return crc;
  }

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this group parameter and its components and increments the memory
//             address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               2 bits  - parameter type (SkParameter_group)
//               6 bits  - number of classes in class pattern
//               4 bytes - parameter name id
//               5*bytes - class type }- repeating
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
void SkGroupParam::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkGroupParam::as_binary_length());

  uint8_t ** data_pp = (uint8_t **)binary_pp;
  uint32_t   length  = m_class_pattern.get_length();

  // 2 bits - parameter type (SkParameter_unary or SkParameter_unary_default)
  uint8_t header = static_cast<uint8_t>(length);

  // 6 bits - number of classes in class pattern
  header   <<= SkParam_type_bits;
  header    |= SkParameter_group;
  **data_pp  = header;
  (*data_pp)++;

  // 4 bytes - parameter name id
  m_name.as_binary(binary_pp);


  // 5*bytes - class type }- repeating
  SkClassDescBase ** class_pp     = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + length;

  for (; class_pp < class_end_pp; class_pp++)
    {
    (*class_pp)->as_binary_ref_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               2 bits  - parameter type (SkParameter_group)
//               6 bits  - number of classes in class pattern
//               4 bytes - parameter name id
//               5*bytes - class type }- repeating
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
uint32_t SkGroupParam::as_binary_length() const
  {
  uint32_t bytes = 5u;
  
  // 5*bytes - class type }- repeating
  SkClassDescBase ** class_pp     = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + m_class_pattern.get_length();

  for (; class_pp < class_end_pp; class_pp++)
    {
    bytes += (*class_pp)->as_binary_ref_typed_length();
    }

  return bytes;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
AString SkGroupParam::as_code() const
  {
  AString str(nullptr, 128u, 0u);

  str.append('{');

  SkClassDescBase ** class_pp     = m_class_pattern.get_array();
  SkClassDescBase ** class_end_pp = class_pp + m_class_pattern.get_length();

  while (class_pp < class_end_pp)
    {
    str.append((*class_pp)->as_code());
    class_pp++;

    if (class_pp < class_end_pp)
      {
      str.append(", ", 2u);
      }
    }

  str.append("} ", 2u);
  str.append(m_name.as_str_dbg());

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
//  Returns a new unary parameter with any generic/reflective classes replaced with their
//  finalized/specific class using scope_type as its scope.
//
// #Examples
//   "{ThisClass_} arg" with "String" as a scope type becomes "{String} arg"
//
// See Also  is_generic(), SkInvokableClass: :as_finalized_generic()
// #Author(s) Conan Reis
SkParameterBase * SkGroupParam::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  SkGroupParam * fgroup_p = SK_NEW(SkGroupParam)(m_name);

  uint32_t length = m_class_pattern.get_length();

  if (length)
    {
    fgroup_p->m_class_pattern.set_size(length, length);

    SkClassDescBase ** fclass_pp    = fgroup_p->m_class_pattern.get_array();
    SkClassDescBase ** class_pp     = m_class_pattern.get_array();
    SkClassDescBase ** class_end_pp = class_pp + length;

    for (; class_pp < class_end_pp; class_pp++, fclass_pp++)
      {
      *fclass_pp = (*class_pp)->as_finalized_generic(scope_type);
      }
    }

  return fgroup_p;
  }

//---------------------------------------------------------------------------------------
//  Appends the specified class to the end of the class pattern
// Arg          cls - class to append to pattern
// See:         get_pattern
// Author(s):    Conan Reis
void SkGroupParam::append_class(const SkClassDescBase & cls)
  {
  cls.reference();
  m_class_pattern.append(cls);
  }

//---------------------------------------------------------------------------------------
// Gets parameter type.
// Returns:    SkParameter_group
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
eSkParameter SkGroupParam::get_kind() const
  {
  return SkParameter_group;
  }

//---------------------------------------------------------------------------------------
// Gets parameter type.
// Returns:    SkParameter_unary_default or SkParameter_unary
// Modifiers:   virtual - overriding pure method from SkParameterBase
// Author(s):   Conan Reis
SkClassDescBase * SkGroupParam::get_expected_type() const
  {
  // Note that the type is calculated rather than stored - it should only be used during
  // parsing so memory is saved rather than speed.
  return SkTypedClass::get_or_create(
    SkBrain::ms_list_class_p,
    SkClassUnion::get_merge(m_class_pattern));
  }

//---------------------------------------------------------------------------------------
//  Determines if this type is a generic/reflective class.
//  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced during
//  parse as its type is determined via its surrounding context.]
//
// #Examples
//   "{ThisClass_} arg" with "String" as a scope type becomes "{String} arg"
//
// #Modifiers virtual
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkGroupParam::is_generic() const
  {
  uint32_t length = m_class_pattern.get_length();

  if (length)
    {
    SkClassDescBase ** class_pp     = m_class_pattern.get_array();
    SkClassDescBase ** class_end_pp = class_pp + length;

    for (; class_pp < class_end_pp; class_pp++)
      {
      (*class_pp)->is_generic();
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determines whether this parameter has a default expression if it is
//             omitted during an invocation call.
// Returns:    true if it has a default, false if not
// Modifiers:   virtual (overriding pure method from SkParameterBase) 
// Author(s):   Conan Reis
bool SkGroupParam::is_defaultable() const
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Determines if this group would be a valid invokable arg to the specified invokable
// group parameter.  In other words are arguments that are accepted as valid for "param"
// group also valid for this group.
//
// See Also  SkParameters: :is_valid_arg_to()
// #Author(s) Conan Reis
bool SkGroupParam::is_valid_arg_to(const SkGroupParam & param) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for "matches anything" cases

  // This group matches anything - {} or {Object}
  if (is_match_all())
    {
    return true;
    }

  // Parameter group must match anything - {} or {Object} - so this group is too picky
  if (param.is_match_all())
    {
    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure class pattern lengths are compatible.
  uint32_t acount = m_class_pattern.get_length();
  uint32_t pcount = param.m_class_pattern.get_length();

  if ((acount > pcount) || ((pcount % acount) != 0u))
    {
    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare class pattern
  SkClassDescBase ** aclass_pp = m_class_pattern.get_array();
  SkClassDescBase ** pclass_pp = param.m_class_pattern.get_array();

  uint32_t aidx = 0u;
  uint32_t pidx = 0u;

  while (pidx < pcount)
    {
    if (!pclass_pp[pidx]->is_class_type(aclass_pp[aidx]))
      {
      return false;
      }

    aidx++;
    if (aidx == acount)
      {
      aidx = 0u;
      }

    pidx++;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// Returns:    amount of dynamic memory in bytes
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkGroupParam::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkGroupParam, 0u),
    m_class_pattern.get_length() * sizeof(void *),
    m_class_pattern.get_size() * sizeof(void *));
  }
