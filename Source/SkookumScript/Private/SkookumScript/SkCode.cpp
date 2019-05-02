// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Classes for custom/compound expressions
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkLiteralClosure.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkCode.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkMind.hpp>


//=======================================================================================
// SkCode Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkCode::~SkCode()
  {
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               2 bytes - start index of temp variables
//               2 bytes - number of temp variables
//               4 bytes - temp variable name id }- Repeating 
//               4 bytes - number of statements
//               n bytes - statement typed binary }- Repeating 
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkCode::assign_binary(const void ** binary_pp)
  {
  // 2 bytes - start index of temp variables
  m_temp_vars_start_idx.assign_binary(binary_pp);

  // 2 bytes - number of temp variables
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);

  // 4 bytes - temp variable name id }- Repeating 
  m_temp_vars.empty_ensure_count_undef(length);

  for (; length > 0u ; length--)
    {
    m_temp_vars.append_last_undef(ASymbol::create_from_binary(binary_pp));
    }

  // 4 bytes - number of statements
  length = A_BYTE_STREAM_UI32_INC(binary_pp);

  // n bytes - argument typed binary }- Repeating 
  // Allocate on stack first, as we might not use all of the statements
  SkExpressionBase ** temp_statements_pp = a_stack_allocate(length, SkExpressionBase *);
  uint32_t num_statements = 0;
  for (uint32_t count = length; count; --count)
    {
    SkExpressionBase * statement_p = from_binary_typed_new(binary_pp);

    // In shipping builds, remove any invocations on the Debug class
    #if !(SKOOKUM & SK_DEBUG)
      if (statement_p->is_debug_call())
        {
        delete statement_p;
        continue;
        }
    #endif

    temp_statements_pp[num_statements++] = statement_p;
    }
  // Now that number of used statements is known, allocate array
  m_statements.set_size_free(num_statements);
  ::memcpy(m_statements.get_array(), temp_statements_pp, num_statements * sizeof(temp_statements_pp[0]));
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this code block and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               2 bytes - start index of temp variables
//               2 bytes - number of temp variables
//               4 bytes - temp variable name id }- Repeating 
//               4 bytes - number of statements
//               n bytes - statement typed binary }- Repeating 
//
//             $Note - CReis This could be packed more
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkCode::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCode::as_binary_length());

  // 2 bytes - start index of temp variables
  m_temp_vars_start_idx.as_binary(binary_pp);

  // 2 bytes - number of temp variables
  uint16_t temp_var_count = (uint16_t)m_temp_vars.get_count();
  A_BYTE_STREAM_OUT16(binary_pp, &temp_var_count);

  // 4 bytes - temp variable name id }- Repeating 
  ASymbol * vars_p = m_temp_vars.get_array();
  ASymbol * vars_end_p = vars_p + temp_var_count;
  for (; vars_p < vars_end_p; vars_p++)
    {
    vars_p->as_binary(binary_pp);
    }

  // 4 bytes - number of statements
  uint32_t ** data_pp = (uint32_t **)binary_pp;
  uint32_t length = m_statements.get_length();
  **data_pp = length;
  (*data_pp)++;

  // n bytes - statement typed binary }- Repeating 
  SkExpressionBase ** exprs_pp = m_statements.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + length;
  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    (*exprs_pp)->as_binary_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               2 bytes - start index of temp variables
//               2 bytes - number of temp variables
//               4 bytes - temp variable name id }- Repeating 
//               4 bytes - number of statements
//               n bytes - statement typed binary }- Repeating 
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkCode::as_binary_length() const
  {
  uint32_t            binary_length = 6u + m_temp_vars_start_idx.as_binary_length() + (4u * m_temp_vars.get_count());
  SkExpressionBase ** exprs_pp      = m_statements.get_array();
  SkExpressionBase ** exprs_end_pp  = exprs_pp + m_statements.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    binary_length += (*exprs_pp)->as_binary_typed_length();
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// #Description
//   Returns this expression (or any *last* sub-expression) that would have no *direct*
//   side-effect (i.e. when a part of its functionality is useless even when it has a
//   sub-expression that has a side effect) when its result is not used (i.e. not used as
//   an argument/receiver or returned as the last expression in a code block).
//   
//   If an expression is returned to determine whether it had no effects or if it has sub
//   expressions with effects call get_side_effect() on it.
//
//   Non-last sub-expressions - especially for code blocks - are automatically tested for
//   side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkCode::find_expr_last_no_side_effect() const
  {
  SkExpressionBase * expr_p = m_statements.get_last();

  return expr_p ? expr_p->find_expr_last_no_side_effect() : this;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Indicates whether this expression (or any sub-expression) has any potential side
//   effects when used as a stand alone statement - i.e. not used as an argument/receiver
//   or returned as the last expression in a code block.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  find_expr_last_no_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns See eSkSideEffect
  eSkSideEffect
SkCode::get_side_effect() const
  {
  uint32_t count = m_statements.get_length();

  if (count)
    {
    // Iterate through statement expressions
    SkExpressionBase ** expr_pp     = m_statements.get_array();
    SkExpressionBase ** expr_end_pp = expr_pp + count;

    while (expr_pp < expr_end_pp)
      {
      if ((*expr_pp)->get_side_effect())
        {
        return SkSideEffect_present;
        }

      expr_pp++;
      }
    }

  return SkSideEffect_none;
  }

#endif

  
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
AString SkCode::as_code() const
  {
  AString str;

  // Give it some breathing room
  str.ensure_size(128u);

  str.append('[');

  // Add temporary variables
  // Note that they may not appear as they did originally in the source code.  All
  // temporary variable creations are moved to the top of the code block and put into
  // alphabetical order.  Also if they were created with bindings, the bindings are added
  // when iterating through the expressions.
  uint length = m_temp_vars.get_count();

  if (length)
    {
    ASymbol * vars_p     = m_temp_vars.get_array();
    ASymbol * vars_end_p = vars_p + length;

    for (; vars_p < vars_end_p; vars_p++)
      {
      str.append("\n!", 2u);
      str.append(vars_p->as_str_dbg());
      }

    // Put an extra line break after temporary variables
    str.append('\n');
    }


  // Add expressions
  length = m_statements.get_length();

  if (length)
    {
    SkExpressionBase ** exprs_pp     = m_statements.get_array();
    SkExpressionBase ** exprs_end_pp = exprs_pp + length;

    for (; exprs_pp < exprs_end_pp; exprs_pp++)
      {
      str.append('\n');
      str.append((*exprs_pp)->as_code());
      }

    }

  str.append("\n]", 2u);


  // Indent
  str.line_indent(SkDebug::ms_indent_size);

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkCode::as_callstack_label() const
  {
  return AString("[ ]", true);
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
SkExpressionBase * SkCode::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkCode *>(this);
    }


  // Iterate through expressions
  SkExpressionBase *  found_p      = nullptr;
  SkExpressionBase ** exprs_pp     = m_statements.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_statements.get_length();

  while (exprs_pp < exprs_end_pp)
    {
    found_p = (*exprs_pp)->find_expr_by_pos(pos, type);

    if (found_p)
      {
      break;
      }

    exprs_pp++;
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
SkCode::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  apply_expr_p->apply_expr(this, invokable_p);

  SkExpressionBase ** exprs_pp     = m_statements.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_statements.get_length();

  while (exprs_pp < exprs_end_pp)
    {
    if ((*exprs_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }

    exprs_pp++;
    }

  return AIterateResult_entire;
  }

//---------------------------------------------------------------------------------------
// Gets the current source character index or ADef_uint32 if it cannot be
//             determined at runtime.
// Returns:    current source character index or ADef_uint32
// Arg         iexpr - invoked expression wrapped around this expression
// Modifiers:   virtual
// Author(s):   Conan Reis
SkDebugInfo SkCode::get_debug_info(const SkInvokedExpression & iexpr) const
  {
  SkExpressionBase * current_expr_p = m_statements.get_at(iexpr.m_index);
  SkDebugInfo        debug_info     = {current_expr_p->m_source_idx, current_expr_p->m_debug_info};

  return debug_info;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkCode::get_type() const
  {
  return SkExprType_code;
  }

//---------------------------------------------------------------------------------------
// Iterate through statements in code block
// 
// Returns:
//   true if the code block has completed its evaluation and there is a resulting instance,
//   false if there is a result pending.
//   
// Params:
//   iexpr_p: the invoked expression wrapped around this code block
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is nullptr, then the result does not need to be returned and
//     only side-effects are desired.
//     
// Notes:     Called by invoke() and SkInvokedExpression::pending_return()
// Author(s): Conan Reis
bool SkCode::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  // If not initial then it is being resumed from a past call that was non-immediate.
  bool initial_call = (iexpr_p->m_index == 0u);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through statements
  SkInvokedContextBase * scope_p;
  SkInvokedBase *        invoked_p = nullptr;
  SkExpressionBase **    exprs_pp  = m_statements.get_array();
  uint32_t               length    = m_statements.get_length() - 1u;  // Treat the last statement differently since it is the only statement where its return value may be kept.

  // Iterate through all but last statement
  // Uses iexpr_p->m_index for iteration since it could be exited/aborted before the
  // iteration has been completed. Can be > length when called back from a non-immediate
  // last expression pending_return().
  while (iexpr_p->m_index < length)
    {
    // $Revisit - CReis Repeated calls to refresh the scope might be redundant though it
    // may be garbage collected during an earlier call. Double-check.
    scope_p = static_cast<SkInvokedContextBase *>(iexpr_p->m_scope_p.get_obj());

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Is scope lost?
    if (scope_p == nullptr)
      {
      // Partial clean up
      SkInvokedBase * caller_p = iexpr_p->get_caller();

      // If this is not the initial call, return to caller if one is waiting
      if (!initial_call && caller_p && caller_p->pending_count())
        {
        caller_p->pending_unregister(iexpr_p);
        }

      // This code block completed, so free up invoked expression wrapper
      SkInvokedExpression::pool_delete(iexpr_p);

      return true;
      }

    // The expressions proceeding the last call do not care about returned objects
    invoked_p = (exprs_pp[iexpr_p->m_index])->invoke(scope_p, iexpr_p);

    if (invoked_p)
      {
      // This code block did not complete immediately.
      iexpr_p->pending_deferred(invoked_p);

      // index incremented on expression completion in `SkInvokedExpression::pending_return()`
      return false;
      }

    iexpr_p->m_index++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Last statement reached - invoke it with 'result_pp'
  scope_p = static_cast<SkInvokedContextBase *>(iexpr_p->m_scope_p.get_obj());

  if (scope_p && (iexpr_p->m_index == length))
    {
    invoked_p = (exprs_pp[iexpr_p->m_index])->invoke(scope_p, iexpr_p, result_pp);

    if (invoked_p)
      {
      // This code block did not complete immediately.
      iexpr_p->pending_deferred(invoked_p);

      return false;
      }

    scope_p = static_cast<SkInvokedContextBase *>(iexpr_p->m_scope_p.get_obj());
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Completed execution - now cleanup

  // Release references to temporary variables
  if (scope_p && m_temp_vars.get_count() > 0)
    {
    scope_p->data_destroy_vars(m_temp_vars_start_idx.m_data_idx, m_temp_vars.get_count(), result_pp ? *result_pp : nullptr);
    }

  SkInvokedBase * caller_p = iexpr_p->get_caller();

  // If this is not the initial call, return to caller if one is waiting
  if (!initial_call && caller_p && caller_p->pending_count())
    {
    caller_p->pending_unregister(iexpr_p);
    }

  // This code block completed, so free up invoked expression wrapper
  SkInvokedExpression::pool_delete(iexpr_p);

  return true;
  }

//---------------------------------------------------------------------------------------
// Evaluates code expression and returns the resulting instance if desired.
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
SkInvokedBase * SkCode::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Only bother if there are expressions
  if (m_statements.is_empty())
    {
    // If result desired in code block without any statements, return 'nil'
    // $Vital - CReis Update the parser result type to <None|LastType> if durational possible.
    // Wanted a return, but was durational - return a nil so there is something
    if (result_pp)
      {
      *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
      }

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up temporary variables
  // Each variable is initially set to the 'nil' object.
  // This is needed despite SkBind/SkRebind because a code block can be exited/aborted before all locals have been bound,
  // so the cleanup code needs to find something valid to clean up
  static_cast<SkInvokedContextBase *>(scope_p)->data_create_vars(m_temp_vars_start_idx.m_data_idx, m_temp_vars.get_count());

  SkInvokedExpression * iexpr_p = SkInvokedExpression::pool_new(*this, caller_p, scope_p);

  SKDEBUG_HOOK_EXPR(this, scope_p, iexpr_p, nullptr, SkDebug::HookContext_current);

  if (invoke_iterate(iexpr_p, result_pp))
    {
    // Completed immediately
    // iexpr_p is cleaned up in invoke_iterate()
    return nullptr;
    }

  // Completion was durational
  return iexpr_p;
  }

//---------------------------------------------------------------------------------------
// Exits / unwinds code block
//
// #Modifiers virtual - overridden from SkExpressionBase
// #Author(s) Conan Reis
void SkCode::invoke_exit(
  // invoked expression wrapped around this code block
  SkInvokedExpression * iexpr_p,
  // Active sub-expression requesting the exit or nullptr if not during active sub-expression.
  SkInvokedExpression * sub_exit_p // = nullptr
  ) const
  {
  // Jump to end of code block
  iexpr_p->m_index = m_statements.get_length();
  }

//---------------------------------------------------------------------------------------
// Called during an abort just before the iexpr gets deleted
void SkCode::on_abort(SkInvokedExpression * iexpr_p) const
  {
  SkInvokedContextBase * scope_p = static_cast<SkInvokedContextBase *>(iexpr_p->m_scope_p.get_obj());

  // Release references to temporary variables
  if (scope_p && m_temp_vars.get_count() > 0)
    {
    scope_p->data_destroy_vars(m_temp_vars_start_idx.m_data_idx, m_temp_vars.get_count());
    // $Revisit MBreyer since temp vars can share memory in nested scopes,
    // on_abort() might get called more than once on the same variable
    // So fill with nil so repeated destruction will not be an issue
    scope_p->data_create_vars(m_temp_vars_start_idx.m_data_idx, m_temp_vars.get_count());
    }
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
bool SkCode::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Determine if any sub-expressions take more than one frame to execute and return.
  SkExpressionBase ** exprs_pp     = m_statements.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_statements.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    if (!(*exprs_pp)->is_immediate(durational_idx_p))
      {
      // Found sub-expression that takes more than one frame to execute and return.
      return false;
      }
    }

  // All sub-exprs execute and return in one frame.
  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCode::track_memory(AMemoryStats * mem_stats_p) const
  {
  // Symbols essentially have no dynamic data
  uint32_t var_bytes = m_temp_vars.get_size_buffer_bytes();

  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkCode, SkDebugInfo_size_used),
    var_bytes + (m_statements.get_length() * sizeof(void *)),
    var_bytes + m_statements.track_memory(mem_stats_p));
  }


//=======================================================================================
// SkConcurrentSync Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkConcurrentSync::SkConcurrentSync(const void ** binary_pp)
  {
  // 1 byte - number of statements
  uint32_t length = A_BYTE_STREAM_UI8_INC(binary_pp);

  // n bytes - statement typed binary }- Repeating 
  m_exprs.set_size_free(length);

  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + length;

  while (exprs_pp < exprs_end_pp)
    {
    (*exprs_pp) = from_binary_typed_new(binary_pp);
    exprs_pp++;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this invocation cascade and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentSync::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkConcurrentSync::as_binary_length());

  // 1 byte - number of statements
  uint32_t length = m_exprs.get_length();

  **(uint8_t **)binary_pp = uint8_t(length);
  (*(uint8_t **)binary_pp)++;


  // 4 bytes - statement }- Repeating 
  SkExpressionBase ** statements_pp     = m_exprs.get_array();
  SkExpressionBase ** statements_end_pp = statements_pp + length;

  for (; statements_pp < statements_end_pp; statements_pp++)
    {
    (*statements_pp)->as_binary_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - receiver expression or inferred this (nullptr)
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkConcurrentSync::as_binary_length() const
  {
  uint32_t binary_length = 1u;

  // 4 bytes - statement }- Repeating 
  SkExpressionBase ** statements_pp     = m_exprs.get_array();
  SkExpressionBase ** statements_end_pp = statements_pp + m_exprs.get_length();

  for (; statements_pp < statements_end_pp; statements_pp++)
    {
    binary_length += (*statements_pp)->as_binary_typed_length();
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


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
AString SkConcurrentSync::as_code() const
  {
  AString str;

  // Give it some breathing room
  str.ensure_size(128u);

  str.append("sync\n", 5u);
  str.append("[\n", 2u);


  // Add expressions
  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    str.append((*exprs_pp)->as_code());
    str.append('\n');
    }

  str.append("]\n", 2u);

  // Indent block
  str.line_indent(SkDebug::ms_indent_size, 5u);

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkConcurrentSync::as_callstack_label() const
  {
  return AString("sync", true);
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
SkExpressionBase * SkConcurrentSync::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkConcurrentSync *>(this);
    }

  // Test sub-expressions
  SkExpressionBase *  found_p;
  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    found_p = (*exprs_pp)->find_expr_by_pos(pos, type);

    if (found_p)
      {
      return found_p;
      }
    }

  return nullptr;
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
SkConcurrentSync::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  apply_expr_p->apply_expr(this, invokable_p);

  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  while (exprs_pp < exprs_end_pp)
    {
    if ((*exprs_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }

    exprs_pp++;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkConcurrentSync::get_type() const
  {
  return SkExprType_concurrent_sync;
  }

//---------------------------------------------------------------------------------------
// Evaluates code expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkConcurrentSync::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SkInvokedBase *       invoked_p;
  SkInvokedExpression * iexpr_p      = SkInvokedExpression::pool_new(*this, caller_p, scope_p);
  uint32_t &            index        = iexpr_p->m_index;
  uint32_t              thread_count = m_exprs.get_length();
  SkExpressionBase **   exprs_pp     = m_exprs.get_array();

  SKDEBUG_HOOK_EXPR(this, scope_p, iexpr_p, nullptr, SkDebug::HookContext_current);

  // Iterate through all threads and start them.
  // Uses iexpr_p->m_index for iteration since it could be exited/aborted before the
  // iteration has been completed.
  while (index < thread_count)
    {
    // This may complete immediately or take several frames to complete
    invoked_p = exprs_pp[index]->invoke(scope_p, iexpr_p);

    if (invoked_p)
      {
      iexpr_p->pending_deferred(invoked_p);
      }

    index++;
    }

  // If result is desired, return 'nil'
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  if (iexpr_p->pending_count())
    {
    // One or more threads are incomplete
    return iexpr_p;
    }

  // All threads completed
  SkInvokedExpression::pool_delete(iexpr_p);

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
bool SkConcurrentSync::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    if (durational_idx_p)
      {
      *durational_idx_p = m_source_idx;
      }
  #endif

  // Concurrent expressions may take more than one frame to execute and return.
  // [No point running them concurrently if they didn't.]
  return false;
  }

//---------------------------------------------------------------------------------------
// Exits / unwinds concurrent sync
//
// #Modifiers virtual - overridden from SkExpressionBase
// #Author(s) Conan Reis
void SkConcurrentSync::invoke_exit(
  // invoked expression wrapped around this expression
  SkInvokedExpression * iexpr_p,
  // Active sub-expression requesting the exit or nullptr if not during active sub-expression.
  SkInvokedExpression * sub_exit_p // = nullptr
  ) const
  {
  // Set index to end since this could be called in the middle of an invoke before all the
  // threads/ have been created.
  iexpr_p->m_index = m_exprs.get_length();

  if (sub_exit_p)
    {
    // Exit during active sub-expression
    if (sub_exit_p->AListNode<SkInvokedBase>::is_in_list())
      {
      // Previously called and caller is pending on it
      iexpr_p->m_calls.remove(sub_exit_p);
      iexpr_p->abort_subcalls(SkNotify_ignore);
      iexpr_p->m_calls.append(sub_exit_p);
      }
    else
      {
      iexpr_p->abort_subcalls(SkNotify_ignore);
      }
    }
  else
    {
    iexpr_p->abort_subcalls();
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentSync::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkConcurrentSync, SkDebugInfo_size_used),
    m_exprs.get_length() * sizeof(void *),
    m_exprs.track_memory(mem_stats_p));
  }


//=======================================================================================
// SkConcurrentRace Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkConcurrentRace::SkConcurrentRace(const void ** binary_pp)
  {
  // 1 byte - number of statements
  uint32_t length = A_BYTE_STREAM_UI8_INC(binary_pp);

  // n bytes - statement typed binary }- Repeating 
  m_exprs.set_size_free(length);

  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + length;

  while (exprs_pp < exprs_end_pp)
    {
    (*exprs_pp) = from_binary_typed_new(binary_pp);
    exprs_pp++;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this object and increments the memory address to just past the
//             last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentRace::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkConcurrentRace::as_binary_length());

  // 1 byte - number of statements
  uint32_t length = m_exprs.get_length();

  **(uint8_t **)binary_pp = uint8_t(length);
  (*(uint8_t **)binary_pp)++;


  // 4 bytes - statement }- Repeating 
  SkExpressionBase ** statements_pp     = m_exprs.get_array();
  SkExpressionBase ** statements_end_pp = statements_pp + length;

  for (; statements_pp < statements_end_pp; statements_pp++)
    {
    (*statements_pp)->as_binary_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - receiver expression or inferred this (nullptr)
//               1 byte  - number of statements
//               n bytes - statement typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkConcurrentRace::as_binary_length() const
  {
  uint32_t binary_length = 1u;

  // 4 bytes - statement }- Repeating 
  SkExpressionBase ** statements_pp     = m_exprs.get_array();
  SkExpressionBase ** statements_end_pp = statements_pp + m_exprs.get_length();

  for (; statements_pp < statements_end_pp; statements_pp++)
    {
    binary_length += (*statements_pp)->as_binary_typed_length();
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


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
AString SkConcurrentRace::as_code() const
  {
  AString str;

  // Give it some breathing room
  str.ensure_size(128u);

  str.append("race\n", 5u);
  str.append("[\n", 2u);


  // Add expressions
  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    str.append((*exprs_pp)->as_code());
    str.append('\n');
    }

  str.append("]\n", 2u);

  // Indent block
  str.line_indent(SkDebug::ms_indent_size, 5u);

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkConcurrentRace::as_callstack_label() const
  {
  return AString("race", true);
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
SkExpressionBase * SkConcurrentRace::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkConcurrentRace *>(this);
    }

  // Test sub-expressions
  SkExpressionBase *  found_p;
  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  for (; exprs_pp < exprs_end_pp; exprs_pp++)
    {
    found_p = (*exprs_pp)->find_expr_by_pos(pos, type);

    if (found_p)
      {
      return found_p;
      }
    }

  return nullptr;
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
SkConcurrentRace::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  apply_expr_p->apply_expr(this, invokable_p);

  SkExpressionBase ** exprs_pp     = m_exprs.get_array();
  SkExpressionBase ** exprs_end_pp = exprs_pp + m_exprs.get_length();

  while (exprs_pp < exprs_end_pp)
    {
    if ((*exprs_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }

    exprs_pp++;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkConcurrentRace::get_type() const
  {
  return SkExprType_concurrent_race;
  }

//---------------------------------------------------------------------------------------
// Evaluates code expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkConcurrentRace::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Start all racing threads
  SkInvokedBase *       invoked_p;
  bool                  completed         = false;
  SkExpressionBase **   statements_pp     = m_exprs.get_array();
  SkExpressionBase **   statements_end_pp = statements_pp + m_exprs.get_length();
  SkInvokedExpression * iexpr_p           = SkInvokedExpression::pool_new(*this, caller_p, scope_p);

  SKDEBUG_HOOK_EXPR(this, scope_p, iexpr_p, nullptr, SkDebug::HookContext_current);

  for (; statements_pp < statements_end_pp; statements_pp++)
    {
    // This may complete immediately or take several frames to complete
    invoked_p = (*statements_pp)->invoke(scope_p, iexpr_p);

    if (invoked_p == nullptr)
      {
      // A racer completed immediately, so race is over.
      completed = true;

      break;
      }

    iexpr_p->pending_deferred(invoked_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If result is desired, return 'nil'
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Did a racer complete immediately?
  if (completed)
    {
    // Completed immediately
    iexpr_p->abort_subcalls(SkNotify_ignore);
    SkInvokedExpression::pool_delete(iexpr_p);

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Did not complete immediately
  // Only wait for first sub-expression to return, but keep track of number of sub-calls
  iexpr_p->m_data = iexpr_p->pending_count();
  // $Revisit - CReis This should probably be = pending - (call count - 1) since calls
  // could add additional things to be pending on.
  iexpr_p->pending_set(1u);

  return iexpr_p;
  }

//---------------------------------------------------------------------------------------
// Exits / unwinds concurrent race
//
// #Modifiers virtual - overridden from SkExpressionBase
// #Author(s) Conan Reis
void SkConcurrentRace::invoke_exit(
  // invoked expression wrapped around this expression
  SkInvokedExpression * iexpr_p,
  // Active sub-expression requesting the exit or nullptr if not during active sub-expression.
  SkInvokedExpression * sub_exit_p // = nullptr
  ) const
  {
  if (sub_exit_p == nullptr)
    {
    invoke_iterate(iexpr_p);
    }
  }

//---------------------------------------------------------------------------------------
// Iterate through sub-expressions/re-entrant calls
// Returns:    true if the invoke expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         iexpr_p - the invoked expression wrapped around this expression
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Notes:      Called by invoke() and SkInvokedExpression::pending_return()
// Author(s):   Conan Reis
bool SkConcurrentRace::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  uint32_t racers_count = uint32_t(iexpr_p->m_data);

  if (racers_count > 0u)
    {
    // Race won - continue

    // Set pending to include remaining racers - this ensures that things other than
    // sub-calls are taken into account.
    iexpr_p->m_data = 0u;
    iexpr_p->pending_set(iexpr_p->pending_count() + racers_count - 1u);
    iexpr_p->abort_subcalls();

    SkInvokedExpression::pool_delete(iexpr_p);
    }

  return true;
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
bool SkConcurrentRace::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    if (durational_idx_p)
      {
      *durational_idx_p = m_source_idx;
      }
  #endif

  // Concurrent racing expressions may take more than one frame to execute and return.
  // [No point running them concurrently if they didn't.]
  return false;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentRace::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkConcurrentRace, SkDebugInfo_size_used),
    m_exprs.get_length() * sizeof(void *),
    m_exprs.track_memory(mem_stats_p));
  }


//=======================================================================================
// SkConcurrentBranch Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkConcurrentBranch::~SkConcurrentBranch()
  {
  //ARefPtr will auto-delete the closure info
  //delete m_info_p;
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this branched expression and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - SkClosureInfoCoroutine
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentBranch::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkConcurrentBranch::as_binary_length());

  m_info_p->as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - SkClosureInfoCoroutine
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkConcurrentBranch::as_binary_length() const
  {
  return m_info_p->as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


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
AString SkConcurrentBranch::as_code() const
  {
  AString str("branch\n", 7u);

  str.append(m_info_p->get_closure_expr()->as_code_block());

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
SkExpressionBase * SkConcurrentBranch::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkConcurrentBranch *>(this);
    }

  // Test sub-expression
  return m_info_p->find_expr_by_pos(pos, type);
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
SkConcurrentBranch::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
   || m_info_p->get_invokable()->iterate_expressions(apply_expr_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkConcurrentBranch::get_type() const
  {
  return SkExprType_concurrent_branch;
  }

//---------------------------------------------------------------------------------------
// Evaluates code expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkConcurrentBranch::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // This may complete immediately or take several frames to complete, but ignore that
  // and let it go (also ignore return and don't bother notifying when it completes).

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Closure is a durational coroutine.

  // Create an invoked coroutine from the closure
  SkInvokedCoroutine * icoro_p = SkInvokedCoroutine::pool_new(m_info_p);
  SkMind * updater_p = (caller_p && caller_p->is_valid_id()) ? caller_p->get_updater() : nullptr;
  // We pass nullptr as caller instead of caller_p, so that the invoked coroutine keeps running independently of the caller
  icoro_p->reset(SkCall_interval_always, nullptr, scope_p->get_topmost_scope(), updater_p, nullptr);

  // Must be called before calling argument expressions
  SKDEBUG_ICALL_SET_EXPR(icoro_p, m_info_p->get_closure_expr());

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Capture any referenced variables
  uint32_t capture_count = m_info_p->get_captured().get_count();
  if (capture_count)
    {
    SK_ASSERTX(scope_p->get_obj_type() == SkObjectType_invoked_context, "SkConcurrentBranch must be invoked with a SkInvokedContextBase scope.");
    SkInvokedContextBase * invoked_p = static_cast<SkInvokedContextBase *>(scope_p);

    const SkNamedIndexed * capture_name_p = m_info_p->get_captured().get_array();
    const SkNamedIndexed * capture_name_end_p = capture_name_p + capture_count;
    while (capture_name_p < capture_name_end_p)
      {
      SK_ASSERTX(capture_name_p->get_data_idx() < invoked_p->get_data().get_size(), "Access out of range");
      SkInstance * var_p = invoked_p->get_data().get_array()[capture_name_p->get_data_idx()];
      var_p->reference();
      icoro_p->data_append_arg(var_p);
      capture_name_p++;
      }
    }

  // Hook must be called after argument expressions and before invoke()
  SKDEBUG_HOOK_EXPR(this, scope_p, icoro_p, caller_p, SkDebug::HookContext_peek);

  // Return the invoked coroutine as a result
  if (result_pp)
    {
    // Ref it before it potentially dies
    *result_pp = icoro_p->as_new_instance();
    }

  // Invoke the coroutine on the receiver - try to have it complete this frame
  icoro_p->on_update();

  // Always claim it completes immediately whether it does or not.
  // The caller and this branch will still share the same updater.
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Iterate through sub-expressions/re-entrant calls
// Returns:    true if the invoke expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         iexpr_p - the invoked expression wrapped around this expression
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Notes:      Called by invoke() and SkInvokedExpression::pending_return()
// Author(s):   Conan Reis
bool SkConcurrentBranch::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  // Do not return to any caller
  SkInvokedExpression::pool_delete(iexpr_p);

  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConcurrentBranch::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkConcurrentBranch, SkDebugInfo_size_used));

  m_info_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkChangeMind Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkChangeMind::~SkChangeMind()
  {
  delete m_mind_p;
  delete m_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this expression and increments the memory address to just past the
//             last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - mind expression typed binary
//               n bytes - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkChangeMind::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkChangeMind::as_binary_length());

  m_mind_p->as_binary_typed(binary_pp);

  m_expr_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - mind expression typed binary
//               n bytes - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkChangeMind::as_binary_length() const
  {
  return m_mind_p->as_binary_typed_length() + m_expr_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// #Description
//   Returns this expression (or any *last* sub-expression) that would have no *direct*
//   side-effect (i.e. when a part of its functionality is useless even when it has a
//   sub-expression that has a side effect) when its result is not used (i.e. not used as
//   an argument/receiver or returned as the last expression in a code block).
//   
//   If an expression is returned to determine whether it had no effects or if it has sub
//   expressions with effects call get_side_effect() on it.
//
//   Non-last sub-expressions - especially for code blocks - are automatically tested for
//   side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkChangeMind::find_expr_last_no_side_effect() const
  {
  return m_expr_p->find_expr_last_no_side_effect();
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
AString SkChangeMind::as_code() const
  {
  AString str("change ", 7u);

  str.append(m_mind_p->as_code());
  str.append(' ');

  if (m_expr_p->get_type() == SkExprType_code)
    {
    str.append('\n');
    }

  str.append(m_expr_p->as_code());

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkChangeMind::as_callstack_label() const
  {
  return AString("change", true);
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
SkExpressionBase * SkChangeMind::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkChangeMind *>(this);
    }

  // Stopping on a change is not really informative, so skip to sub-expressions
  SkExpressionBase * expr_p = m_mind_p->find_expr_by_pos(pos, type);

  if (expr_p)
    {
    return expr_p;
    }

  return m_expr_p->find_expr_by_pos(pos, type);
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
SkChangeMind::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_mind_p->iterate_expressions(apply_expr_p, invokable_p)
    || m_expr_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkChangeMind::get_type() const
  {
  return SkExprType_change;
  }

//---------------------------------------------------------------------------------------
// Gets the mind object from the invoked expression.
SkMind * SkChangeMind::get_changed_mind(const SkInvokedExpression & iexpr) const
  {
  // Emulate AIdPtr<SkMind>.
  // $Revisit - may want to return SkookumScript::get_master_mind() rather than nullptr
  SkMind * mind_p = reinterpret_cast<SkMind *>(iexpr.m_data);
  return (mind_p && (mind_p->m_ptr_id == iexpr.m_index)) ? mind_p : nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the mind that is updating/owning this expression.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
SkMind * SkChangeMind::get_updater(const SkInvokedExpression & iexpr) const
  {
  // Return the updater mind (which might have been deleted, so catch that case as well)
  SkMind * updater_p = get_changed_mind(iexpr);
  return updater_p ? updater_p : SkExpressionBase::get_updater(iexpr);
  }

//---------------------------------------------------------------------------------------
// Evaluates code expression and returns the resulting instance if desired.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkChangeMind::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Special invoked expression that knows to change to a different mind for an updater.
  // It calls SkChangeMind::get_updater() above.
  SkInvokedExpression * iexpr_p = SkInvokedExpression::pool_new(*this, caller_p, scope_p);

  // Evaluate mind expression and store it
  SkInstance * mind_p = m_mind_p->invoke_now(iexpr_p->get_scope(), iexpr_p->get_caller());
  // Mind has an extra ref at this point, make sure to deref when call ends

  // Store info needed to emulate AIdPtr<SkMind>
  iexpr_p->m_data  = reinterpret_cast<uintptr_t>(mind_p);
  iexpr_p->m_index = mind_p ? mind_p->m_ptr_id : 0;

  SKDEBUG_HOOK_EXPR(this, scope_p, iexpr_p, nullptr, SkDebug::HookContext_current);

  // Call statements and ignore any pending expressions
  SkInvokedBase * invoked_p = m_expr_p->invoke(scope_p, iexpr_p, result_pp);

  if (invoked_p)
    {
    // This code block did not complete immediately.
    iexpr_p->pending_deferred(invoked_p);

    return iexpr_p;
    }

  // Finished immediately
  SkInvokedExpression::pool_delete(iexpr_p);

  // Done with using this mind
  mind_p->dereference();

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Iterate through sub-expressions/re-entrant calls
// Returns:    true if the invoke expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
bool SkChangeMind::invoke_iterate(SkInvokedExpression * iexpr_p, SkInstance ** result_pp /*= nullptr*/) const
  {
  // Our caller just incremented the index with good intentions, undo that to restore original value
  --iexpr_p->m_index;

  // Get our changed mind
  SkMind * mind_p = get_changed_mind(*iexpr_p);

  // Wrap up the invocation of this expression - which will also delete the iexpr
  SkExpressionBase::invoke_iterate(iexpr_p, result_pp);

  // Now that we're done, let go of the mind
  if (mind_p)
    {
    mind_p->dereference();
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called during an abort just before the iexpr gets deleted
void SkChangeMind::on_abort(SkInvokedExpression * iexpr_p) const
  {
  // We're done with the changed mind - let it go
  SkMind * mind_p = get_changed_mind(*iexpr_p);
  if (mind_p)
    {
    mind_p->dereference();
    }
  }

//---------------------------------------------------------------------------------------
// Determines if expression completes immediately (true) - i.e. completes in  1 frame
// - or that its completion may be durational (false) - i.e. may take 1 or more frames.
//
// Returns:    true if immediate and false if durational
// Arg         durational_idx_p - if this expression is durational, the address to store the
//             character index position of the first durational expression - either this
//             expression or a sub-expression that it contains.
// Notes:      Coroutines are durational, some expressions such as a code block may be
//             immediate or durational depending on its sub-expressions and some expressions
//             such as identifiers are always immediate.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
bool SkChangeMind::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  return m_expr_p->is_immediate(durational_idx_p);
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkChangeMind::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkChangeMind, SkDebugInfo_size_used));

  m_mind_p->track_memory(mem_stats_p);

  m_expr_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkBind Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkBind::~SkBind()
  {
  delete m_ident_p;
  delete m_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this bind expression and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - identifier expression typed binary
//               n bytes - expression typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkBind::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkBind::as_binary_length());

  // n bytes - identifier expression typed binary
  m_ident_p->as_binary_typed(binary_pp);

  // n bytes - expression typed binary
  m_expr_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - identifier name id
//               n bytes - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkBind::as_binary_length() const
  {
  return m_ident_p->as_binary_typed_length() + m_expr_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// #Description
//   Returns this expression (or any *last* sub-expression) that would have no *direct*
//   side-effect (i.e. when a part of its functionality is useless even when it has a
//   sub-expression that has a side effect) when its result is not used (i.e. not used as
//   an argument/receiver or returned as the last expression in a code block).
//   
//   If an expression is returned to determine whether it had no effects or if it has sub
//   expressions with effects call get_side_effect() on it.
//
//   Non-last sub-expressions - especially for code blocks - are automatically tested for
//   side effects immediately after they are parsed.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  get_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns nullptr or expression that has no side effects and not used as a result.
  const SkExpressionBase *
SkBind::find_expr_last_no_side_effect() const
  {
  // A rebind of a local variable as the last statement is redundant unless it is a return
  // argument.
  //return m_ident_p->is_local() ? this : nullptr;
  
  // @Revisit - CReis No way to test this with available context so assume that it has a
  // valid side effect.
  return nullptr;
  }

#endif  // (SKOOKUM & SK_CODE_IN)

  
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
AString SkBind::as_code() const
  {
  AString expr_str(m_expr_p->as_code());
  AString str(m_ident_p->as_code(), 16u + expr_str.get_length());

  // Determine if it is multi-line code
  if (expr_str.find('\n'))
    {
    // It is multi-line code so indent it.
    str.append(":\n", 2u);

    uint expr_start = str.get_length();

    str.append(expr_str);

    // Code is already indented
    if (m_expr_p->get_type() != SkExprType_code)
      {
      str.line_indent(SkDebug::ms_indent_size, expr_start);
      }
    }
  else
    {
    str.append(": ", 2u);
    str.append(expr_str);
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
SkExpressionBase * SkBind::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_ident_p->find_expr_by_pos(pos, type);

  if (expr_p)
    {
    return expr_p;
    }

  return ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkBind *>(this)
    : m_expr_p->find_expr_by_pos(pos, type);
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
SkBind::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_expr_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different expression types.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkBind::get_type() const
  {
  return SkExprType_bind;
  }

//---------------------------------------------------------------------------------------
// Evaluates bind expression and returns the instance bound to.
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkBind::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SkInstance *    result_p  = SkBrain::ms_nil_p;
  SkInvokedBase * invoked_p = m_expr_p->invoke(scope_p, caller_p, &result_p);
  
  // Hook is inside the bind call.
  m_ident_p->bind_data(
    #if (SKOOKUM & SK_DEBUG)
      const_cast<SkBind *>(this),
    #endif
    result_p,
    scope_p,
    caller_p,
    result_pp != nullptr);

  if (result_pp)
    {
    *result_pp = result_p;
    }
  else
    {
    result_p->dereference();
    }

  return invoked_p;
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
bool SkBind::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  return m_expr_p->is_immediate(durational_idx_p);
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkBind::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkBind, SkDebugInfo_size_used));

  m_ident_p->track_memory(mem_stats_p);
  m_expr_p->track_memory(mem_stats_p);
  }

