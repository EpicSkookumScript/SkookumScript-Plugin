// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Base Object definition file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkObjectBase.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkObjectBase.inl>
#endif
#include <AgogCore/APSorted.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkClass.hpp>



//=======================================================================================
// SkObjectBase Class Data
//=======================================================================================

uint32_t SkObjectBase::ms_ptr_id_prev = 0u;


//=======================================================================================
// SkObjectBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Returns itself as an instance
// Modifiers:   virtual
// Author(s):   Conan Reis
SkInstance * SkObjectBase::as_new_instance() const
  {
  return SkBrain::ms_nil_p;
  }

//---------------------------------------------------------------------------------------
// Returns the topmost context (SkInvokedMethod or SkInvokedCoroutine)
// Returns:    SkInvokedMethod, SkInvokedCoroutine or nullptr
// Notes:      It is useful when a data retrieval error occurs to determine which
//             topmost coroutine / method had the error.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkInvokedContextBase * SkObjectBase::get_scope_context() const
  {
  return nullptr;
  }
