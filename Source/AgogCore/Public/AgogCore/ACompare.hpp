// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ACompare class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ACompareBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    This is a comparison function object template for static functions used for
//          objects that require a dynamic sorting mechanism such as the APSorted class
//          template.  The ACompare template class is used in the place of a comparison
//          function argument since it can also use its m_extra_data object which is
//          passed to the internally stored comparison function whenever it is
//          called/invoked.  The fact that the data that the function acts upon is stored
//          within the ACompare object is especially important for concurrent processing
//          in that it does not rely on global data and thus may operate safely in more
//          than one thread of execution simultaneously.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class ACompare<AString>;
// Arg      _ElementType - the class/type of elements to be sorted
// Arg      _ExtraDataType - the class/type of the extra data element to be stored and
//          passed to each invocation of the comparison function.  (Default void *)
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Examples:    
// Author   Conan Reis
template<class _ElementType, class _ExtraDataType = void *>
class ACompare : public ACompareBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand
    typedef ACompare<_ElementType, _ExtraDataType> tACompare;
    typedef ACompareBase<_ElementType>             tACompareBase;

  // Common Methods

    ACompare(const _ExtraDataType & extra_data, eAEquate (*function_p)(const _ElementType & lhs, const _ElementType & rhs, _ExtraDataType * extra_data_p) = nullptr);
    ACompare(const ACompare & compare_obj);
    ACompare & operator=(const ACompare & compare_obj);
    
  // Accessor Methods

    const _ExtraDataType & get_extra_data() const;
    _ExtraDataType *       get_extra_data_p();
    void                   set_extra_data(const _ExtraDataType & extra_data);
    void                   set_function(eAEquate (*function_p)(const _ElementType & lhs, const _ElementType & rhs, _ExtraDataType * extra_data_p));
    //void (*)()             get_function() const;

  // Modifying Methods

    eAEquate compare(const _ElementType & lhs, const _ElementType & rhs);

  // Non-Modifying Methods

    virtual tACompareBase * copy_new() const override;

  protected:
  // Data Members

    eAEquate (*m_function_p)(const _ElementType & lhs, const _ElementType & rhs, _ExtraDataType * extra_data_p); // pointer to static function to call
    _ExtraDataType  m_extra_data;  // pointer to extra_data of function to call

  };  // ACompare


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Arg          extra_data - this is some extra data passed to the comparison function
// Arg          function_p - the function to be used, if it is nullptr it is not called
//              (default nullptr)
// Examples:    ACompare<AString, int32_t> compare_obj(42, compare_func);
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline ACompare<_ElementType, _ExtraDataType>::ACompare(
  const _ExtraDataType & extra_data,
  eAEquate             (*function_p)(const _ElementType & lhs, const _ElementType & rhs, _ExtraDataType * extra_data_p) // = nullptr
  ) :
  m_extra_data(extra_data),
  m_function_p(function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy Constructor
// Returns:     itself
// Arg          compare_obj - ACompare<> to copy
// Examples:    ACompare<SomeClass> compare_obj(nullptr);
//              ACompare<SomeClass> compare_obj2(compare_obj);
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline ACompare<_ElementType, _ExtraDataType>::ACompare(const tACompare & compare_obj) :
  m_extra_data(compare_obj.m_extra_data),
  m_function_p(compare_obj.m_function_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     a reference to itself to allow assignment stringization
//              compare_obj1 = compare_obj2 = compare_obj3;
// Arg          compare_obj - ACompare<> to copy
// Examples:    compare_obj1 = compare_obj2;
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline ACompare<_ElementType, _ExtraDataType> & ACompare<_ElementType, _ExtraDataType>::operator=(const tACompare & compare_obj)
  {
  m_extra_data = compare_obj.m_extra_data;
  m_function_p = compare_obj.m_function_p;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets a reference to the extra_data object for the function.
// Returns:     reference to the extra_data object for the function.
// Examples:    const _ExtraDataType & extra_data = compare_obj.get_extra_data();
// See:         get_extra_data_p(), set_extra_data()
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline const _ExtraDataType & ACompare<_ElementType, _ExtraDataType>::get_extra_data() const
  {
  return m_extra_data;
  }

//---------------------------------------------------------------------------------------
//  Gets a pointer to the extra_data object for the function.
// Returns:     pointer to the extra_data object for the function.
// Examples:    _ExtraDataType * extra_data_p = compare_obj.get_extra_data_p();
// See:         get_extra_data(), set_extra_data()
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline _ExtraDataType * ACompare<_ElementType, _ExtraDataType>::get_extra_data_p()
  {
  return &m_extra_data;
  }

//---------------------------------------------------------------------------------------
//  Sets the extra_data object for the function.
// Arg          extra_data - the object that owns the function to be wrapped around
// Examples:    compare_obj.set_extra_data(&some_object);
// See:         get_extra_data_p(), get_extra_data()
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline void ACompare<_ElementType, _ExtraDataType>::set_extra_data(const _ExtraDataType & extra_data)
  {
  m_extra_data = extra_data;
  }

//---------------------------------------------------------------------------------------
//  Sets the function pointer.
// Arg          function_p - the function to be used, if it is nullptr it is not called
// Examples:    compare_obj.set_function(SomeClass::some_function);
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
inline void ACompare<_ElementType, _ExtraDataType>::set_function(eAEquate (*function_p)(const _ElementType & lhs, const _ElementType & rhs, _ExtraDataType * extra_data_p))
  {
  m_function_p = function_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Invokes the stored comparison function with a pointer to the stored
//              extra_data object as an argument in addition to the two elements being
//              compared.
// Arg          lhs - left hand side element to compare
// Arg          rhs - right hand side element to compare
// Examples:    compare_obj.compare();
// Notes:       If the internally stored comparison function is nullptr, the elements are
//              sorted by their memory address locations.
// Modifiers:    virtual - overridden from ACompareBase
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
eAEquate ACompare<_ElementType, _ExtraDataType>::compare(
  const _ElementType & lhs,
  const _ElementType & rhs
  )
  {
  if (m_function_p)
    {
    return (m_function_p)(lhs, rhs, &m_extra_data);
    }

  // Compare memory address by default
  ptrdiff_t result = &lhs - &rhs;

  return A_INT_AS_EQUATE(result);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns a new dynamic copy of itself.  Used virtually when a dynamic copy
//              is needed, but the class is unknown.
// Returns:     new dynamic copy of itself
// Examples:    AFunctionBase * new_compare_obj_p = some_compare_obj.copy_new();
// Author(s):    Conan Reis
template<class _ElementType, class _ExtraDataType>
ACompareBase<_ElementType> * ACompare<_ElementType, _ExtraDataType>::copy_new() const
  {
  tACompareBase * func_p = new tACompare(*this);

  A_VERIFY_MEMORY(func_p != nullptr, tACompare);

  return func_p;
  } 
