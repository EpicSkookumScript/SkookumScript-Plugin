// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AFunction class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/AFunction.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Adds copy_new()
AFUNC_COPY_NEW_DECL(AFunction, AFunctionBase)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Invokes the stored function.
// Examples:   func.invoke();
// Notes:      overridden from AFunctionBase
// Modifiers:   virtual
// Author(s):   Conan Reis
void AFunction::invoke()
  {
  if (m_function_f)
    {
    (m_function_f)();
    }
  }

//---------------------------------------------------------------------------------------

bool AFunction::operator == (void (*function_f)()) const
  {
  return m_function_f == function_f;
  }

