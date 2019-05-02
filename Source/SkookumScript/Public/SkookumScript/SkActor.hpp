// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Actor class - i.e. named simulation objects with a list of all instances stored in
// their classes
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkDataInstance.hpp>
#include <SkookumScript/SkClassBindingAbstract.hpp>

//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkActorClass;

//---------------------------------------------------------------------------------------
// Named simulation object with a list of all instances stored in their classes (SkActorClass).
// 
// $Revisit - CReis Some actors may not need data members so there could be a version of
// `SkActor` that derives from `SkInstance` rather than `SkDataInstance`.
class SK_API SkActor :
  public SkClassBindingAbstract<SkActor>, // Adds static class pointer
  public ANamed,         // Adds name
  public SkDataInstance  // Adds instance info and data members.
  {
  public:

  // Common Methods

    SK_NEW_OPERATORS(SkActor);

    SkActor(const ASymbol & name = ASymbol::get_null(), SkActorClass * class_p = nullptr, bool add_to_instance_list = true);
    virtual ~SkActor() override;

  // Accessor Methods


  // Methods

    AString        as_string() const;
    SkActorClass * get_class_actor() const                         { return reinterpret_cast<SkActorClass *>(m_class_p); }
    void           rename(const ASymbol & name);

    // Overriding from SkInstance -> SkDataInstance

      virtual void delete_this() override;

    // Overriding from SkInstance

       #if defined(SK_AS_STRINGS)
         virtual AString         as_string_debug() const override  { return as_string(); }
         virtual const ASymbol & get_name_debug() const override   { return m_name; }
       #endif

    // Overriding from SkObjectBase

      virtual eSkObjectType get_obj_type() const override               { return SkObjectType_actor; } 

 // Class Methods

    static AString generate_unique_name_str(const AString & name_root, uint32_t * create_idx_p = nullptr);
    static ASymbol generate_unique_name_sym(const AString & name_root, uint32_t * create_idx_p = nullptr);


 // SkookumScript Atomics

    static void register_bindings();

  };  // SkActor


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkActor.inl>
#endif
