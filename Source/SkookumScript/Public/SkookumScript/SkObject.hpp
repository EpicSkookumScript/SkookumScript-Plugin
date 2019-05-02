// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic object class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBindingAbstract.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic object class
class SK_API SkObject : public SkClassBindingAbstract<SkObject>, public SkInstance
  {
  public:

    static void       register_bindings();
    static SkClass *  get_class();

  };


