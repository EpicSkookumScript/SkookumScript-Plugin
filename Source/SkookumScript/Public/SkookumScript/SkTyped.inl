// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Typed name and typed data classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================


//=======================================================================================
// SkTypedName Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         name - name of the object (Default ASymbol::ms_null)
// Arg         type_p - class type to use.  The instance stored by this member must be of
//             this class or a subclass.
// Notes:      A SkTypedName may be coerced to or from a ASymbol if only the name is needed.
// Author(s):   Conan Reis
A_INLINE SkTypedName::SkTypedName(
  const ASymbol &         name,
  const SkClassDescBase * type_p
  ) :
  ANamed(name),
  m_type_p(const_cast<SkClassDescBase *>(type_p))
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Returns:    itself
// Arg         source - Typed name to copy
// Notes:      May be coerced to or from a ASymbol if only the name is needed.
// Author(s):   Conan Reis
A_INLINE SkTypedName::SkTypedName(const SkTypedName & source) :
  ANamed(source),
  m_type_p(source.m_type_p)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
A_INLINE SkTypedName::~SkTypedName()
  {
  // Defined here to ensure compiler knows about SkClass details
  }

