// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AList class template
// Notes:          See the AList class comments for more information.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Macros
//=======================================================================================

#ifdef A_EXTRA_CHECK

// If defined, AList objects will do moderate validation checks to insure data integrity.
// $Revisit - This could be implemented as a flag on a template parameter or a specialized
// derived class so that the checks can be localized to particular list(s), but this would
// end up generating more code.
//#define ALIST_EXTRA_CHECK

// If defined, AList objects will do heavy validation checks to insure data integrity.
//#define ALIST_STRICT_CHECK

#endif


// If heavy checking is turned on, auto enable moderate checking too.
#if defined(ALIST_STRICT_CHECK) && !defined(ALIST_EXTRA_CHECK)
  #define ALIST_EXTRA_CHECK
#endif


//=======================================================================================
// Global Structures
//=======================================================================================

// The AList template class is defined below after AListNode
template<class _ElementType, class _NodeIdType> class AList;


//---------------------------------------------------------------------------------------
// Notes    Doubly linked intrinsic node class used as elements for AList<> described below.
template<class _ElementType, class _NodeIdType = _ElementType>
class AListNode
  {
  friend class AList<_ElementType, _NodeIdType>;

  public:
  // Methods

    AListNode()                                             { m_next_p = m_prev_p = this; }
    ~AListNode()                                            { if (m_next_p != this) { remove(); } }
    AListNode(const AListNode & node) = delete;
    AListNode & operator=(const AListNode & node) = delete;

    _ElementType * get_next() const;
    _ElementType * get_prev() const;

    bool is_in_list() const                                 { return (m_next_p != this); }
    bool is_valid_node() const;
    void link_next(_ElementType * elem_p);
    void link_prev(_ElementType * elem_p);
    void remove();

  protected:

  // Data Members

    // $Note - CReis It is debatable, but these members *could* be 'mutable'.
    AListNode * m_next_p;
    AListNode * m_prev_p;

  };  // AListNode


//---------------------------------------------------------------------------------------
// List class implemented as a doubly linked circular list with a sentinel node and the
// nodes are intrusive to reduce the number memory allocations.  The list does not store
// its element count so nodes can be manipulated without referring to the list that
// contains them.
//
// This class is intentionally not derived from APArrayBase since its implementation
// differs significantly.  It does not use virtual methods and no virtual methods should
// be added in the future.
//
// Any modifications to this template should be compile-tested by adding an explicit
// instantiation declaration such as:
//   ```
//   class StringNode: public AListNode<StringNode>, public AString  {};
//   template class AList<AStringNode>;
//   ```
//
// Arg      _ElementType - the class/type of elements that can be added to the list.
//          It must be derived from the AListNode<> class template.
// Arg      _NodeIdType - if the same _ElementType needs to simultaneously exist in more
//          than one AList, this differentiates each list and node structure to have the
//          _ElementType derive from.  The type supplied to _NodeIdType does not need to
//          actually exist or be related to _ElementType - it just needs to be declared.
//
// See Also AListFree<>               - Same as AList<>, but calls free_all() on its destruction
//          APArray<>                 - Ordered array of pointers to elements with retrieval by key type
//            APArrayFree<>           - Same as APArray<>, but calls free_all() on its destruction
//            APArrayLogical<>        - Same as APArray<>, but uses the comparison operators < and == to sort elements
//          APSorted<>                - APSorted array of pointers to elements with retrieval and sorting by key type
//            APSortedFree<>          - Same as APSorted<>, but calls free_all() on its destruction
//            APSortedLogical<>       - Same as APSorted<>, but uses the comparison operators < and == to sort elements
//              APSortedLogicalFree<> - Same as APSortedLogical<>, but calls free_all() on its destruction
template<class _ElementType, class _NodeIdType = _ElementType>
class AList
  {
  public:
  // Common types

    // Local shorthand
    typedef AList<_ElementType, _NodeIdType>     tAList;
    typedef AListNode<_ElementType, _NodeIdType> tAListNode;

  // Common methods

    AList();
    AList(AList * list_p);
    ~AList();

  // Accessor methods

    const _ElementType * get_sentinel() const;

    _ElementType * get_first() const;
    _ElementType * get_first_null() const;
    _ElementType * get_last() const;
    _ElementType * get_last_null() const;
    _ElementType * get_penultimate() const;
    _ElementType * get_next(const _ElementType * elem_p) const;
    _ElementType * get_next_null(const _ElementType * elem_p) const;
    _ElementType * get_prev(const _ElementType * elem_p) const;
    _ElementType * get_prev_null(const _ElementType * elem_p) const;
    _ElementType * get_next_ring(const _ElementType * elem_p) const;
    _ElementType * get_prev_ring(const _ElementType * elem_p) const;
    bool           is_empty() const;
    bool           is_filled() const;


  // Methods

    void           append(_ElementType * elem_p);
    void           append_take(AList * list_p);
    void           apply_method(void (_ElementType::* method_m)()) const;
    void           apply_method(void (_ElementType::* method_m)() const) const;
    void           take(AList * list_p);
    void           empty();
    uint32_t       free_after(_ElementType * elem_p);
    void           free_all();
    void           free_first();
    void           free_last();
    void           insert(_ElementType * elem_p);
    _ElementType * pop(_ElementType * elem_p);
    _ElementType * pop_first();
    _ElementType * pop_last();
    void           remove(_ElementType * elem_p);
    void           remove_first();
    void           remove_last();
    void           rotate_down();
    void           rotate_up();
    void           swap(_ElementType * elem1_p, _ElementType * elem2_p);


  // Expensive/Debug Methods - *** Avoid if Possible

    template<class _KeyType>
      _ElementType * find_key(const _KeyType & key) const;

    bool           append_absent(_ElementType * elem_p);
    bool           find(const _ElementType * elem_p) const;
    bool           find_reverse(const _ElementType * elem_p) const;
    _ElementType * get_at(uint32_t pos) const;
    uint32_t       get_count() const;
    bool           is_valid() const;
    bool           is_valid_quick() const;
    bool           is_valid_strict() const;

  // C++11 range-based iteration

    class Iterator
      {
      public:
                        Iterator(_ElementType * elem_p) : m_elem_p(elem_p) {}
        Iterator        operator ++()                     { Iterator i = *this; m_elem_p = m_elem_p->AListNode<_ElementType, _NodeIdType>::get_next(); return i; }
        Iterator &      operator ++(int)                  { m_elem_p = m_elem_p->AListNode<_ElementType, _NodeIdType>::get_next(); return *this; }
        _ElementType *  operator *()                      { return m_elem_p; }
        _ElementType *  operator ->()                     { return m_elem_p; }
        bool            operator ==(const Iterator & rhs) { return m_elem_p == rhs.m_elem_p; }
        bool            operator !=(const Iterator & rhs) { return m_elem_p != rhs.m_elem_p; }

      private:
        _ElementType *  m_elem_p;
      };

    Iterator  begin() const   { return Iterator(get_first()); }
    Iterator  end() const     { return Iterator(const_cast<_ElementType *>(get_sentinel())); }

  // Future Methods

    //template<class _InvokeType>
    //  void apply(_InvokeType & invoke_obj) const;
 
    //void as_binary(void ** binary_pp) const;
    //uint32_t as_binary_length() const;
    //void insert_take(AList * list_p);
    //void move_down(_ElementType * elem_p);
    //void move_up(_ElementType * elem_p);
    //void replace(_ElementType * elem_in_list_p, _ElementType * elem_to_substitue_p);
    //void reverse();
    //void sort();


  protected:

  // Internal Methods

    AList(const AList & list);
    AList & operator=(const AList & list);


  // Data members

    tAListNode m_sentinel;

  };  // AList


//---------------------------------------------------------------------------------------
// Notes    Same as AList<> above, but deletes its elements on destruction by calling
//          free_all() in its destructor.
//
//          This class is inherited from AList<> rather than being a specialization of
//          AList<> or using template parameters in AList<> to keep the code size down.
//
// Arg      _ElementType - the class/type of elements that can be added to the list.
//          It must be derived from the AListNode<> class template.
// Arg      _NodeIdType - if the same _ElementType needs to simultaneously exist in more
//          than one AList, this differentiates each list and node structure to have the
//          _ElementType derive from.  The type supplied to _NodeIdType does not need to
//          actually exist or be related to _ElementType - it just needs to be declared.
//
// See Also AList<>                   - Same as AListFree<>, but does not call free_all() on its destruction
//          APArray<>                 - Ordered array of pointers to elements with retrieval by key type
//            APArrayFree<>           - Same as APArray<>, but calls free_all() on its destruction
//            APArrayLogical<>        - Same as APArray<>, but uses the comparison operators < and == to sort elements
//          APSorted<>                - APSorted array of pointers to elements with retrieval and sorting by key type
//            APSortedFree<>          - Same as APSorted<>, but calls free_all() on its destruction
//            APSortedLogical<>       - Same as APSorted<>, but uses the comparison operators < and == to sort elements
//              APSortedLogicalFree<> - Same as APSortedLogical<>, but calls free_all() on its destruction
template<class _ElementType, class _NodeIdType = _ElementType>
class AListFree : public AList<_ElementType, _NodeIdType>
  {
  public:
  // Common types

    // Local shorthand
    typedef AList<_ElementType, _NodeIdType> tAList;

  // All the AList<> constructors are hidden, so make appropriate links

    AListFree()                                              {}
    ~AListFree()                                             { tAList::free_all(); }
    AListFree(AList<_ElementType, _NodeIdType> * list_p)     : tAList(list_p) {}

  protected:
  // Internal Methods

    AListFree(const AList<_ElementType, _NodeIdType> & list) : tAList(list) {}
    AListFree(const AListFree & list)                        : tAList(list) {}

  };  // AListFree


//=======================================================================================
// Internal Macros
//=======================================================================================

// This is used internally in the AList/AListNode definitions rather than using
// static_cast<AListNode<_ElementType, _NodeIdType> *>(elem_p).  It generates less code
// and is faster than the default compiler's static_cast.  It is written as a macro rather
// than a inlined function so that it is inlined in non-optimized builds as well.
#define ALIST_ELEM_TO_NODE(_elem_p) \
  ((AListNode<_ElementType, _NodeIdType> *)((uint8_t *)(_elem_p) + (((uintptr_t)&(((_ElementType *)4)->AListNode<_ElementType, _NodeIdType>::m_next_p)) - 4)))

// This is used internally in the AList/AListNode definitions rather than using
// static_cast<_ElementType *>(node_p). / It generates less code and is faster than the
// default compiler's static_cast.  It is written as a macro rather than a function so
// that it is inlined in non-optimized builds as well.
#define ALIST_NODE_TO_ELEM(_node_p) \
  ((_ElementType *)((uint8_t *)(_node_p) - (((uintptr_t)&(((_ElementType *)4)->AListNode<_ElementType, _NodeIdType>::m_next_p)) - 4)))


//=======================================================================================
// AList<> Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default constructor.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline AList<_ElementType, _NodeIdType>::AList()
  {
  }

//---------------------------------------------------------------------------------------
// Take ownership constructor - transfers elements from list_p to this list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline AList<_ElementType, _NodeIdType>::AList(AList * list_p)
  {
  append_take(list_p);
  }

//---------------------------------------------------------------------------------------
// Copy constructor - does nothing for intrinsic lists
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline AList<_ElementType, _NodeIdType>::AList(const AList & list)
  {
  // Leaves 'this' and 'list' unmodified
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline AList<_ElementType, _NodeIdType>::~AList()
  {
  // Each element is removed from the list so that their next and previous pointers are
  // properly reset.
  empty();
  }

//---------------------------------------------------------------------------------------
// Assignment operator - does nothing for intrinsic lists
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline AList<_ElementType, _NodeIdType> & AList<_ElementType, _NodeIdType>::operator=(const AList & list)
  {
  // Leaves 'this' and 'list' unmodified
  return *this;
  }

//---------------------------------------------------------------------------------------
// Insert element at the tail/end of the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::append(_ElementType * elem_p)
  {
  m_sentinel.link_prev(elem_p);
  }

//---------------------------------------------------------------------------------------
// Insert element at the tail/end of the list if the element is not already in the list.
// *** Uses a simple linear search, so can be slow on larger lists.
// 
// Returns:   `true` if element was appended, `false` if element was already in list.
// Author(s): Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AList<_ElementType, _NodeIdType>::append_absent(_ElementType * elem_p)
  {
  if (!find(elem_p))
    {
    m_sentinel.link_prev(elem_p);

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Take ownership - transfers elements from list_p to the end of this list.
// 
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::append_take(AList * list_p)
  {
  if (list_p->m_sentinel.m_next_p == &list_p->m_sentinel)
    {
    // Empty so quit early
    return;
    }

  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid() && list_p->is_valid(), "List failed validity check!");
  #endif

  tAListNode * sentinel_p      = &m_sentinel;
  tAListNode * last_p          = sentinel_p->m_prev_p;
  tAListNode * list_sentinel_p = &list_p->m_sentinel;
  tAListNode * list_first_p    = list_sentinel_p->m_next_p;
  tAListNode * list_last_p     = list_sentinel_p->m_prev_p;

  last_p->m_next_p       = list_first_p;
  list_first_p->m_prev_p = last_p;

  list_last_p->m_next_p = sentinel_p;
  sentinel_p->m_prev_p  = list_last_p;

  list_sentinel_p->m_next_p = list_sentinel_p;
  list_sentinel_p->m_prev_p = list_sentinel_p;
  }

//---------------------------------------------------------------------------------------
// Calls the supplied *non-const* `method_m` method on all elements.
// 
// Params:
//   method_m:
//     method address of a method available to `_ElementType`.  It must take no arguments
//     and return no arguments.
//     
// Examples:  list.apply_method(do_stuff);
// See:       apply()
// Author(s): Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::apply_method(
  void (_ElementType::* method_m)()
  ) const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p = &m_sentinel;
  const tAListNode * next_p     = sentinel_p->m_next_p;

  // If not empty
  if (next_p != sentinel_p)
    {
    do
      {
      (ALIST_NODE_TO_ELEM(next_p)->*method_m)();
      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }
  }

//---------------------------------------------------------------------------------------
// Calls the supplied *constant* `method_m` method on all elements.
// 
// Params:
//   method_m:
//     method address of a method available to `_ElementType`.  It must take no arguments
//     and return no arguments.
//     
// Examples:  list.apply_method(do_stuff);
// See:       apply()
// Author(s): Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::apply_method(
  void (_ElementType::* method_m)() const
  ) const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p = &m_sentinel;
  const tAListNode * next_p     = sentinel_p->m_next_p;

  // If not empty
  if (next_p != sentinel_p)
    {
    do
      {
      (ALIST_NODE_TO_ELEM(next_p)->*method_m)();
      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }
  }

//---------------------------------------------------------------------------------------
// Remove all elements and ensure their next and previous pointers are properly reset.
// 
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::empty()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  tAListNode * sentinel_p = &m_sentinel;
  tAListNode * next_p     = sentinel_p->m_next_p;

  if (next_p != sentinel_p)
    {
    tAListNode * node_p;

    do
      {
      node_p = next_p;
      next_p = next_p->m_next_p;

      node_p->m_next_p = node_p;
      node_p->m_prev_p = node_p;
      }
    while (next_p != sentinel_p);

    sentinel_p->m_next_p = sentinel_p;
    sentinel_p->m_prev_p = sentinel_p;
    }
  }

//---------------------------------------------------------------------------------------
// Determine if the specified element is in this list `true` or not `false`.
// *** Uses a simple linear search, so can be slow on larger lists. Looks in order from
// head to tail - so use if element is more likely to be closer to the head than the tail.
// 
// Returns:   `true` if in list, `false` if not
// Author(s): Conan Reis
template<class _ElementType, class _NodeIdType>
bool AList<_ElementType, _NodeIdType>::find(const _ElementType * elem_p) const
  {
  const tAListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(elem_node_p->is_valid_node() && is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p  = &m_sentinel;
  const tAListNode * next_p      = sentinel_p->m_next_p;

  // If not empty and element is is *some* list
  if ((next_p != sentinel_p) && (elem_node_p->m_next_p != elem_node_p))
    {
    do
      {
      if (next_p == elem_node_p)
        {
        return true;
        }

      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determine if the specified element is in this list (true) or not (false).
//             *** Uses a simple reverse linear search, so can be slow on larger lists.
//             Looks in reverse order from tail to head - so use if element is more likely
//             to be closer to the tail than the head.
// Returns:    true if in list, false if not
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
bool AList<_ElementType, _NodeIdType>::find_reverse(const _ElementType * elem_p) const
  {
  const tAListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(elem_node_p->is_valid_node() && is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p  = &m_sentinel;
  const tAListNode * prev_p      = sentinel_p->m_prev_p;

  // If not empty and element is is *some* list
  if ((prev_p != sentinel_p) && (elem_node_p->m_next_p != elem_node_p))
    {
    do
      {
      if (prev_p == elem_node_p)
        {
        return true;
        }

      prev_p = prev_p->m_prev_p;
      }
    while (prev_p != sentinel_p);
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determine if an element [that can be compared to/coerced to the _KeyType]
//             and is logically equal to key is in this list (true) or not (false).
//             *** Uses a simple linear search, so can be slow on larger lists.
// Returns:    true if in list, false if not
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
template<class _KeyType>
_ElementType * AList<_ElementType, _NodeIdType>::find_key(const _KeyType & key) const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p  = &m_sentinel;
  const tAListNode * next_p      = sentinel_p->m_next_p;

  // If not empty
  if (next_p != sentinel_p)
    {
    _ElementType * elem_p;

    do
      {
      elem_p = ALIST_NODE_TO_ELEM(next_p);

      if (key == *elem_p)
        {
        return elem_p;
        }

      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Frees all elements that follow the supplied element and ensure their next
//             and previous pointers are properly reset.
// Returns:    Number of elements freed
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
uint32_t AList<_ElementType, _NodeIdType>::free_after(_ElementType * elem_p)
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  tAListNode * sentinel_p  = &m_sentinel;
  tAListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);
  tAListNode * last_p      = sentinel_p->m_prev_p;
  uint32_t     free_count  = 0u;

  while (last_p != elem_node_p)
    {
    // Removed in the AListNode destructor
    delete ALIST_NODE_TO_ELEM(last_p);

    free_count++;
    last_p = sentinel_p->m_prev_p;
    }

  return free_count;
  }

//---------------------------------------------------------------------------------------
// Free all elements and ensure their next and previous pointers are properly
//             reset.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::free_all()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  tAListNode * sentinel_p = &m_sentinel;
  tAListNode * next_p     = sentinel_p->m_next_p;

  if (next_p != sentinel_p)
    {
    tAListNode * node_p;

    // Iterate through list deleting elements.
    do
      {
      node_p               = next_p;
      next_p               = next_p->m_next_p;
      sentinel_p->m_next_p = next_p;

      // Ensure test for "in list" = false so remove() is not called in destructor.
      // Don't bother setting node_p->m_prev_p = node_p since it is being deleted anyway.
      node_p->m_next_p = node_p;

      // $Note - CReis This delete can call code that requires this list to be valid.
      // Keep the list integrity as elements are removed in case destruction of one element
      // causes destruction of some other element.
      delete ALIST_NODE_TO_ELEM(node_p);
      }
    while (next_p != sentinel_p);

    sentinel_p->m_prev_p = sentinel_p;
    }
  }

//---------------------------------------------------------------------------------------
// Frees the first element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::free_first()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_next_p;

  // Removed in the AListNode destructor
  delete ALIST_NODE_TO_ELEM(elem_p);
  }

//---------------------------------------------------------------------------------------
// Frees the last element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::free_last()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_prev_p;

  // Removed in the AListNode destructor
  delete ALIST_NODE_TO_ELEM(elem_p);
  }

//---------------------------------------------------------------------------------------
// Find element at specified index position.  [Asserts if index larger than
//             number of elements.]
//             *** Uses a simple linear search, so can be slow on larger lists.
// Returns:    true if in list, false if not
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
_ElementType * AList<_ElementType, _NodeIdType>::get_at(uint32_t index) const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p  = &m_sentinel;
  const tAListNode * next_p      = sentinel_p->m_next_p;
  uint32_t           count       = 0u;

  // If not empty
  if (next_p != sentinel_p)
    {
    do
      {
      if (count == index)
        {
        return ALIST_NODE_TO_ELEM(next_p);
        }

      index++;
      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }

  #ifdef ALIST_EXTRA_CHECK
    A_ERRORX(a_cstr_format("Tried to get an element at index %u, but list only has %u elements!", index, count));
  #endif

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Counts the number of elements in the list.
//             *** Uses a simple linear search, so can be slow on larger lists.
// Returns:    number of elements
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
uint32_t AList<_ElementType, _NodeIdType>::get_count() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  const tAListNode * sentinel_p  = &m_sentinel;
  const tAListNode * next_p      = sentinel_p->m_next_p;
  uint32_t           count       = 0u;

  // If not empty
  if (next_p != sentinel_p)
    {
    do
      {
      count++;
      next_p = next_p->m_next_p;
      }
    while (next_p != sentinel_p);
    }

  return count;
  }

//---------------------------------------------------------------------------------------
// Returns the first element in the list.  If there are no elements in the
//             list, the sentinel node is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_first() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return m_sentinel.get_next();
  }

//---------------------------------------------------------------------------------------
// Returns the first element in the list.  If there are no elements in the
//             list, nullptr is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_first_null() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return (m_sentinel.m_next_p != &m_sentinel)
    ? m_sentinel.get_next()
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Returns the last element in the list.  If there are no elements in the
//             list, the sentinel node is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_last() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return m_sentinel.get_prev();
  }

//---------------------------------------------------------------------------------------
// Returns the last element in the list.  If there are no elements in the
//             list, nullptr is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_last_null() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return (m_sentinel.m_prev_p != &m_sentinel)
    ? m_sentinel.get_prev()
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Returns the second to last element in the list.  If there are no elements
//             in the list, the sentinel node is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_penultimate() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return m_sentinel.m_prev_p->get_prev();
  }

//---------------------------------------------------------------------------------------
// Returns the element that is after elem_p.  If elem_p is the last element,
//             the sentinel is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_next(const _ElementType * elem_p) const
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  return ALIST_ELEM_TO_NODE(elem_p)->get_next();
  }

//---------------------------------------------------------------------------------------
// Returns the element that is after elem_p.  If elem_p is the last element,
//             nullptr is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_next_null(const _ElementType * elem_p) const
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  tAListNode * node_p = ALIST_ELEM_TO_NODE(elem_p)->m_next_p;

  return (node_p == &m_sentinel) ? nullptr : ALIST_NODE_TO_ELEM(node_p);
  }

//---------------------------------------------------------------------------------------
// Returns the element that is before elem_p.  If elem_p is the first element,
//             the sentinel is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_prev(const _ElementType * elem_p) const
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  return ALIST_ELEM_TO_NODE(elem_p)->get_prev();
  }

//---------------------------------------------------------------------------------------
// Returns the element that is before elem_p.  If elem_p is the first element,
//             nullptr is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_prev_null(const _ElementType * elem_p) const
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  tAListNode * node_p = ALIST_ELEM_TO_NODE(elem_p)->m_prev_p;

  return (node_p == &m_sentinel) ? nullptr : ALIST_NODE_TO_ELEM(node_p);
  }

//---------------------------------------------------------------------------------------
// Returns the element that is after elem_p treating this object as a
//             circular list.  If elem_p is the last element, the fist element is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_next_ring(const _ElementType * elem_p) const
  {
  tAListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #if defined(ALIST_STRICT_CHECK)
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #elif defined(ALIST_EXTRA_CHECK)
    A_VERIFYX(is_valid() && elem_node_p->is_valid_node(), "List failed validity check!");
  #endif

  tAListNode *       next_p      = elem_node_p->m_next_p;
  const tAListNode * sentinel_p  = &m_sentinel;

  return ALIST_NODE_TO_ELEM((next_p != sentinel_p) ? next_p : next_p->m_next_p);
  }

//---------------------------------------------------------------------------------------
// Returns the element that is before elem_p treating this object as a
//             circular list.  If elem_p is the first element, the last element is returned.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::get_prev_ring(const _ElementType * elem_p) const
  {
  tAListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #if defined(ALIST_STRICT_CHECK)
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #elif defined(ALIST_EXTRA_CHECK)
    A_VERIFYX(is_valid() && elem_node_p->is_valid_node(), "List failed validity check!");
  #endif

  tAListNode *       prev_p      = elem_node_p->m_prev_p;
  const tAListNode * sentinel_p  = &m_sentinel;

  return ALIST_NODE_TO_ELEM((prev_p != sentinel_p) ? prev_p : prev_p->m_prev_p);
  }

//---------------------------------------------------------------------------------------
// Returns the sentinel node as an element pointer.  It is used to determine
//             the beginning and ending of a list - akin to a nullptr in many other types of
//             containers.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline const _ElementType * AList<_ElementType, _NodeIdType>::get_sentinel() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return ALIST_NODE_TO_ELEM(&m_sentinel);
  }

//---------------------------------------------------------------------------------------
// Insert element at the head/beginning of the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::insert(_ElementType * elem_p)
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return m_sentinel.link_next(elem_p);
  }

//---------------------------------------------------------------------------------------
// Returns true if list is empty and false if it contains 1 or more elements.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AList<_ElementType, _NodeIdType>::is_empty() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return (m_sentinel.m_next_p == &m_sentinel);
  }

//---------------------------------------------------------------------------------------
// Returns true if list contains 1 or more elements and false if it is empty.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AList<_ElementType, _NodeIdType>::is_filled() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  return (m_sentinel.m_next_p != &m_sentinel);
  }

//---------------------------------------------------------------------------------------
// Quickly validates list for integrity by testing sentinel node.
// Returns:    true if list is valid, false if list is invalid.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AList<_ElementType, _NodeIdType>::is_valid_quick() const
  {
  return (this && m_sentinel.is_valid_node());
  }

//---------------------------------------------------------------------------------------
// Strictly validates list for integrity by testing sentinel node and each
//             element in the list.
// Returns:    true if list is valid, false if list is invalid.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
bool AList<_ElementType, _NodeIdType>::is_valid_strict() const
  {
  if ((this == nullptr) || !m_sentinel.is_valid_node())
    {
    // Invalid list
    return false;
    }

  const tAListNode * sentinel_p = &m_sentinel;
  const tAListNode * node_p     = sentinel_p->m_next_p;

  while (node_p != sentinel_p)
    {
    if ((node_p->m_next_p == nullptr) || (node_p->m_next_p->m_prev_p != node_p))
      {
      // Invalid list
      return false;
      }

    node_p = node_p->m_next_p;
    }

  // List is valid
  return true;
  }

//---------------------------------------------------------------------------------------
// Validates list for integrity.
//             [Calls is_valid_strict() if ALIST_STRICT_CHECK is defined and is_valid_quick()
//             if it is not defined.]
// Returns:    true if list is valid, false if list is invalid.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AList<_ElementType, _NodeIdType>::is_valid() const
  {
  #ifdef ALIST_STRICT_CHECK
    return is_valid_strict();
  #else
    return is_valid_quick();
  #endif
  }

//---------------------------------------------------------------------------------------
// Removes and returns the specified element.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::pop(_ElementType * elem_p)
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  ALIST_ELEM_TO_NODE(elem_p)->remove();

  return elem_p;
  }

//---------------------------------------------------------------------------------------
// Removes and returns the first element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::pop_first()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_next_p;

  elem_p->remove();
  return ALIST_NODE_TO_ELEM(elem_p);
  }

//---------------------------------------------------------------------------------------
// Removes and returns the last element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AList<_ElementType, _NodeIdType>::pop_last()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_prev_p;

  elem_p->remove();
  return ALIST_NODE_TO_ELEM(elem_p);
  }

//---------------------------------------------------------------------------------------
// Removes the specified element from the list.
// If `ALIST_STRICT_CHECK` is defined it asserts that the element is actually in this list.
// Note that the element also has a `AListNode<>::remove()` that can be called without
// specifying the containing list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::remove(_ElementType * elem_p)
  {
  #ifdef ALIST_STRICT_CHECK
    A_VERIFYX(find(elem_p), "Element expects to be in this list, but it is not!");
  #endif

  ALIST_ELEM_TO_NODE(elem_p)->remove();
  }

//---------------------------------------------------------------------------------------
// Removes the first element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::remove_first()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_next_p;

  elem_p->remove();
  }

//---------------------------------------------------------------------------------------
// Removes the last element in the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::remove_last()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
    A_VERIFYX(is_filled(), "Cannot perform operation - list is empty!");
  #endif

  tAListNode * elem_p = m_sentinel.m_prev_p;

  elem_p->remove();
  }

//---------------------------------------------------------------------------------------
// Moves last element to first position (head/front) of the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::rotate_down()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  tAListNode * sentinel_p = &m_sentinel;
  tAListNode * prev_p     = sentinel_p->m_prev_p;
  tAListNode * next_p     = sentinel_p->m_next_p;

  // If not empty and at least two elements in list
  if ((prev_p != sentinel_p) && (prev_p != next_p))
    {
    prev_p->remove();

    sentinel_p->m_next_p = prev_p;
    next_p->m_prev_p     = prev_p;
    prev_p->m_next_p     = next_p;
    prev_p->m_prev_p     = sentinel_p;
    }
  }

//---------------------------------------------------------------------------------------
// Moves first element to last position (tail/end) of the list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::rotate_up()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid(), "List failed validity check!");
  #endif

  tAListNode * sentinel_p = &m_sentinel;
  tAListNode * prev_p     = sentinel_p->m_prev_p;
  tAListNode * next_p     = sentinel_p->m_next_p;

  // If not empty and at least two elements in list
  if ((prev_p != sentinel_p) && (prev_p != next_p))
    {
    next_p->remove();

    sentinel_p->m_prev_p = next_p;
    prev_p->m_next_p     = next_p;
    next_p->m_prev_p     = prev_p;
    next_p->m_next_p     = sentinel_p;
    }
  }

//---------------------------------------------------------------------------------------
// Take ownership constructor - transfers elements from list_p to this list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
void AList<_ElementType, _NodeIdType>::swap(
  _ElementType * elem1_p,
  _ElementType * elem2_p
  )
  {
  if (elem1_p != elem2_p)
    {
    tAListNode * elem1_node_p = ALIST_ELEM_TO_NODE(elem1_p);
    tAListNode * elem2_node_p = ALIST_ELEM_TO_NODE(elem2_p);

    #if defined(ALIST_STRICT_CHECK)
      A_VERIFYX(find(elem1_p) && find(elem2_p), "Element expects to be in this list, but it is not!");
    #elif defined(ALIST_EXTRA_CHECK)
      A_VERIFYX(is_valid() && elem1_node_p->is_valid_node() && elem2_node_p->is_valid_node(), "List failed validity check!");
    #endif

    tAListNode * prev1_p = elem1_node_p->m_prev_p;
    tAListNode * next2_p = elem2_node_p->m_next_p;

    elem1_node_p->remove();
    elem2_node_p->remove();

    prev1_p->link_next(elem2_p);
    next2_p->link_prev(elem1_p);
    }
  }

//---------------------------------------------------------------------------------------
// Take/transfer ownership - transfers elements from list_p to this list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AList<_ElementType, _NodeIdType>::take(AList * list_p)
  {
  empty();
  append_take(list_p);
  }


//=======================================================================================
// AListNode<> Methods
//=======================================================================================

// $Revisit - CReis Judicious use of the __restrict C++ keyword could speed up this code
// and make it smaller as well.

//---------------------------------------------------------------------------------------
// Gets the next element.
// Returns:    next element
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AListNode<_ElementType, _NodeIdType>::get_next() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid_node() && is_in_list(), "List node is either not valid or not in a list!");
  #endif

  return ALIST_NODE_TO_ELEM(m_next_p);
  }

//---------------------------------------------------------------------------------------
// Gets the previous element.
// Returns:    previous element
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline _ElementType * AListNode<_ElementType, _NodeIdType>::get_prev() const
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid_node() && is_in_list(), "List node is either not valid or not in a list!");
  #endif

  return ALIST_NODE_TO_ELEM(m_prev_p);
  }

//---------------------------------------------------------------------------------------
// Ensures that this node is valid
// Returns:    true if valid, false if not
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline bool AListNode<_ElementType, _NodeIdType>::is_valid_node() const
  {
  return (this && m_next_p && m_prev_p && (m_prev_p->m_next_p == this) && (m_next_p->m_prev_p == this));
  }

//---------------------------------------------------------------------------------------
// Links/inserts the specified element's node after (next after) this node.
// Arg         elem_p - pointer to element to link.  It should not already be in a list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AListNode<_ElementType, _NodeIdType>::link_next(_ElementType * elem_p)
  {
  AListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid_node() && elem_node_p->is_valid_node() && !elem_node_p->is_in_list(), "Invalid list node.");
  #endif

  AListNode * next_p = m_next_p;

  m_next_p              = elem_node_p;
  next_p->m_prev_p      = elem_node_p;
  elem_node_p->m_next_p = next_p;
  elem_node_p->m_prev_p = this;
  }

//---------------------------------------------------------------------------------------
// Links/inserts the specified element's node before (previous to) this node.
// Arg         elem_p - pointer to element to link.  It should not already be in a list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AListNode<_ElementType, _NodeIdType>::link_prev(_ElementType * elem_p)
  {
  AListNode * elem_node_p = ALIST_ELEM_TO_NODE(elem_p);

  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid_node() && elem_node_p->is_valid_node() && !elem_node_p->is_in_list(), "Invalid list node.");
  #endif

  AListNode * prev_p = m_prev_p;

  m_prev_p              = elem_node_p;
  prev_p->m_next_p      = elem_node_p;
  elem_node_p->m_prev_p = prev_p;
  elem_node_p->m_next_p = this;
  }

//---------------------------------------------------------------------------------------
// Removes this element node from any list that is in. If it is not in a list then do
// nothing.  [It can be called redundantly/multiple times safely.]
// Note that there is a `AList<>::remove()` that can assert that the element is actually
// in the expected list.
// Author(s):   Conan Reis
template<class _ElementType, class _NodeIdType>
inline void AListNode<_ElementType, _NodeIdType>::remove()
  {
  #ifdef ALIST_EXTRA_CHECK
    A_VERIFYX(is_valid_node(), "Invalid list node.");
  #endif

  AListNode * __restrict next_p = m_next_p;
  AListNode * __restrict prev_p = m_prev_p;

  next_p->m_prev_p = prev_p;
  prev_p->m_next_p = next_p;

  m_next_p = m_prev_p = this;
  }
