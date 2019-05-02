// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Base Object declaration file
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AIdPtr.hpp>
#include <AgogCore/ANamed.hpp>
#include <AgogCore/APArray.hpp>
#include <AgogCore/APSorted.hpp>
#include <SkookumScript/Sk.hpp>

//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class  SkInstance;
class  SkInvokedContextBase;

template<class _ObjectType> class AObjBlock;
template<class _ObjectType> class AObjReusePool;

//---------------------------------------------------------------------------------------
// Used by SkObjectBase::get_obj_type() which returns uint32_t rather than eSkObjectType so
// that the types can be easily extended when custom types are derived.
enum eSkObjectType
  {
  // SkInstance, SkInstanceUnreffed(used by nil)
  SkObjectType_instance,

  // SkActor
  SkObjectType_actor,

  // SkMind
  SkObjectType_mind,

  // SkClosure
  SkObjectType_closure,

  // An invokable instance other than a closure
  SkObjectType_invokable,

  // SkMetaClass
  SkObjectType_meta_class,

  // SkInvokedBase(SkInvokedExpression)
  SkObjectType_invoked_expr,

  // SkInvokedContextBase(SkInvokedMethod, SkInvokedCoroutine)
  SkObjectType_invoked_context,

  // Custom object types should be => SkObjectType__custom_start
  SkObjectType__custom_start
  };


//---------------------------------------------------------------------------------------
// Root object abstract base class for all Skookum "instances".
//
// Notes:
//   AIdPtr<SkObjectBase> can be used as a smart pointer for this class and any of its
//   subclasses.
//
// Subclasses:
//   SkInstance(SkBoolean, SkClosure, SkDataInstance(SkActor),
//   SkInstanceUnreffed(SkMetaClass)), SkInvokedBase(SkInvokedExpression,
//   SkInvokedContextBase(SkInvokedMethod, SkInvokedCoroutine))
//
// #Author(s) Conan Reis
class SK_API SkObjectBase
  {
  public:

    // Public Data Members

      // Pointer id of the object that must match the id in any AIdPtr<SkObjectBase> for it
	  // to be valid.
      uint32_t m_ptr_id;

    // Methods
     
      virtual ~SkObjectBase() {}

      void renew_id()                                  { ms_ptr_id_prev = m_ptr_id = ms_ptr_id_prev + 1; }
      bool is_valid_id() const                         { return m_ptr_id && (m_ptr_id <= ms_ptr_id_prev); }

      virtual eSkObjectType get_obj_type() const = 0;
      virtual bool          is_mind() const            { return false; }

    // Data methods

      virtual SkInstance *           as_new_instance() const;
      virtual SkInvokedContextBase * get_scope_context() const;
      virtual SkInstance *           get_topmost_scope() const = 0;

    // Class Data

      static uint32_t ms_ptr_id_prev;

  };  // SkObjectBase


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkObjectBase.inl>
#endif
