// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AVArrayBase class template
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AMemory.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// See AgogCore/ACompareBase.hpp for the class definitions of ACompareAddress and ACompareLogical

// $Revisit - CReis Consider whether a specialization for void * would reduce code replication in
// simple pointer based methods.

//---------------------------------------------------------------------------------------
// Notes    The AVArrayBase class template provides a dynamic length, persistent index (i.e.
//          once an element is appended, it may be accessed via an integer index),
//          insertable (i.e. an element may be inserted at a specific index position)
//          collection of _ElementType.
//
//          The various collection/list/array classes such as the AVArray and AVSorted
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
//          explicit instantiation declaration such as (Note using AVArray rather than
//          AVArrayBase since AVArrayBase may not be constructed except by derived classes):
//            template class AVArray<AString>;
// Arg      _ElementType - the class/type of elements to be pointed to by the array.
// See Also
//   AVArrayBase<>                 - dynamic pointer array base class
//   
//     AVSizedArrayBase<>          - Lazy size buffer - at least 4 bytes larger than AVCompactArrayBase collections.  Array buffer may be larger size than actual number of elements so that it does not need to be resized with each add/remove of an element.
//       AVArray<>                 - Ordered array of pointers to elements with retrieval by key type
//         AVArrayFree<>           - Same as AVArray<>, but calls free_all() on its destruction
//         AVArrayLogical<>        - Same as AVArray<>, but uses the comparison operators < and == to sort elements
//       AVSorted<>                - AVSorted array of pointers to elements with retrieval and sorting by key type
//         AVSortedFree<>          - Same as AVSorted<>, but calls free_all() on its destruction
//         AVSortedLogical<>       - Same as AVSorted<>, but uses the comparison operators < and == to sort elements
//           AVSortedLogicalFree<> - Same as AVSortedLogical<>, but calls free_all() on its destruction
//     
//     AVCompactArrayBase<>        - array buffer is always = # elements.  Less memory though may be slower with add/remove
//       AVCompactArray<>          - Ordered array of pointers to elements with retrieval by key type
//         AVCompactArrayFree<>    - Same as AVCompactArray<>, but calls free_all() on its destruction
//         AVCompactArrayLogical<> - Same as AVCompactArray<>, but uses the comparison operators < and == to sort elements
//     
// Author   Conan Reis
template<class _ElementType>
class AVArrayBase
  {
  public:
  // Common types

    // Local shorthand for AVArrayBase template
    typedef AVArrayBase<_ElementType> tAVArrayBase;

  // Accessor methods

    _ElementType *        get_array();
    _ElementType const *  get_array() const;
    _ElementType *        get_array_end();
    _ElementType const *  get_array_end() const;
    _ElementType *        begin()       { return get_array(); } // So C++11 shorthands can be used on this
    _ElementType const *  begin() const { return get_array(); } // So C++11 shorthands can be used on this
    _ElementType *        end()         { return get_array_end(); }
    _ElementType const *  end() const   { return get_array_end(); }
    _ElementType &        get_at(uint32_t pos);
    _ElementType const &  get_at(uint32_t pos) const;
    _ElementType &        get_first();
    _ElementType const &  get_first() const;
    _ElementType &        get_last();
    _ElementType const &  get_last() const;
    uint32_t              get_length() const;
    bool                  is_empty() const;
    bool                  is_filled() const;
    void                  set_at(uint32_t pos, const _ElementType & elem);
    void                  set_length_unsafe(uint32_t length);
    _ElementType &        operator()(uint32_t pos);
    _ElementType const &  operator()(uint32_t pos) const;
    _ElementType &        operator[](uint32_t pos);
    _ElementType const &  operator[](uint32_t pos) const;

  // Non-modifying Methods

    template<class _InvokeType>
      void apply(_InvokeType & invoke_obj, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;

    void apply_method(void (_ElementType::* method_m)(), uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;
    void apply_method(void (_ElementType::* method_m)() const, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;

    // Binary Serialization Methods

      void     as_binary_elems(void ** binary_pp) const;
      void     as_binary(void ** binary_pp) const;
      void     as_binary8(void ** binary_pp) const;
      uint32_t as_binary_elems_length() const;
      uint32_t as_binary_length() const;
      uint32_t as_binary_length8() const;

    // Future methods
    //template<class _InvokeType>
    //  void         apply_reverse(_InvokeType & invoke_obj, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;
    //               as_ other collections
    //uint32_t       count(Select * sel_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder) const;
    //bool           find(Select * sel_p,  uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder) const;
    //bool           find_reverse(Select * sel_p = nullptr, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder) const;
    //_ElementType & get(Select * sel_p, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder) const;
    //void           get_all(AVArray * collected_p, Select * sel_p,  uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder) const;

  protected:
  // Internal Methods

    AVArrayBase(uint32_t length = 0u, _ElementType * array_p = nullptr);

  // Internal Class Methods

    static _ElementType * alloc_array(uint32_t needed);
    static void           free_array(_ElementType * array_p);

    static void  dtor_elems(_ElementType * elems_p, uint32_t count);
    static void  copy_elems(_ElementType * dest_p, const _ElementType * source_p, uint32_t count);

  // Data members

    _ElementType *  m_array_p;   // Dynamically sizing buffer of pointers to elements
    uint32_t        m_count;     // Number of elements currently in m_array_p

  #ifdef A_BITS64
    // This is a hack to use otherwise wasted space on 64-bit architectures
    // Reduces the size of all array types from 24 bytes to 16 bytes
    // This member variable is not used by this class at all, only by derived classes
    uint32_t        m_size;     // Size of this->m_array_p buffer
  #endif
  };  // AVArrayBase


//=======================================================================================
// Macros
//=======================================================================================

#ifdef A_BOUNDS_CHECK

// These bounds checks are macros rather than methods so that they are a bit faster in debug mode.

  //---------------------------------------------------------------------------------------
  #define AVARRAY_BOUNDS_CHECK(_pos) \
    A_VERIFY( \
      ((_pos) < this->m_count), \
      a_cstr_format(" - invalid index\nGiven %u but length only %u", (_pos), this->m_count), \
      AErrId_invalid_index, \
      tAVArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVARRAY_BOUNDS_LENGTH(_length) \
    A_VERIFY( \
      ((_length) <= this->m_count), \
      a_cstr_format(" - invalid length or insertion index\nGiven %u but length only %u", (_length), this->m_count), \
      AErrId_invalid_index, \
      tAVArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVARRAY_BOUNDS_CHECK_RANGE(_start_pos, _end_pos) \
    A_VERIFY( \
      (((_start_pos) < this->m_count) && ((_end_pos) < this->m_count) && ((_start_pos) <= (_end_pos))), \
      a_cstr_format("(start_pos %u, end_pos %u) - invalid index(es)\nLength is %u", (_start_pos), (_end_pos), this->m_count), \
      AErrId_invalid_index_range, \
      tAVArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVARRAY_BOUNDS_CHECK_SPAN(_pos, _elem_count) \
    A_VERIFY( \
      (((_pos) < this->m_count) && ((_elem_count) <= (this->m_count - (_pos)))), \
      a_cstr_format("(pos %u, elem_count %u) - invalid index(es)\nLength is %u", (_pos), (_elem_count), this->m_count), \
      AErrId_invalid_index_span, \
      tAVArrayBase)

  //---------------------------------------------------------------------------------------
  #define AVARRAY_BOUNDS_CHECK_ARRAY_SPAN(_array, _pos, _elem_count) \
    A_VERIFY( \
      (((_pos) < (_array).m_count) && ((_elem_count) <= ((_array).m_count - (_pos)))), \
      a_cstr_format("(pos %u, elem_count %u) - invalid index(es)\nLength is %u", (_pos), (_elem_count), (_array).m_count), \
      AErrId_invalid_index_span, \
      tAVArrayBase)

#else

  #define AVARRAY_BOUNDS_CHECK(_pos)                                  (void(0))
  #define AVARRAY_BOUNDS_LENGTH(_length)                              (void(0))
  #define AVARRAY_BOUNDS_CHECK_RANGE(_start_pos, _end_pos)            (void(0))
  #define AVARRAY_BOUNDS_CHECK_SPAN(_pos, _elem_count)                (void(0))
  #define AVARRAY_BOUNDS_CHECK_ARRAY_SPAN(_array, _pos, _elem_count)  (void(0))

#endif  // A_BOUNDS_CHECK


//=======================================================================================
// Methods
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Determines if the array is empty - i.e. whether it has no elements or not.
// Returns:     true if empty, false if not
// Examples:    if (array.is_empty())
// Notes:       This method is equivalent to !is_filled().
// See:         is_filled(), get_length()
// Author(s):    Conan Reis
template<class _ElementType>
inline bool AVArrayBase<_ElementType>::is_empty() const
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
inline bool AVArrayBase<_ElementType>::is_filled() const
  {
  return (m_count != 0u);
  }

//---------------------------------------------------------------------------------------
//  Get the array of pointers to _ElementType objects.
// Returns:     static array of pointers to _ElementType objects
// Examples:    _ElementType * elems_p = array.get_array();
// Notes:       This can be used for evil purposes, be careful!  ONLY change the elements
//              in the manner proscribed by the array type - i.e. elements in sorted
//              arrays are expected to be sorted.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * AVArrayBase<_ElementType>::get_array()
  {
  return m_array_p;
  }
template<class _ElementType>
inline _ElementType const * AVArrayBase<_ElementType>::get_array() const
  {
  return m_array_p;
  }

//---------------------------------------------------------------------------------------
//  Get the end (of the elements) of the array of pointers to _ElementType
//              objects.
// Returns:     static array of pointers to _ElementType objects
// Examples:    _ElementType * elems_end_p = array.get_array_end();
// Notes:       This can be used for evil purposes, be careful!  ONLY change the elements
//              in the manner proscribed by the array type - i.e. elements in sorted
//              arrays are expected to be sorted.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * AVArrayBase<_ElementType>::get_array_end()
  {
  return m_array_p + m_count;
  }
template<class _ElementType>
inline const _ElementType * AVArrayBase<_ElementType>::get_array_end() const
  {
  return m_array_p + m_count;
  }

//---------------------------------------------------------------------------------------
//  Get pointer to element at index position pos.
// Returns:     pointer to element at index position pos
// Arg          pos - index position of element to return
// Examples:    _ElementType elem = array.get_at(5);
// See:         operator()
// Notes:       This method is synonymous to operator()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVArrayBase<_ElementType>::get_at(uint32_t pos)
  {
  AVARRAY_BOUNDS_CHECK(pos);
  return m_array_p[pos];
  }
template<class _ElementType>
inline _ElementType const & AVArrayBase<_ElementType>::get_at(uint32_t pos) const
  {
  AVARRAY_BOUNDS_CHECK(pos);
  return m_array_p[pos];
  }

//---------------------------------------------------------------------------------------
//  Get pointer to first element or nullptr if there are no elements
// Returns:     pointer to first element or nullptr if there are no elements
// Examples:    _ElementType elem = array.get_first();
// See:         operator(), get_at(), get_last(), AVArray::operator[]
// Notes:      nullptr can be a valid element pointer value so a nullptr result could mean no
//             elements or just an element pointer set to nullptr only usage or getting the
//             length of this array will resolve the difference.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVArrayBase<_ElementType>::get_first()
  {
  AVARRAY_BOUNDS_CHECK(0);
  return m_array_p[0];
  }
template<class _ElementType>
inline _ElementType const & AVArrayBase<_ElementType>::get_first() const
  {
  AVARRAY_BOUNDS_CHECK(0);
  return m_array_p[0];
  }

//---------------------------------------------------------------------------------------
// Get pointer to last element or nullptr if there are no elements
// Returns:    pointer to last element or nullptr if there are no elements
// Examples:   _ElementType * elem_p = array.get_last();
// See:        operator(), get_at(), get_first(), AVArray::operator[]
// Notes:      nullptr can be a valid element pointer value so a nullptr result could mean no
//             elements or just an element pointer set to nullptr only usage or getting the
//             length of this array will resolve the difference.
// Author(s):   Conan Reis
template<class _ElementType>
inline _ElementType & AVArrayBase<_ElementType>::get_last()
  {
  AVARRAY_BOUNDS_CHECK(m_count - 1u);
  return m_array_p[m_count - 1u];
  }
template<class _ElementType>
inline _ElementType const & AVArrayBase<_ElementType>::get_last() const
  {
  AVARRAY_BOUNDS_CHECK(m_count - 1u);
  return m_array_p[m_count - 1u];
  }

//---------------------------------------------------------------------------------------
//  Get number of element pointers stored in the array
// Returns:     number of element pointers stored in the array
// Examples:    uint32_t length = array.get_length();
// See:         is_empty()
// Notes:       This returns the number of elements in the AVArrayBase, NOT the size of the
//              currently allocated internal static buffer.
// Author(s):    Conan Reis
template<class _ElementType>
inline uint32_t AVArrayBase<_ElementType>::get_length() const
  {
  return m_count;
  }

//---------------------------------------------------------------------------------------
//  Set pointer to element at index position pos.  *** Be very careful using
//              this method with sorted arrays - it is only valid if the element being
//              swapped would sort to the same position as the element it is replacing.
// Arg          pos - index position of element to replace
// Examples:    array.set_at(5, elem_p);
// See:         operator(), get_at(), append(), insert()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline void AVArrayBase<_ElementType>::set_at(
  uint32_t             pos,
  const _ElementType & elem
  )
  {
  AVARRAY_BOUNDS_CHECK(pos);
  m_array_p[pos] = elem;
  }

//---------------------------------------------------------------------------------------
//  Get pointer to element at index position pos.
// Returns:     pointer to element at index position pos
// Arg          pos - index position of element to return
// Examples:    _ElementType * elem_p = array(5);
// See:         get_at()
// Notes:       This method is synonymous to get_at()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVArrayBase<_ElementType>::operator()(uint32_t pos)
  {
  AVARRAY_BOUNDS_CHECK(pos);
  return m_array_p[pos];
  }
template<class _ElementType>
inline _ElementType const & AVArrayBase<_ElementType>::operator()(uint32_t pos) const
  {
  AVARRAY_BOUNDS_CHECK(pos);
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
//              If an index is out of bounds, a AEx<AVArray<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType & AVArrayBase<_ElementType>::operator[](uint32_t pos)
  {
  AVARRAY_BOUNDS_CHECK(pos);
  return m_array_p[pos];
  }
template<class _ElementType>
inline _ElementType const & AVArrayBase<_ElementType>::operator[](uint32_t pos) const
  {
  AVARRAY_BOUNDS_CHECK(pos);
  return m_array_p[pos];
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// *** Potentially Unsafe Method *** Sets length of the array, but does not
//             change the internal pointer array in any way - the new length must also be
//             no larger than the current size of the internal array.  If the old length
//             was smaller than the new length then elements that follow the old length
//             may be undefined.
// Arg         length - number of nullptr pointers elements the array should consist of.
// Examples:   array.set_length_null(5u);  // Makes the array have 5 nullptr elements
// See:        set_length_null(), ensure_length_null(), append_null(),
//             AVArray::append_at_null(), AVArray::null(), AVArray::append_all(elem_count, nullptr)
// Notes:      *** Be careful using this method ***
//             This method is useful when setting the elements by hand using index
//             methods or manipulating the element array directly.
// Author(s):   Conan Reis
template<class _ElementType>
inline void AVArrayBase<_ElementType>::set_length_unsafe(uint32_t length)
  {
  m_count = length;
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
// See:       apply_method(), AVCompactArrayBase<>::apply()
// Author(s): Conan Reis
template<class _ElementType>
template<class _InvokeType>
inline void AVArrayBase<_ElementType>::apply(
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
    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      invoke_obj(array_p);
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
inline void AVArrayBase<_ElementType>::apply_method(
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
    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

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
inline void AVArrayBase<_ElementType>::apply_method(
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
    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      (array_p->*method_m)();
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Applies the supplied apply_func_p static function to elem_count elements
//              starting at index pos.
// Arg          apply_func_p - pointer to static function to call with each element and
//              info_p as arguments
// Arg          info_p - optional pointer to data used by apply_func_p.  (Default nullptr)
// Arg          pos - starting index position of elements to apply function to
// Arg          elem_count - number of elements to apply function to.  If elem_count is
//              ALength_remainder, the number of elements freed = length - pos.
//              (Default ALength_remainder)
// Examples:    array.apply(scan_elems);
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVArrayBase<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
/*
template<class _ElementType>
void AVArrayBase<_ElementType>::apply(
  void (* apply_func_p)(_ElementType * elem_p, void * info_p),
  void * info_p,    // = nullptr
  uint32_t   pos,       // = 0u
  uint32_t   elem_count // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = m_count - pos;
    }
  if (apply_func_p && elem_count)
    {

    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    _ElementType * array_p     = m_array_p + pos;  // for faster than data member access
    _ElementType * array_end_p = array_p + elem_count;

    for (; array_p < array_end_p; array_p++)
      {
      (apply_func_p)(array_p, info_p);
      }
    }
  }
*/

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
void AVArrayBase<_ElementType>::as_binary_elems(
  void ** binary_pp
  ) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, as_binary_elems_length());

  // n bytes - element binary }- repeating
  _ElementType * elems_p     = m_array_p;
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
inline void AVArrayBase<_ElementType>::as_binary(
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
inline void AVArrayBase<_ElementType>::as_binary8(
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
uint32_t AVArrayBase<_ElementType>::as_binary_elems_length() const
  {
  uint32_t bytes = 0u;

  // n bytes - Element binary }- Repeating
  _ElementType * elems_p     = m_array_p;
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
uint32_t AVArrayBase<_ElementType>::as_binary_length() const
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
uint32_t AVArrayBase<_ElementType>::as_binary_length8() const
  {
  return 1u + as_binary_elems_length();
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
inline AVArrayBase<_ElementType>::AVArrayBase(
  uint32_t        length,  // = 0u
  _ElementType *  array_p  // = nullptr
  ) :
  m_array_p(array_p),
  m_count(length)
  {
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Allocates and returns a _ElementType pointer buffer.
// Returns:     returns a dynamic array of pointers to _ElementType  
// Arg          needed - the size of the buffer to allocate
// Examples:    _ElementType * elems_p = alloc_array(20);
// Notes:       Throws a AEx<AVArrayBase<>> exception if unable to allocate sufficient memory.
// Modifiers:    protected
// Author(s):    Conan Reis
template<class _ElementType>
inline _ElementType * AVArrayBase<_ElementType>::alloc_array(uint32_t needed)
  {
  _ElementType * buffer_p = needed ? (_ElementType *)AgogCore::get_app_info()->malloc(sizeof(_ElementType) * needed, "AVArrayBase.buffer") : nullptr;

  A_VERIFY_MEMORY(!needed || (buffer_p != nullptr), tAVArrayBase);

  return buffer_p;
  }

//---------------------------------------------------------------------------------------
template<class _ElementType>
inline void AVArrayBase<_ElementType>::free_array(_ElementType * array_p)
  {
  if (array_p)
    {
    AgogCore::get_app_info()->free(array_p);
    }
  }

//---------------------------------------------------------------------------------------

template<class _ElementType>
inline void AVArrayBase<_ElementType>::dtor_elems(_ElementType * elems_p, uint32_t count)
  {
  _ElementType * elems_end_p = elems_p + count;
  for (; elems_p < elems_end_p; ++elems_p)
    {
    elems_p->~_ElementType();
    }
  }

//---------------------------------------------------------------------------------------

template<class _ElementType>
void AVArrayBase<_ElementType>::copy_elems(_ElementType * dest_p, const _ElementType * source_p, uint32_t count)
  {
  _ElementType * dest_end_p = dest_p + count;
  for (; dest_p < dest_end_p; ++dest_p, ++source_p)
    {
    *dest_p = *source_p;
    }
  }
