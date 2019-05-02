// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Bindings for the Actor (= AActor) class 
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include "SkUEActorComponent.hpp"
#include "SkUEActor.generated.hpp"
#include "SkUEName.hpp"

#include <SkookumScript/SkList.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkUEActorComponent_Impl
  {

  //---------------------------------------------------------------------------------------
  // Helper function for various from methods below
  static bool setup_from(SkInvokedMethod * scope_p, AActor ** actor_pp, SkClass ** sk_class_pp, SkClass ** known_sk_class_pp, UClass ** ue_class_pp)
    {
    // Class we are looking for
    SkClass * sk_class_p = *sk_class_pp = ((SkMetaClass *)scope_p->get_topmost_scope())->get_class_info();

    // Actor we are getting the component(s) from 
    AActor * actor_p = *actor_pp = scope_p->get_arg<SkUEActor>(SkArg_1);
    SK_ASSERTX(actor_p, a_str_format("Failed to get a '%s' from null actor.", sk_class_p->get_name_cstr_dbg()));

    // Figure out which UE4 class to look for
    bool is_class_valid = true;
    if (actor_p)
      {
      SkClass * known_sk_class_p = *known_sk_class_pp = SkUEClassBindingHelper::find_most_derived_super_class_known_to_ue(sk_class_p, ue_class_pp);
      is_class_valid = sk_class_p->is_component_class() || known_sk_class_p == sk_class_p;
      SK_ASSERTX(is_class_valid, a_str_format("Failed to get a '%s' from actor '%S'. Make sure that '%s' is either a class known to UE4 (a Blueprint class or an exposed UCLASS), or it is erived from SkookumScriptBehaviorComponent.", sk_class_p->get_name_cstr_dbg(), *actor_p->GetName(), sk_class_p->get_name_cstr_dbg()));
      }

    return actor_p && is_class_valid;
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@from()
  static void mthd_from(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp) // Do nothing if result not desired
      {
      SkInstance * result_p = nullptr;

      AActor * actor_p = nullptr;
      SkClass * sk_class_p = nullptr;
      UClass * ue_class_p = nullptr;
      SkClass * known_sk_class_p = nullptr;
      if (setup_from(scope_p, &actor_p, &sk_class_p, &known_sk_class_p, &ue_class_p))
        {
        const TSet<UActorComponent *> & components = actor_p->GetComponents();
        for (UActorComponent * component_p : components)
          {
          if (component_p && component_p->IsA(ue_class_p))
            {
            // Is this a SkookumScriptBehaviorComponent?
            if (known_sk_class_p->is_component_class())
              {
              // Yes, check class of its instance
              SkInstance * instance_p = static_cast<USkookumScriptBehaviorComponent *>(component_p)->get_sk_component_instance();
              if (instance_p->get_class()->is_class(*sk_class_p))
                {
                result_p = instance_p;
                result_p->reference();
                break;
                }
              }
            else
              {
              result_p = SkUEActorComponent::new_instance(component_p);
              break;
              }
            }
          }
        }

      *result_pp = result_p ? result_p : SkBrain::ms_nil_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@from_tag()
  static void mthd_from_tag(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp) // Do nothing if result not desired
      {
      SkInstance * result_p = nullptr;

      AActor * actor_p = nullptr;
      SkClass * sk_class_p = nullptr;
      UClass * ue_class_p = nullptr;
      SkClass * known_sk_class_p = nullptr;
      if (setup_from(scope_p, &actor_p, &sk_class_p, &known_sk_class_p, &ue_class_p))
        {
        FName tag = scope_p->get_arg<SkUEName>(SkArg_2);

        const TSet<UActorComponent *> & components = actor_p->GetComponents();
        for (UActorComponent * component_p : components)
          {
          if (component_p && component_p->IsA(ue_class_p) && component_p->ComponentHasTag(tag))
            {
            // Is this a SkookumScriptBehaviorComponent?
            if (known_sk_class_p->is_component_class())
              {
              // Yes, check class of its instance
              SkInstance * instance_p = static_cast<USkookumScriptBehaviorComponent *>(component_p)->get_sk_component_instance();
              if (instance_p->get_class()->is_class(*sk_class_p))
                {
                result_p = instance_p;
                result_p->reference();
                break;
                }
              }
            else
              {
              result_p = SkUEActorComponent::new_instance(component_p);
              break;
              }
            }
          }
        }

      *result_pp = result_p ? result_p : SkBrain::ms_nil_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@list_from()
  static void mthd_list_from(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp) // Do nothing if result not desired
      {
      SkInstance * result_p = SkList::new_instance();

      AActor * actor_p = nullptr;
      SkClass * sk_class_p = nullptr;
      UClass * ue_class_p = nullptr;
      SkClass * known_sk_class_p = nullptr;
      if (setup_from(scope_p, &actor_p, &sk_class_p, &known_sk_class_p, &ue_class_p))
        {
        const TSet<UActorComponent *> & components = actor_p->GetComponents();
        SkInstanceList & list = result_p->as<SkList>();
        list.ensure_size(components.Num()); // Safe bet, it's only pointers, so a few more won't hurt at all compared to reallocation performance hit
        for (UActorComponent * component_p : components)
          {
          if (component_p && component_p->IsA(ue_class_p))
            {
            // Is this a SkookumScriptBehaviorComponent?
            if (known_sk_class_p->is_component_class())
              {
              // Yes, check class of its instance
              SkInstance * elem_p = static_cast<USkookumScriptBehaviorComponent *>(component_p)->get_sk_component_instance();
              if (elem_p->get_class()->is_class(*sk_class_p))
                {
                list.append(*elem_p, true);
                }
              }
            else
              {
              list.append(*SkUEActorComponent::new_instance(component_p), false);
              }
            }
          }
        }

      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@list_from_tag()
  static void mthd_list_from_tag(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    if (result_pp) // Do nothing if result not desired
      {
      SkInstance * result_p = SkList::new_instance();

      AActor * actor_p = nullptr;
      SkClass * sk_class_p = nullptr;
      UClass * ue_class_p = nullptr;
      SkClass * known_sk_class_p = nullptr;
      if (setup_from(scope_p, &actor_p, &sk_class_p, &known_sk_class_p, &ue_class_p))
        {
        FName tag = scope_p->get_arg<SkUEName>(SkArg_2);

        const TSet<UActorComponent *> & components = actor_p->GetComponents();
        SkInstanceList & list = result_p->as<SkList>();
        list.ensure_size(components.Num()); // Safe bet, it's only pointers, so a few more won't hurt at all compared to reallocation performance hit
        for (UActorComponent * component_p : components)
          {
          if (component_p && component_p->IsA(ue_class_p) && component_p->ComponentHasTag(tag))
            {
            // Is this a SkookumScriptBehaviorComponent?
            if (known_sk_class_p->is_component_class())
              {
              // Yes, check class of its instance
              SkInstance * elem_p = static_cast<USkookumScriptBehaviorComponent *>(component_p)->get_sk_component_instance();
              if (elem_p->get_class()->is_class(*sk_class_p))
                {
                list.append(*elem_p, true);
                }
              }
            else
              {
              list.append(*SkUEActorComponent::new_instance(component_p), false);
              }
            }
          }
        }

      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@register_component()
  static void mthd_register_component(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    scope_p->this_as<SkUEActorComponent>()->RegisterComponent();
    }

  //---------------------------------------------------------------------------------------
  // ActorComponent@unregister_component()
  static void mthd_unregister_component(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    scope_p->this_as<SkUEActorComponent>()->UnregisterComponent();
    }

  static const SkClass::MethodInitializerFunc methods_i2[] =
    {
      { "register_component",   mthd_register_component },
      { "unregister_component", mthd_unregister_component },
    };

  static const SkClass::MethodInitializerFunc methods_c2[] =
    {
      { "from",                 mthd_from },
      { "from_tag",             mthd_from_tag },
      { "list_from",            mthd_list_from },
      { "list_from_tag",        mthd_list_from_tag },
    };

  } // SkUEActorComponent_Impl

//---------------------------------------------------------------------------------------

void SkUEActorComponent_Ext::register_bindings()
  {
  ms_class_p->register_method_func_bulk(SkUEActorComponent_Impl::methods_i2, A_COUNT_OF(SkUEActorComponent_Impl::methods_i2), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkUEActorComponent_Impl::methods_c2, A_COUNT_OF(SkUEActorComponent_Impl::methods_c2), SkBindFlag_class_no_rebind);
  }


