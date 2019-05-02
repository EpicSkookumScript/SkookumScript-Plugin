// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Manages Symbols for SkookumScript
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ASymbolTable.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

namespace SkUESymbol
  {
  // Assign values to the static symbols
  void initialize();

  // Clean up static symbols
  void deinitialize();
  }

//---------------------------------------------------------------------------------------
// These Macros will expand to create constants of type ASymbol in the form:
//   ASYM(Object) -> ASymbol_Object  // Representing the symbol 'Object'
//
// Extra care should be taken when using them during global initialization time.
//
// Eventually, in release builds, these could be just be numerical ids and AString objects
// will be dropped all together.
//
// Try to keep them in alphabetical order so that they are easy to scan at a glance.
//
#define SKUE_SYMBOLS \
  ASYM(World) \
  ASYM(_on_init_project) \


//---------------------------------------------------------------------------------------
// Ids that cannot be represented as C++ identifiers.
// They take the form of:
//   ASYMX(nearQ, "nearQ") -> ASymbolX_nearQ
//
// Extra care should be taken when using them during global initialization time.
//
// Try to keep them in alphabetical order so that they are easy to scan at a glance.
//
#define SKUE_SYMBOLS_NAMED \
  ASYMX(c_world,  "@@world") \
  ASYMX(nearQ,    "near?") \


//---------------------------------------------------------------------------------------
// Declare ASymbol constants
#define ASYM(_id)         ASYMBOL_DECLARE(ASymbol, _id)
#define ASYMX(_id, _str)  ASYMBOL_DECLARE(ASymbolX, _id)
  SKUE_SYMBOLS
  SKUE_SYMBOLS_NAMED
#undef ASYMX
#undef ASYM
