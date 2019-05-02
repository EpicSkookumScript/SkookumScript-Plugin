// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Formal Parameter List/Interface Class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkParameterBase.hpp>


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Description
//   Determines if any of the [unary] arguments a default expression set.  
//
// #Author(s) Conan Reis
A_INLINE bool SkParameters::is_defaulted() const
  {
  uint32_t param_count = m_params.get_length();

  if (param_count)
    {
    SkParameterBase ** params_pp     = m_params.get_array();
    SkParameterBase ** params_end_pp = params_pp + param_count;

    while (params_pp < params_end_pp)
      {
      if ((*params_pp)->is_defaultable())
        {
        return true;
        }

      params_pp++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Determines if these parameters are sharable or not.  
//
// #Author(s) Conan Reis
A_INLINE bool SkParameters::is_sharable() const
  {
  // Don't share parameters that have defaults since they may have unique expression
  // source indexes.
  // $Revisit - CReis [Skookum Debug Only]  This check is only needed for serialization
  // with debug information.
  return !is_defaulted();
  }

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference
//
// #Modifiers static
// #Author(s) Conan Reis
A_INLINE SkParameters * SkParameters::get_or_create(
  SkClassDescBase * result_type_p,
  // Optional single unary parameter - No parameter if nullptr.
  SkParameterBase * param_p // = nullptr
  )
  {
  SkParameters params(result_type_p, param_p);

  return get_or_create(&params);
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference
//
// #Modifiers static
// #Author(s) Conan Reis
A_INLINE SkParameters * SkParameters::get_or_create(
  // Pointer to address to read binary serialization info from and to increment
  // - previously filled using as_binary() or a similar mechanism.
  const void ** binary_pp
  )
  {
  SkParameters params(binary_pp);

  return get_or_create(&params);
  }

#endif
