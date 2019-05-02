// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Manages Symbols for SkookumScript
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>

//=======================================================================================
// Local Macros / Defines
//=======================================================================================

//=======================================================================================
// Global Variables
//=======================================================================================

//=======================================================================================
// Method Definitions
//=======================================================================================

namespace SkSymbol_Impl
{

//---------------------------------------------------------------------------------------
// Sk Params Symbol@!existing_id(Integer id) Symbol
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):  Conan Reis
static void mthd_ctor_existing_id(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Results are ignored for constructors
  scope_p->this_as<SkSymbol>() =
    ASymbol::create_existing(scope_p->get_arg<SkInteger>(SkArg_1));
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@!existing_str(String str) Symbol
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):  Conan Reis
static void mthd_ctor_existing_str(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
)
  {
  // Results are ignored for constructors
  scope_p->this_as<SkSymbol>() =
    ASymbol::create_existing(scope_p->get_arg<SkString>(SkArg_1));
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@add(String str) Symbol  / + operator
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_op_add(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkSymbol::new_instance(scope_p->this_as<SkSymbol>().create_add(
      scope_p->get_arg<SkString>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@append(String str) Symbol
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_op_add_assign(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  SkInstance * this_p = scope_p->get_this();

  this_p->as<SkSymbol>() = this_p->as<SkSymbol>().create_add(scope_p->get_arg<SkString>(SkArg_1));

  if (result_pp)
    {
    this_p->reference();
    *result_pp = this_p;
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params =
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      scope_p->this_as<SkSymbol>() == scope_p->get_arg<SkSymbol>(SkArg_1));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params ~=
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      scope_p->this_as<SkSymbol>() != scope_p->get_arg<SkSymbol>(SkArg_1));
    }
  }
    
//---------------------------------------------------------------------------------------
// Sk Params >
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkSymbol>() > scope_p->get_arg<SkSymbol>(SkArg_1)));
    }
  }
    
//---------------------------------------------------------------------------------------
// Sk Params >=
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkSymbol>() >= scope_p->get_arg<SkSymbol>(SkArg_1)));
    }
  }
    
//---------------------------------------------------------------------------------------
// Sk Params <
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkSymbol>() < scope_p->get_arg<SkSymbol>(SkArg_1)));
    }
  }
    
//---------------------------------------------------------------------------------------
// Sk Params <=
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
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
      (scope_p->this_as<SkSymbol>() <= scope_p->get_arg<SkSymbol>(SkArg_1)));
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params String() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_String(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(scope_p->this_as<SkSymbol>().as_str_dbg());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol() Symbol
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_Symbol(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = scope_p->get_this();
    (*result_pp)->reference();
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@id() Integer
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_id(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(scope_p->this_as<SkSymbol>().get_id());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@id_str() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_id_str(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(scope_p->this_as<SkSymbol>().as_id_str());
    }
  }

//---------------------------------------------------------------------------------------
// Sk Params Symbol@null?() Boolean
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
static void mthd_nullQ(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(scope_p->this_as<SkSymbol>().is_null());
    }
  }

//---------------------------------------------------------------------------------------

// Array listing all the above methods
static const SkClass::MethodInitializerFunc methods_i[] =
  {
    { "!existing_id",      mthd_ctor_existing_id },
    { "!existing_str",     mthd_ctor_existing_str },

    { "String",            mthd_String },
    { "Symbol",            mthd_Symbol },

    { "add",               mthd_op_add },
    { "add_assign",        mthd_op_add_assign },
    { "append",            mthd_op_add_assign },
    { "equal?",            mthd_op_equals },
    { "greater?",          mthd_op_greater },
    { "greater_or_equal?", mthd_op_greater_or_equal },
    { "less?",             mthd_op_less },
    { "less_or_equal?",    mthd_op_less_or_equal },
    { "not_equal?",        mthd_op_not_equal },

    { "id",                mthd_id },
    { "id_str",            mthd_id_str },
    { "null?",             mthd_nullQ },
  };

} // namespace

//---------------------------------------------------------------------------------------
void SkSymbol::register_bindings()
  {
  tBindingBase::register_bindings(ASymbolId_Symbol);

  ms_class_p->register_method_func_bulk(SkSymbol_Impl::methods_i, A_COUNT_OF(SkSymbol_Impl::methods_i), SkBindFlag_instance_no_rebind);
  }

//---------------------------------------------------------------------------------------

SkClass * SkSymbol::get_class()
  {
  return ms_class_p;
  }
