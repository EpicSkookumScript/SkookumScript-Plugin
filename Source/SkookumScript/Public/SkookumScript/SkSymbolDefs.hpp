// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// A number of handy symbol definitions
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp>
#include <AgogCore/ASymbol.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

namespace SkSymbolDefs
  {
  // Assign values to the static symbols
  void initialize();

  // Clean up static symbols
  void deinitialize();
  }

//---------------------------------------------------------------------------------------
// These Macros will expand to create constants of type ASymbol in the form:
//   ASYM(Object) -> ASymbol_Object  // Representing the symbol 'Object'
//
// Extra care should be taken when using them during global initialization time.
//
// Eventually, in release builds, these could be just be numerical ids and AString objects
// will be dropped all together.
//
// Try to keep them in alphabetical order so that they are easy to scan at a glance.
//
#define SK_SYMBOLS \
  ASYM(Actor) \
  ASYM(Auto_) \
  ASYM(auto_parse_) \
  ASYM(Boolean) \
  ASYM(Char) \
  ASYM(Class) \
  ASYM(Closure) \
  ASYM(Debug) \
  ASYM(Data) \
  ASYM(DataC) \
  ASYM(Filter) \
  ASYM(Integer) \
  ASYM(InvokedBase) \
  ASYM(InvokedContextBase) \
  ASYM(InvokedMethod) \
  ASYM(InvokedCoroutine) \
  ASYM(ItemClass_) \
  ASYM(List) \
  ASYM(Master) \
  ASYM(Mind) \
  ASYM(None) \
  ASYM(Object) \
  ASYM(Random) \
  ASYM(Real) \
  ASYM(String) \
  ASYM(Symbol) \
  ASYM(ThisClass_) \
  ASYM(TriggerInVehicle) \
  ASYM(TriggerOnBike) \
  ASYM(TriggerOnFoot) \
  ASYM(_Class) \
  ASYM(_Focus) \
  ASYM(_Named) \
  ASYM(_Player) \
  ASYM(_This) \
  ASYM(_anon) \
  ASYM(_closure) \
  ASYM(_pulse) \
  ASYM(_wait) \
  ASYM(_wait_until) \
  ASYM(_user_) \
  ASYM(add) \
  ASYM(add_assign) \
  ASYM(and) \
  ASYM(append) \
  ASYM(append_items) \
  ASYM(append_list) \
  ASYM(as_new) \
  ASYM(as_code) \
  ASYM(assign) \
  ASYM(at) \
  ASYM(at_set) \
  ASYM(branch) \
  ASYM(case) \
  ASYM(change) \
  ASYM(closure) \
  ASYM(coin_toss) \
  ASYM(crop) \
  ASYM(decrement) \
  ASYM(divide) \
  ASYM(divide_assign) \
  ASYM(down_slope) \
  ASYM(down_slope_int) \
  ASYM(else) \
  ASYM(empty) \
  ASYM(exit) \
  ASYM(false) \
  ASYM(find_named) \
  ASYM(first) \
  ASYM(if) \
  ASYM(increment) \
  ASYM(insert) \
  ASYM(insert_items) \
  ASYM(insert_list) \
  ASYM(invoke_script_) \
  ASYM(items_set) \
  ASYM(last) \
  ASYM(length) \
  ASYM(length_after) \
  ASYM(loop) \
  ASYM(multiply) \
  ASYM(multiply_assign) \
  ASYM(name) \
  ASYM(nand) \
  ASYM(negated) \
  ASYM(negate) \
  ASYM(nil) \
  ASYM(nor) \
  ASYM(normal) \
  ASYM(normal_int) \
  ASYM(nose) \
  ASYM(nose_int) \
  ASYM(not) \
  ASYM(nxor) \
  ASYM(object_id_find) \
  ASYM(or) \
  ASYM(origin_actor_update) \
  ASYM(origin_class_ctors) \
  ASYM(origin_default_ctor) \
  ASYM(origin_embedded1) \
  ASYM(origin_embedded2) \
  ASYM(origin_embedded3) \
  ASYM(origin_embedded4) \
  ASYM(origin_embedded5) \
  ASYM(origin_parser_interpreted) \
  ASYM(pop_at) \
  ASYM(pop_first) \
  ASYM(pop_last) \
  ASYM(pop_range) \
  ASYM(pop_range_last) \
  ASYM(race) \
  ASYM(range) \
  ASYM(remove_at) \
  ASYM(remove_first) \
  ASYM(remove_last) \
  ASYM(remove_range) \
  ASYM(remove_range_last) \
  ASYM(reuse) \
  ASYM(reverse) \
  ASYM(rotate_down) \
  ASYM(rotate_up) \
  ASYM(seed) \
  ASYM(seed_set) \
  ASYM(subtract) \
  ASYM(subtract_assign) \
  ASYM(swap) \
  ASYM(sync) \
  ASYM(this) \
  ASYM(this_class) \
  ASYM(this_code) \
  ASYM(this_mind) \
  ASYM(thorn) \
  ASYM(thorn_int) \
  ASYM(triangle) \
  ASYM(triangle_int) \
  ASYM(triangle_symm) \
  ASYM(true) \
  ASYM(uniform) \
  ASYM(uniform_int) \
  ASYM(uniform_symm) \
  ASYM(unless) \
  ASYM(up_slope) \
  ASYM(up_slope_int) \
  ASYM(when) \
  ASYM(xor) \


//---------------------------------------------------------------------------------------
// Ids that cannot be represented as C++ identifiers.
// They take the form of:
//   ASYMX(strat_default, "$default") -> ASymbolX_strat_default
//
// Extra care should be taken when using them during global initialization time.
//
// Try to keep them in alphabetical order so that they are easy to scan at a glance.
//
#define SK_SYMBOLS_NAMED \
  ASYMX(ctor,                "!") \
  ASYMX(ctor_copy,           "!copy") \
  ASYMX(ctor_seed,           "!seed") \
  ASYMX(dtor,                "!!") \
  ASYMX(equalQ,              "equal?") \
  ASYMX(greaterQ,            "greater?") \
  ASYMX(greater_or_equalQ,   "greater_or_equal?") \
  ASYMX(lessQ,               "less?") \
  ASYMX(less_or_equalQ,      "less_or_equal?") \
  ASYMX(not_equalQ,          "not_equal?") \
  ASYMX(op_increment,        "++") \
  ASYMX(op_decrement,        "--") \
  ASYMX(op_negated,          "-expr") \
  ASYMX(op_assign,           ":=") \
  ASYMX(op_add,              "+") \
  ASYMX(op_add_assign,       "+=") \
  ASYMX(op_subtract,         "-") \
  ASYMX(op_subtract_assign,  "-=") \
  ASYMX(op_multiply,         "*") \
  ASYMX(op_multiply_assign,  "*=") \
  ASYMX(op_divide,           "/") \
  ASYMX(op_divide_assign,    "/=") \
  ASYMX(op_equals,           "=") \
  ASYMX(op_not_equal,        "~=") \
  ASYMX(op_greater,          ">") \
  ASYMX(op_greater_or_equal, ">=") \
  ASYMX(op_less,             "<") \
  ASYMX(op_less_or_equal,    "<=") \
  ASYMX(op_index,            "{}") \
  ASYMX(op_index_set,        "{}:") \
  ASYMX(unnamed,             "?unnamed?") \


//---------------------------------------------------------------------------------------
// Declare ASymbol constants
#define ASYM(_id)         SK_API ASYMBOL_DECLARE(ASymbol, _id)
#define ASYMX(_id, _str)  SK_API ASYMBOL_DECLARE(ASymbolX, _id)
  SK_SYMBOLS
  SK_SYMBOLS_NAMED
#undef ASYMX
#undef ASYM


//---------------------------------------------------------------------------------------
// Common Symbol ids
const uint32_t ASymbolId_null               = ASymbol_id_null;

  // Class Names
const uint32_t ASymbolId_Boolean            = 0x4538b1f4;
const uint32_t ASymbolId_Class              = 0x2c8a369b;
const uint32_t ASymbolId_Closure            = 0x87bab44d;
const uint32_t ASymbolId_Enum               = 0xae8c016f;
const uint32_t ASymbolId_Integer            = 0x95b29297;
const uint32_t ASymbolId_InvokedBase        = 0x7f267335;
const uint32_t ASymbolId_InvokedContextBase = 0xd6ea1fc8;
const uint32_t ASymbolId_InvokedMethod      = 0x1387e23f;
const uint32_t ASymbolId_InvokedCoroutine   = 0x13dcc574;
const uint32_t ASymbolId_List               = 0xe4fa5726;
const uint32_t ASymbolId_None               = 0xdfa2aff1;
const uint32_t ASymbolId_Object             = 0xaf01aeda;
const uint32_t ASymbolId_Random             = 0x1197dfe3;  
const uint32_t ASymbolId_Real               = 0x36be666b;
const uint32_t ASymbolId_String             = 0x9912b79f;
const uint32_t ASymbolId_Symbol             = 0xeb6433cf;

  // Reserved Words
const uint32_t ASymbolId_branch             = 0xbb861b1f;
const uint32_t ASymbolId_case               = 0x78089904;
const uint32_t ASymbolId_change             = 0x4057fe20;
const uint32_t ASymbolId_defer              = 0x4cb319c1;
const uint32_t ASymbolId_else               = 0x55bb558b;
const uint32_t ASymbolId_exit               = 0x9409840e;
const uint32_t ASymbolId_false              = 0x2bcd6830;
const uint32_t ASymbolId_if                 = 0x5137067c;
const uint32_t ASymbolId_loop               = 0xa15f1dee;
const uint32_t ASymbolId_nil                = 0x4d33c2a5;
const uint32_t ASymbolId_race               = 0xda6fbbaf;
const uint32_t ASymbolId_random             = 0x163bdad5;
const uint32_t ASymbolId_return             = 0xa79e3f0f;
const uint32_t ASymbolId_rush               = 0xf533deef;
const uint32_t ASymbolId_skip               = 0xf876557d;
const uint32_t ASymbolId_sync               = 0x2c2bf57a;
const uint32_t ASymbolId_this               = 0xfeee8227;
const uint32_t ASymbolId_this_class         = 0x3f064bae;
const uint32_t ASymbolId_this_code          = 0x37c2fcbe;
const uint32_t ASymbolId_this_mind          = 0x5e782f23;
const uint32_t ASymbolId_true               = 0xfdfc4c8d;
const uint32_t ASymbolId_unless             = 0x3eddbf3e;
const uint32_t ASymbolId_when               = 0x23e80e1c;

  // Boolean word operators
const uint32_t ASymbolId_and                = 0x07f59b6d;
const uint32_t ASymbolId_nand               = 0x02091d53;
const uint32_t ASymbolId_nor                = 0xe1665840;
const uint32_t ASymbolId_not                = 0x0805fd75;
const uint32_t ASymbolId_nxor               = 0xfc3101bc;
const uint32_t ASymbolId_or                 = 0x1db77587;
const uint32_t ASymbolId_xor                = 0xf9cd8782;

  // Annotations
const uint32_t ASymbolId_raw                = 0x1ab3db55;
const uint32_t ASymbolId_name               = 0x5e237e06;
const uint32_t ASymbolId_aka                = 0x0ae89ba7;
const uint32_t ASymbolId_reflected_cpp      = 0xd46e9678;
const uint32_t ASymbolId_reflected_data     = 0x52e21d21;
const uint32_t ASymbolId_invokable          = 0x7709b14e;
const uint32_t ASymbolId_blueprint          = 0x2645e266; // $Revisit MBreyer make this data driven 

  // C++ Reserved Words to watch for
const uint32_t ASymbolId_break              = 0x0c7c50da;
const uint32_t ASymbolId_continue           = 0x13e32adf;
const uint32_t ASymbolId_default            = 0xe35e00df;
const uint32_t ASymbolId_do                 = 0x9d45c095;
const uint32_t ASymbolId_for                = 0xef7509f8;
const uint32_t ASymbolId_switch             = 0x6fe94b18;
const uint32_t ASymbolId_while              = 0xc0d455fd;
  
  // Special methods
const uint32_t ASymbolId_ctor               = 0x9e6bffd3; // "!"
const uint32_t ASymbolId_ctor_copy          = 0x567d9537; // "!copy"
const uint32_t ASymbolId_assign             = 0x7222a9a1; // "assign"
const uint32_t ASymbolId_dtor               = 0x812f1742; // "!!"
const uint32_t ASymbolId_assert             = 0xb1ef4fab;
const uint32_t ASymbolId_assert_no_leak     = 0x67f739d1;
