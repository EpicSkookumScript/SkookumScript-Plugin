// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class Type Scope Context
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkTypedContext.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkTypedContext.inl>
#endif
#include <AgogCore/AVCompactSorted.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// SkTypeContext Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// If type_p is a generic/reflective class, it will be replaced with its
//             finalized/specific class using the current object scope.
//             For example: "ThisClass_" could become "String"
// Returns:    Finalized non-generic class - returns type_p if type_p is already not a
//             generic class.
// Arg         type_p - class type to finalize if it is generic.
// Notes:      Would inline though requires SkClass.hpp to be included which causes problems.
// Author(s):   Conan Reis
SkClassDescBase * SkTypeContext::finalize_generic(const SkClassDescBase & type) const
  {
  return type.as_finalized_generic(*m_obj_scope_p);
  }

//---------------------------------------------------------------------------------------
// Starts capturing used local temporary variables until capture_locals_stop()
//             is called.  Appends a new capture variable nest level and makes it the 
//             current scope.
// See:        capture_locals_stop(), nest_locals()
// Author(s):   Conan Reis
void SkTypeContext::capture_locals_start()
  {
  // $Revisit - CReis [Memory] These structures should come from a memory pool
  m_capture_current_p = SK_NEW(CapturedVars)(m_current_vars_p);
  m_capture_stack.append(m_capture_current_p);

  nest_locals(SkNestReason_invocation);
  }

//---------------------------------------------------------------------------------------
// Stops capturing used local temporary variables previously started when
//             capture_locals() was called.  Removes capture variable nest level.
// See:        capture_locals(), unnest_locals()
// Author(s):   Conan Reis
void SkTypeContext::capture_locals_stop(tSkIndexedNames * captured_p)
  {
  if (captured_p)
    {
    uint32_t captured_count = m_capture_current_p->m_vars.get_length();

    // Fix up captured temp variable indices
    // and allow for captured variables to be stored ahead of parameters and locals
    for (auto indexed_p : m_capture_current_p->m_indices_to_patch)
      {
      if (indexed_p.is_valid())
        {
        indexed_p->set_data_idx(indexed_p->get_data_idx() + captured_count);
        }
      }

    // Fix up captured identifiers
    // and allow for captured variables to be stored ahead of parameters and locals
    for (auto indexed_p : m_capture_current_p->m_named_indices_to_patch)
      {
      if (indexed_p.is_valid())
        {
        // data_idx is either the array position of the captured variable ...
        uint32_t data_idx = 0;
        if (!m_capture_current_p->m_vars.get(indexed_p->get_name(), AMatch_first_found, &data_idx))
          {
          // ... or the adjusted original data index
          data_idx = indexed_p->get_data_idx() + captured_count;
          SK_ASSERTX(data_idx >= captured_count && data_idx < m_current_scope_p->m_data_idx_count_max + captured_count, "Bad data index!");
          }
        indexed_p->set_data_idx(data_idx);
        }
      }

    captured_p->empty_ensure_count_undef(captured_count);

    if (captured_count)
      {
      SkTypedNameIndexed ** vars_pp     = m_capture_current_p->m_vars.get_array();
      SkTypedNameIndexed ** vars_end_pp = vars_pp + captured_count;

      while (vars_pp < vars_end_pp)
        {
        captured_p->append_last_undef(SkNamedIndexed((*vars_pp)->get_name(), (*vars_pp)->m_data_idx));
        vars_pp++;
        }

      // Are we capturing inside another capture?
      if (m_capture_current_p != m_capture_stack.get_first())
        {
        // Yes, add our captured vars to the patch list of the parent capture so they get properly adjusted
        CapturedVars * parent_capture_p = m_capture_stack.get_penultimate();
        for (SkNamedIndexed & indexed : *captured_p)
          {
          parent_capture_p->m_named_indices_to_patch.append(AIdPtr<SkNamedIndexed>(&indexed));
          }
        }
      }
    }

  // Set to next nesting capture if any
  m_capture_stack.free_last();
  m_capture_current_p = m_capture_stack.get_last_null();

  unnest_locals(SkUnnestAction_accept);
  }

//---------------------------------------------------------------------------------------
// Get notified of the creation of a data index
void SkTypeContext::on_local_data_index_created(SkIndexed * indexed_p)
  {
  if (m_capture_current_p)
    {
    // Remember temp variable indices used during capture
    m_capture_current_p->m_indices_to_patch.append(AIdPtr<SkIndexed>(indexed_p));
    }
  }

//---------------------------------------------------------------------------------------
// Get notified of the creation of an identifier
void SkTypeContext::on_identifier_created(SkIdentifierLocal * identifier_p)
  {
  if (m_capture_current_p)
    {
    // Remember identifiers created during capture
    m_capture_current_p->m_named_indices_to_patch.append(AIdPtr<SkNamedIndexed>(identifier_p));
    }
  }

//---------------------------------------------------------------------------------------
// Change the type of a variable in the current scope.
// Arg         var_name - variable name to find
// Arg         type_p - new type for variable to use
// See:        get_member_type()
// Author(s):   Conan Reis
void SkTypeContext::change_variable_type(const ASymbol & var_name, SkClassDescBase * type_p)
  {
  SkTypedName * tname_p = m_current_vars_p->get(var_name);

  if (tname_p)
    {
    tname_p->m_type_p = type_p;
    }
  else
    {
    // Variable originated in a higher nesting scope.
    // Put copy of variable in current scope if the type has changed.
    uint32_t data_idx = 0;
    bool is_return_arg = false;
    if (get_variable_type(var_name, false, &data_idx, &is_return_arg) != type_p)
      {
      // $Revisit - CReis [Memory] These structures should come from a memory pool
      m_current_vars_p->append(*SK_NEW(SkTypedNameIndexed)(var_name, type_p, data_idx, is_return_arg));
      }
    }
  }

//---------------------------------------------------------------------------------------
// Akin to calling change_variable_type() with successive variable name and
//             type pairs.
// Arg         vars - variable name and type pairs to change
// See:        nest_locals(), unnest_locals(), merge_locals()
// Author(s):   Conan Reis
void SkTypeContext::change_variable_types(const tSkTypedNamesIndexed & vars)
  {
  SkTypedNameIndexed ** vars_pp     = vars.get_array();
  SkTypedNameIndexed ** vars_end_pp = vars_pp + vars.get_length();

  for (; vars_pp < vars_end_pp; vars_pp++)
    {
    change_variable_type((*vars_pp)->get_name(), (*vars_pp)->m_type_p);
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   [Internal]  Tracks captured variable from specified nesting capture level on down.
//
// #Notes
//   const member so that other const members may call it.
//
// #Author(s) Conan Reis
void SkTypeContext::capture_local(
  SkTypedNameIndexed * var_p,
  CapturedVars * top_nesting_p
  ) const
  {
  CapturedVars * vars_p = m_capture_stack.get_first_null();
  bool           track  = false;

  while (vars_p)
    {
    if (vars_p == top_nesting_p)
      {
      track = true;
      }

    if (track)
      {
      vars_p->m_vars.append_absent(*var_p);
      }

    vars_p = m_capture_stack.get_next_null(vars_p);
    }
  }

//---------------------------------------------------------------------------------------
// Determines if the specified local variable (parameter or temporary) exists within this
// context.
//
// #See Also  get_variable_type(), get_member_type()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if the variable exists locally (parameter or temporary) or false if not
  SkTypedNameIndexed *
SkTypeContext::find_local_variable(const ASymbol & var_name, SkTypedNameIndexed ** duplicate_var_pp) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Search local scope
  SkTypedNameIndexed *  var_p;
  SkTypedNameIndexed *  found_var_p = nullptr;
  int32_t               invocation_level = 0; // We use this to determine if duped variables occur within the same invocation which is ok
  int32_t               found_invocation_level = 0;
  ScopeVars *           scope_p = m_scope_stack.get_first(); // Iterate from outer/top to inner/bottom
  CapturedVars *        capture_top_p = m_capture_stack.get_first_null();

  if (capture_top_p == nullptr)
    {
    // Not capturing
    do
      {
      // Count invocations on the stack
      invocation_level += int32_t(scope_p->m_nest_reason == SkNestReason_invocation);

      // Find variable in this scope
      var_p = scope_p->m_vars.get(var_name);

      if (var_p)
        {
        if (!found_var_p)
          {
          found_var_p = var_p;
          found_invocation_level = invocation_level;
          }
        else if (duplicate_var_pp && invocation_level != found_invocation_level)
          {          
          // Record duped variable if it occurs on a different invocation level
          *duplicate_var_pp = var_p;
          }
        }

      scope_p = m_scope_stack.get_next_null(scope_p);
      }
    while (scope_p);
    }
  else
    {
    // We're parsing a closure and capturing variables from the surrounding scope
    do
      {
      // Count invocations on the stack
      invocation_level += int32_t(scope_p->m_nest_reason == SkNestReason_invocation);

      // Find variable in this scope
      var_p = scope_p->m_vars.get(var_name);

      if (var_p && !var_p->m_is_return_arg) // Do not capture return args
        {
        if (!found_var_p)
          {
          if (capture_top_p)
            {
            capture_local(var_p, capture_top_p);
            }

          found_var_p = var_p;
          found_invocation_level = invocation_level;
          }
        else if (duplicate_var_pp && invocation_level != found_invocation_level)
          {
          // Record duped variable if it occurs on a different invocation level
          *duplicate_var_pp = var_p;
          }
        }

      // Skip variables contained in the scope of the closure itself
      if (capture_top_p && (capture_top_p->m_scope_p == &scope_p->m_vars))
        {
        capture_top_p = m_capture_stack.get_next_null(capture_top_p);
        }

      scope_p = m_scope_stack.get_next_null(scope_p);
      }
    while (scope_p);
    }

  return found_var_p;
  }

//---------------------------------------------------------------------------------------
// Determines if the specified variable exists within this context.
//
// #See Also  get_variable_type(), get_member_type()
// #Author(s) Conan Reis
bool SkTypeContext::is_variable(const ASymbol & var_name) const
  {
  // Search local variables for variable from innermost/most recent scope to outermost
  // scope.
  ScopeVars * scope_p = m_current_scope_p;

  while (scope_p)
    {
    if (scope_p->m_vars.get(var_name))
      {
      return true;
      }

    scope_p = m_scope_stack.get_prev_null(scope_p);
    }

  // Search member variables
  return m_obj_scope_p->get_data_type(var_name) != nullptr;
  }

//---------------------------------------------------------------------------------------

bool SkTypeContext::is_captured_variable(const ASymbol & var_name) const
  {
  return m_capture_current_p && m_capture_current_p->m_vars.find(var_name);
  }

//---------------------------------------------------------------------------------------
// Transfers the typed local variables with the specified names from the
//             current scope to the local variable history.
// Arg         var_names - names of variables to store in history
// See:        free_all_locals(), empty_locals()
// Author(s):   Conan Reis
void SkTypeContext::archive_locals(const AVCompactArrayBase<ASymbol> & var_names)
  {
  uint32_t var_count = var_names.get_count();

  if (var_count)
    {
    m_current_scope_p->m_data_idx_count -= var_count;

    SkTypedName * tname_p;
    ASymbol *     syms_p     = var_names.get_array();
    ASymbol *     syms_end_p = syms_p + var_count;

    tSkTypedNames * var_history_p = &m_current_scope_p->m_var_history;

    while (syms_p < syms_end_p)
      {
      tname_p = m_current_vars_p->pop(*syms_p);

      SK_ASSERTX(tname_p, "Local variable not found in current scope!");

      if (tname_p && !var_history_p->append_absent(*tname_p))
        {
        delete tname_p;
        }

      syms_p++;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Removes the typed local variables with the specified names from the current scope.
// Arg         var_names - names of variables to free
// See:        free_all_locals(), empty_locals()
// Author(s):   Conan Reis
void SkTypeContext::free_locals(const AVCompactArrayBase<ASymbol> & var_names)
  {
  uint32_t var_count = var_names.get_count();
  m_current_scope_p->m_data_idx_count -= var_count;
  A_VERIFYX(m_current_vars_p->free_all(var_names.get_array(), var_names.get_count()) == var_count, "Not all local variables found in current scope!");
  }

//---------------------------------------------------------------------------------------
// Removes the parameters with the specified names from the current scope.
void SkTypeContext::free_locals(const tSkParamList & param_names)
  {
  m_current_scope_p->m_data_idx_count -= param_names.get_length();
  for (SkParameterBase * param_p : param_names)
    {
    A_VERIFYX(m_current_vars_p->free(param_p->get_name()), "Variable not found!");
    }
  }

//---------------------------------------------------------------------------------------

void SkTypeContext::get_names_of_all_locals(AVArray<ASymbol> * out_names_p) const
  {
  out_names_p->empty();

  // Iterate from outer/top to inner/bottom
  for (ScopeVars * scope_p = m_scope_stack.get_first(); scope_p; scope_p = m_scope_stack.get_next_null(scope_p))
    {
    for (SkTypedNameIndexed * var_p : scope_p->m_vars)
      {
      out_names_p->append(*var_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Return type of specified return parameter if in the middle of a routine with the
// parameter interface specified and if var_name is a valid return parameter.
//
// #Author(s) Conan Reis
SkClassDescBase * SkTypeContext::get_rparam_type(const ASymbol & var_name) const
  {
  if (m_params_p)
    {
    SkTypedName * rparam_p = m_params_p->get_param_return_list().get(var_name);

    if (rparam_p)
      {
      return rparam_p->m_type_p;
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// If the specified variable exists within this context, its type is returned
//             otherwise nullptr is returned.
// Returns:    type of variable or nullptr if it does not exist
// Arg         var_name - variable name to find
// Arg         skip_current_scope_b - if true the current scope is skipped
// See:        get_variable_type()
// Author(s):   Conan Reis
SkClassDescBase * SkTypeContext::get_local_variable_type(
  const ASymbol & var_name
  ) const
  {
  // Search local variables for variable from innermost scope to outermost scope.
  ScopeVars * scope_p = m_scope_stack.get_last();

  SkTypedName * tname_p;

  while (scope_p)
    {
    tname_p = scope_p->m_vars.get(var_name);

    if (tname_p)
      {
      // $Revisit - CReis [Redundant?]  This may be unnecessary if local types can never be
      // generic or if they are converted when added.
      return tname_p->m_type_p->as_finalized_generic(*m_obj_scope_p);
      }

    scope_p = m_scope_stack.get_prev_null(scope_p);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// If the specified variable exists within this context, its type is returned
//             otherwise nullptr is returned.
// Returns:    type of variable or nullptr if it does not exist
// Arg         var_name - variable name to find
// Arg         skip_current_scope_b - if true the current scope is skipped
// See:        get_variable_type()
// Author(s):   Conan Reis
SkClassDescBase * SkTypeContext::get_variable_type(
  const ASymbol & var_name,
  bool            skip_current_scope_b, // = false
  uint32_t *      data_idx_p,           // = nullptr
  bool *          is_return_arg_p       // = nullptr
  ) const
  {
  // Search local variables for variable from innermost scope to outermost scope.
  ScopeVars * scope_p = m_scope_stack.get_last();

  if (skip_current_scope_b)
    {
    scope_p = m_scope_stack.get_prev_null(scope_p);
    }

  SkTypedNameIndexed * tname_p;

  while (scope_p)
    {
    tname_p = scope_p->m_vars.get(var_name);

    if (tname_p)
      {
      if (data_idx_p)
        {
        *data_idx_p = tname_p->m_data_idx;
        }

      if (is_return_arg_p)
        {
        *is_return_arg_p = tname_p->m_is_return_arg;
        }

      // $Revisit - CReis [Redundant?]  This may be unnecessary if local types can never be
      // generic or if they are converted when added.
      return tname_p->m_type_p->as_finalized_generic(*m_obj_scope_p);
      }

    scope_p = m_scope_stack.get_prev_null(scope_p);
    }

  // If we get here, there is no data index, so just set to harmless 0
  if (data_idx_p)
    {
    *data_idx_p = 0;
    }

  // If we get here, there are no arguments, so say it's not a return argument
  if (is_return_arg_p)
    {
    *is_return_arg_p = false;
    }

  // Search member variables
  SkTypedName * type_p = m_obj_scope_p->get_data_type(var_name);

  return type_p
    ? type_p->m_type_p->as_finalized_generic(*m_obj_scope_p)
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the specified variable *ever* existed within this context (including
// current scope and no longer in scope).
//
// #See Also  is_variable(), get_variable_type(), get_member_type()
// #Author(s) Conan Reis
bool SkTypeContext::is_previous_variable(const ASymbol & var_name) const
  {
  // Search local variables for variable from innermost/most recent scope to outermost
  // scope.
  ScopeVars * scope_p = m_current_scope_p;

  while (scope_p)
    {
    if (scope_p->m_vars.get(var_name) || scope_p->m_var_history.get(var_name))
      {
      return true;
      }

    scope_p = m_scope_stack.get_prev_null(scope_p);
    }

  // Search member variables
  return m_obj_scope_p->get_data_type(var_name) != nullptr;
  }

//---------------------------------------------------------------------------------------
// Used with alternate code paths - Merge variables in merge_vars_p with
//             their type from this context.
// See:        merge_locals(), nest_locals(), unnest_locals(), change_variable_types()
// Author(s):   Conan Reis
void SkTypeContext::merge(tSkTypedNamesIndexed * merge_vars_p) const
  {
  SkTypedNameIndexed *   var_p;
  SkTypedNameIndexed **  vars_pp     = merge_vars_p->get_array();
  SkTypedNameIndexed **  vars_end_pp = vars_pp + merge_vars_p->get_length();

  for (; vars_pp < vars_end_pp; vars_pp++)
    {
    var_p = *vars_pp;

    // Merge variable was not in current scope, so merge with inherited type
    var_p->m_type_p = SkClassUnion::get_merge(*var_p->m_type_p, *get_variable_type(var_p->get_name()));
    }
  }

//---------------------------------------------------------------------------------------
// Used with alternate code paths - mixes the variables in the current scope
//             with any variables already in place in merge_vars_p.
// Arg         merge_vars_p - variables to mix this context with.
// Arg         first_path_b - true if this is the first alternate path, false if not
// See:        merge(), nest_locals(), unnest_locals(), change_variable_types()
// Author(s):   Conan Reis
void SkTypeContext::merge_locals(
  tSkTypedNamesIndexed * merge_vars_p,
  bool                   first_path_b
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Merge variables in merge_vars_p that do not appear in the current scope with their
  // previous value from the nesting scope.
  SkTypedNameIndexed *   mvar_p;
  SkTypedNameIndexed *   lvar_p;
  const ASymbol *        name_p;
  tSkTypedNamesIndexed * locals_p    = m_current_vars_p;  // Cached for speed
  SkTypedNameIndexed **  vars_pp     = merge_vars_p->get_array();
  SkTypedNameIndexed **  vars_end_pp = vars_pp + merge_vars_p->get_length();

  for (; vars_pp < vars_end_pp; vars_pp++)
    {
    mvar_p = *vars_pp;
    name_p = &mvar_p->get_name();
    lvar_p = locals_p->get(*name_p);

    if (lvar_p == nullptr)
      {
      // Merge variable was not in current scope, so merge with inherited type
      mvar_p->m_type_p = SkClassUnion::get_merge(*mvar_p->m_type_p, *get_variable_type(*name_p, true));
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Merge variables in the current scope with merge_vars_p
  vars_pp     = locals_p->get_array();
  vars_end_pp = vars_pp + locals_p->get_length();

  for (; vars_pp < vars_end_pp; vars_pp++)
    {
    lvar_p = *vars_pp;
    name_p = &lvar_p->get_name();
    mvar_p = merge_vars_p->get(*name_p);

    if (mvar_p)
      {
      // Merge type with existing type from alternate path
      mvar_p->m_type_p = SkClassUnion::get_merge(*mvar_p->m_type_p, *lvar_p->m_type_p);
      }
    else
      {
      // $Revisit - CReis [Memory] These structures should come from a memory pool
      mvar_p = SK_NEW(SkTypedNameIndexed)(
        *name_p,
        first_path_b
          ? (SkClassDescBase *)lvar_p->m_type_p
          : SkClassUnion::get_merge(*lvar_p->m_type_p, *get_variable_type(*name_p, true)),
        lvar_p->m_data_idx,
        lvar_p->m_is_return_arg
        );
      merge_vars_p->append(*mvar_p);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Like unnest_locals() though keeps any type changes.
// Transfers all the type changes in the current nest level to the previous nest level
// then removes the current level and makes previous nest level the current.
//
// #See Also  nest_locals(), unnest_locals(), change_variable_types, merge_locals()
// #Author(s) Conan Reis
void SkTypeContext::accept_nest()
  {
  // Transfer type changes
  ScopeVars * outer_scope_p = m_scope_stack.get_penultimate();

  outer_scope_p->m_vars.append_replace_free_all(*m_current_vars_p);
  m_current_vars_p->empty();

  // Remove nest
  unnest_locals(SkUnnestAction_accept);
  }
