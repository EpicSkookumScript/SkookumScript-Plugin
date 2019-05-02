// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Invocation classes: SkInvokeBase, SkInvocation, SkInvokeSync, SkInvokeRace,
// SkInvokeCascade, SkInstantiate and SkCopyInvoke
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvocation.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvocation.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkObjectBase.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>
#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// SkInvokeBase Class Data Members
//=======================================================================================


//=======================================================================================
// SkInvokeBase Method Definitions
//=======================================================================================

SkInvokeBase::SkInvokeBase(const SkInvokableBase * invokable_p, SkClass * scope_p /*= nullptr*/) 
  : SkQualifier(*invokable_p, scope_p, invokable_p->get_vtable_index())
  {
  //SK_ASSERTX(get_vtable_index() >= 0, "SkInvokeBase must have a valid vtable index!");
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual - Ensures proper destructor called
// Author(s):   Conan Reis
SkInvokeBase::~SkInvokeBase()
  {
  m_arguments.free_all();
  m_return_args.free_all();
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this call and its components and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length(), assign_binary(binary_pp)
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - invokable name id
//               2 bytes - vtable index
//               4 bytes - scope name id or ASymbol_id_null if not used
//               1 byte  - number of arguments
//               n bytes - argument typed binary }- Repeating 
//               1 byte  - number of return arguments
//               n bytes - return argument typed binary }- Repeating 
//
// Modifiers:   virtual from SkInvokeBase
// Author(s):   Conan Reis
void SkInvokeBase::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvokeBase::as_binary_length());

  // 10 bytes
  SkQualifier::as_binary(binary_pp);

  // $Note - See similar code in SkInvokeClosureBase::as_binary()

  uint8_t length = uint8_t(m_arguments.get_length());
  A_BYTE_STREAM_OUT8(binary_pp, &length);

  if (length)
    {
    // n bytes - argument typed binary }- Repeating 
    SkExpressionBase ** args_pp     = m_arguments.get_array();
    SkExpressionBase ** args_end_pp = args_pp + length;

    for (; args_pp < args_end_pp; args_pp++)
      {
      SkExpressionBase::as_binary_typed(*args_pp, binary_pp);
      }
    }


  // 1 byte - number of return args (limits to 255 return arguments)
  length = uint8_t(m_return_args.get_length());
  A_BYTE_STREAM_OUT8(binary_pp, &length);

  if (length)
    {
    // n bytes - return argument typed binary }- Repeating 
    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    for (; rargs_pp < rargs_end_pp; rargs_pp++)
      {
      SkExpressionBase::as_binary_typed(*rargs_pp, binary_pp);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - invokable name id
//               2 bytes - vtable index
//               4 bytes - scope name id or ASymbol_id_null if not used
//               1 byte  - number of arguments
//               n bytes - argument typed binary }- Repeating 
//               1 byte  - number of return arguments
//               n bytes - return argument typed binary }- Repeating 
// Modifiers:   virtual from SkInvokeBase
// Author(s):   Conan Reis
uint32_t SkInvokeBase::as_binary_length() const
  {
  uint32_t binary_length = SkQualifier::as_binary_length() + 2u; // SkQualifier(10) + arg num(1) + return arg num(1)

  // n bytes - argument typed binary }- Repeating
  uint32_t length = m_arguments.get_length();

  if (length)
    {
    SkExpressionBase ** args_pp       = m_arguments.get_array();
    SkExpressionBase ** args_end_pp   = args_pp + length;

    for (; args_pp < args_end_pp; args_pp++)
      {
      binary_length += SkExpressionBase::as_binary_typed_length(*args_pp);
      }
    }

  // n bytes - return argument typed binary - Repeating 
  length = m_return_args.get_length();

  if (length)
    {
    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    for (; rargs_pp < rargs_end_pp; rargs_pp++)
      {
      binary_length += SkExpressionBase::as_binary_typed_length(*rargs_pp);
      }
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Assignment from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - invokable name id
//               2 bytes - vtable index
//               4 bytes - scope name id or ASymbol_id_null if not used
//               1 byte  - number of arguments
//               n bytes - argument typed binary }- Repeating 
//               1 byte  - number of return arguments
//               n bytes - return argument typed binary }- Repeating 
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkInvokeBase::assign_binary(const void ** binary_pp)
  {
  SkQualifier::assign_binary(binary_pp);

  // 1 byte - number of arguments
  uint8_t length = A_BYTE_STREAM_UI8_INC(binary_pp);

  if (length)
    {
    m_arguments.set_size(length);

    SkExpressionBase ** args_pp     = m_arguments.get_array();
    SkExpressionBase ** args_end_pp = args_pp + length;

    // n bytes - argument typed binary }- Repeating 
    while (args_pp < args_end_pp)
      {
      *args_pp = SkExpressionBase::from_binary_typed_new(binary_pp);
      args_pp++;
      }
    }

  // 1 byte - number of return arguments
  length = A_BYTE_STREAM_UI8_INC(binary_pp);

  if (length)
    {
    m_return_args.set_size(length);

    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    // n bytes - return argument typed binary }- Repeating 
    while (rargs_pp < rargs_end_pp)
      {
      *rargs_pp = static_cast<SkIdentifierLocal *>(SkExpressionBase::from_binary_typed_new(binary_pp));
      rargs_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Creates and returns a call created dynamically based on the binary
//             which includes the call's type info.
// Returns:    a dynamically allocated call object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_typed() or a similar
//             mechanism.
// See:        as_binary_typed(), as_binary_typed_length(), as_binary(), as_binary_length()
// Notes:      Binary composition:
//               1 byte  - call type
//               n bytes - call binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkInvokeBase * SkInvokeBase::from_binary_typed_new(const void ** binary_pp)
  {
  // 1 byte - argument type
  eSkInvokeType type = static_cast<eSkInvokeType>(A_BYTE_STREAM_UI8_INC(binary_pp));

  // n bytes - call binary
  switch (type)
    {
    case SkInvokeType__invalid:                  return nullptr;
    case SkInvokeType_coroutine:                 return SK_NEW(SkCoroutineCall)(binary_pp);
    case SkInvokeType_method_on_instance:        return SK_NEW(SkMethodCallOnInstance)(binary_pp);
    case SkInvokeType_method_on_class:           return SK_NEW(SkMethodCallOnClass)(binary_pp);
    case SkInvokeType_method_on_instance_class:  return SK_NEW(SkMethodCallOnInstanceClass)(binary_pp);
    case SkInvokeType_method_on_class_instance:  return SK_NEW(SkMethodCallOnClassInstance)(binary_pp);
    case SkInvokeType_method_boolean_and:        return SK_NEW(SkMethodCallBooleanAnd)(binary_pp);
    case SkInvokeType_method_boolean_or:         return SK_NEW(SkMethodCallBooleanOr)(binary_pp);
    case SkInvokeType_method_boolean_nand:       return SK_NEW(SkMethodCallBooleanNand)(binary_pp);
    case SkInvokeType_method_boolean_nor:        return SK_NEW(SkMethodCallBooleanNor)(binary_pp);
    case SkInvokeType_method_assert:             return SK_NEW(SkMethodCallAssert)(binary_pp);
    case SkInvokeType_method_assert_no_leak:     return SK_NEW(SkMethodCallAssertNoLeak)(binary_pp);
    default:                                     SK_ASSERTX(false, "Unknown eSkInvokeType!");  return nullptr;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


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
AString SkInvokeBase::as_code() const
  {
  AString str(nullptr, 128u, 0u);

  if (m_scope_p)
    {
    str.append(m_scope_p->get_name_str_dbg());
    str.append('@');
    }

  str.append(m_name.as_str_dbg());
  str.append('(');


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Standard arguments
  SkExpressionBase ** args_pp     = m_arguments.get_array();
  SkExpressionBase ** args_end_pp = args_pp + m_arguments.get_length();

  while (args_pp < args_end_pp)
    {
    if (*args_pp)
      {
      // $Revisit - CReis Group args are stored as a list literal so they are printed with
      // curly braces {} surrounding the grouped arguments.  This helps to identify how
      // args are grouped though it does not make a string that would be correctly compiled.
      // In the future keep as list literal and use yet-to-be-defined expansion mechanism
      // on it so that it would compile.
      str.append((*args_pp)->as_code());
      }

    args_pp++;

    if (args_pp < args_end_pp)
      {
      str.append(", ", 2u);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return arguments
  uint32_t rarg_length = m_return_args.get_length();

  if (rarg_length)
    {
    str.append("; ", 2u);

    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + m_return_args.get_length();

    while (rargs_pp < rargs_end_pp)
      {
      if (*rargs_pp)
        {
        str.append((*rargs_pp)->as_code());
        }

      rargs_pp++;

      if (rargs_pp < rargs_end_pp)
        {
        str.append(", ", 2u);
        }
      }
    }

  str.append(')');

  return str;
  }

//---------------------------------------------------------------------------------------
// Converts it's name to a descriptive AgogCore/AString.
// Returns:    It's name as a AString - i.e. "ClassScope@_coroutine()" or "_coroutine()"
// Modifiers:   virtual (overriding pure method from SkInvokableBase)
// Author(s):   Conan Reis
AString SkInvokeBase::as_string_name() const
  {
  AString name;
  AString invoke_name(m_name.as_str_dbg());
  uint32_t    length = invoke_name.get_length() + 2u;

  if (m_scope_p == nullptr)
    {
    name.ensure_size_empty(length);
    }
  else
    {
    AString scope_name(m_scope_p->get_name_str_dbg());

    name.ensure_size_empty(length + scope_name.get_length() + 1u);
    name.append(scope_name);
    name.append('@');
    }

  name.append(invoke_name);
  name.append("()", 2u);

  return name;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if any argument expression was located at or follows the
//             character index position provided and returns it.
// Returns:    The first expression that starts at or follows the given position or nullptr
//             if no such expression was found.
// Arg         pos - code file/string character index position
// See:        ExpressionBase::find_expr_by_pos()
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkInvokeBase::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  // Test argument-expressions

  SkExpressionBase *  found_p;
  SkExpressionBase ** args_pp     = m_arguments.get_array();
  SkExpressionBase ** args_end_pp = args_pp + m_arguments.get_length();

  for (; args_pp < args_end_pp; args_pp++)
    {
    if (*args_pp)  // Could be default argument (nullptr)
      {
      found_p = (*args_pp)->find_expr_by_pos(pos, type);

      if (found_p)
        {
        return found_p;
        }
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
SkInvokeBase::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  SkExpressionBase ** expr_pp     = m_arguments.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_arguments.get_length();

  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    if (*expr_pp && (*expr_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Nulls out the specified argument expression if it is the first argument.
// 
// Params:
//   arg_p: argument to find and set to nullptr.
//   
// Notes:
//   Handy when this expression is to be deleted, but the first argument needs to be kept
//   - as can happen with a structure that is progressively built up during a parse, but
//   midway through a parsing error occurs.
void SkInvokeBase::null_arg1(SkExpressionBase * arg_p)
  {
  if (m_arguments.is_filled())
    {
    SkExpressionBase ** exprs_p = m_arguments.get_array();

    if (*exprs_p == arg_p)
      {
      *exprs_p = nullptr;
      }
    else
      {
      (*exprs_p)->null_receiver(arg_p);
      }
    }
  }


//=======================================================================================
// SkInvocation Class Data Members
//=======================================================================================


//=======================================================================================
// SkInvocation Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkInvocation::~SkInvocation()
  {
  delete m_receiver_p;
  delete m_call_p;
  }


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
//               n bytes - receiver expression typed binary or inferred this (nullptr)
//               n bytes - call typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkInvocation::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvocation::as_binary_length());

  SkExpressionBase::as_binary_typed(m_receiver_p, binary_pp);

  // n bytes - call
  m_call_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - receiver expression typed binary or inferred this (nullptr)
//               n bytes - call typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkInvocation::as_binary_length() const
  {
  return SkExpressionBase::as_binary_typed_length(m_receiver_p)
    + m_call_p->as_binary_typed_length();
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
AString SkInvocation::as_code() const
  {
  AString str;

  if (m_receiver_p)
    {
    str = m_receiver_p->as_code();
    str.append('.');
    }
  else
    {
    str.append("this.", 5u);
    }
  
  str.append(m_call_p->as_code());

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
SkExpressionBase * SkInvocation::find_expr_by_pos(
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
    return const_cast<SkInvocation *>(this);
    }

  return m_call_p->find_expr_by_pos(pos, type);
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
SkInvocation::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || (m_receiver_p && m_receiver_p->iterate_expressions(apply_expr_p, invokable_p))
    || m_call_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


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
SkInvokedBase * SkInvocation::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  if (m_receiver_p)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Evaluate receiver expression. Note that the receiver is always an expression that
    // returns immediately and is never a coroutine call expression.
    SkInstance * receiver_p = m_receiver_p->invoke_now(scope_p, caller_p);

    // Test for nullptr receiver
    if (receiver_p)
      {
      // Store expression debug info for next invoked method/coroutine.
      // Related SKDEBUG_HOOK_EXPR() called in invoked_call()
      SKDEBUG_ICALL_STORE_GEXPR(this);

      // Note that invoke_call() will properly increment the pending count as necessary.
      SkInvokedBase * invoked_p = m_call_p->invoke_call(receiver_p, scope_p, caller_p, result_pp);

      // Clean-up receiver created by expression
      receiver_p->dereference();

      return invoked_p;
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 'this' is inferred
    SkInstance * receiver_p = scope_p->get_topmost_scope();

    // Test for nullptr receiver
    if (receiver_p)
      {
      // Store expression debug info for next invoked method/coroutine.
      // Related SKDEBUG_HOOK_EXPR() called in invoked_call()
      SKDEBUG_ICALL_STORE_GEXPR(this);

      // Note that invoke_call() will properly increment the pending count as necessary.
      return m_call_p->invoke_call(receiver_p, scope_p, caller_p, result_pp);
      }
    }

  // $Revisit - CReis Should nil be substituted for receiver rather than skip invoke call?
  // invoke_now() should return nil if a programmer writes a method that returns nullptr, so
  // only if the scope is gone should receiver_p be nullptr.
  // Should probably be at least a debug runtime assert if receiver = nullptr
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
bool SkInvocation::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Methods execute and return within one frame and coroutines may take more than one frame.

  if (m_call_p->get_invoke_type() == SkInvokeType_coroutine)
    {
    #if (SKOOKUM & SK_DEBUG)
      if (durational_idx_p)
        {
        *durational_idx_p = m_source_idx;
        }
    #endif

    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Determine if this is a debug call
bool SkInvocation::is_debug_call() const
  {
  return m_receiver_p && m_receiver_p->is_debug_class();
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvocation::null_receiver(SkExpressionBase * receiver_p)
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
void SkInvocation::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkInvocation, SkDebugInfo_size_used));

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }
  
  m_call_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkInvokeSync Methods
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
AString SkInvokeSync::as_code() const
  {
  AString str;

  if (m_receiver_p)
    {
    str = m_receiver_p->as_code();

    str.append('%');
    }
  else
    {
    str.append("this%", 5u);
    }
  
  str.append(m_call_p->as_code());

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkInvokeSync::as_callstack_label() const
  {
  return AString("%", true);
  }

#endif // defined(SK_AS_STRINGS)


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
SkInvokedBase * SkInvokeSync::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Note that the receiver is always an expression that returns immediately and is never
  // a coroutine call expression.
  SkInstance * receiver_p = m_receiver_p->invoke_now(scope_p, caller_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If the receiver is nil - ignore the invocation.
  if ((receiver_p == nullptr) || (receiver_p == SkBrain::ms_nil_p))
    {
    // Ensure that at least nil is returned
    if (result_pp)
      {
      *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
      }

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Is receiver a non-list?
  SkInvokedBase * invoked_p = nullptr;

  if (!receiver_p->get_class()->is_class(*SkBrain::ms_list_class_p))
    {
    // Receiver is a normal non-nil single instance - call the invocation on it normally
    // Note that invoke_call() will properly increment the pending count as necessary.

    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    SkInvokedBase * deferred_p = m_call_p->invoke_call(receiver_p, scope_p, caller_p, result_pp);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Clean-up receiver created by expression
    receiver_p->dereference();

    return deferred_p;
    }
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // The receiver is a list.
  SkInstanceList & list = receiver_p->as<SkList>();
  uint32_t elem_count = list.get_length();

  if (result_pp)
    {
    // Lists return themselves as a result
    // $Revisit - CReis Could return list of results
    receiver_p->reference();
    *result_pp = receiver_p;
    }

  // If there are no elements, there is nothing else to do.
  if (elem_count == 0u)
    {
    receiver_p->dereference();

    return nullptr;
    }

  // Call invocation on each element.
  bool                  completed    = true;
  SkInvokedExpression * iexpr_p      = SkInvokedExpression::pool_new(*this, caller_p, scope_p);
  SkInstance **         elems_pp     = list.get_array();
  SkInstance **         elems_end_pp = elems_pp + elem_count;

  // $Revisit - CReis Could evaluate arguments once for all elements which would be more
  // efficient, but could lead to problems if the args are changed by the call.
  while (elems_pp < elems_end_pp)
    {
    // Note that the last to have its return args set will be the final values for the
    // return args.

    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    invoked_p = m_call_p->invoke_call(*elems_pp, scope_p, iexpr_p, nullptr);

    if (invoked_p)
      {
      iexpr_p->pending_deferred(invoked_p);
      
      completed = false;
      }

    elems_pp++;
    }

  // Shouldn't need receiver list anymore
  receiver_p->dereference();

  if (completed)
    {
    SkInvokedExpression::pool_delete(iexpr_p);

    return nullptr;
    }

  return iexpr_p;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeSync::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkInvokeSync, SkDebugInfo_size_used));

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }
  
  m_call_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkInvokeRace Methods
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
AString SkInvokeRace::as_code() const
  {
  AString str;

  if (m_receiver_p)
    {
    str = m_receiver_p->as_code();

    str.append("%>", 2u);
    }
  else
    {
    str.append("this%>", 5u);
    }
  
  str.append(m_call_p->as_code());

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkInvokeRace::as_callstack_label() const
  {
  return AString("%>", true);
  }

#endif // defined(SK_AS_STRINGS)


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
SkInvokedBase * SkInvokeRace::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Note that the receiver is always an expression that returns immediately and is never
  // a coroutine call expression.
  SkInstance * receiver_p = m_receiver_p->invoke_now(scope_p, caller_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If the receiver is nil - ignore the invocation.
  if ((receiver_p == nullptr) || (receiver_p == SkBrain::ms_nil_p))
    {
    // Ensure that at least nil is returned
    if (result_pp)
      {
      *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
      }

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Is receiver a non-list?
  SkInvokedBase * invoked_p = nullptr;

  if (!receiver_p->get_class()->is_class(*SkBrain::ms_list_class_p))
    {
    // Receiver is a normal non-nil single instance - call the invocation on it normally
    // Note that invoke_call() will properly increment the pending count as necessary.

    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    SkInvokedBase * deferred_p = m_call_p->invoke_call(receiver_p, scope_p, caller_p, result_pp);

    // Clean-up receiver created by expression
    receiver_p->dereference();

    return deferred_p;
    }
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // The receiver is a list.
  SkInstanceList & list = receiver_p->as<SkList>();
  uint32_t elem_count = list.get_length();

  if (result_pp)
    {
    // Lists return themselves as a result
    // $Revisit - CReis Could return list of results
    receiver_p->reference();
    *result_pp = receiver_p;
    }

  // If there are no elements, there is nothing else to do.
  if (elem_count == 0u)
    {
    receiver_p->dereference();

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call invocation on each element.
  bool                  completed    = false;
  SkInvokedExpression * iexpr_p      = SkInvokedExpression::pool_new(*this, caller_p, scope_p);
  SkInstance **         elems_pp     = list.get_array();
  SkInstance **         elems_end_pp = elems_pp + elem_count;

  // $Revisit - CReis Could evaluate arguments once for all elements which would be more
  // efficient, but could lead to problems if the args are changed by the call.
  while (elems_pp < elems_end_pp)
    {
    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Note that the last to have its return args set will be the final values for the
    // return args.
    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    invoked_p = m_call_p->invoke_call(*elems_pp, scope_p, iexpr_p, nullptr);

    if (invoked_p == nullptr)
      {
      // A racer completed immediately, so race is over.
      completed = true;

      break;
      }

    iexpr_p->pending_deferred(invoked_p);
    elems_pp++;
    }

  // Shouldn't need receiver list anymore
  receiver_p->dereference();

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
  iexpr_p->pending_set(1u);

  return iexpr_p;
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
bool SkInvokeRace::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  uint32_t racers_count = uint32_t(iexpr_p->m_data);

  if (racers_count > 0u)
    {
    // Race won - continue

    // Set pending to include remaining racers - this ensures that other things other than
    // sub-calls are taken into account.
    iexpr_p->pending_set(iexpr_p->pending_count() + racers_count - 1u);
    iexpr_p->m_data = 0u;
    iexpr_p->abort_subcalls(SkNotify_success);

    SkInvokedExpression::pool_delete(iexpr_p);
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeRace::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkInvokeRace, SkDebugInfo_size_used));

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }
  
  m_call_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkInvokeCascade Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkInvokeCascade::~SkInvokeCascade()
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
//               n bytes - receiver expression or inferred this (nullptr)
//               1 byte  - number of calls
//               n bytes - call typed binary }- repeating
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkInvokeCascade::SkInvokeCascade(const void ** binary_pp)
  {
  // n bytes - receiver expression or inferred this (nullptr)
  m_receiver_p = SkExpressionBase::from_binary_typed_new(binary_pp);

  // 1 byte - number of calls
  uint32_t length = A_BYTE_STREAM_UI8_INC(binary_pp);

  // n bytes - call typed binary }- Repeating 
  m_invoke_calls.ensure_size_empty(length);

  for (; length > 0u ; length--)
    {
    m_invoke_calls.append(*SkInvokeBase::from_binary_typed_new(binary_pp));
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
//               n bytes - receiver expression or inferred this (nullptr)
//               1 byte  - number of calls
//               n bytes - call typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeCascade::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvokeCascade::as_binary_length());

  // n bytes - receiver expression or inferred this (nullptr)
  SkExpressionBase::as_binary_typed(m_receiver_p, binary_pp);


  // 1 byte - number of calls
  uint32_t length = m_invoke_calls.get_length();

  **(uint8_t **)binary_pp = uint8_t(length);
  (*(uint8_t **)binary_pp)++;


  // 4 bytes - call }- Repeating 
  SkInvokeBase ** calls_pp     = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp = calls_pp + length;

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    (*calls_pp)->as_binary_typed(binary_pp);
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
//               1 byte  - number of calls
//               n bytes - call typed binary }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkInvokeCascade::as_binary_length() const
  {
  uint32_t binary_length = 1u + SkExpressionBase::as_binary_typed_length(m_receiver_p);

  // 4 bytes - call }- Repeating 
  SkInvokeBase ** calls_pp      = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp  = calls_pp + m_invoke_calls.get_length();

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    binary_length += (*calls_pp)->as_binary_typed_length();
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
AString SkInvokeCascade::as_code() const
  {
  AString str(m_receiver_p ? m_receiver_p->as_code() : AString("this", 4u), 128u);
  
  SkInvokeBase ** calls_pp     = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp = calls_pp + m_invoke_calls.get_length();

  str.append(".\n", 2u);
  str.append(' ', SkDebug::ms_indent_size);
  str.append("[\n", 2u);

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    str.append(' ', SkDebug::ms_indent_size);
    str.append((*calls_pp)->as_code());
    str.append('\n');
    }

  str.append(' ', SkDebug::ms_indent_size);
  str.append(']');

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkInvokeCascade::as_callstack_label() const
  {
  return AString(".[]", true);
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
SkExpressionBase * SkInvokeCascade::find_expr_by_pos(
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
    return const_cast<SkInvokeCascade *>(this);
    }

  SkInvokeBase ** calls_pp     = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp = calls_pp + m_invoke_calls.get_length();

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    expr_p = (*calls_pp)->find_expr_by_pos(pos, type);

    if (expr_p)
      {
      return expr_p;
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
SkInvokeCascade::iterate_expressions(
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

  SkInvokeBase ** calls_pp     = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp = calls_pp + m_invoke_calls.get_length();

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    if ((*calls_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_invoke_cascade
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkInvokeCascade::get_type() const
  {
  return SkExprType_invoke_cascade;
  }

//---------------------------------------------------------------------------------------
// Cleans up invoked expression wrapper
//
// #Modifiers static
// #Author(s) Conan Reis
inline void SkInvokeCascade::clean_up(SkInvokedExpression * iexpr_p)
  {
  // Clean-up receiver
  SkInstance * receiver_p = reinterpret_cast<SkInstance *>(iexpr_p->m_data);

  if (receiver_p)
    {
    // Clear stored pointer so nothing else tries to access it
    iexpr_p->m_data = 0u;
    receiver_p->dereference();
    }
  }

//---------------------------------------------------------------------------------------
// Iterate through cascaded calls
// Returns:    true if the invoke cascade has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         iexpr_p - the invoked expression wrapped around this invoke cascade
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Notes:      Called by invoke() and SkInvokedExpression::pending_return()
// Author(s):   Conan Reis
bool SkInvokeCascade::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  bool initial_call = (iexpr_p->m_index == 0u);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through all but last call
  SkInvokedBase * invoked_p  = nullptr;
  SkInstance *    receiver_p = reinterpret_cast<SkInstance *>(iexpr_p->m_data);
  SkObjectBase *  scope_p    = iexpr_p->get_scope();
  uint32_t        length     = m_invoke_calls.get_length() - 1u;  // Treat the last statement differently since it is the only statement where its return value may be kept.
  SkInvokeBase ** calls_pp   = m_invoke_calls.get_array();

  while (iexpr_p->m_index < length)
    {
    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    // The expressions prior to the last call do not care about returned objects.
    invoked_p = (calls_pp[iexpr_p->m_index])->invoke_call(receiver_p, scope_p, iexpr_p, nullptr);

    if (invoked_p)
      {
      // Break out of while loop
      break;
      }

    iexpr_p->m_index++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // When the last call is reached, invoke it with 'result_pp'
  // Note that invoke_call() will properly increment the pending count as necessary.
  bool last_call = iexpr_p->m_index == length;

  if (last_call)
    {
    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    invoked_p = (calls_pp[iexpr_p->m_index])->invoke_call(receiver_p, scope_p, iexpr_p, result_pp);
    }

  if (invoked_p)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // This invocation cascade was durational and did not complete
    if (result_pp && !last_call)
      {
      // Ensure that invoked object returned
      *result_pp = invoked_p->as_new_instance();
      }

    // This invocation cascade did not complete
    iexpr_p->pending_deferred(invoked_p);

    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // This invocation cascade completed
  SkInvokedBase * caller_p = iexpr_p->get_caller();

  clean_up(iexpr_p);

  // Did not complete immediately
  // Return to caller if one is waiting
  if (!initial_call && caller_p && caller_p->pending_count())
    {
    caller_p->pending_unregister(iexpr_p);
    }

  // Free up invoked expression wrapper
  SkInvokedExpression::pool_delete(iexpr_p);

  return true;
  }

//---------------------------------------------------------------------------------------
// Evaluates invocation cascade expression and returns the resulting instance
//             if desired.
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
SkInvokedBase * SkInvokeCascade::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SkInvokedExpression * iexpr_p = SkInvokedExpression::pool_new(*this, caller_p, scope_p);

  SkInstance * receiver_p;

  if (m_receiver_p)
    {
    // Evaluate receiver expression - always returns immediately.
    receiver_p = m_receiver_p->invoke_now(scope_p, caller_p);
    }
  else
    {
    // 'this' is inferred
    receiver_p = scope_p->get_topmost_scope();
    receiver_p->reference();
    }

  // Store receiver for future use in invoked expression's m_data.
  // Receiver will be decremented when invoked expression is completed.
  // $Revisit - CReis Should make this a smart pointer
  iexpr_p->m_data = reinterpret_cast<uintptr_t>(receiver_p);

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through statements
  if (invoke_iterate(iexpr_p, result_pp))
    {
    // Completed immediately
    // iexpr_p is cleaned up in invoke_iterate()
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Completion was durational
  // $Vital - CReis If durational call is possible update the parser result type to be
  // <InvokedCoroutine|LastType>

  return iexpr_p;
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
bool SkInvokeCascade::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Methods execute and return within one frame and coroutines may take more than one
  // frame - if the calls are anything other than methods assume that they may take more
  // than one frame.
  SkInvokeBase ** calls_pp     = m_invoke_calls.get_array();
  SkInvokeBase ** calls_end_pp = calls_pp + m_invoke_calls.get_length();

  for (; calls_pp < calls_end_pp; calls_pp++)
    {
    if ((*calls_pp)->get_invoke_type() == SkInvokeType_coroutine)
      {
      // Found sub-call that is not a method so it may execute and return in more than
      // one frame.

      #if (SKOOKUM & SK_DEBUG)
        if (durational_idx_p)
          {
          *durational_idx_p = m_source_idx;
          }
      #endif

      return false;
      }
    }

  // All sub-calls are methods so it will execute and return in one frame.
  return true;
  }

//---------------------------------------------------------------------------------------
// Exits / unwinds invoked cascade
//
// #Modifiers virtual - overridden from SkExpressionBase
// #Author(s) Conan Reis
void SkInvokeCascade::invoke_exit(
  // invoked expression wrapped around this expression
  SkInvokedExpression * iexpr_p,
  // Active sub-expression requesting the exit or nullptr if not during active sub-expression.
  SkInvokedExpression * sub_exit_p // = nullptr
  ) const
  {
  // Clean-up receiver
  //clean_up(iexpr_p);

  // Jump to end of cascade
  iexpr_p->m_index = m_invoke_calls.get_length();
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInvokeCascade::null_receiver(SkExpressionBase * receiver_p)
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
void SkInvokeCascade::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkInvokeCascade, SkDebugInfo_size_used),
    m_invoke_calls.get_length() * sizeof(void *),
    m_invoke_calls.track_memory(mem_stats_p));

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkInstantiate Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkInstantiate::~SkInstantiate()
  {
  delete m_ctor_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkInstantiate::SkInstantiate(const void ** binary_pp)
  {
  // 4 bytes - class reference
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // n bytes - constructor method call binary
  m_ctor_p = SK_NEW(SkMethodCallOnInstance)(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this instantiate expression and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkInstantiate::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInstantiate::as_binary_length());

  // 4 bytes - class reference
  m_class_p->as_binary_ref(binary_pp);

  // n bytes - constructor method call binary
  m_ctor_p->as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkInstantiate::as_binary_length() const
  {
  return SkClass::Binary_ref_size + m_ctor_p->as_binary_length();
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
AString SkInstantiate::as_code() const
  {
  AString str(m_class_p->get_name_str_dbg());

  str.append(m_ctor_p->as_code());

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
SkExpressionBase * SkInstantiate::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  // Return for all find types
  return ((m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkInstantiate *>(this)
    : m_ctor_p->find_expr_by_pos(pos, type);
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
SkInstantiate::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_ctor_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_instantiate
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkInstantiate::get_type() const
  {
  return SkExprType_instantiate;
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
SkInvokedBase * SkInstantiate::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Removed double invocation here
  //SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, SkDebug::HookContext_current);

  // Create new instance
  SkInstance * receiver_p = m_class_p->new_instance();

  // Debugging - store current expression in global rather than passing as argument since
  // only used for debug.
  SKDEBUG_ICALL_STORE_GEXPR(this);

  // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
  // Call constructor method - which will always return immediately
  m_ctor_p->invoke_call(receiver_p, scope_p, caller_p, nullptr);

  // The result of the constructor is ignored and the new instance is used as a result.
  if (result_pp)
    {
    *result_pp = receiver_p;
    }
  else
    {
    // The new object could be referenced on its creation/constructor, otherwise this will
    // destruct it immediately.
    receiver_p->dereference();
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// 
// #Params:
//   receiver_p: sub receiver to find and set to nullptr.
//   
// #Notes:
//   Handy when this expression is to be deleted, but the receiver needs to be kept - as
//   can happen with a structure that is progressively built up during a parse, but
//   midway through a parsing error occurs.
//   
// #Modifiers: virtual overridden from SkExpressionBase
// #Authors:   Conan Reis
void SkInstantiate::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_ctor_p)
    {
    // Since the receiver is could be used as the first argument.
    m_ctor_p->null_arg1(receiver_p);
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkInstantiate::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkInstantiate, SkDebugInfo_size_used));

  m_ctor_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkCopyInvoke Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkCopyInvoke::~SkCopyInvoke()
  {
  delete m_ctor_p;
  delete m_method_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
//               n bytes - initial method call binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkCopyInvoke::SkCopyInvoke(const void ** binary_pp)
  {
  // 4 bytes - class reference
  m_class_p = SkClass::from_binary_ref(binary_pp);

  // n bytes - constructor method call binary
  m_ctor_p = SK_NEW(SkMethodCallOnInstance)(binary_pp);

  // n bytes - initial method call binary
  m_method_p = SK_NEW(SkMethodCallOnInstance)(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this instantiate expression and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
//               n bytes - initial method call binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkCopyInvoke::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCopyInvoke::as_binary_length());

  // 4 bytes - class reference
  m_class_p->as_binary_ref(binary_pp);

  // n bytes - constructor method call binary
  m_ctor_p->as_binary(binary_pp);

  // n bytes - initial method call binary
  m_method_p->as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - class reference
//               n bytes - constructor method call binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkCopyInvoke::as_binary_length() const
  {
  return SkClass::Binary_ref_size + m_ctor_p->as_binary_length() + m_method_p->as_binary_length();
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
AString SkCopyInvoke::as_code() const
  {
  AString ctor_str(m_ctor_p->as_code());
  AString method_str(m_method_p->as_code());
  AString str(
    m_class_p->get_name_str_dbg(),
    ctor_str.get_length() + method_str.get_length() + 1u);

  str.append(ctor_str);
  str.append('.');
  str.append(method_str);

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
SkExpressionBase * SkCopyInvoke::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_ctor_p->find_expr_by_pos(pos, type);

  if (expr_p)
    {
    return expr_p;
    }

  // Return for all find types
  if ((m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkCopyInvoke *>(this);
    }

  return m_method_p->find_expr_by_pos(pos, type);
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
SkCopyInvoke::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || m_ctor_p->iterate_expressions(apply_expr_p, invokable_p)
    || m_method_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_instantiate
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkCopyInvoke::get_type() const
  {
  return SkExprType_copy_invoke;
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
SkInvokedBase * SkCopyInvoke::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // Create new instance
  SkInstance * receiver_p = m_class_p->new_instance();

  // Debugging - store current expression in global rather than passing as argument since
  // only used for debug.
  SKDEBUG_ICALL_STORE_GEXPR(this);

  // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
  // Call copy constructor method - which will always return immediately
  m_ctor_p->invoke_call(receiver_p, scope_p, caller_p, nullptr);

  // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
  // Call initial method - which will always return immediately
  m_method_p->invoke_call(receiver_p, scope_p, caller_p, nullptr);

  // The result of the constructor is ignored and the new instance is used as a result.
  if (result_pp)
    {
    *result_pp = receiver_p;
    }
  else
    {
    // The new object could be referenced on its creation/constructor, otherwise this will
    // destruct it immediately.
    receiver_p->dereference();
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// 
// Params:
//   receiver_p: sub receiver to find and set to nullptr.
//   
// Notes:
//   Handy when this expression is to be deleted, but the receiver needs to be kept - as
//   can happen with a structure that is progressively built up during a parse, but
//   midway through a parsing error occurs.
//   
// Modifiers: virtual overridden from SkExpressionBase
// Author(s): Conan Reis
void SkCopyInvoke::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_ctor_p)
    {
    // Since the receiver is used as the first argument.
    m_ctor_p->null_arg1(receiver_p);
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCopyInvoke::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkCopyInvoke, SkDebugInfo_size_used));

  m_ctor_p->track_memory(mem_stats_p);
  m_method_p->track_memory(mem_stats_p);
  }

