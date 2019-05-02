// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// APCompactArrayBase class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APArrayBase.hpp>
#include <string.h>      // Uses: memcpy(), memmove()


//=======================================================================================
// Global Structures
//=======================================================================================

// See AgogCore/ACompareBase.hpp for the class definitions of ACompareAddress and ACompareLogical

//---------------------------------------------------------------------------------------
// Notes    The APCompactArrayBase class template provides a dynamic length, persistent index (i.e.
//          once an element is appended, it may be accessed via an integer index),
//          insertable (i.e. an element may be inserted at a specific index position)
//          collection of pointers to _ElementType.
//
//          The various collection/list/array classes such as the APCompactArray and APSorted
//          template classes share many of the same method names and interfaces.
//          This class contains the methods that have both identical interfaces and
//          identical code bodies.  It would seem perhaps logical to have the methods
//          with identical interfaces be defined here as pure virtual methods so that the
//          different collection classes could more readily interact with each other and
//          be used with greater polymorphism.  This idea however logically sounding,
//          ignores the fact that such collection class polymorphism is rarely if ever
//          used and there is a virtual table performance hit which often causes
//          programmers to write there own collection mechanisms since collections are
//          frequently performance critical.  This defeats the purpose of collection
//          classes.  Therefore, virtual methods are NOT included in this base class or
//          its derived classes.  Collection classes of different types may still
//          interact with one another by converting to and accepting static arrays and
//          via the methods that are contained in this base class.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as (Note using APCompactArray rather than
//          APCompactArrayBase since APCompactArrayBase may not be constructed except by derived classes):
//            template class APCompactArray<AString>;
// Arg      _ElementType - the class/type of elements to be pointed to by the array.
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
template<class _ElementType>
class APCompactArrayBase : public APArrayBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand for APCompactArrayBase template
    typedef APCompactArrayBase<_ElementType> tAPCompactArrayBase;
    typedef APArrayBase<_ElementType>        tAPArrayBase;

  // Converter Methods

    //AString   as_string() const;
    //ADatum    as_datum() const;
    
  // Accessor methods

    uint32_t        get_size() const;
    uint32_t        get_size_buffer_bytes() const;

  // Modifying Behaviour methods

    void           append_null();
    void           assign(APCompactArrayBase * array_p);
    void           crop(uint32_t pos, uint32_t elem_count);
    void           empty();
    void           ensure_length_null(uint32_t length_min);
    void           free(uint32_t pos = 0u);
    void           free_all();
    void           free_all(uint32_t pos, uint32_t elem_count = ALength_remainder);
    void           free_all_last(uint32_t elem_count);
    _ElementType * pop_last();
    void           remove_last();
    void           set_size(uint32_t size);
    void           set_size_free(uint32_t length);

    // $Revisit - CReis To implement:
    // void           insert(const _ElementType & elem, uint32_t pos = 0u);
    // void           insert(const _ElementType & elem, uint32_t pos, uint32_t count);
    // void           insert_all(const APCompactArrayBase & array, uint32_t pos = 0u);
    // _ElementType * pop();
    // _ElementType * pop(uint32_t pos);
    // void           remove(uint32_t pos = 0u);
    // bool           remove_equiv(const _ElementType & elem, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    // void           remove_all(uint32_t pos, uint32_t elem_count = ALength_remainder);
    // void           remove_all_last(uint32_t elem_count);
    // void           set_length_null(uint32_t length);


  // Non-modifying Methods

    uint32_t track_memory(AMemoryStats * mem_stats_p) const;

  protected:
  // Internal Methods

    APCompactArrayBase(uint32_t length = 0u, _ElementType ** array_p = nullptr);
    APCompactArrayBase(APCompactArrayBase * array_p);

  };  // APCompactArrayBase


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Returns size of the current static buffer in the array.
// Returns:     size of the current static buffer in the array.
// Examples:    if (array.get_size() > 3)
// See:         get_length()
// Notes:       Note that this is NOT the number of elements in the APCompactArrayBase.
// Author(s):    Conan Reis
template<class _ElementType>
inline uint32_t APCompactArrayBase<_ElementType>::get_size() const
  {
  return this->m_count;
  }

//---------------------------------------------------------------------------------------
//  Returns size of the current static buffer in bytes - just the buffer &
//              not the elements - use track_memory() for the memory size of the elements.
// Returns:     size of the current static buffer in bytes
// See:         get_size(), get_length()
// Author(s):    Conan Reis
template<class _ElementType>
inline uint32_t APCompactArrayBase<_ElementType>::get_size_buffer_bytes() const
  {
  return sizeof(void *) * this->m_count;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Appends a nullptr element pointer to the end of the array.
// Arg         elem - the element to append
// Examples:   array.append_null();
// See:        ensure_length_null(), set_length_null(), APCompactArray::append_at_null(),
//             APCompactArray::null(), APCompactArray::append_all(elem_count, nullptr)
// Author(s):   Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::append_null()
  {
  _ElementType ** old_array_p = this->m_array_p;
  uint32_t        length      = this->m_count;

  this->m_array_p = tAPArrayBase::alloc_array(length + 1u);
  ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));

  tAPArrayBase::free_array(old_array_p);

  this->m_array_p[length] = nullptr;  // insert nullptr element pointer
  this->m_count++;
  }

//---------------------------------------------------------------------------------------
// Transfer ownership assignment
// Arg         array_p - address of array to take contents from and then empty
// Author(s):   Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::assign(tAPCompactArrayBase * array_p)
  {
  this->m_count = array_p->m_count;
  array_p->m_count = 0u;

  tAPArrayBase::free_array(this->m_array_p);

  this->m_array_p = array_p->m_array_p;
  array_p->m_array_p = nullptr;
  }

//---------------------------------------------------------------------------------------
//  Keeps elem_count elements starting at index pos and removes the rest.
// Arg          pos - starting index position of elements to keep
// Arg          elem_count - number of elements to keep.
// Examples:    array.crop(5, 3);
// See:         pop_all(), free_all(), remove_all(), empty()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::crop(
  uint32_t pos,
  uint32_t elem_count
  )
  {
  if (elem_count)
    {
    APARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    this->m_count = elem_count;
    ::memmove(this->m_array_p, this->m_array_p + pos, elem_count * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
//  Removes all elements from collection
// Examples:    array.empty();
// See:         compact(), ensure_size(), ensure_size_empty()
// Notes:       How could it be more simple?
// Author(s):    Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::empty()
  {
  tAPArrayBase::free_array(this->m_array_p);

  this->m_array_p = nullptr;
  this->m_count   = 0u;
  }

//---------------------------------------------------------------------------------------
// Ensures that the length (element count) of the array is at least
//             "length_min" by adding nullptr elements as needed if the array does not
//             currently have enough elements.
// Arg         length_min - minimum length
// See:        append_null(), set_length_null(), APCompactArray::append_at_null(),
//             APCompactArray::null(), APCompactArray::append_all(elem_count, nullptr)
// Notes:      *** Be careful using this method when working with a sorted array ***
//             This method is useful when setting the elements by hand using index
//             methods or manipulating the element array directly.
// Author(s):   Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::ensure_length_null(uint32_t length_min)
  {
  if (this->m_count < length_min)
    {
    _ElementType ** old_array_p = this->m_array_p;
    uint32_t        length      = this->m_count;

    this->m_array_p = tAPArrayBase::alloc_array(length_min);
    ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));

    tAPArrayBase::free_array(old_array_p);

    ::memset(this->m_array_p + this->m_count, 0, (length_min - this->m_count) * sizeof(_ElementType *));
    this->m_count = length_min;
    }
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) element at index pos.
// Arg          pos - index position of element to free (Default 0)
// Examples:    array.free(5);
// See:         pop(), remove(), null()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::free(
  uint32_t pos // = 0u
  )
  {
  APARRAY_BOUNDS_CHECK(pos);

  _ElementType ** old_array_p = this->m_array_p;
  _ElementType *  elem_p      = old_array_p[pos];
  uint32_t        length      = this->m_count - 1u;

  // Remove element first
  if (length)
    {
    this->m_array_p = tAPArrayBase::alloc_array(length);

    if (pos)
      {
      ::memcpy(this->m_array_p, old_array_p, pos * sizeof(_ElementType *));
      }

    if (pos < length)
      {
      ::memcpy(this->m_array_p + pos, old_array_p + pos + 1u, (length - pos) * sizeof(_ElementType *));
      }
    }
  else
    {
    this->m_array_p = nullptr;
    }

  tAPArrayBase::free_array(old_array_p);

  this->m_count--;

  // Then delete element
  delete elem_p;
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) all the elements in the APCompactArrayBase.
// Examples:    array.free_all();
// See:         pop_all(), remove_all(), empty()
// Notes:       
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::free_all()
  {
  _ElementType ** old_array_p = this->m_array_p;
  _ElementType ** array_p     = old_array_p;  // for faster than data member access
  _ElementType ** array_end_p = array_p + this->m_count;

  // Remove elements first
  this->m_array_p = nullptr;
  this->m_count   = 0u;

  // Then delete elements
  for (; array_p < array_end_p; array_p++)
    {
    delete (*array_p);
    }

  tAPArrayBase::free_array(old_array_p);
  }

//---------------------------------------------------------------------------------------
// Removes and deletes elem_count elements from the end of the array
// Arg         elem_count - number of elements to free.
// Examples:   array.free_all_last(3u);
// See:        remove_all(), pop_all(), free_all(), empty()
// Notes:      This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
// Author(s):   Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::free_all_last(uint32_t elem_count)
  {
  if (elem_count)
    {
    APARRAY_BOUNDS_LENGTH(elem_count);

    // Realloc array
    _ElementType ** old_array_p = this->m_array_p;
    _ElementType ** array_end_p = old_array_p + this->m_count;
    _ElementType ** array_p     = array_end_p - elem_count;
    uint32_t        length      = this->m_count - elem_count;

    if (length)
      {
      this->m_array_p = tAPArrayBase::alloc_array(length);

      ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));
      }
    else
      {
      this->m_array_p = nullptr;
      }

    this->m_count -= elem_count;

    // Then free elements
    for (; array_p < array_end_p; array_p++)
      {
      delete (*array_p);
      }

    tAPArrayBase::free_array(old_array_p);
    }
  }

//---------------------------------------------------------------------------------------
//  Removes and returns the element from the last index position.
// Returns:     a pointer to the popped element	or nullptr if there are no elements
// Examples:    _ElementType * elem_p = array.pop_last();
// See:         pop(), free(), remove()
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * APCompactArrayBase<_ElementType>::pop_last()
  {
  uint32_t length = this->m_count;

  if (length == 0u)
    {
    return nullptr;
    }

  // new length of array
  length--;
  this->m_count = length;

  _ElementType ** old_array_p = this->m_array_p;
  _ElementType * elem_p       = old_array_p[length];
  
  if (length)
    {
    this->m_array_p = tAPArrayBase::alloc_array(length);

    ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));
    }
  else
    {
    this->m_array_p = nullptr;
    }

  tAPArrayBase::free_array(old_array_p);

  return elem_p;
  }

//---------------------------------------------------------------------------------------
//  Removes the element from the last index position.
// Examples:    array.remove_last();
// See:         pop(), free(), remove()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::remove_last()
  {
  // Note: Though length - 1 would be more appropriate, remove_last should always succeed
  // unless there are zero elements, so using a 0 as an index position is a valid test.
  APARRAY_BOUNDS_CHECK(0u);

  // new length of array
  uint32_t        length      = this->m_count - 1u;
  _ElementType ** old_array_p = this->m_array_p;

  this->m_count = length;

  if (length)
    {
    this->m_array_p = tAPArrayBase::alloc_array(length);

    ::memcpy(this->m_array_p, old_array_p, length * sizeof(_ElementType *));
    }
  else
    {
    this->m_array_p = nullptr;
    }

  tAPArrayBase::free_array(old_array_p);
  }


/*
//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) elem_count elements starting at index pos
// Arg          pos - starting index position of elements to free
// Arg          elem_count - number of elements to free.  If elem_count is ALength_remainder, the
//              number of elements freed = length - pos.  (Default ALength_remainder)
// Examples:    array.free_all(5u, 3u);
// See:         pop_all(), remove_all(), empty()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::free_all(
  uint32_t pos,
  uint32_t elem_count  // = ALength_remainder
  )
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {

    APARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType ** array_p     = this->m_array_p + pos;  // for faster than data member access
    _ElementType ** array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      delete (*array_p);
      }

    this->m_count -= elem_count;
    ::memmove(this->m_array_p + pos, this->m_array_p + pos + elem_count, (this->m_count - pos) * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
//  Inserts elem at index pos
// Arg          elem - the element to insert
// Arg          pos - index position to insert element before.  Any element currently at
//              this index is pushed one index higher.
// Examples:    array.insert(elem, 5);
// See:         operator[], append()
// Notes:       !!!!! Use this method with caution !!!  It does not ensure that pos is
//              the correct sort position for elem - esp. with APSorted<>.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, an AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::insert(
  const _ElementType & elem,
  uint32_t             pos // = 0
  )
  {
  APARRAY_BOUNDS_LENGTH(pos);

  if (this->m_count < this->m_count + 1u)  // Needs more space
    {
    _ElementType ** old_array_p = this->m_array_p;

    this->m_count    = AMemory::request_pointer_count_expand(this->m_count + 1u);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_count);
    ::memcpy(this->m_array_p, old_array_p, pos * sizeof(_ElementType *));
    ::memcpy(this->m_array_p + pos + 1u, old_array_p + pos, (this->m_count - pos) * sizeof(_ElementType *));
    tAPArrayBase::free_array(old_array_p);
    }
  else  // enough size in existing array
    {
    ::memmove(this->m_array_p + pos + 1u, this->m_array_p + pos, (this->m_count - pos) * sizeof(_ElementType *));
    }

  this->m_array_p[pos] = const_cast<_ElementType *>(&elem);  // insert element
  this->m_count++;
  }

//---------------------------------------------------------------------------------------
//  Inserts elem count times at index pos
// Arg          elem - the element to insert multiple times
// Arg          pos - index position to insert element before.  Any element currently at
//              this index is pushed count indexes higher.
// Arg          count - number of times to insert element
// Examples:    array.insert(elem, 5, 42);  // insert elem 42 times at index 5
// See:         operator[], append()
// Notes:       !!!!! Use this method with caution !!!  It does not ensure that pos is
//              the correct sort position for elem - esp. with APSorted<>.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, an AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::insert(
  const _ElementType & elem,
  uint32_t             pos,
  uint32_t             count
  )
  {
  APARRAY_BOUNDS_LENGTH(pos);

  uint32_t new_length = this->m_count + count;

  if (this->m_count < new_length)  // Needs more space
    {
    _ElementType ** old_array_pp = this->m_array_p;

    this->m_count    = AMemory::request_pointer_count(new_length);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_count);
    ::memcpy(this->m_array_p, old_array_pp, pos * sizeof(_ElementType *));
    ::memcpy(this->m_array_p + pos + count, old_array_pp + pos, (this->m_count - pos) * sizeof(_ElementType *));
    tAPArrayBase::free_array(old_array_pp);
    }
  else  // enough size in existing array
    {
    ::memmove(this->m_array_p + pos + count, this->m_array_p + pos, (this->m_count - pos) * sizeof(_ElementType *));
    }

  this->m_count = new_length;

  _ElementType *  elem_p       = const_cast<_ElementType *>(&elem);
  _ElementType ** array_pp     = this->m_array_p + pos;
  _ElementType ** array_end_pp = array_pp + count;

  for (; array_pp < array_end_pp; array_pp++)
    {
    *array_pp = elem_p;
    }
  }

//---------------------------------------------------------------------------------------
//  Inserts elements from array at index pos
// Arg          array - the array of elements to insert
// Arg          pos - index position to insert elements before.  Any elements currently
//              at this index is pushed to a higher index.
// Examples:    array.insert_all(elems, 5);
// See:         operator[], append()
// Notes:       !!!!! Use this method with caution !!!  It does not ensure that pos is
//              the correct sort position for elem - esp. with APSorted<>.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, an AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::insert_all(
  const tAPCompactArrayBase & array,
  uint32_t             pos
  )
  {
  APARRAY_BOUNDS_LENGTH(pos);

  uint32_t count      = array.get_length();
  uint32_t new_length = this->m_count + count;

  if (this->m_count < new_length)  // Needs more space
    {
    _ElementType ** old_array_pp = this->m_array_p;

    this->m_count    = AMemory::request_pointer_count(new_length);
    this->m_array_p = tAPArrayBase::alloc_array(this->m_count);
    ::memcpy(this->m_array_p, old_array_pp, pos * sizeof(_ElementType *));
    ::memcpy(this->m_array_p + pos + count, old_array_pp + pos, (this->m_count - pos) * sizeof(_ElementType *));
    tAPArrayBase::free_array(old_array_pp);
    }
  else  // enough size in existing array
    {
    ::memmove(this->m_array_p + pos + count, this->m_array_p + pos, (this->m_count - pos) * sizeof(_ElementType *));
    }

  this->m_count = new_length;
  ::memcpy(this->m_array_p + pos, array.get_array(), count * sizeof(_ElementType *));
  }

//---------------------------------------------------------------------------------------
//  Removes and returns the element at the first index position.
// Returns:     a pointer to the popped element	or nullptr if there are no elements
// Examples:    _ElementType * elem_p = array.pop();
// See:         pop_last(), free(), remove(), get_first()
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * APCompactArrayBase<_ElementType>::pop()
  {
  if (this->m_count)
    {
    _ElementType * elem_p = *this->m_array_p;

    this->m_count--;  // new length of array
    ::memmove(this->m_array_p, this->m_array_p + 1u, this->m_count * sizeof(_ElementType *));
    return elem_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
//  Removes and returns element at index pos.
// Returns:     a pointer to the popped element
// Arg          pos - index position of element to pop
// Examples:    array.pop(5);
// See:         free(), remove(), get_at(), APCompactArray::null()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * APCompactArrayBase<_ElementType>::pop(uint32_t pos)
  {
  APARRAY_BOUNDS_CHECK(pos);

  _ElementType * elem_p = this->m_array_p[pos];

  this->m_count--;           // new length of array
  ::memmove(this->m_array_p + pos, this->m_array_p + pos + 1u, (this->m_count - pos) * sizeof(_ElementType *));
  return elem_p;
  }

//---------------------------------------------------------------------------------------
//  Removes element at index pos.
// Arg          pos - index position of element to remove (Default 0)
// Examples:    array.remove(5);
// See:         pop(), free(), null()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::remove(
  uint32_t pos // = 0
  )
  {
  APARRAY_BOUNDS_CHECK(pos);

  this->m_count--;  // new length of array
  ::memmove(this->m_array_p + pos, this->m_array_p + pos + 1u, (this->m_count - pos) * sizeof(_ElementType *));
  }

//---------------------------------------------------------------------------------------
// Removes equivalent element (i.e. the exact same object with the same memory
//             address) between start_pos and end_pos, returning true if removed and false
//             if not.
// Returns:    true if elem removed, false if not
// Arg         elem - equivalent element (i.e. the exact same object with the same memory
//             address) to remove
// Arg         find_pos_p - position that element was found, or unchanged if not found.
//             If find_pos_p is nullptr, it is not modified.  (Default nullptr)
// Arg         start_pos - first position to look for element  (Default 0)
// Arg         end_pos - last position to look for element.  If end_pos is ALength_remainder,
//             end_pos is set to last index position of the array (length - 1).
//             (Default ALength_remainder)
// Examples:   if (array.find_equiv(obj, &index))  // if index of obj found ...
// See:        find_equiv(), find_equiv_reverse(), APSorted::remove_elem(), APSorted::find_elem()
// Notes:      This method does not take into account the order that the elements are
//             stored in the array.  So this method is handy to find an element if the
//             elements are sorted irrespective of their addresses.
//             This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APCompactArray<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType>
bool APCompactArrayBase<_ElementType>::remove_equiv(
  const _ElementType & elem,
  uint32_t *           find_pos_p, // = nullptr
  uint32_t             start_pos,  // = 0u
  uint32_t             end_pos     // = ALength_remainder
  )
  {
  uint32_t find_pos;

  if (find_equiv(elem, &find_pos, start_pos, end_pos))
    {
    this->m_count--;  // new length of array
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
//  Removes all elements from the APCompactArrayBase
// Examples:    array.remove_all();
// See:         empty(), other versions of remove_all()
// Notes:       This method is synonymous to empty()
// Author(s):    Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::remove_all()
  {
  this->m_count = 0u;
  }

//---------------------------------------------------------------------------------------
//  Removes elem_count elements starting at index pos
// Arg          pos - starting index position of elements to remove
// Arg          elem_count - number of elements to remove.  If elem_count is ALength_remainder, the
//              number of elements removed = length - pos.  (Default ALength_remainder)
// Examples:    array.remove_all(5, 3);
// See:         pop_all(), free_all(), empty()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::remove_all(
  uint32_t pos,
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

    this->m_count -= elem_count;
    ::memmove(this->m_array_p + pos, this->m_array_p + pos + elem_count, (this->m_count - pos) * sizeof(_ElementType *));
    }
  }

//---------------------------------------------------------------------------------------
// Removes elem_count elements from the end of the array
// Arg         elem_count - number of elements to remove.
// Examples:   array.remove_all_last(3);
// See:        remove_all(), pop_all(), free_all(), empty()
// Notes:      This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<APCompactArrayBase<>> exception is thrown.
// Author(s):   Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::remove_all_last(uint32_t elem_count)
  {
  APARRAY_BOUNDS_LENGTH(elem_count);

  this->m_count -= elem_count;
  }

//---------------------------------------------------------------------------------------
// Sets the length of the array and makes all elements pointers to nullptr.
// Arg         length - number of nullptr pointers elements the array should consist of.
// Examples:   array.set_length_null(5u);  // Makes the array have 5 nullptr elements
// See:        set_length_unsafe(), ensure_length_null(), append_null(),
//             APCompactArray::append_at_null(), APCompactArray::null(), APCompactArray::append_all(elem_count, nullptr)
// Notes:      *** Be careful using this method when working with a sorted array ***
//             This method is useful when setting the elements by hand using index
//             methods or manipulating the element array directly.
// Author(s):   Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::set_length_null(uint32_t length)
  {
  ensure_size_empty(length);
  ::memset(this->m_array_p, nullptr, length * sizeof(_ElementType *));
  this->m_count = length;
  }
*/

//---------------------------------------------------------------------------------------
// *** Potentially Unsafe Method *** Reallocates the buffer if it is not
//             already the specified size and discards any previously stored elements and
//             sets the length to the value specified
// Arg         size - number of element pointers for the buffer array / the capacity.
//             Discards previous buffer and allocates a new one if not already this size.
// Notes:      *** Be careful using this method ***
//             This method is useful when setting the elements by hand using index
//             methods or manipulating the element array directly.
// Author(s):   Conan Reis
template<class _ElementType>
inline void APCompactArrayBase<_ElementType>::set_size(uint32_t size)
  {
  if (this->m_count != size)
    {    
    this->m_count = size;

    // Delete before allocating more
    if (this->m_array_p)
      {
      tAPArrayBase::free_array(this->m_array_p);
      }

    this->m_array_p = tAPArrayBase::alloc_array(size);
    }
  }

//---------------------------------------------------------------------------------------
// *** Potentially Unsafe Method *** Frees any existing elements and
//             if it is not already the specified size reallocates the buffer.
//             Caution: the element values are left uninitialized.
// Arg         size - number of element pointers for the buffer array / the capacity.
//             Discards previous buffer and allocates a new one if not already this size.
// Notes:      *** Be careful using this method ***
//             This method is useful when setting the elements by hand using index
//             methods or manipulating the element array directly.
// Author(s):   Conan Reis
template<class _ElementType>
void APCompactArrayBase<_ElementType>::set_size_free(uint32_t size)
  {
  uint32_t count = this->m_count;

  // First free any elements
  if (count)
    {
    _ElementType ** array_p     = this->m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + count;

    while (array_p < array_end_p)
      {
      delete (*array_p);
      array_p++;
      }
    }

  // Finally resize if needed
  if (count != size)
    {    
    this->m_count = size;

    // Delete before allocating more
    if (this->m_array_p)
      {
      tAPArrayBase::free_array(this->m_array_p);
      }

    this->m_array_p = tAPArrayBase::alloc_array(size);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Tracks memory used by the elements (which can be nullptr) in this array
// Returns:    returns this array's buffer size in bytes
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
template<class _ElementType>
uint32_t APCompactArrayBase<_ElementType>::track_memory(AMemoryStats * mem_stats_p) const
  {
  _ElementType ** elem_pp     = this->m_array_p;
  _ElementType ** elem_end_pp = this->m_array_p + this->m_count;

  while (elem_pp < elem_end_pp)
    {
    if (*elem_pp)
      {
      (*elem_pp)->track_memory(mem_stats_p);
      }

    elem_pp++;
    }

  return this->m_count * sizeof(void *);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Static sorted constructor/converter
// Returns:     itself
// Arg          length - number of elements in the array.  (Default 0u)
// Arg          size - length of the array_p buffer.  (Default 0u)
// Arg          array_p - buffer of pointers to type _ElemType.  (Default nullptr)
// Examples:    called by derived classes
// Modifiers:    protected
// Author(s):    Conan Reis
template<class _ElementType>
inline APCompactArrayBase<_ElementType>::APCompactArrayBase(
  uint32_t        length,  // = 0u
  _ElementType ** array_p  // = nullptr
  ) :
  tAPArrayBase(length, array_p)
  {
  }

//---------------------------------------------------------------------------------------
//  Transfer copy constructor - takes over internal buffer of given array and
//              empties it.
// Returns:     itself
// Arg          array - array to take over the contents of and empty
// Examples:    called by derived classes
// Modifiers:    protected
// Author(s):    Conan Reis
template<class _ElementType>
inline APCompactArrayBase<_ElementType>::APCompactArrayBase(APCompactArrayBase * array_p) :
  tAPArrayBase(array_p->m_count, array_p->m_array_p)
  {
  array_p->m_count   = 0u;
  array_p->m_array_p = nullptr;
  }
