// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Parser and associated data-structures
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AStringRef.hpp>
#include <AgogCore/ASymbolTable.hpp>


#if (SKOOKUM & SK_CODE_IN)

//=======================================================================================
// SkParser::Args Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default Constructor
// See:        other constructors, reset() methods
// Author(s):   Conan Reis
A_INLINE SkParser::Args::Args() :
  m_flags(ArgFlag__default),
  m_idx_probe(0u),
  m_desired_type_p(nullptr),
  m_receiver_type_p(nullptr),
  m_start_pos(0u),
  m_end_pos(0u),
  m_result(SkParser::Result_ok),
  m_expr_type(nullptr)
  {
  // Note that some of the "out" data is left undefined
  }

//---------------------------------------------------------------------------------------
// Constructor
// See:        other constructors, reset() methods
// Author(s):   Conan Reis
A_INLINE SkParser::Args::Args(
  uint32_t start_pos,
  uint32_t flags // = ArgFlag__default
  ) :
  m_flags(flags),
  m_idx_probe(0u),
  m_desired_type_p(nullptr),
  m_receiver_type_p(nullptr),
  m_start_pos(start_pos),
  m_end_pos(0u),
  m_result(SkParser::Result_ok),
  m_expr_type(nullptr)
  {
  // Note that some of the "out" data is left undefined
  }

//---------------------------------------------------------------------------------------
// Copy Constructor
// See:        other constructors, reset() methods
// Author(s):   Conan Reis
A_INLINE SkParser::Args::Args(const Args & args) :
  m_flags(args.m_flags),
  m_idx_probe(args.m_idx_probe),
  m_idx_probe_user(args.m_idx_probe_user),
  m_idx_probe_f(args.m_idx_probe_f),
  m_desired_type_p(args.m_desired_type_p),
  m_receiver_type_p(args.m_receiver_type_p),
  m_invocation_stack(args.m_invocation_stack),
  m_start_pos(args.m_start_pos),
  m_end_pos(args.m_end_pos),
  m_result(args.m_result),
  m_expr_type(args.m_expr_type)
  {
  }

//---------------------------------------------------------------------------------------
// Assignment operator
// Returns:    itself to allow for stringization or to be passed as an argument
// See:        other constructors, reset() methods
// Author(s):   Conan Reis
A_INLINE SkParser::Args & SkParser::Args::operator=(const Args & args)
  {
  m_flags            = args.m_flags;
  m_idx_probe        = args.m_idx_probe;
  m_idx_probe_f      = args.m_idx_probe_f;
  m_idx_probe_user   = args.m_idx_probe_user;
  m_start_pos        = args.m_start_pos;
  m_end_pos          = args.m_end_pos;
  m_result           = args.m_result;
  m_expr_type        = args.m_expr_type;
  m_desired_type_p   = args.m_desired_type_p;
  m_receiver_type_p  = args.m_receiver_type_p;
  m_invocation_stack = args.m_invocation_stack;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Resets the "in" data
// Returns:    itself so that it can be simultaneously reset and passed as an argument
// See:        other constructors, reset() methods
// Author(s):  Conan Reis
A_INLINE SkParser::Args & SkParser::Args::reset()
  {
  m_flags           = ArgFlag__default;
  m_idx_probe       = 0u;
  m_idx_probe_f     = nullptr;
  m_start_pos       = 0u;
  m_expr_type       = nullptr;
  m_desired_type_p  = nullptr;
  m_receiver_type_p = nullptr;

  // Note that some of the "out" data is left unmodified

  return *this;
  }

//---------------------------------------------------------------------------------------
// Resets the "in" data
// 
// Returns:   itself so that it can be simultaneously reset and passed as an argument
// See:       other constructors, reset() methods
// Author(s): Conan Reis
A_INLINE SkParser::Args & SkParser::Args::reset(uint32_t start_pos)
  {
  m_flags           = ArgFlag__default;
  m_idx_probe       = 0u;
  m_idx_probe_f     = nullptr;
  m_start_pos       = start_pos;
  m_expr_type       = nullptr;
  m_desired_type_p  = nullptr;
  m_receiver_type_p = nullptr;
  m_invocation_stack.empty();

  // Note that some of the "out" data is left unmodified

  return *this;
  }

//---------------------------------------------------------------------------------------
A_INLINE SkParser::Args & SkParser::Args::set_idx_probe(
  uint32_t     idx_probe,
  tSkProbeFunc idx_probe_func, // = nullptr
  uintptr_t    user_data // = 0u
  )
  {
  m_flags         |= ArgFlag_parse_to_idx_probe;
  m_idx_probe      = idx_probe;
  m_idx_probe_f    = idx_probe_func;
  m_idx_probe_user = user_data;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Called when parse is at an index probe point, calls any registered index probe
// callback and determines if the parse should halt or continue.
// 
// Returns:
//   `true` if parse should halt and return `Result__idx_probe` or `false` if parse should
//   continue as optionally specified by `m_idx_probe_func` which may alter the probe
//   index or other aspects of the parse.
//   
// Params:
//   parser_p:
//     parser object with interesting context to be passed to optional `m_idx_probe_func`
// 
// See: `ArgFlag_parse_to_idx_probe`, `m_idx_probe_user` and `m_idx_probe_func`
A_INLINE bool SkParser::Args::is_idx_probe_halt(const SkParser * parser_p)
  {
  // Shall we bail or continue?
  if (m_idx_probe_f ? (m_idx_probe_f)(parser_p, *this) : m_end_pos >= m_idx_probe)
    {
    // Halt parse and return probe info
    m_result = Result__idx_probe;
    return true;
    }

  return false;
  }


//=======================================================================================
// SkParser Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// 
// Returns: itself
// Params:
//   str: string to parse
//
// Author(s):   Conan Reis
A_INLINE SkParser::SkParser(
  const AString &      str
  ) :
  AString(str),
  m_flags(ms_default_flags),
  m_member_type(SkMember__invalid),
  m_current_block_p(nullptr)
  {
  reset_scope();
  }

//---------------------------------------------------------------------------------------
// Constructor from character array buffer. Refers to a persistent null-terminated
// C-String or makes its own copy of an existing character array.
// 
// Returns: itself
// Params:
//   cstr_p:
//     pointer to array of characters (does not need to be null terminated unless length
//     is equal to ALength_calculate).  'cstr_p' should never be nullptr.  'cstr_p' will
//     usually be a string literal or if 'persistent' is false, 'cstr_p' may be any 
//     C-String that this string should make a copy of.
//   length:
//     number of characters to use in 'cstr_p' and the index position to place a
//     terminating null character.  The given length must not be more than the size of
//     'cstr_p' and the C-String buffer pointed to by 'cstr_p' should not have any null
//     characters less then the given length. A null terminator is placed only if
//     'persistent' is not true. 'length' may also be set to ALength_calculate in which
//     case the character length is calculated by finding the first terminating null
//     character already present in 'cstr_p'.
//   persistent:
//     Indicates whether the data pointed to by cstr_p will be available for the lifetime
//     of this string.  If 'persistent' is true, the memory pointed to by 'cstr_p' will
//     be used rather than having this object allocate its own memory - also the memory
//     pointed to by 'cstr_p' will not be written to.  If 'persistent' is false, then
//     this AString object will allocate its own memory and copy the contents of 'cstr_p'
//     to it.
//   customizations_p:
//      Customized settings and behavior for the parser - if nullptr it uses the default
//      one stored in `ms_defaults_p` which is set with SkParser::set_custom_defaults().
//     
// Notes:
//   If any modifying methods are called on this string, it will first make its own
//   unique copy of the C-String before it writes to it.
//   
// See:       AString(cstr_p), AString(buffer_p, size, length, deallocate)
// Author(s): Conan Reis
A_INLINE SkParser::SkParser(
  const char *         cstr_p,
  uint32_t             length,        // = ALength_calculate
  bool                 persistent_b   // = true
  ) :
  AString(cstr_p, length, persistent_b),
  m_flags(ms_default_flags),
  m_member_type(SkMember__invalid),
  m_current_block_p(nullptr)
  {
  reset_scope();
  }

//---------------------------------------------------------------------------------------
// Determines if parse string is a constructor call
// Returns:    true if it looks like start_pos is at the start of a constructor call,
//             false if not.
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Author(s):   Conan Reis
A_INLINE bool SkParser::is_constructor(
  uint32_t start_pos // = 0u
  ) const
  {
  uint32_t   ctor_end;
  char * cstr_a = m_str_ref_p->m_cstr_p;

  return ((cstr_a[start_pos] == '!')
    && (cstr_a[start_pos + 1u] != '!')
    && (parse_name_method(start_pos, &ctor_end) == Result_ok)
    && (cstr_a[ctor_end] == '('));
  }


#endif // (SKOOKUM & SK_CODE_IN)


