// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Method parameters & body classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

//=======================================================================================
// SkMethodFunc Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - identifier name for the method
// Arg         scope_p - class scope to use
// Arg         params_p - parameters object to take over contents of
// Arg         atomic_f - C++ static / class member function to call when the method is
//             invoked / called - nullptr if supplied at a later time.  (Default nullptr)
// See:        Transfer constructor of SkParameters.
// Author(s):   Conan Reis
A_INLINE SkMethodFunc::SkMethodFunc(
  const ASymbol & name,
  SkClass *       scope_p,
  SkParameters *  params_p,
  uint32_t        annotation_flags,
  tSkMethodFunc   atomic_f // = nullptr
  ) :
  SkMethodBase(name, scope_p, params_p, params_p->get_arg_count_total(), annotation_flags),
  m_atomic_f(atomic_f)
  {
  }


//=======================================================================================
// SkMethodMthd Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - identifier name for the method
// Arg         scope_p - class scope to use
// Arg         params_p - parameters object to take over contents of
// Arg         atomic_m - C++ method to call when the method is invoked / called - nullptr
//             if supplied at a later time.  (Default nullptr)
// See:        Transfer constructor of SkParameters.
// Author(s):   Conan Reis
A_INLINE SkMethodMthd::SkMethodMthd(
  const ASymbol & name,
  SkClass *       scope_p,
  SkParameters *  params_p,
  uint32_t        annotation_flags,
  tSkMethodMthd   atomic_m // = nullptr
  ) :
  SkMethodBase(name, scope_p, params_p, params_p->get_arg_count_total(), annotation_flags),
  m_atomic_m(atomic_m)
  {
  }

//=======================================================================================
// SkMethod Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Overwrite the expression, written as a hack to allow external Skookum commands
//			   to keep their code after it has run
// Arg         expr_p - pointer to code (can be nullptr)
// Notes:      Function so we can set the expression to nullptr so that it doesn't get
//             deleted by the method destructor intended only to be used as a hack to get
//             the external Skookum calls to work correctly
// Author(s):   Richard Orth
A_INLINE void SkMethod::replace_expression(SkExpressionBase * expr_p)
  {
  m_expr_p = expr_p;
  }

