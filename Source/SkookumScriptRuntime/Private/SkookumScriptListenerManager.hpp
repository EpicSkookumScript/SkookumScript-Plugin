// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Factory/manager class for USkookumScriptListener objects
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptListener.h"

#include <AgogCore/APArray.hpp>
#include <SkookumScript/SkInstance.hpp>

//=======================================================================================
// Global Defines / Macros
//=======================================================================================

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Keep track of USkookumScriptListener instances
class SkookumScriptListenerManager
  {
  public:

    static SkookumScriptListenerManager *   get_singleton();

    // Methods

    SkookumScriptListenerManager(uint32_t pool_init, uint32_t pool_incr);
    ~SkookumScriptListenerManager();

    USkookumScriptListener *                alloc_listener(UObject * obj_p, SkInvokedCoroutine * coro_p, USkookumScriptListener::tUnregisterCallback callback_p);
    void                                    free_listener(USkookumScriptListener * listener_p);

    USkookumScriptListener::EventInfo *     alloc_event();
    void                                    free_event(USkookumScriptListener::EventInfo * event_p, uint32_t num_arguments_to_free);

  protected:

    typedef APArray<USkookumScriptListener> tObjPool;
    typedef AObjReusePool<USkookumScriptListener::EventInfo> tEventPool;

    void        grow_inactive_list(uint32_t pool_incr);

    tObjPool    m_inactive_list;
    tObjPool    m_active_list;
    uint32_t    m_pool_incr;

    tEventPool  m_event_pool;

    UPackage *  m_module_package_p;

  }; // SkookumScriptListenerManager

//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------

inline USkookumScriptListener::EventInfo * SkookumScriptListenerManager::alloc_event()
  {
  return m_event_pool.allocate();
  }

//---------------------------------------------------------------------------------------
            
inline void SkookumScriptListenerManager::free_event(USkookumScriptListener::EventInfo * event_p, uint32_t num_arguments_to_free)
  {
  for (uint32_t i = 0; i < num_arguments_to_free; ++i)
    {
    event_p->m_argument_p[i]->dereference();
    }
  m_event_pool.recycle(event_p);
  }

