// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Named object class declaration file
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ASymbol.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      Named object class
// See Also   ASymbol
// Author(s)  Conan Reis
class A_API ANamed
  {
  public:

    // Common Methods

    explicit ANamed(const ASymbol & name = ASymbol::get_null());
    ANamed(const ANamed & source);
    ANamed & operator=(const ANamed & source);

    // Converter Methods

    operator const ASymbol & () const;

    // Comparison Methods - used for sorting esp. in arrays like APSorted

    bool operator==(const ANamed & named) const;
    bool operator<(const ANamed & named) const;

    // Accessor Methods

    ASymbol &       get_name()                      { return m_name; }
    const ASymbol & get_name() const                { return m_name; }
    uint32_t        get_name_id() const             { return m_name.get_id(); }
    void            set_name(const ASymbol & name);

    #if defined(A_SYMBOL_STR_DB)
      AString       get_name_str() const;
      const char *  get_name_cstr() const;
    #endif

    AString         get_name_str_dbg() const;
    const char *    get_name_cstr_dbg() const;

  protected:

    // Data Members

    ASymbol m_name;

  };  // ANamed


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/ANamed.inl>
#endif
