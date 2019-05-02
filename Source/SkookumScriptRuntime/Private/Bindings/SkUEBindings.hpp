// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Unreal Engine Bindings
//=======================================================================================

#pragma once

class SkUEBindingsInterface;

//---------------------------------------------------------------------------------------
// 
class SkUEBindings
  {
  public:

    // Class Methods

    static void ensure_static_ue_types_registered(SkUEBindingsInterface * game_generated_bindings_p);
    static void begin_register_bindings();
    static void finish_register_bindings(SkUEBindingsInterface * game_generated_bindings_p);

  }; // SkUEBindings

