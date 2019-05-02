// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ASymbol class declaration header
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AStringRef.hpp>
#include <AgogCore/AString.hpp>
#include <AgogCore/ABinaryParse.hpp>


//=======================================================================================
// ASymbolRef Inline Functions
//=======================================================================================

#if defined(A_SYMBOLTABLE_CLASSES)

//---------------------------------------------------------------------------------------
// Default Constructor - used by object reuse pool in ASymbolRef::get_pool()
// Author(s):   Conan Reis
A_INLINE ASymbolRef::ASymbolRef()
  {
  // Nothing initialized here since it is is done in ASymbolRef::pool_new()
  }

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
A_INLINE ASymbolRef::ASymbolRef(AStringRef * str_ref_p, uint32_t sym_id) :
  m_uid(sym_id),
  #if defined(A_SYMBOL_REF_COUNT)
    m_ref_count(1u),
  #endif
  m_str_ref_p(str_ref_p)
  {
  m_str_ref_p->m_ref_count++;
  }

//---------------------------------------------------------------------------------------
// Returns dynamic reference pool. Pool created first call and reused on successive calls.
// 
// #Notes
//   Uses Scott Meyers' tip "Make sure that objects are initialized before they're used"
//   from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd Edition]
//   This is instead of using a non-local static object for a singleton.
//   
// #Modifiers  static
// #Author(s)  Conan Reis
A_INLINE AObjReusePool<ASymbolRef> & ASymbolRef::get_pool()
  {
  static AObjReusePool<ASymbolRef> s_pool(AgogCore::get_app_info()->get_pool_init_symbol_ref(), AgogCore::get_app_info()->get_pool_incr_symbol_ref());
  //A_DSINGLETON_GUARD;
  return s_pool;
  }

//---------------------------------------------------------------------------------------
// Retrieves a AString reference object from the dynamic pool and initializes
//             it for use.  This should be used instead of 'new' because it prevents
//             unnecessary allocations by reusing previously allocated objects.
// Returns:    a dynamic AStringRef
// See:        pool_delete()
// Notes:      To 'deallocate' an object that was retrieved with this method, use
//             'pool_delete()' rather than 'delete'.
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE ASymbolRef * ASymbolRef::pool_new(
  AStringRef * str_ref_p,
  uint32_t     uid
  )
  {
  ASymbolRef * ref_p = get_pool().allocate();

  str_ref_p->m_ref_count++;
  ref_p->m_str_ref_p = str_ref_p;
  ref_p->m_uid       = uid;

  #if defined(A_SYMBOL_REF_COUNT)
    ref_p->m_ref_count = 0u;
  #endif

  return ref_p;
  }

//---------------------------------------------------------------------------------------
// Frees up a symbol reference and puts it into the dynamic pool ready for
//             its next use.  This should be used instead of 'delete' because it
//             prevents unnecessary deallocations by saving previously allocated
//             objects.
// Arg         ref_p - pointer to ASymbol reference to free up and put into the
//             dynamic pool ready for its next use.
// See:        pool_new()
// Notes:      To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE void ASymbolRef::pool_delete(ASymbolRef * ref_p)
  {
  ref_p->m_str_ref_p->dereference();

  get_pool().recycle(ref_p);
  }

#endif // A_SYMBOLTABLE_CLASSES


//=======================================================================================
// ASymbol Inline Functions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Default Constructor - use get_null() or ASymbol sym(&ASymbolRef::get_null())
//             if it needs to be called at global initialization.
// Returns:    itself
// Author(s):   Conan Reis
A_INLINE ASymbol::ASymbol() :
  #if defined(A_SYMBOL_REF_LINK)
    m_ref_p(ms_null.m_ref_p)
  #else
    m_uid(ASymbol_id_null)
  #endif
  {
  ASYMBOL_REF(*this);
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Returns:    itself
// Arg         source - symbol to copy
// Author(s):   Conan Reis
A_INLINE ASymbol::ASymbol(const ASymbol & source)
  {
  #if defined(A_SYMBOL_REF_LINK)
    m_ref_p = source.m_ref_p;
  #else
    m_uid = source.ASYM_MBR_ID;
  #endif

  ASYMBOL_REF(source);
  }


#if defined(A_SYMBOL_REF_LINK)

//---------------------------------------------------------------------------------------
// Symbol reference constructor
// Returns:    itself
// Author(s):   Conan Reis
A_INLINE ASymbol::ASymbol(const ASymbolRef * ref_p) :
  m_ref_p(const_cast<ASymbolRef *>(ref_p))
  {
  ASYMBOL_REF(*this);
  }

#else

//---------------------------------------------------------------------------------------
// Id constructor
// Returns:    itself
// Author(s):   Conan Reis
A_INLINE ASymbol::ASymbol(uint32_t uid) :
  m_uid(uid)
  {
  ASYMBOL_REF(*this);
  }

#endif // A_SYMBOL_REF_LINK


//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
A_INLINE ASymbol::~ASymbol()
  {
  ASYMBOL_DEREF(*this);
  }

//---------------------------------------------------------------------------------------
// Creates a new symbol based on a concatenation of this symbol + supplied
//             C-string.
// Returns:    new symbol
// Arg         suffix_p - pointer to array of characters to append (does not need to be null
//             terminated unless length is equal to ALength_calculate).  'suffix_p' should
//             never be nullptr.  'cstr_p' will usually be a string literal or if
//             'term' is ATerm_short, 'suffix_p' may be any C-String that this string could
//             make a copy of.
// Arg         length - number of characters to use in 'suffix_p'.  The given length must
//             not be more than the size of 'suffix_p' and the C-String buffer pointed to
//             by 'suffix_p' should not have any null characters less then the given length.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in 'suffix_p'.
// Author(s):   Conan Reis
A_INLINE ASymbol ASymbol::create_add(
  const char * suffix_p,
  uint32_t     length // = ALength_calculate
  ) const
  {
  AString suffix(suffix_p, length);
  
  return create_add(suffix);
  }

//---------------------------------------------------------------------------------------
// Assignment operator.  Allows stringization (sym1 = sym2 = sym3)
// Returns:    itself 
// Arg         source - symbol to copy
// Author(s):   Conan Reis
A_INLINE ASymbol & ASymbol::operator=(const ASymbol & source)
  {
  ASYMBOL_REF(source);
  ASYMBOL_DEREF(*this);

  #if defined(A_SYMBOL_REF_LINK)
    m_ref_p = source.m_ref_p;
  #else
    m_uid = source.ASYM_MBR_ID;
  #endif

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Comparison Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Comparison between two symbols.
// Returns:    eAEquate indicating AEquate_equal, AEquate_less, or AEquate_greater
// Arg         sym - symbol to compare
// Examples:   if (sym1.compare(sym2) == AEquate_equal)
// Author(s):   Conan Reis
A_INLINE eAEquate ASymbol::compare(const ASymbol & sym) const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p == sym.m_ref_p
  #else
    return (ASYM_MBR_ID == sym.ASYM_MBR_ID)
  #endif
      ? AEquate_equal
      : ((ASYM_MBR_ID < sym.ASYM_MBR_ID)
        ? AEquate_less
        : AEquate_greater);
  }

//---------------------------------------------------------------------------------------
// Equal to
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator==(const ASymbol & sym) const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p == sym.m_ref_p;
  #else
    return (ASYM_MBR_ID == sym.ASYM_MBR_ID);
  #endif
  }

//---------------------------------------------------------------------------------------
// Not equal to
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator!=(const ASymbol & sym) const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p != sym.m_ref_p;
  #else
    return (ASYM_MBR_ID != sym.ASYM_MBR_ID);
  #endif
  }

//---------------------------------------------------------------------------------------
// Less-than
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator<(const ASymbol & sym) const
  {
  return ASYM_MBR_ID < sym.ASYM_MBR_ID;
  }

//---------------------------------------------------------------------------------------
// Less-than or equal to
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator<=(const ASymbol & sym) const
  {
  return ASYM_MBR_ID <= sym.ASYM_MBR_ID;
  }

//---------------------------------------------------------------------------------------
// Greater-than
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator>(const ASymbol & sym) const
  {
  return ASYM_MBR_ID > sym.ASYM_MBR_ID;
  }

//---------------------------------------------------------------------------------------
// Greater-than or equal to
// Returns:    true or false
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE bool ASymbol::operator>=(const ASymbol & sym) const
  {
  return ASYM_MBR_ID >= sym.ASYM_MBR_ID;
  }


#if defined(A_SYMBOL_STR_DB)

//---------------------------------------------------------------------------------------
// Determine case-sensitive comparison between string versions of symbols.
// Returns:    eAEquate indicating AEquate_equal, AEquate_less, or AEquate_greater
// Arg         sym - symbol to compare
// Author(s):   Conan Reis
A_INLINE eAEquate ASymbol::compare_str(const ASymbol & sym) const
  {
  #if defined(A_SYMBOL_REF_LINK)
    return m_ref_p->m_str_ref_p->compare(*sym.m_ref_p->m_str_ref_p);
  #else
    return as_string().compare(sym.as_string());
  #endif
  }

#endif // A_SYMBOL_STR_DB


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Create a string version of the symbol's id - not a translation - just the id as a
// number in the form |#12345678#|
// 
// Returns:    id in string number form
// Author(s):   Conan Reis
A_INLINE const AString & ASymbol::as_id_str() const
  {
  return ASymbol::id_as_str(ASYM_MBR_ID);
  }

//---------------------------------------------------------------------------------------
// Create a C-string version of the symbol's id - not a translation - just the id as a
// number in the form |#12345678#|
//
// Returns:    id in C-string number form
// Author(s):   Conan Reis
A_INLINE const char * ASymbol::as_id_cstr() const
  {
  return ASymbol::id_as_str(ASYM_MBR_ID).as_cstr();
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE uint32_t ASymbol::get_id() const
  {
  return ASYM_MBR_ID;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE bool ASymbol::is_null() const
  {
  return ASYM_MBR_ID == ASymbol_id_null;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE void ASymbol::set_null()
  {
  ASYMBOL_REF(ms_null);
  ASYMBOL_DEREF(*this);

  #if defined(A_SYMBOL_REF_LINK)
    m_ref_p = ms_null.m_ref_p;
  #else
    m_uid = ASymbol_id_null;
  #endif
  }



