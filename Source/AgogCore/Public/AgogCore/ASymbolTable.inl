// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbolTable class declaration header
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AChecksum.hpp>


//=======================================================================================
// Inline Functions
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Determines if the specified symbol id is currently registered in the table.
//             The null symbol "" is always considered to be registered.
// Author(s):   Conan Reis
A_INLINE bool ASymbolTable::is_registered(uint32_t sym_id) const
  {
  return (sym_id == ASymbol_id_null) || m_sym_refs.find(sym_id);
  }

//---------------------------------------------------------------------------------------
// Looks up symbol reference by symbol id and returns it if found and nullptr if
//             not.
// Returns:    ASymbolRef with matching id or nullptr
// Arg         id - symbol id to lookup
// Author(s):   Conan Reis
A_INLINE ASymbolRef * ASymbolTable::get_symbol(uint32_t id) const
  {
  return (id != ASymbol_id_null)
    ? m_sym_refs.get(id)
  #if defined(A_SYMBOL_REF_LINK)
    : ASymbol::ms_null.m_ref_p;
  #else
    : const_cast<ASymbolRef *>(&ASymbolRef::get_null());
  #endif
  }


//---------------------------------------------------------------------------------------
//  Returns a symbol ref at the given index, null if table empty or index out-of-range.
//  
//  Returns:    ASymbolRef or null symbol ref.
//  Arg         index - symbol ref index to return
//  Author(s):  John Stenersen
A_INLINE ASymbolRef * ASymbolTable::get_symbol_by_index(uint32_t index) const
  {
  if (index + 1 > m_sym_refs.get_length())
    {
  #if defined(A_SYMBOL_REF_LINK)
    return ASymbol::ms_null.m_ref_p;
  #else
    return const_cast<ASymbolRef *>(&ASymbolRef::get_null());
  #endif
    }

  return m_sym_refs.get_at(index);
  }


#endif // A_SYMBOLTABLE_CLASSES
