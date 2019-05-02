// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ACompareBase class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes      This is a function callback object.  This class is used in the place of a
//            function argument since it can also use data included with it in any
//            evaluations that take place (if derived).  The fact that the data that the
//            function acts upon is stored with this functional object is especially
//            important for concurrent processing in that it does not rely on global data
//            and thus may operate safely in more than one thread of execution
//            simultaneously.
// Subclasses ACompare<>, ACompareMethod<>
// Uses       
// InLib      AgogCore/AgogCore.lib
// Author(s)  Conan Reis
template<class _ElementType>
class ACompareBase
  {
  public:
  // Common types

    typedef ACompareBase<_ElementType> tACompareBase;  // Local shorthand for ACompareBase template

  // Common Methods

    virtual ~ACompareBase();

  // Modifying Methods

    virtual eAEquate compare(const _ElementType & lhs, const _ElementType & rhs) = 0;

  // Non-Modifying Methods

    virtual ACompareBase * copy_new() const = 0;

  };  // ACompareBase


//---------------------------------------------------------------------------------------
// $Vital - CReis This should be incorporated into ACompareLogical
template<class _ElementType>
class ACmpLogical : public ACompareBase<_ElementType>
  {
  public:
  // Instance Methods

    ACmpLogical() {}

    virtual eAEquate                     compare(const _ElementType & lhs, const _ElementType & rhs) override;
    virtual ACompareBase<_ElementType> * copy_new() const override;

  };

//---------------------------------------------------------------------------------------
// This is passed as a second argument to various template classes such as APArray and
// APSorted to provide a mechanism for sorting by memory address values of elements.  No
// special methods are required to be present in the elements for this mechanism to be
// used.
template<class _ElementType>
class ACompareAddress
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const _ElementType & lhs, const _ElementType & rhs)
      {
      return reinterpret_cast<uintptr_t>(&lhs) == reinterpret_cast<uintptr_t>(&rhs);
      }

    // Returns 0 if equal, < 0 if lhs is less than rhs and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const _ElementType & lhs, const _ElementType & rhs)
      {
      return reinterpret_cast<uintptr_t>(&lhs) - reinterpret_cast<uintptr_t>(&rhs);
      }
  };


//---------------------------------------------------------------------------------------
// This is passed as a second argument to various template classes such as APArray and
// APSorted to provide a mechanism for logical sorting of elements.  Any elements using
// this sorting mechanism must have a == and < operator.
//
// See Also Specialization ACompareLogical<AString> and the class template
//          ACompareStrInsensitive defined in AgogCore/AString.hpp
template<class _ElementType>
class ACompareLogical : public ACompareBase<_ElementType>
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const _ElementType & lhs, const _ElementType & rhs)
      {
      return lhs == rhs;
      }

    // Returns 0 if equal, < 0 if lhs is less than rhs and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const _ElementType & lhs, const _ElementType & rhs)
      {
      return ((lhs < rhs) ? -1 : 1 - static_cast<ptrdiff_t>(lhs == rhs));
      }
  };


//---------------------------------------------------------------------------------------
// This is passed as a second argument to various template classes such as APArray and
// APSorted to provide a mechanism for descending (reverse) order logical sorting of
// elements.  Any elements using this sorting mechanism must have a == and < operator.
template<class _ElementType>
class ACompareLogicalReverse
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const _ElementType & lhs, const _ElementType & rhs)
      {
      return lhs == rhs;
      }

    // Returns 0 if equal, < 0 if lhs is less than rhs and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const _ElementType & lhs, const _ElementType & rhs)
      {
      return ((lhs < rhs) ? 1 : -1 + static_cast<ptrdiff_t>(lhs == rhs));
      }

  };


//=======================================================================================
// ACompareBase Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// virtual Destructor - Ensures that the proper destructor call is made for
//             derived classes.
// Examples:   called by system
// Author(s):   Conan Reis
template<class _ElementType>
ACompareBase<_ElementType>::~ACompareBase()
  {
  // Does nothing
  }


//=======================================================================================
// ACmpLogical Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Compares two elements using the standard comparison operators < and ==.
// Arg         lhs - left hand side element to compare
// Arg         rhs - right hand side element to compare
// Examples:   compare_obj.compare();
// Modifiers:   virtual - overridden from ACompareBase
// Author(s):   Conan Reis
template<class _ElementType>
eAEquate ACmpLogical<_ElementType>::compare(
  const _ElementType & lhs,
  const _ElementType & rhs
  )
  {
  return (lhs < rhs) ? AEquate_less : eAEquate(1 - static_cast<int>(lhs == rhs));
  }

//---------------------------------------------------------------------------------------
// Makes a dynamic copy of itself
// Modifiers:   virtual - overridden from ACompareBase
// Author(s):   Conan Reis
template<class _ElementType>
ACompareBase<_ElementType> * ACmpLogical<_ElementType>::copy_new() const
  {
  return new ACmpLogical;
  }
