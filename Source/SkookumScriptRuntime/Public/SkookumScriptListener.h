// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// UObject that listens to dynamic multicast delegate events
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Components/PrimitiveComponent.h"

#include <AgogCore/AIdPtr.hpp>
#include <AgogCore/AList.hpp>
#include <AgogCore/AObjReusePool.hpp>
#include <SkookumScript/SkUserData.hpp>

#include "SkookumScriptListener.generated.h"

//=======================================================================================
// Global Defines / Macros
//=======================================================================================

class SkInstance;
class SkInvokedCoroutine;
class USkookumScriptListener;

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// UObject-derived proxy class allowing callbacks from dynamic delegates
UCLASS()
class SKOOKUMSCRIPTRUNTIME_API USkookumScriptListener : public UObject
  {

    GENERATED_UCLASS_BODY()

  public:

  // Types

    struct EventInfo : AListNode<EventInfo>
      {
      SkInstance *  m_argument_p[9];

      EventInfo **  get_pool_unused_next() { return (EventInfo **)&m_argument_p[0]; } // Area in this class where to store the pointer to the next unused object when not in use
      };

    typedef void (*tRegisterCallback)(UObject *, USkookumScriptListener *);
    typedef void (*tUnregisterCallback)(UObject *, USkookumScriptListener *);

  // Public Data Members

  // Methods

    void                initialize(UObject * obj_p, SkInvokedCoroutine * coro_p, tUnregisterCallback callback_p);
    void                deinitialize();

    uint32_t            get_num_arguments() const { return m_num_arguments; }

    bool                has_event() const;
    EventInfo *         pop_event();
    void                free_event(EventInfo * event_p, bool free_arguments);

    static bool         coro_on_event_do(SkInvokedCoroutine * scope_p, tUnregisterCallback register_f, tUnregisterCallback unregister_f, bool do_until);
    static bool         coro_wait_event(SkInvokedCoroutine * scope_p, tUnregisterCallback register_f, tUnregisterCallback unregister_f);

  protected:

    friend class AObjReusePool<EventInfo>;

  // Internal Methods

    static EventInfo *  alloc_event();
    void                push_event_and_resume(EventInfo * event_p, uint32_t num_arguments);
    static void         add_dynamic_function(FName callback_name, UClass * callback_owner_class_p, FNativeFuncPtr exec_p);
    static void         remove_dynamic_function(FName callback_name);

  // Internal Data Members

    FWeakObjectPtr              m_obj_p;                 // UObject we belong to
    AIdPtr<SkInvokedCoroutine>  m_coro_p;                // The coroutine that is suspended waiting for events from this object
    AList<EventInfo>            m_event_queue;           // Queued up events waiting to be processed
    uint32_t                    m_num_arguments;         // How many arguments the event has
    tUnregisterCallback         m_unregister_callback_p; // How to unregister myself from the delegate list I am hooked up to

  };  // USkookumScriptListener

//---------------------------------------------------------------------------------------
// Helper class to hold a pointer to a USkookumScriptListener
// and make sure it gets destroyed when the pointer goes away
class FSkookumScriptListenerAutoPtr : public TWeakObjectPtr<USkookumScriptListener>
  {
  public:
    FSkookumScriptListenerAutoPtr(USkookumScriptListener * delegate_obj_p) : TWeakObjectPtr<USkookumScriptListener>(delegate_obj_p) {}
    ~FSkookumScriptListenerAutoPtr();
  };

//---------------------------------------------------------------------------------------
// Storage specialization
template<> inline FSkookumScriptListenerAutoPtr * SkUserDataBase::as<FSkookumScriptListenerAutoPtr>() const { return as_stored<FSkookumScriptListenerAutoPtr>(); }

//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------

inline bool USkookumScriptListener::has_event() const
  {
  return !m_event_queue.is_empty();
  }

//---------------------------------------------------------------------------------------

inline USkookumScriptListener::EventInfo * USkookumScriptListener::pop_event()
  {
  return m_event_queue.pop_first();
  }
