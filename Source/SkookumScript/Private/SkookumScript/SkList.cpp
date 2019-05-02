// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic List (dynamic array of typed objects) class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkList.hpp>
#include <AgogCore/AMath.hpp>
#include <AgogCore/AString.hpp>

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkList_Impl
  {

  //---------------------------------------------------------------------------------------
  // # Sk Params: as_new() ThisClass_
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_as_new(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * new_p = scope_p->get_this()->get_class()->new_instance();
      new_p->construct<SkList>();
      *result_pp = new_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: String() String
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      AString  str;
      const SkInstanceList & list = scope_p->this_as<SkList>();
      uint32_t length = list.get_length();

      if (length)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // List has items

        str.ensure_size_empty(length * SkInstanceList::Item_str_length_def);
        str.append('{');

        SkInstance ** items_pp     = list.get_array();
        SkInstance ** items_end_pp = items_pp + length;

        // Iterate through items and get their string representations
        while (items_pp < items_end_pp)
          {
          (*items_pp)->as_code_append(&str, SkCodeFlag__default, scope_p);

          items_pp++;

          if (items_pp < items_end_pp)
            {
            str.append(", ", 2u);
            }
          }

        str.append('}');
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Simple empty list
        str.set_cstr("{}", 2u);
        }


      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: + add(ThisClass_ list) ThisClass_
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_op_add(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance *           this_p   = scope_p->get_this();
      const SkInstanceList & lhs_list = this_p->as<SkList>();
      const SkInstanceList & rhs_list = scope_p->get_arg<SkList>(SkArg_1);
      SkInstance *           new_p    = SkList::new_instance(lhs_list.get_length() + rhs_list.get_length());

      new_p->as<SkList>().append_all(lhs_list);
      new_p->as<SkList>().append_all(rhs_list);

      *result_pp = new_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: += add_assign(ItemClass_ item) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_op_add_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *           this_p     = scope_p->get_this();
    SkInstanceList &       this_list  = this_p->as<SkList>();
    const SkInstanceList & other_list = scope_p->get_arg<SkList>(SkArg_1);

    this_list.append_all(other_list);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: List@do((ItemClass_ item) code) ThisClass_
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_do(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    // Return itself if result wanted
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }

    // Quit early if empty
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         length = list.get_length();

    if (length == 0u)
      {
      return;
      }

    // List has items
    SkClosure *   closure_p    = scope_p->get_arg_data<SkClosure>(SkArg_1);
    SkInstance ** items_pp     = list.get_array();
    SkInstance ** items_end_pp = items_pp + length;

    // Iterate through items
    do
      {
      // Ensure that the arg has an extra reference count for the call
      (*items_pp)->reference();
      closure_p->closure_method_call(items_pp, 1u, nullptr, scope_p);
      items_pp++;
      }
    while (items_pp < items_end_pp);
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: List@do_idx((ItemClass_ item Integer idx) code) ThisClass_
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_do_idx(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    // Return itself if result wanted
    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }

    // Quit early if empty
    SkInstanceList & list   = this_p->as<SkList>();
    int32_t          length = int32_t(list.get_length());

    if (length == 0)
      {
      return;
      }

    // List has items

    // Store idx arg
    SkInstance * args_pp[2];
    args_pp[1] = SkInteger::new_instance(0);
    tSkInteger * idx_p = &(args_pp[1]->as<SkInteger>());

    // Ensure that the `idx` arg has extra reference counts for the call
    args_pp[1]->reference(length);

    // Iterate through items
    SkInstance ** items_pp  = list.get_array();
    SkClosure *   closure_p = scope_p->get_arg_data<SkClosure>(SkArg_1);

    do
      {
      args_pp[0] = items_pp[*idx_p];
      // Ensure that the item arg has an extra reference count for the call
      args_pp[0]->reference();
      closure_p->closure_method_call(args_pp, 2u, nullptr, scope_p);
      (*idx_p)++;
      }
    while (*idx_p < length);

    // Clean up `idx` arg
    args_pp[1]->dereference();
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: List@all?((ItemClass_ item) Boolean test) Boolean
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_allQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Return Boolean if result wanted
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(false);
      }

    // Quit early if empty
    SkInstanceList & list   = scope_p->this_as<SkList>();
    uint32_t         length = list.get_length();

    if (length == 0u)
      {
      return;
      }

    // List has items
    SkClosure *   closure_p    = scope_p->get_arg_data<SkClosure>(SkArg_1);
    SkInstance ** items_pp     = list.get_array();
    SkInstance ** items_end_pp = items_pp + length;
    SkInstance *  result_p     = nullptr;

    // Iterate through items
    do
      {
      // Ensure that the arg has an extra reference count for the call
      (*items_pp)->reference();
      closure_p->closure_method_call(items_pp, 1u, &result_p, scope_p);

      bool stop_b = !result_p->as<SkBoolean>();
      result_p->dereference();

      if (stop_b)
        {
        // Found match so exit early
        return;
        }

      items_pp++;
      }
    while (items_pp < items_end_pp);

    if (result_pp)
      {
      (*result_pp)->as<SkBoolean>() = true;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: List@any?((ItemClass_ item) Boolean test) Boolean
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_anyQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Return Boolean if result wanted
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(false);
      }

    // Quit early if empty
    SkInstanceList & list   = scope_p->this_as<SkList>();
    uint32_t         length = list.get_length();

    if (length == 0u)
      {
      return;
      }

    // List has items
    SkClosure *   closure_p    = scope_p->get_arg_data<SkClosure>(SkArg_1);
    SkInstance ** items_pp     = list.get_array();
    SkInstance ** items_end_pp = items_pp + length;
    SkInstance *  result_p     = nullptr;

    // Iterate through items
    do
      {
      // Ensure that the arg has an extra reference count for the call
      (*items_pp)->reference();
      closure_p->closure_method_call(items_pp, 1u, &result_p, scope_p);

      bool match_b = result_p->as<SkBoolean>();
      result_p->dereference();

      if (match_b)
        {
        if (result_pp)
          {
          (*result_pp)->as<SkBoolean>() = true;
          }

        // Found match so exit early
        return;
        }

      items_pp++;
      }
    while (items_pp < items_end_pp);
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_at(Integer index) ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_at(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const SkInstanceList & list = scope_p->this_as<SkList>();
      uint32_t length = list.get_length();
      int32_t  index  = scope_p->get_arg<SkInteger>(SkArg_1);
      uint32_t idx    = index < 0 ? uint32_t(length + index) : uint32_t(index);

      if (idx < length)
        {
        SkInstance * instance_p = list.get_instances().get_at(idx);
        instance_p->reference();
        *result_pp = instance_p;
        }
      else
        {
        // Tried to access beyond range of list
        *result_pp = SkBrain::ms_nil_p;

        if (index >= 0)
          {
          SK_ERROR(a_str_format(
            "Tried to access beyond list range - given 0-based index %i, but length only %u!\n"
            "[Result will be nil.]",
            index,
            length), SkList);
          }
        else
          {
          SK_ERROR(a_str_format(
            "Tried to access before list range - given negative reverse index %i, but length only %u!\n"
            "[Result will be nil.]",
            index,
            length), SkList);
          }
        }
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: set_at(Integer index, ItemClass_ item) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_at_set(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    SkInstance *     item_p = scope_p->get_arg(SkArg_2);
    int32_t          index  = scope_p->get_arg<SkInteger>(SkArg_1);
    uint32_t         idx    = index < 0
      ? uint32_t(list.get_length() + index) : uint32_t(index);

    list.set_at(idx, *item_p);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: List@find?((ItemClass_ item) Boolean test; Integer index_found) Boolean
  // # C++ Args:  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthd_findQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Return Boolean if result wanted
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(false);
      }

    // Quit early if empty
    SkInstanceList & list   = scope_p->this_as<SkList>();
    uint32_t         length = list.get_length();

    if (length == 0u)
      {
      scope_p->set_arg(SkArg_2, SkInteger::new_instance(length));
      return;
      }

    // List has items
    SkClosure *   closure_p    = scope_p->get_arg_data<SkClosure>(SkArg_1);
    SkInstance ** items_pp     = list.get_array();
    SkInstance ** items_end_pp = items_pp + length;
    SkInstance *  result_p     = nullptr;

    // Iterate through items
    do
      {
      // Ensure that the arg has an extra reference count for the call
      (*items_pp)->reference();
      closure_p->closure_method_call(items_pp, 1u, &result_p, scope_p);

      bool match_b = result_p->as<SkBoolean>();
      result_p->dereference();

      if (match_b)
        {
        if (result_pp)
          {
          (*result_pp)->as<SkBoolean>() = true;
          }

        // Found match so exit early
        scope_p->set_arg(
          SkArg_2, SkInteger::new_instance(tSkInteger(items_pp - list.get_array())));
        return;
        }

      items_pp++;
      }
    while (items_pp < items_end_pp);

    scope_p->set_arg(SkArg_2, SkInteger::new_instance(length));
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_count() Integer
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_length(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const SkInstanceList & list = scope_p->this_as<SkList>();
      *result_pp = SkInteger::new_instance(list.get_length());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_count_after(Integer index) Integer
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_length_after(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      int32_t index = scope_p->get_arg<SkInteger>(SkArg_1);
      int32_t count = scope_p->this_as<SkList>().get_length();

      #if (SKOOKUM & SK_DEBUG)
        A_VERIFY(index <= count, a_cstr_format(" - invalid index\nGiven %d but only %d items", index, count), AErrId_invalid_index, SkList);
      #endif

      *result_pp = SkInteger::new_instance(count - index);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: empty?() Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_emptyQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstanceList & list = scope_p->this_as<SkList>();
      *result_pp = SkBoolean::new_instance(list.get_instances().is_empty());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: filled?() Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_filledQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstanceList & list = scope_p->this_as<SkList>();
      *result_pp = SkBoolean::new_instance(!list.get_instances().is_empty());
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_first() ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_first(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const SkInstanceList & list = scope_p->this_as<SkList>();
      SK_ASSERTX(!list.get_instances().is_empty(), "Tried to get first element of empty list");
      SkInstance * instance_p = list.get_instances().is_empty() ? SkBrain::ms_nil_p : list.get_instances().get_first();
      instance_p->reference();
      *result_pp = instance_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_last() ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_last(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      const SkInstanceList & list = scope_p->this_as<SkList>();
      SK_ASSERTX(!list.get_instances().is_empty(), "Tried to get last element of empty list");
      SkInstance * instance_p = list.get_instances().is_empty() ? SkBrain::ms_nil_p : list.get_instances().get_last();
      instance_p->reference();
      *result_pp = instance_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_list_same(List list) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_match_list_same(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      const SkInstanceList & this_list  = this_p->as<SkList>();
      const SkInstanceList & list       = scope_p->get_arg<SkList>(SkArg_1);
      uint32_t               list_count = list.get_length();
      SkInstance *           new_p      = SkList::new_instance(a_min(this_list.get_length(), list.get_length()));
      SkInstanceList &       new_list   = new_p->as<SkList>();

      if (list_count && this_list.get_instances().is_filled())
        {
        SkInstance *  item_p;
        SkInstance ** items_pp = list.get_array();
        SkInstance ** items_end_pp = items_pp + list_count;

        while (items_pp < items_end_pp)
          {
          item_p = *items_pp;

          if (this_list.get_instances().find_equiv(*item_p))
            {
            new_list.append(*item_p);
            }

          items_pp++;
          }
        }

      *result_pp = new_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: get_range(Integer index, Integer item_count: get_count_after(index)) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_range(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      uint32_t               index    = scope_p->get_arg<SkInteger>(SkArg_1);
      uint32_t               count    = scope_p->get_arg<SkInteger>(SkArg_2);
      SkInstance *           this_p   = scope_p->get_this();
      const SkInstanceList & list     = this_p->as<SkList>();
      SkInstance *           new_p    = SkList::new_instance(count);
      SkInstanceList &       new_list = new_p->as<SkList>();

      new_list.append_all(list, index, count);

      *result_pp = new_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: reverse(Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_reverse(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().reverse(
      scope_p->get_arg<SkInteger>(SkArg_1),
      scope_p->get_arg<SkInteger>(SkArg_2));

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: rotate_down() ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_rotate_down(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().rotate_down();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: rotate_up() ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_rotate_up(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().rotate_up();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: swap(Integer index1, Integer index2) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_swap(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().swap(
      scope_p->get_arg<SkInteger>(SkArg_1),
      scope_p->get_arg<SkInteger>(SkArg_2));

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: append(ItemClass_ item) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_append(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    SkInstance *     item_p = scope_p->get_arg(SkArg_1);

    list.append(*item_p);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: append_absent_same(ItemClass_ item) {Integer find_index} Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_append_absent_same(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstanceList & list     = scope_p->this_as<SkList>();
    SkInstance *     item_p   = scope_p->get_arg(SkArg_1);
    bool             appended = false;
    if (!list.get_instances().find(*item_p))
      {
      list.append(*item_p);
      appended = true;
      }

    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(appended);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: append_absent_list_same(ThisClass_ list) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_append_absent_list_same(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *           this_p     = scope_p->get_this();
    const SkInstanceList & list       = scope_p->get_arg<SkList>(SkArg_1);
    uint32_t               list_count = list.get_length();

    if (list_count)
      {
      SkInstanceList & this_list    = this_p->as<SkList>();
      SkInstance **    items_pp     = list.get_array();
      SkInstance **    items_end_pp = items_pp + list_count;

      while (items_pp < items_end_pp)
        {
        if (!this_list.get_instances().find(**items_pp))
          {
          this_list.append(**items_pp);
          }

        items_pp++;
        }
      }

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: append_items(ItemClass_ item, Integer count) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_append_items(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    uint32_t     count  = scope_p->get_arg<SkInteger>(SkArg_2);

    if (count)
      {
      SkInstanceList & list   = this_p->as<SkList>();
      SkInstance *     item_p = scope_p->get_arg(SkArg_1);

      list.append_all(count, *item_p);
      }

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: append_list(List{ItemClass_} list) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_append_list(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *           this_p    = scope_p->get_this();
    SkInstanceList &       this_list = this_p->as<SkList>();
    const SkInstanceList & list      = scope_p->get_arg<SkList>(SkArg_1);

    this_list.append_all(list);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: insert(ItemClass_ item, Integer index: 0) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_insert(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p    = scope_p->get_this();
    SkInstanceList & this_list = this_p->as<SkList>();
    SkInstance *     item_p    = scope_p->get_arg(SkArg_1);
    uint32_t         index     = scope_p->get_arg<SkInteger>(SkArg_2);

    this_list.insert(*item_p, index);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: insert_items(ItemClass_ item, Integer index, Integer count) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_insert_items(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p    = scope_p->get_this();
    SkInstanceList & this_list = this_p->as<SkList>();
    uint32_t         count     = scope_p->get_arg<SkInteger>(SkArg_3);

    if (count)
      {
      SkInstance * item_p = scope_p->get_arg(SkArg_1);
      uint32_t index = scope_p->get_arg<SkInteger>(SkArg_2);

      this_list.insert(*item_p, index, count);
      }

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: insert_list(List{ItemClass_} list, Integer index: 0) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_insert_list(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *           this_p    = scope_p->get_this();
    SkInstanceList &       this_list = this_p->as<SkList>();
    const SkInstanceList & list      = scope_p->get_arg<SkList>(SkArg_1);
    uint32_t               index     = scope_p->get_arg<SkInteger>(SkArg_2);

    this_list.insert_all(list, index);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: set_items(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_items_set(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_3);

    if (count)
      {
      SkInstance * item_p = scope_p->get_arg(SkArg_1);
      uint32_t     index  = scope_p->get_arg<SkInteger>(SkArg_2);

      list.set_all(*item_p, index, count);
      }

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: crop(Integer index, Integer count) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_crop(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         index  = scope_p->get_arg<SkInteger>(SkArg_1);
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_2);

    list.crop(index, count);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: empty() ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_empty(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.empty();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: pop_at(Integer index) ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_pop_at(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstanceList & list   = scope_p->this_as<SkList>();
    SkInstance *     item_p = list.get_instances().pop(scope_p->get_arg<SkInteger>(SkArg_1));

    if (result_pp)
      {
      *result_pp = item_p;
      }
    else
      {
      item_p->dereference();
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: pop_first() ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_pop_first(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstanceList & list   = scope_p->this_as<SkList>();
    SkInstance *     item_p = list.get_instances().pop();

    if (result_pp)
      {
      *result_pp = item_p;
      }
    else
      {
      item_p->dereference();
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: pop_last() ItemClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_pop_last(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstanceList & list   = scope_p->this_as<SkList>();
    SkInstance *     item_p = list.get_instances().pop_last();

    if (result_pp)
      {
      *result_pp = item_p;
      }
    else
      {
      item_p->dereference();
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: pop_range(Integer index, Integer item_count: get_count_after(index)) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_pop_range(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         index  = scope_p->get_arg<SkInteger>(SkArg_1);
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_2);

    if (result_pp)
      {
      SkInstance *      new_p    = SkList::new_instance(count);
      SkInstanceList &  new_list = new_p->as<SkList>();

      list.get_instances().pop_all(&new_list.get_instances(), index, count);

      *result_pp = new_p;
      }
    else
      {
      list.remove_all(index, count);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: pop_range_last(Integer item_count) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_pop_range_last(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_1);
    uint32_t         index  = list.get_length() - count;

    if (result_pp)
      {
      SkInstance *      new_p    = SkList::new_instance(count);
      SkInstanceList &  new_list = new_p->as<SkList>();

      list.get_instances().pop_all(&new_list.get_instances(), index, count);

      *result_pp = new_p;
      }
    else
      {
      list.remove_all(index, count);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_at(Integer index) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_at(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().pop(scope_p->get_arg<SkInteger>(SkArg_1))->dereference();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_same(
  //               ItemClass_ item,
  //               Integer index: 0,
  //               Integer item_count: get_count_after(index)
  //               ;
  //               Integer index_found
  //             ) Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_same(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    uint32_t      count       = scope_p->get_arg<SkInteger>(SkArg_3);
    tSkInteger index_found = -1;
    bool          remove_b    = false;

    if (count)
      {
      uint32_t         found_idx = 0u;
      SkInstanceList & list      = scope_p->this_as<SkList>();
      SkInstance *     item_p    = scope_p->get_arg(SkArg_1);
      uint32_t         index     = scope_p->get_arg<SkInteger>(SkArg_2);
    
      // $Revisit - CReis Could add the instance
      remove_b = list.remove(*item_p, AMatch_first_found, &found_idx, index, index + count - 1u);

      if (remove_b)
        {
        index_found = tSkInteger(found_idx);
        scope_p->set_arg(SkArg_4, SkInteger::new_instance(index_found));
        }
      }

    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(remove_b);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_first() ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_first(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().pop()->dereference();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_last() ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_last(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();

    list.get_instances().pop_last()->dereference();

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_list_same(List{ItemClass_} list) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_list_same(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *           this_p = scope_p->get_this();
    const SkInstanceList & list   = scope_p->get_arg<SkList>(SkArg_1);
    uint32_t               count  = list.get_length();

    if (count)
      {
      SkInstanceList & this_list    = this_p->as<SkList>();
      SkInstance **    items_pp     = list.get_instances().get_array();
      SkInstance **    items_end_pp = items_pp + count;

      while (items_pp < items_end_pp)
        {
        this_list.remove(**items_pp);
        items_pp++;
        }
      }

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_range(Integer index, Integer item_count: get_count_after(index)) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_range(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         index  = scope_p->get_arg<SkInteger>(SkArg_1);
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_2);

    list.remove_all(index, count);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: remove_range_last(Integer item_count) ThisClass_
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_remove_range_last(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    SkInstance *     this_p = scope_p->get_this();
    SkInstanceList & list   = this_p->as<SkList>();
    uint32_t         count  = scope_p->get_arg<SkInteger>(SkArg_1);

    list.remove_all(list.get_length() - count, count);

    if (result_pp)
      {
      this_p->reference();
      *result_pp = this_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: same_items(List{ItemClass_} list) Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_same_items(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstanceList *       this_p  = &scope_p->this_as<SkList>();
      const SkInstanceList & list    = scope_p->get_arg<SkList>(SkArg_1);
      uint32_t               count   = this_p->get_length();
      bool                   equal_b = false;

      // Ensure that the lists are of the same length.
      if (count == list.get_length())
        {
        SkInstance ** this_items_pp = this_p->get_instances().get_array();
        SkInstance ** items_pp      = list.get_instances().get_array();
        SkInstance ** items_end_pp  = items_pp + count;

        equal_b = true;

        // Iterate through items and test equivalence
        while (items_pp < items_end_pp)
          {
          if (*this_items_pp != *items_pp)
            {
            // Found non-equivalent item
            equal_b = false;
            break;  // Exit while loop
            }

          this_items_pp++;
          items_pp++;
          }
        }

      *result_pp = SkBoolean::new_instance(equal_b);
      }
    }

  //---------------------------------------------------------------------------------------
  // # Sk Params: not_same_items(List{ItemClass_} list) Boolean
  // [See script file.]
  // # C++ Args: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_not_same_items(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstanceList *       this_p  = &scope_p->this_as<SkList>();
      const SkInstanceList & list    = scope_p->get_arg<SkList>(SkArg_1);
      uint32_t               count   = this_p->get_length();
      bool                   equal_b = true;  // not equal

      // Determine if the lists are of the same length.
      if (count == list.get_length())
        {
        SkInstance ** this_items_pp = this_p->get_instances().get_array();
        SkInstance ** items_pp      = list.get_array();
        SkInstance ** items_end_pp  = items_pp + count;

        equal_b = false;  // equal

        // Iterate through items and test equivalence
        while (items_pp < items_end_pp)
          {
          if (*this_items_pp != *items_pp)
            {
            // Found non-equivalent item
            equal_b = true;  // not equal
            break;  // Exit while loop
            }

          this_items_pp++;
          items_pp++;
          }
        }

      *result_pp = SkBoolean::new_instance(equal_b);
      }
    }

  //---------------------------------------------------------------------------------------

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      // Construction
      { "as_new",                  mthd_as_new },

      // Conversion
      { "String",                  mthd_String },

      // Comparison
      { "same_items?",             mthd_same_items },
      { "not_same_items?",         mthd_not_same_items },
      //{ "equal?",                  mthd_op_equals },        // =  equals(List list) Boolean
      //{ "not_equal?",              mthd_op_not_equal },     // ~= not_equal(List list) Boolean

      // Operators
      { "add",                     mthd_op_add },
      { "add_assign",              mthd_op_add_assign },
      { "assigning",               SkList::mthd_op_assign },

      // Item Count Methods
      { "length",                  mthd_length },
      { "length_after",            mthd_length_after },
      { "empty?",                  mthd_emptyQ },
      { "filled?",                 mthd_filledQ },

      // Retrieval & Ordering
      { "at",                      mthd_at },
      { "first",                   mthd_first },
      { "last",                    mthd_last },
      { "matching_same",           mthd_match_list_same },
      { "match_list_same",         mthd_match_list_same },
      { "intersection_same",       mthd_match_list_same },
      { "intersecting_same",       mthd_match_list_same },
      { "range",                   mthd_range },
      { "reverse",                 mthd_reverse },
      { "rotate_down",             mthd_rotate_down },
      { "rotate_up",               mthd_rotate_up },
      { "swap",                    mthd_swap },

      // Assignment, Append, & Insert Methods
      { "append",                  mthd_append },
      { "append_absent_same",      mthd_append_absent_same },
      { "append_absent_list_same", mthd_append_absent_list_same },
      { "appending_absent_same",   mthd_append_absent_list_same },
      { "at_set",                  mthd_at_set },
      { "union_same",              mthd_append_absent_list_same },
      { "unioning_same",           mthd_append_absent_list_same },
      { "append_items",            mthd_append_items },
      { "append_list",             mthd_append_list },
      { "appending",               mthd_append_list },
      { "insert",                  mthd_insert },
      { "insert_items",            mthd_insert_items },
      { "insert_list",             mthd_insert_list },
      { "inserting",               mthd_insert_list },
      { "items_set",               mthd_items_set },
      // set_list(List{ItemClass_} list, Integer index: 0, Integer item_count: list.get_count())

      // Removal Methods
      { "crop",                    mthd_crop },
      { "empty",                   mthd_empty },
      { "pop_at",                  mthd_pop_at },
      { "pop_first",               mthd_pop_first },
      { "pop_last",                mthd_pop_last },
      { "pop_range",               mthd_pop_range },
      { "pop_range_last",          mthd_pop_range_last },
      { "remove_at",               mthd_remove_at },
      { "remove_same",             mthd_remove_same },
      { "remove_first",            mthd_remove_first },
      { "remove_last",             mthd_remove_last },
      { "remove_list_same",        mthd_remove_list_same },
      { "removing_same",           mthd_remove_list_same },
      { "remove_range",            mthd_remove_range },
      { "remove_range_last",       mthd_remove_range_last },

      // Iteration Methods
      { "do",                      mthd_do },
      { "do_idx",                  mthd_do_idx },
      { "all?",                    mthd_allQ },
      { "any?",                    mthd_anyQ },
      { "find?",                   mthd_findQ },

      // Search Methods - using logical (=) matching
      // append_absent(ItemClass_ item) {Integer find_index} Boolean
      // append_absent_list(List{ItemClass_} list) Integer  // union
      // append_replace(ItemClass_ item) {Integer find_index} <ItemClass_|None>
      // find(ItemClass_ item, Integer instance: 1, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} Boolean
      // find_reverse(ItemClass_ item, Integer instance: 1, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} Boolean
      // get(ItemClass_ item, Integer instance: 1, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} <ItemClass_|None>
      // get_all(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // get_list(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_  // intersection
      // get_list_all(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // remove(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} Boolean
      // remove_all(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) Integer
      // remove_list(List list, Integer index: 0, Integer item_count: get_count_after(index)) Integer
      // remove_list_all(List list, Integer index: 0, Integer item_count: get_count_after(index)) Integer
      // replace(ItemClass_ old_item, ItemClass_ new_item) {Integer find_index} <ItemClass_|None>
      // replace_all(ItemClass_ old_item, ItemClass_ new_item) <ItemClass_|None>
      // pop(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} <ItemClass_|None>
      // pop_all(ItemClass_ item, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // pop_list(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // pop_list_all(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // intersection(List list) ThisClass_

      // Methods - using equivalence (==) matching
      // get_list_same(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_  // intersection
      // find_same(ItemClass_ item, Integer instance: 1, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} Boolean
      // find_reverse_same(ItemClass_ item, Integer instance: 1, Integer index: 0, Integer item_count: get_count_after(index)) {Integer find_index} Boolean
      // replace_same(ItemClass_ old_item, ItemClass_ new_item) {Integer find_index} Boolean
      // pop_list_same(List list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_

      // Group Versions
      // !group({ItemClass_} items)
      // set_group(Integer index: 0, {ItemClass_} items)
      // append_absent_group({ItemClass_} items) Boolean
      // get_group({ItemClass_} list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // get_group_all({ItemClass_} list, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_
      // remove_group({ItemClass_} items, Integer index: 0, Integer item_count: get_count_after(index)) Integer
      // remove_group_all({ItemClass_} items, Integer index: 0, Integer item_count: get_count_after(index)) Integer
      // pop_group({ItemClass_} items, Integer index: 0, Integer item_count: get_count_after(index)) ThisClass_

      // Iteration Methods
      // apply(Code block, Integer index: 0, Integer item_count: get_count_after(index))
       
      // Script routines to convert to C++:
      //  !fill(Integer count, (Integer idx) ItemClass_ item_gen)
      //  any_but(<ItemClass_|None> excluding, Random rnd_gen: @@random; Integer index_found) ItemClass_
      //  appends(Integer count, (Integer idx) ItemClass_ item_gen) ThisClass_
      //  do_branch(+(ItemClass_ item) code)
      //  do_reverse((ItemClass_ item) code) ThisClass_
      //  reject((ItemClass_ item) Boolean test) ThisClass_
      //  select((ItemClass_ item) Boolean test) ThisClass_
      //  _do(+(ItemClass_ item) code)
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkList::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_List);

  ms_class_p->register_method_func_bulk(SkList_Impl::methods_i, A_COUNT_OF(SkList_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkList::get_class()
  {
  return ms_class_p;
  }
