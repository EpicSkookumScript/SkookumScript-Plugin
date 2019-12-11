// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Dynamic AString class definition module
// Notes:          The AString class should be used in the place of standard
//              C-String character array pointers. 
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/AString.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/AString.inl>
#endif
#include <AgogCore/AObjReusePool.hpp>
#include <AgogCore/APArray.hpp>
#include <stdio.h>      // Uses:  _vsnprintf, _snprintf
#include <stdlib.h>     // Uses:  wcstombs
#include <stdarg.h>     // Uses:  va_start, va_end
#include <wchar.h>      // Uses:  wcslen

#include "Containers/StringConv.h" // For TCHAR conversions

#ifdef A_PLAT_PC
  #define WIN32_LEAN_AND_MEAN // Keep this define out of public header files
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wchar-subscripts" // Allow char-typed indices into arrays
#endif


//=======================================================================================
// AString Class Data Members
//=======================================================================================

// Defined in AgogCore/AgogCore.cpp


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

void AString::initialize()
  {
  // Initialize pool of string refs
  AStringRef::get_pool().reset(AgogCore::get_app_info()->get_pool_init_string_ref(), AgogCore::get_app_info()->get_pool_incr_string_ref());

  // Initialize constants
  const_cast<AString&>(ms_comma) = ",";
  const_cast<AString&>(ms_dos_break) = "\r\n";
  }

//---------------------------------------------------------------------------------------

void AString::deinitialize()
  {
  // Deinitialize constants
  const_cast<AString&>(ms_comma) = AString::ms_empty;
  const_cast<AString&>(ms_dos_break) = AString::ms_empty;

  // Get rid of pool memory
  AStringRef::get_pool().empty();
  }

//---------------------------------------------------------------------------------------

bool AString::is_initialized()
  {
  return !ms_comma.is_empty();
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Constructor from character array buffer.  Refers to a persistent
//             null-terminated C-String or makes its own copy of an existing character
//             array.
// Returns:    itself
// Arg         cstr_p - pointer to array of characters.  'cstr_p' should never be nullptr.
//             'cstr_p' will usually be a string literal or if 'persistent' is false,
//             'cstr_p' may be any C-String that this string should make a copy of.
// Arg         persistent - Indicates whether the data pointed to by cstr_p will be
//             available for the lifetime of this string.  If 'persistent' is true, the
//             memory pointed to by 'cstr_p' will be used rather than having this object
//             allocate its own memory - also the memory pointed to by 'cstr_p' will not
//             be written to.  If 'persistent' is false, then this AString object will
//             allocate its own memory and copy the contents of 'cstr_p' to it.
//             (Default true)
// See:        AString(cstr_p), AString(buffer_p, size, length, deallocate)
// Notes:      If any modifying methods are called on this string, it will first make
//             its own unique copy of the C-String before it writes to it.
// Author(s):   Conan Reis
AString::AString(
  const char * cstr_p,
  bool         persistent
  )
  {
  //A_ASSERT(cstr_p != nullptr, "Given nullptr instead of valid C-String", ErrId_null_cstr, AString);

  uint32_t length = uint32_t(::strlen(cstr_p));

  m_str_ref_p = ((persistent) && (cstr_p[length] == '\0'))
    ? AStringRef::pool_new(cstr_p, length, length + 1u, 1u, false, true)
    : AStringRef::pool_new_copy(cstr_p, length);
  }

//---------------------------------------------------------------------------------------
// Constructor from character array buffer.  Refers to a persistent
//             null-terminated C-String or makes its own copy of an existing character
//             array.
// Returns:    itself
// Arg         cstr_p - pointer to array of characters (does not need to be null
//             terminated unless length is equal to ALength_calculate).  'cstr_p' should
//             never be nullptr.  'cstr_p' will usually be a string literal or if
//             'persistent' is false, 'cstr_p' may be any C-String that this string
//             should make a copy of.
// Arg         length - number of characters to use in 'cstr_p' and the index position to
//             place a terminating null character.  The given length must not be more
//             than the size of 'cstr_p' and the C-String buffer pointed to by 'cstr_p'
//             should not have any null characters less then the given length.  A null
//             terminator is placed only if 'persistent' is not true.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in 'cstr_p'.
// Arg         persistent - Indicates whether the data pointed to by cstr_p will be
//             available for the lifetime of this string.  If 'persistent' is true, the
//             memory pointed to by 'cstr_p' will be used rather than having this object
//             allocate its own memory - also the memory pointed to by 'cstr_p' will not
//             be written to.  If 'persistent' is false, then this AString object will
//             allocate its own memory and copy the contents of 'cstr_p' to it.
// See:        AString(cstr_p), AString(buffer_p, size, length, deallocate)
// Notes:      If any modifying methods are called on this string, it will first make
//             its own unique copy of the C-String before it writes to it.
// Author(s):   Conan Reis
AString::AString(
  const char * cstr_p,
  uint32_t     length,
  bool         persistent // = true
  )
  {
  //A_ASSERT(cstr_p != nullptr, "Given nullptr instead of valid C-String", ErrId_null_cstr, AString);

  if (length == ALength_calculate)
    {
    length = uint32_t(::strlen(cstr_p));
    }

  m_str_ref_p = ((persistent) && (cstr_p[length] == '\0'))
    ? AStringRef::pool_new(cstr_p, length, length + 1u, 1u, false, true)
    : AStringRef::pool_new_copy(cstr_p, length);
  }

//---------------------------------------------------------------------------------------
// Constructor from character array buffer.  See the description of the
//             argument 'buffer_p' for the three possible uses of this constructor.
// Returns:    itself
// Arg         buffer_p - pointer to array of characters (does not need to be null
//             terminated unless 'length' is equal to ALength_calculate).  If 'buffer_p'
//             is nullptr, 'length' and 'deallocate' are ignored.  There are in general 3
//             different types of values that 'buffer_p' will be given:
//               1. A temporary buffer which is allocated on the stack or is recovered
//                  by some other means.
//               2. A pre-allocated character array that this AString object should take
//                  over responsibility for.
//               3. nullptr.  This then creates an empty string with a space of 'size'
//                  already allocated including the null terminating character.
// Arg         size - This is the size in bytes of the memory pointed to by 'buffer_p'
//             or if 'buffer_p' is nullptr, it is the initial memory size that should be 
//             allocated by this string.  Also note that this size must include the null
//             terminating character, so 'size' should never be less than 1 and it
//             should always be greater then the length.
// Arg         length - number of characters to use in 'buffer_p' and the index position
//             to place a terminating null character.  The given length must not be more
//             than the size of 'buffer_p' and the C-String buffer pointed to by
//             'buffer_p' should not have any null characters less then the given length.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in 'buffer_p'.
// Arg         deallocate - Indicates whether this string (or more specifically the
//             AStringRef) should take control of the memory pointed to by buffer_p and
//             do deallocate it when it is no longer in use.
// See:        AString(cstr_p), AString(cstr_p, length, persistent)
// Notes:      To make a copy of a C-String call AString(cstr_p, length, persistent = false).
//             To reference a C-String literal call AString(cstr_p) or
//             AString(cstr_p, length, persistent = true)
// Author(s):   Conan Reis
AString::AString(
  const char * buffer_p,
  uint32_t     size,
  uint32_t     length,
  bool         deallocate // = false
  )
  {
  if (buffer_p)
    {
    if (length == ALength_calculate)
      {
      length = uint32_t(::strlen(buffer_p));
      }

    A_ASSERT(size > length, a_cstr_format("Supplied size (%u) is not large enough for supplied length (%u)", size, length), ErrId_null_cstr, AString);

    m_str_ref_p = AStringRef::pool_new(buffer_p, length, size, 1u, deallocate, false);
    m_str_ref_p->m_cstr_p[length] = '\0';  // Put in null-terminator
    }
  else  // nullptr, so create empty AString with specified buffer size
    {
    size        = AStringRef::request_char_count(size);
    m_str_ref_p = AStringRef::pool_new(AStringRef::alloc_buffer(size), 0u, size, 1u, true, false);
    m_str_ref_p->m_cstr_p[0] = '\0';  // Put in null-terminator
    }
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from a formatted string.
// Returns:    a reference to itself
// Arg         max_size - sets the initial character buffer size (not including the null
//             terminator character).  This is essentially the maximum allowable
//             characters - if the number of the characters in the expanded format
//             string are larger than 'max_size' then any characters above and beyond 
//             that number will be truncated.
// Arg         format_str_p - follows the same format as the C printf(), sprintf(), etc.
//             See the MSDev online help for 'Format Specification Fields' for a
//             description. 
// Arg         ... - variable length arguments expected by the formatted string.
//
//             #### IMPORTANT #### When using the string format type specifier (%s) in
//             the format string and a AString object (or a ASymbol object) is passed in
//             as one of the variable length arguments, use 'str.as_cstr()'.
//             Variable length arguments do not specify a type to the compiler, instead
//             the types of the passed arguments are determined via the standard C
//             string formatting functions - i.e %s, %u, %f, etc.  So if a string type
//             (%s) is specified in the format string and a AString object is passed as
//             one of the untyped variable length arguments, the compiler will not know
//             to coerce the AString object to a C-String - it will instead treat the
//             address of the AString object argument as if it were a C-String character
//             array with disastrous results.
//
// Examples:   AString begin("The answer");  // Example substring to append
//             AString str(100u, "%s to %s is %i", begin.as_cstr(), "everything", 42);
//             // str is equal to "The answer to everything is 42"
//             // character length is 30, buffer size is 101
//
// See:        format(), append_format()
// Author(s):   Conan Reis
AString::AString(
  uint32_t     max_size,
  const char * format_str_p,
  ...
  )
  {
  va_list  args;  // initialize argument list
  uint32_t size   = AStringRef::request_char_count(max_size);
  char *   cstr_p = AStringRef::alloc_buffer(size);  // allocate buffer

  va_start(args, format_str_p);

  #if defined(A_PLAT_PS3) || defined(A_PLAT_PS4)
    int length = vsnprintf(cstr_p, size_t(max_size + 1), format_str_p, args);
  #else
    int length = _vsnprintf(cstr_p, size_t(max_size), format_str_p, args);
  #endif

  va_end(args);  // end argument list processing

  if ((length == -1) || (length == int(max_size))) // More characters than buffer has, so truncate
    {
    length           = int(max_size);
    cstr_p[max_size] = '\0';     // Put in null-terminator
    }

  m_str_ref_p = AStringRef::pool_new(
    cstr_p,
    uint32_t(length),
    size,
    1u,
    true,
    false);
  }

//---------------------------------------------------------------------------------------
// Constructor from byte stream info
// Arg         source_stream_pp - Pointer to address to read byte stream / binary
//             serialization info from and to increment - previously filled using
//             as_binary() or a similar mechanism.
// See:        as_binary(), assign_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - string length
//               n bytes - string
// Author(s):    Conan Reis
AString::AString(const void ** source_stream_pp)
  {
  // 4 bytes - string length
  uint32_t length = A_BYTE_STREAM_UI32_INC(source_stream_pp);
  uint32_t size   = AStringRef::request_char_count(length);

  m_str_ref_p = AStringRef::pool_new(
    AStringRef::alloc_buffer(size), // C-String
    length,                         // Length
    size,                           // Size
    1u,                             // References
    true,                           // Deallocate
    false);                         // Not Read-Only

  // n bytes - string
  memcpy(m_str_ref_p->m_cstr_p, *(char **)source_stream_pp, length);
  m_str_ref_p->m_cstr_p[length] = '\0';
  (*(uint8_t **)source_stream_pp) += length;
  }

//---------------------------------------------------------------------------------------
// Assignment from byte stream info
// Arg         source_stream_pp - Pointer to address to read byte stream / binary
//             serialization info from and to increment - previously filled using
//             as_binary() or a similar mechanism.
// See:        as_binary(), assign_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - string length
//               n bytes - string
// Author(s):    Conan Reis
AString & AString::assign_binary(const void ** source_stream_pp)
  {
  // 4 bytes - string length
  uint32_t length = A_BYTE_STREAM_UI32_INC(source_stream_pp);

  // n bytes - string
  set_cstr(*(char **)source_stream_pp, length, false);
  (*(uint8_t **)source_stream_pp) += length;

  return *this;
  }

//---------------------------------------------------------------------------------------
// APArrayLogical<AString> constructor.  Concatenates all elements with separator 
//             between them.
// Returns:    itself
// Arg         strings - Array of AString objects to convert to a single string.  Ensure
//             that none of the pointers are nullptr.
// Arg         separator - This is the separator to insert between each string of the
//             array.  (Default no separator - all the Strings are squished together)
// See:        tokenize(), get_token()
// Notes:      No separator is put in front of the first element.
// Author(s):   Conan Reis
AString::AString(
  const APArrayLogical<AString> & strings,
  const AString &                 separator // = ms_empty
  )
  {
  uint32_t length = strings.get_length();

  if (length)
    {
    uint32_t   total_length = ((length - 1u) * separator.m_str_ref_p->m_length);  // separators
    AString ** array_p      = strings.get_array();  // for faster than class member access
    AString ** array_end_p  = array_p + length;

    // Accumulate string lengths to pre-allocate sufficient cstr array
    for (; array_p < array_end_p; array_p++)
      {
      total_length += (*array_p)->m_str_ref_p->m_length;
      }

    uint32_t size   = AStringRef::request_char_count(total_length);
    char *   cstr_p = AStringRef::alloc_buffer(size);

    m_str_ref_p = AStringRef::pool_new(cstr_p, total_length, size, 1u, true, false);

    // Accumulate strings
    total_length = 0u;
    length       = separator.m_str_ref_p->m_length;
    array_p      = strings.get_array();

    if (length)
      {
      const char * seperator_cstr_p = separator.m_str_ref_p->m_cstr_p;

      array_end_p = array_p + (length - 1);  // Last string is not followed with separator

      for (; array_p < array_end_p; array_p++)
        {
        memcpy(cstr_p + total_length, (*array_p)->m_str_ref_p->m_cstr_p, size_t((*array_p)->m_str_ref_p->m_length));
        total_length += (*array_p)->m_str_ref_p->m_length;
        memcpy(cstr_p + total_length, seperator_cstr_p, size_t(length));
        total_length += length;
        }

      memcpy(cstr_p + total_length, (*array_p)->m_str_ref_p->m_cstr_p, size_t((*array_p)->m_str_ref_p->m_length + 1u));
      }
    else  // No separator
      {
      for (; array_p < array_end_p; array_p++)
        {
        memcpy(cstr_p + total_length, (*array_p)->m_str_ref_p->m_cstr_p, size_t((*array_p)->m_str_ref_p->m_length));
        total_length += (*array_p)->m_str_ref_p->m_length;
        }

      m_str_ref_p->m_cstr_p[m_str_ref_p->m_length] = '\0';
      }
    }
  else  // Array strings is empty
    {
    m_str_ref_p = AStringRef::get_empty();
    m_str_ref_p->m_ref_count++;
    }
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from a signed integer
// Returns:    String version of the number
// Arg         integer - signed integer to convert to a string
// Arg         base - base (or radix) to use for conversion.  10 is decimal, 16 is
//             hexadecimal, 2 is binary, etc.
// Examples:   str = AString::ctor_int(5);
// See:        as_int32(), as_uint32_t(), as_float64(), AString(max_size, format_str_p, ...)
// Modifiers:   static
// Author(s):   Conan Reis
AString AString::ctor_int(
  int integer,
  uint base // = AString_def_base (10)
  )
  {
  uint32_t size   = AStringRef::request_char_count(AString_int32_max_chars);
  char *   cstr_p = AStringRef::alloc_buffer(size);

  // $Revisit - CReis Should probably write custom _itoa()
  // This should only be called during development, so don't worry too much for now.
  #ifndef A_NO_NUM2STR_FUNCS
    ::_itoa(integer, cstr_p, int(base));
  #else
    // $Vital - CReis Base is ignored
    ::_snprintf(cstr_p, AString_int32_max_chars - 1, "%i", integer);
  #endif


  return AStringRef::pool_new(cstr_p, uint32_t(::strlen(cstr_p)), size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from an unsigned integer
// Returns:    itself
// Arg         natural - unsigned integer to convert to a string
// Arg         base - base (or radix) to use for conversion.  10 is decimal, 16 is
//             hexadecimal, 2 is binary, etc.
// Examples:   str = AString::ctor_uint(5u);
// See:        as_int32(), as_uint32_t(), as_float64(), AString(max_size, format_str_p, ...)
// Modifiers:   static
// Author(s):   Conan Reis
AString AString::ctor_uint(
  uint32_t natural,
  uint32_t base // = AString_def_base (10)
  )
  {
  uint32_t size   = AStringRef::request_char_count(AString_int32_max_chars);
  char *   cstr_p = AStringRef::alloc_buffer(size);

  // $Revisit - CReis Should probably write custom _itoa()
  // This should only be called during development, so don't worry too much for now.
  #ifndef A_NO_NUM2STR_FUNCS
    ::_ultoa(natural, cstr_p, int(base));
  #else
    // $Vital - CReis Base is ignored
    ::_snprintf(cstr_p, AString_int32_max_chars - 1, "%u", natural);
  #endif

  return AStringRef::pool_new(cstr_p, uint32_t(::strlen(cstr_p)), size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from a f32 (float)
// Returns:    itself
// Arg         real - f32 to convert to a string
// Arg         significant - number of significant digits / characters to attempt to fit
//             'real' into.  If it can't fit, it will use a scientific notation with a
//             lowercase 'e'.
// Examples:   str = AString::ctor_f32(5.0f);
// See:        as_int32(), as_uint32_t(), as_float64(), AString(max_size, format_str_p, ...)
// Modifiers:   explicit
// Author(s):   Conan Reis
AString AString::ctor_float(
  f32      real,
  uint32_t significant // = AString_float_sig_digits_def
  )
  {
  uint32_t size   = AStringRef::request_char_count(significant + AString_real_extra_chars);
  char *   cstr_p = AStringRef::alloc_buffer(size);  // for sign, exponent, etc.

  #ifndef A_NO_NUM2STR_FUNCS
    // $Revisit - CReis change this to _fcvt() if _fcvt() is really more efficient for floats - it still takes a f64???
    ::_gcvt(real, int(significant), cstr_p);
  #else
    _snprintf(cstr_p, significant + AString_real_extra_chars, "%g", f64(real));
  #endif

  uint32_t     length    = uint32_t(::strlen(cstr_p));
  AStringRef * str_ref_p = AStringRef::pool_new(cstr_p, length, size, 0u, true, false);

  // Ensure that it ends with a digit
  if (cstr_p[length - 1u] == '.')
    {
    cstr_p[length]      = '0';
    cstr_p[length + 1u] = '\0';
    str_ref_p->m_length++;
    }

  return str_ref_p;
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from a f64 (double)
// Returns:    itself
// Arg         real - f64 to convert to a string
// Arg         significant - number of significant digits / characters to attempt to fit
//             'real' into.  If it can't fit, it will use a scientific notation with a
//             lowercase 'e'.
// Examples:   str = AString::ctor_f64(5.0);
// See:        as_int32(), as_uint32_t(), as_float64(), AString(max_size, format_str_p, ...)
// Modifiers:   static
// Author(s):   Conan Reis
AString AString::ctor_float64(
  f64      real,
  uint32_t significant // = AString_double_sig_digits_def
  )
  {
  uint32_t size   = AStringRef::request_char_count(significant + AString_real_extra_chars);
  char *   cstr_p = AStringRef::alloc_buffer(size);  // for sign, exponent, etc.

  #ifndef A_NO_NUM2STR_FUNCS
    // $Revisit - CReis change this to _fcvt() if _fcvt() is really more efficient for floats - it still takes a f64???
    ::_gcvt(real, int(significant), cstr_p);
  #else
    _snprintf(cstr_p, significant + AString_real_extra_chars, "%g", real);
  #endif

  uint32_t     length    = uint32_t(::strlen(cstr_p));
  AStringRef * str_ref_p = AStringRef::pool_new(cstr_p, length, size, 0u, true, false);

  // Ensure that it ends with a digit
  if (cstr_p[length - 1u] == '.')
    {
    cstr_p[length]      = '0';
    cstr_p[length + 1u] = '\0';
    str_ref_p->m_length++;
    }

  return str_ref_p;
  }

//---------------------------------------------------------------------------------------
// Constructor / converter from stream.
// Returns:    a reference to itself
// Author(s):   Conan Reis
//AString::AString(strstream & strm)
//  {
//  strm << ends;                    // terminate stream with a null
//  m_str_ref_p->m_length = uint32_t(::strlen(strm.str()));   // get number of characters in stream
//  m_str_ref_p->m_size   = m_str_ref_p->m_length + 1;         // calculate size
//  m_str_ref_p->m_cstr_p = AStringRef::alloc_buffer(m_str_ref_p->m_size);
//  memcpy(m_str_ref_p->m_cstr_p, strm.str(), size_t(m_str_ref_p->m_length + 1));  // copy characters from stream
//  }

//---------------------------------------------------------------------------------------
// Converter / constructor from wide character (Unicode) C-String.
// 
// Params:  
//   tchar_p: UE4 TCHAR
//   length: number of characters to copy from the input string in tchar_p (not bytes). If 0, will copy whole string.
//   
// Author(s):   Zachary Burke
AString::AString(
  const TCHAR * tchar_p,
  uint32_t        length
  )
  {
  if (tchar_p)
    {
    FTCHARToUTF8 Convert(tchar_p);
    const char * Converted = Convert.Get(); // Adds a null-terminator to the output

    if (length == 0)
      {
      // Since we were called with a zero length, we should set the length to that 
      // of the entire string. This method gets # characters in string excluding 
      // any null terminator.
      length = Convert.Length();
      }

    if (Converted && length)
      {
      uint32_t size   = AStringRef::request_char_count(length); // adds space for null terminator
      char *   cstr_p = AStringRef::alloc_buffer(size);

      // Copy the desired length from the converted string to our destination pointer.
      memcpy(cstr_p, Converted, length);

      // Always add a null terminator as last char. We could have copied it above from 
      // Converted for the length == 0 case, but this way the logic is smaller.
      cstr_p[length] = '\0';

      // pool_new expects length to be the string length excluding the null terminator
      m_str_ref_p    = AStringRef::pool_new(cstr_p, length, size, 1u, true, false);

      return;
      }
    }

  m_str_ref_p = AStringRef::get_empty();
  m_str_ref_p->m_ref_count++;
  }

//#ifdef ASTR_ENABLE_WIDE_CHAR
//---------------------------------------------------------------------------------------
// Cast / convert a string to a wide character (Unicode) C-String.
// Notes:      ***Warning*** returns dynamic data
// Author(s):   Conan Reis
//AString::operator const wchar_t * () const
//  {
//  wchar_t * buffer_p = new wchar_t[m_str_ref_p->m_length + 1];
//
//  A_VERIFY_MEMORY(buffer_p == nullptr, AString);
//  mbstowcs(buffer_p, m_str_ref_p->m_cstr_p, size_t(m_str_ref_p->m_length + 1));
//  return buffer_p;
//  // $Revisit - CReis Should be something like this so memory is managed: return WString(*this);
//  // Alternatively use a big reusable buffer for quick and dirty converts.
//  }
//#endif

//---------------------------------------------------------------------------------------
// Stream assignment - allows assignment stringization S = S = S
// Returns:    itself
// Arg         strstream & strm - 
// Author(s):   Conan Reis
//AString & AString::operator=(strstream & strm)
//  {
//  strm << ends;                                 // terminate stream with a null
//  m_str_ref_p->m_length = ::strlen(strm.str());                // get number of characters in stream
//  ensure_size_buffer(m_str_ref_p->m_length);
//  memcpy(m_str_ref_p->m_cstr_p, strm.str(), size_t(m_str_ref_p->m_length + 1));  // copy characters from stream
//  return *this;
//  }

//---------------------------------------------------------------------------------------
// Converter to f64.
// Returns:    a f64 interpretation of the specified section
// Arg         start_pos - index position to start parsing string.
// Arg         stop_pos_p - Address to store the index position where conversion stops.
//             If it is set to nullptr, it is ignored.  (Default nullptr)
// Examples:   f64 num = num_str.as_float64();
// See:        AString(real, significant), as_int32(), as_uint32_t()
// Notes:      The acceptable string form is as following:
//
//             [whitespace] [sign] [digits] [.digits] [{d | D | e | E} [sign] digits]
//
//             Whitespace may consist of space and tab characters, which are ignored.
//             sign is either plus (+) or minus (-) and digits are one or more decimal
//             digits.  If no digits appear before the radix character, at least one
//             must appear after the radix character.  The decimal digits can be
//             followed by an exponent, which consists of an introductory letter
//             (d, D, e, or E) and an optionally signed integer.  If neither an exponent
//             part nor a radix character appears, a radix character is assumed to
//             follow the last digit in the string.  The first character that does not
//             fit this form stops the scan.
// Author(s):   Conan Reis
f64 AString::as_float64(
  uint32_t   start_pos,  // = 0u
  uint32_t * stop_pos_p  // = nullptr
  ) const
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, "as_float64");
  #endif

  char * stop_char_p;
  f64    value = ::strtod(&m_str_ref_p->m_cstr_p[start_pos], &stop_char_p);  // convert

  if (stop_pos_p)
    {
    *stop_pos_p = uint32_t(stop_char_p - m_str_ref_p->m_cstr_p);  // determine pos where conversion ended
    }
  return value;
  }

//---------------------------------------------------------------------------------------
// Converter to f32.
// Returns:    a f32 interpretation of the specified section
// Arg         start_pos - index position to start parsing string.
// Arg         stop_pos_p - Address to store the index position where conversion stops.
//             If it is set to nullptr, it is ignored.  (Default nullptr)
// Examples:   f32 num = num_str.as_float32();
// See:        AString(real, significant), as_int32(), as_uint32_t()
// Notes:      The acceptable string form is as following:
//
//             [whitespace] [sign] [digits] [.digits] [{d | D | e | E} [sign] digits]
//
//             Whitespace may consist of space and tab characters, which are ignored.
//             sign is either plus (+) or minus (-) and digits are one or more decimal
//             digits.  If no digits appear before the radix character, at least one
//             must appear after the radix character.  The decimal digits can be
//             followed by an exponent, which consists of an introductory letter
//             (d, D, e, or E) and an optionally signed integer.  If neither an exponent
//             part nor a radix character appears, a radix character is assumed to
//             follow the last digit in the string.  The first character that does not
//             fit this form stops the scan.
// Author(s):   Conan Reis
f32 AString::as_float32(
  uint32_t   start_pos,  // = 0u
  uint32_t * stop_pos_p  // = nullptr
  ) const
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, "as_float64");
  #endif

  // $Revisit - CReis Temp code
  char * stop_char_p;
  f32    value = f32(::strtod(&m_str_ref_p->m_cstr_p[start_pos], &stop_char_p));  // convert

  if (stop_pos_p)
    {
    *stop_pos_p = uint32_t(stop_char_p - m_str_ref_p->m_cstr_p);  // determine pos where conversion ended
    }
  return value;
  }

//---------------------------------------------------------------------------------------
// Converter to int32_t
// Returns:    a int32_t interpretation of the specified section
// Arg         start_pos - index position to start parsing string.  (Default 0u)
// Arg         stop_pos_p - Address to store the index position where conversion stops.
//             If it is set to nullptr, it is ignored.  (Default nullptr)
// Arg         base - (or radix) sets the numerical base that should be expected.
//             Acceptable values are from 2 to 36.  If base is AString_determine_base (0),
//             the initial characters are used to determine the base.  If the first
//             character is 0 and the second character is not 'x' or 'X', the string is
//             interpreted as an octal (base 8) integer; otherwise, it is interpreted as
//             a decimal number.  If the first character is '0' and the second character
//             is 'x' or 'X', the string is interpreted as a hexadecimal integer.  If
//             the first character is '1' through '9', the string is interpreted as a
//             decimal integer.  (Default AString_determine_base)
// Examples:   int32_t num = num_str.as_int32();
// See:        AString(integer, base), as_uint32_t(), as_real64()
// Notes:      The acceptable string form is as following:
//
//             [whitespace] [{+ | -}] [0 [{ x | X }]] [digits]
//
//             Whitespace may consist of space and tab characters, which are ignored.
//             digits are one or more decimal digits.  The letters 'a' through 'z'
//             (or 'A' through 'Z') are assigned the values 10 through 35; only letters
//             whose assigned values are less than 'base' are permitted.  A plus (+) or
//             minus (-) sign is allowed as a prefix; a leading minus sign indicates
//             that the return value is negated.  The first character that does not fit
//             this form stops the scan.  
// Author(s):   Conan Reis
int AString::as_int(
  uint32_t   start_pos,  // = 0u
  uint32_t * stop_pos_p, // = nullptr
  uint32_t   base        // = AString_def_base
  ) const
  {
  char *  stop_char_p;
  int32_t value;

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, "as_int32");

    A_VERIFY(a_is_ordered(AString_determine_base, base, AString_max_base), a_cstr_format("invalid numerical base/radix \nExpected 1-37, but given %u", base), ErrId_invalid_base, AString);
  #endif

  value = int32_t(strtol(&m_str_ref_p->m_cstr_p[start_pos], &stop_char_p, int(base)));

  if (stop_pos_p)
    {
    *stop_pos_p = uint32_t(ptrdiff_t(stop_char_p) - ptrdiff_t(m_str_ref_p->m_cstr_p));  // determine pos where conversion ended
    }

  return value;
  }

//---------------------------------------------------------------------------------------
// Converter to uint32_t
// Returns:    a uint32_t interpretation of the specified section
// Arg         start_pos - index position to start parsing string.  (Default 0u)
// Arg         stop_pos_p - Address to store the index position where conversion stops.
//             If it is set to nullptr, it is ignored.  (Default nullptr)
// Arg         base - (or radix) sets the numerical base that should be expected.
//             Acceptable values are from 2 to 36.  If base is AString_determine_base (0),
//             the initial characters are used to determine the base.  If the first
//             character is 0 and the second character is not 'x' or 'X', the string is
//             interpreted as an octal (base 8) integer; otherwise, it is interpreted as
//             a decimal number.  If the first character is '0' and the second character
//             is 'x' or 'X', the string is interpreted as a hexadecimal integer.  If
//             the first character is '1' through '9', the string is interpreted as a
//             decimal integer.  (Default AString_determine_base)
// Examples:   int32_t num = num_str.as_int32();
// Notes:      The acceptable string form is as following:
//
//             [whitespace] [0 [{ x | X }]] [digits]
//
//             Whitespace may consist of space and tab characters, which are ignored.
//             digits are one or more decimal digits.  The letters 'a' through 'z'
//             (or 'A' through 'Z') are assigned the values 10 through 35; only letters
//             whose assigned values are less than 'base' are permitted.  The first
//             character that does not fit this form stops the scan.  
// Author(s):   Conan Reis
uint AString::as_uint(
  uint32_t   start_pos,  // = 0u
  uint32_t * stop_pos_p, // = nullptr
  uint       base        // = AString_def_base
  ) const
  {
  char *   stop_char_p;
  uint32_t value;
  
  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, "as_uint32_t");

    A_VERIFY(a_is_ordered(AString_determine_base, base, AString_max_base), a_cstr_format("invalid numerical base/radix \nExpected 1-37, but given %u", base), ErrId_invalid_base, AString);
  #endif

  value = uint32_t(strtoul(&m_str_ref_p->m_cstr_p[start_pos], &stop_char_p, int(base)));

  if (stop_pos_p)
    {
    *stop_pos_p = uint32_t(stop_char_p - m_str_ref_p->m_cstr_p);  // determine pos where conversion ended
    }

  return value;
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string at specified index
//              to the supplied substring to determine if it is equal to, less than, or
//              greater than it.
// Returns:     AEquate_equal, AEquate_less, or AEquate_greater
// Arg          substr - sub-string to compare
// See:         icompare_sub() for a case-insensitive version.
// Author(s):    Conan Reis
eAEquate AString::compare_sub(
  const AString & substr,
  uint32_t        index // = 0u
  ) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the commented out section below.
  char * str1_p     = m_str_ref_p->m_cstr_p + index;
  char * str1_end_p = str1_p + a_min(m_str_ref_p->m_length + 1u - index, substr.m_str_ref_p->m_length); // compare the # of characters in the shorter sub-string (without null)
  char * str2_p     = substr.m_str_ref_p->m_cstr_p;

  while (str1_p < str1_end_p)
    {
    if (*str1_p != *str2_p)  // if characters differ
      {
      // select appropriate result
      return (*str1_p < *str2_p) ? AEquate_less : AEquate_greater;
      }

    str1_p++;
    str2_p++;
    }

  return AEquate_equal;


  // Alternate method using standard library functions.
  //int result = ::strncmp(
  //  m_str_ref_p->m_cstr_p + index,
  //  substr.m_str_ref_p->m_cstr_p,
  //  a_min(m_str_ref_p->m_length - index, substr.m_str_ref_p->m_length));
  // 
  // This is a funky way to convert < 0 to -1, > 0 to 1, and 0 to stay 0
  //return static_cast<eAEquate>((result > 0) ? 1 : result >> 31);
  }

//---------------------------------------------------------------------------------------
//  Does a case-insensitive comparison of the current string at specified
//              index to the supplied substring to determine if it is equal to, less than,
//              or greater than it.
// Returns:     AEquate_equal, AEquate_less, or AEquate_greater
// Arg          substr - sub-string to compare
// See:         compare_sub() for a case-sensitive version.
// Author(s):    Conan Reis
eAEquate AString::icompare_sub(
  const AString & substr,
  uint32_t        index // = 0u
  ) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the commented out section below.
  uint8_t * str1_p     = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + index);
  uint8_t * str1_end_p = str1_p + a_min(m_str_ref_p->m_length + 1u - index, substr.m_str_ref_p->m_length); // compare the # of characters in the shorter sub-string (without null)
  uint8_t * str2_p     = reinterpret_cast<uint8_t *>(substr.m_str_ref_p->m_cstr_p);
  char ch1, ch2;

  while (str1_p < str1_end_p)
    {
    ch1 = ms_char2lower[*str1_p];
    ch2 = ms_char2lower[*str2_p];

    if (ch1 != ch2)  // if characters differ
      {
      // select appropriate result
      return (ch1 < ch2) ? AEquate_less : AEquate_greater;
      }

    str1_p++;
    str2_p++;
    }

  return AEquate_equal;


  // Alternate method using standard library functions.
  //int result = ::_strnicmp(
  //  m_str_ref_p->m_cstr_p + index,
  //  substr.m_str_ref_p->m_cstr_p,
  //  a_min(m_str_ref_p->m_length - index, substr.m_str_ref_p->m_length));
  // 
  // This is a funky way to convert < 0 to -1, > 0 to 1, and 0 to stay 0
  //return static_cast<eAEquate>((result > 0) ? 1 : result >> 31);
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              string to determine if they are equal.
// Returns:     true if equal, false if not
// Arg          str - string to compare
// See:         is_equal() for a case-sensitive version.
// Author(s):    Conan Reis
bool AString::is_equal(const AString & str) const
  {
  return m_str_ref_p->is_equal(*str.m_str_ref_p);
  }

//---------------------------------------------------------------------------------------
//  Does a case-insensitive comparison of the current string to the supplied
//              string to determine if they are equal.
// Returns:     true if equal, false if not
// Arg          str - string to compare
// See:         is_equal() for a case-sensitive version.
// Author(s):    Conan Reis
bool AString::is_iequal(const AString & str) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the standard library calls.
  uint32_t length = m_str_ref_p->m_length;

  if (length != str.m_str_ref_p->m_length)
    {
    return false;
    }

  uint8_t * str1_p     = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p);
  uint8_t * str1_end_p = str1_p + length;  // Don't bother comparing null character
  uint8_t * str2_p     = reinterpret_cast<uint8_t *>(str.m_str_ref_p->m_cstr_p);
  char ch1, ch2;

  while (str1_p < str1_end_p)
    {
    ch1 = ms_char2lower[*str1_p];
    ch2 = ms_char2lower[*str2_p];

    if (ch1 != ch2)  // if characters differ
      {
      // select appropriate result
      return false;
      }

    str1_p++;
    str2_p++;
    }

  return true;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Appends a C-string to the current string.
// Returns:    itself
// Arg         cstr_p - pointer to array of characters to append (does not need to be
//             null terminated unless length is equal to ALength_calculate).  'cstr_p'
//             should never be nullptr.
// Arg         length - number of characters to use in 'cstr_p'.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in the 'cstr_p'.
// See:        append(str), insert(), set_cstr(), set_buffer(), add(), operator+=(),
//             operator+(), operator=()
// Author(s):   Conan Reis
void AString::append(
  const char * cstr_p,
  uint32_t     length // = ALength_calculate
  )
  {
  A_ASSERT(cstr_p != nullptr || length == 0 || length == ALength_calculate, "Given nullptr and specified length", ErrId_null_cstr, AString);

  if (length == ALength_calculate)
    {
    length = cstr_p ? uint32_t(::strlen(cstr_p)) : 0;
    }

  if (length)
    {
    // Calculate total length of string
    uint32_t total_length = m_str_ref_p->m_length + length;

    ensure_size(total_length);
    
    // concatenate new string
    memcpy(m_str_ref_p->m_cstr_p + m_str_ref_p->m_length, cstr_p, size_t(length));
    
    m_str_ref_p->m_length               = total_length;
    m_str_ref_p->m_cstr_p[total_length] = '\0';    // Place a terminating character
    }
  }

//---------------------------------------------------------------------------------------
// Appends the the specified formatted string to the internal C-String.
// Arg         format_str_p - follows the same format as the C printf(), sprintf(), etc.
//             See the MSDev online help for 'Format Specification Fields' for a
//             description. 
// Arg         ... - variable length arguments expected by the formatted string.
//
//             #### IMPORTANT #### Since the variable length arguments do not specify a
//             type to the compiler the types passes must be determined via the standard
//             C string formatting functions.  So if a AString *object* is passed in as
//             one of the untyped variable length arguments, the compiler will not know
//             to coerce it to a C-String the C format functions will then treat the
//             address of the AString object as if it were a C-String with disastrous
//             results.  When using the string format type specifier (%s) and a AString
//             object (or a ASymbol object) use 'str.as_cstr()'.
//
// Examples:   AString name("Joe");     // Example substring to append
//             AString str("Hello");    // character length is 5 buffer size is 6
//             str.ensure_size(100u);  // character length is 5 buffer size is 101
//             str.append_format(" %i you %s %s", 2, name.as_cstr(), "Developer");
//             // str is now "Hello 2 you Joe Developer"
//             // character length is 25 buffer size is 101
//
// See:        format(), AString(max_size, format_str_p, ...)
// Notes:      If the sum of the current number of characters and the characters in the
//             expanded format string are larger than the size of this string's current
//             character buffer, then any characters above and beyond the buffer size
//             will be truncated.  So it is important that the maximum potential buffer
//             size has been allocated before this method is called - this can be
//             easily achieved by calling ensure_size() prior to calling this method.
// Author(s):   Conan Reis
void AString::append_format(const char * format_str_p, ...)
  {
  uint32_t length    = m_str_ref_p->m_length;
  uint32_t size      = m_str_ref_p->m_size;
  int      free_size = int(size - length) - 1;  // excluding null terminator

  if (free_size > 0)
    {
    va_list args;  // initialize argument list

    // Ensure buffer is writable and remains the same size
    ensure_size(size - 1u);

    va_start(args, format_str_p);

    #ifdef A_PLAT_PS3
      int fmt_length = vsnprintf(m_str_ref_p->m_cstr_p + length, size_t(free_size + 1), format_str_p, args);
    #else
      int fmt_length = _vsnprintf(m_str_ref_p->m_cstr_p + length, size_t(free_size), format_str_p, args);
    #endif

    va_end(args);  // end argument list processing

    if ((fmt_length == -1) || (fmt_length == free_size))  // More characters than buffer has, so truncate
      {
      fmt_length = free_size;
      m_str_ref_p->m_cstr_p[length + fmt_length] = '\0';     // Put in null-terminator
      }

    m_str_ref_p->m_length += uint32_t(fmt_length);
    }
  }

//---------------------------------------------------------------------------------------
// Removes all the characters but the substring starting at 'pos' with a
//             length of 'char_count'.
// Arg         pos - starting character index position of substring to crop
// Arg         char_count - number of characters that the remaining substring should
//             have.  If 'char_count' is ALength_remainder, the number of characters is
//             equal to the 'length of the string' - 'pos'.  (Default ALength_remainder)
// Examples:   AString str("  the quick brown fox  ");
// 
//             str.crop(3u, 12u);  // str is now "he quick bro"
// See:        crop_quick(), crop(match_type), trim(), truncate(),
//             remove_all(match_type), find(match_type)
// Notes:      This method checks if 'pos' and 'char_count' are valid if A_BOUNDS_CHECK
//             is defined (defined by default in debug builds).
// Author(s):   Conan Reis
void AString::crop(
  uint32_t pos,
  uint32_t char_count // = ALength_remainder
  )
  {
  AStringRef * str_ref_p = m_str_ref_p;

  if (char_count == ALength_remainder)
    {
    char_count = str_ref_p->m_length - pos;
    }

  if (char_count)
    {
    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "crop");
    #endif

    crop_quick(pos, char_count);
    }
  else
    {
    empty();
    }
  }

//---------------------------------------------------------------------------------------
// Removes all the characters of match_type from the beginning and the end
//             of the string until it encounters a character that is not match_type.
// Arg         match_type - classification of characters to remove.
//             (Default ACharMatch_white_space)
// Examples:   AString str("  the quick brown fox  ");
// 
//             str.crop();  // str is now "the quick brown fox"
// See:        trim(), truncate(), remove_all(match_type), find(match_type),
//             not_char_type()
// Author(s):   Conan Reis
AString & AString::crop(
  eACharMatch match_type // = ACharMatch_white_space
  )
  {
  uint32_t    start_pos;
  eACharMatch not_type = not_char_type(match_type);

  if (find(not_type, 1u, &start_pos))
    {
    uint32_t end_pos;

    find_reverse(not_type, 1u, &end_pos);
    crop_quick(start_pos, end_pos - start_pos + 1u);
    }
  else
    {
    empty();
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
// Removes all the characters but the substring starting at 'pos' with a
//             length of 'char_count'.
// Arg         pos - starting character index position of substring to crop
// Arg         char_count - number of characters that the remaining substring should
//             have - it should be at least 1.
// Examples:   AString str("  the quick brown fox  ");
// 
//             str.crop(3u, 12u);  // str is now "he quick bro"
// See:        crop_quick(), crop(match_type), trim(), truncate(),
//             remove_all(match_type), find(match_type)
// Notes:      This method does *not* check if 'pos' and 'char_count' are valid even if
//             A_BOUNDS_CHECK is defined.
// Author(s):   Conan Reis
void AString::crop_quick(
  uint32_t pos,
  uint32_t char_count
  )
  {
  AStringRef * str_ref_p = m_str_ref_p;

  if (char_count == str_ref_p->m_length)
    {
    // Keeping whole string - no need to do anything
    return;
    }

  // If unique and writable
  if ((str_ref_p->m_ref_count + str_ref_p->m_read_only) == 1u)
    {
    if (pos)
      {
      // Shift down
      memmove(str_ref_p->m_cstr_p, str_ref_p->m_cstr_p + pos, char_count);
      }

    str_ref_p->m_cstr_p[char_count] = '\0';
    str_ref_p->m_length             = char_count;
    }
  else  // Shared or read-only
    {
    uint32_t size     = AStringRef::request_char_count(char_count + 1u);
    char *   buffer_p = AStringRef::alloc_buffer(size);

    memcpy(buffer_p, str_ref_p->m_cstr_p + pos, size_t(char_count));
    buffer_p[char_count] = '\0';

    m_str_ref_p = str_ref_p->reuse_or_new(buffer_p, char_count, size);
    }
  }

//---------------------------------------------------------------------------------------
// Sets the internal C-String to the specified formatted string.
// Arg         format_str_p - follows the same format as the C printf(), sprintf(), etc.
//             See the MSDev online help for 'Format Specification Fields' for a
//             description. 
// Arg         ... - variable length arguments expected by the formatted string.
//
//             #### IMPORTANT #### When using the string format type specifier (%s) in
//             the format string and a AString object (or a ASymbol object) is passed in
//             as one of the variable length arguments, use 'str.as_cstr()'.
//             Variable length arguments do not specify a type to the compiler, instead
//             the types of the passed arguments are determined via the standard C
//             string formatting functions - i.e %s, %u, %f, etc.  So if a string type
//             (%s) is specified in the format string and a AString object is passed as
//             one of the untyped variable length arguments, the compiler will not know
//             to coerce the AString object to a C-String - it will instead treat the
//             address of the AString object argument as if it were a C-String character
//             array with disastrous results.
// Examples:   AString begin("The answer");  // Example substring to append
//             AString str("Hello");         // character length is 5 buffer size is 6
//             str.ensure_size(100u);       // character length is 5 buffer size is 101
//             str.format("%s to %s is %i", begin.as_cstr(), "everything", 42);
//             // str is now "The answer to everything is 42"
//             // character length is 30 buffer size is 101
//
// See:        append_format(), AString(max_size, format_str_p, ...)
// Notes:      If the number of characters in the expanded format string is larger than
//             the size of this string's current character buffer, then any characters
//             above and beyond the buffer size will be truncated.  So it is important
//             that the maximum potential buffer size has been allocated before this
//             method is called - this can be easily achieved by calling ensure_size()
//             prior to calling this method.
// Author(s):   Conan Reis
void AString::format(const char * format_str_p, ...)
  {
  va_list args;  // initialize argument list
  int     char_size = int(m_str_ref_p->m_size) - 1;

  if (char_size > 0)
    {
    // Ensure buffer is writable, but don't bother copying existing data.
    ensure_size_buffer(char_size);

    va_start(args, format_str_p);

  #if defined(A_PLAT_PS3) || defined(A_PLAT_PS4)
      int length = vsnprintf(m_str_ref_p->m_cstr_p, size_t(char_size + 1), format_str_p, args);
    #else
      int length = _vsnprintf(m_str_ref_p->m_cstr_p, size_t(char_size), format_str_p, args);
    #endif

    va_end(args);  // end argument list processing

    if ((length == -1) || (length == char_size))  // More characters than buffer has, so truncate
      {
      length = int(char_size);
      m_str_ref_p->m_cstr_p[char_size] = '\0';     // Put in null-terminator
      }

    m_str_ref_p->m_length = uint32_t(length);
    }
  }

//---------------------------------------------------------------------------------------
// Increments any postfixed number by amount specified.  If the string does
//             not have a previously postfixed number then increment_by is appended to
//             the string padded with zeros if necessary as specified by min_digits.
// Returns:    Postfixed value.
// Arg         increment_by - step value / amount to increment by
// Arg         min_digits - minimum number of digits to use for postfix value.  If a
//             previous postfix value is present, its number of digits is used and this
//             number is ignored.
// Author(s):   Conan Reis
uint32_t AString::increment(
  uint32_t increment_by, // = 1u
  uint32_t min_digits    // = 4u
  )
  {
  uint32_t find_pos = 0u;
  uint32_t value    = 0u;
  uint32_t length   = m_str_ref_p->m_length;

  // Look for previous value
  find_reverse(ACharMatch_not_digit, 1u, &find_pos);

  if ((find_pos + 1u) < length)
    {
    // Previous number postfix found
    find_pos++;
    value = as_uint(find_pos, nullptr, 10u);

    min_digits = length - find_pos;

    // remove old value prefix
    length -= min_digits;
    m_str_ref_p->m_length = length;
    }

  // Create new value
  value += increment_by;

  uint32_t new_digits = value < 10u
    ? 1u
    : a_log10ceil(value) + 1u;

  ensure_size(length + a_max(new_digits, min_digits));

  if (new_digits < min_digits)
    {
    // Pad with zeros
    append('0', min_digits - new_digits);
    }

  append_format("%d", value);

  return value;
  }

//---------------------------------------------------------------------------------------
// Indents rows/lines by specified `space_count` spaces from `start_pos` to `end_pos`.
// Works on line breaks that are in Unix style `\n` or DOS style `\r\n`.
//
// Returns:   number of line breaks over the range specified
// Params:  
//   space_count: number of space characters to indent
//   start_pos:   starting index to begin indentation
//   end_pos:     ending index to stop indentation
//   
// Notes:  
//   - Spaces inserted just prior to first non-space (space or tab) character on each row.
//   - Rows with no non-space characters are not indented.
//   - If the range ends just after a line break, the following row is not indented - at
//     least one character on a row must be included for it to be indented
//   - Mimics behaviour of Visual Studio editor.
// 
// See:         line_unindent(), line_*() functions, ARichEditOS::indent_selection()
// Author(s):   Conan Reis
uint32_t AString::line_indent(
  uint32_t space_count, // = AString_indent_spaces_def
  uint32_t start_pos,   // = 0u
  uint32_t end_pos      // = ALength_remainder
  )
  {
  // $Revisit - CReis Could use space_count=0 to indicate that spaces should be used to indent rather than spaces.
  AStringRef * str_ref_p = m_str_ref_p;
  uint32_t     length    = str_ref_p->m_length;

  if (length == 0u)
    {
    return 0u;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = length - 1;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, "indent");
  #endif


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine number of lines.
  uint32_t line_count   = 1u;
  char *   buffer_p     = str_ref_p->m_cstr_p;
  char *   cstr_start_p = buffer_p + start_pos;
  char *   cstr_end_p   = buffer_p + end_pos;
  char *   cstr_p       = cstr_start_p;

  // Use < rather than <= to intentionally stop before last character
  while (cstr_p < cstr_end_p)
    {
    if (*cstr_p == '\n')
      {
      line_count++;
      }

    cstr_p++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure enough memory for indenting
  size_t   char_count;
  char *   dest_p;
  char *   source_p     = buffer_p;
  uint32_t extra_chars  = line_count * space_count;

  // $Revisit - CReis Could reuse existing buffer if it is large enough.
  // Test to see if new buffer needed or if old one can be reused
  //bool new_buffer_b = (str_ref_p->m_size < (length + extra_chars))
  //  || ((str_ref_p->m_ref_count + str_ref_p->m_read_only) != 1u);

  uint32_t new_size     = AStringRef::request_char_count(length + extra_chars);
  char *   new_buffer_p = AStringRef::alloc_buffer(new_size);

  dest_p = new_buffer_p;
  cstr_p = cstr_start_p;

  do 
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Find first non-space
    while ((cstr_p <= cstr_end_p)
      && ((*cstr_p == ' ') || (*cstr_p == '\t')))
      {
      cstr_p++;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Indent if some non-space chars before end of line
    if ((cstr_p <= cstr_end_p) && (*cstr_p != '\r') && (*cstr_p != '\n'))
      {
      // Copy characters so far
      char_count = size_t(cstr_p - source_p);
      memcpy(dest_p, source_p, char_count);
      source_p = cstr_p;
      dest_p  += char_count;

      // Indent
      memset(dest_p, ' ', space_count);
      dest_p += space_count;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Skip past end of line (skip `\r` of `\r\n`) or to end of range
    while (cstr_p <= cstr_end_p)
      {
      if (*cstr_p == '\n')
        {
        // Skip past newline
        cstr_p++;
        break;
        }

      cstr_p++;
      }
    }
  while (cstr_p <= cstr_end_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy any remainder of the string
  char_count = size_t(cstr_p - source_p);
  memcpy(dest_p, source_p, char_count);
  dest_p += char_count;

  length = uint32_t(dest_p - new_buffer_p);
  new_buffer_p[length] = '\0';
  m_str_ref_p = str_ref_p->reuse_or_new(new_buffer_p, length, new_size);

  return line_count;
  }

//---------------------------------------------------------------------------------------
// Indents all rows/lines after the first row by specified `space_count` spaces from
// `start_pos` to `end_pos`. Works on line breaks that are in Unix style `\n` or DOS
// style `\r\n`.
//
// Returns:   number of line breaks over the range specified
// Params:  
//   space_count: number of space characters to indent
//   start_pos:   starting index to begin indentation
//   end_pos:     ending index to stop indentation
//   
// Notes:  
//   - Spaces inserted just prior to first non-space (space or tab) character on each row.
//   - Rows with no non-space characters are not indented.
//   - If the range ends just after a line break, the following row is not indented - at
//     least one character on a row must be included for it to be indented
//   - Mimics behaviour of Visual Studio editor.
// 
// See:         line_unindent(), line_*() functions, ARichEditOS::indent_selection()
// Author(s):   Conan Reis
uint32_t AString::line_indent_next(
  uint32_t space_count, // = AString_indent_spaces_def
  uint32_t start_pos,   // = 0u
  uint32_t end_pos      // = ALength_remainder
  )
  {
  if (find('\n', 1u, &start_pos, start_pos, end_pos))
    {
    return line_indent(space_count, start_pos + 1u, end_pos);
    }

  return 0u;
  }

//---------------------------------------------------------------------------------------
// Unindents rows/lines by specified `space_count` spaces from `start_pos` to `end_pos`.
// Works on line breaks that are in Unix style `\n` or DOS style `\r\n`.
//
// Returns:   number of line breaks over the range specified
// Params:  
//   space_count: number of space characters to unindent
//   tab_stops:   tab stop count in spaces
//   start_pos:   starting index to begin
//   end_pos:     ending index to stop
//   
// Notes:  
//   - Spaces removed just prior to first non-space (space or tab) character on each row.
//   - Rows with no non-space characters are left as is
//   - If the range ends just after a line break, the following row is not unindented - at
//     least one character on a row must be included for it to be unindented
// 
// See:         line_indent(), line_*() functions, ARichEditOS::unindent_selection()
// Author(s):   Conan Reis
uint32_t AString::line_unindent(
  uint32_t space_count, // = AString_indent_spaces_def
  uint32_t tab_stops,   // = AString_tab_stop_def
  uint32_t start_pos,   // = 0u
  uint32_t end_pos      // = ALength_remainder
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct Nested
    {
    static char * set_column(
      char * cstr_p, uint32_t column, uint32_t _tab_stops)
      {
      uint32_t idx = 0u;
      uint32_t tab_spaces = 0u;

      while (idx < column)
        {
        if (*cstr_p == ' ')
          {
          cstr_p++;
          idx++;
          }
        else
          {
          // Must be a tab - determine its number of spaces
          tab_spaces = _tab_stops - (idx % _tab_stops);

          if ((idx + tab_spaces) >= column)
            {
            // Convert tab to spaces as needed and increment space difference
            tab_spaces = column - idx;
            memset(cstr_p, ' ', tab_spaces);

            return cstr_p + tab_spaces;
            }

          idx += tab_spaces;
          }
        }

      return cstr_p;
      }
    };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  AStringRef * str_ref_p = m_str_ref_p;
  uint32_t     length    = str_ref_p->m_length;

  if (length == 0u)
    {
    return 0u;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = length - 1u;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, "indent");
  #endif


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine number of lines.
  uint32_t line_count   = 1u;
  char *   buffer_p     = str_ref_p->m_cstr_p;
  char *   cstr_start_p = buffer_p + start_pos;
  char *   cstr_end_p   = buffer_p + end_pos;
  char *   cstr_p       = cstr_start_p;

  // Use < rather than <= to intentionally stop before last character
  while (cstr_p < cstr_end_p)
    {
    if (*cstr_p == '\n')
      {
      line_count++;
      }

    cstr_p++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure enough memory for unindenting
  // - Note that tabs are converted to spaces as they are reduced and they may add up to
  //   more characters even after space_count characters have been removed.
  size_t   char_count;
  char *   line_p;
  char *   dest_p;
  char *   source_p     = buffer_p;
  uint32_t extra_chars  = 0u;
  uint32_t column       = 0u;
  
  if (tab_stops > space_count)
    {
    extra_chars = line_count * (tab_stops - space_count - 1);
    }

  // $Revisit - CReis Could reuse existing buffer if it is large enough.
  // Test to see if new buffer needed or if old one can be reused
  //bool new_buffer_b = (str_ref_p->m_size < (length + extra_chars))
  //  || ((str_ref_p->m_ref_count + str_ref_p->m_read_only) != 1u);

  uint32_t new_size     = AStringRef::request_char_count(length + extra_chars);
  char *   new_buffer_p = AStringRef::alloc_buffer(new_size);

  dest_p = new_buffer_p;
  cstr_p = cstr_start_p;

  do 
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Find first non-space
    column = 0u;
    line_p = dest_p + size_t(cstr_p - source_p);

    while ((cstr_p <= cstr_end_p)
      && ((*cstr_p == ' ') || (*cstr_p == '\t')))
      {
      column += (*cstr_p == '\t') ? (tab_stops - (column % tab_stops)) : 1u;
      cstr_p++;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Unindent if some non-space chars before end of line
    if ((cstr_p <= cstr_end_p) && (*cstr_p != '\r') && (*cstr_p != '\n'))
      {
      // Copy characters so far
      char_count = size_t(cstr_p - source_p);
      memcpy(dest_p, source_p, char_count);
      source_p = cstr_p;

      // Unindent
      dest_p = Nested::set_column(
        line_p, (column > space_count) ? column - space_count : 0u, tab_stops);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Skip past end of line (skip `\r` of `\r\n`) or to end of range
    while (cstr_p <= cstr_end_p)
      {
      if (*cstr_p == '\n')
        {
        // Skip past newline
        cstr_p++;
        break;
        }

      cstr_p++;
      }
    }
  while (cstr_p <= cstr_end_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy any remainder of the string
  char_count = size_t(cstr_p - source_p);
  memcpy(dest_p, source_p, char_count);
  dest_p += char_count;

  length = uint32_t(dest_p - new_buffer_p);
  new_buffer_p[length] = '\0';
  m_str_ref_p = str_ref_p->reuse_or_new(new_buffer_p, length, new_size);

  return line_count;
  }

//---------------------------------------------------------------------------------------
// Substring character insertion.
// Returns:    a reference to itself
// Author(s):   Conan Reis
void AString::insert(
  char ch,
  uint32_t pos // = 0u
  )
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(pos, "insert");
  #endif

  // $Revisit - CReis This could be optimized - the end may be copied twice
  ensure_size(m_str_ref_p->m_length + 1u);

  memmove(m_str_ref_p->m_cstr_p + pos + 1u, m_str_ref_p->m_cstr_p + pos, size_t(m_str_ref_p->m_length - pos + 1u));
  m_str_ref_p->m_cstr_p[pos] = ch;  // insert character
  m_str_ref_p->m_length++;
  }

//---------------------------------------------------------------------------------------
// Sub-string string insertion.
// Returns:    a reference to itself
// Author(s):   Conan Reis
void AString::insert(
  const AString & str,
  uint32_t        pos // = 0u
  )
  {
  #ifdef A_BOUNDS_CHECK
    bounds_check(pos, "insert");
  #endif

  // $Revisit - CReis This could be optimized - the end may be copied twice
  ensure_size(m_str_ref_p->m_length + str.m_str_ref_p->m_length);

  memmove(m_str_ref_p->m_cstr_p + pos + str.m_str_ref_p->m_length, m_str_ref_p->m_cstr_p + pos, size_t(m_str_ref_p->m_length - pos + 1u));
  memcpy(m_str_ref_p->m_cstr_p + pos, str.m_str_ref_p->m_cstr_p, size_t(str.m_str_ref_p->m_length));
  m_str_ref_p->m_length += str.m_str_ref_p->m_length;
  }

//---------------------------------------------------------------------------------------
// Converts line breaks from to Unix style (\n) to DOS style (\r\n).
//             This method will also work if the string is already in DOS style.
// Returns:    Returns the number of line breaks that were converted.
// See:        line_*() functions
// Author(s):   Conan Reis
uint32_t AString::line_break_unix2dos(
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  uint32_t pre_remove = m_str_ref_p->m_length;

  remove_all('\r', start_pos, end_pos);

  end_pos = (end_pos == ALength_remainder)
    ? m_str_ref_p->m_length - 1u
    : end_pos - (pre_remove - m_str_ref_p->m_length);

  return replace_all('\n', ms_dos_break, start_pos, end_pos);
  }

//---------------------------------------------------------------------------------------
// Substring search and removal. Finds Nth instance string str starting from start_pos
// and stores the index position if found in find_pos. Returns true if found false if not.
// Author(s):   Conan Reis
bool AString::remove(
  const AString & str,
  uint32_t        instance,    // = 1u
  uint32_t *      find_pos_p,  // = nullptr
  uint32_t        start_pos,   // = 0u
  uint32_t        end_pos,     // = ALength_remainder
  eAStrCase       case_check   // = AStrCase_sensitive
  )
  {
  uint32_t pos;

  if (find(str, instance, &pos, start_pos, end_pos, case_check))
    {
    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_writable();

    m_str_ref_p->m_length -= str.m_str_ref_p->m_length;  // length of new string
    memmove(m_str_ref_p->m_cstr_p + pos, m_str_ref_p->m_cstr_p + pos + str.m_str_ref_p->m_length, size_t(m_str_ref_p->m_length - pos + 1u));

    if (find_pos_p)
      {
      *find_pos_p = pos;
      }
      
    return true;
    }
    
  return false;
  }

//---------------------------------------------------------------------------------------
// Quick substring search and removal using the Boyer Moore algorithm.   
//             Good for multiple searches for the same substring.  Finds Nth instance  
//             of bm starting from start_pos and stores the index position if  
//             found in find_pos. Returns true if found false if not.
// Author(s):   Conan Reis
bool AString::remove(
  const AStringBM & bm,
  uint32_t          instance,    // = 1u
  uint32_t *        find_pos_p,  // = nullptr
  uint32_t          start_pos,   // = 0u
  uint32_t          end_pos      // = ALength_remainder
  )
  {
  uint32_t pos;

  if (find(bm, instance, &pos, start_pos, end_pos))
    {
    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_writable();

    m_str_ref_p->m_length -= bm.get_length();           // length of new string
    memmove(m_str_ref_p->m_cstr_p + pos, m_str_ref_p->m_cstr_p + pos + bm.get_length(), size_t(m_str_ref_p->m_length - pos + 1u));

    if (find_pos_p)
      {
      *find_pos_p = pos;
      }

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Substring deletion method
// Returns:    a reference to itself
// Author(s):   Conan Reis
void AString::remove_all(
  uint32_t pos,
  uint32_t char_count // = ALength_remainder
  )
  {
  if (char_count == ALength_remainder)
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  if (char_count)
    {

    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "remove_all");
    #endif

    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_writable();

    m_str_ref_p->m_length -= char_count;           // length of new string
    memmove(m_str_ref_p->m_cstr_p + pos, m_str_ref_p->m_cstr_p + pos + char_count, size_t(m_str_ref_p->m_length - pos + 1u));
    }
  }

//---------------------------------------------------------------------------------------
// Remove all characters of matching ch from start_pos to end_pos.
// Returns:    Returns the number of characters that were removed.
// Arg         ch - character to match
// Arg         start_pos - first position to start removing  (Default 0)
// Arg         end_pos - last position to look for characters to remove.
//             If end_pos is ALength_remainder, end_pos is set to the last index position of
//             the string (length - 1).
//             (Default ALength_remainder)
// Examples:   AString str("  the quick brown fox  ");
// 
//             str.remove_all('o');  // str is now "  the quick brwn fx  "
// See:        trim(), truncate(), remove_all(match_type), remove_all(str),
//             find(match_type)
// Author(s):   Conan Reis
uint32_t AString::remove_all(
  char ch,
  uint32_t start_pos, // = 0u
  uint32_t end_pos    // = ALength_remainder
  )
  {
  uint32_t remove_count = 0u;

  if (m_str_ref_p->m_length)
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "remove_all");
    #endif

    // $Revisit - CReis This could be optimized - some characters may be needlessly copied
    ensure_writable();

    uint8_t * cstr_p     = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + start_pos);
    uint8_t * rewrite_p  = cstr_p;
    uint8_t * cstr_end_p = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + end_pos);
    uint32_t  length     = m_str_ref_p->m_length;

    for (; cstr_p <= cstr_end_p; cstr_p++)
      {
      if (*cstr_p != ch)
        {
        // $Revisit - CReis assuming that a check to see if rewrite_p == cstr_p would just slow things down
        *rewrite_p = *cstr_p;
        rewrite_p++;
        }
      else
        {
        length--;
        }
      }

    *rewrite_p            = '\0';
    remove_count          = m_str_ref_p->m_length - length;
    m_str_ref_p->m_length = length;
    }

  return remove_count;
  }

//---------------------------------------------------------------------------------------
// Remove all characters of match_type from start_pos to end_pos.
// Returns:    Returns the number of characters that were removed.
// Arg         match_type - classification of characters to match
// Arg         start_pos - first position to start removing  (Default 0)
// Arg         end_pos - last position to look for characters to remove.
//             If end_pos is ALength_remainder, end_pos is set to the last index position of
//             the string (length - 1).
//             (Default ALength_remainder)
// Examples:   AString str("  the quick brown fox  ");
// 
//             str.remove_all(ACharMatch_white_space);  // str is now "thequickbrownfox"
// See:        trim(), truncate(), remove_all(str), remove_all(ch), find(match_type),
//             not_char_type()
// Author(s):   Conan Reis
uint32_t AString::remove_all(
  eACharMatch match_type,
  uint32_t    start_pos, // = 0u
  uint32_t    end_pos    // = ALength_remainder
  )
  {
  uint32_t remove_count = 0u;

  if (m_str_ref_p->m_length)
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "remove_all");
    #endif

    // $Revisit - CReis This could be optimized - some characters may be needlessly copied
    ensure_writable();

    uint8_t * cstr_p        = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + start_pos);
    uint8_t * rewrite_p     = cstr_p;
    uint8_t * cstr_end_p    = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + end_pos);
    bool *    match_table_p = ms_char_match_table[not_char_type(match_type)];
    uint32_t  length        = m_str_ref_p->m_length;

    for (; cstr_p <= cstr_end_p; cstr_p++)
      {
      if (match_table_p[*cstr_p])
        {
        *rewrite_p = *cstr_p;
        rewrite_p++;
        }
      else
        {
        length--;
        }
      }

    *rewrite_p            = '\0';
    remove_count          = m_str_ref_p->m_length - length;
    m_str_ref_p->m_length = length;
    }

  return remove_count;
  }

//---------------------------------------------------------------------------------------
// Remove all substrings matching str starting from start_pos to end_pos.
// Returns:    Returns the number of characters that were removed.
// Arg         str - substring to match
// Arg         start_pos - first position to start removing.
// Arg         end_pos - last position to look for substring to remove.
//             If end_pos is ALength_remainder, end_pos is set to the last index position
//             of the string (length - 1).
// Arg         case_check - indicates whether case should be AStrCase_sensitive or
//             AStrCase_insensitive
// Author(s):   Conan Reis
uint32_t AString::remove_all(
  const AString & str,
  uint32_t        start_pos,  // = 0u
  uint32_t        end_pos,    // = ALength_remainder
  eAStrCase       case_check  // = AStrCase_sensitive
  )
  {
  // $Revisit - CReis This could be optimized - some characters may be needlessly copied
  // [Efficiency] Instead of moving from (end of the remove position) - (end of string)
  // to (beginning of the delete position) for each remove, the move could be from (end
  // of the remove position) - (beginning of next remove or end of string, whichever is
  // nearer) to (end of last move or start of string, whichever is later).

  uint32_t   removed    = 0u;
  uint32_t   str_length = str.m_str_ref_p->m_length;
  uint32_t & length     = m_str_ref_p->m_length;  // Faster, but length must be up-to-date in find()

  if (str_length && length)
    {
    ensure_writable();
  
    char * cstr_p = m_str_ref_p->m_cstr_p;
  
    if (end_pos == ALength_remainder)
      {
      end_pos = length - 1u;
      }
  
    while (find(str, 1u, &start_pos, start_pos, end_pos, case_check))
      {
      end_pos -= str_length;
      length  -= str_length;  // length of new string
      memmove(cstr_p + start_pos, cstr_p + start_pos + str_length, size_t(length - start_pos + 1u));
      removed++;
      }
    }

  return removed;
  }

//---------------------------------------------------------------------------------------
// Remove all substrings matching fast finding Boyer-Moore str starting from
//             start_pos to end_pos.
// Returns:    Returns the number of characters that were removed.
// Arg         str - Boyer-Moore substring to match
// Arg         start_pos - first position to start removing.
// Arg         end_pos - last position to look for substring to remove.
//             If end_pos is ALength_remainder, end_pos is set to the last index position
//             of the string (length - 1).
// Arg         case_check - indicates whether case should be AStrCase_sensitive or
//             AStrCase_insensitive
// Author(s):    Conan Reis
uint32_t AString::remove_all(
  const AStringBM & bm,
  uint32_t          start_pos,  // = 0u
  uint32_t          end_pos     // = ALength_remainder
  )
  {
  // $Revisit - CReis This could be optimized - some characters may be needlessly copied
  // [Efficiency] Instead of moving from (end of the remove position) - (end of string)
  // to (beginning of the delete position) for each remove, the move could be from (end
  // of the remove position) - (beginning of next remove or end of string, whichever is
  // nearer) to (end of last move or start of string, whichever is later).

  uint32_t   removed    = 0u;
  uint32_t   str_length = bm.m_str_ref_p->m_length;
  uint32_t & length     = m_str_ref_p->m_length;  // Faster, but length must be up-to-date in find()
  
  if (str_length && length)
    {
    ensure_writable();
    
    char * cstr_p = m_str_ref_p->m_cstr_p;
  
    if (end_pos == ALength_remainder)
      {
      end_pos = length - 1u;
      }
  
    while (find(bm, 1u, &start_pos, start_pos, end_pos))
      {
      end_pos -= str_length;
      length  -= str_length;  // length of new string
      memmove(cstr_p + start_pos, cstr_p + start_pos + str_length, size_t(length - start_pos + 1u));
      removed++;
      }
    }
    
  return removed;
  }

//---------------------------------------------------------------------------------------
//  Removes specified number of characters from the end of the string.
// Arg          char_count - number of characters to remove from the end of the string.
// See:         set_length(), crop(), get_length(), remove(), trim(), truncate()
// Author(s):    Conan Reis
void AString::remove_end(uint32_t char_count)
  {
  #ifdef A_BOUNDS_CHECK
    A_VERIFY(char_count <= m_str_ref_p->m_length, a_cstr_format("- tried to remove %u characters out of %u", char_count, m_str_ref_p->m_length), AErrId_invalid_index_span, AString);
  #endif

  set_length(m_str_ref_p->m_length - char_count);
  }

//---------------------------------------------------------------------------------------
// Replaces chars in string starting at pos with new_char_count chars from
//             new_str at new_pos.  If the pos + new_char_count is greater than the
//             current length of the string, it will extend its length.
// Returns:    a reference to itself
// Arg         new_str - string to replace with
// Arg         pos - position in this string to begin replace
// Arg         char_count - number of chars to replace in this string.  If it is set to
//             ALength_remainder, it is set to length of this string - pos.  To do an
//             "overwrite" use new_str.get_length().  If this value is 0, it is the same
//             as an "insert".
// Arg         new_pos - position to start copying from new_str
// Arg         new_char_count - number of chars to copy from new_str into this string.
//             If it is set to ALength_remainder, it is set to length of new - new_pos.
// Author(s):   Conan Reis
void AString::replace(
  const AString & new_str,
  uint32_t        pos,            // = 0
  uint32_t        char_count,     // = ALength_remainder
  uint32_t        new_pos,        // = 0
  uint32_t        new_char_count // = ALength_remainder
  )
  {
  if (new_char_count == ALength_remainder)
    {
    new_char_count = new_str.m_str_ref_p->m_length - new_pos;
    }

  #ifdef A_BOUNDS_CHECK
    new_str.span_check(new_pos, new_char_count, "replace");
  #endif

  if (new_char_count)
    {
    uint32_t rem_length = m_str_ref_p->m_length - pos;

    char_count = (char_count == ALength_remainder)
      ? rem_length
      : a_min(char_count, rem_length);  // Only remove the characters available

    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "replace");
    #endif

    uint32_t     new_length = m_str_ref_p->m_length - char_count + new_char_count;
    AStringRef * str_ref_p  = m_str_ref_p;

    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_size(new_length);

    // Move remaining characters following old_str up or down
    if (pos + new_char_count <= new_length)
      {
      memmove(m_str_ref_p->m_cstr_p + pos + new_char_count, str_ref_p->m_cstr_p + pos + char_count, str_ref_p->m_length - pos - char_count + 1u);
      }

    // Copy new_str to its replacement position in the current string
    memcpy(m_str_ref_p->m_cstr_p + pos, new_str.m_str_ref_p->m_cstr_p + new_pos, new_char_count);
    m_str_ref_p->m_length = new_length;
    }
  }

//---------------------------------------------------------------------------------------
//  Substring search and replacement
// Author(s):    Conan Reis
bool AString::replace(
  const AString & old_str,
  const AString & new_str,
  uint32_t        instance,    // = 1u
  uint32_t *      find_pos_p,  // = nullptr
  uint32_t        start_pos,   // = 0u
  uint32_t        end_pos,     // = ALength_remainder
  eAStrCase       case_check   // = AStrCase_sensitive
  )
  {
  uint32_t pos;

  if (find(old_str, instance, &pos, start_pos, end_pos, case_check))
    {
    uint32_t     old_length = old_str.m_str_ref_p->m_length;
    uint32_t     new_length = new_str.m_str_ref_p->m_length;
    uint32_t     length     = m_str_ref_p->m_length - old_length + new_length;
    AStringRef * str_ref_p  = m_str_ref_p;

    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_size(length);

    // Move remaining characters following old_str up or down
    if (pos + new_length < length)
      {
      memmove(m_str_ref_p->m_cstr_p + pos + new_length, str_ref_p->m_cstr_p + pos + old_length, str_ref_p->m_length - pos - old_length + 1u);
      }

    // Copy new_str to its replacement position in the current string
    memcpy(m_str_ref_p->m_cstr_p + pos, new_str.m_str_ref_p->m_cstr_p, new_length);
    m_str_ref_p->m_length = m_str_ref_p->m_length - old_length + new_length;

    if (find_pos_p)
      {
      *find_pos_p = pos;
      }

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Substring search and replacement
// Author(s):    Conan Reis
bool AString::replace(
  const AStringBM & bm,
  const AString &   new_str,
  uint32_t          instance,    // = 1u
  uint32_t *        find_pos_p,  // = nullptr
  uint32_t          start_pos,   // = 0u
  uint32_t          end_pos      // = ALength_remainder
  )
  {
  uint32_t pos;

  if (find(bm, instance, &pos, start_pos, end_pos))
    {
    uint32_t old_length = bm.m_str_ref_p->m_length;
    uint32_t new_length = new_str.m_str_ref_p->m_length;
    uint32_t length     = m_str_ref_p->m_length - old_length + new_length;

    // $Revisit - CReis This could be optimized - the end may be copied twice
    ensure_size(length);

    // Move remaining characters following bm up or down
    memmove(m_str_ref_p->m_cstr_p + pos + new_length, m_str_ref_p->m_cstr_p + pos + old_length, m_str_ref_p->m_length - pos - old_length + 1u);

    // Copy new_str to its replacement position in the current string
    memcpy(m_str_ref_p->m_cstr_p + pos, new_str.m_str_ref_p->m_cstr_p, new_length);
    m_str_ref_p->m_length = length;

    if (find_pos_p)
      {
      *find_pos_p = pos;
      }

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Replace all old_ch with new_ch starting at start_pos and ending at end_pos.
// Returns:     Number of characters replaced.
// Author(s):    Conan Reis
uint32_t AString::replace_all(
  char     old_ch,
  char     new_ch,
  uint32_t start_pos,  // = 0u
  uint32_t end_pos     // = ALength_remainder
  )
  {
  uint32_t count  = 0u;
  uint32_t length = m_str_ref_p->m_length;
  
  if (length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "replace_all");
    #endif

    ensure_writable();

    char * cstr_p     = m_str_ref_p->m_cstr_p;
    char * cstr_end_p = cstr_p + length;

    while (cstr_p < cstr_end_p)
      {
      if (*cstr_p == old_ch)
        {
        *cstr_p = new_ch;
        count++;
        }

      cstr_p++;
      }
    }

  return count;
  }

//---------------------------------------------------------------------------------------
//  Replace all old_ch with new_str starting at start_pos and ending at end_pos.
// Returns:     Number of characters replaced.
// Efficiency   See remove_all() efficiency comment for replacements where new_str is
//              shorter than old_str.  It may also be more efficient to apply the same
//              method for the reverse - just determine offset or tally the results.
// Stability    Assumes that new_str is > 1 character in length
uint32_t AString::replace_all(
  char            old_ch,
  const AString & new_str,
  uint32_t        start_pos,  // = 0u
  uint32_t        end_pos     // = ALength_remainder
  )
  {
  uint32_t pos;
  uint32_t found = 0u;
  
  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "replace_all");
    #endif

    found = count(old_ch, start_pos, end_pos);

    if (found)
      {
      uint32_t new_length      = new_str.m_str_ref_p->m_length;
      uint32_t size_difference = new_length - 1u;

      ensure_size(m_str_ref_p->m_length + (found * size_difference));

      // These must be after the ensure_size() since the AStringRef and internal buffer could change
      uint32_t & length     = m_str_ref_p->m_length;  // Faster, but length must be up-to-date in find()
      char *     cstr_p     = m_str_ref_p->m_cstr_p;
      char *     new_cstr_p = new_str.m_str_ref_p->m_cstr_p;

      while ((start_pos <= end_pos) && find(old_ch, 1u, &pos, start_pos, end_pos))
        {
        memmove(cstr_p + pos + new_length, cstr_p + pos + 1u, size_t(length - pos));
        length  += size_difference;
        end_pos += size_difference;
        memcpy(cstr_p + pos, new_cstr_p, size_t(new_length));
        start_pos = pos + new_length;
        }
      }
    }

  return found;
  }

//---------------------------------------------------------------------------------------
//  Replace all bm with new_str starting at start_pos
// Author(s):    Conan Reis
// Efficiency   See remove_all() efficiency comment for replacements where new_str is
//              shorter than old_str.  It may also be more efficient to apply the same
//              method for the reverse - just determine offset or tally the results.
uint32_t AString::replace_all(
  const AStringBM & bm,
  const AString &   new_str,
  uint32_t          start_pos,  // = 0u
  uint32_t          end_pos     // = ALength_remainder
  )
  {
  uint32_t pos;
  uint32_t found = 0u;
  
  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "replace_all");
    #endif

    found = count(bm, start_pos, end_pos);

    if (found)
      {
      uint32_t new_length      = new_str.m_str_ref_p->m_length;
      uint32_t old_length      = bm.get_length();
      int      size_difference = int(new_length) - int(old_length);

      ensure_size(m_str_ref_p->m_length + (found * size_difference));

      // These must be after the ensure_size() since the internal buffer could change
      uint32_t & length     = m_str_ref_p->m_length;  // Faster, but length must be up-to-date in find()
      char *     cstr_p     = m_str_ref_p->m_cstr_p;
      char *     new_cstr_p = new_str.m_str_ref_p->m_cstr_p;

      while ((start_pos <= end_pos) && find(bm, 1u, &pos, start_pos, end_pos))
        {
        if (size_difference != 0)
          {
          memmove(cstr_p + pos + new_length, cstr_p + pos + old_length, size_t(length - old_length + 1u - pos));
          length  += uint32_t(size_difference);  // Silly cast to stop compiler warning
          end_pos += uint32_t(size_difference);
          }

        memcpy(cstr_p + pos, new_cstr_p, size_t(new_length));
        start_pos = pos + new_length;
        }
      }
    }

  return found;
  }

//---------------------------------------------------------------------------------------
//  Reverse order of char_count characters starting at pos.
// Arg          pos - character index to begin reversing characters.  (Default 0u)
// Arg          char_count - number of characters to reverse.  If char_count is ALength_remainder,
//              the number of characters is equal to the length of the string - pos.
//              (Default ALength_remainder)
// Examples:    AString str("12345");
//              str.reverse();         // = "54321"
// See:         as_reverse(), find_reverse()
// Author(s):    Conan Reis
void AString::reverse(
  uint32_t pos,     // = 0u
  uint32_t char_count // = ALength_remainder
  )
  {
  if (char_count == ALength_remainder)  // default
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  if (char_count > 1)  // If less than 2 characters are reversed, there is no point
    {
    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "reverse");
    #endif

    // $Revisit - CReis This could be optimized - characters in specified range may be copied twice.
    ensure_writable();

    char   ch;
    char * cstr_p     = m_str_ref_p->m_cstr_p + pos;
    char * cstr_end_p = cstr_p + char_count - 1;

    while (cstr_p < cstr_end_p)
      {
      ch          = *cstr_p;
      *cstr_p     = *cstr_end_p;
      *cstr_end_p = ch;
      cstr_p++;
      cstr_end_p--;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the C-String character buffer.  See the description of the
//              argument 'buffer_p' for the three possible uses of this constructor.
// Returns:     itself
// Arg          buffer_p - pointer to array of characters (does not need to be null
//              terminated unless 'length' is equal to ALength_calculate).  If 'buffer_p'
//              is nullptr, 'length' and 'deallocate' are ignored.  There are in general 3
//              different types of values that 'buffer_p' will be given:
//                1. A temporary buffer which is allocated on the stack or is recovered
//                   by some other means.
//                2. A pre-allocated character array that this AString object should take
//                   over responsibility for.
//                3. nullptr.  This then creates an empty string with a space of 'size'
//                   already allocated including the null terminating character.
// Arg          size - This is the size in bytes of the memory pointed to by 'buffer_p'
//              or if 'buffer_p' is nullptr, it is the initial memory size that should be 
//              allocated by this string.  Also note that this size includes the null
//              terminating character, so 'size' should never be less than 1.
// Arg          length - number of characters to use in 'buffer_p' and the index position
//              to place a terminating null character.  The given length must not be more
//              than the size of 'buffer_p' and the C-String buffer pointed to by
//              'buffer_p' should not have any null characters less then the given length.
//              'length' may also be set to ALength_calculate in which case the character
//              length is calculated by finding the first terminating null character
//              already present in 'buffer_p'.  (Default ALength_calculate)
// Arg          deallocate - Indicates whether this string (or more specifically the
//              AStringRef) should take control of the memory pointed to by buffer_p and
//              do deallocate it when it is no longer in use.
// Examples:    char buffer_p[100];
//              str.set_buffer(buffer_p, 100u, 0u);
// See:         set_cstr(), operator=(cstr), append(), insert(), AString(cstr_p),
//              AString(cstr_p, length, persistent), AString(buffer_p, size, length, deallocate)
// Notes:       To make a copy of a C-String call AString(cstr_p, length, persistent = false).
//              To reference a C-String literal call AString(cstr_p) or
//              AString(cstr_p, length, persistent = true)
// Author(s):    Conan Reis
void AString::set_buffer(
  const char * buffer_p,
  uint32_t     size,
  uint32_t     length,    // = ALength_calculate
  bool         deallocate // = false
  )
  {
  if (buffer_p)
    {
    if (length == ALength_calculate)
      {
      length = uint32_t(::strlen(buffer_p));
      }

    m_str_ref_p = m_str_ref_p->reuse_or_new(buffer_p, length, size, deallocate);
    m_str_ref_p->m_cstr_p[length] = '\0';  // Put in null-terminator
    }
  else  // nullptr, so create empty string with specified buffer size
    {
    ensure_size_empty(size);
    }
  }

//---------------------------------------------------------------------------------------
// Sets the C-String character buffer.  Refers to a persistent null-terminated
//             C-String or makes its own copy of an existing character array.
// Arg         cstr_p - pointer to array of characters (does not need to be null
//             terminated unless length is set to ALength_calculate or 'persistent' is
//             true).  'cstr_p' should never be nullptr.  'cstr_p' will usually be a string
//             literal or if 'persistent' is false, 'cstr_p' may be any C-String that
//             this string should make a copy of.
// Arg         length - number of characters to use in 'cstr_p' and the index position to
//             place a terminating null character.  The given length must not be more
//             than the size of 'cstr_p' and the C-String buffer pointed to by 'cstr_p'
//             should not have any null characters less then the given length.  A null
//             terminator is placed only if 'persistent' is not true.
//             'length' may also be set to ALength_calculate in which case the character
//             length is calculated by finding the first terminating null character
//             already present in 'cstr_p'.
// Arg         persistent - Indicates whether the data pointed to by cstr_p will be
//             available for the lifetime of this string.  If 'persistent' is true, the
//             memory pointed to by 'cstr_p' will be used rather than having this object
//             allocate its own memory - also the memory pointed to by 'cstr_p' will not
//             be written to.  If 'persistent' is false, then this AString object will
//             allocate its own memory and copy the contents of 'cstr_p' to it.
// Examples:   str.set_cstr("Hello");
// See:        set_buffer(), operator=(cstr), append(), insert(), AString(cstr_p),
//             AString(cstr_p, length, persistent), AString(buffer_p, size, length, deallocate)
// Author(s):   Conan Reis
void AString::set_cstr(
  const char * cstr_p,
  uint32_t     length,    // = ALength_calculate
  bool         persistent // = true
  )
  {
  A_ASSERT(cstr_p != nullptr, "Given nullptr instead of valid C-String", ErrId_null_cstr, AString);

  if (length == ALength_calculate)
    {
    length = uint32_t(::strlen(cstr_p));
    }

  if (persistent)
    {
    m_str_ref_p->dereference();
    m_str_ref_p = AStringRef::pool_new(cstr_p, length, length + 1u, 1u, false, true);
    }
   else  // Not persistent - make a copy
    {
    ensure_size_buffer(length);

    memcpy(m_str_ref_p->m_cstr_p, cstr_p, length);
    m_str_ref_p->m_cstr_p[length] = '\0';  // Put in null-terminator
    m_str_ref_p->m_length         = length;
    }
  }

//---------------------------------------------------------------------------------------
//  Removes all the characters of match_type from the beginning of the string
//              until it encounters a character that is not match_type.
// Arg          match_type - classification of characters to match.
//              (Default ACharMatch_white_space)
// Examples:    AString str("  the quick brown fox  ");
// 
//              str.trim();  // str is now "the quick brown fox  "
// See:         truncate(), crop(), remove_all(match_type), find(match_type), not_char_type()
// Author(s):    Conan Reis
// Efficiency   See efficiency comment for remove_all().
void AString::trim(
  eACharMatch match_type // = ACharMatch_white_space
  )
  {
  uint32_t find_pos;

  if (find(not_char_type(match_type), 1u, &find_pos))
    {
    // $Revisit - CReis this code could be transplanted here for extra speed.
    crop_quick(find_pos, m_str_ref_p->m_length - find_pos);
    }
  else
    {
    empty();
    }
  }

//---------------------------------------------------------------------------------------
//  Removes all the characters of match_type from the end of the string
//              until it encounters a character that is not match_type.
// Arg          match_type - classification of characters to match.
//              (Default ACharMatch_white_space)
// Examples:    AString str("  the quick brown fox  ");
// 
//              str.truncate();  // str is now "  the quick brown fox"
// See:         trim(), crop(), remove_all(match_type), find(match_type), not_char_type()
// Author(s):    Conan Reis
void AString::truncate(
  eACharMatch match_type // = ACharMatch_white_space
  )
  {
  uint32_t find_pos;

  if (find_reverse(not_char_type(match_type), 1u, &find_pos))
    {
    // $Revisit - CReis this code could be transplanted here for extra speed.
    crop_quick(0u, find_pos + 1u);
    }
  else
    {
    empty();
    }
  }

//---------------------------------------------------------------------------------------
//  Add-assignment operator:  string += cstring
// Returns:     a reference to itself
// Arg          cstr_p - Null-terminated C-sting to append.  'cstr_p' should never be 
//              nullptr.
// See:         append(), insert(), set_cstr(), set_buffer(), add(), operator+=(),
//              operator+(), operator=()
// Author(s):    Conan Reis
AString & AString::operator+=(const char * cstr_p)
  {
  A_ASSERT(cstr_p != nullptr, "Given nullptr instead of valid C-String", ErrId_null_cstr, AString);

  uint32_t length = uint32_t(::strlen(cstr_p));

  if (length)
    {
    uint32_t total_length = m_str_ref_p->m_length + length;  // Calculate total length of string

    ensure_size(total_length);
    
    // concatenate new string
    memcpy(m_str_ref_p->m_cstr_p + m_str_ref_p->m_length, cstr_p, size_t(length));
    
    m_str_ref_p->m_length               = total_length;
    m_str_ref_p->m_cstr_p[total_length] = '\0';    // Place a terminating character
    }

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Add - concatenate this string and another string to form a new string.
// Returns:    new string resulting from the concatenation of this string and 'str'
// Arg         str - string to append
// Notes:      Note that the name add() is used as a convention rather than as_append()
// Author(s):   Conan Reis
AString AString::add(const AString & str) const
  {
  uint32_t length_this = m_str_ref_p->m_length;
  uint32_t length_str  = str.m_str_ref_p->m_length;
  uint32_t length_new  = length_this + length_str;
  uint32_t size        = length_new + 1u;
  char *   buffer_p    = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, m_str_ref_p->m_cstr_p, size_t(length_this));  
  ::memcpy(buffer_p + length_this, str.m_str_ref_p->m_cstr_p, size_t(length_str));

  // Add null terminator by hand rather than copying it from str to ensure that it exists.
  buffer_p[length_new] = '\0';

  return AStringRef::pool_new(buffer_p, length_new, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
// Add - concatenate this string and a C-string to form a new string.
// Returns:    new string resulting from the concatenation of this string and 'cstr_p'
// Arg         cstr_p - pointer to array of characters (does not need to be null
//             terminated unless length is equal to ALength_calculate).  'cstr_p' should
//             never be nullptr.
// Arg         length - number of characters to use in 'cstr_p' and used to determine a
//             position to place a terminating null character.  The given length must not
//             be more than the size of 'cstr_p' and 'cstr_p' should not have any null
//             characters less then the given length.  'length' may also be set to
//             ALength_calculate in which case the character length is calculated by
//             finding the first terminating null character already present in 'cstr_p'.
// Notes:      Note that the name add() is used as a convention rather than as_append()
// Author(s):   Conan Reis
AString AString::add(
  const char * cstr_p,
  uint32_t     length // = ALength_calculate
  ) const
  {
  if (length == ALength_calculate)
    {
    length = uint32_t(::strlen(cstr_p));
    }

  uint32_t length_this = m_str_ref_p->m_length;
  uint32_t length_new  = length_this + length;
  uint32_t size        = AStringRef::request_char_count(length_new);
  char *   buffer_p    = AStringRef::alloc_buffer(size);

  ::memcpy(buffer_p, m_str_ref_p->m_cstr_p, size_t(length_this));  
  ::memcpy(buffer_p + length_this, cstr_p, size_t(length));
  buffer_p[length_new] = '\0';  // Put in null-terminator

  return AStringRef::pool_new(buffer_p, length_new, size, 0u, true, false);
  }

//---------------------------------------------------------------------------------------
//  Character concatenation:  string + ch
// Author(s):    Conan Reis
AString AString::add(char ch) const
  {
  if (ch != '\0')
    {
    uint32_t length_this = m_str_ref_p->m_length;
    uint32_t size        = AStringRef::request_char_count(length_this + 1u);
    char *   buffer_p    = AStringRef::alloc_buffer(size);

    ::memcpy(buffer_p, m_str_ref_p->m_cstr_p, size_t(length_this));  
    buffer_p[length_this]      = ch;
    buffer_p[length_this + 1u] = '\0';  // Put in null-terminator

    return AStringRef::pool_new(buffer_p, length_this + 1u, size, 0u, true, false);
    }

  return *this;
  }

//---------------------------------------------------------------------------------------
//  Copies itself to reversed_p with char_count characters starting at pos in
//              reversed order.
// Arg          reversed_p - address to store a copy of this string with all or a portion
//              of its characters reversed.
// Arg          pos - character index to begin reversing characters.  (Default 0u)
// Arg          char_count - number of characters to reverse.  If char_count is
//              ALength_remainder, the number of characters is equal to the length of the
//              string - pos.  (Default ALength_remainder)
// Examples:    AString str("12345");
//              str.as_reverse(&str_rev);  // str_rev = "54321"
// See:         as_reverse(pos, char_count), reverse(), find_reverse()
// Author(s):    Conan Reis
void AString::as_reverse(
  AString * reversed_p,
  uint32_t  pos,       // = 0u
  uint32_t  char_count // = ALength_remainder
  ) const
  {
  if (char_count == ALength_remainder)  // default
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  #ifdef A_BOUNDS_CHECK
    span_check(pos, char_count, "as_reverse");
  #endif

  if (char_count > 1u)  // If less than 2 characters are reversed, there is no point
    {
    reversed_p->ensure_size_empty(m_str_ref_p->m_length);

    char * rev_cstr_p = reversed_p->m_str_ref_p->m_cstr_p;  // Local caching

    // Copy initial non-reversed portion - if any
    if (pos)
      {
      ::memcpy(rev_cstr_p, m_str_ref_p->m_cstr_p, size_t(pos));
      }


    // Copy reversed portion
    char * cstr_p         = m_str_ref_p->m_cstr_p + pos;
    char * cstr_end_p     = cstr_p + char_count;
    char * reversed_end_p = rev_cstr_p + pos + char_count - 1;

    for (; cstr_p < cstr_end_p; cstr_p++, reversed_end_p--)
      {
      *reversed_end_p = *cstr_p;
      }


    // Copy remaining non-reversed portion - if any
    if ((pos + char_count) < m_str_ref_p->m_length)
      {
      // This copy includes the null terminator
      ::memcpy(rev_cstr_p + pos + char_count, m_str_ref_p->m_cstr_p, size_t(m_str_ref_p->m_length - pos - char_count + 1u));
      }
    else
      {
      rev_cstr_p[m_str_ref_p->m_length] = '\0';
      }

    reversed_p->m_str_ref_p->m_length = m_str_ref_p->m_length;
    }
  else  // No reversing - just a standard copy
    {
    *reversed_p = *this;
    }
  }

//---------------------------------------------------------------------------------------
// Determines the number of occurrences of the specified character.
// 
// Returns:     number of occurrences of specified character
// Arg          ch - character to count
// Arg          start_pos - starting position of count search range (Default 0)
// Arg          end_pos - ending position of count search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched.  (Default ALength_remainder)
// Arg          last_counted_p - optional address to store index of last counted character.
//              If nothing is counted then it is set to start_pos.
// Examples:    uint32_t underscores = str.count('_');
// See:         find(), get()
// Author(s):    Conan Reis
uint32_t AString::count(
  char       ch,
  uint32_t   start_pos,     // = 0
  uint32_t   end_pos,       // = ALength_remainder
  uint32_t * last_counted_p // = nullptr
  ) const
  {
  // $Revisit - CReis [Efficiency] This should be faster than repeatedly calling memchr() - profile.
  // It could be rewritten using inline assembly.

  // Ensure not empty
  if (m_str_ref_p->m_length == 0u)
    {
    return 0u;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = m_str_ref_p->m_length - 1u;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, "count");
  #endif

  uint32_t num_count    = 0u;
  char *   cstr_start_p = m_str_ref_p->m_cstr_p;
  char *   cstr_count_p = cstr_start_p + start_pos;
  char *   cstr_p       = cstr_count_p;
  char *   cstr_end_p   = cstr_start_p + end_pos;

  for (; cstr_p <= cstr_end_p; cstr_p++)
    {
    if (*cstr_p == ch)
      {
      cstr_count_p = cstr_p;
      num_count++;
      }
    }

  if (last_counted_p)
    {
    *last_counted_p = uint32_t(reinterpret_cast<uintptr_t>(cstr_count_p) - reinterpret_cast<uintptr_t>(cstr_start_p));
    }

  return num_count;
  }

//---------------------------------------------------------------------------------------
//  Determines the number of occurrences of the specified character.
// Returns:     number of occurrences of specified character
// Arg          match_type - classification of characters to match.
// Arg          start_pos - starting position of count search range (Default 0)
// Arg          end_pos - ending position of count search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched.  (Default ALength_remainder)
// Examples:    uint32_t digits = str.count(ACharMatch_digit);
// See:         find(match_type), get(match_type), not_char_type()
// Author(s):    Conan Reis
uint32_t AString::count(
  eACharMatch match_type,
  uint32_t    start_pos, // = 0
  uint32_t    end_pos    // = ALength_remainder
  ) const
  {
  // $Revisit - CReis [Efficiency] This should be faster than repeatedly calling memchr() - profile.
  // It could be rewritten using inline assembly.

  uint32_t num_count= 0u;

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1u;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "count");
    #endif

    uint8_t * cstr_p        = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + start_pos);
    uint8_t * cstr_end_p    = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + end_pos);
    bool *    match_table_p = ms_char_match_table[match_type];

    for (; cstr_p <= cstr_end_p; cstr_p++)
      {
      if (match_table_p[*cstr_p])
        {
        num_count++;
        }
      }
    }
  return num_count;
  }

//---------------------------------------------------------------------------------------
//  Returns number of substrings found
// Author(s):    Conan Reis
uint32_t AString::count(
  const AStringBM & bm,
  uint32_t          start_pos,  // = 0u
  uint32_t          end_pos     // = ALength_remainder
  ) const
  {
  uint32_t result = 0u;
  uint32_t length = bm.m_str_ref_p->m_length;

  while (find(bm, 1u, &start_pos, start_pos, end_pos) == true)
    {
    result++;
    start_pos += length;  // non-overlapping
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of specified character starting from start_pos and ending
//              at end_pos and if found stores the index position found.
// Returns:     true if instance character found, false if not
// Arg          ch - character to find
// Arg          instance - occurrence of character to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance character if found.
//              It is not modified if the character is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting position of search range (Default 0)
// Arg          end_pos - ending position of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Examples:    if (str.find('_', 2))  // if 2nd underscore is found
//                do_something();
// See:         count(), get()
// Notes:       This character find method is faster than the find method searching for a
//              single character string.
// Author(s):    Conan Reis
bool AString::find(
  char   ch,
  uint32_t   instance,   // = 1
  uint32_t * find_pos_p, // = nullptr
  uint32_t   start_pos,  // = 0
  uint32_t   end_pos     // = ALength_remainder
  ) const
  {
  // $Revisit - CReis [Efficiency] This should be faster than repeatedly calling memchr() - profile.
  // It could be rewritten using inline assembly.

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1u;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find");
    #endif

    char * cstr_p     = m_str_ref_p->m_cstr_p + start_pos;
    char * cstr_end_p = m_str_ref_p->m_cstr_p + end_pos;

    while (cstr_p <= cstr_end_p)
      {
      if (*cstr_p == ch)     // Found one
        {
        if (instance == 1u)  // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_p - m_str_ref_p->m_cstr_p);
            }

          return true;
          }

        instance--;
        }

      cstr_p++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of specified type of character starting from start_pos and
//              ending at end_pos and if found stores the index position found.
// Returns:     true if instance character type found, false if not
// Arg          match_type - classification of characters to match.
// Arg          instance - occurrence of character type to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance character type if found.
//              It is not modified if the character type is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting position of search range (Default 0)
// Arg          end_pos - ending position of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Examples:    if (str.find(ACharMatch_lowercase, 2))  // if 2nd lowercase letter is found
//                do_something();
// See:         count(), get(), not_char_type()
// Author(s):    Conan Reis
bool AString::find(
  eACharMatch match_type,
  uint32_t    instance,   // = 1
  uint32_t *  find_pos_p, // = nullptr
  uint32_t    start_pos,  // = 0
  uint32_t    end_pos     // = ALength_remainder
  ) const
  {
  uint32_t length = m_str_ref_p->m_length;

  if (length && (start_pos < length))  // if not empty or skipped
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = length - 1u;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find");
    #endif

    uint8_t * cstr_p        = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + start_pos);
    uint8_t * cstr_end_p    = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + end_pos);
    bool *    match_table_p = ms_char_match_table[match_type];

    while (cstr_p <= cstr_end_p)
      {
      if (match_table_p[*cstr_p])  // Found one
        {
        if (instance == 1u)        // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_p - reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p));
            }

          return true;
          }

        instance--;
        }

      cstr_p++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of substring str starting from start_pos and ending
//              at end_pos and if found stores the index position found.
// Returns:     true if instance of str found, false if not
// Arg          str - substring to find
// Arg          args_p - find info - see AFindStrArgs declaration.
//              (Default &AFindStrArgs::ms_sensitive)
// Examples:    AString sub_str("hello");
//              if (str.find(sub_str, &AFindStrArgs(2)))  // if 2nd "hello" substring is found
//                do_something();
// See:         find(bm), count(), get()
// Notes:       [Efficiency] This method should be faster than calling strstr() - profile.
//              The case insensitive routine could probably be made more efficient - via
//              a lowercase table and/or only converting the characters once in temporary
//              C-String buffers.
// Author(s):    Conan Reis
/*
bool AString::find2(
  const AString & str,
  AFindStrArgs *  args_p // = &AFindStrArgs::ms_sensitive
  ) const
  {
  if (m_str_ref_p->m_length && (m_str_ref_p->m_length >= str.m_str_ref_p->m_length))  // if not empty and str is no larger than this string
    {
    uint32_t end_pos  = args_p->m_end_pos;
    uint32_t instance = args_p->m_instance;

    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(args_p->m_start_pos, end_pos, instance, "find");
    #endif

    char * match_p;
    char * find_p;
    char * cstr_p       = m_str_ref_p->m_cstr_p + args_p->m_start_pos;
    char * cstr_end_p   = m_str_ref_p->m_cstr_p + end_pos - str.m_str_ref_p->m_length - 1;  // won't match if less than str left
    char * find_start_p = str.m_str_ref_p->m_cstr_p;
    char * find_end_p   = find_start_p + str.m_str_ref_p->m_length;

    if (args_p->m_case_check == AStrCase_sensitive)  // Case sensitive
      {
      while (cstr_p <= cstr_end_p)
        {
        match_p = cstr_p;
        find_p  = find_start_p;

        while ((find_p < find_end_p) && (*match_p == *find_p))
          {
          find_p++;
          match_p++;
          }

        if (find_p == find_end_p)  // Found one
          {
          if (instance == 1u)      // Found it!
            {
            args_p->m_find_start = uint32_t(cstr_p - m_str_ref_p->m_cstr_p);
            args_p->m_find_end   = args_p->m_find_start + str.m_str_ref_p->m_length;
            return true;
            }

          instance--;
          cstr_p = match_p;
          }
        else
          {
          cstr_p++;
          }
        }
      }
    else  // Ignore case
      {
      while (cstr_p <= cstr_end_p)
        {
        match_p = cstr_p;
        find_p  = find_start_p;

        while ((find_p < find_end_p) && !compare_insensitive(*match_p, *find_p))
          {
          find_p++;
          match_p++;
          }

        if (find_p == find_end_p)  // Found one
          {
          if (instance == 1u)      // Found it!
            {
            args_p->m_find_start = uint32_t(cstr_p - m_str_ref_p->m_cstr_p);
            args_p->m_find_end   = args_p->m_find_start + str.m_str_ref_p->m_length;
            return true;
            }

          instance--;
          cstr_p = match_p;
          }
        else
          {
          cstr_p++;
          }
        }
      }
    }

  return false;
  }      
*/

//---------------------------------------------------------------------------------------
//  Finds instance of substring str starting from start_pos and ending
//              at end_pos and if found stores the index position found.
// Returns:     true if instance of str found, false if not
// Arg          str - substring to find
// Arg          instance - occurrence of substring to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting character index of search range (Default 0)
// Arg          end_pos - ending character index of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Arg          case_check - if set to AStrCase_sensitive, case is important (q != Q), if set
//              to AStrCase_ignore, case is not important (q == Q).  (Default AStrCase_sensitive)
// Examples:    AString sub_str("hello");
//              if (str.find(sub_str, 2))  // if 2nd "hello" substring is found
//                do_something();
// See:         find(bm), count(), get()
// Notes:       [Efficiency] This method should be faster than calling strstr() - profile.
//              The case insensitive routine could probably be made more efficient - via
//              a lowercase table and/or only converting the characters once in temporary
//              C-String buffers.
// Author(s):    Conan Reis
bool AString::find(
  const AString & str,
  uint32_t        instance,    // = 1
  uint32_t *      find_pos_p,  // = nullptr
  uint32_t        start_pos,   // = 0
  uint32_t        end_pos,     // = ALength_remainder
  eAStrCase       case_check   // = AStrCase_sensitive
  ) const
  {
  if (m_str_ref_p->m_length && (m_str_ref_p->m_length >= str.m_str_ref_p->m_length))  // if not empty and str is no larger than this string
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find");
    #endif

    char * match_p;
    char * find_p;
    char * cstr_p       = m_str_ref_p->m_cstr_p + start_pos;
    char * cstr_end_p   = m_str_ref_p->m_cstr_p + end_pos - str.m_str_ref_p->m_length + 1u;  // won't match if less than str left
    char * find_start_p = str.m_str_ref_p->m_cstr_p;
    char * find_end_p   = find_start_p + str.m_str_ref_p->m_length;

    if (case_check == AStrCase_sensitive)  // Case sensitive
      {
      while (cstr_p <= cstr_end_p)
        {
        match_p = cstr_p;
        find_p  = find_start_p;

        while ((*match_p == *find_p) && (find_p < find_end_p))    //  Note: Okay to check end condition (find_p < find_end_p) second because if >= it will point to null terminator i.e. still valid memory in first part of condition.
          {
          find_p++;
          match_p++;
          }

        if (find_p == find_end_p)  // Found one
          {
          if (instance == 1u)      // Found it!
            {
            if (find_pos_p)
              {
              *find_pos_p = uint32_t(cstr_p - m_str_ref_p->m_cstr_p);
              }

            return true;
            }

          instance--;
          cstr_p = match_p;
          }
        else
          {
          cstr_p++;
          }
        }
      }
    else  // Ignore case
      {
      while (cstr_p <= cstr_end_p)
        {
        match_p = cstr_p;
        find_p  = find_start_p;

        while (!compare_insensitive(*match_p, *find_p) && (find_p < find_end_p))  //  Note: Okay to check end condition (find_p < find_end_p) second because if >= it will point to null terminator i.e. still valid memory in first part of condition.
          {
          find_p++;
          match_p++;
          }

        if (find_p == find_end_p)  // Found one
          {
          if (instance == 1u)      // Found it!
            {
            if (find_pos_p)
              {
              *find_pos_p = uint32_t(cstr_p - m_str_ref_p->m_cstr_p);
              }

            return true;
            }

          instance--;
          cstr_p = match_p;
          }
        else
          {
          cstr_p++;
          }
        }
      }
    }

  return false;
  }      

//---------------------------------------------------------------------------------------
//  Quick substring search using the Boyer Moore algorithm.  Good for
//              multiple searches for the same substring.  Finds instance of substring
//              bm starting from start_pos and ending
//              at end_pos and if found stores the index position found.
// Returns:     true if instance of bm found, false if not
// Arg          bm - Boyer Moore substring to find
// Arg          instance - occurrence of substring to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting character index of search range (Default 0)
// Arg          end_pos - ending character index of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Arg          case_check - if set to AStrCase_sensitive, case is important (q != Q), if set
//              to AStrCase_ignore, case is not important (q == Q).  (Default AStrCase_sensitive)
// Examples:    AStringBM sub_str("hello");
//              if (str.find(sub_str, 2))  // if 2nd "hello" substring is found
//                do_something();
// See:         find(bm), count(), get()
// Notes:       [Efficiency]  The case insensitive routine could probably be made more
//              efficient - via a lowercase table and/or only converting the characters
//              once in temporary C-String buffers.
// Author(s):    Conan Reis
bool AString::find(
  const AStringBM & bm,
  uint32_t          instance,   // = 1
  uint32_t *        find_pos_p, // = nullptr
  uint32_t          start_pos,  // = 0
  uint32_t          end_pos     // = ALength_remainder
  ) const
  {

  // $Revisit - CReis Rewrite using pointers rather than indexes

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find");
    #endif

    char            insensitive_ch;
    uint32_t        bm_pos;                                // bm_pos is an index into pattern
    uint32_t        patlen     = bm.get_length();          // store pattern length locally (high use)
    uint32_t        target_pos = start_pos + patlen - 1u;  // target_pos is the index into the target_p
    const char *    cstr_p     = m_str_ref_p->m_cstr_p;    // store cstr locally (high use)
    const char *    bm_cstr_p  = bm.m_str_ref_p->m_cstr_p; // store bm cstr locally (high use)
    const uint8_t * delta_p    = bm.m_delta_p;             // store delta table locally (high use)
  
    if (bm.m_case == AStrCase_ignore)            // same, but to_lowercase used
      {
      while (target_pos <= end_pos)
        {
        bm_pos         = patlen - 1u;
        insensitive_ch = to_lowercase(cstr_p[target_pos]);

        while (bm_cstr_p[bm_pos] == insensitive_ch)  // while corresponding chars match
          {
          if (bm_pos > 0u)                // Still characters to check                         
            {                             // move left one character for next comparison
            --bm_pos;                     
            --target_pos;                 
            }                             
          else                            // pattern found!
            {
            if (instance == 1u)           // correct number of occurrences found!
              {                           
              if (find_pos_p)               
                {
                *find_pos_p = target_pos;   
                }

              return true;              
              }                           
            else                          // one less instance to find
              {
              instance--;
              }

            break;                        // Ugly, I know - could be [bm_pos = bm.get_length]
            }

          insensitive_ch = to_lowercase(cstr_p[target_pos]);
          }

        target_pos += a_max(static_cast<uint32_t>(delta_p[uint8_t(insensitive_ch)]), (patlen - bm_pos));  // move target_p index by delta value of mismatched character
        }
      }
    else  // Case sensitive
      {
      while (target_pos <= end_pos)
        {
        bm_pos = patlen - 1u;

        while (bm_cstr_p[bm_pos] == cstr_p[target_pos])  // while corresponding chars match
          {
          if (bm_pos > 0u)                // Still characters to check                         
            {                             // move left one character for next comparison
            --bm_pos;
            --target_pos;
            }
          else                            // pattern found!
            {
            if (instance == 1u)           // correct number of occurrences found!
              {
              if (find_pos_p)
                {
                *find_pos_p = target_pos;
                }

              return true;
              }
            else                          // one less instance to find
              {
              instance--;
              }

            break;                        // Ugly, I know - could be [bm_pos = bm.get_length]
            }
          }

        target_pos += a_max(static_cast<uint32_t>(delta_p[uint8_t(cstr_p[target_pos])]), (patlen - bm_pos));  // move target_p index by delta value of mismatched character
        }
      }
    }

  return false;
  }


//---------------------------------------------------------------------------------------
//  Finds instance of substring str starting from start_pos and ending at end_pos using a
//              fuzzy search and if found stores the start and end position of the found text.
// Returns:     true if instance of str found, false if not
// Arg          str - substring to find
// Arg          instance - occurrence of substring to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_start_p - address to store index of instance of the beginning of the substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          find_end_p - address to store index of instance of the end of the substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting character index of search range (Default 0)
// Arg          end_pos - ending character index of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Arg          case_check - if set to AStrCase_sensitive, case is important (q != Q), if set
//              to AStrCase_ignore, case is not important (q == Q).  (Default AStrCase_sensitive)
// Examples:    AString source("abcde_abxxc");
//              if (source.find_fuzzy("ac", 1))  // returns true, find_first_p returns 0, find_end_p return 2
//                do_something();
// See:         find(bm), count(), get()
// Notes:       Matches do not cross line terminators.
// Author(s):    John Stenersen
bool AString::find_fuzzy(
  const AString & str,
  uint32_t        instance,     //  = 1
  uint32_t *      find_start_p, //  = nullptr
  uint32_t *      find_end_p,   //  = nullptr
  uint32_t        start_pos,    //  = 0
  uint32_t        end_pos,      //  = ALength_remainder
  eAStrCase       case_check    //  = AStrCase_sensitive
  ) const
  {
  if (!str.m_str_ref_p->m_length || (str.m_str_ref_p->m_length > m_str_ref_p->m_length))
    {
    //  The key string is empty or longer than the text being searched.
    return false;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = m_str_ref_p->m_length - 1;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, instance, "find_fuzzy");
  #endif

  const char * key_start_p  = str.m_str_ref_p->m_cstr_p;
  const char * key_end_p    = key_start_p + str.m_str_ref_p->m_length - 1;
  const char * text_start_p = & m_str_ref_p->m_cstr_p[start_pos];
  const char * text_end_p   = & m_str_ref_p->m_cstr_p[end_pos - 1];
  const char * first_p      = nullptr;
  const char * next_p       = nullptr;

  if (case_check == AStrCase_sensitive)  // Case sensitive
    {
    do  // Each instance
      {
      //  Fnd the first character match.
      while (text_start_p <= text_end_p)
        {
        if (*text_start_p == *key_start_p)
          {
          first_p = text_start_p;
          break;
          }
        text_start_p ++;
        }

      if (!first_p)
        {
        //  Did not find the first character match.
        return false;
        }

      //  For the remainder of the key, find a matching character in the text.
      key_start_p ++;
      text_start_p ++;
      next_p = first_p;

      while (key_start_p <= key_end_p)
        {
        //  Find the next character match.
        next_p = nullptr;
        while ((text_start_p <= text_end_p) && (*text_start_p != '\n') && (*text_start_p != '\r'))
          {
          if (*text_start_p == *key_start_p)
            {
            next_p = text_start_p;
            break;
            }
          text_start_p ++;
          }

        if ((*text_start_p == '\n') || (*text_start_p == '\r'))
          {
          //  Start over on next line.
          next_p        = text_start_p;
          first_p       = nullptr;
          key_start_p   = str.m_str_ref_p->m_cstr_p;
          instance ++;
          break;
          }

        if (!next_p)
          {
          return false; //  Match not found.
          }

        key_start_p ++;
        text_start_p ++;
        }

        //  Found a match.
        instance --;
        if (!instance)
          {
          //  The instance of the match.
          if (find_start_p)
            {
            * find_start_p = (uint32_t) (first_p - m_str_ref_p->m_cstr_p);
            }
          if (find_end_p)
            {
            * find_end_p = (uint32_t) (next_p - m_str_ref_p->m_cstr_p + 1);
            }
          return true;
          }

        text_start_p  = next_p + 1;
        first_p       = nullptr;
        key_start_p   = str.m_str_ref_p->m_cstr_p;
      } while (instance);
    }
  else  // Ignore case
    {
    do  // Each instance
      {
      //  Fnd the first character match.
      while (text_start_p <= text_end_p)
        {
        if (!compare_insensitive(*text_start_p, *key_start_p))
          {
          first_p = text_start_p;
          break;
          }
        text_start_p ++;
        }

      if (!first_p)
        {
        //  Did not find the first character match.
        return false;
        }

      //  For the remainder of the key, find a matching character in the text.
      key_start_p ++;
      text_start_p ++;
      next_p = first_p;

      while (key_start_p <= key_end_p)
        {
        //  Find the next character match.
        next_p = nullptr;
        while ((text_start_p <= text_end_p) && (*text_start_p != '\n') && (*text_start_p != '\r'))
          {
          if (!compare_insensitive(*text_start_p, *key_start_p))
            {
            next_p = text_start_p;
            break;
            }
          text_start_p ++;
          }

        if ((*text_start_p == '\n') || (*text_start_p == '\r'))
          {
          //  Start over on next line.
          next_p        = text_start_p;
          first_p       = nullptr;
          key_start_p   = str.m_str_ref_p->m_cstr_p;
          instance ++;
          break;
          }

        if (!next_p)
          {
          return false; //  Match not found.
          }

        key_start_p ++;
        text_start_p ++;
        }

        //  Found a match.
        instance --;
        if (!instance)
          {
          //  The instance of the match.
          if (find_start_p)
            {
            * find_start_p = (uint32_t) (first_p - m_str_ref_p->m_cstr_p);
            }
          if (find_end_p)
            {
            * find_end_p = (uint32_t) (next_p - m_str_ref_p->m_cstr_p + 1);
            }
          return true;
          }

        text_start_p  = next_p + 1;
        first_p       = nullptr;
        key_start_p   = str.m_str_ref_p->m_cstr_p;
      } while (instance);
    }

  return false;   //  Not found.
  } //  AString::find_fuzzy()


//---------------------------------------------------------------------------------------
//  Finds in reverse direction the instance of substring str starting from start_pos and ending at end_pos using a
//              fuzzy search and if found stores the start and end position of the found text.
// Returns:     true if instance of str found, false if not
// Arg          str - substring to find
// Arg          instance - occurrence of substring to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_start_p - address to store index of instance of the beginning of the substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          find_end_p - address to store index of instance of the end of the substring if found.
//              It is not modified if the substring is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - starting character index of search range (Default 0)
// Arg          end_pos - ending character index of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Arg          case_check - if set to AStrCase_sensitive, case is important (q != Q), if set
//              to AStrCase_ignore, case is not important (q == Q).  (Default AStrCase_sensitive)
// Examples:    AString source("abcde_abxxc");
//              if (source.find_fuzzy_reverse("ac", 1))  // returns true, find_first_p returns 6, find_end_p return 10
//                do_something();
// See:         find(bm), count(), get()
// Notes:       Matches do not cross line terminators.
// Author(s):    John Stenersen
bool AString::find_fuzzy_reverse(
  const AString & str,
  uint32_t        instance,     //  = 1
  uint32_t *      find_start_p, //  = nullptr
  uint32_t *      find_end_p,   //  = nullptr
  uint32_t        start_pos,    //  = 0
  uint32_t        end_pos,      //  = ALength_remainder
  eAStrCase       case_check    //  = AStrCase_sensitive
  ) const
  {
  if (!str.m_str_ref_p->m_length || (str.m_str_ref_p->m_length > m_str_ref_p->m_length))
    {
    //  The key string is empty or longer than the text being searched.
    return false;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = m_str_ref_p->m_length - 1;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, instance, "find_fuzzy");
  #endif

  const char * key_start_p  = str.m_str_ref_p->m_cstr_p;
  const char * key_end_p    = key_start_p + str.m_str_ref_p->m_length - 1;
  const char * text_start_p = & m_str_ref_p->m_cstr_p[start_pos];
  const char * text_end_p   = & m_str_ref_p->m_cstr_p[end_pos - 1];
  const char * last_p       = nullptr;
  const char * prev_p       = nullptr;

  if (case_check == AStrCase_sensitive)  // Case sensitive
    {
    do  // Each instance
      {
      //  Fnd the last character match.
      while (text_start_p <= text_end_p)
        {
        if (*text_end_p == *key_end_p)
          {
          last_p = text_end_p;
          break;
          }
        text_end_p --;
        }

      if (!last_p)
        {
        //  Did not find the last character match.
        return false;
        }

      //  For the remainder of the key, find a matching character in the text.
      key_end_p --;
      text_end_p --;
      prev_p = last_p;

      while (key_start_p <= key_end_p)
        {
        //  Find the prev character match.
        prev_p = nullptr;
        while ((text_start_p <= text_end_p) && (*text_end_p != '\n') && (*text_end_p != '\r'))
          {
          if (*text_end_p == *key_end_p)
            {
            prev_p = text_end_p;
            break;
            }
          text_end_p --;
          }

        if ((*text_end_p == '\n') || (*text_end_p == '\r'))
          {
          //  Start over on next line.
          prev_p    = text_end_p;
          last_p    = nullptr;
          key_end_p = key_start_p + str.m_str_ref_p->m_length - 1;
          instance ++;
          break;
          }

        if (!prev_p)
          {
          return false; //  Match not found.
          }

        key_end_p --;
        text_end_p --;
        }

        //  Found a match.
        instance --;
        if (!instance)
          {
          //  The instance of the match.
          if (find_start_p)
            {
            * find_start_p = (uint32_t) (prev_p - m_str_ref_p->m_cstr_p);
            }
          if (find_end_p)
            {
            * find_end_p = (uint32_t) (last_p - m_str_ref_p->m_cstr_p + 1);
            }
          return true;
          }

        text_end_p  = prev_p - 1;
        last_p      = nullptr;
        key_end_p   = key_start_p + str.m_str_ref_p->m_length - 1;
      } while (instance);
    }
  else  // Ignore case
    {
    do  // Each instance
      {
      //  Fnd the last character match.
      while (text_start_p <= text_end_p)
        {
        if (!compare_insensitive(*text_end_p, *key_end_p))
          {
          last_p = text_end_p;
          break;
          }
        text_end_p --;
        }

      if (!last_p)
        {
        //  Did not find the last character match.
        return false;
        }

      //  For the remainder of the key, find a matching character in the text.
      key_end_p --;
      text_end_p --;
      prev_p = last_p;

      while (key_start_p <= key_end_p)
        {
        //  Find the prev character match.
        prev_p = nullptr;
        while ((text_start_p <= text_end_p) && (*text_end_p != '\n') && (*text_end_p != '\r'))
          {
          if (!compare_insensitive(*text_end_p, *key_end_p))
            {
            prev_p = text_end_p;
            break;
            }
          text_end_p --;
          }

        if ((*text_end_p == '\n') || (*text_end_p == '\r'))
          {
          //  Start over on next line.
          prev_p    = text_end_p;
          last_p    = nullptr;
          key_end_p = key_start_p + str.m_str_ref_p->m_length - 1;
          instance ++;
          break;
          }

        if (!prev_p)
          {
          return false; //  Match not found.
          }

        key_end_p --;
        text_end_p --;
        }

        //  Found a match.
        instance --;
        if (!instance)
          {
          //  The instance of the match.
          if (find_start_p)
            {
            * find_start_p = (uint32_t) (prev_p - m_str_ref_p->m_cstr_p);
            }
          if (find_end_p)
            {
            * find_end_p = (uint32_t) (last_p - m_str_ref_p->m_cstr_p + 1);
            }
          return true;
          }

        text_end_p  = prev_p - 1;
        last_p      = nullptr;
        key_end_p   = key_start_p + str.m_str_ref_p->m_length - 1;
      } while (instance);
    }

  return false;   //  Not found.
  } //  AString::find_fuzzy_reverse()


//---------------------------------------------------------------------------------------
// Determine column that starts indent - i.e. first character from `start_pos` that is
// non-space (not ` ` or `\t`). Takes into account tab stops and adjusts column if any
// tab `\t` characters encountered.
// 
// Returns:   0-based row/line number
// Params:  
//   tab_stops:    tab stop count in spaces
//   indent_idx_p: address to store 0-based index position. Ignored if nullptr.
//   start_pos:    starting index to begin
//   end_pos:      ending index to stop
//
// See:  
//   index_to_row(), row_to_index(), line_indent(), line_unindent(), advance_to_indent()
//   
// Author(s):   Conan Reis
uint32_t AString::find_indent_column(
  uint32_t   tab_stops,    // = AString_tab_stop_def
  uint32_t * indent_idx_p, // = nullptr
  uint32_t   start_pos,    // = 0u
  uint32_t   end_pos       // = ALength_remainder
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup
  AStringRef * str_ref_p = m_str_ref_p;
  uint32_t     length    = str_ref_p->m_length;

  if (length == 0u)
    {
    if (indent_idx_p)
      {
      *indent_idx_p = 0u;
      }

    return 0u;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = length - 1u;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, "indent");
  #endif

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t     column   = 0u;
  char *       buffer_p = str_ref_p->m_cstr_p;
  const char * indent_p =
    advance_to_indent(buffer_p + start_pos, buffer_p + end_pos, tab_stops, &column);

  if (indent_idx_p)
    {
    *indent_idx_p = uint(indent_p - buffer_p);
    }

  return column;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of specified character in reverse starting from end_pos
//              and ending at start_pos and if found stores the index position found.
// Returns:     true if instance character found, false if not
// Arg          ch - character to find
// Arg          instance - occurrence of character to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance character if found.
//              It is not modified if the character is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - ending position of search range (Default 0)
// Arg          end_pos - starting position of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Examples:    if (str.find_reverse('_', 2))  // if 2nd to last underscore is found
//                do_something();
// See:         count(), get()
// Notes:       This character find method is faster than the find_reverse() method
//              searching for a single character string.
// Author(s):    Conan Reis
bool AString::find_reverse(
  char       ch,
  uint32_t   instance,   // = 1
  uint32_t * find_pos_p, // = nullptr
  uint32_t   start_pos,  // = 0
  uint32_t   end_pos     // = ALength_remainder
  ) const
  {
  // $Revisit - CReis [Efficiency] This should be faster than repeatedly calling memchr() - profile.
  // It could be rewritten using inline assembly.

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find_reverse");
    #endif

    char * cstr_p     = m_str_ref_p->m_cstr_p + start_pos;
    char * cstr_end_p = m_str_ref_p->m_cstr_p + end_pos;

    while (cstr_p <= cstr_end_p)
      {
      if (*cstr_end_p == ch)     // Found one
        {
        if (instance == 1u)  // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_end_p - m_str_ref_p->m_cstr_p);
            }

          return true;
          }

        instance--;
        }

      cstr_end_p--;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of specified type of character in reverse starting from
//              end_pos and ending at start_pos and if found stores the index position found.
// Returns:     true if instance character type found, false if not
// Arg          match_type - classification of characters to match.
// Arg          instance - occurrence of character type to find.  It may not be less than 1.
//              (Default 1)
// Arg          find_pos_p - address to store index of instance character type if found.
//              It is not modified if the character type is not found or if it is set to nullptr.
//              (Default nullptr)
// Arg          start_pos - ending position of search range (Default 0)
// Arg          end_pos - starting position of search range.  If end_pos is set to
//              ALength_remainder, the entire length of the string (from the starting position)
//              is searched (end_pos = length - start_pos).  (Default ALength_remainder)
// Examples:    if (str.find_reverse(ACharMatch_lowercase, 2))  // if 2nd to last lowercase letter is found
//                do_something();
// See:         count(), get()
// Author(s):    Conan Reis
bool AString::find_reverse(
  eACharMatch match_type,
  uint32_t    instance,   // = 1
  uint32_t *  find_pos_p, // = nullptr
  uint32_t    start_pos,  // = 0
  uint32_t    end_pos     // = ALength_remainder
  ) const
  {
  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, instance, "find_reverse");
    #endif

    uint8_t * cstr_p        = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + start_pos);
    uint8_t * cstr_end_p    = reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p + end_pos);
    bool *    match_table_p = ms_char_match_table[match_type];

    while (cstr_p <= cstr_end_p)
      {
      if (match_table_p[*cstr_end_p ])  // Found one
        {
        if (instance == 1u)             // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_end_p - reinterpret_cast<uint8_t *>(m_str_ref_p->m_cstr_p));
            }

          return true;
          }

        instance--;
        }

      cstr_end_p--;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Substring reverse search.
// 
// Returns: `true` if instance of str found, `false` if not
// 
// Params:
//   str: substring to find
//   instance: occurrence of substring to find.  It may not be less than 1.
//   find_pos_p:
//     Address to store index of instance substring if found.
//     It is not modified if the substring is not found or if it is set to nullptr.
//   start_pos: starting character index of search range.
//   end_pos:
//     Ending character index of search range.  If `end_pos` is set to `ALength_remainder`,
//     the entire length of the string (from the starting position) is searched (`end_pos
//     = length - 1u`).
//   case_check:
//     If set to `AStrCase_sensitive`, case is important (q != Q), if set to
//     `AStrCase_ignore`, case is not important (q == Q).
//     
// See:
//   find_reverse(ch), find_reverse(match_type),
//   find(str), find(bm), find(ch), find(match_type)
bool AString::find_reverse(
  const AString & str,
  uint32_t        instance,    // = 1
  uint32_t *      find_pos_p,  // = nullptr
  uint32_t        start_pos,   // = 0
  uint32_t        end_pos,     // = ALength_remainder
  eAStrCase       case_check   // = AStrCase_sensitive
  ) const
  {
  uint32_t length     = m_str_ref_p->m_length;
  uint32_t str_length = str.m_str_ref_p->m_length;

  if ((length == 0u) || (str_length == 0u) || (length < str_length))
    {
    return false;
    }

  if (end_pos == ALength_remainder)
    {
    end_pos = length;
    }

  #ifdef A_BOUNDS_CHECK
    bounds_check(start_pos, end_pos, instance, "find_reverse");
  #endif

  char * match_p;
  char * find_p;
  char * cstr_p       = m_str_ref_p->m_cstr_p + start_pos;
  char * cstr_end_p   = m_str_ref_p->m_cstr_p + end_pos - str_length + 1u;  // won't match if less than str left
  char * find_start_p = str.m_str_ref_p->m_cstr_p;
  char * find_end_p   = find_start_p + str_length;

  if (case_check == AStrCase_sensitive)  // Case sensitive
    {
    do
      {
      cstr_end_p--;
      match_p = cstr_end_p;
      find_p  = find_start_p;

      while ((find_p < find_end_p) && (*match_p == *find_p))
        {
        find_p++;
        match_p++;
        }

      if (find_p == find_end_p)  // Found one
        {
        if (instance == 1u)      // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_end_p - m_str_ref_p->m_cstr_p);
            }

          return true;
          }

        instance--;
        }
      }
    while (cstr_p < cstr_end_p);
    }
  else  // Ignore case
    {
    do
      {
      cstr_end_p--;
      match_p = cstr_end_p;
      find_p  = find_start_p;

      while (!compare_insensitive(*match_p, *find_p) && (find_p < find_end_p))  //  Note: Okay to check end condition (find_p < find_end_p) second because if >= it will point to null terminator i.e. still valid memory in first part of condition.
        {
        find_p++;
        match_p++;
        }

      if (find_p == find_end_p)  // Found one
        {
        if (instance == 1u)      // Found it!
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(cstr_end_p - m_str_ref_p->m_cstr_p);
            }

          return true;
          }

        instance--;
        }
      }
    while (cstr_p < cstr_end_p);
    }

  return false;
  }


//---------------------------------------------------------------------------------------
//  substring retrieval method
// Author(s):    Conan Reis
void AString::get(
  AString * str_p,
  uint32_t  pos,       // = 0u
  uint32_t  char_count // = ALength_remainder
  ) const
  {
  if (char_count == ALength_remainder)
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  str_p->ensure_size_empty(char_count);

  if (char_count)
    {
    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "get");
    #endif

    memcpy(str_p->m_str_ref_p->m_cstr_p, m_str_ref_p->m_cstr_p + pos, size_t(char_count));
    str_p->m_str_ref_p->m_cstr_p[char_count] = '\0';
    str_p->m_str_ref_p->m_length             = char_count;
    }
  }

//---------------------------------------------------------------------------------------
//  substring retrieval method
// Author(s):    Conan Reis
AString AString::get(
  uint32_t pos,       // = 0u
  uint32_t char_count // = ALength_remainder
  ) const
  {
  if (char_count == ALength_remainder)
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  AString result(nullptr, uint32_t(char_count + 1u), uint32_t(0u));

  if (char_count)
    {
    #ifdef A_BOUNDS_CHECK
      span_check(pos, char_count, "get");
    #endif

    memcpy(result.m_str_ref_p->m_cstr_p, m_str_ref_p->m_cstr_p + pos, size_t(char_count));
    result.m_str_ref_p->m_cstr_p[char_count] = '\0';
    result.m_str_ref_p->m_length             = char_count;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Dynamic substring retrieval
// Author(s):    Conan Reis
AString * AString::get_new(
  uint32_t pos,       // = 0u
  uint32_t char_count // = ALength_remainder
  ) const
  {
  AString * str_p;
  
  if (char_count == ALength_remainder)
    {
    char_count = m_str_ref_p->m_length - pos;
    }

  #ifdef A_BOUNDS_CHECK
    if (char_count)
      {
      span_check(pos, char_count, "get_new");
      }
  #endif

  str_p = new AString(m_str_ref_p->m_cstr_p + pos, char_count);

  A_VERIFY_MEMORY(str_p != nullptr, AString);

  return str_p;
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
// Notes:       If the index given is out of range or if the specified index is
//              between two separators, an empty string is returned.
//              If the index is out of range, the value find_pos_p points to is
//              not changed.
// Examples:    AString str("one, two, three, four");
//              AString third(str.get_token(2, ", "));  // = "three"
// Author(s):    Conan Reis
AString AString::get_token(
  uint32_t          index,
  const AStringBM & token,
  uint32_t *        find_pos_p, // = nullptr
  uint32_t          start_pos,  // = 0
  uint32_t          end_pos     // = ALength_remainder
  ) const
  {
  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    // Get starting position
    if (index)
      {
      #ifdef A_BOUNDS_CHECK
        bool found = find(token, index, &start_pos, start_pos, end_pos);

        A_VERIFY(found, "::get_token() - index out of range", AErrId_invalid_index, AString);
      #endif

      start_pos += token.m_str_ref_p->m_length;
      }

    // Get ending position
    if (!find(token, 1u, &end_pos, start_pos, end_pos))
      {
      end_pos++;
      }
    }
  else
    {
    #ifdef A_BOUNDS_CHECK
      A_VERIFY(index == 0u, "::get_token() - index out of range", AErrId_invalid_index, AString);
    #endif

    end_pos = 0u;
    }

  if (find_pos_p)
    {
    *find_pos_p = start_pos;
    }

  return AString(m_str_ref_p->m_cstr_p + start_pos, end_pos - start_pos);
  }

//---------------------------------------------------------------------------------------
//  Splits current string from start_pos to end_pos into new dynamic
//              substrings with separator as the substring to split on.
// Arg          collect_p - array to accumulate (i.e. append) the dynamically allocated
//              substrings to.  Any previous AString objects will remain in collected_p,
//              they are just added to.
//              *** Remember to free the substrings after they are finished being used.
// Arg          separator - 
// Arg          start_pos - character index position to begin tokenization (Default 0)
// Arg          end_pos - character index position to end tokenization.  If end_pos is
//              ALength_remainder, then end_pos is set to the last character positon.
//              (Default ALength_remainder)
// Arg          case_check - Indicates whether the search for the separator substring
//              should be case sensitive (AStrCase_sensitive) or case insensitive (AStrCase_ignore).
//              (Default AStrCase_sensitive)
// See:         get_token(), AString(APArrayLogical<AString>, separator)
// Notes:       Seperators that are adjacent to one another count as an empty string.
// Author(s):    Conan Reis
void AString::tokenize(
  APArrayLogical<AString> * collect_p,
  const AString &           separator, // = AString(',')
  uint32_t                  start_pos, // = 0
  uint32_t                  end_pos,   // = ALength_remainder
  eAStrCase                 case_check // = AStrCase_sensitive
  ) const
  {
  uint32_t pos;

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "tokenize");
    #endif

    uint32_t length = separator.m_str_ref_p->m_length;

    while ((start_pos <= end_pos) && (find(separator, 1u, &pos, start_pos, end_pos, case_check)))
      {
      collect_p->append(*get_new(start_pos, pos - start_pos));
      start_pos = pos + length;
      }

    collect_p->append(*get_new(start_pos, end_pos - start_pos + 1));
    }
  }

//---------------------------------------------------------------------------------------
//  Splits current string from start_pos to end_pos into new dynamic
//              substrings with separator as the substring to split on.
// Arg          collect_p - array to accumulate (i.e. append) the dynamically allocated
//              substrings to.  Any previous AString objects will remain in collected_p,
//              they are just added to.
//              *** Remember to free the substrings after they are finished being used.
// Arg          separator - 
// Arg          start_pos - character index position to begin tokenization (Default 0)
// Arg          end_pos - character index position to end tokenization.  If end_pos is
//              ALength_remainder, then end_pos is set to the last character positon.
//              (Default ALength_remainder)
// Arg          case_check - Indicates whether the search for the separator substring
//              should be case sensitive (AStrCase_sensitive) or case insensitive (AStrCase_ignore).
//              (Default AStrCase_sensitive)
// See:         get_token(), AString(APArrayLogical<AString>, separator)
// Notes:       Separators that are adjacent to one another count as an empty string.
// Author(s):    Conan Reis
void AString::tokenize(
  APArrayLogical<AString> * collect_p, 
  const AStringBM &         separator,
  uint32_t                  start_pos, // = 0
  uint32_t                  end_pos    // = ALength_remainder
  ) const
  {
  uint32_t pos;

  if (m_str_ref_p->m_length)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = m_str_ref_p->m_length - 1;
      }

    #ifdef A_BOUNDS_CHECK
      bounds_check(start_pos, end_pos, "tokenize");
    #endif

    uint32_t length = separator.get_length();

    while ((start_pos <= end_pos) && (find(separator, 1u, &pos, start_pos, end_pos)))
      {
      collect_p->append(*get_new(start_pos, pos - start_pos));
      start_pos = pos + length;
      }

    collect_p->append(*get_new(start_pos, end_pos - start_pos + 1));
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Makes the string writable and have a unique reference.
// Examples:    called by ensure_writable()
// Notes:       Maintains data integrity.
// Author(s):    Conan Reis
void AString::make_writable()
  {
  AStringRef * str_ref_p = m_str_ref_p;
  uint32_t     length    = str_ref_p->m_length;
  uint32_t     size      = AStringRef::request_char_count(str_ref_p->m_length);
  char *       buffer_p  = AStringRef::alloc_buffer(size);
  
  memcpy(buffer_p, str_ref_p->m_cstr_p, size_t(length));

  // Add null terminator by hand rather than copying it to ensure that it exists.
  buffer_p[length] = '\0';

  m_str_ref_p = str_ref_p->reuse_or_new(buffer_p, length, size, true);
  }

//---------------------------------------------------------------------------------------
//  Makes C-String buffer large enough, writable, and unique (not shared)
//              - copying the data already present in the string.
// Arg          needed_chars - amount of characters that need to be stored not including
//              the null terminating character
// Examples:    Called by ensure_size()
// See:         set_size_buffer(), ensure_size(), ensure_size_empty(),
//              ensure_size_buffer(), set_length()
// Author(s):    Conan Reis
void AString::set_size(uint32_t needed_chars)  
  {
  AStringRef * str_ref_p = m_str_ref_p;
  uint32_t     size      = AStringRef::request_char_count(needed_chars);
  uint32_t     length    = a_min(str_ref_p->m_length, size - 1u);
  char *       buffer_p  = AStringRef::alloc_buffer(size);

  // Copy previous contents
  memcpy(buffer_p, str_ref_p->m_cstr_p, size_t(length));

  // Add null terminator by hand rather than copying it to ensure that it exists.
  buffer_p[length] = '\0';

  m_str_ref_p = str_ref_p->reuse_or_new(buffer_p, length, size);
  }


#ifdef A_BOUNDS_CHECK

// $Revisit - CReis These methods should be changed to macros so that they are faster in debug mode and so that they use the __FUNCSIG__ macro

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AString::bounds_check(
  uint32_t     pos,
  const char * func_name_p
  ) const
  {
  A_VERIFY(
    pos <= m_str_ref_p->m_length,
    a_cstr_format("AString::%s() - invalid index\nGiven %u but length only %u, AString:\n%s",
      func_name_p, pos, m_str_ref_p->m_length, m_str_ref_p->m_cstr_p),
    AErrId_invalid_index,
    AString);
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AString::bounds_check(
  uint32_t     start_pos,
  uint32_t     end_pos,
  const char * func_name_p
  ) const
  {
  A_VERIFY(
    ((start_pos < m_str_ref_p->m_length) && (end_pos < m_str_ref_p->m_length) && (start_pos <= end_pos)),
    a_cstr_format("AString::%s(start_pos %u, end_pos %u) - invalid index(es)\nLength is %u, AString:\n%s",
      func_name_p, start_pos, end_pos, m_str_ref_p->m_length, m_str_ref_p->m_cstr_p),
    AErrId_invalid_index_range,
    AString);
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AString::bounds_check(
  uint32_t     start_pos,
  uint32_t     end_pos,
  uint32_t     instance,
  const char * func_name_p
  ) const
  {
  A_VERIFY(
    ((start_pos < m_str_ref_p->m_length) && (end_pos <= m_str_ref_p->m_length) && (start_pos <= end_pos) && (instance != 0u)),
    a_cstr_format("AString::%s(start_pos %u, end_pos %u, instance %u) - invalid index(es)\nLength is %u, AString:\n%s",
      func_name_p, start_pos, end_pos, instance, m_str_ref_p->m_length, m_str_ref_p->m_cstr_p),
    AErrId_invalid_index_range,
    AString);
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void AString::span_check(
  uint32_t     pos,
  uint32_t     char_count,
  const char * func_name_p
  ) const
  {
  A_VERIFY(
    (pos + char_count) <= m_str_ref_p->m_length,
    a_cstr_format("AString::%s(pos %u, char_count %u) - invalid index(es)\nLength is %u, AString:\n%s",
      func_name_p, pos, char_count, m_str_ref_p->m_length, m_str_ref_p->m_cstr_p),
    AErrId_invalid_index_span,
    AString);
  }

#endif  // A_BOUNDS_CHECK


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Initializes the character matching / classification table.
// 
// Only uses ASCII so as not to be dependent on ANSI Code Pages, etc.
// Probably should be replaced with more comprehensive Unicode UTF-8 mechanism - though
// should work with ASCII subset of UTF-8
// 
// # Notes
//   Use the 2 dimensional table ms_char_match_table instead of the standard character
//   classification functions like isalpha(), isspace(), etc.
//
//   ms_char_match_table[ACharMatch_white_space][ch] is the same as isspace(ch)
//
//   Always convert characters to unsigned if they are greater than 127. For example, use
//   ms_char_match_table[ACharMatch_alphabetic][uint8_t(ch)] rather than
//   ms_char_match_table[ACharMatch_alphabetic][ch].
//
//   If the same character classification test is to be used repeatedly, a temporary
//   variable can be made to remove one level of indirection and use it as a one
//   dimensional array.  For example:
//   ```
//   bool * is_digit_p = ms_char_match_table[ACharMatch_digit];
//   for (...)
//     {
//     ...
//     if (is_digit_p[ch])
//     ...
//     }
//   ```  
//                
// Examples:    Called at system start-up.
// See:         not_char_type()
// Author(s):   Conan Reis
void AString::init_match_table()
  {
  const size_t bool_bytes = sizeof(bool);

  // #################### Affirmative Classifications ####################

  // Most of the classification tests return false
  memset(ms_char_match_table, false, bool_bytes * ACharMatch__not_start * AString_ansi_charset_length);

  // Set alphabetic ACharMatch_alphabetic
  memset(&ms_char_match_table[ACharMatch_alphabetic]['A'], true, bool_bytes * ('Z' - 'A' + 1));
  memset(&ms_char_match_table[ACharMatch_alphabetic]['a'], true, bool_bytes * ('z' - 'a' + 1));

  // Set alphanumeric ACharMatch_alphanumeric
  memcpy(ms_char_match_table[ACharMatch_alphanumeric], ms_char_match_table[ACharMatch_alphabetic], bool_bytes * AString_ansi_charset_length);  // Alphabetic
  memset(&ms_char_match_table[ACharMatch_alphanumeric]['0'], true, bool_bytes * ('9'- '0' + 1));

  // Set alphanumeric ACharMatch_alphascore
  memcpy(ms_char_match_table[ACharMatch_alphascore], ms_char_match_table[ACharMatch_alphabetic], bool_bytes * AString_ansi_charset_length);  // Alphabetic
  ms_char_match_table[ACharMatch_alphascore]['_'] = true;

  // Set digit ACharMatch_DIGIT
  memset(&ms_char_match_table[ACharMatch_digit]['0'], true, bool_bytes * ('9'- '0' + 1));

  // Set alphanumeric ACharMatch_identifier
  memcpy(ms_char_match_table[ACharMatch_identifier], ms_char_match_table[ACharMatch_alphanumeric], bool_bytes * AString_ansi_charset_length);  // Alphanumeric
  ms_char_match_table[ACharMatch_identifier]['_'] = true;

  // Set lowercase ACharMatch_lowercase
  memset(&ms_char_match_table[ACharMatch_lowercase]['a'], true, bool_bytes * ('z' - 'a' + 1));

  // Set Punctuation ACharMatch_punctuation
  ms_char_match_table[ACharMatch_punctuation]['!'] = true;
  ms_char_match_table[ACharMatch_punctuation]['"'] = true;
  ms_char_match_table[ACharMatch_punctuation]['\''] = true;
  ms_char_match_table[ACharMatch_punctuation]['('] = true;
  ms_char_match_table[ACharMatch_punctuation][')'] = true;
  ms_char_match_table[ACharMatch_punctuation][','] = true;
  ms_char_match_table[ACharMatch_punctuation]['-'] = true;  // Note this could be a hyphen as well as a minus sign
  ms_char_match_table[ACharMatch_punctuation]['.'] = true;
  ms_char_match_table[ACharMatch_punctuation][':'] = true;
  ms_char_match_table[ACharMatch_punctuation][';'] = true;
  ms_char_match_table[ACharMatch_punctuation]['?'] = true;
  ms_char_match_table[ACharMatch_punctuation]['['] = true;
  ms_char_match_table[ACharMatch_punctuation][']'] = true;
  ms_char_match_table[ACharMatch_punctuation]['{'] = true;
  ms_char_match_table[ACharMatch_punctuation]['}'] = true;

  // Set symbol ACharMatch_symbol
  memset(&ms_char_match_table[ACharMatch_symbol]['#'], true, bool_bytes * ('&' - '#' + 1));
  ms_char_match_table[ACharMatch_symbol]['*'] = true;
  ms_char_match_table[ACharMatch_symbol]['+'] = true;
  ms_char_match_table[ACharMatch_symbol]['-'] = true;  // Note this could be a hyphen as well as a minus sign
  ms_char_match_table[ACharMatch_symbol]['/'] = true;
  ms_char_match_table[ACharMatch_symbol]['<'] = true;
  ms_char_match_table[ACharMatch_symbol]['='] = true;
  ms_char_match_table[ACharMatch_symbol]['>'] = true;
  ms_char_match_table[ACharMatch_symbol]['@'] = true;
  ms_char_match_table[ACharMatch_symbol]['\\'] = true;
  ms_char_match_table[ACharMatch_symbol]['^'] = true;
  ms_char_match_table[ACharMatch_symbol]['`'] = true;
  ms_char_match_table[ACharMatch_symbol]['|'] = true;
  ms_char_match_table[ACharMatch_symbol]['~'] = true;

  // Set Punctuation and symbols ACharMatch_token
  memset(&ms_char_match_table[ACharMatch_token]['!'], true, bool_bytes * ('/' - '!' + 1));
  memset(&ms_char_match_table[ACharMatch_token][':'], true, bool_bytes * ('@' - ':' + 1));
  memset(&ms_char_match_table[ACharMatch_token]['['], true, bool_bytes * ('`' - '[' + 1));
  memset(&ms_char_match_table[ACharMatch_token]['{'], true, bool_bytes * ('~' - '{' + 1));

  // Set alphabetic ACharMatch_uppercase
  memset(&ms_char_match_table[ACharMatch_uppercase]['A'], true, bool_bytes * ('Z' - 'A' + 1));

  // Set white space ACharMatch_white_space
  // Horizontal Tab, Line Feed, Vertical Tab, Form Feed, Carriage Return
  memset(&ms_char_match_table[ACharMatch_white_space][9], true, bool_bytes * 5);
  // Space
  ms_char_match_table[ACharMatch_white_space][' '] = true;
  
  
  // #################### Negative Classifications #######################

  // Most of the negative classification tests return true
  memset(ms_char_match_table[ACharMatch__not_start], true, bool_bytes * (ACharMatch__length - ACharMatch__not_start) * AString_ansi_charset_length);

  // Set alphabetic ACharMatch_not_alphabetic
  memset(&ms_char_match_table[ACharMatch_not_alphabetic]['A'], false, bool_bytes * ('Z' - 'A' + 1));
  memset(&ms_char_match_table[ACharMatch_not_alphabetic]['a'], false, bool_bytes * ('z' - 'a' + 1));

  // Set alphanumeric ACharMatch_not_alphanumeric
  memcpy(ms_char_match_table[ACharMatch_not_alphanumeric], ms_char_match_table[ACharMatch_not_alphabetic], bool_bytes * AString_ansi_charset_length);  // Alphabetic
  memset(&ms_char_match_table[ACharMatch_not_alphanumeric]['0'], false, bool_bytes * ('9'- '0' + 1));

  // Set alphanumeric ACharMatch_not_alphascore
  memcpy(ms_char_match_table[ACharMatch_not_alphascore], ms_char_match_table[ACharMatch_not_alphabetic], bool_bytes * AString_ansi_charset_length);  // Alphabetic
  ms_char_match_table[ACharMatch_not_alphascore]['_'] = false;

  // Set digit ACharMatch_not_digit
  memset(&ms_char_match_table[ACharMatch_not_digit]['0'], false, bool_bytes * ('9'- '0' + 1));

  // Set alphanumeric ACharMatch_not_identifier
  memcpy(ms_char_match_table[ACharMatch_not_identifier], ms_char_match_table[ACharMatch_not_alphanumeric], bool_bytes * AString_ansi_charset_length);  // Alphanumeric
  ms_char_match_table[ACharMatch_not_identifier]['_'] = false;

  // Set lowercase ACharMatch_not_lowercase
  memset(&ms_char_match_table[ACharMatch_not_lowercase]['a'], false, bool_bytes * ('z' - 'a' + 1));

  // Set Punctuation ACharMatch_not_punctuation
  ms_char_match_table[ACharMatch_not_punctuation]['!'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['"'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['\''] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['('] = false;
  ms_char_match_table[ACharMatch_not_punctuation][')'] = false;
  ms_char_match_table[ACharMatch_not_punctuation][','] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['-'] = false;  // Note this could be a hyphen as well as a minus sign
  ms_char_match_table[ACharMatch_not_punctuation]['.'] = false;
  ms_char_match_table[ACharMatch_not_punctuation][':'] = false;
  ms_char_match_table[ACharMatch_not_punctuation][';'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['?'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['['] = false;
  ms_char_match_table[ACharMatch_not_punctuation][']'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['{'] = false;
  ms_char_match_table[ACharMatch_not_punctuation]['}'] = false;

  // Set symbol ACharMatch_not_symbol
  memset(&ms_char_match_table[ACharMatch_not_symbol]['#'], false, bool_bytes * ('&' - '#' + 1));
  ms_char_match_table[ACharMatch_not_symbol]['*'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['+'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['-'] = false;  // Note this could be a hyphen as well as a minus sign
  ms_char_match_table[ACharMatch_not_symbol]['/'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['<'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['='] = false;
  ms_char_match_table[ACharMatch_not_symbol]['>'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['@'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['\\'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['^'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['`'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['|'] = false;
  ms_char_match_table[ACharMatch_not_symbol]['~'] = false;

  // Set Punctuation and symbols ACharMatch_not_token
  memset(&ms_char_match_table[ACharMatch_not_token]['!'], false, bool_bytes * ('/' - '!' + 1));
  memset(&ms_char_match_table[ACharMatch_not_token][':'], false, bool_bytes * ('@' - ':' + 1));
  memset(&ms_char_match_table[ACharMatch_not_token]['['], false, bool_bytes * ('`' - '[' + 1));
  memset(&ms_char_match_table[ACharMatch_not_token]['{'], false, bool_bytes * ('~' - '{' + 1));

  // Set alphabetic ACharMatch_not_uppercase
  memset(&ms_char_match_table[ACharMatch_not_uppercase]['A'], false, bool_bytes * ('Z' - 'A' + 1));

  // Set white space ACharMatch_not_white_space
  // Horizontal Tab, Line Feed, Vertical Tab, Form Feed, Carriage Return
  memset(&ms_char_match_table[ACharMatch_not_white_space][9], false, bool_bytes * 5);
  memset(&ms_char_match_table[ACharMatch_not_white_space_except_lf][9], false, bool_bytes * 5);
  // Space
  ms_char_match_table[ACharMatch_not_white_space][' '] = false;
  ms_char_match_table[ACharMatch_not_white_space_except_lf][' '] = false;
  // Line feed
  ms_char_match_table[ACharMatch_not_white_space_except_lf]['\n'] = true;
  }


//#######################################################################################
// AStringBM Class
//#######################################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Normal constructor from string.
// Author(s):   Conan Reis
AStringBM::AStringBM(
  const AString & str,       // = ms_empty
  eAStrCase       case_check // = AStrCase_sensitive
  ) :
  AString(str),
  m_case(case_check)
  {
  uint32_t pos    = 0u;
  uint32_t length = m_str_ref_p->m_length;  // get length of pattern
  uint32_t offset = length - 1u;

  if (case_check == AStrCase_ignore)
    {
    lowercase();
    }

  char *    cstr_p  = m_str_ref_p->m_cstr_p;  // for quicker access
  uint8_t * delta_p = m_delta_p;              // for quicker access

  memset(m_delta_p, int(length), static_cast<size_t>(AString_ansi_charset_length));

  for (; pos < length; pos++)  // set table values
    {
    delta_p[static_cast<uint32_t>(cstr_p[pos])] = uint8_t(offset - pos);
    }
  }

//---------------------------------------------------------------------------------------
// Copy constructor.
// Author(s):   Conan Reis
AStringBM::AStringBM(const AStringBM & bm) :
  AString(bm),
  m_case(bm.m_case)
  {
  memcpy(m_delta_p, bm.m_delta_p, static_cast<size_t>(AString_ansi_charset_length));  // copy contents of source
  }

//---------------------------------------------------------------------------------------
// Assignment operator.
// Author(s):   Conan Reis
const AStringBM & AStringBM::operator=(const AStringBM & bm)
  {
  AString::operator=(bm);
  m_case = bm.m_case;
  memcpy(m_delta_p, bm.m_delta_p, static_cast<size_t>(AString_ansi_charset_length));  // copy contents of source

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Converter from AString to existing AStringBM.
// Author(s):   Conan Reis
void AStringBM::convert(
  const AString & str,
  eAStrCase       case_check // = AStrCase_sensitive
  )
  {
  uint32_t pos    = 0u;
  uint32_t length = str.m_str_ref_p->m_length;  // get length of pattern
  uint32_t offset = length - 1u;

  AString::operator=(str);
  m_case = case_check;

  if (case_check == AStrCase_ignore)
    {
    lowercase();
    }

  char *    cstr_p  = str.m_str_ref_p->m_cstr_p;  // for quicker access
  uint8_t * delta_p = m_delta_p;              // for quicker access

  memset(m_delta_p, int(length), static_cast<size_t>(AString_ansi_charset_length));

  for (; pos < length; pos++)  // set table values
    {
    delta_p[static_cast<uint32_t>(cstr_p[pos])] = uint8_t(offset - pos);
    }
  }

#ifdef __clang__
#pragma clang diagnostic pop
#endif

