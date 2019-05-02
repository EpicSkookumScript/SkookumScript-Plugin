// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// List of SkInstances
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APArray.hpp>
#include <SkookumScript/SkInstance.hpp>

//---------------------------------------------------------------------------------------
// List of SkInstances
class SK_API SkInstanceList
  {
  public:
	  SK_NEW_OPERATORS(SkInstanceList);

  // Enumerated constants
    enum
      {
      Item_str_length_def = 8
      };

  // Methods

    explicit              SkInstanceList(uint32_t ensure_size = 0u) : m_items(nullptr, 0u, ensure_size) {}
                          SkInstanceList(const SkInstanceList & other);
                          SkInstanceList(const SkInstanceList && other);
                          ~SkInstanceList();

    SkInstanceList &      operator = (const SkInstanceList & other);

    SkInstance **         get_array() const               { return m_items.get_array(); }
    SkInstance **         get_array_end() const           { return m_items.get_array_end(); }
    SkInstance *          get_last() const                { return m_items.get_last(); }
    uint32_t              get_length() const              { return m_items.get_length(); }
    uint32_t              get_size() const                { return m_items.get_size(); }
    APArray<SkInstance> & get_instances() const           { return const_cast<SkInstanceList *>(this)->m_items; }
    SkInstance *          operator[](uint32_t pos) const  { return m_items[pos]; }

    void                  append(SkInstance & instance, bool reference_b = true);
    void                  append_all(const SkInstanceList & other, uint32_t start_pos = 0, uint32_t count = ALength_remainder);
    void                  append_all(uint32_t elem_count, SkInstance & instance);
    void                  append_nil();
    void                  append_nil(uint32_t count);
    void                  set_at(uint32_t index, SkInstance & instance);
    void                  set_at_no_ref(uint32_t index, SkInstance & instance);
    void                  crop(uint32_t pos, uint32_t count);
    void                  empty();
    void                  empty_compact();
    void                  ensure_size(uint32_t needed)          { m_items.ensure_size(needed); }
    void                  ensure_size_empty(uint32_t needed)    { empty(); m_items.ensure_size(needed); }
    void                  insert(SkInstance & instance, uint32_t pos, uint32_t count = 1);
    void                  insert_all(const SkInstanceList & list, uint32_t pos);
    bool                  remove(SkInstance & instance, uint32_t match = AMatch_first_found, uint32_t * find_pos_p = nullptr, uint32_t start_pos = 0u, uint32_t end_pos = ALength_remainder);
    void                  remove_all(uint32_t pos, uint32_t count = ALength_remainder);
    void                  set_all(SkInstance & instance, uint32_t pos = 0, uint32_t count = ALength_remainder);
    void                  set_custom_memory_empty_unsafe(SkInstance ** custom_mem_pp, uint32_t allocated_count);

    void                  append_elems_as_strings(AString * str_p, SkInvokedBase * caller_p = nullptr) const;

  protected:

  // Data Members

    // Ordered array of items/elements.
    APArray<SkInstance> m_items;

  };  // SkInstanceList

//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Sets the array storage pointer to a custom memory block, not managed by the array itself
// WARNING: (1) DO NOT try to grow the array beyond the given count of elements!!
//          (2) You MUST call this same function with a nullptr argument before this list is destructed 
//              or else it will try to free the custom memory block to the heap
inline void SkInstanceList::set_custom_memory_empty_unsafe(SkInstance ** custom_mem_pp, uint32_t allocated_count)
  {
  m_items.set_custom_memory_empty_unsafe(custom_mem_pp, allocated_count);
  }
