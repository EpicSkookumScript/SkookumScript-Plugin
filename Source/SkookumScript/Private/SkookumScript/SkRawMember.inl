// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Raw member access functionality
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkInvocation.hpp>

//=======================================================================================
// SkRawMemberInfo Inline Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
A_INLINE SkRawMemberInfo::SkRawMemberInfo(const void ** binary_pp) :
  // 4 bytes - owner class
  m_owner_class_p(SkClass::from_binary_ref(binary_pp)),
  // 2 bytes - data_idx
  m_data_idx(A_BYTE_STREAM_UI16_INC(binary_pp))
  {}

#endif

//---------------------------------------------------------------------------------------

A_INLINE const SkTypedNameRaw * SkRawMemberInfo::get_typed_name() const
  {
  return m_owner_class_p->get_instance_data_raw()[m_data_idx];
  }

//=======================================================================================
// SkRawMemberHandle Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------

A_INLINE SkRawMemberHandle::SkRawMemberHandle(const SkRawMemberInfo & raw_member_info, SkInstance * owner_p)
  {
  m_typed_name_p = raw_member_info.get_typed_name(); // Get info about the member
  m_obj_p = raw_member_info.m_owner_class_p->get_raw_pointer(owner_p); // Pointer to raw memory of the object containing the data member
  SK_ASSERTX(m_obj_p, a_str_format("Tried to access %s.%s but the instance is null!", owner_p->get_class()->get_name_cstr(), m_typed_name_p->get_name_cstr()));
  }

//---------------------------------------------------------------------------------------

A_INLINE SkInstance * SkRawMemberHandle::new_instance() const
  {
  if (m_obj_p)
    {
    SkClassDescBase * data_type_p = m_typed_name_p->m_type_p; // Type of the data member itself
    return data_type_p->get_key_class()->new_instance_from_raw_data(m_obj_p, m_typed_name_p->m_raw_data_info, data_type_p);
    }

  return SkBrain::ms_nil_p;
  }

//---------------------------------------------------------------------------------------

A_INLINE void SkRawMemberHandle::assign(SkInstance * value_p) const
  {
  if (m_obj_p)
    {
    SkClassDescBase * data_type_p = m_typed_name_p->m_type_p; // Type of the data member itself
    data_type_p->get_key_class()->assign_raw_data(m_obj_p, m_typed_name_p->m_raw_data_info, data_type_p, value_p);
    }
  }

//=======================================================================================
// SkRawMemberEvaluator Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Upon construction, retrieve and store instance
A_INLINE SkRawMemberEvaluator::SkRawMemberEvaluator(const SkRawMemberInfo & raw_member_info, SkInstance * owner_p)
  : SkRawMemberHandle(raw_member_info, owner_p)
  , m_member_instance_p(new_instance())
  {
  }

//---------------------------------------------------------------------------------------
// Upon destruction, assign instance back to owner, then destroy it
A_INLINE SkRawMemberEvaluator::~SkRawMemberEvaluator()
  {
  assign(m_member_instance_p);
  m_member_instance_p->dereference();
  }

//=======================================================================================
// SkRawMemberBase Inline Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - owner expression typed binary or inferred this (nullptr)
//               6 bytes - member info
//               n bytes - raw owner cascade
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkRawMemberBase::SkRawMemberBase(const void ** binary_pp) :
  // n bytes - owner expression typed binary or inferred this (nullptr)
  m_owner_expr_p(SkExpressionBase::from_binary_typed_new(binary_pp)),
  // 6 bytes - member info
  m_member_info(binary_pp)
  {
  // 1 byte - number of enclosing raw owners
  uint32_t length = A_BYTE_STREAM_UI8_INC(binary_pp);
  m_raw_owner_cascade.empty_ensure_count_undef(length);
  // 6 bytes repeating for each SkRawMemberInfo
  for (; length > 0u; length--)
    {
    m_raw_owner_cascade.append_last_undef(SkRawMemberInfo(binary_pp));
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//=======================================================================================
// SkRawMemberAssignment Inline Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - value expression typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkRawMemberAssignment::SkRawMemberAssignment(const void ** binary_pp) :
  // n bytes - owner expression typed binary or inferred this (nullptr)
  SkRawMemberBase(binary_pp),
  // n bytes - owner expression typed binary or inferred this (nullptr)
  m_value_expr_p(SkExpressionBase::from_binary_typed_new(binary_pp))
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


//=======================================================================================
// SkRawMemberModifyingInvocation Inline Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - SkRawMemberBase
//               n bytes - call typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
A_INLINE SkRawMemberModifyingInvocation::SkRawMemberModifyingInvocation(const void ** binary_pp) :
  // n bytes - owner expression typed binary or inferred this (nullptr)
  SkRawMemberBase(binary_pp),
  // n bytes - call
  m_call_p(SkInvokeBase::from_binary_typed_new(binary_pp))
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


