// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Classes for expressions that can be evaluated/invoked
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkExpressionBase.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkExpressionBase.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkConditional.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkInvocation.hpp>
#include <SkookumScript/SkInvokeClosure.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkLiteral.hpp>
#include <SkookumScript/SkLiteralClosure.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkObjectID.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkRawMember.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>


//=======================================================================================
// Local Macros / Defines
//=======================================================================================

// Enumerated constants
enum
  {
  // These flags specify the current loop state of a SkLoop expression and are stored in
  // the 'm_data' data member of the SkInvokedExpression object that is wrapped around it.

  SkLoop_looping    = 1<<0,  // Flag indicating that loop is still looping
  SkLoop_durational = 1<<1   // Flag indicating that the loop is not immediate - i.e. it has waited waited for sub expressions and has taken more than one frame to complete
  };


//=======================================================================================
// SkExpressionBase Class Data Members
//=======================================================================================

#if (SKOOKUM & SK_DEBUG)
  uint16_t SkExpressionBase::ms_new_expr_debug_info = SkDebugInfo::Flag__default;
#endif


//=======================================================================================
// SkExpressionBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Virtual destructor - ensures calling of proper destructor for subclasses.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase::~SkExpressionBase()
  {
  }

//---------------------------------------------------------------------------------------
// Makes a new dynamic copy of the expression.
//
// #Modifiers virtual
// See Also  SkUnaryParam: :as_finalized_generic()
// #Author(s) Conan Reis
SkExpressionBase * SkExpressionBase::as_copy() const
  {
  #if ((SKOOKUM & SK_COMPILED_OUT) && (SKOOKUM & SK_COMPILED_IN))
    // $HACK - CReis This is a crazy hack until proper as_copy() methods are written.
    // $Revisit - CReis [A_NOTE] ***HACK*** - Using binary serialization to copy expression!


    // Stream out expression to binary
    void * binary_p = AgogCore::get_app_info()->malloc(as_binary_length(), "SkExpressionBase::as_copy()");
    void * stream_p = binary_p;

    as_binary(&stream_p);


    // Recreate expression from binary stream
    stream_p = binary_p;
    SkExpressionBase * expr_p = from_binary_new(get_type(), (const void **)&stream_p);

    AgogCore::get_app_info()->free(binary_p);

    return expr_p;
  #else
    // Assert for now
    SK_ERRORX("Tried to call as_copy() on expression though it is not yet implemented!");

    return nullptr;
  #endif
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
bool SkExpressionBase::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Assume expressions are immediate by default.
  return true;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkExpressionBase::null_receiver(SkExpressionBase * receiver_p)
  {
  // Default behaviour do nothing - not all expressions have sub-receiver expressions.
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Use this instance method instead of the class method when the expression
//             being converted will not be nullptr.  Fills memory pointed to by binary_pp 
//             with the information needed to recreate an expression (without knowing
//             what type of expression it is) and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_typed_length(), from_binary_typed_new(), as_binary(),
//             as_binary_length()
// Notes:      Used in combination with as_binary_typed_length() and from_binary_new()
//
//             Binary composition:
//               1 byte  - expression type
//              [2 bytes - *SkDebug binary* expression source character position]
//               n bytes - expression binary
//
// Author(s):   Conan Reis
void SkExpressionBase::as_binary_typed(void ** binary_pp) const
  {
  // $Note - CReis A modified version of this code is in SkUnaryParam::as_binary()
  // - any changes here may need to be reflected there as well.

  // 1 byte - argument type
  **(uint8_t **)binary_pp = static_cast<uint8_t>(get_type());
  (*(uint8_t **)binary_pp)++;


  // 2 bytes - *ADebug* expression source character position]
  #if (SKOOKUM & SK_DEBUG)
    A_BYTE_STREAM_OUT16(binary_pp, &m_source_idx);
  #else
    // No debug info available so use 0 for index
    uint16_t source_idx = 0u;
    A_BYTE_STREAM_OUT16(binary_pp, &source_idx);
  #endif

  // n bytes - expression binary
  as_binary(binary_pp);
  }

#endif //(SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Creates and returns an expression created dynamically based on the binary
//             which does not include the expression type info.
// Returns:    a dynamically allocated expression object
// Arg         expr_type - type of expression stored in binary form
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_typed() or a similar
//             mechanism.
// See:        as_binary_typed(), as_binary_typed_length(), as_binary(), as_binary_length()
// Notes:      Binary composition:
//               1 byte  - expression type
//               n bytes - expression binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkExpressionBase::from_binary_new(
  eSkExprType expr_type,
  const void ** binary_pp
  )
  {
  switch (expr_type)
    {
    // SkIdentifierLocal - ident
    case SkExprType_identifier_local:
      return SK_NEW(SkIdentifierLocal)(binary_pp);

    // SkIdentifierMember - expr.@ident
    case SkExprType_identifier_member:
      return SK_NEW(SkIdentifierMember)(binary_pp);

    // SkIdentifierRawMember - expr.@ident
    case SkExprType_identifier_raw_member:
      return SK_NEW(SkIdentifierRawMember)(binary_pp);

    // SkIdentifierClassMember - Class.@@ident
    case SkExprType_identifier_class_member:
      return SK_NEW(SkIdentifierClassMember)(binary_pp);

    // SkRawMemberAssignment - expr.@ident := value
    case SkExprType_raw_member_assignment:
      return SK_NEW(SkRawMemberAssignment)(binary_pp);

    // SkRawMemberAssignment - expr.@ident.negate()
    case SkExprType_raw_member_invocation:
      return SK_NEW(SkRawMemberModifyingInvocation)(binary_pp);

    // SkObjectID - MyClass@'some_name'
    case SkExprType_object_id:
      return SK_NEW(SkObjectID)(binary_pp);

    // SkLiteral
    case SkExprType_literal:
      return SK_NEW(SkLiteral)(binary_pp);

    // SkLiteralList - {...}
    case SkExprType_literal_list:
      return SK_NEW(SkLiteralList)(binary_pp);

    // SkLiteralClosure
    case SkExprType_closure_method:
    case SkExprType_closure_coroutine:
      return SK_NEW(SkLiteralClosure)(binary_pp, expr_type);

    // SkBind - variable initial bind expression.  Could be a member variable, a parameter variable, or a temporary variable.
    case SkExprType_bind:
      return SK_NEW(SkBind)(binary_pp);

    // SkCast - expr<>Class
    case SkExprType_cast:
      return SK_NEW(SkCast)(binary_pp);

    // SkConversion - expr>>Class
    case SkExprType_conversion:
      return SK_NEW(SkConversion)(binary_pp);

    // SkCode - usually nested  [...]
    case SkExprType_code:
      return SK_NEW(SkCode)(binary_pp);

    // SkConditional - if bool1 [clause1] bool2 [clause2] else [else_clause]
    case SkExprType_conditional:
      return SK_NEW(SkConditional)(binary_pp);

    // SkCase (switch statement) - case compare test1 [clause1] test2 [clause2] else [else_clause]
    case SkExprType_case:
      return SK_NEW(SkCase)(binary_pp);

    // SkWhen - clause when test
    case SkExprType_when:
      return SK_NEW(SkWhen)(binary_pp);

    // SkUnless - clause unless test
    case SkExprType_unless:
      return SK_NEW(SkUnless)(binary_pp);

    // SkLoop - loop [ if loop_test() [exit] loop_code() ]
    case SkExprType_loop:
      return SK_NEW(SkLoop)(binary_pp);

    // SkLoopExit - (valid only in the scope of the loop it references)
    case SkExprType_loop_exit:
      return SK_NEW(SkLoopExit)(binary_pp);

    // SkInvocation - receiver.call()
    case SkExprType_invoke:
      return SK_NEW(SkInvocation)(binary_pp);

    // SkInvokeSync - receiver%call()
    case SkExprType_invoke_sync:
      return SK_NEW(SkInvokeSync)(binary_pp);

    // SkInvokeRace - receiver%>call()
    case SkExprType_invoke_race:
      return SK_NEW(SkInvokeRace)(binary_pp);

    // SkInvokeCascade - receiver.[call1() call2() call3()]
    case SkExprType_invoke_cascade:
      return SK_NEW(SkInvokeCascade)(binary_pp);

    // SkInvokeClosureMethod - ^[do_this() do_that()]
    case SkExprType_invoke_closure_method:
      return SK_NEW(SkInvokeClosureMethod)(binary_pp);

    // SkInvokeClosureCoroutine - ^[_do_this() _do_that()]
    case SkExprType_invoke_closure_coroutine:
      return SK_NEW(SkInvokeClosureCoroutine)(binary_pp);

    // SkInstantiate - Instantiates / allocates / creates an object and initializes its
    // data members to nil - called just prior to a constructor - Class!ctor()
    case SkExprType_instantiate:
      return SK_NEW(SkInstantiate)(binary_pp);

    // SkCopyInvoke - Instantiates object via !copy() and calls initial method on it
    // - expr!method() -> ExprType!copy(expr).method() or ExprType!copy(expr).[method() self()]
    case SkExprType_copy_invoke:
      return SK_NEW(SkCopyInvoke)(binary_pp);

    // SkConcurrentSync - concurrent convergent expressions - sync [_expr1() _expr2() _expr3()]
    case SkExprType_concurrent_sync:
      return SK_NEW(SkConcurrentSync)(binary_pp);

    // SkConcurrentRace - concurrent racing expressions - race [_expr1() _expr2() _expr3()]
    case SkExprType_concurrent_race:
      return SK_NEW(SkConcurrentRace)(binary_pp);

    // SkConcurrentBranch - concurrent branched expression - branch [_expr()]
    case SkExprType_concurrent_branch:
      return SK_NEW(SkConcurrentBranch)(binary_pp);

    // SkChangeMind - change mind - change [ws expression] ws expression
    case SkExprType_change:
      return SK_NEW(SkChangeMind)(binary_pp);

    // SkNilCoalescing - expr1??expr2
    case SkExprType_nil_coalescing:
      return SK_NEW(SkNilCoalescing)(binary_pp);

    //case SkExprType__default:
    default:
      return nullptr;
    }
  }

//---------------------------------------------------------------------------------------
// Creates and returns an expression created dynamically based on the binary
//             which includes the expression's type info.
// Returns:    a dynamically allocated expression object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_typed() or a similar
//             mechanism.
// See:        as_binary_typed(), as_binary_typed_length(), as_binary(), as_binary_length()
// Notes:      Binary composition:
//               1 byte  - expression type - if default expression (i.e. nullptr) there are no futher bytes
//              [2 bytes - *SkDebug binary* expression source character position]
//               n bytes - expression binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkExpressionBase::from_binary_typed_new(const void ** binary_pp)
  {
  // 1 byte - argument type
  eSkExprType type = static_cast<eSkExprType>(A_BYTE_STREAM_UI8_INC(binary_pp));

  if (type == SkExprType__default)
    {
    return nullptr;
    }

  // SK_ASSERTX(type < SkExprType__max, "Invalid expression type");

  // $Revisit - CReis Currently the compiled code binary always includes debug info

  // 2 bytes - *ADebug* expression source character position
  #if (SKOOKUM & SK_DEBUG)
    uint16_t char_pos =
  #endif
      A_BYTE_STREAM_UI16_INC(binary_pp);

  // n bytes - expression binary
  SkExpressionBase * expr_p = from_binary_new(type, binary_pp);

  #if (SKOOKUM & SK_DEBUG)
    expr_p->m_source_idx = char_pos;
  #endif

  return expr_p;
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent and puts
//             a code block around it if it is not already a code block - [ expr ]
// Returns:    Source code string version of itself in / as a code block
// See:        as_code(), as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Author(s):   Conan Reis
AString SkExpressionBase::as_code_block() const
  {
  if (get_type() == SkExprType_code)
    {
    return as_code();
    }
  else
    {
    AString expr_str(as_code());
    AString str(nullptr, 64u + expr_str.get_length(), 0u);

    // Ensure that it is surrounded by a code block
    str.append("[\n", 2u);
    str.append(expr_str);
    str.append("\n]", 2u);
    str.line_indent(SkDebug::ms_indent_size);

    return str;
    }
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkExpressionBase::as_callstack_label() const
  {
  return AString("???", true);
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Finds last (by index order) expression in this expression - either itself
//             or a sub-expression.
// Author(s):   Conan Reis
SkExpressionBase * SkExpressionBase::find_expr_by_pos_last() const
  {
  SkExpressionBase * last_expr_p;
  SkExpressionBase * expr_p = const_cast<SkExpressionBase *>(this);

  do 
    {
    last_expr_p = expr_p;
    expr_p = find_expr_by_pos(last_expr_p->m_source_idx + 1u);
    } while (expr_p);

  return last_expr_p;
  }

//---------------------------------------------------------------------------------------
// Finds within this expression the sub-expression (or this expression itself)
//             at/on the specified source index position
// Returns:    Expression at/on specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         type - type of expression to look for - see eSkExprFind
// See:        SkMemberInfo::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkExpressionBase::find_expr_on_pos(
  uint32_t    source_idx,
  eSkExprFind type  // = SkExprFind_all
  ) const
  {
  // *** Similar to SkInvokableBase::find_expr_on_pos()
  // - any changes here should be reflected there as well.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if source idx is in range of this expression or its sub-expressions
  SkExpressionBase * expr_p = find_expr_by_pos(source_idx, type);

  if (expr_p == nullptr)
    {
    return nullptr;
    }

  if (expr_p->m_source_idx != source_idx)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Index after desired expression start - find previous expression
    uint32_t           probe_idx   = source_idx;
    SkExpressionBase * next_expr_p = expr_p;

    while ((expr_p == next_expr_p) && (probe_idx > 0u))
      {
      probe_idx--;

      expr_p = find_expr_by_pos(probe_idx, type);
      }
    }

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Finds within this expression the index position span of the specified
//             sub-expression (or this expression itself)
// Arg         expr - sub-expression (or this expression itself)
// Arg         begin_idx_p - starting index of the expression (may be position of
//             sub-expression)
// Arg         end_idx_p - next index following expression (may be start of last
//             sub-expression or beginning of next non-sub-expression)
// See:        find_expr_span(), SkMemberInfo::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
void SkExpressionBase::get_expr_span(
  const SkExpressionBase & expr,
  uint32_t *                   begin_idx_p,  // = nullptr
  uint32_t *                   end_idx_p     // = nullptr
  ) const
  {
  // *** Similar to SkInvokableBase::get_expr_span()
  // - any changes here should be reflected there as well.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (begin_idx_p)
    {
    *begin_idx_p = expr.find_expr_by_pos_first()->m_source_idx;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (end_idx_p)
    {
    SkExpressionBase * last_expr_p    = expr.find_expr_by_pos_last();
    SkExpressionBase * next_non_sub_p = find_expr_by_pos(last_expr_p->m_source_idx + 1u);

    *end_idx_p = next_non_sub_p
      ? next_non_sub_p->m_source_idx - 1u
      : last_expr_p->m_source_idx;
    }
  }

//---------------------------------------------------------------------------------------
// Finds within this expression the sub-expression (or this expression itself)
//             at the specified source index position and optionally determines the
//             beginning and ending index span of the found expression.
// Returns:    Expression at specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         begin_idx_p - starting index of the expression (may be position of
//             sub-expression)
// Arg         end_idx_p - next index following expression (may be start of last
//             sub-expression or beginning of next non-sub-expression)
// See:        SkMemberInfo::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkExpressionBase::find_expr_span(
  uint32_t    source_idx,
  uint32_t *      begin_idx_p,  // = nullptr
  uint32_t *      end_idx_p,    // = nullptr
  eSkExprFind type          // = SkExprFind_all
  ) const
  {
  // *** Similar to SkInvokableBase::find_expr_span()
  // - any changes here should be reflected there as well.

  SkExpressionBase * expr_p = find_expr_on_pos(source_idx, type);

  get_expr_span(*expr_p, begin_idx_p, end_idx_p);

  return expr_p;
  }

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
SkExpressionBase * SkExpressionBase::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  // This default find_expr_by_pos() is currently used by SkIdentifierLocal, SkObjectID,
  // SkLiteral and SkLoopExit - all the other expressions have their own custom version of
  // this method.
  return ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkExpressionBase *>(this)
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Gets the current source character index or ADef_uint32 if it cannot be
//             determined at runtime.
// Returns:    current source character index or ADef_uint32
// Arg         iexpr - invoked expression wrapped around this expression
// Modifiers:   virtual
// Author(s):   Conan Reis
SkDebugInfo SkExpressionBase::get_debug_info(const SkInvokedExpression & iexpr) const
  {
  SkDebugInfo debug_info = {m_source_idx, m_debug_info};

  return debug_info;
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
SkExpressionBase::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  // Default behaviour for any expression that has no sub-expressions
  return apply_expr_p->apply_expr(this, invokable_p);
  }

#endif // (SKOOKUM & SK_DEBUG)


//=======================================================================================
// SkLoop Class Data Members
//=======================================================================================


//=======================================================================================
// SkLoop Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkLoop::~SkLoop()
  {
  delete m_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this loop expression and increments the memory address to just
//             past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - loop name id
//               n bytes - expression typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkLoop::as_binary(void ** binary_pp) const
  {
  // 4 bytes - loop name id
  m_name.as_binary(binary_pp);

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
//               4 bytes - loop name id
//               n bytes - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkLoop::as_binary_length() const
  {
  return 4u + m_expr_p->as_binary_typed_length();
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
AString SkLoop::as_code() const
  {
  AString str("loop", 4u);
  AString expr_str(m_expr_p->as_code_block());
  AString loop_name(m_name.as_str_dbg());
  uint    length = loop_name.get_length();

  str.ensure_size(16u + length + expr_str.get_length());

  // Add name if it has one
  if (length)
    {
    str.append(' ');
    str.append(loop_name);
    }

  str.append('\n');
  str.append(expr_str);

  return str;
  }

//---------------------------------------------------------------------------------------
// Returns a label for display on callstack for invoked expressions
AString SkLoop::as_callstack_label() const
  {
  return m_name.is_null() ? AString("loop", true) : AString(5 + m_name.get_str_length(), "loop %s", m_name.as_cstr());
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
SkExpressionBase * SkLoop::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  // Stopping at the start of a loop is useful since you may want to only break once at
  // the start of a loop.
  return ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkLoop *>(this)
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
SkLoop::iterate_expressions(
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
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_loop
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkLoop::get_type() const
  {
  return SkExprType_loop;
  }

//---------------------------------------------------------------------------------------
// Loop statement until exited
// Returns:    true if the expression has completed its evaluation and there is a
//             resulting instance, false if there is a result pending.
// Arg         iexpr_p - the invoked expression wrapped around this loop
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Notes:      Called by invoke() and SkInvokedExpression::pending_return()
// Author(s):   Conan Reis
bool SkLoop::invoke_iterate(
  SkInvokedExpression * iexpr_p,
  SkInstance **         result_pp // = nullptr
  ) const
  {
  SkInvokedBase * invoked_p = nullptr;
  SkObjectBase *  scope_p   = iexpr_p->get_scope();

  while ((invoked_p == nullptr) && (iexpr_p->m_data & SkLoop_looping))
    {
    scope_p = iexpr_p->m_scope_p;

    if (scope_p)
      {
      invoked_p = m_expr_p->invoke(scope_p, iexpr_p);
      }
    else
      {
      // Lost Scope
      iexpr_p->m_data &= ~SkLoop_looping;

      break;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SkInvokedBase * caller_p;

  if (invoked_p)
    {
    // This loop did not complete immediately
    iexpr_p->pending_deferred(invoked_p);

    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Completed execution - now cleanup
  caller_p = iexpr_p->get_caller();

  // If the loop was non immediate, return to caller if one is waiting
  if ((iexpr_p->m_data & SkLoop_durational) && (caller_p && caller_p->pending_count()))
    {
    caller_p->pending_unregister(iexpr_p);
    }

  // This loop completed, so free up invoked expression wrapper
  SkInvokedExpression::pool_delete(iexpr_p);

  return true;
  }

//---------------------------------------------------------------------------------------
// Evaluates loop expression and returns the nil instance if desired.
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
SkInvokedBase * SkLoop::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SkInvokedExpression * iloop_p = SkInvokedExpression::pool_new(*this, caller_p, scope_p);

  iloop_p->m_data = SkLoop_looping;

  // If a result is desired return nil
  if (result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }

  // Loop doesn't do much work itself though this gives a convenient location to place a
  // break point that will be hit only once for the loop rather than each iteration.
  SKDEBUG_HOOK_EXPR(this, scope_p, iloop_p, nullptr, SkDebug::HookContext_current);

  if (!invoke_iterate(iloop_p))
    {
    // Increment the pending count of the caller - but only do it once
    // $Revisit - CReis Will caller_p ever be nullptr?
    if (caller_p && (iloop_p->m_data == SkLoop_looping))
      {
      iloop_p->m_data |= SkLoop_durational;
      }

    // Did not complete immediately
    return iloop_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Exits / unwinds loop
//
// #Modifiers virtual - overridden from SkExpressionBase
// #Author(s) Conan Reis
void SkLoop::invoke_exit(
  // invoked expression wrapped around this expression
  SkInvokedExpression * iexpr_p,
  // Active sub-expression requesting the exit or nullptr if not during active sub-expression.
  SkInvokedExpression * sub_exit_p // = nullptr
  ) const
  {
  // Stop this loop
  iexpr_p->m_data &= ~SkLoop_looping;
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
bool SkLoop::is_immediate(
  uint32_t * durational_idx_p // = nullptr
  ) const
  {
  // Determine if looping sub-expression executes and returns in one frame.
  return m_expr_p->is_immediate(durational_idx_p);
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkLoop::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkLoop, SkDebugInfo_size_used));

  m_expr_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkLoopExit Class Data Members
//=======================================================================================


//=======================================================================================
// SkLoopExit Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkLoopExit::~SkLoopExit()
  {
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this loop exit expression and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - loop name id
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkLoopExit::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkLoopExit::as_binary_length());

  // 4 bytes - loop name id
  m_name.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - loop name id
//               n bytes - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkLoopExit::as_binary_length() const
  {
  return 4u;
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
AString SkLoopExit::as_code() const
  {
  AString loop_name(m_name.as_str_dbg());
  uint     length = loop_name.get_length();
  AString str("exit", 4u);

  // Add name if it indicates one
  if (length)
    {
    str.ensure_size(8u + length);
    str.append(' ');
    str.append(loop_name);
    }

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_loop_exit
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkLoopExit::get_type() const
  {
  return SkExprType_loop_exit;
  }

//---------------------------------------------------------------------------------------
// Evaluates loop exit expression and returns the nil instance if desired.
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
SkInvokedBase * SkLoopExit::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Jumps out of innermost loop or the named loop if a loop name is supplied.

  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  const SkExpressionBase * expr_p;
  SkInvokedExpression *    icontext_p;
  SkInvokedExpression *    sub_iexpr_p = nullptr;
  ASymbol                  loop_name(m_name);  // Cache for loop
  bool                     found_loop = false;

  do 
    {
    // Ensure caller context not lost - likely inside a fork/branch
    if (caller_p == nullptr)
      {
      SK_ERROR_INFO(
        a_str_format(
          "Could not find loop to exit since call context is missing due to fork/branch!",
          loop_name.is_null() ? "" : loop_name.as_cstr_dbg()),
        caller_p);

      // Exit do-while loop
      break;
      }

    expr_p = caller_p->get_expr();  // Only SkInvokedExpression returns a value

    // Should only need to check for missing nesting loop in debug
    #if (SKOOKUM & SK_DEBUG)
      // Ensure loop exit does not extend beyond its local method/coroutine
      if (expr_p == nullptr)
        {
        SK_ERROR_INFO(
          a_str_format(
            "Tried to exit loop %s - and no loop was found in current context!",
            loop_name.is_null() ? "" : loop_name.as_cstr_dbg()),
          caller_p);

        // Exit do-while loop
        break;
        }
    #endif

    icontext_p = static_cast<SkInvokedExpression *>(caller_p);
    caller_p   = caller_p->m_caller_p;  // Must be called before invoke_exit()
    found_loop = expr_p->is_loop(loop_name);

    // Abort expression though don't notify caller.
    expr_p->invoke_exit(icontext_p, sub_iexpr_p);
    sub_iexpr_p = icontext_p;
    } while (!found_loop);

  // If a result is desired return nil
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
void SkLoopExit::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkLoopExit, SkDebugInfo_size_used));
  }


//=======================================================================================
// SkConversion Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkConversion::~SkConversion()
  {
  delete m_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - desired class type
//               2 bytes - vtable index of conversion method
//               n bytes - typed expression to convert
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkConversion::assign_binary(const void ** binary_pp)
  {
  if (m_expr_p)
    {
    delete m_expr_p;
    }

  // 4 bytes - desired class type
  m_type_p = SkClass::from_binary_ref(binary_pp);

  // 2 bytes - vtable index
  m_vtable_index = A_BYTE_STREAM_UI16_INC(binary_pp);

  // n bytes - expression typed binary
  m_expr_p = from_binary_typed_new(binary_pp);
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
//               4 bytes - desired class
//               2 bytes - vtable index of conversion method
//               n bytes - typed expression to convert
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkConversion::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkConversion::as_binary_length());

  // 4 bytes - desired class type
  m_type_p->as_binary_ref(binary_pp);

  // 2 bytes - vtable index
  A_BYTE_STREAM_OUT16(binary_pp, &m_vtable_index);

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
//               4 bytes - number of clauses
//               2 bytes - vtable index of conversion method
//               n bytes - clause binary }- Repeating 
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkConversion::as_binary_length() const
  {
  return SkClass::Binary_ref_size + 2u + m_expr_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

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
SkConversion::get_side_effect() const
  {
  // It could add itself to a list in which case it would have a side effect - though that
  // is unlikely.
  return m_expr_p->get_side_effect() ? SkSideEffect_secondary : SkSideEffect_none;
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
AString SkConversion::as_code() const
  {
  AString str(m_expr_p->as_code());

  str.append(">>", 2u);
  str.append(m_type_p->get_name_str_dbg());

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
SkExpressionBase * SkConversion::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_expr_p->find_expr_by_pos(pos, type);

  if (expr_p)
    {
    return expr_p;
    }

  // Return for all types
  return ((m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkConversion *>(this)
    : nullptr;
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
SkConversion::iterate_expressions(
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
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_conversion
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkConversion::get_type() const
  {
  return SkExprType_conversion;
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
SkInvokedBase * SkConversion::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Only bother with conversion if result is desired
  if (result_pp)
    {
    SkInstance * convertee_p = m_expr_p->invoke_now(scope_p, caller_p);
    SkInstance * convert_p   = convertee_p;

    // Only call convert method if necessary.
    if (!convertee_p->get_class()->is_class(*m_type_p))
      {
      // Debugging - store current expression in global rather than passing as argument since
      // only used for debug.
      SKDEBUG_ICALL_STORE_GEXPR(this);

      // Call to SKDEBUG_HOOK_EXPR() made in SkMethodCall<>::invoke_call()
      // $Revisit - CReis Conversion methods *could* have arguments with defaults though
      // currently this is not permitted due to the parser - see SkParser::parse_method()
      SkMethodCallOnInstance call(m_type_p->get_name(), m_vtable_index);
      call.invoke_call(convertee_p, scope_p, caller_p, &convert_p); // Direct method call without virtual function table
      convertee_p->dereference();
      }

    *result_pp = convert_p;
    }
  else
    {
    // Evaluate expression for side-effects
    m_expr_p->invoke_now_proc(scope_p, caller_p);

    // Assumes the conversion method does *not* have side-effects.
    }

  // Always returns immediately
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConversion::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_expr_p)
    {
    if (m_expr_p == receiver_p)
      {
      m_expr_p = nullptr;
      }
    else
      {
      m_expr_p->null_receiver(receiver_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkConversion::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkConversion, SkDebugInfo_size_used));

  m_expr_p->track_memory(mem_stats_p);
  }


//=======================================================================================
// SkCast Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkCast::~SkCast()
  {
  delete m_expr_p;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               5*bytes - desired class type
//               n bytes - typed expression to convert
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
void SkCast::assign_binary(const void ** binary_pp)
  {
  if (m_expr_p)
    {
    delete m_expr_p;
    }

  // 5*bytes - desired class type
  m_type_p = SkClassDescBase::from_binary_ref_typed(binary_pp);

  // n bytes - expression typed binary
  m_expr_p = from_binary_typed_new(binary_pp);
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
//               5*bytes - desired class type
//               n bytes - typed expression to convert
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkCast::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCast::as_binary_length());

  // 5*bytes - desired class type
  m_type_p->as_binary_ref_typed(binary_pp);

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
//               5*bytes - desired class type
//               n bytes - typed expression to convert
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkCast::as_binary_length() const
  {
  return m_type_p->as_binary_ref_typed_length() + m_expr_p->as_binary_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

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
SkCast::get_side_effect() const
  {
  return m_expr_p->get_side_effect() ? SkSideEffect_secondary : SkSideEffect_none;
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
AString SkCast::as_code() const
  {
  AString str(m_expr_p->as_code());

  str.append("<>", 2u);
  str.append(m_type_p->as_code());

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
SkExpressionBase * SkCast::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_expr_p->find_expr_by_pos(pos, type);

  if (expr_p)
    {
    return expr_p;
    }

  return ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkCast *>(this)
    : nullptr;
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
SkCast::iterate_expressions(
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
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_cast
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkCast::get_type() const
  {
  return SkExprType_cast;
  }

//---------------------------------------------------------------------------------------
// Determines if the cast is valid and if so passes through the embedded expression.
// 
// Returns:
//   true if the expression has completed its evaluation and there is a resulting
//   instance, false if there is a result pending.
// 
// Params:
//   scope_p:
//     scope for data/method/etc. look-ups.  It should always be an object derived from
//     SkInvokedContextBase.
//   caller_p:
//     object that called/invoked this expression and that may await a result. If it is
//     nullptr, then there is no object that needs to be returned to and notified when
//     this invocation is complete.
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is nullptr, then the result does not need to be returned and
//     only side-effects are desired.
//     
// See:       invoke_now()
// Modifiers: virtual (overriding pure from SkExpressionBase)
// Author(s): Conan Reis
SkInvokedBase * SkCast::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // No work is done - the expression evaluates just like normal. However in debug the
  // cast is checked to see if the type received is what was expected.
  
  // $Revisit - CReis In fact the SkCast wrapper could potentially be removed after parsing
  // has completed once enough context is available - like using indexes for members.

  // Should always complete immediately, but return invoked just in case.
  SkInvokedBase * invoked_p = m_expr_p->invoke(scope_p, caller_p, result_pp);

  #if (SKOOKUM & SK_DEBUG)
    // Runtime check to ensure that this is a valid cast
    if (result_pp)
      {
      SkClassDescBase * cast_class_p = m_type_p;

      // Just accept List classes - too expensive to check
      if (cast_class_p->get_class_type() != SkClassType_typed_class)
        {
        SkInstance * result_p       = *result_pp;
        SkClass *    result_class_p = result_p->get_class();

        // Note that the test order is opposite from `SkParser::parse_class_cast()` since
        // the actual type of the object is fully known.
        SK_ASSERT_INFO(
          result_class_p->is_class_type(cast_class_p),
          a_str_format(
            "Tried to cast %s which is type `%s` however it is not the same class or a subclass of the desired type `%s`!\n"
            "[The compiler thought that the cast *might* work though at runtime here it did not.]",
            result_p->as_code().as_cstr(),
            result_class_p->as_code().as_cstr(),
            cast_class_p->as_code().as_cstr()),
          *this);
          }
      }
  #endif

  return invoked_p;
  }

//---------------------------------------------------------------------------------------
// Nulls out the specified receiver expression.
// Arg         receiver_p - sub receiver to find and set to nullptr.
// Notes:      Handy when this expression is to be deleted, but the receiver needs to be
//             kept - as can happen with a structure that is progressively built up
//             during a parse, but midway through a parsing error occurs.
// Modifiers:   virtual overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCast::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_expr_p)
    {
    if (m_expr_p == receiver_p)
      {
      m_expr_p = nullptr;
      }
    else
      {
      m_expr_p->null_receiver(receiver_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkCast::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkCast, SkDebugInfo_size_used));

  m_expr_p->track_memory(mem_stats_p);
  }
