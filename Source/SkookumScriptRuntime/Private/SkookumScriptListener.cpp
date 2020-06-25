// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// UObject that listens to dynamic multicast delegate events
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptListener.h"
#include "SkookumScriptListenerManager.hpp"
#include "Bindings/VectorMath/SkVector3.hpp"
#include "Bindings/Engine/SkUEName.hpp"
#include <SkUEEntity.generated.hpp>

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>

//=======================================================================================
// FSkookumScriptListenerAutoPtr
//======================================================================================

FSkookumScriptListenerAutoPtr::~FSkookumScriptListenerAutoPtr()
  {
  USkookumScriptListener * listener_p = Get();
  SK_ASSERTX_NO_THROW(listener_p, "USkookumScriptListeners are entirely under Sk control and cannot just go away.");
  SkookumScriptListenerManager::get_singleton()->free_listener(listener_p);
  }

//=======================================================================================
// Class Data
//=======================================================================================

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

USkookumScriptListener::USkookumScriptListener(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , m_num_arguments(0)
  , m_unregister_callback_p(nullptr)
  {
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::initialize(UObject * obj_p, SkInvokedCoroutine * coro_p, tUnregisterCallback callback_p)
  {
  SK_ASSERTX(!coro_p->is_suspended(), "Coroutine must not be suspended yet when delegate object is initialized.");

  m_obj_p = obj_p;
  m_coro_p = coro_p;
  m_unregister_callback_p = callback_p;
  m_num_arguments = 0;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::deinitialize()
  {
  // Kill any events that are still around
  while (has_event())
    {
    free_event(pop_event(), true);
    }

  // Forget the coroutine we keep track of
  m_coro_p.null();

  // Unregister from delegate list if any
  if (m_unregister_callback_p && m_obj_p.IsValid())
    {
    (*m_unregister_callback_p)(m_obj_p.Get(), this);
    }
  }

//---------------------------------------------------------------------------------------

USkookumScriptListener::EventInfo * USkookumScriptListener::alloc_event()
  {
  EventInfo * event_p = SkookumScriptListenerManager::get_singleton()->alloc_event();
  #if (SKOOKUM & SK_DEBUG)
    ::memset(event_p->m_argument_p, 0, sizeof(event_p->m_argument_p));
  #endif
  return event_p;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::free_event(EventInfo * event_p, bool free_arguments)
  {
  SkookumScriptListenerManager::get_singleton()->free_event(event_p, free_arguments ? m_num_arguments : 0);
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::push_event_and_resume(EventInfo * event_p, uint32_t num_arguments)
  {
  #if (SKOOKUM & SK_DEBUG)
    for (uint32_t i = 0; i < num_arguments; ++i) SK_ASSERTX(event_p->m_argument_p[i], "All event arguments must be set.");
    for (uint32_t i = num_arguments; i < A_COUNT_OF(event_p->m_argument_p); ++i) SK_ASSERTX(!event_p->m_argument_p[i], "Unused event arguments must be left alone.");
    SK_ASSERTX(m_num_arguments == 0 || m_num_arguments == num_arguments, "All events must have same argument count.");
  #endif
  m_num_arguments = num_arguments;
  m_event_queue.append(event_p);
  if (m_coro_p.is_valid()) m_coro_p->resume();
  }

//---------------------------------------------------------------------------------------

bool USkookumScriptListener::coro_on_event_do(SkInvokedCoroutine * scope_p, tRegisterCallback register_f, tUnregisterCallback unregister_f, bool do_until)
  {
  UObject * this_p = scope_p->this_as<SkUEEntity>();

  SK_ASSERTX(this_p, a_str_format("Tried to attach an event handler to an Entity of type '%s' but it is null!", scope_p->get_this()->get_class()->get_name_cstr()));
  // If this_p is null, we can't listen for events so return immediately
  if (!this_p) return true;

  // Just started?
  if (scope_p->m_update_count == 0u)
    {
    // Install and store away event listener
    USkookumScriptListener * listener_p = SkookumScriptListenerManager::get_singleton()->alloc_listener(this_p, scope_p, unregister_f);
    scope_p->append_user_data<FSkookumScriptListenerAutoPtr, USkookumScriptListener *>(listener_p);
    (*register_f)(this_p, listener_p);

    // Suspend coroutine
    scope_p->suspend();

    // Coroutine not complete yet - call again when resumed
    return false;
    }

  // Get back stored event listener
  USkookumScriptListener * listener_p = scope_p->get_user_data<FSkookumScriptListenerAutoPtr>()->Get();
  SK_ASSERTX(listener_p->has_event(), "Must have event at this point as coroutine was resumed by delegate object.");

  // Run closure on each event accumulated in the listener
  SkClosure * closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);
  uint32_t num_arguments = listener_p->get_num_arguments();
  bool exit = false;
  SkInstance * closure_result_p = SkBrain::ms_nil_p;
  SkInstance * return_value_p = SkBrain::ms_nil_p;
  do
    {
    // Use event parameters to invoke closure, then recycle event
    USkookumScriptListener::EventInfo * event_p = listener_p->pop_event();
    if (do_until)
      {
      // Add reference to potential return values so they survive closure_method_call 
      for (uint32_t i = 0; i < num_arguments; ++i)
        {
        event_p->m_argument_p[SkArg_1 + i]->reference();
        }
      }
    closure_p->closure_method_call(&event_p->m_argument_p[0], listener_p->get_num_arguments(), &closure_result_p, scope_p);
    if (do_until)
      {
      exit = closure_result_p->as<SkBoolean>();
      if (exit)
        {
        for (uint32_t i = 0; i < num_arguments; ++i)
          {
          scope_p->set_arg(SkArg_2 + i, event_p->m_argument_p[i]); // Store parameters as return values if exiting
          }
        }
      else
        {
        for (uint32_t i = 0; i < num_arguments; ++i)
          {
          event_p->m_argument_p[i]->dereference(); // Dereference parameters if not needed after all
          }
        }
      closure_result_p->dereference(); // Free Boolean return value
      }
    listener_p->free_event(event_p, false);
    } while (listener_p->has_event() && !exit);

    if (!do_until || !exit)
      {
      // We're not done - wait for more events
      scope_p->suspend();
      return false;
      }

    // Ok done, return event parameters and quit
    return true;
  }

//---------------------------------------------------------------------------------------

bool USkookumScriptListener::coro_wait_event(SkInvokedCoroutine * scope_p, tUnregisterCallback register_f, tUnregisterCallback unregister_f)
  {
  UObject * this_p = scope_p->this_as<SkUEEntity>();

  SK_ASSERTX(this_p, a_str_format("Tried to wait for an event on an Entity of type '%s' but it is null!", scope_p->get_this()->get_class()->get_name_cstr()));
  // If this_p is null, we can't listen for events so return immediately
  if (!this_p) return true;

  // Just started?
  if (scope_p->m_update_count == 0u)
    {
    // If this is null, treat it as if there's nothing to do
    if (!this_p) return true;

    // Install and store away event listener
    USkookumScriptListener * listener_p = SkookumScriptListenerManager::get_singleton()->alloc_listener(this_p, scope_p, unregister_f);
    scope_p->append_user_data<FSkookumScriptListenerAutoPtr, USkookumScriptListener *>(listener_p);
    (*register_f)(this_p, listener_p);

    // Suspend coroutine
    scope_p->suspend();

    // Coroutine not complete yet - call again when resumed
    return false;
    }

  // Get back stored event listener
  USkookumScriptListener * listener_p = scope_p->get_user_data<FSkookumScriptListenerAutoPtr>()->Get();
  SK_ASSERTX(listener_p->has_event(), "Must have event at this point as coroutine was resumed by delegate object.");

  // Return first event queued up on listener
  // and *DISCARD* potential other events
  SkClosure * closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);
  uint32_t num_arguments = listener_p->get_num_arguments();
  bool exit = false;
  USkookumScriptListener::EventInfo * event_p = listener_p->pop_event();
  for (uint32_t i = 0; i < num_arguments; ++i)
    {
    scope_p->set_arg(SkArg_1 + i, event_p->m_argument_p[i]); // Store parameters as return values if exiting
    }
  listener_p->free_event(event_p, false);

  // Ok done, return event parameters and quit
  return true;
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::add_dynamic_function(FName callback_name, UClass * callback_owner_class_p, FNativeFuncPtr exec_p)
  {
  // Find the function
  UFunction * function_p = StaticClass()->FindFunctionByName(callback_name, EIncludeSuperFlag::ExcludeSuper);
  if (!function_p)
    {
    // Duplicate the signature function object

    // Find callback event object on owner class
    FMulticastDelegateProperty * event_property_p = CastChecked<FMulticastDelegateProperty>(callback_owner_class_p->FindPropertyByName(callback_name));

    // Duplicate it
    function_p = DuplicateObject<UFunction>(event_property_p->SignatureFunction, StaticClass(), callback_name);

    // Adjust parameters
    function_p->FunctionFlags |= FUNC_Public | FUNC_BlueprintCallable | FUNC_Native;
    function_p->SetNativeFunc(exec_p);
    function_p->StaticLink(true);
    function_p->AddToRoot(); // Since 4.21, classes and their functions are expected to be permanent objects in cooked builds
    for (TFieldIterator<FProperty> param_it(function_p); param_it; ++param_it)
      {
      // Callback parameters are always inputs
      (*param_it)->PropertyFlags &= ~CPF_OutParm;
      }

    // Make method known to its class
    function_p->Next = StaticClass()->Children;
    StaticClass()->Children = function_p;
    StaticClass()->AddNativeFunction(*callback_name.ToString(), exec_p);
    StaticClass()->AddFunctionToFunctionMap(function_p, function_p->GetFName());
    }
  }

//---------------------------------------------------------------------------------------

void USkookumScriptListener::remove_dynamic_function(FName callback_name)
  {
  // Find the function
  UFunction * function_p = StaticClass()->FindFunctionByName(callback_name, EIncludeSuperFlag::ExcludeSuper);
  if (function_p)
    {
    // Unlink it from class
    StaticClass()->RemoveFunctionFromFunctionMap(function_p);
    UField ** prev_field_pp = &StaticClass()->Children;
    for (UField * field_p = *prev_field_pp; field_p; prev_field_pp = &field_p->Next, field_p = *prev_field_pp)
      {
      if (field_p == function_p)
        {
        *prev_field_pp = field_p->Next;
        break;
        }
      }

    // Destroy the function along with its attached properties
    function_p->RemoveFromRoot();
    function_p->MarkPendingKill();
    }
  }
