// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE hook-ins
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE\SkookumIDE.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Author(s):   Conan Reis
SkookumIDE::SkookumIDE(
  SkConsole::eCloseAction close_action // = SkConsole::CloseAction_shutdown
  ) :
  m_console_p(new SkConsole(SkCompiler::Init_phased, close_action))
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
// 
// Author(s):   Conan Reis
SkookumIDE::~SkookumIDE()
  {
  shutdown();
  }

//---------------------------------------------------------------------------------------
// Shuts down IDE
// 
// Author(s):   Conan Reis
void SkookumIDE::shutdown()
  {
  if (m_console_p)
    {
    delete m_console_p;
    m_console_p = nullptr;
    }
  }

