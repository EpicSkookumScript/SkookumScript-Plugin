// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Enumeration class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBinding.hpp>

//---------------------------------------------------------------------------------------
// SkookumScript atomic Enumeration class
class SK_API SkEnum : public SkClassBindingSimpleZero<SkEnum, tSkEnum>
  {
  public:

    // Class Methods

    static SkInstance *   new_instance(tSkEnum value, SkClass * enum_class_p);
    static ANamed *       get_class_data_enum_name(SkClass * enum_class_p, tSkEnum enum_value);

    static void           register_bindings();
    static SkClass *      get_class();

  };

//=======================================================================================
// Inline Methods
//=======================================================================================

inline SkInstance * SkEnum::new_instance(tSkEnum value, SkClass * enum_class_p)
  {
  SkInstance * instance_p = SkInstance::new_instance(enum_class_p);

  instance_p->construct<SkEnum>(value);

  return instance_p;
  }
