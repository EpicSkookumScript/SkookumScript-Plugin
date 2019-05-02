// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Invoked closure classes: SkInvokeClosureBase, SkInvokeClosureMethod and
//             SkInvokeClosureCoroutine
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvokeClosure.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvokeClosure.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInstance.hpp>


//=======================================================================================
// SkInvokeClosureBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkInvokeClosureBase::~SkInvokeClosureBase()
  {
  delete m_receiver_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - invoke info
//               n bytes - receiver expression typed binary or inferred this (nullptr)
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkInvokeClosureBase::SkInvokeClosureBase(const void ** binary_pp)
  : m_invoke_info(binary_pp)
  {
  // n bytes - receiver expression or inferred this (nullptr)
  m_receiver_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this invocation and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - invoke info
//               n bytes - receiver expression typed binary or inferred this (nullptr)
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeClosureBase::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvokeClosureBase::as_binary_length());

  // n bytes - invoke info
  m_invoke_info.as_binary(binary_pp);

  // n bytes - receiver expression typed binary or inferred this (nullptr)
  SkExpressionBase::as_binary_typed(m_receiver_p, binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - invoke info
//               n bytes - receiver expression typed binary or inferred this (nullptr)
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkInvokeClosureBase::as_binary_length() const
  {
  return m_invoke_info.as_binary_length() + SkExpressionBase::as_binary_typed_length(m_receiver_p);
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
AString SkInvokeClosureBase::as_code() const
  {
  AString str;

  if (m_receiver_p)
    {
    str = m_receiver_p->as_code();
    }
  else
    {
    str.append("this", 4u);
    }
  
  str.append(m_invoke_info.as_code());

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
SkExpressionBase * SkInvokeClosureBase::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_receiver_p  // Could be implicit 'this' (nullptr)
    ? m_receiver_p->find_expr_by_pos(pos, type)
    : nullptr;

  if (expr_p)
    {
    return expr_p;
    }

  // Return for all find types
  if ((m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkInvokeClosureBase *>(this);
    }

  // Test arguments
  return m_invoke_info.find_expr_by_pos(pos, type);
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
SkInvokeClosureBase::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || (m_receiver_p && m_receiver_p->iterate_expressions(apply_expr_p, invokable_p)))
    {
    return AIterateResult_early_exit;
    }

  return m_invoke_info.iterate_expressions(apply_expr_p, invokable_p);
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeClosureBase::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_receiver_p)
    {
    if (m_receiver_p == receiver_p)
      {
      m_receiver_p = nullptr;
      }
    else
      {
      m_receiver_p->null_receiver(receiver_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeClosureBase::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    (get_type() == SkExprType_invoke_closure_method) ? "SkInvokeClosureMethod" : "SkInvokeClosureCoroutine",
    sizeof(SkInvokeClosureBase) - sizeof(SkClosureInvokeInfo) - SkDebugInfo_size_used,
    SkDebugInfo_size_used);

  m_invoke_info.track_memory(mem_stats_p);

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkInvokeClosureMethod Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Evaluates invocation expression and returns the resulting instance if
//             desired.
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
SkInvokedBase * SkInvokeClosureMethod::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Note that the receiver is always an expression that returns immediately
  SkInstance * receiver_p = m_receiver_p
    ? m_receiver_p->invoke_now(scope_p, caller_p)  // evaluate receiver expression
    : scope_p->get_topmost_scope();                // 'this' is inferred


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Is receiver good?
  #if (SKOOKUM & SK_DEBUG)
    const char * bad_rcvr_p = nullptr;

    // Ensure receiver is non-nullptr
    if (receiver_p == nullptr)
      {
      bad_rcvr_p = "nullptr";
      }
    else
      {
      // Ensure that the receiver is a closure object
      eSkObjectType obj_type = receiver_p->get_obj_type();
      if (obj_type != SkObjectType_closure && obj_type != SkObjectType_invokable)
        {
        bad_rcvr_p = "not a closure object as expected";

        // Clean-up receiver created by expression
        if (m_receiver_p)
          {
          receiver_p->dereference();
          }
        }
      }

    if (bad_rcvr_p)
      {
      // $Note - CReis This should not need to be checked here at runtime - the parser
      // should have already prevented any misuse.  However an invalid type cast can be
      // used to fool the parser.
      SK_ERROR_INFO(
        a_str_format(
          "Tried to invoke/call a closure but it was %s!\n"
          "[Returning nil rather than calling a closure.]",
          bad_rcvr_p),
        caller_p);

      // Attempt simple recovery by returning nil rather than just crashing
      if (result_pp)
        {
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return nullptr;
      }
  #endif // (SKOOKUM & SK_DEBUG)

  // The actual invocation
  receiver_p->invoke_as_method(scope_p, caller_p, result_pp, m_invoke_info, this);

  // Clean-up receiver created by expression
  if (m_receiver_p)
    {
    receiver_p->dereference();
    }

  // Methods complete immediately
  return nullptr;
  }


//=======================================================================================
// SkInvokeClosureCoroutine Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Evaluates invocation expression and returns the resulting instance if
//             desired.
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
SkInvokedBase * SkInvokeClosureCoroutine::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Note that the receiver is always an expression that returns immediately
  SkInstance * receiver_p = m_receiver_p
    ? m_receiver_p->invoke_now(scope_p, caller_p)  // evaluate receiver expression
    : scope_p->get_topmost_scope();                // 'this' is inferred


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Is receiver good?
  #if (SKOOKUM & SK_DEBUG)
    const char * bad_rcvr_p = nullptr;

    // Ensure receiver is non-nullptr
    if (receiver_p == nullptr)
      {
      bad_rcvr_p = "nullptr";
      }
    else
      {
      // Ensure that the receiver is a closure object
      if (receiver_p->get_obj_type() != SkObjectType_closure)
        {
        bad_rcvr_p = "not a closure object as expected";

        // Clean-up receiver created by expression
        if (m_receiver_p)
          {
          receiver_p->dereference();
          }
        }
      }

    if (bad_rcvr_p)
      {
      // $Note - CReis This should not need to be checked here at runtime - the parser
      // should have already prevented any misuse.  However an invalid type cast can be
      // used to fool the parser.
      SK_ERROR_INFO(
        a_str_format(
          "Tried to invoke/call a closure but it was %s!\n"
          "[Returning nil rather than calling a closure.]",
          bad_rcvr_p),
        caller_p);

      // Attempt simple recovery by returning nil rather than just crashing
      if (result_pp)
        {
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return nullptr;
      }
  #endif // (SKOOKUM & SK_DEBUG)


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Invoke the closure
  SkInvokedBase * invoked_p = receiver_p->invoke(scope_p, caller_p, result_pp, m_invoke_info, this);

  // Clean-up receiver created by expression
  if (m_receiver_p)
    {
    receiver_p->dereference();
    }

  return invoked_p;
  }

