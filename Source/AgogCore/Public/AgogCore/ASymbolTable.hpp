// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbolTable class declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ASymbol.hpp>
#include <AgogCore/APSorted.hpp>


//=======================================================================================
// Defines
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Translation table that enables conversion from symbols to strings.
//
// See the ASymbol class for more info.
class A_API ASymbolTable
  {
  friend class ASymbol;  // ASymbol needs access to ASymbol internal structures

  public:

  // Public Class Data

    // This is the common / default symbol table.  If the main table is needed during
    // global initialization call get_main() instead of using ms_main_p directly to ensure
    // that it has been constructed and associated.  [Defined in AgogCore.cpp]

    static ASymbolTable * ms_main_p;

    #if defined(A_SYMBOLTABLE_CLASSES)
      static ASymbolTable * ms_auto_parse_syms_p;
    #endif

  // Common Methods

    static void initialize();
    static void deinitialize();
    static bool is_initialized();

    explicit ASymbolTable(bool sharing_symbols = false, uint32_t initial_size = 0u);
    ~ASymbolTable();

  // Converter / Serialization Methods

    void     as_binary(void ** binary_pp) const;
    uint32_t as_binary_length() const;
    void     assign_binary(const void ** binary_pp);
    void     merge_binary(const void ** binary_pp);

    #if defined(A_SYMBOL_STR_DB_AGOG)
      void append_shared(const ASymbol & shared_symbol);
    #endif

  // Translation / Test Methods - only retrieves symbol if it is already registered

    bool    is_registered(uint32_t sym_id) const;
    bool    translate_ids(AString * str_p) const;
    AString translate_id(uint32_t sym_id) const;
    bool    translate_known_id(uint32_t sym_id, AString * str_p) const;
    ASymbol translate_str(const AString & str) const;

    uint32_t  get_length() { return m_sym_refs.get_length(); }
    void      track_auto_parse_init();
    void      track_auto_parse_term();

  // Maintenance

    void empty();
    void validate() const;

    #if defined(A_SYMBOL_REF_COUNT)
      void remove_unreferenced();
    #endif

  // Class Methods

    ASymbolRef * get_symbol_by_index(uint32_t index) const;

  protected:

    ASymbolRef * get_symbol(uint32_t id) const;
    ASymbolRef * symbol_reference(uint32_t sym_id, const AString & str, eATerm term);
    ASymbolRef * symbol_reference(uint32_t sym_id, const char * cstr_p, uint32_t length, eATerm term);

  // Data Members

    // Symbols (strings and ids) making up this table.  Sorted in symbol id order.
    // $Revisit - CReis Probably best written as some sort of tree (esp. if there are many
    // symbols created during run-time) rather than a single array - possibly custom to
    // this class.
    APSortedLogical<ASymbolRef, uint32_t> m_sym_refs;

    // Indicates whether or not the symbol table is sharing ASymbol objects with another
    // ASymbolTable.
    bool m_sharing;

  };  // ASymbolTable

#endif // A_SYMBOLTABLE_CLASSES


//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/ASymbolTable.inl>
#endif
