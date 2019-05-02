// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Coroutine parameters & body classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkCoroutine.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkCoroutine.inl>
#endif

#if defined(SK_AS_STRINGS)
  #include <AgogCore/AString.hpp>
#endif

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkSymbol.hpp>


//=======================================================================================
// SkCoroutineBase Method Definitions
//=======================================================================================

// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this invokable's parameters into its source code string
//             equivalent.
// Returns:    Source code string version of its parameters
// See:        as_code()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkCoroutineBase::as_code_params() const
  {
  return m_params_p->as_code(SkParameters::StrFlag__default_no_return);
  }

#endif // defined(SK_AS_STRINGS)


//=======================================================================================
// SkCoroutine Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual - overriding from SkInvokableBase
// Author(s):   Conan Reis
SkCoroutine::~SkCoroutine()
  {
  if (m_expr_p)
    {
    delete m_expr_p;
    }
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of coroutine
// Arg         scope_p - class scope to use
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//               n bytes - expression typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkCoroutine::SkCoroutine(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  // n bytes - parameter list
  SkCoroutineBase(name, scope_p, binary_pp)
  {
  // n bytes - code block
  m_expr_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

//---------------------------------------------------------------------------------------
// Assignment from binary
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//               n bytes - expression typed binary
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
void SkCoroutine::assign_binary_no_name(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p)
  {
  // n bytes - parameter list
  SkCoroutineBase::assign_binary_no_name(binary_pp, update_record_p);

  // n bytes - expression typed binary
  m_expr_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

//---------------------------------------------------------------------------------------

void SkCoroutine::copy_to_update_record(SkRoutineUpdateRecord * update_record_p)
  {
  SkCoroutineBase::copy_to_update_record(update_record_p);

  if (update_record_p)
    {
    update_record_p->m_previous_custom_expr_p = m_expr_p;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this coroutine and its components and increments the memory
//             address to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes - name id
//               n bytes - parameter list
//               n bytes - expression typed binary
//
//             Note that the scope is implied by the class that this coroutine is
//             contained in.
// Modifiers:   virtual (overriding pure method from SkCoroutineBase) 
// Author(s):   Conan Reis
void SkCoroutine::as_binary(void ** binary_pp, bool include_name) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCoroutine::as_binary_length(include_name));

  // 4 bytes - name id
  // n bytes - parameter list
  SkCoroutineBase::as_binary(binary_pp, include_name);

  // n bytes - expression typed binary
  SkExpressionBase::as_binary_typed(m_expr_p, binary_pp);
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes - name id
//               n bytes - parameter list
//               n bytes - expression typed binary
// Modifiers:   virtual (overriding pure method from SkCoroutineBase) 
// Author(s):   Conan Reis
uint32_t SkCoroutine::as_binary_length(bool include_name) const
  {
  return SkCoroutineBase::as_binary_length(include_name) + SkExpressionBase::as_binary_typed_length(m_expr_p);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this into its source code string equivalent.  This is essentially
//             a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkCoroutine::as_code() const
  {
  AString str(m_params_p->as_code(SkParameters::StrFlag__default_no_return), 128u);

  str.append("\n\n", 2u);
  str.append(m_expr_p ? m_expr_p->as_code_block() : "nil");

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkCoroutine::get_custom_expr() const
  {
  return m_expr_p;
  }

#endif


//---------------------------------------------------------------------------------------
// Change the expression
// Arg         expr_p - pointer to code (can be nullptr)
// Author(s):   Conan Reis
void SkCoroutine::set_expression(SkExpressionBase * expr_p)
  {
  if (m_expr_p)
    {
    delete m_expr_p;
    }

  m_expr_p = expr_p;
  }

//---------------------------------------------------------------------------------------
// Transfer ownership assignment
// Returns:    itself
// Arg         method_p - address of method to take the contents of
// Author(s):   Conan Reis
SkCoroutine & SkCoroutine::assign_take(SkCoroutine * coroutine_p)
  {
  SkCoroutineBase::assign(*coroutine_p);

  if (m_expr_p)
    {
    delete m_expr_p;
    }

  m_expr_p = coroutine_p->m_expr_p;
  coroutine_p->m_expr_p = nullptr;

  return *this;
  }

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_coroutine
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkCoroutine::get_invoke_type() const
  {
  return SkInvokable_coroutine;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to code
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkCoroutine::is_bound() const
  {
  return (m_expr_p != nullptr);
  }

//---------------------------------------------------------------------------------------
// Determines if this invokable has an empty code block = does nothing
bool SkCoroutine::is_empty() const
  {
  return (m_expr_p == nullptr || m_expr_p->is_nil());
  }

//---------------------------------------------------------------------------------------
// Determines if this method is a placeholder
bool SkCoroutine::is_placeholder() const
  {
  return (m_expr_p == nullptr);
  }

//---------------------------------------------------------------------------------------
// Called every SkMind::on_update() when in the Actor's list of invoked
//             coroutines.  [Called by SkInvokedCoroutine::on_update()]
// Returns:    Boolean indicating whether the coroutine has completed.  false if the
//             invoked coroutine should be called again in the next update, true if the
//             coroutine has completed and it should be removed from the Actor's
//             invoked coroutines list.
// Arg         scope_p - call scope and working data for the coroutine - see
//             SkInvokedCoroutine for a description of its methods and data members.
// Modifiers:   virtual - overrides from SkCoroutineBase
// Author(s):   Conan Reis
bool SkCoroutine::on_update(SkInvokedCoroutine * scope_p) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // If there's no expression it means this is a placeholder method, which means it failed to compile
    if (!m_expr_p)
      {
      SK_ERRORX(a_str_format("Tried to invoke method '%s@%s' but it cannot be invoked because it had errors during compilation.", get_scope()->get_name_cstr(), get_name_cstr()));
      return true;
      }
  #endif

  SKDEBUG_HOOK_COROUTINE(scope_p);

  SkInvokedBase * invoked_p = m_expr_p->invoke(scope_p, scope_p);

  if (invoked_p)
    {
    // Expression has deferred completion.
    //   - if it has sup-parts pending update it again at its next update time
    //   - if it has sup-parts pending and is waiting for a sub-part to notify it when it is complete, remove it from the update list until resumed

    scope_p->pending_deferred(invoked_p);

    return false;
    }

  // Coroutine completed immediately
  return true;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkCoroutine::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkCoroutine, 0u));
  if (m_expr_p)
    {
    m_expr_p->track_memory(mem_stats_p);
    }

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkCoroutineMthd Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of coroutine
// Arg         scope_p - class scope to use
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkCoroutineMthd::SkCoroutineMthd(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  // 4 bytes - name id
  SkCoroutineBase(name, scope_p, binary_pp),
  m_update_m(nullptr)
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this into its source code string equivalent.  This is essentially
//             a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkCoroutineMthd::as_code() const
  {
  AString str(m_params_p->as_code(SkParameters::StrFlag__default_no_return), 128u);

  str.append("\n\n", 2u);
  str.ensure_size_extra(50u);
  str.append_format("// Atomic - method address: 0x%p\n", m_update_m);
  str.line_indent_next(SkDebug::ms_indent_size);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_coroutine_mthd
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkCoroutineMthd::get_invoke_type() const
  {
  return SkInvokable_coroutine_mthd;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to C++ method
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkCoroutineMthd::is_bound() const
  {
  return (m_update_m != nullptr);
  }

//---------------------------------------------------------------------------------------
// Called every SkMind::on_update() when in the Mind's list of invoked
//             coroutines.  [Called by SkInvokedCoroutine::on_update()]
// Returns:    Boolean indicating whether the coroutine has completed.  false if the
//             invoked coroutine should be called again in the next update, true if the
//             coroutine has completed and it should be removed from the Mind's
//             invoked coroutines list.
// Arg         scope_p - call scope and working data for the coroutine - see
//             SkInvokedCoroutine for a description of its methods and data members.
// Notes:      Assumes that m_update_m is not set to nullptr.
// Modifiers:   virtual - overrides from SkCoroutineBase
// Author(s):   Conan Reis
bool SkCoroutineMthd::on_update(SkInvokedCoroutine * scope_p) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // Check to ensure that the C++ coroutine is registered
    // $Revisit - CReis Since this check is done at startup (Debug) this runtime check
    // might be overkill - it might be best to just have a nullptr access violation.
    if (m_update_m == nullptr)
      {
      if (!ADebug::is_nested_error())
        {
        SK_ERRORX(a_str_format("Tried to call non-registered C++ coroutine '%s'.", as_string_name().as_cstr()));
        }

      return true;
      }
  #endif

  // The scope of a invoked coroutine is always an instance
  SkInstance * receiver_p = static_cast<SkInstance *>(scope_p->m_scope_p.get_obj());

  if (receiver_p)
    {
    SKDEBUG_HOOK_COROUTINE(scope_p);
    return (receiver_p->*m_update_m)(scope_p);
    }
  else
    {
    // $Revisit - CReis Should display error.
    return true;
    }

  // If true returned coroutine successfully completed, free up invoked coroutine
  // If false returned coroutine did not complete this frame
  //   - if it has sup-parts pending update it again at its next update time
  //   - if it has sup-parts pending and is waiting for a sub-part to notify it when it is complete, remove it from the update list until resumed
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkCoroutineMthd::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkCoroutineMthd, 0u));

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkCoroutineFunc Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of coroutine
// Arg         scope_p - class scope to use
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               n bytes - parameter list
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkCoroutineFunc::SkCoroutineFunc(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  // n bytes - parameter list
  SkCoroutineBase(name, scope_p, binary_pp),
  m_update_f(nullptr)
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this into its source code string equivalent.  This is essentially
//             a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkCoroutineFunc::as_code() const
  {
  AString str(m_params_p->as_code(SkParameters::StrFlag__default_no_return), 128u);

  str.append("\n\n", 2u);
  str.ensure_size_extra(50u);
  str.append_format("// Atomic - function address: 0x%p\n", m_update_f);
  str.line_indent_next(SkDebug::ms_indent_size);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_coroutine_func
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkCoroutineFunc::get_invoke_type() const
  {
  return SkInvokable_coroutine_func;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to C++ function
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkCoroutineFunc::is_bound() const
  {
  return (m_update_f != nullptr);
  }

//---------------------------------------------------------------------------------------
// Called every SkMind::on_update() when in the Actor's list of invoked
//             coroutines.  [Called by SkInvokedCoroutine::on_update()]
// Returns:    Boolean indicating whether the coroutine has completed.  false if the
//             invoked coroutine should be called again in the next update, true if the
//             coroutine has completed and it should be removed from the Actor's
//             invoked coroutines list.
// Arg         scope_p - call scope and working data for the coroutine - see
//             SkInvokedCoroutine for a description of its methods and data members.
// Notes:      Assumes that m_update_f is not set to nullptr.
// Modifiers:   virtual - overrides from SkCoroutineBase
// Author(s):   Conan Reis
bool SkCoroutineFunc::on_update(SkInvokedCoroutine * scope_p) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // Check to ensure that the C++ coroutine is registered
    if (m_update_f == nullptr)
      {
      if (!ADebug::is_nested_error())
        {
        SK_ERRORX(a_str_format("Tried to call non-registered C++ coroutine '%s'.", as_string_name().as_cstr()));
        }

      return true;
      }
  #endif

  SKDEBUG_HOOK_COROUTINE(scope_p);
  return (m_update_f)(scope_p);

  // If true returned coroutine successfully completed, free up invoked coroutine
  // If false returned coroutine did not complete this frame
  //   - if it has sup-parts pending update it again at its next update time
  //   - if it has sup-parts pending and is waiting for a sub-part to notify it when it is complete, remove it from the update list until resumed
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkCoroutineFunc::track_memory(AMemoryStats * mem_stats_p) const
  {
  // $Note - CReis Using size of SkCoroutineMthd since SkCoroutineFunc classes use the same
  // memory footprint.
  mem_stats_p->track_memory("SkCoroutineFunc", sizeof(SkCoroutineMthd), 0u);

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }
