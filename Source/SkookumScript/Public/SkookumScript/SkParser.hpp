// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Parser and associated data-structures
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFlagSet.hpp>
#include <AgogCore/AString.hpp>
#include <SkookumScript/SkTypedContext.hpp>
#include <SkookumScript/SkParameters.hpp>


/*

=========================================================================================
SkookumScript Syntax:  http://skookumscript.com/docs/v3.0/lang/syntax/
=========================================================================================

File Names and Bodies:
----------------------

method-file-name   = method-name '()' ['C'] '.sk'
method-file        = ws {annotation ws} parameters [ws code-block] ws

coroutine-file-name = coroutine-name '().sk'
coroutine-file      = ws {annotation ws} parameter-list [ws code-block] ws

data-file-name     = '!Data' ['C'] '.sk'
data-file          = ws [data-definition {wsr data-definition} ws]
data-definition    = [class-desc ws] '!' instance-name

object-id-filename = file-title '.sk-ids'
object-id-file     = {ws symbol-literal | raw-object-id} ws
raw-object-id      = {printable}^1-255 newline | end-of-file

annotation         = '&' instance_name

Expressions (the starting point):
----------------------
expression       = literal | variable-primitive | identifier | invocation
                   | type-primitive | flow-control

Literals:
----------------------
literal           = boolean-literal | integer-literal | real-literal | string-literal
                    | symbol-literal | character-literal | list-literal | closure
boolean-literal   = 'true' | 'false'
integer-literal   = ['-'] digits-lead ['r' big-digit {['_'] big-digit}]
real-literal      = ['-'] digits-lead V ('.' digits-tail) [real-exponent]
real-exponent     = 'E' | 'e' ['-'] digits-lead
digits-lead       = '0' | (non-0-digit {['_'] digit})
digits-tail       = digit {['_'] digit}
string-literal    = simple-string {ws '+' ws simple-string}
simple-string     = '"' {character} '"'
symbol-literal    = ''' {character} '''
character-literal = '`' character
list-literal      = [(list-class constructor-name invocation-args) | class]
                    '{' ws [expression {ws [',' ws] expression} ws] '}'
closure           = ['^' {annotation ws} ['_' ws] [expression ws]] [parameters ws] code-block

Variable Primitives:
----------------------
variable-primitive = create-temporary | bind
create-temporary   = define-temporary [ws binding]
define-temporary   = '!' ws variable-name
bind               = variable-ident ws binding
binding            = ':' ws expression

Identifiers:
----------------------
identifier       = variable-ident | class | reserved-ident | object-id
variable-ident   = variable-name | ([expression ws '.' ws] data-name)
variable-name    = name-predicate
data-name        = '@' | '@@' variable-name
reserved-ident   = 'nil' | 'this' | 'this_class' | 'this_code' | 'this_mind'
object-id        = [class-name] '@' ['?' | '#'] symbol-literal
method-name-core = name-predicate | constructor-name | destructor-name
method-name-conv = convert-name
name-predicate   = instance-name ['?']
constructor-name = '!' [instance-name]
destructor-name  = '!!'
convert-name     = class-name
coroutine-name   = '_' instance-name
instance-name    = lowercase {alphanumeric}
class-name       = uppercase {alphanumeric}
scope            = class '@' 

Invocations:
----------------------
invocation           = invoke-call | invoke-cascade | invoke-apply | instantiation
                       | invoke-operator | index-operator | slice-operator
invoke-operator      = expression bracketed-args
index-operator       = expression '{' ws expression ws '}' [ws binding]
slice-operator       = expression '{' ws range-literal [wsr expression] ws '}'
invoke-call          = (expression ws '.' invoke-selector) | invoke-selector-core | operator-call
invoke-cascade       = expression ws '.' ws '[' {ws invoke-selector | operator-selector}2+ ws ']'
invoke-apply         = expression ws '%' invoke-selector
instantiation        = [class-instance] | expression '!' [instance-name] invocation args
invoke-selector-core = method-call-core | coroutine-call
invoke-selector      = method-call-core | method-call-conv | coroutine-call
method-call-core     = [scope] method-name-core invocation-args
method-call-conv     = [scope] method-name-conv invocation-args
coroutine-call       = [scope] coroutine-name invocation-args
operator-call        = (prefix-operator ws expression) | (expression ws operator-selector)
operator-selector    = postfix-operator | (binary-operator ws expression)
prefix-operator      = 'not'
binary-operator      = math-operator | compare-operator | logical-operator | ':='
math-operator        = '+' | '+=' | '-' | '-=' | '*' | '*=' | '/' | '/='
compare-operator     = '=' | '~=' | '>' | '>=' | '<' | '<='
logical-operator     = 'and' | 'or' | 'xor' | 'nand' | 'nor' | 'nxor'
postfix-operator     = '++' | '--'
invocation-args      = [bracketed-args] | closure-tail-args
bracketed-args       = '(' ws [send-args ws] [';' ws return-args ws] ')'
closure-tail-args    = ws send-args ws closure [ws ';' ws return-args]
send-args            = [argument] {ws [',' ws] [argument]}
return-args          = [return-arg] {ws [',' ws] [return-arg]}
argument*            = [named-spec ws] expression
return-arg*          = [named-spec ws] variable-ident | define-temporary
named-spec           = variable-name ws ':'

  * only trailing arguments may be named

Type Primitives:
----------------------
primitive        = class-cast | class-conversion
class-cast       = expression ws '<>' [class-desc]
class-conversion = expression ws '>>' [class-name]

Flow Control:
----------------------
primitive        = code-block | conditional | case | when | unless | loop | nil-coalescing
                   | sync-block | race-block | branch-block | change-mind
code-block       = '[' ws [statement {wsr statement} ws] ']'
statement        = expression | create-temporary | loop-exit
conditional      = 'if' {ws expression ws code-block}1+ [ws 'else' ws code-block]
case             = 'case' ws expression {ws expression ws code-block}1+ [ws 'else' ws code-block]
when             = expression ws 'when' ws expression
unless           = expression ws 'unless' ws expression
loop             = 'loop' [ws instance-name] ws code-block
loop-exit        = 'exit' [ws instance-name]
nil-coalescing   = expression ws '??' ws expression
sync-block       = 'sync' ws code-block
race-block       = 'race' ws code-block
branch-block     = 'branch' ws expression
change-mind      = 'change' ws expression ws expression

Parameters:
----------------------
parameters      = parameter-list [ws class-desc]
parameter-list  = '(' ws [send-params ws] [';' ws return-params ws] ')'
send-params     = parameter {ws [',' ws] parameter}
return-params   = param-specifier {ws [',' ws] param-specifier}
parameter       = unary-parameter | group-param
unary-parameter = param-specifier [ws binding]
param-specifier = [class-desc wsr] variable-name
group-param     = '{' ws [class-desc {wsr class-desc} ws] '}' ws instance-name

Class Descriptors:
----------------------
class-desc      = class-unary | class-union
class-unary     = class-instance | meta-class
class-instance  = class | list-class | invoke-class
class           = class-name
meta-class      = '<' class-name '>'
class-union     = '<' class-unary {'|' class-unary}1+ '>'
list-class      = List '{' ws [class-desc ws] '}'
invoke-class    = [class-name] ['_' | '+'] parameters

Whitespace:
----------------------
wsr             = whitespace {whitespace}
ws              = {whitespace}
whitespace      = whitespace-char | comment
whitespace-char = ' ' | alert | backspace | formfeed | newline | carriage-return | horizontal-tab | vertical-tab
comment         = single-comment | multi-comment
single-comment  = '//' {printable-char} (newline | end-of-file)
multi-comment   = '/ *' {printable-char} [multi-comment {printable-char}] '* /'

Characters and Digits:
----------------------
character       = escape-sequence | printable-char
escape-sequence = '\' (integer-literal | printable-char)
alphanumeric    = alphabetic | digit | '_'
alphabetic      = uppercase | lowercase
lowercase       = 'a' | ... | 'z'
uppercase       = 'A' | ... | 'Z'
digits          = '0' | (non-0-digit {digit})
digit           = '0' | non-0-digit
non-0-digit     = '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
big-digit       = digit | alphabetic

*/


//=======================================================================================
// Global Macros / Defines
//=======================================================================================


//=======================================================================================
// Global Structures
//=======================================================================================


#if (SKOOKUM & SK_CODE_IN)

// Pre-declarations
enum eSkMember;
class SkBind;
class SkCase;
class SkClass;
class SkClassUnion;
class SkCode;
class SkConcurrentBranch;
class SkConcurrentRace;
class SkConcurrentSync;
class SkConditional;
class SkCoroutineBase;
class SkCoroutineFunc;
class SkCoroutineCall;
class SkChangeMind;
class SkGroupParam;
class SkIdentifierLocal;
class SkIdentifierMember;
class SkInvocation;
class SkInvokeBase;
class SkInvokeCascade;
class SkInvokeRace;
class SkInvokeSync;
class SkLiteral;
class SkLiteralClosure;
class SkLiteralList;
class SkLoop;
class SkLoopExit;
class SkMetaClass;
class SkMethodBase;
class SkMethodCallBase;
class SkMethodFunc;
class SkMethodToOperator;
class SkNilCoalescing;
class SkObjectID;
class SkParameterBase;
class SkTypedClass;
class SkUnaryParam;

#if defined (A_PLAT_PS3) || defined(A_PLAT_PS4) || defined(A_PLAT_LINUX64) || defined(A_PLAT_ANDROID) || defined(A_PLAT_OSX) || defined(A_PLAT_iOS) || defined(A_PLAT_tvOS) || defined(A_PLAT_SWITCH)
  #include <AgogCore/APArray.hpp>
#else
  template<class _ElementType, class _KeyType = _ElementType, class _CompareClass = ACompareAddress<_KeyType> > class APArray;
  template<class _ElementType, class _KeyType = _ElementType, class _CompareClass = ACompareAddress<_KeyType> > class APArrayFree;
#endif


#endif // (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// SkookumScript Parser
// 
// Author(s)  Conan Reis
class SK_API SkParser : public AString
  {
  public:

  #if (SKOOKUM & SK_CODE_IN)

  // Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse flags - used by m_flags and ms_default_flags
    enum eFlag
      {
      // If set, type info is available - either first compile/parse pass (discovery) of
      // the scripts has been completed or all scripts have been fully parsed - and full
      // type-checking is to be performed.
      // If clear, just parse scripts for lexical and syntactical correctness.
      Flag_type_check      = 1 << 0,

      // If set the parser is extra picky about what it accepts as valid script source.
      Flag_strict          = 1 << 1,

      // If set, parser is in first compile/parse pass (discovery) mode
      Flag_preparse        = 1 << 2,

      // Perform deferred validation of object IDs
      Flag_obj_id_validate = 1 << 3,

      Flag__default = Flag_type_check | Flag_strict
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Result of parse
    enum eResult
      {
      Result_ok,                             // Everything is A-Okay

      Result_ok_deferred,                    // Parse is okay and waiting for deferred invocation - used with invoke_script()

      // Intermediary states
      Result__implicit_this,                 // assumes/infers an implicit 'this' prior to an invocation
      Result__idx_probe,                     // Successfully reached desired index position and returning context information when Args has ArgFlag_parse_to_idx_probe set.

      // Warnings
      Result_warn__start,                    // Start of Warnings
      Result_warn_ident_too_long = Result_warn__start,  // Identifier name is longer than the maximum of 255 characters.
      Result_warn_scripts_disabled,          // Indicates that SkookumScript is not in evalutaion/simulation mode - used with invoke_script()
      Result_warn_empty_script_block,        // The evaluated script block has no statements - used with invoke_script()
      Result_warn_expr_no_effect,            // The expression has no side effects so it does not seem useful as a statement on its own.
      Result_warn_expr_sub_effect,           // The expression has only sub-expressions with side effects - it does not seem useful as a statement on its own.
      Result_warn_expr_redundant,            // * The expression is redundant.


      // Errors
      Result_err_unimplemented,              // Feature is not yet implemented.

      Result_err__start = Result_err_unimplemented, // After Result_err_unimplemented so that it is listed in debugging


      // Expected a particular lexical or syntactical element, but did not find it
      Result_err_expected_annotation_arg,     // Expected required argument for an annotation
      Result_err_expected_binding,            // A binding must begin with a colon ':'.
      Result_err_expected_block,              // Expected a code block [ ], but did not find one.
      Result_err_expected_cast_op,            // Expected the class cast operator '<>', but it was not found.
      Result_err_expected_char,               // A character escape sequence must begin with a backslash character '\\'.
      Result_err_expected_char_number,        // Escape sequence character must have an ASCII value between 0 and 255
      Result_err_expected_class,              // A class name must begin with an uppercase letter
      Result_err_expected_class_desc,         // Expected class, list-class, invoke class, metaclass or class union and did not find one.
      Result_err_expected_class_list_end,     // A List class descriptor must end with a closing brace/curly bracket '}'.
      Result_err_expected_class_instance,     // Expected a class, list-class or an invoke class and did not find one.
      Result_err_expected_class_meta,         // A metaclass descriptor must begin with an opening angle bracket '<'.
      Result_err_expected_class_meta_end,     // A metaclass descriptor must end with a closing angle bracket '>'.
      Result_err_expected_class_params,       // Expected a parameter list following the name of an invokable class
      Result_err_expected_class_union,        // A class union descriptor must begin with an opening angle bracket '<'.
      Result_err_expected_class_union_end,    // A class union descriptor must end with a closing angle bracket '>'.
      Result_err_expected_clause_block,       // Expected a clause code block [ ], but did not receive one.
      Result_err_expected_closure,            // A non-inline closure must start with either a caret/hat symbol '^' or an interface ().
      Result_err_expected_code_block,         // A code block must start with an opening square bracket '['
      Result_err_expected_comment_close,      // Multiple line comment missing closing * /
      Result_err_expected_conversion_op,      // Expected the class conversion operator '>>', but it was not found.
      Result_err_expected_data_defn,          // A data definition statement must start with an exclamation mark '!'
      Result_err_expected_data_name,          // A data member name must start with '@' for instance data and '@@' for class data followed by a lowercase letter.
      Result_err_expected_data_name_scope,    // A data member name must start with '@' for instance data and '@@' for class data according to its scope
      Result_err_expected_digit_radix,        // Expected a radix/base appropriate digit to follow the integer literal radix prefix.
      Result_err_expected_exponent,           // Expected a digit for the exponent, but did not receive one.
      Result_err_expected_seperator_digit,    // Expected a digit to follow a separator '_', but did not receive one.
      Result_err_expected_expression,         // Expected an expression, but did not find one
      Result_err_expected_group_param,        // A group parameter specification must begin with an opening brace '{'
      Result_err_expected_instance,           // An instance name must begin with a lowercase letter
      Result_err_expected_invoke_apply,       // An invocation apply must begin with a percent sign '%' character.
      Result_err_expected_invoke_args,        // An invocation argument list must begin with an opening parenthesis (bracket) '('
      Result_err_expected_invoke_cascade,     // Expected cascaded invocations - i.e. a receiver followed by '.' then '[' then two or more invocations and ending in ']' and did not find any.
      Result_err_expected_invoke_cascades,    // Expected cascaded invocations - i.e. a receiver followed by '.' then '[' then two or more invocations and ending in ']' and only found one invocation.
      Result_err_expected_invoke_selector,    // Expected an invocation selector - i.e. a method call or a coroutine call but found neither.
      Result_err_expected_invoke_select_op,   // Expected an invocation selector - i.e. a method call, an operator call or a coroutine call, but found none of these.
      Result_err_expected_literal_char,       // A character literal must begin with an accent [`] character - the one beneath the tilde '~'
      Result_err_expected_literal_int,        // An integer literal must begin with a minus sign '-' or a digit '0-9'
      Result_err_expected_literal_list,       // Expected a List literal opening brace/curly bracket '{', but did not receive one.
      Result_err_expected_literal_real,       // A real literal must begin with a minus sign '-', a digit '0-9' or a decimal '.'.
      Result_err_expected_literal_real_sgnf,  // A real literal must begin with a significand (integer part and / or fractional part) and did not find one.
      Result_err_expected_literal_real_end,   // While parsing a real number, found integer part though also expected a fractional part ('.' {digit}1+), an exponent part ('E' | 'e' ['-'] digits), or both, but received neither.
      Result_err_expected_literal_string,     // A string literal must begin with a double quote ["]
      Result_err_expected_literal_symbol,     // A symbol literal must begin with a single quote [']
      Result_err_expected_literal_symbol_end, // A symbol literal must end with a single quote [']
      Result_err_expected_loop_block,         // Expected a loop code block [ ], but did not receive one.
      Result_err_expected_loop_exit,          // A loop exit must begin with 'exit'.
      Result_err_expected_meta_key,           // * Expected a class meta key name (demand_load, object_id_validate, or annotations).
      Result_err_expected_meta_value,         // * Expected a particular type of class meta value and did not get it.
      Result_err_expected_method_ctor_name,   // A constructor method name must begin with an exclamation mark '!' and be optionally followed by an identifier starting with a lowercase letter.
      Result_err_expected_method_name,        // A method name must begin with a lowercase letter or an exclamation mark '!'
      Result_err_expected_mind,               // Expected an expression of type Mind.
      Result_err_expected_named_arg,          // Expected a named argument specifier identifier# and did not find one.
      Result_err_expected_op_index_end,       // Expected index operator ending curly bracket/brace `}` and did not find one.
      Result_err_expected_operator,           // Expected an operator method call, but did not find one
      Result_err_expected_obj_id,             // Expected an operator id, but did not find the '@', '@?' or '@#'symbols.
      Result_err_expected_parameters,         // A parameter list must start with an opening parenthesis (bracket) '('
      Result_err_expected_parameter,          // The parameter list expected a parameter and did not find one.
      Result_err_expected_parameter_next,     // The parameter list expected a parameter or end of the list.
      Result_err_expected_param_name,         // Parameter specifiers must be named and no name was found. If you were trying to group expressions using (), use square brackets [] instead.
      Result_err_expected_race_block,         // Expected a 'race' code block [ ], but did not find one.
      Result_err_expected_return_arg,         // Expected a return argument, but did not receive one.
      Result_err_expected_coroutine_name,     // A coroutine name must begin with an underscore '_' and then a lowercase letter
      Result_err_expected_scope_op,           // Expected a scope resolution operator to follow the given class scope.
      Result_err_expected_statement_modifier, // Expected a statement modifier and did not find one
      Result_err_expected_string_close,       // The string literal is missing closing double quote ["]
      Result_err_expected_symbol_close,       // The symbol literal is missing closing single quote [']
      Result_err_expected_temporary,          // A create temporary variable statement must start with an exclamation mark '!'
      Result_err_expected_sync_block,         // Expected a 'sync' code block [ ], but did not find one.
      Result_err_expected_whitespace,         // Whitespace required - expected some combination of whitespace characters and/or comments.

      // Found a known lexical or syntactical element where it was not expected
      Result_err_unexpected_bind_expr,        // A variable rebind to an instance may only be applied to an identifier
      Result_err_unexpected_bind_expr_raw,    // Trying to bind to a raw data member
      Result_err_unexpected_bind_expr_captured, // Trying to bind to a captured variable
      Result_err_unexpected_branch_expr,      // A concurrent branch only makes sense when used on an expression that is not immediate and may take more than one frame to execute such as a coroutine call.
      Result_err_unexpected_cdtor,            // While parsing for a 'create temporary variable statement', a constructor or a destructor call was found instead.
      Result_err_unexpected_char,             // Expected a particular character or type of character, but did not receive it.
      Result_err_unexpected_class_class,      // The metaclass '<Object>' should be used instead of the class instance 'Class'.
      Result_err_unexpected_class_pattern,    // The group parameter descriptor expected a class name or }
      Result_err_unexpected_cpp,              // C++ syntax used by mistake/assumption and equivalent syntax is different or not available in SkookumScript.
      Result_err_unexpected_deprecated,       // Deprecated syntax
      Result_err_unexpected_else,             // An else / default clause may not be the sole clause - there must be at least one more prior to it.
      Result_err_unexpected_else_statement,   // Found an 'else' without a matching 'if' or 'case'.
      Result_err_unexpected_eof,              // Hit end of file prior to the completion of a parse.
      Result_err_unexpected_exit,             // Found a loop 'exit' in an invalid location
      Result_err_unexpected_exit_no_loop,     // Found a loop 'exit' when not nested in the specified loop
      Result_err_unexpected_implicit_this,    // Operator calls may not be used with an implicit 'this' - otherwise it is more error prone and even when used correctly it is more difficult to understand
      Result_err_unexpected_parameters_result, // A coroutine parameter list must not specify a primary return type - the return type InvokedCoroutine is always inferred.
      Result_err_unexpected_parameter_rargs,  // The parameter list did not expect an extra semi-colon ';'!  Return parameters already started.
      Result_err_unexpected_parameter_binary, // * Binary operator must have exactly one parameter
      Result_err_unexpected_query_identifier, // Query/predicate methods are not permitted in instantiation invocations.
      Result_err_unexpected_reserved,         // A reserved word/token may not change its binding - including identifiers (this, this_class, this_code, or nil), literals (true or false), primitives (loop), and statements (exit)
      Result_err_unexpected_return_args,      // Invocation argument list indicated that return arguments were to be used, but routine does not have return parameters.
      Result_err_unexpected_statement,        // The code block expected another statement or the end of the code block ']'.
      Result_err_unexpected_unless_statement, // Found an 'unless' expression modifier without an expression to modify.
      Result_err_unexpected_when_statement,   // Found a 'when' expression modifier without an expression to modify.
      
      // Size errors
      Result_err_size_class_union,            // A class union descriptor must union two or more classes.
      Result_err_size_group_param,            // The group parameter descriptor contained too many classes
      Result_err_size_radix_large,            // The radix / base of a number literal must be 36 or less
      Result_err_size_radix_small,            // The radix / base of a number literal must be 2 or greater
      Result_err_size_identifier,             // An identifier may be no more than 255 characters long.
      Result_err_size_symbol,                 // A symbol literal may be no more than 255 characters long.
      Result_err_size_uint16_out_of_range,    // Value must be between 0 and 65535

      // Context errors
      Result_err_context_actor_class_unknown, // Could not determine actor class from project settings - is the proper project loaded?
      Result_err_context_annotation_unknown,  // Unknown annotation found
      Result_err_context_annotation_invalid,  // Annotation is not allowed in this context
      Result_err_context_annotation_duplicate,// Duplicate annotation provided
      Result_err_context_case_compare,        // * The case comparison expression must resolve to a class type that has an equals operator '='.
      Result_err_context_conversion_params,   // A conversion method may not have any parameters [this may change in the future].
      Result_err_context_duped_data,          // * This data member name is a duplicate of one already existing in this class
      Result_err_context_duped_data_super,    // * This data member name is a duplicate of one already existing in a superclass of this class
      Result_err_context_duped_data_sub,      // * This data member name is a duplicate of one already existing in a subclass of this class
      Result_err_context_duped_loop_name,     // Loop with the same name already present in the current scope.
      Result_err_context_duped_param_name,    // Argument with the same name already present in the parameter list
      Result_err_context_duped_rparam_name,   // Argument with the same name already present in the return parameter list
      Result_err_context_duped_variable,      // A variable with the same name is already present in the current scope
      Result_err_context_invoke_arg1,         // Argument passed to routine that has no parameters.
      Result_err_context_invoke_arg_end,      // Expected the end of the invocation list ')', but did not find it.  [Too many arguments supplied?]
      Result_err_context_invoke_arg_missing,  // One or more arguments that do not have a default expression were not supplied.
      Result_err_context_invoke_arg_misnamed, // No such argument with the specified name exists in the parameter list.
      Result_err_context_invoke_arg_preexist, // An argument with the specified name already exists in the current invocation list.
      Result_err_context_invoke_arg_skipped,  // An argument that does not have a default expression was skipped.
      Result_err_context_invoke_arg_unnamed,  // Once a named argument is used, any following arguments must also be named.
      Result_err_context_non_class,           // A class with the specified name does not exist - ensure that it is registered prior to this parse.
      Result_err_context_non_identifier,      // * An identifier with the specified name does not exist in the current scope.
      Result_err_context_non_ident_member,    // * A data member with the specified name does not exist in the supplied object.
      Result_err_context_non_method,          // * A method with the specified name does not exist or is not registered.
      Result_err_context_non_coroutine,       // * A coroutine with the specified name does not exist or is not registered.
      Result_err_context_object_id_bad_class, // * Object ID was used with a class that does not support object IDs.
      Result_err_context_object_id_invalid,   // * Object ID is invalid - no object with the specified name will be present.
      Result_err_context_immediate,           // Deferred statements (such as coroutines) found where only immediate statements are accepted.
      Result_err_context_deferred,            // Only immediate statements were found where deferred statements (such as coroutines) are expected.
      Result_err_context_concurrent_redundant,  // A concurrent block (sync or race) should have at least two deferred expressions or running concurrently is redundant.
      Result_err_context_side_effect,         // An expression has a side effect when none is allowed
      Result_err_context_last_no_side_effect, // The last expression in a code block has no side effect when one is required
      Result_err_context_raw_access,          // An expression has raw access when that's not supported

      // Type errors
      Result_err_typecheck_return_type,       // * The primary return class type of the code block was not compatible with the return class type of the method (as specified by its parameters).
      Result_err_typecheck_rparam_type,       // * A return parameter was not bound to an object of the expected class type before the end of the code block (as specified by its parameters).
      Result_err_typecheck_case,              // * The result class type of a case test expression must be compatible as an operand to the equals operator '=' of the comparison expression.
      Result_err_typecheck_cast,              // The result class of the expression being cast is not a superclass or subclass the class being cast to so the cast is not valid.
      Result_err_typecheck_closure_generics,  // Generic types are not supported in closure parameter lists and might never be - too many levels of indirection to wrap your head around
      Result_err_typecheck_conversion,        // The result type of a conversion method must be of the same type as or a subclass of the method name.
      Result_err_typecheck_default_param,     // The result type of the default expression was not compatible with the specified parameter type.
      Result_err_typecheck_infer,             // * Unable to infer type.
      Result_err_typecheck_invoke_arg,        // * Supplied argument is not of the expected class type.
      Result_err_typecheck_invoke_apply_recv, // Cannot do an invoke apply [receiver%invocation()] on a receiver that is guaranteed to be nil.
      Result_err_typecheck_list,              // Expected a List class or subclass, but given a non-list class.
      Result_err_typecheck_list_item,         // Supplied list item is not of the specified desired class type.
      Result_err_typecheck_member_retype,     // * Invalid member type change.  A data member can only be rebound to objects of the type specified in its declaration.
      Result_err_typecheck_operand,           // * Operand argument supplied to operator method is not of the expected class type.
      Result_err_typecheck_nil_union,         // * Expected a union class that includes None(nil)
      Result_err_typecheck_query_data,        // * Query/predicate data members ending with a question mark '?' must be specified as a Boolean or omit the type in which case Boolean is inferred.
      Result_err_typecheck_query_result,      // * Query/predicate methods ending with a question mark '?' must either specify a Boolean result or omit the result type in which case Boolean is inferred.
      Result_err_typecheck_query_variable,    // * Query/predicate temporary variables ending with a question mark '?' must only be bound ':' to Boolean objects - true/false.
      Result_err_typecheck_query_param,       // * Query/predicate parameter ending with a question mark '?' must be specified as a Boolean or omit the type in which case Boolean is inferred.
      Result_err_typecheck_rparam_retype,     // * Invalid return parameter type change.  A return argument can only be rebound to objects of the type specified in the parameter interface.
      Result_err_typecheck_scope,             // The specified class scope qualifier is not the same class or a superclass of the class of the receiver expression (or implied 'this').  Note that a NilClass may only have a scope qualifier of "Object".
      Result_err_typecheck_test,              // The result type of a test expression for an if/when/unless must be a Boolean class.
      Result_err_typecheck_ue4_blueprint_param, // A routine annotated with &blueprint cannot take a parameter or return a value that is a Blueprint-generated class.
      Result_err_typecheck_union_trivial,     // This class union descriptor is trivial.  It is lexically correct, but it can be represented more simply as a single class instance or metaclass.

      Result__max
      };  // SkParser::eResult


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Types of code to be identified
    enum eIdentify
      {
      Identify_normal_text,
      Identify_reserved_word,
      Identify_class,          // Verified as an existing class
      Identify_class_like,     // Looks like alass, but not verified to be an existing class
      Identify_operator,
      Identify_op_group_open,  // ( { [
      Identify_op_group_close, // ) } [
      Identify_comment,        // It could be a block of comments which could include a series of single line comments
      Identify_string,         // Simple string
      Identify_symbol,         // Symbol literal
      Identify_data_member,    // @instance or @@class data member
      Identify_object_id,      // Object ID
      Identify_number,
      Identify_annotation,
      Identify_lexical_error
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eIdentifyFlag
      {
      IdentifyFlag_none            = 0,
      IdentifyFlag_break_strings   = 1 << 0,  // Treat only double quote as string and the sub parts of the string identified as code. Useful for search fields that group exact phrases with double quotes.
      IdentifyFlag_break_symbols   = 1 << 1,  // Treat only single quote as symbol and the sub parts of the string identified as code. Useful for search fields that group exact phrases with double quotes.
      IdentifyFlag_break_comments  = 1 << 2,  // Treat internals of comments as regular code

      // Use for word break logic in editors so long sections such as comments have their internals treated separately rather than as a big atomic chunk.
      IdentifyFlag__word_break     = IdentifyFlag_break_strings | IdentifyFlag_break_symbols | IdentifyFlag_break_comments,

      IdentifyFlag__default        = IdentifyFlag_none
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eClassCheck
      {
      ClassCheck_no_validate      = 0x0,  // Do not validate and allow "Class"
      ClassCheck_validate         = 0x1,  // Validate class and allow "Class"
      ClassCheck_no_validate_meta = 0x2,  // Do not validate and do not allow "Class" - use <Object> instead
      ClassCheck_validate_meta    = 0x3,  // Validate class and do not allow "Class" - use <Object> instead

      // Used for bit operations
      ClassCheck__validate = 0x1,
      ClassCheck__meta     = 0x2,
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eResultDesired
      {
      ResultDesired_false  = 0,
      ResultDesired_true   = 1
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eStatementTiming
      {
      StatementTiming_sequential = 0,
      StatementTiming_concurrent = 1
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Type of a parsed expression
    // Carries type pointer plus extra information 
    class ExprType
      {
      public:
        ExprType(SkClassDescBase * type_p = nullptr, bool is_raw_access = false) : m_type_p(type_p), m_is_raw_access(is_raw_access) {}
        ExprType(const ExprType & other, const SkClassDescBase * obj_scope_p) : m_type_p(other.m_type_p->as_finalized_generic(*obj_scope_p)), m_is_raw_access(other.m_is_raw_access) {}

        SkClassDescBase * get_type() const        { return m_type_p; }
        operator SkClassDescBase * () const       { return m_type_p; }
        SkClassDescBase & operator * () const     { return *m_type_p; }
        SkClassDescBase * operator -> () const    { return m_type_p; }
        bool              is_raw_access() const   { return m_is_raw_access; }

        void              set(SkClassDescBase * type_p, bool is_raw_access = false)  { m_type_p = type_p; m_is_raw_access = is_raw_access; }
        void              operator = (SkClassDescBase * type_p)                      { m_type_p = type_p; m_is_raw_access = false; }
        void              clear_raw_access()                                         { m_is_raw_access = false; }

      private:
        // Expression type information
        SkClassDescBase * m_type_p;

        // Means that this expression uses `&raw` access
        // which has certain limitations (can not generically be assumed to be writable, 
        // except in certain special cases like assignment operators)
        bool m_is_raw_access;
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Used by SkParser::Args.m_flags below.
    enum eArgFlag
      {
      // Create a data-structure (and any dependencies )as a result of a successful parse
      ArgFlag_make_struct    = 1 << 0,

      // Parse only to the specified index position and get context information for
      // auto-complete, etc.
      // 
      //   Args struct values on return:
      //     m_result:         Result__idx_probe or if result before idx (Result_ok or error)
      //     m_start_pos:      Starting position of last/current expression
      //     m_end_pos:        Ending position of last/current expression
      //     m_type_p:         Type of last/current expression
      //     m_desired_type_p: Type that is desired at current position
      //     
      //   Future *nice-to-have* context:
      //     - desired and result eSkInvokeTime expression duration
      //     - has side effect?
      //     - routine + argument/idx
      //     - object ID type (could store in m_type_p)
      //     - expression type
      //     - on whitespace/comment?
      //     - full syntax terminal
      ArgFlag_parse_to_idx_probe = 1 << 1,

      ArgFlag__none              = 0x0,
      ArgFlag__default           = ArgFlag_make_struct,
      ArgFlag__default_no_struct = ArgFlag__none
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Information about the invocation(s) currently being parsed - used only for probe parsing
    struct SK_API InvocationInfo
      {
      InvocationInfo(const SkParameters * params_p = nullptr, uint32_t pos = 0)
        : m_params_p(params_p)
        , m_pos(pos)
        , m_param_idx(0)
      {}

      bool is_set() const { return !!m_params_p; }

      const SkParameters *  m_params_p;  // Parameters being invoked
      uint32_t              m_pos;       // Current parsing position
      uint32_t              m_param_idx; // Index of current parameter being parsed
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Used to simplify the arguments passed to the common parse methods
    // 
    // Author(s): Conan Reis
    struct SK_API Args
      {

      typedef bool (*tSkProbeFunc)(const SkParser * parser_p, const Args & args);

      // Data Members

        // IN Data

          // See SkParser::eArgFlag above.
          uint32_t m_flags;

          // Get context at specified index position when `m_flags` is set to
          // `ArgFlag_parse_to_idx_probe`.
          // See: `ArgFlag_parse_to_idx_probe`, `m_idx_probe_user` and `m_idx_probe_func`.
          uint32_t  m_idx_probe;

          // Optional user data to be used by m_idx_probe_func if set.
          uintptr_t m_idx_probe_user;

          // Callback function to invoke at every probe location
          tSkProbeFunc m_idx_probe_f;

        // IN/OUT Data

          // Type hint for upcoming parse so that less type info needs to be specified.
          // nullptr if don't desire a specific type or desired type not known.
          SkClassDescBase * m_desired_type_p;

          // Type of most recent receiver - used only during probe parsing
          // We use ARefPtr because we might want the receiver type to stay around after the parse 
          // while this Args object is still alive
          SkClassDescBase * m_receiver_type_p;

          // Invocation(s) whose parameters are being currently parsed - used only for probe parsing
          AVArray<InvocationInfo> m_invocation_stack;

          // Whether upcoming parse should be immediate (method), durational (coroutine)
          // or either - see `eSkInvokeTime` and `m_exec_time`
          //eSkInvokeTime m_desired_exec_time

          // Character index position to begin lexical analysis - defaults to 0u.
          // [Once an Args has been passed in to a parse method, this value may become
          // modified so it cannot be relied upon to retain the same value that it had
          // when it was originally passed in.]
          uint32_t m_start_pos;

        // OUT Data

          // Character index position that lexical analysis stopped at.  If the parse was
          // successful it will be the character immediately following the parse - i.e.
          // the position to continue with a follow-up parse.  If the parse was not a
          // success then it is the position where the parse failed.
          // [A flag could be stored that indicates whether m_end_pos is desired or not,
          // but since it is desired the vast majority of the time and takes little
          // overhead it is always returned.]
          uint32_t m_end_pos;

          // Result of the parse - Result_ok or any other warning or error.
          // If it is not Result_ok and the method returns a data-structure, nullptr will be
          // returned.  [If a data-structure could be returned but ArgFlag_make_struct is
          // not set then nullptr is always returned for the data-structure and m_result must
          // be checked to determine if the parse was valid.]  Some parse methods are more
          // simplistic and do not return a data-structure and so return true if it is
          // Result_ok and false if it is not.
          // [This is occasionally used as IN data internally too.]
          eResult m_result;

          // Resulting class type of the expression / statement just parsed.
          // Only set / valid if it makes sense for the parse method (i.e. it is
          // something that *can* have a type - see parse method comments) and the code
          // has already been through the preparse phase (Flag_preparse is not set and
          // Flag_type_check is set).
          // [This is occasionally used as IN data internally too.]
          ExprType m_expr_type;

          // Whether resulting expression/statement just parsed was immediate (method) or
          // durational (coroutine) - see `eSkInvokeTime` and `m_desired_exec_time`
          //eSkInvokeTime m_exec_time


      // Methods

        SK_NEW_OPERATORS(SkParser::Args);

        Args();
        Args(uint32_t start_pos, uint32_t flags = ArgFlag__default);
        Args(const Args & args);

        Args & operator=(const Args & args);

        Args & reset();
        Args & reset(uint32_t start_pos);
        Args & set_start(uint32_t start_pos)  { m_start_pos = start_pos; return *this; }
        Args & set_idx_probe(uint32_t idx_probe, tSkProbeFunc idx_probe_func = nullptr, uintptr_t user_data = 0u);

        bool is_ok() const                { return (m_result == Result_ok); }
        bool is_struct_wanted() const     { return (m_flags & ArgFlag_make_struct) != 0u; }
        bool is_idx_probe_halt(const SkParser * parser_p);

      };  // SkParser::Args


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Keep track of important nesting.
    // - currently tracking loops
    // - used by SkParser::m_nest_stack
    struct NestInfo : public ANamed, AListNode<NestInfo>
      {
      NestInfo(const ASymbol & name) : ANamed(name) {}
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Represents parsed list of annotations
    struct Annotations
      {
      uint32_t  m_flags;    // One bit per type of annotation
      AString   m_name;     // Name argument of &raw or &name annotation
      tSkAkas   m_akas;     // Alternative names for invokables      
      
      Annotations() : m_flags(0) {}
      };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Common Methods

    SK_NEW_OPERATORS(SkParser);

    SkParser(const AString & str);
    SkParser(const char * cstr_p, uint32_t length = ALength_calculate, bool persistent = true);

    SkParser & operator=(const AString & str)              { AString::operator=(str); return *this; }
    SkParser & operator=(const SkParser & parser)          { AString::operator=(parser); return *this; }

    SkTypeContext & get_context() const                    { return m_context; }
    eSkMember       get_member_type() const                { return m_member_type; }
    void            reset_scope(SkClassUnaryBase * scope_p = nullptr, const ASymbol & scope_name = ASymbol::ms_null) const;
    void            set_class_scope(SkClassUnaryBase * scope_p = nullptr) const;
    void            set_member_type(eSkMember type) const  { m_member_type = type; }
    void            set_scope(SkClassUnaryBase * scope_p = nullptr, const ASymbol & scope_name = ASymbol::ms_null) const;


  // Comparison Methods

  // Methods

    // $Note - CReis Most of the parse methods have 2 modes: validation parse without
    // type-checking or creating data-structures and full parse with type-checking
    // and data-structures are created.  Making data-structures is done by request by
    // testing the Args::m_flags for ArgFlag_make_struct and typechecking is performed
    // whenever type info is available (it is not available until after the initial
    // preparse of the code and often only if data-structures are being created).

    bool              parse_class_meta_source(SkClass * scope_p, Args & args = ms_def_args.reset(), bool apply_meta_data_b = true);
    bool              parse_data_members_source(SkClassUnaryBase * scope_p, Args & args = ms_def_args.reset(), bool append_to_class_b = true, uint32_t * num_data_members_p = nullptr);
    SkMethodBase *    parse_method_source(const ASymbol & name, SkClassUnaryBase * scope_p, Args & args = ms_def_args.reset(), bool append_to_class_b = true);
    SkCoroutineBase * parse_coroutine_source(const ASymbol & name, SkClassUnaryBase * scope_p, Args & args = ms_def_args.reset(), bool append_to_class_b = true);
    uint32_t          parse_symbol_ids_source(ASymbolTable * ids_p, Args & args = ms_def_args.reset());

    // Configuration / Settings text file Methods

      bool parse_cfg_boolean(uint32_t start_idx = 0u, uint32_t * end_idx_p = nullptr, bool def_value = false) const;


    // Class Result Type Methods

      // $Note - CReis Methods that determine a resulting class type pass use SkParser::Args.
      // Args are passed in non-const so that coercion can be used to create simple Args
      // structures.  Additionally SkParser::ms_def_args is used as a default so that the
      // results can be accessed provided that it is examined soon after the call before it
      // is reused as a default in another parser call.

      SkExpressionBase * parse_binding(Args & args = ms_def_args.reset()) const;
      SkLiteralClosure * parse_closure(Args & args = ms_def_args.reset(), bool allow_inline = false) const;
      SkCode *           parse_code_block(Args & args = ms_def_args.reset(), eSkInvokeTime desired_exec_time = SkInvokeTime_any, eStatementTiming statement_timing = StatementTiming_sequential, eResultDesired result = ResultDesired_true) const;
      SkExpressionBase * parse_code_block_optimized(Args & args = ms_def_args.reset(), eSkInvokeTime desired_exec_time = SkInvokeTime_any, eResultDesired result = ResultDesired_true) const;
      SkExpressionBase * parse_expression(Args & args = ms_def_args.reset(), eSkInvokeTime desired_exec_time = SkInvokeTime_any) const;
      SkExpressionBase * parse_instantiate_or_list(Args & args = ms_def_args.reset()) const;
      SkLiteralList *    parse_literal_list(Args & args = ms_def_args.reset(), SkTypedClass * list_class_p = nullptr, bool item_type_b = false, SkMethodCallBase * ctor_p = nullptr) const;
      SkLiteral *        parse_literal_number(Args & args = ms_def_args.reset(), bool * simple_int_p = nullptr) const;
      SkLoopExit *       parse_loop_exit(Args & args = ms_def_args.reset()) const;
      SkMethodBase *     parse_method(Args & args = ms_def_args.reset(), const ASymbol & name = ASymbol::get_null(), eSkInvokeTime desired_exec_time = SkInvokeTime_any, bool append_to_class_b = true) const;
      SkCoroutineBase *  parse_coroutine(Args & args = ms_def_args.reset(), const ASymbol & name = ASymbol::get_null(), bool append_to_class_b = true) const;
      bool               parse_temporary(Args & args = ms_def_args.reset(), ASymbol * ident_p = nullptr, SkExpressionBase ** expr_pp = nullptr, uint32_t * bind_pos_p = nullptr, bool * predicate_p = nullptr, bool allow_binding = true) const;


    // Simple Parse Methods

      eResult parse_annotations(          uint32_t start_pos, uint32_t * end_pos_p, Annotations * annotations_p, eSkAnnotationTarget target) const;
      eResult parse_class(                uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClass ** class_pp = nullptr) const;
      eResult parse_class_desc(           uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClassDescBase ** type_p = nullptr) const;
      eResult parse_class_instance(       uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClassUnaryBase ** class_pp = nullptr, bool * item_type_b_p = nullptr) const;
      eResult parse_class_list_items(     uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClass * class_p = nullptr, SkTypedClass ** tclass_p = nullptr, bool * item_type_b_p = nullptr) const;
      eResult parse_class_union(          uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClassUnion ** union_p = nullptr) const;
      eResult parse_class_meta(           uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkMetaClass ** mclass_p = nullptr) const;
      eResult parse_class_scope(          uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, SkClass ** qual_scope_pp = nullptr, SkClassDescBase * scope_p = nullptr) const;
      eResult parse_comment_line(         uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr) const;
      eResult parse_comment_multiline(    uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr) const;
      eResult parse_comment(              uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr) const;
      eResult parse_data_definition(      uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, bool append_to_class_b = true) const;
      eResult parse_literal_char_esc_seq( uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, char * ch_p = nullptr) const;
      eResult parse_literal_integer(      uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, tSkInteger * int_p = nullptr, uint32_t * radix_p = nullptr) const;
      eResult parse_literal_real(         uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, tSkReal * real_p = nullptr, bool int_as_real_b = true) const;
      eResult parse_literal_simple_string(uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, AString * str_p = nullptr) const;
      eResult parse_literal_string(       uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, AString * str_p = nullptr) const;
      eResult parse_literal_symbol(       uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, ASymbol * symbol_p = nullptr) const;
      eResult parse_name_class(           uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, ASymbol * name_p = nullptr, eClassCheck check = ClassCheck_no_validate_meta) const;
      eResult parse_name_data_member(     uint32_t start_pos, uint32_t * end_pos_p, ASymbol * name_p = nullptr, bool * predicate_p = nullptr, bool * class_member_p = nullptr) const;
      eResult parse_name_instance(        uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, ASymbol * name_p = nullptr) const;
      eResult parse_name_method(          uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, ASymbol * name_p = nullptr) const;
      eResult parse_name_coroutine(       uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, ASymbol * name_p = nullptr) const;
      eResult parse_whitespace(           uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr) const;
      eResult parse_ws_any(               uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, bool treat_lf_as_ws = true) const;
      bool    parse_ws_any(               Args & args) const  { args.m_result = parse_ws_any(args.m_start_pos, &args.m_end_pos); return args.m_result == Result_ok; }
      bool    parse_ws_end(               Args & args) const;
      eResult parse_ws_required(          uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr) const;

    // Preparse Methods - Partially parses code for context for later full parse.

      SkMethodBase *    preparse_method_source(const ASymbol & name, SkClassUnaryBase * scope_p, Args & args = ms_def_args.reset(), bool * has_signature_changed_p = nullptr) const;
      SkCoroutineBase * preparse_coroutine_source(const ASymbol & name, SkClassUnaryBase * scope_p, Args & args = ms_def_args.reset(), bool * has_signature_changed_p = nullptr) const;

    // Identification Methods - Quickly identifies/categorizes a section of code without necessarily doing a full analysis.

      eIdentify identify_text(uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, uint flags = IdentifyFlag__default) const;
      eIdentify identify_class(uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, AString * class_name_p = nullptr, SkClass ** class_pp = nullptr) const;
      eSkMember identify_member_filename(SkQualifier * ident_p = nullptr, bool * class_member_p = nullptr, bool optional_scope = true) const;
      eResult   identify_member_name(SkMemberInfo * member_p, uint32_t start_pos = 0u, uint32_t * end_pos_p = nullptr, eSkMember accept_to = SkMember_class_meta) const;

  // Class Methods

    static void initialize();
    static void deinitialize();

    static void clear_stats();
    static void print_stats();

    static AFlagSet32 &         get_default_flags()                                          { return ms_default_flags; }
    static bool                 is_ident_operator(uint32_t sym_id);
    static bool                 is_ident_reserved(uint32_t sym_id);
    static bool                 is_strict()                                                  { return ms_default_flags.is_set_any(Flag_strict); }
    static void                 enable_strict(bool strict = true)                            { ms_default_flags.enable(Flag_strict, strict); }
    static AString              get_result_context_string(const AString & code, eResult result, uint32_t result_pos, uint32_t result_start = ADef_uint32, uint32_t start_pos = 0u);
    static AString              get_result_string(eResult result);
    static eResult              invoke_script(const AString & code, AString * result_str_p = nullptr, SkInstance ** result_pp = nullptr, SkInstance * instance_p = nullptr, bool print_info = true);

  #endif // (SKOOKUM & SK_CODE_IN)


  #ifdef SK_CODE

    static ASymbol method_to_operator(const ASymbol & method_name);

  #endif


  #if (SKOOKUM & SK_CODE_IN)

  protected:

  // Internal Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Used by parameter parse methods
    enum eParamFlag
      {
      // If class type is omitted use the "Auto_" placeholder class (rather than the "Object"
      // class) and try to infer type through use of defaults or via the parsing of an
      // associated code block.
      ParamFlag_auto_type        = 1 << 0,

      // Ensure no result class is present and use `InvokedCoroutine` as result class.
      // [May not use with ParamFlag_result_bool.]
      ParamFlag_coroutine        = 1 << 1,

      // Query/predicate method - ensure that result class is `Boolean` or infer `Boolean` if
      // omitted. [May not use with ParamFlag_coroutine.]
      ParamFlag_result_bool      = 1 << 2,

      ParamFlag__mask_result = ParamFlag_coroutine | ParamFlag_result_bool,
      ParamFlag__none        = 0,
      ParamFlag__default     = ParamFlag__none,
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eInvokeBrackets
      {
      InvokeBrackets_required          = 0,
      InvokeBrackets_opt_args0         = 1 << 0,
      InvokeBrackets_opt_closure_tail  = 1 << 1,
      InvokeBrackets_opt_args0_closure = InvokeBrackets_opt_args0 | InvokeBrackets_opt_closure_tail
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Info for tightly coupled invocation argument parsing functions
    //   - tracks group parameters and cleans up for parent function on errors
    struct InvokeArgsInfo
      {
      // Data Members

        Args * m_args_p;
        SkClassDescBase * m_final_rcvr_type_p;
        const SkParameters * m_params_p;
        SkParameterBase ** m_plist_pp;
        SkParameterBase * m_param_p;

        SkLiteralList * m_group_arg_p;

        APCompactArray<SkExpressionBase> * m_arg_exprs_p;

        bool m_group_arg_b;
        bool m_named_args_b;
        bool m_implicit_arg1_b;

        // Positional arg count (group-param args count as 1)
        uint32_t m_arg_count;
        uint32_t m_group_idx;
        uint32_t m_group_count;  // Pattern length

        uint32_t m_pattern_start;
        uint32_t m_pattern_offset;

      // Methods

      InvokeArgsInfo(Args * args_p, const SkParameters * params_p, APCompactArray<SkExpressionBase> * arg_exprs_p, bool implicit_arg1_b);
      ~InvokeArgsInfo();

      bool complete_group_arg();
      };


  // Internal Methods

    SkMethodBase *            find_method_inherited(SkClassDescBase * class_p, const ASymbol & method_name, bool * is_class_member_p = nullptr) const;
    bool                      find_nested(const ASymbol & name) const;
    static SkMethodCallBase * create_method_call(SkMethodBase * method_p, bool is_class_method, SkClassDescBase * qualified_receiver_type_p, SkClass * qual_scope_p);

    // Internal Interface/Parameter Methods

      bool parse_parameters(     Args & args, SkParameters * params_p = nullptr, uint32_t flags = ParamFlag__default, uint32_t annotation_flags = SkAnnotation__default) const;
      bool parse_param_append(   Args & args, SkParameters * params_p, uint32_t param_flags, uint32_t annotation_flags) const;
      bool parse_parameter(      Args & args, SkParameterBase ** param_new_pp, uint32_t annotation_flags) const;
      bool parse_parameter_unary(Args & args, SkUnaryParam * uparam_p, uint32_t annotation_flags) const;

      eResult parse_parameter_specifier(  uint32_t start_pos, uint32_t * end_pos_p, SkTypedName * tname_p, uint32_t param_flags, uint32_t annotation_flags) const;
      eResult parse_parameter_group(      uint32_t start_pos, uint32_t * end_pos_p, SkGroupParam * vparam_p, uint32_t annotation_flags) const;
      eResult parse_param_return_append(  uint32_t start_pos, uint32_t * end_pos_p, SkParameters * params_p, uint32_t param_flags, uint32_t annotation_flags) const;

      eResult preparse_param_append(      uint32_t start_pos, uint32_t * end_pos_p, SkParameters * params_p, uint32_t param_flags, uint32_t annotation_flags) const;
      eResult preparse_parameter(         uint32_t start_pos, uint32_t * end_pos_p, SkParameterBase ** param_new_pp, uint32_t annotation_flags) const;
      eResult preparse_parameter_unary(   SkUnaryParam * uparam_p, uint32_t start_pos, uint32_t * end_pos_p, uint32_t annotation_flags) const;

      void    parameters_context(const SkParameters & params, Args * result_type_p = nullptr) const;
      bool    parameters_typecheck(Args & args, SkParameters * params_p, bool check_result = true) const;


    // Internal Class Result Type Methods

      SkExpressionBase *   parse_expression_alpha(Args & args) const;
      SkCase *             parse_case_tail(Args & args = ms_def_args.reset()) const;
      SkConditional *      parse_conditional_tail(Args & args = ms_def_args.reset()) const;
      SkLoop *             parse_loop_tail(Args & args) const;
      SkConcurrentSync *   parse_concurrent_sync_block(Args & args) const;
      SkConcurrentRace *   parse_concurrent_race_block(Args & args) const;
      SkConcurrentBranch * parse_concurrent_branch_block(Args & args) const;
      SkChangeMind *       parse_change_mind(Args & args) const;
      SkObjectID *         parse_object_id_tail(Args & args, SkClass * class_p = nullptr) const;
      SkInvocation *       parse_prefix_operator_expr(const ASymbol & op_name, Args & args) const;
      bool                 parse_statement_append(Args & args, eSkInvokeTime desired_exec_time = SkInvokeTime_any) const;
      bool                 parse_temporary_append(Args & args) const;

      // Internal Methods that need args.m_type_p to be set with class type of receiver on
      // entry and are set to resulting type on exit

        SkBind *             parse_bind(Args & args, SkExpressionBase * receiver_p) const;
        SkExpressionBase *   parse_class_cast(Args & args, SkExpressionBase * receiver_p) const;
        SkExpressionBase *   parse_class_conversion(Args & args, SkExpressionBase * receiver_p) const;
        SkIdentifierLocal *  parse_data_accessor(Args & args, SkExpressionBase * owner_p) const;
        SkExpressionBase *   parse_expression_string(Args & args, SkExpressionBase ** receiver_pp) const;
        SkInvokeCascade *    parse_invoke_cascade(Args & args, SkExpressionBase * receiver_p) const;
        SkExpressionBase *   parse_invoke_instantiate(Args & args, SkExpressionBase * receiver_p, bool * is_raw_redundant_copy_p) const;
        SkInvokeSync *       parse_invoke_apply(Args & args, SkExpressionBase * receiver_p) const;
        SkInvokeRace *       parse_invoke_race(Args & args, SkExpressionBase * receiver_p) const;
        SkExpressionBase *   parse_invoke_operator(Args & args, SkExpressionBase * receiver_p) const;
        SkInvocation *       parse_invoke_index_operator(Args & args, SkExpressionBase * receiver_p) const;
        SkMethodCallBase *   parse_invoke_method(Args & args, SkExpressionBase ** receiver_pp = nullptr) const;
        SkMethodCallBase *   parse_invoke_instance_method_arg1(Args & args, SkMethodBase * method_p, SkExpressionBase * arg1_p) const;
        SkMethodCallBase *   parse_invoke_ctor(Args & args) const;
        SkInvokeBase *       parse_invoke_selector(Args & args, bool test_op, SkExpressionBase ** receiver_pp = nullptr) const;
        SkCoroutineCall *    parse_invoke_coroutine(Args & args) const;
        SkExpressionBase *   parse_modifier_tail(Args & args, SkExpressionBase * expr_p) const;
        SkNilCoalescing *    parse_nil_coalescing_tail(Args & args, SkExpressionBase * receiver_p) const;
        SkMethodCallBase *   parse_operator_call(Args & args) const;

        bool      parse_invoke_args_arg1(Args & args, SkExpressionBase * arg1_p, APCompactArray<SkExpressionBase> * args_p = nullptr, const SkMethodBase * method_p = nullptr) const;
        bool      parse_invoke_args(Args & args, APCompactArray<SkExpressionBase> * args_p = nullptr, APCompactArray<SkIdentifierLocal> * ret_args_p = nullptr, const SkParameters * params_p = nullptr, eInvokeBrackets brackets = InvokeBrackets_opt_args0_closure, bool implicit_arg1_b = false) const;
        eAConfirm parse_invoke_arg(Args & args, InvokeArgsInfo * args_info_p, uint32_t bracket_flags) const;
        eAConfirm parse_invoke_arg_default(Args & args, InvokeArgsInfo * args_info_p) const;
        eResult   parse_invoke_return_args(uint32_t start_pos, uint32_t * end_pos_p, APCompactArray<SkIdentifierLocal> * ret_args_p, const SkClassUnaryBase * receiver_type_p, const SkParameters * params_p, uint32_t bracket_flags) const;

    // Internal Simple Parse Methods

      ASymbol as_symbol(                uint32_t start_pos, uint32_t end_pos) const;
      bool    is_constructor(           uint32_t start_pos = 0u) const;
      eResult parse_digits_lead(        uint32_t start_pos, uint32_t * end_pos_p, tSkInteger * int_p) const;
      void    parse_name_symbol(        uint32_t start_pos, uint32_t * end_pos_p, ASymbol * name_p = nullptr) const;
      eResult parse_name_predicate(     uint32_t start_pos, uint32_t * end_pos_p, ASymbol * name_p = nullptr, bool * predicate_p = nullptr, bool test_resevered = true) const;
      eResult parse_named_specifier(    uint32_t start_pos, uint32_t * end_pos_p = nullptr, const SkParameters * params_p = nullptr, uint32_t * arg_idx_p = nullptr, SkParameters::eType param_type = SkParameters::Type_send) const;

      // Test methods that depend on expression data-structure
      // $Vital - CReis These tests need to be rewritten to work even if an expression structure is not available

      bool              ensure_expr_effect(const SkExpressionBase * expr_p, uint32_t * pos_p, Args & args) const;
      bool              ensure_exec_time(const SkExpressionBase & expr, Args & args, eSkInvokeTime desired_exec_time) const;
      SkClassDescBase * identifier_desired_type(SkIdentifierLocal * identifier_p, SkClassDescBase * identifier_type_p, SkClassDescBase * context_type_p) const;
      eResult           identifier_validate_bind(SkExpressionBase * identifier_p) const;
      eResult           identifier_validate_bind_type(SkIdentifierLocal * identifier_p, SkClassDescBase * old_type_p, SkClassDescBase * new_type_p) const;


  // Data Members

    // Parse flags - see SkParser::eFlag
    AFlagSet32 m_flags;

    // Member type being parsed
    mutable eSkMember m_member_type;

    // Scope context of current parse
    mutable SkTypeContext m_context;

    // Nesting stack - see NestInfo
    mutable AList<NestInfo> m_nest_stack;

    // Most recent code block
    mutable SkCode * m_current_block_p;

  // Class Data Members

    // Initial flags for new SkParser objects if flags are not otherwise specified.
    // - see m_flags and eFlag
    static AFlagSet32 ms_default_flags;

    // $Revisit - CReis This should be part of a result structure.
    // Extra info to give more context for parsing errors.
    static AString ms_error_str;

    // Default arguments to use if no SkParser::Args structure is supplied.
    static Args ms_def_args;

    // Conversion table from method to operator
    static const SkMethodToOperator * ms_method_to_operator_p;

  #endif // (SKOOKUM & SK_CODE_IN)

  };  // SkParser


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkParser.inl>
#endif
