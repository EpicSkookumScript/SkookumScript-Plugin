// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Single parameter class
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
//  Constructor
// Returns:     itself
// Arg          name - name of the parameter
// Arg          type_p - class type expected for this parameter.  If it is nullptr, Object
//              (SkBrain::ms_object_class_p) is inferred.
// Arg          default_p - optional default expression.  If this parameter is required,
//              it should be set to nullptr.
// Author(s):    Conan Reis
A_INLINE SkUnaryParam::SkUnaryParam(
  const ASymbol &    name,     // = ASymbol::ms_null
  SkClassDescBase *  type_p,   // = nullptr
  SkExpressionBase * default_p // = nullptr
  ) :
  SkParameterBase(name),
  m_type_p(type_p ? type_p : SkBrain::ms_object_class_p),
  m_default_p(default_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Transfer copy constructor - takes over internal contents of supplied
//              unary parameter and sets it to default values.
// Returns:     itself
// Arg          uparam_p - unary parameter to take over contents of
// Notes:       This method is useful when the contents of a local stack allocated unary
//              parameter needs to be promoted to a dynamic heap allocated object.
// Author(s):    Conan Reis
A_INLINE SkUnaryParam::SkUnaryParam(SkUnaryParam * uparam_p) :
  SkParameterBase(uparam_p->m_name),
  m_type_p(uparam_p->m_type_p),
  m_default_p(uparam_p->m_default_p)
  {
  uparam_p->m_type_p    = SkBrain::ms_object_class_p;
  uparam_p->m_default_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Logical equals 
//
// #Author(s) Conan Reis
A_INLINE bool SkUnaryParam::compare_equal(const SkParameterBase & param) const
  {
  // $Note - CReis The default is not compared [since parameters with defaults are not
  // shared.

  return (m_name == param.get_name())
    && (get_kind() != SkParameter_group)
    && (m_type_p == static_cast<const SkUnaryParam *>(&param)->m_type_p);
  }

//---------------------------------------------------------------------------------------
// Logical less than 
//
// #Author(s) Conan Reis
A_INLINE bool SkUnaryParam::compare_less(const SkParameterBase & param) const
  {
  // $Note - CReis The default is not compared [since parameters with defaults are not
  // shared.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compare name first then kind, class type.
  // [Comparing by kind/type is probably more relevant than by name though comparing by
  // name is the fastest.]
  return (m_name < param.get_name())
    || ((m_name == param.get_name())
      && ((param.get_kind() == SkParameter_group)
        || (m_type_p->compare(*static_cast<const SkUnaryParam *>(&param)->m_type_p) == AEquate_less)));
  }

//---------------------------------------------------------------------------------------
A_INLINE uint32_t SkUnaryParam::generate_crc32() const
  {
  // NOTE: default argument is not considered here!
  // Simple XOR shall suffice for combining the two CRCs here since order is fixed anyway
  return get_name_id() ^ m_type_p->generate_crc32();  
  }

