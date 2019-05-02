// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Integer class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic Integer class
class SK_API SkInteger : public SkClassBindingSimpleZero<SkInteger, tSkInteger>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };


