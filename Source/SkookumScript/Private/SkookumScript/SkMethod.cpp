// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Method parameters & body classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkMethod.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkMethod.inl>
#endif

#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkSymbol.hpp>

#if (SKOOKUM & SK_COMPILED_OUT)
  #include <SkookumScript/SkParser.hpp>
#endif


//=======================================================================================
// SkMethodBase Method Definitions
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
AString SkMethodBase::as_code_params() const
  {
  return m_params_p->as_code();
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// Determines if it is a class member (true) or an instance member (false)
//
// #Author(s) Conan Reis
bool SkMethodBase::is_class_member() const
  {
  return m_scope_p->find_class_method(m_name) == this;
  }


//=======================================================================================
// SkMethod Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual - overriding from SkInvokableBase
// Author(s):   Conan Reis
SkMethod::~SkMethod()
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
// Arg         name - name of method
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
SkMethod::SkMethod(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  // n bytes - parameter list
  SkMethodBase(name, scope_p, binary_pp)
  {
  // n bytes - expression typed binary
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
void SkMethod::assign_binary_no_name(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p)
  {
  // n bytes - parameter list
  SkMethodBase::assign_binary_no_name(binary_pp, update_record_p);

  // n bytes - expression typed binary
  m_expr_p = SkExpressionBase::from_binary_typed_new(binary_pp);
  }

//---------------------------------------------------------------------------------------

void SkMethod::copy_to_update_record(SkRoutineUpdateRecord * update_record_p)
  {
  SkMethodBase::copy_to_update_record(update_record_p);

  if (update_record_p)
    {
    update_record_p->m_previous_custom_expr_p = m_expr_p;
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this method and its components and increments the memory
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
//             Note that the scope is implied by the class that this method is
//             contained in.
// Modifiers:   virtual (overriding pure method from SkMethodBase) 
// Author(s):   Conan Reis
void SkMethod::as_binary(void ** binary_pp, bool include_name) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkMethod::as_binary_length(include_name));

  // 4 bytes - name id
  // n bytes - parameter list
  SkMethodBase::as_binary(binary_pp, include_name);

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
// Modifiers:   virtual (overriding pure method from SkMethodBase) 
// Author(s):   Conan Reis
uint32_t SkMethod::as_binary_length(bool include_name) const
  {
  return SkMethodBase::as_binary_length(include_name) + SkExpressionBase::as_binary_typed_length(m_expr_p);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkMethod::as_code() const
  {
  AString code_str(m_expr_p ? m_expr_p->as_code_block() : "nil");
  AString str(m_params_p->as_code(), 2u + code_str.get_length());

  str.append("\n\n", 2u);
  str.append(code_str);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkMethod::get_custom_expr() const
  {
  return m_expr_p;
  }

#endif


//---------------------------------------------------------------------------------------
// Change the expression
// Arg         expr_p - pointer to code (can be nullptr)
// Author(s):   Conan Reis
void SkMethod::set_expression(SkExpressionBase * expr_p)
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
SkMethod & SkMethod::assign_take(SkMethod * method_p)
  {
  SkMethodBase::assign(*method_p);

  if (m_expr_p)
    {
    delete m_expr_p;
    }

  m_expr_p = method_p->m_expr_p;
  method_p->m_expr_p = nullptr;

  return *this;
  }

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_method
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkMethod::get_invoke_type() const
  {
  return SkInvokable_method;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to code
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkMethod::is_bound() const
  {
  return (m_expr_p != nullptr);
  }

//---------------------------------------------------------------------------------------
// Determines if this invokable has an empty code block = does nothing
bool SkMethod::is_empty() const
  {
  return (m_expr_p == nullptr || m_expr_p->is_nil());
  }

//---------------------------------------------------------------------------------------
// Determines if this method is a placeholder
bool SkMethod::is_placeholder() const
  {
  return (m_expr_p == nullptr);
  }

//---------------------------------------------------------------------------------------
// Calls the method using the supplied scope
// Arg         scope_p - scope for data/method/etc. look-ups.  It will always be an
//             instance of SkookumScript/SkInvokedMethod.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Modifiers:   virtual - overriding from SkMethodBase
// Author(s):   Conan Reis
void SkMethod::invoke(
  SkInvokedMethod * scope_p,
  SkInvokedBase *   caller_p, // = nullptr
  SkInstance **     result_pp // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // If there's no expression it means this is a placeholder method, which means it failed to compile
    if (!m_expr_p)
      {
      SK_ERRORX(a_str_format("Tried to invoke method '%s@%s' but it cannot be invoked because it had errors during compilation.", get_scope()->get_name_cstr(), get_name_cstr()));
      if (result_pp) *result_pp = SkBrain::ms_nil_p;
      return;
      }
  #endif

  SKDEBUG_STORE_CALL(scope_p);

  SKDEBUG_HOOK_METHOD(scope_p);

  // Notice that the scope becomes the caller
  m_expr_p->invoke(scope_p, scope_p, result_pp);

  SKDEBUG_RESTORE_CALL();
  }

//---------------------------------------------------------------------------------------
// Calls the method essentially as a simple coroutine - with the possibility 
//             that completion will be deferred - i.e. it will be durational.
// Returns:    deferred invoked sub-expression or nullptr if completed immediately.
// Arg         scope_p - scope for data/method/etc. look-ups.  It will always be an
//             instance of SkookumScript/SkInvokedMethod.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Modifiers:   virtual - overriding from SkMethodBase
// Author(s):   Conan Reis
SkInvokedBase * SkMethod::invoke_deferred(
  SkInvokedMethod * scope_p,
  SkInvokedBase *   caller_p, // = nullptr
  SkInstance **     result_pp // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // If there's no expression it means this is a placeholder method, which means it failed to compile
    if (!m_expr_p)
      {
      SK_ERRORX(a_str_format("Tried to invoke method '%s@%s' but it cannot be invoked because it had errors during compilation.", get_scope()->get_name_cstr(), get_name_cstr()));
      if (result_pp) *result_pp = SkBrain::ms_nil_p;
      return nullptr;
      }
  #endif

  SKDEBUG_STORE_CALL(scope_p);

  // Notice that the scope becomes the caller
  SkInvokedBase * invoked_p = m_expr_p->invoke(scope_p, scope_p, result_pp);

  SKDEBUG_RESTORE_CALL();

  return invoked_p;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkMethod::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkMethod, 0u));
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
// SkMethodFunc Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of method
// Arg         scope_p - class scope to use
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes - name id
//               n bytes - parameter list
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkMethodFunc::SkMethodFunc(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  SkMethodBase(name, scope_p, binary_pp),
  m_atomic_f(nullptr)
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkMethodFunc::as_code() const
  {
  AString str(m_params_p->as_code(), 128u);

  str.append("\n\n", 2u);
  str.ensure_size_extra(50u);
  str.append_format("// Atomic - function address: 0x%p\n", m_atomic_f);
  str.line_indent_next(SkDebug::ms_indent_size);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_method_func
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkMethodFunc::get_invoke_type() const
  {
  return SkInvokable_method_func;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to C++ function
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkMethodFunc::is_bound() const
  {
  return (m_atomic_f != nullptr);
  }

//---------------------------------------------------------------------------------------
// Calls the method using the supplied scope
// Arg         scope_p - scope for data/method/etc. look-ups.  It will always be an
//             instance of SkookumScript/SkInvokedMethod.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  The caller is
//             ignored here since it should already be stored as the caller in scope_p.
//             (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Modifiers:   virtual - overriding from SkMethodBase
// Author(s):   Conan Reis
void SkMethodFunc::invoke(
  SkInvokedMethod * scope_p,
  SkInvokedBase *   caller_p, // = nullptr
  SkInstance **     result_pp // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // Check to ensure that the C++ method is registered
    // $Revisit - CReis This should be done at SkookumScript startup.
    if (m_atomic_f == nullptr)
      {
      if (!ADebug::is_nested_error())
        {
        SK_ERRORX(a_str_format("Tried to call non-registered C++ method '%s'.", as_string_name().as_cstr()));
        }

      // Probably wanted something more substantial than a nil, but it is better than nothing...
      return;
      }
  #endif

  if (result_pp)
    {
    // Clear result so can determine if a result was supplied
    *result_pp = nullptr;
    }

  SKDEBUG_STORE_CALL(scope_p);
  SKDEBUG_HOOK_METHOD(scope_p);

  (m_atomic_f)(scope_p, result_pp);

  SKDEBUG_RESTORE_CALL();

  // For lazy methods that did not supply a result - set nil for them.
  // $Vital - CReis Should ensure correct result type if result_pp is not nullptr and SK_DEBUG is set
  // Should really be an error if expected a type and did not get it and for calls that do
  // not need to return something should auto return nil.
  if (result_pp && !*result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkMethodFunc::track_memory(AMemoryStats * mem_stats_p) const
  {
  // $Note - CReis Using size of SkMethodMthd since SkMethodFunc classes use the same
  // memory footprint.
  mem_stats_p->track_memory("SkMethodFunc", sizeof(SkMethodMthd), 0u);

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }


//=======================================================================================
// SkMethodMthd Method Definitions
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         name - name of method
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
SkMethodMthd::SkMethodMthd(
  const ASymbol & name,
  SkClass *       scope_p,
  const void **   binary_pp
  ) :
  // n bytes - parameter list
  SkMethodBase(name, scope_p, binary_pp),
  m_atomic_m(nullptr)
  {
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual (overriding pure method from SkInvokableBase) 
// Author(s):   Conan Reis
AString SkMethodMthd::as_code() const
  {
  AString str(m_params_p->as_code(), 128u);

  str.append("\n\n", 2u);
  str.ensure_size_extra(50u);
  str.append_format("// Atomic - method address: 0x%p\n", m_atomic_m);
  str.line_indent_next(SkDebug::ms_indent_size);

  return str;
  }

#endif // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             invokables when it is only known that it is an instance is of type
//             SkookumScript/SkInvokableBase.
// Returns:    SkInvokable_method_mthd
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
eSkInvokable SkMethodMthd::get_invoke_type() const
  {
  return SkInvokable_method_mthd;
  }

//---------------------------------------------------------------------------------------
// Returns true if code expression or C++ routine has been
//             associated yet or false if not.
// Returns:    true if bound to C++ method
// Modifiers:   virtual from SkInvokableBase
// Author(s):   Conan Reis
bool SkMethodMthd::is_bound() const
  {
  return (m_atomic_m != nullptr);
  }

//---------------------------------------------------------------------------------------
// Calls the method using the supplied scope
// Arg         scope_p - scope for data/method/etc. look-ups.  It will always be an
//             instance of SkookumScript/SkInvokedMethod.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  The caller is
//             ignored here since it should already be stored as the caller in scope_p.
//             (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// Modifiers:   virtual - overriding from SkMethodBase
// Author(s):   Conan Reis
void SkMethodMthd::invoke(
  SkInvokedMethod * scope_p,
  SkInvokedBase *   caller_p, // = nullptr
  SkInstance **     result_pp // = nullptr
  ) const
  {
  #if (SKOOKUM & SK_DEBUG)
    // Check to ensure that the C++ method is registered
    if (m_atomic_m == nullptr)
      {
      if (!ADebug::is_nested_error())
        {
        SK_ERRORX(a_str_format("Tried to call non-registered C++ method '%s'.", as_string_name().as_cstr()));
        }

      // Probably wanted something more substantial than a nil, but it is better than nothing...
      if (result_pp)
        {
        // nil does not need to be referenced/dereferenced
        *result_pp = SkBrain::ms_nil_p;
        }
      return;
      }
  #endif

  SKDEBUG_STORE_CALL(scope_p);

  // The scope of an invoked method is always some form of an SkInstance
  SkInstance * receiver_p = static_cast<SkInstance *>(scope_p->m_scope_p.get_obj());

  if (result_pp)
    {
    // Clear result so can determine if a result was supplied
    *result_pp = nullptr;
    }

  if (receiver_p)
    {
    SKDEBUG_HOOK_METHOD(scope_p);
    (receiver_p->*m_atomic_m)(scope_p, result_pp);
    }

  SKDEBUG_RESTORE_CALL();

  // For lazy methods that did not supply a result - set nil for them.
  // $Vital - CReis Should ensure correct result type if result_pp is not nullptr and SK_DEBUG is set
  // Should really be an error if expected a type and did not get it and for calls that do
  // not need to return something should auto return nil.
  if (result_pp && !*result_pp)
    {
    *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
    }
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Author(s):   Conan Reis
void SkMethodMthd::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkMethodMthd, 0u));

  if (!m_params_p->is_sharable())
    {
    m_params_p->track_memory(mem_stats_p);
    }
  }
