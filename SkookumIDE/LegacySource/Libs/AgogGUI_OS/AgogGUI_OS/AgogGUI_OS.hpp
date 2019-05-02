// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Components common to the AgogGUI_OS library
//=======================================================================================


#ifndef __AGOGGUI_OS_HPP
#define __AGOGGUI_OS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO/AWinTypes.hpp>


//=======================================================================================
// Global Functions
//=======================================================================================

namespace AgogGUI_OS
  {

  void initialize();
  void deinitialize();

  void initialize_com();
  void deinitialize_com();

  void initialize_non_agog(HINSTANCE app_instance, const char * cmd_line_p = "", HINSTANCE res_instance = 0u);

  }


#endif // __AGOG2D_OS_HPP


