// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript atomic String class
//=======================================================================================

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>

//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Converts regular string without character escape sequences into a string with escape
// sequences.
//
// #Notes
//   escape-sequence = '\' (integer-literal | printable-char)
//
//   Valid escape sequences:
//     Alert           \a  7
//     Backspace       \b  8
//     Form feed       \f  12
//     New line        \n  10
//     Carriage return \r  13
//     Horizontal tab  \t  9
//     Vertical tab    \v  11
//     Any Character (0-255) \ASCII number
//     Any Printable Character \ASCII character (other than above)
//
// #Modifiers static
// See Also  SkParser: :parse_literal_char_esc_seq()
// #Author(s) Conan Reis
void SkString::to_escape_string(
  // String to convert
  const AString & str,
  // Address of string to make escape sequenced version of 'str'.
  AString * esc_str_p,
  // If true and a new line character is encountered then replace with escaped new line
  // and also split string literal with new line:
  //   This:
  //     "First line
  //      Second line"
  //      
  //   if true becomes:
  //     "First line\n" +
  //     "Second line"
  //     
  //   if false becomes:
  //     "First line\nSecond line"
  bool break_newlines // = false
  )
  {
  uint32_t str_length = str.get_length();

  esc_str_p->ensure_size_empty(str.get_length() + 1u);

  if (str_length)
    {
    uint32_t     str_idx      = 0u;
    uint32_t     extra_count  = 0u;
    uint32_t     estr_size    = esc_str_p->get_size();
    const char * cstr_begin_p = str.as_cstr();
    const char * cstr_p       = cstr_begin_p;
    const char * cstr_end_p   = cstr_p + str_length;
    char *       ecstr_p      = esc_str_p->as_cstr_writable();

    while (cstr_p < cstr_end_p)
      {
      if (str_length + extra_count + 1u >= estr_size)
        {
        esc_str_p->set_length(str_idx + extra_count);
        esc_str_p->ensure_size(str_length + extra_count + 1u);
        ecstr_p = esc_str_p->as_cstr_writable() + str_idx + extra_count;
        }

      str_idx++;

      switch (*cstr_p)
        {
        case '\a':  // Alert
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'a';
          extra_count++;
          break;

        case '\b':  // Backspace
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'b';
          extra_count++;
          break;

        case '\f':  // Form Feed
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'f';
          extra_count++;
          break;

        case '\n':  // New line
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'n';
          extra_count++;

          if (break_newlines && (str_idx < str_length))
            {
            if (str_length + extra_count + 5u >= estr_size)
              {
              esc_str_p->set_length(str_idx + extra_count);
              esc_str_p->ensure_size(str_length + extra_count + 5u);
              ecstr_p = esc_str_p->as_cstr_writable() + str_idx + extra_count;
              }

            *ecstr_p = '"';
            ecstr_p++;
            *ecstr_p = ' ';
            ecstr_p++;
            *ecstr_p = '+';
            ecstr_p++;
            *ecstr_p = '\n';
            ecstr_p++;
            *ecstr_p = '"';
            extra_count += 3u;
            }
          break;

        case '\r':  // Carriage return
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'r';
          extra_count++;
          break;

        case '\t':  // Horizontal tab
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 't';
          extra_count++;
          break;

        case '\v':  // Vertical tab
          *ecstr_p = '\\';
          ecstr_p++;
          *ecstr_p = 'v';
          extra_count++;
          break;

        default:
          *ecstr_p = *cstr_p;
        }

      ecstr_p++;
      cstr_p++;
      }

    esc_str_p->set_length(str_length + extra_count);
    }

  }

//---------------------------------------------------------------------------------------

namespace SkString_Impl
{

//---------------------------------------------------------------------------------------
// Sk Params: String@Integer() Integer
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_Integer(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkString>().as_int());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@Real() Real
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_Real(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkReal::new_instance(scope_p->this_as<SkString>().as_float32());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String() String
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_as_string(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();
    this_p->reference();
    *result_pp = this_p;
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: as_symbol() Symbol
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_as_symbol(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkSymbol::new_instance(
      ASymbol::create(scope_p->this_as<SkString>(), ATerm_short));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: append(String str) String
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_append(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkString>().append(scope_p->get_arg<SkString>(SkArg_1));

    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkString>().append(scope_p->get_arg<SkString>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: += append(String str)
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_op_add_assign(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  if (result_pp)
    {
    SkInstance * this_p = scope_p->get_this();

    this_p->as<SkString>().append(scope_p->get_arg<SkString>(SkArg_1));

    this_p->reference();
    *result_pp = this_p;
    }
  else
    {
    scope_p->this_as<SkString>().append(scope_p->get_arg<SkString>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: = equal?(String str)
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() == scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: ~= not_equal?(String str)
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() != scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: >
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() > scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: >=
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() >= scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: <
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() < scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: <=
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkString>() <= scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@add(String str) String  / + operator
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_op_add(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    AString result_str(scope_p->this_as<SkString>());
    result_str.append(scope_p->get_arg<SkString>(SkArg_1));
    *result_pp = SkString::new_instance(result_str);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@as_crc32() Integer
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_as_crc32(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkString>().as_crc32());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@empty()
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_empty(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  scope_p->this_as<SkString>().empty();
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@enumerate(Integer increment_by: 1, Integer min_digits: 4) Integer
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_enumerate(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  tSkInteger value = scope_p->this_as<SkString>().increment(
    uint32_t(scope_p->get_arg<SkInteger>(SkArg_1)),
    uint32_t(scope_p->get_arg<SkInteger>(SkArg_2)));

  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(value);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@empty?() Boolean
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_emptyQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(
      scope_p->this_as<SkString>().is_empty());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@filled?() Boolean
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_filledQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(
      scope_p->this_as<SkString>().is_filled());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params:
//   String@find?(
//     String  str
//     Integer instance:  1
//     Integer begin_idx: 0
//     Integer end_idx:   @@length_remainder
//     Boolean case?:     true
//     ;
//     Integer find_idx
//   ) Boolean
//     
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_findQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  uint32_t find_idx;

  bool found = scope_p->this_as<SkString>().find(
    scope_p->get_arg<SkString>(SkArg_1),
    scope_p->get_arg<SkInteger>(SkArg_2),
    &find_idx,
    scope_p->get_arg<SkInteger>(SkArg_3),
    scope_p->get_arg<SkInteger>(SkArg_4),
    scope_p->get_arg<SkBoolean>(SkArg_5) ? AStrCase_sensitive : AStrCase_ignore);

  // Set `find_idx`
  scope_p->set_arg(SkArg_6, SkInteger::new_instance(found ? find_idx : -1));

  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(found);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params:
//   String@find_reverse?(
//     String  str
//     Integer instance:  1
//     Integer begin_idx: 0
//     Integer end_idx:   @@length_remainder
//     Boolean case?:     true
//     ;
//     Integer find_idx
//   ) Boolean
//     
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_find_reverseQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  uint32_t find_idx;

  bool found = scope_p->this_as<SkString>().find_reverse(
    scope_p->get_arg<SkString>(SkArg_1),
    scope_p->get_arg<SkInteger>(SkArg_2),
    &find_idx,
    scope_p->get_arg<SkInteger>(SkArg_3),
    scope_p->get_arg<SkInteger>(SkArg_4),
    scope_p->get_arg<SkBoolean>(SkArg_5) ? AStrCase_sensitive : AStrCase_ignore);

  // Set `find_idx`
  scope_p->set_arg(SkArg_6, SkInteger::new_instance(found ? find_idx : -1));

  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(found);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@length() Integer
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_length(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(
      scope_p->this_as<SkString>().get_length());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@lowercase()
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_lowercase(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  scope_p->this_as<SkString>().lowercase();
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@parse_int(Integer start_idx: 0, Integer radix: 10; Integer stop_idx) Integer
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_parse_int(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  uint32_t      stop_idx = 0u;
  tSkInteger value    = scope_p->this_as<SkString>().as_int(
    scope_p->get_arg<SkInteger>(SkArg_1),
    &stop_idx,
    scope_p->get_arg<SkInteger>(SkArg_2));

  // Set `stop_idx`
  scope_p->set_arg(SkArg_3, SkInteger::new_instance(stop_idx));

  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(value);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@parse_real(Integer start_idx: 0; Integer stop_idx) Real
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_parse_real(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  uint32_t   stop_idx = 0u;
  tSkReal value    = scope_p->this_as<SkString>().as_float32(
    scope_p->get_arg<SkInteger>(SkArg_1),
    &stop_idx);

  // Set `stop_idx`
  scope_p->set_arg(SkArg_2, SkInteger::new_instance(stop_idx));

  if (result_pp)
    {
    *result_pp = SkReal::new_instance(value);
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params: String@uppercase()
// [See script file.]
// C++ Params: See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_uppercase(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  scope_p->this_as<SkString>().uppercase();
  }

//---------------------------------------------------------------------------------------
// Array listing all the above methods
static const SkClass::MethodInitializerFunc methods_i[] =
  {
    { "Integer",           mthd_Integer},
    { "Real",              mthd_Real},
    { "String",            mthd_as_string },
    { "Symbol",            mthd_as_symbol },

    { "add",               mthd_op_add },
    { "add_assign",        mthd_op_add_assign },
    { "equal?",            mthd_op_equals },
    { "greater?",          mthd_op_greater },
    { "greater_or_equal?", mthd_op_greater_or_equal },
    { "less?",             mthd_op_less },
    { "less_or_equal?",    mthd_op_less_or_equal },
    { "not_equal?",        mthd_op_not_equal },

    { "append",            mthd_append },
    { "as_crc32",          mthd_as_crc32 },
    { "empty",             mthd_empty },
    { "empty?",            mthd_emptyQ },
    { "enumerate",         mthd_enumerate },
    { "filled?",           mthd_filledQ },
    { "find?",             mthd_findQ },
    { "find_reverse?",     mthd_find_reverseQ },
    { "length",            mthd_length },
    { "lowercase",         mthd_lowercase },
    { "parse_int",         mthd_parse_int },
    { "parse_real",        mthd_parse_real },
    { "uppercase",         mthd_uppercase },
  };

} // namespace

//---------------------------------------------------------------------------------------

void SkString::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_String);

  ms_class_p->register_method_func_bulk(SkString_Impl::methods_i, A_COUNT_OF(SkString_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkString::get_class()
  {
  return ms_class_p;
  }
