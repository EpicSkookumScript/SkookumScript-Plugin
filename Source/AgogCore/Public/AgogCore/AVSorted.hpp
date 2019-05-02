// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AVSorted class template
//=======================================================================================


#ifndef _AVSORTED_HPP
#define _AVSORTED_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AVSizedArrayBase.hpp>
#include <AgogCore/ACompareBase.hpp>
#include <stdarg.h>          // Uses: va_array, va_start(), va_arg(), va_end()
#include <stdlib.h>          // Uses: qsort()


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Notes    The AVSorted class template provides a dynamic length, persistent index (i.e.
//          once an element is appended, it may be accessed via an integer index),
//          insertable (i.e. an element may be inserted at a specific index position)
//          collection of _ElementType.
//
//          Any modifications to this template should be compile-tested by adding an
//          explicit instantiation declaration such as:
//            template class AVSorted<AString>;
// Arg      _ElementType - the class/type of elements to be pointed to by the sorted.
// Arg      _KeyType - the class/type to use for searching and sorting.  It is often a
//          data member of the _ElementType.  To get access to the key, _ElementType must
//          have a conversion operator to the _KeyType (or the _KeyType must have a non-
//          explicit constructor/converter for _ElementType).
//          For example, it is common for classes to have a symbol name identifying them
//          and it is this name by which they are sorted.  So these element classes
//          could use ASymbol as the _KeyType and have 'operator const ASymbol & () const'
//          as a conversion operator.
//          The _KeyType may be required to possess various methods or even data members
//          due to the _CompareClass that is used.  For example, the default
//          _CompareClass is ACompareAddress<> which uses the memory address of the
//          _KeyType and so relies on no particular methods.  The next most common
//          _CompareClass is ACompareLogical<> which relies on the equal to (=) and the
//          less than (<) operators of the _KeyType.    
//          (Default  _ElementType)  So the keys are the elements themselves.
// Arg      _CompareClass - a class template that provides the static sorting functions
//          equals() and comparison().  These functions should be inlined so that there is
//          no function call overhead.  See ACompareAddress and ACompareLogical in
//          AgogCore/ACompareBase.hpp for examples.  See AgogCore/AString.hpp for the class definition of
//          ACompareStrInsensitive.  (Default ACompareAddress<_KeyType>)
// See Also
//   AVArrayBase<>                 - dynamic pointer array base class
//   
//     AVSizedArrayBase<>          - Lazy size buffer - at least 4 bytes larger than AVCompactArrayBase collections.  Array buffer may be larger size than actual number of elements so that it does not need to be resized with each add/remove of an element.
//       AVArray<>                 - Ordered array of pointers to elements with retrieval by key type
//         AVArrayFree<>           - Same as AVArray<>, but calls free_all() on its destruction
//         AVArrayLogical<>        - Same as AVArray<>, but uses the comparison operators < and == to sort elements
//       AVSorted<>                - AVSorted array of pointers to elements with retrieval and sorting by key type
//         AVSortedFree<>          - Same as AVSorted<>, but calls free_all() on its destruction
//     
//     AVCompactArrayBase<>        - array buffer is always = # elements.  Less memory though may be slower with add/remove
//       AVCompactArray<>          - Ordered array of pointers to elements with retrieval by key type
//         AVCompactArrayFree<>    - Same as AVCompactArray<>, but calls free_all() on its destruction
//         AVCompactArrayLogical<> - Same as AVCompactArray<>, but uses the comparison operators < and == to sort elements
//     
// Author   Conan Reis
template<
  class _ElementType,
  class _KeyType      = _ElementType,
  class _CompareClass = ACompareLogical<_KeyType>
  >
class AVSorted : public AVSizedArrayBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AVSorted<_ElementType, _KeyType, _CompareClass> tAVSorted;
    typedef AVSizedArrayBase<_ElementType>                  tAVSizedArrayBase;
    typedef AVArrayBase<_ElementType>                       tAVArrayBase;


  // Unhide Inherited Methods

    // Methods in this class with the same name as methods in AVSizedArrayBase<> are 'hidden'
    // (even if they do not have the same interface), this makes them visible again.
    // Ensure that any new methods added to this class that also have the same name
    // as methods in AVSizedArrayBase<> are included in this list to preserve the 'is-type-of'
    // relationship.  These using directives must precede the new methods in this class.
    using AVSizedArrayBase<_ElementType>::free;
    using AVSizedArrayBase<_ElementType>::free_all;
    using AVSizedArrayBase<_ElementType>::pop;
    using AVSizedArrayBase<_ElementType>::remove;
    using AVSizedArrayBase<_ElementType>::remove_all;
    void remove_fast(uint32_t) = delete; // remove_fast reorders elements therefore must not be used with AVSorted

  // Common methods

    AVSorted();
    AVSorted(const AVSorted & sorted);
    ~AVSorted();
    AVSorted & operator=(const AVSorted & sorted);

  // Converter Methods

    explicit AVSorted(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted);
    
  // Modifying Behaviour methods

    bool           append(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    bool           append_absent(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    void           append_absent_all(const AVSorted & sorted);
    void           append_all(const AVArrayBase<_ElementType> & array, bool pre_sorted = false);
    void           append_all(const AVSorted & sorted);
    void           append_all(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted = false);
    void           append_replace(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    void           append_replace_free_all(const AVSorted & sorted);
    bool           free(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all(const AVArrayBase<_KeyType> & array, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all(const _KeyType * keys_p, uint32_t key_count, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       free_all_all(const AVSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    _ElementType * pop(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           pop_all(AVSorted * collected_p, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder);
    uint32_t       pop_all(AVSorted * collected_p, const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       pop_all_all(AVSorted * collected_p, const AVSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    bool           remove(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    bool           remove_elem(const _ElementType & elem, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       remove_all(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       remove_all(const AVSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    uint32_t       remove_all_all(const AVSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           sort(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           xfer_absent_all_free_dupes(AVSorted * sorted_p);


  // Non-modifying Methods

    uint32_t       count(const _KeyType & key,  uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    AVSorted *     as_new() const;
    bool           find(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool           find_elem(const _ElementType & elem, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    _ElementType * get(const _KeyType & key) const;
    _ElementType * get(const _KeyType & key, uint32_t instance, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    void           get_all(AVSorted * collected_p, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;
    void           get_all(AVSorted * collected_p, const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    void           get_all(AVSorted * collected_p, const AVSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    uint32_t       get_instance(uint32_t index) const;
    void           validate_unique() const;


  protected:
  // Internal Methods

    bool find_instance(const _KeyType & key, uint32_t instance, uint32_t * find_pos_p, _ElementType * first_p, _ElementType * last_p) const;

  // Internal Class Methods

    static int sort_compare(const void * lhs_p, const void * rhs_p);

  };  // AVSorted


//---------------------------------------------------------------------------------------
// AVSortedLogicalFree only differs from AVSortedLogical in that it automatically calls
// free_all() in its destructor.  Note that although AVSortedLogicalFree is derived from
// AVSortedLogical, there is no loss in efficiency.
template<
  class _ElementType,
  class _KeyType      = _ElementType
  >
class AVSortedFree : public AVSorted<_ElementType, _KeyType>
  {
  public:
    // Local shorthand
    typedef AVArrayBase<_ElementType>         tAVArrayBase;
    typedef AVSorted<_ElementType, _KeyType>  tAVSorted;
    
  // All the constructors are hidden (stupid!), so make appropriate links
    AVSortedFree() : tAVSorted() {}
    AVSortedFree(const AVSorted<_ElementType, _KeyType, ACompareLogical<_KeyType> > & sorted) : tAVSorted(sorted) {}
    explicit AVSortedFree(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted);
    explicit AVSortedFree(int elem_count, ...);

    ~AVSortedFree() { this->free_all(); }
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
// Examples:    AVSorted<SomeClass> sorted;
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVSorted<_ElementType, _KeyType, _CompareClass>::AVSorted() :
  tAVSizedArrayBase(0u, 0u, nullptr)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor
// Returns:     itself
// Arg          sorted - sorted to copy
// Examples:    AVSorted<SomeClass> sorted1;
//              AVSorted<SomeClass> sorted2(sorted1);
// Notes:       If sorted points to a ACompareBase derived object for sorting, then the
//              address of that ACompareBase derived object is also pointed to by this
//              AVSorted object.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVSorted<_ElementType, _KeyType, _CompareClass>::AVSorted(const tAVSorted & sorted) :
  tAVSizedArrayBase(sorted.m_count, sorted.m_count, tAVArrayBase::alloc_array(sorted.m_count))
  {
  ::memcpy(this->m_array_p, sorted.m_array_p, this->m_count * sizeof(_ElementType));
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVSorted<_ElementType, _KeyType, _CompareClass>::~AVSorted()
  {
  tAVArrayBase::free_array(this->m_array_p);
  }

//---------------------------------------------------------------------------------------
//  Assignment operator
// Returns:     reference to itself to allow for stringization
//              sorted1 = sorted2 = sorted3;
// Examples:    sorted1 = sorted2;
// See:         append_all()
// Notes:       If sorted points to a ACompareBase derived object for sorting, then the
//              address of that ACompareBase derived object is also pointed to by this
//              AVSorted object.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
AVSorted<_ElementType, _KeyType, _CompareClass> & AVSorted<_ElementType, _KeyType, _CompareClass>::operator=(const tAVSorted & sorted)
  {
  this->m_count = sorted.m_count;

  if (this->m_count > this->m_size)
    {
    tAVArrayBase::free_array(this->m_array_p);
    this->m_size    = AMemory::request_pointer_count(this->m_count);
    this->m_array_p = tAVArrayBase::alloc_array(this->m_size);
    }

  ::memcpy(this->m_array_p, sorted.m_array_p, this->m_count * sizeof(_ElementType));
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Constructor (for pre-existing arrays of elements)
// Returns:     itself
// Arg          elems_p - array of elements of type _ElementType.
// Arg          elem_count - number of elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// Arg          pre_sorted - specifies whether the elements in elems_p are already
//              appropriately sorted.  If false, the elements are sorted.  If true, the
//              element order is left unchanged and the extra overhead of a sort is not
//              taken - ensure that they are sorted appropriately, or this AVSorted will
//              not function properly.
// See:         AVSorted<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
AVSorted<_ElementType, _KeyType, _CompareClass>::AVSorted(
  const _ElementType * elems_p,
  uint32_t             elem_count,
  bool                 pre_sorted // = false
  ) :
  tAVSizedArrayBase(0u, elem_count, nullptr)
  {
  // $Revisit - CReis Currently ignores 'pre_sorted'
  if (elem_count)
    {
    uint32_t             find_pos;
    const _ElementType * elem_p      = elems_p;
    const _ElementType * elems_end_p = elems_p + elem_count;
    _ElementType *       array_p     = tAVArrayBase::alloc_array(elem_count);  // For quick access

    this->m_array_p = array_p;

    for(; elem_p < elems_end_p; this->m_count++, elem_p++)
      {
      find(*elem_p, AMatch_first_found, &find_pos);
      ::memmove(array_p + find_pos + 1u, array_p + find_pos, (this->m_count - find_pos) * sizeof(_ElementType));
      new (array_p + find_pos) _ElementType(*elem_p); // insert element
      }
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Appends element to the current AVSorted array.  The appended element is
//              inserted at its appropriately sorted index position.  If there are other
//              matching (==) elements, the order of elem relative to the other matching
//              elements is undefined - it could be inserted to the the last or a middle
//              position, but it will be after the first.
// Returns:     true if element has unique key false if one or more elements have the
//              same key.
// Arg          elem - the element to append
// Arg          insert_pos_p - address to store the append insertion index position.  If
//              this pointer is set to nullptr, it is ignored.  (Default nullptr)
// Examples:    sorted.append(elem);
// See:         append_absent(), append_replace()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::append(
  const _ElementType & elem,
  uint32_t *           insert_pos_p // = nullptr
  )
  {
  uint32_t pos;

  bool found = find(elem, AMatch_first_found, &pos);

  if (this->m_size < this->m_count + 1u)  // Needs more space
    {
    _ElementType * old_array_p = this->m_array_p;

    this->m_size    = AMemory::request_pointer_count_expand(this->m_count + 1);
    this->m_array_p = tAVArrayBase::alloc_array(this->m_size);
    ::memcpy(this->m_array_p, old_array_p, pos * sizeof(_ElementType));
    ::memcpy(this->m_array_p + pos + 1u, old_array_p + pos, (this->m_count - pos) * sizeof(_ElementType));
    tAVArrayBase::free_array(old_array_p);
    }
  else  // enough size in existing array
    {
    ::memmove(this->m_array_p + pos + 1u, this->m_array_p + pos, (this->m_count - pos) * sizeof(_ElementType));
    }

  new (this->m_array_p + pos) _ElementType(elem); // insert element
  this->m_count++;

  if (insert_pos_p)
    {
    *insert_pos_p = pos;
    }

  return !found;
  }

//---------------------------------------------------------------------------------------
//  Appends an element to the AVSorted array if it is not already present.
// Returns:     true if element appended, false if not
// Arg          elem - element to append
// Arg          insert_pos_p - address to store the index position that elem was inserted
//              or found.  If insert_pos_p is nullptr, it is not modified.  (Default nullptr)
// Examples:    sorted.append_absent(elem);
// See:         append(), append_replace()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::append_absent(
  const _ElementType & elem,
  uint32_t *           insert_pos_p // = nullptr
  )
  {
  uint32_t pos;
  bool found = find(elem, AMatch_first_found, &pos);

  if (!found)
    {
    this->insert(elem, pos);
    }

  if (insert_pos_p)
    {
    *insert_pos_p = pos;
    }

  return !found;
  }

//---------------------------------------------------------------------------------------
// Appends all elements from sorted that do not already exist in the current array.
//
// #Examples
//   sorted1.append_absent_all(sorted2);
//
// #Notes
//   This is essentially a 'set union'.
//   [Efficiency] Instead of just sort() - sort only the new elements then do merge sort
//   of previous elements and new elements.
//
// #See Also
//   get_all() - 'set intersection', remove_all() - 'set subtraction',
//   xfer_absent_all_free_dupes()
//   
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_absent_all(
  // elements to append
  const tAVSorted & sorted
  )
  {
  uint32_t sorted_length = sorted.m_count;

  if (sorted_length)
    {
    uint32_t length = this->m_count;
    uint32_t size   = this->m_size;

    // An ensure_size() before all the appends could needlessly expand the size of the
    // array if the elements are already present.

    uint32_t       pos;
    _ElementType * array_p     = this->m_array_p;  // for faster than data member access
    _ElementType * elems_p     = sorted.m_array_p;
    _ElementType * elems_end_p = elems_p + sorted_length;
    _ElementType * old_array_p;

    for (; elems_p < elems_end_p; elems_p++)
      {
      if (!find(*elems_p, AMatch_first_found, &pos))
        {
        // Resize needed?
        if (size < length + 1u)  // Needs more space
          {
          old_array_p = this->m_array_p;

          size            = AMemory::request_pointer_count_expand(length + 1u);
          this->m_size    = size;
          array_p         = tAVArrayBase::alloc_array(this->m_size);
          this->m_array_p = array_p;
          ::memcpy(array_p, old_array_p, pos * sizeof(_ElementType));
          ::memcpy(array_p + pos + 1u, old_array_p + pos, (length - pos) * sizeof(_ElementType));
          tAVArrayBase::free_array(old_array_p);
          }
        else  // enough size in existing array
          {
          ::memmove(array_p + pos + 1u, array_p + pos, (length - pos) * sizeof(_ElementType));
          }

        new (array_p + pos) _ElementType(*elems_p); // insert element
        length++;

        this->m_count = length;
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Appends elem_count elements from elems_p to the current sorted.
// Arg         array - array of elements to append
// Arg         pre_sorted - specifies whether the elements in array are already
//             appropriately sorted.  If false, the elements are sorted.  If true, the
//             element order is left unchanged and the extra overhead of a sort is not
//             taken - ensure that they are sorted appropriately, or this AVSorted will
//             not function properly.
// Examples:   sorted.append_all(objs);
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_all(
  const AVArrayBase<_ElementType> & array,
  bool                              pre_sorted // = false
  )
  {
  uint32_t length = array.get_length();

  this->ensure_size(this->m_count + length);
  ::memcpy(this->m_array_p + this->m_count, array.get_array(), length * sizeof(_ElementType));
  this->m_count += length;

  // Check for special case where no merge is necessary
  if ((!pre_sorted) || (this->m_count != length))
    {
    // $Revisit - CReis [Efficiency] Instead of just sort() - sort only the new elements if
    // presorted, then do merge sort of previous elements and new elements.
    sort();
    }
  }

//---------------------------------------------------------------------------------------
//  Appends all elements in sorted to the current sorted.
// Arg          sorted - sorted of elements to append
// Examples:    sorted.append_all(sorted);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_all(const tAVSorted & sorted)
  {
  uint32_t length = sorted.get_length();

  this->ensure_size(this->m_count + length);
  ::memcpy(this->m_array_p + this->m_count, sorted.get_array(), length * sizeof(_ElementType));
  this->m_count += length;

  // Check for special case where no merge is necessary
  if (this->m_count != length)
    {
    // $Revisit - CReis [Efficiency] Instead of just sort() - sort only the new elements
    // then do merge sort of previous elements and new elements.
    sort();
    }
  }

//---------------------------------------------------------------------------------------
//  Appends elem_count elements from elems_p to the current sorted.
// Arg          elems_p - array of pointers to elements of type _ElementType
// Arg          elem_count - number of pointers to elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// Arg          pre_sorted - specifies whether the elements in elems_p are already
//              appropriately sorted.  If false, the elements are sorted.  If true, the
//              element order is left unchanged and the extra overhead of a sort is not
//              taken - ensure that they are sorted appropriately, or this AVSorted will
//              not function properly.
// Examples:    sorted.append_all(objs.get_array(), objs.get_length());
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_all(
  const _ElementType * elems_p,
  uint32_t             elem_count,
  bool                 pre_sorted // = false
  )
  {
  if (elem_count)
    {
    this->ensure_size(this->m_count + elem_count);

    uint32_t           find_pos;
    const _ElementType * elem_p      = elems_p;
    const _ElementType * elems_end_p = elems_p + elem_count;
    _ElementType **      array_p     = this->m_array_p;  // For quick access

    // $Revisit - CReis [Efficiency] Appropriately use 'pre_sorted'

    for(; elem_p < elems_end_p; elem_p++)
      {
      find(*elem_p, AMatch_first_found, &find_pos);
      ::memmove(array_p + find_pos + 1u, array_p + find_pos, (this->m_count - find_pos) * sizeof(_ElementType));
      new (array_p + find_pos) _ElementType(*elem_p); // insert element
      this->m_count++;
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Appends an element to the AVSorted array if it is not already present or
//              replaces first found matching element.
// Returns:     pointer to the replaced element or nullptr if the element was appended, but
//              there was no matching element
// Arg          elem - element to append
// Arg          insert_pos_p - address to store the index position that elem was inserted
//              or replaced.  If insert_pos_p is nullptr, it is not modified.  (Default nullptr)
// Examples:    sorted.append_replace(elem);
// See:         append(), append_absent()
// Notes:       This treats the AVSorted array like a set - all elements are unique (no
//              duplicates)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_replace(
  const _ElementType & elem,
  uint32_t *           insert_pos_p // = nullptr
  )
  {
  uint32_t       pos;

  if (find(elem, AMatch_first_found, &pos))
    {
    this->m_array_p[pos].~_ElementType();
    new (this->m_array_p + pos) _ElementType(elem); // insert element
    }
  else
    {
    this->insert(elem, pos);
    }

  if (insert_pos_p)
    {
    *insert_pos_p = pos;
    }
  }

//---------------------------------------------------------------------------------------
// Appends all elements from sorted into the current array and for any elements that match
// the original element is freed and replaced with the element from sorted.
//
// #Examples
//   sorted1.append_replace_free_all(sorted2);
//
// #Notes
//   This is essentially a form of a 'set union'.
//
// #See Also
//   get_all() - 'set intersection', remove_all() - 'set subtraction',
//   xfer_absent_all_free_dupes()
//   
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::append_replace_free_all(
  // elements to append
  const tAVSorted & sorted
  )
  {
  uint32_t sorted_length = sorted.m_count;

  if (sorted_length)
    {
    uint32_t length = this->m_count;
    uint32_t size   = this->m_size;

    // An ensure_size() before all the appends could needlessly expand the size of the
    // array if the elements are already present.

    uint32_t       pos;
    _ElementType * array_p     = this->m_array_p;  // for faster than data member access
    _ElementType * elems_p     = sorted.m_array_p;
    _ElementType * elems_end_p = elems_p + sorted_length;
    _ElementType * old_array_p;

    for (; elems_p < elems_end_p; elems_p++)
      {
      if (!find(*elems_p, AMatch_first_found, &pos))
        {
        // Resize needed?
        if (size < length + 1u)  // Needs more space
          {
          old_array_p = this->m_array_p;

          size            = AMemory::request_pointer_count_expand(length + 1u);
          this->m_size    = size;
          array_p         = tAVArrayBase::alloc_array(this->m_size);
          this->m_array_p = array_p;
          ::memcpy(array_p, old_array_p, pos * sizeof(_ElementType));
          ::memcpy(array_p + pos + 1u, old_array_p + pos, (length - pos) * sizeof(_ElementType));
          tAVArrayBase::free_array(old_array_p);
          }
        else  // enough size in existing array
          {
          ::memmove(array_p + pos + 1u, array_p + pos, (length - pos) * sizeof(_ElementType));
          }

        new (array_p + pos) _ElementType(*elems_p); // insert element
        length++;

        this->m_count = length;
        }
      else
        {
        // Element already present - free it and replace with new element
        array_p[pos].~_ElementType();
        new (array_p + pos) _ElementType(*elems_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) instance of element matching key between
//              start_pos and end_pos, returning true if found, false if not.
// Returns:     true if instance element freed, false if not
// Arg          key - key to match for element to free
// Arg          instance - occurrence of key to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//                - looking for instance AMatch_first_found could return e2_1, e2_2, or
//                  e2_3 depending on which element is happened upon first during the
//                  binary search.
//                - looking for instance AMatch_last will only return e2_3.
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that key was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for key  (Default 0)
// Arg          end_pos - last position to look for key.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.free(key, 2);  // free the second occurrence of key
// See:         pop(), remove()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::free(
  const _KeyType & key,
  uint32_t         instance,    // = AMatch_first_found
  uint32_t *       find_pos_p,  // = nullptr
  uint32_t         start_pos,   // = 0
  uint32_t         end_pos      // = ALength_remainder
  ) 
  {
  uint32_t find_pos;
  bool found = find(key, instance, &find_pos, start_pos, end_pos);

  if (found)
    {
    _ElementType ** array_pp = this->m_array_p;
    _ElementType *  elem_p   = array_pp[find_pos];

    // Remove from array first
    this->m_count--;           // new length of sorted
    ::memmove(array_pp + find_pos, array_pp + find_pos + 1u, (this->m_count - find_pos) * sizeof(_ElementType));

    // Then delete the element
    delete elem_p;
    }

  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return found;
  }

//---------------------------------------------------------------------------------------
//  Frees (removes and deletes) all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements freed
// Arg          key - key to match elements to free
// Arg          start_pos - first position to look for key  (Default 0)
// Arg          end_pos - last position to look for key.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.free_all(key);  // free all the elements matching key
// See:         pop_all(), remove_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
// Stability    Determine if a matching element in the last position is properly found
//              - i.e. this->m_array_p + end_pos.
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::free_all(
  const _KeyType & key,
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t first_match;
  uint32_t free_count = 0u;

  if (find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    free_count = last_match - first_match + 1u;
    this->free_all(first_match, free_count);
    }

  return free_count;
  }

//---------------------------------------------------------------------------------------
// #Description
//   For every key in the supplied "removing" array, free (remove and delete) the first
//   matching element (if one exists) in the current array between start_pos and end_pos.
//
// #Examples
//   sorted.free_all(to_free_array);
//
// #Notes
//   This method assumes that there are no elements with duplicate keys - free_all_all()
//   checks for duplicate keys.
//   The match test is performed using _CompareClass::equals(elem_i, elem_j).
//   This method performs index range checking when A_BOUNDS_CHECK is defined.  If an
//   index is out of bounds, a AEx<AVSorted<>> exception is thrown.  A_BOUNDS_CHECK is
//   defined by default in debug mode and turned off in release mode.
//
// #See Also  pop_all(), remove_all()
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  // number of elements freed
  uint32_t
AVSorted<_ElementType, _KeyType, _CompareClass>::free_all(
  // keys to match
  const AVArrayBase<_KeyType> & removing,
  // first position to look for elements
  uint32_t start_pos, // = 0u
  // last position to look for elements.  If end_pos is ALength_remainder, end_pos is set
  // to last index position of the array (length - 1)
  uint32_t end_pos // = ALength_remainder
  )
  {
  uint32_t total_freed = 0u;

  if (this->m_count)
    {
    _KeyType ** array_p     = removing.get_array();  // for faster than data member access
    _KeyType ** array_end_p = array_p + removing.get_length();

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      if (free(**array_p, AMatch_first_found, nullptr, start_pos, end_pos))
        {
        end_pos--;
        total_freed++;
        }
      }
    }

  return total_freed;
  }

//---------------------------------------------------------------------------------------
// #Description
//   For every key in the supplied "removing" array, free (remove and delete) the first
//   matching element (if one exists) in the current array between start_pos and end_pos.
//
// #Examples
//   sorted.free_all(to_free_array);
//
// #Notes
//   This method assumes that there are no elements with duplicate keys - free_all_all()
//   checks for duplicate keys.
//   The match test is performed using _CompareClass::equals(elem_i, elem_j).
//   This method performs index range checking when A_BOUNDS_CHECK is defined.  If an
//   index is out of bounds, a AEx<AVSorted<>> exception is thrown.  A_BOUNDS_CHECK is
//   defined by default in debug mode and turned off in release mode.
//
// #See Also  pop_all(), remove_all()
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  // number of elements freed
  uint32_t
AVSorted<_ElementType, _KeyType, _CompareClass>::free_all(
  // array of keys to match
  const _KeyType * keys_p,
  // Number of key objects in keys_p
  uint32_t key_count,
  // first position to look for elements
  uint32_t start_pos, // = 0u
  // last position to look for elements.  If end_pos is ALength_remainder, end_pos is set
  // to last index position of the array (length - 1)
  uint32_t end_pos // = ALength_remainder
  )
  {
  uint32_t total_freed = 0u;

  if (this->m_count)
    {
    const _KeyType * keys_end_p = keys_p + key_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; keys_p < keys_end_p; keys_p++)
      {
      if (free(*keys_p, AMatch_first_found, nullptr, start_pos, end_pos))
        {
        end_pos--;
        total_freed++;
        }
      }
    }

  return total_freed;
  }

//---------------------------------------------------------------------------------------
// For every element in the supplied "removing", all matching elements in the
//             current array between start_pos and end_pos are freed (removed and deleted).
// Returns:    number of elements freed
// Arg         removing - elements to free
// Arg         start_pos - first position to look for elements  (Default 0)
// Arg         end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//             end_pos is set to last index position of the array (length - 1).
//             (Default ALength_remainder)
// Examples:   sorted.free_all_all(to_free_array);
// See:        pop_all(), remove_all()
// Notes:      The match test is performed using _CompareClass::equals(elem_i, elem_j).
//             This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
// Efficiency  If removing uses the same sorting mechanism, the start_pos could be
//             incremented with each call to free_all(key).
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::free_all_all(
  const tAVSorted & removing,
  uint32_t          start_pos, // = 0u
  uint32_t          end_pos    // = ALength_remainder
  )
  {
  uint32_t free_count;
  uint32_t total_freed = 0u;

  if (this->m_count)
    {
    _ElementType ** array_p     = removing.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + removing.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      free_count   = this->free_all(**array_p, start_pos, end_pos);
      end_pos     -= free_count;
      total_freed += free_count;
      }
    }
  return total_freed;
  }

//---------------------------------------------------------------------------------------
//  Removes and returns instance of key between start_pos and end_pos
// Returns:     pointer to element if found, nullptr if not found
// Arg          key - key to match element to pop
// Arg          instance - occurrence of element to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//                - looking for instance AMatch_first_found could return e2_1, e2_2, or
//                  e2_3 depending on which element is happened upon first during the
//                  binary search.
//                - looking for instance AMatch_last will only return e2_3.
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    _ElementType * elem_p = sorted.pop(key, 2);  // pop the second occurrence of element
// See:         free(), remove(), get()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
_ElementType * AVSorted<_ElementType, _KeyType, _CompareClass>::pop(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  )
  {
  uint32_t       find_pos;
  _ElementType * elem_p = nullptr;
  
  if (find(key, instance, &find_pos, start_pos, end_pos))
    {
    elem_p = this->m_array_p[find_pos];

    this->m_count--;           // new length of sorted
    ::memmove(this->m_array_p + find_pos, this->m_array_p + find_pos + 1, (this->m_count - find_pos) * sizeof(_ElementType));
    }
  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }
  return elem_p;
  }

//---------------------------------------------------------------------------------------
//  Removes and accumulates elem_count elements starting at index pos
// Arg          collected_p - pointer to an sorted to append all of the popped elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          pos - starting index position of elements to pop
// Arg          elem_count - number of elements to pop.  If elem_count is ALength_remainder, the
//              number of elements popped = length - pos.  (Default ALength_remainder)
// Examples:    // append 3 elements to popped from sorted starting at index 5
//              sorted.pop_all(&popped, 5, 3);
// See:         free_all(), remove_all(), empty(), get_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::pop_all(
  tAVSorted * collected_p,
  uint32_t    pos,         // = 0
  uint32_t    elem_count   // = ALength_remainder
  )
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {

    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    get_all(collected_p, pos, elem_count);
    this->m_count -= elem_count;
    ::memmove(this->m_array_p + pos, this->m_array_p + pos + elem_count, (this->m_count - pos) * sizeof(_ElementType));
    }
  }

//---------------------------------------------------------------------------------------
//  Removes and accumulates all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Arg          collected_p - pointer to an sorted to append all of the popped elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          key - key to match elements to pop
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // remove all the elements from sorted matching key and append them to popped
//              sorted.pop_all(&popped, key);
// See:         free_all(), remove_all(), get_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::pop_all(
  tAVSorted *      collected_p,
  const _KeyType & key,
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t first_match;
  uint32_t pop_count = 0u;

  if (find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    pop_count = last_match - first_match + 1u;
    pop_all(collected_p, first_match, pop_count);
    }
  return pop_count;
  }
  
//---------------------------------------------------------------------------------------
//  Removes and accumulates all the elements from the current AVSorted
//              between start_pos and end_pos that match elements in sorted using
//              _CompareClass::equals(key, elem_i).
// Arg          collected_p - pointer to an sorted to append all of the popped elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          sorted - elements to free
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // remove all the elements from sorted matching those in to_pop_array and append them to popped
//              sorted.pop_all_all(&popped, to_pop_array);
// See:         free_all(), remove_all(), get_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::pop_all_all(
  tAVSorted *       collected_p,
  const tAVSorted & sorted,
  uint32_t          start_pos, // = 0
  uint32_t          end_pos    // = ALength_remainder
  )
  {
  uint32_t pop_count;
  uint32_t total_count = 0u;

  if (this->m_count)
    {
    _ElementType ** array_p     = sorted.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + sorted.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      pop_count    = pop_all(collected_p, **array_p, start_pos, end_pos);
      end_pos     -= pop_count;
      total_count += pop_count;
      }
    }
  return total_count;
  }

//---------------------------------------------------------------------------------------
//  Removes instance of element matching key between start_pos and end_pos,
//              returning true if found, false if not.
// Returns:     true if instance element removed, false if not
// Arg          key - key to match element to remove
// Arg          instance - occurrence of element to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//                - looking for instance AMatch_first_found could return e2_1, e2_2, or
//                  e2_3 depending on which element is happened upon first during the
//                  binary search.
//                - looking for instance AMatch_last will only return e2_3.
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0u)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.remove(key, 2);  // remove the second occurrence of element matching key
// See:         pop(), free()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::remove(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) 
  {
  uint32_t find_pos;
  bool found = find(key, instance, &find_pos, start_pos, end_pos);

  if (found)
    {
    this->m_count--;           // new length of sorted
    ::memmove(this->m_array_p + find_pos, this->m_array_p + find_pos + 1, (this->m_count - find_pos) * sizeof(_ElementType));
    }

  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return found;
  }

//---------------------------------------------------------------------------------------
//  Removes specific element between start_pos and end_pos, returning true if
//              found, false if not.  Useful in lists that may have multiple elements with
//              the same key.
// Returns:     true if specific element removed, false if not
// Arg          element - specific element to remove
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.
// Arg          start_pos - first position to look for element
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
// Examples:    sorted.remove_elem(elem);  // remove the specific element elem
// See:         remove(), pop(), free()
// Notes:       calls find()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::remove_elem(
  const _ElementType & elem,
  uint32_t *           find_pos_p, // = nullptr
  uint32_t             start_pos,  // = 0u
  uint32_t             end_pos     // = ALength_remainder
  ) 
  {
  uint32_t find_pos;
  bool found = find_elem(elem, &find_pos, start_pos, end_pos);

  if (found)
    {
    this->m_count--;           // new length of sorted
    ::memmove(this->m_array_p + find_pos, this->m_array_p + find_pos + 1, (this->m_count - find_pos) * sizeof(_ElementType));
    }

  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return found;
  }

//---------------------------------------------------------------------------------------
//  Removes all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements removed
// Arg          key - key to match elements to remove
// Arg          start_pos - first position to look for elements (Default 0u)
// Arg          end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.remove_all(key);  // remove all the elements matching key
// See:         pop_all(), free_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::remove_all(
  const _KeyType & key,
  uint32_t         start_pos, // = 0u
  uint32_t         end_pos    // = ALength_remainder
  )
  {
  uint32_t first_match;
  uint32_t remove_count = 0u;

  if (find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    remove_count = last_match - first_match + 1u;
    this->m_count -= remove_count;
    ::memmove(this->m_array_p + first_match, this->m_array_p + first_match + remove_count, (this->m_count - first_match) * sizeof(_ElementType));
    }

  return remove_count;
  }

//---------------------------------------------------------------------------------------
//  Removes first matching element from the current AVSorted between start_pos
//              and end_pos for each element in sorted using _CompareClass::equals(key, elem_i).
// Returns:     number of elements removed
// Arg          sorted - elements to remove
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.remove_all(to_remove_array);
// See:         pop_all(), free_all()
//              get_all() - 'set intersection', append_absent_all() - 'set union'
// Notes:       This method is effectively a 'set subtraction'.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::remove_all(
  const tAVSorted & sorted,
  uint32_t          start_pos, // = 0
  uint32_t          end_pos    // = ALength_remainder
  )
  {
  uint32_t total_removed = 0;

  if (this->m_count && sorted.m_count)
    {
    _ElementType ** array_p     = sorted.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + sorted.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      if (remove(**array_p, AMatch_first_found, nullptr, start_pos, end_pos))
        {
        end_pos--;
        total_removed++;
        }
      }
    }
     
  return total_removed;
  }

//---------------------------------------------------------------------------------------
//  Removes all the elements from the current AVSorted between start_pos and
//              end_pos that match elements in sorted using _CompareClass::equals(key, elem_i).
// Returns:     number of elements removed
// Arg          sorted - elements to remove
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.remove_all_all(to_remove_array);
// See:         pop_all(), free_all()
//              get_all() - 'set intersection', append_absent_all() - 'set union'
// Notes:       This method is effectively a 'set subtraction'.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::remove_all_all(
  const tAVSorted & sorted,
  uint32_t          start_pos, // = 0
  uint32_t          end_pos    // = ALength_remainder
  )
  {
  uint32_t total_removed = 0;

  if (this->m_count && sorted.m_count)
    {
    uint32_t        remove_count;
    _ElementType ** array_p     = sorted.m_array_p;  // for faster than data member access
    _ElementType ** array_end_p = array_p + sorted.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      remove_count   = remove_all(**array_p, start_pos, end_pos);
      end_pos       -= remove_count;
      total_removed += remove_count;
      }
    }
  return total_removed;
  }

//---------------------------------------------------------------------------------------
// Transfers all elements from sorted_p that do not already exist in the current array and
// frees any duplicate/matching elements.
//
// #Examples
//   sorted1.xfer_absent_all_free_dupes(&sorted2);
//
// #See Also
//   get_all() - 'set intersection', remove_all() - 'set subtraction', append_absent_all()
//   Future: xfer_absent_all() - keeps dupes, xfer_absent_all_empty() - empties sorted_p
//   
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::xfer_absent_all_free_dupes(
  // elements to append
  tAVSorted * sorted_p
  )
  {
  uint32_t sorted_length = sorted_p->m_count;

  if (sorted_length)
    {
    uint32_t length = this->m_count;
    uint32_t size   = this->m_size;

    // An ensure_size() before all the appends could needlessly expand the size of the
    // array if the elements are already present.

    uint32_t        pos;
    _ElementType ** array_pp     = this->m_array_p;  // for faster than data member access
    _ElementType ** elems_pp     = sorted_p->m_array_p;
    _ElementType ** elems_end_pp = elems_pp + sorted_length;
    _ElementType ** old_array_pp;

    for (; elems_pp < elems_end_pp; elems_pp++)
      {
      if (!find(**elems_pp, AMatch_first_found, &pos))
        {
        // Resize needed?
        if (size < length + 1u)  // Needs more space
          {
          old_array_pp = this->m_array_p;

          size            = AMemory::request_pointer_count_expand(length + 1u);
          this->m_size    = size;
          array_pp        = tAVArrayBase::alloc_array(this->m_size);
          this->m_array_p = array_pp;
          ::memcpy(array_pp, old_array_pp, pos * sizeof(_ElementType));
          ::memcpy(array_pp + pos + 1u, old_array_pp + pos, (length - pos) * sizeof(_ElementType));
          tAVArrayBase::free_array(old_array_pp);
          }
        else  // enough size in existing array
          {
          ::memmove(array_pp + pos + 1u, array_pp + pos, (length - pos) * sizeof(_ElementType));
          }

        array_pp[pos] = const_cast<_ElementType *>(*elems_pp);  // insert element
        length++;

        this->m_count = length;
        }
      else
        {
        // Element already in array so free it
        delete *elems_pp;
        }
      }

    // Empty originating array
    sorted_p->m_count = 0u;
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Non-Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Counts the number of elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements matched
// Arg          key - key to match elements to count
// Arg          start_pos - first position to look for elements  (Default 0)
// Arg          end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    uint32_t elem_count = sorted.count(key);
// See:         find(), get_instance()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::count(
  const _KeyType & key,
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t first_match;
  uint32_t num_count = 0u;

  if (find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    num_count = last_match - first_match + 1u;
    }

  return num_count;
  }

//---------------------------------------------------------------------------------------
//  Creates a dynamically allocated instance of the same class type as this
//              AVSorted.
// Returns:     a dynamically allocated instance of the same class type as this AVSorted
// Examples:    AVSorted<SomeClass> * array_p = sorted.as_new();
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVSorted<_ElementType, _KeyType, _CompareClass> * AVSorted<_ElementType, _KeyType, _CompareClass>::as_new() const
  {
  tAVSorted * new_array_p = new tAVSorted();

  A_VERIFY_MEMORY(new_array_p != nullptr, tAVSorted);

  return new_array_p;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of key between start_pos and end_pos using
//              _CompareClass::comparison(key, elem_i), returning true if found, false if
//              not.
// Returns:     true if instance element found, false if not
// Arg          key - key to match element to find
// Arg          instance - occurrence of element to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//                - looking for instance AMatch_first_found could return e2_1, e2_2, or
//                  e2_3 depending on which element is happened upon first during the
//                  binary search.
//                - looking for instance AMatch_last will only return e2_3.
//              (Default AMatch_first_found)
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (sorted.find(key, 2))  // if second occurrence of element found ...
// See:         find_reverse(), count(), pop(), get(), insert()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::find(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count)
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    ptrdiff_t      result;
    _ElementType * middle_p;
    _ElementType * first_p = this->m_array_p + start_pos;
    _ElementType * last_p  = this->m_array_p + end_pos;

    // Use functional object or default comparison?  Only one line is different, but it
    // cuts out a comparison

    // Find insert position
    A_LOOP_INFINITE
      {
      middle_p = first_p + ((last_p - first_p) >> 1);
      result   = _CompareClass::comparison(key, *middle_p);

      if (result < 0)
        {
        if (first_p == middle_p)  // Not found
          {
          if (find_pos_p)
            {
            *find_pos_p = uint32_t(middle_p - this->m_array_p);
            }

          return false;
          }
        else  // more to go
          {
          last_p = middle_p - 1;  // Already checked the middle
          }
        }
      else
        {
        if (result > 0)
          {
          if (last_p == middle_p)  // Not found
            {
            if (find_pos_p)
              {
              *find_pos_p = uint32_t(last_p + 1 - this->m_array_p);
              }

            return false;
            }
          else  // more to go
            {
            first_p = middle_p + 1;  // Already checked the middle
            }
          }
        else  // must be equal
          {
          uint32_t pos   = uint32_t(middle_p - this->m_array_p);
          bool     found = (instance != AMatch_first_found) ? find_instance(key, instance, &pos, first_p, last_p) : true;

          if (find_pos_p)
            {
            *find_pos_p = pos;
            }

          return found;
          }
        }
      }
    }

  if (find_pos_p)
    {
    *find_pos_p = 0u;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Finds specific element between start_pos and end_pos, returning true if
//              found, false if not.  Useful in lists that may have multiple elements with
//              the same key.
// Returns:     true if specific element found, false if not
// Arg          elem - specific element to find
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (sorted.find(key, 2))  // if second occurrence of element found ...
// See:         find_reverse(), count(), pop(), get(), insert()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::find_elem(
  const _ElementType & elem,
  uint32_t *           find_pos_p, // = nullptr
  uint32_t             start_pos,  // = 0u
  uint32_t             end_pos     // = ALength_remainder
  ) const
  {
  if (this->m_count == 0u)
    {
    return false;
    }
  
  if (end_pos == ALength_remainder)
    {
    end_pos = this->m_count - 1u;
    }

  uint32_t         pos;
  const _KeyType & key = elem;

  if (find(key, 1u, &pos, start_pos, end_pos))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Found element with the same key, now find element with specific memory address
    const _ElementType * elem_p  = &elem;
    _ElementType *       elems_p = this->m_array_p;

    do
      {
      if (elems_p[pos] == elem_p)
        {
        if (find_pos_p)
          {
          *find_pos_p = pos;
          }

        return true;
        }

      pos++;
      }
    while ((pos <= end_pos) && _CompareClass::equals(key, elems_p[pos]));
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  Simple get - finds first instance of key starting at the zeroth index
//              position using _CompareClass::equals(key, elem_i), returning a pointer
//              to the element if found, nullptr if not.
//              Equivalent to get(key, AMatch_first_found)
// Returns:     a pointer to the element if found, nullptr if not.
// Arg          key - key to match element to find
// Examples:    if (sorted.get(key))  // if occurrence of element found ...
// See:         count(), pop(), find(), get() with more args
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline _ElementType * AVSorted<_ElementType, _KeyType, _CompareClass>::get(
  const _KeyType & key
  ) const
  {
  if (this->m_count)
    {
    ptrdiff_t      result;
    _ElementType * middle_p;
    _ElementType * first_p = this->m_array_p;
    _ElementType * last_p  = this->m_array_p + this->m_count - 1u;

    // Find insert position
    A_LOOP_INFINITE
      {
      middle_p = first_p + ((last_p - first_p) >> 1);
      result   = _CompareClass::comparison(key, *middle_p);

      if (result == 0)
        {
        return middle_p;
        }

      if (result < 0)  // Less than
        {
        if (first_p == middle_p)  // Not found
          {
          return nullptr;
          }

        // more to go
        last_p = middle_p - 1;  // Already checked the middle
        }
      else  // Greater than
        {
        if (last_p == middle_p)  // Not found
          {
          return nullptr;
          }

        // more to go
        first_p = middle_p + 1;  // Already checked the middle
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
//  Finds instance of element between start_pos and end_pos using
//              _CompareClass::equals(key, elem_i), returning a pointer to the element
//              if found, nullptr if not.
// Returns:     a pointer to the element if found, nullptr if not.
// Arg          key - key to match element to find
// Arg          instance - occurrence of element to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
//                - looking for instance 1 will only return e2_1.
//                - looking for instance 2 will only return e2_2.
//                - looking for instance 3 will only return e2_3.
//                - looking for instance 4 or greater will not find a successful match
//                - looking for instance AMatch_first_found could return e2_1, e2_2, or
//                  e2_3 depending on which element is happened upon first during the
//                  binary search.
//                - looking for instance AMatch_last will only return e2_3.
// Arg          find_pos_p - position that element was found, or if not found the insert
//              position (i.e. the sorted location where the element would be located if
//              it were in the sorted element array).  If find_pos_p is nullptr, it is not
//              modified.  (Default nullptr)
// Arg          start_pos - first position to look for element  (Default 0u)
// Arg          end_pos - last position to look for element.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    if (sorted.get(key, 2))  // if second occurrence of element found ...
// See:         count(), pop(), find()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline _ElementType * AVSorted<_ElementType, _KeyType, _CompareClass>::get(
  const _KeyType & key,
  uint32_t         instance,
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t       find_pos;
  _ElementType * elem_p = find(key, instance, &find_pos, start_pos, end_pos) ? &this->m_array_p[find_pos] : nullptr;
  
  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return elem_p;
  }

//---------------------------------------------------------------------------------------
//  Accumulates elem_count elements starting at index pos.
// Arg          collected_p - pointer to an sorted to append all of the elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          pos - starting index position of elements to get
// Arg          elem_count - number of elements to get.  If elem_count is ALength_remainder, the
//              number of elements retrieved = length - pos.  (Default ALength_remainder)
// Examples:    // append 3 elements to collected from sorted starting at index 5
//              sorted.get_all(&collected, 5, 3);
// See:         pop_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
// Efficiency   A merge sort could be faster than an insertion sort.
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVSorted * collected_p,
  uint32_t    pos,         // = 0
  uint32_t    elem_count   // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {
    AVARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    collected_p->ensure_size(collected_p->m_count + elem_count);

    // Check for special case where no merge is necessary
    if (collected_p->m_count == 0u)
      {
      ::memcpy(collected_p->m_array_p + collected_p->m_count, this->m_array_p + pos, elem_count * sizeof(_ElementType));
      collected_p->m_count += elem_count;
      }
    else
      {
      _ElementType * array_p     = this->m_array_p + pos;  // for faster than data member access
      _ElementType * array_end_p = array_p + elem_count;

      for (; array_p < array_end_p; array_p++)
        {
        collected_p->append(*array_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Accumulates all the elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Arg          collected_p - pointer to an sorted to append all of the matched elements to.
//              Any previous elements will remain in collected_p, they are just added to.
// Arg          key - key to match elements to get
// Arg          start_pos - first position to look for elements  (Default 0)
// Arg          end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    // append all the elements from sorted matching key to collected
//              sorted.pop_all(&collected, key);
// See:         pop_all()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVSorted *      collected_p,
  const _KeyType & key,
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t first_match;

  if (find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    get_all(collected_p, first_match, last_match - first_match + 1u);
    }
  }

//---------------------------------------------------------------------------------------
// Accumulates all the elements from the current AVSorted between start_pos and
//             end_pos that match elements in sorted using _CompareClass::equals(key, elem_i).
// Arg         collected_p - pointer to an sorted to append all of the matched elements to.
//             Any previous elements will remain in collected_p, they are just added to.
// Arg         sorted - elements to match
// Arg         start_pos - first position to look for elements (Default 0)
// Arg         end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//             end_pos is set to last index position of the array (length - 1).
//             (Default ALength_remainder)
// Examples:   // append all the elements from sorted matching those in to_get_array to collected
//             sorted.pop_all(&collected, to_get_array);
// See:        pop_all(),
//             append_absent_all() - 'set union', remove_all() - 'set subtraction'
// Notes:      This method is effectively a non-destructive 'set intersection'.
//
//             This method performs index range checking when A_BOUNDS_CHECK is defined.
//             If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVSorted *       collected_p,
  const tAVSorted & sorted,
  uint32_t          start_pos, // = 0u
  uint32_t          end_pos    // = ALength_remainder
  ) const
  {
  if (this->m_count)
    {
    uint32_t       first_match;
    uint32_t       last_match;
    _ElementType * array_p;         // for faster than data member access
    _ElementType * array_end_p;
    _ElementType * sorted_array_p     = sorted.m_array_p;
    _ElementType * sorted_array_end_p = sorted_array_p + sorted.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; sorted_array_p < sorted_array_end_p; sorted_array_p++)
      {
      if (find(*sorted_array_p, 1u, &first_match, start_pos, end_pos))
        {
        last_match = first_match;
        find_instance(*sorted_array_p, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);

        collected_p->ensure_size(collected_p->m_count + last_match - first_match + 1u);

        array_p     = this->m_array_p + first_match;
        array_end_p = this->m_array_p + last_match;

        for (; array_p <= array_end_p; array_p++)
          {
          collected_p->append(*array_p);
          }
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
//  Determines the instance number (occurrence) of the element at the
//              specified index if the element matches more than one element in the sorted
//              using _CompareClass::equals(key, elem_i).
// Returns:     occurrence number of element 
// Arg          index - index position of the element to determine the instance number for
// Examples:    uint32_t instance = sorted.get_instance(8);
// See:         count()
// Notes:       If there is only one match - i.e. itself - then the instance is 1.
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVSorted<_ElementType, _KeyType, _CompareClass>::get_instance(uint32_t index) const
  {
  AVARRAY_BOUNDS_CHECK(index);

  uint32_t       instance        = 1u;
  _ElementType * first_element_p = this->m_array_p;
  _ElementType * elements_p      = first_element_p + index;  // Faster access than using []
  _ElementType * element_p       = elements_p;

  elements_p--;

  while ((elements_p > first_element_p) && (_CompareClass::equals(*element_p, *elements_p)))
    {
    instance++;
    elements_p--;
    }

  return instance;
  }

//---------------------------------------------------------------------------------------
//  Counts the number of elements matching key using
//              _CompareClass::equals(key, elem_i) between start_pos and end_pos.
// Returns:     number of elements matched
// Arg          key - key to match elements to count
// Arg          start_pos - first position to look for elements  (Default 0)
// Arg          end_pos - last position to look for elements.  If end_pos is ALength_remainder,
//              end_pos is set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    uint32_t elem_count = sorted.count(key);
// See:         find(), get_instance()
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVSorted<_ElementType, _KeyType, _CompareClass>::validate_unique() const
  {
  uint32_t count = this->m_count;

  if (count > 1u)
    {
    _ElementType * prev_elem_p = nullptr;
    _ElementType * elems_p     = this->m_array_p;       // for faster than class member access
    uint32_t       idx         = 0u;

    for (; idx < count; idx++)
      {
	  A_VERIFYX(
        (prev_elem_p == nullptr) || (_CompareClass::comparison(*prev_elem_p, elems_p[idx]) < 0),
		a_cstr_format("Elements in sorted array are either out of order or not unique - #%u and #%u are not in proper sequence.", idx - 1u, idx));

      prev_elem_p = &elems_p[idx];
      }
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Finds if instance # of element is between first and last inclusive
//              but excluding pos.
// Returns:     Returns pointer to instance element that is equal to key, or nullptr
//              if not found.
// Arg          key - key to compare
// Arg          instance - occurrence number to look for (must be greater than 0).  If
//              instance is set to AMatch_last, then the last matching element is
//              desired.
// Examples:    elem_p = some_collection.copy(some_elemect);
// Notes:       It uses a quick binary search for look up.
// Modifiers:    protected
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVSorted<_ElementType, _KeyType, _CompareClass>::find_instance(
  const _KeyType & key,
  uint32_t         instance,
  uint32_t *       find_pos_p,
  _ElementType *   first_p,
  _ElementType *   last_p
  ) const
  {
  _ElementType * elements_p;  // Faster access than using []
  _ElementType * fist_found_p = this->m_array_p + *find_pos_p;
  _ElementType * match_p      = fist_found_p;

  if (instance != AMatch_last)
    {
    // Find instance 1
    elements_p = match_p - 1;

    while ((elements_p >= first_p) && (_CompareClass::equals(key, *elements_p)))
      {
      match_p = elements_p;
      elements_p--;
      }

    match_p += (instance - 1u);
    *find_pos_p = uint32_t(match_p - this->m_array_p);

    if (match_p > fist_found_p)  // Beyond areas already tested for match
      {
      return _CompareClass::equals(key, *match_p);
      }

    return true;
    }

  // Find last instance
  elements_p = match_p + 1;

  while ((elements_p <= last_p) && (_CompareClass::equals(key, *elements_p)))
    {
    match_p = elements_p;
    elements_p++;
    }

  *find_pos_p = uint32_t(match_p - this->m_array_p);

  return true;
  }

//---------------------------------------------------------------------------------------
//  Sorts the elements in the AVSorted from start_pos to end_pos.
// Arg          start_pos - first position to start sorting  (Default 0)
// Arg          end_pos - last position to sort.  If end_pos is ALength_remainder, end_pos is
//              set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.sort();
// Notes:       calls sort_compare() which calls _CompareClass::comparison()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void AVSorted<_ElementType, _KeyType, _CompareClass>::sort(
  uint32_t start_pos, // = 0u
  uint32_t end_pos    // = ALength_remainder
  )
  {
  // $Revisit - CReis Efficiency: Could probably get more speed by writing a custom version of qsort.
  if (this->m_count > 1u)
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1;
      }

    AVARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);
  
    ::qsort(this->m_array_p + start_pos, end_pos - start_pos + 1, sizeof(_ElementType), sort_compare);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Used by qsort() in the sort() method
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
int AVSorted<_ElementType, _KeyType, _CompareClass>::sort_compare(
  const void * lhs_p,
  const void * rhs_p
  )
  {
  return int(_CompareClass::comparison(*((_ElementType *)lhs_p), *((_ElementType *)rhs_p)));
  }


#endif  // _AVSORTED_HPP


