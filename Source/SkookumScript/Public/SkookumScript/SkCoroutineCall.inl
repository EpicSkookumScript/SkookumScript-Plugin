// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for info used to make a method call/invocation - i.e. coroutine
//             identifier (name/index) and passed argument info.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================


//=======================================================================================
// Inline Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - coroutine name id
//               4 bytes - scope name id or ASymbol_id_null if not used
//               4 bytes - number of arguments
//               1 byte  - argument type     \_ Repeating
//               n bytes - argument binary   / 
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkCoroutineCall::SkCoroutineCall(const void ** binary_pp)
  {
  assign_binary(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


