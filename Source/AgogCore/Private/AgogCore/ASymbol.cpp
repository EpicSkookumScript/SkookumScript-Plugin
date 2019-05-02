// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbol class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/ASymbol.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/ASymbol.inl>
#endif
#include <AgogCore/ASymbolTable.hpp>
#include <string.h>      // Uses:  strlen


//=======================================================================================
// ASymbolRef Method Definitions
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Empty/null symbol reference "" - can be used safely at global init.
//             Also see ASymbol::get_null(), ASymbol::ms_null
// Returns:    Null symbol reference - ""
// Modifiers:   static
// Author(s):   Conan Reis
const ASymbolRef & ASymbolRef::get_null()
  {
  // $Note - CReis Uses Scott Meyers' tip "Make sure that objects are initialized before
  // they're used" from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd
  // Edition] This is instead of using a non-local static object for a singleton.

  static const ASymbolRef s_null_sym_ref(AStringRef::get_empty(), ASymbol_id_null);
  //A_DSINGLETON_GUARD;

  return s_null_sym_ref;
  }

#endif // A_SYMBOLTABLE_CLASSES


//=======================================================================================
// ASymbol Class Data
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)
  ASymbolTable * ASymbol::ms_serialized_syms_p = nullptr;
#endif


//=======================================================================================
// ASymbol Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this symbol and increments the memory address to just past the
//             last byte written.
// Arg         sym - symbol to convert to binary form
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.
// See:        get_symbol(binary_pp), track_serialized()
// Notes:      Every symbol that has this function (as_binary) called on it is also added
//             to the serialization symbol table set in the *main* symbol table (if one is
//             set) to be saved out to a symbol table binary.
//
//             Binary composition:
//               4 bytes - symbol id
// Author(s):   Conan Reis
void ASymbol::as_binary(
  void ** binary_pp
  ) const
  {
  **(uint32_t **)binary_pp = ASYM_MBR_ID;
  (*(uint32_t **)binary_pp)++;

  #if defined(A_SYMBOLTABLE_CLASSES)
    if (ms_serialized_syms_p)
      {
      // Every symbol that is serialized is added to the serialization shared symbol table
      // thus keeping track of only the symbols that are actually used.
      #if defined(A_SYMBOL_STR_DB_AGOG)
        ms_serialized_syms_p->append_shared(*this);
      #else
        if (m_uid != ASymbol_id_null)
          {
          const char * sym_cstr_p = ASYMBOL_ID_TO_CSTR(m_uid);

          // Assumes that the string exists for the life of this serialization symbol table
          ms_serialized_syms_p->symbol_reference(m_uid, sym_cstr_p, uint32_t(::strlen(sym_cstr_p)), ATerm_long);
          }
      #endif
      }
  #endif
  }


#if defined(A_SYMBOL_STR_DB)

//---------------------------------------------------------------------------------------
// Converter to a AString
// Returns:    itself as a AString
// Author(s):   Conan Reis
AString ASymbol::as_string() const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p;
  #else
    return ASYMBOL_ID_TO_STR(m_uid);
  #endif
  }

//---------------------------------------------------------------------------------------
// Converter to a constant C-string pointer
// Returns:    itself as a C-string
// Author(s):   Conan Reis
const char * ASymbol::as_cstr() const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p->m_cstr_p;
  #else
    return ASYMBOL_ID_TO_CSTR(m_uid);
  #endif
  }

//---------------------------------------------------------------------------------------
// Gets length of string translation of itself
// Returns:    length of string version of itself
// Author(s):   Conan Reis
uint32_t ASymbol::get_str_length() const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p->m_length;
  #else
    return ASYMBOL_ID_TO_STR(m_uid).m_str_ref_p->m_length;
  #endif
  }

#endif  // A_SYMBOL_STR_DB


//---------------------------------------------------------------------------------------
// Converter to a AString
// Returns:    itself as a AString
// Author(s):   Conan Reis
AString ASymbol::as_str_dbg() const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p;
  #else
    return ASYMBOL_ID_TO_STR(m_uid);
  #endif
  }

//---------------------------------------------------------------------------------------
// Converter to a constant C-string pointer
// Returns:    itself as a C-string
// Author(s):   Conan Reis
const char * ASymbol::as_cstr_dbg() const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p->m_cstr_p;
  #else
    return ASYMBOL_ID_TO_CSTR(m_uid);
  #endif
  }

//---------------------------------------------------------------------------------------
// Creates a new symbol based on the supplied string.
// Returns:    new symbol
// Arg         str - string to make a symbol from
// Arg         term - lifespan of str: ATerm_long if can just reference str's internal
//             buffer etc. or ATerm_short if a copy should be made.
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create(
  const AString & str,
  eATerm          term // = ATerm_long
  )
  {
  uint32_t sym_id = ASYMBOL_STR_TO_ID(str);

  #if defined(A_SYMBOL_REF_LINK)

    //  If the auto-parse symbol table is defined, make a copy any symbol not already in
    //  in the main table so it can be removed once the auto-parse completes.
    if (ASymbolTable::ms_auto_parse_syms_p)
      {
      ASymbolRef * sym_ref = ASymbolTable::ms_main_p->get_symbol(sym_id);
      if (!sym_ref)
        {
        //A_DPRINT(A_SOURCE_STR "Adding symbol = %ld\n", sym_id);
        ASymbolTable::ms_auto_parse_syms_p->symbol_reference(sym_id, str, term);
        }
      else
        {
        return sym_ref;
        }
      }

    return ASymbolTable::ms_main_p->symbol_reference(sym_id, str, term);
  #elif defined(A_SYMBOL_STR_DB)
    ASYMBOL_STR_STORE(sym_id, str, term);
    return sym_id;
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Creates a new symbol based on the supplied string.
// Returns:    new symbol
// Arg         cstr_p - pointer to array of characters (does not need to be null
//             terminated unless length is equal to ALength_calculate).  'cstr_p' can be
//             be nullptr (though it probably shouldn't).  'cstr_p' will usually be a string
//             literal or if 'term' is ATerm_short, 'cstr_p' may be any C-String that this
//             string could make a copy of.
// Arg         length - number of characters to use in 'cstr_p' and the index position to
//             place a terminating null character.  The given length must not be more
//             than the size of 'cstr_p' and the C-String buffer pointed to by 'cstr_p'
//             should not have any null characters less then the given length.  A null
//             terminator is placed only if 'term' is not ATerm_short.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in 'cstr_p'.
// Arg         term - lifespan of cstr_p: ATerm_long if can just reference cstr_p or
//             ATerm_short if a copy should be made.
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create(
  const char * cstr_p,
  uint32_t     length, // = ALength_calculate
  eATerm       term    // = ATerm_long
  )
  {
  if ((length == 0u) || (cstr_p == nullptr) || (*cstr_p == '\0'))
    {
    return ms_null;
    }

  if (length == ALength_calculate)
    {
    length = uint32_t(::strlen(cstr_p));
    }

  uint32_t sym_id = ASYMBOL_CSTR_TO_ID(cstr_p, length);

  #if defined(A_SYMBOL_REF_LINK)

    //  If the auto-parse symbol table is defined, make a copy any symbol not already in
    //  in the main table so it can be removed once the auto-parse completes.
    if (ASymbolTable::ms_auto_parse_syms_p)
      {
      ASymbolRef * sym_ref = ASymbolTable::ms_main_p->get_symbol(sym_id);
      if (!sym_ref)
        {
        //A_DPRINT(A_SOURCE_STR "Adding symbol = %ld\n", sym_id);
        ASymbolTable::ms_auto_parse_syms_p->symbol_reference(sym_id, cstr_p, length, term);
        }
      else
        {
        return sym_ref;
        }
      }

    return ASymbolTable::ms_main_p->symbol_reference(sym_id, cstr_p, length, term);
  #elif defined(A_SYMBOL_STR_DB)
    ASYMBOL_CSTR_STORE(sym_id, cstr_p, length, term);
    return sym_id;
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Gets the symbol based on the given binary.
//             Uses the *main* symbol table.
// Returns:    a unique symbol or ASymbol::get_null() if not found
// Arg         sym_binary_pp - Pointer to address to read binary symbol serialization
//             info from and to increment - previously filled using as_binary(sym, binary_pp)
//             or a similar mechanism.
// Arg         require_existance - If it is an error in case the id does not exist in the symbol table
// See:        as_binary(), create()
// Notes:      Empty strings "" are synonymous with ASymbol::get_null().
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create_from_binary(const void ** sym_binary_pp, bool require_existance)
  {
  #if defined(A_SYMBOL_REF_LINK)
    #if defined(A_EXTRA_CHECK)
      uint32_t     sym_id    = A_BYTE_STREAM_UI32_INC(sym_binary_pp);
      ASymbolRef * sym_ref_p = ASymbolTable::ms_main_p->get_symbol(sym_id);

      if (sym_ref_p == nullptr)
        {
        A_ASSERTX(!require_existance, a_cstr_format(
          "The symbol id 0x%x loaded from binary could not be found!\n"
          "This may be due to a corrupted binary or mismatch between two different binaries - deleting and regenerating the binaries may solve this problem."
          "[Using null symbol.]",
          sym_id))

        return ms_null;
        }

      return sym_ref_p;
    #else
      return ASymbolTable::ms_main_p->get_symbol(A_BYTE_STREAM_UI32_INC(sym_binary_pp));
    #endif  // A_EXTRA_CHECK
  #else
    return A_BYTE_STREAM_UI32_INC(sym_binary_pp);
  #endif
  }

//---------------------------------------------------------------------------------------
// Gets existing symbol based on the given id - if not found null symbol '' returned.
// Uses the *main* symbol table.
// 
// Returns:    a unique symbol or ASymbol::get_null() if not found
// See:        as_binary(), create()
// Notes:      Empty strings "" are synonymous with ASymbol::get_null().
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create_existing(uint32_t sym_id)
  {
  #if defined(A_SYMBOL_REF_LINK)
    ASymbolRef * sym_ref_p = ASymbolTable::ms_main_p->get_symbol(sym_id);

    if (sym_ref_p == nullptr)
      {
      return ms_null;
      }

    return sym_ref_p;
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Gets existing symbol based on the given id - if not found null symbol '' returned.
// Uses the *main* symbol table.
// 
// Returns:    a unique symbol or ASymbol::get_null() if not found
// See:        as_binary(), create()
// Notes:      Empty strings "" are synonymous with ASymbol::get_null().
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create_existing(const AString & str)
  {
  uint32_t sym_id = ASYMBOL_STR_TO_ID(str);

  #if defined(A_SYMBOL_REF_LINK)
    ASymbolRef * sym_ref_p = ASymbolTable::ms_main_p->get_symbol(sym_id);

    if (sym_ref_p == nullptr)
      {
      return ms_null;
      }

    return sym_ref_p;
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Gets existing symbol based on the given C-string - if not found null symbol '' returned.
// Uses the *main* symbol table.
// 
// Returns:    a unique symbol or ASymbol::get_null() if not found
// See:        as_binary(), create()
// Notes:      Empty strings "" are synonymous with ASymbol::get_null().
// Modifiers:   static
// Author(s):   Conan Reis
ASymbol ASymbol::create_existing(
  const char * cstr_p,
  uint32_t length // = ALength_calculate
  )
  {
  uint32_t sym_id = ASYMBOL_CSTR_TO_ID(cstr_p, length);

  #if defined(A_SYMBOL_REF_LINK)
    ASymbolRef * sym_ref_p = ASymbolTable::ms_main_p->get_symbol(sym_id);

    if (sym_ref_p == nullptr)
      {
      return ms_null;
      }

    return sym_ref_p;
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Creates a new symbol based on a concatenation of this symbol + supplied string.
// Returns:    new symbol
// Arg         suffix - string to append to this symbol
// Author(s):   Conan Reis
ASymbol ASymbol::create_add(
  const AString & suffix
  ) const
  {
  uint32_t root_id = ASYM_MBR_ID;
  uint32_t sym_id  = ASYMBOL_STR_TO_ID_CONCAT(root_id, suffix);

  #if defined(A_SYMBOL_STR_DB)
    #if defined(A_SYMBOL_REF_LINK)
      AString concat_str(m_ref_p->m_str_ref_p);
      concat_str.append(suffix);
      return ASymbolTable::ms_main_p->symbol_reference(sym_id, concat_str, ATerm_long);
    #else
      AString concat_str(as_string(), suffix.get_length());
      concat_str.append(suffix);
      ASYMBOL_STR_STORE(sym_id, concat_str, ATerm_long);

      return sym_id;
    #endif
  #else
    return sym_id;
  #endif
  }

//---------------------------------------------------------------------------------------
// Create a string version of the symbol id - not a translation - just the id as a number
// in the form: |#12345678#|
// 
// Author(s):   Conan Reis
const AString & ASymbol::id_as_str(uint32_t sym_id)
  {
  static AString s_id_str;

  s_id_str.ensure_size_empty(11u);

  // Use the form |#12345678#|
  s_id_str.format("|#%08x#|", sym_id);

  return s_id_str;
  }

//---------------------------------------------------------------------------------------
// Empty/null symbol "" - can be used safely at global initialization.
//             Can use ASymbol::ms_null outside of global initialization for better speed.
// Returns:    Null symbol - ""
// Modifiers:   static
// Author(s):   Conan Reis
const ASymbol & ASymbol::get_null()
  {
  // $Note - CReis Uses Scott Meyers' tip "Make sure that objects are initialized before
  // they're used" from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd
  // Edition] This is instead of using a non-local static object for a singleton.

  #if defined(A_SYMBOL_REF_LINK)
    static const ASymbol s_null_sym(&ASymbolRef::get_null());
  #else
    static const ASymbol s_null_sym;
  #endif
  //A_DSINGLETON_GUARD;

  return s_null_sym;
  }


#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Parses symbol table binary and allows for non-Agog style (ASymbolTable)
//             symbol/string database - modified from AgogCore/ASymbolTable::assign_binary()
// Arg         binary_pp - Pointer to address to read from and increment.
// Notes:      Binary composition:
//               The data in the binary byte stream is in the form:
//               4 bytes - number of symbols
//               4 bytes - symbol id                \
//               1 byte  - length of string *        }-- Repeating in symbol id order
//               n bytes - string (non-terminated)  /
//
//             * Note that the string length is limited to 255 characters which could be
//               insufficient in extreme cases, but it should be unlikely.
// Modifiers:   static
// Author(s):   Conan Reis
void ASymbol::table_from_binary(const void ** binary_pp)
{
  // 4 bytes - number of symbols
  uint32_t sym_count = A_BYTE_STREAM_UI32_INC(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Repeating in symbol id order

  uint32_t sym_id;
  uint32_t str_length;

  while (sym_count)
    {
    // 4 bytes - symbol id
    sym_id = A_BYTE_STREAM_UI32_INC(binary_pp);

    // 1 byte  - length of string
    str_length = A_BYTE_STREAM_UI8_INC(binary_pp);

    // n bytes - string
    ASYMBOL_CSTR_STORE(sym_id, (const char *)*binary_pp, str_length, ATerm_short);
    (*(uint8_t **)binary_pp) += str_length;

    sym_count--;
    }
}


//---------------------------------------------------------------------------------------
// Sets the optional serialization tracking table.  It keeps track of any
//             symbol that is serialized - i.e. whenever as_binary(sym, binary_pp) is
//             called to store a symbol id in a binary.
// Arg         used_sym_p - pointer to symbol table to append a shared version of any
//             serialized symbols or nullptr if no tracking is required.  The table must be
//             set to 'shared' and is usually initially empty, but does not have to be
//             - if it already has some symbols they must be shared from this table.
//             Serialization tracking is disabled if this is nullptr.
// See:        as_binary()
// Notes:      This is useful when saving out a symbol table to binary so that only the
//             symbols that were used need to be saved out.  After all the data using
//             symbols is serialized - using as_binary() whenever a symbol is written to
//             the binary - then call as_datum() on used_syms_p rather than this table and
//             only the symbols used by the serialized data will be stored.
// Author(s):   Conan Reis
void ASymbol::track_serialized(
  ASymbolTable * used_syms_p // = nullptr
  )
  {
  ms_serialized_syms_p = used_syms_p;

  #if defined(A_SYMBOL_STR_DB_AGOG)
    if (used_syms_p)
      {
      used_syms_p->m_sharing = true;
      }
  #endif
  }


#endif  // A_SYMBOLTABLE_CLASSES
