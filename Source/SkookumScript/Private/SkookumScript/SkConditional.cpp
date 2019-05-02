// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Classes for expressions that can be conditionally evaluated/invoked
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkConditional.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkConditional.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================


//=======================================================================================
// SkClause Method Definitions
//=======================================================================================

// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Author(s):   Conan Reis
AString SkClause::as_code() const
  {
  AString clause_str(m_clause_p->as_code_block());
  AString str(m_test_p ? m_test_p->as_code() : AString("else", 4u), 32u + clause_str.get_length());

  str.append('\n');

  uint clause_idx = str.get_length();

  str.append(clause_str);
  str.line_indent(SkDebug::ms_indent_size, clause_idx);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines the first sub-expression that it contains that was located at
//             or following the character index position provided and returns it.
// Returns:    The first expression that starts or follows the given position or nullptr if
//             no such expression was found.
// Arg         pos - code file/string character index position
// Notes:      Some sub-expressions may have the same starting character position as
//             their containing expression - in these cases only the containing
//             expression is returned.
// Author(s):   Conan Reis
SkExpressionBase * SkClause::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * found_p = nullptr;

  if (m_test_p)
    {
    found_p = m_test_p->find_expr_by_pos(pos, type);
    }

  if (found_p == nullptr)
    {
    found_p = m_clause_p->find_expr_by_pos(pos, type);
    }

  return found_p;
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkClause::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if ((m_test_p && m_test_p->iterate_expressions(apply_expr_p, invokable_p))
    || m_clause_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkClause::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkClause, SkDebugInfo_size_used));

  if (m_test_p)
    {
    m_test_p->track_memory(mem_stats_p);
    }

  m_clause_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkConditional Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkConditional::~SkConditional()
  {
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkConditional::assign_binary(const void ** binary_pp)
  {
  // 4 bytes - number of statements
  uint32_t length = A_BYTE_STREAM_UI32_INC(binary_pp);

  // n bytes - argument typed binary }- Repeating 
  m_clauses.set_size_free(length);

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + length;

  while (clauses_pp < clauses_end_pp)
    {
    (*clauses_pp) = SK_NEW(SkClause)(binary_pp);
    clauses_pp++;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this conditional expression and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkConditional::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkConditional::as_binary_length());

  m_clauses.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkConditional::as_binary_length() const
  {
  return m_clauses.as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Returns this expression (or any *last* sub-expression) that would have no *direct*
// side-effect (i.e. when a part of its functionality is useless even when it has a
// sub-expression that has a side effect) when its result is not used (i.e. not used as
// an argument/receiver or returned as the last expression in a code block).
// 
// If an expression is returned to determine whether it had no effects or if it has sub
// expressions with effects call get_side_effect() on it.
//
// Non-last sub-expressions - especially for code blocks - are automatically tested for
// side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkConditional::find_expr_last_no_side_effect() const
  {
  const SkExpressionBase * expr_p  = nullptr;

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    expr_p = (*clauses_pp)->m_clause_p->find_expr_last_no_side_effect();

    // $Revisit - CReis Allow for nil clauses since they affect the 'else' clause.  Though
    // consider a less wasteful mechanism.
    if (expr_p && !expr_p->is_nil())
      {
      return expr_p;
      }

    clauses_pp++;
    }

  return nullptr;
  }

#endif // (SKOOKUM & SK_CODE_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkConditional::as_code() const
  {
  AString str(nullptr, 256u, 0u);

  str.append("if ", 3u);

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    str.append((*clauses_pp)->as_code());
    clauses_pp++;

    if (clauses_pp < clauses_end_pp)
      {
      str.append('\n');
      str.append(' ', SkDebug::ms_indent_size);
      }
    }

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains
//             was located at or follows at the character index position provided and
//             returns it.
// Returns:    The first expression that starts or follows the given position or nullptr if
//             no such expression was found.
// Arg         pos - code file/string character index position
// Notes:      Some sub-expressions may have the same starting character position as
//             their containing expression - in these cases only the containing
//             expression is returned.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkConditional::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkConditional *>(this);
    }

  // Test sub-expressions
  SkExpressionBase * found_p        = nullptr;
  SkClause **        clauses_pp     = m_clauses.get_array();
  SkClause **        clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    found_p = (*clauses_pp)->find_expr_by_pos(pos, type);

    if (found_p)
      {
      break;
      }

    clauses_pp++;
    }

  return found_p;
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// #Modifiers virtual - override if expression has sub-expressions
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkConditional::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    if ((*clauses_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }

    clauses_pp++;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_conditional
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkConditional::get_type() const
  {
  return SkExprType_conditional;
  }

//---------------------------------------------------------------------------------------
// Evaluates conditional expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkConditional::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkInstance *       result_p;
  tSkBoolean      result_bool;
  SkExpressionBase * clause_p       = nullptr;
  SkClause **        clauses_pp     = m_clauses.get_array();
  SkClause **        clauses_end_pp = clauses_pp + m_clauses.get_length();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through clauses
  while (clauses_pp < clauses_end_pp)
    {
    // nullptr indicates default/else clause
    if ((*clauses_pp)->m_test_p)
      {
      result_p    = (*clauses_pp)->m_test_p->invoke_now(scope_p, caller_p);
      result_bool = result_p->as<SkBoolean>();
      // $Revisit - CReis Should put a run-time check here to ensure that it is of class type Boolean

      // Free up result - if not referenced elsewhere
      result_p->dereference();

      if (result_bool)
        {
        clause_p = (*clauses_pp)->m_clause_p;

        // Jump out of while
        break;
        }
      }
    else  // This is a default / else clause
      {
      clause_p = (*clauses_pp)->m_clause_p;

      break;  // Jump out of while
      }

    clauses_pp++;
    }  // while

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call successful clause if there is one
  if (clause_p)
    {
    return clause_p->invoke(scope_p, caller_p, result_pp);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If a result is desired and none of the clauses succeeded, return nil
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if expression completes immediately (true) - i.e. completes in
//             1 frame - or that its completion may be durational (false) - i.e. may take
//             1 or more frames.
// Returns:    true if immediate and false if durational
// Arg         durational_idx_p - if this expression is durational, the address to store the
//             character index position of the first durational expression - either this
//             expression or a sub-expression that it contains.
// Notes:      Coroutines are durational, some expressions such as a code block may be
//             immediate or durational depending on its sub-expressions and some expressions
//             such as identifiers are always immediate.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkConditional::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Determine if the "then" part of any clause takes more than one frame to execute and return.
  // [The test part of the clauses should be immediate.]
  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  for (; clauses_pp < clauses_end_pp; clauses_pp++)
    {
    if (!(*clauses_pp)->m_clause_p->is_immediate(durational_idx_p))
      {
      // Found clause that takes more than one frame to execute and return.
      return false;
      }
    }

  // All clauses execute and return in one frame.
  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConditional::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkConditional, SkDebugInfo_size_used),
    m_clauses.get_length() * sizeof(void *),
    m_clauses.get_size_buffer_bytes());

  m_clauses.track_memory(mem_stats_p);
  }


//=======================================================================================
// SkCase Class Data Members
//=======================================================================================


//=======================================================================================
// SkCase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkCase::~SkCase()
  {
  delete m_compare_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - compare expression typed binary
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkCase::assign_binary(const void ** binary_pp)
  {
  // n bytes - compare expression typed binary
  m_compare_expr_p = from_binary_typed_new(binary_pp);

  // 4 bytes - number of statements
  uint32_t length = A_BYTE_STREAM_UI32_INC(binary_pp);

  // n bytes - argument typed binary }- Repeating 
  m_clauses.set_size_free(length);

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + length;

  while (clauses_pp < clauses_end_pp)
    {
    (*clauses_pp) = SK_NEW(SkClause)(binary_pp);
    clauses_pp++;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this case expression and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - compare expression typed binary
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkCase::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCase::as_binary_length());

  // n bytes - compare expression typed binary
  m_compare_expr_p->as_binary_typed(binary_pp);

  // 4 bytes - number of clauses
  // n bytes - clause binary }- Repeating 
  m_clauses.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - compare expression typed binary
//               4 bytes - number of clauses
//               n bytes - clause binary }- Repeating 
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkCase::as_binary_length() const
  {
  return m_compare_expr_p->as_binary_typed_length() + m_clauses.as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Returns this expression (or any *last* sub-expression) that would have no *direct*
// side-effect (i.e. when a part of its functionality is useless even when it has a
// sub-expression that has a side effect) when its result is not used (i.e. not used as
// an argument/receiver or returned as the last expression in a code block).
//   
// If an expression is returned to determine whether it had no effects or if it has sub
// expressions with effects call get_side_effect() on it.
//
// Non-last sub-expressions - especially for code blocks - are automatically tested for
// side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkCase::find_expr_last_no_side_effect() const
  {
  const SkExpressionBase * expr_p  = nullptr;

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    expr_p = (*clauses_pp)->m_clause_p->find_expr_last_no_side_effect();

    // $Revisit - CReis Allow for nil clauses since they affect the 'else' clause.  Though consider a less wasteful mechanism.
    if (expr_p && !expr_p->is_nil())
      {
      return expr_p;
      }

    clauses_pp++;
    }

  return nullptr;
  }

#endif // (SKOOKUM & SK_CODE_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkCase::as_code() const
  {
  AString str(nullptr, 256u, 0u);

  str.append("case ", 5u);
  str.append(m_compare_expr_p->as_code());
  str.append('\n');
  str.append(' ', SkDebug::ms_indent_size);

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    str.append((*clauses_pp)->as_code());
    clauses_pp++;

    if (clauses_pp < clauses_end_pp)
      {
      str.append('\n');
      str.append(' ', SkDebug::ms_indent_size);
      }
    }

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains
//             was located at or follows at the character index position provided and
//             returns it.
// Returns:    The first expression that starts or follows the given position or nullptr if
//             no such expression was found.
// Arg         pos - code file/string character index position
// Notes:      Some sub-expressions may have the same starting character position as
//             their containing expression - in these cases only the containing
//             expression is returned.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkCase::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkCase *>(this);
    }

  // Test sub-expressions
  SkExpressionBase * found_p = m_compare_expr_p->find_expr_by_pos(pos, type);

  if (found_p == nullptr)
    {
    SkClause ** clauses_pp     = m_clauses.get_array();
    SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

    while (clauses_pp < clauses_end_pp)
      {
      found_p = (*clauses_pp)->find_expr_by_pos(pos, type);

      if (found_p)
        {
        break;
        }

      clauses_pp++;
      }
    }

  return found_p;
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// #Modifiers virtual - override if expression has sub-expressions
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkCase::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_compare_expr_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  while (clauses_pp < clauses_end_pp)
    {
    if ((*clauses_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }

    clauses_pp++;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_case
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkCase::get_type() const
  {
  return SkExprType_case;
  }

//---------------------------------------------------------------------------------------
// Evaluates case expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkCase::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Evaluate expression to compare clauses against
  SkInstance * compare_p = m_compare_expr_p->invoke_now(scope_p, caller_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find equals `=` operator
  // $Revisit - This should be an index offset rather than a runtime lookup!
  SkMethodBase * method_p = compare_p->is_metaclass()
    ? static_cast<SkMetaClass *>(compare_p)->find_method_inherited(ASymbolX_equalQ)
    : compare_p->get_class()->find_instance_method_inherited(ASymbolX_equalQ);
  SkExpressionBase * clause_p = nullptr;

  #if defined(SK_RUNTIME_RECOVER)
    if (method_p == nullptr)
      {
      // Notify about object that has no equals operator method.
      SK_ERROR_INFO(
        a_str_format(
          "Tried to call case statement on '%s', but it does not have an equals (=) operator!\n"
          "[This error is often due to a bad typecast.\n"
          "Ignoring will run the 'else' clause if one exists.]",
          compare_p->as_code().as_cstr()),
        *this);

      // Recover by trying to use "else" clause if there is one
      SkClause * last_clause_p = m_clauses.get_last();

      if (last_clause_p->m_test_p == nullptr)
        {
        clause_p = last_clause_p->m_clause_p;
        }
      }
    else
  #endif
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Create an invoked method wrapper for equals (=)
      SkInvokedMethod imethod(caller_p, compare_p, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

      SKDEBUG_ICALL_SET_INTERNAL(&imethod);

      // $Revisit - CReis Name probably not needed 99.99% of the time - could just be 'operand' or ''.
      // Assumes only one argument.
      SK_ASSERTX(method_p->get_invoked_data_array_size() > 0, "Method has no space for arguments.");
      imethod.data_append_var();

      SkInstance *  result_p;
      tSkBoolean result_bool;
      SkClause **   clauses_pp     = m_clauses.get_array();
      SkClause **   clauses_end_pp = clauses_pp + m_clauses.get_length();

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Iterate through clauses
      while (clauses_pp < clauses_end_pp)
        {
        // nullptr indicates default/else clause
        if ((*clauses_pp)->m_test_p)
          {
          // Set argument
          imethod.set_arg(SkArg_1, (*clauses_pp)->m_test_p->invoke_now(scope_p, caller_p));

          // Call = operator
          method_p->invoke(&imethod, caller_p, &result_p);
          result_bool = result_p->as<SkBoolean>();

          // Free up result - if not referenced elsewhere
          result_p->dereference();

          if (result_bool)
            {
            clause_p = (*clauses_pp)->m_clause_p;

            // Jump out of while
            break;
            }

          // Ensure each call to a clause has a unique id
          imethod.renew_id();
          }
        else  // This is a default / else clause
          {
          clause_p = (*clauses_pp)->m_clause_p;
          break;  // Jump out of while
          }

        clauses_pp++;
        }  // while
      }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call successful clause if there is one
  if (clause_p)
    {
    SkInvokedBase * invoked_p = clause_p->invoke(scope_p, caller_p, result_pp);

    // Free up compare result - if not referenced elsewhere
    compare_p->dereference();

    return invoked_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // No clause succeeded

  // Free up compare result - if not referenced elsewhere
  compare_p->dereference();

  // If a result is desired and none of the clauses succeeded, return nil
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if expression completes immediately (true) - i.e. completes in
//             1 frame - or that its completion may be durational (false) - i.e. may take
//             1 or more frames.
// Returns:    true if immediate and false if durational
// Arg         durational_idx_p - if this expression is durational, the address to store the
//             character index position of the first durational expression - either this
//             expression or a sub-expression that it contains.
// Notes:      Coroutines are durational, some expressions such as a code block may be
//             immediate or durational depending on its sub-expressions and some expressions
//             such as identifiers are always immediate.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkCase::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Determine if the "then" part of any clause takes more than one frame to execute and return.
  // [The test part of the clauses and the compare expression should be immediate.]
  SkClause ** clauses_pp     = m_clauses.get_array();
  SkClause ** clauses_end_pp = clauses_pp + m_clauses.get_length();

  for (; clauses_pp < clauses_end_pp; clauses_pp++)
    {
    if (!(*clauses_pp)->m_clause_p->is_immediate(durational_idx_p))
      {
      // Found clause that takes more than one frame to execute and return.
      return false;
      }
    }

  // All clauses execute and return in one frame.
  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCase::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkCase, SkDebugInfo_size_used),
    m_clauses.get_length() * sizeof(void *),
    m_clauses.get_size_buffer_bytes());

  m_compare_expr_p->track_memory(mem_stats_p);
  m_clauses.track_memory(mem_stats_p);
  }


//=======================================================================================
// SkWhen Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - clause expression
//               n bytes - test expression
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkWhen::assign_binary(const void ** binary_pp)
  {
  // n bytes - clause expression
  m_clause_p = SkExpressionBase::from_binary_typed_new(binary_pp);

  // n bytes - test expression
  m_test_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this conditional expression and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - clause expression
//               n bytes - test expression
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkWhen::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkWhen::as_binary_length());

  m_clause_p->as_binary_typed(binary_pp);
  m_test_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - clause expression
//               n bytes - test expression
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkWhen::as_binary_length() const
  {
  return m_clause_p->as_binary_typed_length()
    + m_test_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Returns this expression (or any *last* sub-expression) that would have no *direct*
// side-effect (i.e. when a part of its functionality is useless even when it has a
// sub-expression that has a side effect) when its result is not used (i.e. not used as
// an argument/receiver or returned as the last expression in a code block).
//   
// If an expression is returned to determine whether it had no effects or if it has sub
// expressions with effects call get_side_effect() on it.
//
// Non-last sub-expressions - especially for code blocks - are automatically tested for
// side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkWhen::find_expr_last_no_side_effect() const
  {
  return m_clause_p->find_expr_last_no_side_effect();
  }

//---------------------------------------------------------------------------------------
// Indicates whether this expression (or any sub-expression) has any potential side
// effects when used as a stand alone statement - i.e. not used as an argument/receiver
// or returned as the last expression in a code block.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  find_expr_last_no_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns See eSkSideEffect
  eSkSideEffect
SkWhen::get_side_effect() const
  {
  if (m_clause_p->get_side_effect())
    {
    return SkSideEffect_present;
    }

  if (m_test_p->get_side_effect())
    {
    return SkSideEffect_secondary;
    }

  return SkSideEffect_none;
  }

#endif // (SKOOKUM & SK_CODE_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkWhen::as_code() const
  {
  AString test_str(m_test_p->as_code());
  AString str(m_clause_p->as_code(), 6u + test_str.get_length());

  str.append(" when ", 6u);
  str.append(test_str);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains
//             was located at or follows at the character index position provided and
//             returns it.
// Returns:    The first expression that starts or follows the given position or nullptr if
//             no such expression was found.
// Arg         pos - code file/string character index position
// Notes:      Some sub-expressions may have the same starting character position as
//             their containing expression - in these cases only the containing
//             expression is returned.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkWhen::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkWhen *>(this);
    }

  // Test sub-expressions
  SkExpressionBase * found_p = m_clause_p->find_expr_by_pos(pos, type);

  if (found_p)
    {
    return found_p;
    }

  return m_test_p->find_expr_by_pos(pos, type);
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// #Modifiers virtual - override if expression has sub-expressions
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkWhen::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_clause_p->iterate_expressions(apply_expr_p, invokable_p)
    || m_test_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Evaluates expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkWhen::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkInstance * result_p = m_test_p->invoke_now(scope_p, caller_p);

  // $Revisit - CReis Should put a run-time check here to ensure that it is of class type Boolean
  tSkBoolean call_clause = result_p->as<SkBoolean>();

  // Free up result - if not referenced elsewhere
  result_p->dereference();

  if (call_clause)
    {
    return m_clause_p->invoke(scope_p, caller_p, result_pp);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If a result is desired and none of the clauses succeeded, return nil
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if expression completes immediately (true) - i.e. completes in
//             1 frame - or that its completion may be durational (false) - i.e. may take
//             1 or more frames.
// Returns:    true if immediate and false if durational
// Arg         durational_idx_p - if this expression is durational, the address to store the
//             character index position of the first durational expression - either this
//             expression or a sub-expression that it contains.
// Notes:      Coroutines are durational, some expressions such as a code block may be
//             immediate or durational depending on its sub-expressions and some expressions
//             such as identifiers are always immediate.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkWhen::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Determine if the "then" part of any clause takes more than one frame to execute and return.
  // [The test part of the clauses should be immediate.]
  return m_clause_p->is_immediate(durational_idx_p);
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// The clause is not technically a receiver but precedes the test so is subject to the same recovery mechanism
// Arg         receiver_p - sub receiver to find and set to nullptr.
void SkWhen::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_clause_p)
    {
    if (m_clause_p == receiver_p)
      {
      m_clause_p = nullptr;
      }
    else
      {
      m_clause_p->null_receiver(receiver_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkWhen::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkWhen, SkDebugInfo_size_used));
  m_clause_p->track_memory(mem_stats_p);
  m_test_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkUnless Method Definitions
//=======================================================================================

// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkUnless::as_code() const
  {
  AString test_str(m_test_p->as_code());
  AString str(m_clause_p->as_code(), 8u + test_str.get_length());

  str.append(" unless ", 8u);
  str.append(test_str);

  return str;
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Evaluates expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkUnless::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkInstance * result_p = m_test_p->invoke_now(scope_p, caller_p);

  // $Revisit - CReis Should put a run-time check here to ensure that it is of class type Boolean
  tSkBoolean call_clause = !result_p->as<SkBoolean>();

  // Free up result - if not referenced elsewhere
  result_p->dereference();

  if (call_clause)
    {
    return m_clause_p->invoke(scope_p, caller_p, result_pp);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If a result is desired and none of the clauses succeeded, return nil
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkUnless::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkUnless, SkDebugInfo_size_used));
  m_clause_p->track_memory(mem_stats_p);
  m_test_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkNilCoalescing Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
//
// #Params:
//   binary_pp:
//     Pointer to address to read binary serialization info from and to increment
//     - previously filled using as_binary() or a similar mechanism.
//
// #See: as_binary()
// #Notes:
//   Binary composition:
//     n bytes - trial expression
//     n bytes - alternate expression
//
//   Little error checking is done on the binary info as it assumed that it was
//   previously validated upon input.
void SkNilCoalescing::assign_binary(const void ** binary_pp)
  {
  // n bytes - trial expression
  m_trial_p = SkExpressionBase::from_binary_typed_new(binary_pp);

  // n bytes - alternate expression
  m_alternate_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to recreate this
// nil coalescing operator expression and increments the memory address to just past the
// last byte written.
//
// #Params:
//   binary_pp:
//     Pointer to address to fill and increment.  Its size *must* be large enough to
//     fit all the binary data.  Use the get_binary_length() method to determine the
//     size needed prior to passing binary_pp to this method.
//
// #Notes:
//   Used in combination with as_binary_length().
//
//   Binary composition:
//     n bytes - trial expression
//     n bytes - alternate expression
//
// #Modifiers: virtual from SkExpressionBase
// #See:       as_binary_length()
void SkNilCoalescing::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkNilCoalescing::as_binary_length());

  m_trial_p->as_binary_typed(binary_pp);
  m_alternate_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
//
// #Notes:
//   Used in combination with as_binary().
//
//   Binary composition:
//     n bytes - trial expression
//     n bytes - alternate expression
//
// #Modifiers: virtual from SkExpressionBase
// #See:       as_binary_length()
uint32_t SkNilCoalescing::as_binary_length() const
  {
  return m_trial_p->as_binary_typed_length()
    + m_alternate_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Returns this expression (or any *last* sub-expression) that would have no *direct*
// side-effect (i.e. when a part of its functionality is useless even when it has a
// sub-expression that has a side effect) when its result is not used (i.e. not used as
// an argument/receiver or returned as the last expression in a code block).
//   
// If an expression is returned to determine whether it had no effects or if it has sub
// expressions with effects call get_side_effect() on it.
//
// Non-last sub-expressions - especially for code blocks - are automatically tested for
// side effects immediately after they are parsed.
//
// #Returns:   nullptr or expression that has no side effects and not used as a result.
// #Modifiers: virtual - overridden from SkExpressionBase
// #See:       get_side_effect()
const SkExpressionBase * SkNilCoalescing::find_expr_last_no_side_effect() const
  {
  const SkExpressionBase * effect1_p = m_trial_p->find_expr_last_no_side_effect();

  if (effect1_p == nullptr)
    {
    return nullptr;
    }

  const SkExpressionBase * effect2_p = m_alternate_p->find_expr_last_no_side_effect();

  if (effect2_p == nullptr)
    {
    return nullptr;
    }

  return effect1_p;
  }

//---------------------------------------------------------------------------------------
// Indicates whether this expression (or any sub-expression) has any potential side
// effects when used as a stand alone statement - i.e. not used as an argument/receiver
// or returned as the last expression in a code block.
//
// #Returns: See eSkSideEffect
//
// #Modifiers: virtual - overridden from SkExpressionBase
// #See:       find_expr_last_no_side_effect()
eSkSideEffect SkNilCoalescing::get_side_effect() const
  {
  if (m_trial_p->get_side_effect() || m_alternate_p->get_side_effect())
    {
    return SkSideEffect_present;
    }

  return SkSideEffect_none;
  }

#endif // (SKOOKUM & SK_CODE_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is essentially
// a disassembly of the internal data-structures to source code.
//
// #Returns:    Source code string version of itself
//
// #Notes:
//   The code generated may not look exactly like the original source - for example any
//   comments will not be retained, but it should parse equivalently.
//
// #Modifiers: virtual - overridden from SkExpressionBase
// #See:       as_binary()
AString SkNilCoalescing::as_code() const
  {
  AString alt_str(m_alternate_p->as_code());
  AString str(m_trial_p->as_code(), 4u + alt_str.get_length());

  str.append(" ?? ", 4u);
  str.append(alt_str);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains was located
// at or follows at the character index position provided and returns it.
//
// #Returns:
//   The first expression that starts or follows the given position or nullptr if no such
//   expression was found.
// #Params:
//   pos: code file/string character index position
//
// #Notes:
//   Some sub-expressions may have the same starting character position as their
//   containing expression - in these cases only the containing expression is returned.
//
// #Modifiers: virtual - overridden from SkExpressionBase
SkExpressionBase * SkNilCoalescing::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * found_p = m_trial_p->find_expr_by_pos(pos, type);

  if (found_p)
    {
    return found_p;
    }

  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkNilCoalescing *>(this);
    }

  return m_alternate_p->find_expr_by_pos(pos, type);
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// #Returns:
//   AIterateResult_early_exit if iteration stopped/aborted early or AIterateResult_entire
//   if full iteration performed.
//
// #Modifiers: virtual - overridden from SkExpressionBase
// #See:       SkApplyExpressionBase, *: :iterate_expressions()
eAIterateResult SkNilCoalescing::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_trial_p->iterate_expressions(apply_expr_p, invokable_p)
    || m_alternate_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Evaluates expression and returns the resulting instance if desired.
//
// #Returns:
//   true if the expression has completed its evaluation and there is a resulting
//   instance, false if there is a result pending.
//
// #Params:
//   scope_p:
//     scope for data/method/etc. look-ups. It should always be an object derived from
//     SkInvokedContextBase.
//   caller_p:
//     object that called/invoked this expression and that may await a result.  If it is
//     nullptr, then there is no object that needs to be returned to and notified when
//     this invocation is complete.  (Default nullptr)
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is nullptr, then the result does not need to be returned and
//     only side-effects are desired.  (Default nullptr)
//
// #Modifiers: virtual - overridden from SkExpressionBase
// #See:       invoke_now()
SkInvokedBase * SkNilCoalescing::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkInstance * result_p = m_trial_p->invoke_now(scope_p, caller_p);

  if (result_p != SkBrain::ms_nil_p)
    {
    // Trial result is non-nill so keep it.
    if (result_pp)
      {
      *result_pp = result_p;
      }

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Trial result is nill so return result of alternate expression instead.
  result_p->dereference();

  return m_alternate_p->invoke(scope_p, caller_p, result_pp);
  }

//---------------------------------------------------------------------------------------
// Determines if expression completes immediately (true) - i.e. completes in 1 frame
// - or that its completion may be durational (false) - i.e. may take 1 or more frames.
//
// #Returns: true if immediate and false if durational
//
// #Params:
//   durational_idx_p:
//     if this expression is durational, the address to store the character index
//     position of the first durational expression - either this expression or a
//     sub-expression that it contains.
//
// #Notes:
//   Coroutines are durational, some expressions such as a code block may be immediate or
//   durational depending on its sub-expressions and some expressions such as identifiers
//   are always immediate.
//
// #Modifiers: virtual - overridden from SkExpressionBase
bool SkNilCoalescing::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// The trial expression is not technically a receiver but it precedes the test so is
// subject to the same recovery mechanism.
//
// #Params:
//   receiver_p: sub receiver to find and set to nullptr.
void SkNilCoalescing::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_trial_p)
    {
    if (m_trial_p == receiver_p)
      {
      m_trial_p = nullptr;
      }
    else
      {
      m_trial_p->null_receiver(receiver_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
//
// #See:       SkDebug, AMemoryStats
// #Modifiers: virtual - overridden from SkExpressionBase
void SkNilCoalescing::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkNilCoalescing, SkDebugInfo_size_used));
  m_trial_p->track_memory(mem_stats_p);
  m_alternate_p->track_memory(mem_stats_p);
  }
