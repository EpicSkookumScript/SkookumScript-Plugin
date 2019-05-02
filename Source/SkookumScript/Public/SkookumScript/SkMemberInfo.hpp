// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Member Identifier class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkQualifier.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkClassUnaryBase;
class SkExpressionBase;
class SkInvokableBase;
class SkInvokedBase;
class SkObjectBase;
struct SkOverlay;


//---------------------------------------------------------------------------------------
// Used to find expressions by source index position
enum eSkExprFind
  {
  SkExprFind_all,           // Find all expressions
  SkExprFind_interesting,   // Find all interesting expressions (expressions that do some sort of work)
  SkExprFind_invoke         // Find all expressions that call methods/coroutines
  };


//---------------------------------------------------------------------------------------
// Stores context needed to identify particular class or class member
// 
// Subclasses: SkContextInfo
struct SK_API SkMemberInfo
  {
  // Nested Structures

    enum ePathFlag
      {
      PathFlag_scope        = 1 << 0,
      PathFlag_extension    = 1 << 1,
      PathFlag_translate    = 1 << 2, // Convert invalid file characters like ? to -Q

      PathFlag__none        = 0x0,
      PathFlag__file        = PathFlag_extension | PathFlag_translate,
      PathFlag__scope_ext   = PathFlag_scope | PathFlag_extension,
      };

    // Used when converting this structure to and from a binary stream
    enum eByteFlag
      {
      ByteFlag__type_mask   = 0x3f,

      ByteFlag_closure      = 1 << 6,
      ByteFlag_class_member = 1 << 7,
      };

  // Public Data

    SkQualifier m_member_id;
    eSkMember   m_type;
    bool        m_class_scope;  // true = class member,  false = instance member
    bool        m_is_closure;   // true if this actually refers to a closure within the member


  // Methods

    //SK_NEW_OPERATORS(SkMemberInfo);

    SkMemberInfo()                                                                                          : m_type(SkMember__invalid), m_class_scope(false), m_is_closure(false) {}
    SkMemberInfo(const SkMemberInfo & info)                                                                 : m_member_id(info.m_member_id), m_type(info.m_type), m_class_scope(info.m_class_scope), m_is_closure(info.m_is_closure) {}
    SkMemberInfo(const SkQualifier & member_id, eSkMember type, bool class_scope, bool is_closure = false)  : m_member_id(member_id), m_type(type), m_class_scope(class_scope), m_is_closure(is_closure)  {}
    
    SkMemberInfo & operator=(const SkMemberInfo & info)   { m_member_id = info.m_member_id; m_type = info.m_type; m_class_scope = info.m_class_scope; m_is_closure = info.m_is_closure; return *this; }

    // Comparison Methods

      bool operator==(const SkMemberInfo & info) const;
      bool operator!=(const SkMemberInfo & info) const    { return !operator==(info); }
      bool operator<(const SkMemberInfo & info) const;

    // Accessor & Conversion Methods

      SkInvokableBase  * as_invokable() const;
      SkClass          * get_class() const                { return m_member_id.get_scope(); }
      SkClassUnaryBase * get_class_scope() const;
      bool               is_valid() const                 { return m_type != SkMember__invalid; }
      void               invalidate()                     { m_type = SkMember__invalid; m_member_id.invalidate(); }
      void               set_class(SkClass * scope_p)     { return m_member_id.set_scope(scope_p); }

      operator const SkQualifier & () const               { return m_member_id; }

      #if defined(SK_AS_STRINGS)
        AString as_file_title(ePathFlag flags = PathFlag_scope) const;
      #endif

      #if (SKOOKUM & SK_DEBUG)
        SkExpressionBase * find_expr_by_pos(uint32_t pos, eSkExprFind type = SkExprFind_all) const;
        SkExpressionBase * find_expr_on_pos(uint32_t pos, eSkExprFind type = SkExprFind_all) const;
        SkExpressionBase * find_expr_by_pos_last() const;
        SkExpressionBase * find_expr_span(uint32_t source_idx, uint32_t * idx_begin_p = nullptr, uint32_t * idx_end_p = nullptr, eSkExprFind type = SkExprFind_all) const;
        SkExpressionBase * get_body_expr() const;
        void               get_expr_span(const SkExpressionBase & expr, uint32_t * idx_begin_p, uint32_t * idx_end_p) const;
        void               set_context(SkObjectBase * scope_p, SkInvokedBase * caller_p);
      #endif

    // Binary Stream Methods

      SkMemberInfo(const void ** binary_pp);

      #if (SKOOKUM & SK_COMPILED_OUT)
        void     as_binary(void ** binary_pp) const;
        uint32_t as_binary_length() const;
      #endif

  };  // SkMemberInfo


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkMemberInfo.inl>
#endif
