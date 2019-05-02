// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Actor class - i.e. named simulation objects
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkActor.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkActor.inl>
#endif

#include <SkookumScript/SkActorClass.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Constructor
// 
// Returns:   itself
// Params:
//   name:
//     identifier name for the actor.  Each actor name should be unique, though there is
//     currently no mechanism in place to ensure this.
//     
// Author(s):  Conan Reis
SkActor::SkActor(
  const ASymbol & name,                // = ASymbol::ms_null
  SkActorClass *  class_p,             // = nullptr
  bool            add_to_instance_list // = true
  ) :
  ANamed(name),
  SkDataInstance(class_p ? class_p : SkBrain::ms_actor_class_p)
  {
  if (add_to_instance_list)
    {
    // This will have to be called again elsewhere if Actor data-structures are reused with
    // different classes.
    static_cast<SkActorClass *>(m_class_p)->append_instance(*this);
    }
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Examples:    called by system
// Author(s):    Conan Reis
SkActor::~SkActor()
  {
  static_cast<SkActorClass *>(m_class_p)->remove_instance(*this);
  }

//---------------------------------------------------------------------------------------
// Rename actor - also ensures that lists that store this actor and that sort by name
// properly resort this actor.
// 
// Arg         name - new name for actor
// Author(s):   Conan Reis
void SkActor::rename(const ASymbol & name)
  {
  SkActorClass * class_p = static_cast<SkActorClass *>(m_class_p);
  
  class_p->remove_instance(*this, ATerm_short);
  m_name = name;
  class_p->append_instance(*this);
        }

//---------------------------------------------------------------------------------------
//  Frees up an actor
// Examples:    called by dereference()
// See:         dereference()
// Modifiers:    virtual
// Author(s):    Conan Reis
void SkActor::delete_this()
  {
  m_ptr_id = AIdPtr_null;

  // Actors are not pooled by default
  delete this;
  }

//---------------------------------------------------------------------------------------
// Returns a string representation of itself for debugging purposes
// Returns:    Debug string
// Modifiers:   virtual - overridden from SkInstance
// Author(s):   Conan Reis
AString SkActor::as_string() const
  {
  AString class_name(m_class_p->get_name_str_dbg());
  AString actor_name(m_name.as_str_dbg());
  AString str(nullptr, 6u + actor_name.get_length() + class_name.get_length(), 0u);

  str.append('\'');
  str.append(actor_name);
  str.append("' <", 3u);
  str.append(class_name);
  str.append('>');

  return str;
  }

//---------------------------------------------------------------------------------------

namespace SkActor_Impl
  {

  //---------------------------------------------------------------------------------------
  // Skoo Params !named(Symbol name) Actor
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_ctor_named(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkActor * this_p = static_cast<SkActor *>(scope_p->get_this());

    this_p->set_name(scope_p->get_arg<SkSymbol>(SkArg_1));

    static_cast<SkActorClass *>(this_p->get_class())->append_instance(*this_p);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params =(Actor operand) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_op_equals(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() == scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params ~=(Actor operand) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_op_not_equal(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        scope_p->get_this() != scope_p->get_arg(SkArg_1));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params name() Symbol
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_name(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkSymbol::new_instance(static_cast<SkActor *>(scope_p->get_this())->get_name());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@String() String
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkString::new_instance(static_cast<SkActor *>(scope_p->get_this())->as_string());
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@find_named(Symbol name) <ThisClass_|None>
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_find_named(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkActor * actor_p =
        ((SkActorClass *)((SkMetaClass *)scope_p->get_topmost_scope())->get_class_info())->find_instance(
          scope_p->get_arg<SkSymbol>(SkArg_1));

      *result_pp = actor_p ? actor_p : SkBrain::ms_nil_p;
      (*result_pp)->reference();
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@generate_name_str(String name) String
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_generate_name_str(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkString::new_instance(
        SkActor::generate_unique_name_str(scope_p->get_arg<SkString>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@generate_name_sym(String name) Symbol
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_generate_name_sym(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkSymbol::new_instance(
        SkActor::generate_unique_name_sym(scope_p->get_arg<SkString>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@named(Symbol name) ThisClass_
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_named(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      ASymbol       name     = scope_p->get_arg<SkSymbol>(SkArg_1);
      SkMetaClass * mclass_p = (SkMetaClass *)scope_p->get_topmost_scope();
      SkActor *     actor_p  = (mclass_p && mclass_p->get_class_info())
        ? ((SkActorClass *)mclass_p->get_class_info())->find_instance(name)
        : nullptr;

      #if (SKOOKUM & SK_DEBUG)
        if (actor_p == nullptr)
          {
          A_ERRORX(a_str_format("Tried to get instance named '%s' from class '%s', but no such instance exists!\n", name.as_cstr_dbg(), (mclass_p && mclass_p->get_class_info()) ? mclass_p->get_key_class_name().as_cstr_dbg() : "???"));

          *result_pp = SkBrain::ms_nil_p;

          return;
          }
      #endif

      actor_p->reference();
      *result_pp = actor_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@instances_first() ThisClass_
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instances_first(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkMetaClass *  mclass_p = (SkMetaClass *)scope_p->get_topmost_scope();
      SkActorClass * aclass_p = nullptr;
      SkActor *      actor_p  = nullptr;

      if (mclass_p && mclass_p->get_class_info())
        {
        aclass_p = (SkActorClass *)mclass_p->get_class_info();
        actor_p  = aclass_p->get_instances().get_first();
        }

      #if (SKOOKUM & SK_DEBUG)
        if (actor_p == nullptr)
          {
          A_ERRORX(a_str_format("Tried to get first instance of class '%s', but it has no instances!\n", aclass_p ? aclass_p->get_name_cstr_dbg() : "???"));

          *result_pp = SkBrain::ms_nil_p;

          return;
          }
      #endif

      actor_p->reference();
      *result_pp = actor_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@get_instances_length() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instances_length(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkActorClass * actor_class_p =
        (SkActorClass *)((SkMetaClass *)scope_p->get_topmost_scope())->get_class_info();

      *result_pp = SkInteger::new_instance(tSkInteger(actor_class_p->get_instances().get_length()));
      }
    }

  //---------------------------------------------------------------------------------------
  // Skoo Params Actor@instances() List{ThisClass_}
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_instances(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const tSkActors & actors =
        ((SkActorClass *)((SkMetaClass *)scope_p->get_topmost_scope())->get_class_info())->get_instances();

      uint32_t actor_count = actors.get_length();
      SkInstance * new_p = SkList::new_instance(actor_count);
      SkInstanceList & new_list = new_p->as<SkList>();

      APArray<SkInstance> & instances = new_list.get_instances();

      instances = *reinterpret_cast<const APSortedLogical<SkInstance, ASymbol> *>(&actors);
      instances.apply_method(&SkInstance::reference);

      *result_pp = new_p;
      }
    }

  //---------------------------------------------------------------------------------------

  // Instance methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!named",            mthd_ctor_named },
      { "String",            mthd_string },
      { "equal?",            mthd_op_equals },
      { "name",              mthd_name },
      { "not_equal?",        mthd_op_not_equal },
    };

  // Class methods
  static const SkClass::MethodInitializerFunc methods_c[] =
    {
      { "find_named",        mthdc_find_named },
      { "generate_name_str", mthdc_generate_name_str },
      { "generate_name_sym", mthdc_generate_name_sym },
      { "named",             mthdc_named },
      { "instances_first",   mthdc_instances_first },
      { "instances_length",  mthdc_instances_length },
      { "instances",         mthdc_instances },
    };

  } // namespace

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Registers the atomic classes, coroutines, etc.
// Examples:   This method is called by SkAtomic::register_bindings()
// Modifiers:   static
// Author(s):   Conan Reis
void SkActor::register_bindings()
  {
  if (SkookumScript::get_app_info()->use_builtin_actor())
    {
    initialize_class(ASymbol_Actor);

    ms_class_p->register_method_func_bulk(SkActor_Impl::methods_i, A_COUNT_OF(SkActor_Impl::methods_i), SkBindFlag_instance_no_rebind);
    ms_class_p->register_method_func_bulk(SkActor_Impl::methods_c, A_COUNT_OF(SkActor_Impl::methods_c), SkBindFlag_class_no_rebind);
    }
  }

//---------------------------------------------------------------------------------------
// Generates a unique actor name based on supplied name root and ensuring that there is
// no conflict/collision with actor objects that currently exist.
//
// #Examples
//   AString gen_name(SkActor::generate_unique_name_str("Actor"));
//   
//   // Successively creates:
//   //   "Actor1"
//   //   "Actor2"
//   //   "Actor3"
//   //   ... etc. wherever there is an open name slot/index
//
// #Notes
//   The generated name is only reserved if an actor is created with it.
//
// #See Also  generate_unique_name_sym()
// #Modifiers static
// #Author(s) Conan Reis
AString SkActor::generate_unique_name_str(
  const AString & name_root,
  uint32_t *      create_idx_p // = nullptr
  )
  {
  AString   unique_name(name_root, 3u);
  uint32_t  root_length = name_root.get_length();
  uint32_t  create_idx  = 0u;

  SK_ASSERTX(SkookumScript::get_app_info()->use_builtin_actor(), "SkBrain::ms_actor_class_p must be of type SkActorClass here");
  SkActorClass * actor_class_p = static_cast<SkActorClass *>(SkBrain::ms_actor_class_p);

  do
    {
    create_idx++;

    unique_name.set_length(root_length);

    // $Revisit - CReis could make more efficient with in-place uint->str
    unique_name.append(AString::ctor_uint(create_idx));
    }
  while (actor_class_p->find_instance(ASymbol::create(unique_name, ATerm_short)));

  if (create_idx_p)
    {
    *create_idx_p = create_idx;
    }

  return unique_name;
  }

//---------------------------------------------------------------------------------------
// Generates a unique actor name symbol based on supplied name root and ensuring that
// there is no conflict/collision with actor objects that currently exist.
//
// #Examples
//   ASymbol gen_name(SkActor::generate_unique_name_sym("Actor"));
//   
//   // Successively creates:
//   //   'Actor1'
//   //   'Actor2'
//   //   'Actor3'
//   //   ... etc.
//
// #Notes
//   The generated name is only reserved if an actor is created with it.
//
// #See Also  generate_unique_name_sym()
// #Modifiers static
// #Author(s) Conan Reis
ASymbol SkActor::generate_unique_name_sym(
  const AString & name_root,
  uint32_t *      create_idx_p // = nullptr
  )
  {
  ASymbol   unique_sym;
  AString   unique_name(name_root, 3u);
  uint32_t  root_length = name_root.get_length();
  uint32_t  create_idx  = 0u;

  SK_ASSERTX(SkookumScript::get_app_info()->use_builtin_actor(), "SkBrain::ms_actor_class_p must be of type SkActorClass here");
  SkActorClass * actor_class_p = static_cast<SkActorClass *>(SkBrain::ms_actor_class_p);

  do
    {
    create_idx++;

    unique_name.set_length(root_length);

    // $Revisit - CReis could make more efficient with in-place uint->str
    unique_name.append(AString::ctor_uint(create_idx));
    unique_sym = ASymbol::create(unique_name, ATerm_short);
    }
  while (actor_class_p->find_instance(unique_sym));

  if (create_idx_p)
    {
    *create_idx_p = create_idx;
    }

  return unique_sym;
  }

