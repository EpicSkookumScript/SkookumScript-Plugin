// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic List (dynamic array of typed objects) class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>
#include <SkookumScript/SkInstanceList.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic List (dynamic array of typed objects) class
class SK_API SkList : public SkClassBindingSimple<SkList, SkInstanceList>
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };
