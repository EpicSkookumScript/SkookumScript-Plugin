// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Named object class inline file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AString.hpp>


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Default constructor / ASymbol converter
// Returns:     itself
// Arg          name - name of the object
// Modifiers:    explicit
// Author(s):    Conan Reis
A_INLINE ANamed::ANamed(
  const ASymbol & name // = ASymbol::get_null()
  ) :
  m_name(name)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Arg          source - named to copy
// Author(s):    Conan Reis
A_INLINE ANamed::ANamed(const ANamed & source) :
  m_name(source.m_name)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow for stringization (named1 = named2 = named3)
// Arg          source - named to copy
// Author(s):    Conan Reis
A_INLINE ANamed & ANamed::operator=(const ANamed & source)
  {
  m_name = source.m_name;
  return *this;
  }

//---------------------------------------------------------------------------------------
//  Converts to a symbol representation of itself - its name
// Returns:     a symbol representation of itself - its name
// Author(s):    Conan Reis
A_INLINE ANamed::operator const ASymbol & () const
  {
  return m_name;
  }

//---------------------------------------------------------------------------------------
//  Equal to operator
// Returns:     true if equal, false if not
// Arg          named - named to compare
// Author(s):    Conan Reis
A_INLINE bool ANamed::operator==(const ANamed & named) const
  {
  return m_name == named.m_name;
  }

//---------------------------------------------------------------------------------------
//  Less than operator
// Returns:     true if less than, false if not
// Arg          named - named to compare
// Author(s):    Conan Reis
A_INLINE bool ANamed::operator<(const ANamed & named) const
  {
  return m_name < named.m_name;
  }

//---------------------------------------------------------------------------------------
//  Sets the name of this object
// Arg          name - name of the object
// See:         get_name()
// Author(s):    Conan Reis
A_INLINE void ANamed::set_name(const ASymbol & name)
  {
  m_name = name;
  }


#if defined(A_SYMBOL_STR_DB)

//---------------------------------------------------------------------------------------
//  Gets the string name of this object - see ASymbol::as_string()
// Returns:     The string version of the name of this object
// See:         get_name(), set_name()
// Author(s):    Conan Reis
A_INLINE AString ANamed::get_name_str() const
  {
  return m_name.as_string();
  }

//---------------------------------------------------------------------------------------
//  Gets the string name of this object - see ASymbol::as_cstr()
// Returns:     The string version of the name of this object
// See:         get_name(), set_name()
// Author(s):    Conan Reis
A_INLINE const char * ANamed::get_name_cstr() const
  {
  return m_name.as_cstr();
  }

#endif  // A_SYMBOL_STR_DB


//---------------------------------------------------------------------------------------
//  Gets the string name of this object - see ASymbol::as_str_dbg()
// Returns:     The string version of the name of this object
// See:         get_name(), set_name()
// Author(s):    Conan Reis
A_INLINE AString ANamed::get_name_str_dbg() const
  {
  return m_name.as_str_dbg();
  }

//---------------------------------------------------------------------------------------
//  Gets the string name of this object - see ASymbol::as_cstr_dbg()
// Returns:     The string version of the name of this object
// See:         get_name(), set_name()
// Author(s):    Conan Reis
A_INLINE const char * ANamed::get_name_cstr_dbg() const
  {
  return m_name.as_cstr_dbg();
  }

