// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Binding class for objects that are the data themselves
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>

//---------------------------------------------------------------------------------------
// Default class binding for data-less types (e.g. None and Object)
template<class _BindingClass>
class SkClassBindingAbstract
  {
  public:

    // Types

    typedef _BindingClass tDataType; // Default data type is itself

    //---------------------------------------------------------------------------------------
    // Default data is the object itself
    const _BindingClass & get_data() const  { return *static_cast<const _BindingClass *>(this); }
          _BindingClass & get_data()        { return *static_cast<_BindingClass *>(this); }

    // Class methods

    static SkClass *  initialize_class(ASymbol class_name);
    static SkClass *  initialize_class(const char * class_name_p)  { return initialize_class(ASymbol::create_existing(class_name_p)); }
    static SkClass *  initialize_class(uint32_t class_name_id)     { return initialize_class(ASymbol::create_existing(class_name_id)); }

  protected:

    // Class data

    static SkClass * ms_class_p; // Pointer to the SkookumScript class belonging to this binding

  };



//=======================================================================================
// Class Data Definitions
//=======================================================================================

// Pointer to Sk class is stored here
template<class _BindingClass>
SkClass * SkClassBindingAbstract<_BindingClass>::ms_class_p;

//=======================================================================================
// Class Method Implementations
//=======================================================================================

//---------------------------------------------------------------------------------------
// Initialize class information
template<class _BindingClass>
inline SkClass * SkClassBindingAbstract<_BindingClass>::initialize_class(ASymbol class_name)
  {
  // Initialize class information
  SkClass * class_p = SkBrain::get_class(class_name);
  SK_ASSERTX(class_p, a_str_format("Tried to initialize class pointer for '%s' but it is unknown!", class_name.as_cstr_dbg()));
  // Try to recover from class not found situation
  #ifdef SK_RUNTIME_RECOVER
    if (!class_p)
      {
      class_p = SkBrain::ms_object_class_p;
      }
  #endif
  ms_class_p = class_p;
  return class_p;
  }

