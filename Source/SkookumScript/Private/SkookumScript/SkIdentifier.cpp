// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Identifier for named code objects - i.e. temporary variables, etc.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkIdentifier.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkIdentifier.inl>
#endif

#include <AgogCore/AString.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkObjectBase.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkRawMember.hpp>


//=======================================================================================
// SkIdentifierLocal Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_identifier_local
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkIdentifierLocal::get_type() const
  {
  return SkExprType_identifier_local;
  }

//---------------------------------------------------------------------------------------
// Determine if identifier refers to a variable in the local scope.
// Returns:    true if local false if from some other instance
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkIdentifierLocal::is_local() const
  {
  return true;
  }

//---------------------------------------------------------------------------------------
// Evaluates identifier expression and returns the instance referred to.
// Returns:    true - indicating that the expression has completed its evaluation and
//             that there is a resulting instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Notes:      No caller object is needed since SkIdentifierLocal always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkIdentifierLocal::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Do the hook regardless of whether look-up is done
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // If no result is desired then don't bother doing any evaluation - note however that
  // runtime error checking for valid identifiers is also bypassed when this occurs.
  if (result_pp)
    {
    SkInvokedContextBase * invoked_p = scope_p->get_scope_context();
    SkInstance * instance_p = invoked_p->get_data().get_array()[m_data_idx];
    instance_p->reference();
    *result_pp = instance_p;
    }

  return nullptr;
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
// supplied instance, assuming the variable was previously *uninitialized*.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkIdentifierLocal::bind_data(
  SkBind *        bind_p,
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  )
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SKDEBUG_HOOK_EXPR(bind_p, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  bind_data(obj_p, scope_p, caller_p, return_result);
  }

#endif


//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierLocal::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkIdentifierLocal, SkDebugInfo_size_used));
  }


//=======================================================================================
// SkIdentifierMember Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkIdentifierMember::~SkIdentifierMember()
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
//             Binary composition:
//               6 bytes - identifier name id + data index
//               n bytes - owner expression typed binary or inferred this (nullptr)
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierMember::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkIdentifierMember::as_binary_length());

  SkIdentifierLocal::as_binary(binary_pp);

  // n bytes - owner expression typed binary or inferred this (nullptr)
  SkExpressionBase::as_binary_typed(m_owner_expr_p, binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               6 bytes - identifier name id + data index
//               n bytes - owner expression typed binary or inferred this (nullptr)
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkIdentifierMember::as_binary_length() const
  {
  return SkIdentifierLocal::as_binary_length() + as_binary_typed_length(m_owner_expr_p);
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
SkIdentifierMember::get_side_effect() const
  {
  return (m_owner_expr_p && m_owner_expr_p->get_side_effect())
    ? SkSideEffect_secondary
    : SkSideEffect_none;
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
AString SkIdentifierMember::as_code() const
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
  
  str.append(get_name_str_dbg());

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
SkExpressionBase * SkIdentifierMember::find_expr_by_pos(
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

  return ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    ? const_cast<SkIdentifierMember *>(this)
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
SkIdentifierMember::iterate_expressions(
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


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_identifier_member
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkIdentifierMember::get_type() const
  {
  return SkExprType_identifier_member;
  }

//---------------------------------------------------------------------------------------
// Determine if identifier refers to a variable in the local scope.
// Returns:    true if local false if from some other instance
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkIdentifierMember::is_local() const
  {
  return false;
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
// Notes:      No caller object is needed since SkIdentifierMember always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkIdentifierMember::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  if (result_pp)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If result is desired then do full evaluation

    // Note that the receiver is always an expression that returns immediately
    SkInstance * owner_p = m_owner_expr_p
      ? m_owner_expr_p->invoke_now(scope_p, caller_p)  // evaluate owner expression
      : scope_p->get_topmost_scope();             // 'this' is inferred

    // Note that the debug of the invocation is called after the evaluation of the receiver.
    // Do the hook regardless of whether look-up is done
    SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

    // If the instance has data members, it must be an SkDataInstance
    SkDataInstance * data_instance_p = static_cast<SkDataInstance *>(owner_p);
    SkInstance * instance_p = data_instance_p->get_data_by_idx(m_data_idx);
    instance_p->reference();
    *result_pp = instance_p;

    if (m_owner_expr_p)
      {
      owner_p->dereference();
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If no result is desired then limit evaluation to owner expression for any side
    // effects - note however that runtime error checking for valid identifiers is also
    // bypassed when this occurs.
    if (m_owner_expr_p)
      {
      m_owner_expr_p->invoke_now_proc(scope_p, caller_p);

      // Do the hook regardless of whether look-up is done
      SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);
      }
    }

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
void SkIdentifierMember::null_receiver(SkExpressionBase * receiver_p)
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

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
// supplied instance, assuming the variable was previously *uninitialized*.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         return_result - if data_p is going to be returned as a result of an
//             invocation then return_result should be set to true to ensure that data_p
//             is not prematurely dereferenced.
// Notes:      called by SkBind
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkIdentifierMember::bind_data(
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  )
  {
  // Note that the receiver is always an expression that returns immediately
  SkInstance * owner_p = m_owner_expr_p
    ? m_owner_expr_p->invoke_now(scope_p, caller_p)  // evaluate owner expression
    : scope_p->get_topmost_scope();             // 'this' is inferred

  // If the instance has data members, it must be an SkDataInstance
  SkDataInstance * data_instance_p = static_cast<SkDataInstance *>(owner_p);
  data_instance_p->set_data_by_idx(m_data_idx, obj_p);

  // House keeping
  if (m_owner_expr_p)
    {
    if (return_result)
      {
      // Non-class instances could have a member set to themselves so ensure that the owner
      // is dereferenced if it is also the result.
      obj_p->reference();
      owner_p->dereference();
      obj_p->dereference_delay();
      }
    else
      {
      owner_p->dereference();
      }
    }
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
//             supplied instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         return_result - if data_p is going to be returned as a result of an
//             invocation then return_result should be set to true to ensure that data_p
//             is not prematurely dereferenced.
// Notes:      called by SkBind
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkIdentifierMember::bind_data(
  SkBind *        bind_p,
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  )
  {
  // Note that the debug of the invocation is called after the evaluation of the receiver
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SKDEBUG_HOOK_EXPR(bind_p, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkIdentifierMember::bind_data(obj_p, scope_p, caller_p, return_result); // Call virtual function directly, bypassing vtable
  }

#endif


//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierMember::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkIdentifierMember, SkDebugInfo_size_used));

  if (m_owner_expr_p)
    {
    m_owner_expr_p->track_memory(mem_stats_p);
    }
  }

//=======================================================================================
// SkIdentifierRawMember Method Definitions
//=======================================================================================

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
//             Binary composition:
//               n bytes - SkIdentifierMember
//               4 bytes - owner class reference
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierRawMember::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkIdentifierRawMember::as_binary_length());

  SkIdentifierMember::as_binary(binary_pp); 
  m_owner_class_p->as_binary_ref(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes - SkIdentifierMember
//               4 bytes - owner class reference
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkIdentifierRawMember::as_binary_length() const
  {
  return SkIdentifierMember::as_binary_length() + 4u;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_identifier_raw_member
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkIdentifierRawMember::get_type() const
  {
  return SkExprType_identifier_raw_member;
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
// Notes:      No caller object is needed since SkIdentifierMember always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkIdentifierRawMember::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  if (result_pp)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If result is desired then do full evaluation

    // Note that the receiver is always an expression that returns immediately
    SkInstance * owner_p = m_owner_expr_p
      ? m_owner_expr_p->invoke_now(scope_p, caller_p)  // evaluate owner expression
      : scope_p->get_topmost_scope();             // 'this' is inferred

    // Note that the debug of the invocation is called after the evaluation of the receiver.
    // Do the hook regardless of whether look-up is done
    SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

    // Create instance from raw data
    *result_pp = SkRawMemberHandle(SkRawMemberInfo(m_owner_class_p, m_data_idx), owner_p).new_instance();

    if (m_owner_expr_p)
      {
      owner_p->dereference();
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If no result is desired then limit evaluation to owner expression for any side
    // effects - note however that runtime error checking for valid identifiers is also
    // bypassed when this occurs.
    if (m_owner_expr_p)
      {
      m_owner_expr_p->invoke_now_proc(scope_p, caller_p);

      // Do the hook regardless of whether look-up is done
      SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierRawMember::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkIdentifierRawMember, SkDebugInfo_size_used));

  if (m_owner_expr_p)
    {
    m_owner_expr_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkIdentifierClassMember Method Definitions
//=======================================================================================

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
//             Binary composition:
//               6 bytes - identifier name id + data index
//               4 bytes - owner class reference
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierClassMember::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkIdentifierClassMember::as_binary_length());

  SkIdentifierLocal::as_binary(binary_pp);

  // 4 bytes - owner class reference
  m_owner_class_p->as_binary_ref(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               6 bytes - identifier name id + data index
//               4 bytes - owner class reference
//
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkIdentifierClassMember::as_binary_length() const
  {
  return SkIdentifierLocal::as_binary_length() + SkClass::Binary_ref_size;
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
AString SkIdentifierClassMember::as_code() const
  {
  AString str;

  if (m_owner_class_p)
    {
    str = m_owner_class_p->as_code();
    str.append('.');
    }
  else
    {
    // $Revisit - CReis If this expression appears in class scope already then "this_class" might not be correct.
    str.append("this_class.", 11u);
    }
  
  str.append(get_name_str_dbg());

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_identifier_member
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkIdentifierClassMember::get_type() const
  {
  return SkExprType_identifier_class_member;
  }

//---------------------------------------------------------------------------------------
// Determine if identifier refers to a variable in the local scope.
// Returns:    true if local false if from some other instance
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkIdentifierClassMember::is_local() const
  {
  return false;
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
// Notes:      No caller object is needed since SkIdentifierClassMember always returns the result
//             immediately.
//             Note that the 'reserved' identifiers `this`, `this_class`, `this_code`,
//             `this_mind`, `nil`, and classes are handled by SkookumScript/SkLiteral.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkIdentifierClassMember::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  if (result_pp)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If result is desired then do full evaluation

    // Note that the debug of the invocation is called after the evaluation of the receiver
    // Do the hook regardless of whether look-up is done
    SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

    SkInstance * instance_p = m_owner_class_p->get_class_data_value_by_idx(m_data_idx);
    instance_p->reference();
    *result_pp = instance_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
//             supplied instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Notes:      called by SkBind
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkIdentifierClassMember::bind_data(
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  ) 
  {
  m_owner_class_p->set_class_data_value_by_idx(m_data_idx, obj_p);
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
//             supplied instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Notes:      called by SkBind
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkIdentifierClassMember::bind_data(
  SkBind *        bind_p,
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  ) 
  {
  // Note that the debug of the invocation is called after the evaluation of the receiver
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SKDEBUG_HOOK_EXPR(bind_p, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  SkIdentifierClassMember::bind_data(obj_p, scope_p, caller_p, return_result); // Call virtual function directly, bypassing vtable
  }

#endif


//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkIdentifierClassMember::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkIdentifierClassMember, SkDebugInfo_size_used));
  }
