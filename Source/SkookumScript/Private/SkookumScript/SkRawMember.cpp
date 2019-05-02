// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Raw member access functionality
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkRawMember.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkRawMember.inl>
#endif

#include <AgogCore/AString.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkInvocation.hpp>


//=======================================================================================
// SkRawMemberInfo Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------

void SkRawMemberInfo::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkRawMemberInfo::as_binary_length());

  // 4 bytes - owner class reference
  m_owner_class_p->as_binary_ref(binary_pp);

  // 2 bytes - data_idx
  A_BYTE_STREAM_OUT16(binary_pp, &m_data_idx);
  }

//---------------------------------------------------------------------------------------

uint32_t SkRawMemberInfo::as_binary_length() const
  {
  return SkClass::Binary_ref_size + 2u;
  }

#endif

//=======================================================================================
// SkRawMemberBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkRawMemberBase::SkRawMemberBase(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade)
  : m_owner_expr_p(owner_expr_p)
  , m_member_info(member_info)
  , m_raw_owner_cascade(raw_owner_cascade)
  {
  }

//---------------------------------------------------------------------------------------

SkRawMemberBase::~SkRawMemberBase()
  {
  if (m_owner_expr_p)
    {
    delete m_owner_expr_p;
    }
  }

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this expression and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
// Notes:      Binary composition:
//               n bytes - owner expression typed binary or inferred this (nullptr)
//               6 bytes - member info
//               n bytes - raw owner cascade
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Markus Breyer
void SkRawMemberBase::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkRawMemberBase::as_binary_length());

  // n bytes - owner expression typed binary or inferred this (nullptr)
  SkExpressionBase::as_binary_typed(m_owner_expr_p, binary_pp);

  // 6 bytes - raw member info
  m_member_info.as_binary(binary_pp);

  // n bytes - raw owner cascade
  m_raw_owner_cascade.as_binary8(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
// Notes:      Binary composition:
//               n bytes - owner expression typed binary or inferred this (nullptr)
//               6 bytes - member info
//               n bytes - raw owner cascade
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkRawMemberBase::as_binary_length() const
  {
  return as_binary_typed_length(m_owner_expr_p) + m_member_info.as_binary_length() + m_raw_owner_cascade.as_binary_length8();
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
AString SkRawMemberBase::as_code() const
  {
  AString str;

  if (m_owner_expr_p)
    {
    str = m_owner_expr_p->as_code();
    str.append('.');
    }
  else
    {
    str.append("this.", 5u);
    }

  // Cascade counts down from owner to member
  for (SkRawMemberInfo & info : m_raw_owner_cascade)
    { 
    str.append(info.get_typed_name()->get_name_cstr());
    str.append('.');
    }

  str.append(m_member_info.get_typed_name()->get_name_cstr());

  return str;
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------

void SkRawMemberBase::null_receiver(SkExpressionBase * receiver_p)
  {
  if (m_owner_expr_p)
    {
    if (m_owner_expr_p == receiver_p)
      {
      m_owner_expr_p = nullptr;
      }
    else
      {
      m_owner_expr_p->null_receiver(receiver_p);
      }
    }
  }

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
SkExpressionBase * SkRawMemberBase::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_owner_expr_p  // Could be implicit 'this' (nullptr)
    ? m_owner_expr_p->find_expr_by_pos(pos, type)
    : nullptr;

  if (expr_p)
    {
    return expr_p;
    }

  if ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkRawMemberBase *>(this);
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
SkRawMemberBase::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || (m_owner_expr_p && m_owner_expr_p->iterate_expressions(apply_expr_p, invokable_p)))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)

//=======================================================================================
// SkRawMemberAssignment Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkRawMemberAssignment::SkRawMemberAssignment(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade, SkExpressionBase * value_expr_p)
  : SkRawMemberBase(owner_expr_p, member_info, raw_owner_cascade)
  , m_value_expr_p(value_expr_p)
  {
  }

//---------------------------------------------------------------------------------------

SkRawMemberAssignment::~SkRawMemberAssignment()
  {
  delete m_value_expr_p;
  }

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this expression and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - value expression typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkRawMemberAssignment::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkRawMemberAssignment::as_binary_length());

  // n bytes - SkRawMemberBase
  SkRawMemberBase::as_binary(binary_pp);

  // n bytes - value expression typed binary
  SkExpressionBase::as_binary_typed(m_value_expr_p, binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - value expression typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkRawMemberAssignment::as_binary_length() const
  {
  return SkRawMemberBase::as_binary_length() + as_binary_typed_length(m_value_expr_p);
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
AString SkRawMemberAssignment::as_code() const
  {
  AString str(SkRawMemberBase::as_code());

  str.append(" := ", 4);
  str.append(m_value_expr_p->as_code());

  return str;
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_raw_member_assignment
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkRawMemberAssignment::get_type() const
  {
  return SkExprType_raw_member_assignment;
  }

//---------------------------------------------------------------------------------------
// Evaluates identifier expression and returns the instance referred to.
// Returns:    true - indicating that the expression has completed its evaluation and
//             that there is a resulting instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.
// See:        invoke_now()
// Notes:      No caller object is needed since SkRawMemberClassMember always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:  virtual (overriding pure from SkExpressionBase)
// Author(s):  Conan Reis
SkInvokedBase * SkRawMemberAssignment::invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p /*= nullptr*/, SkInstance ** result_pp /*= nullptr*/) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If result is desired then do full evaluation

  // Note that the receiver is always an expression that returns immediately
  SkInstance * owner_p = m_owner_expr_p
    ? m_owner_expr_p->invoke_now(scope_p, caller_p)  // evaluate owner expression
    : scope_p->get_topmost_scope();                  // 'this' is inferred

  // Get value, must be non-null
  SkInstance * value_p = m_value_expr_p->invoke_now(scope_p, caller_p);  // evaluate value expression

  // Note that the debug of the invocation is called after the evaluation of the receiver.
  // Do the hook regardless of whether look-up is done
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // Evaluate owner cascade if any
  SkRawMemberEvaluator * eval_stack_p = nullptr;
  SkInstance * nested_owner_p = owner_p;
  uint32_t cascade_count = m_raw_owner_cascade.get_count();
  if (cascade_count) // zero in 99% of cases
    {
    eval_stack_p = a_stack_allocate(cascade_count, SkRawMemberEvaluator);
    // Crawl down cascade, passing instance of outer member to inner member
    SkRawMemberEvaluator * eval_p = eval_stack_p;
    const SkRawMemberInfo * info_p = m_raw_owner_cascade.get_array();
    for (uint32_t count = cascade_count; count; --count, ++eval_p, ++info_p)
      {
      new (eval_p) SkRawMemberEvaluator(*info_p, nested_owner_p);
      nested_owner_p = eval_p->get_member_instance();
      }
    }

  // Perform assignment
  SkRawMemberHandle(m_member_info, nested_owner_p).assign(value_p);

  // Return result
  if (result_pp)
    {
    *result_pp = value_p;
    }
  else
    {
    value_p->dereference();
    }

  // Promote modified data back up the cascade
  if (cascade_count) // zero in 99% of cases
    {
    SkRawMemberEvaluator * eval_p = eval_stack_p + cascade_count;
    for (uint32_t count = cascade_count; count; --count)
      {
      (--eval_p)->~SkRawMemberEvaluator(); // Destructor will assign instance back
      }
    }

  if (m_owner_expr_p)
    {
    owner_p->dereference();
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkRawMemberAssignment::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkRawMemberAssignment, SkDebugInfo_size_used));
  }

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
SkExpressionBase * SkRawMemberAssignment::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = SkRawMemberBase::find_expr_by_pos(pos, type);
  if (expr_p)
    {
    return expr_p;
    }

  return m_value_expr_p->find_expr_by_pos(pos, type);
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
  SkRawMemberAssignment::iterate_expressions(
    // Calls apply_expr() on each expression - see SkApplyExpressionBase
    SkApplyExpressionBase * apply_expr_p,
    // Optional invokable (method, coroutine) where this expression originates or nullptr.
    const SkInvokableBase * invokable_p // = nullptr
    )
  {
  if (SkRawMemberBase::iterate_expressions(apply_expr_p, invokable_p)
    || m_value_expr_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)

//=======================================================================================
// SkRawMemberModifyingInvocation Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

SkRawMemberModifyingInvocation::SkRawMemberModifyingInvocation(SkExpressionBase * owner_expr_p, const SkRawMemberInfo & member_info, const AVCompactArray<SkRawMemberInfo> & raw_owner_cascade, SkInvokeBase * call_p)
  : SkRawMemberBase(owner_expr_p, member_info, raw_owner_cascade)
  , m_call_p(call_p)
  {
  }

//---------------------------------------------------------------------------------------

SkRawMemberModifyingInvocation::~SkRawMemberModifyingInvocation()
  {
  delete m_call_p;
  }

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this expression and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - call typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkRawMemberModifyingInvocation::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkRawMemberModifyingInvocation::as_binary_length());

  // n bytes - SkRawMemberBase
  SkRawMemberBase::as_binary(binary_pp);

  // n bytes - call
  m_call_p->as_binary_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - call typed binary
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkRawMemberModifyingInvocation::as_binary_length() const
  {
  return SkRawMemberBase::as_binary_length() + m_call_p->as_binary_typed_length();
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
AString SkRawMemberModifyingInvocation::as_code() const
  {
  AString str(SkRawMemberBase::as_code());

  str.append('.');
  str.append(m_call_p->as_code());

  return str;
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_raw_member_invocation
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkRawMemberModifyingInvocation::get_type() const
  {
  return SkExprType_raw_member_invocation;
  }

//---------------------------------------------------------------------------------------
// Evaluates identifier expression and returns the instance referred to.
// Returns:    true - indicating that the expression has completed its evaluation and
//             that there is a resulting instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.
// See:        invoke_now()
// Notes:      No caller object is needed since SkRawMemberClassMember always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:  virtual (overriding pure from SkExpressionBase)
// Author(s):  Conan Reis
SkInvokedBase * SkRawMemberModifyingInvocation::invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p /*= nullptr*/, SkInstance ** result_pp /*= nullptr*/) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If result is desired then do full evaluation

  // Note that the receiver is always an expression that returns immediately
  SkInstance * owner_p = m_owner_expr_p
    ? m_owner_expr_p->invoke_now(scope_p, caller_p)  // evaluate owner expression
    : scope_p->get_topmost_scope();                  // 'this' is inferred

  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // Evaluate owner cascade if any
  SkRawMemberEvaluator * eval_stack_p = nullptr;
  SkInstance * nested_owner_p = owner_p;
  uint32_t cascade_count = m_raw_owner_cascade.get_count();
  if (cascade_count) // zero in 99% of cases
    {
    eval_stack_p = a_stack_allocate(cascade_count, SkRawMemberEvaluator);
    // Crawl down cascade, passing instance of outer member to inner member
    SkRawMemberEvaluator * eval_p = eval_stack_p;
    const SkRawMemberInfo * info_p = m_raw_owner_cascade.get_array();
    for (uint32_t count = cascade_count; count; --count, ++eval_p, ++info_p)
      {
      new (eval_p) SkRawMemberEvaluator(*info_p, nested_owner_p);
      nested_owner_p = eval_p->get_member_instance();
      }
    }

  // Perform invocation
  SkRawMemberHandle member_handle(m_member_info, nested_owner_p);
  SkInstance * receiver_p = member_handle.new_instance();
  // Note that invoke_call() will properly increment the pending count as necessary.
  SkInvokedBase * invoked_p = m_call_p->invoke_call(receiver_p, scope_p, caller_p, result_pp);
  // Store back and clean up
  member_handle.assign(receiver_p);
  receiver_p->dereference();

  // Promote modified data back up the cascade
  if (cascade_count) // zero in 99% of cases
    {
    SkRawMemberEvaluator * eval_p = eval_stack_p + cascade_count;
    for (uint32_t count = cascade_count; count; --count)
      {
      (--eval_p)->~SkRawMemberEvaluator(); // Destructor will assign instance back
      }
    }

  if (m_owner_expr_p)
    {
    owner_p->dereference();
    }

  return invoked_p;
  }

//---------------------------------------------------------------------------------------

bool SkRawMemberModifyingInvocation::is_immediate(uint32_t * durational_idx_p /*= nullptr*/) const
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
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkRawMemberModifyingInvocation::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkRawMemberModifyingInvocation, SkDebugInfo_size_used));
  }

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
SkExpressionBase * SkRawMemberModifyingInvocation::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = SkRawMemberBase::find_expr_by_pos(pos, type);
  if (expr_p)
    {
    return expr_p;
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
SkRawMemberModifyingInvocation::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p,
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (SkRawMemberBase::iterate_expressions(apply_expr_p, invokable_p)
    || m_call_p->iterate_expressions(apply_expr_p, invokable_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)

