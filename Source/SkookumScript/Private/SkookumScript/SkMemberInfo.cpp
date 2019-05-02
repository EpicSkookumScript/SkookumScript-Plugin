// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Member Identifier class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkMemberInfo.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkMemberInfo.inl>
#endif
#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokedBase.hpp>


//=======================================================================================
// SkMemberInfo Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Gets class scope - a SkMetaClass if it is a class member and SkClass if it
//             is an instance member.
// Notes:      Would inline though requires SkClass.hpp to be included which causes problems.
// Author(s):   Conan Reis
SkClassUnaryBase * SkMemberInfo::get_class_scope() const
  {
  return m_class_scope
    ? (SkClassUnaryBase *)&m_member_id.get_scope()->get_metaclass()
    : (SkClassUnaryBase *)m_member_id.get_scope();
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Set the member based on the supplied context.
// Arg         scope_p - scope for the expression
// Arg         caller_p - invokable for the expression
// Author(s):   Conan Reis
void SkMemberInfo::set_context(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p
  )
  {
  SkInvokedContextBase * context_p   = scope_p->get_scope_context();
  SkInvokableBase *      member_p    = context_p->get_invokable();
  SkInstance *           top_scope_p = context_p->get_topmost_scope();

  m_member_id   = *member_p;
  m_type        = member_p->get_member_type();
  m_class_scope = top_scope_p->is_metaclass();
  m_is_closure  = !!member_p->get_closure_info();
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Constructor from binary stream
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary stream and to increment
//             - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - member class
//               4 bytes - member identifier name
//               1 byte  - member type (+ class scope in high bit)
// Author(s):   Conan Reis
SkMemberInfo::SkMemberInfo(const void ** binary_pp)
  {
  // 4 bytes - member class
  m_member_id.set_scope(SkClass::from_binary_ref(binary_pp));

  // 4 bytes - member identifier name
  m_member_id.set_name(ASymbol::create_from_binary(binary_pp));

  // 1 byte - member type (+ class scope in high bit)
  uint8_t member_bits = A_BYTE_STREAM_UI8_INC(binary_pp);

  m_type = eSkMember(member_bits & ByteFlag__type_mask);

  m_class_scope = (member_bits & ByteFlag_class_member) != 0u;

  m_is_closure = (member_bits & ByteFlag_closure) != 0u;
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with binary information needed to
//             recreate this literal and its components and increments the memory address
//             to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the as_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - member class
//               4 bytes - member identifier name
//               1 byte  - member type (+ class scope in high bit)
// Author(s):   Conan Reis
void SkMemberInfo::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkMemberInfo::as_binary_length());

  // 4 bytes - member class
  m_member_id.get_scope()->as_binary_ref(binary_pp);

  // 4 bytes - member identifier name
  m_member_id.get_name().as_binary(binary_pp);

  // 1 byte - member type (+ class scope in high bit)
  uint8_t member_bits = uint8_t(m_type);

  // Stuff in class member info
  if (m_class_scope)
    {
    member_bits |= ByteFlag_class_member;
    }

  if (m_is_closure)
    {
    member_bits |= ByteFlag_closure;
    }

  A_BYTE_STREAM_OUT8(binary_pp, &member_bits);
  }

//---------------------------------------------------------------------------------------

uint32_t SkMemberInfo::as_binary_length() const
  {
  return SkClass::Binary_ref_size + 4u + 1u;
  }

#endif


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this member info to its file title/name equivalent with optional
//             scope as a prefix.
// Arg         flags - see ePathFlag
// Author(s):   Conan Reis
AString SkMemberInfo::as_file_title(
  ePathFlag flags // = PathFlag_scope
  ) const
  {
  AString name(nullptr, 64u, 0u);

  if (is_valid())
    {
    if ((flags & PathFlag_scope) && m_member_id.get_scope())
      {
      // Add class scope
      name.append(m_member_id.get_scope()->get_name_str_dbg());
      name.append('@');
      }

    // Add name
    switch (m_type)
      {
      case SkMember_data:
        name.append("!Data", 5u);
        break;

      case SkMember_class_meta:
        name.append("!Class", 6u);
        break;

      case SkMember_method:
      case SkMember_method_func:
      case SkMember_method_mthd:
        {
        name.append(m_member_id.get_name_str_dbg());
        char *   cstr_a      = name.as_cstr_writable();
        uint32_t name_length = name.get_length();

        // Substitute '-Q' for query/predicate methods since filenames cannot use a 
        // question mark '?'.
        if ((flags & PathFlag_translate) && (cstr_a[name_length - 1u] == '?'))
          {
          cstr_a[name_length - 1u] = '-';
          name.append("Q()", 3u);
          }
        else
          {
          name.append("()", 2u);
          }

        break;
        }
       
      //case SkMember_coroutine:
      //case SkMember_coroutine_func:
      //case SkMember_coroutine_mthd:
      default:
        name.append(m_member_id.get_name_str_dbg());
        name.append("()", 2u);
      }

    if (m_class_scope && (m_type != SkMember_class_meta))
      {
      name.append('C');
      }

    if (flags & PathFlag_extension)
      {
      if (m_type == SkMember_class_meta)
        {
        name.append(".sk-meta", 8u);
        }
      else
        {
        name.append(".sk", 3u);
        }
      }
    }

  return name;
  }

#endif  // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Finds invokable that this member info refers to.
// Returns:    An SkInvokableBase or nullptr
// Author(s):   Conan Reis
SkInvokableBase * SkMemberInfo::as_invokable() const
  {
  SkClass * scope_p = m_member_id.get_scope();

  if (scope_p)
    {
    switch (m_type)
      {
      case SkMember_method:
      case SkMember_method_func:  // Atomic method (C++ function)
      case SkMember_method_mthd:  // Atomic method (C++ method)
        return m_class_scope
          ? scope_p->find_class_method(m_member_id.get_name())
          : scope_p->find_instance_method(m_member_id.get_name());

      case SkMember_coroutine:
      case SkMember_coroutine_func:  // Atomic coroutine (C++ function)
      case SkMember_coroutine_mthd:  // Atomic coroutine (C++ method)
        return scope_p->find_coroutine(m_member_id.get_name());
          
      default: break; // Make Clang happy
      }
    }

  return nullptr;
  }


#if (SKOOKUM & SK_DEBUG)

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
// See:        SkExpressionBase::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkMemberInfo::get_body_expr() const
  {
  SkInvokableBase * invokable_p = as_invokable();

  return invokable_p ? invokable_p->get_custom_expr() : nullptr;
  }

//---------------------------------------------------------------------------------------
// Finds within this expression the sub-expression (or this expression itself)
//             at/following the specified source index position
// Returns:    Expression at/following specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         type - type of expression to look for - see eSkExprFind
// See:        SkExpressionBase::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkMemberInfo::find_expr_by_pos(
  uint32_t    source_idx,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkInvokableBase * invokable_p = as_invokable();

  return invokable_p ? invokable_p->find_expr_by_pos(source_idx, type) : nullptr;
  }

//---------------------------------------------------------------------------------------
// Finds within this expression the sub-expression (or this expression itself)
//             at/on the specified source index position
// Returns:    Expression at/on specified index or nullptr
// Arg         source_idx - index to look for expression
// Arg         type - type of expression to look for - see eSkExprFind
// See:        SkExpressionBase::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkMemberInfo::find_expr_on_pos(
  uint32_t    source_idx,
  eSkExprFind type  // = SkExprFind_all
  ) const
  {
  SkInvokableBase * invokable_p = as_invokable();

  return invokable_p ? invokable_p->find_expr_on_pos(source_idx, type) : nullptr;
  }

//---------------------------------------------------------------------------------------
// Finds last (by index order) subexpression in the body expression
SkExpressionBase * SkMemberInfo::find_expr_by_pos_last() const
  {
  SkInvokableBase * invokable_p = as_invokable();
  if (invokable_p)
    {
    SkExpressionBase * body_expr_p = invokable_p->get_custom_expr();
    if (body_expr_p)
      {
      return body_expr_p->find_expr_by_pos_last();
      }
    }

  return nullptr;
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
// See:        SkExpressionBase::find_expr_span()
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkMemberInfo::find_expr_span(
  uint32_t    source_idx,
  uint32_t *      begin_idx_p,  // = nullptr
  uint32_t *      end_idx_p,    // = nullptr
  eSkExprFind type          // = SkExprFind_all
  ) const
  {
  SkInvokableBase * invokable_p = as_invokable();

  return invokable_p ? invokable_p->find_expr_span(source_idx, begin_idx_p, end_idx_p, type) : nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines the beginning and ending index span of the specified expression
//             in this member.
// Arg         expr - expression to determine span of
// Arg         begin_idx_p - starting index of the expression
// Arg         end_idx_p - next index following expression (may be start of last
//             sub-expression or beginning of next non-sub-expression)
// Modifiers:   static
// Author(s):   Conan Reis
void SkMemberInfo::get_expr_span(
  const SkExpressionBase & expr,
  uint32_t *                   begin_idx_p,
  uint32_t *                   end_idx_p
  ) const
  {
  SkInvokableBase * invokable_p = as_invokable();

  if (invokable_p)
    {
    invokable_p->get_expr_span(expr, begin_idx_p, end_idx_p);
    }
  }

#endif  // (SKOOKUM & SK_DEBUG)
