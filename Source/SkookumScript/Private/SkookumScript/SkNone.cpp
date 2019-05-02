// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic none/nil class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkNone.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkNone::SkNone() : SkInstanceUnreffed(ms_class_p)
  {
  SK_ASSERTX(ms_class_p, "Class instance must be non-null here.");
  }

//---------------------------------------------------------------------------------------

void SkNone::initialize()
  {
  SkClassBindingAbstract::initialize_class(ASymbolId_None);
  }

//---------------------------------------------------------------------------------------

SkClass * SkNone::get_class()
  {
  return ms_class_p;
  }
