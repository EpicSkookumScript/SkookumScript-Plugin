// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// A number of handy symbol definitions
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkSymbolDefs.hpp>

//=======================================================================================
// Global Variables
//=======================================================================================

namespace SkSymbolDefs
  {

  //---------------------------------------------------------------------------------------
  // Assign values to the static symbols
  void initialize()
    {
    #define ASYM(_id)         ASYMBOL_ASSIGN(ASymbol, _id)
    #define ASYMX(_id, _str)  ASYMBOL_ASSIGN_STR(ASymbolX, _id, _str)
      SK_SYMBOLS
      SK_SYMBOLS_NAMED
    #undef ASYMX
    #undef ASYM
    }

  //---------------------------------------------------------------------------------------
  // Clean up static symbols
  void deinitialize()
    {
    #define ASYM(_id)         ASYMBOL_ASSIGN_NULL(ASymbol, _id)
    #define ASYMX(_id, _str)  ASYMBOL_ASSIGN_STR_NULL(ASymbolX, _id, _str)
      SK_SYMBOLS
      SK_SYMBOLS_NAMED
    #undef ASYMX
    #undef ASYM
    }

  }

//---------------------------------------------------------------------------------------
// Define ASymbol constants

// Symbol Table to use in macros

#define ASYM(_id)         ASYMBOL_DEFINE_NULL(ASymbol, _id)
#define ASYMX(_id, _str)  ASYMBOL_DEFINE_STR_NULL(ASymbolX, _id, _str)
  SK_SYMBOLS
  SK_SYMBOLS_NAMED
#undef ASYMX
#undef ASYM

