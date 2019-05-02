// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Special None class for the single nil instance.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBindingAbstract.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
class SK_API SkNone : public SkClassBindingAbstract<SkNone>, public SkInstanceUnreffed
  {
  public:
    
    SK_NEW_OPERATORS(SkNone);

    SkNone();

    static void       initialize();
    static SkClass *  get_class();

  };  // SkNone
