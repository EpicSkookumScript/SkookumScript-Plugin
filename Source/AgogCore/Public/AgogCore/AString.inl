// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Dynamic AString class declaration header
// Notes:          The AString class should be used in the place of standard C-String
//              character array pointers.
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AStringRef.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AChecksum.hpp>
#include <AgogCore/AMath.hpp>
#include <string.h>      // Uses:  strncmp, _strnicmp, memcpy, strlen, memset


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Stream input function
// Author(s):    Conan Reis
//A_INLINE istream & operator>>(
//  istream & strm,
//  AString & str
//  )
//  {
//  char endline;
//  char buffer_p[AString_input_stream_max];
//
//  strm.get(buffer_p, AString_input_stream_max, '\n');
//  strm.get(endline);
//  str.set_cstr(buffer_p);
//  return strm;
//  }

//---------------------------------------------------------------------------------------
// Add/plus operator - concatenate str1 and str2 to form a new string.
// Returns:    New concatenated string
// Arg         str1 - string for first part of concatenation
// Arg         str2 - string for second part of concatenation
// Examples:   AString cat_str = str1 + str2;
// Notes:      If you try to add a C-String to a char or vice-versa without one
//             of the first two elements being a string or without making an
//             explicit conversion to string, then it will increment the
//             C-String char pointer by char.  Not what you want.  If you try
//             to add two C-strings you should get a "unable to add two pointers
//             error.  Adding two chars together is adding two numbers together.
// Author(s):   Conan Reis
A_INLINE AString operator+(
  const AString & str1,
  const AString & str2
  )
  {
  // This is a AString friend function

  uint32_t length_str1 = str1.m_str_ref_p->m_length;
  uint32_t length_str2 = str2.m_str_ref_p->m_length;
  uint32_t length_new  = length_str1 + length_str2;
  uint32_t size        = AStringRef::request_char_count(length_new);
  char *   buffer_p    = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, str1.m_str_ref_p->m_cstr_p, size_t(length_str1));  
  ::memcpy(buffer_p + length_str1, str2.m_str_ref_p->m_cstr_p, size_t(length_str2 + 1u));  // +1 to include nullptr character

  return AStringRef::pool_new(buffer_p, length_new, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Add/plus operator - concatenate str and cstr_p to form a new string.
// Returns:    New concatenated string
// Arg         str - string for first part of concatenation
// Arg         cstr_p - C-string for second part of concatenation
// Examples:   AString cat_str = str + "str";
// Notes:      If you try to add a C-String to a char or vice-versa without one
//             of the first two elements being a string or without making an
//             explicit conversion to string, then it will increment the
//             C-String char pointer by char.  Not what you want.  If you try
//             to add two C-strings you should get a "unable to add two pointers
//             error.  Adding two chars together is adding two numbers together.
// Author(s):   Conan Reis
A_INLINE AString operator+(
  const AString & str,
  const char *    cstr_p
  )
  {
  // This is a AString friend function

  uint32_t length_str  = str.m_str_ref_p->m_length;
  uint32_t length_cstr = uint32_t(::strlen(cstr_p));
  uint32_t length_new  = length_str + length_cstr;
  uint32_t size        = AStringRef::request_char_count(length_new);
  char *   buffer_p    = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, str.m_str_ref_p->m_cstr_p, size_t(length_str));  
  ::memcpy(buffer_p + length_str, cstr_p, size_t(length_cstr + 1u));  // +1 to include nullptr character

  return AStringRef::pool_new(buffer_p, length_new, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
//  Concatenation:  string + ch  NOTE:  This is a friend function.
// Returns:     New concatenated string
// Arg          str - string for first part of concatenation
// Arg          ch - character for second part of concatenation
// Examples:    AString str_ch = str + 'c';
// See:         append(ch), insert(ch)
// Notes:       If you try to add a C-String to a char or vice-versa without one
//              of the first two elements being a string or without making an
//              explicit conversion to string, then it will increment the
//              C-String char pointer by char.  Not what you want.  If you try
//              to add two C-strings you should get a "unable to add two pointers
//              error.  Adding two chars together is adding two numbers together.
// Author(s):    Conan Reis
A_INLINE AString operator+(
  const AString & str,
  char            ch
  )
  {
  uint32_t length_str = str.m_str_ref_p->m_length;
  uint32_t size       = AStringRef::request_char_count(length_str + 1u);
  char *   buffer_p   = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, str.m_str_ref_p->m_cstr_p, size_t(length_str));  
  buffer_p[length_str]      = ch;
  buffer_p[length_str + 1u] = '\0';  // Put in null-terminator

  return AStringRef::pool_new(buffer_p, length_str + 1u, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Add/plus operator - concatenate cstr_p and str to form a new string.
// Returns:    New concatenated string
A_INLINE AString operator+(
  const char *    cstr_p,
  const AString & str
  )
  {
  // This is a AString friend function

  uint32_t length_str = str.m_str_ref_p->m_length;
  uint32_t length_cstr = uint32_t(::strlen(cstr_p));
  uint32_t length_new = length_str + length_cstr;
  uint32_t size = AStringRef::request_char_count(length_new);
  char *   buffer_p = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, cstr_p, size_t(length_cstr));
  ::memcpy(buffer_p + length_cstr, str.m_str_ref_p->m_cstr_p, size_t(length_str + 1u));  // +1 to include null character

  return AStringRef::pool_new(buffer_p, length_new, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Concatenation:  ch + string  NOTE:  This is a friend function.
// Returns:     New concatenated string
A_INLINE AString operator+(
  char            ch,
  const AString & str
  )
  {
  uint32_t length_str = str.m_str_ref_p->m_length;
  uint32_t size = AStringRef::request_char_count(length_str + 1u);
  char *   buffer_p = AStringRef::alloc_buffer(size);

  buffer_p[0] = ch;
  ::memcpy(buffer_p + 1, str.m_str_ref_p->m_cstr_p, size_t(length_str + 1u));  // +1 to include null character

  return AStringRef::pool_new(buffer_p, length_str + 1u, size, 0u, true, false);
  }


//#######################################################################################
// AStrArgs Class
//#######################################################################################

//---------------------------------------------------------------------------------------
// Default / case constructor
// Returns:    itself
// Arg         case_check - AStrCase_sensitive or AStrCase_ignore.  Note that some methods do not use
//             the case info.  (Default AStrCase_sensitive)
// Author(s):   Conan Reis
A_INLINE AStrArgs::AStrArgs(
  eAStrCase case_check // = AStrCase_sensitive
  ) :
  m_case_check(case_check),
  m_start_pos(0u),
  m_end_pos(ALength_remainder)
  {
  }

//---------------------------------------------------------------------------------------
// Range Constructor
// Returns:    itself
// Arg         start_pos - Index position to start operation/search in string.
// Arg         end_pos - Index position to end operation/search in string.  If it is
//             ALength_remainder, the last index position of the string is used (length - 1).
//             (Default ALength_remainder)
// Arg         case_check - AStrCase_sensitive or AStrCase_ignore.  Note that some methods do not use
//             the case info.  (Default AStrCase_sensitive)
// Author(s):   Conan Reis
A_INLINE AStrArgs::AStrArgs(
  uint32_t  start_pos,
  uint32_t  end_pos,   // = ALength_remainder
  eAStrCase case_check // = AStrCase_sensitive
  ) :
  m_case_check(case_check),
  m_start_pos(start_pos),
  m_end_pos(end_pos)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Returns:    itself
// Arg         args - args to copy
// Author(s):   Conan Reis
A_INLINE AStrArgs::AStrArgs(const AStrArgs & args) :
  m_case_check(args.m_case_check),
  m_start_pos(args.m_start_pos),
  m_end_pos(args.m_end_pos)
  {
  }

//---------------------------------------------------------------------------------------
// "Constructor" for a 'spanned' range - this cannot be an actual
//             constructor since it would be ambiguous with the (start, end) constructor.
// Returns:    A String arguments object
// Arg         case_check - AStrCase_sensitive or AStrCase_ignore.  Note that some methods do not use
//             the case info.  (Default AStrCase_sensitive)
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE AStrArgs AStrArgs::span(
  uint32_t pos,
  uint32_t char_count // = ALength_remainder
  )
  {
  return AStrArgs(pos, (char_count == ALength_remainder) ? ALength_remainder : pos + char_count);
  }


//#######################################################################################
// AFindStrArgs Class
//#######################################################################################

//---------------------------------------------------------------------------------------
// Default / (instance-case) constructor
// Returns:    itself
// Arg         instance - Occurrence of match to find.  It may not be less than 1.
//             (Default 1u)
// Arg         case_check - AStrCase_sensitive or AStrCase_ignore.  Note that some methods do not use
//             the case info.  (Default AStrCase_sensitive)
// Author(s):   Conan Reis
A_INLINE AFindStrArgs::AFindStrArgs(
  uint32_t  instance,  // = 1u
  eAStrCase case_check // = AStrCase_sensitive
  ) :
  AStrArgs(case_check),
  m_instance(instance)
  {
  }

//---------------------------------------------------------------------------------------
// Case-instance constructor
// Returns:    itself
// Arg         case_check - AStrCase_sensitive or AStrCase_ignore.  Note that some methods do not use
//             the case info.
// Arg         instance - Occurrence of match to find.  It may not be less than 1.
//             (Default 1u)
// Author(s):   Conan Reis
A_INLINE AFindStrArgs::AFindStrArgs(
  eAStrCase case_check,
  uint32_t  instance // = 1u
  ) :
  AStrArgs(case_check),
  m_instance(instance)
  {
  }

//---------------------------------------------------------------------------------------
// Args Constructor
// Returns:    itself
// Arg         args - Range and case args to use.
// Arg         instance - Occurrence of match to find.  It may not be less than 1.
//             (Default 1u)
// Author(s):   Conan Reis
A_INLINE AFindStrArgs::AFindStrArgs(
  const AStrArgs & args,
  uint32_t         instance // = 1u
  ) :
  m_instance(instance)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor
// Returns:    itself
// Arg         args - find string args to copy.
// Author(s):   Conan Reis
A_INLINE AFindStrArgs::AFindStrArgs(const AFindStrArgs & args) :
  AStrArgs(args),
  m_instance(args.m_instance)
  {
  }


//#######################################################################################
// AString Class
//#######################################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor.  Makes an empty string.
// Returns:     itself
// See:         Other AString constructors
// Notes:       Rather than creating an empty AString, consider using ms_empty instead.
// Author(s):    Conan Reis
A_INLINE AString::AString() :
  m_str_ref_p(AStringRef::get_empty())
  {
  m_str_ref_p->m_ref_count++;
  }

//---------------------------------------------------------------------------------------
// Copy constructor.
// Returns:    itself
// Arg         str - string to copy
// See:        AString(str, extra_space), other AString constructors
// Author(s):   Conan Reis
A_INLINE AString::AString(const AString & str) :
  m_str_ref_p(str.m_str_ref_p)
  {
  m_str_ref_p->m_ref_count++;
  }

//---------------------------------------------------------------------------------------
// Copy constructor - specifies whether str to copy is persistent or not.
// Returns:    itself
// Arg         str - string to copy
// Arg         term - ATerm_short (not persistent - make a copy of string provided) or
//             ATerm_long (just reference string provided).
// See:        AString(str, extra_space), other AString constructors
// Author(s):   Conan Reis
A_INLINE AString::AString(
  const AString & str,
  eATerm          term
  )
  {
  if (term == ATerm_short)
    {
    m_str_ref_p = AStringRef::pool_new_copy(
      str.m_str_ref_p->m_cstr_p,
      str.m_str_ref_p->m_length);
    }
  else
    {
    m_str_ref_p = str.m_str_ref_p;
    m_str_ref_p->m_ref_count++;
    }
  }

//---------------------------------------------------------------------------------------
// Extra space copy constructor.
// Returns:    itself
// Arg         str - string to copy
// Arg         extra_space - amount of extra character space to allocate in addition to
//             the space needed for copying str.
// Notes:      This method assumes that the extra space is for a immanent write method
//             and so makes a copy of the C-String rather than just making a reference.
// Author(s):   Conan Reis
A_INLINE AString::AString(
  const AString & str,
  uint32_t        extra_space
  )
  {
  if (extra_space)
    {
    uint32_t size = AStringRef::request_char_count(str.m_str_ref_p->m_length + extra_space);

    m_str_ref_p = AStringRef::pool_new(
      AStringRef::alloc_buffer(size),  // C-String
      str.m_str_ref_p->m_length,       // Length
      size,                            // Size
      1u,                              // References
      true,                            // Deallocate
      false);                          // Not Read-Only

    ::memcpy(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, size_t(m_str_ref_p->m_length + 1u));  // +1 to include nullptr character
    }
  else
    {
    m_str_ref_p = str.m_str_ref_p;
    m_str_ref_p->m_ref_count++;
    }
  }

//---------------------------------------------------------------------------------------
// Converter / constructor from C-String.
// Returns:    a reference to itself
// Arg         cstr_p - Null terminated persistent C-String to refer to - usually a
//             string literal.
// Examples:   AString("Hello");
// See:        AString(cstr_p, length, persistent),
//             AString(buffer_p, size, length, deallocate)
// Notes:      This method assumes that 'cstr_p' is persistent for the lifespan of this
//             object and that this object should not write to the memory pointed to by
//             'cstr_p'.
//             If any modifying methods are called on this string, it will first make
//             its own unique copy of the C-String before it writes to it.
//             Use AString(cstr_p, length, persistent = true) if the length of cstr_p is
//             already known.  Also use it with 'persistent = false' if a copy of a
//             C-String is desired instead of a reference.
// Author(s):   Conan Reis
A_INLINE AString::AString(const char * cstr_p)
  {
  //A_ASSERT(cstr_p != nullptr, "Given nullptr instead of valid C-String", ErrId_null_cstr, AString);

  uint32_t length = uint32_t(::strlen(cstr_p));

  m_str_ref_p = AStringRef::pool_new_copy(cstr_p, length, 1u);
  }

//---------------------------------------------------------------------------------------
//  Character Constructor / Converter
// Returns:     a reference to itself
// Arg          ch - character to create string from
// Examples:    AString str('c');
// See:         AString(ch, char_count)
// Notes:       This constructor is a bit more efficient than calling AString(ch, 1u).
// Author(s):    Conan Reis
A_INLINE AString::AString(char ch)
  {
  uint32_t size = AStringRef::request_char_count(2u);

  m_str_ref_p = AStringRef::pool_new(AStringRef::alloc_buffer(2u), 1u, size, 1u, true, false);
  m_str_ref_p->m_cstr_p[0u] = ch;
  m_str_ref_p->m_cstr_p[1u] = '\0';
  }

//---------------------------------------------------------------------------------------
//  Constructor - Fills a new string with char_count ch.
// Returns:     a reference to itself
// Arg          ch - character to fill string with
// Arg          char_count - number of characters to
// Examples:    AString str('-', 80u);
// See:         AString(ch)
// Author(s):    Conan Reis
A_INLINE AString::AString(
  char ch,
  uint32_t char_count // = 1u
  )
  {
  uint32_t size = AStringRef::request_char_count(char_count);

  m_str_ref_p = AStringRef::pool_new(
    AStringRef::alloc_buffer(size),
    char_count,
    size,
    1u,
    true,
    false);

  memset(m_str_ref_p->m_cstr_p, ch, char_count);
  m_str_ref_p->m_cstr_p[char_count] = '\0';
  }

//---------------------------------------------------------------------------------------
//  Creates a string from an already existing AStringRef object.
// Returns:     a reference to itself
// Arg          m_str_ref_p - AStringRef object to wrap around.
// Author(s):    Conan Reis
A_INLINE AString::AString(AStringRef * str_ref_p) :
  m_str_ref_p(str_ref_p)
  {
  m_str_ref_p->m_ref_count++;
  }

//---------------------------------------------------------------------------------------
//  Destructor - frees the C-String character buffer.
// Author(s):    Conan Reis
A_INLINE AString::~AString()
  {
  m_str_ref_p->dereference();
  }

//---------------------------------------------------------------------------------------
//  Assignment operator - allows assignment stringization str1 = str2 = str3
// Returns:     a reference to itself
// Arg          str - string to copy
// Author(s):    Conan Reis
A_INLINE AString & AString::operator=(const AString & str)
  {
  // Increment prior to dereference since they could be the same AStringRef
  str.m_str_ref_p->m_ref_count++;
  m_str_ref_p->dereference();
  m_str_ref_p = str.m_str_ref_p;
  return *this;
  }

//---------------------------------------------------------------------------------------
//  C-String assignment.
// Returns:     a reference to itself
// Arg          cstr_p - Null terminated persistent C-String to refer to - usually a
//              string literal.
// Examples:    str = "Hello";
// See:         set_cstr(), set_buffer()
// Notes:       This method assumes that 'cstr_p' is persistent for the lifespan of this
//              object and that this object should not write to the memory pointed to by
//              'cstr_p'.
//              If any modifying methods are called on this string, it will first make
//              its own unique copy of the C-String before it writes to it.
//              Use set_cstr(cstr_p, length, persistent = true) if the length of 'cstr_p'
//              already known.  Also use it with 'persistent = false' if a copy of a
//              C-String is desired instead of a reference - it will then copy to the
//              existing C-String buffer of this object if possible.
// Author(s):    Conan Reis
A_INLINE AString & AString::operator=(const char * cstr_p)
  {
  uint32_t length = uint32_t(::strlen(cstr_p));

  m_str_ref_p->dereference();
  m_str_ref_p = AStringRef::pool_new(cstr_p, length, length + 1u, 1u, false, true);

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Cast / convert a AString to a C-String.
// Returns:     C-String internal character array
// See:         as_cstr()
// Notes:       Allows AString to be used synonymously with const char *
// Author(s):    Conan Reis
A_INLINE AString::operator const char * () const
  {
  return m_str_ref_p->m_cstr_p;
  }

//---------------------------------------------------------------------------------------
//  Returns standard C-String buffer
// Returns:     C-String internal character array
// See:         operator const char * (), as_cstr_writable()
// Notes:       This should be used without changing the array - the const is there for
//              a reason - use as_cstr_writable() if you want to modify it.
// Author(s):    Conan Reis
A_INLINE const char * AString::as_cstr() const
  {
  return m_str_ref_p->m_cstr_p;
  }

//---------------------------------------------------------------------------------------
//  Returns writable version of standard C-String buffer
// Returns:     writable version of C-String internal character array
// See:         operator const char * (), as_cstr(), set_length(), ensure_size()
// Notes:       !!!Be cautious calling this method!!!  Call set_length() if its
//              character length is changed.
//              This method calls ensure_writable().
// Author(s):    Conan Reis
A_INLINE char * AString::as_cstr_writable()
  {
  ensure_writable();
  return m_str_ref_p->m_cstr_p;
  }

//---------------------------------------------------------------------------------------
// Returns a checksum version of itself via the CRC (cyclic redundancy check)
//             algorithm.  Note that its case is retained so the checksum will be case
//             sensitive.  This can be called successively with previous CRC checksum
//             values.
// Returns:    CRC32 version of itself
// Arg         prev_crc - previous CRC checksum to iterate on
// See:        AChecksum
// Author(s):   Conan Reis
A_INLINE uint32_t AString::as_crc32(
  uint32_t prev_crc // = 0u
  ) const
  {
  return AChecksum::generate_crc32_cstr(m_str_ref_p->m_cstr_p, m_str_ref_p->m_length, prev_crc);
  }

//---------------------------------------------------------------------------------------
// Returns a checksum version of itself via the CRC (cyclic redundancy check)
//             algorithm.  Note that its case is treated as uppercase so the checksum
//             will not be case sensitive.  This can be called successively with previous
//             CRC checksum values.
// Returns:    CRC32 version of itself
// Arg         prev_crc - previous CRC checksum to iterate on
// See:        AChecksum
// Author(s):   Conan Reis
A_INLINE uint32_t AString::as_crc32_upper(
  uint32_t prev_crc // = 0u
  ) const
  {
  return AChecksum::generate_crc32_cstr_upper(m_str_ref_p->m_cstr_p, m_str_ref_p->m_length, prev_crc);
  }

//---------------------------------------------------------------------------------------
// Returns the space needed in bytes for the memory to be allocated for a
//             given string's byte stream binary form.
// Returns:    Byte stream size of this string
// See:        as_binary(), assign_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - string length
//               n bytes - string
// Author(s):   Conan Reis
A_INLINE uint32_t AString::as_binary_length() const
  {
  return sizeof(uint32_t) + m_str_ref_p->m_length;
  }

//---------------------------------------------------------------------------------------
// Fills byte stream pointed to by dest_stream_pp with the information needed
//             to recreate this string and increments the byte stream to just past the
//             last byte written.
// Arg         dest_stream_pp - Pointer to address of byte stream to fill and increment.
//             Its size *must* be large enough to fit all the binary data.  Use the
//             as_binary_length() method to determine the size needed prior to passing
//             dest_stream_pp to this method.
// See:        as_binary(), assign_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - string length
//               n bytes - string
// Author(s):   Conan Reis
A_INLINE void AString::as_binary(void ** dest_stream_pp) const
  {
  uint32_t length = m_str_ref_p->m_length;

  // 4 bytes - string length
  A_BYTE_STREAM_OUT32(dest_stream_pp, &length);

  // n bytes - string
  ::memcpy(*dest_stream_pp, m_str_ref_p->m_cstr_p, length);
  (*(uint8_t **)dest_stream_pp) += length;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Comparison Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Compares the current string to the one given to determine if it
//              is equal to, less than, or greater than it.
// Returns:     AEquate_equal, AEquate_less, or AEquate_greater
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE eAEquate AString::compare(const AString & str) const
  {
  return m_str_ref_p->compare(*str.m_str_ref_p);
  }

//---------------------------------------------------------------------------------------
//  Compares the current string to the one given to determine if it
//              is equal to, less than, or greater than it.
// Returns:     AEquate_equal, AEquate_less, or AEquate_greater
// Arg          str - string to compare
// Arg          case_check - indicates whether it should do a case sensitive or
//              case insensitive comparison (AStrCase_sensitive or AStrCase_ignore)
// Author(s):    Conan Reis
A_INLINE eAEquate AString::compare(
  const AString & str,
  eAStrCase       case_check
  ) const
  {
  // $Revisit - CReis This should be profiled, but I believe that it is faster than the commented out
  // section.  Also, strcmp() is not used, since this should be faster - once again this
  // should be profiled.
  if (m_str_ref_p != str.m_str_ref_p)  // if not AStringRef object in memory
    {
    int result = (case_check == AStrCase_sensitive) ?
      ::strncmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length) + 1) :
    #if defined(A_PLAT_PS4)
      ::strncasecmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length) + 1);
    #else
      ::_strnicmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length) + 1);
    #endif
    // This is a funky way to convert < 0 to -1, > 0 to 1, and 0 to stay 0
    return static_cast<eAEquate>((result > 0) ? 1 : result >> 31);
    }
  return AEquate_equal;

  //if (this != &str)  // if not same object in memory
  //  {
  //  char *str1_p     = m_str_ref_p->m_cstr_p;
  //  char *str2_p     = str.m_str_ref_p->m_cstr_p;
  //  char *str1_end_p = str1_p + MIN(m_str_ref_p->m_length, str.m_str_ref_p->m_length) + 1; // compare the # of characters in the shorter string (plus the null)
  //
  //  if (case_check == AStrCase_sensitive)  // case sensitive comparison
  //    {
  //    for (; str1_p < str1_end_p; str1_p++, str2_p++)
  //      {
  //      if (*str1_p != *str2_p)  // if characters differ
  //        {
  //        if (*str1_p < *str2_p)
  //          return AEquate_less;
  //        else
  //          return AEquate_greater;
  //        }
  //      }
  //    }
  //  else  // case insensitive comparison
  //    {
  //    char ch1, ch2;
  //
  //    for (; str1_p < str1_end_p; str1_p++, str2_p++)
  //      {
  //      ch1 = char(to_lowercase(*str1_p));
  //      ch2 = char(to_lowercase(*str2_p));
  //      if (ch1 != ch2)  // if characters differ
  //        {
  //        if (ch1 < ch2)  // select appropriate result
  //          return AEquate_less;
  //        else
  //          return AEquate_greater;
  //        }
  //      }
  //    }
  //  }
  //return AEquate_equal;
  }

//---------------------------------------------------------------------------------------
//  Compares the current string to the one given to determine if it
//              is equal to, less than, or greater than it.
// Returns:     AEquate_equal, AEquate_less, or AEquate_greater
// Arg          str - string to compare
// Arg          case_check - indicates whether it should do a case sensitive or
//              case insensitive comparison (AStrCase_sensitive or AStrCase_ignore)
// Examples:    AString  test("hello");
//              eAEquate cv = test.compare("aardvark");  // cv == AEquate_greater
//              test.compare("HeLlo", AStrCase_ignore);             // cv == AEquate_equal
// Author(s):    Conan Reis
A_INLINE eAEquate AString::compare(
  const char * cstr_p,
  eAStrCase    case_check // = AStrCase_sensitive
  ) const
  {
  if (case_check == AStrCase_sensitive)
    {
    // Efficiency   strncmp() may be faster
    // Note strcmp() returns -1, 0, or 1
    return eAEquate(::strcmp(m_str_ref_p->m_cstr_p, cstr_p));
    }
  else
    {
    #if defined(A_PLAT_PS4)
      int result = ::strcasecmp(m_str_ref_p->m_cstr_p, cstr_p);
    #else
      int result = ::_stricmp(m_str_ref_p->m_cstr_p, cstr_p);
    #endif
   
    // This is a funky way to convert < 0 to -1, > 0 to 1, and 0 to stay 0
    return static_cast<eAEquate>((result > 0) ? 1 : result >> 31);
    }
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the end of the current string to the
//              supplied substring.
// Returns:     true if there is a match, false if not
// Arg          substr - substring to match
// See:         is_imatch_end() for a case-insensitive version.
// Author(s):    Conan Reis
A_INLINE bool AString::is_match_end(const AString & substr) const
  {
  uint32_t this_len = m_str_ref_p->m_length;
  uint32_t str_len  = substr.m_str_ref_p->m_length;

  return this_len >= str_len
    && (compare_sub(substr, this_len - str_len) == AEquate_equal); 
  }

//---------------------------------------------------------------------------------------
//  Does a case-insensitive comparison of the end of the current string to
//              the supplied substring.
// Returns:     true if there is a match, false if not
// Arg          substr - substring to match
// See:         is_match_end() for a case-sensitive version.
// Author(s):    Conan Reis
A_INLINE bool AString::is_imatch_end(const AString & substr) const
  {
  uint32_t this_len = m_str_ref_p->m_length;
  uint32_t str_len  = substr.m_str_ref_p->m_length;

  return this_len >= str_len
    && (icompare_sub(substr, this_len - str_len) == AEquate_equal); 
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              string/substring using the specified matching type.
// Returns:     true if there is a match, false if not
// Arg          str - string/substring to compare
// See:         is_imatch() for a case-insensitive version.
// Author(s):    Conan Reis
A_INLINE bool AString::is_match(
  const AString & str,
  eAStrMatch      match_type
  ) const
  {
  uint32_t str_len = str.get_length();

  if (str_len)
    {
    switch (match_type)
      {
      case AStrMatch_subpart:
        return find(str);

      case AStrMatch_prefix:
        return (compare_sub(str) == AEquate_equal);

      case AStrMatch_suffix:
        return is_match_end(str);
        
      default: // This makes Clang happy
        break;
      }

    //case AStrMatch_exact:
    return is_equal(str);
    }

  return (match_type != AStrMatch_exact) || (m_str_ref_p->m_length == 0u);
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              string/substring using the specified matching type.
// Returns:     true if there is a match, false if not
// Arg          str - string/substring to compare (note that the Boyer-Moore String is set
//              to use case sensitive or case insensitive comparison when it is created.
// See:         is_imatch() for a case-insensitive version.
// Author(s):    Conan Reis
A_INLINE bool AString::is_match(
  const AStringBM & str,
  eAStrMatch        match_type
  ) const
  {
  uint32_t str_len = str.get_length();

  if (str_len)
    {
    switch (match_type)
      {
      case AStrMatch_subpart:
        return find(str);

      case AStrMatch_prefix:
        // $Revisit - CReis Make a AStringBM version of this call.
        return (((str.m_case == AStrCase_sensitive) ? compare_sub(str) : icompare_sub(str)) == AEquate_equal);

      case AStrMatch_suffix:
        // $Revisit - CReis Make a AStringBM version of this call.
        return (str.m_case == AStrCase_sensitive) ? is_match_end(str) : is_imatch_end(str);
        
      default: // case AStrMatch_exact
        // $Revisit - CReis Make a AStringBM version of this call.
        return (str.m_case == AStrCase_sensitive) ? is_equal(str) : is_iequal(str);
      }
    }

  return (match_type != AStrMatch_exact) || (m_str_ref_p->m_length == 0u);
  }

//---------------------------------------------------------------------------------------
//  Does a case-insensitive comparison of the current string to the supplied
//              string/substring using the specified matching type.
// Returns:     true if there is a match, false if not
// Arg          str - string/substring to compare
// See:         is_match() for a case-sensitive version.
// Author(s):    Conan Reis
A_INLINE bool AString::is_imatch(
  const AString & str,
  eAStrMatch      match_type
  ) const
  {
  uint32_t str_len = str.get_length();

  if (str_len)
    {
    switch (match_type)
      {
      case AStrMatch_subpart:
        return find(str, 1u, nullptr, 0u, ALength_remainder, AStrCase_ignore);

      case AStrMatch_prefix:
        return (icompare_sub(str) == AEquate_equal);

      case AStrMatch_suffix:
        return is_imatch_end(str);
        
      default: // case AStrMatch_exact:
        return is_iequal(str);
      }
    }

  return (match_type != AStrMatch_exact) || (m_str_ref_p->m_length == 0u);
  }

//---------------------------------------------------------------------------------------
//  Less-than comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator<(const AString & str) const
  {
  return (::strncmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length)) < 0);
  }

//---------------------------------------------------------------------------------------
//  Greater-than comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator>(const AString & str) const
  {
  return (::strncmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length)) > 0);
  }

//---------------------------------------------------------------------------------------
//  Less-than or equal to comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator<=(const AString & str) const
  {
  return (::strncmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length)) <= 0);
  }

//---------------------------------------------------------------------------------------
//  Greater-than or equal to comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator>=(const AString & str) const
  {
  return (::strncmp(m_str_ref_p->m_cstr_p, str.m_str_ref_p->m_cstr_p, a_min(m_str_ref_p->m_length, str.m_str_ref_p->m_length)) >= 0);
  }

//---------------------------------------------------------------------------------------
//  Equal to comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator==(const AString & str) const
  {
  return m_str_ref_p->is_equal(*str.m_str_ref_p);
  }

//---------------------------------------------------------------------------------------
//  Equal to comparison.
// Arg          cstr_p - Null terminated C-string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator==(const char * cstr_p) const
  {
  return m_str_ref_p->is_equal(cstr_p);
  }

//---------------------------------------------------------------------------------------
//  Not equal to comparison.
// Arg          str - string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator!=(const AString & str) const
  {
  return !m_str_ref_p->is_equal(*str.m_str_ref_p);
  }

//---------------------------------------------------------------------------------------
//  Not equal to comparison.
// Arg          cstr_p - Null terminated C-string to compare
// Author(s):    Conan Reis
A_INLINE bool AString::operator!=(const char * cstr_p) const
  {
  return !m_str_ref_p->is_equal(cstr_p);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Returns character at index pos.  Can only be used as an r-value.
// Returns:    character
// Arg         pos - index of character to retrieve
// Examples:   char ch = str.get_at(5);
// See:        operator(), operator[]
// Notes:      This method is equivalent to operator()
// Author(s):   Conan Reis
A_INLINE char AString::get_at(uint32_t pos) const
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(uint32_t(pos), "get_at");
  #endif

  return m_str_ref_p->m_cstr_p[pos];
  }

//---------------------------------------------------------------------------------------
// Returns first character in string.
// Returns:    First character in string or null terminator ('\0') if string is empty.
// Examples:   char ch = str.get_first();
// See:        operator(), operator[], get_at(), get_last()
// Notes:      Commonly desired character - accessor more intuitive than str.get_at(0u)
//             [Assumes string is null-terminated.]
// Author(s):   Conan Reis
A_INLINE char AString::get_first() const
  {
  return m_str_ref_p->m_cstr_p[0u];
  }

//---------------------------------------------------------------------------------------
// Returns last character in string.
// Returns:    Last character in string or null terminator ('\0') if string is empty.
// Examples:   char ch = str.get_last();
// See:        operator(), operator[], get_at(), get_first()
// Notes:      Commonly desired character - accessor more intuitive than str.get_at(str.get_length())
// Author(s):   Conan Reis
A_INLINE char AString::get_last() const
  {
  return m_str_ref_p->m_length
    ? m_str_ref_p->m_cstr_p[m_str_ref_p->m_length - 1u]
    : '\0';
  }

//---------------------------------------------------------------------------------------
// Returns number of characters in string
// Returns:    number of characters in string
// Examples:   uint32_t length = str.get_length();
// See:        get_size(), ensure_size(), is_empty()
// Author(s):   Conan Reis
A_INLINE uint32_t AString::get_length() const
  {
  return m_str_ref_p->m_length;
  }

//---------------------------------------------------------------------------------------
// Returns C-String buffer space allocated for characters
// Returns:    C-String buffer space allocated for characters
// Examples:   uint32_t buffer_size = str.get_size();
// See:        get_length(), ensure_size(), compact()
// Author(s):   Conan Reis
A_INLINE uint32_t AString::get_size() const
  {
  return m_str_ref_p->m_size;
  }

//---------------------------------------------------------------------------------------
// Returns reference to character at index pos.  Can be used as both an
//             l-value and an r-value.
// Returns:    reference to character
// Arg         pos - index of character to retrieve
// Examples:   str[5] = 'X';
// See:        get_at(), as_cstr_writable()
// Notes:      Instead of calling this method repeatedly, use as_cstr_writable() and
//             then use the returned C-String.
//             This method calls ensure_writable().
// Author(s):   Conan Reis
A_INLINE char & AString::operator[](int pos)
  {
  // $Revisit - CReis - Remove this int version if there are no compiler ambiguities

  #ifdef A_BOUNDS_CHECK
    bounds_check(uint32_t(pos), "operator[]");
  #endif

  ensure_writable();
  return m_str_ref_p->m_cstr_p[pos];
  }

//---------------------------------------------------------------------------------------
// Returns reference to character at index pos.  Can be used as both an
//             l-value and an r-value.
// Returns:    reference to character
// Arg         pos - index of character to retrieve
// Examples:   str[5] = 'X';
// See:        get_at(), as_cstr_writable()
// Notes:      Instead of calling this method repeatedly, use as_cstr_writable() and
//             then use the returned C-String.
//             This method calls ensure_writable().
// Author(s):   Conan Reis
A_INLINE char & AString::operator[](uint32_t pos)
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(pos, "operator[]");
  #endif

  ensure_writable();
  return m_str_ref_p->m_cstr_p[pos];
  }

//---------------------------------------------------------------------------------------
// Returns character at index pos.  Can only be used as an r-value.
// Returns:    character
// Arg         pos - index of character to retrieve
// Examples:   char ch = str(5);
// See:        get_at(), operator[]
// Notes:      This method is equivalent to get_at()
//             $Revisit - CReis - Remove this int version if there are no compiler ambiguities
// Author(s):   Conan Reis
A_INLINE char AString::operator()(int pos) const
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(uint32_t(pos), "operator()");
  #endif

  return m_str_ref_p->m_cstr_p[pos];
  }

//---------------------------------------------------------------------------------------
// Returns character at index pos.  Can only be used as an r-value.
// Returns:    character
// Arg         pos - index of character to retrieve
// Examples:   char ch = str(5);
// See:        get_at(), operator[]
// Notes:      This method is equivalent to get_at()
// Author(s):   Conan Reis
A_INLINE char AString::operator()(uint32_t pos) const
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(pos, "operator()");
  #endif

  return m_str_ref_p->m_cstr_p[pos];
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  String concatenation - appends another string onto the existing string
// Arg          str - string to append
// Examples:    str.append(str1);
// See:         operator+=(), add(), operator+(), operator=(), insert()
// Notes:       The existing buffer is always retained.
//              This method is identical to operator+=(), but does not return a reference
//              to itself.
// Author(s):    Conan Reis
A_INLINE void AString::append(const AString & str)
  {
  uint32_t str_length = str.m_str_ref_p->m_length;

  if (str_length)
    {
    // calculate total length of string
    uint32_t total_length = m_str_ref_p->m_length + str_length;

    ensure_size(total_length);
    
    // concatenate new string
    memcpy(m_str_ref_p->m_cstr_p + m_str_ref_p->m_length, str.m_str_ref_p->m_cstr_p, size_t(str_length + 1u));
    
    m_str_ref_p->m_length = total_length;
    }
  }

//---------------------------------------------------------------------------------------
//  Character concatenation / append:  string += ch
// Arg          ch - character to append.  Null characters will be ignored.
// Examples:    str.append('c');
// See:         operator+=(), add(), operator+(), operator=(), insert()
// Notes:       The existing buffer is always retained.
// Author(s):    Conan Reis
A_INLINE void AString::append(char ch)
  {
  if (ch != '\0')  // if null character, ignore
    {
    uint32_t length = m_str_ref_p->m_length + 1u;

    ensure_size(length);

    m_str_ref_p->m_cstr_p[length - 1u] = ch;      // Concatenate new character
    m_str_ref_p->m_cstr_p[length]      = '\0';    // Place a terminating character
    m_str_ref_p->m_length              = length;  // Modify length
    }
  }

//---------------------------------------------------------------------------------------
//  Multiple Character concatenation / append
// Arg          ch - type of characters to append.  Null characters will be ignored.
// Examples:    str.append('=', 100u);  // Adds 100 = characters
// See:         append(ch), operator+=(), add(), operator+(), operator=(), insert()
// Notes:       The existing buffer is always retained.
// Author(s):    Conan Reis
A_INLINE void AString::append(
  char ch,
  uint32_t char_count
  )
  {
  if (ch != '\0')  // if null character, ignore
    {
    A_ASSERTX(char_count < uint32_t(UINT16_MAX), a_cstr_format("Tried to append %u characters to '%s' - which is probably an error!", char_count, m_str_ref_p->m_cstr_p));

    uint32_t old_length = m_str_ref_p->m_length;
    uint32_t length     = old_length + char_count;

    ensure_size(length);

    // Add char_count X characters
    memset(m_str_ref_p->m_cstr_p + old_length, ch, char_count);
    m_str_ref_p->m_cstr_p[length] = '\0';    // Place a terminating character
    m_str_ref_p->m_length         = length;  // Modify length
    }
  }

//---------------------------------------------------------------------------------------
// Squish the buffer down to the smallest size possible while still retaining
//             enough space to keep the current string.
// Notes:      Does nothing if the size is already compacted.
// Author(s):   Conan Reis
A_INLINE void AString::compact()
  {
  if (m_str_ref_p->m_size > (m_str_ref_p->m_length + 1u))
    {
    set_size(m_str_ref_p->m_length);
    }
  }

//---------------------------------------------------------------------------------------
// Remove all characters from the string
// See:        remove(), remove_all()
// Author(s):   Conan Reis
A_INLINE void AString::empty()
  {
  // If unique and writable
  if ((m_str_ref_p->m_ref_count + m_str_ref_p->m_read_only) == 1u)
    {
    m_str_ref_p->m_cstr_p[0] = '\0';
    m_str_ref_p->m_length    = 0;
    }
  else // Not unique or not writable
    {
    m_str_ref_p->dereference();
    m_str_ref_p = AStringRef::get_empty();
    m_str_ref_p->m_ref_count++;
    }
  }

//---------------------------------------------------------------------------------------
//  Ensures that the C-String buffer is large enough, writable, and unique
//              (not shared) - copying the data already present in the string.
// Arg          needed_chars - amount of characters that need to be stored not including
//              the null terminating character
// See:         ensure_size_buffer(), ensure_size_empty(), empty(), set_length()
// Author(s):    Conan Reis
A_INLINE void AString::ensure_size(uint32_t needed_chars)  
  {
  AStringRef * str_ref_p = m_str_ref_p;

  // If not big enough or shared or read_only
  if ((needed_chars >= str_ref_p->m_size)
    || ((str_ref_p->m_ref_count + str_ref_p->m_read_only) != 1u))
    {
    // This method does the actual work, but above test separated so that it is a good
    // size for being made A_INLINE.
    set_size(needed_chars);
    }
  }

//---------------------------------------------------------------------------------------
//  Ensures that the C-String buffer is large enough, writable, and unique
//              (not shared).  This method does not maintain data integrity - any data
//              currently present is ignored and the null terminator and length are not
//              modified if the buffer is resized.
// Arg          needed_chars - amount of characters that need to be stored not including
//              the null terminating character
// See:         ensure_size(), ensure_size_empty(), empty(), set_length()
// Notes:       This method may be called prior to modifying the C-String buffer directly
//              via a method like as_cstr_writable().  Make sure that set_length() is
//              called immediately afterwards to ensure the integrity of the AString
//              object's length value and null terminator.
// Author(s):    Conan Reis
A_INLINE void AString::ensure_size_buffer(uint32_t needed_chars)
  {
  AStringRef * str_ref_p = m_str_ref_p;

  // If not big enough or shared or read_only
  if ((needed_chars >= str_ref_p->m_size)
    || ((str_ref_p->m_ref_count + str_ref_p->m_read_only) != 1u))
    {
    uint32_t size = AStringRef::request_char_count(needed_chars);

    m_str_ref_p = m_str_ref_p->reuse_or_new(AStringRef::alloc_buffer(size), 0u, size);
    }
  }

//---------------------------------------------------------------------------------------
//  Ensures that the C-String buffer is large enough, writable, and unique
//              (not shared), then empty the string.
// Arg          needed_chars - amount of characters that need to be stored not including
//              the null terminating character
// See:         ensure_size(), ensure_size_buffer(), empty(), set_length()
// Author(s):    Conan Reis
A_INLINE bool AString::ensure_size_empty(uint32_t needed_chars)  
  {
  AStringRef * str_ref_p = m_str_ref_p;
  bool         resized   = false;

  // If not big enough or shared or read_only
  if ((needed_chars >= str_ref_p->m_size)
    || ((str_ref_p->m_ref_count + str_ref_p->m_read_only) != 1u))
    {
    uint32_t size = AStringRef::request_char_count(needed_chars);

    m_str_ref_p = m_str_ref_p->reuse_or_new(AStringRef::alloc_buffer(size), 0u, size);
    resized = true;
    }

  m_str_ref_p->m_length    = 0u;
  m_str_ref_p->m_cstr_p[0] = '\0';

  return resized;
  }

//---------------------------------------------------------------------------------------
// Ensures that there is enough space in the buffer for the current length
//             of the string plus the extra space specified.
// Author(s):   Conan Reis
A_INLINE void AString::ensure_size_extra(uint32_t extra_chars)
  {
  ensure_size(m_str_ref_p->m_length + extra_chars);
  }

//---------------------------------------------------------------------------------------
//  Ensures that the string is writable and has a unique reference.
// Examples:    str.ensure_writable()
// Notes:       Most non-const methods will call this method.
//              The internal C-String character array may change its address after
//              calling this method.
// Author(s):    Conan Reis
A_INLINE void AString::ensure_writable()
  {
  // If shared or read_only
  if ((m_str_ref_p->m_ref_count + m_str_ref_p->m_read_only) != 1u)
    {
    // This method does the actual work, but above test separated so that it is a good
    // size for being made A_INLINE.
    make_writable();
    }
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from DOS style (\r\n) to RichEdit control style (\r).
//             This method will also work if the string is already in RichEdit style.
// Returns:    Returns the number of line breaks that were converted.
// See:        line_*() functions
// Author(s):   Conan Reis
A_INLINE uint32_t AString::line_break_dos2rich(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  return remove_all('\n', start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from DOS style (\r\n) to Unix style (\n).
//             This method will also work if the string is already in Unix style.
// Returns:    Returns the number of line breaks that were converted.
// See:        line_*() functions
// Author(s):   Conan Reis
A_INLINE uint32_t AString::line_break_dos2unix(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  return remove_all('\r', start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from RichEdit control style (\r) to DOS style (\r\n).
//             This method will *not* work if the string is already in dos style.
// Returns:    Returns the number of line breaks that were converted.
// See:        line_*() functions
// Author(s):   Conan Reis
A_INLINE uint32_t AString::line_break_rich2dos(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  return replace_all('\r', ms_dos_break, start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from RichEdit control style (\r) to Unix style (\n).
//             This method will also work if the string is already in Unix style.
// See:        line_*() functions
// Author(s):   Conan Reis
A_INLINE uint32_t AString::line_break_rich2unix(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  return replace_all('\r', '\n', start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from RichEdit control style (\r) to Unix style (\n).
//             This method will also work if the string is already in Unix style.
// See:        line_*() functions
// Author(s):   Conan Reis
A_INLINE uint32_t AString::line_break_unix2rich(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  return replace_all('\n', '\r', start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
//  Convert any uppercase characters in the string to lowercase.
// See:         uppercase(), as_uppercase(), as_lowercase(), to_lowercase(), not_char_type(),
//              ms_char_match_table
// Notes:       $Revisit - CReis [Completeness] Should allow for operations on substring portion.
// Author(s):    Conan Reis
A_INLINE void AString::lowercase()
  {
  uint32_t length = m_str_ref_p->m_length;

  if (length)
    {
    // $Revisit - CReis This could make a redundant write.
    ensure_writable();

    uint8_t * cstr_p     = (uint8_t *)m_str_ref_p->m_cstr_p;
    uint8_t * cstr_end_p = cstr_p + length;

    while (cstr_p < cstr_end_p)
      {
      *cstr_p = ms_char2lower[*cstr_p];
      cstr_p++;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the character length of this string.  If the AString character buffer
//              was changed externally this will ensure the integrity the AString length.
// Returns:     a reference to itself
// Arg          length - number of characters to use already present in this AString's
//              C-String buffer (the 'buffer') and the index position to place a
//              terminating null character.  The given length must not be more than the
//              current size of the buffer and the buffer should not have any null
//              characters less then the given length.
//              'length' may also be set to ALength_calculate in which case the character
//              length is calculated by finding the first terminating null character
//              already present in the buffer.
//              (Default ALength_calculate)
// See:         crop(), get_length(), remove(), remove_end()
// Notes:       May call ensure_writable()
// Author(s):    Conan Reis
A_INLINE void AString::set_length(
  uint32_t length // = ALength_calculate
  )
  {
  if (length == ALength_calculate)
    {
    m_str_ref_p->m_length = uint32_t(::strlen(m_str_ref_p->m_cstr_p));
    }
  else
    {
    // If length different or terminator not already in place
    if ((m_str_ref_p->m_length != length) || (m_str_ref_p->m_cstr_p[length] != '\0'))
      {
      // Ensure unique, writable, and copy only necessary portion
      ensure_size(length);

      m_str_ref_p->m_length = length;

      // Put in null-terminator
      m_str_ref_p->m_cstr_p[m_str_ref_p->m_length] = '\0';
      }
    }
  }

//---------------------------------------------------------------------------------------
// Reallocates C-String buffer and copies existing string contents making it
//             writable and unique (not shared) regardless of its previous state.
// Notes:      This is useful to make a dynamic buffer for a string that was wrapped
//             around a temporary buffer.
// Author(s):   Conan Reis
A_INLINE void AString::rebuffer()
  {
  set_size(m_str_ref_p->m_length);
  }

//---------------------------------------------------------------------------------------
//  Replace all str with new_str starting at start_pos
// Author(s):    Conan Reis
// Efficiency   See remove_all() efficiency comment for replacements where new_str is
//              shorter than old_str.  It may also be more efficient to apply the same
//              method for the reverse - just determine offset or tally the results.
A_INLINE uint32_t AString::replace_all(
  const AString & old_str,
  const AString & new_str,
  uint32_t        start_pos,  // = 0u
  uint32_t        end_pos,    // = ALength_remainder
  eAStrCase       case_check  // = AStrCase_sensitive
  )
  {
  return replace_all(AStringBM(old_str, case_check), new_str, start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
//  Convert any lowercase characters in the string to uppercase.
// See:         lowercase(), as_uppercase(), as_lowercase(), to_lowercase(), not_char_type(),
//              ms_char_match_table
// Notes:       $Revisit - CReis [Completeness] Should allow for operations on substring portion.
// Author(s):    Conan Reis
A_INLINE void AString::uppercase()
  {
  uint32_t length = m_str_ref_p->m_length;

  if (length)
    {
    // $Revisit - CReis This could make a redundant write.
    ensure_writable();

    uint8_t * cstr_p     = (uint8_t *)m_str_ref_p->m_cstr_p;
    uint8_t * cstr_end_p = cstr_p + length;

    while (cstr_p < cstr_end_p)
      {
      *cstr_p = ms_char2uppper[*cstr_p];
      cstr_p++;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Add-assignment operator for string concatenation
// Returns:     reference to itself to allow for stringization:  str += str1 += str2
// Arg          str - string to append
// Examples:    str
//                += str1
//                += str2
//                += str3;
// See:         append(), add(), operator+(), operator=()
// Notes:       This method is identical to append(), but returns a reference to itself.
// Author(s):    Conan Reis
A_INLINE AString & AString::operator+=(const AString & str)
  {
  // $Revisit - CReis [Clearness] Ensure that
  //    str +=  str1  += str2   is the same as:
  //   (str +=  str1) += str2   and not:
  //    str += (str1  += str2)
  append(str);
  return *this;
  }

//---------------------------------------------------------------------------------------
//  Add-assignment operator for character concatenation
// Returns:     reference to itself to allow for stringization:  str += '1' += '2'
// Arg          str - string to append
// Examples:    str
//                += 'a'
//                += 'b'
//                += 'c';
// See:         append(), add(), operator+(), operator=()
// Notes:       This method is identical to append(), but returns a reference to itself.
// Author(s):    Conan Reis
A_INLINE AString & AString::operator+=(char ch)
  {
  // $Revisit - CReis [Clearness] Ensure that
  //    str +=  '1'  += '2'   is the same as:
  //   (str +=  '1') += '2'   and not:
  //    str += ('1'  += '2')
  append(ch);
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Return a copy of itself that has converted any uppercase characters in
//              the resulting string to lowercase.
// Returns:     lowercase copy of itself
// See:         lowercase(), uppercase(), as_uppercase(), to_lowercase(), not_char_type(),
//              ms_char_match_table
// Notes:       $Revisit - CReis [Completeness] Should allow for operations on substring portion.
// Author(s):    Conan Reis
A_INLINE AString AString::as_lowercase() const
  {
  uint32_t length = m_str_ref_p->m_length;

  if (length)
    {
    AString result(nullptr, length + 1u, 0u);

    uint8_t * rcstr_p    = (uint8_t *)result.m_str_ref_p->m_cstr_p;
    uint8_t * cstr_p     = (uint8_t *)m_str_ref_p->m_cstr_p;
    uint8_t * cstr_end_p = cstr_p + length;

    while (cstr_p < cstr_end_p)
      {
      *rcstr_p = ms_char2lower[*cstr_p];
      cstr_p++;
      }

    *rcstr_p = '\0';  // Put in null-terminator

    result.m_str_ref_p->m_length = length;

    return result;
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
//  Return a copy of itself that has reversed the order of char_count
//              characters starting at pos.
// Returns:     copy of itself with all or a portion of its characters reversed.
// Arg          pos - character index to begin reversing characters.  (Default 0u)
// Arg          char_count - number of characters to reverse.  If char_count is
//              ALength_remainder, the number of characters is equal to the length of the
//              string - pos.  (Default ALength_remainder)
// Examples:    AString str("12345");
//              AString str_rev(str.as_reverse());  // = "54321"
// See:         as_reverse(reversed_p, pos, char_count), reverse(), find_reverse()
// Notes:       as_reverse(reversed_p, pos, char_count) is likely to be more efficient.
// Author(s):    Conan Reis
A_INLINE AString AString::as_reverse(
  uint32_t pos,       // = 0u
  uint32_t char_count // = ALength_remainder
  ) const
  {
  if (char_count == ALength_remainder)  // default
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  AString result(nullptr, uint32_t(char_count + 1u), uint32_t(0u));

  as_reverse(&result, pos, char_count);

  return result;
  }

//---------------------------------------------------------------------------------------
//  Return a copy of itself that has converted any lowercase characters in
//              the resulting string to uppercase.
// Returns:     uppercase copy of itself
// See:         lowercase(), uppercase(), as_lowercase(), to_lowercase(), not_char_type(),
//              ms_char_match_table
// Notes:       $Revisit - CReis [Completeness] Should allow for operations on substring portion.
// Author(s):    Conan Reis
A_INLINE AString AString::as_uppercase() const
  {
  uint32_t length = m_str_ref_p->m_length;

  if (length)
    {
    AString result(nullptr, length + 1u, 0u);

    uint8_t * rcstr_p    = (uint8_t *)result.m_str_ref_p->m_cstr_p;
    uint8_t * cstr_p     = (uint8_t *)m_str_ref_p->m_cstr_p;
    uint8_t * cstr_end_p = cstr_p + length;

    while (cstr_p < cstr_end_p)
      {
      *rcstr_p = ms_char2uppper[*cstr_p];
      cstr_p++;
      }

    *rcstr_p = '\0';  // Put in null-terminator

    result.m_str_ref_p->m_length = length;

    return result;
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
//  Returns number of substrings found
// Author(s):    Conan Reis
/*
A_INLINE uint32_t AString::count2(
  const AString &  str,
  const AStrArgs & args // = AStrArgs::ms_sensitive
  ) const
  {
  // Makes the assumption that the use of a AStringBM outweighs cost of AStringBM
  // construction.
  return count(AStringBM(str, args.m_case_check), args.m_start_pos, args.m_end_pos);
  }
*/

//---------------------------------------------------------------------------------------
//  Returns number of substrings found
// Author(s):    Conan Reis
A_INLINE uint32_t AString::count(
  const AString & str,
  uint32_t        start_pos,  // = 0u
  uint32_t        end_pos,    // = ALength_remainder
  eAStrCase       case_check  // = AStrCase_sensitive
  ) const
  {
  // Makes the assumption that the use of a AStringBM outweighs cost of AStringBM
  // construction.
  return count(AStringBM(str, case_check), start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
//  Returns a substring containing num_char characters from the end of this
//              string.
// Returns:     substring with num_char characters copied from the end of this string.
// Arg          char_count - number of characters to copy from the end.
// See:         get(), get_new()
// Author(s):    Conan Reis
A_INLINE AString AString::get_end(uint32_t char_count) const
  {
  #ifdef A_BOUNDS_CHECK
    span_check(0, char_count, "get_end");
  #endif

  return AString(&m_str_ref_p->m_cstr_p[m_str_ref_p->m_length - char_count], char_count, false);
  }

//---------------------------------------------------------------------------------------
//  Substring at index position in between tokens in current string
//              looking from start_pos to end_pos.
//              Separators that are adjacent to one another count as an empty string.
// Returns:     Substring at index position in between tokens
// Arg          index - index position of substring to return (zero indexed)
// Arg          token - substring to separate on
// Arg          find_pos_p - pointer to store index position in current string
//              where returned substring is found.  Not set if nullptr
// Arg          start_pos - index position in current string to begin looking
// Arg          end_pos - index position in current string to stop looking
//              (ALength_remainder = current length of string)
// Arg          case_check - whether or not the token should be case sensitive or not
// Examples:    AString str("one, two, three, four");
//              AString third(str.get_token(2));   // = "three"
// Notes:       If the index given is out of range or if the specified index is
//              between two separators, an empty string is returned.
//              If the index is out of range, the value find_pos_p points to is
//              not changed.
// Author(s):    Conan Reis
A_INLINE AString AString::get_token(
  uint32_t        index,      // = 0
  const AString & token,      // = ", "
  uint32_t *      find_pos_p, // = nullptr
  uint32_t        start_pos,  // = 0
  uint32_t        end_pos,    // = ALength_remainder
  eAStrCase       case_check  // = AStrCase_sensitive
  ) const
  {
  return get_token(index, AStringBM(token, case_check), find_pos_p, start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Determines if this string is empty - i.e. it has no characters other than
//             its null terminator (length == 0).
// Returns:    true if empty, false if not
// Examples:   if (str.is_empty())
//               do_stuff();
// See:        is_filled(), get_length(), empty(), get_size()
// Author(s):   Conan Reis
A_INLINE bool AString::is_empty() const
  {
  return (m_str_ref_p->m_length == 0u);
  }

//---------------------------------------------------------------------------------------
// Determines if this string has characters - i.e. it is not empty and has
//             more characters than its null terminator (length > 0).
// Returns:    true if empty, false if not
// Examples:   if (str.is_filled())
//               do_stuff();
// See:        is_empty(), get_length(), empty(), get_size()
// Author(s):   Conan Reis
A_INLINE bool AString::is_filled() const
  {
  return (m_str_ref_p->m_length > 0u);
  }

//---------------------------------------------------------------------------------------
//  Write string to the specified output stream.
// Returns:     reference to the output stream to allow for stringization
// Arg          ostrm - output stream to write to
// Arg          str - string to write
// Examples:    cout << str;
// Author(s):    Conan Reis
//A_INLINE ostream & operator<<(
//  ostream &       ostrm,
//  const AString & str
//  )
//  {
//  if (str.m_str_ref_p->m_length)
//    {
//    ostrm << str.m_str_ref_p->m_cstr_p;
//    }
//  return ostrm;
//  }

//---------------------------------------------------------------------------------------
// Returns pointer to first non-space (not ` ` or `\t`) character and calculates column
// position taking into account tab size.
// 
// Returns:   pointer to first non-space (not ` ` or `\t`) character in `cstr_p`
// Params:  
//   cstr_p:     string to search
//   cstr_end_p: end position of string to search
//   tab_stops:  tab stop count in spaces (assuming monospace/fixed-space font)
//   column_p:
//     address to store 0-based column relative to `cstr_p`. Ignored if nullptr.
//     
// Author(s):   Conan Reis
A_INLINE const char * AString::advance_to_indent(
  const char * cstr_p,
  const char * cstr_end_p,
  uint32_t     tab_stops,
  uint32_t *   column_p
  )
  {
  uint32_t column = 0u;

  // Find first non-space
  while ((cstr_p < cstr_end_p)
    && ((*cstr_p == ' ') || (*cstr_p == '\t')))
    {
    column += (*cstr_p == '\t') ? (tab_stops - (column % tab_stops)) : 1u;
    cstr_p++;
    }

  *column_p = column;

  return cstr_p;
  }

//---------------------------------------------------------------------------------------
// Returns `cstr_p` specified number of columns ahead. It takes into account tab stops
// and any encountered tab `\t` characters.
// 
// Returns:   `cstr_p` specified number of columns ahead
// Params:  
//   column:    0-based column offset to advance
//   cstr_p:    start string to advance - i.e. column 0 & usually start of row
//   tab_stops: tab stop count in spaces
//
// Notes:  
//   If column count is beyond current line (marked with `\r`, `\n` or null terminator)
//   then the end of the line is returned.
//   
//   *Range* There is no check if the column is outside the range of the string if it is
//   not null-terminated!
//
// Author(s):   Conan Reis
A_INLINE const char * AString::advance_to_column(
  uint32_t     column,
  const char * cstr_p,
  uint32_t     tab_stops
  )
  {
  uint32_t current_column = 0u;

  while (current_column < column)
    {
    switch (*cstr_p)
      {
      case '\t':
        current_column += tab_stops - (current_column % tab_stops);
        break;

      case '\r':
      case '\n':
      case '\0':
        // Found end of line or end of string
        return cstr_p;

      default:
        // Regular character
        current_column++;
      }

    cstr_p++;
    }

  return cstr_p;
  }

//---------------------------------------------------------------------------------------
// Counts number of columns taking into account tab stops and any encountered tab `\t`
// characters between `row_cstr_p` and `column_cstr_p`
// 
// Returns:   number of 0-based columns between `row_cstr_p` and `column_cstr_p`
// Params:  
//   row_cstr_p:    start of "row" in string - i.e. column 0
//   column_cstr_p: position in string to determine column value
//   tab_stops:     tab stop count in spaces
//     
// Author(s):   Conan Reis
A_INLINE uint32_t AString::count_columns(
  const char * row_cstr_p,
  const char * column_cstr_p,
  uint32_t     tab_stops
  )
  {
  uint32_t column = 0u;

  while (row_cstr_p < column_cstr_p)
    {
    column += (*row_cstr_p == '\t') ? (tab_stops - (column % tab_stops)) : 1u;
    row_cstr_p++;
    }

  return column;
  }

//---------------------------------------------------------------------------------------
// Convert a character index to a row / line number (and optional column number).
// 
// Returns:   0-based row/line number
// Params:  
//   idx: 0-based character index
//   row_idx_p:
//     index offset from start of row - like column but only counts characters and does
//     not take tab characters into account. Ignored if nullptr.
//   column_p:
//     address to store 0-based display column (taking tabs `\t` into account and assuming
//     a monospace/fixed-width font). Ignored if nullptr.
//   tab_stops: tab stop count in spaces (assuming monospace/fixed-space font)
//
// Notes:  
//   Assumes that the string is in Unix line ending format (newline or '\n').
//   [DOS line ending \r\n should also work fine since it ends with newline.]
//   
// See:         row_to_index()
// Author(s):   Conan Reis
A_INLINE uint32_t AString::index_to_row(
  uint32_t   idx,
  uint32_t * row_idx_p, // = nullptr
  uint32_t * column_p,  // = nullptr
  uint32_t   tab_stops  // = AString_tab_stop_def
  ) const
  {
  uint32_t row     = 0u;
  uint32_t row_idx = 0u;

  if (idx && m_str_ref_p->m_length)
    {
    // Use 1 character less so that it is not counted if it is a newline
    row = count('\n', 0u, idx - 1u, &row_idx);

    if (row)
      {
      // Skip past \n
      row_idx++;
      }
    }

  if (row_idx_p)
    {
    *row_idx_p = idx - row_idx;
    }

  if (column_p)
    {
    const char * cstr_p = m_str_ref_p->m_cstr_p;

    *column_p = count_columns(cstr_p + row_idx, cstr_p + idx, tab_stops);
    }

  return row;
  }

//---------------------------------------------------------------------------------------
// Convert a character index to a column - taking tabs into account.
// 
// Returns:   0-based display column (taking tabs `\t` into account and assuming
//     a monospace/fixed-width font).
// Params:  
//   idx: 0-based character index
//   tab_stops: tab stop count in spaces (assuming monospace/fixed-space font)
//
// Notes:  
//   Use `index_to_row()` if you want both row and column.
//   Assumes that the string is in Unix line ending format (newline or '\n').
//   [DOS line ending \r\n should also work fine since it ends with newline.]
//   
// See:         find_indent_column(), index_to_row(), row_to_index()
// Author(s):   Conan Reis
A_INLINE uint32_t AString::index_to_column(
  uint32_t idx,
  uint32_t tab_stops  // = AString_tab_stop_def
  ) const
  {
  if ((idx == 0u) || (m_str_ref_p->m_length == 0u))
    {
    return 0u;
    }

  // Find index for start of row
  uint32_t row_idx = 0u;

  if (find_reverse('\n', 1u, &row_idx, 0u, idx))
    {
    // Skip past \n
    row_idx++;
    }

  // Determine column position
  const char * cstr_p = m_str_ref_p->m_cstr_p;

  return count_columns(cstr_p + row_idx, cstr_p + idx, tab_stops);
  }

//---------------------------------------------------------------------------------------
// Convert a row / line number (and optional 0-based column which takes tabs
// into account) to a character index.
// 
// Returns:   row character index (plus column) or length of string if row not found
// Params:  
//   row:       0-based row/line number
//   column:    0-based column number taking tabs `\t` into account
//   tab_stops: tab stop count in spaces (assuming monospace/fixed-space font)
//   
// Notes:  
//   Assumes that the string is in Unix line ending format (newline or '\n').
//   [DOS line ending \r\n should also work fine since it ends with newline.]
//   
// See:         index_to_row()
// Author(s):   Conan Reis
A_INLINE uint32_t AString::row_to_index(
  uint32_t row,
  uint32_t column,   // = 0u
  uint32_t tab_stops // = AString_tab_stop_def
  ) const
  {
  uint32_t row_idx = 0u;

  if (row != 0u)
    {
    if (!find('\n', row, &row_idx))
      {
      return m_str_ref_p->m_length;
      }

    row_idx += 1u;
    }

  if (column == 0u)
    {
    return row_idx;
    }

  return uint(advance_to_column(column, m_str_ref_p->m_cstr_p + row_idx, tab_stops)
    - m_str_ref_p->m_cstr_p);
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Compares two characters and ignores their case.
// Returns:     0 if true, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
// Arg          lhs - left hand side character to compare
// Arg          rhs - right hand side character to compare
// See:         to_lowercase(), not_char_type(), ms_char_match_table
// Author(s):    Conan Reis
A_INLINE int AString::compare_insensitive(
  char lhs,
  char rhs
  )
  {
  // $Revisit - CReis Profile - Should be faster than: to_lowercase(lhs) compare_op to_lowercase(rhs)

  return ms_char2lower[static_cast<uint8_t>(lhs)] - ms_char2lower[static_cast<uint8_t>(rhs)];
  }

//---------------------------------------------------------------------------------------
//  Negates the character match type.
// Returns:     the negation of the given character match type
// Arg          eACharMatch match_type - character match type to negate
// Examples:    eACharMatch not_match_type = not_char_tyoe(match_type)
//              // not_char_tyoe(ACharMatch_white_space)     = ACharMatch_not_white_space
//              // not_char_tyoe(ACharMatch_not_white_space) = ACharMatch_white_space
// See:         init_match_table()
// Author(s):    Conan Reis
A_INLINE eACharMatch AString::not_char_type(eACharMatch match_type)
  {
  return static_cast<eACharMatch>(
    (match_type < ACharMatch__not_start) ?
      (match_type + ACharMatch__not_start) :
      (match_type - ACharMatch__not_start));
  }

//---------------------------------------------------------------------------------------
//  Returns lowercase version of ch if it is uppercase or ch if it is not.
// Returns:     lowercase version of ch if it is uppercase or ch if it is not.
// Arg          ch - character to convert to uppercase
// Author(s):    Conan Reis
A_INLINE char AString::to_lowercase(char ch)
  {
  // $Revisit - CReis Profile - Should be faster than tolower() in the standard library.

  return ms_char2lower[static_cast<uint8_t>(ch)];
  }


//#######################################################################################
// AStringBM Class
//#######################################################################################


//=======================================================================================
// Inline Functions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Get delta value for a given character.
// Author(s):   Conan Reis
A_INLINE uint32_t AStringBM::get_delta(char ch) const
  {
  return m_delta_p[uint8_t(ch)];
  }

//---------------------------------------------------------------------------------------
// Get char from pattern string.
// Author(s):   Conan Reis
A_INLINE char AStringBM::operator[](int index) const
  {
  return m_str_ref_p->m_cstr_p[index];
  }



