// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic String class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>
#include <AgogCore/AString.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic String class
class SK_API SkString : public SkClassBindingSimple<SkString, AString>
  {
  public:

    static void       to_escape_string(const AString & str, AString * esc_str_p, bool break_newlines = false);
    static AString    to_escape_string(const AString & str, bool break_newlines = false)  { AString estr; to_escape_string(str, &estr, break_newlines); return estr; }

    static void       register_bindings();
    static SkClass *  get_class();

  };

