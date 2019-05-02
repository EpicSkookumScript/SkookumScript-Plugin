// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AVCompactSorted class template
//=======================================================================================


#ifndef _AVCOMPACTSORTED_HPP
#define _AVCOMPACTSORTED_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AVCompactArrayBase.hpp>
#include <AgogCore/ACompareBase.hpp>
#include <stdarg.h>          // Uses: va_array, va_start(), va_arg(), va_end()
#include <stdlib.h>          // Uses: qsort()


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Description
//   The AVCompactSorted class template provides a dynamic length, persistent index (i.e.
//   once an element is appended, it may be accessed via an integer index), insertable
//   (i.e. an element may be inserted at a specific index position) collection of
//   _ElementType objects.
//
//   Any modifications to this template should be compile-tested by adding an explicit
//   instantiation declaration such as:
//     template class AVCompactSorted<AString>;
//   
// #See Also
//   ~AVCompactArray<>           - Ordered array of pointers to elements with retrieval by key type
//     ~AVCompactArrayLogical<>  - Same as AVCompactArray<>, but uses the comparison operators < and == to sort elements
//   AVCompactSorted<>           - AVCompactSorted array of pointers to elements with retrieval and sorting by key type
//     AVCompactSortedLogical<>  - Same as AVCompactSorted<>, but uses the comparison operators < and == to sort elements
// #Author   Conan Reis
template<
  // the class/type of element objects in the sorted array.
  class _ElementType,
  // The class/type to use for searching and sorting.  It is often a data member of the
  // _ElementType.  To get access to the key, _ElementType must have a conversion operator
  // to the _KeyType (or the _KeyType must have a non-explicit constructor/converter for
  // _ElementType).  For example, it is common for classes to have a symbol name
  // identifying them and it is this name by which they are sorted.  So these element
  // classes could use ASymbol as the _KeyType and have 'operator const ASymbol & () const'
  // as a conversion operator.  The _KeyType may be required to possess various methods or
  // even data members due to the _CompareClass that is used.  For example, the default
  // _CompareClass is ACompareAddress<> which uses the memory address of the _KeyType and
  // so relies on no particular methods.  The next most common _CompareClass is
  // ACompareLogical<> which relies on the equal to (=) and the less than (<) operators of
  // the _KeyType.  (Default  _ElementType)  So the keys are the elements themselves.
  class _KeyType = _ElementType,
  // a class template that provides the static sorting functions equals() and comparison().
  // These functions should be inlined so that there is no function call overhead.  See
  // ACompareAddress and ACompareLogical in AgogCore/ACompareBase.hpp for examples.  See
  // AgogCore/AString.hpp for the class definition of ACompareStrInsensitive.
  // (Default ACompareAddress<_KeyType>)
  class _CompareClass = ACompareAddress<_KeyType>
  >
class AVCompactSorted : public AVCompactArrayBase<_ElementType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AVCompactSorted<_ElementType, _KeyType, _CompareClass> tAVCompactSorted;
    typedef AVCompactArrayBase<_ElementType>                       tAVCompactArrayBase;


  // Unhide Inherited Methods

    // Methods in this class with the same name as methods in AVCompactArrayBase<> are 'hidden'
    // (even if they do not have the same interface), this makes them visible again.
    // Ensure that any new methods added to this class that also have the same name
    // as methods in AVCompactArrayBase<> are included in this list to preserve the 'is-type-of'
    // relationship.  These using directives must precede the new methods in this class.
    using AVCompactArrayBase<_ElementType>::remove;
    //using AVCompactArrayBase<_ElementType>::remove_all;

  // Common methods

    AVCompactSorted();
    AVCompactSorted(const AVCompactSorted & sorted);
    AVCompactSorted(AVCompactSorted * sorted_p);
    ~AVCompactSorted();
    AVCompactSorted & operator=(const AVCompactSorted & sorted);

  // Converter Methods

    //explicit AVCompactSorted(const _ElementType ** elems_p, uint32_t elem_count, bool pre_sorted = false);
    //explicit AVCompactSorted(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted);
    //explicit AVCompactSorted(int elem_count, ...);
    
  // Modifying Behaviour methods

    bool           append(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    bool           append_absent(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    bool           append_replace(const _ElementType & elem, uint32_t * insert_pos_p = nullptr);
    bool           remove(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void           sort(uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);

    //void           append_absent_all(const AVCompactSorted & sorted);
    //void           append_all(const AVCompactArrayBase<_ElementType> & array, bool pre_sorted = false);
    //void           append_all(const AVCompactSorted & sorted);
    //void           append_all(const _ElementType ** elems_p, uint32_t elem_count, bool pre_sorted = false);
    //void           append_all(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted = false);
    //uint32_t       remove_all(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    //uint32_t       remove_all(const AVCompactSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    //uint32_t       remove_all_all(const AVCompactSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);

  // Non-modifying Methods

    uint32_t       count(const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    bool           find(const _KeyType & key, uint32_t instance = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    _ElementType * get(const _KeyType & key) const;
    _ElementType * get(const _KeyType & key, uint32_t instance, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    uint32_t       get_instance(uint32_t index) const;
    void           validate_unique() const;

    //void           get_all(AVCompactSorted * collected_p, uint32_t pos = 0u, uint32_t elem_count = ALength_remainder) const;
    //void           get_all(AVCompactSorted * collected_p, const _KeyType & key, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;
    //void           get_all(AVCompactSorted * collected_p, const AVCompactSorted & sorted, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder) const;

  protected:
  // Internal Methods

    bool find_instance(const _KeyType & key, uint32_t instance, uint32_t * find_pos_p, _ElementType * first_p, _ElementType * last_p) const;

  // Internal Class Methods

    static int sort_compare(const void * lhs_p, const void * rhs_p);


  // Data members

    // Inherited from AVCompactArrayBase<>
    //uint32_t        m_count;   // Amount of array used
    //_ElementType ** m_array_p;  // Array of pointers to elements

  };  // AVCompactSorted


//---------------------------------------------------------------------------------------
// AVCompactSortedLogical is a shorthand for AVCompactSorted<_ElementType, _KeyType, ACompareLogical<_KeyType> >
// Note: Although AVCompactSortedLogical is derived from AVCompactSorted, there is no loss in efficiency.
template<
  class _ElementType,
  class _KeyType      = _ElementType
  >
class AVCompactSortedLogical : public AVCompactSorted<_ElementType, _KeyType, ACompareLogical<_KeyType> >
  {
  public:
    // Local shorthand
    typedef AVCompactArrayBase<_ElementType>                                     tAVCompactArrayBase;
    typedef AVCompactSorted<_ElementType, _KeyType, ACompareLogical<_KeyType> >  tAVCompactSorted;
    typedef AVCompactSortedLogical<_ElementType, _KeyType>                       tAVCompactSortedLogical;
    
  // All the constructors are hidden (stupid!), so make appropriate links
    AVCompactSortedLogical() : tAVCompactSorted() {}
    AVCompactSortedLogical(const AVCompactSorted<_ElementType, _KeyType, ACompareLogical<_KeyType> > & sorted) : tAVCompactSorted(sorted) {}
    AVCompactSortedLogical(AVCompactSortedLogical<_ElementType, _KeyType> & sorted_p) : tAVCompactSorted(sorted_p) {}
    explicit AVCompactSortedLogical(const _ElementType ** elems_p, uint32_t elem_count, bool pre_sorted = false);
    explicit AVCompactSortedLogical(const _ElementType * elems_p, uint32_t elem_count, bool pre_sorted);
    //explicit AVCompactSortedLogical(int elem_count, ...);
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
// Examples:    AVCompactSorted<SomeClass> sorted;
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted() :
  tAVCompactArrayBase(0u, nullptr)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor - since elements are values they are copied too with
//              calls to their copy constructors.
// Returns:     itself
// Arg          sorted - sorted to copy
// Examples:    AVCompactSorted<SomeClass> sorted1;
//              AVCompactSorted<SomeClass> sorted2(sorted1);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted(
  const tAVCompactSorted & sorted
  ) :
  tAVCompactArrayBase(sorted.m_count, tAVCompactArrayBase::alloc_array(sorted.m_count))
  {
  if (sorted.m_count)
    {
    _ElementType * array_p     = this->m_array_p;
    _ElementType * elems_p     = sorted.m_array_p;
    _ElementType * elems_end_p = elems_p + sorted.m_count;

    while (elems_p < elems_end_p)
      {
      new (array_p) _ElementType(*elems_p);
      array_p++;
      elems_p++;
      }
    }
  }

  //---------------------------------------------------------------------------------------
//  Transfer constructor - takes data from sorted_p and empties it.
// Returns:     itself
// Arg          sorted_p - to take data from
// Examples:    AVCompactSorted<SomeClass> sorted1;
//              AVCompactSorted<SomeClass> sorted2(&sorted1);
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted(
  tAVCompactSorted * sorted_p
  ) :
  tAVCompactArrayBase(sorted_p->m_count, sorted_p->m_array_p)
  {
  sorted_p->m_count = 0u;
  sorted_p->m_array_p = nullptr;
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline AVCompactSorted<_ElementType, _KeyType, _CompareClass>::~AVCompactSorted()
  {
  tAVCompactArrayBase::dtor_elems(this->m_array_p, this->m_count);
  tAVCompactArrayBase::free_array(this->m_array_p);
  }

//---------------------------------------------------------------------------------------
//  Assignment operator.  ***Watch out*** - does a deep copy of the elements.
// Returns:     reference to itself to allow for stringization
//              sorted1 = sorted2 = sorted3;
// Examples:    sorted1 = sorted2;
// See:         append_all()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  AVCompactSorted<_ElementType, _KeyType, _CompareClass> &
  AVCompactSorted<_ElementType, _KeyType, _CompareClass>::operator=(
  const tAVCompactSorted & sorted
  )
  {
  uint32_t new_count = sorted.m_count;

  if (this->m_count != new_count)
    {
    // Size not the same.  Empty and initialize a copy.
    tAVCompactArrayBase::dtor_elems(this->m_array_p, this->m_count);
    tAVCompactArrayBase::free_array(this->m_array_p);

    if (new_count)
      {
      // Initialize elements with copy constructors
      _ElementType * array_p     = tAVCompactArrayBase::alloc_array(new_count);
      _ElementType * elems_p     = sorted.m_array_p;
      _ElementType * elems_end_p = elems_p + new_count;

      this->m_array_p = array_p;
      this->m_count = new_count;

      while (elems_p < elems_end_p)
        {
        new (array_p) _ElementType(*elems_p);
        array_p++;
        elems_p++;
        }
      }
    else
      {
      this->m_array_p = nullptr;
      this->m_count = 0u;
      }
    }
  else
    {
    // Size is the same.
    if (new_count)
      {
      // Reassign elements using assignment operator
      _ElementType * array_p     = this->m_array_p;
      _ElementType * elems_p     = sorted.m_array_p;
      _ElementType * elems_end_p = elems_p + new_count;

      while (elems_p < elems_end_p)
        {
        *array_p = *elems_p;
        array_p++;
        elems_p++;
        }
      }
    }

  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Converter Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* $Revisit - CReis Still needs refactoring

//---------------------------------------------------------------------------------------
//  Variable argument constructor.
// Returns:     itself
// Arg          elem_count - number of pointers to elements in the variable length
//              argument list.
//              Note, this argument is signed to help differentiate it from the array
//              constructor.
// Arg          ... - pointers to elements of type _ElementType
// Examples:    AVCompactSorted<SomeClass> (3, &elem1, elem2_p, get_elem_p());
// See:         AVCompactSorted<_ElementType>(elems_p, elem_count, buffer_size)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted(
  int elem_count,
  ...
  ) :
  tAVCompactArrayBase(0u, uint32_t(elem_count), nullptr)
  {
  if (elem_count)
    {
    va_list        arg_array;
    _ElementType * elem_p;
    uint32_t       find_pos;
    uint32_t       pos = 0u;

	this->m_array_p = tAVCompactArrayBase::alloc_array(uint32_t(elem_count));
    va_start(arg_array, elem_count);     // Initialize variable arguments

    while(pos < uint32_t(elem_count))
      {
      elem_p = va_arg(arg_array, _ElementType *);
      find(*elem_p, AMatch_first_found, &find_pos);
      ::memmove(this->m_array_p + find_pos + 1u, this->m_array_p + find_pos, (this->m_count - find_pos) * sizeof(_ElementType));
      this->m_array_p[find_pos] = elem_p;  // insert element
      this->m_count++;
      pos++;
      }

    va_end(arg_array);  // Reset variable arguments
    }
  }

//---------------------------------------------------------------------------------------
//  Constructor (for pre-existing buffers or for pre-allocated sizes)
// Returns:     itself
// Arg          elems_p - array of pointers to elements of type _ElementType.  It can be
//              nullptr if elems_p is non-zero - see the description of the elems_p arg.
// Arg          elem_count - number of pointers to elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// Arg          buffer_size - If this argument is non-zero and elems_p is not nullptr, it
//              indicates the size of the elems_p buffer and that ownership of the
//              elems_p buffer is taken over by this sorted array and be eventually 
//              deleted when the AVCompactSorted instance is destroyed.  If this argument is
//              non-zero and elems_p is nullptr, it indicates the initial buffer size that
//              this sorted array should allocate.  (Default 0u)
// Arg          pre_sorted - specifies whether the elements in elems_p are already
//              appropriately sorted.  If false, the elements are sorted.  If true, the
//              element order is left unchanged and the extra overhead of a sort is not
//              taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//              not function properly.
// See:         AVCompactSorted<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  bool                  pre_sorted   // = false
  ) :
  tAVCompactArrayBase(elem_count, elem_count, nullptr)
  {
  this->m_array_p = tAVCompactArrayBase::alloc_array(elem_count);
  ::memcpy(this->m_array_p, elems_p, elem_count * sizeof(_ElementType));

  if (!pre_sorted && elem_count)
    {
    sort();
    }
  }

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
//              taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//              not function properly.
// See:         AVCompactSorted<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
AVCompactSorted<_ElementType, _KeyType, _CompareClass>::AVCompactSorted(
  const _ElementType * elems_p,
  uint32_t             elem_count,
  bool                 pre_sorted // = false
  ) :
  tAVCompactArrayBase(0u, elem_count, nullptr)
  {
  // $Revisit - CReis Currently ignores 'pre_sorted'
  if (elem_count)
    {
    uint32_t             find_pos;
    const _ElementType * elem_p      = elems_p;
    const _ElementType * elems_end_p = elems_p + elem_count;
    _ElementType **      array_p     = tAVCompactArrayBase::alloc_array(elem_count);  // For quick access

    this->m_array_p = array_p;

    for(; elem_p < elems_end_p; this->m_count++, elem_p++)
      {
      find(*elem_p, AMatch_first_found, &find_pos);
      ::memmove(array_p + find_pos + 1u, array_p + find_pos, (this->m_count - find_pos) * sizeof(_ElementType));
      array_p[find_pos] = const_cast<_ElementType *>(elem_p);  // insert element
      }
    }
  }

*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Appends element to the current AVCompactSorted array.  The appended element is
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
bool AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append(
  const _ElementType & elem,
  uint32_t *           insert_pos_p // = nullptr
  )
  {
  uint32_t pos;
  bool found = this->find(elem, AMatch_first_found, &pos);

  this->insert(elem, pos);

  if (insert_pos_p)
    {
    *insert_pos_p = pos;
    }

  return !found;
  }

//---------------------------------------------------------------------------------------
//  Appends an element to the AVCompactSorted array if it is not already present.
// Returns:     true if element appended, false if not
// Arg          elem - element to append
// Arg          insert_pos_p - address to store the index position that elem was inserted
//              or found.  If insert_pos_p is nullptr, it is not modified.  (Default nullptr)
// Examples:    sorted.append_absent(elem);
// See:         append(), append_replace()
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_absent(
  const _ElementType & elem,
  uint32_t *           insert_pos_p // = nullptr
  )
  {
  uint32_t pos;
  bool found = this->find(elem, AMatch_first_found, &pos);

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
// #Description
//   Appends a copy of an object to the sorted array if it is not already present or
//   replaces first found matching element.
//
// #Examples
//   sorted.append_absent(elem);
//
// #See Also  append_absent(), append()
// #Author(s) Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
  // true if element appended, false if not
  bool
AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_replace(
  // element to append copy of
  const _ElementType & elem,
  // address to store the index position that elem was inserted or found.  If it is nullptr,
  // it is not modified.
  uint32_t * insert_pos_p // = nullptr
  )
  {
  uint32_t pos;
  bool found = this->find(elem, AMatch_first_found, &pos);

  if (found)
    {
    this->m_array_p[pos] = elem;
    }
  else
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
//  Removes instance of element matching key between start_pos and end_pos,
//              returning true if found, false if not.
// Returns:     true if instance element removed, false if not
// Arg          key - key to match element to remove
// Arg          instance - occurrence of element to find.  If instance is set to AMatch_last,
//              then the last matching element is desired.  If instance is set to
//              AMatch_first_found, the first element to match is desired - this may not
//              be the ordinally first matching element, but it is more efficient than
//              indicating a specific instance to find.
//              For example, searching the AVCompactSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
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
bool AVCompactSorted<_ElementType, _KeyType, _CompareClass>::remove(
  const _KeyType & key,
  uint32_t         instance,   // = AMatch_first_found
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) 
  {
  uint32_t find_pos;
  bool found = this->find(key, instance, &find_pos, start_pos, end_pos);

  if (found)
    {
    this->remove(find_pos);
    }

  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return found;
  }


/* $Revisit - CReis Still needs refactoring

//---------------------------------------------------------------------------------------
//  Appends all elements from sorted that do not already exist in the current
//              sorted.
// Arg          sorted - sorted of elements to append
// Examples:    sorted.append_absent_all(sorted);
// See:         get_all() - 'set intersection', remove_all() - 'set subtraction'
// Notes:       This is essentially a 'set union'.
//              [Efficiency] Instead of just sort() - sort only the new elements then do
//              merge sort of previous elements and new elements.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_absent_all(const tAVCompactSorted & sorted)
  {
  uint32_t sorted_length = sorted.m_count;

  if (sorted_length)
    {
    uint32_t length = this->m_count;
    uint32_t size   = this->m_size;

    // An ensure_size() before all the appends could needlessly expand the size of the
    // array if the elements are already present.

    uint32_t        pos;
    _ElementType ** array_pp     = this->m_array_p;  // for faster than data member access
    _ElementType ** elems_pp     = sorted.m_array_p;
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
          array_pp        = tAVCompactArrayBase::alloc_array(this->m_size);
          this->m_array_p = array_pp;
          ::memcpy(array_pp, old_array_pp, pos * sizeof(_ElementType));
          ::memcpy(array_pp + pos + 1u, old_array_pp + pos, (length - pos) * sizeof(_ElementType));
          tAVCompactArrayBase::free_array(old_array_pp);
          }
        else  // enough size in existing array
          {
          ::memmove(array_pp + pos + 1u, array_pp + pos, (length - pos) * sizeof(_ElementType));
          }

        array_pp[pos] = const_cast<_ElementType *>(*elems_pp);  // insert element
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
//             taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//             not function properly.
// Examples:   sorted.append_all(objs);
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_all(
  const AVCompactArrayBase<_ElementType> & array,
  bool                                     pre_sorted // = false
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
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_all(const tAVCompactSorted & sorted)
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
// Appends elem_count elements from elems_p to the current sorted.
// Arg         elems_p - array of pointers to elements of type _ElementType
// Arg         elem_count - number of pointers to elements in elems_p
//             Note, this argument is unsigned to help differentiate it from the
//             variable length element argument constructor.
// Arg         pre_sorted - specifies whether the elements in elems_p are already
//             appropriately sorted.  If false, the elements are sorted.  If true, the
//             element order is left unchanged and the extra overhead of a sort is not
//             taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//             not function properly.
// Examples:   sorted.append_all(objs.get_array(), objs.get_length());
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_all(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  bool                  pre_sorted // = false
  )
  {
  this->ensure_size(this->m_count + elem_count);
  ::memcpy(this->m_array_p + this->m_count, elems_p, elem_count * sizeof(_ElementType));
  this->m_count += elem_count;

  // Check for special case where no merge is necessary
  if ((!pre_sorted) || (this->m_count != elem_count))
    {
    // $Revisit - CReis [Efficiency] Instead of just sort() - sort only the new elements if
    // presorted, then do merge sort of previous elements and new elements.
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
//              taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//              not function properly.
// Examples:    sorted.append_all(objs.get_array(), objs.get_length());
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::append_all(
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
      array_p[find_pos] = const_cast<_ElementType *>(elem_p);  // insert element
      this->m_count++;
      }
    }
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVCompactSorted<_ElementType, _KeyType, _CompareClass>::remove_all(
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
//  Removes first matching element from the current AVCompactSorted between start_pos
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVCompactSorted<_ElementType, _KeyType, _CompareClass>::remove_all(
  const tAVCompactSorted & sorted,
  uint32_t                 start_pos, // = 0
  uint32_t                 end_pos    // = ALength_remainder
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

    AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

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
//  Removes all the elements from the current AVCompactSorted between start_pos and
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVCompactSorted<_ElementType, _KeyType, _CompareClass>::remove_all_all(
  const tAVCompactSorted & sorted,
  uint32_t                 start_pos, // = 0
  uint32_t                 end_pos    // = ALength_remainder
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

    AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; array_p < array_end_p; array_p++)
      {
      remove_count   = remove_all(**array_p, start_pos, end_pos);
      end_pos       -= remove_count;
      total_removed += remove_count;
      }
    }
  return total_removed;
  }

*/


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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVCompactSorted<_ElementType, _KeyType, _CompareClass>::count(
  const _KeyType & key,
  uint32_t         start_pos,  // = 0
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t first_match;
  uint32_t num_count = 0u;

  if (this->find(key, 1u, &first_match, start_pos, end_pos))
    {
    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    uint32_t last_match = first_match;

    this->find_instance(key, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);
    num_count = last_match - first_match + 1u;
    }

  return num_count;
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
//              For example, searching the AVCompactSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
bool AVCompactSorted<_ElementType, _KeyType, _CompareClass>::find(
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

    AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

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
          bool found = (instance != AMatch_first_found) ? find_instance(key, instance, &pos, first_p, last_p) : true;

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
//  Simple get - finds first instance of key starting at the zeroth index
//              position using _CompareClass::equals(key, elem_i), returning a pointer
//              to the element if found, nullptr if not.
//              Equivalent to get(key, AMatch_first_found)
// Returns:     a pointer to the element if found, nullptr if not.
// Arg          key - key to match element to find
// Examples:    if (sorted.get(key))  // if occurrence of element found ...
// See:         count(), pop(), find(), get() with more args
// Notes:       This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline _ElementType * AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get(
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
//              For example, searching the AVCompactSorted (e1, e2_1, e2_2, e2_3, e3, ...) for e2
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline _ElementType * AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get(
  const _KeyType & key,
  uint32_t         instance,
  uint32_t *       find_pos_p, // = nullptr
  uint32_t         start_pos,  // = 0u
  uint32_t         end_pos     // = ALength_remainder
  ) const
  {
  uint32_t       find_pos;
  _ElementType * elem_p = find(key, instance, &find_pos, start_pos, end_pos) ? this->m_array_p + find_pos : nullptr;
  
  if (find_pos_p)
    {
    *find_pos_p = find_pos;
    }

  return elem_p;
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
uint32_t AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get_instance(uint32_t index) const
  {
  AVCOMPACTARRAY_BOUNDS_CHECK(index);

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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::validate_unique() const
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


/* $Revisit - CReis Needs refactoring

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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
// Efficiency   A merge sort could be faster than an insertion sort.
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVCompactSorted * collected_p,
  uint32_t           pos,         // = 0
  uint32_t           elem_count   // = ALength_remainder
  ) const
  {
  if (elem_count == ALength_remainder)
    {
    elem_count = this->m_count - pos;
    }

  if (elem_count)
    {
    AVCOMPACTARRAY_BOUNDS_CHECK_SPAN(pos, elem_count);

    collected_p->ensure_size(collected_p->m_count + elem_count);

    // Check for special case where no merge is necessary
    if (collected_p->m_count == 0u)
      {
      ::memcpy(collected_p->m_array_p + collected_p->m_count, this->m_array_p + pos, elem_count * sizeof(_ElementType));
      collected_p->m_count += elem_count;
      }
    else
      {
      _ElementType ** array_p     = this->m_array_p + pos;  // for faster than data member access
      _ElementType ** array_end_p = array_p + elem_count;

      for (; array_p < array_end_p; array_p++)
        {
        collected_p->append(**array_p);
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
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVCompactSorted * collected_p,
  const _KeyType &   key,
  uint32_t           start_pos,  // = 0
  uint32_t           end_pos     // = ALength_remainder
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
// Accumulates all the elements from the current AVCompactSorted between start_pos and
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
//             If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//             A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//             release mode.
// Author(s):   Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::get_all(
  tAVCompactSorted *       collected_p,
  const tAVCompactSorted & sorted,
  uint32_t                 start_pos, // = 0u
  uint32_t                 end_pos    // = ALength_remainder
  ) const
  {
  if (this->m_count)
    {
    uint32_t        first_match;
    uint32_t        last_match;
    _ElementType ** array_p;         // for faster than data member access
    _ElementType ** array_end_p;
    _ElementType ** sorted_array_p     = sorted.m_array_p;
    _ElementType ** sorted_array_end_p = sorted_array_p + sorted.m_count;

    if (end_pos == ALength_remainder)
      {
      end_pos = this->m_count - 1u;
      }

    AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);

    for (; sorted_array_p < sorted_array_end_p; sorted_array_p++)
      {
      if (find(**sorted_array_p, 1u, &first_match, start_pos, end_pos))
        {
        last_match = first_match;
        find_instance(**sorted_array_p, AMatch_last, &last_match, this->m_array_p + first_match, this->m_array_p + end_pos);

        collected_p->ensure_size(collected_p->m_count + last_match - first_match + 1u);

        array_p     = this->m_array_p + first_match;
        array_end_p = this->m_array_p + last_match;

        for (; array_p <= array_end_p; array_p++)
          {
          collected_p->append(**array_p);
          }
        }
      }
    }
  }

*/

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
bool AVCompactSorted<_ElementType, _KeyType, _CompareClass>::find_instance(
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
//  Sorts the elements in the AVCompactSorted from start_pos to end_pos.
// Arg          start_pos - first position to start sorting  (Default 0)
// Arg          end_pos - last position to sort.  If end_pos is ALength_remainder, end_pos is
//              set to last index position of the array (length - 1).
//              (Default ALength_remainder)
// Examples:    sorted.sort();
// Notes:       calls sort_compare() which calls _CompareClass::comparison()
//              This method performs index range checking when A_BOUNDS_CHECK is defined.
//              If an index is out of bounds, a AEx<AVCompactSorted<>> exception is thrown.
//              A_BOUNDS_CHECK is defined by default in debug mode and turned off in
//              release mode.
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
inline void AVCompactSorted<_ElementType, _KeyType, _CompareClass>::sort(
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

    AVCOMPACTARRAY_BOUNDS_CHECK_RANGE(start_pos, end_pos);
  
    qsort(this->m_array_p + start_pos, end_pos - start_pos + 1, sizeof(_ElementType), sort_compare);
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Used by qsort() in the sort() method
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType, class _CompareClass>
int AVCompactSorted<_ElementType, _KeyType, _CompareClass>::sort_compare(
  const void * lhs_p,
  const void * rhs_p
  )
  {
  return int(_CompareClass::comparison(*((_ElementType *)lhs_p), *((_ElementType *)rhs_p)));
  }


//#######################################################################################
// AVCompactSortedLogical
//#######################################################################################

//---------------------------------------------------------------------------------------
//  Constructor (for pre-existing buffers or for pre-allocated sizes)
// Returns:     itself
// Arg          elems_p - array of pointers to elements of type _ElementType.  It can be
//              nullptr if elems_p is non-zero - see the description of the elems_p arg.
// Arg          elem_count - number of pointers to elements in elems_p
//              Note, this argument is unsigned to help differentiate it from the
//              variable length element argument constructor.
// Arg          pre_sorted - specifies whether the elements in elems_p are already
//              appropriately sorted.  If false, the elements are sorted.  If true, the
//              element order is left unchanged and the extra overhead of a sort is not
//              taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//              not function properly.
// See:         AVCompactSortedLogical<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
AVCompactSortedLogical<_ElementType, _KeyType>::AVCompactSortedLogical(
  const _ElementType ** elems_p,
  uint32_t              elem_count,
  bool                  pre_sorted   // = false
  ) :
  tAVCompactSorted(elems_p, elem_count, pre_sorted)
  {
  }

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
//              taken - ensure that they are sorted appropriately, or this AVCompactSorted will
//              not function properly.
// See:         AVCompactSortedLogical<_ElementType>(elem_count, ...)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
AVCompactSortedLogical<_ElementType, _KeyType>::AVCompactSortedLogical(
  const _ElementType * elems_p,
  uint32_t             elem_count,
  bool                 pre_sorted // = false
  ) :
  tAVCompactSorted(elems_p, elem_count, pre_sorted)
  {
  }


/*
//---------------------------------------------------------------------------------------
//  Variable argument constructor.
// Returns:     itself
// Arg          elem_count - number of pointers to elements in the variable length
//              argument list.
//              Note, this argument is signed to help differentiate it from the array
//              constructor.
// Arg          ... - pointers to elements of type _ElementType
// Examples:    AVCompactSortedLogical<SomeClass> (3, &elem1, elem2_p, get_elem_p());
// See:         AVCompactSortedLogical<_ElementType>(elems_p, elem_count, buffer_size)
// Author(s):    Conan Reis
template<class _ElementType, class _KeyType>
AVCompactSortedLogical<_ElementType, _KeyType>::AVCompactSortedLogical(
  int elem_count,
  ...
  )
  {
  va_list arg_array;
  
  this->m_count   = 0u;
  this->m_size    = elem_count;
  this->m_array_p = nullptr;

  if (elem_count)
    {
    this->m_array_p = tAVCompactArrayBase::alloc_array(elem_count);

    va_start(arg_array, elem_count);     // Initialize variable arguments

    while(this->m_count != static_cast<uint32_t>(elem_count))
      {
      append(*va_arg(arg_array, _ElementType *));
      }

    va_end(arg_array);  // Reset variable arguments
    }
  }
*/


#endif  // _AVCOMPACTSORTED_HPP


