// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AException class definition header
//              Contains: AException, and AEx<>
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AExceptionBase.hpp>
#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Errors for small classes where creating new error classes is overkill.  Any
//            time an exception is thrown there should be enough memory for a few smallish
//            objects to be created.
// Subclasses AEx<>
// See Also   AErrorOutputBase, ADebug
// UsesLibs   
// InLibs     AgogCore/AgogCore.lib
// Examples:    
// Author(s)  Conan Reis
class A_API AException : public AExceptionBase
  {
  public:
  // Common Methods

    AException(uint32_t err_id = AErrId_unknown, eAErrAction action = AErrAction_continue);
    AException(const AException & source);

  // Modifying Methods

    virtual eAErrAction resolve() override;

  protected:
  // Data Members

    // Numeric id for programmatic identification of the specific exception - usually
    // from an enumerated type
    uint32_t m_err_id;

    // Suggested course of action to resolve this error
    eAErrAction m_action;

  };  // AException


//---------------------------------------------------------------------------------------
// Notes      Allows the creation of exceptions specific to a particular class - which
//            creates the possibility of catching exceptions that are specific to a
//            particular class.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AEx<AString>;
// Arg        _ExClass - Class / module
// Subclasses 
// See Also   AException, AErrorOutputBase, ADebug
// UsesLibs     
// InLibs     AgogCore/AgogCore.lib
// Examples:      
// Author(s)  Conan Reis
template<class _ExClass>
class AEx : public AException
  {
  public:
  // Common Methods

    AEx(uint32_t err_id, eAErrAction action = AErrAction_continue);
    AEx(const AEx<_ExClass> & source);

  };  // AEx<>


//=======================================================================================
// AException Inline Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          source - AException object to copy
// Author(s):    Conan Reis
inline AException::AException(const AException & source) :
  m_err_id(source.m_err_id),
  m_action(source.m_action)
  {
  }


//=======================================================================================
// AEx<> Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Constructor for simple error which displays a given message and the file
//              name and line where the exception is thrown.  If an exception level is
//              not given, it defaults to an informational exception.
// Returns:     itself
// Arg          err_id - Numeric id for programmatic identification of the specific
//              exception - usually from an enumerated type or the address of the
//              excepting function
// Arg          action - Suggested course of action to resolve this error.  See available
//              choices and there descriptions at eAErrAction.
// Examples:    throw AException("The move event has unexpected arguments.", on_move, EX_SOURCE);
// See:         AEx<>, eAErrLevel, AErrorOutputBase
// Author(s):    Conan Reis
template<class _ExClass>
inline AEx<_ExClass>::AEx(
  uint32_t    err_id,
  eAErrAction action // = AErrAction_continue
  ) :
  AException(err_id, action)
  {
  // Use the break in the AException constructor
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          source - AException object to copy
// Author(s):    Conan Reis
template<class _ExClass>
inline AEx<_ExClass>::AEx(const AEx<_ExClass> & source) :
  AException(source)
  {
  }
