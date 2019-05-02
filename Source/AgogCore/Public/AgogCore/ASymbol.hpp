// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbol class declaration header
// Notes:          See the ASymbol class for more info.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AObjReusePool.hpp>


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

//---------------------------------------------------------------------------------------
// If 'A_SYMBOL_STR_DB' is defined, ASymbol objects can be converted to the original
// strings that they are based on.
//
//   Default:  if 'A_EXTRA_CHECK' is defined, 'A_SYMBOL_STR_DB' is defined.
//   Always enabled:  When 'A_EXTRA_CHECK' is not defined and symbol to C-string conversion
//     is desired, just define 'A_SYMBOL_STR_DB' as a compiler preprocessor directive.
//   Never enabled:  If 'A_NO_SYMBOL_STR_DB' is defined as a compiler preprocessor
//     directive, 'A_SYMBOL_STR_DB' will not be defined even if 'A_EXTRA_CHECK' is defined.
#if defined(A_EXTRA_CHECK) && !defined(A_NO_SYMBOL_STR_DB)
  #define A_SYMBOL_STR_DB
#endif

//---------------------------------------------------------------------------------------
// If 'A_SYMBOL_STR_DB_AGOG' is defined, the Agog mechanism for a symbol/string database
// is used.  [Some APIs have their own string id databases.]
//
//   Default:  if 'A_SYMBOL_STR_DB' is defined, 'A_SYMBOL_STR_DB_AGOG' is defined.
//   Always enabled:  Regardless of what else is defined just define 'A_SYMBOL_STR_DB' as 
//     a compiler preprocessor directive.
//   Never enabled:  If 'A_NO_SYMBOL_STR_DB_AGOG' is defined as a compiler preprocessor
//     directive, 'A_SYMBOL_STR_DB' will not be defined even if 'A_SYMBOL_STR_DB' is defined.
#if defined(A_SYMBOL_STR_DB) && !defined(A_NO_SYMBOL_STR_DB_AGOG)
  #define A_SYMBOL_STR_DB_AGOG
#endif

//---------------------------------------------------------------------------------------
// If 'A_SYMBOLTABLE_CLASSES' is defined, ASymbolTable and associated classes are defined.
// [Even if the Agog mechanism for a symbol/string database is not used some Agog code]
//
//   Default:  if 'A_SYMBOL_STR_DB_AGOG' is defined, 'A_SYMBOLTABLE_CLASSES' is defined.
//   Always enabled:  Regardless of what else is defined just define 'A_SYMBOLTABLE_CLASSES' 
//     as a compiler preprocessor directive.
#if defined(A_SYMBOL_STR_DB_AGOG)
  #define A_SYMBOLTABLE_CLASSES
#endif

//---------------------------------------------------------------------------------------
// If the string data-base exists have the ASymbol objects directly point to the ASymbolRef
// objects for fast symbol->string conversions.  Memory usage is the same.
// [Requires A_SYMBOL_STR_DB to be defined.]
#if defined(A_SYMBOL_STR_DB_AGOG) && !defined(A_NO_SYMBOL_REF_LINK)
  #define A_SYMBOL_REF_LINK
#endif

//---------------------------------------------------------------------------------------
// Reference counting of symbols.  Ensures that symbols that have no references are cleaned
// up and don't take up memory.  This adds a fair amount of maintenance overhead to a class
// that is used for efficiency so it should only be used if memory is at a premium.
// By default only bother doing reference counting of symbols on non-PC - PC has enough
// memory to handle it.
// [Requires A_SYMBOL_REF_LINK to be defined.]
#if defined(A_SYMBOL_REF_LINK) && 0
  #define A_SYMBOL_REF_COUNT
#endif


//---------------------------------------------------------------------------------------
// Id access: use sym.ASYM_MBR_ID instead of sym.m_ref_p->m_uid or sym.m_uid
#if defined(A_SYMBOL_REF_LINK)
  #define ASYM_MBR_ID   m_ref_p->m_uid
#else
  #define ASYM_MBR_ID   m_uid
#endif


//---------------------------------------------------------------------------------------
// API Symbol Macros
#if !defined(A_SYMID_MACROS)

  #include <AgogCore/AChecksum.hpp>

  // Indicate that the symbol macros have been defined
  #define A_SYMID_MACROS

  #define ASYMBOL_CSTR_STORE(_sym_id, _cstr_p, _length, _term)  ASymbolTable::ms_main_p->symbol_reference(_sym_id, _cstr_p, _length, _term)
  #define ASYMBOL_CSTR_TO_ID(_cstr_p, _char_count)              AChecksum::generate_crc32_cstr(_cstr_p, _char_count)

  #ifdef A_SYMBOL_STR_DB_AGOG
    #define ASYMBOL_ID_TO_STR(_sym_id)                          ASymbolTable::ms_main_p->translate_id(_sym_id)
    #define ASYMBOL_ID_TO_CSTR(_sym_id)                         ASymbolTable::ms_main_p->translate_id(_sym_id)
  #else
    #define ASYMBOL_ID_TO_STR(_sym_id)                          ASymbol::id_as_str(_sym_id)
    #define ASYMBOL_ID_TO_CSTR(_sym_id)                         ASymbol::id_as_str(_sym_id)
  #endif

  #define ASYMBOL_IS_REFFED(_sym_id)                            ASymbolTable::ms_main_p->is_registered(_sym_id)
  #define ASYMBOL_STR_STORE(_sym_id, _str, _term)               ASymbolTable::ms_main_p->symbol_reference(_sym_id, _str, _term);
  #define ASYMBOL_STR_TO_ID(_str)                               AChecksum::generate_crc32(_str)
  #define ASYMBOL_STR_TO_ID_CONCAT(_root_id, _str_suffix)       AChecksum::generate_crc32(_str_suffix, _root_id)
  #define ASYMBOL_STR_TO_SYM_IF_EXIST(_str)                     ASymbolTable::ms_main_p->translate_str(ident_str)

  #define A_SYMID_DEREF(_sym)                                   ((_sym).m_ref_p)->m_ref_count--
  #define A_SYMID_REF(_sym)                                     ((_sym).m_ref_p)->m_ref_count++

#endif


//---------------------------------------------------------------------------------------
// Symbol reference counting
#if defined(A_SYMBOL_REF_COUNT)
  #define ASYMBOL_REF(_sym)                                     A_SYMID_REF(_sym)
  #define ASYMBOL_DEREF(_sym)                                   A_SYMID_DEREF(_sym)
#else
  #define ASYMBOL_REF(_sym)                                     (void(0))
  #define ASYMBOL_DEREF(_sym)                                   (void(0))
#endif


//---------------------------------------------------------------------------------------
// Constant Symbol Helper Macros - see SkSymbol.hpp for usage examples for these defines.

// Define for identifier declaration
#define ASYMBOL_DECLARE(_prefix, _id)             extern const ASymbol _prefix##_##_id;

#if !defined(ASYMBOL_DEFINE_STR)
  // Define for identifier definition
  #define ASYMBOL_DEFINE_STR(_prefix, _id, _str)       const ASymbol _prefix##_##_id(ASymbol::create(_str));
  #define ASYMBOL_DEFINE_STR_NULL(_prefix, _id, _str)  const ASymbol _prefix##_##_id(ASymbol::get_null());
  #define ASYMBOL_ASSIGN_STR(_prefix, _id, _str)       const_cast<ASymbol&>(_prefix##_##_id) = ASymbol::create(_str);
  #define ASYMBOL_ASSIGN_STR_NULL(_prefix, _id, _str)  const_cast<ASymbol&>(_prefix##_##_id) = ASymbol::ms_null;
#endif

// Define for identifier definition
#define ASYMBOL_DEFINE(_prefix, _id)         ASYMBOL_DEFINE_STR(_prefix, _id, #_id)
#define ASYMBOL_DEFINE_NULL(_prefix, _id)    ASYMBOL_DEFINE_STR_NULL(_prefix, _id, #_id)
#define ASYMBOL_ASSIGN(_prefix, _id)         ASYMBOL_ASSIGN_STR(_prefix, _id, #_id)
#define ASYMBOL_ASSIGN_NULL(_prefix, _id)    ASYMBOL_ASSIGN_STR_NULL(_prefix, _id, #_id)


//---------------------------------------------------------------------------------------
// ASymbol constants
const uint32_t ASymbol_id_null     = UINT32_MAX;  // Same as ~0u
const uint32_t ASymbol_length_max  = 255u;


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
struct AStringRef;
class ASymbolTable;


#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Indirect id and string structure used internally by ASymbol objects and ASymbolTable.
struct A_API ASymbolRef
  {
  public:

  // Public Data Members

    // Unique identifier for symbol.  Generally the hash value created from the string
    // that the symbol is based on.
    uint32_t m_uid;

    #if defined(A_SYMBOL_REF_COUNT)
      // Number of references to this symbol.  See A_SYMBOL_REF_COUNT.
      uint32_t m_ref_count;
    #endif

    // The string reference that this symbol is based on - this allows quick translation 
    // from a symbol to a string and simpler debugging.
    // [This *could* be a AString rather than a AStringRef.]
    AStringRef * m_str_ref_p;

  // Methods

    ASymbolRef();
    ASymbolRef(AStringRef * str_ref_p, uint32_t sym_id);

    operator uint32_t () const    { return m_uid; }


  // Class Methods

    static const ASymbolRef &          get_null();
    static ASymbolRef *                pool_new(AStringRef * str_ref_p, uint32_t uid);
    static void                        pool_delete(ASymbolRef * ref_p);
    static AObjReusePool<ASymbolRef> & get_pool();

  protected:

    friend class AObjReusePool<ASymbolRef>;

    ASymbolRef ** get_pool_unused_next() { return (ASymbolRef **)&m_str_ref_p; } // Area in this class where to store the pointer to the next unused object when not in use

  };  // ASymbolRef

#endif // A_SYMBOLTABLE_CLASSES


//---------------------------------------------------------------------------------------
// A symbol object is a sort of hybrid between a string and an enumerated type.  A symbol
// is like string in that it represents a string of characters and can be compared to
// other symbols to determine if it is the same, lesser or greater [unlike strings symbols
// are not in alphabetical order though their order is consistent].  Symbols are much
// faster than strings since they only use a single number for identification and
// comparison like an enumerated type.
//
// Wherever a string is to be used like an enumerated type, in parsing, or for quick
// comparisons, ASymbol objects should probably be used instead of AString.
//
// Symbols also have the option to not store the strings that they represent in which case
// they can have significant memory savings over strings.
class A_API ASymbol
  {
  friend class ASymbolTable;  // ASymbolTable needs access to ASymbol internal structures
  friend class ANamed;

  public:

  // Common Class Data

    // Null/empty "" symbol - use get_null() or ASymbol sym(&ASymbolRef::get_null()) if it
    // needs to be accessed at global initialization.
    static const ASymbol ms_null;  

  // Common Methods

    A_NEW_OPERATORS(ASymbol);

    ASymbol();
    ASymbol(const ASymbol & source);
    ~ASymbol();

  // Creation Methods

    // These are class/static functions rather than constructors to underline the fact
    // that symbols can have a substantial creation cost and should be created only when
    // absolutely necessary.  Whenever possible a symbol global constant - usually created
    // via macros (see SkSymbol for examples) - should be used instead.
    // All these class methods use the *main* symbol table by default - use methods in
    // ASymbolTable if a table other than *main* is needed.

    static ASymbol create(const AString & str, eATerm term = ATerm_long);
    static ASymbol create(const char * cstr_p, uint32_t length = ALength_calculate, eATerm term = ATerm_long);
    static ASymbol create_from_binary(const void ** sym_binary_pp, bool require_existance = true);
    static ASymbol create_existing(uint32_t id);
    static ASymbol create_existing(const AString & str);
    static ASymbol create_existing(const char * cstr_p, uint32_t length = ALength_calculate);

    ASymbol        create_add(const AString & suffix) const;
    ASymbol        create_add(const char * suffix_p, uint32_t length = ALength_calculate) const;

  // Assignment Methods

    ASymbol & operator=(const ASymbol & source);

  // Comparison Methods

    eAEquate compare(const ASymbol & sym) const;
    bool     operator==(const ASymbol & sym) const;  // Equal to
    bool     operator!=(const ASymbol & sym) const;  // Not equal to
    bool     operator<(const ASymbol & sym) const;   // Less-than
    bool     operator<=(const ASymbol & sym) const;  // Less-than or equal to
    bool     operator>(const ASymbol & sym) const;   // Greater-than
    bool     operator>=(const ASymbol & sym) const;  // Greater-than or equal to

    #if defined(A_SYMBOL_STR_DB)
      eAEquate compare_str(const ASymbol & sym) const;
    #endif

  // Converter Methods

  void            as_binary(void ** binary_pp) const;
  static uint32_t as_binary_length() { return 4u; }

    #if defined(A_SYMBOL_STR_DB)

      // Converters to the string that the symbol was based on - and they will only work
      // if the symbol->string database (A_SYMBOL_STR_DB) is available.
      // The symbol->string database is often not available in user/final product builds.
      // A_SYMBOL_STR_DB is defined by default if A_EXTRA_CHECK is defined.  They are
      // compiled out of if A_SYMBOL_STR_DB is not defined to ensure that no code that
      // depends on getting valid strings is invalid if the string database is not available.

      AString      as_string() const;
      const char * as_cstr() const;
      uint32_t     get_str_length() const;

    #endif

    // These _dbg() methods return strings if the database is available otherwise a string
    // version of the id is given in the form |#12345678#|  These methods should be used in
    // asserts and when giving other similar debug info.  They are still available even if
    // A_EXTRA_CHECK is not defined so that some sort of error info can be given even in
    // user/final builds.

      AString      as_str_dbg() const;
      const char * as_cstr_dbg() const;

    // Unique id as as string in the form |#12345678#|

      const AString & as_id_str() const;
      const char *    as_id_cstr() const;

  // Accessor Methods

    uint32_t get_id() const;
    bool     is_null() const;
    void     set_null();

  // Class Methods

    static const ASymbol & get_null();
    static const AString & id_as_str(uint32_t sym_id);

    #if defined(A_SYMBOLTABLE_CLASSES)
      static bool is_tracking_serialized()    { return (ms_serialized_syms_p != nullptr); }
      static void track_serialized(ASymbolTable * used_syms_p = nullptr);
      static void table_from_binary(const void ** binary_pp);
    #endif

  protected:
  // Internal Methods

    #if defined(A_SYMBOL_REF_LINK)
      ASymbol(const ASymbolRef * ref_p);
    #else
      ASymbol(uint32_t uid);
    #endif

  // Data Members

    #if defined(A_SYMBOL_REF_LINK)
      ASymbolRef * m_ref_p;
    #else
      uint32_t m_uid;
    #endif


  // Class Data Members

    #if defined(A_SYMBOLTABLE_CLASSES)
      // Tracks any symbols that have been serialized - i.e. any symbol ids that have been
      // written to a binary via as_binary(sym, binary_pp).  Not used if nullptr.
      static ASymbolTable * ms_serialized_syms_p;
    #endif

  };  // ASymbol


//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/ASymbol.inl>
#endif
