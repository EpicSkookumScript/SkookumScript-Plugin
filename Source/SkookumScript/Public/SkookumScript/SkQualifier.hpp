// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Qualifier class - full qualification consisting of member name and owner class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkNamed.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

class SkClass;  // Pre-declaration


//---------------------------------------------------------------------------------------
// Notes      Qualifier class - full qualification consisting of member name and owner class
// Subclasses SkInvokableBase(SkMethodBase, SkCoroutineBase),
//            SkInvokeBase(SkMethodCallBase, SkCoroutineCall)
// See Also   SkTypedName
// Author(s)  Conan Reis
class SK_API SkQualifier : public SkNamedIndexed
  {
  friend class SkQualifierCompareName;

  public:

    static const int16_t ms_invalid_vtable_index = -1;

    SK_NEW_OPERATORS(SkQualifier);
  // Common Methods

    explicit SkQualifier(const ASymbol & name = ASymbol::get_null(), SkClass * scope_p = nullptr, int16_t vtable_index = ms_invalid_vtable_index);
    SkQualifier(const SkQualifier & source) : SkNamedIndexed(source), m_scope_p(source.m_scope_p) {}

    SkQualifier & operator=(const SkQualifier & source) { m_name = source.m_name; m_scope_p = source.m_scope_p; return *this; }

  // Conversion Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkQualifier(const void ** binary_pp);
      void assign_binary(const void ** binary_pp);
    #endif

    #if (SKOOKUM & SK_COMPILED_OUT)
      void     as_binary(void ** binary_pp) const;
      uint32_t as_binary_length() const { return SkNamedIndexed::as_binary_length() + 4u; }
    #endif

  // Comparison Methods

    bool operator==(const SkQualifier & qual) const;
    bool operator<(const SkQualifier & qual) const;
    bool equal_ids(const SkQualifier & qual) const;
    bool equal_ids_scope_name(const SkQualifier & qual) const;
    bool less_ids_scope_name(const SkQualifier & qual) const;
    bool less_ids_name_scope(const SkQualifier & qual) const;

  // Methods

    SkClass * get_scope() const;
    void      set_scope(const SkClass * scope_p);
    int16_t   get_vtable_index() const                { return m_data_idx; }
    void      set_vtable_index(int16_t vtable_index)  { m_data_idx = vtable_index; }
    void      invalidate()                            { m_name.set_null(); m_data_idx = ms_invalid_vtable_index; m_scope_p = nullptr; }

    // Data Methods - only valid if qualifier represents data member

      SkClassDescBase * get_data_type() const;

  // Class Methods

  protected:

    friend struct SkMemberReference;
    friend struct SkMatchReference;

    SkQualifier ** get_pool_unused_next() { return (SkQualifier **)&m_scope_p; } // Area in this class where to store the pointer to the next unused object when not in use

  // Data Members

    // $Revisit - CReis Should probably be SkClassUnaryBase so it could be either
    // SkClass/SkActorClass or SkMetaClass
    SkClass * m_scope_p;

  };  // SkQualifier


//---------------------------------------------------------------------------------------
// This is passed as a second argument to various template classes such as APArray and
// APSorted to provide a mechanism for logical sorting of elements.
//
// Does a logical comparison of the name and then the scope.
class SK_API SkQualifierCompareName
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const SkQualifier & lhs, const SkQualifier & rhs)     { return (lhs.m_name == rhs.m_name) && (lhs.m_scope_p == rhs.m_scope_p); }

    // Returns 0 if equal, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const SkQualifier & lhs, const SkQualifier & rhs);
  };


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkQualifier.inl>
#endif
