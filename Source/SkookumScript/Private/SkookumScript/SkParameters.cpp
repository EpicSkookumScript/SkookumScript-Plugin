// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Formal Parameter List/Interface Class
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkParameters.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkParameters.inl>
#endif


#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkGroupParam.hpp>
#include <SkookumScript/SkTyped.hpp>


//=======================================================================================
// Class Data
//=======================================================================================

uint32_t SkParameters::ms_param_count = 0u;
APSortedLogical<SkParameters> SkParameters::ms_shared_params;


//=======================================================================================
// Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Author(s):   Conan Reis
SkParameters::SkParameters() :
  // $Note - CReis It might be an idea to initialize this to nullptr so it is possible to
  // determine an "uninitialized" state
  m_result_type_p(SkBrain::ms_object_class_p)
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  }

//---------------------------------------------------------------------------------------
// Transfer copy constructor - takes over internal contents of supplied
//             'params_p' and sets it to default values.
// Returns:    itself
// Arg         params_p - parameters object to take over contents of
// Notes:      This method is useful when the contents of a local stack allocated
//             SkParameters object needs to be promoted to a dynamic heap allocated
//             object.
// Author(s):   Conan Reis
SkParameters::SkParameters(SkParameters * params_p) :
  m_params(&params_p->m_params),
  m_return_params(&params_p->m_return_params),
  m_result_type_p(params_p->m_result_type_p)
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  params_p->m_result_type_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Simple constructor
// Returns:    itself
// Arg         result_type_p - return class type
// Arg         param_p - optional single unary parameter - No parameter if nullptr.
// Notes:      This method is useful when the contents of a local stack allocated
//             SkParameters object needs to be promoted to a dynamic heap allocated
//             object.
// Author(s):   Conan Reis
SkParameters::SkParameters(
  SkClassDescBase * result_type_p,
  SkParameterBase * param_p
  ) :
  m_result_type_p(result_type_p)
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  if (param_p)
    {
    m_params.append(*param_p);
    }
  }

//---------------------------------------------------------------------------------------
// Destructor
// Examples:   called by system
// Notes:      Frees up dynamically allocated unary and group parameters
// Author(s):   Conan Reis
SkParameters::~SkParameters()
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  m_params.free_all();
  m_return_params.free_all();
  }

//---------------------------------------------------------------------------------------
// Logical Equals operator
//
// #Author(s) Conan Reis
bool SkParameters::operator==(const SkParameters & params) const
  {
  // First test if they are the same object in memory
  if (this == &params)
    {
    return true;
    }

  // Class types are unique so just compare addresses.
  if (m_result_type_p != params.m_result_type_p)
    {
    return false;
    }

  uint32_t param_count = m_params.get_length();

  if (param_count != params.m_params.get_length())
    {
    return false;
    }

  uint32_t rparam_count = m_return_params.get_length();

  if (rparam_count != params.m_return_params.get_length())
    {
    return false;
    }

  if (param_count)
    {
    SkParameterBase ** params_pp     = m_params.get_array();
    SkParameterBase ** params_end_pp = params_pp + param_count;
    SkParameterBase ** pparams_pp    = params.m_params.get_array();

    while ((params_pp < params_end_pp) && ((*params_pp)->compare_equal(**pparams_pp)))
      {
      params_pp++;
      pparams_pp++;
      }

    if (params_pp < params_end_pp)
      {
      return false;
      }
    }

  if (rparam_count)
    {
    SkTypedName ** rparams_pp     = m_return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + rparam_count;
    SkTypedName ** prparams_pp    = params.m_return_params.get_array();

    while ((rparams_pp < rparams_end_pp) && (**rparams_pp == **prparams_pp))
      {
      rparams_pp++;
      prparams_pp++;
      }

    if (rparams_pp < rparams_end_pp)
      {
      return false;
      }
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Logical less than operator
//
// #Author(s) Conan Reis
bool SkParameters::operator<(const SkParameters & params) const
  {
  uint32_t param_count  = m_params.get_length();
  uint32_t pparam_count = params.m_params.get_length();

  if (param_count != pparam_count)
    {
    return param_count < pparam_count;
    }

  uint32_t rparam_count  = m_return_params.get_length();
  uint32_t prparam_count = params.m_return_params.get_length();

  if (rparam_count != prparam_count)
    {
    return rparam_count < prparam_count;
    }

  eAEquate result = m_result_type_p->compare(*params.m_result_type_p);

  if (result != AEquate_equal) 
    {
    return result == AEquate_less;
    }

  if (param_count)
    {
    SkParameterBase ** params_pp     = m_params.get_array();
    SkParameterBase ** params_end_pp = params_pp + param_count;
    SkParameterBase ** pparams_pp    = params.m_params.get_array();

    while (params_pp < params_end_pp)
      {
      if (!(*params_pp)->compare_equal(**pparams_pp))
        {
        return (*params_pp)->compare_less(**pparams_pp);
        }

      params_pp++;
      pparams_pp++;
      }
    }

  if (rparam_count)
    {
    SkTypedName ** rparams_pp     = m_return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + rparam_count;
    SkTypedName ** prparams_pp    = params.m_return_params.get_array();

    while (rparams_pp < rparams_end_pp)
      {
      if (!(**rparams_pp == **prparams_pp))
        {
        return **rparams_pp < **prparams_pp;
        }

      rparams_pp++;
      prparams_pp++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Determines if this parameter list would be a valid invokable arg to the specified
// invokable parameter list parameter.
//
// See Also  SkInvokableClass: :is_class_type() 
// #Author(s) Conan Reis
bool SkParameters::is_valid_arg_to(const SkParameters & params) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If they are the same they are obviously compatible.
  if (this == &params)
    {
    return true;
    }


  // $Revisit - CReis It would be nice to return/store more info on a mismatch (like what
  // part did not match) for an improved user debugging experience rather than a simple 
  // "false" - did not match.
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check result type
  if (!m_result_type_p->is_class_type(params.m_result_type_p))
    {
    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check send parameters
  // - the runtime only assigns arguments by index not by type or name
  // - complicated due to potential group parameters (for example: group arguments are
  //   modified to List literals by the parser so parameter group/List match argument
  //   group/List interchangeably).
  uint32_t alength = m_params.get_length();  // Argument params length
  uint32_t plength = params.m_params.get_length();  // Parameter params length
  uint32_t aidx    = 0u;
  uint32_t pidx    = 0u;

  eSkParameter       akind;
  SkClassDescBase *  pclass_p;
  SkParameterBase *  aparam_p;
  SkParameterBase *  pparam_p;
  SkParameterBase ** aparams_pp = m_params.get_array();
  SkParameterBase ** pparams_pp = params.m_params.get_array();

  // Test if each arg passed to Parameter params will be valid for Argument params
  while (pidx < plength)
    {
    // Ensure that there are still Argument parameters to match
    if (aidx >= alength)
      {
      return false;
      }

    pparam_p = pparams_pp[pidx];
    aparam_p = aparams_pp[aidx];
    akind    = aparam_p->get_kind();

    switch (pparam_p->get_kind())
      {
      case SkParameter_unary:
        pclass_p = pparam_p->get_expected_type();

        if (akind != SkParameter_group)
          {
          // Argument parameter must be compatible unary parameter
          if (!pclass_p->is_class_type(aparam_p->get_expected_type()))
            {
            return false;
            }
          }
        else
          {
          // Match if Parameter param is compatible List type
          SkClassDescBase * item_class_p = pclass_p->get_item_type();

          if (item_class_p == nullptr)
            {
            return false;
            }

          SkGroupParam pgroup;
          pgroup.append_class(*item_class_p);

          if (!static_cast<SkGroupParam *>(aparam_p)->is_valid_arg_to(pgroup))
            {
            return false;
            }
          }
        break;

      case SkParameter_unary_default:
        // Argument parameter must be compatible unary parameter with default
        if ((akind != SkParameter_unary_default)
          || !pparam_p->get_expected_type()->is_class_type(aparam_p->get_expected_type()))
          {
          return false;
          }
        break;

      case SkParameter_group:
        // Argument parameter must be compatible group, compatible List or Object
        if (akind == SkParameter_group)
          {
          if (!static_cast<SkGroupParam *>(aparam_p)->is_valid_arg_to(
              *static_cast<SkGroupParam *>(pparam_p)))
            {
            return false;
            }
          }
        else
          {
          if (!pparam_p->get_expected_type()->is_class_type(aparam_p->get_expected_type()))
            {
            return false;
            }
          }

        break;
      }

    aidx++;
    pidx++;
    }

  // Ensure any remaining Argument params have a default
  while (aidx < alength)
    {
    if (!aparams_pp[aidx]->is_defaultable())
      {
      return false;
      }

    aidx++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check return parameters
  // - the runtime only assigns return arguments by index not by type or name
  alength = m_return_params.get_length();
  plength = params.m_return_params.get_length();

  // Ensure Argument params have enough return arguments
  // [The Parameter params return arguments could be set to nil though assume a mismatch.]
  if (alength < plength)
    {
    return false; 
    }

  aidx = 0u;

  if (plength)
    {
    SkTypedName ** arparams_pp = m_return_params.get_array();
    SkTypedName ** prparams_pp = params.m_return_params.get_array();

    while (pidx < plength)
      {
      if (!arparams_pp[pidx]->m_type_p->is_class_type(prparams_pp[pidx]->m_type_p))
        {
        return false;
        }

      pidx++;
      }
    }

  // Any extra Argument param return arguments can be ignored.

  return true;
  }

//---------------------------------------------------------------------------------------
uint32_t SkParameters::generate_crc32() const
  {
  uint32_t crc = m_result_type_p->generate_crc32();

  SkParameterBase ** params_pp = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + m_params.get_length();
  for (; params_pp < params_end_pp; ++params_pp)
    {
    crc = AChecksum::generate_crc32_uint32((*params_pp)->generate_crc32(), crc);
    }

  SkTypedName ** rparams_pp = m_return_params.get_array();
  SkTypedName ** rparams_end_pp = rparams_pp + m_return_params.get_length();
  for (; rparams_pp < rparams_end_pp; ++rparams_pp)
    {
    crc = AChecksum::generate_crc32_uint32((*rparams_pp)->generate_crc32(), crc);
    }

  return crc;
  }

//---------------------------------------------------------------------------------------
// Transfer copy assignment - takes over internal contents of supplied
//             'params_p' and sets it to default values.
// Returns:    itself
// Arg         params_p - parameters object to take over contents of
// Author(s):   Conan Reis
SkParameters & SkParameters::assign(SkParameters * params_p)
  {
  m_params.free_all();
  m_params.assign(&params_p->m_params);

  m_return_params.free_all();
  m_return_params.assign(&params_p->m_return_params);

  m_result_type_p = params_p->m_result_type_p;
  params_p->m_result_type_p = nullptr;

  return *this;
  }

//---------------------------------------------------------------------------------------
// Get expected result class type
// Returns:    result class type
// Author(s):   Conan Reis
SkClassDescBase * SkParameters::get_result_class() const
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  return m_result_type_p;
  }

//---------------------------------------------------------------------------------------
// Set expected result class type
// Author(s):   Conan Reis
void SkParameters::set_result_type(const SkClassDescBase & rclass)
  {
  // $Note - CReis This would be made inline, but it needs to know about SkClass and including
  // SkookumScript/SkClass.hpp in the .inl file leads to circular includes.
  m_result_type_p = const_cast<SkClassDescBase *>(&rclass);
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this parameter list and its components and increments the memory
//             address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               1 byte  - number of parameters (limits to 255)
//               n bytes - parameter binary }- Repeating
//               1 byte  - number of return parameters (limits to 255)
//               9*bytes - return parameter }- Repeating
//               5*bytes - return class type
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkParameters::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkParameters::as_binary_length());

  m_params.as_binary8(binary_pp);
  m_return_params.as_binary8(binary_pp);
  m_result_type_p->as_binary_ref_typed(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               1 byte  - number of parameters
//               n bytes - parameter binary }- Repeating
//               1 byte  - number of return parameters
//               9*bytes - return parameter }- Repeating
//               5*bytes - return class type
// Author(s):   Conan Reis
uint32_t SkParameters::as_binary_length() const
  {
  return m_params.as_binary_length8()
    + m_return_params.as_binary_length8()
    + m_result_type_p->as_binary_ref_typed_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               1 byte  - number of parameters
//               n bytes - parameter binary }- Repeating
//               1 byte  - number of return parameters
//               9*bytes - return parameter }- Repeating
//               5*bytes - return class type
//
//             Little error checking is done on the binary info as it assumed that it was
//             previously validated upon input.
// Author(s):   Conan Reis
SkParameters::SkParameters(const void ** binary_pp)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 1 byte - number of parameters
  uint32_t length = uint32_t(A_BYTE_STREAM_UI8_INC(binary_pp));

  m_params.free_all();
  m_params.set_size(length);

  SkParameterBase ** params_pp     = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + length;

  // n bytes - parameter binary }- Repeating
  while (params_pp < params_end_pp)
    {
    *params_pp = SkParameterBase::from_binary_new(binary_pp);
    params_pp++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 1 byte - number of return parameters
  length = uint32_t(A_BYTE_STREAM_UI8_INC(binary_pp));

  m_return_params.free_all();
  m_return_params.set_size(length);

  SkTypedName ** rparams_pp     = m_return_params.get_array();
  SkTypedName ** rparams_end_pp = rparams_pp + length;

  // 9 bytes - return parameter }- Repeating
  while (rparams_pp < rparams_end_pp)
    {
    *rparams_pp = SK_NEW(SkTypedName)(binary_pp);
    rparams_pp++;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 5*bytes - return class type
  m_result_type_p = SkClassDescBase::from_binary_ref_typed(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#ifdef SK_AS_STRINGS

//---------------------------------------------------------------------------------------
// Converts this into its source code string equivalent.  This is essentially
//             a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// Arg         return_type - indicates whether the return type should be included.
//             (Default true)
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Author(s):   Conan Reis
AString SkParameters::as_code(
  uint32_t str_flags // = StrFlag__default
  ) const
  {
  AString str(nullptr, 128u, 0u);

  str.append('(');

  bool names_only_b = (str_flags & StrFlag_names_only) != 0u;

  SkParameterBase ** parameters_pp     = m_params.get_array();
  SkParameterBase ** parameters_end_pp = parameters_pp + m_params.get_length();

  while (parameters_pp < parameters_end_pp)
    {
    str.append(names_only_b
      ? (*parameters_pp)->get_name_str_dbg()
      : (*parameters_pp)->as_code());
    parameters_pp++;

    if (parameters_pp < parameters_end_pp)
      {
      str.append(", ", 2u);
      }
    }

  uint32_t return_param_count = m_return_params.get_length();

  if (return_param_count)
    {
    str.append("; ", 2u);

    SkTypedName ** return_params_pp     = m_return_params.get_array();
    SkTypedName ** return_params_end_pp = return_params_pp + return_param_count;

    while (return_params_pp < return_params_end_pp)
      {
      if (!names_only_b)
        {
        str.append((*return_params_pp)->m_type_p->as_code());
        str.append(' ');
        }

      str.append((*return_params_pp)->get_name_str_dbg());

      return_params_pp++;

      if (return_params_pp < return_params_end_pp)
        {
        str.append(", ", 2u);
        }
      }
    }

  str.append(')');

  if (str_flags & StrFlag_return)
    {
    str.append(' ');
    str.append(m_result_type_p->as_code());
    }

  return str;
  }

#endif // SK_AS_STRINGS


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if any default parameter expression was located at or follows
//             the character index position provided and returns it.
// Returns:    The first expression that starts at or follows the given position or nullptr
//             if no such expression was found.
// Arg         source_idx - code file/string character index position
// See:        SkInvokeBase::find_expr_by_pos()
// Author(s):   Conan Reis
SkExpressionBase * SkParameters::find_expr_by_pos(
  uint         source_idx,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  SkExpressionBase * expr_p;
  SkExpressionBase * found_p;
  SkParameterBase ** params_pp     = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + m_params.get_length();

  for (; params_pp < params_end_pp; params_pp++)
    {
    expr_p = (*params_pp)->get_default_expr();

    if (expr_p)
      {
      found_p = expr_p->find_expr_by_pos(source_idx, type);

      if (found_p)
        {
        return found_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Iterates over this expression and any sub-expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
//
// #Modifiers virtual - override if expression has sub-expressions
// See Also  SkApplyExpressionBase, *: :iterate_expressions()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Returns AIterateResult_early_exit if iteration stopped/aborted early or
  // AIterateResult_entire if full iteration performed.
  eAIterateResult
SkParameters::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  SkExpressionBase * expr_p;
  SkParameterBase ** params_pp     = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + m_params.get_length();

  for (; params_pp < params_end_pp; params_pp++)
    {
    expr_p = (*params_pp)->get_default_expr();

    if (expr_p && expr_p->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }
    }

  return AIterateResult_entire;
  }

#endif  // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// Determine the minimum arguments (arity) accepted by this parameter list.
// 
// #Author(s)   Conan Reis
uint SkParameters::get_arg_count_min() const
  {
  uint32_t length = m_params.get_length();

  if (length == 0u)
    {
    return 0u;
    }

  uint                arg_min       = 0u;
  SkParameterBase ** params_pp     = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + length;

  for (; params_pp < params_end_pp; params_pp++)
    {
    if (!(*params_pp)->is_defaultable())
      {
      arg_min++;
      }
    }

  return arg_min;
  }

//---------------------------------------------------------------------------------------
// Determine the minimum arguments (arity) accepted by this parameter list
// not including the first argument.
// 
// #Notes
//   Useful in situations where initial argument is already supplied/available.
// 
// #Author(s)   Conan Reis
uint SkParameters::get_arg_count_min_after_arg1() const
  {
  uint32_t length = m_params.get_length();

  if (length <= 1u)
    {
    return 0u;
    }

  uint                arg_min       = 0u;
  SkParameterBase ** params_pp     = m_params.get_array();
  SkParameterBase ** params_end_pp = params_pp + length;

  params_pp++;

  for (; params_pp < params_end_pp; params_pp++)
    {
    if (!(*params_pp)->is_defaultable())
      {
      arg_min++;
      }
    }

  return arg_min;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkParameters::track_memory(AMemoryStats * mem_stats_p) const
  {
  uint32_t rparam_count = m_return_params.get_length();

  if (rparam_count)
    {
    mem_stats_p->track_memory(SKMEMORY_ARGS(SkTypedName, 0u), 0u, 0u, rparam_count);
    }

  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkParameters, 0u),
    (m_params.get_length() + rparam_count) * sizeof(void *),
    m_params.track_memory(mem_stats_p) + (rparam_count * sizeof(void *)));
  }

//---------------------------------------------------------------------------------------
//  Determines if last parameter is a required/non-defaulted closure.
//  Used to determine if closure tail invocation args are allowed or not.
//
// #Author(s) Conan Reis
bool SkParameters::is_last_closure() const
  {
  SkParameterBase * param_p = m_params.get_last();

  return param_p
    && !param_p->is_defaultable()
    && (param_p->get_expected_type()->is_class_type(SkBrain::ms_closure_class_p));
  }

//---------------------------------------------------------------------------------------
//  Determines if one or more parameters in this parameter list is a generic/reflective
//  class.  [Generic classes are: ThisClass_ and ItemClass_.  The Auto_ class is replaced
//  during parse as its type is determined via its surrounding context.]
//
// #Examples
//   (ThisClass_ arg)
//   // With String passed as a scope type becomes:
//   (String arg)
//
// #See Also  as_finalized_generic()
// #Author(s) Conan Reis
bool SkParameters::is_generic() const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check result type
  if (m_result_type_p->is_generic())
    {
    return true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check send parameters
  uint32_t length = m_params.get_length();

  if (length)
    {
    SkParameterBase ** params_pp     = m_params.get_array();
    SkParameterBase ** params_end_pp = params_pp + length;

    while (params_pp < params_end_pp)
      {
      if ((*params_pp)->is_generic())
        {
        return true;
        }

      params_pp++;
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check return parameters
  length = m_return_params.get_length();

  if (length)
    {
    SkTypedName ** rparams_pp     = m_return_params.get_array();
    SkTypedName ** rparams_end_pp = rparams_pp + length;

    while (rparams_pp < rparams_end_pp)
      {
      if ((*rparams_pp)->m_type_p->is_generic())
        {
        return true;
        }

      rparams_pp++;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
//  If one or more parameters in this parameter list is a generic/reflective class, it
//  a new parameter list will be returned with any generic/reflective classes replaced
//  with their finalized/specific class using scope_type as its scope.
//
// #Examples
//   (ThisClass_ arg)
//   // With String passed as a scope type becomes:
//   (String arg)
//
// See Also  is_generic(), SkInvokableClass: :as_finalized_generic()
// #Author(s) Conan Reis
SkParameters * SkParameters::as_finalized_generic(const SkClassDescBase & scope_type) const
  {
  if (is_generic())
    {
    SkParameters final_params;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Set result type
    final_params.m_result_type_p = m_result_type_p->as_finalized_generic(scope_type);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Set send parameters
    uint32_t length = m_params.get_length();

    if (length)
      {
      final_params.m_params.set_size(length);

      SkParameterBase ** fparams_pp    = final_params.m_params.get_array();
      SkParameterBase ** params_pp     = m_params.get_array();
      SkParameterBase ** params_end_pp = params_pp + length;

      while (params_pp < params_end_pp)
        {
        *fparams_pp = (*params_pp)->as_finalized_generic(scope_type);

        fparams_pp++;
        params_pp++;
        }
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Set return parameters
    length = m_return_params.get_length();

    if (length)
      {
      final_params.m_return_params.set_size(length);

      SkTypedName *  rparam_p;
      SkTypedName ** frparams_pp    = final_params.m_return_params.get_array();
      SkTypedName ** rparams_pp     = m_return_params.get_array();
      SkTypedName ** rparams_end_pp = rparams_pp + length;

      while (rparams_pp < rparams_end_pp)
        {
        rparam_p = *rparams_pp;
        *frparams_pp = SK_NEW(SkTypedName)(
          rparam_p->get_name(), rparam_p->m_type_p->as_finalized_generic(scope_type));

        frparams_pp++;
        rparams_pp++;
        }
      }

    return get_or_create(&final_params);
    }

  return const_cast<SkParameters *>(this);
  }

//---------------------------------------------------------------------------------------
// #Description
//   Creates new structure or finds an existing one to reference
//
// #Modifiers static
// #Author(s) Conan Reis
SkParameters * SkParameters::get_or_create(
  // parameters object to take over contents of
  SkParameters * params_p
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!params_p->is_sharable())
    {
    ms_param_count++;

    // Parameters that cannot be shared are just created dynamically.
    return SK_NEW(SkParameters)(params_p);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for existing matching parameters
  uint32_t       idx            = 0u;
  SkParameters * shared_param_p = ms_shared_params.get(*params_p, AMatch_first_found, &idx);

  if (shared_param_p)
    {
    // Found it!
    return shared_param_p;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a new dynamic parameters to share
  shared_param_p = SK_NEW(SkParameters)(params_p);

  // Give it extra reference for being stored in shared list.
  shared_param_p->reference();
  ms_shared_params.insert(*shared_param_p, idx);

  return shared_param_p;
  }

//---------------------------------------------------------------------------------------
// Ensures that all the globally available parameters are referenced.
//             Parsing may create some temporary parameters - this method frees them from
//             memory.
// Modifiers:   static
// Author(s):   Conan Reis
bool SkParameters::shared_ensure_references()
  {
  bool anything_changed = false;

  SkParameters ** params_pp     = ms_shared_params.get_array();
  SkParameters ** params_end_pp = params_pp + ms_shared_params.get_length();

  // Traverse in reverse order to ensure a free can occur without a skip.
  while (params_pp < params_end_pp)
    {
    params_end_pp--;

    // Shared list gives a reference so reference count of 1 means it isn't used
    if ((*params_end_pp)->m_ref_count == 1u)
      {
      ms_shared_params.free(uint32_t(params_end_pp - params_pp));
      anything_changed = true;
      }
    }

  // If empty, get rid of memory
  if (ms_shared_params.is_empty())
    {
    ms_shared_params.compact();
    }

  return anything_changed;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by the shared parameters.
// See:        SkDebug, AMemoryStats
// Modifiers:   static
// Author(s):   Conan Reis
void SkParameters::shared_track_memory(AMemoryStats * mem_stats_p)
  {
  // Note that the array buffer is added as dynamic memory
  ms_shared_params.track_memory_and_array(mem_stats_p, "SkParameters");
  }
