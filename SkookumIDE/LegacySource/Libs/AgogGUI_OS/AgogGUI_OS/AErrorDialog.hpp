// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AErrorDialog class declaration header
//=======================================================================================


#ifndef __AERRORDIALOG_HPP
#define __AERRORDIALOG_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\AgogCore.hpp>
#include <AgogCore\ADebug.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    Creates a pop-up error box.
// UsesLibs AgogCore\AgogCore.lib User32.lib   
// Inlibs   AgogIO\AgogIO.lib
// See Also AErrPopUp
// Examples:    
// Author   Conan Reis
class AErrorDialog : public AErrorOutputBase
  {
  public:

  // Modifying Methods

  virtual bool determine_choice(const AErrMsg & info, eAErrAction * action_p);

  };


//=======================================================================================
// Inline Functions
//=======================================================================================


#endif // __AERRORDIALOG_HPP


