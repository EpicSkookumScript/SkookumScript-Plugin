// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbolTable class definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/ASymbolTable.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/ASymbolTable.inl>
#endif
#include <AgogCore/AStringRef.hpp>
#include <AgogCore/AString.hpp>


//=======================================================================================
// Class Data
//=======================================================================================

// ASymbolTable::ms_main_p  - defined in AgogCore.cpp


//=======================================================================================
// Method Definitions
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)

ASymbolTable * ASymbolTable::ms_auto_parse_syms_p = nullptr;


//---------------------------------------------------------------------------------------

void ASymbolTable::initialize()
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    ms_main_p = new ASymbolTable(false, AgogCore::get_app_info()->get_pool_init_symbol_ref());
  #endif
  }

//---------------------------------------------------------------------------------------

void ASymbolTable::deinitialize()
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    delete ms_main_p;
  #endif
  }

//---------------------------------------------------------------------------------------

bool ASymbolTable::is_initialized()
  {
  #if defined(A_SYMBOLTABLE_CLASSES)
    return ms_main_p != nullptr;
  #else
    return true;
  #endif
  }

//---------------------------------------------------------------------------------------
// Default constructor
// Arg         sharing_symbols - indicates whether or not the symbol table is sharing
//             ASymbol objects with another ASymbolTable.
// Returns:    itself
// Author(s):   Conan Reis
ASymbolTable::ASymbolTable(
  bool sharing_symbols, // = false
  uint32_t initial_size     // = 0
  ) :
  m_sym_refs((const ASymbolRef **)nullptr, 0u, initial_size, true),
  m_sharing(sharing_symbols)
  {
  // This ensures that the symbol reference pool is allocated and that it is feed *after*
  // the destructor of this symbol table.
  ASymbolRef::get_pool();
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
ASymbolTable::~ASymbolTable()
  {
  empty();
  }

//---------------------------------------------------------------------------------------
// Empties the symbol table - setting its length to 0.  If the symbol table
//             is not sharing symbols, it deallocates all the symbols in itself.
// Examples:   sym_table.empty();
// Notes:      Ensure that none of the Symbols contained in this table are being pointed
//             to anywhere else in the application before calling this method.
// Author(s):   Conan Reis
void ASymbolTable::empty()
  {
  uint32_t length = m_sym_refs.get_length();

  if (length)
    {
    if (!m_sharing)
      {
      ASymbolRef ** syms_pp     = m_sym_refs.get_array();  // for faster than class member access
      ASymbolRef ** syms_end_pp = syms_pp + length;

      for (; syms_pp < syms_end_pp; syms_pp++)
        {
        ASymbolRef::pool_delete(*syms_pp);
        }
      }

    m_sym_refs.empty();
    }
  }

//---------------------------------------------------------------------------------------
// Determines if the symbol order and symbol ids are valid.
// Notes:      This is a test function that should hopefully never need to be called.
//             It was written to discover a potential memory stomp bug.
// Author(s):   Conan Reis
void ASymbolTable::validate() const
  {
  uint32_t length = m_sym_refs.get_length();

  if (length)
    {
    ASymbolRef ** syms_pp     = m_sym_refs.get_array();  // for faster than class member access
    ASymbolRef ** syms_end_pp = syms_pp + length;

    uint32_t     id;
    uint32_t     sym_id;
    ASymbolRef * sym_p;
    ASymbolRef * prev_sym_p = nullptr;

    for (; syms_pp < syms_end_pp; syms_pp++)
      {
      sym_p  = *syms_pp;
      id     = ASYMBOL_CSTR_TO_ID(sym_p->m_str_ref_p->m_cstr_p, sym_p->m_str_ref_p->m_length);
	  sym_id = sym_p->m_uid;

	  A_VERIFYX(
        id == sym_id,
		a_cstr_format(
          "Stored symbol '%s'#%u should have id #%u!",
          sym_p->m_str_ref_p->m_cstr_p, sym_id, id));

      if (prev_sym_p)
        {
	    A_VERIFYX(
          (prev_sym_p->m_uid < sym_id),
		  a_cstr_format(
            "Symbol ids '%s'#%u and '%s'#%u are not in proper sequence!",
            prev_sym_p->m_str_ref_p->m_cstr_p, prev_sym_p->m_uid, sym_p->m_str_ref_p->m_cstr_p, sym_id));
        }

      prev_sym_p = sym_p;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this symbol table and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length(), merge_binary()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               The data in the binary byte stream is in the form:
//               4 bytes - number of symbols
//               4 bytes - symbol id          \
//               1 byte  - length of string *  }-- Repeating in symbol id order
//               n bytes - string             /
//
//             * Note that the string length is limited to 255 characters which could be
//               insufficient in extreme cases, but it should be unlikely.
//
// Author(s):   Conan Reis
void ASymbolTable::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, as_binary_length());

  // $Revisit - CReis This could be written so that the strings are saved off in their own
  // single contiguous chunk which could be loaded and referenced persistently on load.
  // Before this is done - a large number of symbols already populate the symbol table and
  // those strings would be wasted space - ensure symbols defined in code are added after
  // initial load or change them so that they don't need a string or they aren't saved to
  // the binary.

  uint32_t length = m_sym_refs.get_length();

  // 4 bytes - number of symbols
  A_BYTE_STREAM_OUT32(binary_pp, &length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Repeating in symbol id order

  uint8_t           str_len;
  ASymbolRef *  sym_ref_p;
  AStringRef *  str_ref_p;
  ASymbolRef ** syms_pp     = m_sym_refs.get_array(); 
  ASymbolRef ** syms_end_pp = syms_pp + length;

  for (; syms_pp < syms_end_pp; syms_pp++)
    {
    sym_ref_p = *syms_pp;

    // 4 bytes - symbol id
    A_BYTE_STREAM_OUT32(binary_pp, &sym_ref_p->m_uid);

    str_ref_p = sym_ref_p->m_str_ref_p;

    // 1 byte  - length of string
    length  = str_ref_p->m_length;
    str_len = uint8_t(length);
    A_BYTE_STREAM_OUT8(binary_pp, &str_len);

    // n bytes - string
    ::memcpy(*binary_pp, str_ref_p->m_cstr_p, length);
    (*(uint8_t **)binary_pp) += length;
    }
  }

//---------------------------------------------------------------------------------------
// Get byte sized needed for binary memory stream of this symbol table.
//             Used to allocate enough memory for as_binary()
//             [See as_binary() for byte stream composition.]
// See:        as_binary()
// Author(s):   Conan Reis
uint32_t ASymbolTable::as_binary_length() const
  {
  uint32_t      length        = m_sym_refs.get_length();
  uint32_t      binary_length = 4u + (5u * length); // symbol_length(4) + symbol_length * (sym_id(4) + string_length(1))
  ASymbolRef ** syms_pp       = m_sym_refs.get_array(); 
  ASymbolRef ** syms_end_pp   = syms_pp + length;

  // Determine total data length
  for (; syms_pp < syms_end_pp; syms_pp++)
    {
    binary_length += (*syms_pp)->m_str_ref_p->m_length;
    }

  return binary_length;
  }

//---------------------------------------------------------------------------------------
// Empties this symbol table and adds symbols described by binary byte stream.
//             [See as_binary() for byte stream composition.]
// Arg         binary_pp - Pointer to address to read from and increment.
// See:        as_binary(), merge_binary(), ASymbol::table_from_binary()
// Author(s):   Conan Reis
void ASymbolTable::assign_binary(const void ** binary_pp)
  {
  empty();

  // 4 bytes - number of symbols
  uint32_t length = A_BYTE_STREAM_UI32_INC(binary_pp);

  m_sym_refs.ensure_size_empty(length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Repeating in symbol id order

  uint32_t sym_id;
  uint32_t str_len;

  ASymbolRef ** syms_pp     = m_sym_refs.get_array(); 
  ASymbolRef ** syms_end_pp = syms_pp + length;

  // Determine total data length
  for (; syms_pp < syms_end_pp; syms_pp++)
    {
    // 4 bytes - symbol id
    sym_id = A_BYTE_STREAM_UI32_INC(binary_pp);

    // 1 byte  - length of string
    str_len = A_BYTE_STREAM_UI8_INC(binary_pp);

    // n bytes - string
    *syms_pp = ASymbolRef::pool_new(
      AStringRef::pool_new_copy((const char *)*binary_pp, str_len),
      sym_id);
    (*(uint8_t **)binary_pp) += str_len;
    }
  }

//---------------------------------------------------------------------------------------
// Merges symbols described by binary byte stream into existing symbol table.
//             [See as_binary() for byte stream composition.]
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary(), assign_binary(), ASymbol::table_from_binary()
// Author(s):   Conan Reis
void ASymbolTable::merge_binary(const void ** binary_pp)
  {
  uint32_t init_length = m_sym_refs.get_length();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If table is empty, don't do any special merge code
  if (init_length == 0u)
    {
    assign_binary(binary_pp);

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Merge in symbols from binary

  // 4 bytes - number of symbols
  uint32_t length = A_BYTE_STREAM_UI32_INC(binary_pp);

  // Assume that there will be no overlap
  m_sym_refs.ensure_size(init_length + length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Repeating in symbol id order

  uint32_t sym_id;
  uint32_t str_len;

  while (length)
    {
    // 4 bytes - symbol id
    sym_id = A_BYTE_STREAM_UI32_INC(binary_pp);

    // 1 byte  - length of string
    str_len = A_BYTE_STREAM_UI8_INC(binary_pp);

    // n bytes - string
    symbol_reference(sym_id, (const char *)*binary_pp, str_len, ATerm_short);
    (*(uint8_t **)binary_pp) += str_len;

    length--;
    }
  }


#if defined(A_SYMBOL_REF_COUNT)

//---------------------------------------------------------------------------------------
// Removes any unreferenced symbols.
// See:        print_unreferenced()
// Author(s):   Conan Reis
void ASymbolTable::remove_unreferenced()
  {
  uint32_t length = m_sym_refs.get_length();

  if (length)
    {
    uint32_t      remomve_count = 0u;
    ASymbolRef ** ref_syms_pp   = m_sym_refs.get_array();  // for faster than class member access
    ASymbolRef ** syms_pp       = ref_syms_pp;
    ASymbolRef ** syms_end_pp   = syms_pp + length;

    for (; syms_pp < syms_end_pp; syms_pp++)
      {
	  if ((*syms_pp)->m_ref_count == 0u)
        {
        ASymbolRef::pool_delete(*syms_pp);
        remomve_count++;
        }
      else
        {
        *ref_syms_pp = *syms_pp;
        ref_syms_pp++;
        }
      }

    if (remomve_count)
      {
      m_sym_refs.remove_all_last(remomve_count);
      }
    }
  }

#endif // A_SYMBOL_REF_COUNT


#if defined(A_SYMBOL_STR_DB_AGOG)

//---------------------------------------------------------------------------------------
// Appends a symbol from another symbol table.  If the symbol was already
//             present in the table, no action is performed.
// Arg         shared_symbol - symbol previously created from another symbol table
// Examples:   table.append_shared(sym);
// See:        track_serialized()
// Notes:      This method is useful when creating a sub-table of an existing table -
//             for example when writing out a binary file using sort ids, this method
//             can make a local sub-table of the symbols written out to the binary file.
//             The translate() method can convert a local sort id to a symbol from a
//             global symbol table.
//             
//             This method ensures that if it is called more than once with the same
//             symbol, that that symbol is only appended once.
//             
//             This method assumes that the symbol table is in share symbol mode - which
//             is set on the symbol table's constructor.
// Author(s):   Conan Reis
void ASymbolTable::append_shared(const ASymbol & shared_symbol)
  {
  uint32_t sym_id = shared_symbol.ASYM_MBR_ID;

  if (sym_id != ASymbol_id_null)
    {
    #if defined(A_SYMBOL_REF_LINK)
      m_sym_refs.append_absent(*shared_symbol.m_ref_p);
    #else
      uint32_t insert_pos;
      
      if (!m_sym_refs.find(sym_id, AMatch_first_found, &insert_pos))
        {
        // Assuming symbol exists in main table.
        m_sym_refs.insert(*ms_main_p->m_sym_refs.get(sym_id), insert_pos);
        }
    #endif
    }
  }

#endif


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
ASymbol ASymbolTable::translate_str(const AString & str) const
  {
  if (str.is_filled())
    {
    uint32_t     sym_id    = ASYMBOL_STR_TO_ID(str);
    ASymbolRef * sym_ref_p = m_sym_refs.get(sym_id);

    if (sym_ref_p)
      {
      #if defined(A_SYMBOL_REF_LINK)
        return sym_ref_p;
      #else
        return sym_id;
      #endif
      }
    }

  return ASymbol::ms_null;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AString ASymbolTable::translate_id(uint32_t sym_id) const
  {
  if (sym_id == ASymbol_id_null)
    {
    return AString::ms_empty;
    }

  ASymbolRef * sym_ref_p = m_sym_refs.get(sym_id);

  if (sym_ref_p)
    {
    return sym_ref_p->m_str_ref_p;
    }

  // Make a numerical string version of the id so that there is at least some info.
  return ASymbol::id_as_str(sym_id);
  }

//---------------------------------------------------------------------------------------
// Translates supplied symbol id into a string if the id is known - i.e. if
//             it is currently present in this table.
// Returns:    true if converted false if not
// Arg         sym_id - symbol id to attempt to convert
// Arg         str_p - address to string to store translation.  Unchanged if id not found.
// See:        translate_id(), translate_str(), translate_ids(), is_registered()
// Author(s):   Conan Reis
bool ASymbolTable::translate_known_id(
  uint32_t  sym_id,
  AString * str_p
  ) const
  {
  if (sym_id == ASymbol_id_null)
    {
    str_p->empty();

    return true;
    }

  ASymbolRef * sym_ref_p = m_sym_refs.get(sym_id);

  if (sym_ref_p)
    {
    // $Revisit - CReis Would use a AString::=(AStringRef *) but not currently written
    sym_ref_p->m_str_ref_p->m_ref_count++;
    str_p->m_str_ref_p->dereference();
    str_p->m_str_ref_p = sym_ref_p->m_str_ref_p;

    return true;
    }

  // Did not find symbol in DB
  return false;
  }

//---------------------------------------------------------------------------------------
// Converts any occurrences of symbol ids in the form |#12345678#| to their string
// equivalents.
// 
// Returns:  
//   `true` if completely converted and `false` if there were some ids that the string
//   equivalent could not be found for.
//   
// Params:  
//   str_p: pointer to string to convert
//   
// Author(s):   Conan Reis
bool ASymbolTable::translate_ids(AString * str_p) const
  {
  uint32_t length = str_p->get_length();

  const uint32_t ASymbol_id_str_length = 12u;

  if (length < ASymbol_id_str_length)
    {
    return true;
    }

  bool     sym_replaced;
  bool     translated = true;
  uint32_t sym_id;
  uint32_t str_length;
  uint32_t find_idx;
  uint32_t end_idx;
  uint32_t start_idx = 0u;
  uint32_t max_idx   = length - ASymbol_id_str_length;

  AString sym_str;

  const char * cstr_p = str_p->as_cstr();

  // If found special |#12345678#| pattern
  while ((start_idx <= max_idx)
    && str_p->find('|', 1u, &find_idx, start_idx)
    && (find_idx <= max_idx)
    && (cstr_p[find_idx + 1u] == '#')
    && (cstr_p[find_idx + ASymbol_id_str_length - 2u] == '#')
    && (cstr_p[find_idx + ASymbol_id_str_length - 1u] == '|'))
    {
    sym_replaced = false;

    // $Revisit - CReis Ensure that ids with leading zeros 00123 are converted properly
    sym_id = str_p->as_uint(find_idx + 1u, &end_idx, 16u);

    if (end_idx == find_idx + ASymbol_id_str_length - 1u)
      {
      if (sym_id != ASymbol_id_null)
        {
        if (translate_known_id(sym_id, &sym_str))
          {
          // Replace symbol id with corresponding string
          str_p->replace(sym_str, find_idx, ASymbol_id_str_length);
          str_length   = sym_str.get_length();
          max_idx      = max_idx + str_length - ASymbol_id_str_length;
          start_idx    = find_idx + str_length;
          sym_replaced = true;
          }
        else
          {
          // Unable to translate all of supplied string
          translated = false;
          }
        }
      else
        {
        // It is the empty symbol "" - remove symbol id
        str_p->remove_all(find_idx, ASymbol_id_str_length);
        max_idx -= ASymbol_id_str_length;
        start_idx = find_idx;
        sym_replaced = true;
        }
      }
    else
      {
      translated = false;
      A_DPRINT("Bad symbol id!\n");
      }

    if (!sym_replaced)
      {
      start_idx = find_idx + ASymbol_id_str_length;
      }
    }  // while

  return translated;
  }

//---------------------------------------------------------------------------------------
// Returns a symbol reference from the table that matches the supplied symbol
//             id and string.  If it is not already in the table it is added.
// Author(s):   Conan Reis
ASymbolRef * ASymbolTable::symbol_reference(uint32_t sym_id, const AString & str, eATerm term)
  {
  if (sym_id == ASymbol_id_null)
    {
    #if defined(A_SYMBOL_REF_LINK)
      return ASymbol::ms_null.m_ref_p;
    #else
      return const_cast<ASymbolRef *>(&ASymbolRef::get_null());
    #endif
    }

  // Ensure symbol string no larger than 255 characters since only 1-byte is used to store
  // length in binary.
  A_ASSERTX(
    str.m_str_ref_p->m_length <= UINT8_MAX,
    AErrMsg(
      a_str_format(
        "Tried to create symbol '%s' (0x%X) but it too long!\n"
        "Its length is %u and the max length is 255 characters.\n"
        "[Try to use a different shorter string if possible.]",
        str.as_cstr(),
        sym_id,
        str.m_str_ref_p->m_length),
      AErrLevel_notify));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Use existing symbol reference if it is already registered.

  uint32_t     idx;
  ASymbolRef * sym_ref_p = m_sym_refs.get(sym_id, AMatch_first_found, &idx);

  if (sym_ref_p)
    {
    // Found existing symbol reference

    // Check for name collision
    A_ASSERTX(
      sym_ref_p->m_str_ref_p->is_equal(*str.m_str_ref_p),
      AErrMsg(
        a_str_format(
          "Symbol id collision!  The new string '%s' (len=%d) and the existing symbol '%s' (len=%d) are different,\n"
          "but they both have the same id 0x%X.\n"
          "[Try to use a different string if possible and hope that it has a unique id.]",
          str.as_cstr(),
          (int32_t)str.get_length(),
          sym_ref_p->m_str_ref_p->m_cstr_p,
          (int32_t)sym_ref_p->m_str_ref_p->m_length,
          sym_id),
        AErrLevel_notify));

    return sym_ref_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create new symbol reference

  sym_ref_p = (term == ATerm_long)
    ? ASymbolRef::pool_new(str.m_str_ref_p, sym_id)
    : ASymbolRef::pool_new(AStringRef::pool_new_copy(str.m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_length), sym_id);

  m_sym_refs.insert(*sym_ref_p, idx);

  return sym_ref_p;
  }

//---------------------------------------------------------------------------------------
// Modifiers:   static
// Author(s):   Conan Reis
ASymbolRef * ASymbolTable::symbol_reference(uint32_t sym_id, const char * cstr_p, uint32_t length, eATerm term)
  {
  if (sym_id == ASymbol_id_null)
    {
    #if defined(A_SYMBOL_REF_LINK)
      return ASymbol::ms_null.m_ref_p;
    #else
      return const_cast<ASymbolRef *>(&ASymbolRef::get_null());
    #endif
    }

  // Ensure symbol string no larger than 255 characters since only 1-byte is used to store
  // length in binary.
  A_ASSERTX(
    length <= UINT8_MAX,
    AErrMsg(
      a_str_format(
        "Tried to create symbol '%s' (0x%X) but it too long!\n"
        "Its length is %u and the max length is 255 characters.\n"
        "[Try to use a different shorter string if possible.]",
        cstr_p,
        sym_id,
        length),
      AErrLevel_notify));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Use existing symbol reference if it is already registered.

  uint32_t     idx;
  ASymbolRef * sym_ref_p = m_sym_refs.get(sym_id, 1u, &idx);

  if (sym_ref_p)
    {
    // Found existing symbol reference

    // Check for name collision
    A_ASSERTX(
      sym_ref_p->m_str_ref_p->is_equal(cstr_p, length),
      AErrMsg(
        a_str_format(
          "Symbol id collision!  The new string '%s' (len=%d) and the existing symbol '%s' (len=%d) are different,\n"
          "but they both have the same id 0x%X.\n"
          "[Try to use a different string if possible and hope that it has a unique id.]",
          cstr_p, 
          (int32_t)length,
          sym_ref_p->m_str_ref_p->m_cstr_p,
          (int32_t)sym_ref_p->m_str_ref_p->m_length,
          sym_id),
        AErrLevel_notify));

    return sym_ref_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create new symbol reference

  AStringRef * str_ref_p = (term == ATerm_long)
    ? AStringRef::pool_new(cstr_p, length, length + 1u, 1u, false, true)
    : AStringRef::pool_new_copy(cstr_p, length);

  sym_ref_p = ASymbolRef::pool_new(str_ref_p, sym_id);

  m_sym_refs.insert(*sym_ref_p, idx);

  return sym_ref_p;
  }

//---------------------------------------------------------------------------------------
//  Setups the auto-parse temporary symbol table. Symbol creation calls will put shared copies of
//  new symbols into the auto-parse symbol table. The auto-parse symbol table with then be used to
//  remove these temporary symbols once the auto-parse terminates.
//  
//  Author(s)  John Stenersen
void ASymbolTable::track_auto_parse_init()
  {
  if (ms_auto_parse_syms_p)
    {
    A_DPRINT(A_SOURCE_STR "ms_auto_parse_syms_p is not null. Forgotten call to track_auto_parse_term()?\n");
    track_auto_parse_term();
    }

  ms_auto_parse_syms_p = this;
  }


//---------------------------------------------------------------------------------------
//  Setups the auto-parse temporary symbol table. Symbol creation calls will put shared copies of
//  new symbols into the auto-parse symbol table. The auto-parse symbol table with then be used to
//  remove these temporary symbols once the auto-parse terminates.
//  
//  Author(s)  John Stenersen
void ASymbolTable::track_auto_parse_term()
  {
  if (!ms_auto_parse_syms_p)
    {
    A_DPRINT(A_SOURCE_STR "ms_auto_parse_syms_p is null (terminated) already.?\n");
    return;
    }

  //  Remove any symbols found in the auto-parse symbol table from the main symbol table.
  uint32_t length = ms_auto_parse_syms_p->get_length();
  for (uint32_t i = 0; i < length; i++)
    {
    ASymbolRef * sym_ref = ms_auto_parse_syms_p->m_sym_refs.get_at(i);
    ms_main_p->m_sym_refs.remove(sym_ref->m_uid, AMatch_first_found);

    //A_DPRINT(A_SOURCE_STR "Removing symbol = %ld\n", sym_ref->m_uid);
    }

  ms_auto_parse_syms_p = nullptr;
  }


#endif // A_SYMBOLTABLE_CLASSES
