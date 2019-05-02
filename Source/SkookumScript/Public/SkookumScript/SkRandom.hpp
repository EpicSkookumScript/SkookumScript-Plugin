// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Atomic Random Number Generator class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>
#include <AgogCore/ARandom.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript Atomic Random Number Generator class.  It is high speed and
// seed driven (allowing the reproduction of generated sequences).
class SK_API SkRandom : public SkClassBindingSimple<SkRandom, ARandom>
  {
  public:

    enum { Binding_has_dtor = false }; // If to generate destructor

    static void       register_bindings();
    static SkClass *  get_class();

  };
