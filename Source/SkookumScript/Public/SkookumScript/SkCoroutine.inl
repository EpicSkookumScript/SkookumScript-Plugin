// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Coroutine parameters & body classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================


//=======================================================================================
// SkCoroutineMthd Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - identifier name for the coroutine 
// Arg         scope_p - class scope to use
// Arg         params_p - parameters object to take over contents of
// Arg         update_m - Atomic method to call when the coroutine is invoked / updating
//             - nullptr if supplied at a later time.  (Default nullptr)
// See:        Transfer constructor of SkParameters.
// Author(s):   Conan Reis
A_INLINE SkCoroutineMthd::SkCoroutineMthd(
  const ASymbol & name,
  SkClass *       scope_p,
  SkParameters *  params_p,
  uint32_t        annotation_flags,
  tSkCoroutineMthd update_m // = nullptr
  ) :
  SkCoroutineBase(name, scope_p, params_p, params_p->get_arg_count_total(), annotation_flags),
  m_update_m(update_m)
  {
  }

//---------------------------------------------------------------------------------------
// Sets the update method for this coroutine
// Arg         update_m - Atomic method to call when the coroutine is invoked / updating
// Author(s):   Conan Reis
A_INLINE void SkCoroutineMthd::set_update(tSkCoroutineMthd update_m)
  {
  m_update_m = update_m;
  }


//=======================================================================================
// SkCoroutineFunc Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - identifier name for the coroutine 
// Arg         scope_p - class scope to use
// Arg         params_p - parameters object to take over contents of
// Arg         update_f - Atomic method to call when the coroutine is invoked / updating
//             - nullptr if supplied at a later time.  (Default nullptr)
// See:        Transfer constructor of SkParameters.
// Author(s):   Conan Reis
A_INLINE SkCoroutineFunc::SkCoroutineFunc(
  const ASymbol & name,
  SkClass *       scope_p,
  SkParameters *  params_p,
  uint32_t        annotation_flags,
  tSkCoroutineFunc update_f // = nullptr
  ) :
  SkCoroutineBase(name, scope_p, params_p, params_p->get_arg_count_total(), annotation_flags),
  m_update_f(update_f)
  {
  }

//---------------------------------------------------------------------------------------
// Sets the update method for this coroutine
// Arg         update_f - Atomic method to call when the coroutine is invoked / updating
// Author(s):   Conan Reis
A_INLINE void SkCoroutineFunc::set_update(tSkCoroutineFunc update_f)
  {
  m_update_f = update_f;
  }

