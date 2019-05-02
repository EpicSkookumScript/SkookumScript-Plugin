// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// SkookumScript C++ library.
//
// Member Identifier class
// Author(s):   Conan Reis
// Notes:          
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================


//=======================================================================================
// SkMemberInfo Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Equals operator
// Returns:    true if logically equal, false if not
// Author(s):   Conan Reis
A_INLINE bool SkMemberInfo::operator==(const SkMemberInfo & info) const
  {
  return ((m_type == info.m_type)
    && ((m_type == SkMember__invalid)
      || ((m_class_scope == info.m_class_scope) /*&& (m_is_closure == info.m_is_closure)*/ && (m_member_id == info.m_member_id)))); 
  }

//---------------------------------------------------------------------------------------
// Less than operator - sort by type -> member_id(scope name -> name) -> class scope
// Returns:    true if logically less than, false if not
// Author(s):   Conan Reis
A_INLINE bool SkMemberInfo::operator<(const SkMemberInfo & info) const
  {
  return ((m_type < info.m_type)
       || ((m_type == info.m_type)
         && (m_type != SkMember__invalid)
         && (m_member_id.less_ids_scope_name(info.m_member_id)
           || (m_member_id.equal_ids_scope_name(info.m_member_id)
             && ((m_class_scope < info.m_class_scope)
               /*|| (m_class_scope == info.m_class_scope
                 && (m_is_closure < info.m_is_closure))*/)))));
  }

