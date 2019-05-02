// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Boolean (true/false) class - allows short circuit evaluation.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>


//---------------------------------------------------------------------------------------
// Notes      SkookumScript Atomic Boolean (true/false) - allows short circuit evaluation.
//            Has same data as SkInstance - only differs in that it has a different
//            virtual method table.
class SK_API SkBoolean : public SkClassBindingSimpleZero<SkBoolean, tSkBoolean>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };

