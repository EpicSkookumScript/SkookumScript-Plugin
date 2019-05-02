// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Identifier for named code objects - i.e. temporary variables, etc.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// SkIdentifierLocal Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         identifier_name - Name of a parameter variable or a temporary variable to
//             identify/refer to.
// Examples:   Called by SkParser
// See:        SkParser
// Author(s):   Conan Reis
A_INLINE SkIdentifierLocal::SkIdentifierLocal(const ASymbol & ident_name, uint32_t data_idx) :
  SkNamedIndexed(ident_name, data_idx)
  {
  }

//---------------------------------------------------------------------------------------
// Sets the variable that is identified by this identifier expression to the
// supplied instance, assuming the variable was previously *uninitialized*.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Modifiers:   virtual
// Author(s):   Conan Reis
A_INLINE void SkIdentifierLocal::bind_data(
  SkInstance *    obj_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p,
  bool            return_result // = true
  )
  {
  SkInvokedContextBase * invoked_p = scope_p->get_scope_context();
  invoked_p->bind_arg_and_ref(m_data_idx, obj_p);
  }

//=======================================================================================
// SkIdentifierMember Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         identifier_name - Name of a member data variable to identify/refer to.
// Examples:   Called by SkParser
// See:        SkParser
// Author(s):   Conan Reis
A_INLINE SkIdentifierMember::SkIdentifierMember(
  const ASymbol &    member_name,
  uint32_t           data_idx,
  SkExpressionBase * owner_p
  ) :
  SkIdentifierLocal(member_name, data_idx),
  m_owner_expr_p(owner_p)
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
//               6 bytes - SkIdentifierLocal
//               n bytes - owner expression typed binary or inferred this (nullptr)
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkIdentifierMember::SkIdentifierMember(const void ** binary_pp) :
  // 6 bytes - identifier name id + data index
  SkIdentifierLocal(binary_pp)
  {
  // n bytes - owner expression typed binary or inferred this (nullptr)
  m_owner_expr_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

//=======================================================================================
// SkIdentifierRawMember Inline Methods
//=======================================================================================

A_INLINE SkIdentifierRawMember::SkIdentifierRawMember(const void ** binary_pp)
  : SkIdentifierMember(binary_pp)
  , m_owner_class_p(SkClass::from_binary_ref(binary_pp))
  {
  }

//=======================================================================================
// SkIdentifierClassMember Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         identifier_name - Name of a member data variable to identify/refer to.
// Examples:   Called by SkParser
// See:        SkParser
// Author(s):   Conan Reis
A_INLINE SkIdentifierClassMember::SkIdentifierClassMember(
  const ASymbol &    class_member_name,
  uint32_t           data_idx,
  SkClass *          owner_class_p
  ) :
  SkIdentifierLocal(class_member_name, data_idx), m_owner_class_p(owner_class_p)
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
//               6 bytes - SkIdentifierLocal
//               4 bytes - owner class reference
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkIdentifierClassMember::SkIdentifierClassMember(const void ** binary_pp) :
  SkIdentifierLocal(binary_pp)
  {
  // 4 bytes - owner class reference
  m_owner_class_p = SkClass::from_binary_ref(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

