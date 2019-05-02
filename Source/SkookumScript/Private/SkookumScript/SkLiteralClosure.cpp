// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Literal expression for closures & anonymous/lambda code/functions
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkLiteralClosure.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>


//=======================================================================================
// Local Global Structures
//=======================================================================================


//=======================================================================================
// SkClosureInfoBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual
// Author(s):   Conan Reis
SkClosureInfoBase::~SkClosureInfoBase()
  {
  }

//---------------------------------------------------------------------------------------
// #Description
//   Transfer contents ownership constructor
//
// #Author(s) Conan Reis
SkClosureInfoBase::SkClosureInfoBase(
  // Closure to take ownership of contents from
  SkClosureInfoBase * closure_p
  ) :
  m_captured(&closure_p->m_captured)
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
//               4 bytes - capture count
//               4 bytes - temp variable name id }- Repeating 
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkClosureInfoBase::SkClosureInfoBase(const void ** binary_pp)
  {
  // 4 bytes - number of temp variables
  uint32_t count = A_BYTE_STREAM_UI32_INC(binary_pp);

  m_captured.empty_ensure_count_undef(count);

  // 6 bytes * count - temp variable name id + data index
  for (; count > 0u; count--)
    {
    m_captured.append_last_undef(SkNamedIndexed(binary_pp));
    }
  }

#endif


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this list literal and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes  - capture count
//               4 bytes }- capture variable id
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkClosureInfoBase::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClosureInfoBase::as_binary_length());

  // 4 bytes - capture count
  // 4 bytes - capture variable id
  m_captured.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes  - capture count
//               4 bytes }- capture variable id
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkClosureInfoBase::as_binary_length() const
  {
  return 4u + (6u * m_captured.get_count());
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// #Description
//   Gets sum of all captured variable name lengths - used to make a string representation
//   when debugging etc.
//
// #Author(s) Conan Reis
uint32_t SkClosureInfoBase::get_captured_name_lengths() const
  {
  uint32_t count = m_captured.get_count();

  #if defined(A_SYMBOL_STR_DB)
    uint32_t length_sum = 0u;

    if (count)
      {
      SkNamedIndexed * sym_p     = m_captured.get_array();
      SkNamedIndexed * sym_end_p = sym_p + count;

      do 
        {
        length_sum += sym_p->get_name().get_str_length();
        sym_p++;
        }
      while (sym_p < sym_end_p);
      }

    return length_sum;
  #else
    // Guess lengths
    return count * 12u;
  #endif
  }

#endif

#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Fill in SkMemberInfo with info about the member that contains this closure
void SkClosureInfoBase::get_member_info(SkMemberInfo * member_info_p) const
  {
  SkInvokableBase * invokable_p = get_invokable();
  member_info_p->m_member_id    = *invokable_p;
  member_info_p->m_type         = eSkMember(invokable_p->get_vtable_index() & SkMemberInfo::ByteFlag__type_mask);
  member_info_p->m_class_scope  = !!(invokable_p->get_vtable_index() & SkMemberInfo::ByteFlag_class_member);
  member_info_p->m_is_closure   = true;
  }

#endif

//=======================================================================================
// SkClosureInfoMethod Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Description
//   Default constructor
//
// #Author(s) Conan Reis
SkClosureInfoMethod::SkClosureInfoMethod() :
  SkMethod(ASymbol_closure, SkBrain::ms_object_class_p, 0u, 0u)
  {
  // The decorated/fancy name for a closure method is created in real-time rather than at
  // compile/parse-time (to reduce auto symbol creation) and looks similar to:
  // 
  // Class@func()-Type^closure[207]
  // Class@func()-^closure[207]
  // 
  // Where:
  //   Class@func() - method/coroutine that the closure is written in
  //   207          - index position where closure starts in Class@func()
  //   Type         - class type of closure receiver/this [though might not be available]
  }

//---------------------------------------------------------------------------------------
// #Description
//   Transfer contents ownership constructor
//
// #Author(s) Conan Reis
SkClosureInfoMethod::SkClosureInfoMethod(
  // Closure to take ownership of contents from
  SkClosureInfoMethod * closure_p
  ) :
  SkClosureInfoBase(closure_p),
  SkMethod(closure_p->m_name, closure_p->m_scope_p, closure_p->m_params_p, closure_p->m_invoked_data_array_size, closure_p->m_annotation_flags, closure_p->m_expr_p)
  {
  closure_p->m_expr_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkClosureInfoMethod::~SkClosureInfoMethod()
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
//               4 bytes  - capture count
//               4 bytes }- capture variable id
//               n bytes  - parameter list
//               n bytes  - expression typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkClosureInfoMethod::SkClosureInfoMethod(const void ** binary_pp) :
  // 4 bytes - capture count
  // 4 bytes - capture variable id
  SkClosureInfoBase(binary_pp),
  SkMethod(ASymbol_closure, nullptr, 0u, 0u)
  {
  #if (SKOOKUM & SK_DEBUG)
    // In debug builds, remember which routine this closure is defined in
    const SkMemberInfo & current_routine = SkRuntimeBase::ms_singleton_p->m_current_routine;
    set_name(current_routine.m_member_id.get_name());
    set_scope(current_routine.m_member_id.get_scope());
    // Use vtable index to store type and class membership
    set_vtable_index(int16_t(current_routine.m_type | (current_routine.m_class_scope ? SkMemberInfo::ByteFlag_class_member : 0)));
  #endif

  // n bytes  - parameter list
  // n bytes  - expression typed binary
  // Here rather than in initializer list since list doesn't guarantee order.
  SkMethod::assign_binary_no_name(binary_pp, nullptr);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
void SkClosureInfoMethod::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClosureInfoMethod::as_binary_length());
  
  SkClosureInfoBase::as_binary(binary_pp);
  SkMethod::as_binary(binary_pp, false);
  }

//---------------------------------------------------------------------------------------
uint32_t SkClosureInfoMethod::as_binary_length() const
  {
  return SkClosureInfoBase::as_binary_length() + SkMethod::as_binary_length(false);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkClosureInfoBase
// Author(s):   Conan Reis
void SkClosureInfoMethod::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkClosureInfoMethod, 0u));
  m_expr_p->track_memory(mem_stats_p);

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkClosureInfoCoroutine Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Description
//   Default constructor
//
// #Author(s) Conan Reis
SkClosureInfoCoroutine::SkClosureInfoCoroutine() :
  SkCoroutine(ASymbol__closure, SkBrain::ms_object_class_p, 0u, 0u)
  {
  // The decorated/fancy name for a closure coroutine is created in real-time rather than
  // at compile/parse-time (to reduce auto symbol creation) and looks similar to:
  // 
  // Class@func()-Type^closure[207]
  // Class@func()-^closure[207]
  // 
  // Where:
  //   Class@func() - method/coroutine that the closure is written in
  //   207          - index position where closure starts in Class@func()
  //   Type         - class type of closure receiver/this [though might not be available]
  }

//---------------------------------------------------------------------------------------
// #Description
//   Transfer contents ownership constructor
//
// #Author(s) Conan Reis
SkClosureInfoCoroutine::SkClosureInfoCoroutine(
  // Closure to take ownership of contents from
  SkClosureInfoMethod * closure_p
  ) :
  SkClosureInfoBase(closure_p),
  SkCoroutine(closure_p->get_name(), closure_p->get_scope(), &closure_p->get_params(), closure_p->get_invoked_data_array_size(), closure_p->get_annotation_flags(), closure_p->get_custom_expr())
  {
  closure_p->replace_expression(nullptr);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkClosureInfoCoroutine::~SkClosureInfoCoroutine()
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
//               4 bytes  - capture count
//               4 bytes }- capture variable id
//               n bytes  - parameter list
//               n bytes  - expression typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkClosureInfoCoroutine::SkClosureInfoCoroutine(const void ** binary_pp) :
  // 4 bytes  - capture count
  // 4 bytes }- capture variable id
  SkClosureInfoBase(binary_pp),
  SkCoroutine(ASymbol__closure, nullptr, 0u, 0u)
  {
  #if (SKOOKUM & SK_DEBUG)
    // In debug builds, remember which routine this closure is defined in
    const SkMemberInfo & current_routine = SkRuntimeBase::ms_singleton_p->m_current_routine;
    set_name(current_routine.m_member_id.get_name());
    set_scope(current_routine.m_member_id.get_scope());
    // Use vtable index to store type and class membership
    set_vtable_index(int16_t(current_routine.m_type | (current_routine.m_class_scope ? SkMemberInfo::ByteFlag_class_member : 0)));
  #endif

  // n bytes  - parameter list
  // n bytes  - expression typed binary
  // Here rather than in initializer list since list doesn't guarantee order.
  SkCoroutine::assign_binary_no_name(binary_pp, nullptr);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
void SkClosureInfoCoroutine::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClosureInfoCoroutine::as_binary_length());

  SkClosureInfoBase::as_binary(binary_pp);
  SkCoroutine::as_binary(binary_pp, false);
  }

//---------------------------------------------------------------------------------------
uint32_t SkClosureInfoCoroutine::as_binary_length() const
  {
  return SkClosureInfoBase::as_binary_length() + SkCoroutine::as_binary_length(false);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkClosureInfoCoroutine::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkClosureInfoCoroutine, 0u));
  m_expr_p->track_memory(mem_stats_p);

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkLiteralClosure Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Description
//   
//
// #Examples
//   
//
// #Notes
//   
//
// #Modifiers 
// #See Also  
// #Author(s) Conan Reis
SkLiteralClosure::SkLiteralClosure(
  SkExpressionBase * recv_p // = nullptr
  ) :
  m_receiver_p(recv_p)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkLiteralClosure::~SkLiteralClosure()
  {
  delete m_receiver_p;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Transfer contents ownership constructor - used by SkParser
//
// #Author(s) Conan Reis
SkLiteralClosure::SkLiteralClosure(
  // Closure *method* to take ownership of contents from
  SkLiteralClosure * closure_method_p,
  // SkExprType_closure_method or SkExprType_closure_coroutine
  eSkExprType type
  ) :
  m_receiver_p(closure_method_p->m_receiver_p)
  {
  SkClosureInfoBase *   info_p = closure_method_p->m_info_p;
  SkClosureInfoMethod * method_info_p = static_cast<SkClosureInfoMethod *>(info_p);

  m_info_p = (type == SkExprType_closure_method)
    ? (SkClosureInfoBase *)SK_NEW(SkClosureInfoMethod)(method_info_p)
    : (SkClosureInfoBase *)SK_NEW(SkClosureInfoCoroutine)(method_info_p);

  closure_method_p->m_receiver_p = nullptr;
  closure_method_p->m_info_p     = nullptr;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - receiver expression binary
//               n bytes - closure info
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkLiteralClosure::SkLiteralClosure(
  const void ** binary_pp,
  eSkExprType   type
  ) :
  m_receiver_p(from_binary_typed_new(binary_pp))
  {
  // n bytes - closure info
  m_info_p = (type == SkExprType_closure_method)
    ? (SkClosureInfoBase *)SK_NEW(SkClosureInfoMethod)(binary_pp)
    : (SkClosureInfoBase *)SK_NEW(SkClosureInfoCoroutine)(binary_pp);
  }

#endif


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this list literal and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               n bytes - receiver expression typed binary
//               4 bytes - capture count
//               n bytes - closure info 
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteralClosure::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkLiteralClosure::as_binary_length());

  // n bytes - receiver expression typed binary
  as_binary_typed(m_receiver_p, binary_pp);

  // n bytes - closure info 
  m_info_p->as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               n bytes  - receiver expression typed binary
//               4 bytes  - capture count
//               4 bytes }- capture variable id
//               n bytes  - parameter list
//               n bytes  - expression typed binary
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkLiteralClosure::as_binary_length() const
  {
  return as_binary_typed_length(m_receiver_p)
    + m_info_p->as_binary_length();
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
SkLiteralClosure::get_side_effect() const
  {
  return (m_receiver_p && m_receiver_p->get_side_effect())
    ? SkSideEffect_secondary
    : SkSideEffect_none;
  }

#endif


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
AString SkLiteralClosure::as_code() const
  {
  AString recv_str;

  if (m_receiver_p)
    {
    recv_str.append(m_receiver_p->as_code());
    }
  else
    {
    recv_str.append("this", 4u);
    }

  SkClosureInfoBase * info_p = m_info_p;

  AString str;
  bool    durational = !info_p->is_method();
  AString param_str(info_p->get_closure_params().as_code(durational ? SkParameters::StrFlag__default_no_return : SkParameters::StrFlag__default));
  AString code_str(info_p->get_closure_expr()->as_code_block());
  uint32_t    capture_count = info_p->m_captured.get_count();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine needed string length
  uint32_t str_len = durational ? 2u : 1u;  // "^_" | "^"

  str_len += recv_str.get_length() + 1u;

  if (m_receiver_p == nullptr)
    {
    // "this"
    str_len += 4u;
    capture_count++;
    }

  // "// captured: "
  const uint32_t captured_str_len = 13u;

  if (capture_count)
    {
    str_len += SkDebug::ms_indent_size
      + captured_str_len
      + info_p->get_captured_name_lengths()
      + ((capture_count - 1u) * 2u)  // ", "
      + 1u;  // "\n"
    }

  str_len += SkDebug::ms_indent_size + param_str.get_length() + 1u
    + code_str.get_length() + 1u;

  str.ensure_size_empty(str_len);
    

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Build code string
  if (durational)
    {
    str.append("^_ ", 3u);
    }
  else
    {
    str.append('^');
    }

  str.append(recv_str);
  str.append('\n');

  // List captured variables as a comment
  if (capture_count)
    {
    str.append(' ', SkDebug::ms_indent_size);
    str.append("// captured: ", captured_str_len);

    if (m_receiver_p == nullptr)
      {
      capture_count--;

      if (capture_count)
        {
        str.append("this, ", 6u);
        }
      else
        {
        str.append("this\n", 5u);
        }
      }
    }

  if (capture_count)
    {
    SkNamedIndexed * sym_p     = info_p->m_captured.get_array();
    SkNamedIndexed * sym_end_p = sym_p + capture_count;

    A_LOOP_INFINITE
      {
      str.append(sym_p->get_name_str_dbg());
      sym_p++;

      if (sym_p < sym_end_p)
        {
        str.append(", ", 2u);
        }
      else
        {
        str.append('\n');

        // Quit loop
        break;
        }
      }
    }

  str.append(' ', SkDebug::ms_indent_size);
  str.append(param_str);
  str.append('\n');
  str.append(code_str);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


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
SkExpressionBase * SkLiteralClosure::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  // Stopping at the start of a closure literal is useful since you may want to break just
  // prior to its creation/instantiation when it captures any need variables.
  if ((type <= SkExprFind_interesting) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
    {
    return const_cast<SkLiteralClosure *>(this);
    }

  // Receiver is executed when making a closure instance.
  if (m_receiver_p)
    {
    SkExpressionBase * expr_p = m_receiver_p->find_expr_by_pos(pos, type);

    if (expr_p)
      {
      return expr_p;
      }
    }

  // Closure expression is executed only when the closure instance is invoked.
  return m_info_p->get_closure_expr()->find_expr_by_pos(pos, type);
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
SkLiteralClosure::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || (m_receiver_p && m_receiver_p->iterate_expressions(apply_expr_p, invokable_p))
    || m_info_p->get_invokable()->iterate_expressions(apply_expr_p))
    {
    return AIterateResult_early_exit;
    }

  return AIterateResult_entire;
  }

#endif


//---------------------------------------------------------------------------------------
// Evaluates literal list expression and returns an instance version of
//             itself.
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
// Notes:      No caller object is needed since SkLiteral always returns the result
//             immediately.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkLiteralClosure::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SkClosureInfoBase * info_p = m_info_p;

  #if (SKOOKUM & SK_DEBUG)
    //SkDebug::print_ide(as_code());
    //SkDebug::print_ide("\n\n");

    // Ensure code is available.
    if (info_p == nullptr)
      {
      SK_ERROR_INFO(
        "Code where closure was defined is no longer available!\n"
        "[It may have been a demand loaded class that was unloaded.]",
        caller_p);

      if (result_pp)
        {
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return nullptr;
      }
  #endif

  // Note that the receiver is always an expression that returns immediately and is never
  // a coroutine call expression.
  SkInstance * receiver_p = m_receiver_p
    ? m_receiver_p->invoke_now(scope_p, caller_p)  // evaluate receiver expression
    : scope_p->get_topmost_scope();                // 'this' is inferred

  if (result_pp)
    {
    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);
    // Call to SKDEBUG_HOOK_EXPR()?

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Capture any referenced variables
    SkClosure * closure_p = SkClosure::new_instance(info_p, receiver_p);

    *result_pp = closure_p;

    uint32_t capture_count = info_p->get_captured().get_count();

    if (capture_count)
      {
      SkInstance **    captures_pp     = closure_p->get_captured_array();
      SkInstance **    captures_end_pp = captures_pp + capture_count;
      SkNamedIndexed * capture_name_p  = info_p->get_captured().get_array();

      SK_ASSERTX(scope_p->get_obj_type() == SkObjectType_invoked_context, "SkLiteralClosure must be invoked with a SkInvokedContextBase scope.");
      SkInvokedContextBase * invoked_p = static_cast<SkInvokedContextBase *>(scope_p);

      while (captures_pp < captures_end_pp)
        {
        SK_ASSERTX(capture_name_p->get_data_idx() < invoked_p->get_data().get_size(), "Access out of range");
        (*captures_pp) = invoked_p->get_data().get_array()[capture_name_p->get_data_idx()];
        (*captures_pp)->reference();
        captures_pp++;
        capture_name_p++;
        }
      }
    }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean-up receiver created by expression
  if (m_receiver_p)
    {
    receiver_p->dereference();
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteralClosure::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkLiteralClosure, SkDebugInfo_size_used));

  if (m_receiver_p)
    {
    m_receiver_p->track_memory(mem_stats_p);
    }

  SkClosureInfoBase * info_p = m_info_p;

  if (info_p)
    {
    info_p->track_memory(mem_stats_p);
    }
  }

