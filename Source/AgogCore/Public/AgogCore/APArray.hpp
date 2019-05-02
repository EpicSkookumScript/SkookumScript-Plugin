// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Dynamic APArray class declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APSizedArrayBase.hpp>
#include <AgogCore/ACompareBase.hpp>  // Uses: ACompareAddress<>, ACompareLogical<>
#include <stdarg.h>          // Uses: va_array, va_start, va_arg, va_end
#include <stdlib.h>          // Uses: qsort


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    The APArray class template provides a dynamic length, persistent index (i.e.
//          once an element is appended, it may be accessed via an integer index),
//          insertable (i.e. an element may be inserted at a specific index position)
//          collection of pointers to _ElementType.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class APArray<AString>;
// Arg      _ElementType - the class/type of elements to be pointed to by the array.
// Arg      _KeyType - the class/type to use for searching and sorting.  It is often a
//          data member or superclass of the _ElementType.  To get access to the key,
//          _ElementType must have a conversion operator to the _KeyType (or the _KeyType
//          must be derived from _KeyType or have a non-explicit constructor/converter
//          for _ElementType).  For example, it is common for classes to have a symbol
//          name identifying them and it is this name by which they are sorted.  So these
//          'named' element classes could use ASymbol as the _KeyType and have 'operator
//          const ASymbol & () const' as a conversion operator.  The _KeyType may be
//          required to possess various methods or even data members due to the
//          _CompareClass that is used.  For example, the default _CompareClass is
//          ACompareAddress<> which uses the memory address of the _KeyType and so relies
//          on no particular methods, but the next most common _CompareClass is
//          ACompareLogical<> which relies on the equal to (=) and the less than (<)
//          operators of the _KeyType.    
//          (Default  _ElementType)  So the keys are the elements themselves.
// Arg      _CompareClass - a class template that provides the static sorting functions
//          equals() and comparison().  These functions should be inlined so that there is
//          no function call overhead.  See ACompareAddress and ACompareLogical in
//          AgogCore/ACompareBase.hpp for examples.  See AgogCore/AString.hpp for the class definition of
//          ACompareStrInsensitive.  (Default ACompareAddress<_ElementType>)
// See Also
//   APArrayBase<>                 - dynamic pointer array base class
//   
//     APSizedArrayBase<>          - Lazy size buffer - at least 4 bytes larger than APCompactArrayBase collections.  Array buffer may be larger size than actual number of elements so that it does not need to be resized with each add/remove of an element.
//       APArray<>                 - Ordered array of pointers to elements with retrieval by key type
//         APArrayFree<>           - Same as APArray<>, but calls free_all() on its destruction
//         APArrayLogical<>        - Same as APArray<>, but uses the comparison operators < and == to sort elements
//       APSorted<>                - APSorted array of pointers to elements with retrieval and sorting by key type
//         APSortedFree<>          - Same as APSorted<>, but calls free_all() on its destruction
//         APSortedLogical<>       - Same as APSorted<>, but uses the comparison operators < and == to sort elements
//           APSortedLogicalFree<> - Same as APSortedLogical<>, but calls free_all() on its destruction
//     
//     APCompactArrayBase<>        - array buffer is always = # elements.  Less memory though may be slower with add/remove
//       APCompactArray<>          - Ordered array of pointers to elements with retrieval by key type
//         APCompactArrayFree<>    - Same as APCompactArray<>, but calls free_all() on its destruction
//         APCompactArrayLogical<> - Same as APCompactArray<>, but uses the comparison operators < and == to sort elements
//     
// Author   Conan Reis
template<
  class _ElementType,
  class _KeyType      = _ElementType,
  class _CompareClass = ACompareAddress<_KeyType>
  >
  class APArray : public APSizedArrayBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand
    typedef APArray<_ElementType, _KeyType, _CompareClass>  tAPArray;
    typedef APSizedArrayBase<_ElementType>                  tAPSizedArrayBase;
    typedef APArrayBase<_ElementType>                       tAPArrayBase;

    // Unhide Inherited Methods

    // Methods in this class with the same name as methods in APSizedArrayBase<> are 'hidden'
    // (even if they do not have the same interface), this makes them visible again.
    // Ensure that any new methods added to this class that also have the same name
    // as methods in APSizedArrayBase<> are included in this list to preserve the 'is-type-of'
    // relationship.  These using directives must precede the new methods in this class.
    using APSizedArrayBase<_ElementType>::free;
    using APSizedArrayBase<_ElementType>::free_all;
    using APSizedArrayBase<_ElementType>::pop;
    using APSizedArrayBase<_ElementType>::remove;
    using APSizedArrayBase<_ElementType>::remove_all;

  // Common methods

    APArray();
    APArray(const APArray & array);
    APArray(tAPSizedArrayBase * array_p);
    ~APArray();
    APArray & operator=(const APArray & array);
    APArray & operator=(const APArrayBase<_ElementType> & array);


  // Converter Methods

    explicit APArray(uint32_t elem_count, ...);
    explicit APArray(const _ElementType ** elems_p, uint32_t elem_count, uint32_t buffer_size = 0u);
    explicit APArray(const _ElementType * elems_p, uint32_t elem_count);

    
  // Accessor methods

    void             null(uint32_t pos = 0u);

  // Modifying Behaviour methods

    // $Revisit - CReis Change APArray class to APArrayBase where valid.

    void           append(const _ElementType & elem);
    bool           append_absent(const _ElementType & elem);
    void           append_all(const APArray & array);
    void           append_all(const APArray & array, uint32_t start_pos, uint32_t elem_count = ALength_remainder);
    void           append_all(_ElementType ** elems_p, uint32_t elem_count);
    void           append_all(_ElementType * elems_p, uint32_t elem_count);
    void           append_all(uint32_t elem_count, const _ElementType * elem_p = nullptr);
    uint32_t       append_at_null(const _ElementType & elem);
    _ElementType * append_replace(const _ElementType & elem, uint32_t * replace_pos_p = nullptr);
    bool           free(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all_all(const APArray & array, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           move_down(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           move_up(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    _ElementType * pop(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           pop_all(APArray * collected_p, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder);
    uint32_t       pop_all(APArray * collected_p, const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       pop_all_all(APArray * collected_p, const APArray & array, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    bool           remove(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       remove_all(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       remove_all_all(const APArray & array, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           reverse(uint32_t pos = 0, uint32_t elem_count = ALength_remainder);
    void           rotate_down();
    void           rotate_up();
    void           set_all(const _ElementType * elem_p, uint32_t pos = 0, uint32_t elem_count = ALength_remainder);
    void           sort(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           swap(uint32_t pos1, uint32_t pos2);

  // Non-modifying Methods

    uint32_t       count(const _KeyType & key,  uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    APArray *      as_new() const;
    bool           find(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool           find_reverse(const _KeyType & key,  uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    _ElementType * get(const _KeyType & key) const;
    _ElementType * get(const _KeyType & key, uint32_t instance, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    void           get_all(APArray * collected_p, const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    void           get_all(APArray * collected_p, const APArray & array, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    uint32_t       get_instance(uint32_t index) const;
    _ElementType * next(const _KeyType & key) const;
    _ElementType * next_equiv(const _ElementType & elem) const;


  protected:
  // Internal Class Methods

    static int sort_compare(const void * lhs_p, const void * rhs_p);

  };  // APArray


//---------------------------------------------------------------------------------------
// APArrayLogical is a shorthand for APArray<_ElementType, _KeyType, ACompareLogical<_KeyType> >
// Note: Although APArrayLogical is derived from APArray, there is no loss in efficiency.
template<
  class _ElementType,
  class _KeyType      = _ElementType
  >
class APArrayLogical : public APArray<_ElementType, _KeyType, ACompareLogical<_KeyType> >
  {
  public:
  // Common types

    // Local shorthand
    typedef APArray<_ElementType, _KeyType, ACompareLogical<_KeyType> >  tAPArray;
    typedef APArrayBase<_ElementType>                                    tAPArrayBase;

  // All the constructors are hidden (stupid!), so make appropriate links

    APArrayLogical();
    APArrayLogical(const APArray<_ElementType, _KeyType, ACompareLogical<_KeyType> > & array);
    APArrayLogical(APSizedArrayBase<_ElementType> * array_p);
    explicit APArrayLogical(const _ElementType ** elems_p, uint32_t elem_count, uint32_t buffer_size = 0u);
    explicit APArrayLogical(const _ElementType * elems_p, uint32_t elem_count);
    explicit APArrayLogical(uint32_t elem_count, ...);
  };

//---------------------------------------------------------------------------------------
// APArrayFree only differs from APArray in that it automatically calls free_all() in its
// destructor.  Note that although APArrayFree is derived from APArray, there is no loss in
// efficiency.
template<
  class _ElementType,
  class _KeyType      = _ElementType,
  class _CompareClass = ACompareAddress<_KeyType>
  >
class APArrayFree : public APArray<_ElementType, _KeyType, _CompareClass>
  {
  public:
    // Common types

    // Local shorthand
    typedef APArray<_ElementType, _KeyType, _CompareClass>  tAPArray;
    typedef APArrayBase<_ElementType>                       tAPArrayBase;

  // All the constructors are hidden (stupid!), so make appropriate links

    APArrayFree();
    APArrayFree(const APArray<_ElementType, _KeyType, _CompareClass> & array);
    APArrayFree(APSizedArrayBase<_ElementType> * array_p);
    explicit APArrayFree(const _ElementType ** elems_p, uint32_t elem_count, uint32_t buffer_size = 0u);
    //explicit APArrayFree(const _ElementType * elems_p, uint32_t elem_count);
    //explicit APArrayFree(uint32_t elem_count, ...);

    ~APArrayFree();
  };

//---------------------------------------------------------------------------------------
// APArrayLogicalFree only differs from APArrayLogical in that it automatically calls
// free_all() in its destructor.  Note that although APArrayLogicalFree is derived from
// APArrayFree, there is no loss in efficiency.
template<
  class _ElementType,
  class _KeyType = _ElementType
>
class APArrayLogicalFree : public APArrayFree<_ElementType, _KeyType, ACompareLogical<_KeyType>>
  {
  public:
    // Local shorthand
    typedef APArrayFree<_ElementType, _KeyType, ACompareLogical<_KeyType>>  tAPArrayFree;
    typedef APArrayLogicalFree<_ElementType, _KeyType>                      tAPArrayLogicalFree;

  // All the constructors are hidden (stupid!), so make appropriate links
    APArrayLogicalFree() : tAPArrayFree() {}
    APArrayLogicalFree(const APArrayLogicalFree & other) : tAPArrayFree(other) {}
    explicit APArrayLogicalFree(const _ElementType ** elems_p, uint32_t elem_count, uint32_t buffer_size) : tAPArrayFree(elems_p, elem_count, buffer_size) {}
  };


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor
// Returns:     itself
// Examples:    APArray<SomeClass> array;
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArray<_ElementType, _KeyType, _CompareClass>::APArray()
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Arg          array - array to copy
// Examples:    APArray<SomeClass> array1;
//              APArray<SomeClass> array2(array1);
// See:         operator=()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArray<_ElementType, _KeyType, _CompareClass>::APArray(const tAPArray & array) :
  APSizedArrayBase<_ElementType>(array.m_count, array.m_count, tAPArrayBase::alloc_array(array.m_count))
  {
  // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
  ::memcpy(this->m_array_p, array.m_array_p, this->m_count * sizeof(_ElementType *));
  }

//---------------------------------------------------------------------------------------
//  Transfer copy constructor - takes over internal buffer of given array and
//              empties it.
// Returns:     itself
// Arg          array - array to take over the contents of and empty
// Examples:    APArray<SomeClass> array1;
//              APArray<SomeClass> array2(&array1);
// See:         operator=(), APArray(const APArray &)
// Notes:       This method is useful when the contents of a local stack allocated array
//              needs to be promoted to a dynamic heap allocated object.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArray<_ElementType, _KeyType, _CompareClass>::APArray(tAPSizedArrayBase * array_p) :
  tAPSizedArrayBase(array_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Destructor - deallocates internal C-string character buffer.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArray<_ElementType, _KeyType, _CompareClass>::~APArray()
  {
  // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
  tAPArrayBase::free_array(this->m_array_p);
  }

//---------------------------------------------------------------------------------------
// Assignment operator
// Returns:    reference to itself to allow for stringization
//             array1 = array2 = array3;
// Examples:   array1 = array2;
// Notes:      Just replaces any existing elements - if it is a *Free version of the class
//             the elements are not freed first - free_all() must be called prior to this
//             method if any existing items are to be freed.
//             If an APArrayBase derived class has an _ElementType that is derived from
//             this element type then it can be passed as an argument, but it may need a
//             reinterpret_cast.
// See:        append_all()
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
APArray<_ElementType, _KeyType, _CompareClass> & APArray<_ElementType, _KeyType, _CompareClass>::operator=(
  const APArray & array
  )
  {
  // $Note - CReis This = operator that takes the APArray<> type must be defined in
  // addition to the version that takes the APArraybase<> below otherwise an erroneous
  // APArray<> version will be auto generated.

  uint32_t length = array.m_count;

  // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
  if (length > this->m_size)
    {
    tAPArrayBase::free_array(this->m_array_p);
    this->m_size    = AMemory::request_pointer_count(length);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_size);
    }

  this->m_count = length;
  ::memcpy(this->m_array_p, array.m_array_p, length * sizeof(_ElementType *));

  return *this;
  }

//---------------------------------------------------------------------------------------
// Assignment operator
// Returns:    reference to itself to allow for stringization
//             array1 = array2 = array3;
// Examples:   array1 = array2;
// Notes:      Just replaces any existing elements - if it is a *Free version of the class
//             the elements are not freed first - free_all() must be called prior to this
//             method if any existing items are to be freed.
//             If an APArrayBase derived class has an _ElementType that is derived from
//             this element type then it can be passed as an argument, but it may need a
//             reinterpret_cast.
// See:        append_all()
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
APArray<_ElementType, _KeyType, _CompareClass> & APArray<_ElementType, _KeyType, _CompareClass>::operator=(
  const APArrayBase<_ElementType> & array
  )
  {
  // $Note - CReis The = operator that takes the APArray<> type above must be defined in
  // addition to this version that takes the APArraybase<> otherwise an erroneous
  // APArray<> version will be auto generated.

  uint32_t length = array.get_length();

  // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
  if (length > this->m_size)
    {
    tAPArrayBase::free_array(this->m_array_p);
    this->m_size    = AMemory::request_pointer_count(length);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_size);
    }

  this->m_count = length;
  ::memcpy(this->m_array_p, array.get_array(), length * sizeof(_ElementType *));

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Variable argument constructor.
// Returns:     itself
// Arg          elem_count - number of pointers to elements in the variable length
//              argument list
// Arg          ... - pointers to elements of type _ElementType
// Examples:    APArray<SomeClass> (3, &elem1, elem2_p, get_elem_p());
// See:         APArray<_ElementType>(elems_p, elem_count, buffer_size)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
APArray<_ElementType, _KeyType, _CompareClass>::APArray(
  uint32_t elem_count,
  ...
  ) :
  tAPSizedArrayBase(elem_count, elem_count, nullptr)
  {
  if (elem_count)
    {
    va_list         arg_array;
    uint32_t        pos      = 0u;
    _ElementType ** array_pp = tAPArrayBase::alloc_array(elem_count);

    va_start(arg_array, elem_count);     // Initialize variable arguments

    while(pos < uint32_t(elem_count))
      {
      array_pp[pos] = va_arg(arg_array, _ElementType *);
      pos++;
      }

    va_end(arg_array);  // Reset variable arguments

    // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
    this->m_array_p = array_pp;
    }
  }

//---------------------------------------------------------------------------------------
//  Static array constructor/converter
// Returns:     itself
// Arg          elems_p - array of pointers to elements of type _ElementType.  It can be
//              nullptr if elems_p is non-zero - see the description of the elems_p arg.
// Arg          elem_count - number of pointers to elements in elems_p
// Arg          buffer_size - If this argument is non-zero and elems_p is not nullptr, it
//              indicates the size of the elems_p buffer and that ownership of the
//              elems_p buffer is taken over by this array and be eventually deleted when
//              the APArray instance is destroyed.  If this argument is non-zero and
//              elems_p is nullptr, it indicates the initial buffer size that this array
//              should allocate.  (Default 0u)
// Examples:    APArray<AString> strings(strs_p, 5u);
// See:         APArray<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
APArray<_ElementType, _KeyType, _CompareClass>::APArray(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  uint32_t              buffer_size // = 0u
  ) :
  tAPSizedArrayBase(elem_count)
  {
  // $Note - CReis The GCC compiler cannot resolve inherited members without "this->" or "SourceClass::" prefixing them.
  if (buffer_size)
    {
    this->m_size    = buffer_size;
    this->m_array_p = (elems_p) ? const_cast<_ElementType **>(elems_p) : tAPArrayBase::alloc_array(buffer_size);
    }
  else if (elem_count)
    {
    this->m_size    = elem_count;
    this->m_array_p = tAPArrayBase::alloc_array(elem_count);
    ::memcpy(this->m_array_p, elems_p, elem_count * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
//  Constructor (for pre-existing arrays of elements)
// Returns:     itself
// Arg          elems_p - array of elements of type _ElementType.
// Arg          elem_count - number of elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// See:         APArray<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
APArray<_ElementType, _KeyType, _CompareClass>::APArray(
  const _ElementType * elems_p,
  uint32_t             elem_count
  ) :
  tAPSizedArrayBase(elem_count, elem_count, nullptr)
  {
  if (elem_count)
    {
    uint32_t        pos      = 0u;
    _ElementType ** array_pp = tAPArrayBase::alloc_array(elem_count);

    while(pos < elem_count)
      {
      array_pp[pos] = const_cast<_ElementType *>(&elems_p[pos]);
      pos++;
      }

    this->m_array_p = array_pp;
    }
  }

//---------------------------------------------------------------------------------------
//  Constructor/converter from a ADatum
// Returns:     itself with dynamically allocated elements as specified by the given ADatum
// Arg          datum - ADatum to construct from
// Notes:       Each element class in the ADatum to be converted must be registered for
//              the ADatum::as_new_object() method.
// Examples:    APArray array(datum);
// Author       Conan Reis
/* $Revisit - CReis this should be rewritten
APArray::APArray(const ADatum & datum)
  {
  A_VERIFY(
    datum.get_datum_id() == get_id(),
    a_cstr_format(
      "APArray<>::APArray() - invalid ADatum conversion\nWanted id #%i, not #%i",
      get_id(),
      datum.get_datum_id()),
    AErrId_invalid_datum_id,
    tAPArray);

  uint8_t * buffer_p     = (uint8_t *)datum.get_data_buffer();
  uint8_t * buffer_end_p = buffer_p + datum.get_data_length();

  this->m_array_p = tAPArrayBase::alloc_array(this->m_size);

  // Create element objects
  while (buffer_p < buffer_end_p)
    {
    ADatum element_db(buffer_p);

    append(*element_db.as_new_object());
    buffer_p += element_db.get_length();
    }
  }
*/

//---------------------------------------------------------------------------------------
// AFunction  Converter to a ADatum
// Returns    a ADatum version of itself
// Examples:      ADatum datum(array.as_datum());
// Author     Conan Reis
/* $Revisit - CReis this should be rewritten using as_datum_size()
ADatum APArray::as_datum() const
  {
  _ElementType * elem_p;
  uint8_t *      data_p;
  uint8_t *      pos_p;
  uint32_t       data_length;
  ADatum *       element_db_p;
  APArray        element_dbs;
  uint32_t       element_length = 0;
  uint32_t       pos            = 0;

  // Accumulate DataBlocks for data elements
  element_dbs.ensure_size(this->m_count);
  for (; pos < this->m_count; pos++)
    {
    elem_p = this->m_array_p[pos];

    // Append element ADatum
    if (elem_p)
      element_db_p = new ADatum(*elem_p);
    else
      element_db_p = new ADatum();
    element_length += element_db_p->get_length();
    element_dbs.append(*element_db_p);
    }

  // Create APArray ADatum header
  data_length = ADatum_header_size + element_length;
  data_p = pos_p = new uint8_t[data_length];
  *((uint32_t *)pos_p) = data_length;           // Length of ADatum
  pos_p += 4;                                                 
  *((uint32_t *)pos_p) = get_id();              // ClassId
  pos_p += 4;                                                 

  // Copy accumulated data element DataBlocks in complete DataBlock
  for (pos = 0; pos < this->m_count; pos++)
    {
    element_db_p = (ADatum *)element_dbs.copy_quick(pos);
    element_length = element_db_p->get_length();
    ::memcpy(pos_p, element_db_p->get_buffer(), element_length);
    delete element_db_p;
    pos_p += element_length;
    }

  return ADatum(data_p, true, data_length);
  }
*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Appends element to the end of the array
// Arg         elem - the element to append
// Examples:   array.append(elem);
// See:        operator[], insert(), append_absent(), append_at_null(), append_replace()
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::append(const _ElementType & elem)
  {
  uint32_t length = this->m_count;

  if (this->m_size < length + 1u)  // Needs more space
    {
    _ElementType ** old_array_p = this->m_array_p;

    this->m_size    = AMemory::request_pointer_count_expand(length + 1u);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_size);
    ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));
    tAPArrayBase::free_array(old_array_p);
    }

  this->m_array_p[length] = const_cast<_ElementType *>(&elem);  // insert element
  this->m_count++;
  }

//---------------------------------------------------------------------------------------
//  Appends an element to the array if it is not already present in the array
// Returns:     true if element appended, false if not
// Arg          elem - element to append
// Examples:    array.append_absent(elem);
// See:         operator[], insert(), append(), append_at_null(), append_replace()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline bool APArray<_ElementType, _KeyType, _CompareClass>::append_absent(const _ElementType & elem)
  {
  if (!find(elem))
    {
    append(elem);
    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Appends all elements in array to the current array.
// Arg          array - array of elements to append
// Examples:    array1.append_all(array2);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::append_all(const tAPArray & array)
  {
  uint32_t length = array.get_length();

  this->ensure_size(this->m_count + length);
  ::memcpy(this->m_array_p + this->m_count, array.get_array(), length * sizeof(_ElementType *));
  this->m_count += length;
  }

//---------------------------------------------------------------------------------------
//  Appends specified range of elements in array to the current array.
// Arg          array - array of elements to append
// Arg          pos - starting index position of elements in array to get
// Arg          elem_count - number of elements from array to get.  If elem_count is
//              ALength_remainder, the number of elements retrieved = length - pos.
// Examples:    // append 10 elements from array2 starting at index 3 to array1
//              array1.append_all(array2, 3, 10);
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::append_all(
  const tAPArray & array,
  uint32_t         pos,         // = 0u
  uint32_t         elem_count   // = ALength_remainder
  )
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = array.m_count - pos;
    }

  if (elem_count)
    {
    APARRAY_BOUNDS_CHECK_ARRAY_SPAN(array, pos, elem_count);

    this->ensure_size(this->m_count + elem_count);
    ::memcpy(this->m_array_p + this->m_count, array.m_array_p + pos, elem_count * sizeof(_ElementType *));
    this->m_count += elem_count;
    }
  }

//---------------------------------------------------------------------------------------
//  Appends elem_count elements from elems_p to the current array.
// Arg          elems_p - array of pointers to elements of type _ElementType
// Arg          elem_count - number of pointers to elements in elems_p
// Examples:    array.append_all(elems_p, elem_count);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::append_all(
  _ElementType ** elems_p,
  uint32_t        elem_count
  )
  {
  this->ensure_size(this->m_count + elem_count);
  ::memcpy(this->m_array_p + this->m_count, elems_p, elem_count * sizeof(_ElementType *));
  this->m_count += elem_count;
  }

//---------------------------------------------------------------------------------------
//  Appends elem_count elements from elems_p to the current array.
// Arg          elems_p - array of elements of type _ElementType
// Arg          elem_count - number of elements in elems_p
// Examples:    _ElementType elems[42];
//              array.append_all(elems, 42);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::append_all(
  _ElementType * elems_p,
  uint32_t       elem_count
  )
  {
  if (elem_count)
    {
    this->ensure_size(this->m_count + elem_count);

    uint32_t        pos     = 0u;
    _ElementType ** array_p = this->m_array_p + this->m_count;

    while (pos < elem_count)
      {
      array_p[pos] = const_cast<_ElementType *>(&elems_p[pos]);
      pos++;
      }

    this->m_count += elem_count;
    }
  }

//---------------------------------------------------------------------------------------
//  Appends the same element (or a nullptr pointer) elem_count times to the
//              current array.
// Arg          elem_count - number of times to append elem_p
// Arg          elem_p - pointer to element to add
// Examples:    array.append_all(elem_count, dupe_p);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::append_all(
  uint32_t             elem_count,
  const _ElementType * elem_p // = nullptr
  )
  {
  this->ensure_size(this->m_count + elem_count);

  _ElementType ** array_p     = this->m_array_p + this->m_count;
  _ElementType ** array_end_p = array_p + elem_count;

  for (; array_p < array_end_p; array_p++)
    {
    *array_p = const_cast<_ElementType *>(elem_p);
    }

  this->m_count += elem_count;
  }

//---------------------------------------------------------------------------------------
//  Appends element in the first index position with a nullptr pointer or at the
//              end of the array if no elements are nullptr.
// Returns:     index position where element is placed
// Arg          elem - the element to append
// Examples:    array.append_at_null(elem);
// See:         operator[], null(), append_null(), set_length_null(), append_all(elem_count, nullptr).
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::append_at_null(const _ElementType & elem)
  {
  _ElementType ** array_p     = this->m_array_p;  // for faster than data member access
  _ElementType ** array_end_p = array_p + this->m_count;

  while ((array_p < array_end_p) && (*array_p))
    {
    array_p++;
    }

  if (array_p < array_end_p)
    {
    *array_p = const_cast<_ElementType *>(&elem);
    return uint32_t(array_p - this->m_array_p);
    }

  append(elem);
  return this->m_count - 1;
  }

//---------------------------------------------------------------------------------------
//  Appends an element to the APArray if it is not already present or replaces
//              first found matching element.
// Returns:     pointer to the replaced element or nullptr if the element was appended, but
//              there was no matching element
// Arg          elem - element to append
// Arg          replace_pos_p - address to store the index position that elem was
//              appended or replaced.  If insert_pos_p is nullptr, it is not modified.
//              (Default nullptr)
// Examples:    array.append_replace(elem);
// See:         append(), append_absent()
// Notes:       This treats the APArray like a set - all elements are unique (no duplicates)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline _ElementType * APArray<_ElementType, _KeyType, _CompareClass>::append_replace(
  const _ElementType & elem,
  uint32_t *           replace_pos_p // = nullptr
  )
  {
  uint32_t       pos;
  _ElementType * old_elem_p = nullptr;

  if (find(elem, AMatch_first_found, &pos))
    {
    old_elem_p     = this->m_array_p[pos];
    this->m_array_p[pos] = const_cast<_ElementType *>(&elem);  // insert element
    }
  else
    {
    insert(elem, pos);
    }
  if (replace_pos_p)
    {
    *replace_pos_p = pos;
    }

  return old_elem_p;
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) instance of elem between start_pos and
//              end_pos, returning true if found, false if not.
// Returns:     true if instance elem freed, false if not
// Arg          key - key to match for element to free
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that elem was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.free(key, 2);  // free the second occurrence of key
// See:         pop(), remove()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool APArray<_ElementType, _KeyType, _CompareClass>::free(
  const _KeyType & key,
  uint32_t         instance,    // = AMatch_first_found
  uint32_t *       find_pos_p,  // = nullptr
  uint32_t         start_pos,   // = 0u
  uint32_t         end_pos      // = ALength_remainder
  ) 
  {
  uint32_t find_pos;

  if (find(key, instance, &find_pos, start_pos, end_pos))
    {
    delete this->m_array_p[find_pos];
    this->m_count--;           // new length of array
    ::memmove(this->m_array_p + find_pos, this->m_array_p + find_pos + 1u, (this->m_count - find_pos) * sizeof(_ElementType *));

    if (find_pos_p)
      {
      *find_pos_p = find_pos;
      }

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements freed
// Arg          key - key to match elements to free
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.free_all(key);  // free all the elements matching key
// See:         pop_all(), remove_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::free_all(
  const _KeyType & key,
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  )
  {
  uint32_t num_freed = 0;

  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        delete *array_p;
        this->m_count--;     // new length of array
        ::memmove(array_p, array_p + 1, size_t(array_end_p - array_p) * sizeof(_ElementType *));
        array_end_p--;       // since an element is removed
        num_freed++;
        }
      else
        {
        array_p++;
        }
      }
    }
  return num_freed;
  }

//---------------------------------------------------------------------------------------
// For every element in the supplied "removing", all matching elements in the
//             current array between start_pos and end_pos are freed (removed and deleted).
// Returns:    number of elements freed
// Arg         removing - elements to free
// Arg         start_pos - first position to look for elem  (Default 0)
// Arg         end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//             end_pos is set to last index position of the array (length - 1).
//             (Default ALength_remainder)
// Examples:   array.free_all_all(to_free_array);
// See:        pop_all(), remove_all()
// Notes:      This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::free_all_all(
  const tAPArray & removing,
  uint32_t         start_pos, // = 0
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t num_freed;
  uint32_t total_freed = 0;

  if (this->m_count)
    {
    _ElementType ** array_p     = removing.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + removing.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      num_freed    = free_all(**array_p, start_pos, end_pos);
      end_pos     -= num_freed;
      total_freed += num_freed;
      }
    }
  return total_freed;
  }

//---------------------------------------------------------------------------------------
//  Swaps instance of elem (if found) with the elem one index lower (if one
//              exists).
// Arg          key - key to match element to move
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the qSet (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Notes:       calls Find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::move_down(
  const _KeyType & key,
  uint32_t         instance,  // = AMatch_first_found
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t find_pos;

  if ((this->m_count >= 2u)
    && find(key, instance, &find_pos, start_pos, end_pos)
    && (find_pos > 0u))
    {
    _ElementType ** elems_pp = this->m_array_p;
    _ElementType *  elem_p   = elems_pp[find_pos];

    elems_pp[find_pos]      = elems_pp[find_pos - 1u];
    elems_pp[find_pos - 1u] = elem_p;
    }
  }

//---------------------------------------------------------------------------------------
//  Swaps instance of elem (if found) with the elem one index higher (if one
//              exists).
// Arg          key - key to match element to move
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the qSet (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Notes:       calls Find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::move_up(
  const _KeyType & key,
  uint32_t         instance, // = AMatch_first_found
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t find_pos;
  uint32_t count = this->m_count;

  if ((count >= 2u)
    && find(key, instance, &find_pos, start_pos, end_pos)
    && (find_pos < (count - 1u)))
    {
    _ElementType ** elems_pp = this->m_array_p;
    _ElementType *  elem_p   = elems_pp[find_pos];

    elems_pp[find_pos]      = elems_pp[find_pos + 1u];
    elems_pp[find_pos + 1u] = elem_p;
    }
  }

//---------------------------------------------------------------------------------------
//  Sets the element pointer at index pos to nullptr
// Returns:     a pointer to the popped element
// Arg          pos - index position of element to set to nullptr
// Examples:    array.null(5);
// See:         free(), remove(), append_at_null()
// Notes:       This method is usually used in conjunction with append_at_null().
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::null(
  uint32_t pos // = 0
  )
  {
  APARRAY_BOUNDS_CHECK(pos);

  this->m_array_p[pos] = nullptr;
  }

//---------------------------------------------------------------------------------------
//  Removes and returns instance of elem between start_pos and end_pos
// Returns:     pointer to elem if found, nullptr if not found
// Arg          key - key to match element to pop
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that elem was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    _ElementType * elem_p = array.pop(key, 2);  // pop the second occurrence of key
// See:         free(), remove(), get()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
_ElementType * APArray<_ElementType, _KeyType, _CompareClass>::pop(
  const _KeyType & key,
  uint32_t         instance,    // = AMatch_first_found
  uint32_t *       find_pos_p,  // = nullptr
  uint32_t         start_pos,   // = 0u
  uint32_t         end_pos      // = ALength_remainder
  )
  {
  uint32_t find_pos;

  if (find(key, instance, &find_pos, start_pos, end_pos))
    {
    _ElementType * elem_p = this->m_array_p[find_pos];

    this->m_count--;           // new length of array
    ::memmove(&this->m_array_p[find_pos], &this->m_array_p[find_pos + 1], (this->m_count - find_pos) * sizeof(_ElementType *));

    if (find_pos_p)
      {
      *find_pos_p = find_pos;
      }

    return elem_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
//  Removes and accumulates elem_count elements starting at index pos
// Arg          collected_p - pointer to an array to append all of the popped elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          pos - starting index position of elements to pop
// Arg          elem_count - number of elements to pop.  If elem_count is ALength_remainder, the
//              number of elements popped = length - pos.  (Default ALength_remainder)
// Examples:    // append 3 elements to popped from array starting at index 5
//              array.pop_all(&popped, 5, 3);
// See:         free_all(), remove_all(), empty(), get_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::pop_all(
  tAPArray * collected_p,
  uint32_t   pos,         // = 0
  uint32_t   elem_count   // = ALength_remainder
  )
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {
    APARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    collected_p->ensure_size(collected_p->m_count + elem_count);
    ::memcpy(collected_p->m_array_p + collected_p->m_count, this->m_array_p + pos, elem_count * sizeof(_ElementType *));
    collected_p->m_count += elem_count;
    this->m_count        -= elem_count;
    ::memmove(this->m_array_p + pos, this->m_array_p + pos + elem_count, (this->m_count - pos) * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
//  Removes and accumulates all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Arg          collected_p - pointer to an array to append all of the popped elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          key - key to match element to pop
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // remove all the elements from array matching key and append them to popped
//              array.pop_all(&popped, key);
// See:         free_all(), remove_all(), get_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::pop_all(
  tAPArray *       collected_p,
  const _KeyType & key,
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t pop_count = 0u;

  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        collected_p->append(**array_p);
        pop_count++;
        this->m_count--;     // new length of array
        ::memmove(array_p, array_p + 1, size_t(array_end_p - array_p) * sizeof(_ElementType *));
        array_end_p--;  // since an element is removed
        }
      else
        {
        array_p++;
        }
      }
    }

  return pop_count;
  }
  
//---------------------------------------------------------------------------------------
// Removes and accumulates all the elements from the current APArray
//             between start_pos and end_pos that match elements in array using
//             _CompareClass::equals(key, elem_i).
// Arg         collected_p - pointer to an array to append all of the popped elements to.
//             Any previous elements will remain in collected_p, they are just added to.
// Arg         array - elements to free
// Arg         start_pos - first position to look for elem  (Default 0)
// Arg         end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//             end_pos is set to last index position of the array (length - 1).
//             (Default ALength_remainder)
// Examples:   // remove all the elements from array matching those in to_pop_array and append them to popped
//             array.pop_all_all(&popped, to_pop_array);
// See:        free_all(), remove_all(), get_all()
// Notes:      This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::pop_all_all(
  tAPArray *       collected_p,
  const tAPArray & array,
  uint32_t         start_pos, // = 0
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t pop_count = 0u;

  if ((this->m_count) && (array.get_length())) // if not empty
    {
    _ElementType ** array_p;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** sub_array_p     = array.get_array();
    _ElementType ** sub_array_end_p = sub_array_p + array.get_length();
    _ElementType ** array_end_p     = this->m_array_p + end_pos;

    for (; sub_array_p < sub_array_end_p; sub_array_p++)
      {
      array_p = this->m_array_p + start_pos;

      while (array_p <= array_end_p)
        {
        if (_CompareClass::equals(**sub_array_p, **array_p))
          {
          collected_p->append(**array_p);
          pop_count++;
          this->m_count--;     // new length of array
          ::memmove(array_p, array_p + 1, size_t(array_end_p - array_p) * sizeof(_ElementType *));
          array_end_p--;  // since an element is removed
          }
        else
          {
          array_p++;
          }
        }
      }
    }

  return pop_count;
  }

//---------------------------------------------------------------------------------------
//  Removes instance of elem between start_pos and end_pos, returning true if
//              found, false if not.
// Returns:     true if instance elem removed, false if not
// Arg          key - key to match element to remove
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that elem was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for elem  (Default 0u)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.remove(key, 2u);  // remove the second occurrence of key
// See:         pop(), free()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool APArray<_ElementType, _KeyType, _CompareClass>::remove(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) 
  {
  uint32_t find_pos;

  if (find(key, instance, &find_pos, start_pos, end_pos))
    {
    this->m_count--;           // new length of array
    ::memmove(this->m_array_p + find_pos, this->m_array_p + find_pos + 1, (this->m_count - find_pos) * sizeof(_ElementType *));

    if (find_pos_p)
      {
      *find_pos_p = find_pos;
      }

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Removes all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements removed
// Arg          key - key to match element to remove
// Arg          start_pos - first position to look for elem  (Default 0u)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.remove_all(key);  // remove all the elements matching key
// See:         pop_all(), free_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::remove_all(
  const _KeyType & key,
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t num_removed = 0u;

  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        this->m_count--;     // new length of array
        ::memmove(array_p, array_p + 1, size_t(array_end_p - array_p) * sizeof(_ElementType *));
        array_end_p--;  // since an element is removed
        num_removed++;
        }
      else
        {
        array_p++;
        }
      }
    }

  return num_removed;
  }

//---------------------------------------------------------------------------------------
//  Removes all the elements from the current APArray between start_pos and
//              end_pos that match elements in array using _CompareClass::equals(key, elem_i).
// Returns:     number of elements removed
// Arg          array - elements to remove
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.remove_all_all(to_remove_array);
// See:         pop_all(), free_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::remove_all_all(
  const tAPArray & array,
  uint32_t         start_pos, // = 0
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t num_removed;
  uint32_t total_removed = 0;

  if (this->m_count)
    {
    _ElementType ** array_p     = array.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + array.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      num_removed    = remove_all(**array_p, start_pos, end_pos);
      end_pos       -= num_removed;
      total_removed += num_removed;
      }
    }

  return total_removed;
  }

//---------------------------------------------------------------------------------------
//  Reverses the order of elem_count elements starting at index pos.
// Arg          pos - starting index position of elements to reverse
// Arg          elem_count - number of elements to reverse.  If elem_count is
//              ALength_remainder, the number of elements reversed = length - pos.
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::reverse(
  uint32_t pos,       // = 0
  uint32_t elem_count // = ALength_remainder
  )
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {
    APARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType *  elem_p;
    _ElementType ** elems1_p = this->m_array_p + pos;
    _ElementType ** elems2_p = elems1_p + elem_count - 1;

    while (elems1_p < elems2_p)
      {
      elem_p    = *elems1_p;
      *elems1_p = *elems2_p;
      *elems2_p = elem_p;

      elems1_p++;
      elems2_p--;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Puts the first element in the array at the end and shifts all elements
//              down one position.
// Examples:    array.rotate_down();
// See:         rotate_up()
// Notes:       1 2 3 4  becomes  2 3 4 1
//              This function has no effect on arrays that are empty or have only one
//              element.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::rotate_down()
  {
  if (this->m_count > 1u)
    {
    _ElementType * elem_p = *this->m_array_p;

    ::memmove(this->m_array_p, this->m_array_p + 1u, (this->m_count - 1u) * sizeof(_ElementType *));
    this->m_array_p[this->m_count - 1u] = elem_p;
    }
  }

//---------------------------------------------------------------------------------------
//  Puts the last element in the array at the beginning and shifts all
//              elements up one position.
// Examples:    array.rotate_up();
// See:         rotate_down()
// Notes:       1 2 3 4  becomes  4 1 2 3
//              This function has no effect on arrays that are empty or have only one
//              element.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::rotate_up()
  {
  if (this->m_count > 1)
    {
    _ElementType * elem_p = this->m_array_p[this->m_count - 1u];

    ::memmove(this->m_array_p + 1u, this->m_array_p, (this->m_count - 1u) * sizeof(_ElementType *));
    *this->m_array_p = elem_p;
    }
  }

//---------------------------------------------------------------------------------------
// Sets elem_count elements starting at index pos to the specified element.
//             Any existing elements in the range will be overwritten and the array will
//             extend its total length if the range extends beyond its current length.
// Arg         elem_p - pointer to element to set multiple index positions to
// Arg         pos - index of current array to start setting/overwriting items.  It must
//             be an existing element or the end of the array - i.e. pos <= current length
// Arg         elem_count - number of times elem_p is to be stored in current array.
//             Note that elem_count may specify a range beyond the current length - the
//             length will increased as needed.  If count is set to ALength_remainder,
//             the elements set will be all those which currently exist after and
//             including pos - i.e. elem_count = length - pos.
// Notes:      This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::set_all(
  const _ElementType * elem_p,
  uint32_t             pos,       // = 0
  uint32_t             elem_count // = ALength_remainder
  )
  {
  APARRAY_BOUNDS_LENGTH(pos);

  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {
    // $Revisit - CReis [Efficiency] Some unnecessary copying can occur here
    this->ensure_size(pos + elem_count);

    _ElementType ** array_p = this->m_array_p + pos;

    if (this->m_count < (pos + elem_count))
      {
      this->m_count = pos + elem_count;
      }

    while (elem_count)
      {
      *array_p = const_cast<_ElementType *>(elem_p);
      elem_count--;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Sorts the elements in the APArray from start_pos to end_pos.
// Arg          start_pos - first position to start sorting  (Default 0)
// Arg          end_pos - last position to sort.  If end_pos is ALength_remainder, end_pos is
//              set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    array.sort();
// Notes:       calls sort_compare() which calls _CompareClass::comparison()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
// Efficiency   Could probably get more speed by writing a custom version of qsort.
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::sort(
  uint32_t start_pos, // = 0u
  uint32_t end_pos    // = ALength_remainder
  )
  {
  if (this->m_count > 1u)
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    ::qsort(this->m_array_p + start_pos, end_pos - start_pos + 1, sizeof(_ElementType *), sort_compare);
    }
  }

//---------------------------------------------------------------------------------------
//  Swaps the two elements at the specified index positions quickly.
// Returns:     inline 
// Arg          pos1 - index position of first element to swap
// Arg          pos2 - index position of second element to swap
// Examples:    array.swap(8, 11);
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void APArray<_ElementType, _KeyType, _CompareClass>::swap(
  uint32_t pos1,
  uint32_t pos2
  )
  {
  APARRAY_BOUNDS_CHECK(pos1);
  APARRAY_BOUNDS_CHECK(pos2);

  _ElementType ** element1_p = this->m_array_p + pos1;
  _ElementType ** element2_p = this->m_array_p + pos2;

  A_SWAP32(*element1_p, *element2_p);

  // $Revisit - CReis determine if this is more efficient
  //_ElementType * element1_p = this->m_array_p[pos1];
  //this->m_array_p[pos1] = this->m_array_p[pos2];
  //this->m_array_p[pos2] = element1_p;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Counts the number of elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements matched
// Arg          key - key to match elements to count
// Arg          start_pos - first position to look for key  (Default 0u)
// Arg          end_pos - last position to look for key.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    uint32_t elem_count = array.count(key);
// See:         find(), get_instance()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::count(
  const _KeyType & key,
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t num_count = 0u;

  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    for (; array_p <= array_end_p; array_p++)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        num_count++;
        }
      }
    }

  return num_count;
  }

//---------------------------------------------------------------------------------------
//  Creates a dynamically allocated instance of the same class type as this
//              APArray.
// Returns:     a dynamically allocated instance of the same class type as this APArray
// Examples:    APArray<SomeClass> * array_p = array.as_new();
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArray<_ElementType, _KeyType, _CompareClass> * APArray<_ElementType, _KeyType, _CompareClass>::as_new() const
  {
  tAPArray * new_array_p = new ("tAPArray") tAPArray();

  A_VERIFY_MEMORY(new_array_p != nullptr, tAPArray);

  return new_array_p;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of elem between start_pos and end_pos using
//              _CompareClass::equals(key, elem_i), returning true if found, false if
//              not.
// Returns:     true if instance elem found, false if not
// Arg          key - key to match element to find
// Arg          instance - occurrence of element to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that element was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (array.find(key, 2))  // if second occurrence of element found ...
// See:         find_reverse(), count(), pop(), get()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool APArray<_ElementType, _KeyType, _CompareClass>::find(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        if (instance < 2u)  // found it
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(array_p - this->m_array_p);
            }

          return true;
          }

        instance--;
        }

      array_p++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Finds in reverse order the instance of elem between start_pos and end_pos
//              using _CompareClass::equals(key, elem_i) returning true if found,
//              false if not.
// Returns:     true if instance elem found, false if not
// Arg          key - key to match element to find
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_3.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_1.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that elem was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for elem  (Default 0u)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (array.find(key, 2))  // if second occurrence of element found ...
// See:         count(), pop(), get()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool APArray<_ElementType, _KeyType, _CompareClass>::find_reverse(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_end_p))
        {
        if (instance < 2u)  // found it
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(array_end_p - this->m_array_p);
            }

          return true;
          }

        instance--;
        }

      array_end_p--;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Simple get - finds first instance of key starting at the zeroth index
//              position using _CompareClass::equals(key, elem_i), returning a pointer to
//              the element if found, nullptr if not.
//              Equivalent to get(key, AMatch_first_found)
// Returns:     a pointer to the element if found, nullptr if not.
// Arg          key - key to match element to find
// Examples:    if (array.get(key))  // if first occurrence of key found ...
// See:         count(), pop(), find(), get() with more args
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
_ElementType * APArray<_ElementType, _KeyType, _CompareClass>::get(
  const _KeyType & key
  ) const
  {
  if (this->m_count)  // if not empty
    {
    _ElementType ** array_p     = this->m_array_p;
    _ElementType ** array_end_p = array_p + this->m_count;

    while (array_p < array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        return *array_p;
        }

      array_p++;
      }
    }

  return nullptr;
  }


//---------------------------------------------------------------------------------------
//  Finds instance of elem between start_pos and end_pos using
//              _CompareClass::equals(key, elem_i), returning a pointer to the element
//              if found, nullptr if not.
// Returns:     a pointer to the element if found, nullptr if not.
// Arg          key - key to match element to find
// Arg          instance - occurrence of elem to find.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this is
//              equivalent to an instance of 1.
//              For example, searching the APArray (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 or AMatch_first_found will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that elem was found, or unchanged if not found.
//              If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (array.get(key, 2))  // if second occurrence of key found ...
// See:         count(), pop(), find()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
_ElementType * APArray<_ElementType, _KeyType, _CompareClass>::get(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    while (array_p <= array_end_p)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        if (instance < 2u)  // found it
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(array_p - this->m_array_p);
            }

          return *array_p;
          }

        instance--;
        }

      array_p++;
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
//  Accumulates all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Arg          collected_p - pointer to an array to append all of the matched elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          key - key to match element to find
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // append all the elements from array matching key to collected
//              array.pop_all(&collected, key);
// See:         pop_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::get_all(
  tAPArray *       collected_p,
  const _KeyType & key,
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count)  // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** array_p     = this->m_array_p + start_pos;
    _ElementType ** array_end_p = this->m_array_p + end_pos;

    for (; array_p <= array_end_p; array_p++)
      {
      if (_CompareClass::equals(key, **array_p))
        {
        collected_p->append(**array_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Accumulates all the elements from the current APArray between start_pos and
//              end_pos that match elements in array using _CompareClass::equals(key, elem_i).
// Arg          collected_p - pointer to an array to append all of the matched elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          array - elements to match
// Arg          start_pos - first position to look for elem  (Default 0)
// Arg          end_pos - last position to look for elem.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // append all the elements from array matching those in to_get_array to collected
//              array.pop_all(&collected, to_get_array);
// See:         pop_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void APArray<_ElementType, _KeyType, _CompareClass>::get_all(
  tAPArray *       collected_p,
  const tAPArray & array,
  uint32_t         start_pos, // = 0
  uint32_t         end_pos    // = ALength_remainder
  ) const
  {
  if ((this->m_count) && (array.get_length())) // if not empty
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    APARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    _ElementType ** sub_array_p     = array.get_array();
    _ElementType ** sub_array_end_p = sub_array_p + array.get_length();
    _ElementType ** array_p         = this->m_array_p + start_pos;
    _ElementType ** array_end_p     = this->m_array_p + end_pos;

    for (; sub_array_p < sub_array_end_p; sub_array_p++)
      {
      for (; array_p <= array_end_p; array_p++)
        {
        if (_CompareClass::equals(**sub_array_p, **array_p))
          {
          collected_p->append(**array_p);
          }
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Determines the instance number (occurrence) of the element at the
//              specified index if the element matches more than one element in the array
//              using _CompareClass::equals(key, elem_i).
// Returns:     occurence number of element 
// Arg          index - index position of the element to determine the instance number for
// Examples:    uint32_t instance = array.get_instance(8);
// See:         count()
// Notes:       If there is only one match - i.e. itself - then the instance is 1.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t APArray<_ElementType, _KeyType, _CompareClass>::get_instance(uint32_t index) const
  {
  APARRAY_BOUNDS_CHECK(index);

  uint32_t      instance        = 1u;
  _ElementType ** first_element_p = this->m_array_p;
  _ElementType ** elements_p      = first_element_p + index;  // Faster access than using []
  _ElementType *  element_p       = *elements_p;

  elements_p--;
  while ((elements_p > first_element_p) && (_CompareClass::equals(*element_p, **elements_p)))
    {
    instance++;
    elements_p--;
    }

  return instance;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Get the next index element following element matching key or nullptr if not found or
//   there are no  elements following.  Uses _CompareClass::equals(key, elem_i).
//
// #See Also  find(), get(), next_equiv()
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  // pointer to the next element after key or nullptr.
  _ElementType *
APArray<_ElementType, _KeyType, _CompareClass>::next(
  // key to match element to find
  const _KeyType & key
  ) const
  {
  if (this->m_count)  // if not empty
    {
    _ElementType ** array_pp     = this->m_array_p;
    _ElementType ** array_end_pp = array_pp + this->m_count;

    do
      {
      if (_CompareClass::equals(key, **array_pp))
        {
        array_pp++;

        return (array_pp < array_end_pp) ? *array_pp : nullptr;
        }

      array_pp++;
      }
   while (array_pp < array_end_pp);
   }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Get the next index element following elem or nullptr if not found or there are no 
//   elements following.
//
// #See Also  find(), get(), next()
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  // pointer to the next element after key or nullptr.
  _ElementType *
APArray<_ElementType, _KeyType, _CompareClass>::next_equiv(
  // key to match element to find
  const _ElementType & elem
  ) const
  {
  if (this->m_count)  // if not empty
    {
    const _ElementType * elem_p = &elem;

    _ElementType ** array_pp     = this->m_array_p;
    _ElementType ** array_end_pp = array_pp + this->m_count;

    do
      {
      if ((*array_pp) == elem_p)
        {
        array_pp++;

        return (array_pp < array_end_pp) ? *array_pp : nullptr;
        }

      array_pp++;
      }
   while (array_pp < array_end_pp);
   }

  return nullptr;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Used by qsort() in the sort() method
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
int APArray<_ElementType, _KeyType, _CompareClass>::sort_compare(
  const void * lhs_p,
  const void * rhs_p
  )
  {
  return int(_CompareClass::comparison(**((_ElementType **)lhs_p), **((_ElementType **)rhs_p)));
  }


//#######################################################################################
// APArrayLogical
//#######################################################################################

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
inline APArrayLogical<_ElementType, _KeyType>::APArrayLogical()
  {
  }
  
//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
inline APArrayLogical<_ElementType, _KeyType>::APArrayLogical(const APArray<_ElementType, _KeyType, ACompareLogical<_KeyType> > & array) :
  tAPArray(array)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
inline APArrayLogical<_ElementType, _KeyType>::APArrayLogical(APSizedArrayBase<_ElementType> * array_p) :
  tAPArray(array_p)
  {
  }
  
//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
inline APArrayLogical<_ElementType, _KeyType>::APArrayLogical(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  uint32_t              buffer_size // = 0u
  ) :
  tAPArray(elems_p, elem_count, buffer_size)
  {
  }
  
//---------------------------------------------------------------------------------------
//  Constructor (for pre-existing arrays of elements)
// Returns:     itself
// Arg          elems_p - array of elements of type _ElementType.
// Arg          elem_count - number of elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// See:         APArrayLogical<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
APArrayLogical<_ElementType, _KeyType>::APArrayLogical(
  const _ElementType * elems_p,
  uint32_t             elem_count
  ) :
  tAPArray(elems_p, elem_count)
  {
  }

//---------------------------------------------------------------------------------------
//  Variable argument constructor.
// Returns:     itself
// Arg          elem_count - number of pointers to elements in the variable length
//              argument list
// Arg          ... - pointers to elements of type _ElementType
// Examples:    APArrayLogical<SomeClass> (3, &elem1, elem2_p, get_elem_p());
// See:         APArrayLogical<_ElementType, _KeyType>(elems_p, elem_count, buffer_size)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
APArrayLogical<_ElementType, _KeyType>::APArrayLogical(
  uint32_t elem_count,
  ...
  ) :
  tAPArray(nullptr, elem_count)
  {
  if (elem_count)
    {
    va_list arg_array;
    uint32_t    pos = 0u;

    this->m_array_p = tAPArrayBase::alloc_array(elem_count);
    va_start(arg_array, elem_count);     // Initialize variable arguments

    while(pos < uint32_t(elem_count))
      {
      this->m_array_p[pos] = va_arg(arg_array, _ElementType *);
      pos++;
      }

    va_end(arg_array);  // Reset variable arguments
    }
  }


//#######################################################################################
// APArrayFree
//#######################################################################################

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArrayFree<_ElementType, _KeyType, _CompareClass>::APArrayFree()
  {
  }
  
//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArrayFree<_ElementType, _KeyType, _CompareClass>::APArrayFree(const APArray<_ElementType, _KeyType, _CompareClass> & array) :
  tAPArray(array)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArrayFree<_ElementType, _KeyType, _CompareClass>::APArrayFree(APSizedArrayBase<_ElementType> * array_p) :
  tAPArray(array_p)
  {
  }
  
//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArrayFree<_ElementType, _KeyType, _CompareClass>::APArrayFree(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  uint32_t              buffer_size // = 0u
  )
  // $Revisit - CReis It doesn't like this line for some reason tAPArray(elems_p, elem_count, buffer_size)
  {
  this->m_count = elem_count;

  if (buffer_size)
    {
    this->m_size    = buffer_size;
    this->m_array_p = (elems_p) ? const_cast<_ElementType **>(elems_p) : tAPArrayBase::alloc_array(buffer_size);
    }
  else
    {
    this->m_size    = elem_count;
    this->m_array_p = tAPArrayBase::alloc_array(elem_count);
    ::memcpy(this->m_array_p, elems_p, elem_count * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline APArrayFree<_ElementType, _KeyType, _CompareClass>::~APArrayFree()
  {
  this->free_all();
  }
