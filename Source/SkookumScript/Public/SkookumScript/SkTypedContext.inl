// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class Type Scope Context
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AMath.hpp>

//=======================================================================================
// SkTypeContext Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default Constructor
// Author(s):   Conan Reis
A_INLINE SkTypeContext::SkTypeContext() :
  m_obj_scope_p(nullptr),
  m_params_p(nullptr),
  m_top_scope(SkNestReason_invocation, 0),
  m_current_scope_p(&m_top_scope),
  m_current_vars_p(&m_top_scope.m_vars),
  m_capture_current_p(nullptr)
  {
  m_scope_stack.append(&m_top_scope);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
A_INLINE SkTypeContext::~SkTypeContext()
  {
  m_scope_stack.remove(&m_top_scope);
  m_scope_stack.free_all();
  m_capture_stack.free_all();
  }

//---------------------------------------------------------------------------------------
// Appends a local typed variable name to the context.
// Returns runtime data index
// Arg         tname_p - typed variable name to add
// See:        free_all_locals(), empty_locals()
// Author(s):   Conan Reis
A_INLINE uint32_t SkTypeContext::append_local(
  const ASymbol &   var_name,
  SkClassDescBase * type_p,
  bool              is_return_arg
  )
  {
  // Compute data index
  uint32_t data_idx = m_current_scope_p->m_data_idx_count++;

  // Remember high water mark
  m_current_scope_p->m_data_idx_count_max = a_max(m_current_scope_p->m_data_idx_count_max, m_current_scope_p->m_data_idx_count);

  // $Revisit - CReis [Memory] These structures should come from a memory pool
  m_current_scope_p->m_vars.append(*SK_NEW(SkTypedNameIndexed)(var_name, type_p, data_idx, is_return_arg));

  return data_idx;
  }

//---------------------------------------------------------------------------------------
// Frees (deletes) the typed local variables from the current scope and all
//             the old local history.
// Arg         var_names - names of variables to free
// See:        free_all_locals(), empty_locals()
// Author(s):   Conan Reis
A_INLINE void SkTypeContext::free_all_locals()
  {
  m_top_scope.empty();
  m_scope_stack.remove(&m_top_scope);
  m_scope_stack.free_all();
  m_scope_stack.append(&m_top_scope);
  m_params_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines if the current scope has any variables.
//             [Yes, the method name is poor grammar.]
// Author(s):   Conan Reis
A_INLINE bool SkTypeContext::is_locals() const
  {
  return m_current_vars_p->is_filled();
  }

//---------------------------------------------------------------------------------------
// Appends a new scope nest level and makes it the current scope.
// See:        unnest_locals(), accept_nest(), change_variable_types, merge_locals()
// Author(s):   Conan Reis
A_INLINE void SkTypeContext::nest_locals(eSkNestReason nest_reason)
  {
  // $Revisit - CReis [Memory] These structures should come from a memory pool
  ScopeVars * inner_scope_p = SK_NEW(ScopeVars)(nest_reason, nest_reason == SkNestReason_invocation ? 0 : m_current_scope_p->m_data_idx_count);

  m_current_scope_p = inner_scope_p;
  m_current_vars_p = &inner_scope_p->m_vars;
  m_scope_stack.append(inner_scope_p);
  }

//---------------------------------------------------------------------------------------
// Removes a scope nest level and makes the previous nest level the current
//             scope.
// See:        nest_locals(), accept_nest(), change_variable_types, merge_locals()
// Author(s):   Conan Reis
A_INLINE void SkTypeContext::unnest_locals(eSkUnnestAction unnest_action)
  {
  ScopeVars * inner_scope_p = m_scope_stack.pop_last();
  ScopeVars * outer_scope_p = m_scope_stack.get_last();

  m_current_scope_p = outer_scope_p;
  m_current_vars_p = &outer_scope_p->m_vars;

  if (inner_scope_p->m_nest_reason == SkNestReason_exploratory 
   && unnest_action == SkUnnestAction_accept)
    {
    // Remember past variables
    if (inner_scope_p->m_var_history.is_filled())
      {
      outer_scope_p->m_var_history.xfer_absent_all_free_dupes(&inner_scope_p->m_var_history);
      }

    // Also remember data indices
    outer_scope_p->m_data_idx_count = inner_scope_p->m_data_idx_count;
    outer_scope_p->m_data_idx_count_max = a_max(outer_scope_p->m_data_idx_count_max, inner_scope_p->m_data_idx_count_max);
    }

  delete inner_scope_p;
  }
