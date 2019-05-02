// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class Type Scope Context
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AList.hpp>
#include <AgogCore/AVArray.hpp>
#include <SkookumScript/SkNamed.hpp>
#include <SkookumScript/SkTyped.hpp>
#include <SkookumScript/SkParameters.hpp>

//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkClassUnaryBase;
class SkExpressionBase;
class SkInstance;
class SkIdentifierLocal;
class SkParameters;

#ifdef A_PLAT_PC
  template<class _ElementType> class AVCompactArrayBase;
  template<class _ElementType, class _KeyType = _ElementType> class AVCompactSortedLogical;
#else
  #include <AgogCore/AVCompactSorted.hpp>
#endif

typedef AVArrayLogical<AIdPtr<SkIndexed>, ASymbol>      tSkIndexedArray;
typedef AVArrayLogical<AIdPtr<SkNamedIndexed>, ASymbol> tSkNamedIndexedArray;

//---------------------------------------------------------------------------------------
// Why we are nesting
enum eSkNestReason
  {
  SkNestReason_invocation,   // The nested block will be invoked in its own invoked context scope
  SkNestReason_exploratory,  // The nested block is in the same invoked context scope as its parent scope, but might get discarded when backtracking
  };

//---------------------------------------------------------------------------------------
// How we are unnesting
enum eSkUnnestAction
  {
  SkUnnestAction_accept,  // The nested block will be invoked in its own invoked context scope
  SkUnnestAction_reject,  // The nested block is in the same invoked context scope as its parent scope, but might get discarded when backtracking
  };

//---------------------------------------------------------------------------------------
// Notes      Class Type Scope Context - used to track variable scope & type when
//            parsing/compiling.
// Author(s)  Conan Reis
struct SK_API SkTypeContext
  {
  // Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Variables (and member variables that have their type changed) and their class types
    // for a specific scope (invoked context) level.
    struct ScopeVars : public AListNode<ScopeVars>
      {
      // Public Data Members

        // Why we are nesting - invocation or backtracking
        eSkNestReason m_nest_reason;

        // Available/type-modified variables at a particular scope.
        tSkTypedNamesIndexed m_vars;

        // Current number of data indices in use
        uint32_t m_data_idx_count;

        // Maximum number of data indices concurrently in use during lifetime of this scope
        uint32_t m_data_idx_count_max;

        // Variables that are no longer available - used to ensure unique names across scopes.
        tSkTypedNames m_var_history;

      // Methods

        SK_NEW_OPERATORS(ScopeVars);

        ScopeVars(eSkNestReason nest_reason, uint32_t data_idx_count) : m_nest_reason(nest_reason), m_data_idx_count(data_idx_count), m_data_idx_count_max(data_idx_count) {}
        ~ScopeVars() { empty(); }

        void empty() { m_vars.free_all(); m_var_history.free_all(); }

      };
  
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Tracking info for a set of variables captured during the parse of a closure.
    struct CapturedVars : public AListNode<CapturedVars>
      {
      // Public Data Members

        // Variables captured by this closure - note that data members do not need to be
        // captured - just 'this'.
        tSkTypedNamesIndexed m_vars;

        // Parent scope at which the capture was created
        tSkTypedNamesIndexed * m_scope_p;

        // Keep track of any indices used during this capture session
        tSkIndexedArray       m_indices_to_patch;
        tSkNamedIndexedArray  m_named_indices_to_patch;

        // Methods

        SK_NEW_OPERATORS(CapturedVars);

        CapturedVars(tSkTypedNamesIndexed * scope_p) : m_scope_p(scope_p) {}

      };
  

  // Public Data Members

    // Class type scope - used to retrieve member data and methods
    SkClassUnaryBase * m_obj_scope_p;

    // If in a routine the parameters that are being used otherwise nullptr.
    // Used to determine the desired type for a return parameter.
    SkParameters * m_params_p;

    // Scope name - member/etc.
    ASymbol m_scope_name;

    // Initial variable scope on the stack - namely the arguments & temporaries created
    // within the topmost code block or single path nested code blocks.
    ScopeVars m_top_scope;

    // Stack of typed variables - local variables and any member variables that have had
    // their types changed.  Class types of variables can change throughout the life of a
    // code block.  A new variable scope is placed on the stack each time there are
    // alternate code paths to follow.
    AList<ScopeVars> m_scope_stack;

    // The current scope 
    ScopeVars * m_current_scope_p;
    tSkTypedNamesIndexed * m_current_vars_p;

    // Captured temporary variables stack
    AList<CapturedVars> m_capture_stack;

    // Innermost capture context - if non-nullptr then there are one or more capture contexts.
    CapturedVars * m_capture_current_p;

  // Methods

    SK_NEW_OPERATORS(SkTypeContext);

    SkTypeContext();
    ~SkTypeContext();

    SkClassDescBase * finalize_generic(const SkClassDescBase & type) const;

    // Local Data Methods - variables from arguments and code block temporaries

      SkClassDescBase * get_rparam_type(const ASymbol & var_name) const;
      SkClassDescBase * get_local_variable_type(const ASymbol & var_name) const;

      uint32_t              append_local(const ASymbol & var_name, SkClassDescBase * type_p, bool is_return_arg);
      void                  archive_locals(const AVCompactArrayBase<ASymbol> & var_names);
      SkTypedNameIndexed *  find_local_variable(const ASymbol & var_name, SkTypedNameIndexed ** duplicate_var_pp = nullptr) const;
      void                  free_locals(const AVCompactArrayBase<ASymbol> & var_names);
      void                  free_locals(const tSkParamList & param_names);
      void                  free_all_locals();
      void                  get_names_of_all_locals(AVArray<ASymbol> * out_names_p) const;
      bool                  is_locals() const;
      void                  merge(tSkTypedNamesIndexed * merge_vars_p) const;
      void                  merge_locals(tSkTypedNamesIndexed * merge_vars_p, bool first_path_b) const;
      void                  capture_locals_start();
      void                  capture_locals_stop(tSkIndexedNames * captured_p);
      void                  on_local_data_index_created(SkIndexed * indexed_p);
      void                  on_identifier_created(SkIdentifierLocal * identifier_p);
      void                  nest_locals(eSkNestReason nest_reason);
      void                  unnest_locals(eSkUnnestAction unnest_action);
      void                  accept_nest();

    // Combined Member & Local Methods

      void              change_variable_type(const ASymbol & var_name, SkClassDescBase * type_p);
      void              change_variable_types(const tSkTypedNamesIndexed & vars);
      SkClassDescBase * get_variable_type(const ASymbol & var_name, bool skip_current_scope_b = false, uint32_t * data_idx_p = nullptr, bool * is_return_arg_p = nullptr) const;
      bool              is_previous_variable(const ASymbol & var_name) const;
      bool              is_variable(const ASymbol & var_name) const;
      bool              is_captured_variable(const ASymbol & var_name) const;

  protected:

  // Internal Methods

    void capture_local(SkTypedNameIndexed * var_p, CapturedVars * top_nesting_p) const;

  };  // SkTypeContext


//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkTypedContext.inl>
#endif
