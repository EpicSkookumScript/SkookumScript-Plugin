// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Manages Symbols for SkookumScript
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>
#include <AgogCore/ASymbolTable.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic Symbol class
class SK_API SkSymbol : public SkClassBindingSimple<SkSymbol, ASymbol>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
