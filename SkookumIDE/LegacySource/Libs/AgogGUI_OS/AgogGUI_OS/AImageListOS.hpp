// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AImageListOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __AIMAGELISTOS_HPP
#define __AIMAGELISTOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore\AgogCore.hpp>


//=======================================================================================
// Defines / Constants
//=======================================================================================

// This is designed to mimic the HIMAGELIST defined in the windows header if it is not included
#ifndef HIMAGELIST
  struct _IMAGELIST;
  typedef struct _IMAGELIST * HIMAGELIST;
#endif


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      An AImageListOS is a built-in list control/manager from the operating system.
// Subclasses 
// See Also   
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, ComCtl32.lib
// Inlibs     AgogGUI_OS\AgogGUI_OS.lib
// Examples:      
// Author(s)  Conan Reis
class AImageListOS
  {
  public:
  // Common Methods

    AImageListOS(HIMAGELIST handle = nullptr)    : m_handle(handle) {}
    AImageListOS(const AImageListOS & source) : m_handle(source.m_handle) {}
    ~AImageListOS();
    
    AImageListOS & operator=(const AImageListOS & source) { m_handle = source.m_handle; return *this; }

  // Methods
    
    void       set_handle(HIMAGELIST handle)              { destroy(); m_handle = handle; }
    HIMAGELIST get_handle() const                         { return m_handle; }
    bool       is_initialized() const                     { return m_handle != nullptr; }

    void destroy();

  protected:

  // Data Members

    HIMAGELIST m_handle;

  };  // AImageListOS


#endif  // __AIMAGELISTOS_HPP


