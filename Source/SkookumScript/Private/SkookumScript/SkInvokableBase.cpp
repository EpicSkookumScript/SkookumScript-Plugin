// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Invokable parameters & body class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInvokableBase.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkInvokableBase.inl>
#endif
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkExpressionBase.hpp>

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif



//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Virtual destructor - ensures calling of proper destructor for subclasses.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkInvokableBase::~SkInvokableBase()
  {
  }

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of method
// Arg         scope_p - class scope to use
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//               2 bytes - invoked data array size
//               4 bytes - annotation flags
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkInvokableBase::SkInvokableBase(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  SkInvokableBase(name, scope_p, 0u, 0u)
  {
  // n bytes - parameter list
  m_params_p = SkParameters::get_or_create(binary_pp);

  // 2 bytes - invoked data array size
  m_invoked_data_array_size = A_BYTE_STREAM_UI16_INC(binary_pp);

  // 4 bytes - annotation flags
  m_annotation_flags = A_BYTE_STREAM_UI32_INC(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Assignment from binary
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//               2 bytes - invoked data array size
//               4 bytes - annotation flags
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkInvokableBase::assign_binary_no_name(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p)
  {
  // Save previous state if desired
  copy_to_update_record(update_record_p);

  // n bytes - parameter list
  m_params_p = SkParameters::get_or_create(binary_pp);

  // 2 bytes - invoked data array size
  m_invoked_data_array_size = A_BYTE_STREAM_UI16_INC(binary_pp);

  // 4 bytes - annotation flags
  m_annotation_flags = A_BYTE_STREAM_UI32_INC(binary_pp);
  }

//---------------------------------------------------------------------------------------

void SkInvokableBase::copy_to_update_record(SkRoutineUpdateRecord * update_record_p)
  {
  if (update_record_p)
    {
    update_record_p->m_routine_p = this;
    update_record_p->m_previous_params_p = m_params_p;
    update_record_p->m_previous_invoked_data_array_size = m_invoked_data_array_size;
    update_record_p->m_previous_annotation_flags = m_annotation_flags;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this method and its components and increments the memory
//             address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - name id
//               n bytes - parameter list
//               2 bytes - invoked data array size
//               4 bytes - annotation flags
//
//             Note that the scope is implied by the class that this method is
//             contained in.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
void SkInvokableBase::as_binary(void ** binary_pp, bool include_name) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkInvokableBase::as_binary_length(include_name));

  // 4 bytes - name id
  if (include_name)
    {
    m_name.as_binary(binary_pp);
    }

  // n bytes - parameter list
  m_params_p->as_binary(binary_pp);

  // 2 bytes - invoked data array size
  A_BYTE_STREAM_OUT16(binary_pp, &m_invoked_data_array_size);

  // 4 bytes - annotation flags
  A_BYTE_STREAM_OUT32(binary_pp, &m_annotation_flags);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - name id
//               n bytes - parameter list
//               2 bytes - invoked data array size
//               4 bytes - annotation flags
// Modifiers:   virtual
// Author(s):   Conan Reis
uint32_t SkInvokableBase::as_binary_length(bool include_name) const
  {
  return (include_name ? 10u : 6u) + m_params_p->as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------

const SkClosureInfoBase * SkInvokableBase::get_closure_info() const
  {
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if this invokable has an empty code block = does nothing
bool SkInvokableBase::is_empty() const
  {
  return false;
  }

//---------------------------------------------------------------------------------------
// Returns true if the invokable is a placeholder for parser context
// Returns:    true if the invokable is a placeholder for parser context, false if not
// Modifiers:   virtual
// Author(s):   Conan Reis
bool SkInvokableBase::is_placeholder() const
  {
  return false;
  }

//---------------------------------------------------------------------------------------
// Converts it's name to a descriptive AgogCore/AString.
// Returns:    It's name as a AString - i.e. `ClassScope@_coroutine()` or `_coroutine()`
// Arg         qualified - if true, includes `ClassScope@` in the AgogCore/AString.
// See:        as_code()
// Modifiers:   virtual
// Author(s):   Conan Reis
AString SkInvokableBase::as_string_name(
  bool qualified // = true
  ) const
  {
  AString invoke_name(m_name.as_str_dbg());

  if (qualified)
    {
    AString name(m_scope_p->get_name_str_dbg(), 3u + invoke_name.get_length());
    
    name.append('@');
    name.append(invoke_name);
    name.append("()", 2u);

    return name;
    }
  else
    {
    AString name(invoke_name, 2u);
    
    name.append("()", 2u);

    return name;
    }
  }

//---------------------------------------------------------------------------------------
// Transfer ownership assignment
SkInvokableBase & SkInvokableBase::assign(const SkInvokableBase & invokable)
  {
  SkQualifier::operator=(invokable); 
  
  m_params_p                  = invokable.m_params_p;
  m_invoked_data_array_size   = invokable.m_invoked_data_array_size;
  m_user_data                 = invokable.m_user_data;
  m_annotation_flags          = invokable.m_annotation_flags;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkInvokableBase::get_custom_expr() const
  {
  return nullptr;
  }


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
eSkMember SkInvokableBase::get_member_type() const
  {
  return eSkMember(get_invoke_type());
  }

//---------------------------------------------------------------------------------------
// Finds within this invokable the sub-expression at/following the specified
//             source index position
// Returns:    Expression at/following specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         type - type of expression to look for - see eSkExprFind
// See:        SkExpressionBase::find_expr*()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkInvokableBase::find_expr_by_pos(
  uint32_t    source_idx,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p = m_params_p->find_expr_by_pos(source_idx);

  if (expr_p)
    {
    return expr_p;
    }

  SkExpressionBase * body_expr_p = get_custom_expr();

  if (body_expr_p == nullptr)
    {
    return nullptr;
    }

  return body_expr_p->find_expr_by_pos(source_idx, type);
  }

//---------------------------------------------------------------------------------------
// Finds within this invokable the sub-expression at/on the specified source
//             index position
// Returns:    Expression at/on specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         type - type of expression to look for - see eSkExprFind
// See:        SkExpressionBase::find_expr*()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkInvokableBase::find_expr_on_pos(
  uint32_t    source_idx,
  eSkExprFind type  // = SkExprFind_all
  ) const
  {
  // *** Similar to SkExpressionBase::find_expr_on_pos()
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
// Determines the beginning and ending index span of the specified expression
//             in this invokable.
// Arg         expr - expression to determine span of
// Arg         begin_idx_p - starting index of the expression
// Arg         end_idx_p - next index following expression (may be start of last
//             sub-expression or beginning of next non-sub-expression)
// See:        SkExpressionBase::find_expr*()
// Modifiers:   static
// Author(s):   Conan Reis
void SkInvokableBase::get_expr_span(
  const SkExpressionBase & expr,
  uint32_t *                   begin_idx_p,
  uint32_t *                   end_idx_p
  ) const
  {
  // *** Similar to SkExpressionBase::get_expr_span()
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
// Finds within this invokable the sub-expression at the specified source
//             index position and optionally determines the beginning and ending index
//             span of the found expression.
// Returns:    Expression at specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         begin_idx_p - starting index of the expression (may be position of
//             sub-expression)
// Arg         end_idx_p - next index following expression (may be start of last
//             sub-expression or beginning of next non-sub-expression)
// See:        SkExpressionBase::find_expr*()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkInvokableBase::find_expr_span(
  uint32_t    source_idx,
  uint32_t *      begin_idx_p,  // = nullptr
  uint32_t *      end_idx_p,    // = nullptr
  eSkExprFind type          // = SkExprFind_all
  ) const
  {
  // *** Similar to SkExpressionBase::find_expr_span()
  // - any changes here should be reflected there as well.

  SkExpressionBase * expr_p = find_expr_on_pos(source_idx, type);

  get_expr_span(*expr_p, begin_idx_p, end_idx_p);

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Iterates through all expressions in this invokable applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkInvokableBase::iterate_expressions(SkApplyExpressionBase * apply_expr_p)
  {
  // Iterate through any default argument expressions in parameters
  if(m_params_p->iterate_expressions(apply_expr_p, this))
    {
    return AIterateResult_early_exit;
    }

  // Iterate through any custom expression body
  SkExpressionBase * body_expr_p = get_custom_expr();

  if (body_expr_p && body_expr_p->iterate_expressions(apply_expr_p, this))
    {
    return AIterateResult_early_exit;
    }

  // Full iteration performed
  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)
