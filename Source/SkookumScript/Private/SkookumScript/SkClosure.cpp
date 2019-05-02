// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class wrapper for executed/called/invoked methods definition file
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkIdentifier.hpp>
#include <SkookumScript/SkInvokeClosure.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkBoolean.hpp>

#if (SKOOKUM & SK_DEBUG)
#include <AgogCore/AString.hpp>
#endif

//=======================================================================================
// SkClosureInvokeInfo Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Transfer contents constructor.
SkClosureInvokeInfo::SkClosureInvokeInfo(const SkParameters * params_p, APCompactArray<SkExpressionBase> * send_args_p, APCompactArray<SkIdentifierLocal> * return_args_p)
  : m_params_p(params_p)
  , m_arguments(send_args_p)
  , m_return_args(return_args_p)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
SkClosureInvokeInfo::~SkClosureInvokeInfo()
  {
  m_arguments.free_all();
  m_return_args.free_all();
  }

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
SkClosureInvokeInfo::SkClosureInvokeInfo(const void ** binary_pp)
  {
  // n bytes - parameter list
  m_params_p = SkParameters::get_or_create(binary_pp);

  // 1 byte - number of actual arguments
  uint8_t length = A_BYTE_STREAM_UI8_INC(binary_pp);
  if (length)
    {
    m_arguments.set_size(length);

    SkExpressionBase ** args_pp     = m_arguments.get_array();
    SkExpressionBase ** args_end_pp = args_pp + length;

    // n bytes - argument typed binary }- Repeating 
    while (args_pp < args_end_pp)
      {
      *args_pp = SkExpressionBase::from_binary_typed_new(binary_pp);
      args_pp++;
      }
    }

  // 1 byte - number of actual return arguments
  length = A_BYTE_STREAM_UI8_INC(binary_pp);
  if (length)
    {
    m_return_args.set_size(length);

    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    // n bytes - return argument typed binary }- Repeating 
    while (rargs_pp < rargs_end_pp)
      {
      *rargs_pp = static_cast<SkIdentifierLocal *>(SkExpressionBase::from_binary_typed_new(binary_pp));
      rargs_pp++;
      }
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this invocation and increments the memory address to just past
//             the last byte written.
void SkClosureInvokeInfo::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkClosureInvokeInfo::as_binary_length());

  // n bytes - parameter list
  m_params_p->as_binary(binary_pp);

  // 1 byte - number of actual args (limits to 255 arguments)
  uint8_t length = uint8_t(m_arguments.get_length());
  A_BYTE_STREAM_OUT8(binary_pp, &length);
  if (length)
    {
    // n bytes - argument typed binary }- Repeating 
    SkExpressionBase ** args_pp     = m_arguments.get_array();
    SkExpressionBase ** args_end_pp = args_pp + length;

    for (; args_pp < args_end_pp; args_pp++)
      {
      SkExpressionBase::as_binary_typed(*args_pp, binary_pp);
      }
    }

  // 1 byte - number of actual return args (limits to 255 return arguments)
  length = uint8_t(m_return_args.get_length());
  A_BYTE_STREAM_OUT8(binary_pp, &length);
  if (length)
    {
    // n bytes - return argument typed binary }- Repeating 
    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    for (; rargs_pp < rargs_end_pp; rargs_pp++)
      {
      SkExpressionBase::as_binary_typed(*rargs_pp, binary_pp);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
uint32_t SkClosureInvokeInfo::as_binary_length() const
  {
  uint32_t binary_length = 2u; // arg num(1) + return arg num(1)

  // n bytes - parameter list
  binary_length += m_params_p->as_binary_length();

  // n bytes - argument typed binary }- Repeating
  uint32_t length = m_arguments.get_length();
  if (length)
    {
    SkExpressionBase ** args_pp       = m_arguments.get_array();
    SkExpressionBase ** args_end_pp   = args_pp + length;

    for (; args_pp < args_end_pp; args_pp++)
      {
      binary_length += SkExpressionBase::as_binary_typed_length(*args_pp);
      }
    }

  // n bytes - return argument typed binary }- Repeating 
  length = m_return_args.get_length();
  if (length)
    {
    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + length;

    for (; rargs_pp < rargs_end_pp; rargs_pp++)
      {
      binary_length += SkExpressionBase::as_binary_typed_length(*rargs_pp);
      }
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
AString SkClosureInvokeInfo::as_code() const
  {
  AString str;
  str.ensure_size(256);

  str.append('(');

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Standard arguments
  SkExpressionBase ** args_pp     = m_arguments.get_array();
  SkExpressionBase ** args_end_pp = args_pp + m_arguments.get_length();

  while (args_pp < args_end_pp)
    {
    if (*args_pp)
      {
      // $Revisit - CReis Group args are stored as a list literal so they are printed with
      // curly braces {} surrounding the grouped arguments.  This helps to identify how
      // args are grouped though it does not make a string that would be correctly compiled.
      // In the future keep as list literal and use yet-to-be-defined expansion mechanism
      // on it so that it would compile.
      str.append((*args_pp)->as_code());
      }

    args_pp++;

    if (args_pp < args_end_pp)
      {
      str.append(", ", 2u);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return arguments
  uint32_t rarg_length = m_return_args.get_length();

  if (rarg_length)
    {
    str.append("; ", 2u);

    SkIdentifierLocal ** rargs_pp     = m_return_args.get_array();
    SkIdentifierLocal ** rargs_end_pp = rargs_pp + m_return_args.get_length();

    while (rargs_pp < rargs_end_pp)
      {
      if (*rargs_pp)
        {
        str.append((*rargs_pp)->as_code());
        }

      rargs_pp++;

      if (rargs_pp < rargs_end_pp)
        {
        str.append(", ", 2u);
        }
      }
    }

  str.append(')');

  return str;
  }

#endif // defined(SK_AS_STRINGS)

// Debugging Methods
#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains
//             was located at or follows at the character index position provided and
//             returns it.
SkExpressionBase * SkClosureInvokeInfo::find_expr_by_pos(uint pos, eSkExprFind type /*= SkExprFind_all*/) const
  {
  // Test argument-expressions
  SkExpressionBase *  found_p;
  SkExpressionBase ** args_pp     = m_arguments.get_array();
  SkExpressionBase ** args_end_pp = args_pp + m_arguments.get_length();

  for (; args_pp < args_end_pp; args_pp++)
    {
    if (*args_pp)  // Could be default argument (nullptr)
      {
      found_p = (*args_pp)->find_expr_by_pos(pos, type);

      if (found_p)
        {
        return found_p;
        }
      }
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Iterates over argument expressions applying operation supplied by
// apply_expr_p and exiting early if its apply_expr() returns AIterateResult_early_exit.
eAIterateResult SkClosureInvokeInfo::iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p /*= nullptr*/)
  {
  SkExpressionBase ** expr_pp     = m_arguments.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_arguments.get_length();

  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    if (*expr_pp && (*expr_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }
    }

  return AIterateResult_entire;
  }

#endif // (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
void SkClosureInvokeInfo::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    "SkClosureInvokeInfo",
    sizeof(SkClosureInvokeInfo),
    0,
    (m_arguments.get_length() + m_return_args.get_length()) * sizeof(void *),
    m_arguments.track_memory(mem_stats_p) + m_return_args.track_memory(mem_stats_p));
  }

//=======================================================================================
// SkClosure Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// #Author(s) Conan Reis
SkClosure::SkClosure(SkClosureInfoBase * literal_p, SkInstance * receiver_p) :
  SkInstance(SkBrain::ms_closure_class_p)
  {
  // $Revisit - MBreyer HACK
  m_user_data.m_data.m_ptr[0] = literal_p;
  m_user_data.m_data.m_ptr[1] = receiver_p;
  literal_p->reference();
  receiver_p->reference();
  }

//---------------------------------------------------------------------------------------
// Returns a AString representation of itself for debugging purposes
// Returns:    ADebug AString
// See:        get_scope_context()
// Modifiers:   virtual - overridden from SkInvokedContextBase
// Author(s):   Conan Reis
AString SkClosure::as_string() const
  {
  AString recv_str;

  recv_str.append("this: ", 6u);
  get_receiver()->as_code_append(&recv_str);

  SkClosureInfoBase * info_p = get_info();

  AString  str;
  bool     durational = !info_p->is_method();
  uint32_t capture_count = info_p->get_captured().get_count();

  #if defined(SK_AS_STRINGS)
    AString param_str(info_p->get_closure_params().as_code(durational ? SkParameters::StrFlag__default_no_return : SkParameters::StrFlag__default));
    AString code_str(info_p->get_closure_expr()->as_code_block());
  #else
    AString param_str("(\?\?\?)"); // Silly \? to avoid trigraph errors
    AString code_str("[\?\?\?]");
  #endif

  code_str.line_indent(SkDebug::ms_indent_size);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine needed string length
  uint32_t str_len = durational ? 2u : 1u;  // "^_" | "^"

  str_len += recv_str.get_length() + 1u;

  str_len += SkDebug::ms_indent_size + param_str.get_length() + 1u
    + code_str.get_length() + 1u;

  if (capture_count)
    {
    // Guess extra length for captured variables
    str_len += capture_count * (SkDebug::ms_indent_size + 20u);
    }

  str.ensure_size_empty(str_len);
    

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Build code string
  if (durational)
    {
    str.append("^_ ", 3u);
    }
  else
    {
    str.append('^');
    }

  str.append(recv_str);
  str.append('\n');

  // List captured variables
  if (capture_count)
    {
    SkInstance **    captures_pp = get_captured_array();
    SkNamedIndexed * sym_p       = info_p->get_captured().get_array();
    SkNamedIndexed * sym_end_p   = sym_p + capture_count;

    while (sym_p < sym_end_p)
      {
      str.append(' ', SkDebug::ms_indent_size);
      str.append(sym_p->get_name_str_dbg());
      str.append(": ", 2u);
      (*captures_pp)->as_code_append(&str);
      str.append('\n');

      sym_p++;
      captures_pp++;
      }
    }

  str.append(' ', SkDebug::ms_indent_size);
  str.append(param_str);
  str.append('\n');
  str.append(code_str);

  return str;
  }

//---------------------------------------------------------------------------------------
// Retrieves a closure object from the instance pool or from memory (if it has captured
// variables) and initializes it for use.  This method should be used instead of 'new'
// because it prevents unnecessary allocations by reusing previously allocated objects.
//
// #Notes
//   To 'deallocate' an object that was retrieved with this method, use 'pool_delete()'
//   rather than 'delete'.
//
// #Modifiers static
// #See Also  pool_delete()
// #Author(s) Conan Reis
SkClosure * SkClosure::new_instance(
  // Closure invocation info
  SkClosureInfoBase * closure_info_p,
  // Receiver that the closure uses
  SkInstance * receiver_p
  )
  {
  uint32_t capture_count = closure_info_p->get_captured().get_count();

  void * buffer_p = (capture_count)
    ? AgogCore::get_app_info()->malloc(
        sizeof(SkClosure) + (capture_count * sizeof(SkInstance *)),
        "SkClosure")
    : get_pool().allocate();  // Use pooled SkInstance since it is the same size

  SkClosure * closure_p = new (buffer_p) SkClosure(closure_info_p, receiver_p);

  closure_p->m_ref_count  = 1u;

  return closure_p;
  }

//---------------------------------------------------------------------------------------
// Frees up a closure and puts it into the dynamic pool (if it didn't use captured
// variables) ready for its next use.  This should be used instead of 'delete' because it
// prevents unnecessary deallocations by saving previously allocated objects.
//
// #Notes
//   called by dereference()
//
// #Modifiers virtual
// #See Also  pool_new()
// #Author(s) Conan Reis
void SkClosure::delete_this()
  {
  uint32_t capture_count = get_info()->get_captured().get_count();

  get_receiver()->dereference();
  get_info()->dereference();
  m_ptr_id = AIdPtr_null;

  if (capture_count)
    {
    // Dereference captured variables
    SkInstance ** captures_pp     = get_captured_array();
    SkInstance ** captures_end_pp = captures_pp + capture_count;

    while (captures_pp < captures_end_pp)
      {
      (*captures_pp)->dereference();
      captures_pp++;
      }

    AgogCore::get_app_info()->free(this);
    }
  else
    {
    new (this) SkInstance(ALeaveMemoryUnchanged);  // Reset v-table back to SkInstance
    get_pool().recycle(this);
    }
  }

//---------------------------------------------------------------------------------------
SkInvokedBase * SkClosure::invoke(
  SkObjectBase * scope_p, 
  SkInvokedBase * caller_p, 
  SkInstance ** result_pp, 
  const SkClosureInvokeInfo & invoke_info, 
  const SkExpressionBase * invoking_expr_p) const
  {
  // Closures of indeterminate duration type + could be a method or a coroutine
  SkClosureInfoBase * info_p = get_info();
  if (info_p->is_method())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Closure is an immediate method.

    SkClosure::invoke_as_method(scope_p, caller_p, result_pp, invoke_info, invoking_expr_p); // Make a direct, non-virtual function call
    return nullptr;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Closure is a durational coroutine.

  SkClosureInfoCoroutine * coroutine_p = static_cast<SkClosureInfoCoroutine *>(info_p);
  SkInvokedCoroutine *     icoro_p     = SkInvokedCoroutine::pool_new(coroutine_p);

  icoro_p->reset(
    SkCall_interval_always, caller_p, const_cast<SkClosure *>(this), nullptr, invoke_info.m_return_args.is_filled() ? &invoke_info.m_return_args : nullptr);

  // Store expression debug info for next invoked method/coroutine.
  SKDEBUG_ICALL_STORE_GEXPR(invoking_expr_p);

  // Must be called before calling argument expressions
  SKDEBUG_ICALL_SET_EXPR(icoro_p, invoking_expr_p);

  // First, copy over captured variables
  const tSkIndexedNames & captured_names = info_p->get_captured();
  icoro_p->data_append_vars_ref(get_captured_array(), captured_names.get_count());

  // Then, fill invoked coroutine's argument list
  icoro_p->data_append_args_exprs(invoke_info.m_arguments, coroutine_p->get_params(), scope_p);

  // Hook must be called after argument expressions and before invoke()
  SKDEBUG_HOOK_EXPR(invoking_expr_p, scope_p, icoro_p, nullptr, SkDebug::HookContext_peek);

  // Return the invoked coroutine as a result
  if (result_pp)
    {
    *result_pp = icoro_p->as_new_instance();
    }

  // Invoke the coroutine on the receiver - try to have it complete this frame
  SkInvokedBase * invoked_p = icoro_p->on_update()
                            ? nullptr   // Completed this frame/immediately
                            : icoro_p;  // Deferred completion

  return invoked_p;
  }

//---------------------------------------------------------------------------------------
void SkClosure::invoke_as_method(
  SkObjectBase * scope_p, 
  SkInvokedBase * caller_p, 
  SkInstance ** result_pp, 
  const SkClosureInvokeInfo & invoke_info, 
  const SkExpressionBase * invoking_expr_p) const
  {
  SkClosureInfoMethod * method_p = static_cast<SkClosureInfoMethod *>(get_info());
  SK_ASSERTX(method_p->is_method(), "SkClosure::invoke_as_method() called on a closure that is not a method.");
  SkInvokedMethod imethod(caller_p, const_cast<SkClosure *>(this), method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  // Store expression debug info for next invoked method/coroutine.
  SKDEBUG_ICALL_STORE_GEXPR(invoking_expr_p);

  // Must be called before calling argument expressions
  SKDEBUG_ICALL_SET_EXPR(&imethod, invoking_expr_p);

  // First, copy over captured variables
  const tSkIndexedNames & captured_names = method_p->get_captured();
  imethod.data_append_vars_ref(get_captured_array(), captured_names.get_count());

  // Then append invoked closure's argument list
  imethod.data_append_args_exprs(invoke_info.m_arguments, method_p->get_params(), scope_p);

  // Hook must be called after argument expressions and before invoke()
  SKDEBUG_HOOK_EXPR(invoking_expr_p, scope_p, &imethod, nullptr, SkDebug::HookContext_peek);

  // Call method
  method_p->invoke(&imethod, caller_p, result_pp);

  // Bind any return arguments
  if (!invoke_info.m_return_args.is_empty())
    {
    imethod.data_bind_return_args(invoke_info.m_return_args, method_p->get_params());
    }
  }

//---------------------------------------------------------------------------------------
void SkClosure::closure_method_call(
    SkInstance **   args_pp,
    uint32_t        arg_count,
    SkInstance **   result_pp, // = nullptr
    SkInvokedBase * caller_p   // = nullptr
    )
  {
  SkClosureInfoBase * info_p = get_info();

  #ifdef SK_RUNTIME_RECOVER
    if (!info_p->is_method())
      {
      SK_ERROR("Tried to call coroutine closure as a method!", SkClosure);

      if (result_pp)
        {
        // Wanted a return so return a nil so there is something
        *result_pp = SkBrain::ms_nil_p;  // nil does not need to be referenced/dereferenced
        }

      return;
      }
  #endif

  SkClosureInfoMethod * method_p  = static_cast<SkClosureInfoMethod *>(info_p);
  SkInvokedMethod imethod(caller_p, this, method_p, a_stack_allocate(method_p->get_invoked_data_array_size(), SkInstance*));

  SKDEBUG_ICALL_SET_INTERNAL(&imethod);
  SKDEBUG_HOOK_SCRIPT_ENTRY(method_p->get_name());

  // First, copy over captured variables
  const tSkIndexedNames & captured_names = info_p->get_captured();
  imethod.data_append_vars_ref(get_captured_array(), captured_names.get_count());

  // Then fill invoked method's argument list
  imethod.data_append_args(args_pp, arg_count, method_p->get_params());

  // Call method
  method_p->invoke(&imethod, caller_p, result_pp);

  SKDEBUG_HOOK_SCRIPT_EXIT();

  // $Revisit - CReis Bind any return arguments
  //if (!m_return_args.is_empty())
  //  {
  //  imethod.data_bind_return_args(m_return_args, *method_p);
  //  }
  }

//---------------------------------------------------------------------------------------
bool SkClosure::closure_method_query(
  SkInstance *    arg_p,     // = nullptr
  SkInvokedBase * caller_p   // = nullptr
  )
  {
  SkInstance * bool_p = nullptr;

  closure_method_call(&arg_p, arg_p ? 1u : 0u, &bool_p, caller_p);

  bool result = bool_p && bool_p->as<SkBoolean>();

  bool_p->dereference();

  return result;
  }

//---------------------------------------------------------------------------------------
// Cleans up the closure instance and adds it to the reuse pool if possible.
// Modifiers:   virtual - overridden from SkInstance
// Author(s):   Conan Reis
void SkClosure::on_no_references()
  {
  // No destructor to call - all clean-up done in pool_delete()
  delete_this();
  }

//---------------------------------------------------------------------------------------
// Returns the 'this' using this object as the scope.
// Returns:    itself
// Modifiers:   virtual from SkObjectBase
// Author(s):   Conan Reis
SkInstance * SkClosure::get_topmost_scope() const
  {
  return get_receiver();
  }

//---------------------------------------------------------------------------------------
// Skoo Params Closure@String() String
// [See script file.]
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkClosure::mthd_String(
  SkInvokedMethod * scope_p,
  SkInstance **     result_pp
  )
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(scope_p->this_as_data<SkClosure>()->as_string());
    }
  }

//---------------------------------------------------------------------------------------
// #Description
//   Registers the atomic C++ classes, methods, etc.
//
// #Notes
//   This method is called by Brain::initialize_post_load()
//
// #Modifiers static
// #Author(s) Conan Reis
void SkClosure::register_bindings()
  {
  SkBrain::ms_closure_class_p->register_method_func(ASymbol_String,      mthd_String);

  // Note that a Closure destructor is not needed - just SkClosure::pool_delete() is used

  // $Revisit - CReis Closures data-structures have different sizes depending on how many
  // variables are captured so they have tricky instantiation making the current mechanism
  // not viable without adding a special case. Only bother going down this path if making
  // a copy of a closure becomes needed. See SkClosure::pool_new()
  //SkBrain::ms_closure_class_p->register_method_func(ASymbolX_ctor_copy,  mthd_ctor_copy);
  }
