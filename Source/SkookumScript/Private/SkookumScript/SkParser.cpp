// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Parser and associated data-structures
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkParser.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkParser.inl>
#endif

#if (SKOOKUM & SK_CODE_IN) || defined(SK_AS_STRINGS)
  #include <SkookumScript/SkSymbol.hpp>
#endif

#if (SKOOKUM & SK_CODE_IN)

#include <math.h>    // Uses pow()
#include <AgogCore/AMath.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkConditional.hpp>
#include <SkookumScript/SkCoroutineCall.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkGroupParam.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkInvokeClosure.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkLiteral.hpp>
#include <SkookumScript/SkLiteralClosure.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkObjectId.hpp>
#include <SkookumScript/SkNone.hpp>
#include <SkookumScript/SkParameters.hpp>
#include <SkookumScript/SkRawMember.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkTypedClass.hpp>
#include <SkookumScript/SkUnaryParam.hpp>


//=======================================================================================
// Local Globals & Constants
//=======================================================================================

// Use unnamed namespace so that global namespace is not cluttered up
namespace
{

// Maximum identifier character length
const uint32_t SkParser_ident_length_max = 255u;

// Enumerated constants
enum
  {
  SkParser_integer_radix_min       =    2,  // Binary
  SkParser_integer_radix_default   =   10,  // Decimal
  SkParser_integer_radix_max       =   36,  // Base 36
  SkParser_error_str_reserve_chars = 1024
  };


} // End unnamed namespace

// If to test for disallowed access of raw data members
#define SK_PARSER_CHECK_RAW_ACCESS 0

#ifdef SK_CODE

//---------------------------------------------------------------------------------------
// Method name to operator translator
class SkMethodToOperator
  {
  public:

    SK_NEW_OPERATORS(SkMethodToOperator);

    SkMethodToOperator();

    ASymbol method_to_operator(const ASymbol & method_name) const;

  private:

    struct SkTranslate
      {
      ASymbol m_from;
      ASymbol m_to;

      void set(const ASymbol & from, const ASymbol & to) { m_from = from; m_to = to; }

      operator const ASymbol & () const { return m_from; }
      };

    APSortedLogical<SkTranslate, ASymbol> m_mthd2op;
    SkTranslate                           m_mthd2op_table[27];

  };  // SkMethodToOperator

#endif


//=======================================================================================
// SkMethodToOperator definitions
//=======================================================================================

#ifdef SK_CODE

//---------------------------------------------------------------------------------------

SkMethodToOperator::SkMethodToOperator()  
  {
  int i = 0;
  m_mthd2op_table[i++].set(ASymbol_negated,            ASymbolX_op_negated);          // -
                                                                           
  // Note: When creating any more assignments here, make sure to also support in SkRawMemberAssignment parsing code
  m_mthd2op_table[i++].set(ASymbol_assign,             ASymbolX_op_assign);           // :=
  m_mthd2op_table[i++].set(ASymbol_add,                ASymbolX_op_add);              // +
  m_mthd2op_table[i++].set(ASymbol_add_assign,         ASymbolX_op_add_assign);       // +=
  m_mthd2op_table[i++].set(ASymbol_subtract,           ASymbolX_op_subtract);         // -
  m_mthd2op_table[i++].set(ASymbol_subtract_assign,    ASymbolX_op_subtract_assign);  // -=
  m_mthd2op_table[i++].set(ASymbol_multiply,           ASymbolX_op_multiply);         // *
  m_mthd2op_table[i++].set(ASymbol_multiply_assign,    ASymbolX_op_multiply_assign);  // *=
  m_mthd2op_table[i++].set(ASymbol_divide,             ASymbolX_op_divide);           // /
  m_mthd2op_table[i++].set(ASymbol_divide_assign,      ASymbolX_op_divide_assign);    // /=

  m_mthd2op_table[i++].set(ASymbolX_equalQ,            ASymbolX_op_equals);           // =
  m_mthd2op_table[i++].set(ASymbolX_not_equalQ,        ASymbolX_op_not_equal);        // ~=
  m_mthd2op_table[i++].set(ASymbolX_greaterQ,          ASymbolX_op_greater);          // >
  m_mthd2op_table[i++].set(ASymbolX_greater_or_equalQ, ASymbolX_op_greater_or_equal); // >=
  m_mthd2op_table[i++].set(ASymbolX_lessQ,             ASymbolX_op_less);             // <
  m_mthd2op_table[i++].set(ASymbolX_less_or_equalQ,    ASymbolX_op_less_or_equal);    // <=

  m_mthd2op_table[i++].set(ASymbol_and,                ASymbol_and);                  // and
  m_mthd2op_table[i++].set(ASymbol_or,                 ASymbol_or);                   // or
  m_mthd2op_table[i++].set(ASymbol_xor,                ASymbol_xor);                  // xor
  m_mthd2op_table[i++].set(ASymbol_nand,               ASymbol_nand);                 // nand
  m_mthd2op_table[i++].set(ASymbol_nor,                ASymbol_nor);                  // nor
  m_mthd2op_table[i++].set(ASymbol_nxor,               ASymbol_nxor);                 // nxor
  m_mthd2op_table[i++].set(ASymbol_not,                ASymbol_not);                  // not

  m_mthd2op_table[i++].set(ASymbol_increment,          ASymbolX_op_increment);        // ++
  m_mthd2op_table[i++].set(ASymbol_decrement,          ASymbolX_op_decrement);        // --

  m_mthd2op_table[i++].set(ASymbol_at,                 ASymbolX_op_index);            // {}
  m_mthd2op_table[i++].set(ASymbol_at_set,             ASymbolX_op_index_set);        // {}:

  SK_ASSERTX(i == A_COUNT_OF(m_mthd2op_table), "Count must match!");

  APSortedLogical<SkTranslate, ASymbol> mthd2op(m_mthd2op_table, i, false);
  m_mthd2op = mthd2op;
  }

//---------------------------------------------------------------------------------------
// Converts method name symbol to the associated operator symbol if one exists.
// 
// Returns:
//   Operator symbol or null if 'method_name' does not have an operator symbol
//   associated with it.
//   
// Params:
//   method_name: method name symbol to convert
//   
// Modifiers: static
// Author(s): Conan Reis
ASymbol SkMethodToOperator::method_to_operator(const ASymbol & method_name) const
  {
  SkTranslate * trans_p = m_mthd2op.get(method_name);
  return trans_p ? trans_p->m_to : ASymbol::get_null();
  }

#endif // SK_CODE


//=======================================================================================
// Class Data Members
//=======================================================================================

AFlagSet32                  SkParser::ms_default_flags(Flag__default);
AString                     SkParser::ms_error_str;
SkParser::Args              SkParser::ms_def_args;
const SkMethodToOperator *  SkParser::ms_method_to_operator_p;


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// This initializes the parser class data structures - specifically the
//             reserved word list.
// Examples:   Called by SkookumScript::initialize()
// Modifiers:   static
// Author(s):   Conan Reis
void SkParser::initialize()
  {
  ms_default_flags.set(Flag_type_check);

  #ifdef SK_CODE
    if (!ms_method_to_operator_p)
      {
      ms_method_to_operator_p = new SkMethodToOperator();
      }
  #endif
  }


//---------------------------------------------------------------------------------------
void SkParser::deinitialize()
  {
  #ifdef SK_CODE
    if (ms_method_to_operator_p)
      {
      delete ms_method_to_operator_p;
      ms_method_to_operator_p = nullptr;
      }
  #endif

  ms_error_str = AString::ms_empty;
  }

// $Vital - CReis Nail down the usages of SkParser re creating data-structures & type-checking
// and whether they are co-dependent.


//---------------------------------------------------------------------------------------
void SkParser::clear_stats()
  {
  SkParameters::ms_param_count = 0u;
  }

//---------------------------------------------------------------------------------------
void SkParser::print_stats()
  {
  SkDebug::print_ide(a_str_format("\nParameters with defaults: %u\n", SkParameters::ms_param_count));
  }


//---------------------------------------------------------------------------------------
// Returns a portion of the parser string as a symbol.
// Returns:    ASymbol
// Arg         start_pos - starting position
// Arg         end_pos - ending position
// Notes:      If the symbol is longer than SkParser_ident_length_max, it is truncated.
// Author(s):   Conan Reis
ASymbol SkParser::as_symbol(
  uint32_t start_pos,
  uint32_t end_pos
  ) const
  {
  uint32_t length = a_min(end_pos - start_pos, SkParser_ident_length_max);

  // $Revisit - CReis [Lexical Check] Give parser warning if name too long rather than assert

  return ASymbol::create(m_str_ref_p->m_cstr_p + start_pos, length, ATerm_short);
  }

//---------------------------------------------------------------------------------------
// [Internal method] Ensure expression has an effect and set pos_p and args with
// appropriate (sub)expression info if it does not.
//
// Returns: true if effect present, false if not
// 
// See: ensure_exec_time()
bool SkParser::ensure_expr_effect(
  const SkExpressionBase * expr_p,
  uint32_t * pos_p,
  Args &     args
  ) const
  {
  if (expr_p == nullptr)
    {
    return true;
    }

  const SkExpressionBase * sub_expr_p = expr_p->find_expr_last_no_side_effect();

  if (sub_expr_p == nullptr)
    {
    return true;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // No side effect determined
  #if (SKOOKUM & SK_DEBUG)
    *pos_p = sub_expr_p->m_source_idx;
    args.m_start_pos = *pos_p;
  #endif

  args.m_result = (sub_expr_p->get_side_effect() == SkSideEffect_secondary)
    ? Result_warn_expr_sub_effect
    : Result_warn_expr_no_effect;

  return false;
  }

//---------------------------------------------------------------------------------------
// Sets the current parse class scope - i.e. specifies what methods / data
//             members are available.
// Arg         scope_p - class scope to use for parsing
// Author(s):   Conan Reis
void SkParser::set_class_scope(
  SkClassUnaryBase * scope_p // = nullptr
  ) const
  {
  m_context.m_obj_scope_p = scope_p
    ? scope_p
    : SkBrain::ms_object_class_p;
  }

//---------------------------------------------------------------------------------------
// Sets the current parse class scope - i.e. specifies what methods / data
//             members are available.
// Arg         scope_p - class scope to use for parsing
// Arg         scope_name - name of member/etc.
// Author(s):   Conan Reis
void SkParser::set_scope(
  SkClassUnaryBase * scope_p,    // = nullptr
  const ASymbol &    scope_name  // = ASymbol::ms_null
  ) const
  {
  set_class_scope(scope_p);
  m_context.m_scope_name = scope_name;
  }

//---------------------------------------------------------------------------------------
// Sets the current parse class scope - i.e. specifies what methods / data
//             members are available *and* frees local context variables.
// Arg         scope_p - class scope to use for parsing
// Arg         scope_name - name of member/etc.
// Author(s):   Conan Reis
void SkParser::reset_scope(
  SkClassUnaryBase * scope_p,    // = nullptr
  const ASymbol &    scope_name  // = ASymbol::ms_null
  ) const
  {
  set_scope(scope_p, scope_name);

  // Should already be empty, but this makes sure.
  m_context.free_all_locals();
  m_nest_stack.empty();
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a bind primitive.
// 
// #Notes
//   This method assumes that the variable identifier has already been parsed and it
//   determines if identifier_p is a valid SkIdentifierLocal.
//             
//   bind           = variable-ident ws binding
//   variable-ident = variable-name | ([expression ws '.' ws] data-name)
//   variable-name  = name-predicate
//   data-name      = '@' | '@@' variable-name
//   name-predicate = instance-name ['?']
//   instance-name  = lowercase {alphanumeric}
//   binding        = ':' ws expression
//
// #Author(s)   Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkBind data-structure otherwise nullptr is returned. 
  SkBind *
SkParser::parse_bind(
  // See SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of identifier receiver on entry and changed to result
  //     type on exit.
  //   m_result: Result_ok, 
  Args & args,
  // expression that identifies what to bind.  See Notes.
  SkExpressionBase * identifier_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure expression receiver is valid identifier type
  args.m_result = identifier_validate_bind(identifier_p);
  
  if (!args.is_ok())
    {
    // Hack - advance position past the colon to not confuse stringed parsing
    args.m_end_pos = args.m_start_pos + 1;
    return nullptr;
    }

  SkIdentifierLocal * ident_p = static_cast<SkIdentifierLocal *>(identifier_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse binding part
  uint32_t          start_pos       = args.m_start_pos;
  SkClassDescBase * old_type_p      = args.m_expr_type;
  bool              predicate_var_b = false;

  #if defined(A_SYMBOL_STR_DB)
    predicate_var_b = m_flags.is_set_any(Flag_type_check)
      && ident_p->is_local()
      && (ident_p->get_name().as_string().get_last() == '?');
  #endif

  // If member identifier adjust context desired type by restricted member type
  args.m_desired_type_p = predicate_var_b
    ? SkBrain::ms_boolean_class_p
    : identifier_desired_type(ident_p, old_type_p, args.m_desired_type_p);

  SkExpressionBase * bind_expr_p = parse_binding(args);

  if (!args.is_ok())
    {
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure new type is valid
  args.m_result = identifier_validate_bind_type(ident_p, old_type_p, args.m_expr_type);
  // args.m_start_pos = start_pos;

  if (!args.is_ok())
    {
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make Bind expression
  SkBind * expr_p = nullptr;

  if (bind_expr_p)
    {
    if (ident_p->get_type() == SkExprType_identifier_local)
      {
      SkTypedNameIndexed * var_p = m_context.find_local_variable(ident_p->get_name());
      SK_ASSERTX(var_p, "Must exist at this point.");
      var_p->m_has_been_bound = true;
      }
    expr_p = SK_NEW(SkBind)(ident_p, bind_expr_p);

    SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
    }

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a binding.
//
// # Returns:
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically
//   allocated expression otherwise it returns nullptr.
//
// args: see SkParser::Args, specific considerations below:
//   m_result:
//     Result_ok, Result_err_expected_binding, Result_err_unexpected_eof,
//     or any result returned from parsing an expression.
//
// Notes:
//   binding = ':' ws expression
SkExpressionBase * SkParser::parse_binding(
  Args & args // = ms_def_args.reset()  Start pos 0 with default flags
  ) const
  {
  uint32_t start_pos = args.m_start_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure there are enough characters to continue
  // && Ensure correct bind operator symbol
  if ((m_str_ref_p->m_length <= start_pos)
    || (m_str_ref_p->m_cstr_p[start_pos] != ':'))
    {
    args.m_result  = Result_err_expected_binding;
    args.m_end_pos = start_pos;
    return nullptr;
    }

  args.m_start_pos++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse expression being bound
  // Pass on desired type
  return parse_expression(args);
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a case (also known as switch, select, etc.) expression.
// [Assumes 'case' token has already been parsed.]
//
// #Notes
//   case       = 'case' ws expression {ws expression ws code-block}1+ 
//                 [ws 'else' ws code-block]
//   code-block = '[' ws [statement {wsr statement} ws] ']'
//
//   Example:
//
//     case compare_expr
//       test_expr1 [ clause1 ]
//       test_expr2 [ clause2 ]
//       else       [ else_clause ]
//
// #See Also  parse_conditional_tail()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // data-structure otherwise nullptr is returned.
  SkCase *
SkParser::parse_case_tail(
  // see SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_context_case_compare, Result_err_typecheck_case,
  //     Result_err_expected_clause_block, or any other expression warning or error.
  Args & args // = ms_def_args.reset()
  ) const
  {
  // Nested Structures

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Structure to ensure proper cleanup
  struct Nested
    {
    // Data Members

      const SkParser * m_parser_p;
      Args * m_args_p;
      SkCase * m_case_p;
      SkExpressionBase * m_test_p;
      bool m_parsing_test;
      SkClassUnion m_result_type;
      // Type changes made during clause block
      tSkTypedNamesIndexed m_alt_context;
      // Ending position of previous {test-clause} pair and starting position of potential
      // next {test-clause} pair and used for backtrack if following expression mistakenly
      // parsed as a test expression.
      uint32_t m_pair_end_pos;

    // Methods

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Nested(const SkParser * parser_p, Args * args_p, SkExpressionBase * compare_expr_p) :
        m_parser_p(parser_p),
        m_args_p(args_p),
        m_case_p(nullptr),
        m_test_p(nullptr),
        m_parsing_test(false)
        {
        if (args_p->is_struct_wanted())
          {
          m_case_p = SK_NEW(SkCase)();
          m_case_p->m_compare_expr_p = compare_expr_p;
          SKDEBUG_SET_CHAR_POS(m_case_p, args_p->m_start_pos);
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Nest type context in case a backtrack is performed
      void pre_test_expr()
        {
        m_parser_p->m_context.nest_locals(SkNestReason_exploratory);
        m_parsing_test = true;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Test expression was valid so accept any type changes
      void post_test_expr()
        {
        m_parser_p->m_context.accept_nest();
        m_parsing_test = false;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // May have read following expression too eagerly - ignore it
      void ignore_test_expr()
        {
        // Undo any type changes made while attempting to parse test expression
        if (m_parsing_test)
          {
          m_parser_p->m_context.unnest_locals(SkUnnestAction_reject);
          m_parsing_test = false;
          }

        // Get rid of any unused test expression
        if (m_test_p)
          {
          delete m_test_p;
          m_test_p = nullptr;
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Try to backtrack if expression following 1+ valid case [test-clause} pairs
      // had an error or a valid expression that wasn't followed by a code block (which is
      // a *potentially error prone* mechanism to signal the end of a case.
      SkCase * backtrack()
        {
        ignore_test_expr();

        // Manage type information
        if (m_parser_p->m_flags.is_set_any(Flag_type_check))
          {
          // There is no 'else' clause so:
          
          // Merge original type context since that is a possible path
          m_parser_p->m_context.merge(&m_alt_context);
          // Merge in nil as a possible result.
          m_result_type.merge_class(*SkNone::get_class());
          }

        m_args_p->m_end_pos = m_pair_end_pos;
        m_args_p->m_result  = Result_ok;

        return m_case_p;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Perform clean-up and ensure proper resulting context state
      ~Nested()
        {
        ignore_test_expr();

        if (m_args_p->is_ok())
          {
          // Manage type information
          if (m_parser_p->m_flags.is_set_any(Flag_type_check))
            {
            // Update variables whose types changed during the alternate code paths.
            m_parser_p->m_context.change_variable_types(m_alt_context);

            // Store result type
            m_args_p->m_expr_type = m_result_type.is_trivial()
              ? static_cast<SkClassDescBase *>(m_result_type.get_common_class())
              : SkClassUnion::get_or_create(m_result_type);
            }
          }
        else
          {
          if (m_case_p)
            {
            delete m_case_p;
            }
          }

        m_alt_context.free_all();
        }
    };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t start_pos = args.m_start_pos;
  uint32_t pos       = start_pos;

  // Eat {whitespace}
  args.m_result = parse_ws_any(pos, &args.m_end_pos);

  if (!args.is_ok())
    {
    return nullptr;
    }

  pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for comparison expression
  SkClassDescBase * desired_type_p = args.m_desired_type_p;

  args.m_start_pos = pos;
  args.m_desired_type_p = nullptr;
  SkExpressionBase * compare_p = parse_expression(args, SkInvokeTime_immediate);

  if (!args.is_ok())
    {
    return nullptr;
    }

  pos = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that the result type of the comparison expression has an equals
  // operator [=] and determine what type it takes as an operand.
  SkClassDescBase * compare_expr_type_p = args.m_expr_type;
  SkClassDescBase * compare_op_type_p   = SkBrain::ms_object_class_p;
  bool              class_equal         = false;
          
  if (m_flags.is_set_any(Flag_type_check))
    {
    SkMethodBase * equals_p = compare_expr_type_p->find_method_inherited(ASymbolX_equalQ, &class_equal);

    // The only class equals?() method is for Class objects that take a <Object> as an
    // operand so fail if a class method and not a Class.
    if (class_equal && !compare_expr_type_p->is_metaclass())
      {
      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The `case` comparison expression is the class type `%s` which has no equals operator `=`.\n"
          "This is needed by `case` to compare with test expressions.\n"
          "Add an `=` operator (`equal?()` method) or use `if` instead of `case`.",
          compare_expr_type_p->as_code().as_cstr());
      #endif

      args.m_result = Result_err_context_case_compare;

      return nullptr;
      }

    // Get operand type
    compare_op_type_p =
      equals_p->get_params().m_params.get_first()->get_expected_type()->as_finalized_generic(*compare_expr_type_p);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup structures
  Nested nested(this, &args, compare_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse {test expression - clause block} pairs

  uint32_t length = m_str_ref_p->m_length;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  // Clause info
  uint32_t clause_count = 0u;
  bool     is_else_b    = false;

  SkExpressionBase * clause_p = nullptr;

  APCompactArrayFree<SkClause> * clauses_p = nested.m_case_p ? &nested.m_case_p->m_clauses : nullptr;

  do
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Note that nested.~Nested() does the cleanup.
      return nullptr;
      }

    pos = args.m_end_pos;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for test expression or "else"

    // Look for an 'else' clause and ensure it is not some other identifier that has the
    // root word 'else'
    if (((length - pos) >= 4u)
      && (cstr_a[pos]      == 'e')
      && (cstr_a[pos + 1u] == 'l')
      && (cstr_a[pos + 2u] == 's')
      && (cstr_a[pos + 3u] == 'e')
      && ms_char_match_table[ACharMatch_not_identifier][uint8_t(cstr_a[pos + 4u])])
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Found an else clause
      pos += 4u;
      is_else_b   = true;

      if (clause_count == 0u)
        {
        args.m_result  = Result_err_unexpected_else;
        args.m_end_pos = pos;
        // Note that nested.~Nested() does the cleanup.
        return nullptr;
        }
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // If an else clause was not found look for a regular test expression

      nested.pre_test_expr();
      args.m_start_pos = pos;
      args.m_desired_type_p = SkBrain::ms_boolean_class_p;
      nested.m_test_p = parse_expression(args, SkInvokeTime_immediate);

      if (!args.is_ok())
        {
        // If at least 1+ {test-clause} pairs try to backtrack and accept what is valid
        // so far.  Note that nested.~Nested() does the cleanup.
        return (clause_count == 0u)
          ? nullptr
          : nested.backtrack();
        }

      pos = args.m_end_pos;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Only assume expression is part of a {test - clause} pair if it looks like it is
    // followed by a code block.
      
    // Eat {whitespace}
    args.m_result = parse_ws_any(pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Note that nested.~Nested() does the cleanup.
      return nullptr;
      }

    pos = args.m_end_pos;


    // Look for code block
    if (cstr_a[pos] != '[')
      {
      args.m_result = Result_err_expected_clause_block;
      // If not an 'else' block and at least 1+ {test-clause} pairs try to backtrack and
      // accept what is valid so far.  Note that nested.~Nested() does the cleanup.
      // $Note - CReis This *may* be intentended as another test expression for the
      // case and the clause block has a typo.
      // $Revisit - CReis Expression could be checked if it has a Boolean result type
      // meaning it is more likely (though still not certain) to be a clause block typo.
      return (is_else_b || (clause_count == 0u))
        ? nullptr
        : nested.backtrack();
      }

    if (!is_else_b)
      {
      // Looks like a {test - clause} pair so commit to last expression being test expression
      nested.post_test_expr();

      // Ensure that the result type of the clause expression is compatible with the
      // comparison expression equals operator [=] operand type
      if (m_flags.is_set_any(Flag_type_check)
        && !args.m_expr_type->is_class_type(compare_op_type_p))
        {
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The class type of this `case` test expression is %s\n"
            "which is not compatible as %s expected by the operand\n"
            "to the equals operator '=' of the comparison expression which is %s.",
            args.m_expr_type->get_scope_desc().as_cstr(),
            compare_op_type_p->get_scope_desc().as_cstr(),
            compare_expr_type_p->get_scope_desc().as_cstr());
        #endif

        args.m_result = Result_err_typecheck_case;
        // Note that nested.~Nested() does the cleanup.
        return nullptr;
        }
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse clause block
    if (m_flags.is_set_any(Flag_type_check))
      {
      // Nest context for alternate code path.
      m_context.nest_locals(SkNestReason_exploratory);
      }

    args.m_start_pos = pos;
    args.m_desired_type_p = desired_type_p;
    clause_p = parse_code_block_optimized(args);

    if (!args.is_ok())
      {
      m_context.unnest_locals(SkUnnestAction_reject);
      // Note that nested.~Nested() does additional cleanup.
      return nullptr;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Accumulate {test expression - clause block} pair
    pos = args.m_end_pos;
    nested.m_pair_end_pos = pos;
    clause_count++;

    if (clauses_p)
      {
      clauses_p->append(*SK_NEW(SkClause)(nested.m_test_p, clause_p));
      nested.m_test_p = nullptr;
      }

    // Manage type information
    if (m_flags.is_set_any(Flag_type_check))
      {
      // Accumulate result type of case
      nested.m_result_type.merge_class(*args.m_expr_type);

      // Merge context changes into alternate context
      if (nested.m_alt_context.is_filled() || m_context.is_locals())
        {
        m_context.merge_locals(&nested.m_alt_context, clause_count == 1u);
        }

      // Remove nested context for alternate code path.
      m_context.unnest_locals(SkUnnestAction_accept);
      }
    }
  while (!is_else_b);
  // Exit if successful 'else' clause all other exits are via errors or backtracks.

  // Note that nested.~Nested() does the cleanup.
  return nested.m_case_p;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to parse method/coroutine annotations
// and populates annotations_p
SkParser::eResult SkParser::parse_annotations(uint32_t start_pos, uint32_t * end_pos_p, Annotations * annotations_p, eSkAnnotationTarget target) const
  {
  eResult result = Result_ok;
  uint32_t annotation_flags = 0;
  uint32_t pos = start_pos;
  char * cstr_a = m_str_ref_p->m_cstr_p;
  uint32_t length = m_str_ref_p->m_length;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure enough characters

  if (length < pos)
    {
    result = Result_err_unexpected_eof;
    goto done;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Loop parsing annotations until done
  while (cstr_a[pos] == '&')
    {
    ++pos;

    // Find last A-Z, a-z, _, 0-9, or European character + optional '?'
    // [First char was already checked so skip]
    uint32_t end_pos = pos;
    if (!find(ACharMatch_not_identifier, 1u, &end_pos, pos))
      {
      end_pos = m_str_ref_p->m_length;
      }

    // Create symbol from the name of the annotation
    uint32_t name_length = end_pos - pos;
    if (name_length > ASymbol_length_max)
      {
      result = Result_err_size_identifier;
      goto done;
      }
    ASymbol name(as_symbol(pos, end_pos));
    pos = end_pos;

    // HACK annotations are currently hardwired, will later be data driven
    AString value;
    switch (name.get_id())
      {
      case ASymbolId_raw:
        // This annotation is only allowed on data members
        if (target != SkAnnotationTarget_instance_data
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (annotation_flags & SkAnnotation_raw)
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        annotation_flags |= SkAnnotation_raw;

        // Check for optional name argument
        if (cstr_a[pos] == '(')
          {
          // White space is allowed
          if ((result = parse_ws_any(++pos, &pos)) != Result_ok)
            {
            goto done;
            }

          // Get string
          if ((result = parse_literal_string(pos, &pos, &annotations_p->m_name)) != Result_ok)
            {
            goto done;
            }

          // White space is allowed
          if ((result = parse_ws_any(pos, &pos)) != Result_ok)
            {
            goto done;
            }

          // Closing parenthesis must follow
          if (cstr_a[pos] != ')')
            {
            result = Result_err_expected_annotation_arg;
            goto done;
            }

          ++pos;
          }

        break;

      case ASymbolId_name:
        // This annotation is only allowed on classes
        if (target != SkAnnotationTarget_class
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (!annotations_p->m_name.is_empty())
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        // Open parenthesis must follow a &name annotation
        if (cstr_a[pos] != '(')
          {
          result = Result_err_expected_annotation_arg;
          goto done;
          }

        // White space is allowed
        if ((result = parse_ws_any(++pos, &pos)) != Result_ok)
          {
          goto done;
          }

        // Get string
        if ((result = parse_literal_string(pos, &pos, &annotations_p->m_name)) != Result_ok)
          {
          goto done;
          }

        // White space is allowed
        if ((result = parse_ws_any(pos, &pos)) != Result_ok)
          {
          goto done;
          }

        // Closing parenthesis must follow
        if (cstr_a[pos] != ')')
          {
          result = Result_err_expected_annotation_arg;
          goto done;
          }

        ++pos;
        break;

      case ASymbolId_aka:
        // This annotation is only allowed on invokables
        if (target != SkAnnotationTarget_invokable
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Open parenthesis must follow a &aka annotation
        if (cstr_a[pos] != '(')
          {
          result = Result_err_expected_annotation_arg;
          goto done;
          }

        // White space is allowed
        if ((result = parse_ws_any(++pos, &pos)) != Result_ok)
          {
          goto done;
          }

        // Get string
        if ((result = parse_literal_string(pos, &pos, &value)) != Result_ok)
          {
          goto done;
          }
        annotations_p->m_akas.append(value);

        // White space is allowed
        if ((result = parse_ws_any(pos, &pos)) != Result_ok)
          {
          goto done;
          }

        // Closing parenthesis must follow
        if (cstr_a[pos] != ')')
          {
          result = Result_err_expected_annotation_arg;
          goto done;
          }
        ++pos;
        break;

      case ASymbolId_reflected_cpp:
        // This annotation is only allowed on classes
        if (target != SkAnnotationTarget_class
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (annotation_flags & SkAnnotation_reflected_cpp)
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        annotation_flags |= SkAnnotation_reflected_cpp;
        break;

      case ASymbolId_reflected_data:
        // This annotation is only allowed on classes
        if (target != SkAnnotationTarget_class
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (annotation_flags & SkAnnotation_reflected_data)
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        annotation_flags |= SkAnnotation_reflected_data;
        break;

      case ASymbolId_invokable:
        // This annotation is only allowed on classes
        if (target != SkAnnotationTarget_class
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (annotation_flags & SkAnnotation_invokable)
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        annotation_flags |= SkAnnotation_invokable;
        break;

      case ASymbolId_blueprint:
        // This annotation is only allowed on invokables
        if (target != SkAnnotationTarget_invokable
         && target != SkAnnotationTarget__any)
          {
          result = Result_err_context_annotation_invalid;
          goto done;
          }

        // Must not be used twice
        if (annotation_flags & SkAnnotation_ue4_blueprint)
          {
          result = Result_err_context_annotation_duplicate;
          goto done;
          }

        annotation_flags |= SkAnnotation_ue4_blueprint;
        break;

      default:
        result = Result_err_context_annotation_unknown;
        goto done;

      }

    // Eat whitespace after annotation to get ready for next annotation or parameters
    result = parse_ws_any(pos, &pos);
    if (result != Result_ok)
      {
      goto done;
      }
    }

done:
  // Tell caller where we left off
  *end_pos_p = pos;

  // Pass back annotation flags
  annotations_p->m_flags = annotation_flags;

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a class name and then
//             sets 'class_pp' to a pointer to an existing class with that name.
// Returns:    Result_ok, Result_err_expected_class, Result_err_context_non_class, or
//             Result_err_unexpected_eof 
// Arg         start_pos - character position to begin lexical analysis.
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.
// Arg         class_pp - Pointer to an address to store a pointer to the class
//             specified by the parameter specifier.  It is not written to if it is set
//             to nullptr or if the result is not Result_ok.  (Default nullptr)
// Examples:   if (parse.parse_class(11u, &end_pos, &class_name) == Result_ok)
// See:        parse_name_class(), parse_name_instance(), parse_name_coroutine()
// Notes:      class        = class-name
//             class-name   = uppercase {alphanumeric}
//             alphanumeric = uppercase | lowercase | digit | '_'
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_class(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  SkClass ** class_pp   // = nullptr
  ) const
  {
  // Get class name
  ASymbol class_name;
  eResult result = parse_name_class(start_pos, end_pos_p, &class_name);

  if (result != Result_ok)
    {
    return result;
    }

  // Get class
  SkClass * class_p = SkBrain::get_class(class_name);
    
  if (class_p == nullptr)
    {
    return Result_err_context_non_class;
    }

  if (class_pp)
    {
    *class_pp = class_p;
    }
  
  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a class instance
// (SkClass or SkTypedClass)
//
// Returns: Result_ok or error
//
// Params:
//   start_pos: character position to begin lexical analysis.
//   end_pos_p:
//     character position that lexical analysis stopped at.  If it is set to nullptr, it
//     is not written to.
//   type_pp:
//     Pointer to an address to store a pointer to the parsed class type.  It is not
//     written to if it is set to nullptr or if the result is not Result_ok.
//   item_type_b_p: Address to store whether an item class type was specified (true) or
//     not (false).  It is not modified if it is set to nullptr or if the result is not
//     Result_ok.
//
// Notes:      
//   class-instance  = class | list-class | invoke-class
//   class           = class-name
//   list-class      = List '{' ws [class-desc ws] '}'
//   invoke-class    = ['_' | '+'] parameters
//   class-name      = uppercase {alphanumeric}
//   alphanumeric    = uppercase | lowercase | digit | '_'
//
// See: parse_class(), parse_class_meta(), parse_class_union(), parse_name_class()
SkParser::eResult SkParser::parse_class_instance(
  uint32_t            start_pos,    // = 0u
  uint32_t *          end_pos_p,    // = nullptr
  SkClassUnaryBase ** class_pp,     // = nullptr
  bool *              item_type_b_p // = nullptr
  ) const
  {
  // Next possible error
  eResult result = Result_err_unexpected_eof;

  uint32_t pos         = start_pos;
  bool     item_type_b = false;

  if (m_str_ref_p->m_length > pos)
    {
    // Next possible error
    result = Result_err_expected_class_instance;

    SkClass * class_p = nullptr;
    const char * cstr_a = m_str_ref_p->m_cstr_p;
    bool look_for_invokable_params = true;
    bool require_invokable_params = false;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if it starts with uppercase - this is a class name
    if (ms_is_uppercase[uint8_t(cstr_a[pos])])  
      {
      result = parse_class(start_pos, &pos, &class_p);

      if (result == Result_ok)
        {
        // Check for List class with item types
        if (class_p->is_class(*SkBrain::ms_list_class_p))
          {
          if (cstr_a[pos] == '{')
            {
            result = parse_class_list_items(
              pos + 1u, &pos, class_p, reinterpret_cast<SkTypedClass **>(class_pp), &item_type_b);
            }
          else if (class_pp)
            {
            // Switch "List" with "List{Object}
            *class_pp = SkTypedClass::get_or_create(class_p, SkBrain::ms_object_class_p);
            }
          }
        else if (class_pp)
          {
          *class_pp = static_cast<SkClassUnaryBase *>(class_p);
          }
        }

      // If class is annotated with &invokable, require a signature following the class name
      look_for_invokable_params = require_invokable_params = (result == Result_ok && (class_p->get_annotation_flags() & SkAnnotation_invokable));
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Check for invokable class
    if (look_for_invokable_params)
      {
      // Allow for preceding white space if following a class name
      if (require_invokable_params)
        {
        result = parse_ws_any(pos, &pos);
        if (result != Result_ok)
          {
          if (end_pos_p) *end_pos_p = pos;
          return result;
          }

        // Since params are required, this is our new default error
        result = Result_err_expected_class_params;
        }

      eSkInvokeTime invoke_type = SkInvokeTime_immediate;
      switch (cstr_a[pos])
        {
        case '_':
          pos++;
          invoke_type = SkInvokeTime_durational;
          // Allow fall-through

        case '+':
          if (invoke_type == SkInvokeTime_immediate)
            {
            pos++;
            invoke_type = SkInvokeTime_any;
            }
          // Allow fall-through

        case '(':
          {
          m_context.nest_locals(SkNestReason_exploratory);

          SkParameters params;
          Args         args(pos);

          parse_parameters(
            args,
            class_pp ? &params : nullptr,
            (invoke_type == SkInvokeTime_durational) ? ParamFlag_coroutine : ParamFlag__none,
            0);

          result = args.m_result;
          pos    = args.m_end_pos;

          m_context.unnest_locals(SkUnnestAction_reject);

          if ((result == Result_ok) && class_pp)
            {
            // $Revisit - CReis Don't allow invokable classes with both generics and default types
            // since the expression as_copy() methods aren't all available yet.
            if (params.is_defaulted() && params.is_generic()) 
              {
              result = Result_err_unimplemented;

              #if defined(SK_AS_STRINGS)
                ms_error_str.ensure_size_empty(500u);
                ms_error_str.append(
                  "Invokable class types with both generics and default values aren't yet supported.\n"
                  "[Use one or the other for now.]");
              #endif
              }

            *class_pp = SkInvokableClass::get_or_create(
              class_p ? class_p : SkBrain::ms_closure_class_p,
              SkParameters::get_or_create(&params),
              invoke_type);
            }

          break;
          }
        }
      }
    }

  if ((result == Result_ok) && item_type_b_p)
    {
    *item_type_b_p = item_type_b;
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a class descriptor.
//
// Returns:
//   Result_ok, Result_err_expected_class, Result_err_context_non_class, or
//   Result_err_unexpected_eof
//
// Params:
//   start_pos: character position to begin lexical analysis.
//   end_pos_p:
//     character position that lexical analysis stopped at.  If it is set to nullptr, it
//     is not written to.
//   type_pp:
//     Pointer to an address to store a pointer to the parsed class type.  It is not
//     written to if it is set to nullptr or if the result is not Result_ok.
//
// Notes:      
//   class-desc      = class-unary | class-union
//   class-unary     = class-instance | meta-class
//   class-instance  = class | list-class | invoke-class
//   class           = class-name
//   meta-class      = '<' class-name '>'
//   class-union     = '<' class-unary {'|' class-unary}1+ '>'
//   list-class      = List '{' ws [class-desc ws] '}'
//   invoke-class    = ['_' | '+'] parameters
//   class-name      = uppercase {alphanumeric}
//   alphanumeric    = uppercase | lowercase | digit | '_'
//
// See: parse_class(), parse_class_meta(), parse_class_union(), parse_name_class()
SkParser::eResult SkParser::parse_class_desc(
  uint32_t           start_pos, // = 0u
  uint32_t *         end_pos_p, // = nullptr
  SkClassDescBase ** type_pp    // = nullptr
  ) const
  {
  uint32_t pos = start_pos;

  // Next possible error
  eResult result = Result_err_unexpected_eof;

  if (m_str_ref_p->m_length > pos)
    {
    // Next possible error
    result = Result_err_expected_class_desc;

    char * cstr_a = m_str_ref_p->m_cstr_p;

    if (cstr_a[pos] == '<')
      {
      // Could be a metaclass or a class union.
      uint32_t end_pos = pos + 1u;

      // Set pos past the last A-Z, a-z, _, 0-9, or European character
      find(ACharMatch_not_identifier, 1u, &end_pos, pos + 1u);

      // If it is a metaclass the end of identifier character must be '>' 
      result = (cstr_a[end_pos] == '>')
        ? parse_class_meta(start_pos, &pos, reinterpret_cast<SkMetaClass **>(type_pp))
        : parse_class_union(start_pos, &pos, reinterpret_cast<SkClassUnion **>(type_pp));
      }
    else
      {
      result = parse_class_instance(start_pos, &pos, reinterpret_cast<SkClassUnaryBase **>(type_pp));

      if ((pos == start_pos) && (result == Result_err_expected_class_instance))
        {
        result = Result_err_expected_class_desc; 
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a metaclass descriptor.
// Returns:    Result_ok, Result_err_expected_class, Result_err_context_non_class, or
//             Result_err_unexpected_eof 
// Arg         start_pos - character position to begin lexical analysis.
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it is
//             set to nullptr, it is not written to.
// Arg         mclass_pp - Pointer to an address to store a pointer to the parsed
//             metaclass.  It is not written to if it is set to nullptr or if the result is
//             not Result_ok.
// See:        parse_class(), parse_name_class()
// Notes:      
//               meta-class   = '<' class-name '>'
//               class-name   = uppercase {alphanumeric}
//               alphanumeric = uppercase | lowercase | digit | '_'
//
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_class_meta(
  uint32_t       start_pos, // = 0u
  uint32_t *     end_pos_p, // = nullptr
  SkMetaClass ** mclass_pp  // = nullptr
  ) const
  {
  uint32_t pos = start_pos;

  // Next possible error
  eResult result = Result_err_unexpected_eof;

  if (m_str_ref_p->m_length - pos >= 3u)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    // Next possible error
    result = Result_err_expected_class_meta;

    if (cstr_a[pos] == '<')
      {
      SkClass * class_p;

      result = parse_class(pos + 1u, &pos, &class_p);

      if (result == Result_ok)
        {
        // Next possible error
        result = Result_err_expected_class_meta_end;

        if (cstr_a[pos] == '>')
          {
          pos++;
          result = Result_ok;

          if (mclass_pp)
            {
            *mclass_pp = &class_p->get_metaclass();
            }
          }
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create the tail end of a List typed item
// class descriptor.
// 
// Returns:
//   Result_ok, Result_err_expected_class, Result_err_context_non_class, or
//   Result_err_unexpected_eof 
// 
// Params:
//   start_pos: character position to begin lexical analysis.
//   end_pos_p: character position that lexical analysis stopped at.  If it is
//             set to nullptr, it is not written to.
//   tclass_pp: Pointer to an address to store a pointer to the parsed typed
//             class.  It is not written to if it is set to nullptr or if the result is not
//             Result_ok.
//   item_type_b_p: Address to store whether an item class type was specified
//             (true) or not (false).  It is not modified if it is set to nullptr or if the
//             result is not Result_ok.
// 
// See:
//   parse_class_desc(), parse_class(), parse_class_meta(), parse_class_union(),
//   parse_name_class()
//   
// Notes:      
//   list-class = List '{' ws [class-desc ws] '}'
//                         ^ starts here
// Author(s): Conan Reis
SkParser::eResult SkParser::parse_class_list_items(
  uint32_t        start_pos,    // = 0u
  uint32_t *      end_pos_p,    // = nullptr
  SkClass *       class_p,      // = nullptr
  SkTypedClass ** tclass_pp,    // = nullptr
  bool *          item_type_b_p // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  uint32_t pos;
  eResult  result = parse_ws_any(start_pos, &pos);

  if (result != Result_ok)
    {
    *end_pos_p = pos;

    return result;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional item type descriptor
  uint32_t          item_pos;
  bool              item_b      = true;
  SkClassDescBase * item_type_p = SkBrain::ms_object_class_p;

  result = parse_class_desc(pos, &item_pos, tclass_pp ? &item_type_p : nullptr);

  // Determine if parse advanced
  if (item_pos == pos)
    {
    item_b = false;
    result = Result_ok;
    }
  else
    {
    if (result != Result_ok)
      {
      *end_pos_p = pos;

      return result;
      }
    }

  pos = item_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (item_b)
    {
    // Eat {whitespace}
    result = parse_ws_any(pos, &pos);

    if (result != Result_ok)
      {
      *end_pos_p = pos;

      return result;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (m_str_ref_p->m_cstr_p[pos] != '}')
    {
    *end_pos_p = pos;

    return Result_err_expected_class_list_end;
    }

  pos++;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Store requested results
  if (tclass_pp)
    {
    *tclass_pp = SkTypedClass::get_or_create(class_p, item_type_p);
    }

  if (item_type_b_p)
    {
    *item_type_b_p = item_b;
    }

  *end_pos_p = pos;

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a class union descriptor.
//
// Returns:
//   Result_ok, Result_err_expected_class, Result_err_context_non_class, or
//   Result_err_unexpected_eof 
//
// Params:
//   start_pos: character position to begin lexical analysis.
//   end_pos_p:
//     character position that lexical analysis stopped at.  If it is set to nullptr, it
//     is not written to.
//   type_pp:
//     Pointer to an address to store a pointer to the parsed class type.  It is not
//     written to if it is set to nullptr or if the result is not Result_ok.
//
// Notes:      
//   class-union     = '<' class-unary {'|' class-unary}1+ '>'
//   class-unary     = class-instance | meta-class
//   class-instance  = class | list-class | invoke-class
//   class           = class-name
//   meta-class      = '<' class-name '>'
//   list-class      = List '{' ws [class-desc ws] '}'
//   invoke-class    = ['_' | '+'] parameters
//   class-name      = uppercase {alphanumeric}
//   alphanumeric    = uppercase | lowercase | digit | '_'
//
// See: parse_class(), parse_name_class()
SkParser::eResult SkParser::parse_class_union(
  uint32_t        start_pos, // = 0u
  uint32_t *      end_pos_p, // = nullptr
  SkClassUnion ** type_pp    // = nullptr
  ) const
  {
  uint32_t pos = start_pos;

  // Next possible error
  eResult result = Result_err_unexpected_eof;

  if (m_str_ref_p->m_length - pos >= 3u)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for start of class union

    // Next possible error
    result = Result_err_expected_class_union;

    if (cstr_a[pos] == '<')
      {
      SkClassUnion        class_union;
      SkClassUnaryBase *  class_p     = nullptr;
      SkClassUnaryBase ** class_pp    = type_pp ? &class_p : nullptr;
      uint32_t            class_count = 0u;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Iterate through unioned classes
      do
        {
        pos++;

        result = (cstr_a[pos] == '<')
          ? parse_class_meta(pos, &pos, reinterpret_cast<SkMetaClass **>(class_pp))
          : parse_class_instance(pos, &pos, reinterpret_cast<SkClassUnaryBase **>(class_pp));

        if (result == Result_ok)
          {
          class_count++;

          if (class_pp)
            {
            class_union.merge_class(*class_p);
            }
          }
        }
      while ((result == Result_ok) && (cstr_a[pos] == '|'));


      if (result == Result_ok)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Look for end of class union

        // Next possible error
        result = Result_err_expected_class_union_end;  // Expected class union end

        if (cstr_a[pos] == '>')
          {
          pos++;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Ensure that the union has at least two classes in it

          // Next possible error
          result = Result_err_size_class_union;  // Too few classes

          if (class_count >= 2u)
            {
            result = Result_ok;

            if (type_pp)
              {
              // Next possible error
              result = Result_err_typecheck_union_trivial;  // Trivial union

              if (!class_union.is_trivial())
                {
                result   = Result_ok;
                *type_pp = SkClassUnion::get_or_create(class_union);
                }
              }
            }
          }
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a class cast primitive.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkCast data-structure when the cast is necessary or just receiver_p
//             if the cast is not necessary otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok, Result_err_expected_cast_op, or Result_err_typecheck_cast.
// Arg         receiver_p - expression to cast.  See Notes.
// Notes:      class-cast = expression ws '<>' [class-desc]
//             This method assumes that the [expression ws] has already been parsed and
//             is passed as receiver_p.
// Author(s):   Conan Reis
SkExpressionBase * SkParser::parse_class_cast(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure start of cast
  char * cstr_a    = m_str_ref_p->m_cstr_p;
  uint32_t   start_pos = args.m_start_pos;
  uint32_t   pos       = start_pos;

  if (((m_str_ref_p->m_length - pos) < 2u)
    || (cstr_a[pos] != '<')
    || (cstr_a[pos + 1u] != '>'))
    {
    args.m_result = Result_err_expected_cast_op;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse *optional* class to cast to
  SkClassDescBase * type_p = nullptr;

  args.m_start_pos = pos + 2u;
  args.m_result    = parse_class_desc(args.m_start_pos, &args.m_end_pos, &type_p);

  if ((!args.is_ok()) && (args.m_end_pos != args.m_start_pos))
    {
    return nullptr;
    }

  SkClassDescBase * recv_type_p = args.m_expr_type; 

  args.m_result = Result_ok;

  // If type omitted, try to infer it
  if (type_p == nullptr)
    {
    if (!m_flags.is_set_any(Flag_type_check))
      {
      // Not doing type-checking so assume it is okay
      return receiver_p;
      }

    type_p = args.m_desired_type_p;

    if (type_p == nullptr)
      {
      // No desired type specified so if receiver type is a union including None class,
      // try union - None as inferred type.
      if (recv_type_p->get_class_type() == SkClassType_class_union)
        {
        type_p = SkClassUnion::get_reduced(
          *static_cast<SkClassUnion *>(recv_type_p),
          *SkNone::get_class());

        if (type_p == recv_type_p)
          {
          // Type didn't change
          type_p = nullptr;
          }
        }

      if (type_p == nullptr)
        {
        args.m_start_pos = start_pos;
        args.m_end_pos   = start_pos + 2u;
        args.m_result    = Result_err_typecheck_infer;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "Unable to infer cast class type - class needs to be specified explicitly.\n"
            "[No desired type context available and expression being cast is class type "
            "'%s' which isn't a union class that can have None/nil removed.]",
            recv_type_p->as_code().as_cstr());
        #endif

        return nullptr;
        }
      }
    }


  if (m_flags.is_set_any(Flag_type_check))
    {
    type_p = m_context.finalize_generic(*type_p);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Optimization: Determine if expression and type_p will always be compatible,
    // thus making the cast redundant (it is the same class or it would remove info
    // by replacing the type with a less specific superclass).
    if (recv_type_p->is_class_type(type_p))
      {
      // The cast is unnecessary - it will always be of the desired type.
      args.m_result = Result_warn_expr_redundant;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The expression being cast is %s "
          "and it is already %s so the cast is redundant.\n",
          recv_type_p->get_scope_desc().as_cstr(),
          type_p->get_scope_desc().as_cstr());
      #endif

      // $Revisit - CReis Could ignore this or just *log* a warning and just return receiver_p
      return nullptr;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure that cast is possible.
    // Note that the test order is opposite from `SkCast::invoke()` since the actual type
    // of the object is *not* fully known.
    if (!type_p->is_class_type(recv_type_p)
      && ((recv_type_p->get_class_type() != SkClassType_class_union)
        || !static_cast<SkClassUnion *>(recv_type_p)->is_class_maybe(type_p)))
      {
      args.m_result = Result_err_typecheck_cast;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The expression being cast is %s and it cannot be cast to %s since "
          "there is no derivation path - i.e. not a subclass or superclass.\n"
          "[Perhaps try using a conversion '>>' rather than a cast '<>'.]",
          recv_type_p->get_scope_desc().as_cstr(),
          type_p->get_scope_desc().as_cstr());
      #endif

      return nullptr;
      }

    args.m_expr_type = type_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if structure is desired
  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  SkExpressionBase * expr_p = SK_NEW(SkCast)(type_p, receiver_p);

  SKDEBUG_SET_CHAR_POS(expr_p, start_pos);

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a class conversion
//             primitive.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkConversion data-structure when the conversion is necessary or
//             just receiver_p if the conversion is not necessary otherwise nullptr is
//             returned. 
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok, Result_err_expected_conversion_op, or
//                 Result_err_context_non_method.
// Arg         receiver_p - expression to convert.  See Notes.
// Notes:      class-conversion = expression ws '>>' [class]
//
//             This method assumes that the expression ws has already been parsed and
//             is passed as receiver_p.
// Author(s):   Conan Reis
SkExpressionBase * SkParser::parse_class_conversion(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure start of convert
  char *     cstr_a    = m_str_ref_p->m_cstr_p;
  uint32_t   start_pos = args.m_start_pos;
  uint32_t   pos       = start_pos;

  if (((m_str_ref_p->m_length - pos) < 2u)
    || (cstr_a[pos] != '>')
    || (cstr_a[pos + 1u] != '>'))
    {
    args.m_result = Result_err_expected_conversion_op;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse class to convert to
  SkClass *      class_p  = nullptr;
  SkMethodBase * method_p = nullptr;

  args.m_start_pos = pos + 2u;
  args.m_result    = parse_class(args.m_start_pos, &args.m_end_pos, &class_p);

  if (!args.is_ok() && (args.m_end_pos != args.m_start_pos))
    {
    return nullptr;
    }

  SkClassDescBase * recv_type_p = args.m_expr_type;

  args.m_result = Result_ok;

  // If type omitted, try to infer it
  if (class_p == nullptr)
    {
    if (m_flags.is_set_any(Flag_type_check))
      {
      SkClassDescBase * type_p = args.m_desired_type_p;

      if (type_p == nullptr)
        {
        // No desired type specified so if receiver type is a union including None class,
        // try union - None as inferred type.
        if (recv_type_p->get_class_type() == SkClassType_class_union)
          {
          type_p = SkClassUnion::get_reduced(
            *static_cast<SkClassUnion *>(recv_type_p),
            *SkNone::get_class());

          if (type_p == recv_type_p)
            {
            // Type didn't change
            type_p = nullptr;
            }
          }
        }

      if (type_p && (type_p->get_class_type() == SkClassType_class))
        {
        class_p = static_cast<SkClass *>(type_p);
        }
      }

    if (class_p == nullptr)
      {
      args.m_start_pos = start_pos;
      args.m_end_pos   = start_pos + 2u;
      args.m_result    = Result_err_typecheck_infer;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Unable to infer convert class type - class needs to be specified explicitly.\n"
          "[No desired type context available and expression being converted is class type "
          "'%s' which isn't a union class that can have None/nil removed.]",
          recv_type_p ? recv_type_p->as_code().as_cstr() : "Unknown???");
      #endif

      return nullptr;
      }
    }


  if (m_flags.is_set_any(Flag_type_check))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Optimization: Determine if expression and type_p will always be compatible,
    // thus making the conversion redundant.
    if (args.m_expr_type->is_class_type(class_p))
      {
      // The conversion is unnecessary - it will always be of the desired type.
      args.m_result = Result_warn_expr_redundant;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The expression being converted is already known to be %s "
          "so converting it to %s is redundant.",
          args.m_expr_type->get_scope_desc().as_cstr(),
          class_p->get_scope_desc().as_cstr());
      #endif

      // $Revisit - CReis Could ignore this or just *log* a warning and just return receiver_p
      return nullptr;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure that a proper conversion method exists.
    bool is_class_method = false;
    method_p = find_method_inherited(args.m_expr_type, class_p->get_name(), &is_class_method);
    if (!method_p || is_class_method)
      {
      args.m_result = Result_err_context_non_method;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          method_p 
            ? "Trying to invoke class method '%s()' as a conversion operator on %s. Conversion methods must be instance methods." 
            : "The class conversion method '%s()' does not exist for %s.",
          class_p->get_name_cstr_dbg(),
          args.m_expr_type->get_scope_desc().as_cstr());
      #endif

      return nullptr;
      }

    args.m_expr_type = class_p;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if structure is desired
  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  SkExpressionBase * expr_p = SK_NEW(SkConversion)(class_p, method_p->get_vtable_index(), receiver_p);

  SKDEBUG_SET_CHAR_POS(expr_p, start_pos);

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Forgiving parse of Boolean value - generally from a configuration/settings file where
// the value may be entered by hand by a user.
//
// #Examples
//   true values:
//     true  [preferred]
//     t*
//     yes
//     y*
//     1*
//   false values:
//     false  [preferred]
//     f*
//     no
//     n*
//     0*
//
// #See Also  parse_class_meta_source()
// #Author(s) Conan Reis
bool SkParser::parse_cfg_boolean(
  // Index position to start parse
  uint32_t start_idx, // = 0u
  // Pointer to address to store index position where parse completed
  uint32_t * end_idx_p, // = nullptr
  // Default value for Boolean if cannot otherwise determine the value
  bool def_value // = false
  ) const
  {
  uint32_t end_idx = m_str_ref_p->m_length;
      
  // Set pos past the last A-Z, a-z, _, 0-9, or European character
  find(ACharMatch_not_identifier, 1u, &end_idx, start_idx, end_idx - 1u);

  if (end_idx_p)
    {
    *end_idx_p = end_idx;
    }

  if (start_idx < end_idx)
    {
    switch (m_str_ref_p->m_cstr_p[start_idx])
      {
      case 't':
      case 'y':
      case '1':
        return true;

      case 'f':
      case 'n':
      case '0':
        return false;
      }
    }

  return def_value;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create class meta information and
// applying it to the supplied class.
//
// #Notes
//   Essentially:
// 
//     meta-file = ws {instance-name ':' ws value ws}
//   
//     Valid instance-name: value pairs:
//     
//       demand_load:        true | false*
//       object_id_validate: none | any* | parse | exist | defer
//       
//       * defaults
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if parsed successfully, else false
  bool
SkParser::parse_class_meta_source(
  // class to apply meta info to
  SkClass * scope_p,  
  // see SkParser::Args
  Args & args,              // = ms_def_args.reset()
  bool   apply_meta_data_b  //  = true
  )
  {
  // Does a *quick* parse rather than a comprehensive parse.
  // $Revisit - [Future] Extend this method into config/class parse

  char     ch;
  uint32_t key_idx;
  uint32_t key_end_idx;
  ASymbol  key_name;
  ASymbol  value_name;
  char *   cstr_a = m_str_ref_p->m_cstr_p;
  uint32_t length = m_str_ref_p->m_length;

  A_LOOP_INFINITE
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      return false;
      }

    args.m_start_pos = args.m_end_pos;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if the parse is at end of source
    if (args.m_end_pos >= length)
      {
      // Exit - successful class meta info parse.
      return true;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse key name

    key_idx = args.m_start_pos;
    parse_name_symbol(key_idx, &args.m_end_pos, &key_name);

    ch = (args.m_end_pos < length) ? cstr_a[args.m_end_pos] : '\0';

    if (ch != ':')
      {
      args.m_result = Result_err_expected_binding;
      return false;
      }

    key_end_idx = args.m_end_pos;
    args.m_start_pos = key_end_idx + 1u;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      return false;
      }

    args.m_start_pos = args.m_end_pos;

    // $Revisit - CReis Might want a check that if the whitespace read over a newline that
    // the value is assumed to be skipped.


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse key value and apply it
    enum eMetaSymbolId
      {
      MetaSymbolId_demand_load        = 0xfbc4445b,
      MetaSymbolId_object_id_validate = 0xd06dbecf,
      MetaSymbolId_annotations        = 0x48931805,

      // Validate values - see SkClass::eFlag
        MetaSymbolId_none  = 0x7f9000cf,
        MetaSymbolId_any   = 0x64f3f7b4,
        MetaSymbolId_parse = 0xd2d58468,
        MetaSymbolId_exist = 0x2d07e8ec,
        MetaSymbolId_defer = 0x4cb319c1
      };

    switch (key_name.get_id())
      {
      case MetaSymbolId_demand_load:
        {
        bool demand_b = parse_cfg_boolean(args.m_start_pos, &args.m_end_pos);
        if (apply_meta_data_b)
          {
          scope_p->enable_demand_load(demand_b);
          }
        break;
        }

      case MetaSymbolId_object_id_validate:
        {
        parse_name_symbol(args.m_start_pos, &args.m_end_pos, &value_name);

        SkClass::eFlag validate_flag = SkClass::Flag__id_valid_none;

        switch (value_name.get_id())
          {
          case MetaSymbolId_none:
            validate_flag = SkClass::Flag__id_valid_none;
            break;

          case MetaSymbolId_any:
            validate_flag = SkClass::Flag__id_valid_any;
            break;

          case MetaSymbolId_parse:
            validate_flag = SkClass::Flag__id_valid_parse;
            break;

          case MetaSymbolId_exist:
            validate_flag = SkClass::Flag__id_valid_exist;
            break;

          case MetaSymbolId_defer:
            validate_flag = SkClass::Flag__id_valid_defer;
            break;

          default:
            args.m_result = Result_err_expected_meta_value;

            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "Class '%s' meta value 'object_id_validate' given unrecognized '%s'!\n"
                "[Valid values: none, any, parse, defer or exist]",
                scope_p->get_name_cstr_dbg(),
                value_name.as_cstr_dbg());
            #endif

            return false;
          }

        if (apply_meta_data_b)
          {
          scope_p->set_object_id_validate(validate_flag);
          }
        break;
        }

      case MetaSymbolId_annotations:
        {
        Annotations annotations;
        args.m_result = parse_annotations(args.m_start_pos, &args.m_end_pos, &annotations, SkAnnotationTarget_class);
        if (!args.is_ok())
          {
          return false;
          }
        if (apply_meta_data_b)
          {
          scope_p->set_annotation_flags(scope_p->get_annotation_flags() | annotations.m_flags);
          if (!annotations.m_name.is_empty())
            {
            scope_p->set_bind_name(annotations.m_name);
            }
          }
        break;
        }

      default:
        args.m_result    = Result_err_expected_meta_key;
        args.m_start_pos = key_idx;
        args.m_end_pos   = key_end_idx;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "Class `%s` encountered unexpected meta key name `%s` in its !Class.sk-meta file!\n"
            "[Valid key names: demand_load, object_id_validate, or annotations]",
            scope_p->get_name_cstr_dbg(),
            key_name.as_cstr_dbg());
        #endif

        return false;
      }

    args.m_start_pos = args.m_end_pos;
    }

    //  Should never get here

  } //  SkParser::parse_class_meta_source()


//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create an optional class
//             scope.
// Returns:    Result_ok, Result_err_expected_class, Result_err_context_non_class,
//             Result_err_expected_scope_op, or Result_err_unexpected_eof 
// Arg         start_pos - character position to begin lexical analysis.
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.
// Arg         qual_scope_pp - Pointer to an address to store a pointer to the class scope if
//             present or nullptr if it is not present.  If the class scope is present, it
//             is set to a pointer to an existing class makes a class with that name if
//             it does not yet exist.  It is not written to if it is set to nullptr or if
//             the result is not Result_ok.
// Arg         scope_p - class scope to qualify/narrow for type-checking.  Ignored if nullptr.
// See:        parse_class(), parse_name_coroutine(), parse_name_scoped_instance()
// Notes:      optional-scope = [class-name '@']
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_class_scope(
  uint32_t          start_pos,     // = 0u
  uint32_t *            end_pos_p,     // = nullptr
  SkClass **        qual_scope_pp, // = nullptr
  SkClassDescBase * scope_p        // = nullptr
  ) const
  {
  SkClass * qual_scope_p = nullptr;
  uint32_t  pos          = start_pos;
  eResult   result       = parse_class(start_pos, &pos, qual_scope_pp ? &qual_scope_p : nullptr);

  if (result == Result_ok)
    {
    // Next possible error
    result = Result_err_unexpected_eof;

    if (m_str_ref_p->m_length > pos)
      {
      // Next possible error
      result = Result_err_expected_scope_op;

      if (m_str_ref_p->m_cstr_p[pos] == '@')
        {
        pos++;

        // Next possible error
        result = Result_err_typecheck_scope;

        // Ensure that qualified scope is a valid with respect to receiving scope
        if ((scope_p == nullptr) || qual_scope_p->is_scope_qualifier(scope_p))
          {
          result = Result_ok;
          }
        }
      }

    if ((result != Result_ok) && (result != Result_err_typecheck_scope))
      {
      // Class was valid, didn't get past '@', and class scope is optional so assume
      // parse is something else that starts with a class.
      result       = Result_ok;
      pos          = start_pos;
      qual_scope_p = nullptr;
      }
    }
  else
    {
    // Ok if parse did not advance since class scope is optional
    if (pos == start_pos)
      {
      result = Result_ok;
      }
    }

  if (qual_scope_pp && (result == Result_ok))
    {
    *qual_scope_pp = qual_scope_p;
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Parses attempting to create a closure literal.
//   Also known as or similar to in other languages: code block, anonymous function,
//   lambda expression, etc.
// 
// #Notes
//   reset_scope() must be called with an appropriate surrounding/default scope type
//   before calling this method.
//
//   closure    = ['^' ['_' ws] [expression ws]] [parameters ws] code-block
//   parameters = parameter-list [ws class-desc]
//   
//   *Either the optional '^' section or parameters or both must be provided unless used
//   in a closure-tail-args.  The optional expression (which may not be a code block or
//   another closure-literal [to ensure that that it is not confused for the parameters or
//   code-block] will be captured and used as the receiver/this/scope for the code-block
//   - if not provided this is inferred.  The optional '_' indicates it is durational
//   (like a coroutine) - if not present durational/immediate inferred via code-block.
//   Parameter types, return type, scope, whether the surrounding this or temporary/
//   parameter variables are used and to be captured or not may all be inferred if omitted.
// 
// #See Also  parse_method(), parse_method_source()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #Returns
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
//   SkMethodBase data-structure otherwise nullptr is returned.
SkLiteralClosure *
SkParser::parse_closure(
  // See SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_closure,
  //     Result_err_typecheck_return_type or pretty much any other warning or error.
  Args & args, // = ms_def_args.reset()
  // If true allow closures without ^ or interface () and just use []
  bool allow_inline // = false
  ) const
  {
  // Nested Structures

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Structure to ensure proper cleanup
  struct NestedDtor
    {
    // Data Members

      uint32_t m_parse_flags;
      const SkParser * m_parser_p;
      SkClassUnaryBase * m_old_class_p;
      SkParameters * m_old_params_p;
      eSkMember m_old_member_type;
      bool m_capturing;

    // Methods

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      NestedDtor(const SkParser * parser_p) :
        m_parser_p(parser_p),
        m_old_class_p(parser_p->m_context.m_obj_scope_p),
        m_old_params_p(parser_p->m_context.m_params_p),
        m_old_member_type(parser_p->get_member_type()),
        m_capturing(false)
        {
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      ~NestedDtor()
        {
        SkTypeContext * context_p = &m_parser_p->m_context;

        if (m_capturing)
          {
          context_p->capture_locals_stop(nullptr);
          }

        context_p->m_obj_scope_p = m_old_class_p;
        context_p->m_params_p = m_old_params_p;
        m_parser_p->set_member_type(m_old_member_type);
        }
    };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure enough characters
  uint32_t pos = args.m_start_pos;

  if (m_str_ref_p->m_length < pos)
    {
    args.m_end_pos = pos;
    args.m_result  = Result_err_unexpected_eof;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional code literal marker + receiver expression
  //   ['^' ['_' ws] [expression ws]]

  // Indicates whether expressions in the code block should be restricted to:
  //   SkInvokeTime_immediate - only immediate expressions permitted and returns SkClosureInfoMethod
  //   SkInvokeTime_durational - both immediate and durational permitted and returns
  //     SkClosureInfoCoroutine (even if only immediate expressions)
  //   SkInvokeTime_any - both immediate and durational permitted and returns SkClosureInfoMethod
  //     or SkClosureInfoCoroutine as necessary
  eSkInvokeTime desired_exec = SkInvokeTime_any;

  // Could be a coroutine (durational) though assume method (immediate) for now
  SkLiteralClosure    closure;
  SkClosureInfoMethod closure_info;

  // It is created on the stack so give it an initial reference.
  closure_info.reference();
  closure.m_info_p = &closure_info;

  // Uses destructor to ensure proper cleanup
  NestedDtor dtor(this);

  // Remember desired type before it is stomped on when parsing sub-expressions.
  SkClassDescBase * desired_type_p = args.m_desired_type_p;

  SkClassUnaryBase * obj_class_p  = dtor.m_old_class_p;
  char *             cstr_a       = m_str_ref_p->m_cstr_p;
  uint32_t           start_pos    = pos;
  bool               caret_marker = cstr_a[pos] == '^';
  Annotations        annotations;

  if (caret_marker)
    {
    pos++;

    // Ensure enough characters
    if (m_str_ref_p->m_length < pos)
      {
      args.m_end_pos = pos;
      args.m_result  = Result_err_unexpected_eof;

      // Context reset and extra cleanup by dtor.~NestedDtor()
      return nullptr;
      }

    // Parse annotations
    args.m_result = parse_annotations(pos, &pos, &annotations, SkAnnotationTarget_invokable);
    if (!args.is_ok())
      {
      args.m_end_pos = pos;

      // Context reset and extra cleanup by dtor.~NestedDtor()
      return nullptr;
      }
    closure_info.set_annotation_flags(annotations.m_flags);
    closure_info.set_akas((tSkAkas &&)annotations.m_akas);

    // Look for optional durational marker
    //   ['_' ws]
    if ((cstr_a[pos] == '_') && (!ms_is_lowercase[uint8_t(cstr_a[pos + 1u])]))
      {
      pos++;
      desired_exec = SkInvokeTime_durational;

      // Eat {whitespace}
      args.m_result = parse_ws_any(pos, &pos);

      if (!args.is_ok())
        {
        args.m_end_pos = pos;

        // Context reset and extra cleanup by dtor.~NestedDtor()
        return nullptr;
        }
      }

    args.m_start_pos = pos;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If no optional parameter list or code-block yet Look for optional receiver object
    // expression using current context scope.
    if ((cstr_a[pos] != '[') && (cstr_a[pos] != '('))
      {
      // [expression ws]
      args.m_desired_type_p = nullptr;
      closure.m_receiver_p = parse_expression(args, SkInvokeTime_immediate);

      if (!args.is_ok())
        {
        // Context reset and extra cleanup by dtor.~NestedDtor()
        return nullptr;
        }

      obj_class_p = m_context.finalize_generic(*args.m_expr_type)->as_unary_class();

      // Eat {whitespace}
      args.m_result = parse_ws_any(args.m_end_pos, &pos);

      if (!args.is_ok())
        {
        args.m_end_pos = pos;

        // Context reset and extra cleanup by dtor.~NestedDtor()
        return nullptr;
        }

      // Change to code literal object/this scope if different from current
      if (m_context.m_obj_scope_p != obj_class_p)
        {
        set_class_scope(obj_class_p);
        }
      }
    }  // End of optional ^ marker section


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Adjust Context - save old member type and assume for now that it is going to be
  // durational(coroutine).
  // $Revisit - CReis May need to backtrack to this point if type is determined to be
  // different - mainly for "this_code" identifier.
  m_member_type = SkMember_coroutine;

  if (m_flags.is_set_any(Flag_type_check))
    {
    m_context.capture_locals_start();
    dtor.m_capturing = true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for parameters - optional if ^ marker was used or if inline closure argument
  // 
  // [parameters ws]
  // parameters = parameter-list [ws class-desc]
  SkParameters       params;
  SkParameters *     params_p = &params;
  bool               interface_provided = false;
  SkInvokableClass * inferred_class_p = nullptr;
  uint32_t           param_start = pos;

  m_context.m_params_p = params_p;

  if (cstr_a[pos] == '(')
    {
    args.m_start_pos = pos;

    if (!parse_parameters(
      args, args.is_struct_wanted() ? &params : nullptr, ParamFlag_auto_type, annotations.m_flags))
      {
      // Parse of parameters was invalid.
      // Context reset and extra cleanup by dtor.~NestedDtor()
      return nullptr;
      }

    // Parse of parameters was okay
    args.m_start_pos = args.m_end_pos;
    interface_provided = true;
      
    // Eat {whitespace}
    if (!parse_ws_any(args))
      {
      // Context reset and extra cleanup by dtor.~NestedDtor()
      return nullptr;
      }

    pos = args.m_end_pos;
    }
  else
    {
    // Interface not provided

    // If ^ not used and not inline closure argument then error
    if (!allow_inline && !caret_marker)
      {
      // Non-inline closure must have either ^ or interface ()
      args.m_end_pos = pos;
      args.m_result  = Result_err_expected_closure;

      // Context reset and extra cleanup by dtor.~NestedDtor()
      return nullptr;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Infer parameter interface?
 
    // If ^ used or inline closure argument then use desired_type_p if closure type
    // or use () Auto_
    if (desired_type_p && (desired_type_p->get_class_type() == SkClassType_invokable_class))
      {
      // $Revisit - CReis The file part of the Debug info for the parameters will not be
      // correctly inferred since the parameter version will be used.
      inferred_class_p = static_cast<SkInvokableClass *>(desired_type_p);
      desired_exec = inferred_class_p->get_invoke_type();
      params_p = inferred_class_p->get_parameters();
      m_context.m_params_p = params_p;

      // Add parameter info to parse context
      parameters_context(*params_p);
      }
    else
      {
      // Try to figure out return type from final expression in closure code block
      params.set_result_type(*SkBrain::ms_auto_class_p);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse code block

  // Set desired code block result type if it is known.
  args.m_desired_type_p =
    ((desired_exec != SkInvokeTime_durational)
      && (params_p->get_result_class() != SkBrain::ms_auto_class_p))
    ? params_p->get_result_class()
    : nullptr;
  
  // Desired type of code block may be checked again later when more info is available.
  args.m_start_pos = pos;
  closure_info.m_expr_p = (desired_exec == SkInvokeTime_durational)
    ? parse_code_block_optimized(args, SkInvokeTime_any, ResultDesired_false)
    : parse_code_block_optimized(args, desired_exec, ResultDesired_true);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure returned values from code block are compatible with parameters
  bool make_struct = false;
  
  if (args.is_ok())
    {
    if (desired_exec != SkInvokeTime_durational)
      {
      make_struct = parameters_typecheck(args, params_p, true) && closure_info.m_expr_p;
      }
    else
      {
      make_struct = parameters_typecheck(args, params_p, false) && closure_info.m_expr_p;

      params_p->set_result_type(*SkBrain::ms_invoked_coroutine_class_p);
      }

    // $Revisit - CReis Don't allow generic types in closures until they are fully supported
    if (params_p->is_generic())
      {
      make_struct = false;
      args.m_end_pos = param_start;
      args.m_result  = Result_err_typecheck_closure_generics;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make/fix-up closure structure
  SkLiteralClosure * closure_p = nullptr; 

  if (make_struct)
    {
    // Determine if it is immediate or durational
    SkExpressionBase * body_p = closure_info.m_expr_p;

    if (desired_exec == SkInvokeTime_any)
      {
      desired_exec = body_p
        ? (body_p->is_immediate() ? SkInvokeTime_immediate : SkInvokeTime_durational)
        : SkInvokeTime_immediate;
      }

    // $Revisit - CReis [A_NOTE] ***Incomplete*** - [Closure] Need to determine exec type even if no structure is created?
    // $Revisit - CReis [A_NOTE] ***Optimization*** - [Closure] Set receiver to nil if this (or this_class) not used/captured so that this is not needlessly reference counted
    // $Revisit - CReis [A_NOTE] ***Optimization*** - [Closure] Set receiver to class? if only this_class used/captured so that this is not needlessly reference counted

    eSkExprType expr_type;

    if (desired_exec == SkInvokeTime_immediate)
      {
      expr_type = SkExprType_closure_method;
      }
    else
      {
      expr_type = SkExprType_closure_coroutine;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Ensure that last statement has side effects since a result is not needed
      if (body_p && !ensure_expr_effect(body_p->find_expr_last_no_side_effect(), &args.m_end_pos, args))
        {
        args.m_result = Result_err_context_last_no_side_effect;
        // Context reset and extra cleanup by dtor.~NestedDtor()
        return nullptr;
        }
      }

    if (inferred_class_p)
      {
      args.m_expr_type = inferred_class_p;
      }
    else
      {
      params_p = SkParameters::get_or_create(&params);
      args.m_expr_type = SkInvokableClass::get_or_create(
        SkBrain::ms_closure_class_p,
        params_p,
        desired_exec);
      }

    // Create literal closure structure.
    closure_info.m_params_p = params_p;
    closure_info.m_invoked_data_array_size = (uint16_t)(m_context.m_capture_current_p->m_vars.get_length() + m_context.m_current_scope_p->m_data_idx_count_max);
    closure_p = SK_NEW(SkLiteralClosure)(&closure, expr_type);
    SKDEBUG_SET_CHAR_POS(closure_p, start_pos);

    SkClosureInfoBase * info_p = closure_p->get_closure_info();

    // Let go of locals created from closure parameters
    m_context.free_locals(m_context.m_params_p->get_param_list());

    // Remember captured variables in closure_info
    m_context.capture_locals_stop(&info_p->m_captured);
    dtor.m_capturing = false;

    // Set enclosing scope - for debugging
    info_p->set_scope(dtor.m_old_class_p->get_key_class());
    info_p->set_name(m_context.m_scope_name);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Context reset and extra cleanup by dtor.~NestedDtor()
  return closure_p;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Parses attempting to create a code block
//
// #Notes
//   code-block = '[' ws [statement {wsr statement} ws] ']'
//   statement  = expression | create-temporary | loop-exit
//
// #See Also  parse_code_block_optimized()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Result
  //  If parse valid and args.is_struct_wanted() == true it returns a dynamically
  //  allocated SkCode data-structure otherwise nullptr is returned.
  SkCode *
SkParser::parse_code_block(
  // See SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_code_block
  //   Result_err_unexpected_statement or any other expression warning or error.
  Args &            args,              // = ms_def_args.reset()
  eSkInvokeTime     desired_exec_time, // = SkInvokeTime_any
  eStatementTiming  statement_timing,  // = StatementTiming_sequential
  eResultDesired    result             // = ResultDesired_true
  ) const
  {
  char *   cstr_a = m_str_ref_p->m_cstr_p;
  uint32_t length = m_str_ref_p->m_length;
  uint32_t pos    = args.m_start_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for start of code block
  if ((length == 0u) || (cstr_a[pos] != '['))
    {
    args.m_result  = Result_err_expected_code_block;
    args.m_end_pos = pos;

    return nullptr;
    }

  pos++;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  args.m_result = parse_ws_any(pos, &pos);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remember for later
  uint32_t temp_vars_start_idx = m_context.m_current_scope_p->m_data_idx_count; 

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for early end of code block
  SkCode * code_p = nullptr;

  if (args.is_struct_wanted())
    {
    code_p = SK_NEW(SkCode)(temp_vars_start_idx);
    m_context.on_local_data_index_created(code_p->get_temp_vars_start_idx_ptr());
    SKDEBUG_SET_CHAR_POS(code_p, args.m_start_pos);
    }

  if (cstr_a[pos] == ']')
    {
    // Empty code block
    pos++;

    // Empty blocks always return "nil"
    args.m_expr_type = SkNone::get_class();
    args.m_end_pos   = pos;
    args.m_result    = Result_ok;

    return code_p;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Code block with statement(s)
  uint32_t mark_pos;
  uint32_t idx_begin;
  bool     complete   = false;  // Found end of code block or error
  SkCode * old_code_p = m_current_block_p; // Save old code block

  // Save current desired type to replace later
  SkClassDescBase * desired_type_p = args.m_desired_type_p;

  m_current_block_p = code_p;

  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  while (pos < length)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse statement

    // $Vital - CReis Desired type is for the desired *result* and should only
    // be used by last statement though can only determine last statement after
    // the fact so 3 possible actions:
    //   - determine which statement is last then backtrack and reparse [most
    //     *correct* though this would be slow since last statement would need
    //     to be parsed twice plus type context overhead.]
    //   - pass on desired type to every statement [earlier statements could be
    //     incorrectly parsed]
    //   - pass unknown desired type (nullptr) to every statement - including last.
    //     [Current default.  Safest and fastest though last statement does not
    //     benefit from desired type info.]
    args.m_desired_type_p = nullptr;

    args.m_start_pos = pos;
    idx_begin        = pos;
    complete         = !parse_statement_append(args, desired_exec_time);
    pos              = args.m_end_pos;

    // When parsing concurrent statements, make sure each statement gets its own stack space of variables
    if (statement_timing == StatementTiming_concurrent
      && m_flags.is_set_any(Flag_type_check))
      {
      // Next statement's data lives after the maximum needs of the previous statement
      m_context.m_current_scope_p->m_data_idx_count = m_context.m_current_scope_p->m_data_idx_count_max;
      }

    if (complete)
      {
      // Found bad statement parse, so exit while loop
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    mark_pos      = pos;
    args.m_result = parse_ws_any(pos, &pos);

    if (!args.is_ok())
      {
      args.m_start_pos = mark_pos;

      // Bad comment parse, so exit while loop
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for statement delimiter or end of code block
    switch (cstr_a[pos])
      {
      case ';':
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Unnecessary C++ mistake/assumption - end of statement delimiter
        args.m_result = Result_err_unexpected_cpp,

        #if defined(SK_AS_STRINGS)
          ms_error_str.empty();
          ms_error_str.append(
            "SkookumScript uses whitespace to delimit statements - a C++ style end "
            "of statement delimiter ';' is not used.\n"
            "[If ';' was intended to specify return arguments then the prior routine does not "
            "have return arguments or some other syntax error occurred.]");
        #endif

        // Exit while loop
        complete = true;
        break;

      case ']':
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Found end of code block / statement series
        // - no final delimiter (a final delimiter is optional)
        // 'result' is already set to Result_ok at this point.
        pos++;
        complete = true;
        break;

      default:
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Serial "whitespace" delimiter or unexpected text?
        if (mark_pos == pos)
          {
          // Expected ']', but did not find it.
          args.m_result = Result_err_unexpected_statement;
          complete      = true;
          }
        //else
          // Serial statement (delimited by whitespace) - call next statement after
          // previous statement has returned.
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If found end of block or unexpected characters, exit while loop
    if (complete)
      {
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure all but last statement has side effects or it is redundant.
    // Last statement may or may not require a side effect depending on whether a
    // result is desired or not.
    // 
    // $Note CReis - Can only determine whether a statement is the last expression
    // and thus may need to return a value *after* it has already been parsed
    // since the block close ']' is found after the fact.  This means that whether
    // a statement requires a side effect must be determined after the parse
    // of an expression / code block by querying its data structure - unless each
    // parse returned whether an expression had side effects or not in addition to
    // its result type.
    if (code_p)
      {
      if (!ensure_expr_effect(code_p->m_statements.get_last(), &pos, args))
        {
        // Exit while loop
        break;
        }
      }

    // Next possible error
    args.m_result = Result_err_unexpected_eof;
    }  // while

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If no result desired ensure last statement has side effects or it is redundant.
  if ((result == ResultDesired_false) && code_p && args.is_ok())
    {
    ensure_expr_effect(code_p->m_statements.get_last(), &pos, args);
    }
            
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Final clean-up
  if (args.is_ok())
    {
    if (m_flags.is_set_any(Flag_type_check))
      {
      // Ensure that type being returned is not garbage collected
      args.m_expr_type->reference();

      if (code_p)
        {
        // $Revisit - CReis should track temp vars even if structure not returned
        // Remove variables local to this code block from scope & remember that
        // they were used.
        m_context.archive_locals(code_p->m_temp_vars);
        // Restore data index after we are done with concurrent code block
        if (statement_timing == StatementTiming_concurrent)
          {
          m_context.m_current_scope_p->m_data_idx_count = temp_vars_start_idx;
          }
        }

      args.m_expr_type->dereference_delay();
      }
    }
  else
    {
    if (m_flags.is_set_any(Flag_type_check))
      {
      if (code_p)
        {
        // $Revisit - CReis should track temp vars even if structure not returned
        // Remove variables local to this code block from scope
        m_context.free_locals(code_p->m_temp_vars);
        // Restore data index after we are done with concurrent code block
        if (statement_timing == StatementTiming_concurrent)
          {
          m_context.m_current_scope_p->m_data_idx_count = temp_vars_start_idx;
          }
        }
      }

    if (code_p)
      {
      // If there was a parse error - free any accumulated code.
      delete code_p;
      code_p = nullptr;
      }
    }

  // Return previously saved current desired type
  args.m_desired_type_p = desired_type_p;

  m_current_block_p = old_code_p;

  args.m_end_pos = pos;

  return code_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a code block.
//             If there are no statements substitute code with a nil instance (SkLiteral).
//             If there is only one statement and no temporary variables substitute code
//             with just the single statement.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkExpressionBase data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_code_block
//                 Result_err_unexpected_statement or any other expression warning or error.
// See:        parse_code_block()
// Notes:      code-block = '[' ws [statement {wsr statement} ws] ']'
//             statement  = expression | create-temporary | loop-exit
// Author(s):   Conan Reis
SkExpressionBase * SkParser::parse_code_block_optimized(
  Args &         args,              // = ms_def_args.reset()
  eSkInvokeTime  desired_exec_time, // = SkInvokeTime_any
  eResultDesired result             // = ResultDesired_true
  ) const
  {
  if (!args.is_struct_wanted())
    {
    // Validation parse only
    return parse_code_block(args, desired_exec_time, StatementTiming_sequential, result);
    }

  // Optimize code block with 0-1 statements.
  SkExpressionBase * expr_p = nullptr;
  SkCode *           code_p = parse_code_block(args, desired_exec_time, StatementTiming_sequential, result);

  if (code_p)
    {
    switch (code_p->m_statements.get_length())
      {
      case 0u:
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // If there are no statements substitute code with a nil instance
        expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__nil);
        SKDEBUG_SET_CHAR_POS(expr_p, code_p->m_source_idx);
        delete code_p;
        break;

      case 1u:
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // If there is only one statement and no temporary variables substitute code
        // with just the single statement.
        if (code_p->m_temp_vars.is_empty())
          {
          expr_p = code_p->m_statements.get_first();
          code_p->m_statements.empty();
          delete code_p;
          break;
          }
        // Allow fall-through to default case.

      default:
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // The code block cannot be further reduced / optimized so just keep code block
        expr_p = code_p;
      }
    }

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Determines if a portion of code lexically conforms to a comment.
// Returns:    Result_ok, Result_err_expected_comment_close, or Result_err_unexpected_char
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Examples:   if (parse.parse_comment(11u, &end_pos) == Result_ok)
// See:        parse_comment_line(), parse_comment_multiline()
// Notes:      comment = single-comment | multi-comment
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_comment(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p  // = nullptr
  ) const
  {
  eResult result = parse_comment_line(start_pos, end_pos_p);

  return (result == Result_ok) ? result : 
    parse_comment_multiline(start_pos, end_pos_p);
  }

//---------------------------------------------------------------------------------------
// Determines if a portion of code lexically conforms to a single-line
//             comment.
// Returns:    Result_ok or Result_err_unexpected_char
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Examples:   if (parse.parse_comment_line(11u, &end_pos) == Result_ok)
// See:        parse_comment_multiline(), parse_comment()
// Notes:      single-comment = '//' {printable-char} (newline | end-of-file)
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_comment_line(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p  // = nullptr
  ) const
  {
  uint32_t length       = m_str_ref_p->m_length;
  char *   cstr_start_p = m_str_ref_p->m_cstr_p;
  char *   cstr_a       = cstr_start_p + start_pos;

  if ((start_pos + 2u) <= length)
    {
    if ((*cstr_a == '/') && (*(++cstr_a) == '/'))
      {
      char * cstr_end_p = cstr_start_p + length - 1u;

      while ((cstr_a < cstr_end_p) && (*cstr_a != '\n'))
        {
        cstr_a++;
        }

      if (end_pos_p)
        {
        *end_pos_p = uint32_t(cstr_a - cstr_start_p) + 1u;
        }

      return Result_ok;
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = uint32_t(cstr_a - cstr_start_p);
    }
  

  return Result_err_unexpected_char;
  }

//---------------------------------------------------------------------------------------
// Determines if a portion of code lexically conforms to a multi-line
//             comment - taking into account any other nested multi-line comments.
// Returns:    Result_ok, Result_err_expected_comment_close, or Result_err_unexpected_char
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Examples:   if (parse.parse_comment_multiline(11u, &end_pos) == Result_ok)
// See:        parse_comment_line(), parse_comment()
// Notes:      Multi-line SkookumScript comments may be nested.
//             No special action is taken if a multi-line comment end '*/' is contained
//             within a single line comment, a string literal, or a symbol literal.
//
//             multi-comment = '/*' {printable-char} [multi-comment {printable-char}] '*/'
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_comment_multiline(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p  // = nullptr
  ) const
  {
  uint32_t length = m_str_ref_p->m_length;
  uint32_t  pos    = start_pos;

  if ((start_pos + 2u) <= length)
    {
    char *  cstr_a = m_str_ref_p->m_cstr_p;

    if ((cstr_a[pos] == '/') && (cstr_a[++pos] == '*'))
      {
      pos += 2u;  // increments of 2

      while (pos < length)
        {
        switch (cstr_a[pos])
          {
          case '*':
            if (parse_comment_multiline(pos - 1u, &pos) != Result_ok)
              {  // pos now 1 less
              if (cstr_a[pos + 2u] == '/')
                {
                if (end_pos_p)
                  {
                  *end_pos_p = pos + 3u;
                  }
                return Result_ok;
                }
              else
                {
                pos += 3u;
                }
              }
            else
              {
              pos++;
              }
            break;

          case '/':
            if (cstr_a[pos - 1u] == '*')
              {
              if (end_pos_p)
                {
                *end_pos_p = pos + 1u;
                }
              return Result_ok;
              }
            else
              {
              if (parse_comment_multiline(pos, &pos) != Result_ok)
                {
                pos += 2u;
                }
              else
                {
                pos++;
                }
              }
            break;

          default:
            pos += 2u;
          }
        }

      if (end_pos_p)
        {
        *end_pos_p = pos;
        }

      return Result_err_expected_comment_close;
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return Result_err_unexpected_char;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a conditional expression.
// [Assumes 'if' token has already been parsed.]
//
// #Notes
//   conditional ='if' {ws expression ws code-block}1+ [ws 'else' ws code-block]
//   code-block  = '[' ws [statement {wsr statement} ws] ']'
//
//   Example:
//
//     if bool_test1 [ clause1 ]
//       bool_test2  [ clause2 ]
//       else        [ else_clause ]
//
// #See Also  parse_case_tail()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // data-structure otherwise nullptr is returned.
  SkConditional *
SkParser::parse_conditional_tail(
  // see SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_unexpected_else, Result_err_typecheck_test,
  //     Result_err_expected_clause_block, or any other expression warning or error.
  Args & args // = ms_def_args.reset()
  ) const
  {
  // Nested Structures

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Structure to ensure proper cleanup
  struct Nested
    {
    // Data Members

      const SkParser * m_parser_p;
      Args * m_args_p;
      SkConditional * m_cond_p;
      SkExpressionBase * m_test_p;
      bool m_parsing_test;
      SkClassUnion m_result_type;
      // Type changes made during clause block
      tSkTypedNamesIndexed m_alt_context;
      // Ending position of previous {test-clause} pair and starting position of potential
      // next {test-clause} pair and used for backtrack if following expression mistakenly
      // parsed as a test expression.
      uint32_t m_pair_end_pos;

    // Methods

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Nested(const SkParser * parser_p, Args * args_p) :
        m_parser_p(parser_p),
        m_args_p(args_p),
        m_cond_p(nullptr),
        m_test_p(nullptr),
        m_parsing_test(false)
        {
        if (args_p->is_struct_wanted())
          {
          m_cond_p = SK_NEW(SkConditional)();
          SKDEBUG_SET_CHAR_POS(m_cond_p, args_p->m_start_pos);
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Nest type context in case a backtrack is performed
      void pre_test_expr()
        {
        m_parser_p->m_context.nest_locals(SkNestReason_exploratory);
        m_parsing_test = true;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Test expression was valid so accept any type changes
      void post_test_expr()
        {
        m_parser_p->m_context.accept_nest();
        m_parsing_test = false;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // May have read following expression too eagerly - ignore it
      void ignore_test_expr()
        {
        // Undo any type changes made while attempting to parse test expression
        if (m_parsing_test)
          {
          m_parser_p->m_context.unnest_locals(SkUnnestAction_reject);
          m_parsing_test = false;
          }

        // Get rid of any unused test expression
        if (m_test_p)
          {
          delete m_test_p;
          m_test_p = nullptr;
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Try to backtrack if expression following 1+ valid conditional [test-clause} pairs
      // had an error or a valid expression that wasn't followed by a code block (which is
      // a *potentially error prone* mechanism to signal the end of a conditional.
      SkConditional * backtrack()
        {
        ignore_test_expr();

        // Manage type information
        if (m_parser_p->m_flags.is_set_any(Flag_type_check))
          {
          // There is no 'else' clause so:
          
          // Merge original type context since that is a possible path
          m_parser_p->m_context.merge(&m_alt_context);
          // Merge in nil as a possible result.
          m_result_type.merge_class(*SkNone::get_class());
          }

        m_args_p->m_end_pos = m_pair_end_pos;
        m_args_p->m_result  = Result_ok;

        return m_cond_p;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Perform clean-up and ensure proper resulting context state
      ~Nested()
        {
        ignore_test_expr();

        if (m_args_p->is_ok())
          {
          // Manage type information
          if (m_parser_p->m_flags.is_set_any(Flag_type_check))
            {
            // Update variables whose types changed during the alternate code paths.
            m_parser_p->m_context.change_variable_types(m_alt_context);

            // Store result type
            m_args_p->m_expr_type = m_result_type.is_trivial()
              ? static_cast<SkClassDescBase *>(m_result_type.get_common_class())
              : SkClassUnion::get_or_create(m_result_type);
            }
          }
        else
          {
          if (m_cond_p)
            {
            delete m_cond_p;
            }
          }

        m_alt_context.free_all();
        }
    };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup structures
  Nested nested(this, &args);

  SkClassDescBase * desired_type_p = args.m_desired_type_p;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse {test expression - clause block} pairs

  uint32_t start_pos = args.m_start_pos;
  uint32_t pos       = start_pos;
  uint32_t length    = m_str_ref_p->m_length;
  char *   cstr_a    = m_str_ref_p->m_cstr_p;

  // Clause info
  uint32_t clause_count = 0u;
  bool     is_else_b    = false;

  SkExpressionBase * clause_p = nullptr;

  APCompactArrayFree<SkClause> * clauses_p = nested.m_cond_p ? &nested.m_cond_p->m_clauses : nullptr;

  do
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Note that nested.~Nested() does the cleanup.
      return nullptr;
      }

    pos = args.m_end_pos;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for test expression or "else"

    // Look for an 'else' clause and ensure it is not some other identifier that has the
    // root word 'else'
    if (((length - pos) >= 4u)
      && (cstr_a[pos]      == 'e')
      && (cstr_a[pos + 1u] == 'l')
      && (cstr_a[pos + 2u] == 's')
      && (cstr_a[pos + 3u] == 'e')
      && ms_char_match_table[ACharMatch_not_identifier][uint8_t(cstr_a[pos + 4u])])
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Found an else clause
      pos += 4u;
      is_else_b   = true;

      if (clause_count == 0u)
        {
        args.m_result  = Result_err_unexpected_else;
        args.m_end_pos = pos;
        // Note that nested.~Nested() does the cleanup.
        return nullptr;
        }
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // If an else clause was not found look for a regular test expression

      nested.pre_test_expr();
      args.m_start_pos = pos;
      args.m_desired_type_p = SkBrain::ms_boolean_class_p;
      nested.m_test_p = parse_expression(args, SkInvokeTime_immediate);

      if (!args.is_ok())
        {
        // If at least 1+ {test-clause} pairs try to backtrack and accept what is valid
        // so far.  Note that nested.~Nested() does the cleanup.
        return (clause_count == 0u)
          ? nullptr
          : nested.backtrack();
        }

      pos = args.m_end_pos;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Only assume expression is part of a {test - clause} pair if it looks like it is
    // followed by a code block.
      
    // Eat {whitespace}
    args.m_result = parse_ws_any(pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Note that nested.~Nested() does the cleanup.
      return nullptr;
      }

    pos = args.m_end_pos;


    // Look for code block
    if (cstr_a[pos] != '[')
      {
      args.m_result = Result_err_expected_clause_block;
      // If not an 'else' block and at least 1+ {test-clause} pairs try to backtrack and
      // accept what is valid so far.  Note that nested.~Nested() does the cleanup.
      // $Note - CReis This *may* be intentended as another test expression for the
      // conditional and the clause block has a typo.
      // $Revisit - CReis Expression could be checked if it has a Boolean result type
      // meaning it is more likely (though still not certain) to be a clause block typo.
      return (is_else_b || (clause_count == 0u))
        ? nullptr
        : nested.backtrack();
      }

    if (!is_else_b)
      {
      // Looks like a {test - clause} pair so commit to last expression being test expression
      nested.post_test_expr();

      // Ensure that the result type of the test expression is a Boolean.
      if (m_flags.is_set_any(Flag_type_check)
        && !args.m_expr_type->is_class_type(SkBrain::ms_boolean_class_p))
        {
        args.m_result = Result_err_typecheck_test;
        // Note that nested.~Nested() does the cleanup.
        return nullptr;
        }
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse clause block
    if (m_flags.is_set_any(Flag_type_check))
      {
      // Nest context for alternate code path.
      m_context.nest_locals(SkNestReason_exploratory);
      }

    args.m_start_pos = pos;
    args.m_desired_type_p = desired_type_p;
    clause_p = parse_code_block_optimized(args);

    if (!args.is_ok())
      {
      m_context.unnest_locals(SkUnnestAction_reject);
      // Note that nested.~Nested() does additional cleanup.
      return nullptr;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Accumulate {test expression - clause block} pair
    pos = args.m_end_pos;
    nested.m_pair_end_pos = pos;
    clause_count++;

    if (clauses_p)
      {
      clauses_p->append(*SK_NEW(SkClause)(nested.m_test_p, clause_p));
      nested.m_test_p = nullptr;
      }

    // Manage type information
    if (m_flags.is_set_any(Flag_type_check))
      {
      // Accumulate result type of conditional
      nested.m_result_type.merge_class(*args.m_expr_type);

      // Merge context changes into alternate context
      if (nested.m_alt_context.is_filled() || m_context.is_locals())
        {
        m_context.merge_locals(&nested.m_alt_context, clause_count == 1u);
        }

      // Accept and remove nested context for alternate code path.
      m_context.unnest_locals(SkUnnestAction_accept);
      }
    }
  while (!is_else_b);
  // Exit if successful 'else' clause all other exits are via errors or backtracks.

  // Note that nested.~Nested() does the cleanup.
  return nested.m_cond_p;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to parse a data member.
// Returns:    Result_ok, Result_err_expected_instance, Result_err_context_duped_data,
//             Result_err_context_duped_data_super, Result_err_context_duped_data_sub, or
//             Result_err_unexpected_eof 
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Arg         append_to_class_b - if true, the data member will be appended to the supplied
//             class scope.  If false, the data member is parsed/validated for correctness
//             only - no data structures are created or modified.
// Examples:   if (parse.parse_data_definition(class_p, 11u, &end_pos, is_class_data) == Result_ok)
// See:        parse_temporary() - fairly similar parse if not identical
// Notes:      data-definition = [class-desc ws] '!' instance-name
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_data_definition(
  uint32_t   start_pos,        // = 0u
  uint32_t * end_pos_p,        // = nullptr
  bool       append_to_class_b // = true
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse optional class type
  SkClassDescBase * type_p;
  uint32_t          pos;
  uint32_t          class_idx  = start_pos;
  bool              inferred_b = false;

  Annotations annotations;
  eResult result = parse_annotations(start_pos, &pos, &annotations, SkAnnotationTarget_instance_data);
  if (result != Result_ok)
    {
    if (end_pos_p) *end_pos_p = pos;
    return result;
    }

  result = parse_class_desc(pos, &pos, &type_p);

  if (result == Result_ok)
    {
    class_idx = pos;

    // Eat whitespace
    parse_ws_any(pos, &pos);
    }
  else
    {
    // Ok if parse did not advance since class type is optional
    if (pos == start_pos)
      {
      // Default to class type "Object"
      type_p = SkBrain::ms_object_class_p;
      result = Result_ok;
      inferred_b = true;
      }
    else
      {
      // Class type error - exit
      if (end_pos_p)
        {
        *end_pos_p = pos;
        }

      return result;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse data member name
  
  // Look for exclamation mark separator - just to look more like "create temporary" it
  // is not actually *needed*.
  if (m_str_ref_p->m_cstr_p[pos] != '!')
    {
    if (end_pos_p)
      {
      *end_pos_p = pos;
      }

    return Result_err_expected_data_defn;
    }

  pos++;

  // Parse data member name
  ASymbol name;
  bool    predicate_b  = false;
  bool    class_data_b = false;

  result = parse_name_data_member(pos, &pos, &name, &predicate_b, &class_data_b);

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  if (result != Result_ok)
    {
    return result;
    }

  // Ensure correct number of @ based on whether @instance or @@class data expected.
  bool class_expected_b = m_context.m_obj_scope_p->is_metaclass();

  if (class_expected_b != class_data_b)
    {
    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "%s data members must start with %s.",
        class_expected_b ? "Class" : "Instance",
        class_expected_b ? "two '@@' \"at\" symbols" : "one '@' \"at\" symbol");
    #endif

    return Result_err_expected_data_name_scope;
    }

  if (predicate_b)
    {
    if (inferred_b)
      {
      type_p = SkBrain::ms_boolean_class_p;
      }
    else
      {
      // Ensure type specified was Boolean
      if (type_p != SkBrain::ms_boolean_class_p)
        {
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "'%s' was specified as the type when Boolean was expected.\n"
            "Query/predicate data members ending with a question mark '?' must be specified "
            "as a Boolean or omit the type in which case Boolean is inferred.",
            type_p->as_code().as_cstr());
        #endif

        // Don't return immediately - still allow the data member to be added so that it
        // is displayed in the class browser.
        pos    = class_idx;
        result = Result_err_typecheck_query_data;
        }
      }
    }

  if (append_to_class_b)
    {
    // Ensure no instance or class data member with the same name was already specified
    // for this class, any superclass or any subclasses - some of which could have been
    // set when passing through earlier overlay libraries.
    SkClass * current_class_p = m_context.m_obj_scope_p->get_key_class();
    SkClass * dupe_class_p = class_expected_b
      ? current_class_p->find_class_data_scope(name)
      : current_class_p->find_instance_data_scope(name);

    if (dupe_class_p == nullptr)
      {
      if (annotations.m_flags & SkAnnotation_raw)
        {
        m_context.m_obj_scope_p->append_data_member_raw(name, type_p, annotations.m_name);
        }
      else
        {
        m_context.m_obj_scope_p->append_data_member(name, type_p);
        }
      }
    else
      {
      const char * relation_cstr_p;

      // Duplicate data member was found
      if (dupe_class_p == current_class_p)
        {
        relation_cstr_p = "same class";
        result = Result_err_context_duped_data;
        }
      else
        {
        if (dupe_class_p->is_superclass(*current_class_p))
          {
          relation_cstr_p = "superclass";
          result = Result_err_context_duped_data_super;
          }
        else
          {
          relation_cstr_p = "subclass";
          result = Result_err_context_duped_data_sub;
          }
        }

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(1000u);
        ms_error_str.format(
          "Duplicate data member!\n"
          "The '%s' %s data member in the class '%s' is already present as "
          "a data member in the %s '%s'.",
          name.as_cstr_dbg(),
          class_expected_b ? "class" : "instance",
          current_class_p->get_name_cstr_dbg(),
          relation_cstr_p,
          dupe_class_p->get_name_cstr_dbg()
          );
      #endif
      }
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses data member accessor with specified owner expression.  Assumes any owner
// expression and initial '.' accessor has already been parsed. May also use implicit this
// as owner - see owner_p.
//
// #Notes
//   variable-ident = variable-name | ([expression ws '.' ws] data-name)
//   variable-name  = name-predicate
//   data-name      = '@' | '@@' variable-name
//   name-predicate = instance-name ['?']
//   instance-name  = lowercase {alphanumeric}
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkIdentifierMember data-structure otherwise nullptr is returned. 
  SkIdentifierLocal *
SkParser::parse_data_accessor(
  // See SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and result type on exit.
  //   m_result: Result_ok, Result_err_context_non_ident_member,
  //     Result_err_expected_instance, Result_err_unexpected_reserved,
  //     Result_err_unexpected_eof
  Args & args,
  // Expression that owns the data member or nullptr if implicit 'this' should be used.
  SkExpressionBase * owner_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get valid member identifier
  ASymbol   name;
  uint32_t  start_pos             = args.m_start_pos;
  bool      is_ident_class_member = false;
  bool      is_data_class_member  = false;
  eSkScope  data_member_scope     = SkScope__none;
  uint32_t  data_idx              = 0;
  SkClass * data_owner_class_p    = nullptr;

  args.m_result = parse_name_data_member(
    start_pos, &args.m_end_pos, &name, nullptr, &is_ident_class_member);

  if (!args.is_ok())
    {
    return nullptr;
    }

  if (m_flags.is_set_any(Flag_type_check))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure that the member exists and get the type.
    // Note that data members do not need to be captured when building a closure like
    // parameters and temporary variables since only their owner potentially needs to be
    // captured / made available.
    // $Revisit - CReis Since expected scope is already known (class_data_b) class and
    // instance versions of get_data_type() could be used wich would be a bit faster.
    SkClassDescBase * owner_type_p = args.m_expr_type;
    SkTypedName * type_p = owner_type_p->get_data_type(name, &data_member_scope, &data_idx, &data_owner_class_p);
    is_data_class_member = (data_member_scope == SkScope_class);

    if (type_p == nullptr || is_ident_class_member != is_data_class_member)
      {
      args.m_result = Result_err_context_non_ident_member;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The data member '%s' does not exist in %s.",
          name.as_cstr_dbg(),
          owner_type_p->get_scope_desc().as_cstr());
      #endif

      return nullptr;
      }

    args.m_expr_type.set(type_p->m_type_p->as_finalized_generic(*owner_type_p), data_member_scope == SkScope_instance_raw);

    // Since we boil down class member access to just the class pointer
    // the owner expression must be free of side effects since it will be thrown away
    if (is_data_class_member && owner_p && owner_p->get_side_effect() != SkSideEffect_none)
      {
      args.m_result = Result_err_context_side_effect;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Trying to access a class data member through an expression with a side effect (the compiler forgets the expression and stores just the class type, so expressions used to access the class member must not have side effects)."
          );
      #endif

      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create structure if desired
  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  SkIdentifierLocal * ident_p;
  switch (data_member_scope)
    {
    case SkScope_class:        ident_p = SK_NEW(SkIdentifierClassMember)(name, data_idx, data_owner_class_p); break;
    case SkScope_instance:     ident_p = SK_NEW(SkIdentifierMember)(name, data_idx, owner_p); break;
    case SkScope_instance_raw: ident_p = SK_NEW(SkIdentifierRawMember)(name, data_idx, owner_p, data_owner_class_p); break;
    default:                   ident_p = nullptr; SK_ERRORX("Unexpected data_member_scope!"); break;
    }

  SKDEBUG_SET_CHAR_POS(ident_p, start_pos);

  return ident_p;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to parse a number of data
//             members from a source string.
// Returns:    Result_ok, Result_err_expected_instance, Result_err_context_duped_data,
//             Result_err_context_duped_data_super, Result_err_context_duped_data_sub, or
//             Result_err_unexpected_eof 
// Arg         scope_p            - scope of parse
//             args               - see Args
//             append_to_class_b  - if true, the data members will be appended to the supplied
//                                  class scope.  If false, the data members are parsed/validated for correctness
//                                  only - no data-structures are created or modified.
// Examples:   if (parse.parse_data_members_source(class_p, args, is_class_data) == Result_ok)
// Notes:      data-file       = ws [data-definition {wsr data-definition} ws]
//             data-definition = [class-desc ws] '!' instance-name
// Author(s):   Conan Reis
bool SkParser::parse_data_members_source(
  SkClassUnaryBase  * scope_p,
  Args              & args,              // = ms_def_args.reset()
  bool                append_to_class_b, // = true
  uint32_t *          num_data_members_p // = nullptr
  )
  {
  uint32_t length = m_str_ref_p->m_length;

  reset_scope(scope_p, ASymbol_Data);

  A_LOOP_INFINITE
    {
    // Eat whitespace
    args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);
    if (!args.is_ok())
      {
      m_context.free_all_locals();
      return false;   //  Exit on error.
      }

    // Determine if the parse is at end of source.
    if (args.m_end_pos >= length)
      {
      m_context.free_all_locals();
      return true;    // Exit since at end of source.
      }
    args.m_start_pos = args.m_end_pos;

    args.m_result = parse_data_definition(args.m_start_pos, &args.m_end_pos, append_to_class_b);
    if (args.is_ok())
      {
      if (num_data_members_p)
        {
        ++*num_data_members_p;
        }
      }
    else
      {
      m_context.free_all_locals();
      return false;   // Exit on error.
      }
    args.m_start_pos = args.m_end_pos;
    }

    //  Should never get here.

  } //  SkParser::parse_data_members_source()


//---------------------------------------------------------------------------------------
// Parses newline terminated symbol name identifiers text.
//
// #Notes
//   Used by object IDs to validate names.
//
//   object-id-file = {ws symbol-literal | raw-object-id} ws
//   raw-object-id  = {printable}^1-255 newline | end-of-file
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // number of ids successfully parsed - see args.m_result to determine if an error was
  // encountered or not.
  uint32_t
SkParser::parse_symbol_ids_source(
  // Optional shared symbol table to *merge/union* symbol ids into.  It is an id "set" -
  // so if it already has a symbol it won't be added more than once.  Symbols are shared
  // with the main common/default symbol table.  If nullptr the symbols are just merged into
  // the main symbol table.
  ASymbolTable * ids_p,
  // args - see SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_unexpected_eof or Result_err_unexpected_char
  Args & args // = ms_def_args.reset()
  )
  {
  uint32_t id_count = 0u;

  ASymbol id_name;
  uint32_t    id_length;
  uint32_t    idx_end;
  uint32_t    idx    = args.m_start_pos;
  uint32_t    length = m_str_ref_p->m_length;
  char *  cstr_a = m_str_ref_p->m_cstr_p;

  A_LOOP_INFINITE
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(idx, &args.m_end_pos);

    if (!args.is_ok())
      {
      return id_count;
      }

    idx = args.m_end_pos;
    args.m_start_pos = idx;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if the parse is at end of source
    if (idx >= length)
      {
      // Exit - successful parse.
      return id_count;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if symbol-literal or raw-object-id
    if (cstr_a[idx] == '\'')
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // It is a symbol-literal
      args.m_result = parse_literal_symbol(idx, &args.m_end_pos, &id_name);

      if (!args.is_ok())
        {
        // Error parsing symbol literal
        return id_count;
        }

      idx = args.m_end_pos;
      args.m_start_pos = idx;
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // It is a raw-object-id
      // Raw ids are newline terminated, must have at least 1 character and may not have
      // leading whitespace (ws) or single quote (') nor contain a newline character.
      idx_end = length;
      
      // Find newline and check for DOS line ending
      if (find('\n', 1u, &idx_end, idx) && (cstr_a[idx_end - 1u] == '\r'))
        {
        idx_end--;
        }

      args.m_end_pos = idx_end;
      id_length = idx_end - idx;

      if (id_length > SkParser_ident_length_max)
        {
        // Symbol id is too long.
        args.m_result = Result_err_size_symbol;

        return id_count;
        }

      id_name = ASymbol::create(cstr_a + idx, id_length, ATerm_short);

      idx = idx_end;
      args.m_start_pos = idx_end;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Merge in symbol id.
    id_count++;

    #if defined(A_SYMBOL_STR_DB_AGOG)
      // $Revisit - CReis Deal with symbol tables when shared string references aren't available.
      // Merge symbol into shared id table if one given.
      if (ids_p)
        {
        ids_p->append_shared(id_name);
        }
    #endif
    }  // End of infinite loop
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an expression.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated data-structure derived from SkExpressionBase otherwise nullptr is
//             returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_unexpected_char,
//                 or pretty much any warning or error.
// Notes:      expression = identifier | invocation | literal
//             identifier = scoped-instance | class-name | reserved-ident
//             invocation = [expression wsr] (invoke-selector | invoke-cascade)
//             literal    = boolean-literal | integer-literal | real-literal | string-literal | symbol-literal | character-literal | list-literal
//             primitive  = bind | conditional | case | loop | class-conversion | class-cast | code-block
// Author(s):   Conan Reis
SkExpressionBase * SkParser::parse_expression(
  Args &        args,             // = ms_def_args.reset()
  eSkInvokeTime desired_exec_time // = SkInvokeTime_any
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure there are enough characters to parse
  uint32_t length    = m_str_ref_p->m_length;
  uint32_t start_pos = args.m_start_pos;

  if (start_pos >= length)
    {
    args.m_result  = Result_err_unexpected_eof;
    args.m_end_pos = start_pos;

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine type of expression
  uint32_t pos    = start_pos;
  char *   cstr_a = m_str_ref_p->m_cstr_p;
  char     ch     = cstr_a[pos];
  char     ch2    = ((pos + 1u) < length) ? cstr_a[pos + 1u] : '\0';

  bool simple_int_b = false;

  // Save desired context type
  SkClassDescBase * desired_type_p = args.m_desired_type_p;

  SkExpressionBase * expr_p = nullptr;

  // Test for an expression receiver/target/subject or infer an implicit 'this'

  switch (ch)
    {
    // Literals

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '\'':  // symbol
      {
      ASymbol sym;

      args.m_result = parse_literal_symbol(pos, &pos, args.is_struct_wanted() ? &sym : nullptr);

      if (args.is_ok())
        {
        args.m_expr_type = SkBrain::ms_symbol_class_p;

        if (args.is_struct_wanted())
          {
          expr_p = SK_NEW(SkLiteral)(sym);
          SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
          }
        }
      }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '"':   // string
      {
      AString str;

      args.m_result = parse_literal_string(pos, &pos, args.is_struct_wanted() ? &str : nullptr);

      if (args.is_ok())
        {
        args.m_expr_type = SkBrain::ms_string_class_p;

        if (args.is_struct_wanted())
          {
          expr_p = SK_NEW(SkLiteral)(str);
          SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
          }
        }
      }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '{':   // List
      expr_p = parse_literal_list(args);
      pos    = args.m_end_pos;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '^':   // Closure
    case '(':
      expr_p = parse_closure(args);
      pos    = args.m_end_pos;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '@':   // Data member with implicit this or Object ID
      if ((ch2 == '\'') || (ch2 == '?') || (ch2 == '#'))
        {
        // Object Id
        expr_p = parse_object_id_tail(args);
        }
      else
        {
        // Data member with implicit 'this'
        // Set owner / this type.
        args.m_expr_type = m_context.m_obj_scope_p;
        args.m_receiver_type_p = m_context.m_obj_scope_p;
        expr_p = parse_data_accessor(args, nullptr);
        }

      pos = args.m_end_pos;

      if (expr_p)
        {
        SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
        }
      break;


    // Mixed

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '_':   // Coroutine call
      // Intermediary parser state that assumes/infers an implicit 'this' prior to an
      // invocation unless a better expression receiver/target/subject is found.
      args.m_result    = Result__implicit_this;
      args.m_expr_type = m_context.m_obj_scope_p;
      break;

    // Primitives

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '[':   // Code Block
      expr_p = parse_code_block_optimized(args);
      pos    = args.m_end_pos;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '+':   // + ++ +=
    case '*':   // * *=
    case '/':   // / /=
    case '=':   // =
    case '<':   // < <=
    case '>':   // > >=
    case '~':   // ~=    [~ ~& ~| ~^ deprecated]

    case '&':   // [& deprecated]
    case '|':   // [| deprecated]
      // Operators not allowed to use implicit 'this'
      args.m_result = Result_err_unexpected_implicit_this;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '!':   // constructor call, or destructor call
      // Note that 'create temporary variable' is a statement not an expression.

      if (desired_type_p && (desired_type_p->get_class_type() != SkClassType_class_union))
        {
        // If desired type is known, try to infer class to instantiate
        args.m_expr_type          = desired_type_p;
        args.m_receiver_type_p    = desired_type_p;
        SkMethodCallBase * ctor_p = parse_invoke_ctor(args);

        if (ctor_p)
          {
          expr_p = SK_NEW(SkInstantiate)(desired_type_p->get_key_class(), ctor_p);
          SKDEBUG_SET_CHAR_POS(expr_p, pos);
          }

        pos = args.m_end_pos;

        // Set result class type
        if (m_flags.is_set_any(Flag_type_check))
          {
          args.m_expr_type = desired_type_p;
          }
        }
      else
        {
        // Intermediary parser state that assumes/infers an implicit 'this' prior to an
        // invocation unless a better expression receiver/target/subject is found.
        args.m_result    = Result__implicit_this;
        args.m_expr_type = m_context.m_obj_scope_p;
        }

      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ':':   // := or invalid character
      // Bind : and assignment := cannot have an implicit "this".
      args.m_result = Result_err_unexpected_implicit_this;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '-':   // - -- -= binary op, - negation prefix op or number literal
      if ((ch2 == '.') || ms_is_digit[uint8_t(ch2)])
        {
        // Real or Integer literal
        args.m_start_pos = pos;
        expr_p = parse_literal_number(args, &simple_int_b);
        pos = args.m_end_pos;
        break;
        }

      if (!ms_is_space[uint8_t(ch2)] && (ch2 != '-') && (ch2 != '='))
        {
        // - negation prefix operator
        args.m_start_pos = pos + 1u;
        expr_p = parse_prefix_operator_expr(ASymbol_negated, args);
        pos = args.m_end_pos;
        break;
        }

      // - -- -= binary operator
      args.m_result = Result_err_unexpected_implicit_this;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '.':   // Real literal
      args.m_start_pos = pos;
      expr_p = parse_literal_number(args, &simple_int_b);
      pos = args.m_end_pos;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    default:
      // Integer or Real
      if (ms_is_digit[uint8_t(ch)])
        {
        args.m_start_pos = pos;
        expr_p = parse_literal_number(args, &simple_int_b);
        pos = args.m_end_pos;
        }
      else  // Look for expressions that start with a letter
        {
        // Next possible error
        args.m_result = Result_err_expected_expression;

        if (ms_char_match_table[ACharMatch_alphabetic][uint8_t(ch)])  // If A-Z, a-z, European characters
          {
          // Check for: Identifier, invoke series, or binding (:)
          // Also look for reserved words: true, false, loop, this, this_class, this_code, nil
          args.m_start_pos = pos;
          expr_p = parse_expression_alpha(args);
          pos = args.m_end_pos;
          }
        }
    }  // switch end

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for parse to index probe BEFORE we check for errors
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    // We just found a new receiver, store if no errors occurred
    if (args.m_result <= Result__implicit_this)
      {
      args.m_receiver_type_p = (args.m_result == Result__implicit_this) ? nullptr : args.m_expr_type;
      }

    if (args.m_result != Result__implicit_this)
      {
      //args.m_start_pos = start_pos;
      args.m_end_pos = pos;

      if (args.is_idx_probe_halt(this))
        {
        // Found probe index, so exit
        delete expr_p;

        return nullptr;
        }
      }
    }

  // If result is not ok or implicit "this" exit.
  if (args.m_result > Result__implicit_this)
    {
    SK_ASSERTX(!expr_p, "Expression leak!");

    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for string of nesting/stringized expressions following receiver expression
  // (or inferred 'this') to merge into a single expression.
  Args strung_args(args);

  strung_args.m_start_pos = pos;

  // strung_args.m_type_p needs to be the class type of the receiver - which should
  // have been previously set when the receiver was parsed.  If "this" is inferred,
  // set receiver type to the current scope.
  if (args.m_result == Result__implicit_this)
    {
    strung_args.m_expr_type = m_context.m_obj_scope_p;
    }

  SkExpressionBase * invocation_p = parse_expression_string(strung_args, &expr_p);

  // Determine if parse advanced
  if (pos != strung_args.m_end_pos)
    {
    // Yes, check for error
    if (strung_args.is_ok())
      {
      // No error - accept result
      expr_p = invocation_p;
      }
    else if (expr_p)
      {
      // Error - forget expression
      delete expr_p;
      expr_p = nullptr;
      }

    args = strung_args;
    pos = strung_args.m_end_pos;
    }
  else
    {
    // There was no invocation, so just use the expression by itself
    if (args.m_result != Result__implicit_this)
      {
      // Successful invocation without strung invocation.
      args.m_result = Result_ok;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Switch Integer literal to Real literal if desired context type is Real
      if (simple_int_b && (desired_type_p == SkBrain::ms_real_class_p))
        {
        args.m_expr_type = SkBrain::ms_real_class_p;

        if (expr_p)
          {
          SkLiteral * literal_p = static_cast<SkLiteral *>(expr_p);

          SkLiteral::UserData data = literal_p->get_data();
          tSkInteger int_num = *data.as<tSkInteger>();
          tSkReal real_num = tSkReal(int_num);
          data.set(real_num);
          literal_p->set(SkLiteral::Type_real, data);
          }
        }
      }
    else
      {
      args.m_result = Result_err_expected_expression;
      }
    }

  // Note end of expression
  args.m_end_pos = pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure desired execution time
  if (expr_p && !ensure_exec_time(*expr_p, args, desired_exec_time))
    {
    args.m_start_pos = start_pos;
    args.m_end_pos   = pos;

    delete expr_p;
    expr_p = nullptr;
    }

  #if SK_PARSER_CHECK_RAW_ACCESS
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure raw data is not returned from any expression
    if (args.is_ok() && args.m_expr_type.is_raw_access())
      {
      args.m_result = Result_err_context_raw_access;

      if (expr_p)
        {
        delete expr_p;
        expr_p = nullptr;
        }
      }
  #endif

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an expression subject/target/receiver that begins with a
// letter.  Check for: Identifier or binding (:) Also look for an identifier (including:
// this, this_class, this_code, or nil), a literal (one of: true or false), or a
// primitive (loop) [`exit` already looked for by parse_statement_append()]
// 
// # Returns:
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically
//   allocated data-structure derived from SkExpressionBase otherwise nullptr is returned.
//   
// # Params:
//   args: see SkParser::Args, specific considerations below:
//     m_result: Result_ok, Result_err_unexpected_eof, or pretty much any warning or error.
//     
// # Examples:  called by parse_expression()
// # Modifiers: protected
// # Author(s): Conan Reis
SkExpressionBase * SkParser::parse_expression_alpha(Args & args) const
  {
  uint32_t start_pos = args.m_start_pos;
  uint32_t pos       = start_pos;
  uint32_t end_pos   = start_pos + 1u;  // The first character was already tested

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find end of identifier name
  // Using find() rather than parse_name_instance(), parse_name_predicate() or
  // parse_name_symbol() since the full context is not yet known.
  find(ACharMatch_not_identifier, 1u, &end_pos, end_pos);

  SkExpressionBase * expr_p = nullptr;
  char *             cstr_a = m_str_ref_p->m_cstr_p;
  char               end_ch = cstr_a[end_pos];

  // Intermediary parser state that assumes/infers an implicit 'this' prior to an
  // invocation unless a better expression receiver/target/subject is found.
  args.m_result = Result__implicit_this;

  if (ms_is_uppercase[uint8_t(cstr_a[start_pos])])
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Starts with an uppercase letter - this is a class name.  It could be:
    //   - class identifier (with optional param list for invokable classes)
    //   - class scope
    //   - instantiation
    //   - typed literal (List)

    // Peek behind any white space
    uint32_t ws_end_pos;
    if (parse_ws_any(end_pos, &ws_end_pos) == Result_ok)
      {
      // Class identifier with param list?
      char ws_end_ch = cstr_a[ws_end_pos];
      if (ws_end_ch == '(' || ws_end_ch == '+' || ws_end_ch == '_')
        {
        return parse_instantiate_or_list(args);
        }
      }

    switch (end_ch)
      {
      case '@':  // Class scope or Object ID
        {
        char end_ch2 = ((end_pos + 1u) < m_str_ref_p->m_length) ? cstr_a[end_pos + 1u] : '\0';

        if ((end_ch2 == '?') || (end_ch2 == '\'') || (end_ch2 == '#'))
          {
          // It is an Object ID
          SkClass * class_p = nullptr;
        
          args.m_result = parse_class(pos, &pos, &class_p);

          if (class_p)
            {
            args.m_start_pos = pos;
            expr_p = parse_object_id_tail(args, class_p);
            pos    = args.m_end_pos;
            }
          }
        // else
          // It is a class scope - do nothing: class scopes are handled in
          // parse_expression_string() assuming an implicit "this" receiver.
        break;
        }

      case '!':
      case '{':
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is an instantiation or typed list literal
        return parse_instantiate_or_list(args);

      default:
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is a Class identifier
        SkClass * class_p = nullptr;
        
        args.m_result = parse_class(pos, &pos, &class_p);

        if (class_p)
          {
          SkMetaClass & mclass = class_p->get_metaclass();

          args.m_expr_type = &mclass;

          if (args.is_struct_wanted())
            {
            expr_p = SK_NEW(SkLiteral)(mclass);
            // Debug character position is set at end of this method
            }
          }
        }  // end default clause
      }  // end switch (end_ch)
    }
  else 
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Starts with a lowercase letter, could be:
    //   - an identifier (a custom identifier, this, this_class, this_code, or nil)
    //   - a literal (true or false)
    //   - a primitive (such as: loop)
    //   - a bind (:)
    //   - a method_call()
    // Note: 'exit' already looked for by parse_statement_append()

    // If treated as invocation() and type info is available try to treat it as a data
    // member first otherwise treat it as a routine call.

    bool predicate = false;

    // Is it a predicate (and not a nil coalescing op)?
    if ((end_ch == '?') && (cstr_a[end_pos + 1u] != '?'))
      {
      end_pos++;
      end_ch = cstr_a[end_pos];
      predicate = true;
      }

    // Find identifier/token
    ASymbol token;
    bool    token_test_b = !predicate;

    pos = end_pos;

    if (m_flags.is_set_any(Flag_type_check) || (end_ch != '('))
      {
      token_test_b = true;

      token = as_symbol(start_pos, end_pos);

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine if it is a prefix operator
      switch (token.get_id())
        {
        case ASymbolId_not:
          if (end_ch != '(')
            {
            // It is a logical 'not' operator
            args.m_start_pos = pos;
            expr_p = parse_prefix_operator_expr(token, args);
            pos    = args.m_end_pos;
            token_test_b = false;
            break;
            }
        }
      }

    if (token_test_b)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine if it is a reserved word.
      bool reserved_word_b = !predicate
        && is_ident_reserved(token.get_id());

      if (reserved_word_b)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is an identifier, literal, or primitive based on a reserved word

        args.m_result = Result_ok;

        switch (token.get_id())
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_true:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(true);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = SkBrain::ms_boolean_class_p;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_false:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(false);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = SkBrain::ms_boolean_class_p;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_if:
            args.m_start_pos = pos;
            expr_p = parse_conditional_tail(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_case:
            args.m_start_pos = pos;
            expr_p = parse_case_tail(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_else:
            args.m_result = Result_err_unexpected_else_statement;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_unless:
            args.m_result = Result_err_unexpected_unless_statement;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_when:
            args.m_result = Result_err_unexpected_when_statement;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_loop:
            args.m_start_pos = pos;
            expr_p = parse_loop_tail(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_exit:
            // Rather than being parsed here as an expression, "exit" statements are
            // parsed in parse_statement_append() which calls parse_loop_exit().
            pos -= 4u;
            args.m_result = Result_err_unexpected_exit;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_this:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__this);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = m_context.m_obj_scope_p;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_this_class:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__this_class);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = &m_context.m_obj_scope_p->get_metaclass();
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_this_code:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__this_code);
              // Debug character position is set at end of this method
              }

            // Set type
            switch (m_member_type)
              {
              case SkMember_method:
                args.m_expr_type = SkBrain::ms_invoked_method_class_p;
                break;

              case SkMember_coroutine:
                args.m_expr_type = SkBrain::ms_invoked_coroutine_class_p;
                break;

              default:
                args.m_expr_type = SkNone::get_class();
              }
            break;

          case ASymbolId_this_mind:
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__this_mind);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = SkBrain::ms_mind_class_p;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_sync:
            // Concurrent sync/converge flow statement - sync [ _expr1() _expr2() _expr3() ]
            // Call each statement in code block  simultaneously and do not call the next
            // statement until the *last* statement - i.e. all the statements - in code
            // block are complete.
            args.m_start_pos = end_pos;
            expr_p = parse_concurrent_sync_block(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_race:
            // Concurrent racing expressions - race [ _expr1() _expr2() _expr3() ]
            // Call each statement in code block simultaneously and do not call the next
            // statement until the *first* statement is complete.
            args.m_start_pos = end_pos;
            expr_p = parse_concurrent_race_block(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_branch:
            // Concurrent branch flow statement - branch [ _expr() ]
            // Call block at the same time as next statement, but do not wait for block to
            // return.
            args.m_start_pos = end_pos;
            expr_p = parse_concurrent_branch_block(args);
            pos    = args.m_end_pos;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          case ASymbolId_change:
            // change statement - change [ws expression] ws expression
            // Call change updater mind for trailing expression
            args.m_start_pos = end_pos;
            expr_p = parse_change_mind(args);
            pos    = args.m_end_pos;
            break;

          case ASymbolId_defer:
          case ASymbolId_random:
          case ASymbolId_rush:
          case ASymbolId_skip:
            args.m_result = Result_err_unimplemented;
            break;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          default:  // ASymbolId_nil
            if (args.is_struct_wanted())
              {
              expr_p = SK_NEW(SkLiteral)(SkLiteral::Type__nil);
              // Debug character position is set at end of this method
              }

            args.m_expr_type = SkNone::get_class();
          }
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is a variable identifier not based on a reserved word
        // Assume method call when not type-checking and invocation brackets are present.
        uint32_t data_idx = 0;
        bool var_exists_b = false;
        bool var_duped_b = false;
        if (m_flags.is_set_any(Flag_type_check))
          {
          SkTypedNameIndexed * dupe_p = nullptr;
          SkTypedNameIndexed * var_p = m_context.find_local_variable(token, &dupe_p);
          if (var_p)
            {
            data_idx = var_p->m_data_idx;
            var_exists_b = true;
            var_duped_b = (dupe_p != nullptr);
            }
          }
        else
          {
          var_exists_b = (end_ch != '(');
          }

        if (var_duped_b)
          {
          // Multiple copies of the variable were found
          args.m_result = Result_err_context_duped_variable;

          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "The '%s' variable name is duped!\n"
              "Different variables with the same name may not exist in the same scope (shadowing).\n\n"
              "One possible solution is to rename one of the variables. For nested closures that have a "
              "variable name auto-supplied you may have to specify the parameters for a closure with a "
              "different name than the default.",
              token.as_cstr_dbg());
          #endif
          }
        else if (var_exists_b)
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is a simple parameter/temporary variable identifier
          args.m_result = Result_ok;

          if (args.is_struct_wanted())
            {
            // Create identifier and remember it
            SkIdentifierLocal * ident_p = SK_NEW(SkIdentifierLocal)(token, data_idx);
            m_context.on_identifier_created(ident_p);
            expr_p = ident_p;
            // Debug character position is set at end of this method
            // $Revisit - CReis [A_NOTE] ***Enhancement*** - [Auto-Type] If desired type specified and identifier is Auto_ param type then constrain to desired type.

            if (m_flags.is_set_any(Flag_type_check))
              {
              args.m_expr_type = m_context.get_local_variable_type(token);
              }
            }
          }
        else
          {
          pos = start_pos;

          // If it is not a method invocation or call operator (which will be handled in
          // parse_expression_string() assuming an implicit "this" receiver).
          if ((end_ch != '(') && (!m_context.m_obj_scope_p->is_method_inherited_valid(token)))
            {
            args.m_result = Result_err_context_non_identifier;
            pos = end_pos;

            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "The identifier '%s' does not exist in the current scope.",
                token.as_cstr_dbg());

              // Check for specific C++ etc. keyword/identifier mistakes and assumptions.
              switch (token.get_id())
                {
                case ASymbolId_break:
                  ms_error_str.append("\n[SkookumScript uses blocks to group statements and 'exit' to break out of loops.]");
                  break;

                case ASymbolId_continue:
                  ms_error_str.append("\n[Use flow control and logic tests to do early repeats of a loop and use 'exit' to break out of the loop.  SkookumScript has a planned future command 'skip' to have a loop restart.]");
                  break;

                case ASymbolId_default:
                  ms_error_str.append("\n[SkookumScript uses 'else' to denote a alternate/default/otherwise block.]");
                  break;

                case ASymbolId_do:
                case ASymbolId_for:
                case ASymbolId_while:
                  ms_error_str.append(
                    "\nSkookumScript uses 'loop' and a nested 'exit' to do traditional iteration:\n"
                    "  loop\n"
                    "    [\n"
                    "    do_stuff()\n"
                    "    if exit_test() [exit]\n"
                    "    ]\n\n"
                    "Also see the *do*() routines found in Integer, List and elsewhere.");
                  break;

                case ASymbolId_return:
                  ms_error_str.append("\n[SkookumScript automatically returns the result of the last expression in any code block and all expressions/statements return a value.  There is no explicit 'return' statement though 'exit' is used to leave a loop.]");
                  break;

                case ASymbolId_switch:
                  ms_error_str.append("\n[SkookumScript uses 'case' for multi-path flow control - rather than 'switch', 'select', 'inspect', 'given', '?""?', etc.]"); // Double quote to disable trigraph
                  break;
                }
            #endif
            }
          }
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Store character position for debugging
  #if (SKOOKUM & SK_DEBUG)
    if (expr_p)
      {
      SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
      }
  #endif

  args.m_end_pos = pos;

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an instantiation or a list literal.
//             [Instantiations and list literals have an identical syntax except that a
//             list literal also specifies initial elements.  This method creates
//             whichever one is found with no backtracking.]
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkInstantiate data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, or pretty much any other
//               warning or error.
// See:        parse_literal_list(), parse_instantiation()
// Notes:      instantiation = [class-instance] constructor-name invocation-args
//             list-literal  = [(list-class constructor-name invocation-args) | class]
//                             '{' ws [expression {ws ',' ws expression} ws] '}'
// Author(s):   Conan Reis
SkExpressionBase * SkParser::parse_instantiate_or_list(
  Args & args // = ms_def_args.reset()
  ) const
  {
  SkExpressionBase * expr_p      = nullptr;
  uint32_t           start_pos   = args.m_start_pos;
  SkClassUnaryBase * class_p     = nullptr;
  bool               item_type_b = false;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse class to instantiate
  args.m_result = parse_class_instance(start_pos, &args.m_end_pos, &class_p, &item_type_b);

  if (!args.is_ok())
    {
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for simple list Type{...}
  if (m_str_ref_p->m_cstr_p[args.m_end_pos] == '{')
    {
    args.m_start_pos = args.m_end_pos;
    // Note that while a simple list can specify an item class, the list itself may only
    // be of type `List` and not a subclass.
    return parse_literal_list(
      args, SkTypedClass::get_or_create(SkBrain::ms_list_class_p, class_p), true);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse constructor method call
  args.m_expr_type = class_p;
  args.m_receiver_type_p = class_p;
  args.m_start_pos = args.m_end_pos;

  SkMethodCallBase * ctor_p = parse_invoke_ctor(args);

  if (!args.is_ok())
    {
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Can now differentiate between instantiation and list literal
  if ((m_str_ref_p->m_cstr_p[args.m_end_pos] != '{')
    || (class_p->get_class_type() == SkClassType_class))
    {
    // It is an instantiation
    if (ctor_p)
      {
      expr_p = SK_NEW(SkInstantiate)(class_p->get_key_class(), ctor_p);
      SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
      }

    // Set result class type
    if (m_flags.is_set_any(Flag_type_check))
      {
      args.m_expr_type = class_p;
      }
    }
  else
    {
    // Optimize out redundant constructor call
    if (ctor_p
      && (ctor_p->get_name() == ASymbolX_ctor)
      && (ctor_p->get_args().is_empty()))
      {
      // It is the default constructor without any arguments, so there is no need for
      // it to be called when the list is created.
      delete ctor_p;
      ctor_p = nullptr;
      }

    args.m_start_pos = args.m_end_pos;
    expr_p = parse_literal_list(args, static_cast<SkTypedClass *>(class_p), item_type_b, ctor_p);
    }

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an expression with a receiver/owner potentially leading to
// a string/nesting of expressions.
// 
// #Notes
//   The initial receiver/owner expression (or implied 'this') must be supplied.
//   
//   variable-ident   = expression ws '.' ws data-name)
//   data-name        = '@' | '@@' variable-name
//   invocation       = invoke-call | invoke-cascade | invoke-apply | instantiation
//   invoke-call      = ([expression ws '.'] invoke-selector) | operator-call
//   invoke-cascade   = expression ws '.' ws '[' {ws invoke-selector | operator-selector}2+ ws ']'
//   invoke-apply     = expression ws '%' invoke-selector
//   class-conversion = expression ws '>>' [class]
//   class-cast       = expression ws '<>' [class-desc]
//   invoke-selector  = method-call | coroutine-call
//   method-call      = [scope] method-name invocation-args
//   coroutine-call   = [scope] coroutine-name invocation-args
//
// #Author(s)   Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // data-structure derived from SkExpressionBase otherwise nullptr is returned.
  SkExpressionBase * SkParser::parse_expression_string(
  // See SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and changed to result type on exit.
  //   m_result: Result_ok, Result_err_unexpected_char, or pretty much any warning or error.
  Args & args,
  // The initial receiver/owner expression or inferred receiver (such as implied 'this').
  SkExpressionBase ** receiver_pp
  ) const
  {
  bool              implicit_this_b = args.m_result == Result__implicit_this;
  uint32_t          start_pos       = args.m_start_pos;
  SkClassDescBase * desired_type_p  = args.m_desired_type_p;
  SkInvokeBase *    call_p          = nullptr;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  uint32_t ws_pos = start_pos;
  args.m_result = parse_ws_any(ws_pos, &args.m_end_pos);

  if (!args.is_ok())
    {
    return nullptr;
    }

  // Not all expression strings allow leading whitespace so note if some is present.
  bool leading_ws_b = ws_pos != args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Simple invoke selectors can be called on an implicit this
  uint32_t pos        = args.m_end_pos;
  uint32_t invoke_pos = pos;   // Start of Nth invocation "sub-string"

  if (implicit_this_b)
    {
    // Indicate that there is an implicit receiver.
    args.m_result    = Result__implicit_this;
    args.m_start_pos = pos;
    call_p = parse_invoke_selector(args, false, receiver_pp);
    pos = args.m_end_pos;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Start successively nesting invocation sub-strings around initial receiver
  char     next_ch;
  bool     parse_op_b;                // true if should test for operator invocation
  bool     raw_access_b     = false;  // true if receiver has raw access
  bool     strung_b         = false;  // true if any invocations have been nested around initial receiver
  uint32_t length           = m_str_ref_p->m_length;
  char *   cstr_a           = m_str_ref_p->m_cstr_p;

  SkExpressionBase * temp_recv_p = nullptr;
  SkExpressionBase * recv_p      = *receiver_pp;  // Receiver built up by successively nesting invocation sub-strings

  #if SK_PARSER_CHECK_RAW_ACCESS
    bool bad_raw_access_b = false;  // true if illegal raw access happened
  #endif

  do
    {
    if (!implicit_this_b)
      {
      invoke_pos       = pos;
      parse_op_b       = false;
      raw_access_b     = args.m_expr_type.is_raw_access();
      #if SK_PARSER_CHECK_RAW_ACCESS
        bad_raw_access_b = raw_access_b; // by default it's bad if the receiver is a raw data member
      #endif
      call_p           = nullptr;
      temp_recv_p      = nullptr;
      next_ch          = ((pos + 1u) < length) ? cstr_a[pos + 1u] : '\0';

      switch (cstr_a[pos])
        {
        case '.':
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is a Real literal, a data member access, an invoke cascade or an invoke
          // call (method or coroutine)
          
          if (ms_is_digit[uint8_t(cstr_a[pos + 1u])])
            {
            // Seems to be a Real literal
            args.m_result = Result_err_expected_invoke_selector;
            break;
            }

          // Eat {whitespace}
          args.m_result = parse_ws_any(pos + 1u, &pos);

          if (!args.is_ok())
            {
            // exit switch
            break;
            }

          next_ch = cstr_a[pos];

          switch (next_ch)
            {
            case '@':
              //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              // It is a data member
              args.m_start_pos = pos;
              temp_recv_p      = parse_data_accessor(args, recv_p);
              pos              = args.m_end_pos;
              break;

            case '[':
              //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              // It is an invoke cascade
              args.m_start_pos = pos;
              temp_recv_p      = parse_invoke_cascade(args, recv_p);
              pos              = args.m_end_pos;
              break;

            default:
              //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              // Determine if invoke call
              args.m_start_pos = pos;
              call_p = parse_invoke_selector(args, false);
              pos = args.m_end_pos;
            }
          break;
          }

        case '!':
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Object instantiation operator
          // Needs to be immediately following receiver expression without any whitespace.
          if (leading_ws_b)
            {
            // Must have whitespace so no advance - check for an operator call
            parse_op_b = true;
            break;
            }

          bool is_raw_redundant_copy = false;
          args.m_start_pos = pos;
          temp_recv_p = parse_invoke_instantiate(args, recv_p, &is_raw_redundant_copy);
          pos = args.m_end_pos;

          #if SK_PARSER_CHECK_RAW_ACCESS
            // If raw data was detected and passed through, it's ok now
            if (is_raw_redundant_copy)
              {
              args.m_expr_type.clear_raw_access();
              bad_raw_access_b = false;
              }
          #endif
          }
          break;

        case ':':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is an assignment operator (:=)
          if (next_ch == '=')
            {
            // Test for operator (:=)
            parse_op_b = true;
            }
          else
            {
            // It is a bind
            args.m_start_pos      = pos;
            args.m_desired_type_p = desired_type_p;
            temp_recv_p = parse_bind(args, recv_p);
            pos = args.m_end_pos;
            }
          break;

        case '%':
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Is it an apply operator?
          if (next_ch == '>')
            {
            // It is an apply race operator
            args.m_start_pos = pos;
            temp_recv_p = parse_invoke_race(args, recv_p);
            pos         = args.m_end_pos;
            }
          else 
            {
            // It is an apply operator
            args.m_start_pos = pos;
            temp_recv_p = parse_invoke_apply(args, recv_p);
            pos         = args.m_end_pos;
            }
          break;
          }

        case '?':
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          if (next_ch == '?')
            {
            // It is a nil coalescing operator (??)
            args.m_start_pos = pos;
            temp_recv_p = parse_nil_coalescing_tail(args, recv_p);
            pos         = args.m_end_pos;
            }
          else
            {
            args.m_result = Result_err_expected_invoke_selector;
            }
          break;
          }

        case '-':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Operator starting with '-'?
          // If just '-' (and not '--' or '-=') it is a negative number or negation
          // operator on an expression if there is space before and no space after the '-'.
          if (leading_ws_b && (next_ch != '-') && (next_ch != '=')
            && !ms_is_space[uint8_t(next_ch)])
            {
            // It is an expression that starts with '-' like a negative number or negation
            // prefix operator rather than a subtract binary operator.
            args.m_result = Result_err_expected_invoke_selector;
            break;
            }

          parse_op_b = true;
          break;

        case '<':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is a cast '<>' or an operator starting with '<'
          if (next_ch == '>')
            {
            // It is a cast operator
            args.m_start_pos      = pos;
            args.m_desired_type_p = desired_type_p;
            temp_recv_p = parse_class_cast(args, recv_p);
            pos         = args.m_end_pos;
            }
          else
            {
            parse_op_b = true;
            }
          break;

        case '>':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is a conversion '>>' or an operator starting with '>'
          if (next_ch == '>')
            {
            // It is a conversion operator
            args.m_start_pos      = pos;
            args.m_desired_type_p = desired_type_p;
            temp_recv_p = parse_class_conversion(args, recv_p);
            pos         = args.m_end_pos;
            }
          else
            {
            parse_op_b = true;
            }
          break;

        case '(':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It is an invoke operator, call typo or bad spacing before closure
          if (!leading_ws_b)
            {
            args.m_start_pos = pos;
            temp_recv_p = parse_invoke_operator(args, recv_p);
            pos         = args.m_end_pos;
            }
          else
            {
            args.m_result = Result_err_expected_invoke_selector;
            }
          break;

        case '{':
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          if (!leading_ws_b)
            {
            // It is an index or slice operator
            args.m_start_pos = pos;
            temp_recv_p = parse_invoke_index_operator(args, recv_p);
            pos         = args.m_end_pos;
            }
          else
            {
            args.m_result = Result_err_expected_invoke_selector;
            }
          break;

        default:
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Check for statement modifier
          args.m_start_pos = pos;
          temp_recv_p = parse_modifier_tail(args, recv_p);

          if (!args.is_ok() && (args.m_end_pos == pos))
            {
            // Didn't advance - check for an operator call
            args.m_result = Result_ok;
            parse_op_b = true;
            }

          pos = args.m_end_pos;
        } // end switch

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Look for an operator call if requested
      if (parse_op_b)
        {
        args.m_start_pos = pos;
        call_p = parse_operator_call(args);
        pos    = args.m_end_pos;
        }
      }  // if implicit this

    // The implicit this will have been wrapped by an invocation call at this point or
    // there was a parse error.
    implicit_this_b = false;

    if (!args.is_ok())
      {
      // Determine if parse advanced last invoke stringization
      if (pos == invoke_pos)
        {
        pos = ws_pos;

        if (strung_b)
          {
          args.m_result = Result_ok;
          }
        }

      // exit do-while loop
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Check for bad raw data access
    #if SK_PARSER_CHECK_RAW_ACCESS
      if (bad_raw_access_b)
        {
        args.m_result = Result_err_context_raw_access;

        // Clean out various temporaries
        if (temp_recv_p)
          {
          recv_p = temp_recv_p;
          strung_b = true;
          }
        if (call_p)
          {
          delete call_p;
          }

        break;
        }
    #endif
     
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Was anything found?
    if (temp_recv_p || call_p)
      {
      // Test for parse to index probe
      if (args.m_flags & ArgFlag_parse_to_idx_probe)
        {
        // We just found a new receiver
        args.m_receiver_type_p = args.m_expr_type;
        args.m_end_pos = pos;

        if (args.is_idx_probe_halt(this))
          {
          // Found probe index, so exit
          if (recv_p && (recv_p != *receiver_pp))
            {
            // Keep original receiver
            recv_p->null_receiver(*receiver_pp);
            delete recv_p;
            }

          return nullptr;
          }
        }
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Make any sub-string invocation structure and get ready for next invocation sub-string
    SkExpressionBase * last_recv_p = recv_p;

    if (temp_recv_p)
      {
      recv_p = temp_recv_p;
      }
    else if (call_p)
      {
      // Check for special case: assignment to or invocation on raw data member
      if (raw_access_b)
        {
        SK_ASSERTX(recv_p && recv_p->get_type() == SkExprType_identifier_raw_member, "Internal error: Inconsistent receiver access!");

        SkIdentifierRawMember * ident_p = static_cast<SkIdentifierRawMember *>(recv_p);
        SkRawMemberInfo ident_info(ident_p->get_owner_class(), ident_p->get_data_idx());

        // Create owner cascade if any
        AVCompactArray<SkRawMemberInfo> raw_owner_cascade;
        SkExpressionBase * cascade_owner_p = ident_p;
        SkExpressionBase * cascade_owner_parent_p = ident_p->get_owner_expr();
        // Delete original raw identifier expression
        cascade_owner_p->null_receiver(cascade_owner_parent_p); // Make sure cascade_owner_parent_p won't die along with cascade_owner_p
        delete cascade_owner_p;
        if (cascade_owner_p == *receiver_pp) *receiver_pp = nullptr;
        // Build cascade array
        for (cascade_owner_p = cascade_owner_parent_p;
              cascade_owner_p && cascade_owner_p->get_type() == SkExprType_identifier_raw_member;
              cascade_owner_p = cascade_owner_parent_p)
          {
          SkIdentifierRawMember * cascade_owner_ident_p = static_cast<SkIdentifierRawMember *>(cascade_owner_p);
          raw_owner_cascade.insert(SkRawMemberInfo(cascade_owner_ident_p->get_owner_class(), cascade_owner_ident_p->get_data_idx()));
          cascade_owner_parent_p = cascade_owner_ident_p->get_owner_expr();
          // After insertion into cascade, delete original raw identifier expression
          cascade_owner_p->null_receiver(cascade_owner_parent_p); // Make sure cascade_owner_parent_p won't die along with cascade_owner_p
          delete cascade_owner_p;
          if (cascade_owner_p == *receiver_pp) *receiver_pp = nullptr;
          }

        // Is it a vanilla assignment?
        if (call_p->get_name() == ASymbol_assign)
          {
          // Insert SkRawMemberAssignment expression to store result into raw data member
          // Throw away the assignment operator call, and instead directly assign its argument
          SkExpressionBase * value_expr_p = call_p->get_args()[SkArg_1];
          recv_p = SK_NEW(SkRawMemberAssignment)(cascade_owner_parent_p, ident_info, raw_owner_cascade, value_expr_p);

          // Delete orphaned method call data structure
          call_p->null_arg1(value_expr_p); // Make sure value_expr_p won't die along with call_p
          delete call_p;
          }
        else
          { 
          // Insert SkRawMemberModifyingInvocation expression to invoke modifying method/coroutine on raw data member
          recv_p = SK_NEW(SkRawMemberModifyingInvocation)(cascade_owner_parent_p, ident_info, raw_owner_cascade, call_p);
          }

        last_recv_p = cascade_owner_parent_p;
        }
      else
        {
        // Generic call to method
        recv_p = SK_NEW(SkInvocation)(call_p, recv_p);
        }
      SKDEBUG_SET_CHAR_POS(recv_p, invoke_pos);
      }

    strung_b = true;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure receiver is immediate completion
    if (last_recv_p 
     && recv_p->get_type() != SkExprType_when
     && recv_p->get_type() != SkExprType_unless
     && !ensure_exec_time(*last_recv_p, args, SkInvokeTime_immediate))
      {
      pos              = invoke_pos;
      args.m_start_pos = start_pos;

      // Just to make invoke_pos and pos different
      invoke_pos = pos + 1u;
      }
 
    if (!args.is_ok())
      {
      // exit do-while loop
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    ws_pos = pos;
    args.m_result = parse_ws_any(ws_pos, &pos);
    leading_ws_b = pos != ws_pos;

    // If good parse continue looking for invocation sub-strings
    }
  while (args.is_ok());


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Good parse if any invocations were strung on and last "sub-string" did not advance
  // parse position.
  if (!args.is_ok())
    {
    // Free built up receiver if some invocations were strung onto it
    if (strung_b && recv_p && (recv_p != *receiver_pp))
      {
      // Keep original receiver
      recv_p->null_receiver(*receiver_pp);
      delete recv_p;
      }

    recv_p = nullptr;
    }

  args.m_end_pos = pos;

  return recv_p;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a named argument specifier.
//
// #Notes
//   named-spec = variable-name ':'
//
// #See Also  parse_invoke_args()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_named_arg, Result_err_size_identifier,
  // Result_err_context_invoke_arg_misnamed
  SkParser::eResult
SkParser::parse_named_specifier(
  // Character position to begin lexical analysis
  uint32_t start_pos,
  // Character position that lexical analysis stopped at - only advances if named
  // specification is present.  If it is set to nullptr, it is not
  // written to.
  uint32_t * end_pos_p, // = nullptr
  // Formal parameters used to validate arguments
  const SkParameters * params_p, // = nullptr
  // Address to store argument index of named specifier if found
  uint32_t * arg_idx_p, // = nullptr
  // Type of parameter send/return that is being named
  SkParameters::eType param_type // = SkParameters::Type_send
  ) const
  {
  uint32_t pos     = start_pos;
  uint32_t end_pos = m_str_ref_p->m_length;

  // set end_pos to start_pos for most error cases
  if (end_pos_p)
    {
    *end_pos_p = start_pos;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure it is a valid looking identifier name
  // $Revisit - could use parse_name_predicate() 

  // Ensure that it is long enough
  if ((pos + 1u) >= end_pos)
    {
    return Result_err_expected_named_arg;
    }


  // Look for identifier name
  char * cstr_a = m_str_ref_p->m_cstr_p;

  if (ms_char_match_table[ACharMatch_not_lowercase][uint8_t(cstr_a[pos])])  
    {
    return Result_err_expected_named_arg;
    }

  pos++;

  // Set pos past the last A-Z, a-z, _, 0-9, or European character
  find(ACharMatch_not_identifier, 1u, &end_pos, pos);

  // Look for optional ? in predicate names (and not nil coalescing op)
  if ((cstr_a[end_pos] == '?') && (cstr_a[end_pos + 1u] != '?'))
    {
    end_pos++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  eResult result = parse_ws_any(end_pos, &pos);

  if (result != Result_ok)
    {
    if (end_pos_p)
      {
      *end_pos_p = pos;
      }

    return result;
    }

  // Look for named argument marker
  if (cstr_a[pos] != ':')  
    {
    return Result_err_expected_named_arg;
    }

  // Note that getting past this point now matches with a named argument specifier and
  // *intentionally* prevents a bind expression from being used as an argument.
  
  if (end_pos_p)
    {
    *end_pos_p = pos + 1u;
    }

  uint32_t name_length = end_pos - start_pos;

  if (name_length > ASymbol_length_max)
    {
    return Result_err_size_identifier;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // End early without checking name to parameters if no parameters supplied
  if (params_p == nullptr)
    {
    return Result_ok;
    }
 

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure name is one of the parameters
  ASymbol param_name(as_symbol(start_pos, end_pos));

  if ((param_type == SkParameters::Type_send)
    ? (params_p->m_params.get(param_name, AMatch_first_found, arg_idx_p) != nullptr)
    : (params_p->m_return_params.get(param_name, AMatch_first_found, arg_idx_p) != nullptr))
    {
    // Valid named specifier was found
    return Result_ok;
    }

  // Specified parameter name not found
  #if defined(SK_AS_STRINGS)
    ms_error_str.ensure_size_empty(500u);

    // Determine if it was a local variable name
    if (m_context.get_local_variable_type(param_name))
      {
      ms_error_str.append("Binding local variables in an argument is not allowed.\n");
      }

    ms_error_str.append_format(
      "No %s with the name '%s' exists in the parameter list:\n"
      "  %s",
      (param_type == SkParameters::Type_send) ? "parameter" : "return parameter",
      param_name.as_cstr_dbg(),
      params_p->as_code(SkParameters::StrFlag__simple).as_cstr());
  #endif

  return Result_err_context_invoke_arg_misnamed;
  }

//---------------------------------------------------------------------------------------
inline SkParser::InvokeArgsInfo::InvokeArgsInfo(
  Args * args_p,
  const SkParameters * params_p,
  APCompactArray<SkExpressionBase> * arg_exprs_p,
  bool implicit_arg1_b
  ) :
  m_args_p(args_p),
  m_params_p(params_p),
  m_plist_pp(nullptr),
  m_param_p(nullptr),
  m_group_arg_p(nullptr),
  m_arg_exprs_p(arg_exprs_p),
  m_group_arg_b(false),
  m_named_args_b(false),
  m_implicit_arg1_b(implicit_arg1_b),
  m_arg_count(implicit_arg1_b ? 1u : 0u),
  m_group_idx(0u),
  m_group_count(0u),
  m_pattern_offset(0u)
  {
  if (params_p)
    {
    m_plist_pp = params_p->m_params.get_array();
    }
  }

//---------------------------------------------------------------------------------------
inline SkParser::InvokeArgsInfo::~InvokeArgsInfo()
  {
  if (m_arg_exprs_p && (m_args_p->m_result != Result_ok))
    {
    if (m_implicit_arg1_b)
      {
      // Ensure implicit arg1 is not deleted twice
      m_arg_exprs_p->null();
      }

    m_arg_exprs_p->free_all();
    }
  }


//---------------------------------------------------------------------------------------
inline bool SkParser::InvokeArgsInfo::complete_group_arg()
  {
  // Group parameters always match - either they will be an empty list
  // or as many arguments as will fit into complete class patterns.
  m_arg_count++;
  m_group_arg_b = false;

  // Ensure that it has a proper number of arguments matching the group
  // parameter class pattern.
  if (m_pattern_offset)
    {
    // Move parse point back
    m_args_p->m_start_pos = m_pattern_start;

    if (m_group_arg_p)
      {
      // Remove any extra expressions greater than pattern 
      m_group_arg_p->m_item_exprs.free_all_last(m_pattern_offset);
      }

    // Continue while loop
    return true;
    }
      
  return false;  
  }

//---------------------------------------------------------------------------------------
// Parse and append argument expression
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // AConfirm_abort - return from parent function
  // AConfirm_no    - continue (skip) loop
  // AConfirm_yes   - next statement in loop
  inline eAConfirm
SkParser::parse_invoke_arg(
  Args & args, 
  InvokeArgsInfo * args_info_p,
  uint32_t bracket_flags
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional argument name specification
  // named-spec = variable-name ':'
  uint32_t arg_idx = args_info_p->m_arg_count;

  args.m_result = parse_named_specifier(
    args.m_start_pos, &args.m_end_pos, args_info_p->m_params_p, &arg_idx);

  if (!args.is_ok() && (args.m_result != Result_err_expected_named_arg))
    {
    // Bad named specifier
    return AConfirm_abort;
    }

  args.m_start_pos = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine argument index and type
  SkClassDescBase * ptype_p = nullptr;

  if (args.m_result != Result_err_expected_named_arg)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Found a named argument

    // Named arg during build of group arg infers the end of the group arg.
    if (args_info_p->m_group_arg_b && args_info_p->complete_group_arg())
      {
      // Need to backtrack - restart while loop  
      return AConfirm_no;
      }

    // Must be set after group check above since it may need to backtrack.
    args_info_p->m_named_args_b = true;

    // Ensure specified named argument has not already been supplied.
    if (args_info_p->m_params_p
      && args_info_p->m_arg_exprs_p
      && (args_info_p->m_arg_exprs_p->get_length() > arg_idx)
      && (args_info_p->m_arg_exprs_p->get_array()[arg_idx] != nullptr))
      {
      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "An argument with the specified name '%s' already exists in the current invocation list.",
          args_info_p->m_plist_pp[arg_idx]->get_name_cstr_dbg());
      #endif

      args.m_result = Result_err_context_invoke_arg_preexist;

      return AConfirm_abort;
      }

    // Set expected type for named parameter
    if (args_info_p->m_params_p)
      {
      // Note that named group parameters require the argument to be a List type.
      args_info_p->m_param_p = args_info_p->m_plist_pp[arg_idx];

      if (m_flags.is_set_any(Flag_type_check))
        {
        ptype_p = args_info_p->m_param_p->get_expected_type()->as_finalized_generic(
          *args_info_p->m_final_rcvr_type_p);
        }
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Bad whitespace - exit
      return AConfirm_abort;
      }

    args.m_start_pos = args.m_end_pos;
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Argument not named
    args.m_result = Result_ok;

    // Once a named argument is used, any following arguments must also be named.
    if (args_info_p->m_named_args_b)
      {
      args.m_result = Result_err_context_invoke_arg_unnamed;

      return AConfirm_abort;
      }

    // Determine expected class type for argument
    if (args_info_p->m_params_p)
      {
      if (!args_info_p->m_group_arg_b)
        {
        args_info_p->m_param_p = args_info_p->m_plist_pp[args_info_p->m_arg_count];

        if (args_info_p->m_param_p->get_kind() == SkParameter_group)
          {
          args_info_p->m_group_arg_b = true;
          args_info_p->m_group_idx   = 0u;
          args_info_p->m_group_count =
            static_cast<SkGroupParam *>(args_info_p->m_param_p)->get_pattern_length();
          }
        }

      if (!args_info_p->m_group_arg_b)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Unary parameter
        ptype_p = args_info_p->m_param_p->get_expected_type()->as_finalized_generic(
          *m_context.finalize_generic(*args_info_p->m_final_rcvr_type_p));
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Group Parameter
        if (args_info_p->m_arg_exprs_p && (args_info_p->m_group_idx == 0u))
          {
          args_info_p->m_group_arg_p = SK_NEW(SkLiteralList)(SkBrain::ms_list_class_p);
          SKDEBUG_SET_CHAR_POS(args_info_p->m_group_arg_p, args.m_start_pos ? args.m_start_pos - 1u : 0u);
          args_info_p->m_arg_exprs_p->append(*args_info_p->m_group_arg_p);
          }

        args_info_p->m_pattern_offset = args_info_p->m_group_idx % args_info_p->m_group_count;

        // Set expression position if beginning of new pattern
        if (args_info_p->m_pattern_offset == 0u)
          {
          args_info_p->m_pattern_start = args.m_start_pos;
          }

        ptype_p = static_cast<SkGroupParam *>(args_info_p->m_param_p)
          ->get_pattern_type(args_info_p->m_group_idx)->as_finalized_generic(
            *args_info_p->m_final_rcvr_type_p);
        }
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse argument expression
  SkExpressionBase * arg_p;

  args.m_desired_type_p = ptype_p;

  if ((bracket_flags & InvokeBrackets_opt_closure_tail)
    && (m_str_ref_p->m_cstr_p[args.m_start_pos] == '[')
    && (ptype_p->is_class_type(SkBrain::ms_closure_class_p)))
    {
    // $Revisit - CReis Consider: Group arguments currently do not allow inline 
    // closures - though they could.

    // Allow inline closure - no ^ or ()
    arg_p = parse_closure(args, true);
    }
  else
    {
    arg_p = parse_expression(args, SkInvokeTime_immediate);
    }

  if (!args.is_ok())
    {
    // Bad argument expression - exit
    return AConfirm_abort;
    }

  // Class type of arg is stored in args.m_type_p

  // Ensure arg is expected type
  bool expected_type = !m_flags.is_set_any(Flag_type_check)
    || args.m_expr_type->is_class_type(ptype_p);

  if (!args_info_p->m_group_arg_b)
    {
    // Unary argument
    if (!expected_type)
      {
      args.m_result = Result_err_typecheck_invoke_arg;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The argument supplied to parameter named `%s` was expected to be of type `%s` "
          "and it is type `%s` which is not compatible.",
          args_info_p->m_param_p->get_name_cstr_dbg(),
          ptype_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());

        // Look for conversion method
        // This is a simple check that won't iterate over a union class, though it is good
        // for simple types such as Integer and Real.
        bool is_class_method = false;
        if (find_method_inherited(
          args.m_expr_type, ptype_p->get_key_class_name(), &is_class_method))
          {
          ms_error_str.append_format("\n"
            "However, `%s` has a `%s()` conversion method so it can easily be converted with the `>>` class conversion operator.",
            args.m_expr_type->as_code().as_cstr(),
            ptype_p->get_key_class()->get_name_cstr_dbg());
          }

        // $Revisit - CReis Maybe also specify if there is a constructor in the desired
        // type that takes the current type as an argument.
      #endif

      delete arg_p;

      return AConfirm_abort;
      }

    args.m_start_pos = args.m_end_pos;

    if (args_info_p->m_arg_exprs_p)
      {
      args_info_p->m_arg_exprs_p->set_at_expand(arg_idx, arg_p);
      }

    // Append argument to argument list
    if (!args_info_p->m_named_args_b || (args_info_p->m_params_p == nullptr))
      {
      args_info_p->m_arg_count++;
      }
    }
  else
    {
    // Group argument

    args.m_start_pos = args.m_end_pos;

    if (expected_type)
      {
      // Matched next class in group pattern
      args_info_p->m_group_idx++;

      if (args_info_p->m_group_arg_p)
        {
        args_info_p->m_group_arg_p->m_item_exprs.append(*arg_p);
        }
      }
    else
      {
      // Did not match next class in group pattern
      args_info_p->complete_group_arg();

      if (arg_p)
        {
        delete arg_p;
        }

      // Continue while loop
      return AConfirm_no;
      }
    }

  // End of parse argument
  return AConfirm_yes;
  }

//---------------------------------------------------------------------------------------
// Use default expression for skipped argument.
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // AConfirm_abort - return from parent function
  // AConfirm_no    - continue (skip) loop
  // AConfirm_yes   - next statement in loop
  inline eAConfirm
SkParser::parse_invoke_arg_default(
  Args & args, 
  InvokeArgsInfo * args_info_p
  ) const
  {
  // Once a named argument is used, any following arguments must also be named.
  if (args_info_p->m_named_args_b)
    {
    args.m_result = Result_err_context_invoke_arg_unnamed;

    return AConfirm_abort;
    }

  // Skipped arg during build of group arg infers the end of the group arg.
  if (args_info_p->m_group_arg_b && args_info_p->complete_group_arg())
    {
    // Continue while loop  
    return AConfirm_no;
    }

  // Ensure that argument has a default to skip
  if (args_info_p->m_plist_pp && !args_info_p->m_plist_pp[args_info_p->m_arg_count]->is_defaultable())
    {
    args.m_result = Result_err_context_invoke_arg_skipped;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "The '%s' parameter does not have a default and may not be skipped.",
        args_info_p->m_plist_pp[args_info_p->m_arg_count]->get_name_cstr_dbg());
    #endif

    return AConfirm_abort;
    }

  // $Revisit - CReis Ensure that no more than 255 arguments are permitted.
  args_info_p->m_arg_count++;

  if (args_info_p->m_arg_exprs_p)
    {
    // Use nullptr to indicate that default expression should be used
    args_info_p->m_arg_exprs_p->append_null();
    }

  return AConfirm_yes;
  }

//---------------------------------------------------------------------------------------
// Adds initial argument to invocation list prior to calling parse_invoke_args()
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if successful parse and false if not
bool SkParser::parse_invoke_args_arg1(
    Args & args,
    SkExpressionBase * arg1_p,
    APCompactArray<SkExpressionBase> * args_p, // = nullptr
    const SkMethodBase * method_p // = nullptr
    ) const
  {
  if (method_p && m_flags.is_set_any(Flag_type_check))
    {
    const SkParameters * params_p = &method_p->get_params();
    SkParameterBase *    param_p  = params_p->get_param_list().get_first();

    if (param_p == nullptr)
      {
      args.m_result = Result_err_context_invoke_arg1;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Argument passed to method '%s' but it has no parameters.\n"
          "[Either pass no arguments or add one or more parameters to the method.]",
          method_p->as_string_name().as_cstr());
      #endif

      return false;
      }

    SkClassDescBase * param_type_p =
      param_p->get_expected_type()->as_finalized_generic(*args.m_expr_type);

    if (!args.m_expr_type->is_class_type(param_type_p))
      {
      args.m_result = Result_err_typecheck_invoke_arg;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The first argument supplied to the '%s' method parameter '%s' was expected to "
          "be an object of the type '%s' and it was given type '%s' which is not compatible.",
          method_p->as_string_name().as_cstr(),
          param_p->get_name_cstr_dbg(),
          param_type_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());
      #endif

      return false;
      }
    }

  if (args_p)
    {
    args_p->append(*arg1_p);
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a invocation argument list - used
// when calling a method or coroutine.
//
// #Notes
//   invocation-args   = [bracketed-args] | closure-tail-args
//   bracketed-args    = '(' ws [send-args ws] [';' ws return-args ws] ')'
//   closure-tail-args = ws send-args ws closure [ws ';' ws return-args]
//   send-args         = [argument] {ws [',' ws] [argument]}
//   return-args       = [return-arg] {ws [',' ws] [return-arg]}
//   named-spec        = variable-name ':'
//             
//     * Only trailing arguments may be named and once one is named the rest must be named
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if successful parse and false if not
  bool
SkParser::parse_invoke_args(
  // see SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and changed to result type on
  //     exit.  Used to finalize any generic types in params_p.
  //   m_result: Result_ok or any expression warning or error.
  Args & args,
  // Address of array to append parsed send arguments to.  If it is non-nullptr then params_p
  // must also be non-nullptr.  It is not written to if it is set to nullptr or if the result is
  // not Result_ok.
  APCompactArray<SkExpressionBase> * args_p,  // = nullptr
  // Address of array to append parsed return arguments to.  If it is non-nullptr then
  // params_p must also be non-nullptr.  It is not written to if it is set to nullptr or if
  // the result is not Result_ok.
  APCompactArray<SkIdentifierLocal> * ret_args_p,  // nullptr
  // Formal parameters used to validate arguments
  const SkParameters * params_p,  // = nullptr
  // See eInvokeBrackets
  eInvokeBrackets brackets, // = InvokeBrackets_opt_closure_tail
  // Optional implicit initial argument or nullptr. Parse runs as normal except that first
  // argument is provided.
  bool implicit_arg1_b // = false
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if brackets / parentheses are being used.
  char *   cstr_a         = m_str_ref_p->m_cstr_p;
  uint32_t bracket_flags  = InvokeBrackets_required;

  args.m_end_pos = args.m_start_pos;

  if (cstr_a[args.m_start_pos] != '(')
    {
    if ((brackets & InvokeBrackets_opt_args0)
      && ((params_p == nullptr)
        || ((implicit_arg1_b
          ? params_p->get_arg_count_min_after_arg1()
          : params_p->get_arg_count_min()) == 0u)))
      {
      // Quick exit if brackets optional for 0 args
      args.m_result = Result_ok;

      return true;
      }

    if ((brackets & InvokeBrackets_opt_closure_tail)
      && ((params_p == nullptr) || params_p->is_last_closure()))
      {
      bracket_flags = InvokeBrackets_opt_closure_tail;
      }

    if (bracket_flags == InvokeBrackets_required)
      {
      args.m_result = Result_err_expected_invoke_args;

      return false;
      }
    }
  else
    {
    // Skip past opening bracket
    args.m_start_pos++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through arguments

  // $Vital - CReis Ensure the proper evaluation order of the arguments esp. considering
  // side effects and the fact that default parameters can reference other parameters.
  // Decide whether it should be order of occurrence in code or index order of parameters.
  // [It will probably have to be index order of parameters since default parameters can
  // reference other parameters.]

  char     ch;
  bool     return_args_b   = false;
  bool     return_params_b = true;
  bool     closure_tail_b  = (bracket_flags & InvokeBrackets_opt_closure_tail) != 0u;
  uint32_t arg_idx_end     = 0u;
  uint32_t length          = m_str_ref_p->m_length;
  uint32_t param_length    = UINT32_MAX;

  ASymbol        param_name;
  InvokeArgsInfo args_info(&args, params_p, args_p, implicit_arg1_b);

  SkClassUnaryBase * receiver_type_p = static_cast<SkClassUnaryBase *>(args.m_expr_type.get_type());

  if (params_p)
    {
    return_params_b = params_p->is_result_params(); 
    param_length    = params_p->m_params.get_length();
    }

  args_info.m_final_rcvr_type_p = m_context.finalize_generic(*receiver_type_p);

  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  while (args.m_start_pos < length)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

    if (!args.is_ok())
      {
      // Error in whitespace so exit
      // ~InvokeArgsInfo() does any cleanup that is required.
      return false;
      }

    args.m_start_pos = args.m_end_pos;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Note that if ch matches one of these characters don't advance past it until later.
    ch = cstr_a[args.m_start_pos];

    if (((ch != ')') || closure_tail_b)
      && ((ch != ';') || !return_params_b))
      {
      // Determine if more arguments may be supplied
      if (args_info.m_arg_count == param_length)
        {
        // Too many arguments
        args.m_result = Result_err_context_invoke_arg_end;

        // ~InvokeArgsInfo() does any cleanup that is required.
        return false;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Query parse probe before we parse the next argument
      if (args.m_flags & ArgFlag_parse_to_idx_probe)
        {
        args.m_invocation_stack.get_last().m_param_idx = args_info.m_arg_count;

        if (args.is_idx_probe_halt(this))
          {
          return false;
          }
        }

      eAConfirm result = (ch == ',')
        // Skip argument and use default expression
        ? parse_invoke_arg_default(args, &args_info)
        // Append argument
        : parse_invoke_arg(args, &args_info, bracket_flags);

      if (result == AConfirm_abort)
        {
        return false; // An error occurred
        }

      if (result == AConfirm_no)
        {
        continue; // Need to backtrack
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Advance to next argument?
      
      // Eat {whitespace}
      args.m_result = parse_ws_any(args.m_end_pos, &arg_idx_end);

      if (!args.is_ok())
        {
        // Error in whitespace so exit
        args.m_start_pos = args.m_end_pos;
        args.m_end_pos   = arg_idx_end;

        // ~InvokeArgsInfo() does any cleanup that is required.
        return false;
        }

      ch = cstr_a[arg_idx_end];

      // If last argument in closure tail invocation and no return arguments then done
      if (closure_tail_b
        && (args_info.m_arg_count == param_length)
        && ((ch != ';') || !return_params_b))
        {
        // Exit while loop
        break;
        }

      // Accept skipped whitespace
      args.m_start_pos = arg_idx_end;
      args.m_end_pos   = arg_idx_end;

      // End of argument parse
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if there are more arguments to follow

    if (ch == ',')
      {
      // One or more arguments are to follow
      args.m_start_pos++;

      // Query parse probe assuming another argument is following
      if (args.m_flags & ArgFlag_parse_to_idx_probe)
        {
        args.m_invocation_stack.get_last().m_param_idx = args_info.m_arg_count;
        args.m_end_pos = args.m_start_pos;

        if (args.is_idx_probe_halt(this))
          {
          return false;
          }
        }
      }
    else
      {
      if (((ch == ')') && !closure_tail_b)
        || ((ch == ';') && return_params_b))
        {
        // If currently building group argument, finish it off
        if (args_info.m_group_arg_b && args_info.complete_group_arg())
          {
          // Continue while loop  
          continue;
          }

        if (ch == ';')
          {
          return_args_b = true;
          }

        args.m_start_pos++;
        args.m_end_pos++;
        args.m_result = Result_ok;

        // Exit while loop
        break;
        }
      }

    // Next possible error
    args.m_result = Result_err_unexpected_eof;
    } // end while loop


  if (!args.is_ok())
    {
    // ~InvokeArgsInfo() does any cleanup that is required.
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prep and check accumulated arguments
  if (args_p)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure that any argument that expects a default has a default specified in
    // the parameter list.

    // $Note - CReis [Type-check] Any valid unsupplied default arguments could be
    // placed now rather than at runtime.  However this would expand memory and
    // compiled binary code sizes without much of a speed savings - it would remove one test for
    // nullptr for each argument evaluation.  [In addition, this may require an
    // additional preparse pass and there would need to be a mechanism to use the
    // receiver scope for the default args and the caller scope for the rest.]

    SkExpressionBase ** args_pp = args_p->get_array();

    length = args_p->get_length();
    // $Revisit - CReis Ensure that no more than 255 arguments are permitted.

    // Check skipped arguments - including args not specified before named args.
    while (args_info.m_arg_count < length)
      {
      if ((args_pp[args_info.m_arg_count] == nullptr)
        && !args_info.m_plist_pp[args_info.m_arg_count]->is_defaultable())
        {
        // Missing argument for parameter that has no default.
        args.m_result = Result_err_context_invoke_arg_missing;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The skipped '%s' parameter does not have a default and must be supplied an argument expression.",
            args_info.m_plist_pp[args_info.m_arg_count]->get_name_cstr_dbg());
        #endif

        // ~InvokeArgsInfo() does any cleanup that is required.
        return false;
        }

      args_info.m_arg_count++;
      }

    if (args.is_ok())
      {
      // Ensure that args that were not supplied at all have a default specified in
      // the parameter list.
      while (args_info.m_arg_count < param_length)
        {
        if (!args_info.m_plist_pp[args_info.m_arg_count]->is_defaultable())
          {
          // Found missing parameter that has no default.
          args.m_result = Result_err_context_invoke_arg_missing;

          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "The trailing omitted '%s' parameter does not have a default and must be supplied an argument expression.",
              args_info.m_plist_pp[args_info.m_arg_count]->get_name_cstr_dbg());
          #endif

          // ~InvokeArgsInfo() does any cleanup that is required.
          return false;
          }

        // $Revisit - CReis Ensure that no more than 255 arguments are permitted.
        args_info.m_arg_count++;
        }
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse return arguments
  if (return_args_b)
    {
    args.m_result = parse_invoke_return_args(
      args.m_start_pos, &args.m_end_pos, ret_args_p, receiver_type_p, params_p, bracket_flags);
    }

  return args.is_ok();
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a invocation return argument list
// - used when calling a method or coroutine.
//
// #Notes
//   invocation-args   = [bracketed-args] | closure-tail-args
//   bracketed-args    = '(' ws [send-args ws] [';' ws return-args ws] ')'
//   closure-tail-args = ws send-args ws closure [ws ';' ws return-args]
//   send-args         = [argument] {ws [',' ws] [argument]}
//   return-args       = [return-arg] {ws [',' ws] [return-arg]}
//   return-arg*       = [named-spec ws] variable-ident | define-temporary
//   named-spec        = variable-name ':'
//   define-temporary  = '!' ws variable-name
//             
//   * Only trailing arguments may be named and once one is named the rest must be named
//
// #See Also  parse_invoke_args()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_unexpected_return_args, Result_err_expected_return_arg,
  // Result_err_unexpected_char, Result_err_unexpected_eof, or any other warning or error. 
SkParser::eResult SkParser::parse_invoke_return_args(
  // character position to begin lexical analysis
  uint32_t start_pos,
  // character position that lexical analysis stopped at.  If it is set to nullptr, it is not
  // written to.
  uint32_t * end_pos_p,
  // pointer to empty argument list to append parsed arguments to. If args_p is non-nullptr
  // then params_p must also be non-nullptr.  It is not written to if it is set to nullptr or if
  // the result is not Result_ok.
  APCompactArray<SkIdentifierLocal> * ret_args_p,
  // class type of receiver for invocation.
  const SkClassUnaryBase * receiver_type_p,
  // parameters used to type-check arguments
  const SkParameters * params_p,
  // Either InvokeBrackets_required or InvokeBrackets_opt_closure_tail
  uint32_t bracket_flags
  ) const
  {
  // $Revisit - CReis Refactor to use Parser::Args

  SkTypedName * param_p      = nullptr;
  uint32_t      param_length = UINT32_MAX; 

  if (params_p)
    {
    param_length = params_p->m_return_params.get_length();

    if (ret_args_p)
      {
      ret_args_p->empty();
      }

    if (param_length == 0u)
      {
      *end_pos_p = start_pos;
      return Result_err_unexpected_return_args;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through return arguments
  char     ch;
  eResult  result         = Result_err_expected_return_arg;
  uint32_t arg_count      = 0u;     // Positional arg count
  uint32_t length         = m_str_ref_p->m_length;
  bool     named_args_b   = false;
  bool     brackets_b     = bracket_flags == InvokeBrackets_required;
  char *   cstr_a         = m_str_ref_p->m_cstr_p;
  Args     args(start_pos, ret_args_p ? ArgFlag__default : ArgFlag__default_no_struct);
  ASymbol  param_name;

  SkExpressionBase *  arg_p   = nullptr;
  SkIdentifierLocal * ident_p = nullptr;

  while (start_pos < length)
    {
    // Eat {whitespace}
    result = parse_ws_any(start_pos, &start_pos);

    if (result != Result_ok)
      {
      // Error in whitespace so exit while loop
      break;
      }

    ch = cstr_a[start_pos];

    // [CReis Using multiple "if" statements here rather than "switch" so "break" can
    // be used to exit "while" loop.]
    if ((ch == ',') || ((ch == ')') && brackets_b))
      {
      start_pos++;

      if (arg_count || (ch == ','))
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Skipped argument

        // Once a named argument is used, any following arguments must also be named.
        if (named_args_b)
          {
          result = Result_err_context_invoke_arg_unnamed;
          break;
          }

        // Ensure that more arguments may be supplied
        if (arg_count == param_length)
          {
          result = Result_err_context_invoke_arg_end;
          break;
          }
          
        // $Revisit - CReis Ensure that no more than 255 return arguments are permitted.
        arg_count++;

        if (ret_args_p)
          {
          // Use nullptr to indicate that the return argument was skipped
          ret_args_p->append_null();
          }
        }

      if (ch == ')')
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Found end of argument list - with no arguments or after an argument is skipped
        result = Result_ok;

        // Exit while loop
        break;
        }
      }
    else // Was not zero arguments or skipped argument
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Look for argument [Start]
         
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Ensure that more arguments may be supplied
      if (arg_count == param_length)
        {
        result = Result_err_context_invoke_arg_end;
        break;
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Look for optional argument name specification
      // named-spec = variable-name ':'
      uint32_t arg_idx = arg_count;

      result = parse_named_specifier(
        start_pos, &start_pos, params_p, &arg_idx, SkParameters::Type_return);


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine argument index and type
      SkClassDescBase * ptype_p = nullptr;

      if (result != Result_err_expected_named_arg)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Found a named argument

        if (result != Result_ok)
          {
          // Exit while loop
          break;
          }

        // Must be set after group check above since it may need to backtrack.
        named_args_b = true;

        // Ensure specified named argument has not already been supplied.
        if (params_p && ret_args_p && (ret_args_p->get_length() > arg_idx)
          && (ret_args_p->get_array()[arg_idx] != nullptr))
          {
          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "A return argument with the specified name '%s' already exists in the current invocation list.",
              params_p->m_return_params.get_array()[arg_idx]->get_name_cstr_dbg());
          #endif

          result = Result_err_context_invoke_arg_preexist;

          // Exit while loop
          break;
          }

        // Set expected type
        if (params_p)
          {
          // Note that named group parameters require an argument of List type.
          param_p = params_p->m_return_params.get_array()[arg_idx];

          if (m_flags.is_set_any(Flag_type_check))
            {
            ptype_p = param_p->m_type_p->as_finalized_generic(*receiver_type_p);
            }
          }


        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Eat {whitespace}
        result = parse_ws_any(start_pos, &start_pos);

        if (result != Result_ok)
          {
          // Bad whitespace - exit while loop
          break;
          }
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Argument not named
        result = Result_ok;

        // Once a named argument is used, any following arguments must also be named.
        if (named_args_b)
          {
          result = Result_err_context_invoke_arg_unnamed;

          // Exit while loop
          break;
          }

        // Determine expected class type for argument
        if (params_p)
          {
          param_p = params_p->m_return_params.get_at(arg_count);
          ptype_p = param_p->m_type_p->as_finalized_generic(*m_context.finalize_generic(*receiver_type_p));
          }
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      args.m_start_pos = start_pos;
      args.m_desired_type_p = ptype_p;  // Expected type

      if (cstr_a[start_pos] == '!')
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Look for variable identifier
        // '!' ws variable-name
        ASymbol  ident_name;
        uint32_t ident_data_idx = 0; // Runtime data array index of ident_name
        bool     predicate;
        uint32_t ident_idx = start_pos;

        parse_temporary(args, &ident_name, nullptr, nullptr, &predicate, false);
        start_pos = args.m_end_pos;
        result    = args.m_result;

        if (result != Result_ok)
          {
          // Found error in temporary - exit while loop
          break;
          }

        if (m_flags.is_set_any(Flag_type_check))
          {
          if (predicate && (ptype_p != SkBrain::ms_boolean_class_p))
            {
            result = Result_err_typecheck_query_variable;
        
            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "The return argument is type `%s` when `Boolean` was expected.\n"
                "Query/predicate temporary variables ending with `?` may only be bound "
                "to a Boolean `true`/`false` expression",
                ptype_p->as_code().as_cstr());
            #endif

            // Exit while loop
            break;
            }

          ident_data_idx = m_context.append_local(ident_name, ptype_p, true);
          }

        // Add create temporary variable to innermost code block scope
        if (m_current_block_p)
          {
          m_current_block_p->m_temp_vars.append(ident_name);
          }

        // Create identifier to bind return argument to
        ident_p = SK_NEW(SkIdentifierLocal)(ident_name, ident_data_idx);
        m_context.on_identifier_created(ident_p);

        SKDEBUG_SET_CHAR_POS(ident_p, ident_idx);
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Look for identifier expression
        arg_p = parse_expression(args);

        start_pos = args.m_end_pos;
        result    = args.m_result;

        if (result != Result_ok)
          {
          if ((result == Result_err_expected_expression) && (start_pos == args.m_start_pos))
            {
            // Be more specific
            result = Result_err_expected_return_arg;
            }

          // Bad return argument expression - exit while loop
          break;
          }

        // $Revisit CReis - Should be able to do these type checks without needing arg object.
        if (arg_p)
          {
          // Ensure expression is identifier
          result = identifier_validate_bind(arg_p);

          if (result != Result_ok)
            {
            delete arg_p;

            // Bad return argument identifier - exit while loop
            break;
            }

          ident_p = static_cast<SkIdentifierLocal *>(arg_p);

          // Ensure arg is expected type
          result  = identifier_validate_bind_type(ident_p, args.m_expr_type, ptype_p);

          if (result != Result_ok)
            {
            delete ident_p;

            // Exit while loop
            break;
            }
          }
        }


      if (ident_p && ret_args_p)
        {
        ret_args_p->set_at_expand(arg_idx, ident_p);
        }

      // Append argument to argument list
      if (!named_args_b || (params_p == nullptr))
        {
        arg_count++;
        }


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine if there are more arguments to follow

      // If last return argument in closure tail invocation then done
      if (!brackets_b && (arg_count == param_length))
        {
        // Exit while loop
        break;
        }

      // Eat {whitespace}
      result = parse_ws_any(start_pos, &start_pos);

      if (result != Result_ok)
        {
        // Error in whitespace so exit while loop
        break;
        }

      ch = cstr_a[start_pos];

      if (ch == ',')
        {
        // One or more arguments are to follow
        start_pos++;
        }
      else
        {
        if ((ch == ')') && brackets_b)
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Found end of argument list
          start_pos++;
          result = Result_ok;

          // Exit while loop
          break;
          }
        }
      }

    // Next possible error
    result = Result_err_expected_return_arg;
    } // end while loop

  if (ret_args_p && (result != Result_ok))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If parse error, free any accumulated arguments.
    ret_args_p->free_all();
    }

  *end_pos_p = start_pos;

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an invoke apply.
// 
// Returns:
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically
//   allocated `SkInvokeSync` data-structure otherwise `nullptr` is returned.
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//       m_type_p:
//         set with class type of receiver on entry and changed to result type on exit.
//       m_result:
//         Result_ok, Result_err_expected_invoke_apply, Result_err_typecheck_invoke_apply_recv,
//         or any expression warning or error.
//   receiver_p:
//     Target for the invoke apply.  If it is `nullptr` then 'this' - i.e. the topmost
//     scope - is inferred.
//
// Notes:
//   invoke-apply    = receiver ws '%' invoke-selector
//   receiver*       = expression
//   invoke-selector = method-call | coroutine-call
//
//   * The receiver is supplied to this method and the first whitespace has been skipped.
SkInvokeSync * SkParser::parse_invoke_apply(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  uint32_t   start_pos = args.m_start_pos;
  uint32_t   pos       = start_pos;
  uint32_t   length    = m_str_ref_p->m_length;
  char *     cstr_a    = m_str_ref_p->m_cstr_p;

  // If it has a '%' delimiter and not a '%>' delimiter which is used for invoke_race.
  if (((length - pos) < 2u)
    || (cstr_a[pos] != '%')
    || (cstr_a[pos + 1u] == '>'))
    {
    args.m_result = Result_err_expected_invoke_apply;
    return nullptr;
    }

  pos++;

  SkInvokeSync *    invoke_p    = nullptr;
  SkInvokeBase *    call_p      = nullptr;
  SkClassDescBase * recv_type_p = m_context.finalize_generic(*args.m_expr_type);

  // Next possible error
  args.m_result = Result_err_typecheck_invoke_apply_recv;

  if (recv_type_p != SkNone::get_class())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Change receiver type class to apply an invocation to.

    bool              list_receiver_b = false;
    SkClassDescBase * invoke_type_p   = recv_type_p;

    switch (recv_type_p->get_class_type())
      {
      case SkClassType_class_union:
        {
        invoke_type_p = SkClassUnion::get_reduced(
          *static_cast<SkClassUnion *>(recv_type_p),
          *SkNone::get_class());

        // If it is a list, get the element type
        SkClassDescBase * key_class_p = invoke_type_p->get_key_class();

        if (key_class_p->is_class_type(SkBrain::ms_list_class_p))
          {
          list_receiver_b = true;

          if (key_class_p->get_class_type() == SkClassType_typed_class)
            {
            // List with typed elements.
            invoke_type_p = static_cast<SkTypedClass *>(key_class_p)->get_item_type();
            }
          else
            {
            // Generic List with Object elements.
            invoke_type_p = SkBrain::ms_object_class_p;
            }
          }
        }
        break;

      case SkClassType_typed_class:
        if (recv_type_p->get_key_class()->is_class_type(SkBrain::ms_list_class_p))
          {
          // List with typed elements.
          list_receiver_b = true;
          invoke_type_p   = static_cast<SkTypedClass *>(recv_type_p)->get_item_type();
          }
        else
          {
          // Non-list class - treat standard way
          invoke_type_p = recv_type_p;
          }
        break;

      case SkClassType_class:
        if (recv_type_p->is_class_type(SkBrain::ms_list_class_p))
          {
          // Generic List with Object elements.
          list_receiver_b = true;
          invoke_type_p   = SkBrain::ms_object_class_p;
          break;
          }
        // It is a standard class so let it fall through to the next case

      //case SkClassType_metaclass:
      default:
        // It is just a standard class, don't do anything special
        // $Revisit - CReis since this is equivalent to a standard invocation, the
        // parser could give a warning that it is essentially overkill.
        invoke_type_p = recv_type_p;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse invoke selector
    args.m_expr_type = invoke_type_p;
    args.m_start_pos = pos;

    call_p = parse_invoke_selector(args, true);

    pos = args.m_end_pos;

    if (args.is_ok())
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Adjust return type
      if (list_receiver_b)
        {
        // Lists return themselves as a result
        args.m_expr_type = recv_type_p;
        }
      else
        {
        args.m_expr_type = SkClassUnion::get_merge(*args.m_expr_type, *SkNone::get_class());
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Create data structure
      if (args.is_struct_wanted())
        {
        invoke_p = SK_NEW(SkInvokeSync)(call_p, receiver_p);
        SKDEBUG_SET_CHAR_POS(invoke_p, start_pos);
        }
      }
    }

  args.m_end_pos = pos;

  return invoke_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an instantiation invocation.
// It is assumed that the initial part is an expression other than a class instance and
// the initial exclamation mark '!' has already been parsed.
// 
// #Notes
//   * The receiver is supplied to this method and the initial '!' has already been tested.
//   
//   instantiation   = [class-instance] | expression '!' [instance-name] invocation-args
//   
//   Acts as "syntactical sugar" as follows:
//
//                              Equivalent
//                              ----------
//     expr!                    ExprClass!copy(expr)
//     expr!ctor                ExprClass!ctor(expr)
//     expr!method              ExprClass!copy(expr).[method() self()]
//     expr!(arg1, arg2)        ExprClass!copy(expr, arg1, arg2)
//     expr!ctor(arg1, arg2)    ExprClass!ctor(expr, arg1, arg2)
//     expr!method(arg1, arg2)  ExprClass!copy(expr).[method(arg1, arg2) self()]
// 
// [!var: ExprClass!copy(expr) var.method(arg1, arg2) var]
// #Author(s)   Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkInvokeSync data-structure otherwise nullptr is returned. 
  SkExpressionBase * SkParser::parse_invoke_instantiate(
  // see SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and changed to result type on exit.
  //   m_result: Result_ok or any expression warning or error.
  Args & args,
  // the target for the invoke cascade subroutines calls.  If it is nullptr then 'this'
  // - i.e. the topmost scope - is inferred.
  SkExpressionBase * receiver_p,
  // If we encountered a plain copy of raw data and just passed it through
  bool * is_raw_redundant_copy_p
  ) const
  {
  enum eInstantiateType
    {
    IType_ctor_copy,
    IType_ctor_named,
    IType_method
    };

  ASymbol  ident_sym = ASymbolX_ctor_copy;
  uint32_t start_pos = args.m_start_pos;
  uint32_t pos       = start_pos + 1u;
  uint32_t length    = m_str_ref_p->m_length;
  char *   cstr_a    = m_str_ref_p->m_cstr_p;

  SkClassDescBase * receiver_type_p = args.m_expr_type;
  SkMethodBase *    method_p        = nullptr;
  bool              is_class_method = false;
  SkParameters *    params_p        = nullptr;
  eInstantiateType  itype           = IType_ctor_copy;

  // Determine if instance name follows exclamation.
  if (ms_is_lowercase[uint8_t(cstr_a[pos])])
    {
    bool predicate_b = false;

    pos = a_min(pos + 1u, length - 1u);
    args.m_result = parse_name_predicate(start_pos + 1u, &pos, nullptr, &predicate_b);

    if (predicate_b)
      {
      args.m_result = Result_err_unexpected_query_identifier;
      }

    if (!args.is_ok())
      {
      args.m_end_pos = pos;

      return nullptr;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // First try to find named constructor - ExprClass!identifier(expr ...
    AString ident_str(cstr_a + start_pos, pos - start_pos, false);

    itype = IType_ctor_named;

    if (m_flags.is_set_any(Flag_type_check))
      {
      // Don't create symbol unless it already exists - don't want to spam sym/str DB
      ident_sym = ASYMBOL_STR_TO_SYM_IF_EXIST(ident_str);

      if (!ident_sym.is_null())
        {
        method_p = find_method_inherited(receiver_type_p, ident_sym, &is_class_method);
        }

      if (method_p == nullptr)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // If constructor not found try ExprClass!copy(expr).identifier( ...
        // Remove initial !
        ident_str.remove_all(0u, 1u);

        // Don't create symbol unless it already exists - don't want to spam sym/str DB
        ident_sym = ASYMBOL_STR_TO_SYM_IF_EXIST(ident_str);

        if (!ident_sym.is_null())
          {
          method_p = find_method_inherited(receiver_type_p, ident_sym, &is_class_method);
          itype = IType_method;
          }
        }

      if (!method_p || is_class_method)
        {
        args.m_result  = Result_err_context_non_method;
        args.m_end_pos = pos;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            method_p
              ? "Tried to invoke constructor '!%s()'/'%s()' on %s which is a class method. Expected an instance method."
              : "Neither the method '!%s()' nor '%s()' exists for %s.",
            ident_str.as_cstr(),
            ident_str.as_cstr(),
            receiver_type_p->get_scope_desc().as_cstr());
        #endif

        return nullptr;
        }

      params_p = &method_p->get_params();
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for parse to index probe
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    // We just found a new receiver, and a new invocation
    args.m_receiver_type_p = receiver_type_p;
    args.m_invocation_stack.append(InvocationInfo(params_p, args.m_start_pos));

    if (args.is_idx_probe_halt(this))
      {
      // Found probe index, so exit
      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optionally get auto inferred copy constructor
  SkMethodBase * copy_ctor_p   = nullptr;
  SkParameters * copy_params_p = nullptr;

  if (itype != IType_ctor_named)
    {
    if (m_flags.is_set_any(Flag_type_check))
      {
      bool is_class_ctor = false;
      copy_ctor_p = find_method_inherited(receiver_type_p, ASymbolX_ctor_copy, &is_class_ctor);

      if (copy_ctor_p == nullptr || is_class_ctor)
        {
        args.m_result  = Result_err_context_non_method;
        args.m_end_pos = start_pos + 1u;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            copy_ctor_p 
              ? "Found %s@!copy() as a class member but must be an instance member."
              : "Inferred a copy constructor in an instantiation invocation but %s@!copy() does not exist.",
            receiver_type_p->as_code().as_cstr());
        #endif

        return nullptr;
        }

      copy_params_p = &copy_ctor_p->get_params();
      }
    }

  args.m_end_pos = pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse any remaining arguments

  SkMethodCallBase * ctor_call_p;
  SkExpressionBase * expr_p = nullptr;
  bool is_raw_redundant_copy = false;

  args.m_start_pos = args.m_end_pos;

  switch (itype)
    {
    case IType_ctor_copy:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // If we are making a plain copy of a raw data instance, skip since it is already a copy
      if (args.m_expr_type.is_raw_access())
        {
        is_raw_redundant_copy = true;
        expr_p = receiver_p;
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // expr!  to  ExprClass!copy(expr ...)
      method_p = copy_ctor_p;
      params_p = copy_params_p;

      // Allow fall-through to IType_ctor_named

    case IType_ctor_named:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // expr!ctor  to  ExprClass!ctor(expr ...)
      if (args.is_struct_wanted())
        {
        // method_p is guaranteed to be a instance method at this point
        ctor_call_p = create_method_call(method_p, false, nullptr, nullptr);

        APCompactArray<SkExpressionBase> * iargs_p = &ctor_call_p->m_arguments;

        if (!parse_invoke_args_arg1(args, receiver_p, iargs_p, method_p)
          || !parse_invoke_args(
            args, iargs_p, &ctor_call_p->m_return_args, params_p, InvokeBrackets_opt_args0_closure, true))
          {
          delete ctor_call_p;

          return nullptr;
          }

        expr_p = SK_NEW(SkInstantiate)(receiver_type_p->get_key_class(), ctor_call_p);
        SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
        }
      else
        {
        // Validation parse
        if (!parse_invoke_args_arg1(args, nullptr, nullptr, method_p)
          || !parse_invoke_args(args, nullptr, nullptr, params_p, InvokeBrackets_opt_args0_closure, true))
          {
          return nullptr;
          }
        }
      break;

    case IType_method:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // expr!method  to  ExprClass!copy(expr).[method ... self()]
      
      // Create copy constructor method call
      args.m_desired_type_p = nullptr;  // Get desired type from first parameter
      args.m_expr_type = receiver_type_p;
      ctor_call_p = parse_invoke_instance_method_arg1(args, copy_ctor_p, receiver_p);

      if (!args.is_ok())
        {
        return nullptr;
        }

      // Get remaining args for method call
      if (args.is_struct_wanted())
        {
        // method_p is guaranteed to be a instance method at this point
        SkMethodCallBase * method_call_p = create_method_call(method_p, false, nullptr, nullptr);

        parse_invoke_args(
          args, &method_call_p->m_arguments, &method_call_p->m_return_args, params_p);

        if (!args.is_ok())
          {
          // Ensure receiver_p is not deleted twice
          ctor_call_p->m_arguments.null();
          delete ctor_call_p;
          delete method_call_p;

          return nullptr;
          }

        expr_p = SK_NEW(SkCopyInvoke)(
          receiver_type_p->get_key_class(), ctor_call_p, method_call_p);
        SKDEBUG_SET_CHAR_POS(expr_p, start_pos);
        }
      else
        {
        // Validation parse
        parse_invoke_args(args, nullptr, nullptr, params_p);
        }

      break;
    }

  if (m_flags.is_set_any(Flag_type_check))
    {
    // Note that receiver type is reused as result since same type is being constructed.
    args.m_expr_type = receiver_type_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Done parsing the instantiation
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    args.m_invocation_stack.pop_last();
    }

  // Tell caller if we passed raw data through a plain copy
  *is_raw_redundant_copy_p = is_raw_redundant_copy;

  return expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an invoke race.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkInvokeSync data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok or any expression warning or error.
// Arg         receiver_p - the target for the invoke cascade subroutines calls.  If it
//             is nullptr then 'this' - i.e. the topmost scope - is inferred.
// Notes:      invoke-race     = receiver ws '%>' invoke-selector
//             receiver*       = expression
//             invoke-selector = method-call | coroutine-call
//
//             * The receiver is supplied to this method and the first whitespace has
//               been skipped.
// Author(s):   Conan Reis
SkInvokeRace * SkParser::parse_invoke_race(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  uint32_t       start_pos = args.m_start_pos;
  uint32_t       pos       = start_pos;
  uint32_t       length    = m_str_ref_p->m_length;
  char *         cstr_a    = m_str_ref_p->m_cstr_p;
  SkInvokeRace * invoke_p  = nullptr;
  SkInvokeBase * call_p    = nullptr;

  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  if ((length - pos) >= 3u)
    {
    // Next possible error
    args.m_result = Result_err_expected_invoke_apply;

    if ((cstr_a[pos] == '%') && (cstr_a[pos + 1u] == '>'))
      {
      pos += 2u;

      SkClassDescBase * recv_type_p = m_context.finalize_generic(*args.m_expr_type);

      // Next possible error
      args.m_result = Result_err_typecheck_invoke_apply_recv;

      if (recv_type_p != SkNone::get_class())
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Change receiver type class to apply an invocation to.

        bool              list_receiver_b = false;
        SkClassDescBase * invoke_type_p   = recv_type_p;

        switch (recv_type_p->get_class_type())
          {
          case SkClassType_class_union:
            {
            invoke_type_p = SkClassUnion::get_reduced(
              *static_cast<SkClassUnion *>(recv_type_p),
              *SkNone::get_class());

            // If it is a list, get the element type
            SkClassDescBase * key_class_p = invoke_type_p->get_key_class();

            if (key_class_p->is_class_type(SkBrain::ms_list_class_p))
              {
              list_receiver_b = true;

              if (key_class_p->get_class_type() == SkClassType_typed_class)
                {
                // List with typed elements.
                invoke_type_p = static_cast<SkTypedClass *>(key_class_p)->get_item_type();
                }
              else
                {
                // Generic List with Object elements.
                invoke_type_p = SkBrain::ms_object_class_p;
                }
              }
            }
            break;

          case SkClassType_typed_class:
            if (recv_type_p->get_key_class()->is_class_type(SkBrain::ms_list_class_p))
              {
              // List with typed elements.
              list_receiver_b = true;
              invoke_type_p   = static_cast<SkTypedClass *>(recv_type_p)->get_item_type();
              }
            else
              {
              // Non-list class - treat standard way
              invoke_type_p = recv_type_p;
              }
            break;

          case SkClassType_class:
            if (recv_type_p->is_class_type(SkBrain::ms_list_class_p))
              {
              // Generic List with Object elements.
              list_receiver_b = true;
              invoke_type_p   = SkBrain::ms_object_class_p;
              break;
              }
            // It is a standard class so let it fall through to the next case

          //case SkClassType_metaclass:
          default:
            // It is just a standard class, don't do anything special
            // $Revisit - CReis since this is equivalent to a standard invocation, the
            // parser could give a warning that it is essentially overkill.
            invoke_type_p = recv_type_p;
          }


        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Parse invoke selector
        args.m_expr_type = invoke_type_p;
        args.m_start_pos = pos;

        call_p = parse_invoke_selector(args, true);

        pos = args.m_end_pos;

        if (args.is_ok())
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Adjust return type
          if (list_receiver_b)
            {
            // Lists return themselves as a result
            args.m_expr_type = recv_type_p;
            }
          else
            {
            args.m_expr_type = SkClassUnion::get_merge(*args.m_expr_type, *SkNone::get_class());
            }

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Create data structure
          if (args.is_struct_wanted())
            {
            invoke_p = SK_NEW(SkInvokeRace)(call_p, receiver_p);
            SKDEBUG_SET_CHAR_POS(invoke_p, start_pos);
            }
          }
        }
      }
    }

  args.m_end_pos = pos;

  return invoke_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an invoke cascade [or an invocation that has
//             only a single invocation or a single concurrent call].
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkInvokeCascade data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok, Result_err_expected_invoke_cascade,
//                 Result_err_expected_invoke_cascades, or any expression warning or error.
// Arg         receiver_p - the target for the invoke cascade subroutines calls.  If it
//             is nullptr then 'this' - i.e. the topmost scope - is inferred.
// Notes:      invoke-cascade  = expression ws '.' ws '[' {ws invoke-selector | operator call}2+ ws ']'
//             receiver*       = expression
//             invoke-selector = method-call | coroutine-call
//             method-call     = [scope] method-name invocation-args
//             coroutine-call  = [scope] coroutine-name invocation-args
//
//             * The receiver is supplied to this method and the first whitespace has
//               been skipped.
//               
//             The initial portion prior to the '[' will already have been parsed prior
//             to calling this method:
//               expression ws '.' ws 
//               
// Author(s):   Conan Reis
SkInvokeCascade * SkParser::parse_invoke_cascade(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure initial '['
  char * cstr_a = m_str_ref_p->m_cstr_p;
  uint32_t   length = m_str_ref_p->m_length;
  uint32_t   pos    = args.m_start_pos;

  if (cstr_a[pos] != '[')
    {
    args.m_result  = Result_err_expected_invoke_cascade;
    args.m_end_pos = pos;

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  args.m_result = parse_ws_any(pos + 1u, &pos);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make data-structure if desired
  SkInvokeCascade * invoke_p = nullptr;

  // Make data-structure if desired
  if (args.is_struct_wanted() && (pos < length))
    {
    invoke_p = SK_NEW(SkInvokeCascade)(receiver_p);
    SKDEBUG_SET_CHAR_POS(invoke_p, args.m_start_pos);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through invocation calls
  uint32_t          call_count  = 0u;
  SkInvokeBase *    call_p      = nullptr;
  SkClassDescBase * recv_type_p = args.m_expr_type;

  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  while (pos < length)
    {
    if (cstr_a[pos] == ']')
      {
      // Found close of cascade block - exit while
      pos++;
      args.m_result = Result_ok;
      break;
      }

    // The invoke selector needs to know the type of the receiver.  Only the result type
    // of the last invocation in the cascade is kept
    args.m_expr_type = recv_type_p;

    // Parse invoke selector
    args.m_start_pos = pos;
    call_p = parse_invoke_selector(args, true);
    pos = args.m_end_pos;

    if (!args.is_ok())
      {
      // Found bad invoke selector parse, so exit while loop
      break;
      }

    call_count++;

    if (invoke_p)
      {
      // Append call selector
      invoke_p->m_invoke_calls.append(*call_p);
      }

    // Eat {whitespace}
    args.m_result = parse_ws_any(pos, &pos);

    if (!args.is_ok())
      {
      // Found bad comment parse, so exit while loop
      break;
      }

    // Next possible error
    args.m_result = Result_err_unexpected_eof;
    }  // while


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Interpret results
  if (args.is_ok() && (call_count < 2u))
    {
    // There should be at least two calls for a cascade
    args.m_result = (call_count == 0u) ? Result_err_expected_invoke_cascade : Result_err_expected_invoke_cascades;
    }

  if (invoke_p && !args.is_ok())
    {
    // Parse error - free any accumulated data structures.
    invoke_p->m_receiver_p = nullptr;
    delete invoke_p;
    invoke_p = nullptr;
    }

  args.m_end_pos = pos;

  return invoke_p;
  }


//---------------------------------------------------------------------------------------
// Parses attempting to create an invocation operator.
// 
// [Note that the args.m_type_p type context that is passed in is needed to properly parse
// the invoke operator.]
//
// Returns:
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically allocated
//   `SkExpression` derived data-structure otherwise `nullptr` is returned.
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//       m_type_p:
//         set with class type of receiver on entry and changed to result type on exit.
//       m_result: Result_ok or any expression warning or error.
//   receiver_p:
//     Target for the invocation operator call.  If it is `nullptr` then 'this'
//     - i.e. the topmost scope - is inferred.
//
// Notes:
//   invoke-operator  = expression bracketed-args
//   bracketed-args   = '(' ws [send-args ws] [';' ws return-args ws] ')'
//
//   * The receiver expression is supplied to this method.
SkExpressionBase * SkParser::parse_invoke_operator(
  Args & args,
  SkExpressionBase * receiver_p
  ) const
  {
  if (m_str_ref_p->m_cstr_p[args.m_start_pos] != '(')
    {
    args.m_result = Result_err_expected_invoke_args;
    return nullptr;
    }

  // Determine the type having an invoke operator called on it
  SkClassUnaryBase * recv_type_p =
    m_context.finalize_generic(*args.m_expr_type)->as_unary_class();

  // Is it an invokable class?
  if (recv_type_p->get_class_type() == SkClassType_invokable_class)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // It is an invoked closure
    SkExpressionBase * result_p = nullptr;
    SkInvokableClass * iclass_p = static_cast<SkInvokableClass *>(recv_type_p);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for parse to index probe
    if (args.m_flags & ArgFlag_parse_to_idx_probe)
      {
      // We just found a new receiver, and a new invocation
      args.m_receiver_type_p = recv_type_p;
      args.m_invocation_stack.append(InvocationInfo(iclass_p->get_parameters(), args.m_start_pos));
      args.m_end_pos = args.m_start_pos;

      if (args.is_idx_probe_halt(this))
        {
        // Found probe index, so exit
        return nullptr;
        }
      }

    if (args.m_flags & ArgFlag_make_struct)
      {
      APCompactArray<SkExpressionBase>  send_args;
      APCompactArray<SkIdentifierLocal> return_args;

      // type for finalizing parameter generics - not needed since closure parameters disallow generics.
      args.m_desired_type_p = nullptr;

      if (parse_invoke_args(
        args,
        &send_args,
        &return_args,
        iclass_p->get_parameters(),
        InvokeBrackets_required))
        {
        result_p = iclass_p->is_immediate()
          ? (SkExpressionBase *)SK_NEW(SkInvokeClosureMethod)(receiver_p, iclass_p->get_parameters(), &send_args, &return_args)
          : (SkExpressionBase *)SK_NEW(SkInvokeClosureCoroutine)(receiver_p, iclass_p->get_parameters(), &send_args, &return_args);
        }
      }
    else
      {
      // type for finalizing parameter generics - not needed since closure parameters disallow generics.
      args.m_desired_type_p = nullptr;

      // Validation parse only - don't make structure
      parse_invoke_args(
        args,
        nullptr,
        nullptr,
        iclass_p->get_parameters(),
        InvokeBrackets_required);  // brackets required
      }

    if (args.is_ok())
      {
      args.m_expr_type = iclass_p->get_parameters()->get_result_class();
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Done parsing the invocation
    if (args.m_flags & ArgFlag_parse_to_idx_probe)
      {
      args.m_invocation_stack.pop_last();
      }

    return result_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // It is a regular class that should have an invoke() method or _invoke() coroutine.


  // $Revisit - CReis [A_NOTE] ***Incomplete*** - [Missing Tech] Code for invoke operator for non-closure objects.


  args.m_result = Result_err_unimplemented;
  args.m_end_pos = args.m_start_pos + 1u;

  #if defined(SK_AS_STRINGS)
    ms_error_str.ensure_size_empty(500u);
    ms_error_str.append("Non-closure invoke operator 'expr()' is not fully implemented yet.");
  #endif

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an invocation index or slice operator.
// 
// [Note that the args.m_type_p type context that is passed in is needed to properly parse
// the invoke index/slice operator.]
//
// Returns:
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically allocated
//   `SkExpression` derived data-structure otherwise `nullptr` is returned.
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//       m_type_p:
//         set with class type of receiver on entry and changed to result type on exit.
//       m_result: Result_ok or any expression warning or error.
//   receiver_p:
//     Target for the invocation operator call.  If it is `nullptr` then 'this'
//     - i.e. the topmost scope - is inferred.
//
// Notes:
//   index-operator = expression '{' ws expression ws '}' [ws binding]
//   slice-operator = expression '{' ws range-literal [wsr expression] ws '}'
//
//   * The receiver expression is supplied to this method.
SkInvocation * SkParser::parse_invoke_index_operator(
  Args & args,
  SkExpressionBase * receiver_p
  ) const
  {
  SkParameters *    params_p        = nullptr;
  SkClassDescBase * receiver_type_p = args.m_expr_type;
  SkClassDescBase * expected_type_p = nullptr;
  SkMethodBase *    method_p        = nullptr;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Skip over '{'
  uint32_t start_pos = args.m_start_pos;
  uint32_t pos       = args.m_start_pos + 1u;

  args.m_end_pos = pos;

  if (m_flags.is_set_any(Flag_type_check))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure method is available
    // - try `at()` `{expr}` first and then determine if it was a `at_set()` `{expr}: expr`
    bool is_class_method = false;
    method_p = receiver_type_p->find_method_inherited(ASymbol_at, &is_class_method);

    if (method_p == nullptr || is_class_method)
      {
      args.m_result  = Result_err_context_non_method;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          method_p
            ? "%s@at() must be an instance member but found as class member."
            : "The operator instance method `at()` / `{}` does not exist for %s.\n"
              "[at() is also used when parsing `at_set()` / `{}:` and must match index type.]",
          receiver_type_p->get_scope_desc().as_cstr());
      #endif
  
      return nullptr;
      }

    // Determine desired type for index
    params_p = &method_p->get_params();
    
    SkParameterBase * param_p = params_p->m_params.get_first();

    if (param_p == nullptr)
      {
      args.m_result  = Result_err_context_invoke_arg1;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.append(
          "Method `at()` / `{}` needs a parameter to use as an index.\n"
          "[at() is also used when parsing `at_set()` / `{}:` and must match index type.]");
      #endif
  
      return nullptr;
      }

    expected_type_p = param_p->get_expected_type()->as_finalized_generic(*receiver_type_p);

    // $Revisit - CReis Should check to ensure any additional args have defaults.
    // Maybe call parse_invoke_instance_method_arg1() or something like it
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  args.m_start_pos = pos;
  args.m_result    = parse_ws_any(pos, &pos);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse index object
  args.m_start_pos      = pos;
  args.m_desired_type_p = expected_type_p;

  SkExpressionBase * idx_expr_p = parse_expression(args, SkInvokeTime_immediate);

  pos = args.m_end_pos;

  if (!args.is_ok())
    {
    return nullptr;
    }

  // Ensure that index object is of the expected argument type.
  if (m_flags.is_set_any(Flag_type_check))
    {
    if (!args.m_expr_type->is_class_type(expected_type_p))
      {
      args.m_result = Result_err_typecheck_operand;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The first argument supplied to `at()` / `{}` was expected to be an object "
          "of the type '%s' and it was given type '%s' which is not compatible.\n"
          "[at() is also used when parsing `at_set()` / `{}:` and must match index type.]",
          expected_type_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());
      #endif

      delete idx_expr_p;

      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  args.m_start_pos = pos;
  args.m_result    = parse_ws_any(pos, &pos);
  args.m_end_pos   = pos;

  if (!args.is_ok())
    {
    delete idx_expr_p;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find closing `}`
  const char * cstr_a = m_str_ref_p->m_cstr_p;

  if (cstr_a[pos] != '}')
    {
    args.m_result    = Result_err_expected_op_index_end;
    args.m_start_pos = pos;

    delete idx_expr_p;

    return nullptr;
    }

  pos++;
  args.m_start_pos = pos;
  args.m_end_pos   = pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional `:` which indicates that it is a `at_set()` index operator
  
  // Eat {whitespace} and look for `:`
  if ((parse_ws_any(pos, &pos) != Result_ok) || (cstr_a[pos] != ':'))
    {
    // Looks like it is just an index operator `{}`

    if (m_flags.is_set_any(Flag_type_check))
      {
      args.m_expr_type = params_p->get_result_class()->as_finalized_generic(*receiver_type_p);
      }

    if (!args.is_struct_wanted())
      {
      return nullptr;
      }

    // method_p is guaranteed to be an instance method
    SkMethodCallBase * mcall_p = create_method_call(method_p, false, nullptr, nullptr);

    mcall_p->m_arguments.append(*idx_expr_p);

    SkInvocation * invoke_p = SK_NEW(SkInvocation)(mcall_p, receiver_p);
    SKDEBUG_SET_CHAR_POS(invoke_p, start_pos);

    return invoke_p;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Looks like a set at index operator `{expr}: expr`

  // Skip `:`
  pos++;
  args.m_start_pos = pos;
  args.m_end_pos   = pos;

  if (m_flags.is_set_any(Flag_type_check))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure `at_set()` method is available
    bool is_class_method = false;
    method_p = receiver_type_p->find_method_inherited(ASymbol_at_set, &is_class_method);

    if (method_p == nullptr || is_class_method)
      {
      args.m_result  = Result_err_context_non_method;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          method_p
            ? "%s@at_set() must be an instance member but found as class member."
            : "The operator method `at_set()` / `{}:` does not exist for %s.",
          receiver_type_p->get_scope_desc().as_cstr());
      #endif
  
      delete idx_expr_p;

      return nullptr;
      }

    // Determine desired type for index
    params_p = &method_p->get_params();
    
    if ((params_p->m_params.get_length() < 2u) || (params_p->get_arg_count_min() > 2))
      {
      args.m_result  = Result_err_context_invoke_arg1;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.append("Method `at_set()` / `{}:` needs index and object parameters.");
      #endif
  
      delete idx_expr_p;

      return nullptr;
      }

    SkClassDescBase * idx_type_p
      = params_p->m_params.get_first()->get_expected_type()->as_finalized_generic(*receiver_type_p);

    if (idx_type_p != expected_type_p)
      {
      args.m_result  = Result_err_typecheck_operand;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.append("Method `at_set()` first parameter must be same type as paired `at()`.");
      #endif
  
      delete idx_expr_p;

      return nullptr;
      }

    expected_type_p = params_p->m_params.get_at(1u)->get_expected_type()->as_finalized_generic(*receiver_type_p);

    // $Revisit - CReis Should check to ensure any additional args have defaults.
    }
  else
    {
    expected_type_p = nullptr;
    }
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  args.m_result = parse_ws_any(pos, &args.m_end_pos);

  if (!args.is_ok())
    {
    delete idx_expr_p;
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse object to set/assign/bind
  args.m_start_pos      = args.m_end_pos;
  args.m_desired_type_p = expected_type_p;

  SkExpressionBase * obj_expr_p = parse_expression(args, SkInvokeTime_immediate);

  if (!args.is_ok())
    {
    delete idx_expr_p;

    return nullptr;
    }

  // Ensure that set object is of the expected argument type.
  if (m_flags.is_set_any(Flag_type_check))
    {
    if (!args.m_expr_type->is_class_type(expected_type_p))
      {
      args.m_result = Result_err_typecheck_operand;

      #if defined(SK_AS_STRINGS)
        SkParameterBase * param_p = params_p->m_params.get_at(1u);

        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The '%s' argument supplied to `at_set()` / `{}:` was expected to be an object "
          "of the type '%s' and it was given type '%s' which is not compatible.",
          param_p->get_name_cstr_dbg(),
          expected_type_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());
      #endif

      delete idx_expr_p;
      delete obj_expr_p;

      return nullptr;
      }

    // Set result type
    args.m_expr_type = params_p->get_result_class()->as_finalized_generic(*receiver_type_p);
    }
  
  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  // Create invocation structure
  // SkMethodcall is guaranteed to be instance method here
  SkMethodCallBase * mcall_p = create_method_call(method_p, false, nullptr, nullptr);

  mcall_p->m_arguments.append(*idx_expr_p);
  mcall_p->m_arguments.append(*obj_expr_p);

  SkInvocation * invoke_p = SK_NEW(SkInvocation)(mcall_p, receiver_p);
  SKDEBUG_SET_CHAR_POS(invoke_p, start_pos);

  return invoke_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a constructor method call.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkMethodCallBase data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok, Result_err_context_non_method, or pretty much any
//                 other warning or error.
// See:        parse_method()
// Notes:      constructor-call = constructor-name invocation-args
//             constructor-name = '!' [instance-name]
// Author(s):   Conan Reis
SkMethodCallBase * SkParser::parse_invoke_ctor(Args & args) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse method name
  uint32_t pos    = args.m_start_pos;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  // Ensure it starts with a constructor name and not a destructor
  if ((m_str_ref_p->m_length < 3)
    || (cstr_a[pos] != '!')
    || (cstr_a[pos + 1u] == '!'))
    {
    args.m_result  = Result_err_expected_method_ctor_name;
    args.m_end_pos = pos;

    return nullptr;
    }

  ASymbol ctor_name;

  args.m_result = parse_name_method(
    pos, &args.m_end_pos, args.is_struct_wanted() ? &ctor_name : nullptr);

  if (!args.is_ok())
    {
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure method is available
  SkMethodBase * method_p = nullptr;
  SkParameters * params_p = nullptr;

  SkClassDescBase * receiver_type_p = args.m_expr_type;

  if (m_flags.is_set_any(Flag_type_check))
    {
    // Find constructor and when calling an instance constructor, ensure that if no
    // instance constructor is found do not use class constructor.
    method_p = receiver_type_p->is_metaclass()
      ? receiver_type_p->find_method_inherited(ctor_name)
      : receiver_type_p->get_key_class()->find_instance_method_inherited(ctor_name);

    if (method_p == nullptr)
      {
      args.m_result = Result_err_context_non_method;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The constructor method '%s()' does not exist for %s.",
          ctor_name.as_cstr_dbg(),
          args.m_expr_type->get_scope_desc().as_cstr());
      #endif

      return nullptr;
      }

    params_p = &method_p->get_params();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for parse to index probe
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    // We just found a new receiver, and a new invocation
    args.m_receiver_type_p = receiver_type_p;
    args.m_invocation_stack.append(InvocationInfo(&method_p->get_params(), args.m_end_pos));
    args.m_expr_type = receiver_type_p;

    if (args.is_idx_probe_halt(this))
      {
      // Found probe index, so exit
      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse optional invocation arguments
  SkMethodCallBase * mcall_p = nullptr;

  args.m_start_pos = args.m_end_pos;

  if (args.is_struct_wanted())
    {
    mcall_p = create_method_call(method_p, receiver_type_p->is_metaclass(), receiver_type_p, nullptr);

    parse_invoke_args(args, &mcall_p->m_arguments, &mcall_p->m_return_args, params_p);

    if (!args.is_ok())
      {
      delete mcall_p;

      return nullptr;
      }
    }
  else
    {
    // Validation parse
    parse_invoke_args(args, nullptr, nullptr, params_p);
    }

  // Note that initial result type is reused since a constructor returns instance being
  // constructed.
  args.m_expr_type = receiver_type_p;

  // Done parsing the invocation
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    args.m_invocation_stack.pop_last();
    }

  return mcall_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a method call.
//
// #Returns:
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
//   SkMethodCallBase data-structure otherwise nullptr is returned.
// 
// #Params:
//   args:
//     See SkParser::Args, specific considerations below:
//       m_type_p: set with class type of receiver on entry and changed to result type on exit.
//       m_result: Result_ok, Result_err_context_non_method, or pretty much any other warning or error.
//   receiver_pp:
//     receiver expression of the method
//     
// #Notes
//   method-call      = [scope] method-name invocation-args
//   scope            = class-name '@'
//   method-name      = name-predicate | constructor-name | destructor-name | convert-name
//   name-predicate   = instance-name ['?']
//   instance-name    = lowercase {alphanumeric}
//   constructor-name = '!' [instance-name]
//   destructor-name  = '!!'
//   
// #See: parse_method()
  SkMethodCallBase * SkParser::parse_invoke_method(
  // 
  Args &              args,
  SkExpressionBase ** receiver_pp // = nullptr
  ) const
  {
  SkMethodCallBase * mcall_p          = nullptr;
  SkClass *          qual_scope_p     = nullptr;  // Optional qualified class scope
  uint32_t           pos              = args.m_start_pos;
  bool               infer_receiver_b = args.m_result == Result__implicit_this;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the optional qualified class scope if it is present
  args.m_result = parse_class_scope(pos, &pos, &qual_scope_p, args.m_expr_type);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse method name
  ASymbol method_name;

  args.m_result = parse_name_method(pos, &pos, args.is_struct_wanted() ? &method_name : nullptr);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure method is available
  SkParameters *    params_p         = nullptr;
  SkMethodBase *    method_p         = nullptr;
  bool              is_class_method  = false;
  SkClassDescBase * result_type_p    = nullptr;
  SkClassDescBase * receiver_type_p  = args.m_expr_type;
  SkClassDescBase * qual_recv_type_p = receiver_type_p->qualify(qual_scope_p);

  if (m_flags.is_set_any(Flag_type_check))
    {
    method_p = find_method_inherited(qual_recv_type_p, method_name, &is_class_method);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If method not found and inferred receiver and desired type know, then infer
    // desired type class and try to match class methods from it.
    SkClass * infer_class_p = nullptr;

    if ((method_p == nullptr)
      && infer_receiver_b
      && args.m_desired_type_p
      && (args.m_desired_type_p->get_class_type() != SkClassType_class_union))
      {
      infer_class_p = args.m_desired_type_p->get_key_class();

      method_p = infer_class_p->find_class_method_inherited(method_name, &is_class_method);

      if (method_p)
        {
        receiver_type_p  = &infer_class_p->get_metaclass();
        qual_recv_type_p = receiver_type_p;

        if (receiver_pp && args.is_struct_wanted())
          {
          // receiver_pp will be nullptr prior to setting class as receiver.
          *receiver_pp = SK_NEW(SkLiteral)(infer_class_p->get_metaclass());
          SKDEBUG_SET_CHAR_POS(*receiver_pp, args.m_start_pos);
          }
        }
      }

    if (method_p == nullptr)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Make a fancy error message for not finding method.
      args.m_result  = Result_err_context_non_method;
      args.m_end_pos = pos;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The method '%s()' does not exist for %s.",
          method_name.as_cstr_dbg(),
          qual_recv_type_p->get_scope_desc().as_cstr());

        // Determine if it was an instance call on a class
        if (qual_recv_type_p->is_metaclass())
          {
          SkClass * instance_class_p = qual_recv_type_p->get_key_class();

          method_p = find_method_inherited(instance_class_p, method_name, &is_class_method);

          if (method_p)
            {
            // It was an instance method on a class
            ms_error_str.append("\nThough it does exist as an instance method.");

            if (qual_scope_p == nullptr)
              {
              ms_error_str.append_format(
                "\nDid you intend to specify the scope and possibly call an overridden method `%s@%s()`?",
                instance_class_p->get_name_cstr(),
                method_name.as_cstr_dbg());
              }
            }
          }

        if (infer_class_p)
          {
          ms_error_str.append_format(
            "\nNor does it exist as a class method `%s.%s()` for the expected type %s.",
            infer_class_p->get_name_cstr(),
            method_name.as_cstr_dbg(),
            infer_class_p->get_name_cstr());
          }

        // Check for specific C++ etc. keyword/identifier mistakes and assumptions.
        switch (method_name.get_id())
          {
          case ASymbolId_for:
          case ASymbolId_while:
            ms_error_str.append(
              "\nSkookumScript uses 'loop' and a nested 'exit' to do traditional iteration:\n"
              "  loop\n"
              "    [\n"
              "    do_stuff\n"
              "    if exit_test? [exit]\n"
              "    ]");
            break;
          }
      #endif  // SK_AS_STRINGS

      return nullptr;
      }

    params_p      = &method_p->get_params();
    result_type_p = params_p->get_result_class()->as_finalized_generic(*receiver_type_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for parse to index probe
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    // We just found a new receiver, and a new invocation
    args.m_receiver_type_p = result_type_p;
    args.m_invocation_stack.append(InvocationInfo(&method_p->get_params(), pos));
    args.m_end_pos = pos;
    args.m_expr_type = result_type_p;

    if (args.is_idx_probe_halt(this))
      {
      // Found probe index, so exit
      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse any passed arguments
  args.m_start_pos = pos;

  if (args.is_struct_wanted())
    {
    mcall_p = create_method_call(method_p, is_class_method, receiver_type_p, qual_scope_p);

    if (!parse_invoke_args(args, &mcall_p->m_arguments, &mcall_p->m_return_args, params_p))
      {
      delete mcall_p;
      mcall_p = NULL;
      }
    }
  else
    {
    // No struct desired
    parse_invoke_args(args, nullptr, nullptr, params_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set type after args
  if (m_flags.is_set_any(Flag_type_check))
    {
    args.m_expr_type = result_type_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Done parsing the invocation
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    args.m_invocation_stack.pop_last();
    }

  return mcall_p;
  }

//---------------------------------------------------------------------------------------
// Create a SkMethodCallBase object for the supplied method and first argument.
//
// #See Also  parse_invoked_method(), parse_invoke_instantiate()
// #Author(s) Conan Reis
SkMethodCallBase * SkParser::parse_invoke_instance_method_arg1(
  // See SkParser::Args, specific considerations below:
  //   m_desired_type_p: type of first parameter or nullptr;
  //   m_type_p: argument 1 type
  //   m_result: Result_ok, Result_err_expected_operator, Result_err_context_non_method,
  //     Result_err_typecheck_operand, or pretty much any other warning or error.
  Args & args,
  SkMethodBase * method_p,
  SkExpressionBase * arg1_p
  ) const
  {
  if (m_flags.is_set_any(Flag_type_check))
    {
    SkParameters *    params_p = &method_p->get_params();
    SkParameterBase * param_p  = params_p->get_param_list().get_first();

    if (param_p == nullptr)
      {
      args.m_result = Result_err_context_invoke_arg1;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Argument passed to method '%s' but it has no parameters.\n"
          "[Either pass no arguments or add one or more parameters to the method.]",
          method_p->as_string_name().as_cstr());
      #endif

      return nullptr;
      }

    uint32_t param_remain = params_p->get_arg_count_min_after_arg1();

    if (param_remain)
      {
      args.m_result = Result_err_context_invoke_arg_missing;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Only the first argument was supplied to the method '%s' and %u more argument%s expected.\n"
          "[Either more arguments must be supplied or more parameters must be either given "
          "defaults or removed.]",
          method_p->as_string_name().as_cstr(),
          (param_remain == 1u) ? " was" : "s were",
          param_remain);
      #endif

      return nullptr;
      }

    SkClassDescBase * param_type_p = args.m_desired_type_p;

    if (param_type_p == nullptr)
      {
      param_type_p = param_p->get_expected_type()->as_finalized_generic(*args.m_expr_type);
      }

    if (!args.m_expr_type->is_class_type(param_type_p))
      {
      args.m_result = Result_err_typecheck_invoke_arg;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The first argument supplied to the '%s' method parameter '%s' was expected to "
          "be an object of the type '%s' and it was given type '%s' which is not compatible.",
          method_p->as_string_name().as_cstr(),
          param_p->get_name_cstr_dbg(),
          param_type_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());
      #endif

      return nullptr;
      }
    }

  args.m_result = Result_ok;

  if (args.is_struct_wanted())
    {
    SkMethodCallBase * mcall_p = create_method_call(method_p, false, nullptr, nullptr);

    mcall_p->m_arguments.append(*arg1_p);

    return mcall_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an operator call.
//
// #Notes
//   operator-selector= postfix-operator | (binary-operator ws expression)
//   binary-operator  = math-operator | compare-operator | logical-operator | ':='
//   math-operator    = '+' | '+=' | '-' | '-=' | '*' | '*=' | '/' | '/='
//   compare-operator = '=' | '~=' | '>' | '>=' | '<' | '<='
//   logical-operator = 'and' | 'or' | 'xor' | 'nand' | 'nor' | 'nxor'
//   postfix-operator = '++' | '--'
//
// #See Also  parse_method()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkMethodCallBase data-structure otherwise nullptr is returned.
  SkMethodCallBase *
SkParser::parse_operator_call(
  // See SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and changed to result type on exit.
  //   m_result: Result_ok, Result_err_expected_operator, Result_err_context_non_method,
  //     Result_err_typecheck_operand, or pretty much any other warning or error.
  Args & args
  ) const
  {
  ASymbol             op_name;
  SkMethodCallBase *  mcall_p   = nullptr;
  uint32_t            pos       = args.m_start_pos;
  bool                binary_op = true;

  args.m_result = Result_ok;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up quick access chars.
  // - This mechanism ensures that no read will occur beyond the end of the string/file.
  uint32_t     length = m_str_ref_p->m_length;
  const char * cstr_a = m_str_ref_p->m_cstr_p;

  char ch1 = cstr_a[pos];    
  char ch2 = '\0';
  char ch3 = '\0';
  char ch4 = '\0';
  char ch5 = '\0';

  if ((pos + 1u) < length)
    {
    ch2 = cstr_a[pos + 1u];

    if ((pos + 2u) < length)
      {
      ch3 = cstr_a[pos + 2u];

      if ((pos + 3u) < length)
        {
        ch4 = cstr_a[pos + 3u];

        if ((pos + 4u) < length)
          {
          ch5 = cstr_a[pos + 4u];
          }
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse operator name
  switch (ch1)
    {
    case '=':
      pos++;

      // Check for C++ mistake/assumption to use ==
      if (ch2 == '=')
        {
        args.m_result = Result_err_unexpected_cpp;

        #if defined(SK_AS_STRINGS)
            ms_error_str.empty();
            ms_error_str.append(
              "SkookumScript uses a single '=' for a logical/Boolean 'equal to' operator.\n"
              "[Also ':=' is used for assignment and ':' is used to bind a variable to a new object.]");
        #endif
        break;
        }

      op_name = ASymbolX_equalQ;
      break;

    case '>':
      if (ch2 == '=')                      // >=
        {
        pos     += 2u;
        op_name  = ASymbolX_greater_or_equalQ;
        }
      else                                 // >
        {
        pos++;
        op_name = ASymbolX_greaterQ;
        }
      break;

    case '<':
      switch (ch2)
        {
        case '=':                          // <=
          pos     += 2u;
          op_name  = ASymbolX_less_or_equalQ;
          break;
        
        case '<':                          // <<
          pos++;
          args.m_result  = Result_err_unexpected_cpp;

          #if defined(SK_AS_STRINGS)
            ms_error_str.empty();
            ms_error_str.append(
              "SkookumScript uses Integer.bit_shift_up() instead of a bitwise shift << operator\n"
              "  - num.bit_shift_up(bit_count).");
          #endif

        default:                           // <
          pos++;
          op_name = ASymbolX_lessQ;
        }
      break;

    case '+':
      switch (ch2)
        {
        case '=':                          // +=
          pos     += 2u;
          op_name  = ASymbol_add_assign;
          break;

        case '+':                          // ++
          pos       += 2u;
          op_name    = ASymbol_increment;
          binary_op  = false;
          break;

        default:                           // +
          pos++;
          op_name = ASymbol_add;
        }
      break;

    case '-':
      // Note:  deprecated "->" was checked for previously
      switch (ch2)
        {
        case '=':                          // -=
          pos     += 2u;
          op_name  = ASymbol_subtract_assign;
          break;

        case '-':                          // --
          pos       += 2u;
          op_name    = ASymbol_decrement;
          binary_op  = false;
          break;

        default:                           // -
          pos++;
          op_name = ASymbol_subtract;
        }
      break;

    case '*':
      if (ch2 == '=')                      // *=
        {
        pos     += 2u;
        op_name  = ASymbol_multiply_assign;
        }
      else                                 // *
        {
        pos++;
        op_name = ASymbol_multiply;
        }
      break;

    case '/':
      if (ch2 == '=')                      // /=
        {
        pos     += 2u;
        op_name  = ASymbol_divide_assign;
        }
      else                                 // /
        {
        pos++;
        op_name = ASymbol_divide;
        }
      break;

    case ':':
      if (ch2 == '=')                      // :=
        {
        pos     += 2u;
        op_name  = ASymbol_assign;
        }
      else  // $Revisit - CReis Take a close look at this - could be a bind?
        {
        args.m_result = Result_err_expected_operator;
        }
      break;

    case 'a':
      // Check for "and"
      if ((ch2 ==  'n') && (ch3 == 'd') && (ch4 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch4)])
        {
        pos     += 3u;
        op_name  = ASymbol_and;
        break;
        }

      args.m_result = Result_err_expected_operator;
      break;

    case 'o':
      // Check for "or"
      if ((ch2 ==  'r') && (ch3 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch3)])
        {
        pos     += 2u;
        op_name  = ASymbol_or;
        break;
        }

      args.m_result = Result_err_expected_operator;
      break;

    case 'x':
      // Check for "xor"
      if ((ch2 ==  'o') && (ch3 == 'r') && (ch4 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch4)])
        {
        pos     += 3u;
        op_name  = ASymbol_xor;
        break;
        }

      args.m_result = Result_err_expected_operator;
      break;

    case 'n':
      // Check for "nand"
      if ((ch2 ==  'a') && (ch3 == 'n') && (ch4 == 'd') && (ch5 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch5)])
        {
        pos     += 4u;
        op_name  = ASymbol_nand;
        break;
        }

      // Check for "nor"
      if ((ch2 ==  'o') && (ch3 == 'r') && (ch4 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch4)])
        {
        pos     += 3u;
        op_name  = ASymbol_nor;
        break;
        }

      // Check for "nxor"
      if ((ch2 ==  'x') && (ch3 == 'o') && (ch4 == 'r') && (ch5 != '(') && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch5)])
        {
        pos     += 4u;
        op_name  = ASymbol_nxor;
        break;
        }

      args.m_result = Result_err_expected_operator;
      break;

    case '~':
      switch (ch2)
        {
        case '=':                          // ~=
          pos     += 2u;
          op_name  = ASymbolX_not_equalQ;
          break;

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Deprecated operators
        default:                           // ~
          pos++;
          args.m_result = Result_err_unexpected_deprecated;

          #if defined(SK_AS_STRINGS)
            ms_error_str.empty();
            ms_error_str.append("Use 'not' prefix operator or '.not()' method for Boolean logical negation - not expr [OR] expr.not");
          #endif
        }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Deprecated/common mistake operators
    case '&':                              // & &&
      // Also catches C++ mistake/assumption to use &&
      pos++;
      args.m_result = Result_err_unexpected_cpp;

      #if defined(SK_AS_STRINGS)
        ms_error_str.empty();
        ms_error_str.append(
          "Use 'and' for Boolean logical and operator - expr1 and expr2.\n"
          "[And use Integer.bit_and() for bitwise modifications - num.bit_and(flag).]");
      #endif
      break;

    case '|':                              // | ||
      // Also catches C++ mistake/assumption to use ||
      pos++;
      args.m_result  = Result_err_unexpected_cpp;

      #if defined(SK_AS_STRINGS)
        ms_error_str.empty();
        ms_error_str.append(
          "Use 'or' for Boolean logical or operator - expr1 or expr2.\n"
          "[And use Integer.bit_or() for bitwise modifications - num.bit_or(flag).]");
      #endif
      break;

    case '!':
      // Check for C++ mistake/assumption to use !=
      if (ch2 == '=')
        {
        pos++;
        args.m_result  = Result_err_unexpected_cpp;

        #if defined(SK_AS_STRINGS)
          ms_error_str.empty();
          ms_error_str.append(
            "SkookumScript uses '~=' for a logical/Boolean 'not equal to' comparison operator.\n"
             "['=' is used for a logical/Boolean 'equal to' comparison operator.]");
        #endif
        break;
        }

      args.m_result = Result_err_expected_operator;
      break;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    default:  // Invalid operator
      args.m_result = Result_err_expected_operator;
    }

  if (args.is_ok())
    {
    SkParameters *    params_p        = nullptr;
    SkClassDescBase * receiver_type_p = args.m_expr_type;
    SkClassDescBase * result_type_p   = SkBrain::ms_object_class_p;
    SkMethodBase *    method_p        = nullptr;
    bool              is_class_method = false;

    if (m_flags.is_set_any(Flag_type_check))
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Ensure method is available

      method_p = receiver_type_p->find_method_inherited(op_name, &is_class_method);

      if (method_p)
        {
        params_p      = &method_p->get_params();
        result_type_p = params_p->get_result_class()->as_finalized_generic(*receiver_type_p);
        }
      else
        {
        args.m_result = Result_err_context_non_method;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The operator instance method '%s()' [%s] does not exist for %s.",
            op_name.as_cstr_dbg(),
            method_to_operator(op_name).as_cstr_dbg(),
            receiver_type_p->get_scope_desc().as_cstr());
        #endif
        }
      }

    if (args.is_ok())
      {
      SkExpressionBase * operand_p = nullptr;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Parse operand if it is a binary operator
      // $Revisit - CReis Update with extra checking like parse_invoke_instance_method_arg1()
      if (binary_op)
        {
        // Eat {whitespace}
        args.m_result = parse_ws_any(pos, &pos);

        if (args.is_ok())
          {
          SkClassDescBase * expected_type_p = nullptr;

          // Check for proper argument count
          if (params_p && params_p->m_params.get_length() != 1)
            {
            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "The binary operator method '%s@%s()' has %d arguments which is not permissable. All binary operators must have exactly one argument.",
                receiver_type_p->get_scope_desc().as_cstr(),
                op_name.as_cstr_dbg(),
                params_p ? params_p->m_params.get_length() : 0);
            #endif

            args.m_result = Result_err_unexpected_parameter_binary;
            }
          else
            {
            if (m_flags.is_set_any(Flag_type_check))
              {
              expected_type_p = params_p->m_params.get_first()->get_expected_type()->as_finalized_generic(*receiver_type_p);
              args.m_desired_type_p = expected_type_p;
              }

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Test for parse to index probe
            if (args.m_flags & ArgFlag_parse_to_idx_probe)
              {
              // We just found a new receiver
              args.m_receiver_type_p = receiver_type_p;
              args.m_expr_type = result_type_p;
              args.m_end_pos = pos;

              if (args.is_idx_probe_halt(this))
                {
                // Found probe index, so exit
                return nullptr;
                }
              }

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Parse operand
            args.m_start_pos = pos;
            operand_p = parse_expression(args, SkInvokeTime_immediate);
            pos = args.m_end_pos;

            // Ensure that operand is of the expected argument type.
            if (args.is_ok()
              && m_flags.is_set_any(Flag_type_check)
              && !args.m_expr_type->is_class_type(expected_type_p))
              {
              args.m_result = Result_err_typecheck_operand;

              #if defined(SK_AS_STRINGS)
                SkParameterBase * param_p = params_p->get_param_list().get_first();

                ms_error_str.ensure_size_empty(500u);
                ms_error_str.format(
                  "The argument supplied to operator parameter `%s` was expected to be an object "
                  "of the type `%s` and it was given type `%s` which is not compatible.",
                  param_p->get_name_cstr_dbg(),
                  expected_type_p->as_code().as_cstr(),
                  args.m_expr_type->as_code().as_cstr());
              #endif

              if (operand_p)
                {
                // Free operand structure if once was created
                delete operand_p;
                operand_p = nullptr;
                }
              }
            }
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Store results
      if (args.is_ok())
        {
        args.m_expr_type = result_type_p;

        if (args.is_struct_wanted())
          {
          mcall_p = create_method_call(method_p, is_class_method, receiver_type_p, nullptr);

          if (operand_p)
            {
            mcall_p->m_arguments.append(*operand_p);
            }
          }
        }
      }
    }

  args.m_end_pos = pos;
  
  return mcall_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a coroutine call.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkCoroutineCall data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_type_p: set with class type of receiver on entry and changed to result
//                 type on exit.
//               m_result: Result_ok, Result_err_context_non_coroutine, or pretty much
//                 any other warning or error.
// See:        parse_coroutine()
// Notes:      coroutine-call  = [scope] coroutine-name invocation-args
//             scope           = class-name '@'
//             script-name     = '_' lowercase {alphanumeric}
// Author(s):   Conan Reis
SkCoroutineCall * SkParser::parse_invoke_coroutine(Args & args) const
  {
  SkCoroutineCall * rcall_p      = nullptr;
  SkClass *         qual_scope_p = nullptr;  // Optional qualified class scope
  uint32_t          pos          = args.m_start_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the optional qualified class scope if it is present
  args.m_result = parse_class_scope(pos, &pos, &qual_scope_p, args.m_expr_type);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse method name
  ASymbol coroutine_name;

  args.m_result = parse_name_coroutine(pos, &pos, args.is_struct_wanted() ? &coroutine_name : nullptr);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that the coroutine is valid
  SkParameters *    params_p    = nullptr;
  SkCoroutineBase * coroutine_p = nullptr;

  if (m_flags.is_set_any(Flag_type_check))
    {
    SkClassDescBase * qual_recv_type_p = args.m_expr_type->qualify(qual_scope_p);
    coroutine_p = qual_recv_type_p->find_coroutine_inherited(coroutine_name);

    if (coroutine_p == nullptr)
      {
      args.m_result  = Result_err_context_non_coroutine;
      args.m_end_pos = pos;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The coroutine '%s()' does not exist for %s.",
          coroutine_name.as_cstr_dbg(),
          qual_recv_type_p->get_scope_desc().as_cstr());
      #endif

      return nullptr;
      }

    params_p = &coroutine_p->get_params();
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Test for parse to index probe
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    // We just found a new receiver, and a new invocation
    args.m_receiver_type_p = SkBrain::ms_invoked_coroutine_class_p;
    args.m_invocation_stack.append(InvocationInfo(&coroutine_p->get_params(), pos));
    args.m_end_pos = pos;
    args.m_expr_type = SkBrain::ms_invoked_coroutine_class_p;

    if (args.is_idx_probe_halt(this))
      {
      // Found probe index, so exit
      return nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse any passed arguments
  args.m_start_pos = pos;

  if (args.is_struct_wanted())
    {
    rcall_p = SK_NEW(SkCoroutineCall)(coroutine_p, qual_scope_p);

    if (!parse_invoke_args(args, &rcall_p->m_arguments, &rcall_p->m_return_args, params_p))
      {
      delete rcall_p;
      }
    }
  else
    {
    // No struct desired
    parse_invoke_args(args, nullptr, nullptr, params_p);
    }

  // Store result type of coroutine
  args.m_expr_type = SkBrain::ms_invoked_coroutine_class_p;

  // Done parsing the invocation
  if (args.m_flags & ArgFlag_parse_to_idx_probe)
    {
    args.m_invocation_stack.pop_last();
    }

  return rcall_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create an invoke selector which is a call to a method or
// coroutine.
// 
// #Returns:
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
//   data-structure derived from SkInvokeBase otherwise nullptr is returned.
//   
// #Params:
//   args: see SkParser::Args, specific considerations below:
//     m_type_p: set with class type of receiver on entry and changed to result type on exit.
//     m_result: Result_ok, Result_err_expected_invoke_selector,
//       Result_err_expected_invoke_select_op, or pretty much any other warning or error.
//   test_op: indicates whether an operator call should be tested for or not.
//   receiver_pp:
//     The initial receiver/owner expression or inferred receiver (such as implied 'this').
//     
// #Notes:
//   invocation      = invoke-call | invoke-cascade
//   invoke-call     = ([expression ws '.'] invoke-selector) | operator-call
//   invoke-cascade  = expression ws '.' ws '[' {ws invoke-selector | operator-selector}2+ ws ']'
//   invoke-selector = method-call | coroutine-call
//   method-call     = [scope] method-name invocation-args
//   coroutine-call  = [scope] coroutine-name invocation-args
SkInvokeBase * SkParser::parse_invoke_selector(
  Args &              args,
  bool                test_op,
  SkExpressionBase ** receiver_pp // = nullptr
  ) const
  {
  uint32_t       pos;
  uint32_t       start_pos        = args.m_start_pos;
  bool           infer_receiver_b = args.m_result == Result__implicit_this;
  bool           set_end_b        = true;
  SkInvokeBase * call_p           = nullptr;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Go past class scope if it is present
  args.m_result = parse_class_scope(start_pos, &pos);

  if (args.is_ok())
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
    char   ch     = cstr_a[pos];

    // Next possible error
    args.m_result = Result_err_expected_invoke_selector;

    if (ch == '_')
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // It is a coroutine?
      if (ms_is_lowercase[uint8_t(cstr_a[++pos])])
        {
        set_end_b = false;

        // It is a coroutine call
        call_p = parse_invoke_coroutine(args);
        }
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // It is a method call or an operator call

      // Is it a non-operator method?
      if ((ch == '!') || ms_char_match_table[ACharMatch_alphabetic][uint8_t(ch)])
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is a non-operator method
        if (infer_receiver_b)
          {
          // Indicate that there is an implicit receiver.
          args.m_result    = Result__implicit_this;
          }

        set_end_b = false;
        call_p    = parse_invoke_method(args, receiver_pp);
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // It is an operator or an error
        if (test_op)
          {
          set_end_b = false;
          call_p    = parse_operator_call(args);

          // Determine if parse advanced
          if (args.m_end_pos == start_pos)
            {
            args.m_result = Result_err_expected_invoke_select_op;
            }
          }
        }
      }
    }

  if (set_end_b)
    {
    args.m_end_pos = pos;
    }

  return call_p;
  }

//---------------------------------------------------------------------------------------
//  Parses starting at 'start_pos' attempting to create a character escape
//              sequence.
// Returns:     Result_ok, Result_err_expected_char, Result_err_expected_char_number,
//              Result_err_size_radix_small, Result_err_size_radix_large or 
//              Result_err_unexpected_eof 
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Arg          ch_p - pointer to location to store parsed character.  It is not
//              written to if it is set to nullptr or if the result is not Result_ok.
//              (Default nullptr)
// Examples:    if (parse.parse_literal_char_esc_seq(11u, &end_pos, &ch) == Result_ok)
// Notes:       escape-sequence = '\' (integer-literal | printable-char)
//
//              Valid escape sequences:
//                Alert           \a  7
//                Backspace       \b  8
//                Form feed       \f  12
//                New line        \n  10
//                Carriage return \r  13
//                Horizontal tab  \t  9
//                Vertical tab    \v  11
//                Any Character (0-255) \ASCII number
//                Any Printable Character \ASCII character (other than above)
// See:         SkString::to_escape_string()
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_literal_char_esc_seq(
  uint32_t   start_pos, // = 0
  uint32_t * end_pos_p, // = nullptr
  char * ch_p   // = nullptr
  ) const
  {
  eResult result = Result_err_unexpected_eof;  // Next possible error
  uint32_t    length = m_str_ref_p->m_length;

  if ((length - start_pos) >= 2u)
    {
    char ch = m_str_ref_p->m_cstr_p[start_pos];

    // Next possible error
    result = Result_err_expected_char;

    if (ch == '\\')
      {
      start_pos++;
      ch = m_str_ref_p->m_cstr_p[start_pos];
      // All possibilities increment and give an OK result - except for digits
      start_pos++;
      result = Result_ok;
      switch (ch)
        {
        case 'a':  // Alert
          ch = '\a';
          break;

        case 'b':  // Backspace
          ch = '\b';
          break;

        case 'f':  // Form Feed
          ch = '\f';
          break;

        case 'n':  // New line
          ch = '\n';
          break;

        case 'r':  // Carriage return
          ch = '\r';
          break;

        case 't':  // Horizontal tab
          ch = '\t';
          break;

        case 'v':  // Vertical tab
          ch = '\v';
          break;

        default:
          if (ms_is_digit[uint8_t(ch)])
            {
            tSkInteger value;
            
            start_pos--; // Take back premature advance
            result = parse_literal_integer(start_pos, &start_pos, &value);

            if (result == Result_ok) // Use char ASCII value
              {
              // Next possible error
              result = Result_err_expected_char_number;

              // Check character range
              if (a_is_ordered(tSkInteger(0), value, tSkInteger(AString_ansi_charset_length - 1)))
                {
                ch     = char(value);
                result = Result_ok;
                }
              }
            }
          // else insert the character as if the \ did not exist
        }  // End switch

      if (ch_p && (result == Result_ok))
        {
        *ch_p = ch;
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = start_pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Helper method that parses simple string of leading digits and optional separators.
// It is assumed that the initial character at start_pos has already been confirmed to be
// a digit.
// 
// #Notes
//   digits-lead = '0' | (non-0-digit {['_'] digit})
//   digit       = '0' | non-0-digit
//   non-0-digit = '1' | ... | '9'
//   
// #Author(s)  Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns Result_ok or Result_err_expected_seperator_digit 
  SkParser::eResult
SkParser::parse_digits_lead(
  uint32_t start_pos,
  uint32_t * end_pos_p,
  tSkInteger * int_p
  ) const
  {
  char *  cstr_a = m_str_ref_p->m_cstr_p;
  tSkInteger value = cstr_a[start_pos] - '0';

  start_pos++;

  // If first digit not zero, continue - don't allow leading zeros
  if (value)
    {
    char ch          = cstr_a[start_pos];
    bool separator_b = false;

    // Skip any separator that is between digits
    if (ch == '_')
      {
      // Encountered a separator
      separator_b = true;
      start_pos++;
      ch = cstr_a[start_pos];
      }

    while (ms_is_digit[uint8_t(ch)])
      {
      value *= 10;
      value += ch - '0';
      start_pos++;
      ch = cstr_a[start_pos];

      separator_b = false;

      // Skip any separator that is between digits
      if (ch == '_')
        {
        // Encountered a separator
        separator_b = true;
        start_pos++;
        ch = cstr_a[start_pos];
        }
      }

    // Ensure no trailing separator - can only be between digits
    if (separator_b)
      {
      *end_pos_p = start_pos;

      return Result_err_expected_seperator_digit;
      }
    }

  *int_p = value;
  *end_pos_p = start_pos;

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create an integer number.
//
// #Examples
//   if (parse.parse_literal_integer(11u, &end_pos, &int_num) == Result_ok)
//
// #Notes
//   integer-literal = ['-'] digits-lead ['r' big-digit {['_'] big-digit}]
//   digits-lead     = '0' | (non-0-digit {['_'] digit})
//   digit           = '0' | non-0-digit
//   non-0-digit     = '1' | ... | '9'
//   big-digit       = digit | alphabetic
//   
//   Also checks for common C++ non-decimal integer literals 0b##, 0## and 0x## and gives
//   error message describing how to make non-decimal integers with SkookumScript using
//   the radix/base prefix.
//
// #See Also  parse_literal_real(), parse_literal_number()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
  // Result_ok, Result_err_expected_literal_int, Result_err_expected_seperator_digit,
  // Result_err_unexpected_cpp, Result_err_size_radix_small, Result_err_size_radix_large,
  // Result_err_expected_digit_radix
  SkParser::eResult
SkParser::parse_literal_integer(
  // Index to begin lexical analysis
  uint32_t start_pos, // = 0u
  // Address to store index that lexical analysis stopped at - ignored if nullptr.
  uint32_t * end_pos_p, // = nullptr
  // Address to store parsed integer - ignored if nullptr.
  tSkInteger * int_p, // = nullptr
  // Address to store specified radix/base - ignored if nullptr.  Set to 0 if radix not
  // specified and SkParser_integer_radix_default(10) used.
  uint32_t * radix_p // = nullptr
  ) const
  {
  ASetOnReturn<uint32_t> set_end(&start_pos, end_pos_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure there is at least 1 character to parse
  if (m_str_ref_p->m_length <= start_pos)
    {
    return Result_err_expected_literal_int;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional negation
  bool   negative_b = false;
  char * cstr_a     = m_str_ref_p->m_cstr_p;
  char   ch         = cstr_a[start_pos];

  if (ch == '-')
    {
    negative_b = true;
    start_pos++;
    ch = cstr_a[start_pos];
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for initial leading digits.
  // If radix not used then it will be the end value. If it is used as the radix value any
  // separators '_' will probably be overkill in a max 2 digit radix value though it is
  // allowed for simplicity for the syntax and parsing efficiency.
  // Syntax:  '0' | (non-0-digit {['_'] digit})
  eResult    result = Result_ok;
  tSkInteger value  = 0;  // $Revisit CReis - Could use int64_t to allow checking for values that are too large.

  if (ms_is_digit[uint8_t(ch)])
    {
    result = parse_digits_lead(start_pos, &start_pos, &value);

    if (result != Result_ok)
      {
      return result;
      }

    ch = cstr_a[start_pos];
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for optional radix 'r'.
  // Also look for any unexpected C++ style integer literals like 0x42, 042 or 0b101.
  bool radix_b = false;

  switch (ch)
    {
    case 'r':  // Radix/base specified
      radix_b = true;
      break;

    case 'b':  // Unexpected C++ binary integer literal
    case 'B':
      start_pos++;
      #if defined(SK_AS_STRINGS)
        ms_error_str.empty();
        ms_error_str.append(
          "SkookumScript uses a (r)adix prefix 2r### to indicate a binary integer literal like 2r101 for 5.\n"
          "Syntax:  ['-'] '2r' '0' | '1' {['_'] '0' | '1'}");
      #endif
      return Result_err_unexpected_cpp;

    case 'x':  // Unexpected C++ hexadecimal integer literal
    case 'X':
      start_pos++;
      #if defined(SK_AS_STRINGS)
        ms_error_str.empty();
        ms_error_str.append(
          "SkookumScript uses a (r)adix prefix 16r### to indicate a hexadecimal integer literal like 16rff for 255.\n"
          "Syntax:  ['-'] '16r' big-digit {['_'] big-digit}");
      #endif
      return Result_err_unexpected_cpp;

    default:
      if ((value == 0) && ms_is_digit[uint8_t(ch)])
        {
        // Leading zero or unexpected C++ octal integer literal
        #if defined(SK_AS_STRINGS)
          ms_error_str.empty();
          ms_error_str.append(
            "Integer literals may not have leading zeros.\n\n"
            "If an octal number was intended, SkookumScript uses a (r)adix prefix 8r### like 8r20 for 16.\n"
            "Syntax:  ['-'] '8r' 0..7 {['_'] 0..7}");
        #endif
        return Result_err_unexpected_cpp;
        }
    }

  
  int32_t radix = 0;  // 0 Indicates radix not specified.

  if (radix_b)
    {
    radix = value;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure radix size is valid
    if (radix < SkParser_integer_radix_min)
      {
      return Result_err_size_radix_small;
      }

    if (radix > SkParser_integer_radix_max)
      {
      return Result_err_size_radix_large;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Get radix based value
    // Syntax:  big-digit {['_'] big-digit}
    start_pos++;
    ch = cstr_a[start_pos];
    value = 0;

    uint32_t value_idx   = start_pos;  // Starting index of value
    bool     separator_b = false;

    if (radix <= 10)
      {
      // Literal may only include digits
      while (a_is_ordered('0', ch, char('0' + radix - 1)))
        {
        value *= radix;
        value += ch - '0';
        start_pos++;
        ch = cstr_a[start_pos];

        separator_b = false;

        // Skip any separator that is between digits
        if (ch == '_')
          {
          // Encountered a separator
          separator_b = true;
          start_pos++;
          ch = cstr_a[start_pos];
          }
        }
      }
    else
      {
      // Literal may included digits 0..9 and letters a..(a+radix-10)
      while (ms_is_digit[uint8_t(ch)] || a_is_ordered(int('a'), int(to_lowercase(ch)), (int)('a' + radix - 11)))
        {
        value *= radix;
        value += ms_is_digit[uint8_t(ch)] ? (ch - '0') : (to_lowercase(ch) - 'a' + 10);
        start_pos++;
        ch = cstr_a[start_pos];

        separator_b = false;

        // Skip any separator that is between digits
        if (ch == '_')
          {
          // Encountered a separator
          separator_b = true;
          start_pos++;
          ch = cstr_a[start_pos];
          }
        }
      }

    // Ensure at least one radix based digit found
    if (start_pos == value_idx)
      {
      return Result_err_expected_digit_radix;
      }

    // Ensure no trailing separator - can only be between digits
    if (separator_b)
      {
      return Result_err_expected_seperator_digit;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Integer successfully parsed
  if (result == Result_ok)
    {
    if (int_p)
      {
      *int_p = negative_b ? -value : value;
      }

    if (radix_p)
      {
      *radix_p = static_cast<uint32_t>(radix);
      }
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a List literal tail.
//
// Returns:
//   If parse valid and args.is_struct_wanted() == true it returns a dynamically
//   allocated SkLiteralList data-structure otherwise nullptr is returned.
//
// Params:
//   args: see SkParser::Args, specific considerations below:
//     m_result: Result_ok or pretty much any other warning or error.
//   list_class_p:
//     Previously parsed list class type. It includes an item class if `item_type_b` is
//     set to true.
//   item_type_b:
//     Indicates whether an item class type was specified (true) or not (false).  It is
//     not modified if it is set to nullptr or if the result is not Result_ok.
//   ctor_p: optional constructor call with any needed argument expressions
//
// Notes:
//   list-literal       = list-literal-class list-literal-tail
//   list-literal-class = [(list-class constructor-name invocation-args) | class-name]
//   list-literal-tail  = '{' ws [expression {ws ',' ws expression} ws] '}'
//
//   This method parses list-literal-tail and the optional list-literal-class is parsed
//   by parse_instantiate_or_list().
//
// See: parse_instantiate_or_list()
SkLiteralList * SkParser::parse_literal_list(
  Args &             args,         // = ms_def_args.reset()
  SkTypedClass *     list_class_p, // = nullptr
  bool               item_type_b,  // = false
  SkMethodCallBase * ctor_p        // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for start of list
  uint32_t start_pos = args.m_start_pos;
  uint32_t length    = m_str_ref_p->m_length;
  char *   cstr_a    = m_str_ref_p->m_cstr_p;

  if (((length - start_pos) == 0u) || (cstr_a[start_pos] != '{'))
    {
    args.m_result  = Result_err_expected_literal_list;
    args.m_end_pos = start_pos;
    delete ctor_p;
    return nullptr;
    }

  args.m_start_pos++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    delete ctor_p;
    return nullptr;
    }

  uint32_t pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup list type and item type

  // Item type determined via in preferred order of:
  // - list-class constructor
  // - specified class
  // [above done prior to calling this method]
  // - type inferred using desired type
  // - types of initial items used
  // - Object

  // Infer or adjust list type based on desired type
  if (args.m_desired_type_p)
    {
    SkClassDescBase * desired_type_p = m_context.finalize_generic(*args.m_desired_type_p);

    // Determine if desired class type is same
    if (desired_type_p == list_class_p)
      {
      // Same type as desired so assume that item type was specified
      item_type_b = true;
      }
    else
      {
      // Determine if desired class type is a valid list type
      if ((desired_type_p->get_class_type() == SkClassType_typed_class)
        && desired_type_p->get_key_class()->is_class(*SkBrain::ms_list_class_p))
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Determine list type
        // If list type not specified or if the list type specified is just `List` then
        // override with desired list class.
        SkClass *      inferred_list_class_p;
        SkTypedClass * desired_list_p = static_cast<SkTypedClass *>(desired_type_p);
        bool           inferred_b     = false;

        if ((list_class_p == nullptr)
          || (list_class_p->get_key_class() == SkBrain::ms_list_class_p))
          {
          inferred_b = true;
          inferred_list_class_p = desired_list_p->get_key_class();
          }
        else
          {
          inferred_list_class_p = list_class_p->get_key_class();
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Determine item type
        SkClassDescBase * inferred_item_class_p;

        if (!item_type_b)
          {
          inferred_b = true;
          item_type_b = true;
          inferred_item_class_p = desired_list_p->get_item_type();
          }
        else
          {
          inferred_item_class_p = list_class_p->get_item_type();
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Infer type or use one specified?
        if (inferred_b)
          {
          // The previous SkTypedClass will be collected after all the parsing is completed
          list_class_p = SkTypedClass::get_or_create(inferred_list_class_p, inferred_item_class_p);
          }
        }
      }
    }

  if (list_class_p)
    {
    args.m_expr_type = list_class_p;
    }

  SkClassUnion      item_type;
  SkClassDescBase * item_type_p = item_type_b
    ? list_class_p->get_item_type()
    : &item_type;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup list object
  SkLiteralList * list_p = nullptr;

  if (args.is_struct_wanted())
    {
    list_p = SK_NEW(SkLiteralList)(
      list_class_p ? list_class_p->get_key_class() : SkBrain::ms_list_class_p,
      ctor_p);
    SKDEBUG_SET_CHAR_POS(list_p, start_pos);

    // Ensure that constructor call is not deleted twice
    ctor_p = nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for empty list and early end of list literal
  if (cstr_a[pos] == '}')
    {
    // It is a simple empty list
    pos++;

    if (!item_type_b)
      {
      item_type_p = SkBrain::ms_object_class_p;
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Iterate through item expressions
    char               ch;
    SkExpressionBase * item_p;

    // Next possible error
    args.m_result = Result_err_unexpected_eof;

    while (pos < length)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Parse item expression
      args.m_start_pos = pos;
      args.m_desired_type_p = item_type_b ? item_type_p : nullptr;
      item_p = parse_expression(args, SkInvokeTime_immediate);
      pos = args.m_end_pos;

      if (!args.is_ok())
        {
        // Error in item so exit while loop
        break;
        }

      if (item_p)
        {
        list_p->m_item_exprs.append(*item_p);
        }

      // Ensure item is of the correct type
      if (m_flags.is_set_any(Flag_type_check))
        {
        if (item_type_b)
          {
          if (!args.m_expr_type->is_class_type(item_type_p))
            {
            // Element is not of the specified desired class type
            args.m_result = Result_err_typecheck_list_item;

            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "Supplied list item is of class type '%s' which is not compatible with expected item type '%s'.\n",
                args.m_expr_type->as_code().as_cstr(),
                item_type_p->as_code().as_cstr());
            #endif

            // Exit while loop
            break;
            }
          }
        else
          {
          // Element type was not specified so determine it by merging the class
          // types of the item expressions.
          item_type.merge_class(*args.m_expr_type);
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Eat {whitespace}
      args.m_result = parse_ws_any(pos, &pos);

      if (!args.is_ok())
        {
        // Error in whitespace so exit while loop
        break;
        }

      ch = cstr_a[pos];

      // [CReis Using multiple "if" statements here rather than "switch" so "break" can be
      // used to exit "while" loop.]
      if (ch == '}')
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Found end of list literal
        pos++;

        // Exit while loop
        break;
        }
      else
        {
        if (ch == ',')
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Another expression is to follow
                  
          // Eat {whitespace}
          args.m_result = parse_ws_any(pos + 1u, &pos);

          if (!args.is_ok())
            {
            // Error in whitespace so exit while loop
            break;
            }

          // Next possible error
          args.m_result = Result_err_unexpected_eof;

          // Loop back to beginning of while.
          }
        }
      }  // while end
    }

  // Set result class type
  if (m_flags.is_set_any(Flag_type_check) && args.is_ok())
    {
    if (!item_type_b)
      {
      if (item_type_p == &item_type)
        {
        item_type_p = item_type.is_trivial()
          ? static_cast<SkClassDescBase *>(item_type.get_common_class())
          : SkClassUnion::get_or_create(item_type);
        }

      list_class_p = SkTypedClass::get_or_create(
        list_class_p ? list_class_p->get_key_class() : SkBrain::ms_list_class_p,
        item_type_p);
      }

    args.m_expr_type = list_class_p;
    }

  if (!args.is_ok())
    {
    delete ctor_p;
    delete list_p;
    list_p = nullptr;
    }

  args.m_end_pos = pos;

  return list_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a real or an integer literal number.
//
// #Notes
//   number-literal  = integer-literal | real-literal
//   integer-literal = ['-'] digits-lead ['r' big-digit {['_'] big-digit}]
//   real-literal    = ['-'] digits-lead V ('.' digits-tail) [real-exponent]
//   real-exponent   = 'E' | 'e' ['-'] digits-lead
//   digits-lead     = '0' | (non-0-digit {['_'] digit})
//   digits-tail     = digit {['_'] digit}
//   digit           = '0' | non-0-digit
//   non-0-digit     = '1' | ... | '9'
//   big-digit       = digit | alphabetic
//
// #See Also  parse_literal_real(), parse_literal_integer()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkLiteral data-structure otherwise nullptr is returned.
  SkLiteral *
SkParser::parse_literal_number(
  // see SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_size_radix_small, Result_err_size_radix_large,
  //   Result_err_expected_literal_real_end or Result_err_unexpected_eof.
  Args & args, // = ms_def_args.reset()
  // Address to store whether number is 'simple' integer with no radix specified (true) or
  // it is an integer with a radix specified or a real - ignored if nullptr or invalid parse.
  bool * simple_int_p // = nullptr
  ) const
  {
  uint32_t    pos;
  tSkReal     real_num  = tSkReal(0);
  uint32_t    start_pos = args.m_start_pos;
  SkLiteral * num_p     = nullptr;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Try to parse a real number
  args.m_result = parse_literal_real(
    start_pos,
    &pos,
    args.is_struct_wanted() ? &real_num : nullptr,
    false);  // Don't allow Integer as a Real - checked in parse_expression() after any calls are strung on

  if (args.is_ok())
    {
    if (args.is_struct_wanted())
      {
      num_p = SK_NEW(SkLiteral)(real_num);
      SKDEBUG_SET_CHAR_POS(num_p, start_pos);
      args.m_expr_type = SkBrain::ms_real_class_p;
      }

    if (simple_int_p)
      {
      *simple_int_p = false;
      }

    args.m_end_pos = pos;
    return num_p;
    }

  if (args.m_result != Result_err_expected_literal_real_end)
    {
    args.m_end_pos = pos;
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Try to parse an integer
  tSkInteger  int_num = 0;
  uint32_t    radix   = 0u;

  args.m_result = parse_literal_integer(
    start_pos, &args.m_end_pos, args.is_struct_wanted() ? &int_num : nullptr, &radix);

  if (!args.is_ok())
    {
    return nullptr;
    }

  if (args.is_struct_wanted())
    {
    num_p = SK_NEW(SkLiteral)(int_num);
    SKDEBUG_SET_CHAR_POS(num_p, start_pos);
    args.m_expr_type = SkBrain::ms_integer_class_p;
    }

  if (simple_int_p)
    {
    *simple_int_p = (radix == 0u);
    }

  return num_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a real number.
// 
// Forms:
//   42.0 0.123 .123 -.8  decimal notation (single 0 prior to decimal . is optional)
//   42 -123              simple integer (if int_as_real_b is true)
//   4.2e1 42e0 -1.23e-2  E notation (does not need to be normalized)
//
// Part Names:
//   _____________significand_____________
//   __integer-part___   _fractional-part_ E-Notation-part
//   ['-'] digits-lead V ('.' digits-tail) [real-exponent]
//   
//   * The integer or fractional part (or both) must be present - they cannot both be
//     omitted. The integer part can be the only part present if integer_as_real is true
//     otherwise one or both of the fractional or E notation parts must also be present.
// 
// #Examples
//   if (parse.parse_literal_real(11u, &end_pos, &real_num) == Result_ok)
// 
// #Notes
//   real-literal    = ['-'] digits-lead V ('.' digits-tail) [real-exponent]
//   real-exponent   = 'E' | 'e' ['-'] digits-lead
//   digits-lead     = '0' | (non-0-digit {['_'] digit})
//   digits-tail     = digit {['_'] digit}
//   digit           = '0' | non-0-digit
//   non-0-digit     = '1' | ... | '9'
//   
// #Author(s)  Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_literal_real, Result_err_expected_seperator_digit,
  // Result_err_expected_exponent, Result_err_expected_literal_real_sgnf or
  // Result_err_expected_literal_real_end
  SkParser::eResult
SkParser::parse_literal_real(
  // character index to begin lexical analysis
  uint32_t start_pos, // = 0u
  // character index that lexical analysis stopped at.  If nullptr, it is ignored.
  uint32_t * end_pos_p, // = nullptr
  // Address to store parsed real number
  tSkReal * real_p, // = nullptr
  // If true, a simple integer (without a fractional or exponent part) will be accepted as
  // a complete real number. Used when it is known that a real is desired over an integer.
  // If false, a fractional or exponent part must be present.
  bool int_as_real_b // = true
  ) const
  {
  ASetOnReturn<uint32_t> set_end(&start_pos, end_pos_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure there is at least 1 character to parse
  if (m_str_ref_p->m_length <= start_pos)
    {
    return Result_err_expected_literal_real;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional negation
  bool   negative = false;
  char * cstr_a   = m_str_ref_p->m_cstr_p;
  char   ch       = cstr_a[start_pos];

  if (ch == '-')
    {
    negative = true;
    start_pos++;
    ch = cstr_a[start_pos];
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for integer part
  // '0' | (non-0-digit {['_'] digit})
  eResult result     = Result_ok;
  int32_t int_part   = 0;
  bool    int_part_b = false;

  if (ms_is_digit[uint8_t(ch)])
    {
    int_part_b = true;

    result = parse_digits_lead(start_pos, &start_pos, &int_part);

    if (result != Result_ok)
      {
      return result;
      }

    ch = cstr_a[start_pos];
    }

  // $Note - Rather than doing it here, a check for unexpected C++ style integer literals
  // like 0x42, 042 or 0b101 is done in parse_literal_integer().


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for fractional part
  // '.' digit {['_'] digit}
  bool fractional_part_b = false;
  f64  frac_part         = 0.0;  // Doubles are used for greater accuracy during parsing

  // Optionally include fractional part if decimal . found
  if ((ch == '.') && ms_is_digit[uint8_t(cstr_a[start_pos + 1u])])
    {
    fractional_part_b = true;
    start_pos++;
    ch = cstr_a[start_pos];

    f64  place       = 1.0;
    bool separator_b = false;

    do
      {
      place     *= 0.1;
      frac_part += place * (ch - '0');
      start_pos++;
      ch = cstr_a[start_pos];

      separator_b = false;

      // Skip any separator that is between digits
      if (ch == '_')
        {
        // Encountered a separator
        separator_b = true;
        start_pos++;
        ch = cstr_a[start_pos];
        }
      }
    while (ms_is_digit[uint8_t(ch)]);

    // Ensure no trailing separator - can only be between digits
    if (separator_b)
      {
      return Result_err_expected_seperator_digit;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for exponent part
  // 'E' | 'e' ['-'] digits-lead
  int32_t exponent_part   = 0;
  bool    exponent_part_b = false;
  bool    exponent_neg_b  = false;

  if ((ch == 'E') || (ch == 'e'))
    {
    exponent_part_b = true;
    start_pos++;

    // Look for optional negation

    if (cstr_a[start_pos] == '-')
      {
      exponent_neg_b = true;
      start_pos++;
      }

    if (!ms_is_digit[uint8_t(cstr_a[start_pos])])
      {
      return Result_err_expected_exponent;
      }

    result = parse_digits_lead(start_pos, &start_pos, &exponent_part);

    if (result != Result_ok)
      {
      return result;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure required parts are present
  if ((!fractional_part_b && !int_part_b)
    || (!int_as_real_b && !fractional_part_b && !exponent_part_b))
    {
    return int_part_b
      ? Result_err_expected_literal_real_end
      : Result_err_expected_literal_real_sgnf;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return real value if one is requested
  if (real_p)
    {
    f64 real = negative ? (f64(-int_part) - frac_part) : (f64(int_part) + frac_part);

    if (exponent_part_b)
      {
      real *= pow(10.0, exponent_neg_b ? -exponent_part : exponent_part);
      }

    *real_p = tSkReal(real);
    }

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a simple string literal.
//  
// Returns:     Result_ok, Result_err_expected_literal_string, Result_err_expected_char, or
//              Result_err_expected_char_number, Result_err_unexpected_eof 
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Arg          sym_p - pointer to location to store parsed string - any existing string
//              is appended to.  It is not written to if it is set to nullptr or if the
//              result is not Result_ok.  (Default nullptr)
// Examples:    if (parse.parse_simple_string(11u, &end_pos, &str) == Result_ok)
// Notes:       simple-string   = '"' {character} '"'
//              character       = escape-sequence | printable-char
//              escape-sequence = '\' (integer-literal | printable-char)
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_literal_simple_string(
  uint32_t  start_pos, // = 0u
  uint32_t *    end_pos_p, // = nullptr
  AString * str_p      // = nullptr
  ) const
  {
  eResult result = Result_err_unexpected_eof;  // Next possible error
  uint32_t    pos    = start_pos;
  uint32_t    length = m_str_ref_p->m_length;

  if ((length - start_pos) >= 2u)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    // Next possible error
    result = Result_err_expected_literal_string;

    // Must start with double quote "
    if (cstr_a[pos] == '"')
      {
      char   ch;
      uint32_t   end_pos;
      uint32_t   prev_length = 0u;
      char * str_a       = nullptr;

      pos++;


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine approximate length of string
      // Set pos to the last end quote - skipping over any quote character escape sequences

      while (pos < length)
        {
        ch = cstr_a[pos];

        if (ch == '"')
          {
          break;
          }
        else
          {
          if (ch == '\\')
            {
            pos++;
            }

          pos++;
          }
        }

      end_pos = pos;


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Create parsed string looking for character escape sequences

      if (str_p)
        {
        prev_length = str_p->get_length();
        // This increases the character buffer size to make it large enough to append the
        // literal.  The increase may be a bit more than is needed since it does not
        // take into account character escape sequences.
        str_p->ensure_size(prev_length + (end_pos - (start_pos + 1u)));
        str_a = str_p->as_cstr_writable();
        }

      result = Result_ok;
      pos    = start_pos + 1u;
      length = 0u;

      while ((pos < end_pos) && (result == Result_ok))
        {
        ch = cstr_a[pos];

        if (ch == '\\')
          {
          result = parse_literal_char_esc_seq(pos, &pos, &ch);
          }
        else
          {
          pos++;
          }

        if (str_p)
          {
          str_a[prev_length + length] = ch;
          }

        length++;
        }  // while

      if (result == Result_ok)
        {
        pos          = end_pos + 1u;
        prev_length += length;
        }

      if (str_p)
        {
        str_p->set_length(prev_length);
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Parses starting at 'start_pos' attempting to create a string literal.
// Returns:     Result_ok, Result_err_expected_literal_string, Result_err_expected_char, or
//              Result_err_expected_char_number, Result_err_unexpected_eof 
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Arg          str_p - pointer to location to store parsed string.  It is not written to
//              if it is set to nullptr or if the result is not Result_ok.  (Default nullptr)
// Examples:    if (parse.parse_string(11u, &end_pos, &str) == Result_ok)
// Notes:       string-literal  = simple-string {ws '+' ws simple-string}
//              simple-string   = '"' {character} '"'
//              character       = escape-sequence | printable-char
//              escape-sequence = '\' (integer-literal | printable-char)
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_literal_string(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  AString *  str_p      // = nullptr
  ) const
  {
  eResult result = parse_literal_simple_string(start_pos, &start_pos, str_p);

  if (result == Result_ok)
    {
    // Parse {ws '+' ws simple-string}

    uint32_t   pos;
    uint32_t   end_pos;
    uint32_t   length = m_str_ref_p->m_length;
    char * cstr_a = m_str_ref_p->m_cstr_p;

    while (start_pos < length)
      {
      // Eat {whitespace}
      result = parse_ws_any(start_pos, &pos);

      // Check for good whitespace parse, determine if end of parse string and ensure
      // there is a plus '+' operator
      if ((result != Result_ok) || (pos >= length) || (cstr_a[pos] != '+'))
        {
        // Un-good - exit while
        break;
        }

      // Increment past plus '+' operator
      pos++;

      // Eat {whitespace}
      result = parse_ws_any(pos, &pos);

      // Check for good whitespace parse and determine if end of parse string
      if ((result != Result_ok) || (pos >= length))
        {
        // Un-good - exit while
        break;
        }

      result = parse_literal_simple_string(pos, &end_pos, str_p);

      if (result != Result_ok)
        {
        // Ignore read ahead if string parse did not progress the parse
        if (end_pos == pos)
          {
          result = Result_ok;
          }

        // Found invalid string, so exit while
        break;
        }

      start_pos = end_pos;
      }  // while
    }

  if (end_pos_p)
    {
    *end_pos_p = start_pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Parses starting at 'start_pos' attempting to create a symbol literal.
// Returns:     Result_ok, Result_err_expected_literal_symbol,
//              Result_err_expected_literal_symbol_end, Result_err_size_symbol,
//              Result_err_expected_char, Result_err_expected_char_number, or Result_err_unexpected_eof 
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Arg          sym_p - pointer to location to store parsed symbol.  It is not written to
//              if it is set to nullptr or if the result is not Result_ok.  (Default nullptr)
// Examples:    if (parse.parse_symbol(11u, &end_pos, &sym) == Result_ok)
// Notes:       symbol-literal  = ''' {character} '''
//              character       = escape-sequence | printable-char
//              escape-sequence = '\' (integer-literal | printable-char)
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_literal_symbol(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  ASymbol *  sym_p      // = nullptr
  ) const
  {
  // Next possible error - EOF not used since this gives a bit more info
  eResult  result = Result_err_expected_literal_symbol;
  uint32_t pos    = start_pos;
  uint32_t length = m_str_ref_p->m_length;

  if ((length - start_pos) >= 2u)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    // Must start with single quote '
    if (cstr_a[pos] == '\'')
      {
      pos++;

      uint32_t end_pos;
      bool found_end;

      // Set pos to the last end quote - skipping over any quote character escape sequences
      do
        {
        end_pos   = length;
        found_end = find('\'', 1u, &end_pos, pos);
        pos       = end_pos;
        } while ((pos != length) && (cstr_a[pos - 1] == '\\'));

      // Create parsed symbol looking for character escape sequences
      char ch;
      char buffer_a[SkParser_ident_length_max + 1];

      result = found_end ? Result_ok : Result_err_expected_literal_symbol_end;
      pos    = start_pos + 1u;
      length = 0;

      while ((pos < end_pos) && (result == Result_ok))
        {
        ch = cstr_a[pos];

        if (ch == '\\')
          {
          result = parse_literal_char_esc_seq(pos, &pos, &ch);
          }
        else
          {
          pos++;
          }

        buffer_a[length] = ch;
        length++;

        if (length > SkParser_ident_length_max)
          {
          result = Result_err_size_symbol;
          }
        }  // while

      if (result == Result_ok)
        {
        pos = end_pos + 1u;

        if (sym_p)
          {
          buffer_a[length] = '\0';

          *sym_p = ASymbol::create(buffer_a, length, ATerm_short);
          }
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Determines if a loop with specified `name` exists or if there are *any* loops if
// `name` is null.
bool SkParser::find_nested(const ASymbol & name) const
  {
  // $Vital - CReis Ensure that closures do not use loops in surrounding context.
  if (m_nest_stack.is_empty())
    {
    return false;
    }

  if (name.is_null())
    {
    return true;
    }

  const NestInfo * sentinel_p  = m_nest_stack.get_sentinel();
  const NestInfo * nest_info_p = sentinel_p->get_next();

  do
    {
    if (name == *nest_info_p)
      {
      return true;
      }

    nest_info_p = nest_info_p->get_next();
    }
  while (nest_info_p != sentinel_p);

  return false;
  }

//---------------------------------------------------------------------------------------
// Create a method call from a method
SkMethodCallBase * SkParser::create_method_call(SkMethodBase * method_p, bool is_class_method, SkClassDescBase * receiver_type_p, SkClass * qual_scope_p)
  {
  bool is_invoked_on_class = false;
  if (receiver_type_p)
    {
    receiver_type_p = receiver_type_p->qualify(qual_scope_p);
    is_invoked_on_class = (receiver_type_p == SkBrain::ms_class_class_p || receiver_type_p->is_metaclass());

    SkClass * receiver_key_class_p = receiver_type_p->get_key_class();

    // Check for boolean "short-circuit" evaluation
    if (receiver_key_class_p == SkBrain::ms_boolean_class_p)
      {
      uint32_t name_id = method_p->get_name_id();
      switch (name_id)
        {
        case ASymbolId_and:  return SK_NEW(SkMethodCallBooleanAnd)(method_p, qual_scope_p);
        case ASymbolId_or:   return SK_NEW(SkMethodCallBooleanOr)(method_p, qual_scope_p);
        case ASymbolId_nand: return SK_NEW(SkMethodCallBooleanNand)(method_p, qual_scope_p);
        case ASymbolId_nor:  return SK_NEW(SkMethodCallBooleanNor)(method_p, qual_scope_p);
        default: break;
        }
      }

    // Check for special assert methods
    if (receiver_key_class_p == SkBrain::ms_debug_class_p)
      {
      uint32_t name_id = method_p->get_name_id();
      switch (name_id)
        {
        case ASymbolId_assert:          return SK_NEW(SkMethodCallAssert)(method_p, qual_scope_p);
        case ASymbolId_assert_no_leak:  return SK_NEW(SkMethodCallAssertNoLeak)(method_p, qual_scope_p);
        default: break;
        }
      }
    }

  // Regular method call evaluation
  return is_invoked_on_class
    ? (is_class_method
      ? static_cast<SkMethodCallBase *>(SK_NEW(SkMethodCallOnClass)(method_p, qual_scope_p))
      : static_cast<SkMethodCallBase *>(SK_NEW(SkMethodCallOnClassInstance)(method_p, qual_scope_p)))
    : (is_class_method 
      ? static_cast<SkMethodCallBase *>(SK_NEW(SkMethodCallOnInstanceClass)(method_p, qual_scope_p))
      : static_cast<SkMethodCallBase *>(SK_NEW(SkMethodCallOnInstance)(method_p, qual_scope_p)));
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a loop exit.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkLoopExit data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_expected_loop_exit,
//                 Result_err_expected_instance, or Result_err_unexpected_eof.
// Notes:      loop-exit     = 'exit' [ws instance-name]
//             instance-name = lowercase {alphanumeric}
// Author(s):   Conan Reis
SkLoopExit * SkParser::parse_loop_exit(
  Args & args // = ms_def_args.reset()
  ) const
  {
  uint32_t start_pos = args.m_start_pos;
  char *   cstr_a    = m_str_ref_p->m_cstr_p;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if text starts with 'exit' and that it is not some other identifier that
  // has a root of 'exit' - i.e. ensure that the next character is not A-Z, a-z, _, 0-9,
  // or a European character.
  if (((start_pos + 4u) >= m_str_ref_p->m_length)
    || (cstr_a[start_pos]      != 'e')
    || (cstr_a[start_pos + 1u] != 'x')
    || (cstr_a[start_pos + 2u] != 'i')
    || (cstr_a[start_pos + 3u] != 't')
    || ms_char_match_table[ACharMatch_identifier][uint8_t(cstr_a[start_pos + 4u])])
    {
    args.m_result  = Result_err_expected_loop_exit;
    args.m_end_pos = start_pos;

    return nullptr;
    }

  uint32_t pos = start_pos + 4u;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional loop name
  uint32_t id_pos = pos;

  // Eat {whitespace}
  args.m_result = parse_ws_any(pos, &id_pos);

  if (!args.is_ok())
    {
    args.m_end_pos = id_pos;

    return nullptr;
    }

  ASymbol  loop_name;
  uint32_t id_end;

  args.m_result = parse_name_instance(id_pos, &id_end, &loop_name);

  if (!args.is_ok())
    {
    if (id_end != id_pos)
      {
      args.m_end_pos = id_end;

      return nullptr;
      }

    // Wasn't an instance so ignore
    args.m_result = Result_ok;
    }
  else
    {
    pos = id_end;
    }

  args.m_end_pos = pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that the loop [and optional name] exists.
  if (!find_nested(loop_name))
    {
    args.m_result  = Result_err_unexpected_exit_no_loop;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      if (loop_name.is_null())
        {
        ms_error_str.append("Not in the scope of a loop.");
        }
      else
        {
        ms_error_str.append_format("Not in the scope of a loop with the name `%s`.", loop_name.as_cstr_dbg());
        }
    #endif

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Valid loop
  
  // $Note - CReis Since a loop exit can cause the flow control to jump, the
  // type-checking may be a little off after a loop exit.

  // Loop exit statements result in a "nil" object
  args.m_expr_type  = SkNone::get_class();

  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  SkLoopExit * exit_p = SK_NEW(SkLoopExit)(loop_name);
  SKDEBUG_SET_CHAR_POS(exit_p, start_pos);

  return exit_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a loop expression.
// 
// Returns:
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically
//   allocated `SkLoopExit` data-structure otherwise `nullptr` is returned.
//   
// Params:  
//   args:
//     see SkParser::Args, specific considerations below:
//       m_result:
//         Result_ok, Result_err_expected_loop_block, or any other warning or error.
//         
// Notes:
//   ```
//   loop          = 'loop' loop-tail
//   loop-tail     = ws [instance-name ws] code-block
//   instance-name = lowercase {alphanumeric}
//   code-block    = '[' ws [statement {wsr statement} ws] ']'
//   ```
//
// See:       parse_loop_exit()
// Modifiers: protected
// Author(s): Conan Reis
SkLoop * SkParser::parse_loop_tail(Args & args) const
  {
  uint32_t pos = args.m_start_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat whitespace
  args.m_result = parse_ws_any(pos, &pos);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;

    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for optional loop name
  ASymbol loop_name;
  char *   cstr_a = m_str_ref_p->m_cstr_p;
  char     ch     = (pos < m_str_ref_p->m_length) ? cstr_a[pos] : '\0';

  if (ch != '[')
    {
    uint32_t name_pos;

    args.m_result = parse_name_instance(pos, &name_pos, args.is_struct_wanted() ? &loop_name : nullptr);

    if (!args.is_ok())
      {
      args.m_end_pos = name_pos;

      if (name_pos == pos)
        {
        args.m_result = Result_err_expected_loop_block;
        }

      return nullptr;
      }

    // Ensure loop name is unique
    if (find_nested(loop_name))
      {
      args.m_result    = Result_err_context_duped_loop_name;
      args.m_start_pos = pos;
      args.m_end_pos   = name_pos;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Loop with the name `%s` already present in the current scope.", loop_name.as_cstr_dbg());
      #endif

      return nullptr;
      }

    // Eat {whitespace}
    args.m_result = parse_ws_any(name_pos, &pos);

    if (!args.is_ok())
      {
      args.m_end_pos = pos;

      return nullptr;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for clause block
  ch = (pos < m_str_ref_p->m_length) ? cstr_a[pos] : '\0';

  if (ch != '[')
    {
    args.m_result  = Result_err_expected_loop_block;
    args.m_end_pos = pos;

    return nullptr;
    }

  // Add loop to nesting stack
  NestInfo loop_info(loop_name);
  m_nest_stack.append(&loop_info);

  SkLoop * loop_p = nullptr;

  args.m_start_pos = pos;

  // $Vital - CReis Should use specific SkInvokeTime_* here not just *_any
  SkExpressionBase * expr_p = parse_code_block_optimized(
    args, SkInvokeTime_any, ResultDesired_false);

  // Remove loop from nesting stack
  loop_info.remove();

  // Loops always return "nil"
  args.m_expr_type = SkNone::get_class();

  if (expr_p)
    {
    loop_p = SK_NEW(SkLoop)(expr_p, loop_name);
    // Debug character position is set in parse_expression_alpha()
    }

  return loop_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a method with the supplied name and optionally
//             appending it to the supplied class.  It is an "atomic" method (i.e. it is
//             a forward declaration for a C++ method) if the optional code block is
//             absent.  It is a "custom" method if the optional code block is present.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkMethodBase data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_typecheck_return_type,
//                 or pretty much any other warning or error.
// Arg         name - name to identify method (usually taken from the file name).
// Arg         append_to_class_b - indicates if the method should be appended to the
//             current class scope.
// See:        parse_invoke_method()
// Notes:      method         = parameters [ws code-block]
//             parameters     = parameter-list [ws class-desc]
//             parameter-list = '(' ws [send-params ws] [';' ws return-params ws] ')'
//             send-params    = parameter {ws ',' ws parameter}
//             return-params  = param-specifier {ws ',' ws param-specifier}
//             code-block     = '[' ws [statement {wsr statement} ws] ']'
//
//             parse_method_source() ensures that the method has a unique name - i.e. a
//             method with the same name has not already been added to the class - this
//             method however does not do such a check.
// Author(s):   Conan Reis
SkMethodBase * SkParser::parse_method(
  Args &          args,              // = ms_def_args.reset()
  const ASymbol & name,              // = ASymbol::ms_null
  eSkInvokeTime   desired_exec_time, // = SkInvokeTime_any
  bool            append_to_class_b  // = true
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse parameters
  uint32_t       pos         = args.m_start_pos;
  SkMethodBase * method_p    = nullptr;
  SkClass *      key_scope_p = m_context.m_obj_scope_p->get_key_class();
  SkParameters   params;

  m_member_type = SkMember_method;

  #if (SKOOKUM & SK_DEBUG)
    // Get old method to replace if this is a reparse
    if (SkClass::ms_reparse_info.m_is_active)
      {
      SkQualifier ident(name, key_scope_p);

      method_p = m_context.m_obj_scope_p->is_metaclass()
        ? SkClass::ms_reparse_info.m_class_methods.pop(ident)
        : SkClass::ms_reparse_info.m_methods.pop(ident);
      }
  #endif

  // Parse annotations
  Annotations annotations;
  args.m_result = parse_annotations(pos, &pos, &annotations, SkAnnotationTarget_invokable);

  // Parse parameters
  if (args.is_ok())
    {
    bool predicate_method_b = false;

    #if defined(A_SYMBOL_STR_DB)
      predicate_method_b = name.as_string().get_last() == '?';
    #endif

    m_context.m_params_p = args.is_struct_wanted() ? &params : nullptr;
    args.m_start_pos = pos;

    parse_parameters(
      args,
      m_context.m_params_p,
      predicate_method_b ? (ParamFlag__default | ParamFlag_result_bool) : ParamFlag__default,
      annotations.m_flags);

    pos = args.m_end_pos;

    if (args.m_result == Result_ok)
      {
      #if defined(A_SYMBOL_STR_DB)
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Is it a conversion method?
        // $Revisit - CReis These checks should probably be moved to somewhere like parameters_typecheck()
        SkClassDescBase * result_class_p = params.get_result_class();

        if (!name.is_null() && ms_is_uppercase[uint8_t(name.as_cstr_dbg()[0])])
          {
          // Ensure that conversion methods have parameters that returns a type compatible
          // with the class specified by the method name.
          if (!m_context.finalize_generic(*result_class_p)->is_class_type(SkBrain::get_class(name)))
            {
            args.m_result = Result_err_typecheck_conversion;
            }
          else
            {
            // Ensure that conversion methods have no parameters
            if (params.m_params.get_length())
              {
              args.m_result = Result_err_context_conversion_params;

              // $Revisit - CReis [Type-check] In the future, conversion methods could have
              // parameters if they all have defaults.  SkConversion::invoke() is already
              // set up to place the defaults.
              }
            }
          }
      #endif  // A_SYMBOL_STR_DB

      // $Revisit - CReis [Type-check] Should constructors disallow a return type?

      // $Vital - CReis [Type-check] An overriding method should be tested to ensure that it has
      // the same parameters and defaults as the method that it overrides.

      if (args.is_ok())
        {
        // Eat {whitespace}
        args.m_result = parse_ws_any(pos, &pos);

        if (args.is_ok())
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Check for optional code block - Is it a custom method?

          SkExpressionBase * expr_p = nullptr;

          bool make_struct = args.is_struct_wanted();
          bool atomic_code = m_str_ref_p->m_cstr_p[pos] != '[';

          if (!atomic_code)
            {
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Parse custom code block
            args.m_start_pos = pos;

            expr_p = parse_code_block_optimized(args, desired_exec_time);

            pos = args.m_end_pos;

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Ensure returned values from code block are compatible with parameters
            make_struct = args.is_ok() && parameters_typecheck(args, &params) && expr_p;
            }

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Make/fix-up method structure
          if (make_struct)
            {
            SkParameters * params_p = SkParameters::get_or_create(&params);

            if (method_p)
              {
              eSkInvokable type = method_p->get_invoke_type();

              if ((atomic_code && (type != SkInvokable_method))
                || (!atomic_code && (type == SkInvokable_method)))
                {
                // Reuse old method prior to reparse to keep bindings, etc.
                method_p->set_params(params_p);
                method_p->set_annotation_flags(annotations.m_flags);
                method_p->set_invoked_data_array_size(m_context.m_current_scope_p->m_data_idx_count_max);

                if (!atomic_code)
                  {
                  static_cast<SkMethod *>(method_p)->set_expression(expr_p);
                  }
                }
              else
                {
                // Different method type
                delete method_p;
                method_p = nullptr;
                }
              }

            if (method_p == nullptr)
              {
              if (atomic_code)
                {
                // $Note - CReis Using a SkMethodMthd for the C++ method rather than a
                // SkMethodFunc since it is either larger or the same size as SkMethodFunc
                // which can be swapped in-place during the binding of the C++ code as needed.
                method_p = SK_NEW(SkMethodMthd)(name, key_scope_p, params_p, annotations.m_flags);
                }
              else
                {
                method_p = SK_NEW(SkMethod)(name, key_scope_p, params_p, m_context.m_current_scope_p->m_data_idx_count_max, annotations.m_flags, expr_p);
                }
              }

            method_p->set_akas((tSkAkas &&)annotations.m_akas);
            }
          else
            {
            if (expr_p)
              {
              delete expr_p;
              }
            }
          }
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append to class as requested
  if (append_to_class_b && method_p && !name.is_null())
    {
    m_context.m_obj_scope_p->append_method(method_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove parameters and modified data members from variable list
  if (m_flags.is_set_any(Flag_type_check))
    {
    m_context.free_all_locals();
    }

  args.m_end_pos = pos;

  m_member_type = SkMember__invalid;

  return method_p;
  }


//---------------------------------------------------------------------------------------
// Parses starting at `start_pos` attempting to create a method with the supplied name
// and appending it to the supplied class.  If the optional code block is absent it is a
// C++ method (i.e. it is a forward script declaration for a C++ method).  If the
// optional code block is present it is a custom script method.
// 
// Returns:   Result_ok or pretty much any other warning or error. 
// Params:  
//   name:    name to identify method (usually taken from the file name)
//   scope_p: scope of parse
//   args:    see Args
//   
// Notes:  
//   ```
//   method-file = ws method ws
//   method      = parameters [ws code-block]
//   ```
// See:         preparse_method_source(), parse_method(), parse_invoke_method()
// Author(s):   Conan Reis
SkMethodBase * SkParser::parse_method_source(
  const ASymbol &    name,
  SkClassUnaryBase * scope_p,
  Args &             args,             // = ms_def_args.reset()
  bool               append_to_class_b // = true
  )
  {
  SK_ASSERTX(!args.is_struct_wanted() || m_flags.is_set_any(Flag_type_check), "Type checking must be on if struct generation is desired.");

  reset_scope(scope_p, name);

  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;

  SkMethodBase * method_p = parse_method(args, name, SkInvokeTime_immediate, append_to_class_b);

  if (args.m_result != Result_ok)
    {
    return nullptr;
    }

  // $Revisit - Should delete result if there is an error at the end.
  args.m_start_pos = args.m_end_pos;
  parse_ws_end(args);
 
  return method_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create the tail end of a statement/expression modifier.
//
// #Notes
//   Already parsed before this method is called:
//     expression ws
//     
//   when   = expression ws 'when' ws expression
//   unless = expression ws 'unless' ws expression
//
//
//   Example:
//
//     clause when bool_test
//
//     clause unless bool_test
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If parse valid and args.is_struct_wanted() == true it returns a dynamically allocated
  // SkMethodCallBase data-structure otherwise nullptr is returned.
  SkExpressionBase *
SkParser::parse_modifier_tail(
  // See SkParser::Args, specific considerations below:
  //   m_type_p: set with class type of receiver on entry and changed to result type on exit.
  //   m_result: Result_ok or pretty much any other warning or error.
  Args & args,
  SkExpressionBase * expr_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up quick access chars.
  // - This mechanism ensures that no read will occur beyond the end of the string/file.
  uint32_t     pos    = args.m_start_pos;
  uint32_t     length = m_str_ref_p->m_length;
  const char * cstr_a = m_str_ref_p->m_cstr_p;

  char ch1 = cstr_a[pos];    
  char ch2 = '\0';
  char ch3 = '\0';
  char ch4 = '\0';
  char ch5 = '\0';
  char ch6 = '\0';
  char ch7 = '\0';

  if ((pos + 1u) < length)
    {
    ch2 = cstr_a[pos + 1u];

    if ((pos + 2u) < length)
      {
      ch3 = cstr_a[pos + 2u];

      if ((pos + 3u) < length)
        {
        ch4 = cstr_a[pos + 3u];

        if ((pos + 4u) < length)
          {
          ch5 = cstr_a[pos + 4u];

          if ((pos + 5u) < length)
            {
            ch6 = cstr_a[pos + 5u];

            if ((pos + 6u) < length)
              {
              ch7 = cstr_a[pos + 6u];
              }
            }
          }
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse modifier name
  uint32_t modifier_type = ASymbolId_null;

  args.m_result  = Result_ok;
  args.m_end_pos = pos;

  switch (ch1)
    {
    case 'w':
      // Check for "when"
      if ((ch2 ==  'h') && (ch3 == 'e') && (ch4 == 'n')
        && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch5)])
        {
        args.m_end_pos = pos + 5u;
        modifier_type = ASymbolId_when;
        break;
        }

      args.m_result = Result_err_expected_statement_modifier;
      return nullptr;

    case 'u':
      // Check for "unless"
      if ((ch2 ==  'n') && (ch3 == 'l') && (ch4 == 'e') && (ch5 == 's') && (ch6 == 's')
        && ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch7)])
        {
        args.m_end_pos = pos + 6u;
        modifier_type = ASymbolId_unless;
        break;
        }

      args.m_result = Result_err_expected_statement_modifier;
      return nullptr;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    default:  // Invalid modifier
      args.m_result = Result_err_expected_statement_modifier;
      return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse test expression
  // Remember expr_p type.
  SkClassDescBase * expr_type_p = args.m_expr_type;

  // Eat {whitespace}
  args.m_start_pos = args.m_end_pos;
  args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

  if (!args.is_ok())
    {
    return nullptr;
    }

  // Parse expression
  args.m_start_pos = args.m_end_pos;
  args.m_desired_type_p = SkBrain::ms_boolean_class_p;
  SkExpressionBase * test_expr_p = parse_expression(args, SkInvokeTime_immediate);

  if (!args.is_ok())
    {
    return nullptr;
    }

  // Ensure that the result type of the clause expression is a Boolean.
  if (m_flags.is_set_any(Flag_type_check) && !args.m_expr_type->is_class_type(SkBrain::ms_boolean_class_p))
    {
    args.m_result = Result_err_typecheck_test;
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Store results

  if (m_flags.is_set_any(Flag_type_check))
    {
    // $Revisit - CReis Can't do alternate context paths for types for the clause expression
    // without doing some sort of multiple passes since the clause expression is parsed
    // before the statement modifier is encountered.

    // Store initial clause type
    SkClassUnion result_type(*expr_type_p);

    // Might not actually run clause so merge result type with None
    result_type.merge_class(*SkNone::get_class());

    // Store result type
    args.m_expr_type = result_type.is_trivial()
      ? static_cast<SkClassDescBase *>(result_type.get_common_class())
      : SkClassUnion::get_or_create(result_type);
    }

  if (!args.is_struct_wanted())
    {
    return nullptr;
    }

  return (modifier_type == ASymbolId_when)
    ? SK_NEW(SkWhen)(test_expr_p, expr_p)
    : SK_NEW(SkUnless)(test_expr_p, expr_p);
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a class name.
// 
// #Examples
//   if (parse.parse_name_class(11u, &end_pos, &class_name) == Result_ok)
//   
// #See Also
//   parse_class(), parse_name_instance(), parse_name_coroutine()
//   
// #Notes
//   class-name   = uppercase {alphanumeric}
//   alphanumeric = uppercase | lowercase | digit | '_'
//   
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_class, Result_err_unexpected_class_class or
  // Result_err_context_non_class
  SkParser::eResult
SkParser::parse_name_class(
  // character position to begin lexical analysis
  uint32_t start_pos, // = 0u
  // character position that lexical analysis stopped at. Not written to if set to nullptr.
  uint32_t * end_pos_p, // = nullptr
  // pointer to location to store parsed instance name. Not written to if set to nullptr or
  // Result_err_context_non_class
  // if result is not Result_ok.
  ASymbol * name_p, // = nullptr
  // Specifies if and how class should be checked against existing classes - see eClassCheck
  eClassCheck check // = ClassCheck_no_validate_meta
  ) const
  {
  uint32_t pos = start_pos;

  // Ensure end_pos_p is set on return
  ASetOnReturn<uint32_t> set_end(&pos, end_pos_p);

  char * cstr_a = m_str_ref_p->m_cstr_p;

  if ((m_str_ref_p->m_length <= pos)
    || !ms_is_uppercase[uint8_t(cstr_a[pos])])
    {
    return Result_err_expected_class;
    }

  // Starts with uppercase - this is a class name
  pos++;

  if ((check & ClassCheck__validate) == 0u)
    {
    // Don't validate name
    parse_name_symbol(pos - 1u, &pos, name_p);

    return ((check & ClassCheck__meta) && name_p && (*name_p == ASymbol_Class))
      ? Result_err_unexpected_class_class
      : Result_ok;
    }

  // Validate name
  ASymbol name;

  parse_name_symbol(pos - 1u, &pos, &name);

  if ((check & ClassCheck__meta) && (name == ASymbol_Class))
    {
    return Result_err_unexpected_class_class;
    }

  if (!SkBrain::is_class_present(name))
    {
    return Result_err_context_non_class;
    }

  if (name_p)
    {
    *name_p = name;
    }

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create an instance name.
// 
// #Notes
//   instance-name = lowercase {alphanumeric}
//   alphanumeric  = uppercase | lowercase | digit | '_'
//   
// #See Also     parse_name_class(), parse_name_coroutine()
// #Author(s)    Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_instance, or Result_err_unexpected_eof 
  SkParser::eResult
SkParser::parse_name_instance(
  // Character index to begin lexical analysis
  uint32_t start_pos, // = 0u
  // Pointer to store index where lexical analysis stopped at.  Not written to if nullptr.
  uint32_t * end_pos_p, // = nullptr
  // Pointer to store parsed instance name.  Not written to if nullptr or result not Result_ok.
  ASymbol * name_p // = nullptr
  ) const
  {
  eResult  result = Result_err_unexpected_eof;  // Next possible error
  uint32_t pos    = start_pos;

  if (m_str_ref_p->m_length > pos)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    // Next possible error
    result = Result_err_expected_instance;

    if (ms_is_lowercase[uint8_t(cstr_a[pos])])  // Starts with lowercase - this is a instance name
      {
      ASymbol name;

      pos++;
      parse_name_symbol(start_pos, &pos, &name);

      // Ensure that identifier is not a reserved word
      if (!is_ident_reserved(name.get_id()))
        {
        result = Result_ok;
        
        if (name_p)
          {
          *name_p = name;
          }
        }
      else
        {
        result = Result_err_unexpected_reserved;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The reserved word '%s' cannot be used as a variable identifier.\n\n"
            "SkookumScript reserved words/tokens include:\n"
            "  primitives - if, case, when, unless, else, loop, sync, race, branch, change, [rush], [fork]\n"
            "  statements - exit, [skip]\n"
            "  identifiers - this, this_class, this_code, nil\n"
            "  literals - true, false",
            name.as_cstr_dbg());
        #endif
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a method name.
// 
// Returns: Result_ok, Result_err_expected_method_name, or Result_err_unexpected_eof 
// 
// Params:
//   start_pos: character position to begin lexical analysis (Default 0u)
//   end_pos_p:
//     Character position that lexical analysis stopped at.  If it is set to nullptr, it
//     is not written to.
//   name_p:
//     Pointer to location to store parsed instance name.  It is not written to if it is
//     set to nullptr or if the result is not Result_ok.
//     
// Examples: if (parse.parse_name_method(11u, &end_pos, &script_name) == Result_ok)
// 
// Notes:
//   ```
//   method-name      = name-predicate | constructor-name | destructor-name | convert-name
//   name-predicate   = instance-name ['?']
//   instance-name    = lowercase {alphanumeric}
//   constructor-name = '!' [instance-name]
//   destructor-name  = '!!
//   convert-name     = class
//   alphanumeric     = uppercase | lowercase | digit | '_'
//   ```
//   
// See:        parse_name_class(), parse_name_instance()
// Author(s):  Conan Reis
SkParser::eResult SkParser::parse_name_method(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  ASymbol *  name_p     // = nullptr
  ) const
  {
  uint32_t    pos    = start_pos;
  eResult result = Result_err_unexpected_eof;  // Next possible error

  if (m_str_ref_p->m_length > start_pos)
    {
    char * cstr_a = m_str_ref_p->m_cstr_p;
  
    // Next possible error
    result = Result_err_expected_method_name;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Is it a constructor or destructor?
    if (cstr_a[pos] == '!')
      {
      pos++;

      // There is bound to be a valid parse past this point
      result = Result_ok;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it a destructor?  '!!'
      if (cstr_a[pos] == '!')
        {
        pos++;

        if (name_p)
          {
          *name_p = ASymbolX_dtor;
          }
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Is it a named constructor?
        if (ms_is_lowercase[uint8_t(cstr_a[pos])])
          {
          pos++;
          parse_name_symbol(start_pos, &pos, name_p);
          }
        else
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // It must be an unnamed constructor?
          if (name_p)
            {
            *name_p = ASymbolX_ctor;
            }
          }
        }
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it a normal method?
      if (ms_is_lowercase[uint8_t(cstr_a[pos])])
        {
        pos++;
        result = parse_name_predicate(start_pos, &pos, name_p);
        }
      else
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Is is a class conversion method?
        if (ms_is_uppercase[uint8_t(cstr_a[pos])])
          {
          result = parse_name_class(start_pos, &pos, name_p, name_p ? ClassCheck_validate_meta : ClassCheck_no_validate_meta);
          }
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Parses starting at 'start_pos' attempting to create a script name.
// Returns:     Result_ok, Result_err_expected_coroutine_name, or Result_err_unexpected_eof 
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Arg          name_p - pointer to location to store parsed instance name.  It is not
//              written to if it is set to nullptr or if the result is not Result_ok.
//              (Default nullptr)
// Examples:    if (parse.parse_name_coroutine(11u, &end_pos, &script_name) == Result_ok)
// See:         parse_name_class(), parse_name_instance()
// Notes:       script-name  = '_' lowercase {alphanumeric}
//              alphanumeric = uppercase | lowercase | digit | '_'
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_name_coroutine(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  ASymbol *  name_p     // = nullptr
  ) const
  {
  uint32_t pos = start_pos;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  ASetOnReturn<uint32_t> set_end(&pos, end_pos_p);

  // Must start with underscore '_' then a lowercase letter
  if (((m_str_ref_p->m_length - pos) < 2u)
    || (cstr_a[pos] != '_')
    || !ms_is_lowercase[uint8_t(cstr_a[++pos])])
    {
    return Result_err_expected_coroutine_name;
    }

  // Parse remainder of name after first 2 chars
  pos++;
  parse_name_symbol(start_pos, &pos, name_p);

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a nil coalescing operator.
// 
// Returns:
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically
//   allocated `SkInvokeSync` data-structure otherwise `nullptr` is returned.
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//       m_type_p:
//         set with class type of receiver on entry and changed to result type on exit.
//       m_result:
//         Result_ok, Result_err_expected_invoke_apply, Result_err_typecheck_invoke_apply_recv,
//         or any expression warning or error.
//   receiver_p:
//     Target for the invoke apply.  If it is `nullptr` then 'this' - i.e. the topmost
//     scope - is inferred.
//
// Notes:
//   nil-coalescing = expression ws '??' ws expression
//
//   * This method starts at a preparsed `??` and just parses the tail whitespace and
//     expression.
SkNilCoalescing * SkParser::parse_nil_coalescing_tail(
  Args &             args,
  SkExpressionBase * receiver_p
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure receiver is class union with nil as a possibility.
  SkClassDescBase * coalesced_type_p = nullptr;

  if (m_flags.is_set_any(Flag_type_check))
    {
    if (receiver_p && !ensure_exec_time(*receiver_p, args, SkInvokeTime_immediate))
      {
      return nullptr;
      }

    SkClassDescBase * recv_type_p = args.m_expr_type;

    if (recv_type_p->get_class_type() == SkClassType_class_union)
      {
      coalesced_type_p = SkClassUnion::get_reduced(
        *static_cast<SkClassUnion *>(recv_type_p),
        *SkNone::get_class());

      if (coalesced_type_p == recv_type_p)
        {
        // Type didn't change
        coalesced_type_p = nullptr;
        }
      }

    if (m_flags.is_set_any(Flag_type_check) && (coalesced_type_p == nullptr))
      {
      args.m_end_pos = args.m_start_pos + 2u;  // Skip past `??`
      args.m_result  = Result_err_typecheck_nil_union;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Expected a class union that includes None (nil) and instead got:\n"
          "  %s\n"
          "[If `nil` is not a possibility, `??` will always choose the first operand and never the alternate operand.]",
          recv_type_p->as_code().as_cstr());
      #endif

      return nullptr;
      }
    }

  // Skip past `??`
  args.m_start_pos += 2u;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse alternate operand and pass on desired type
  args.m_desired_type_p = coalesced_type_p;

  // $Revisit - like a clause in a conditional, the alternate expression should merge any
  // changes to the context rather than just have changes replace prior context.
  SkExpressionBase * alt_expr_p = parse_expression(args, SkInvokeTime_immediate);

  if (args.m_result != Result_ok)
    {
    return nullptr;
    }

  // Ensure alternate operand type is compatible
  if (m_flags.is_set_any(Flag_type_check)
    && !args.m_expr_type->is_class_type(coalesced_type_p))
    {
    args.m_result = Result_err_typecheck_operand;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "Type of alternate expression (2nd operand): %s\n"
        "Does not match test expression (1st operand with `None` removed): %s",
        args.m_expr_type->as_code().as_cstr()
      );
    #endif

    return nullptr;
    }

  // It is a good nil coalescing operator

  // Create object ID
  if (args.is_struct_wanted())
    {
    return SK_NEW(SkNilCoalescing)(receiver_p, alt_expr_p);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a unary parameter or group parameter.
//
// Returns: true if successfully parsed, false if not
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//     m_result:
//       Result_ok, Result_err_context_duped_param_name, Result_err_expected_class,
//       Result_err_unexpected_class_pattern, Result_err_expected_param_name, or
//       Result_err_unexpected_eof
//   param_new_pp:
//     Pointer to an address to store a pointer to a dynamically allocated parameter.
//     It is not written to if it is set to nullptr or if the result is not Result_ok.
//   annotation_flags: Annotations of the associated invokable
//
// Notes:
//   If the result of this parse a value other than Result_ok and '*end_pos_p' is set to
//   a value other than the original given by 'start_pos', it should be assumed that this
//   portion of the parse was identified as a parameter, but that it had errors or
//   warnings.
//
//   parameter = unary-parameter | group-param
//
// See: preparse_parameter()
bool SkParser::parse_parameter(
  Args &             args,
  SkParameterBase ** param_new_pp,
  uint32_t           annotation_flags
  ) const
  {
  bool unary_param_b = m_str_ref_p->m_cstr_p[args.m_start_pos] != '{';

  if (param_new_pp == nullptr)
    {
    // Validation parse only
    if (unary_param_b)
      {
      return parse_parameter_unary(args, nullptr, annotation_flags);
      }

    // Group parameter
    args.m_result = parse_parameter_group(
      args.m_start_pos, &args.m_end_pos, nullptr, annotation_flags);

    return (args.m_result == Result_ok);
    }

  // Data structure creation parse
  if (unary_param_b)
    {
    // Parse for unary parameter
    SkUnaryParam uparam;

    if (!parse_parameter_unary(args, &uparam, annotation_flags))
      {
      return false;
      }

    // Transfer contents of vparam to heap version
    *param_new_pp = SK_NEW(SkUnaryParam)(&uparam);
    }
  else
    {
    // Parse for variable length parameter
    SkGroupParam vparam;

    args.m_result = parse_parameter_group(
      args.m_start_pos, &args.m_end_pos, &vparam, annotation_flags);

    if (args.m_result != Result_ok)
      {
      return false;
      }

    // Transfer contents of vparam to heap version
    *param_new_pp = SK_NEW(SkGroupParam)(&vparam);
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a parameter specifier.
// Returns:    Result_ok, Result_err_expected_class, Result_err_context_non_class,
//             Result_err_expected_param_name, Result_err_typecheck_query_param or
//             Result_err_unexpected_eof
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Arg         tname_p - Pointer to an address to store a pointer to the class type
//             and name specified by the parameter specifier.  If there is no class part,
//             it automatically defaults to the 'Object' class.  It is not written to if
//             it is set to nullptr.
// Notes:      param-specifier = [class-desc wsr] variable-name
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_parameter_specifier(
  uint32_t      start_pos,
  uint32_t *    end_pos_p,
  SkTypedName * tname_p,
  uint32_t      param_flags, // = ParamFlag__default
  uint32_t      annotation_flags
  ) const
  {
  uint32_t end_pos;

  ASetOnReturn<uint32_t> set_end(&end_pos, end_pos_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get optional Class Descriptor
  SkClassDescBase * type_p = nullptr;
  eResult           result = parse_class_desc(start_pos, &end_pos, tname_p ? &type_p : nullptr);

  if (result == Result_ok)
    {
    // Eat {whitespace}
    result = parse_ws_required(end_pos, &end_pos);
    }
  else
    {
    // Class part is optional, so ignore if parse did not get part way
    if (end_pos != start_pos)
      {
      return result;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get parameter name
  ASymbol param_name;
  bool    predicate_b = false;

  result = parse_name_predicate(end_pos, &end_pos, &param_name, &predicate_b);

  if (result != Result_ok)
    {
    return (result == Result_err_expected_instance)
      ? Result_err_expected_param_name
      : result;
    }

  // Ensure that the variable name is not already used
  if (m_flags.is_set_any(Flag_type_check))
    {
    if (m_context.is_previous_variable(param_name))
      {
      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "A variable with the name '%s' has already been created and is available in this scope "
          "and duplicate/shadowed variable names are not allowed.\n"
          "Choose a different parameter name.\n"
          "[Note that this can include parameter names in closures that infer their interface.]",
          param_name.as_cstr_dbg());
      #endif

      return Result_err_context_duped_variable;
      }

    if (predicate_b)
      {
      if (type_p)
        {
        // Ensure type is Boolean since parameter name is a predicate
        if (type_p != SkBrain::ms_boolean_class_p)
          {
          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "'%s' was specified as the parameter type when Boolean was expected.\n"
              "Query/predicate parameters ending with a question mark '?' must be specified "
              "as a Boolean or omit the type in which case Boolean is inferred.",
              type_p->as_code().as_cstr());
          #endif

          return Result_err_typecheck_query_param;
          }
        }
      else
        {
        type_p = SkBrain::ms_boolean_class_p;
        }
      }
    }

  if (type_p == nullptr)
    {
    // Default to 'Object'/'Auto_' if no class specified
    // $Revisit - CReis [A_NOTE] ***Enhancement*** - [Auto-Type] Use SkTypedClass with Auto_ as 'key class' that can keep track of type narrowing/alternatives for send parameters
    type_p = (param_flags & ParamFlag_auto_type) ? SkBrain::ms_auto_class_p : SkBrain::ms_object_class_p;
    }

  // Check for type compatibility with the invokable's annotations
  // $UE4-specific
  if ((annotation_flags & SkAnnotation_ue4_blueprint)
   && (type_p->get_key_class()->get_annotation_flags() & SkAnnotation_reflected_data))
    {
    return Result_err_typecheck_ue4_blueprint_param;
    }

  // Note, the name could be added to the context at this point, but it could then
  // be erroneously used by the default code of later arguments.
  result = Result_ok;

  if (tname_p)
    {
    tname_p->m_type_p = type_p;
    tname_p->set_name(param_name);
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a unary parameter.
//
// Returns: true if successfully parsed, false if not
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//     m_result:
//       Result_ok, Result_err_expected_class, Result_err_expected_param_name,
//       or Result_err_unexpected_eof
//   uparam_p:
//     pointer to location to store parsed unary parameter. It is not written to if it is
//     set to nullptr.
//   annotation_flags: Annotations of the associated invokable
//
// Notes:
//   If the result of this parse a value other than Result_ok and 'args.m_end_pos' is set
//   to a value other than the original given by 'args.m_start_pos', it should be assumed
//   that this portion of the parse was identified as a unary parameter, but that it had
//   errors or warnings.
//
//   unary-parameter = param-specifier [ws binding]
//   param-specifier = [class-desc wsr] variable-name
//
// See: preparse_parameter_unary()
bool SkParser::parse_parameter_unary(
  Args &         args,
  SkUnaryParam * uparam_p,
  uint32_t       annotation_flags
  ) const
  {
  SkTypedName tname;

  args.m_result = parse_parameter_specifier(args.m_start_pos, &args.m_end_pos, &tname, ParamFlag__default, annotation_flags);
  
  if (args.m_result != Result_ok)
    {
    return false;
    }

  args.m_desired_type_p = tname.m_type_p;
  args.m_start_pos      =  args.m_end_pos;


  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    return false;
    }

  args.m_start_pos = args.m_end_pos;


  // Determine if there is a default binding
  uint32_t           pos            = args.m_start_pos;
  SkExpressionBase * default_expr_p = parse_binding(args);

  if (args.m_end_pos == pos)
    {
    // Wasn't even a partial parse, so ignore since default binding is optional
    args.m_result = Result_ok;
    }
  else
    {
    // Ensure that the result type of the default is compatible with the expected type
    // of the parameter.
    if (m_flags.is_set_any(Flag_type_check)
      && (args.m_result == Result_ok) 
      && !args.m_expr_type->is_class_type(tname.m_type_p))
      {
      args.m_result = Result_err_typecheck_default_param;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The `%s` parameter expects a `%s` type and the default expression has the type `%s` which is not compatible.",
          tname.get_name_cstr_dbg(),
          tname.m_type_p->as_code().as_cstr(),
          args.m_expr_type->as_code().as_cstr());
      #endif

      delete default_expr_p;
      }

    if (args.m_result != Result_ok)
      {
      return false;
      }

    // Advance end parse position
    args.m_start_pos =  args.m_end_pos;
    }

  if (uparam_p)
    {
    uparam_p->set_name(tname.get_name());
    uparam_p->m_type_p = tname.m_type_p;
    uparam_p->set_default_expr(default_expr_p);
    }

  // Add parameter to context scope - parameters that follow this one may use it.
  if (m_flags.is_set_any(Flag_type_check))
    {
    m_context.append_local(tname.get_name(), tname.m_type_p, false);
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a group parameter.
// Returns:    Result_ok, Result_err_unexpected_class_pattern, Result_err_expected_param_name,
//             Result_err_unexpected_char, Result_err_context_non_class, or Result_err_unexpected_eof
// Arg         start_pos - character position to begin lexical analysis
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.
// Arg         vparam_p - pointer to location to store parsed group parameter.
//             It is not written to if it is set to nullptr or if the result is not Result_ok.
// Examples:   if (parse.parse_parameter_group(11u, &end_pos, &vparam) == Result_ok)
// Notes:      If the result of this parse a value other than Result_ok and '*end_pos_p'
//             is set to a value other than the original given by 'start_pos', it should
//             be assumed that this portion of the parse was identified as a group
//             parameter, but that it had errors or warnings.
//
//             group-param = '{' ws [class-desc {wsr class-desc} ws] '}' ws instance-name
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_parameter_group(
  uint32_t       start_pos,
  uint32_t *     end_pos_p,
  SkGroupParam * gparam_p,
  uint32_t       annotation_flags
  ) const
  {
  eResult  result = Result_err_unexpected_eof;  // Next possible error
  uint32_t length = m_str_ref_p->m_length;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  if ((length - start_pos) >= 2u)
    {
    // Next possible error
    result = Result_err_expected_group_param;

    if (cstr_a[start_pos] == '{')
      {
      SkClassDescBase *  class_p;
      SkClassDescBase ** class_pp    = &class_p;
      SkClassDescBase *  type_p      = nullptr;   // Type class used for list
      uint32_t           class_count = 0u;

      start_pos++;

      // Parse class pattern list
      while (start_pos < length)
        {
        // Eat {whitespace}
        parse_ws_any(start_pos, &start_pos);

        if (cstr_a[start_pos] == '}')
          {
          result = Result_ok;

          start_pos++;
          
          // Found end of group parameter, so jump to end of while
          break;
          }
        else
          {
          // Get class
          result = parse_class_desc(start_pos, &start_pos, class_pp);

          if (result == Result_ok)
            {
            class_count++;

            if (class_count > SkGroupParam_max_class_count)
              {
              result = Result_err_size_group_param;
              // Too many classes in pattern, so jump to end of while
              break;
              }

            // Determine the List class type
            if (m_flags.is_set_any(Flag_type_check))
              {
              type_p = type_p
                ? SkClassUnion::get_merge(*type_p, *class_p)
                : class_p;
              }

            // Append class to class pattern
            if (gparam_p)
              {
              gparam_p->append_class(*class_p);
              }
            }
          else  // Bad class name
            {
            // $Revisit - CReis [Error Info] By substituting an error message info is lost
            // - a compound error type should be made
            result = Result_err_unexpected_class_pattern;
            // Found invalid class name, so jump to end of while
            break;
            }
          }
        }  // while

      if (result == Result_ok)
        {
        // Parse identifier name component

        // Eat {whitespace}
        parse_ws_any(start_pos, &start_pos);

        ASymbol name;

        // Get parameter identifier name
        result = parse_name_instance(start_pos, &start_pos, &name);

        if (result == Result_ok)
          {
          // Ensure that the variable name is not already used
          if (m_flags.is_set_any(Flag_type_check) && m_context.is_previous_variable(name))
            {
            // Next possible error
            result = Result_err_context_duped_variable;

            #if defined(SK_AS_STRINGS)
              ms_error_str.ensure_size_empty(500u);
              ms_error_str.format(
                "A variable with the name '%s' has already been created and is available in this scope "
                "and duplicate/shadowed variable names are not allowed.\n"
                "Choose a different parameter name.\n"
                "[Note that this can include parameter names in closures that infer their interface.]",
                name.as_cstr_dbg());
            #endif
            }
          else
            {
            result = Result_ok;

            // Add parameter to context scope - parameters that follow this one may use it.
            if (m_flags.is_set_any(Flag_type_check))
              {
              m_context.append_local(
                name,
                SkTypedClass::get_or_create(
                  SkBrain::ms_list_class_p,
                  type_p ? type_p : SkBrain::ms_object_class_p),
                false);
              }

            if (gparam_p)
              {
              gparam_p->get_name() = name;
              }
            }
          }
        else
          {
          // $Revisit - CReis [Error Info] By substituting an error message info is lost
          // - a compound error type should be made that contains all the relevant info.
          result = Result_err_expected_param_name;
          }
        }
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = start_pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Attempts to parse a parameters specification - used when defining a method,
//   coroutine, closure, etc.
//
// #Notes
//   parameters      = parameter-list [ws class-desc]
//   parameter-list  = '(' ws [send-params ws] [';' ws return-params ws] ')'
//   send-params     = parameter {ws [',' ws] parameter}
//   return-params   = param-specifier {ws [',' ws] param-specifier}
//   parameter       = unary-parameter | group-param
//   param-specifier = [class-desc wsr] variable-name
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if parsed correctly and false otherwise
  bool
SkParser::parse_parameters(

  // m_result:
  //   Result_ok, Result_err_expected_parameters, Result_err_expected_parameter_next
  //   Result_err_expected_parameter, Result_err_unexpected_parameter_rargs,
  //   Result_err_unexpected_parameters_result, Result_err_typecheck_query_result,
  //   Result_err_typecheck_ue4_blueprint_param or any other warning or error.
  Args & args,

  // Parameter list to fill. Ignored if nullptr.
  // $Revisit - This method should just return a parameters structure, though the old
  // version of this method passed in a param structure and param structures are also
  // pooled via SkParameters::get_or_create(). 
  SkParameters * params_p, // = nullptr

  // See SkParser::eParamFlag
  uint32_t flags, // = ParamFlag__default

  // Annotations of the invokable that we belong to
  uint32_t annotation_flags // = SkAnnotation__none

  ) const
  {
  enum eParamType
    {
    ParamType_send_preparse,
    ParamType_send,
    ParamType_return
    };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure it starts with parameters opening bracket
  char *   cstr_a = m_str_ref_p->m_cstr_p;
  uint32_t length = m_str_ref_p->m_length;

  if ((length <= args.m_start_pos) || (cstr_a[args.m_start_pos] != '('))
    {
    args.m_result  = Result_err_expected_parameters;
    args.m_end_pos = args.m_start_pos;

    return false;
    }

  args.m_start_pos++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through send parameters and return parameters
  char       next_char;
  bool       send_params_b  = true;  // false = parsing "return params"
  bool       param_first    = false; // has the first send/return param been seen?
  bool       param_required = false;
  uint32_t   arg_flags      = ParamFlag__default;
  eParamType param_type
    = m_flags.is_set_any(Flag_preparse) ? ParamType_send_preparse : ParamType_send;

  SkParameters params;  // Working prarameters

  // $Revisit - CReis [A_NOTE] ***Enhancement*** - [Auto-Type] Modify parameter parse to allow auto-typing

  args.m_result = Result_err_expected_parameter_next;

  while (args.m_start_pos < length)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Eat {whitespace}
    if (!parse_ws_any(args))
      {
      return false;
      }

    args.m_start_pos = args.m_end_pos;


    // [Using if statements here rather than switch so break & continue can be used to
    // control while loop.]

    next_char = cstr_a[args.m_start_pos];

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for end of parameter list
    if (next_char == ')')
      {
      if (param_required)
        {
        args.m_result  = Result_err_expected_parameter;  // missing parameter got ')'
        args.m_end_pos = args.m_start_pos;

        return false;
        }

      // Found end of parameter list
      args.m_start_pos++;

      break;  // break out of while loop
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for end of send parameters & start of return parameters
    if (next_char == ';')
      {
      if (param_required)
        {
        args.m_result  = Result_err_expected_parameter;  // missing parameter got ';'
        args.m_end_pos = args.m_start_pos;

        return false;
        }

      // Has parsing of return parameters already started?
      if (!send_params_b)
        {
        // Return parameters already started - found extra semi-colon ';'!
        args.m_result  = Result_err_unexpected_parameter_rargs;
        args.m_end_pos = args.m_start_pos;

        return false;
        }

      args.m_start_pos++;
      param_type    = ParamType_return;
      send_params_b = false;
      param_first   = false;
      arg_flags     = ParamFlag_auto_type;

      continue;  // continue while loop
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for parameter delimiter
    if (next_char == ',')
      {
      if (param_required || !param_first)
        {
        args.m_result  = Result_err_expected_parameter;  // missing parameter got ','
        args.m_end_pos = args.m_start_pos;

        return false;
        }

      args.m_start_pos++;
      param_required = true;

      continue;  // continue while loop
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse/preparse send/return parameter
    switch (param_type)
      {
      case ParamType_send_preparse:
        args.m_result = preparse_param_append(
          args.m_start_pos, &args.m_end_pos, &params, arg_flags, annotation_flags);
        break;

      case ParamType_send:
        parse_param_append(args, &params, arg_flags, annotation_flags);
        break;

      case ParamType_return:
        args.m_result = parse_param_return_append(
          args.m_start_pos, &args.m_end_pos, &params, arg_flags, annotation_flags);
      }

    if (args.m_result != Result_ok)
      {
      // Found bad parameter parse, so exit
      return false;
      }

    param_required = false;
    param_first    = true;

    // Next possible error
    args.m_result    = param_required ? Result_err_expected_parameter : Result_err_expected_parameter_next;
    args.m_start_pos = args.m_end_pos;
    }  // end of while loop


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for optional return class type
  // args.m_start_pos = end of parameters
  uint32_t params_end = args.m_start_pos;  // end of parameters and beginning of optional whitespace prior to an optional result class

  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    return false;
    }

  args.m_start_pos = args.m_end_pos;


  // Determine result type
  SkClassDescBase * result_class_p;

  switch (flags & ParamFlag__mask_result)
    {
    case ParamFlag_coroutine:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Automatically use `InvokedCoroutine` as result class and
      // ensure no result class is given since that is a common error
      args.m_result = parse_class_desc(args.m_start_pos, &args.m_end_pos);

      if (args.m_start_pos != args.m_end_pos)
        {
        // Found full/partial result type which wasn't desired
        // Keep error if there already was one
        if (args.m_result == Result_ok)
          {
          args.m_result = Result_err_unexpected_parameters_result;
          }

        return false;
        }

      // Parse did not advance - result type was left out as desired
      args.m_result  = Result_ok;
      args.m_end_pos = params_end;
      params.set_result_type(*SkBrain::ms_invoked_coroutine_class_p);
      break;

    case ParamFlag_result_bool:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Predicate/query method that requires or infers Boolean result.
      params.set_result_type(*SkBrain::ms_boolean_class_p);
      args.m_result = parse_class_desc(args.m_start_pos, &args.m_end_pos, &result_class_p);

      if (args.m_result == Result_ok)
        {
        // Optional return class type present - ensure it is Boolean type
        if (result_class_p != SkBrain::ms_boolean_class_p)
          {
          args.m_result = Result_err_typecheck_query_result;

          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "'%s' was specified as the result type when Boolean was expected.\n"
              "Query/predicate methods ending with a question mark `?` must either specify "
              "a Boolean result or omit the result type in which case Boolean is inferred.",
              result_class_p->as_code().as_cstr());
          #endif

          return false;
          }
        }
      else
        {
        if (args.m_start_pos != args.m_end_pos)
          {
          // Keep class description error and return
          return false;
          }

        // Parse did not advance so result type omitted - infer Boolean result type.
        args.m_result  = Result_ok;
        args.m_end_pos = params_end;
        }
      break;

    default:
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Result class desired (or inferred)
      args.m_result = parse_class_desc(args.m_start_pos, &args.m_end_pos, &result_class_p);

      if (args.m_result == Result_ok)
        {
        // Check for type compatibility with the invocable's annotations
        // $UE4-specific
        if ((annotation_flags & SkAnnotation_ue4_blueprint)
         && (result_class_p->get_key_class()->get_annotation_flags() & SkAnnotation_reflected_data))
          {
          args.m_result = Result_err_typecheck_ue4_blueprint_param;
          return false;
          }

        // Optional return class type present
        params.set_result_type(*result_class_p);
        }
      else
        {
        if (args.m_start_pos != args.m_end_pos)
          {
          // Keep class description error and return
          return false;
          }

        // Parse did not advance so result type was left out.  Attempt to auto infer type
        // later by examining code block body etc.
        args.m_result = Result_ok;
        params.set_result_type(
            (flags & ParamFlag_auto_type)
              ? *SkBrain::ms_auto_class_p
              : *SkBrain::ms_object_class_p);
        }
    }  // switch param flag

  if ((params_p == nullptr) || !args.is_struct_wanted())
    {
    return false;
    }

  params_p->assign(&params);

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a coroutine with the supplied name and
//             optionally appending it to the supplied class.  It is an "atomic" coroutine
//             (i.e. it is a forward declaration for a C++ method) if the optional code
//             block is absent.  It is a "custom" coroutine if the optional code block is
//             present.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkMethodBase data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, or pretty much any other
//                 warning or error.
// Arg         name - name to identify coroutine (usually taken from the file name).
// Arg         append_to_class_b - indicates if the coroutine should be appended to the
//             current class scope.
// See:        parse_invoke_coroutine()
// Notes:      It may be useful to return/fill the appropriate coroutine data structure
//             instead of just appending it to the class.
//
//             coroutine      = parameter-list [ws code-block]
//             parameter-list = '(' ws [send-params ws] [';' ws return-params ws] ')'
//             send-params    = parameter {ws ',' ws parameter}
//             return-params  = param-specifier {ws ',' ws param-specifier}
// Author(s):   Conan Reis
SkCoroutineBase * SkParser::parse_coroutine(
  Args &          args,             // = ms_def_args.reset()
  const ASymbol & name,             // = ASymbol::ms_null
  bool            append_to_class_b // = true
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse parameters
  uint32_t          pos         = args.m_start_pos;
  SkParameters      params;
  SkCoroutineBase * coroutine_p = nullptr;
  SkClass *         key_scope_p = m_context.m_obj_scope_p->get_key_class();

  m_member_type = SkMember_coroutine;

  #if (SKOOKUM & SK_DEBUG)
    // Get old method to replace if this is a reparse
    if (SkClass::ms_reparse_info.m_is_active)
      {
      SkQualifier ident(name, key_scope_p);

      coroutine_p = SkClass::ms_reparse_info.m_coroutines.pop(ident);
      }
  #endif

  // Parse annotations
  Annotations annotations;
  args.m_result = parse_annotations(pos, &pos, &annotations, SkAnnotationTarget_invokable);

  if (args.m_result == Result_ok)
    {
    // Parse parameters
    m_context.m_params_p = args.is_struct_wanted() ? &params : nullptr;

    args.m_start_pos = pos;
    parse_parameters(args, m_context.m_params_p, ParamFlag__default, annotations.m_flags);
    pos = args.m_end_pos;

    if (args.m_result == Result_ok)
      {
      // $Revisit - CReis [Type-check] An overriding coroutine should be tested to ensure that
      // it has the same parameters and defaults as the coroutine that it overrides.

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Check for optional code block
      uint32_t block_pos;

      args.m_result = parse_ws_any(pos, &block_pos);

      if (args.is_ok())
        {
        SkExpressionBase * expr_p = nullptr;

        bool make_struct = args.is_struct_wanted();
        bool atomic_code = m_str_ref_p->m_cstr_p[block_pos] != '[';

        if (!atomic_code)
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Parse custom code block
          args.m_start_pos = block_pos;

          expr_p = parse_code_block_optimized(args, SkInvokeTime_any, ResultDesired_false);

          pos = args.m_end_pos;

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Ensure returned values from code block are compatible with parameters
          make_struct = args.is_ok() && parameters_typecheck(args, &params, false) && expr_p;
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Make/fix-up method structure
        if (make_struct)
          {
          SkParameters * params_p = SkParameters::get_or_create(&params);

          if (coroutine_p)
            {
            eSkInvokable type = coroutine_p->get_invoke_type();

            if ((atomic_code && (type != SkInvokable_coroutine))
              || (!atomic_code && (type == SkInvokable_coroutine)))
              {
              // Reuse old coroutine prior to reparse to keep bindings, etc.
              coroutine_p->set_params(params_p);
              coroutine_p->set_annotation_flags(annotations.m_flags);
              coroutine_p->set_invoked_data_array_size(m_context.m_current_scope_p->m_data_idx_count_max);

              if (!atomic_code)
                {
                static_cast<SkCoroutine *>(coroutine_p)->set_expression(expr_p);
                }
              }
            else
              {
              // Different coroutine type
              delete coroutine_p;
              coroutine_p = nullptr;
              }
            }

          if (coroutine_p == nullptr)
            {
            if (atomic_code)
              {
              // $Note - CReis Using a SkCoroutineMthd for the C++ method rather than a
              // SkCoroutineFunc since it is either larger or the same size as SkCoroutineFunc
              // which can be swapped in-place during the binding of the C++ code as needed.
              coroutine_p = SK_NEW(SkCoroutineMthd)(name, key_scope_p, params_p, annotations.m_flags);
              }
            else
              {
              coroutine_p = SK_NEW(SkCoroutine)(name, key_scope_p, params_p, m_context.m_current_scope_p->m_data_idx_count_max, annotations.m_flags, expr_p);
              }
            }

          coroutine_p->set_akas((tSkAkas &&)annotations.m_akas);
          }
        else
          {
          if (expr_p)
            {
            delete expr_p;
            }
          }
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append to class as requested
  if (append_to_class_b && coroutine_p && !name.is_null())
    {
    m_context.m_obj_scope_p->append_coroutine(coroutine_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove parameters and modified data members from variable list
  if (m_flags.is_set_any(Flag_type_check))
    {
    m_context.free_all_locals();
    }

  args.m_end_pos = pos;

  m_member_type = SkMember__invalid;

  return coroutine_p;
  }

//---------------------------------------------------------------------------------------
//  Parses starting at 'start_pos' attempting to create a coroutine with the
//              supplied name and appending it to the supplied class.  It is an atomic
//              coroutine (i.e. it is a forward declaration for a C++ method) if the
//              optional code block is absent.  It is a custom coroutine if the optional
//              code block is present
// Returns:     Result_ok or pretty much any other warning or error. 
// Arg          name - name to identify coroutine (usually taken from the file name)
// Arg          scope_p - class to associate with / append to - it must be an Actor class
// Arg          args - starting & ending position, result, etc. see SkParser::Args
// Examples:    if (parse.parse_coroutine(coro_name, class_p) == Result_ok)
// See:         parse_coroutine(), parse_invoke_coroutine()
// Notes:       It may be useful to return/fill the appropriate coroutine data structure
//              instead of just appending it to the class.
//
//                coroutine-file = ws coroutine ws
//                coroutine      = parameter-list [ws code-block]
//
// Author(s):    Conan Reis
SkCoroutineBase * SkParser::parse_coroutine_source(
  const ASymbol &    name,
  SkClassUnaryBase * scope_p,
  Args &             args,             // = ms_def_args.reset()
  bool               append_to_class_b // = true
  )
  {
  SK_ASSERTX(!args.is_struct_wanted() || m_flags.is_set_any(Flag_type_check), "Type checking must be on if struct generation is desired.");

  reset_scope(scope_p, name);

  // Eat {whitespace}
  args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos);

  SkCoroutineBase * coroutine_p = nullptr;

  if (args.is_ok())
    {
    args.m_start_pos = args.m_end_pos;

    coroutine_p = parse_coroutine(args, name, append_to_class_b);

    if (args.is_ok())
      {
      args.m_start_pos = args.m_end_pos;

      parse_ws_end(args);
      }
    }

  return coroutine_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a temporary variable. Any temporary variable is added to
// the current context - no expression is created for a temporary variable.
// 
// Returns: true if parsed correctly, false if not
// 
// Params:
//   args:
//     See SkParser::Args, specific considerations below:
//       ArgFlag_make_struct -
//         if set then `ident_` (and `expr_pp` if `allow_binding`= true) must be non-nullptr
//       m_result:
//         Result_ok, Result_err_expected_temporary, Result_err_unexpected_cdtor,
//         Result_err_expected_binding, Result_err_unexpected_reserved,
//         Result_err_context_duped_variable, Result_err_unexpected_eof, or any result
//         returned from parsing an expression.
//   ident_p:
//     Address to store variable name. Ignored if ArgFlag_make_struct is not set
//   expr_pp:
//     Address to store optional default expression. If there is no default expression,
//     the expression part of the bind will be set to `nullptr`.  Ignored if
//     ArgFlag_make_struct is not set or if `allow_binding` set to `false`.
//   bind_pos_p:
//     Address to store the text location of the bind operator ':'
//   predicate_p:
//     Set to `true` if variable is a predicate ending with `?` or `false` if not
//   allow_binding:
//     If `true` parse for optional binding part `[ws binding]` if `false` just test for
//     define-temporary part `'!' ws variable-name`.
//     
// Notes:
//   This method also ensures that neither a constructor call nor a destructor call is
//   considered to be a valid create temporary statement - both cases return
//   `Result_err_unexpected_cdtor` and do not advance the parse position.
//
//   If the result of this parse is other than `Result_ok` and `args.m_end_pos` is set to
//   a value other than the original given by `args.m_start_pos`, it should be assumed
//   that this portion of the parse was identified as a create temporary but that it had
//   errors or warnings.
//
//   ```
//   create-temporary = define-temporary [ws binding]
//   define-temporary = '!' ws variable-name
//   binding          = ':' ws expression
//   ```
//             
// See:       parse_data_definition() - fairly similar parse if not identical
// Author(s): Conan Reis
bool SkParser::parse_temporary(
  Args &              args,         // = ms_def_args.reset()  Start pos 0 with default flags
  ASymbol *           ident_p,      // = nullptr
  SkExpressionBase ** expr_pp,      // = nullptr
  uint32_t *          bind_pos_p,   // = nullptr
  bool *              predicate_p,  // = nullptr
  bool                allow_binding // = true
  ) const
  {
  uint32_t pos    = args.m_start_pos;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  if (cstr_a[pos] != '!')
    {
    args.m_result = Result_err_expected_temporary;
    return false;
    }

  pos++;

  // Ensure that it is not a constructor or a destructor
  if ((cstr_a[pos] == '(') || (cstr_a[pos] == '!'))
    {
    args.m_result = Result_err_unexpected_cdtor;
    return false;
    }

  // Eat {whitespace}
  args.m_result = parse_ws_any(pos, &pos);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse variable name
  bool predicate_b = false;
  args.m_result = parse_name_predicate(pos, &pos, ident_p, &predicate_b);

  if (!args.is_ok())
    {
    args.m_end_pos = pos;
    return false;
    }

  // Next possible error

  // Ensure that it is not a named constructor
  if (cstr_a[pos] == '(')
    {
    // Don't advance parse if constructor
    args.m_result  = Result_err_unexpected_cdtor;
    args.m_end_pos = args.m_start_pos;
    return false;
    }

  // It is definitely a create temporary and not a constructor or a destructor

  // Ensure that the variable name is not already used
  if (m_flags.is_set_any(Flag_type_check)
    && ident_p
    && m_context.is_previous_variable(*ident_p))
    {
    args.m_end_pos = pos;
    args.m_result  = Result_err_context_duped_variable;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "A variable with the name '%s' has already been created and is available in this "
        "scope and duplicate/shadowed variable names are not allowed to ensure thread safety and "
        "coder sanity.\n"
        "Choose a different variable name or just use previous variable without recreating it.",
        ident_p->as_cstr_dbg());
    #endif

    return false;
    }

  if (predicate_p)
    {
    *predicate_p = predicate_b;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // At this point it is a valid create temporary without an initial binding

  // Parse optional initial binding
  uint32_t           bind_start;
  bool               binding_b   = false;
  SkExpressionBase * bind_expr_p = nullptr;

  // Eat {whitespace}
  if (allow_binding
    && (parse_ws_any(pos, &bind_start) == Result_ok)
    && ((cstr_a[bind_start] == ':')
      || (cstr_a[bind_start] == '='))) // Also check for = since that is a common error
    {
    // Check for common error using = or := instead of :
    if ((cstr_a[bind_start] == '=')
      || (cstr_a[bind_start + 1] == '='))  // Check for := used in Go and other languages
      {
      args.m_end_pos = bind_start + ((cstr_a[bind_start + 1] == '=') ? 2u : 1u);
      args.m_result  = Result_err_unexpected_cpp;

      #if defined(SK_AS_STRINGS)
        ms_error_str.empty();
        ms_error_str.append(
          "SkookumScript uses a colon `:` for the initial bind of a temporary variable to an object.\n"
          "!var: obj_expression\n"
          "[After a variable is created, `:=` is used for assignment and `:` is used to bind a variable to a new object.]");
      #endif

      return false;
      }

    // Pass on desired type (if there is one)
    if (predicate_b)
      {
      args.m_desired_type_p = SkBrain::ms_boolean_class_p;
      }

    args.m_start_pos = bind_start;
    bind_expr_p = parse_binding(args);

    // Determine if parse advanced
    if (args.m_end_pos != bind_start)
      {
      // There was a binding - or an attempt at one but there was an error
      pos = args.m_end_pos;
      binding_b = true;

      // If typechecking and result type != Boolean
      if (predicate_b
        && args.is_ok()
        && m_flags.is_set_any(Flag_type_check)
        && (args.m_expr_type != SkBrain::ms_boolean_class_p))
        {
        if (bind_expr_p)
          {
          delete bind_expr_p;
          }

        args.m_end_pos = bind_start;
        args.m_result  = Result_err_typecheck_query_variable;
        
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The bound object is type '%s' when Boolean was expected.\n"
            "Query/predicate temporary variables ending with `?` may only be bound `:` "
            "to a Boolean `true`/`false` expression",
            args.m_expr_type->as_code().as_cstr());
        #endif

        return false;
        }

      if (expr_pp)
        {
        *expr_pp = bind_expr_p;
        }

      if (bind_pos_p)
        {
        *bind_pos_p = bind_start;
        }
      }
    }

  if (!binding_b)
    {
    // No initial binding - ignore read ahead
    args.m_result = Result_ok;

    // If there is no specific initial binding, the temporary will be bound to "nil".
    // $Revisit - CReis Query/predicate variables could be set to a false literal by default.
    args.m_expr_type = SkNone::get_class();
    }

  args.m_end_pos = pos;

  return (args.is_ok());
  }

//---------------------------------------------------------------------------------------
//  Determines if a portion of code lexically conforms to whitespace
// Returns:     Result_ok, Result_err_expected_comment_close, or Result_err_unexpected_char
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Examples:    if (parse.parse_whitespace(11u, &end_pos) == Result_ok)
// See:         parse_ws_any(), parse_ws_required(), parse_comment(),
//              parse_comment_line(), parse_comment_multiline()
// Notes:       whitespace = whitespace-char | comment
// Author(s):    Conan Reis
SkParser::eResult SkParser::parse_whitespace(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p  // = nullptr
  ) const
  {
  if (ms_is_space[uint8_t(m_str_ref_p->m_cstr_p[start_pos])])
    {
    if (end_pos_p)
      {
      *end_pos_p = start_pos + 1u;
      }

    return Result_ok;
    }
  else
    {
    return parse_comment(start_pos, end_pos_p);
    }
  }

//---------------------------------------------------------------------------------------
// Determines if a portion of code lexically conforms to zero or more
//             instances of whitespace.
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Arg         treat_lf_as_ws - if line feed is considered white space
// Examples:   if (parse.parse_ws_any(11u, &end_pos) == Result_ok)
// See:        parse_whitespace(), parse_ws_required(), parse_comment(),
//             parse_comment_line(), parse_comment_multiline()
// Notes:      No eResult is returned since a zero match is acceptable and this
//             method would always return Result_ok.
//
//             ws = {whitespace}
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_ws_any(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  bool       treat_lf_as_ws   // = true
  ) const
  {
  uint32_t    pos    = start_pos;
  uint32_t    length = m_str_ref_p->m_length;
  eResult result = Result_ok;

  if (start_pos < length)
    {
    uint32_t end_pos = start_pos;

    do
      {
      pos = end_pos;

      if (pos >= length)
        {
        break;
        }

      end_pos = length;
      find(treat_lf_as_ws ? ACharMatch_not_white_space : ACharMatch_not_white_space_except_lf, 1u, &end_pos, pos);
      pos = end_pos;

      if (pos >= length)
        {
        break;
        }

      result = parse_comment(pos, &end_pos);
      }
    while (result == Result_ok);

    if (result != Result_err_expected_comment_close)
      {
      result = Result_ok;
      }
    }
  
  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Ensures that the start position to the end of the string is only whitespace and
// nothing else.
// 
// #Params
//   args:
//     m_result - Result_ok, Result_err_expected_comment_close, Result_err_unexpected_char
//       or Result_err_unexpected_char
//       
// #See
//   parse_whitespace(), parse_ws_any(), parse_comment(), parse_comment_line(),
//   parse_comment_multiline()
//   
// #Authors Conan Reis
bool SkParser::parse_ws_end(Args & args) const
  {
  if (!parse_ws_any(args))
    {
    return false;
    }

  args.m_start_pos = args.m_end_pos;

  // Ensure that parse went to end of source
  if (args.m_end_pos < m_str_ref_p->m_length)
    {
    args.m_result = Result_err_unexpected_char;
    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Determines if a portion of code lexically conforms to one or more
//             instances of whitespace.
// Returns:    Result_ok, Result_err_expected_whitespace, Result_err_expected_comment_close,
//             or Result_err_unexpected_char
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Examples:   if (parse.parse_whitespace(11u, &end_pos) == Result_ok)
// See:        parse_whitespace(), parse_ws_any(), parse_comment(),
//             parse_comment_line(), parse_comment_multiline()
// Notes:      wsr = whitespace {whitespace}
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_ws_required(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p  // = nullptr
  ) const
  {
  eResult result; 
  uint32_t    end_pos;
  uint32_t    pos = m_str_ref_p->m_length;

  find(ACharMatch_not_white_space, 1u, &pos, start_pos);
  end_pos = pos;
  result  = parse_comment(pos, &end_pos);

  if (result == Result_ok)
    {
    do
      {
      pos     = end_pos;
      end_pos = m_str_ref_p->m_length;
      find(ACharMatch_not_white_space, 1u, &end_pos, pos);
      pos = end_pos;
      }
    while (parse_comment(pos, &end_pos) == Result_ok);
    }

  if (result == Result_err_expected_comment_close)
    {
    // Found incomplete comment
    pos = end_pos;
    }
  else
    {
    // If parse advanced then there must have been some valid whitespace
    result = (pos != start_pos) ? Result_ok : Result_err_expected_whitespace;
    }

  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Preparses starting at `start_pos` attempting to create a partially parsed method with
// the supplied name and appending it to the supplied class for context.
// Quick parse - only parameters parsed any default arguments and code block body ignored.
// 
// Returns:   MethodFunc wrapper (regardless of whether code body is present) or nullptr.
// Params:  
//   name:    name to identify method (usually taken from the file name)
//   scope_p: scope of parse
//   args:    see Args - adds to class if ArgFlag_make_struct flag set.
//   
// Notes:  
//   ```
//   method-file    = ws method ws
//   method         = parameters [ws code-block]
//   parameters     = parameter-list [ws class-desc]
//   parameter-list = '(' ws [send-params ws] [';' ws return-params ws] ')'
//   send-params    = parameter {ws ',' ws parameter}
//   return-params  = param-specifier {ws ',' ws param-specifier}
//   ```
// See:         parse_method_source(), parse_method(), parse_invoke_method()
// Author(s):   Conan Reis
SkMethodBase * SkParser::preparse_method_source(
  const ASymbol &    name,
  SkClassUnaryBase * scope_p,
  Args &             args, // = ms_def_args.reset()
  bool *             has_signature_changed_p // = nullptr
  ) const
  {
  m_member_type = SkMember_method;
  reset_scope(scope_p, name);

  // Default assumption in presence of errors is that signature didn't change
  if (has_signature_changed_p)
    {
    *has_signature_changed_p = false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse annotations
  Annotations annotations;

  args.m_result = parse_annotations(args.m_start_pos, &args.m_end_pos, &annotations, SkAnnotationTarget_invokable);

  if (args.m_result != Result_ok)
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse parameters
  SkParameters params;

  if (!parse_parameters(args, &params, ParamFlag__default, annotations.m_flags))
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!args.is_struct_wanted())
    {
    SK_ASSERTX(has_signature_changed_p == nullptr,
      "`has_signature_changed_p` cannot be determined unless `is_struct_wanted()` is `true`.");

    m_member_type = SkMember__invalid;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make first pass of method
  // All of these data structures should be replaced during the full parse pass unless
  // there is a parse error.
  SkMethod * method_p = SK_NEW(SkMethod)(
    name,
    scope_p->get_key_class(),
    SkParameters::get_or_create(&params),
    params.get_arg_count_total(),
    annotations.m_flags,
    nullptr);
  method_p->set_akas((tSkAkas &&)annotations.m_akas);
  scope_p->append_method(method_p, has_signature_changed_p);

  m_member_type = SkMember__invalid;

  return method_p;
  }

//---------------------------------------------------------------------------------------
// Preparses starting at `start_pos` attempting to create a partially parsed method with
// the supplied name and appending it to the supplied class for context.
// Quick parse - only parameters parsed any default arguments and code block body ignored.
// 
// Returns:   SkCoroutineFunc wrapper (regardless of whether code body is present) or nullptr.
// Params:  
//   name:    name to identify method (usually taken from the file name)
//   scope_p: scope of parse
//   args:    see Args - adds to class if ArgFlag_make_struct flag set.
//   
// Notes:  
//   ```
//   coroutine-file = ws coroutine ws
//   coroutine      = parameter-list [ws code-block]
//   ```
// Author(s):   Conan Reis
SkCoroutineBase * SkParser::preparse_coroutine_source(
  const ASymbol &    name,
  SkClassUnaryBase * scope_p,
  Args &             args, // = ms_def_args.reset()
  bool *             has_signature_changed_p // = nullptr
  ) const
  {
  m_member_type = SkMember_coroutine;
  reset_scope(scope_p, name);

  // Default assumption in presence of errors is that signature didn't change
  if (has_signature_changed_p)
    {
    *has_signature_changed_p = false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Eat {whitespace}
  if (!parse_ws_any(args))
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse annotations
  Annotations annotations;

  args.m_result = parse_annotations(args.m_start_pos, &args.m_end_pos, &annotations, SkAnnotationTarget_invokable);

  if (args.m_result != Result_ok)
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse parameters
  SkParameters params;

  if (!parse_parameters(args, &params, ParamFlag_coroutine, annotations.m_flags))
    {
    m_member_type = SkMember__invalid;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!args.is_struct_wanted())
    {
    SK_ASSERTX(has_signature_changed_p == nullptr,
      "`has_signature_changed_p` cannot be determined unless `is_struct_wanted()` is `true`.");

    m_member_type = SkMember__invalid;
    return nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make first pass of method
  // All of these data structures should be replaced during the full parse pass unless
  // there is a parse error.
  SkCoroutineBase * coroutine_p = SK_NEW(SkCoroutine)(
    name,
    scope_p->get_key_class(),
    SkParameters::get_or_create(&params),
    params.get_arg_count_total(),
    annotations.m_flags,
    nullptr);
  coroutine_p->set_akas((tSkAkas &&)annotations.m_akas);
  scope_p->append_coroutine(coroutine_p, has_signature_changed_p);

  m_member_type = SkMember__invalid;

  return coroutine_p;
  }

//---------------------------------------------------------------------------------------
// Preparses starting at 'start_pos' attempting to create a unary parameter.
//             Only the parameter specifier is fully parsed - any default arguments are
//             only parsed enough to determine whether they are present and their number
//             of characters.
// Returns:    Result_ok, Result_err_expected_class, Result_err_expected_param_name,
//             or Result_err_unexpected_eof
// Arg         uparam_p - pointer to location to store parsed unary parameter.
// Arg         start_pos - character position to begin lexical analysis
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.
// Examples:   if (parse.preparse_parameter_unary(11u, &end_pos, &uparam) == Result_ok)
// See:        parse_parameter_unary()
// Notes:      If the result of this parse a value other than Result_ok and '*end_pos_p'
//             is set to a value other than the original given by 'start_pos', it should
//             be assumed that this portion of the parse was identified as a unary
//             parameter, but that it had errors or warnings.
//
//             unary-parameter = param-specifier [ws binding]
//             param-specifier = [class-desc wsr] variable-name
// Author(s):   Conan Reis
SkParser::eResult SkParser::preparse_parameter_unary(
  SkUnaryParam * uparam_p,
  uint32_t       start_pos,
  uint32_t *     end_pos_p,
  uint32_t       annotation_flags
  ) const
  {
  SkTypedName tname;
  eResult     result = parse_parameter_specifier(start_pos, &start_pos, &tname, ParamFlag__default, annotation_flags);
    
  if (result == Result_ok)
    {
    uint32_t pos = start_pos;

    // Eat {whitespace}
    parse_ws_any(pos, &pos);

    // Determine if there is a default binding, but don't bother fully parsing it
    SkExpressionBase * default_p = nullptr;
    Args               args(pos, ArgFlag__default_no_struct);
    
    args.m_desired_type_p = tname.m_type_p;
    parse_binding(args);

    if (args.m_end_pos == pos)
      {
      // Wasn't even a partial parse, so ignore since default binding is optional
      result = Result_ok;
      }
    else
      {
      // Advance end parse position
      start_pos = args.m_end_pos;
      result    = args.m_result;

      // Indicate that there is a default binding with a "nil"
      if (result == Result_ok)
        {
        default_p = SK_NEW(SkLiteral)(SkLiteral::Type__nil);
        }
      }

    uparam_p->set_name(tname.get_name());
    uparam_p->m_type_p = tname.m_type_p;
    uparam_p->set_default_expr(default_p);
    }

  if (end_pos_p)
    {
    *end_pos_p = start_pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
//  Preparses starting at 'start_pos' attempting to create a unary parameter
//              or group parameter but not fully parsing any default arguments.
// Returns:     Result_ok, Result_err_expected_class, Result_err_unexpected_class_pattern,
//              Result_err_expected_param_name, or Result_err_unexpected_eof
// Arg          param_new_pp - Pointer to an address to store a pointer to a dynamically
//              allocated parameter.
// Arg          start_pos - character position to begin lexical analysis (Default 0u)
// Arg          end_pos_p - character position that lexical analysis stopped at.  If it
//              is set to nullptr, it is not written to.  (Default nullptr)
// Examples:    if (parse.preparse_parameter(11u, &end_pos, &param_p) == Result_ok)
// See:         parse_parameter()
// Notes:       If the result of this parse a value other than Result_ok and '*end_pos_p'
//              is set to a value other than the original given by 'start_pos', it should
//              be assumed that this portion of the parse was identified as a parameter,
//              but that it had errors or warnings.
//
//              parameter = unary-parameter | group-param
// Author(s):    Conan Reis
SkParser::eResult SkParser::preparse_parameter(
  uint32_t           start_pos,
  uint32_t *         end_pos_p,
  SkParameterBase ** param_new_pp,
  uint32_t           annotation_flags
  ) const
  {
  uint32_t    end_pos;
  eResult result;

  // Parse for unary parameter
  SkUnaryParam uparam;

  result = preparse_parameter_unary(&uparam, start_pos, &end_pos, annotation_flags);

  if (result == Result_ok)
    {
    // Transfer contents of vparam to heap version
    *param_new_pp = SK_NEW(SkUnaryParam)(&uparam);
    }

  // If parse did not advance, 
  if (end_pos == start_pos)
    {
    // Parse for variable length parameter
    SkGroupParam vparam;

    result = parse_parameter_group(start_pos, &end_pos, &vparam, annotation_flags);

    if (result == Result_ok)
      {
      // Transfer contents of vparam to heap version
      *param_new_pp = SK_NEW(SkGroupParam)(&vparam);
      }
    }

  if (end_pos_p)
    {
    *end_pos_p = end_pos;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Preparses starting at 'start_pos' attempting to create a unary parameter
//             or group parameter (but not fully parsing any default arguments) and
//             appends it to params_p.
// Returns:    Result_ok, Result_err_context_duped_param_name, Result_err_expected_class,
//             Result_err_unexpected_class_pattern, Result_err_expected_param_name, or
//             Result_err_unexpected_eof
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Arg         params_p - pointer to empty parameter list to append parsed arguments to.
// Arg         param_flags - see SkParser::eParamFlag.  Currently only uses ParamFlag_auto_type
// Examples:   if (parse.preparse_param_append(11u, &end_pos, &params) == Result_ok)
// See:        parse_param_append()
// Notes:      Called by parse_parameters()
//             
//             If the result of this parse a value other than Result_ok and '*end_pos_p'
//             is set to a value other than the original given by 'start_pos', it should
//             be assumed that this portion of the parse was identified as a parameter,
//             but that it had errors or warnings.
//
//             param-specifier = [class-desc wsr] variable-name
// Author(s):   Conan Reis
SkParser::eResult SkParser::preparse_param_append(
  uint32_t       start_pos,
  uint32_t *     end_pos_p,
  SkParameters * params_p,
  uint32_t       param_flags,
  uint32_t       annotation_flags
  ) const
  {
  eResult           result;
  SkParameterBase * param_p = nullptr;

  // Parse parameter
  result = preparse_parameter(start_pos, end_pos_p, params_p ? &param_p : nullptr, annotation_flags);

  if (param_p)
    {
    // Add to parameter list
    params_p->m_params.append_absent(*param_p);
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Determines if next identifier looks like a class and whether the class is currently
// in the class hierarchy.
// 
// #Notes     Ensures no symbol is created.
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Identify_class, Identify_class_like or Identify_lexical_error
  SkParser::eIdentify
SkParser::identify_class(
  uint32_t start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  // Store name of class if looks like class. Ignored if nullptr.
  AString * class_name_p, // = nullptr
  // Store class if matches existing class. Ignored if nullptr.
  SkClass ** class_pp // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure it starts with capital letter
  uint32_t     length = m_str_ref_p->m_length;
  const char * cstr_a = m_str_ref_p->m_cstr_p;

  if ((start_pos >= length)
    || !ms_is_uppercase[uint8_t(cstr_a[start_pos])])
    {
    if (end_pos_p)
      {
      *end_pos_p = start_pos;
      }

    return Identify_lexical_error;
    }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if it is an existing class 
  uint32_t end_pos = a_min(start_pos + 1u, length - 1u);

  parse_name_symbol(start_pos, &end_pos);

  uint32_t ident_length = end_pos - start_pos;

  // Don't create symbol unless it already exists - don't want to spam sym/str DB
  ASymbol   class_sym(ASymbol::create_existing(cstr_a + start_pos, ident_length));
  SkClass * class_p = class_sym.is_null() ? nullptr : SkBrain::get_class(class_sym);

  if (class_name_p)
    {
    get(class_name_p, start_pos, ident_length);
    }

  if (class_pp)
    {
    *class_pp = class_p;
    }

  if (end_pos_p)
    {
    *end_pos_p = end_pos;
    }

  // Does it just look like a class or is it a confirmed class?
  return (class_p && !class_p->is_deleted()) ? Identify_class : Identify_class_like;
  }

//---------------------------------------------------------------------------------------
// Quickly identifies/categorizes a section of code without necessarily doing
//             a full analysis.
// Returns:    See SkParser::eIdentify
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Author(s):   Conan Reis
SkParser::eIdentify SkParser::identify_text(
  uint32_t   start_pos, // = 0u
  uint32_t * end_pos_p, // = nullptr
  uint       flags      // = IdentifyFlag__default
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure the text has some characters or don't bother
  uint32_t length = m_str_ref_p->m_length;

  if (start_pos >= length)
    {
    if (end_pos_p)
      {
      *end_pos_p = length;
      }

    return Identify_normal_text;
    }


  // This method should not create any symbols since it would create many as identifiers
  // are being typed in - for each character.

  uint32_t  end_pos = length;
  eIdentify ident   = Identify_normal_text;

  char * cstr_a = m_str_ref_p->m_cstr_p;
  char   ch     = cstr_a[start_pos];
  char   ch2    = ((start_pos + 1u) < end_pos) ? cstr_a[start_pos + 1u] : '\0';
  char   value;

  switch (ch)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '"':   // String literal
      if (flags & IdentifyFlag_break_strings)
        {
        // Treat only the double quote as a string and the sub parts identified as code.
        // Useful for search fields that group exact phrases with double quotes and word
        // breaking logic.
        ident   = Identify_string;
        end_pos = start_pos + 1u;
        break;
        }

      start_pos++;

      // Set pos to the last end quote - skipping over any quote character escape sequences
      if (start_pos < length)
        {
        do
          {
          end_pos = length;

          if (find(ch, 1u, &end_pos, start_pos))
            {
            end_pos++;
            }

          start_pos = end_pos;
          } while ((start_pos != length) && (cstr_a[start_pos - 1] == '\\'));
        }

      ident = Identify_string;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '\'':  // Symbol literal
      if (flags & IdentifyFlag_break_symbols)
        {
        // Treat only the single quote as a symbol and the sub parts identified as code.
        // Useful for search fields that group phrases with single quotes and word
        // breaking logic.
        ident   = Identify_symbol;
        end_pos = start_pos + 1u;
        break;
        }

      start_pos++;

      // Set pos to the last end quote - skipping over any quote character escape sequences
      if (start_pos < length)
        {
        do
          {
          end_pos = start_pos - 1u;

          do
            {
            end_pos++;
            value = cstr_a[end_pos];
            }
          while ((value != '\'') && (value != '\n') && (end_pos < length));

          if (value == '\'')
            {
            end_pos++;
            }

          start_pos = end_pos;
          } while ((start_pos != length) && (cstr_a[start_pos - 1] == '\\'));
        }

      ident = Identify_symbol;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '(':
    case '{':
    case '[':
      end_pos = start_pos + 1u;
      ident   = Identify_op_group_open;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ')':
    case '}':
    case ']':
      end_pos = start_pos + 1u;
      ident   = Identify_op_group_close;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '_':   // coroutine?
      end_pos = a_min(start_pos + 1u, length - 1u);
      ch      = cstr_a[end_pos];

      if (ms_is_lowercase[uint8_t(ch)])
        {
        parse_name_symbol(start_pos, &end_pos);
        }
      else
        {
        // Is it an invokable class?
        end_pos++;

        if (ch == '(')
          {
          ident = Identify_op_group_open;
          }
        else
          {
          ident = Identify_lexical_error;
          }
        }
      // else normal text
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Delimiters - require full parse to determine if truly valid
    case '.':
      // Is it a number?
      if (ms_is_digit[uint8_t(ch2)])
        {
        Args args(start_pos, ArgFlag__default_no_struct);

        parse_literal_number(args);
        ident = args.is_ok() ? Identify_number : Identify_lexical_error;
        end_pos = args.m_end_pos;
        break;
        }

      // Assume it is a "member access" operator
      ident = Identify_operator;
      end_pos = start_pos + 1u;
      break;

    case ',':
    case ';':  // Return arguments delimiter - note not valid for a statement terminator
      ident   = Identify_operator;
      end_pos = start_pos + 1u;
      break;

    case '%':
      ident = Identify_operator;
      if (ch2 == '>')                     // %> race apply
        {
        end_pos = start_pos + 2u;
        }
      else                                 // % apply
        {
        end_pos = start_pos + 1u;
        }
      break;

    case '|':
      end_pos = start_pos + 1u;
      if (ms_is_uppercase[uint8_t(ch2)])
        {
        // Assume it is part of a union class <ClassA|ClassB>
        ident = Identify_operator;
        }
      else
        {
        ident = Identify_lexical_error;
        }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '@':   // Data member or Object ID
      end_pos = start_pos + 1u;
      ident   = Identify_operator;

      ch = ((start_pos + 1u) < length) ? cstr_a[start_pos + 1u] : '\0';

      switch (ch)
        {
        case '\'':
          // It is an object ID reference
          start_pos++;
          ident = Identify_object_id;
          break;

        case '?':
          // It is an object ID possible reference - might not find object and return nil
          start_pos += 2;
          end_pos    = start_pos;
          ch         = cstr_a[start_pos];
          ident      = Identify_object_id;
          break;

        case '#':
          // It is an object ID identifier - a class specific validated name
          start_pos += 2;
          end_pos    = start_pos;
          ch         = cstr_a[start_pos];
          ident      = Identify_object_id;
          break;

        default:
          // Scope resolution operator or data member
          // Determine character prior to @
          ch = (start_pos >= 1u) ? cstr_a[start_pos - 1u] : '\0';

          // If not identifier character prior (tail end of Class name) then must be
          // data member else must be scope resolution operator.
          if (ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch)]) 
            {
            if (parse_name_data_member(start_pos, &end_pos) != Result_ok)
              {
              ident = Identify_lexical_error;
              break;
              }

            ident = Identify_data_member;
            }
        }

      if ((ident == Identify_object_id) && (ch == '\''))
        {
        // Set pos to the last end quote - skipping over any quote character escape sequences
        start_pos++;

        if (start_pos < length)
          {
          do
            {
            end_pos = start_pos - 1u;

            do
              {
              end_pos++;
              value = cstr_a[end_pos];
              }
            while ((value != '\'') && (value != '\n') && (end_pos < length));

            if (value == '\'')
              {
              end_pos++;
              }

            start_pos = end_pos;
            } while ((start_pos != length) && (cstr_a[start_pos - 1] == '\\'));
          }
        }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '&':   // Annotation
      {
      Annotations annotations;
      if (parse_annotations(start_pos, &end_pos, &annotations, SkAnnotationTarget__any) != Result_ok)
        {
        ident = Identify_lexical_error;
        break;
        }
      }
      ident = Identify_annotation;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '^':   // Closure
      end_pos = start_pos + 1u;
      ident   = Identify_operator;

      ch = ((start_pos + 1u) < length) ? cstr_a[start_pos + 1u] : '\0';

      switch (ch)
        {
        case '_':
          // Is it a closure coroutine?
          ch = ((start_pos + 2u) < length) ? cstr_a[start_pos + 2u] : '\0';

          // Ensure it is not a coroutine name
          if (!ms_is_lowercase[uint8_t(ch)])
            {
            end_pos = start_pos + 2u;
            }
          break;
        }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '/':   // Comment (or divide operator)
      switch (ch2)
        {
        case '/':  // Line comment
        case '*':  // Multi-line comment
          ident = Identify_comment;

          if (flags & IdentifyFlag_break_comments)
            {
            end_pos = start_pos + 2u;
            break;
            }

          end_pos = start_pos;

          do
            {
            start_pos = end_pos;
            parse_comment(start_pos, &end_pos);
            } while (start_pos != end_pos);
          break;

        case '=':  // divide assign op /=
          end_pos = start_pos + 2u;
          ident   = Identify_operator;
          break;

        default:  // assume it is a divide op /
          end_pos = start_pos + 1u;
          ident   = Identify_operator;
          break;
          }
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Operators
    case '=':
      if (ch2 == '=')  // == C++ mistake/assumption
        {
        end_pos = start_pos + 2u;
        ident   = Identify_lexical_error;
        }
      else  // =
        {
        end_pos = start_pos + 1u;
        ident   = Identify_operator;
        }
      break;

    case '>':
      ident = Identify_operator;
      switch (ch2)
        {
        case '=':  // >=
        case '>':  // >> Conversion
          end_pos = start_pos + 2u;
          break;

        default:   // >
          end_pos = start_pos + 1u;
        }
      break;

    case '<':
      ident = Identify_operator;
      switch (ch2)
        {
        case '=':  // <=
        case '>':  // <> Cast
          end_pos = start_pos + 2u;
          break;

        case '<':  // Use Integer.bit_shift_up() instead of << C++ style operator
          ident   = Identify_lexical_error;
          end_pos = start_pos + 2u;
          break;

        default:   // <
          end_pos = start_pos + 1u;
        }
      break;

    case '+':
      ident = Identify_operator;
      switch (ch2)
        {
        case '=':                          // +=
        case '+':                          // ++
          end_pos = start_pos + 2u;
          break;

        default:                           // +
          end_pos = start_pos + 1u;
        }
      break;

    case '-':
      switch (ch2)
        {
        case '=':                          // -=
        case '-':                          // --
          ident   = Identify_operator;
          end_pos = start_pos + 2u;
          break;

        case '>':                          // deprecated ->
          ident   = Identify_lexical_error;
          end_pos = start_pos + 2u;
          break;

        default:                           
          // Is it a number?
          if (((start_pos == 0u) || ms_is_space[uint8_t(cstr_a[start_pos - 1u])])
            && ((ch2 == '.') || ms_is_digit[uint8_t(ch2)]))
            {
            Args args(start_pos, ArgFlag__default_no_struct);

            parse_literal_number(args);
            ident = args.is_ok() ? Identify_number : Identify_lexical_error;
            end_pos = args.m_end_pos;
            }
          else                             // -
            {
            ident = Identify_operator;
            end_pos = start_pos + 1u;
            }
        }
      break;

    case '*':
      ident = Identify_operator;
      if (ch2 == '=')                      // *=
        {
        end_pos = start_pos + 2u;
        }
      else                                 // *
        {
        end_pos = start_pos + 1u;
        }
      break;

    case ':':
      ident = Identify_operator;
      if (ch2 == '=')                      // := assign
        {
        end_pos = start_pos + 2u;
        }
      else                                 // : bind or named argument arg:value
        {
        end_pos = start_pos + 1u;
        }
      break;

    case '~':
      switch (ch2)
        {
        case '=':                          // ~=
          ident   = Identify_operator;
          end_pos = start_pos + 2u;
          break;

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Deprecated operators
        case '&':                          // ~&
        case '|':                          // ~|
          ident   = Identify_lexical_error;
          end_pos = start_pos + 2u;
          break;

        default:                           // ~
          ident   = Identify_lexical_error;
          end_pos = start_pos + 1u;
        }
      break;

    case '!':
      // Check for C++ mistake/assumption to use !=
      if (ch2 == '=')                      // != used instead of ~=
        {
        ident   = Identify_lexical_error;
        end_pos = start_pos + 2u;
        }
      else                                 // !
        {
        // Might also be constructor
        ident   = Identify_operator;
        end_pos = start_pos + 1u;
        }
        break;

    case '?':
      if (ch2 == '?')  // ?? nil coalescing op
        {
        ident   = Identify_operator;
        end_pos = start_pos + 2u;
        }
      else
        {
        ident   = Identify_lexical_error;
        end_pos = start_pos + 1u;
        }
      break;

    default:    // Normal text or word operator
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it an operator?
      //if (ms_char_match_table[ACharMatch_token][uint8_t(ch)])
      //  {
      //  // It is an operator
      //  // - Also see 'word operator' form below
      //
      //  // $Revisit - CReis Could gather as much "operator" as possible, but easy now to just go a char at a time
      //  end_pos = start_pos + 1u;
      //  ident   = Identify_operator;
      //
      //  break;
      //  }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it an identifier?
      if (ms_is_lowercase[uint8_t(ch)])
        {
        bool predicate_b = false;

        end_pos = a_min(start_pos + 1u, length - 1u);

        if (parse_name_predicate(start_pos, &end_pos, nullptr, &predicate_b, false) != Result_ok)
          {
          ident = Identify_lexical_error;
          break;
          }

        if (!predicate_b)
          {
          uint32_t name_id = ASYMBOL_CSTR_TO_ID(cstr_a + start_pos, end_pos - start_pos);

          // Determine if it is a reserved word
          if (is_ident_reserved(name_id))
            {
            ident = Identify_reserved_word;
            break;
            }

          // Determine if it is an operator
          // Also see 'symbol operator' form above
          if ((cstr_a[end_pos] != '(') && ((start_pos == 0) || (cstr_a[start_pos - 1u] != '.'))
            && is_ident_operator(name_id))
            {
            ident = Identify_operator;
            break;
            }
          }

        // else normal text
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it a class?
      if (ms_is_uppercase[uint8_t(ch)])
        {
        ident = identify_class(start_pos, &end_pos);
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Is it a number?
      if (ms_is_digit[uint8_t(ch)])
        {
        Args args(start_pos, ArgFlag__default_no_struct);

        parse_literal_number(args);
        ident   = args.is_ok() ? Identify_number : Identify_lexical_error;
        end_pos = args.m_end_pos;
        break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // It is whitespace - normal text
      find(ACharMatch_not_white_space, 1u, &end_pos, start_pos);

      // $Revisit - CReis Just-in-case catch-all safety valve.
      if (end_pos == start_pos)
        {
        // It is *something* else unanticipated
        ident   = Identify_lexical_error;
        end_pos = a_min(start_pos + 1u, length);
        }
      }
      break;
    }  // switch

  if (end_pos_p)
    {
    *end_pos_p = end_pos;
    }

  return ident;
  }

//---------------------------------------------------------------------------------------
// Determines the identified class or instance member name.
// 
// [This method is different than other SkParser methods in that it assumes that the
// entire parser string is just the name to be identified.]
//             
// See Also  SkEditBox: :caret_context()
// #Author(s) Conan Reis
eSkMember SkParser::identify_member_filename(
  SkQualifier * ident_p,        // = nullptr
  bool *        class_member_p, // = nullptr
  bool          optional_scope  // = true
  ) const
  {
  // If it has a .sk extension it should conform unless it is prefixed with '-'
  eSkMember type    = SkMember__error;
  uint32_t  pos     = 0u;
  uint32_t  length  = m_str_ref_p->m_length;
  SkClass * class_p = nullptr;

  if ((!optional_scope || (parse_class_scope(0u, &pos, &class_p) == Result_ok))
    && (pos < length))
    {
    ASymbol      name;
    bool         class_member = false;
    const char * cstr_a       = m_str_ref_p->m_cstr_p;
    char         last_char    = cstr_a[length - 1u];
    char         ch;

    switch (cstr_a[pos])
      {
      case '_':
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Looks like a coroutine
        {
        uint32_t name_pos = pos;

        pos++;

        if ((pos < length) && ms_is_lowercase[uint8_t(cstr_a[pos])])
          {
          // Set pos past the last A-Z, a-z, _, 0-9, or European character
          find(ACharMatch_not_identifier, 1u, &pos, pos + 1u);

          if (cstr_a[pos] == '(')
            {
            type = SkMember_coroutine;
            }

          if ((type < SkMember__invalid) && (last_char != ')'))
            {
            // $Revisit - CReis This output should be an actual error code rather than a side-effect.

            A_DPRINT("\n    %s: Coroutine file names must end with '()'.\n", as_cstr());

            type = SkMember__error;
            }

          if (ident_p && (type < SkMember__invalid))
            {
            name = as_symbol(name_pos, pos);
            }
          }
        }
        break;

      case '-':  // Always ignore files that start with '-'
        type = SkMember__invalid;
        break;

      case '!':
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Looks like class meta info / data member / constructor/destructor method
        ch = ((pos + 1u) < length) ? cstr_a[pos + 1u] : '\0';

        // Using if rather than switch so break can still be used for enclosing switch
        if (ch == 'C')
          {
          // It must be a class meta info file
          type = SkMember_class_meta;
          break;
          }
        else
          {
          if (ch == 'D')
            {
            // It must be a data member
            // data-file-name = '!Data' ['C']

            class_member = (last_char == 'C') || (last_char == 'c');
            type         = SkMember_data;
            break;
            }
          }

        // Parse constructors and destructors
        if (parse_name_method(pos, nullptr, ident_p ? &name : nullptr) == Result_ok)
          {
          class_member = (last_char == 'C') || (last_char == 'c');
          type = (class_member || (last_char == ')')) ? SkMember_method : SkMember__error;
          }
        break;

      default:
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Try method file
        // method-file-name = method-name '()' ['C'] '.sk'

        class_member = (last_char == 'C') || (last_char == 'c');

        if (!class_member && (last_char != ')'))
          {
          // Invalid method member
          break;
          }

        type = SkMember_method;

        // Look for trailing "-Q" to represent "?" for predicate methods since "?" is an
        // illegal filename character.
        const uint32_t qmark_ioffset = 3u;  // length of () + 1
        const uint32_t qmark_coffset = 4u;  // length of ()C + 1
        uint32_t   qmark_offset  = class_member ? qmark_coffset : qmark_ioffset;

        if ((length > qmark_offset) 
          && ((cstr_a[length - qmark_offset] == 'Q') && (cstr_a[length - qmark_offset - 1u] == '-')))
          {
          // $Revisit - CReis Should use a temp stack buffer to avoid memory allocation.
          uint32_t predicate_length = length - qmark_offset;
          SkParser predicate_name(cstr_a, predicate_length, false);

          predicate_name.as_cstr_writable()[predicate_length - 1u] = '?';

          if (predicate_name.parse_name_method(pos, nullptr, ident_p ? &name : nullptr) != Result_ok)
            {
            type = SkMember__error;
            }
          }
        else
          {
          if (parse_name_method(pos, nullptr, ident_p ? &name : nullptr) != Result_ok)
            {
            type = SkMember__error;
            }
          }
        break;
      }

    if (type < SkMember__invalid)
      {
      if (ident_p)
        {
        ident_p->set_scope(class_p);
        ident_p->set_name(name);
        }

      if (class_member_p)
        {
        *class_member_p = class_member;
        }
      }
    }

  return type;
  }

//---------------------------------------------------------------------------------------
// Determines the identified member or class
// 
// Returns:
//   Parse result. Also sets `member_info_p->m_type = SkMember__error` if not successful.
//   
// Prams:
//   member_p:
//     Info structure to store member info. Possible `m_type` values will be:
//       SkMember_method
//       SkMember_coroutine
//       SkMember_data
//       SkMember_class_meta
//       SkMember__error
//   start_pos:
//     Character position to begin lexical analysis
//   end_pos_p:
//     Character position that lexical analysis stopped at.
//     If set to nullptr, it is not written to.
//   accept_to: accept just members up to SkMember_data or include classes SkMember_class_meta 
// 
// Notes:
//   It has the following syntax
//     member_name =
//       class-name |
//       ((class-name | meta-class)
//         ('.' data-name)
//         | ('.' | '@' method-name | coroutine-name ['()' 'C']))
//       
// See: identify_member_filename(), SkEditBox::caret_context()
SkParser::eResult SkParser::identify_member_name(
  SkMemberInfo * member_p,
  uint32_t       start_pos, // = 0u
  uint32_t *     end_pos_p, // = nullptr
  eSkMember      accept_to  // = SkMember_class_meta
  ) const
  {
  eResult      result;
  SkClass *    class_p = nullptr;
  uint32_t     pos     = start_pos;
  const char * cstr_a  = m_str_ref_p->m_cstr_p;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse class part
  member_p->m_type        = SkMember__error;
  member_p->m_class_scope = cstr_a[pos] == '<';

  if (member_p->m_class_scope)
    {
    // Meta class - so it is a member with a class scope
    SkMetaClass * mclass_p;

    result = parse_class_meta(pos, &pos, &mclass_p);

    if (result == Result_ok)
      {
      class_p = mclass_p->get_class_info();
      }
    }
  else
    {
    result = parse_class(pos, &pos, &class_p);
    }

  if (result == Result_ok)
    {
    uint32_t class_end = pos;
    member_p->m_member_id.set_scope(class_p);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse member name
    ASymbol name;
    char    ch = cstr_a[pos];

    result = Result_err_expected_scope_op;

    if ((ch == '.') && (cstr_a[pos + 1u] == '@'))
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Data member
      member_p->m_type = SkMember_data;

      result = parse_name_data_member(pos + 1u, &pos, &name, &member_p->m_class_scope);
      }
    else
      {
      if ((ch == '.') || (ch == '@'))
        {
        // Routine member
        pos++;

        if (cstr_a[pos] == '_')
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Coroutine
          member_p->m_type = SkMember_coroutine;
          result = parse_name_coroutine(pos, &pos, &name);
          }
        else
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Method
          member_p->m_type = SkMember_method;
          result = parse_name_method(pos, &pos, &name);
          }
        }
      else
        {
        if (accept_to == SkMember_class_meta)
          {
          member_p->m_type = SkMember_class_meta;

          if (end_pos_p)
            {
            *end_pos_p = class_end;
            }

          return Result_ok;
          }
        }

      // Skip end brackets on routine if present
      if ((cstr_a[pos] == '(') && (cstr_a[pos + 1u] == ')'))
        {
        pos += 2u;

        // Look for 'C' marking member as class scope
        if ((member_p->m_type == SkMember_method) && (cstr_a[pos] == 'C'))
          {
          pos++;
          member_p->m_class_scope = true;
          }
        }
      }

    member_p->m_member_id.set_name(name);
    }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (end_pos_p)
    {
    *end_pos_p = pos;
    }

  return result;
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Ensures that the specified expression is immediate or deferred as desired.
// 
// Params:
//   expr: expression to check
//   args: Arg struct from parse of expression
//   desired_exec_time: see eSkInvokeTime
//
// See: ensure_expr_effect()
bool SkParser::ensure_exec_time(
  const SkExpressionBase & expr,
  Args &                   args,
  eSkInvokeTime            desired_exec_time
  ) const
  {
  // $Vital - CReis This test needs to work even if an expression structure is not available
  if (desired_exec_time != SkInvokeTime_any)
    {
    uint32_t deferred_idx;

    if (expr.is_immediate(&deferred_idx))
      {
      if (desired_exec_time == SkInvokeTime_durational)
        {
        args.m_result = Result_err_context_deferred;

        return false;
        }
      }
    else
      {
      if (desired_exec_time == SkInvokeTime_immediate)
        {
        args.m_start_pos = deferred_idx;
        // $Revisit - CReis a bit of a hack - it will be at least 2 char long.
        args.m_end_pos   = deferred_idx + 2u;
        args.m_result    = Result_err_context_immediate;

        return false;
        }
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Returns a context based result string based on a the parsed code, a parse result, a
// result position, and a start of parse position.
// 
// Returns:   A context based result string - it will contain newlines (\n).
// Params:  
//   code:         string containing text that was parsed
//   result:       the result type of the earlier parse
//   result_pos:   index that the parse error occurred
//   result_start: start index of most recent parse element
//   start_pos:
//     index that parse initially started for code - if code only a substring
//     
// Examples:  
//   ```
//   AString SkParser::error_str(get_result_context_string(code, result, end_pos);
//   ```
//   
// See:   get_result_string()
// Notes:  
//   It creates a string that contains the a string representation of the parse result,
//   the parsed code 4 lines above and below the result position, and a text marker
//   pointing to the result position.
//   
// Modifiers:   static
// Author(s):   Conan Reis
AString SkParser::get_result_context_string(
  const AString & code,
  eResult         result,
  uint32_t        result_pos,
  uint32_t        result_start, // = ADef_uint32
  uint32_t        start_pos     // = 0u
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Fix up result position
  uint32_t snip_end = code.get_length();

  // Error can occur conceptually after the end of the code if the error is semantic rather than
  // lexical.
  if (result_pos >= snip_end && result_pos > 0)
    {
    // $Revisit - CReis This can make the error message ignore the last character - should fix though currently needed for find_reverse() below
    result_pos = snip_end - 1u;
    }

  if (result_start > result_pos)
    {
    result_start = result_pos;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t snip_start = start_pos;

  // 4 lines up from error
  if (code.find_reverse('\n', 4u, &snip_start, start_pos, result_pos))
    {
    if (snip_start)
      {
      // Skip newline
      snip_start++;
      }
    }

  // 4 lines down from error
  code.find('\n', 4u, &snip_end, result_pos);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create start of error snippet
  AString error_snippet(nullptr, SkParser_error_str_reserve_chars, uint32_t(0u));

  error_snippet.append((result == Result_ok) ? "INFO: " : ((result < Result_err__start) ? "WARNING: " : "ERROR: "));
  error_snippet.append(get_result_string(result));
  error_snippet.append("\n");
  error_snippet.append('v', 90u);
  error_snippet.append("\n");
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get snippet up to error position
  error_snippet.append(code.as_cstr() + snip_start, result_pos - snip_start);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Continue code copy up to end of error line or end of this error string, whichever comes first
  uint32_t line_end      = snip_end - 1u;  // if not found use snip_end
  bool     found_newline = code.find('\n', 1u, &line_end, result_pos);

  if (line_end + 1u - result_pos)
    {
    error_snippet.append(code.as_cstr() + result_pos, line_end + 1u - result_pos);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine line and column - both line and column are 0-based until they are
  // printed and then they are 1-based
  uint32_t row_idx;
  uint32_t column;
  uint32_t line =
    code.index_to_row(result_pos, &row_idx, &column, SkDebug::ms_tab_stops);

  uint32_t column_start =
    code.index_to_column(a_max(result_start, result_pos - row_idx), SkDebug::ms_tab_stops);

  uint32_t column_count = column - column_start;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add marker on following line
  if (!found_newline)
    {
    // If line had its own \n, this is not needed
    error_snippet.append('\n');
    }

  // Add initial stream of >>>>>>
  if (column_start >= 2)
    {
    error_snippet.append('>', column_start - 1u);
    }

  // Add extra space
  if (column_start >= 1)
    {
    error_snippet.append(' ');
    }

  // Add parse element mark characters ------
  if (column_count >= 2)
    {
    error_snippet.append('-', column_count - 1u);
    }

  // Ensure enough additional space
  error_snippet.ensure_size(error_snippet.get_length() + 256u);

  // Add error mark character ^ and line & column - 1 based when displayed to user
  if (column_start == column)
    {
    error_snippet.append_format("^ <<<< Line: %u, column: %u\n", line + 1u, column + 1u);
    }
  else
    {
    error_snippet.append_format(
      "^ <<<< Line: %u, columns: %u-%u\n", line + 1u, column_start + 1u, column + 1u);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy the rest of the code line
  if (found_newline)
    {
    error_snippet.append(code.as_cstr() + line_end + 1u, snip_end - (line_end + 1u));
    }

  // $Vital - CReis Ensure that indexes are correct
  error_snippet.line_break_dos2unix();

  error_snippet.append("\n");
  error_snippet.append('^', 90u);
  error_snippet.append("\n");

  return error_snippet;
  }

//---------------------------------------------------------------------------------------
//  Returns a string based on a parse result.
// Returns:     A string based on a parse result.  It may have newlines (\n) in it.
// Arg          result - the result type of an earlier parse
// See:         get_result_context_string()
// Notes:       AString result_str(parser.get_result_string(result);
// Modifiers:    static
// Author(s):    Conan Reis
AString SkParser::get_result_string(eResult result)
  {
  // $Revisit - CReis [Error Info] These should be put into an array of strings.

  switch (result)
    {
    case Result_ok:
      return "The text was parsed without error.";

    case Result_warn_ident_too_long:
      return "Identifier name is longer than the maximum of 255 characters.";
    case Result_warn_expr_no_effect:
      return "The expression has no side effects so it does not seem useful as a statement "
             "on its own [not used as an argument or result (last expression) of a code block].";
    case Result_warn_expr_sub_effect:
      return "The expression has only sub-expressions with side effects - it does not seem "
             "useful as a statement on its own [not used as an argument or result (last "
             "expression) of a code block].";

    case Result_err_size_group_param:
      return AString(256u, "The group parameter descriptor contained too many classes.\nThere may not be more than %i classes in the pattern.", SkGroupParam_max_class_count);
    case Result_err_context_invoke_arg_end:
      return "Expected the end of the invocation list ')', but did not find it.\n[Too many arguments supplied?]";
    case Result_err_context_invoke_arg_unnamed:
      return "Once a named argument is used, any following arguments must also be named.";
    case Result_err_context_duped_param_name:
      return "Argument with the same name already present in the parameter list.";
    case Result_err_context_duped_rparam_name:
      return "Argument with the same name already present in the return parameter list.";
    case Result_err_expected_cast_op:
      return "Expected the class cast operator '<>', but it was not found.";
    case Result_err_typecheck_invoke_apply_recv:
      return "Cannot do an invoke apply [receiver%invocation()] on a receiver that is guaranteed to be nil.";
    case Result_err_typecheck_list:
      return "Expected a List class or subclass, but given a non-list class.";
    case Result_err_typecheck_closure_generics:
      return "Generic types are not supported in closure parameter lists and might never be - too many levels of indirection to wrap your head around";
    case Result_err_expected_clause_block:
      return "Expected a clause code block [ ], but did not receive one.";
    case Result_err_expected_closure:
      return "A non-inline closure must start with either a caret/hat symbol '^' or an interface ().";
    case Result_err_expected_conversion_op:
      return "Expected the class conversion operator '>>', but it was not found.";
    case Result_err_expected_exponent:
      return "Expected a digit for the exponent, but did not receive one.";
    case Result_err_expected_digit_radix:
      return "Expected a radix/base appropriate digit to follow the integer literal radix prefix.";
    case Result_err_expected_invoke_cascade:
      return "Expected cascaded invocations - i.e. a receiver followed by '.' then '[' then two or more invocations and ending in ']' and did not find any.";
    case Result_err_expected_invoke_cascades:
      return "Expected cascaded invocations - i.e. a receiver followed '.' then '[' then two or more invocations and ending in ']' and only found one invocation.\n"
             "[If only one invocation is desired, then just use '.' - i.e. receiver.invoke()].\n";
    case Result_err_expected_invoke_selector:
      return "Expected an invocation selector - i.e. a method call or coroutine call, "
             "but found neither.";
    case Result_err_expected_invoke_select_op:
      return "Expected an invocation selector - i.e. a method call, an operator call "
             "or coroutine call, but found neither.";
    case Result_err_expected_loop_block:
      return "Expected a loop code block [ ], but did not receive one.";
    case Result_err_expected_race_block:
      return "Expected a 'race' code block [ ], but did not find one.";
    case Result_err_expected_return_arg:
      return "Expected a return argument, but did not receive one.";
    case Result_err_expected_named_arg:
      return "Expected a named argument specifier (arg_name:) and did not find one.";
    case Result_err_expected_param_name:
      return "Parameter specifiers must be named and no name was found.\n"
             "If you were trying to group expressions using ( ), use square brackets [ ] instead.";
    case Result_err_expected_scope_op:
      return "Expected a scope resolution operator to follow the given class scope.";
    case Result_err_expected_statement_modifier:
      return "Expected a statement modifier and did not find one.";
    case Result_err_expected_annotation_arg:
      return "Expected required annotation argument and did not find one.";
    case Result_err_expected_binding:
      return "A binding must begin with a colon ':'.";
    case Result_err_context_actor_class_unknown:
      return "Could not determine actor class from project settings - is the proper project loaded?";
    case Result_err_context_annotation_unknown:
      return "Unknown annotation";
    case Result_err_context_annotation_invalid:
      return "Annotation is not allowed in this context";
    case Result_err_context_annotation_duplicate:
      return "Duplicate annotation";
    case Result_err_context_conversion_params:
      return "A conversion method may not have any parameters [this may change in the future].";
    case Result_err_expected_char:
      return "A character escape sequence must begin with a backslash character '\\'.";
    case Result_err_expected_char_number:
      return "A character escape sequence that uses a number must have ASCII value between 0 and 255.";
    case Result_err_expected_class:
      return "Class name must begin with an uppercase letter.";
    case Result_err_unexpected_class_class:
      return "The metaclass '<Object>' must be used instead of the class instance 'Class'.";
    case Result_err_expected_class_desc:
      return "Expected class, list-class, invoke class, metaclass or class union and did not find one.";
    case Result_err_expected_class_instance:
      return "Expected a class, list-class or invoke class and did not find one.";
    case Result_err_expected_class_list_end:
      return "A List class descriptor must end with a closing brace/curly bracket '}'.";
    case Result_err_expected_class_meta:
      return "A metaclass descriptor must begin with an angle bracket '<'.";
    case Result_err_expected_class_meta_end:
      return "A metaclass descriptor must end with a closing angle bracket '>'.";
    case Result_err_expected_class_params:
      return "Expected a parameter list following the name of an invokable class.";
    case Result_err_expected_class_union:
      return "A class union descriptor must begin with an opening angle bracket '<'.";
    case Result_err_expected_class_union_end:
      return "A class union descriptor must end with a closing angle bracket '>'.";
    case Result_err_size_class_union:
      return "A class union descriptor must union two or more classes.";
    case Result_err_typecheck_union_trivial:
      return "This class union descriptor is trivial.\nIt is lexically correct, but it can be represented more simply as a single class instance or metaclass.";
    case Result_err_expected_code_block:
      return "A code block must start with an opening square bracket '['.";
    case Result_err_typecheck_test:
      return "The result type of a test expression for an if/when/unless must be a Boolean class.";
    case Result_err_typecheck_ue4_blueprint_param:
      return "A routine annotated with &blueprint cannot take a parameter or return a value that is a Blueprint-generated class. A possible resolution is to use its parent class instead.";
    case Result_err_typecheck_conversion:
      return "The result type of a conversion method must be of the same type as or a subclass of the method name.";
    case Result_err_expected_data_defn:
      return "A data definition statement must start with an exclamation mark '!'.";
    case Result_err_expected_data_name:
      return "A data member name must start with '@' for instance data and '@@' for class data followed by a lowercase letter.";
    case Result_err_expected_expression:
      return "Expected an expression, but did not find one.";
    case Result_err_expected_instance:
      return "Instance name must begin with a lowercase letter.";
    case Result_err_expected_invoke_apply:
      return "An invocation apply must begin with a percent sign '%' character.";
    case Result_err_expected_invoke_args:
      return "This invocation argument list must begin with an opening parenthesis / bracket '('.  [Parentheses are optional for invocation calls that have a trailing closure argument and for constructors that may have zero arguments.]";
    case Result_err_expected_literal_char:
      return "A character literal must begin with an accent [`] character - the one beneath the tilde '~'.";
    case Result_err_expected_literal_int:
      return "An integer literal must begin with a minus sign '-' or a digit '0-9'";
    case Result_err_expected_literal_list:
      return "Expected a List literal opening brace/curly bracket '{', but did not receive one.";
    case Result_err_expected_literal_real:
      return "A real literal must begin with a minus sign '-', a digit '0-9' or a decimal '.'.";
    case Result_err_expected_literal_real_sgnf:
      return "A real literal must begin with a significand (integer part and / or fractional part) and did not find one.";
    case Result_err_expected_literal_real_end:
      return "While parsing a real number, found integer part though also expected a fractional part ('.' {digit}1+), an exponent part ('E' | 'e' ['-'] digits), or both, but received neither.";
    case Result_err_expected_literal_string:
      return "A string literal must begin with a double quote [\"]";
    case Result_err_expected_literal_symbol:
      return "A symbol literal must begin with a single quote ['].";
    case Result_err_expected_literal_symbol_end:
      return "A symbol literal must end with a single quote ['].";
    case Result_err_expected_loop_exit:
      return "A loop exit must begin with 'exit'.";
    case Result_err_expected_operator:
      return "Expected an operator method call, but did not find one.";
    case Result_err_expected_method_ctor_name:
      return "A constructor method name must begin with an exclamation mark '!' and be optionally followed by an identifier starting with a lowercase letter.";
    case Result_err_expected_method_name:
      return "A method name must begin with a lowercase letter or an exclamation mark '!'";
    case Result_err_expected_mind:
      return "Expected an expression of type Mind.";
    case Result_err_expected_obj_id:
      return "Expected an operator id, but did not find the '@', '@?' or '@#' symbols.";
    case Result_err_expected_op_index_end:
      return "Expected index operator ending curly bracket/brace `}` and did not find one.";
    case Result_err_expected_parameters:
      return "A parameter list must start with an opening parenthesis (bracket) '('.";
    case Result_err_expected_parameter:
      return "The parameter list expected a parameter and did not find one.";
    case Result_err_expected_parameter_next:
      return "The parameter list expected a parameter or end of the list.";
    case Result_err_typecheck_scope:
      return "The specified class scope qualifier is not the same class or a superclass of the class of the receiver expression (or implied 'this').  Note that a NilClass may only have a scope qualifier of 'Object'.";
    case Result_err_expected_coroutine_name:
      return "A coroutine name must begin with an underscore '_' and then a lowercase letter.";
    case Result_err_expected_temporary:
      return "A create temporary variable statement must start with an exclamation mark '!'.";
    case Result_err_expected_sync_block:
      return "Expected a 'sync' code block [ ], but did not find one.";
    case Result_err_expected_whitespace:
      return "Whitespace required - expected some combination of whitespace characters and/or comments.";
    case Result_err_expected_group_param:
      return "A group parameter specification must begin with an opening brace '{'.";
    case Result_err_expected_comment_close:
      return "Multiple line comment missing closing delimiters '*""/' .";
    case Result_err_expected_string_close:
      return "String literal missing closing double quotation mark '\"'.";
    case Result_err_expected_symbol_close:
      return "String literal missing closing quotation mark (').";
    case Result_err_expected_block:
      return "Expected a code block [ ], but did not find one.";
    case Result_err_context_non_class:
      return "A class with the specified name does not exist - ensure that it is registered prior to this parse.";
    case Result_err_context_immediate:
      return "A deferred statement (such as a coroutine) was found where an immediate statement (such as a method) was expected.";
    case Result_err_context_deferred:
      return "An immediate statement (such as a method) was found where a deferred statement (such as a coroutine) was expected.";
    case Result_err_context_concurrent_redundant:
      return "A concurrent block (sync or race) must have at least two durational expressions or running concurrently is redundant.";
    case Result_err_context_side_effect:
      return "Expression has side effect but none allowed.";
    case Result_err_context_last_no_side_effect:
      return "The expression (or the last expression within this code block) has no effect.";
    case Result_err_context_raw_access:
      return "Direct use of raw data member not allowed here. In this context, a raw data member must be copied before it can be used, e.g. by appending '!' (exclamation mark) to the raw data member.";
    case Result_err_size_radix_small:
      return "Radix too small - it must be between 2 and 36 inclusively.";
    case Result_err_size_radix_large:
      return "Radix too large - it must be between 2 and 36 inclusively.";
    case Result_err_size_identifier:
      return "An identifier may be no more than 255 characters long.";
    case Result_err_size_symbol:
      return "A symbol literal may be no more than 255 characters long.";
    case Result_err_size_uint16_out_of_range:
      return "Value must be between 0 and 65535.";
    case Result_err_unexpected_else:
      return "An else / default clause may not be the sole clause - there must be at least one more prior to it.";
    case Result_err_unexpected_else_statement:
      return "Found an 'else' without a matching 'if' or 'case'.";
    case Result_err_unexpected_exit:
      return "Found a loop exit in an invalid location.";
    case Result_err_unexpected_parameter_rargs:
      return "The parameter list did not expect an extra semi-colon ';'!  Return parameters already started.";
    case Result_err_unexpected_parameters_result:
      return "A coroutine parameter list must not specify a primary return type - the return type InvokedCoroutine is always inferred.";
    case Result_err_unexpected_query_identifier:
      return "Query/predicate methods are not permitted in instantiation invocations.";
    case Result_err_unexpected_return_args:
      return "Invocation argument list indicated that return arguments were to be used, but routine does not have return parameters.";
    case Result_err_unexpected_statement:
      return "The code block expected another statement or the end of the code block ']'.";
    case Result_err_unexpected_unless_statement:
      return "Found an 'unless' expression modifier without an expression to modify.\n"
             "Note that it may not follow a *statement* such as !var or exit without them being wrapped "
             "by a code block to make them an expression - like [exit].";
    case Result_err_unexpected_when_statement:
      return "Found a 'when' expression modifier without an expression to modify.\n"
             "Note that it may not follow a *statement* such as !var or exit without them being wrapped "
             "by a code block to make them an expression - like [exit].";
    case Result_err_unexpected_bind_expr:
      return "A variable rebind to an instance may only be applied to an identifier.";
    case Result_err_unexpected_bind_expr_raw:
      return "Tried to bind an instance to a raw data member. This is not possible as raw data members cannot store instances. Did you mean to use an assignment ':=' instead?";
    case Result_err_unexpected_bind_expr_captured:
      return "Tried to bind an instance to a captured variable. We don't allow this since captured variables are internal copies of the original variables, so binding to them would not affect the original variable, thus creating unexpected behavior. You can, however, assign ':=' something to a captured variable.";
    case Result_err_unexpected_branch_expr:
      return "A concurrent branch only makes sense when used on an expression that is not immediate "
             "and may take more than one frame to execute such as a coroutine call.";
    case Result_err_unexpected_class_pattern:
      return "Group parameter descriptor expected a class name or '}', but neither were found.";
    case Result_err_unexpected_cdtor:
      return "While parsing for a 'create temporary variable statement', a constructor or a destructor call was found instead.";
    case Result_err_unexpected_char:
      return "Expected a particular character or type of character, but did not receive it.";
    case Result_err_unexpected_eof:
      return "Hit end of file prior to the completion of a parse.\n"
             "[Mismatched brackets [] {} ()?]";
    case Result_err_unexpected_implicit_this:
      return "Operator calls may not be used with an implicit 'this' - otherwise it is more error "
             "prone and even when used correctly code is more difficult to understand.";

    default:
      #if defined(SK_AS_STRINGS)
        return ms_error_str;
      #else
        return a_cstr_format(
          "Parse error: #%u.\n[Use a build with additional error checking (SK_CODE_OUT defined) for more context.]",
          result);
      #endif
    }
  }

//---------------------------------------------------------------------------------------
// Parse script string as a code block on the supplied object and execute it.
// *** This method is intended as a handy "execute script text" for "console" tools
// rather than as an efficient runtime mechanism.  Pre-compiled script binaries should be
// used in production/final/end-user code.***
//
// #Modifiers static
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok                      - parsed successfully and invoked with immediate completion
  // Result_ok_deferred             - parsed successfully and invoked with deferred completion
  // Result_warn_scripts_disabled   - could not parse, scripts disabled
  // Result_warn_empty_script_block - code was empty 
  // *or* some other result if not a successful parse.  See SSparser::eResult
  SkParser::eResult
SkParser::invoke_script(
  // Script code to evaluate.  It can be any number of statements and will be treated as
  // the body of a code block [...].
  const AString & code,
  // Address to store string version of result if Result_ok is returned.  It is ignored if
  // it is nullptr.  Also see print_info.
  AString * result_str_p, // = nullptr
  // Address to store pointer to resulting object instance.  It already will have a
  // reference count on it so dereference() must be called on it when it is no longer
  // needed.  Ignored if nullptr.
  SkInstance ** result_pp, // = nullptr
  // Object instance to use as scope/receiver for the statements to execute on.  If it is
  // nullptr, the default "master mind" object is used.
  SkInstance * instance_p, // = nullptr
  // Print/log information related to executing these statements.  The string form of the
  // result is printed if print_info is true and result_str_p is nullptr.
  bool print_info // = true
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // In evaluation mode?
  if (SkookumScript::get_initialization_level() < SkookumScript::InitializationLevel_sim)
    {
    if (print_info)
      {
      SkDebug::print_agog("\nCannot execute command - evaluation of scripts not enabled.\n[Toggle 'Evaluate Scripts']\n");
      }

    return Result_warn_scripts_disabled;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap code block around code statements to treat it as single element
  uint32_t length = code.get_length();

  if (length == 0u)
    {
    // Nothing to evaluate
    return Result_warn_empty_script_block;
    }

  if (instance_p == nullptr)
    {
    instance_p = SkookumScript::get_master_mind_or_meta_class();
    }

  SkClass * class_p = instance_p->get_class();
  SkParser  parser(AString(nullptr, length + 6u, 0u));

  parser.reset_scope(class_p, ASymbol_invoke_script_);

  // $Revisit - CReis Should probably make a mechanism to parse without adding fake parameters and block characters
  parser.append("()[", 3u);
  parser.append(code);
  parser.append("\n]", 2u);  // Must be on the next line to ensure that it is not commented out with single line comment

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse code
  SkParser::Args args;
  SkMethod *     method_p = static_cast<SkMethod *>(parser.parse_method(args));

  if (!method_p)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Display Parse Error

    if (print_info)
      {
      // 3 chars are subtracted to account for "()[" which was added
      SkDebug::print_parse_error(args.m_result, ms_empty, &code, args.m_end_pos - 3u, args.m_start_pos - 3u);
      }

    return args.m_result;
    }

  method_p->set_name(ASymbol_invoke_script_);
  method_p->set_scope(class_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Keep method around and delete later, as it may contain `branch` statements that require the method to stay valid while the branched code is running
  SkClassUpdateRecord * class_update_record_p = SkookumScript::get_program_update_record()->get_or_create_class_update_record(class_p->get_name());
  SkRoutineUpdateRecord * routine_update_record_p = new SkRoutineUpdateRecord;
  routine_update_record_p->m_previous_routine_p = method_p;
  class_update_record_p->m_updated_routines.append(*routine_update_record_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // It is a valid method so invoke it

  SkInstance * result_p      = nullptr;
  bool         result_wanted = (print_info || result_str_p || result_pp);

  SkInvokedDeferrableMethod * imethod_p = new SkInvokedDeferrableMethod(instance_p, method_p);

  SkookumScript::update_time();

  SKDEBUG_HOOK_SCRIPT_ENTRY(ASymbol_origin_parser_interpreted);
  SkInvokedBase * deferred_p = imethod_p->invoke_deferred(result_wanted ? &result_p : nullptr);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Was the completion deferred?
  if (deferred_p)
    {
    if (print_info)
      {
      SkDebug::print(a_str_format(
        "\n[Running code in background until completed - invoked method 0x%p]\n",
        imethod_p));
      }

    if (result_p)
      {
      if (result_pp)
        {
        *result_pp = result_p;
        }
      else
        {
        result_p->dereference();
        }
      }

    SKDEBUG_HOOK_SCRIPT_EXIT();

    return Result_ok_deferred;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Completed immediately

  delete imethod_p;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Show results of evaluation
  if (result_wanted)
    {
    AString result_str;

    if (result_p && (result_str_p || print_info))
      {
      if (result_p->get_class() == SkBrain::ms_invoked_coroutine_class_p)
        {
        SkInvokedCoroutine * icoro_p = result_p->as_data<SkInvokedCoroutine>();

        if (icoro_p)
          {
          #if (SKOOKUM & SK_DEBUG)
            result_str = icoro_p->as_string_debug();
          #else
            result_str.append("InvokedCoroutine[running]");
          #endif
          }
        else
          {
          result_str.append("InvokedCoroutine[stale - completed immediately]");
          }
        }
      else
        {
        result_str = result_p->as_code(true);
        }
      }

    if (result_str_p)
      {
      *result_str_p = result_str;
      }
    else
      {
      if (print_info)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Add the string version of the result to the log
        
        AString log_str(nullptr, result_str.get_length() + 3u, 0u);

        log_str.append('\n');
        log_str.append(result_str);
        log_str.append('\n');
        SkDebug::print(log_str, SkLocale_all, SkDPrintType_result);
        }
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (result_p)
      {
      if (result_pp)
        {
        *result_pp = result_p;
        }
      else
        {
        // Garbage collect result instance
        result_p->dereference();
        }
      }
    }

  SKDEBUG_HOOK_SCRIPT_EXIT();

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Determines if symbol id represents an operator word:
//   and, or, xor, nand, nor, nxor, not
//
// #Modifiers static
// #See Also  is_ident_reserved()
// #Author(s) Conan Reis
bool SkParser::is_ident_operator(uint32_t sym_id)
  {
  switch (sym_id)
    {
    case ASymbolId_and:
    case ASymbolId_nand:
    case ASymbolId_nor:
    case ASymbolId_not:
    case ASymbolId_nxor:
    case ASymbolId_or:
    case ASymbolId_xor:
      return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determines if symbol id represents a reserved word:
//   primitives  - if, case, when, unless, else, loop, sync, race, branch, change, [rush]
//   statements  - exit, [skip]
//   identifiers - this, this_class, this_code, nil
//   literals    - true, false
//
// #Modifiers static
// #See Also  is_ident_reserved()
// #Author(s) Conan Reis
bool SkParser::is_ident_reserved(uint32_t sym_id)
  {
  switch (sym_id)
    {
    case ASymbolId_branch:
    case ASymbolId_case:
    case ASymbolId_change:
    case ASymbolId_defer:   // Planned
    case ASymbolId_else:
    case ASymbolId_exit:
    case ASymbolId_false:
    case ASymbolId_if:
    case ASymbolId_loop:
    case ASymbolId_nil:
    case ASymbolId_race:
    case ASymbolId_random:  // Planned
    case ASymbolId_rush:    // Planned
    case ASymbolId_skip:    // Planned
    case ASymbolId_sync:
    case ASymbolId_this:
    case ASymbolId_this_class:
    case ASymbolId_this_code:
    case ASymbolId_this_mind:
    case ASymbolId_true:
    case ASymbolId_unless:
    case ASymbolId_when:
      return true;
    }

  return false;
  }


#endif //(SKOOKUM & SK_CODE_IN)



#ifdef SK_CODE

//---------------------------------------------------------------------------------------
// Converts method name symbol to the associated operator symbol if one exists.
// 
// Returns:
//   Operator symbol or null if 'method_name' does not have an operator symbol
//   associated with it.
//   
// Params:
//   method_name: method name symbol to convert
//   
// Modifiers: static
// Author(s): Conan Reis
ASymbol SkParser::method_to_operator(const ASymbol & method_name)
  {
  return ms_method_to_operator_p->method_to_operator(method_name);
  }

#endif // SK_CODE


#if (SKOOKUM & SK_CODE_IN)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Completes partial name parse starting at '*end_pos_p' attempting to
//             create a symbol name starting at 'start_pos'.
// Arg         start_pos - starting character position of symbol name
// Arg         end_pos_p - pointer to current parse position and place to store position
//             that lexical analysis stopped at.
// Arg         name_p - pointer to location to store parsed instance name.  It is not
//             written to if it is set to nullptr.
// Notes:      {alphanumeric}
//             alphanumeric = uppercase | lowercase | digit | '_'
// Author(s):   Conan Reis
void SkParser::parse_name_symbol(
  uint32_t  start_pos,
  uint32_t *    end_pos_p,
  ASymbol * name_p
  ) const
  {
  uint32_t pos = *end_pos_p;  // Set pos to current position
  
  *end_pos_p = m_str_ref_p->m_length;
      
  // Set pos past the last A-Z, a-z, _, 0-9, or European character
  find(ACharMatch_not_identifier, 1u, end_pos_p, pos);

  // $Revisit - CReis [Lexical Check] Give warning if name too long.
  //result = ((pos - start_pos) <= SkParser_ident_length_max) ? Result_ok : Result_warn_ident_too_long;

  if (name_p)
    {
    *name_p = as_symbol(start_pos, *end_pos_p);
    }
  }

//---------------------------------------------------------------------------------------
// Parse data member name symbol starting at 'start_pos' - similar to variable-name but
// starts with @ or @@.
// 
// #Notes
//   data-name      = '@' | '@@' variable-name
//   variable-name  = name-predicate
//   name-predicate = lowercase {alphanumeric} ['?']
//   alphanumeric   = uppercase | lowercase | digit | '_'
//   
// #See Also  parse_name_predicate()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_data_name, or Result_warn_ident_too_long 
  SkParser::eResult
SkParser::parse_name_data_member(
  // Starting index of name.
  uint32_t start_pos,
  // Address to store where lexical analysis stopped at.
  uint32_t * end_pos_p,
  // Address to store parsed instance name.  Not written to if nullptr.
  ASymbol * name_p,
  // Address to store whether name is a predicate/query.  Not written to if nullptr.
  bool * predicate_p, // = nullptr
  // Address to store whether name is a @instance/@@class member.  Not written to if nullptr.
  bool * class_member_p // = nullptr
  ) const
  {
  uint32_t pos    = start_pos;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Must start with '@'
  if (cstr_a[pos] != '@')
    {
    return Result_err_expected_data_name;
    }

  pos++;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optional second '@' indicating it is a class data member.
  bool class_member_b = false;

  if (cstr_a[pos] == '@')
    {
    class_member_b = true;
    pos++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // First character after '@'/'@@' must start with lowercase, digit or underscore
  if (!ms_is_lowercase[uint8_t(cstr_a[pos])] 
   && !ms_is_digit[uint8_t(cstr_a[pos])] // Allow to start with digit for Blueprint variable compatibility's sake
   && cstr_a[pos] != '_')       // Allow to start with underscore for Blueprint variable compatibility's sake
    {
    *end_pos_p = pos;
    return Result_err_expected_data_name;
    }

  pos++;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find last A-Z, a-z, _, 0-9, or European character + optional '?'
  // [First char was already checked so skip]
  if (!find(ACharMatch_not_identifier, 1u, &pos, pos))
    {
    pos = m_str_ref_p->m_length;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if it is a predicate/query name (and not nil coalescing op ??)
  bool predicate = false;

  if ((cstr_a[pos] == '?') && (cstr_a[pos + 1u] != '?'))
    {
    pos++;
    predicate = true;
    }

  *end_pos_p = pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure name not too long.
  uint32_t name_length = pos - start_pos;

  if (name_length > SkParser_ident_length_max)
    {
    return Result_warn_ident_too_long;
    }

  // Get symbol version of name
  if (name_p)
    {
    *name_p = as_symbol(start_pos, pos);
    }

  if (predicate_p)
    {
    *predicate_p = predicate;
    }

  if (class_member_p)
    {
    *class_member_p = class_member_b;
    }

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Completes partial name parse starting at '*end_pos_p' attempting to create a symbol
// name starting at 'start_pos'.
// 
// #Notes
//   name-predicate = lowercase {alphanumeric} ['?']
//   alphanumeric   = uppercase | lowercase | digit | '_'
//   
// #See Also  parse_name_data_member()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Result_ok, Result_err_expected_instance, Result_warn_ident_too_long,
  // Result_err_unexpected_reserved
  SkParser::eResult
SkParser::parse_name_predicate(
  // Starting index of name.
  uint32_t start_pos,
  // Address to store where lexical analysis stopped at. Initial value must be set to
  // already parsed to index.
  uint32_t * end_pos_p,
  // Address to store parsed instance name. Must be passed in and set to start_pos if no
  // chars parsed yet or start_pos + n for however many initial chars have already been
  // parsed.
  ASymbol * name_p,
  // Address to store whether name is a predicate/query. Not written to if nullptr.
  bool * predicate_p, // = nullptr
  // Ensures not a reserved word if set to true.
  bool test_resevered // = true
  ) const
  {
  uint32_t pos    = *end_pos_p;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  if (*end_pos_p == start_pos)
    {
    // If not already parsed, ensure first character starts with a lowercase letter
    if (!ms_is_lowercase[uint8_t(cstr_a[start_pos])])
      {
      return Result_err_expected_instance;
      }

    pos++;
    }

  // Find last A-Z, a-z, _, 0-9, or European character + optional '?'
  // [First char was already checked so skip]
  if (!find(ACharMatch_not_identifier, 1u, &pos, pos))
    {
    pos = m_str_ref_p->m_length;
    }

  // Determine if it is a predicate/query name (and not a nil coalescing op ??)
  bool predicate = false;

  if ((cstr_a[pos] == '?') && (cstr_a[pos + 1u] != '?'))
    {
    pos++;
    predicate = true;
    }

  *end_pos_p = pos;

  // Ensure name not too long.
  uint32_t name_length = pos - start_pos;

  if (name_length > SkParser_ident_length_max)
    {
    return Result_warn_ident_too_long;
    }

  // Get symbol version of name
  if (name_p)
    {
    *name_p = as_symbol(start_pos, pos);
    }

  // Ensure name not reserved word
  if (test_resevered && !predicate)
    {
    uint32_t name_id = name_p
      ? name_p->get_id()
      : ASYMBOL_CSTR_TO_ID(cstr_a + start_pos, name_length);

    if (is_ident_reserved(name_id))
      {
      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The reserved word '%s' cannot be used as a variable identifier.\n\n"
          "SkookumScript reserved words/tokens include:\n"
          "  primitives - if, case, when, unless, else, loop, sync, race, branch, change, [rush]\n"
          "  statements - exit, [skip]\n"
          "  identifiers - this, this_class, this_code, this_mind, nil\n"
          "  literals - true, false",
          get(start_pos, name_length).as_cstr());
      #endif

      return Result_err_unexpected_reserved;
      }
    }

  if (predicate_p)
    {
    *predicate_p = predicate;
    }

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a change mind expression.
//
// #Returns
//   If parse valid and `args.is_struct_wanted() == true` it returns a dynamically
//   allocated `SkChangeMind` data-structure otherwise `nullptr` is returned.
//
// #Params
//   args: see `SkParser::Args`, specific considerations below:
//     m_result:
//       Result_ok, Result_err_unexpected_eof, Result_err_expected_loop_block,
//       or pretty much any other warning or error.
//
// #Notes
//   change-mind = 'change' ws expression ws expression
//
// #Authors Conan Reis
SkChangeMind * SkParser::parse_change_mind(Args & args) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Advance past whitespace
  if (!parse_ws_any(args))
    {
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse optional mind expression

  // Store previous desired type
  SkClassDescBase * desired_type_p = args.m_desired_type_p;

  args.m_desired_type_p = SkBrain::ms_mind_class_p;
  
  SkExpressionBase * mind_p = parse_expression(args);

  if (!args.is_ok())
    {
    return nullptr;
    }

  // Determine if the result type of the expression is a Mind.
  if (!args.m_expr_type->is_class_type(SkBrain::ms_mind_class_p))
    {
    // It's not a Mind
    args.m_result = Result_err_expected_mind;

    delete mind_p;
    return nullptr;
    }

  // Restore previous desired type
  args.m_desired_type_p = desired_type_p;

  args.m_start_pos = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Advance past whitespace
  if (!parse_ws_any(args))
    {
    delete mind_p;
    return nullptr;
    }

  args.m_start_pos = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for expression to give new updater mind
  // $Note - CReis Could ensure that expression is durational, but it should work
  // with branched code too which is immediate.
  SkExpressionBase * expr_p = parse_expression(args);

  // If error, or if no struct wanted, bail here
  if (!args.is_ok() || expr_p == nullptr)
    {
    delete mind_p;
    return nullptr;
    }

  SK_ASSERTX(args.is_struct_wanted(), "The above test should ensure a struct is wanted.")

  // Create structure
  return SK_NEW(SkChangeMind)(mind_p, expr_p);
  }

//---------------------------------------------------------------------------------------
// Parse tail part [after optional class name] of validated object ID expression.
//
// #Notes
//   object-id = [class-name] '@' ['?' | '#'] symbol-literal
//
// #Author(s) Conan Reis
SkObjectID * SkParser::parse_object_id_tail(
  // Result_ok, Result_err_expected_obj_id, Result_err_context_object_id_invalid, or
  // errors from sub-parts of the expression.
  Args & args,
  // Class to use for object look-up.  If it is nullptr Actor is used by default.
  SkClass * class_p // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse '@'
  uint32_t pos    = args.m_start_pos;
  uint32_t length = m_str_ref_p->m_length;
  char *   cstr_a = m_str_ref_p->m_cstr_p;

  if ((pos >= length) || (cstr_a[pos] != '@'))
    {
    args.m_result = Result_err_expected_obj_id;
    return nullptr;
    }

  pos++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Infer Class part if not already supplied

  if (class_p == nullptr)
    {
    // Implicitly set class to expected type if available or `Actor`
    SkClass * desired_class_p = args.m_desired_type_p
      ? args.m_desired_type_p->get_key_class()
      : nullptr;

    class_p = desired_class_p
      ? desired_class_p
      : SkBrain::get_class_actor();

    // If class still null at this point, SkBrain::get_class_actor() must have returned nullptr
    if (!class_p)
      {
      args.m_result = Result_err_context_actor_class_unknown;
      return nullptr;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse optional flags
  uint32_t flags = SkObjectID::Flag__default;
  switch (cstr_a[pos])
    {
    case '?':
      flags |= SkObjectID::Flag_possible;
      pos++;
      break;

    case '#':
      // $Revisit - CReis Could store identifier variant in its own custom expression type
      // to save space and invocation time.
      flags |= SkObjectID::Flag_identifier;
      pos++;
      break;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse name symbol-literal
  ASymbol obj_name;
  args.m_result = parse_literal_symbol(pos, &args.m_end_pos, &obj_name);

  if (!args.is_ok())
    {
    return nullptr;
    }

  AString obj_name_str = obj_name.as_string();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Validate object ID
  if (!class_p->is_object_id_lookup())
    {
    args.m_result = Result_err_context_object_id_bad_class;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "The class '%s' in %s does not support validated object ID instance lookup!\n"
        "[Set `object_id_validate` in the !Class.sk-meta file for the class.]",
        class_p->get_name_cstr_dbg(),
        SkObjectID::as_code(obj_name_str, class_p, SkObjectID::get_variant(flags)).as_cstr());
    #endif

    return nullptr;
    }

  SkClassDescBase * type_p = class_p->object_id_validate(SkBindName(obj_name_str), m_flags.is_set_bit(Flag_obj_id_validate));

  if (type_p == nullptr)
    {
    args.m_result = Result_err_context_object_id_invalid;

    #if defined(SK_AS_STRINGS)
      ms_error_str.ensure_size_empty(500u);
      ms_error_str.format(
        "Object ID %s invalid - no instance named '%s' exists for the class '%s'!",
        SkObjectID::as_code(obj_name_str, class_p, SkObjectID::get_variant(flags)).as_cstr(),
        obj_name_str.as_cstr(),
        class_p->get_name_cstr_dbg());
    #endif

    return nullptr;
    }

  switch (SkObjectID::get_variant(flags))
    {
    case SkObjectID::Variant_reference:     // Class@'name'
      args.m_expr_type = type_p;
      break;

    case SkObjectID::Variant_possible_ref:  // Class@?'name'
      args.m_expr_type = SkClassUnion::get_merge(*type_p, *SkNone::get_class());;
      break;

    case SkObjectID::Variant_identifier:    // Class@#'name'
      args.m_expr_type = SkBindName::get_class();
      break;
    }

  // Create object ID
  if (args.is_struct_wanted())
    {
    return SK_NEW(SkObjectID)(obj_name_str, class_p, flags);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses expression tail of prefix operator expression.  For example: not expression
//
// #Notes
//   Looks for the following:
//     ws expression
//
// #Author(s) Conan Reis
SkInvocation * SkParser::parse_prefix_operator_expr(
  const ASymbol & op_name,
  Args & args
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Advance past whitespace
  if (!parse_ws_any(args))
    {
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get expression to modify
  args.m_start_pos = args.m_end_pos;
  args.m_desired_type_p = nullptr;  // Could be any type that has .not()
  SkExpressionBase * expr_p = parse_expression(args, SkInvokeTime_immediate);

  if (!args.is_ok())
    {
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure method is available
  SkParameters *    params_p        = nullptr;
  SkClassDescBase * receiver_type_p = args.m_expr_type;
  //SkClassDescBase * result_type_p   = SkBrain::ms_object_class_p;
  SkMethodBase *    method_p        = nullptr;
  bool              is_class_method = false;

  if (m_flags.is_set_any(Flag_type_check))
    {
    method_p = receiver_type_p->find_method_inherited(op_name, &is_class_method);

    if (method_p == nullptr)
      {
      args.m_result = Result_err_context_non_method;

      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "The prefix operator method '%s' does not exist for %s.",
          op_name.as_cstr_dbg(),
          receiver_type_p->get_scope_desc().as_cstr());
      #endif

      if (expr_p)
        {
        delete expr_p;
        }

      return nullptr;
      }

    // $Revisit - CReis Update with extra checking like parse_invoke_instance_method_arg1()

    params_p      = &method_p->get_params();
    //result_type_p = params_p->get_result_class()->as_finalized_generic(*receiver_type_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create expression structure if requested
  SkInvocation * prefix_expr_p = nullptr;

  if (expr_p)
    {
    SkMethodCallBase * mcall_p = create_method_call(method_p, is_class_method, receiver_type_p, nullptr);
    prefix_expr_p = SK_NEW(SkInvocation)(mcall_p, expr_p);
    // Receiver type stays the same for all Sk prefix operators - just clear the raw access flag here
    // Also, no Sk prefix operator modifies its receiver, so no need to insert a SkRawMemberModifyingInvocation here
    args.m_expr_type.set(receiver_type_p, false);
    // Debug info set in parse_expression_alpha()
    }

  return prefix_expr_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a branch expression.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkConcurrentBranch data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_loop_block,
//                 or pretty much any other warning or error.
// Notes:      branch-block  = 'branch' ws expression
// Modifiers:   protected
// Author(s):   Conan Reis
SkConcurrentBranch * SkParser::parse_concurrent_branch_block(Args & args) const
  {
  SkConcurrentBranch * branch_p = nullptr;

  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  uint32_t pos = args.m_start_pos;

  if ((m_str_ref_p->m_length - pos) >= 2u)
    {
    // Advance past whitespace & parse branch block
    args.m_result  = parse_ws_any(pos, &pos);
    args.m_end_pos = pos;

    if (args.is_ok())
      {
      args.m_start_pos = pos;

      // Parse statement following the branch keyword
      eSkMember saved_member_type = m_member_type;
      m_member_type = SkMember_coroutine;
      if (m_flags.is_set_any(Flag_type_check))
        {
        m_context.capture_locals_start();
        }
      SkExpressionBase * expr_p = parse_expression(args, SkInvokeTime_any);

      if (args.is_ok())
        {
        // Ensure that last statement has side effects since a result is not needed
        if (!ensure_expr_effect(expr_p->find_expr_last_no_side_effect(), &args.m_end_pos, args))
          {
          args.m_result = Result_err_context_last_no_side_effect;
          }
        else if (!ensure_exec_time(*expr_p, args, SkInvokeTime_durational))
          {
          args.m_result = Result_err_context_deferred;
          }
        else
          {
          // All good, create the closure info + coroutine
          SkClosureInfoCoroutine * info_p = SK_NEW(SkClosureInfoCoroutine);
          info_p->set_scope(m_context.m_obj_scope_p->get_key_class());
          info_p->set_name(m_context.m_scope_name);
          info_p->set_params(SkParameters::get_or_create(SkBrain::ms_object_class_p, nullptr));
          info_p->set_expression(expr_p);
          info_p->set_invoked_data_array_size(m_context.m_capture_current_p->m_vars.get_length() + m_context.m_current_scope_p->m_data_idx_count_max);
          branch_p = SK_NEW(SkConcurrentBranch)(info_p);
          args.m_expr_type = SkBrain::ms_invoked_coroutine_class_p;
          m_context.capture_locals_stop(&info_p->m_captured);
          }
        }

      if (!args.is_ok())
        {
        if (expr_p) delete expr_p;
        m_context.capture_locals_stop(nullptr);
        }

      m_member_type = saved_member_type;
      }
    }

  return branch_p;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a sync expression.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkConcurrentSync data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_loop_block,
//                 or pretty much any other warning or error.
// Notes:      sync-block = 'sync' ws code-block
//             code-block = '[' ws [statement {wsr statement} ws] ']'
// Modifiers:   protected
// Author(s):   Conan Reis
SkConcurrentSync * SkParser::parse_concurrent_sync_block(Args & args) const
  {
  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  uint32_t pos = args.m_start_pos;

  if ((m_str_ref_p->m_length - pos) >= 2u)
    {
    // Advance past whitespace & parse
    args.m_result  = parse_ws_any(pos, &pos);
    args.m_end_pos = pos;

    if (args.is_ok())
      {
      args.m_result = Result_err_expected_sync_block;

      // Look for block of concurrent expressions
      if (m_str_ref_p->m_cstr_p[pos] == '[')
        {
        // $Vital - CReis Disallow certain code block features like changing types and no statements (making temporaries)?
        // $Revisit - CReis [A_NOTE] ***Incomplete*** - [Too Permissive] Disallow certain code block features for concurrent expressions.

        args.m_start_pos  = pos;

        SkCode * code_p = parse_code_block(args, SkInvokeTime_durational, StatementTiming_concurrent, ResultDesired_false);

        SkConcurrentSync * concurrent_p = nullptr;

        if (args.is_ok())
          {
          // Statements run in convergent concurrency always return "nil"
          args.m_expr_type = SkNone::get_class();

          if (code_p)
            {
            // Ensure that at least 2 deferred expressions are present
            if (code_p->m_statements.get_length() >= 2u)
              {
              concurrent_p = SK_NEW(SkConcurrentSync)(&code_p->m_statements);
              // Debug character position is set in parse_expression_alpha()
              }
            else
              {
              args.m_result = Result_err_context_concurrent_redundant;
              }

            delete code_p;
            }
          }

        return concurrent_p;
        }
      }
    }

  args.m_end_pos = pos;

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a concurrent race expression.
// Returns:    If parse valid and args.is_struct_wanted() == true it returns a dynamically
//             allocated SkConcurrentRace data-structure otherwise nullptr is returned.
// Arg         args - see SkParser::Args, specific considerations below:
//               m_result: Result_ok, Result_err_unexpected_eof, Result_err_expected_loop_block,
//                 or pretty much any other warning or error.
// Notes:      race-block  = 'race' ws code-block
//             code-block  = '[' ws [statement {wsr statement} ws] ']'
// Modifiers:   protected
// Author(s):   Conan Reis
SkConcurrentRace * SkParser::parse_concurrent_race_block(Args & args) const
  {
  // Next possible error
  args.m_result = Result_err_unexpected_eof;

  uint32_t pos = args.m_start_pos;

  if ((m_str_ref_p->m_length - pos) >= 2u)
    {
    // Advance past whitespace & parse
    args.m_result  = parse_ws_any(pos, &pos);
    args.m_end_pos = pos;

    if (args.is_ok())
      {
      args.m_result = Result_err_expected_race_block;

      // Look for block of concurrent expressions
      if (m_str_ref_p->m_cstr_p[pos] == '[')
        {
        // $Vital - CReis Disallow certain code block features like changing types and no statements (making temporaries)?
        // $Revisit - CReis [A_NOTE] ***Incomplete*** - [Too Permissive] Disallow certain code block features for concurrent expressions.

        args.m_start_pos = pos;

        SkCode * code_p = parse_code_block(args, SkInvokeTime_durational, StatementTiming_concurrent, ResultDesired_false);

        SkConcurrentRace * concurrent_p = nullptr;

        if (args.is_ok())
          {
          // Statements run in racing concurrency always return "nil"
          args.m_expr_type = SkNone::get_class();

          if (code_p)
            {
            // Ensure that at least 2 deferred expressions are present
            if (code_p->m_statements.get_length() >= 2u)
              {
              concurrent_p = SK_NEW(SkConcurrentRace)(&code_p->m_statements);
              // Debug character position is set in parse_expression_alpha()
              }
            else
              {
              args.m_result = Result_err_context_concurrent_redundant;
              }

            delete code_p;
            }
          }

        return concurrent_p;
        }
      }
    }

  args.m_end_pos = pos;

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a unary parameter or group parameter and then appends it
// to `params_p`.
//
// Returns: true if successfully parsed, false if not
//
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//     m_result:
//       Result_ok, Result_err_context_duped_param_name, Result_err_expected_class,
//       Result_err_unexpected_class_pattern, Result_err_expected_param_name, or
//       Result_err_unexpected_eof
//   params_p:
//     pointer to empty parameter list to append parsed arguments to. It is not written
//     to if it is set to nullptr.
//   param_flags: see SkParser::eParamFlag. Currently only uses ParamFlag_auto_type
//   annotation_flags: Annotations of the associated invokable
//
// Notes:
//   Called by parse_parameters()
//             
//   If the result of this parse a value other than Result_ok and '*end_pos_p' is set to
//   a value other than the original given by 'start_pos', it should  be assumed that
//   this portion of the parse was identified as a parameter, but that it had errors or
//   warnings.
//
//   param-specifier = [class-desc wsr] variable-name
//
// See: preparse_param_append()
bool SkParser::parse_param_append(
  Args &         args,
  SkParameters * params_p,
  uint32_t       param_flags,
  uint32_t       annotation_flags
  ) const
  {
  if (params_p == nullptr)
    {
    // Validation parse only
    return parse_parameter(args, nullptr, annotation_flags);
    }

  SkParameterBase * param_p;

  // Parse parameter
  if (!parse_parameter(args, &param_p, annotation_flags))
    {
    return false;
    }

  // Add to parameter list and check for duplicate parameter names
  if (!params_p->m_params.append_absent(*param_p))
    {
    // Found duplicate - free dynamically allocated parameter
    delete param_p;
    args.m_result = Result_err_context_duped_param_name;

    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a unary parameter or
//             group parameter and appends it to params_p.
// Returns:    Result_ok, Result_err_context_duped_param_name, Result_err_expected_class,
//             Result_err_expected_param_name, or Result_err_unexpected_eof
// Arg         start_pos - character position to begin lexical analysis (Default 0u)
// Arg         end_pos_p - character position that lexical analysis stopped at.  If it
//             is set to nullptr, it is not written to.  (Default nullptr)
// Arg         params_p - pointer to parameters to append parsed return
//             arguments to.  It is not written to if it is set to nullptr.  (Default nullptr)
// Arg         param_flags - see SkParser::eParamFlag.  Currently only uses ParamFlag_auto_type
// Examples:   if (parse.parse_param_return_append(11u, &end_pos, &rparams) == Result_ok)
// Notes:      Called by parse_parameters()
//             
//             If the result of this parse a value other than Result_ok and '*end_pos_p'
//             is set to a value other than the original given by 'start_pos', it should
//             be assumed that this portion of the parse was identified as a return
//             parameter, but that it had errors or warnings.
//
//             parameter = unary-parameter | group-param
// Author(s):   Conan Reis
SkParser::eResult SkParser::parse_param_return_append(
  uint32_t       start_pos,
  uint32_t *     end_pos_p,
  SkParameters * params_p,
  uint32_t       param_flags,
  uint32_t       annotation_flags
  ) const
  {
  eResult result;

  if (params_p)
    {
    SkTypedName return_param;

    // Parse parameter
    result = parse_parameter_specifier(start_pos, end_pos_p, &return_param, param_flags, annotation_flags);

    if (result == Result_ok)
      {
      // Next possible error
      result = Result_err_context_duped_rparam_name;

      // Ensure no argument with the same name already present
      if (!params_p->m_return_params.find(return_param))
        {
        result = Result_ok;

        // Add to parameter list
        params_p->m_return_params.append(*SK_NEW(SkTypedName)(return_param));

        // Add return parameter to context scope - but use None as type since it is not
        // initially bound.
        if (m_flags.is_set_any(Flag_type_check))
          {
          m_context.append_local(return_param.get_name(), SkNone::get_class(), true);
          }
        }
      }
    }
  else  // Validation parse only
    {
    result = parse_parameter_specifier(start_pos, end_pos_p, nullptr, param_flags, annotation_flags);
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Parses attempting to create a 'create temporary variable' statement and appends it to
// current code block
// 
// Returns: true if parsed correctly, false if not
// 
// Params:
//   args:
//     see SkParser::Args, specific considerations below:
//       m_result: Result_ok, Result_err_expected_temporary, Result_err_unexpected_cdtor,
//         Result_err_expected_binding, Result_err_unexpected_reserved,
//         Result_err_context_duped_variable, Result_err_unexpected_eof, or any result
//         returned from parsing an expression.
//         
// Notes:
//   Called by parse_statement_append()
//              
//   If the result of this parse a value other than Result_ok and the end pos is set to a
//   value other than the original start pos, it should be assumed that this portion of
//   the parse was identified as a 'create temporary variable' statement, but that it had
//   errors or warnings.
//
//   create-temporary = '!' ws variable-name [ws binding]
//
// See:       parse_temporary(), parse_statement_append()
// Author(s): Conan Reis
bool SkParser::parse_temporary_append(Args & args) const
  {
  ASymbol            ident_name;
  uint32_t           ident_data_idx = 0;
  SkExpressionBase * expr_p    = nullptr;
  uint32_t           start_pos = args.m_start_pos;
  uint32_t           bind_pos  = 0;

  // Parse temporary
  if (parse_temporary(args, &ident_name, &expr_p, &bind_pos))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Add temp var to code block
    m_current_block_p->m_temp_vars.append(ident_name);

    if (m_flags.is_set_any(Flag_type_check))
      {
      ident_data_idx = m_context.append_local(ident_name, args.m_expr_type, false);
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Add bind if create temporary has initial bind
    if (expr_p)
      {
      // Create bind
      SkTypedNameIndexed * var_p = m_context.find_local_variable(ident_name);
      SK_ASSERTX(var_p, "Must exist at this point.");
      var_p->m_has_been_bound = true;

      SkIdentifierLocal * ident_p = SK_NEW(SkIdentifierLocal)(ident_name, ident_data_idx);
      SkBind * bind_p = SK_NEW(SkBind)(ident_p, expr_p);

      m_context.on_identifier_created(ident_p);

      SKDEBUG_SET_CHAR_POS(ident_p, start_pos);
      SKDEBUG_SET_CHAR_POS(bind_p, bind_pos);
      m_current_block_p->m_statements.append(*bind_p);
      }

    return true;
    }

  return false;
  }


//---------------------------------------------------------------------------------------
// Parses starting at 'start_pos' attempting to create a statement and
//             appends it to 'code_p'.
// Returns:    true if parsed correctly, false if not
// Arg         args - see SkParser::Args, specific considerations below:
//               ArgFlag_make_struct - if set then "code_p" must be non-nullptr.
//               m_result: Result_ok, Result_err_unexpected_eof, or just about any other
//                 warning or error.
// Arg         code_p - address of code block to store statements.  It is ignored if
//             ArgFlag_make_struct is not set.
// Notes:      Called by parse_code_block()
//             
//             If the result of this parse a value other than Result_ok and '*end_pos_p'
//             is set to a value other than the original given by 'start_pos', it should
//             be assumed that this portion of the parse was identified as a statement,
//             but that it had errors or warnings.
//
//             statement        = expression | create-temporary | loop-exit
//             create-temporary = '!' ws variable-name [ws binding]
//             loop-exit        = 'exit' [ws identifier]
// Author(s):   Conan Reis
bool SkParser::parse_statement_append(
  Args &        args,
  eSkInvokeTime desired_exec_time // = SkInvokeTime_any
  ) const
  {
  bool statement_b = false;
  uint32_t start_pos   = args.m_start_pos;

  SkExpressionBase * expr_p = nullptr;

  switch (m_str_ref_p->m_cstr_p[start_pos])
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for create temporary
    case '!':
      if (args.is_struct_wanted())
        {
        if (parse_temporary_append(args))
          {
          expr_p = m_current_block_p->m_statements.get_last();
          }
        }
      else
        {
        // Validation parse
        parse_temporary(args);
        }

      // If parse advanced, it is a statement
      statement_b = args.m_end_pos != start_pos;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for loop exit
    case 'e':
      {
      expr_p = parse_loop_exit(args);

      // If parse advanced, it is a statement
      statement_b = args.m_end_pos != start_pos;

      if (expr_p)
        {
        m_current_block_p->m_statements.append(*expr_p);
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If no statement yet found look for an expression
  if (!statement_b)
    {
    // Pass on desired type
    expr_p = parse_expression(args, desired_exec_time);

    if (expr_p)
      {
      m_current_block_p->m_statements.append(*expr_p);
      }
    }
  else
    {
    uint32_t end_pos = args.m_end_pos;

    if (expr_p && !ensure_exec_time(*expr_p, args, desired_exec_time))
      {
      args.m_start_pos = start_pos;
      args.m_end_pos   = end_pos;
      }
    }

  return args.is_ok();
  }

//---------------------------------------------------------------------------------------
// #Description
//   Adds the parameters to the current parse context.
//
// #Author(s) Conan Reis
void SkParser::parameters_context(
  // Parameters to match and to convert Auto_ class to the class used.
  const SkParameters & params,
  // If not nullptr sets m_desired_type_p to result type of params
  Args * result_type_p // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Specify desired result type if requested
  if (result_type_p)
    {
    result_type_p->m_desired_type_p = params.get_result_class();
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Only add parameters to context if type-checking is enabled.
  if (!m_flags.is_set_any(Flag_type_check))
    {
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add send parameters to context
  uint32_t param_count = params.m_params.get_length();

  if (param_count)
    {
    SkParameterBase ** params_pp     = params.m_params.get_array();
    SkParameterBase ** params_end_pp = params_pp + param_count;

    while (params_pp < params_end_pp)
      {
      // $Revisit - CReis Finalize type?
      m_context.append_local((*params_pp)->get_name(), (*params_pp)->get_expected_type(), false);

      params_pp++;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add return parameters to context
  uint32_t rparam_count = params.m_return_params.get_length();

  if (rparam_count)
    {
    SkTypedName ** rparams_pp     = params.m_return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + rparam_count;

    while (rparams_pp < rparams_end_pp)
      {
      // Note that return parameters are initially nil.
      m_context.append_local((*rparams_pp)->get_name(), SkNone::get_class(), true);

      rparams_pp++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   Ensures that the return values are the correct type.
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if parsed correctly, false if not
  bool
SkParser::parameters_typecheck(
  // see SkParser::Args, specific considerations below:
  //   m_result: Result_ok, Result_err_unexpected_eof, or just about any other warning or
  //   error.
  Args & args,
  // Parameters to match and to convert Auto_ class to the class used.
  SkParameters * params_p,
  // if true then do a type-check of the primary result type.
  bool check_result // = true
  ) const
  {
  if (!m_flags.is_set_any(Flag_type_check))
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Don't check types
    args.m_result = Result_ok;

    return true;
    }

  // $Revisit - CReis [A_NOTE] ***Enhancement*** - [Auto-Type] Modify parameter parse to allow auto-typing

  if (check_result)
    {
    SkClassDescBase * result_class_p = params_p->get_result_class();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // If parameter result is Auto_ use the result class from the code body.
    if (result_class_p == SkBrain::ms_auto_class_p)
      {
      // Note - intentionally not finalizing type
      params_p->set_result_type(*args.m_expr_type);
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
      // Ensure result type of expression (from args) is compatible with primary return
      // type of parameters
      SkClassDescBase * ptype_p = m_context.finalize_generic(*result_class_p);

      if (!args.m_expr_type->is_class_type(ptype_p))
        {
        args.m_result = Result_err_typecheck_return_type;

        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The primary return type for the last expression of the code block was expected to be "
            "the type '%s' (based on its parameters), but it is the type '%s' which "
            "is not compatible.",
            ptype_p->as_code().as_cstr(),
            args.m_expr_type->as_code().as_cstr());
        #endif

        return false;
        }
      }
    }

  args.m_result = Result_ok;

  uint32_t rparam_count = params_p->m_return_params.get_length();

  if (rparam_count)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Ensure each return parameter was bound with a compatible type
    SkClassDescBase * rptype_p;
    SkClassDescBase * rtype_p;
    SkTypedName *     rparam_p;
    SkTypedName **    rparams_pp     = params_p->m_return_params.get_array();
    SkTypedName **    rparams_end_pp = rparams_pp + rparam_count;

    while (rparams_pp < rparams_end_pp)
      {
      rparam_p = (*rparams_pp);
      rtype_p  = m_context.get_variable_type(rparam_p->get_name());
      rptype_p = rparam_p->m_type_p;  // Not finalizing yet

      if (rptype_p == SkBrain::ms_auto_class_p)
        {
        // Infer type using bound object type.

        // If type is None then either result variable was never bound or it was only ever
        // bound to a nil - neither of which is useful so flag as an error.
        if (rtype_p == SkNone::get_class())
          {
          args.m_result = Result_err_typecheck_rparam_type;

          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "Tried to auto infer the type for the return parameter '%s' though it was either "
              "only ever bound to nil (which isn't very interesting) or not bound to an object at all.",
              rparam_p->get_name_cstr_dbg());
          #endif

          return false;
          }

        // Note - intentionally not finalizing type
        rparam_p->m_type_p = rtype_p;
        }
      else
        {
        // Ensure return parameter compatible with specified desired type
        rptype_p = m_context.finalize_generic(*rptype_p);

        if (!rtype_p->is_class_type(rptype_p))
          {
          args.m_result = Result_err_typecheck_rparam_type;

          #if defined(SK_AS_STRINGS)
            ms_error_str.ensure_size_empty(500u);
            ms_error_str.format(
              "The return parameter '%s' was expected to be bound to an object of the type '%s' "
              "(based on the parameters of the code block) but it is bound to type '%s' which is "
              "not compatible.",
              rparam_p->get_name_cstr_dbg(),
              rptype_p->as_code().as_cstr(),
              rtype_p->as_code().as_cstr());
          #endif

          return false;
          }
        }

      rparams_pp++;
      }
    }

  args.m_result = Result_ok;

  return true;
  }

//---------------------------------------------------------------------------------------
// Determines bind type desired based on context and member restrictions.
//
// #See Also  identifier_validate_bind(), identifier_validate_bind_type()
// #Author(s) Conan Reis
SkClassDescBase * SkParser::identifier_desired_type(
  // Identifier being bound to an expression
  SkIdentifierLocal * identifier_p,
  // Previous/restricted type of identifier
  SkClassDescBase * identifier_type_p,
  // Type desired by current context - ex: if bind passed as argument
  SkClassDescBase * context_type_p
  ) const
  {
  if (identifier_p == nullptr)
    {
    return context_type_p;
    }

  if (identifier_p->is_local())
    {
    // Determine if it is a return argument
    identifier_type_p = m_context.get_rparam_type(identifier_p->get_name());

    if (identifier_type_p == nullptr)
      {
      // Local non-return arg temporaries can be any type so just return desired context type.
      return context_type_p;
      }
    }

  if (identifier_type_p == nullptr)
    {
    return context_type_p;
    }

  if (context_type_p == nullptr)
    {
    return identifier_type_p;
    }

  return context_type_p->is_class_type(identifier_type_p)
    ? context_type_p      // Narrow to desired context type
    : identifier_type_p;  // Narrow to desired identifier type
  }

//---------------------------------------------------------------------------------------
// Ensures that the supplied expression is an identifier that can be bound
// Returns:    Result_ok, Result_err_unexpected_reserved, Result_err_unexpected_bind_expr
// Arg         identifier_p - expression to test
// Author(s):   Conan Reis
SkParser::eResult SkParser::identifier_validate_bind(SkExpressionBase * identifier_p) const
  {
  // $Revisit - CReis Should be able to do this test without needing a SkExpressionBase.
  // Rewrite parse_expression to return a eSkExprType so this method will be unnecessary.
  eSkExprType expr_type = identifier_p->get_type();

  switch (expr_type)
    {
    // SkIdentifierLocal
    case SkExprType_identifier_local:
      return m_context.is_captured_variable(static_cast<SkIdentifierLocal*>(identifier_p)->get_name()) ? Result_err_unexpected_bind_expr_captured : Result_ok;

    // SkIdentifierMember, SkIdentifierClassMember
    case SkExprType_identifier_member:
    case SkExprType_identifier_class_member:
      return Result_ok;

    // SkIdentifierRawMember - cannot be bound, only assigned to
    case SkExprType_identifier_raw_member:
      return Result_err_unexpected_bind_expr_raw;

    // SkLiteral
    case SkExprType_literal:
      if (static_cast<SkLiteral *>(identifier_p)->get_kind() >= SkLiteral::Type__class)
        {
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "The `%s` identifier cannot change its binding to a different object.",
            ((SkLiteral *)identifier_p)->as_code().as_cstr());
        #endif
        return Result_err_unexpected_reserved;
        }
      return Result_err_unexpected_bind_expr;
        
    // Other Expression Types - see eSkExprType
    default:
      return Result_err_unexpected_bind_expr;
   }
  }

//---------------------------------------------------------------------------------------
// Ensures that the supplied identifier can be bound to the specified type and updates
// the context to reflect the bind if it can.
// 
// Returns: Result_ok, Result_err_typecheck_member_retype, Result_err_typecheck_rparam_retype
SkParser::eResult SkParser::identifier_validate_bind_type(
  SkIdentifierLocal * identifier_p,
  SkClassDescBase *   old_type_p,
  SkClassDescBase *   new_type_p
  ) const
  {
  if (m_flags.is_set_any(Flag_type_check) && (new_type_p != old_type_p))
    {
    // If the variable is member data, ensure that the new type is compatible
    // with the registered member type so that its type may be known from one
    // routine's code block to the next.
    eSkExprType expr_type = identifier_p->get_type();
    if ((expr_type != SkExprType_identifier_local) && !new_type_p->is_class_type(old_type_p))
      {
      #if defined(SK_AS_STRINGS)
        ms_error_str.ensure_size_empty(500u);
        ms_error_str.format(
          "Invalid member type change!\n"
          "The %s data member '%s' is being bound to %s.\n"
          "According to its declaration, it may only be bound to %s.",
          (expr_type == SkExprType_identifier_member) ? "instance" : "class",
          identifier_p->as_code().as_cstr(),
          new_type_p->get_scope_desc().as_cstr(),
          old_type_p->get_scope_desc().as_cstr());
      #endif

      return Result_err_typecheck_member_retype;
      }

    if (expr_type == SkExprType_identifier_local)
      {
      // Determine if it is a return argument
      SkClassDescBase * rtype_p = m_context.get_rparam_type(identifier_p->get_name());

      // Ensure type is compatible with desired return parameter type
      if (rtype_p && !new_type_p->is_class_type(rtype_p))
        {
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "Invalid return parameter type change!\n"
            "The return argument '%s' is being bound to %s.\n"
            "According to the parameter interface, it may only be bound to %s.",
            identifier_p->as_code().as_cstr(),
            new_type_p->get_scope_desc().as_cstr(),
            rtype_p->get_scope_desc().as_cstr());
        #endif

        return Result_err_typecheck_rparam_retype;
        }

      // Ensure predicates ending in `?` only bound to Boolean
      // $Revisit - CReis Could pass in whether it is predicate rather than doing test here
      if ((identifier_p->get_name_str_dbg().get_last() == '?')
        && (new_type_p != SkBrain::ms_boolean_class_p))
        {
        #if defined(SK_AS_STRINGS)
          ms_error_str.ensure_size_empty(500u);
          ms_error_str.format(
            "Tried to bind to type '%s' when Boolean was expected!\n"
            "Query/predicate variables ending with `?` may only be bound "
            "to a Boolean `true`/`false` expression",
            new_type_p->as_code().as_cstr());
        #endif

        return Result_err_typecheck_query_variable;
        }

      // Track local changes to variables.
      // $Revisit - CReis [A_NOTE] ***Incomplete*** - [Too Constrained] Should track member variable type changes too - move out of 'identifier' guard.
      // There is a memory bug that must be fixed for the above change.
      m_context.change_variable_type(identifier_p->get_name(), new_type_p);
      }
    }

  return Result_ok;
  }

//---------------------------------------------------------------------------------------
// Returns pointer to requested method if it exists
// Returns:    Method or nullptr
// Arg         class_p - class scope to search for method
// Arg         method_name - name of method to search for
// Author(s):   Conan Reis
SkMethodBase * SkParser::find_method_inherited(
  SkClassDescBase * class_p,
  const ASymbol &   method_name,
  bool *            is_class_member_p
  ) const
  {
  // $Note - CReis Disallow String() conversion method on Symbol objects.
  if (m_flags.is_set_any(Flag_strict) && (method_name == ASymbol_String))
    {
    if (class_p->get_class_type() == SkClassType_class_union)
      {
      if (static_cast<SkClassUnion *>(class_p)->is_class_maybe(SkBrain::ms_symbol_class_p))
        {
        return nullptr;
        }
      }
    else
      {
      if (class_p->is_class_type(SkBrain::ms_symbol_class_p))
        {
        return nullptr;
        }
      }
    }

  return class_p->find_method_inherited(method_name, is_class_member_p);
  }


#endif  // #if (SKOOKUM & SK_CODE_IN)
