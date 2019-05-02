// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Real (floating point) class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic Real (floating point) class
class SK_API SkReal : public SkClassBindingSimpleZero<SkReal, tSkReal>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };


