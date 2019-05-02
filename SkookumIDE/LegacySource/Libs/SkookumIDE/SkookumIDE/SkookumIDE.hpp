// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE hook-ins
//=======================================================================================


#ifndef __SKOOKUMIDE_HPP
#define __SKOOKUMIDE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE\SkConsole.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Wrapper for SkConsole - just instantiate it and you've got a scripting IDE.
// Alternatively, SkConsole can be used directly.
class SkookumIDE
  {
  public:

  // Public Data Members

    SkConsole * m_console_p;


  // Methods

    SkookumIDE(SkConsole::eCloseAction close_action = SkConsole::CloseAction_shutdown);
    ~SkookumIDE();

    void shutdown();

  };  // SkookumIDE



#endif  // __SKOOKUMIDE_HPP

