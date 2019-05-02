// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Additional bindings for the EntityClass (= UClass) class 
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkUEEntityClass.hpp"
#include "../SkUEUtils.hpp"

//---------------------------------------------------------------------------------------

namespace SkUEEntityClass_Impl
  {

  //=======================================================================================
  // Methods
  //=======================================================================================

  //---------------------------------------------------------------------------------------
  // # Skookum:   UClass@String() String
  // # Author(s): Markus Breyer
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp) // Do nothing if result not desired
      {
      UClass * uclass_p = scope_p->this_as<SkUEEntityClass>();
      AString uclass_name = uclass_p ? FStringToAString(uclass_p->GetName()) : AString("???", true);

      AString str(nullptr, 3u + uclass_name.get_length(), 0u);
      str.append('(');
      str.append(uclass_name);
      str.append(')');
      *result_pp = SkString::new_instance(str);
      }
    }

  static const SkClass::MethodInitializerFunc methods_i2[] =
    {
      { "String", mthd_String },
    };

  } // SkUEEntityClass_Impl

//---------------------------------------------------------------------------------------
void SkUEEntityClass_Ext::register_bindings()
  {
  ms_class_p->register_method_func_bulk(SkUEEntityClass_Impl::methods_i2, A_COUNT_OF(SkUEEntityClass_Impl::methods_i2), SkBindFlag_instance_no_rebind);
  }
