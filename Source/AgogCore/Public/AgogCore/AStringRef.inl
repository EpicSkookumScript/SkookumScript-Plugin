// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AStringRef class declaration header
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <string.h>
#include <AgogCore/AMath.hpp>
#include <AgogCore/AObjReusePool.hpp>


//=======================================================================================
// Inline Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
A_INLINE AStringRef::AStringRef(
  const char * cstr_p,
  uint32_t     length,
  uint32_t     size,
  uint16_t     ref_count,
  bool         deallocate,
  bool         read_only
  ) :
  m_cstr_p(const_cast<char *>(cstr_p)),
  m_length(length),
  m_size(size),
  m_ref_count(ref_count),
  m_deallocate(deallocate),
  m_read_only(read_only)
  {}

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
A_INLINE eAEquate AStringRef::compare(const AStringRef & str_ref) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the standard library calls - like strncmp().

  char * str1_p = m_cstr_p;
  char * str2_p = str_ref.m_cstr_p;

  if (str1_p == str2_p)
    {
    // Same string buffer
    return AEquate_equal;
    }

  char * str1_end_p = str1_p + m_length + 1u;  // Ensure that null character is compared

  while (str1_p < str1_end_p)
    {
    if (*str1_p != *str2_p)  // if characters differ
      {
      // strings do not match
      return (*str1_p < *str2_p) ? AEquate_less : AEquate_greater;
      }

    str1_p++;
    str2_p++;
    }

  return AEquate_equal;
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              string to determine if they are equal.
// Returns:     true if equal, false if not
// Arg          str_ref - string to compare
// See:         is_equal() for a case-sensitive version.
// Author(s):    Conan Reis
A_INLINE bool AStringRef::is_equal(const AStringRef & str_ref) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the standard library calls.
  uint32_t length = m_length;

  if (length != str_ref.m_length)
    {
    return false;
    }

  char * str1_p = m_cstr_p;
  char * str2_p = str_ref.m_cstr_p;

  if (str1_p == str2_p)
    {
    // Same string buffer
    return true;
    }

  char * str1_end_p = str1_p + length;  // Don't bother comparing null character

  while (str1_p < str1_end_p)
    {
    if (*str1_p != *str2_p)  // if characters differ
      {
      // strings do not match
      return false;
      }

    str1_p++;
    str2_p++;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              C-string (with unknown length) to determine if they are equal.
// Returns:     true if equal, false if not
// Arg          cstr_p - C-string to compare
// See:         is_equal() for a case-sensitive version.
// Author(s):    Conan Reis
A_INLINE bool AStringRef::is_equal(const char * cstr_p) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the standard library calls - like strncmp().

  char *       str1_p     = m_cstr_p;
  char *       str1_end_p = str1_p + m_length + 1u;  // Ensure that null character is compared
  const char * str2_p     = cstr_p;

  while (str1_p < str1_end_p)
    {
    if (*str1_p != *str2_p)  // if characters differ
      {
      // strings do not match
      return false;
      }

    str1_p++;
    str2_p++;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
//  Does a case-sensitive comparison of the current string to the supplied
//              C-string (with known length) to determine if they are equal.
// Returns:     true if equal, false if not
// Arg          cstr_p - C-string to compare
// Arg          cstr_length - length of C-string
// See:         is_equal() for a case-sensitive version.
// Author(s):    Conan Reis
A_INLINE bool AStringRef::is_equal(const char * cstr_p, uint32_t cstr_length) const
  {
  // $Revisit - CReis This should be profiled, but I believe that the custom code is faster
  // than the standard library calls.
  uint32_t length = m_length;

  if (length != cstr_length)
    {
    return false;
    }

  char *       str1_p     = m_cstr_p;
  char *       str1_end_p = str1_p + length;  // Don't bother comparing null character
  const char * str2_p     = cstr_p;

  while (str1_p < str1_end_p)
    {
    if (*str1_p != *str2_p)  // if characters differ
      {
      // strings do not match
      return false;
      }

    str1_p++;
    str2_p++;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
A_INLINE void AStringRef::dereference()
  {
  if (--m_ref_count == 0u)
    {
    pool_delete(this);
    }
  }

//---------------------------------------------------------------------------------------
// Retrieves a string reference object from the dynamic pool and initializes it for use.
// This should be used instead of 'new' because it prevents unnecessary allocations by
// reusing previously allocated objects.
// 
// Notes:  
//   To 'deallocate' an object that was retrieved with this method, use `pool_delete()`
//   rather than `delete`.
//   
// Returns:     a dynamic AStringRef
// See:         pool_new_copy(), pool_delete(), reuse_or_new()
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE AStringRef * AStringRef::pool_new(
  const char * cstr_p,
  uint32_t     length,
  uint32_t     size,
  uint16_t     ref_count,
  bool         deallocate,
  bool         read_only
  )
  {
  AStringRef * str_ref_p = get_pool().allocate();

  str_ref_p->m_cstr_p = const_cast<char *>(cstr_p);
  str_ref_p->m_length = length;
  str_ref_p->m_size = size;
  str_ref_p->m_ref_count = ref_count;
  str_ref_p->m_deallocate = deallocate;
  str_ref_p->m_read_only = read_only;

  return str_ref_p;
  }

//---------------------------------------------------------------------------------------
//  Retrieves a string reference object from the dynamic pool and initializes
//              it for use with a copy of cstr_p.  This should be used instead of 'new'
//              because it prevents unnecessary allocations by reusing previously
//              allocated objects.
// Returns:     a dynamic AStringRef
// See:         pool_new(), pool_delete()
// Notes:       To 'deallocate' an object that was retrieved with this method, use
//              'pool_delete()' rather than 'delete'.
// Modifiers:    static
// Author(s):    Conan Reis
A_INLINE AStringRef * AStringRef::pool_new_copy(
  const char * cstr_p,
  uint32_t     length,
  uint16_t     ref_count, // = 1u
  bool         read_only  // = false
  )
  {
  AStringRef * str_ref_p = get_pool().allocate();
  uint32_t     size = request_char_count(length);
  char *       copy_cstr_p = alloc_buffer(size);

  memcpy(copy_cstr_p, cstr_p, length);
  copy_cstr_p[length] = '\0';  // Put in null-terminator

  str_ref_p->m_cstr_p = copy_cstr_p;
  str_ref_p->m_length = length;
  str_ref_p->m_size = size;
  str_ref_p->m_ref_count = ref_count;
  str_ref_p->m_deallocate = true;
  str_ref_p->m_read_only = read_only;

  return str_ref_p;
  }

//---------------------------------------------------------------------------------------
//  Frees up a AString reference and puts it into the dynamic pool ready for
//              its next use.  This should be used instead of 'delete' because it
//              prevents unnecessary deallocations by saving previously allocated
//              objects.
// Arg          str_ref_p - pointer to AString reference to free up and put into the
//              dynamic pool ready for its next use.
// See:         pool_new()
// Notes:       To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:    static
// Author(s):    Conan Reis
A_INLINE void AStringRef::pool_delete(AStringRef * str_ref_p)
  {
  if (str_ref_p->m_deallocate)
    {
    free_buffer(str_ref_p->m_cstr_p);
    }

  get_pool().recycle(str_ref_p);
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Determines the actual amount of memory allocated that will fit at least
//             the requested number of bytes.  Often memory systems use fixed size pools
//             so though a particular size is requested, a larger size may actually be
//             given and the extra space will be wasted if it is not used.
// Returns:    Actual amount of memory to allocate (in bytes) including null terminating
//             character.
// Arg         needed_chars - minimum size requested to allocate not counting the null
//             terminating character.
// Author(s):   Conan Reis
A_INLINE uint32_t AStringRef::request_char_count(uint32_t needed_chars)
  {
  // Add 1 for terminating null character.
  return AgogCore::get_app_info()->request_byte_size(needed_chars + 1u);
  }

//---------------------------------------------------------------------------------------
//  Allocates and returns a C-String character buffer
// Returns:     returns a C-String buffer
// Arg          needed - the size of the buffer to allocate including any null terminator
//              character.  So the string "123" should have needed=4.
// Examples:    char * cstr_p = alloc_array(20);
// See:         calc_size()
// Notes:       Throws a AEx<AStringRef> exception if unable to allocate sufficient memory.
// Modifiers:    static
// Author(s):    Conan Reis
A_INLINE char * AStringRef::alloc_buffer(uint32_t needed)
  {
  // $Revisit - CReis [Efficiency] Should check efficiency difference for memory allocation between new and malloc().
  #ifdef A_EXTRA_CHECK

    char * cstr_p = (char*)AgogCore::get_app_info()->malloc( sizeof(char) * needed, "AStringRef.buffer" );

    A_VERIFY_MEMORY(cstr_p, AStringRef);

    //A_DPRINT("AStringRef::alloc_buffer(%u): 0x%p - 0x%p\n", needed, cstr_p, cstr_p + needed);

    return cstr_p;

  #else

  return (char*)AgogCore::get_app_info()->malloc( sizeof(char) * needed, "AStringRef.buffer" );

  #endif
  }

//---------------------------------------------------------------------------------------
A_INLINE void AStringRef::free_buffer(char * buffer)
  {
  AgogCore::get_app_info()->free(buffer);
  }



