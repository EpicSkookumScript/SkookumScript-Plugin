// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AVCompactArrayBase class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AMemory.hpp>
#include <string.h>      // Uses: memcpy()

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdynamic-class-memaccess" // Allow overclobbering of vtable pointers
#endif

//=======================================================================================
// Global Structures
//=======================================================================================

// See AgogCore/ACompareBase.hpp for the class definitions of ACompareAddress and ACompareLogical

//---------------------------------------------------------------------------------------
// #Description
//   The AVCompactArrayBase class template provides a value-based (V), dynamic length,
//   persistent index (i.e. once an element is appended, it may be accessed via an integer
//   index), insertable (i.e. an element may be inserted at a specific index position)
//   collection of _ElementType objects.
//
//   There are numerous - though subtlety different collection/list/array classes that
//   share many of the same method names and interfaces - some even with identical
//   interfaces and identical or near identical code bodies.  It would seem perhaps
//   logical to have the methods with identical interfaces be defined here as pure virtual
//   methods so that the different collection classes could more readily interact with
//   each other and be used with greater polymorphism however such collection class
//   polymorphism is rarely if ever used and there is a virtual table performance hit
//   which often causes programmers to write their own collection mechanisms since
//   collections are frequently performance critical.  This defeats the purpose of
//   collection classes.  Therefore, virtual methods are NOT included in this base class
//   or derived classes.  Collection classes of different types may still interact with
//   one another by converting to and accepting static arrays and via the methods that are
//   contained in this base class.
//
//   Any modifications to this template should be compile-tested by adding an explicit
//   instantiation declaration such as (Note using AVCompactSorted rather than
//   AVCompactArrayBase since AVCompactArrayBase may not be constructed except by derived
//   classes):  template class AVCompactSorted<AString>;
//   
// #See Also
//   ~AVCompactArray<>           - Ordered array of pointers to elements with retrieval by key type
//     ~AVCompactArrayLogical<>  - Same as AVCompactArray<>, but uses the comparison operators < and == to sort elements
//   AVCompactSorted<>           - AVCompactSorted array of pointers to elements with retrieval and sorting by key type
//     AVCompactSortedLogical<>  - Same as AVCompactSorted<>, but uses the comparison operators < and == to sort elements
// #Author   Conan Reis
template<
  // the class/type of element objects in the array.
  class _ElementType
  >
class AVCompactArrayBase
  {
  public:
  // Common types

    // Local shorthand for AVCompactArrayBase template
    typedef AVCompactArrayBase<_ElementType> tAVCompactArrayBase;

  // Converter methods

    void     as_binary_elems(void ** binary_pp) const;
    void     as_binary(void ** binary_pp) const;
    void     as_binary8(void ** binary_pp) const;
    uint32_t as_binary_elems_length() const;
    uint32_t as_binary_length() const;
    uint32_t as_binary_length8() const;
    uint32_t get_size_buffer_bytes() const;
    uint32_t track_memory(AMemoryStats * mem_stats_p) const;

  // Accessor methods

    _ElementType *  get_array() const;
    _ElementType *  get_array_end() const;
    _ElementType *  begin() const { return get_array(); } // So C++11 shorthands can be used on this
    _ElementType *  end() const { return get_array_end(); }
    _ElementType &  get_at(uint32_t pos) const;
    _ElementType *  get_first() const;
    _ElementType *  get_last() const;
    uint32_t        get_count() const;
    bool            is_empty() const;
    bool            is_filled() const;
    void            set_at(uint32_t pos, const _ElementType & elem);
    _ElementType &  operator()(uint32_t pos) const;
    _ElementType &  operator[](uint32_t pos) const;

  // Modifying Behaviour methods

    void append_last_undef(const _ElementType & elem);
    void assign(AVCompactArrayBase * array_p);
    void empty();
    void empty_ensure_count_undef(uint32_t count);
    void insert(const _ElementType & elem, uint32_t pos = 0u);
    void remove(uint32_t pos = 0u);
    void remove_last();
    void set_count_unsafe(uint32_t length);

    //void crop(uint32_t pos, uint32_t elem_count);
    //void insert(const _ElementType & elem, uint32_t pos, uint32_t count);
    //void insert_all(const AVCompactArrayBase & array, uint32_t pos = 0u);
    //void remove_all(uint32_t pos, uint32_t elem_count = ALength_remainder);
    //void remove_all_last(uint32_t elem_count);


  // Non-modifying Methods

    template<class _InvokeType>
      void apply(_InvokeType & invoke_obj, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;

    void apply_method(void (_ElementType::* method_m)(), uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;
    void apply_method(void (_ElementType::* method_m)() const, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;

  protected:
  // Internal Methods

    AVCompactArrayBase(uint32_t count = 0u, _ElementType * array_p = nullptr);

  // Internal Class Methods

    static _ElementType * alloc_array(uint32_t count);
    static void           dtor_elems(_ElementType * array_p, uint32_t count);
    static void           free_array(_ElementType * array_p);

  // Data members

    uint32_t       m_count;    // Number of elements in and size of m_array_p
    _ElementType * m_array_p;  // Dynamically sizing buffer of elements

  };  // AVCompactArrayBase


//=======================================================================================
// Macros
//=======================================================================================

#ifdef A_BOUNDS_CHECK

// These bounds checks are macros rather than methods so that they are a bit faster in debug mode.

  //---------------------------------------------------------------------------------------
  #define AVCOMPACTARRAY_BOUNDS_CHECK(_pos) \
    A_VERIFY( \
      ((_pos) < this->m_count), \
      a_cstr_format(" - invalid index\nGiven %u but length only %u", (_pos), this->m_count), \
      AErrId_invalid_index, \
      tAVCompactArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVCOMPACTARRAY_BOUNDS_LENGTH(_length) \
    A_VERIFY( \
      ((_length) <= this->m_count), \
      a_cstr_format(" - invalid length or insertion index\nGiven %u but length only %u", (_length), this->m_count), \
      AErrId_invalid_index, \
      tAVCompactArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(_start_pos, _end_pos) \
    A_VERIFY( \
      (((_start_pos) < this->m_count) && ((_end_pos) < this->m_count) && ((_start_pos) <= (_end_pos))), \
      a_cstr_format("(start_pos %u, end_pos %u) - invalid index(es)\nLength is %u", (_start_pos), (_end_pos), this->m_count), \
      AErrId_invalid_index_range, \
      tAVCompactArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(_pos, _elem_count) \
    A_VERIFY( \
      (((_pos) < this->m_count) && ((_elem_count) <= (this->m_count - (_pos)))), \
      a_cstr_format("(pos %u, elem_count %u) - invalid index(es)\nLength is %u", (_pos), (_elem_count), this->m_count), \
      AErrId_invalid_index_span, \
      tAVCompactArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVCOMPACTARRAY_BOUNDS_CHECK_ARRAY_SPAN(_array, _pos, _elem_count) \
    A_VERIFY( \
      (((_pos) < (_array).m_count) && ((_elem_count) <= ((_array).m_count - (_pos)))), \
      a_cstr_format("(pos %u, elem_count %u) - invalid index(es)\nLength is %u", (_pos), (_elem_count), (_array).m_count), \
      AErrId_invalid_index_span, \
      tAVCompactArrayBase)

#else

  #define AVCOMPACTARRAY_BOUNDS_CHECK(_pos)                             (void(0))
  #define AVCOMPACTARRAY_BOUNDS_LENGTH(_length)                         (void(0))
  #define AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(_start_pos, _end_pos)       (void(0))
  #define AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(_pos, _elem_count)           (void(0))
  #define AVCOMPACTARRAY_BOUNDS_CHECK_ARRAY_SPAN(_array, _pos, _elem_count)  (void(0))

#endif  // A_BOUNDS_CHECK


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Get the array of _ElementType objects.
// Returns:     static array of _ElementType objects
// Examples:    _ElementType * elems_a = array.get_array();
// Notes:       This can be used for evil purposes, be careful!  ONLY change the elements
//              in the manner proscribed by the array type - i.e. elements in sorted
//              arrays are expected to be sorted.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * AVCompactArrayBase<_ElementType>::get_array() const
  {
  return m_array_p;
  }

//---------------------------------------------------------------------------------------
//  Get the end (of the elements) of the array of _ElementType objects.
// Returns:     static array of _ElementType objects
// Examples:    _ElementType * elems_end_p = array.get_array_end();
// Notes:       This can be used for evil purposes, be careful!  ONLY change the elements
//              in the manner proscribed by the array type - i.e. elements in sorted
//              arrays are expected to be sorted.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * AVCompactArrayBase<_ElementType>::get_array_end() const
  {
  return m_array_p + m_count;
  }

//---------------------------------------------------------------------------------------
//  Get element at index position pos.
// Returns:     element at index position pos
// Arg          pos - index position of element to return
// Examples:    _ElementType & elem = array.get_at(5);
// See:         operator()
// Notes:       This method is synonymous to operator()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVCompactArrayBase<_ElementType>::get_at(uint32_t pos) const
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(pos);

  return m_array_p[pos];
  }

//---------------------------------------------------------------------------------------
// Get pointer to first element or nullptr if there are no elements
// Returns:    pointer to first element or nullptr if there are no elements
// Examples:   _ElementType * elem_p = array.get_first();
// See:        operator(), get_at(), get_last(), AVCompactArray::operator[]
// Notes:      nullptr can be a valid element pointer value so a nullptr result could mean no
//             elements or just an element pointer set to nullptr only usage or getting the
//             length of this array will resolve the difference.
// Author(s):   Conan Reis
template<class _ElementType>
inline _ElementType * AVCompactArrayBase<_ElementType>::get_first() const
  {
  return m_count ? m_array_p : nullptr;
  }

//---------------------------------------------------------------------------------------
// Get pointer to last element or nullptr if there are no elements
// Returns:    pointer to last element or nullptr if there are no elements
// Examples:   _ElementType * elem_p = array.get_last();
// See:        operator(), get_at(), get_first(), AVCompactArray::operator[]
// Notes:      nullptr can be a valid element pointer value so a nullptr result could mean no
//             elements or just an element pointer set to nullptr only usage or getting the
//             length of this array will resolve the difference.
// Author(s):   Conan Reis
template<class _ElementType>
inline _ElementType * AVCompactArrayBase<_ElementType>::get_last() const
  {
  return m_count ? &m_array_p[m_count - 1u] : nullptr;
  }

//---------------------------------------------------------------------------------------
//  Get number of elements stored in the array
// Returns:     number of elements stored in the array
// Examples:    uint32_t count = array.get_count();
// See:         is_empty(), get_size()
// Notes:       This returns the number of elements in the AVCompactArrayBase, NOT the
//              size of the currently allocated internal static buffer.
// Author(s):    Conan Reis
template<class _ElementType>
inline uint32_t AVCompactArrayBase<_ElementType>::get_count() const
  {
  return m_count;
  }

//---------------------------------------------------------------------------------------
//  Returns size of the current static buffer in bytes - use track_memory()
//              for the memory size of the dynamic aspect of the elements.
// Returns:     size of the current static buffer in bytes
// See:         get_size(), get_length()
// Author(s):    Conan Reis
template<class _ElementType>
inline uint32_t AVCompactArrayBase<_ElementType>::get_size_buffer_bytes() const
  {
  return m_count * sizeof(_ElementType);
  }

//---------------------------------------------------------------------------------------
//  Determines if the array is empty - i.e. whether it has no elements or not.
// Returns:     true if empty, false if not
// Examples:    if (array.is_empty())
// Notes:       This method is equivalent to !is_filled().
// See:         is_filled(), get_count()
// Author(s):    Conan Reis
template<class _ElementType>
inline bool AVCompactArrayBase<_ElementType>::is_empty() const
  {
  return (m_count == 0u);
  }

//---------------------------------------------------------------------------------------
//  Determines if the array has any elements
// Returns:     true if it has elements, false if it is empty
// Examples:    if (array.is_filled())
// Notes:       This method is equivalent to !is_empty().
// See:         is_empty(), get_length()
// Author(s):    Conan Reis
template<class _ElementType>
inline bool AVCompactArrayBase<_ElementType>::is_filled() const
  {
  return (m_count != 0u);
  }

//---------------------------------------------------------------------------------------
//  Set pointer to element at index position pos.  *** Be very careful using
//              this method with sorted arrays - it is only valid if the element being
//              swapped would sort to the same position as the element it is replacing.
// Arg          pos - index position of element to replace
// Examples:    array.set_at(5, elem);
// See:         operator(), get_at(), append(), insert()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVCompactArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::set_at(
  uint32_t             pos,
  const _ElementType & elem
  )
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(pos);

  m_array_p[pos] = elem;
  }

//---------------------------------------------------------------------------------------
//  Get element at index position pos.
// Returns:     element at index position pos
// Arg          pos - index position of element to return
// Examples:    _ElementType & elem = array(5);
// See:         get_at()
// Notes:       This method is synonymous to get_at()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVCompactArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVCompactArrayBase<_ElementType>::operator()(uint32_t pos) const
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(pos);

  return m_array_p[pos];
  }

//---------------------------------------------------------------------------------------
//  Get or set pointer to element at index position pos.  It may be used as
//              either a r-value or an l-value.
// Returns:     reference pointer to element at index position pos
// Arg          pos - index position of element to return
// Examples:    elem_p   = array[5];
//              array[3] = elem_p;
// See:         operator(), get_at(), append(), insert()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<APArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVCompactArrayBase<_ElementType>::operator[](uint32_t pos) const
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(pos);

  return m_array_p[pos];
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// #Description
//   Appends element to the end of the array by initializing the element in the array
//   with a call to its copy constructor and increments the count of this array.
//   Must be called after empty_ensure_count_undef() and no more elements than space
//   has been made for may be appended.
//
// #Examples  array.append_last_undef(elem);
// #See Also  empty_ensure_count_undef()
// #Author(s) Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::append_last_undef(
  // the element to append a copy of
  const _ElementType & elem
  )
  {
  new (m_array_p + m_count) _ElementType(elem);
  m_count++;
  }

//---------------------------------------------------------------------------------------
// Transfer ownership assignment
// Arg         array_p - address of array to take contents from and then empty
// Author(s):   Conan Reis
template<class _ElementType>
void AVCompactArrayBase<_ElementType>::assign(tAVCompactArrayBase * array_p)
  {
  empty();

  m_count = array_p->m_count;
  array_p->m_count = 0u;

  m_array_p = array_p->m_array_p;
  array_p->m_array_p = nullptr;
  }

//---------------------------------------------------------------------------------------
//  Removes all elements from collection
// Examples:    array.empty();
// Author(s):    Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::empty()
  {
  if (m_count)
    {
    dtor_elems(m_array_p, m_count);
    free_array(m_array_p);

    m_array_p = nullptr;
    m_count   = 0u;
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   Empties array (calling any element destructors) and then sets the size of the array
//   to be able to hold "count" elements - though it does *not* call the constructor for
//   any elements and it sets the current count to 0.  Call append_last_undef() to
//   add/construct an element and to increment the current count or call similar such
//   methods.
//
// #Notes
//   *** Be careful using this method when working with a sorted array ***
//   This method is useful when setting the elements by hand using index methods or
//   manipulating the element array directly.
//
// #See Also  append_last_undef()
// #Author(s) Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::empty_ensure_count_undef(uint32_t count)
  {
  if (m_count == 0u)
    {
    if (m_count != count)
      {
      m_array_p = alloc_array(count);
      }
    }
  else
    {
    dtor_elems(m_array_p, m_count);

    if (m_count != count)
      {
      free_array(m_array_p);
      m_array_p = alloc_array(count);
      }
    }

  m_count = 0u;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Inserts copy of elem at index pos
//
// #Examples
//   array.insert(elem, 5u);
//
// #Notes
//   !!!!! Use this method with caution !!!  It does not ensure that pos is the correct
//   sort position for elem - esp. with AVCompactSorted<>.
//   This method performs index range checking when A_BOUNDS_CHECK is defined.  If an
//   index is out of bounds, an AEx<APCompactArrayBase<>> exception is thrown.
//   A_BOUNDS_CHECK is defined by default in debug mode and turned off in release mode.
//
// #See Also  operator[], append*()
// #Author(s) Conan Reis
template<class _ElementType>
void AVCompactArrayBase<_ElementType>::insert(
  // element to insert
  const _ElementType & elem,
  // index position to insert element before.  Any element currently at this index is
  // pushed one index higher.
  uint32_t pos // = 0u
  )
  {
  AVCOMPACTARRAY_BOUNDS_LENGTH(pos);

  uint32_t       old_count = m_count;
  uint32_t       new_count = old_count + 1u;
  _ElementType * array_p   = alloc_array(new_count);

  if (old_count)
    {
    _ElementType * old_array_p = m_array_p;

    // Copy any elements prior to insert pos
    if (pos)
      {
      ::memcpy(array_p, old_array_p, pos * sizeof(_ElementType));
      }


    // Copy any elements following insert pos
    uint32_t post_count = old_count - pos;

    if (post_count)
      {
      ::memcpy(array_p + pos + 1u, old_array_p + pos, post_count * sizeof(_ElementType));
      }


    free_array(old_array_p);
    }

  m_count   = new_count;
  m_array_p = array_p;

  // insert element
  new (m_array_p + pos) _ElementType(elem);
  }

//---------------------------------------------------------------------------------------
// #Description
//   Removes elem at index pos
//
// #Examples
//   array.remove(5u);
//
// #Notes
//   This method performs index range checking when A_BOUNDS_CHECK is defined.  If an
//   index is out of bounds, an AEx<APCompactArrayBase<>> exception is thrown.
//   A_BOUNDS_CHECK is defined by default in debug mode and turned off in release mode.
//
// #See Also  operator[], append*()
// #Author(s) Conan Reis
template<class _ElementType>
void AVCompactArrayBase<_ElementType>::remove(
  // index position to remove element.  Any element after this index has its index reduced
  // by one.
  uint32_t pos // = 0u
  )
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(pos);

  _ElementType * old_array_p = m_array_p;

  // Call destructor on removed element
  old_array_p[pos].~_ElementType();

  uint32_t old_count = m_count;
  uint32_t new_count = old_count - 1u;

  if (new_count)
    {
    _ElementType * array_p = alloc_array(new_count);

    // Copy any elements prior to removed pos
    if (pos)
      {
      ::memcpy(array_p, old_array_p, pos * sizeof(_ElementType));
      }


    // Copy any elements following removed pos
    uint32_t post_count = old_count - pos - 1u;

    if (post_count)
      {
      ::memcpy(array_p + pos, old_array_p + pos + 1u, post_count * sizeof(_ElementType));
      }


    free_array(old_array_p);
    m_array_p = array_p;
    m_count   = new_count;
    }
  else
    {
    free_array(old_array_p);
    m_array_p = nullptr;
    m_count   = 0u;
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   *** Potentially Unsafe Method *** Sets element count of the array, but does not
//   change the internal pointer array in any way - the new length must also be no larger
//   than the current size of the internal array.  If the old length was smaller than the
//   new length then elements that follow the old length may be undefined.
//
// #Notes
//   *** Be careful using this method ***
//   This method is useful when setting the elements by hand using index methods or
//   manipulating the element array directly.
//
// #Modifiers 
// #See Also  empty_ensure_count_undef(), append_last_undef()
// #Author(s) Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::set_count_unsafe(uint32_t count)
  {
  m_count = count;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Applies the supplied invoke_obj to elem_count elements starting at index pos.
// 
// Params:
//   invoke_obj:
//     this may be either:
//
//       - a simple static function which is invoked with a pointer to each element in the
//         specified range.  The function must be of the form*:
//
//         void apply_func(_ElemType * elem_p)
//
//       - a 'function object' with an invocation operator() method of the form*:
//
//         void operator() (_ElemType * elem_p)
//
//         This invocation operator will be invoked with a pointer to each element in the
//         specified range.  The 'function object' may contain additional working info,
//         call methods of other objects, etc.  This is difficult to do with a simple
//         static function - esp. if thread safety is important.
//
//         A side note: the 'function object' must have external scope - i.e. its class
//         may not be defined locally [nested] in the method calling this method.
//         [However, the class can be defined just outside of the method - hopefully using
//         a namespace, etc. so that it is not global.]
//         CReis This may be MS Visual C++ specific and not ANSI C++ specific.
//
//       * [Actually, the return type is ignored and it may have more than one argument
//         provided that a default is provided for any additional arguments.]
//
//   pos: starting index position of elements to apply invoke_obj to
//   elem_count:
//     number of elements to apply invoke_obj to.  If elem_count is ALength_remainder, the
//     number of elements freed = length - pos.
//     
// Notes:
//   This method performs index range checking when A_BOUNDS_CHECK is defined. If an index
//   is out of bounds, a AEx<ArrayBase<>> exception is thrown. A_BOUNDS_CHECK is defined
//   by default in debug mode and turned off in release mode.
//   
// Examples:  array.apply(scan_elems);
// See:       apply_method(), APArrayBase<>::apply()
// Author(s): Conan Reis
template<class _ElementType>
template<class _InvokeType>
inline void AVCompactArrayBase<_ElementType>::apply(
  _InvokeType & invoke_obj,
  uint32_t      pos,       // = 0u
  uint32_t      elem_count // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = m_count - pos;
    }

  if (elem_count)
    {
    AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      invoke_obj(*array_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Calls the supplied *non-const* `method_m` method on `elem_count` elements starting at
// index `pos`.
// 
// Params:
//   method_m:
//     method address of a method available to `_ElementType`.  It must take no arguments
//     and return no arguments.
//   pos: starting index position of elements to call `method_m` on.
//   elem_count:
//     number of elements to call `method_m` on.  If `elem_count` is `ALength_remainder`,
//     the `number of elements freed` = `length` - `pos`.
//     
// Notes:
//   This method performs index range checking when `A_BOUNDS_CHECK` is defined.  If an
//   index is out of bounds, a `AEx<ArrayBase<>>` exception is thrown. `A_BOUNDS_CHECK` is
//   defined by default in debug mode and turned off in release mode.
//   
// Examples:  array.apply_method(do_stuff);
// See:       apply(), AVCompactArrayBase<_>::apply_method()
// Author(s): Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::apply_method(
  void (_ElementType::* method_m)(),
  uint32_t              pos,       // = 0u
  uint32_t              elem_count // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = m_count - pos;
    }

  if (elem_count)
    {
    AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      (array_p->*method_m)();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Calls the supplied *constant* `method_m` method on `elem_count` elements starting at
// index `pos`.
// 
// Params:
//   method_m:
//     method address of a method available to `_ElementType`.  It must take no arguments
//     and return no arguments.
//   pos: starting index position of elements to call `method_m` on.
//   elem_count:
//     number of elements to call `method_m` on.  If `elem_count` is `ALength_remainder`,
//     the `number of elements freed` = `length` - `pos`.
//     
// Notes:
//   This method performs index range checking when `A_BOUNDS_CHECK` is defined.  If an
//   index is out of bounds, a `AEx<ArrayBase<>>` exception is thrown. `A_BOUNDS_CHECK` is
//   defined by default in debug mode and turned off in release mode.
//   
// Examples:  array.apply_method(do_stuff);
// See:       apply(), AVCompactArrayBase<_>::apply_method()
// Author(s): Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::apply_method(
  void (_ElementType::* method_m)() const,
  uint32_t              pos,       // = 0u
  uint32_t              elem_count // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = m_count - pos;
    }

  if (elem_count)
    {
    AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      (array_p->*method_m)();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the binary information needed to
//             recreate the elements and increments the memory address to just past
//             the last byte written.  Does not store element count.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing ppBinary to this
//             method.
// See:        as_binary_elems_length(), as_binary(), as_binary8()
// Notes:      Used in combination with as_binary_elems_length().
//
//             Binary composition:
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
void AVCompactArrayBase<_ElementType>::as_binary_elems(
  void ** binary_pp
  ) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, as_binary_elems_length());

  // n bytes - element binary }- repeating
  _ElementType * elems_p = m_array_p;
  _ElementType * elems_end_p = elems_p + m_count;

  for (; elems_p < elems_end_p; elems_p++)
    {
    elems_p->as_binary(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the binary information needed to
//             recreate the elements and increments the memory address to just past
//             the last byte written.  Uses 32-bits to store element count.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing ppBinary to this
//             method.
// See:        as_binary_length(), as_binary8(), as_binary_length8()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - element count
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::as_binary(
  void ** binary_pp
  ) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, as_binary_length());

  // 4 bytes - element count
  uint32_t count = m_count;
  A_BYTE_STREAM_OUT32(binary_pp, &count);

  if (count)
    {
    // n bytes - element binary }- repeating
    as_binary_elems(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             create the element binaries and increments the memory address to just past
//             the last byte written.  Uses 8-bits to store element count.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing ppBinary to this
//             method.
// See:        as_binary_length8(), as_binary(), as_binary_length()
// Notes:      Used in combination with as_binary_length8().
//
//             Binary composition:
//               1 byte  - element count
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::as_binary8(
  void ** binary_pp
  ) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, as_binary_length8());

  // 1 byte - elements
  uint8_t count = uint8_t(m_count);
  A_BYTE_STREAM_OUT8(binary_pp, &count);

  // n bytes - element binary }- repeating
  as_binary_elems(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.  Does not store
//             element count.
// Returns:    length of binary version of itself in bytes
// See:        as_binary_elems(), as_binary(), as_binary8(), as_binary_length8()
// Notes:      Used in combination with as_binary_elems()
//
//             Binary composition:
//               4 bytes - element count
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
inline uint32_t AVCompactArrayBase<_ElementType>::as_binary_elems_length() const
  {
  uint32_t bytes = 0u;

  // n bytes - Element binary }- Repeating
  _ElementType * elems_p = m_array_p;
  _ElementType * elems_end_p = elems_p + m_count;

  for (; elems_p < elems_end_p; elems_p++)
    {
    bytes += elems_p->as_binary_length();
    }

  return bytes;
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.  Uses 32-bits for
//             element count.
// Returns:    length of binary version of itself in bytes
// See:        as_binary(), as_binary8(), as_binary_length8()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - element count
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
uint32_t AVCompactArrayBase<_ElementType>::as_binary_length() const
  {
  return 4u + as_binary_elems_length();
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.  Uses 8-bits for
//             element count.
// Returns:    length of binary version of itself in bytes
// See:        as_binary8(), as_binary(), as_binary_length()
// Notes:      Used in combination with as_binary8()
//
//             Binary composition:
//               1 byte  - element count
//               n bytes - element binary }- repeating
// Author(s):   Conan Reis
template<class _ElementType>
uint32_t AVCompactArrayBase<_ElementType>::as_binary_length8() const
  {
  return 1u + as_binary_elems_length();
  }

//---------------------------------------------------------------------------------------
// #Description
//   Tracks dynamic memory used by the element objects in this array
//
// #See Also  SkDebug, AMemoryStats
// #Author(s) Conan Reis
template<class _ElementType>
  // this array's buffer size in bytes
  uint32_t
AVCompactArrayBase<_ElementType>::track_memory(AMemoryStats * mem_stats_p) const
  {
  _ElementType * elem_p     = m_array_p;
  _ElementType * elem_end_p = elem_p + m_count;

  while (elem_p < elem_end_p)
    {
    elem_p->track_memory(mem_stats_p);

    elem_p++;
    }

  return m_count * sizeof(void *);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Static sorted constructor/converter
// Returns:     itself
// Arg          length - number of elements in the array.  (Default 0u)
// Arg          array_p - buffer of pointers to type _ElemType.  (Default nullptr)
// Examples:    called by derived classes
// Modifiers:    protected
// Author(s):    Conan Reis
template<class _ElementType>
inline AVCompactArrayBase<_ElementType>::AVCompactArrayBase(
  uint32_t       length,  // = 0u
  _ElementType * array_p  // = nullptr
  ) :
  m_count(length),
  m_array_p(array_p)
  {
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Allocates and returns a _ElementType buffer.
// Returns:     returns a dynamic array of non-constructed/non-initialized _ElementType
//              element objects
// Arg          count - the number of elements to allocate
// Examples:    _ElementType * elems_p = alloc_array(20);
// Notes:       Throws an AEx<AVCompactArrayBase<>> exception if unable to allocate
//              sufficient memory.
// Modifiers:    protected
// Author(s):    Conan Reis
// Efficiency   Should check efficiency difference for memory allocation between new and
//              malloc().
template<class _ElementType>
inline _ElementType * AVCompactArrayBase<_ElementType>::alloc_array(uint32_t count)
  {
  _ElementType * buffer_p = count ? (_ElementType *)AgogCore::get_app_info()->malloc(
    sizeof(_ElementType) * count, "AVCompactArray") : nullptr;

  A_VERIFY_MEMORY(!count || (buffer_p != nullptr), tAVCompactArrayBase);

  return buffer_p;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Calls destructors for specified count of element objects.
//
// #Modifiers protected
// #See Also  alloc_array(), free_array()
// #Author(s) Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::dtor_elems(
  _ElementType * array_p,
  // number of elements to call destructor for
  uint32_t count
  )
  {
  _ElementType * array_end_p = array_p + count;

  while (array_p < array_end_p)
    {
    array_p->~_ElementType();
    array_p++;
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   Frees memory for specified element object array - does not call destructors
//
// #Modifiers protected
// #See Also  alloc_array(), dtor_elems()
// #Author(s) Conan Reis
template<class _ElementType>
inline void AVCompactArrayBase<_ElementType>::free_array(_ElementType * array_p)
  {
  if (array_p)
    {
    AgogCore::get_app_info()->free(array_p);
    }
  }

#ifdef __clang__
  #pragma clang diagnostic pop
#endif
