// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic Real (floating point) class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkEnum.hpp>

#include <AgogCore/AString.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkTyped.hpp>


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Finds any class data member (not including inherited members) that is the enum_class_p
// specified and it is set to enum_value.
//
// Modifiers: static
ANamed * SkEnum::get_class_data_enum_name(SkClass * enum_class_p, tSkEnum enum_value)
  {
  const SkInstanceList & class_data_values = enum_class_p->get_class_data_values();

  uint32_t data_count = class_data_values.get_length();
  if (data_count == 0u)
    {
    return nullptr;
    }

  SkInstance ** data_pp = class_data_values.get_array();
  for (uint32_t i = 0; i < data_count; ++i)
    {
    SkInstance * obj_p = data_pp[i];
    if ((obj_p->as<SkEnum>() == enum_value) && (obj_p->get_class() == enum_class_p))
      {
      return enum_class_p->get_class_data()[i];
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------

namespace SkEnum_Impl
  {

  //---------------------------------------------------------------------------------------
  // Enum@!int
  static void mthd_ctor_int(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    scope_p->get_this()->construct<SkEnum>(scope_p->get_arg<SkInteger>(SkArg_1));
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@Integer() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_Integer(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(scope_p->this_as<SkEnum>());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@as_string() AString
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance *  this_obj_p = scope_p->get_this();
      AString       str(this_obj_p->get_class()->get_name_str_dbg());
      tSkEnum       enum_value = this_obj_p->as<SkEnum>();
      ANamed *      named_p = SkEnum::get_class_data_enum_name(this_obj_p->get_class(), enum_value);
      AString       enumerator_str;

      if (named_p)
        {
        // EnumType.enumeration
        enumerator_str = named_p->get_name_str_dbg();
        }
      else
        {
        // Possibly invalid enumeration
        // EnumType.#42#
        enumerator_str.append('#');
        enumerator_str.append(AString::ctor_int(enum_value));
        enumerator_str.append('#');
        }

      str.ensure_size(str.get_length() + 1u + enumerator_str.get_length());
      str.append('.');
      str.append(enumerator_str);

      *result_pp = SkString::new_instance(str);
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@at(Integer index) ThisClass_
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthdc_op_at(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      APArray<SkInstance> & class_data = scope_p->get_this()->get_key_class()->get_class_data_values().get_instances();
      uint32_t              length     = class_data.get_length();
      tSkInteger            index      = scope_p->get_arg<SkInteger>(SkArg_1);
      uint32_t              idx        = index < 0 ? uint32_t(length + index) : uint32_t(index);

      if (idx < length)
        {
        SkInstance * instance_p = class_data.get_at(idx);
        instance_p->reference();
        *result_pp = instance_p;
        }
      else
        {
        // Tried to access beyond number of enumerations
        *result_pp = SkBrain::ms_nil_p;

        if (index >= 0)
          {
          SK_ERROR(a_str_format(
            "Tried to access beyond enumeration range - given 0-based index %i, but length only %u!\n"
            "[Result will be nil.]",
            index,
            length), SkEnum);
          }
        else
          {
          SK_ERROR(a_str_format(
            "Tried to access before enumeration range - given negative reverse index %i, but length only %u!\n"
            "[Result will be nil.]",
            index,
            length), SkEnum);
          }
        }
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@between?(Integer min Integer max) Boolean
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_betweenQ(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      tSkEnum this_num = scope_p->this_as<SkEnum>();

      *result_pp = SkBoolean::new_instance(
        (this_num >= scope_p->get_arg<SkEnum>(SkArg_1))
        && (this_num <= scope_p->get_arg<SkEnum>(SkArg_2)));
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@index() Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):  Conan Reis
  static void mthd_index(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      tSkEnum               this_value   = scope_p->this_as<SkEnum>();
      SkClass *             enum_class_p = scope_p->get_this()->get_class();
      APArray<SkInstance> & class_data   = enum_class_p->get_class_data_values().get_instances();
      uint32_t              length       = class_data.get_length();
      tSkInteger            index        = -1;
      
      // Ensure value matches index
      if ((this_value < length)
        && (this_value == class_data.get_at(this_value)->as<SkEnum>()))
        {
        // Easy - value matches index
        index = this_value;
        }
      else
        {
        // Value does not match index position so search for enumeration
        SkInstance ** enums_pp = class_data.get_array();
        SkInstance *  enum_p;

        for (uint32_t i = 0; i < length; ++i)
          {
          enum_p = enums_pp[i];

          if ((enum_p->as<SkEnum>() == this_value)
            && (enum_p->get_class() == enum_class_p))
            {
            index = i;
            break;
            }
          }
        }

      *result_pp = SkInteger::new_instance(index);
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@length() Integer
  // [See script file.]
  // C++ Args  See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  static void mthdc_length(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkInteger::new_instance(
        scope_p->get_this()->get_key_class()->get_class_data_values().get_instances().get_length());
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@max(Integer num) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_max(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * this_obj_p = scope_p->get_this();
      SkInstance * num_obj_p = scope_p->get_arg(SkArg_1);
      SkInstance * result_p = (this_obj_p->as<SkEnum>() >= num_obj_p->as<SkEnum>())
        ? this_obj_p
        : num_obj_p;

      result_p->reference();
      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // Sk Params Enum@min(Integer num) Integer
  // [See script file.]
  // C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
  // Author(s):   Conan Reis
  static void mthd_min(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      SkInstance * this_obj_p = scope_p->get_this();
      SkInstance * num_obj_p = scope_p->get_arg(SkArg_1);
      SkInstance * result_p = (this_obj_p->as<SkEnum>() <= num_obj_p->as<SkEnum>())
        ? this_obj_p
        : num_obj_p;

      result_p->reference();
      *result_pp = result_p;
      }
    }

  //---------------------------------------------------------------------------------------
  // ++
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params () Integer
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_increment(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      (this_p->as<SkEnum>())++;

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      (scope_p->this_as<SkEnum>())++;
      }
    }

  //---------------------------------------------------------------------------------------
  // ++
  // Arg         scope_p - Calling scope and working data for the invoked code - this is
  //             the path for accessing run-time information, passed arguments, data
  //             members, the call stack, etc. - see SkInvokedMethod for a description of
  //             its methods and data members.
  // Arg         result_pp - pointer to a pointer to store the instance resulting from the
  //             invocation of this expression.  If it is nullptr, then the result does not
  //             need to be returned and only side-effects are desired.  (Default nullptr)
  // Sk Params () Integer
  // Modifiers:   static
  // Author(s):   Conan Reis
  static void mthd_op_decrement(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    if (result_pp)
      {
      SkInstance * this_p = scope_p->get_this();

      (this_p->as<SkEnum>())--;

      this_p->reference();
      *result_pp = this_p;
      }
    else
      {
      (scope_p->this_as<SkEnum>())--;
      }
    }

  //---------------------------------------------------------------------------------------
  // = equal?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_equals(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() == scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // ~= not_equal?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_not_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() != scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // > greater?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_greater(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() > scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // >= greater_or_equal?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_greater_or_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() >= scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // < less?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_less(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() < scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------
  // <= less_or_equal?()
  // Sk Params (Integer num) Boolean
  // Author(s):   Conan Reis
  static void mthd_op_less_or_equal(
    SkInvokedMethod * scope_p,
    SkInstance **     result_pp
    )
    {
    // Do nothing if result not desired
    if (result_pp)
      {
      *result_pp = SkBoolean::new_instance(
        (scope_p->this_as<SkEnum>() <= scope_p->get_arg<SkEnum>(SkArg_1)));
      }
    }

  //---------------------------------------------------------------------------------------

  // Array listing all the above methods
  static const SkClass::MethodInitializerFunc methods_i[] =
    {
      { "!int",              mthd_ctor_int },

      { "Integer",           mthd_Integer },
      { "String",            mthd_String },

      { "decrement",         mthd_op_decrement },
      { "equal?",            mthd_op_equals },
      { "not_equal?",        mthd_op_not_equal },
      { "greater?",          mthd_op_greater },
      { "greater_or_equal?", mthd_op_greater_or_equal },
      { "increment",         mthd_op_increment },
      { "less?",             mthd_op_less },
      { "less_or_equal?",    mthd_op_less_or_equal },

      { "between?",          mthd_betweenQ },
      { "index",             mthd_index },
      { "max",               mthd_max },
      { "min",               mthd_min },
    };

  // Class methods
  static const SkClass::MethodInitializerFunc methods_c[] =
    {
      { "at",     mthdc_op_at },
      { "length", mthdc_length },
    };

  } // namespace

//---------------------------------------------------------------------------------------

void SkEnum::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Enum);

  ms_class_p->register_method_func_bulk(SkEnum_Impl::methods_i, A_COUNT_OF(SkEnum_Impl::methods_i), SkBindFlag_instance_no_rebind);
  ms_class_p->register_method_func_bulk(SkEnum_Impl::methods_c, A_COUNT_OF(SkEnum_Impl::methods_c), SkBindFlag_class_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkEnum::get_class()
  {
  return ms_class_p;
  }

