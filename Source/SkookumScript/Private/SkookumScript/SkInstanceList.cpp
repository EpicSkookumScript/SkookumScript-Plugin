// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// List of SkInstances
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkInstanceList.hpp>
#include <SkookumScript/SkMethod.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <AgogCore/AMath.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Copy constructor
SkInstanceList::SkInstanceList(const SkInstanceList & other)
  {
  SkInstance ** end_pp = other.m_items.get_array_end();
  for (SkInstance ** obj_pp = other.m_items.get_array(); obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->reference();
    }
  m_items.append_all(other.m_items);
  }

//---------------------------------------------------------------------------------------
// Move constructor
SkInstanceList::SkInstanceList(const SkInstanceList && other)
  {
  memcpy(&m_items, &other.m_items, sizeof(m_items));
  }

//---------------------------------------------------------------------------------------
// Destructor
SkInstanceList::~SkInstanceList()
  {
  SkInstance ** end_pp = m_items.get_array_end();
  for (SkInstance ** obj_pp = m_items.get_array(); obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->dereference();
    }
  }

//---------------------------------------------------------------------------------------
// Assignment
SkInstanceList & SkInstanceList::operator=(const SkInstanceList & other)
  {
  SkInstance ** end_pp = other.m_items.get_array_end();
  for (SkInstance ** obj_pp = other.m_items.get_array(); obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->reference();
    }
  m_items = other.m_items;
  return *this;
  }

//---------------------------------------------------------------------------------------
// Appends instance element to list and optionally increments its reference
//             count.
// Arg         instance - instance to append
// Arg         reference_b - true auto increments reference to instance, false leaves it
//             as is.
// See:        remove()
void SkInstanceList::append(
  SkInstance & instance,
  bool         reference_b // = true
  )
  {
  m_items.append(instance);
  
  if (reference_b)
    {
    instance.reference();
    }
  }

//---------------------------------------------------------------------------------------
// Appends all elements in list to the current list
void SkInstanceList::append_all(const SkInstanceList & other, uint32_t start_pos, uint32_t count)
  {
  if (count == ALength_remainder)
    {
    count = other.m_items.get_length() - start_pos;
    }

  SkInstance ** obj_pp = other.m_items.get_array() + start_pos;
  SkInstance ** end_pp = obj_pp + count;
  for (; obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->reference();
    }
  m_items.append_all(other.m_items, start_pos, count);
  }

//---------------------------------------------------------------------------------------
// Appends the same element elem_count times to the list
void SkInstanceList::append_all(uint32_t elem_count, SkInstance & instance)
  {
  instance.reference(elem_count);
  m_items.append_all(elem_count, &instance);
  }

//---------------------------------------------------------------------------------------
// Appends nil count times to the list
void SkInstanceList::append_nil(uint32_t count)
  {
  m_items.append_all(count, SkBrain::ms_nil_p);
  }

//---------------------------------------------------------------------------------------
// Appends nil to the list
void SkInstanceList::append_nil()
  {
  m_items.append(*SkBrain::ms_nil_p);
  }

//---------------------------------------------------------------------------------------
// Sets instance at a given index
void SkInstanceList::set_at(uint32_t index, SkInstance & instance)
  {
  if (index >= get_length())
    {
    SK_ERRORX(a_str_format(
      "Tried to access beyond list range - given 0-based index %i, but length only %u!",
      index,
      m_items.get_length()));

    return;
    }

  SkInstance ** elem_pp = &m_items.get_array()[index];
  instance.reference();
  (*elem_pp)->dereference();
  *elem_pp = &instance;
  }

//---------------------------------------------------------------------------------------
// Sets instance at a given index, does not increment its ref count
void SkInstanceList::set_at_no_ref(uint32_t index, SkInstance & instance)
  {
  SkInstance ** elem_pp = &m_items.get_array()[index];
  (*elem_pp)->dereference();
  *elem_pp = &instance;
  }

//---------------------------------------------------------------------------------------
// Keeps elem_count elements starting at index pos and removes the rest.
void SkInstanceList::crop(uint32_t pos, uint32_t count)
  {
  if (count == ALength_remainder)
    {
    count = m_items.get_length() - pos;
    }

  SkInstance ** pos_pp = m_items.get_array() + pos;
  for (SkInstance ** obj_pp = m_items.get_array(); obj_pp < pos_pp; ++obj_pp)
    {
    (*obj_pp)->dereference();
    }
  SkInstance ** end_pp = m_items.get_array_end();
  for (SkInstance ** obj_pp = pos_pp + count; obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->dereference();
    }
  m_items.crop(pos, count);
  }

//---------------------------------------------------------------------------------------
//  Removes all elements from collection
void SkInstanceList::empty()
  {
  #if 1

    SkInstance ** end_pp = m_items.get_array_end();
    for (SkInstance ** obj_pp = m_items.get_array(); obj_pp < end_pp; ++obj_pp)
      {
      (*obj_pp)->dereference();
      }
    m_items.empty();

  #else

    // Use prefetch to speed things up - profile didn't show enough of a benefit 
    SkInstance ** begin_pp = m_items.get_array();
    SkInstance ** end_pp = m_items.get_array_end();
    if (begin_pp < end_pp)
      {
      --end_pp;
      for (SkInstance ** obj_pp = begin_pp; obj_pp <= end_pp; ++obj_pp)
        {
        a_prefetch(*a_min(obj_pp + 1, end_pp));
        (*obj_pp)->dereference();
        }
      m_items.empty();
      }

  #endif
  }

//---------------------------------------------------------------------------------------
//  Removes all elements from collection and frees array memory
void SkInstanceList::empty_compact()
  {
  empty();
  m_items.compact();
  }

//---------------------------------------------------------------------------------------
// Inserts elem count times at index pos
void SkInstanceList::insert(SkInstance & instance, uint32_t pos, uint32_t count)
  {
  instance.reference(count);
  m_items.insert(instance, pos, count);
  }

//---------------------------------------------------------------------------------------
// Inserts elements from array at index pos
void SkInstanceList::insert_all(const SkInstanceList & list, uint32_t pos)
  {
  SkInstance ** end_pp = list.m_items.get_array_end();
  for (SkInstance ** obj_pp = list.m_items.get_array(); obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->reference();
    }
  m_items.insert_all(list.m_items, pos);
  }

//---------------------------------------------------------------------------------------
// Removes instance of elem between start_pos and end_pos, returning true if found, false if not.
bool SkInstanceList::remove(SkInstance & instance, uint32_t match /*= AMatch_first_found*/, uint32_t * find_pos_p /*= nullptr*/, uint32_t start_pos /*= 0u*/, uint32_t end_pos /*= ALength_remainder*/)
  {
  if (m_items.remove(instance, match, find_pos_p, start_pos, end_pos))
    {
    instance.dereference();
    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Removes elem_count elements starting at index pos
void SkInstanceList::remove_all(uint32_t pos /*= 0u*/, uint32_t count /*= ALength_remainder*/)
  {
  if (count == ALength_remainder)
    {
    count = m_items.get_length() - pos;
    }

  SkInstance ** obj_pp = m_items.get_array() + pos;
  SkInstance ** end_pp = obj_pp + count;
  for (; obj_pp < end_pp; ++obj_pp)
    {
    (*obj_pp)->dereference();
    }
  m_items.remove_all(pos, count);
  }

//---------------------------------------------------------------------------------------
// Sets elem_count elements starting at index pos to the specified element.
//             Any existing elements in the range will be overwritten and the array will
//             extend its total length if the range extends beyond its current length.
void SkInstanceList::set_all(SkInstance & instance, uint32_t pos /*= 0*/, uint32_t count /*= ALength_remainder*/)
  {
  if (count == ALength_remainder)
    {
    count = m_items.get_length() - pos;
    }

  instance.reference(count);

  uint32_t overlap = a_min(count, get_length() - a_min(pos, get_length()));
  if (overlap)
    {
    for (SkInstance ** obj_pp = m_items.get_array() + pos; overlap; --overlap, ++obj_pp)
      {
      (*obj_pp)->dereference();
      }
    }

  m_items.set_all(&instance, pos, count);
  }

//---------------------------------------------------------------------------------------
// Appends all elements in the list as strings to the supplied string.
void SkInstanceList::append_elems_as_strings(
  AString *       str_p,
  SkInvokedBase * caller_p // = nullptr
  ) const
  {
  uint32_t obj_count = m_items.get_length();

  if (obj_count)
    {
    SkInstance **     objs_pp     = m_items.get_array();
    SkInstance **     objs_end_pp = objs_pp + obj_count;
    
    // Make a guess at the needed size - it will expand if needed
    str_p->ensure_size(str_p->get_length() + (obj_count * Item_str_length_def));

    while (objs_pp < objs_end_pp)
      {
      (*objs_pp)->as_string_invoke_append(str_p, caller_p);
      objs_pp++;
      }
    }
  }
