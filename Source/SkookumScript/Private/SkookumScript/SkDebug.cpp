// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Debugging and error handling classes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkDebug.hpp>

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AFunctionArg.hpp>
#include <AgogCore/AMath.hpp>
#include <AgogCore/AObjReusePool.hpp>
#include <AgogCore/AStringRef.hpp>
#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkClosure.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInteger.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkInvokedCoroutine.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkLiteral.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkParameterBase.hpp>
#include <SkookumScript/SkRemoteRuntimeBase.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkTypedClass.hpp>


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // Enumerated constants
  enum
    {
    SkDebug_log_str_length = 2048,
    };

  //const bool SkDebug_save_compiled_def  = true;
  //const bool SkDebug_ensure_atomics_def = true;
  //const bool SkDebug_evaluate_def       = true;

} // End unnamed namespace


//=======================================================================================
// SkDebugInfo Method Definitions
//=======================================================================================

const SkExpressionBase * SkDebugInfo::ms_expr_default_p = SkDebugInfo::get_expr_default();
const SkExpressionBase * SkDebugInfo::ms_expr_p         = SkDebugInfo::ms_expr_default_p;


//=======================================================================================
// SkDebugInfo Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default expression - just used for its default source index and debug info.
// Modifiers:   static
// Author(s):   Conan Reis
SkExpressionBase * SkDebugInfo::get_expr_default()
  {
  #if (SKOOKUM & SK_DEBUG)
    // SkLiteral seems to be the simplest type of expression.
    static SkLiteral s_default_expr(SkLiteral::Type__nil);

    s_default_expr.m_source_idx = 0u;
    s_default_expr.m_debug_info = uint16_t(Flag__default);

    return &s_default_expr;
  #else
    return nullptr;
  #endif
  }


//=======================================================================================
// SkDebugInfoSetter Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkDebugInfoSetter::SkDebugInfoSetter()
  {
  #if (SKOOKUM & SK_DEBUG)
    SkDebugInfo::ms_expr_p = SkInvokedContextBase::ms_last_expr_p;
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkDebugInfoSetter::SkDebugInfoSetter(const SkExpressionBase & expr)
  {
  #if (SKOOKUM & SK_DEBUG)
    SkDebugInfo::ms_expr_p = &expr;
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkDebugInfoSetter::SkDebugInfoSetter(SkInvokedBase * caller_p)
  {
  #if (SKOOKUM & SK_DEBUG)
    SkDebugInfo::ms_expr_p = SkInvokedContextBase::ms_last_expr_p;

    if (caller_p)
      {
      SkInvokedContextBase * current_call_p = caller_p->get_caller_context();

      if (current_call_p)
        {
        SkDebug::ms_current_call_p = current_call_p;
        }
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkDebugInfoSetter::~SkDebugInfoSetter()
  {
  SkDebugInfo::ms_expr_p = SkDebugInfo::ms_expr_default_p;
  }


//=======================================================================================
// SkMemberExpression Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkMemberExpression::SkMemberExpression(
  const SkMemberInfo & member_info,
  SkExpressionBase *   expr_p // = nullptr
  ) :
  SkMemberInfo(member_info),
  m_expr_p(expr_p),
  #if (SKOOKUM & SK_DEBUG)
    m_source_idx(expr_p ? expr_p->m_source_idx : SkExpr_char_pos_invalid)
  #else
    m_source_idx(SkExpr_char_pos_invalid)
  #endif
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkMemberExpression::operator==(const SkMemberExpression & expr_info) const
  {
  return SkMemberInfo::operator==(expr_info)
    && ((m_expr_p == nullptr) || (expr_info.m_expr_p == nullptr) || (m_expr_p == expr_info.m_expr_p));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkMemberExpression::operator<(const SkMemberExpression & expr_info) const
  {
  return (SkMemberInfo::operator<(expr_info)
    || ((SkMemberInfo::operator==(expr_info)
      && (m_expr_p && expr_info.m_expr_p && (m_source_idx < expr_info.m_source_idx)))));
  }

//---------------------------------------------------------------------------------------
// Get the expression associated with this member expression info
// Author(s):   Conan Reis
SkExpressionBase * SkMemberExpression::get_expr() const
  {
  #if (SKOOKUM & SK_DEBUG)
    if ((m_source_idx != SkExpr_char_pos_invalid)
      && ((m_expr_p == nullptr) || (m_source_idx == m_expr_p->m_source_idx)))
      {
      m_expr_p = find_expr_on_pos(m_source_idx);

      if (m_expr_p == nullptr)
        {
        SkDebug::print(
          a_str_format("Could not find expression for member '%s' at index pos %u!\n", as_file_title().as_cstr(), m_source_idx),
          SkLocale_local,
          SkDPrintType_warning);

        m_source_idx = SkExpr_char_pos_invalid;

        return nullptr;
        }

      m_source_idx = m_expr_p->m_source_idx;
      }
  #endif  // (SKOOKUM & SK_DEBUG)

  return m_expr_p;
  }

//---------------------------------------------------------------------------------------
// Gets the source index position of the expression.
// Author(s):   Conan Reis
uint32_t SkMemberExpression::get_source_idx()
  {
  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase * expr_p = get_expr();

    return expr_p ? expr_p->m_source_idx : 0u;
  #else
    return 0u;
  #endif
  }

//---------------------------------------------------------------------------------------
// Determine if the source that this represents comes from standard script files.
// Author(s):   Conan Reis
bool SkMemberExpression::is_origin_source() const
  {
  #if (SKOOKUM & SK_DEBUG)
    return (m_expr_p == nullptr)
      || ((m_expr_p->m_debug_info & SkDebugInfo::Flag_origin__mask) == SkDebugInfo::Flag_origin_source);
  #else
    return true;
  #endif
  }

//---------------------------------------------------------------------------------------
// Remove hard pointer to expression structure.
// Called when expressions are being reparsed, etc.
void SkMemberExpression::release_expr()
  {
  m_expr_p = nullptr;
  // Note that the value in `m_source_idx` is kept so that `m_expr_p` can be recached in
  // a later call to `get_expr()`.
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Set the member and expression based on the supplied context.
// Params:
//   scope_p:  scope for the expression
//   caller_p: invokable for the expression
//   expr_p:   expression context
// Author(s):   Conan Reis
void SkMemberExpression::set_context(
  SkObjectBase *     scope_p,
  SkInvokedBase *    caller_p,
  SkExpressionBase * expr_p
  )
  {
  set_context(scope_p, caller_p);

  m_expr_p = expr_p;

  m_source_idx = expr_p ? expr_p->m_source_idx : SkExpr_char_pos_invalid;
  }

#endif


//---------------------------------------------------------------------------------------
// Constructor from binary stream
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary stream and to increment
//             - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               9 bytes - Member info
//               2 bytes - expression so7urce index
// Author(s):   Conan Reis
SkMemberExpression::SkMemberExpression(const void ** binary_pp) :
  SkMemberInfo(binary_pp),
  m_expr_p(nullptr)
  {
  // 2 bytes - expression source index
  m_source_idx = A_BYTE_STREAM_UI16_INC(binary_pp);

  // $Note - could call m_expr_p = find_expr_by_pos(m_source_idx) but relying on it being
  // retrieved lazily in the future.
  }


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with binary information needed to
//             recreate this literal and its components and increments the memory address
//             to just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the as_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               9 bytes - Member info
//               2 bytes - expression source index
// Author(s):   Conan Reis
void SkMemberExpression::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkMemberExpression::as_binary_length());

  // 9 bytes - Member info
  SkMemberInfo::as_binary(binary_pp);

  // 2 bytes - expression source index
  A_BYTE_STREAM_OUT16(binary_pp, &m_source_idx);
  }

#endif


#if (SKOOKUM & SK_DEBUG)

//=======================================================================================
// SkBreakPoint Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Breakpoint constructor - must call reaquire_expr() after breakpoint added
//             to tracking arrays.  Enabled by default if break_expr_p is non-nullptr.
// Author(s):   Conan Reis
SkBreakPoint::SkBreakPoint(
  const SkMemberInfo & member_info,
  SkExpressionBase *   break_expr_p // = nullptr
  ) :
  SkMemberExpression(member_info, break_expr_p),
  m_table_idx(0u),
  m_enabled(break_expr_p != nullptr)
  {
  }

//---------------------------------------------------------------------------------------
// Breakpoint constructor - must call reaquire_expr() after breakpoint added
//             to tracking arrays
// Author(s):   Conan Reis
SkBreakPoint::SkBreakPoint(
  const SkMemberExpression & info,
  uint32_t table_idx,
  bool enabled // = true
  ) :
  SkMemberExpression(info),
  m_table_idx(table_idx),
  m_enabled(enabled)
  {
  }

//---------------------------------------------------------------------------------------
// Reacquire debug expression if it had gone stale or if `force = true` and recache all
// relevant information from expression to breakpoint and from breakpoint to expression.
// Author(s):   Conan Reis
void SkBreakPoint::reaquire_expr(
  bool force // = false
  ) const
  {
  if (force)
    {
    m_expr_p = nullptr;
    }

  SkExpressionBase * expr_p = SkMemberExpression::get_expr();

  if (expr_p == nullptr)
    {
    return;
    }

  uint16_t debug_flags = (expr_p->m_debug_info & SkDebugInfo::Flag_origin__mask) | uint16_t(m_table_idx);

  if (m_enabled)
    {
    debug_flags |= uint16_t(SkDebugInfo::Flag_debug_enabled);
    }

  expr_p->m_debug_info = debug_flags;
  }

//---------------------------------------------------------------------------------------
// Release debug expression - clears all data in expression associated with this breakpoint
// Author(s):   Conan Reis
void SkBreakPoint::release_expr()
  {
  if (m_expr_p)
    {
    release_expr(m_expr_p);
    m_expr_p = nullptr;
    }

  // Note that the value in `m_source_idx` is kept so that `m_expr_p` can be recached in
  // a later call to `get_expr()`.
  }

//---------------------------------------------------------------------------------------
// Clears all data in expression associated with this breakpoint
void SkBreakPoint::release_expr(SkExpressionBase * expr_p)
  {
  expr_p->m_debug_info = SkDebugInfo::Flag_debug_idx__none | (expr_p->m_debug_info & SkDebugInfo::Flag_origin__mask);
  }

//---------------------------------------------------------------------------------------
// Get the expression associated with this breakpoint
// Author(s):   Conan Reis
SkExpressionBase * SkBreakPoint::get_expr() const
  {
  if (m_expr_p == nullptr)
    {
    reaquire_expr();
    }

  return m_expr_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkBreakPoint::enable(
  bool set_break // = true
  )
  {
  if (m_enabled != set_break)
    {
    m_enabled = set_break;

    SkExpressionBase * expr_p = get_expr();

    if (expr_p)
      {
      if (set_break)
        {
        expr_p->m_debug_info |= uint16_t(SkDebugInfo::Flag_debug_enabled);
        }
      else
        {
        expr_p->m_debug_info &= ~uint16_t(SkDebugInfo::Flag_debug_enabled);
        }
      }
    }
  }

//=======================================================================================
// SkWatch Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// If var_address is 0, var_p is used as var address
SkWatch::SkWatch(
  Kind               kind,
  const ASymbol &    var_name,
  const ASymbol &    scope_name,
  const SkInstance * var_p,
  uint64_t           var_guid
  )
  : m_kind(kind)
  , m_var_name(var_name)
  , m_type_name(var_p->get_class()->get_name())
  , m_scope_name(scope_name)
  , m_value(var_p->as_watch_value())
  , m_var_guid(var_guid)
  , m_ref_count(var_p->is_ref_counted() ? var_p->get_references() : SkInstanceUnreffed_infinite_ref_count)
  , m_ptr_id(var_p->m_ptr_id)
  , m_obj_address((uintptr_t)var_p)
  {
  }

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------

void SkWatch::assign_binary(const void ** binary_pp)
  {
  // 1 byte - argument type
  m_kind = Kind(A_BYTE_STREAM_UI8_INC(binary_pp));

  // 4 bytes - var name
  m_var_name = ASymbol::create_from_binary(binary_pp);

  // 4 bytes - type name
  m_type_name = ASymbol::create_from_binary(binary_pp);

  // 4 bytes - scope name
  m_scope_name = ASymbol::create_from_binary(binary_pp);

  // n bytes - value string
  m_value.assign_binary(binary_pp);

  // 8 bytes - var guid
  m_var_guid = A_BYTE_STREAM_UI64_INC(binary_pp);

  // 4 bytes - ref count
  m_ref_count = A_BYTE_STREAM_UI32_INC(binary_pp);

  // 4 bytes - id
  m_ptr_id = A_BYTE_STREAM_UI32_INC(binary_pp);

  // 8 bytes - obj address
  m_obj_address = A_BYTE_STREAM_UI64_INC(binary_pp);

  // n bytes - children
  m_children.ensure_size_empty(64);
  uint32_t count = A_BYTE_STREAM_UI32_INC(binary_pp);
  while (count--)
    {
    m_children.append(*new SkWatch(binary_pp));
    }

  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------

void SkWatch::as_binary(void ** binary_pp) const
  {
  // 1 byte - type of variable  
  *(*(uint8_t **)binary_pp)++ = uint8_t(m_kind);

  // 4 bytes - var name
  m_var_name.as_binary(binary_pp);

  // 4 bytes - type name
  m_type_name.as_binary(binary_pp);

  // 4 bytes - scope name
  m_scope_name.as_binary(binary_pp);

  // n bytes - value string
  m_value.as_binary(binary_pp);

  // 8 bytes - var guid
  A_BYTE_STREAM_OUT64(binary_pp, &m_var_guid);

  // 4 bytes - ref count
  A_BYTE_STREAM_OUT32(binary_pp, &m_ref_count);

  // 4 bytes - id
  A_BYTE_STREAM_OUT32(binary_pp, &m_ptr_id);

  // 8 bytes - obj address
  A_BYTE_STREAM_OUT64(binary_pp, &m_obj_address);

  // n bytes - children
  m_children.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------

uint32_t SkWatch::as_binary_length() const
  {
  return 37u + m_value.as_binary_length() + m_children.as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

//=======================================================================================
// SkCallStack Methods
//=======================================================================================

#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------

void SkCallStack::assign_binary(const void ** binary_pp)
  {
  m_stack.ensure_size_empty(64);

  // 2 bytes - initial level to show
  m_current_level_idx = A_BYTE_STREAM_UI16_INC(binary_pp);

  // n bytes - the stack
  uint32_t count = A_BYTE_STREAM_UI32_INC(binary_pp);
  while (count--)
    {
    m_stack.append(*new Level(binary_pp));
    }
  }

//---------------------------------------------------------------------------------------

SkCallStack::Level::Level(const void ** binary_pp)
  : SkMemberExpression(binary_pp)
  {
  // n bytes - label
  m_label.assign_binary(binary_pp);

  // 1 byte - is_context
  m_is_context = !!(A_BYTE_STREAM_UI8_INC(binary_pp));

  // n bytes - locals
  m_locals.ensure_size_empty(64);
  uint32_t count = A_BYTE_STREAM_UI32_INC(binary_pp);
  while (count--)
    {
    m_locals.append(*new SkWatch(binary_pp));
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)

#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------

void SkCallStack::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkCallStack::as_binary_length());

  // 2 bytes - initial level to show
  A_BYTE_STREAM_OUT16(binary_pp, &m_current_level_idx);

  // n bytes - tha stack
  m_stack.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------

uint32_t SkCallStack::as_binary_length() const
  {
  return sizeof(m_current_level_idx) + m_stack.as_binary_length();
  }

//---------------------------------------------------------------------------------------

void SkCallStack::Level::as_binary(void ** binary_pp) const
  {
  // n bytes - member expression
  SkMemberExpression::as_binary(binary_pp);

  // n bytes - label
  m_label.as_binary(binary_pp);

  // 1 byte - is_context
  *(*(uint8_t **)binary_pp)++ = uint8_t(m_is_context);

  // n bytes - locals
  m_locals.as_binary(binary_pp);
  }

//---------------------------------------------------------------------------------------

uint32_t SkCallStack::Level::as_binary_length() const
  {
  return 1u + SkMemberExpression::as_binary_length() + m_label.as_binary_length() + m_locals.as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)

#endif // (SKOOKUM & SK_DEBUG)

//=======================================================================================
// SkDebug::Hook Methods
//=======================================================================================

#if defined(SKDEBUG_HOOKS)

//---------------------------------------------------------------------------------------
// Constructor
// Arg         hook_name - name of execution hook
// Author(s):   Conan Reis
SkDebug::Hook::Hook(const AString & hook_name) :
  ANamed(ASymbol::create(hook_name)),
  m_hook_name(hook_name),
  m_hook_method_f(nullptr),
  m_hook_coroutine_f(nullptr),
  m_hook_script_entry_f(nullptr),
  m_hook_script_exit_f(nullptr),
  m_updater_class_p(nullptr),
  m_updater_subclass_check(false),
  m_flags(Flag__none)
  {
  update_flags();
  }

//---------------------------------------------------------------------------------------
// Copy Constructor
// Arg         hook - execution hook to copy settings from
// Author(s):   Conan Reis
SkDebug::Hook::Hook(const Hook & hook) :
  ANamed(hook),
  m_hook_name(hook.m_hook_name),
  m_hook_method_f(hook.m_hook_method_f),
  m_hook_coroutine_f(hook.m_hook_coroutine_f),
  m_hook_script_entry_f(hook.m_hook_script_entry_f),
  m_hook_script_exit_f(hook.m_hook_script_exit_f),
  m_updater_class_p(hook.m_updater_class_p),
  m_updater_subclass_check(hook.m_updater_subclass_check),
  m_scope_actor_name(hook.m_scope_actor_name),
  m_invoked_caller_p(hook.m_invoked_caller_p),
  m_flags(Flag__none)  // Flags not copied
  {
  update_flags();
  }

//---------------------------------------------------------------------------------------
// Copy Constructor
// Arg         hook - execution hook to copy settings from
// Author(s):   Conan Reis
SkDebug::Hook & SkDebug::Hook::operator=(const Hook & hook)
  {
  m_name                   = hook.m_name;
  m_hook_name              = hook.m_hook_name;
  m_hook_method_f          = hook.m_hook_method_f;
  m_hook_coroutine_f       = hook.m_hook_coroutine_f;
  m_hook_script_entry_f    = hook.m_hook_script_entry_f;
  m_hook_script_exit_f     = hook.m_hook_script_exit_f;
  m_updater_class_p        = hook.m_updater_class_p;
  m_updater_subclass_check = hook.m_updater_subclass_check;
  m_scope_actor_name       = hook.m_scope_actor_name;
  m_invoked_caller_p       = hook.m_invoked_caller_p;
  m_flags                  = Flag__none;  // // Flags not copied

  update_flags();

  return *this;
  }

//---------------------------------------------------------------------------------------
// Sets the name of the hook prior to being appended
// Arg         name - name to use for hook group
// See:        append_hook()
// Author(s):   Conan Reis
void SkDebug::Hook::set_name(const AString & name)
  {
  m_hook_name = name;
  m_name      = ASymbol::create(name);
  }

//---------------------------------------------------------------------------------------
// Updates condition flags based on its data members.
// Author(s):   Conan Reis
void SkDebug::Hook::update_flags()
  {
  // Remove condition flags
  m_flags &= ~Flag__condition_mask;

  // Reset condition flags
  if (m_updater_class_p)
    {
    m_flags |= Flag_only_updater_class;
    }

  if (!m_scope_actor_name.is_null())
    {
    m_flags |= Flag_only_scope_name;
    }

  if (m_invoked_caller_p.is_set())
    {
    m_flags |= Flag_only_invoked_caller;
    }
  }

//---------------------------------------------------------------------------------------
// Determines if the current conditions are met in order to run the hook.
// Returns:    true if hook should be run and false if not
// Arg         icontext_p - current call context
// Author(s):   Conan Reis
bool SkDebug::Hook::is_conditions_met(SkInvokedContextBase * icontext_p) const
  {
  // $Revisit - CReis This can probably be sped up with inlining, reordering, moving the
  // tests to different areas of the code, swapping in different functions, etc.

  if (m_flags & Flag__condition_mask)
    {
    if (m_flags & Flag_only_updater_class)
      {
      // $Revisit - CReis [Efficiency] This should probably be a test of the flags
      // in SkMind::on_update().
      SkMind * updater_p = icontext_p->get_updater();

      if (updater_p == nullptr)
        {
        return false;
        }

      SkClass * current_class_p = updater_p->get_class();
        
      if (m_updater_subclass_check
        ? (!current_class_p->is_class(*m_updater_class_p))
        : (current_class_p != m_updater_class_p))
        {
        return false;
        }
      }

    if (m_flags & Flag_only_scope_name)
      {
      // $Revisit - CReis [Efficiency] This should probably be a test of the flags
      // in SKDEBUG_HOOK_METHOD() and SKDEBUG_HOOK_COROUTINE().
      SkInstance * receiver_p = icontext_p->get_topmost_scope();

      if ((receiver_p == nullptr) || (receiver_p->get_name_debug() != m_scope_actor_name))
        {
        return false;
        }
      }

    if (m_flags & Flag_only_invoked_caller)
      {
      // $Revisit - CReis [Efficiency] Not sure if there is a faster way to do this test
      SkInvokedContextBase * icaller_p = m_invoked_caller_p;

      if (icaller_p == nullptr)
        {
        // Invokable is stale - no point in tracking it
        enable_hook(m_name, false);

        return false;
        }

      if (!icontext_p->is_caller(icaller_p))
        {
        return false;
        }
      }
    }

  return true;
  }

#endif  // SKDEBUG_HOOKS


//=======================================================================================
// SkDebug Class Data Members
//=======================================================================================

bool SkDebug::ms_no_step_default_hack = false;

bool                       SkDebug::ms_engine_present_b = false;
bool                       SkDebug::ms_suppress_prints = false;
void                    (* SkDebug::ms_scripted_break_f)(const AString & message, SkInvokedMethod * scope_p) = nullptr;
tSkPrintFunc *             SkDebug::ms_log_func_p = nullptr;
AFreePtr<AFunctionBase>    SkDebug::ms_print_mem_ext_p;

#if (SKOOKUM & SK_DEBUG)
  uint32_t                      SkDebug::ms_pref_flags     = SkDebug::PrefFlag__default;
  SkDebug::eState               SkDebug::ms_exec_state     = SkDebug::State_running;
  SkMemberExpression            SkDebug::ms_next_expr;
  APArray<SkBreakPoint>         SkDebug::ms_breakpoint_table;
  AIdPtr<SkInvokedContextBase>  SkDebug::ms_current_call_p;
  AIdPtr<SkInvokedBase>         SkDebug::ms_next_invokable_p;

  SkDebug::eStep                SkDebug::ms_step_type      = SkDebug::Step__none;
  AIdPtr<SkInvokedContextBase>  SkDebug::ms_step_icontext_p;
  AIdPtr<SkInvokedBase>         SkDebug::ms_step_topmost_caller_p;
  SkExpressionBase *            SkDebug::ms_step_expr_p    = nullptr;

  APSortedLogicalFree<SkBreakPoint, SkMemberExpression> SkDebug::ms_breakpoints;
#endif

uint32_t  SkDebug::ms_indent_size = AString_indent_spaces_def;
uint32_t  SkDebug::ms_tab_stops   = AString_tab_stop_def;

#if defined(SKDEBUG_COMMON)
  uint32_t SkDebug::ms_expr_hook_flag = SkDebugInfo::Flag_debug_disabled;
  uint32_t SkDebug::ms_flags          = SkDebug::Flag__default;
#endif

#if defined(SKDEBUG_HOOKS)

  void (* SkDebug::ms_hook_expr_f)(SkExpressionBase * expr_p, SkObjectBase * scope_p, SkInvokedBase * caller_p) = nullptr;

  APSortedLogicalFree<SkDebug::Hook, ASymbol> SkDebug::ms_hooks;
  AList<SkDebug::Hook, tSkMethodHook>         SkDebug::ms_hook_methods;
  AList<SkDebug::Hook, tSkCoroutineHook>      SkDebug::ms_hook_coroutines;
  AList<SkDebug::Hook, tSkScriptSystemHook>   SkDebug::ms_hook_origins;

  ASymbol  SkDebug::ms_hook_script_origin_stack[8u];
  uint32_t SkDebug::ms_hook_script_origin_idx = 0;
#endif


//=======================================================================================
// SkDebug Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Initializes Debug structures prior to loading Skookum scripts.
// 
// #Notes
//             The initialization sequence is:
//               1 SkookumScript::initialize()
//               2 [Load SkookumScript Code or compiled binary code]
//               3 SkookumScript::initialize_post_load()
//               4 [Initialize game specific atomics]
//              *5 SkookumScript::initialize_session()
//               6 SkookumScript::initialize_instances()
//               7 [... use SkookumScript]
//               8 SkookumScript::deinitialize_instances()
//               9 SkookumScript::deinitialize_session()
//              10 SkookumScript::deinitialize()
//  
// Modifiers:  static
// Author(s):  Conan Reis
void SkDebug::initialize()
  {
  #if (SKOOKUM & SK_DEBUG)
    static bool s_register_skookum_context = false;

    if (!s_register_skookum_context)
      {
      ADebug::register_context_func(new AFunctionArg<AString *>(SkDebug::context_append));
      s_register_skookum_context = true;
      }
  #endif

  #if defined(SKDEBUG_HOOKS)
    ms_hook_script_origin_idx = 0;
    ms_hook_script_origin_stack[0u] = ASymbolX_unnamed;

    // $Revisit - CReis Turn off example hooks for now
    // Should probably put them in some more "user" centric place other than SkDebug.
    //hook_examples();
  #endif
  }

//---------------------------------------------------------------------------------------
// Cleans up debug structures for Skookum shutdown
// 
// #Notes
//             The initialization sequence is:
//               1 SkookumScript::initialize()
//               2 [Load SkookumScript Code or compiled binary code]
//               3 SkookumScript::initialize_post_load()
//               4 [Initialize game specific atomics]
//              *5 SkookumScript::initialize_session()
//               6 SkookumScript::initialize_instances()
//               7 [... use SkookumScript]
//               8 SkookumScript::deinitialize_instances()
//               9 SkookumScript::deinitialize_session()
//              10 SkookumScript::deinitialize()
//  
// Modifiers:  static
// Author(s):  Conan Reis
void SkDebug::deinitialize()
  {
  ms_print_mem_ext_p.null();

  #if (SKOOKUM & SK_DEBUG)
    ms_breakpoints.empty_compact();
    ms_breakpoint_table.free_all_compact();
  #endif

  #if defined(SKDEBUG_HOOKS)
    //ms_hook_script_origin_stack[0u] = ASymbolX_unnamed;
    ms_hooks.free_all_compact();
  #endif
  }

#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Get debug info of next expression that would be invoked after this breakpoint
SkDebugInfo SkDebug::get_next_debug_info()
  {
  // Use next expression if it is set
  SkMemberExpression & next_expr_info = SkDebug::get_next_expression();
  SkExpressionBase *   next_expr_p = next_expr_info.is_valid() ? next_expr_info.get_expr() : nullptr;

  if (next_expr_p)
    {
    return { next_expr_p->m_source_idx, next_expr_p->m_debug_info };
    }
  else
    {
    // Use the most recent invokable
    return { SkDebugInfo::ms_expr_p->m_source_idx, SkDebugInfo::ms_expr_p->m_debug_info };
    }
  }

//---------------------------------------------------------------------------------------
// Get callstack for this invokable
SkCallStack * SkDebug::get_callstack(const SkInvokedBase * invoked_p, const SkMemberExpression * initial_member_expr_p, uint32_t stack_flags /*= SkInvokeInfo__callstack_def*/)
  {
  return get_callstack(invoked_p, nullptr, initial_member_expr_p, stack_flags);
  }

//---------------------------------------------------------------------------------------
// Get callstack for this invokable
// `invoked_caller_p` is the caller of `invoked_p`, supplied when `invoked_p` is a branch
SkCallStack * SkDebug::get_callstack(const SkInvokedBase * invoked_p, const SkInvokedBase * invoked_caller_p, const SkMemberExpression * initial_member_expr_p, uint32_t stack_flags /*= SkInvokeInfo__callstack_def*/)
  {
  SkCallStack * callstack_p = new SkCallStack;
  callstack_p->m_stack.ensure_size(64);

  SkInvokedContextBase * context_p;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Skip this entire context if desired
  if (stack_flags & SkInvokeInfo_skip_this)
    {
    // Jump up to context and get its caller
    context_p = invoked_p->get_caller_context();
    if (context_p)
      {
      invoked_p = context_p->m_caller_p;
      // If it doesn't know its caller, use the supplied caller, but only once
      if (!invoked_p)
        {
        invoked_p = invoked_caller_p;
        invoked_caller_p = nullptr;
        }
      initial_member_expr_p = nullptr;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Crawl up the callstack
  while (invoked_p)
    {
    // The context of this invokable (same or a caller up the stack)
    context_p = invoked_p->get_caller_context();

    SkMemberExpression member_expr;
    SkInvokableBase * invokable_p = context_p->get_invokable();
    if (initial_member_expr_p && initial_member_expr_p->is_valid())
      {
      member_expr = *initial_member_expr_p;
      // If we are in peek mode, use initial_member_expr_p twice
      if (!(stack_flags & SkInvokeInfo_peek) || callstack_p->m_stack.get_length() > 0)
        {
        initial_member_expr_p = nullptr;
        }
      }
    else
      {
      const SkClosureInfoBase * closure_info_p = invokable_p ? invokable_p->get_closure_info() : nullptr;
      if (closure_info_p)
        {
        // For closures, insert the parent member info instead since closures itself aren't actual members
        closure_info_p->get_member_info(&member_expr);
        }
      else
        {
        member_expr.set_context(context_p, context_p->get_caller(), nullptr);
        }

      // Get debug info of current invoked
      member_expr.m_source_idx = invoked_p->get_debug_info().m_source_idx;
      // Or, if it's a context, try to get the debug info of its top expression instead
      if (invoked_p == context_p)
        {
        const SkExpressionBase * expr_p = context_p->get_invokable()->get_custom_expr();
        if (expr_p)
          {
          member_expr.m_source_idx = expr_p->m_source_idx;
          }
        }
      }
    // Build string with arguments of this invoked
    AString label;
    if (invoked_p == context_p)
      {
      const SkClosureInfoBase * closure_info_p = invokable_p ? invokable_p->get_closure_info() : nullptr;
      if (closure_info_p)
        {
        label = closure_info_p->is_method() ? "closure method" : "closure coroutine";
        }
      else
        {
        // It is important to store a local copy of ms_current_call_p since the process of making
        // the callstack string may call Skookum methods and coroutines and change it.
        SkInvokedContextBase * current_call_p = ms_current_call_p;
        // Build invoke string, potentially invoking Sk routines to generate it
        label = context_p->as_invoke_string(stack_flags);
        // Put the current call back.
        ms_current_call_p = current_call_p;
        }
      }
    else
      {
      SK_ASSERTX(invoked_p->get_obj_type() == SkObjectType_invoked_expr, "If not an SkInvokedContextBase, must be an SkInvokedExpression!");
      label = static_cast<const SkInvokedExpression *>(invoked_p)->get_expr()->as_callstack_label();
      }
    // Create a new stack level
    SkCallStack::Level * level_p = new SkCallStack::Level(member_expr, label, invoked_p == context_p);
    // Add locals
    append_watch_locals(&level_p->m_locals, invoked_p);
    // Store this level
    callstack_p->m_stack.append(*level_p);

    // Keep crawling up the stack
    invoked_p = invoked_p->get_caller();
    // If it doesn't know its caller, use the supplied caller, but only once
    if (!invoked_p)
      {
      invoked_p = invoked_caller_p;
      invoked_caller_p = nullptr;
      }
    }

  // Set level where we are currently at
  callstack_p->m_current_level_idx = (uint16_t)a_min((stack_flags & SkInvokeInfo_peek) ? 1u : 0u, callstack_p->m_stack.get_length() - 1);

  return callstack_p;
  }

//---------------------------------------------------------------------------------------

void SkDebug::append_watch_locals(APArray<SkWatch> * m_locals_p, const SkInvokedBase * invoked_p)
  {
  SkInvokedContextBase *  context_p  = invoked_p->get_caller_context();
  SkInstance **           var_pp     = context_p->get_data().get_array();
  SkInstance *            this_p     = context_p->get_this();
  ASymbol                 scope_name = this_p ? this_p->get_class()->get_name() : ASymbol::ms_null;

  if (invoked_p == context_p)
    {
    // "this"
    if (this_p && !this_p->is_metaclass())
      {
      // If `this` has a raw pointer, use that to identify the variable, as `this` can never be reassigned, otherwise, use the context which contains it
      uint64_t var_guid = this_p->get_class()->get_raw_pointer_func() ? (uintptr_t)this_p->get_class()->get_raw_pointer(this_p) : (uintptr_t)context_p;
      SkWatch * this_watch_p = new SkWatch(SkWatch::Kind_this, ASymbol_this, scope_name, this_p, var_guid);
      append_watch_members(&this_watch_p->m_children, this_p); // Always append members of `this` by default
      m_locals_p->append(*this_watch_p);
      }

    const SkInvokableBase *    invokable_p        = context_p->get_invokable();
    const tSkParamList &       param_list         = invokable_p->get_params().get_param_list();
    const tSkParamReturnList & return_param_list  = invokable_p->get_params().get_param_return_list();

    // Captured vars if any
    const SkClosureInfoBase * info_p = invokable_p->get_closure_info();
    if (info_p)
      {
      for (const SkNamedIndexed & var_name : info_p->get_captured())
        {
        m_locals_p->append(*new SkWatch(SkWatch::Kind_captured, var_name, scope_name, *var_pp, (uintptr_t)var_pp));
        ++var_pp;
        }
      }

    // Parameters if any
    for (const SkParameterBase * param_p : param_list)
      {
      m_locals_p->append(*new SkWatch(SkWatch::Kind_parameter, param_p->get_name(), scope_name, *var_pp, (uintptr_t)var_pp));
      ++var_pp;
      }

    // Return parameters if any
    for (const SkTypedName * param_p : return_param_list)
      {
      m_locals_p->append(*new SkWatch(SkWatch::Kind_return_parameter, param_p->get_name(), scope_name, *var_pp, (uintptr_t)var_pp));
      ++var_pp;
      }
    }
  else
    {
    // Temporaries if any
    const SkExpressionBase * expr_p = invoked_p->get_expr();
    if (expr_p && expr_p->get_type() == SkExprType_code)
      {
      const SkCode * code_p = static_cast<const SkCode *>(expr_p);
      var_pp += code_p->get_temp_vars_start_idx();
      for (const ASymbol & var_name : code_p->get_temp_vars())
        {
        m_locals_p->append(*new SkWatch(SkWatch::Kind_temporary, var_name, scope_name, *var_pp, (uintptr_t)var_pp));
        ++var_pp;
        }
      }
    }
  }

//---------------------------------------------------------------------------------------

void SkDebug::append_watch_members(APArray<SkWatch> * m_members_p, SkInstance * obj_p)
  {
  SkClass * class_p = obj_p->get_key_class();

  // Loop through classes - used to get scope rather than using data table
  uint32_t data_idx = class_p->get_total_data_count();
  for (SkClass * current_class_p = class_p; current_class_p; current_class_p = current_class_p->get_superclass())
    {
    // 1) Sk instance data
    uint32_t var_count = current_class_p->get_instance_data().get_length();
    data_idx -= var_count;
    for (SkTypedName * var_p : current_class_p->get_instance_data())
      {
      SkInstance ** var_pp = static_cast<SkDataInstance *>(obj_p)->get_data_addr_by_idx(data_idx++);
      m_members_p->append(*new SkWatch(SkWatch::Kind_instance_data, var_p->get_name(), current_class_p->get_name(), *var_pp, (uintptr_t)var_pp));
      }
    data_idx -= var_count;

    // 2) raw instance data
    for (SkTypedNameRaw * var_p : current_class_p->get_instance_data_raw())
      {
      SkClassDescBase * data_type_p = var_p->m_type_p; // Type of the data member itself
      // Ask the owner class and data class to fetch the raw data member for us
      SkInstance * data_p = SkBrain::ms_nil_p;
      void * member_p = class_p->get_raw_pointer(obj_p); // Pointer to raw memory of the object containing the data member
      if (member_p)
        {
        data_p = data_type_p->get_key_class()->new_instance_from_raw_data(member_p, var_p->m_raw_data_info, data_type_p);
        }
      // For the var guid, we just xor the object address with the raw data info
      m_members_p->append(*new SkWatch(SkWatch::Kind_instance_data, var_p->get_name(), current_class_p->get_name(), data_p, (uintptr_t)member_p ^ var_p->m_raw_data_info));
      data_p->dereference();
      }
    }

  // Loop through classes - used to get scope rather than using data table
  for (SkClass * current_class_p = class_p; current_class_p; current_class_p = current_class_p->get_superclass())
    {
    uint32_t var_count = current_class_p->get_class_data().get_length();
    if (var_count)
      {
      // $Revisit - CReis Alphabetically sort by name
      SkInstance ** value_pp = current_class_p->get_class_data_values().get_array();
      SkTypedName ** var_pp = current_class_p->get_class_data().get_array();
      for (uint32_t i = 0; i < var_count; ++i)
        {
        m_members_p->append(*new SkWatch(SkWatch::Kind_class_data, var_pp[i]->get_name(), current_class_p->get_name(), value_pp[i], (uintptr_t)(value_pp + i)));
        }
      }
    }
  }

#endif

//---------------------------------------------------------------------------------------
// Appends the script callstack for the supplied invoked object or if nullptr
//             for the currently running invoked object if there is one to the supplied
//             string.
// Arg         invoked_p - invoked object to get the callstack for.  If nullptr the currently
//             running invoked object is used if there is one.
// Arg         stack_flags - see eSkInvokeInfo
// Notes:      This can be called in the C++ Debugging "Immediate" window to print out the
//             current script callstack.
// Modifiers:   static.
// Author(s):   Conan Reis
void SkDebug::append_callstack_string(
  AString *       str_p,
  SkInvokedBase * invoked_p,  // = nullptr
  uint32_t        stack_flags // = SkInvokeInfo__callstack_def
  )
  {
  #if (SKOOKUM & SK_DEBUG)
    bool current_call = false;

    // It is important to store a local copy of ms_current_call_p since the process of making
    // the callstack string may call Skookum methods and coroutines and change it.
    SkInvokedContextBase * current_context_p = ms_current_call_p;

    if (invoked_p == nullptr)
      {
      invoked_p    = current_context_p;
      current_call = true;
      }

    if (invoked_p)
      {
      AString callstack_str;

      ARefPtr<SkCallStack> callstack_p = get_callstack(invoked_p, &get_next_expression(), stack_flags);

      for (const SkCallStack::Level * level_p : callstack_p->m_stack)
        {
        callstack_str.append(level_p->m_label);

        if (level_p->m_source_idx != SkExpr_char_pos_invalid
         && (stack_flags & SkInvokeInfo_index))
          {
          callstack_str.append(" [", 2);
          callstack_str.append(AString::ctor_uint(level_p->m_source_idx));
          callstack_str.append(']');
          }

        callstack_str.append('\n');
        }

      callstack_str.line_indent(SkDebug::ms_indent_size);

      str_p->ensure_size(str_p->get_length() + 45u + callstack_str.get_length());

      // Put the current call back.
      ms_current_call_p = current_context_p;

      if (current_call)
        {
        str_p->append("\nSkookum current call stack:", 27u);
        }
      else
        {
        str_p->append("\nSkookum call stack:", 19u);
        }

      str_p->append(callstack_str);
      str_p->append('\n');
      }
    else
  #endif
      {
      if ((stack_flags & SkInvokeInfo_ignore_absent) == 0u)
        {
        // No debug info available
        str_p->append("\n[Skookum call stack not available.]\n", 37u);
        }
      }
  }

//---------------------------------------------------------------------------------------
// Get string dump of the locals from supplied invoked context (or current
//             context).
// Arg         str_p - string to append to
// Arg         invoked_p - invoked context to use.  If nullptr use current context (if there
//             is one)
// Arg         flags - see eSkInvokeInfo
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::append_locals_string(
  AString *       str_p,
  SkInvokedBase * caller_p,
  SkInvokedBase * invoked_p, // = nullptr
  uint32_t        flags      // = SkInvokeInfo__locals_def
  )
  {
  #if (SKOOKUM & SK_DEBUG)
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    struct Nested
      {
      static void append_str_variable(AString * out_str_p, const ASymbol & name, const AString & scope, SkInstance * obj_p, SkClass * expected_p = nullptr)
        {
        out_str_p->ensure_size_extra(256u);

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Get reference count string
        char     ref_cstr_p[256u];
        AString  ref_str(ref_cstr_p, 256u, 0u);

        if (obj_p && obj_p->is_ref_counted())
          {
          // $Revisit - CReis This may needlessly create a string buffer on the heap
          ref_str.append(AString::ctor_uint(obj_p->get_references()));
          }
        else
          {
          ref_str.append('-');
          }


        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Get Type(ExpectedType) string
        char      type_cstr_p[256u];
        AString   type_str(type_cstr_p, 256u, 0u);
        SkClass * type_p = nullptr;

        if (obj_p)
          {
          type_p = obj_p->get_class();
          type_str.append(type_p->get_name_cstr_dbg());
          }
        else
          {
          type_str.append("<??""?>", 5u); // Two inert quotes to disable the unwanted trigraph here 
          }

        if (expected_p && (expected_p != type_p))
          {
          type_str.append('/');
          type_str.append(expected_p->get_name_str_dbg());
          }

        out_str_p->append_format(
          "  %-35s | %-20s | %5s | %-23s | ",
          name.as_cstr_dbg(),
          scope.as_cstr(),
          ref_cstr_p,
          type_cstr_p);

        if (obj_p)
          {
          obj_p->as_code_append(out_str_p);
          }
        else
          {
          out_str_p->append("<instance is nullptr>");
          }

        out_str_p->append('\n');
        }
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if supplied invoked object or current invoked object (if one exists)
    // should be used.
    // It is important to store a local copy of ms_current_call_p since the process of
    // converting some objects to strings may call Skookum methods and coroutines.
    bool                   current_call      = false;
    SkInvokedContextBase * context_call_p    = invoked_p ? invoked_p->get_caller_context() : nullptr;
    SkInvokedContextBase * current_context_p = ms_current_call_p;

    if (context_call_p == nullptr)
      {
      context_call_p = current_context_p ? current_context_p->get_caller_context() : nullptr;
      current_call   = true;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Start with this invokable or skip this invokable?
    if ((flags & SkInvokeInfo_skip_this) && context_call_p)
      {
      // Skip the most recent invokable
      context_call_p = context_call_p->get_caller_context();
      }

    uint32_t inital_length = str_p->get_length();

    if (context_call_p)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Build locals string

      SkInvokableBase * invokable_p = context_call_p->get_invokable();

      str_p->ensure_size_extra(256u);
      str_p->append("\nSkookum Local Variables - ", 27u);
      str_p->append(invokable_p->as_string_name(invokable_p->get_scope() != nullptr));
      str_p->append_format(" [Invoked id = %u]\n", context_call_p->m_ptr_id);

      str_p->append("= Name ===============================+ Scope ===============+= Refs + Class/expected =========+ Value =========\n", 113u);

      const tSkParamList &       param_list        = invokable_p->get_params().get_param_list();
      const tSkParamReturnList & return_param_list = invokable_p->get_params().get_param_return_list();

      // Figure out if we are inside a closure
      SkClosure * closure_p = nullptr;
      if (invoked_p)
        {
        SkObjectBase * obj_p = invoked_p->get_scope();
        if (obj_p && obj_p->get_obj_type() == SkObjectType_closure)
          {
          closure_p = static_cast<SkClosure *>(obj_p);
          }
        }

      // The indexes of the arguments and the parameters should match so there should be no
      // need to look them up by name.
      // The number of arguments can be less than the number of parameters if printing out
      // during the initialization of the argument list - such as with default arguments.

      SkInstance ** data_pp         = context_call_p->get_data().get_array();
      uint32_t      captured_count  = closure_p ? closure_p->get_captured_count() : 0u;
      uint32_t      param_count     = param_list.get_length();
      uint32_t      ret_param_count = return_param_list.get_length();

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (flags & SkInvokeInfo_captured)
        {
        if (captured_count)
          {
          str_p->append("[Captured] ---------------------------+----------------------+-------+-------------------------+----------------\n", 113u);

          AString scope_str("<Captured>", 10u);
          SkClosureInfoBase * info_p = closure_p->get_info();
          for (uint32_t i = 0; i < captured_count; ++i)
            {
            Nested::append_str_variable(str_p, info_p->get_captured()[i].get_name(), scope_str, data_pp[i]);
            }
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (flags & SkInvokeInfo_temporaries)
        {
        // Crawl up call stack and check if we got temp variables
        bool have_temp_vars = false;
        for (SkInvokedBase * call_stack_p = caller_p; call_stack_p && call_stack_p != context_call_p; call_stack_p = call_stack_p->get_caller())
          {
          const SkExpressionBase * expr_p = call_stack_p->get_expr();
          if (expr_p && expr_p->get_type() == SkExprType_code)
            {
            if (!static_cast<const SkCode *>(expr_p)->get_temp_vars().is_empty())
              {
              have_temp_vars = true;
              break;
              }
            }
          }

        if (have_temp_vars && !ms_no_step_default_hack)
          {
          str_p->append("[Temporaries] ------------------------+----------------------+-------+-------------------------+----------------\n", 113u);

          // Crawl up call stack and gather temp variables
          AString scope_str("<Temp Var>", 10u);
          for (SkInvokedBase * call_stack_p = caller_p; call_stack_p && call_stack_p != context_call_p; call_stack_p = call_stack_p->get_caller())
            {
            const SkExpressionBase * expr_p = call_stack_p->get_expr();
            if (expr_p && expr_p->get_type() == SkExprType_code)
              {
              const SkCode * code_p = static_cast<const SkCode *>(expr_p);
              SkInstance ** var_pp = data_pp + code_p->get_temp_vars_start_idx();
              const AVCompactArray<ASymbol> & code_vars = code_p->get_temp_vars();
              const ASymbol * code_var_end_p = code_vars.get_array_end();
              for (const ASymbol * code_var_p = code_vars.get_array(); code_var_p < code_var_end_p; ++code_var_p, ++var_pp)
                {
                Nested::append_str_variable(str_p, *code_var_p, scope_str, *var_pp);
                }
              }
            }
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (flags & SkInvokeInfo_args)
        {
        if (param_count && !ms_no_step_default_hack)
          {
          str_p->append("[Arguments] --------------------------+----------------------+-------+-------------------------+----------------\n", 113u);

          AString  scope_str("<Arg>", 5u);
          SkInstance ** vars_pp = data_pp + captured_count;
          for (uint32_t i = 0; i < param_count; ++i)
            {
            Nested::append_str_variable(str_p, param_list[i]->get_name(), scope_str, vars_pp[i]);
            }
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (ret_param_count && !ms_no_step_default_hack)
          {
          str_p->append("[Return Arguments] -------------------+----------------------+-------+-------------------------+----------------\n", 113u);

          // $Revisit - CReis Add expected class type
          AString  scope_str("<Return Arg>", 12u);
          SkInstance ** vars_pp = data_pp + captured_count + param_count;
          for (uint32_t i = 0; i < ret_param_count; ++i)
            {
            Nested::append_str_variable(str_p, return_param_list[i]->get_name(), scope_str, vars_pp[i]);
            }
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      SkInstance * scope_top_p = context_call_p->get_scope() ? context_call_p->get_scope()->get_topmost_scope() : nullptr;

      if (flags & SkInvokeInfo_this)
        {
        str_p->append("[Receiver] ---------------------------+----------------------+-------+-------------------------+----------------\n", 113u);

        AString scope_str("<this>");

        Nested::append_str_variable(str_p, ASymbol_this, scope_str, scope_top_p);
        }

      if (scope_top_p)
        {
        SkClass * class_p = scope_top_p->get_key_class();

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (flags & SkInvokeInfo_instance_data)
          {
          // Note - Can't just use data in instance table - need scope info from class
          if (class_p->get_total_data_count() + class_p->compute_total_raw_data_count())
            {
            str_p->append("[Instance Data] ----------------------+----------------------+-------+-------------------------+----------------\n", 113u);

            uint32_t var_count;
            uint32_t data_idx = class_p->get_total_data_count();

            // Loop through classes - used to get scope rather than using data table
            for (SkClass * current_class_p = class_p; current_class_p; current_class_p = current_class_p->get_superclass())
              {
              // 1) Sk instance data
              var_count = current_class_p->get_instance_data().get_length();
              data_idx -= var_count;
              for (SkTypedName * var_p : current_class_p->get_instance_data())
                {
                Nested::append_str_variable(
                  str_p,
                  var_p->get_name(),
                  current_class_p->get_name_cstr_dbg(),
                  static_cast<SkDataInstance *>(scope_top_p)->get_data_by_idx(data_idx++),
                  var_p->m_type_p->get_key_class());
                }
              data_idx -= var_count;

              // 2) raw instance data
              for (SkTypedNameRaw * var_p : current_class_p->get_instance_data_raw())
                {
                SkClassDescBase * data_type_p = var_p->m_type_p; // Type of the data member itself
                // Ask the owner class and data class to fetch the raw data member for us
                SkInstance * data_p = SkBrain::ms_nil_p;
                void * obj_p = class_p->get_raw_pointer(scope_top_p); // Pointer to raw memory of the object containing the data member
                if (obj_p)
                  {
                  data_p = data_type_p->get_key_class()->new_instance_from_raw_data(obj_p, var_p->m_raw_data_info, data_type_p);
                  }
                Nested::append_str_variable(
                  str_p,
                  var_p->get_name(),
                  current_class_p->get_name_cstr_dbg(),
                  data_p,
                  data_type_p->get_key_class());
                data_p->dereference();
                }
              }
            }
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (flags & SkInvokeInfo_class_data)
          {
          // Note - Can't just use data in class table - need scope info from class
          if (class_p->get_total_class_data_count())
            {
            str_p->append("[Class Data] -------------------------+----------------------+-------+-------------------------+----------------\n", 113u);

            // Loop through classes - used to get scope rather than using data table
            for (SkClass * current_class_p = class_p; current_class_p; current_class_p = current_class_p->get_superclass())
              {
              uint32_t var_count = current_class_p->get_class_data().get_length();
              if (var_count)
                {
                // $Revisit - CReis Alphabetically sort by name
                SkInstance ** value_pp = current_class_p->get_class_data_values().get_array();
                SkTypedName ** var_pp = current_class_p->get_class_data().get_array();
                for (uint32_t i = 0; i < var_count; ++i)
                  {
                  SkTypedName * var_p = var_pp[i];
                  Nested::append_str_variable(
                    str_p,
                    var_p->get_name(),
                    current_class_p->get_name_cstr_dbg(),
                    value_pp[i],
                    var_p->m_type_p->get_key_class());
                  }
                }
              }
            }
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Optional indent
      if (flags & SkInvokeInfo_indent)
        {
        str_p->line_indent(ms_indent_size, inital_length);
        }

      str_p->append('\n');

	  // Put the current call back.
      ms_current_call_p = current_context_p;
      }
    else
  #endif
      {
      // No debug info available
      str_p->append("\nSkookum Local Variables - not available!\n", 42u);
     }
  }

//---------------------------------------------------------------------------------------
// Prints out the script callstack for the supplied invoked object or if nullptr
//             for the currently running invoked object if there is one.
// Arg         invoked_p - invoked object to get the callstack for.  If nullptr the currently
//             running invoked object is used if there is one.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::print_callstack(
  SkInvokedBase * invoked_p,  // = nullptr
  uint32_t        stack_flags // = SkInvokeInfo__callstack_def
  )
  {
  AString stack_str;

  append_callstack_string(&stack_str, invoked_p, stack_flags);
  ADebug::print(stack_str);
  }

//---------------------------------------------------------------------------------------
// Prints out the script debug context.
// Notes:      This can be called in the C++ Debugging "Immediate" window to print out the
//             current Skookum debug context.
// Modifiers:   static.
// Author(s):   Conan Reis
void SkDebug::print_info()
  {
  print_callstack();
  }

//---------------------------------------------------------------------------------------
// Gets string giving context information
// Arg         description - Description of error, warning, or status
// Arg         call_scope_p - pointer to calling scope context of method or coroutine (usually
//             caller_p).
// Arg         alt_scope_p - pointer to alternate scope context of method or coroutine
//             (usually scope_p).
// Arg         stack_flags - see eSkInvokeInfo
// Author(s):   Conan Reis
AString SkDebug::get_context_string(
  const AString & description,
  SkObjectBase *  call_scope_p,
  SkObjectBase *  alt_scope_p, // = nullptr
  uint32_t        stack_flags  // = SkInvokeInfo__callstack_def
  )
  {
  SkObjectBase * use_scope_p = call_scope_p
    ? call_scope_p
    : alt_scope_p;

  SkInvokedContextBase * context_p = use_scope_p ? use_scope_p->get_scope_context() : nullptr;

  if (context_p)
    {
    AString error_str(description, 256u);

    error_str.append("\n\n", 2u);
    append_callstack_string(&error_str, context_p, stack_flags);

    return error_str;
    }

  return AString::ms_empty;
  }

//---------------------------------------------------------------------------------------
// Prints string to IDE (local and/or remote as specified) and drops the
//             print if no IDE is available.
// Returns:    true if able to print to IDE and false if not
// Arg         str - string to append
// Arg         local - 
// Arg         type - see eSkDPrintType
// See:        set_log_func(), ADebug::register_print_func() 
// Modifiers:   static
// Author(s):   Conan Reis
bool SkDebug::print_ide(
  const AString & str,
  eSkLocale       locale, // = SkLocale_all
  uint32_t        type    // = SkDPrintType_system
  )
  {
  bool printed = false;

  locale = SkRemoteBase::locale_as_virtual(locale);

  if ((locale != SkLocale_remote) && ms_log_func_p)
    {
    ms_log_func_p->invoke(SkPrintInfo(str, type));
    printed = true;
    }

  #ifdef SKOOKUM_REMOTE
    if ((locale & SkLocale_remote)
      && SkRemoteBase::ms_default_p
      && SkRemoteBase::ms_default_p->is_remote_runtime()
      && SkRemoteBase::ms_default_p->is_connected())
      {
      // Send print to remote side
      SkRemoteBase::ms_default_p->cmd_print(str, type);
      printed = true;
      }
  #endif  // SKOOKUM_REMOTE

  return printed;
  }

//---------------------------------------------------------------------------------------
// Appends string to log (or debug output if log is not registered) and
//             optionally to remote IDE.
// Arg         str - string to append
// Arg         local - 
// Arg         type - see eSkDPrintType
// See:        set_log_func(), ADebug::register_print_func() 
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::print(
  const AString & str,
  eSkLocale       locale, // = SkLocale_all
  uint32_t        type    // = SkDPrintType_system
  )
  {
  if (!print_ide(str, locale, type))
    {
    // Ensure that the print went somewhere - use regular debug print as a backup.
    // call_print_funcs is set to false to ensure that this method is not
    // recursively called again.
    ADebug::print(str, false);
    }
  }

//---------------------------------------------------------------------------------------
// Same as calling ADebug::print() though it allows a Skookum print type to
//             be specified.  Appends string to log, debug output and optionally to
//             remote IDE.
// Arg         str - string to append
// Arg         local - 
// Arg         type - see eSkDPrintType
// See:        set_log_func(), ADebug::register_print_func() 
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::print_agog(
  const AString & str,
  eSkLocale       locale, // = SkLocale_all
  uint32_t        type    // = SkDPrintType_system
  )
  {
  print_ide(str, locale, type);
  // call_print_funcs is set to false to ensure that this method is not recursively
  // called again.
  ADebug::print(str, false);
  }

//---------------------------------------------------------------------------------------
// Append code error message to the console window.
// Arg         err_msg - error string
// Author(s):   Conan Reis
void SkDebug::print_error(
  const AString & err_msg,
  eAErrLevel      level // = AErrLevel_error
  )
  {
  AString error_str;
  error_str.ensure_size_empty(SkDebug_log_str_length);
  uint32_t print_type = SkDPrintType_system;

  if (level >= AErrLevel_error)
    {
    error_str.append("\nERROR (Skookum): ", 18u);
    print_type = SkDPrintType_error;
    }
  else
    {
    error_str.append("\nWARNING (Skookum): ", 20u);
    print_type = SkDPrintType_warning;
    }

  error_str.append(err_msg);
  error_str.append("\n\n", 2u);

  print(error_str, SkLocale_all, print_type);
  }

//---------------------------------------------------------------------------------------
// Prints out specified message with script member context to the console window.
//
// #Author(s) Conan Reis
void SkDebug::print_script_context(
  const AString & msg,
  const SkInvokableBase * invokable_p, // = nullptr
  SkExpressionBase * expr_p, // = nullptr
  uint32_t type // = SkDPrintType_system
  )
  {
  AString msg_str;
  msg_str.ensure_size_empty(SkDebug_log_str_length);

  msg_str.append(msg);

  if (invokable_p)
    {
    msg_str.append("\nMember: ", 9u);
    msg_str.append(invokable_p->as_string_name().as_cstr());

    if (invokable_p->is_class_member())
      {
      msg_str.append('C');
      }

    #if (SKOOKUM & SK_DEBUG)
      if (expr_p && expr_p->is_valid_origin_source())
        {
        msg_str.append_format("[%u]", expr_p->m_source_idx);
        }
    #endif
    }

  msg_str.append("\n\n", 2u);

  print(msg_str, SkLocale_all, type);
  }


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// Append code error message to the console window.
// 
// Params:  
//   result:       the result type of the earlier parse
//   path:         file path of parsed script
//   code_p:       address to string containing text that was parsed
//   result_pos:   index that the parse error occurred
//   result_start: start index of most recent parse element
//   start_pos:
//     index that parse initially started for code (if only a substring of code is parsed.
//     
// Author(s):   Conan Reis
void SkDebug::print_parse_error(
  SkParser::eResult result,
  const AString &   path,         // = AString::ms_empty
  const AString *   code_p,       // = nullptr
  uint32_t          result_pos,   // = 0u
  uint32_t          result_start, // = ADef_uint32
  uint32_t          start_pos     // = 0u
  )
  {
  AString error_str;
  error_str.ensure_size_empty(SkDebug_log_str_length);

  error_str.append("\n\n");

  if (code_p)
    {
    error_str.append(SkParser::get_result_context_string(*code_p, result, result_pos, result_start, start_pos));
    error_str.append('\n');
    }
  else
    {
    error_str.append("\nERROR: ");
    error_str.append(SkParser::get_result_string(result));
    error_str.append("\n\n", 2u);
    }

  // Add origin info
  if (!path.is_empty())
    {
    error_str.append("File: ", 6u);
    error_str.append(path);
    }
  else
    {
    if (code_p)
      {
      error_str.append("Script code index: ", 19u);
      }
    }

  // Add character index
  if (code_p)
    {
    // Specific character index?
    if (path.is_empty() || (result_pos != 0u))
      {
      // Note that these indexes will likely be from files in DOS format (/r/n).

      if ((result_start == ADef_uint32) || (result_pos == result_start))
        {
        // Tack on character index
        error_str.append_format("[%u]\n", result_pos);
        }
      else
        {
        // Tack on character index range
        error_str.append_format("[%u-%u]\n", result_start, result_pos);
        }
      }
    }

  print(error_str, SkLocale_all, (result >= SkParser::Result_err__start) ? SkDPrintType_error : SkDPrintType_warning);
  }

#endif // (SKOOKUM & SK_CODE_IN)


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Add Skookum context string (in the form of a callstack) if in the middle
//             of a Skookum call (method/coroutine)
// Arg         str_p - string to append context to
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::context_append(AString * str_p)
  {
  append_callstack_string(str_p, (SkInvokedBase *)nullptr, SkInvokeInfo_ignore_absent | SkInvokeInfo__callstack_def);
  }

#endif


//---------------------------------------------------------------------------------------
// Set the log function to call with strings to append to log / debug output.
// Arg         log_func_p - function object with (const & AString) as an argument.
// See:        print(), register_print_with_agog()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::set_print_func(tSkPrintFunc * log_func_p)
  {
  ms_log_func_p = log_func_p;
  }

//---------------------------------------------------------------------------------------
// Registers SkDebug::print_ide_all() with ADebug::print*()
// See:        set_print_func()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::register_print_with_agog()
  {
  ADebug::register_print_func(new AFunctionArg<const AString &>(&SkDebug::print_ide_all));
  }

//---------------------------------------------------------------------------------------
// Print Skookum memory usage to output.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::print_memory(
  eSkCodeSerialize type // = SkCodeSerialize_static_demand
  )
  {
  uint32_t total_memory = print_memory_runtime() + print_memory_code(type);

  ADebug::print_format(
    "\n  *** Total %s code + runtime bytes: %u ***\n",
    (type == SkCodeSerialize_static) ? "static" : "static (+ demand loaded)",
    total_memory);

  if (ms_print_mem_ext_p)
    {
    ADebug::print("\n\n");
    ms_print_mem_ext_p->invoke();
    }
  }

//---------------------------------------------------------------------------------------
// Print Skookum runtime memory pool usage to output.
// Returns:    total runtime bytes used
// Modifiers:   static
// Author(s):   Conan Reis
uint32_t SkDebug::print_memory_runtime()
  {
  ADebug::print(
    "\n\n"
    "================================================================================\n"
    "Skookum Runtime Memory Pool Usage\n"
    "================================================================================\n\n");

  AObjReusePool<SkInstance> &          instance_pool   = SkInstance::get_pool();
  AObjReusePool<SkInvokedExpression> & iexpr_pool      = SkInvokedExpression::get_pool();
  AObjReusePool<SkInvokedCoroutine> &  icoroutine_pool = SkInvokedCoroutine::get_pool();
  AObjReusePool<AStringRef> &          str_ref_pool    = AStringRef::get_pool();

  uint32_t instance_bytes   = instance_pool.get_bytes_allocated();
  uint32_t iexpr_bytes      = iexpr_pool.get_bytes_allocated();
  uint32_t icoroutine_bytes = icoroutine_pool.get_bytes_allocated();
  uint32_t str_ref_bytes    = str_ref_pool.get_bytes_allocated();
  uint32_t runtime_bytes    = instance_bytes + iexpr_bytes + icoroutine_bytes + str_ref_bytes;

  // 20    8         8        9          8
  // Pool  Max Used  Current  Available  Initial  Overflow!
  ADebug::print(
    " Pool                 | Max Used |  Current | Available |  Initial | Overflow! |    Bytes \n"
    "----------------------+----------+----------+-----------+----------+-----------+----------\n");

  ADebug::print_format(
    " SkInstance           | %8u | %8u | %9u | %8u | %9u | %8u\n", instance_pool.get_count_max(), instance_pool.get_count_used(), instance_pool.get_count_available(), instance_pool.get_count_initial(), instance_pool.get_count_overflow(), instance_bytes);
  ADebug::print_format(
    " SkInvokedExpression  | %8u | %8u | %9u | %8u | %9u | %8u\n", iexpr_pool.get_count_max(), iexpr_pool.get_count_used(), iexpr_pool.get_count_available(), iexpr_pool.get_count_initial(), iexpr_pool.get_count_overflow(), iexpr_bytes);
  ADebug::print_format(
    " SkInvokedCoroutine   | %8u | %8u | %9u | %8u | %9u | %8u\n", icoroutine_pool.get_count_max(), icoroutine_pool.get_count_used(), icoroutine_pool.get_count_available(), icoroutine_pool.get_count_initial(), icoroutine_pool.get_count_overflow(), icoroutine_bytes);
  // SkDataInstance
  ADebug::print_format(
    " AStringRef [shared]  | %8u | %8u | %9u | %8u | %9u | %8u\n", str_ref_pool.get_count_max(), str_ref_pool.get_count_used(), str_ref_pool.get_count_available(), str_ref_pool.get_count_initial(), str_ref_pool.get_count_overflow(), str_ref_bytes);

  #if !defined(AORPOOL_USAGE_COUNT)
    ADebug::print(
      "\n  [Reuse Pool tracking is disabled, so 'Max Used' and 'Overflow!' are approximate values.]\n");
  #endif

  ADebug::print(
    "\n  [Dynamic memory and runtime memory created via scripts is not tracked.]\n");

  ADebug::print_format(
    "\n  Total runtime bytes: %u\n", runtime_bytes);

  return runtime_bytes;
  }

//---------------------------------------------------------------------------------------
// #Description
//   Print Skookum code memory usage to output.
//
// #Modifiers static
// #See Also  print_memory_runtime(), print_memory()
// #Author(s) Conan Reis
uint32_t SkDebug::print_memory_code(
  // Type of code to track memory for: static or static + demand loaded - see eSkCodeSerialize
  eSkCodeSerialize type, // = SkCodeSerialize_static_demand
  // Class to track.  If nullptr then Object is used and all code structures are tracked.
  SkClass * from_class_p, // = nullptr
  // Track subclasses or just specified class - see eAHierarchy
  eAHierarchy iterate // = AHierarchy__all
  )
  {
  bool track_global       = false;
  bool skip_demand_loaded = (type == SkCodeSerialize_static);

  if (from_class_p == nullptr)
    {
    from_class_p = SkBrain::ms_object_class_p;
    track_global = (iterate == AHierarchy__all);
    }

  ADebug::print(
    "\n\n"
    "================================================================================\n");
  ADebug::print(skip_demand_loaded
    ? "Skookum Static Code Memory Usage\n"
    : "Skookum Static + Demand Loaded Code Memory Usage\n");
  ADebug::print(
    "================================================================================\n\n");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Accumulate Skookum code memory
  AMemoryStats mem_stats;

  // Iterate through class / hierarchy
  if (iterate & AHierarchy__all)
    {
    from_class_p->track_memory_recursive(&mem_stats, skip_demand_loaded);
    }
  else
    {
    from_class_p->track_memory(&mem_stats, skip_demand_loaded);
    }

  if (track_global)
    {
    // Add any unused SkBrain class list buffer as dynamic memory under SkClass
    mem_stats.track_memory_shared(
      "SkClass", 0u, 0u, sizeof(void *) * (SkBrain::get_classes().get_size() - SkBrain::get_classes().get_length()));

    // Iterate through shared parameter interfaces.
    SkParameters::shared_track_memory(&mem_stats);

    // Iterate through class unions
    SkClassUnion::shared_track_memory(&mem_stats);

    // Iterate through typed classes
    SkTypedClass::shared_track_memory(&mem_stats);

    // Iterate through invokable classes
    SkInvokableClass::shared_track_memory(&mem_stats);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Print Summary
  uint32_t debug_code_bytes;
  uint32_t total_code_bytes = mem_stats.print_summary(&debug_code_bytes);

  if (!track_global)
    {
    ADebug::print("\n  * Shared structures are not included above\n    - they are only displayed for global memory stats.\n");
    }

  ADebug::print_format(
    "\n"
    "  Total actual code bytes: %9u\n"
    "    + Extra bytes when debugging: %9u  [Only approximate when using fixed size pools.]\n",
    total_code_bytes,
    debug_code_bytes);

  return total_code_bytes;
  }


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Gets the current invokable that is used for and sacrosanct to step-wise
//             debugging - ms_current_call_p can change or be lost.
// Modifiers:   static
// Author(s):   Conan Reis
SkInvokedBase * SkDebug::get_next_invokable()
  {
  return ms_next_invokable_p;
  }

//---------------------------------------------------------------------------------------
// Stop/suspend execution at specified expression context.
//
// #Notes
//   Usually SkRemoteRuntimeBase::ms_client_p->on_breakpoint_hit() or on_break_expression() is
//   called immediately following to notify the IDE.
//   Sets ms_next_expr so it can be accessed after this call.
//
// #Modifiers static
// #See Also
//   break_invokable(), SkRemoteRuntimeBase::on_breakpoint_hit(),
//   SkRemoteRuntimeBase::on_break_expression()
//   
// #Author(s) Conan Reis
void SkDebug::break_expression(
  // Current context scope
  SkObjectBase * scope_p,
  // Current caller
  SkInvokedBase * caller_p,
  // Current expression
  SkExpressionBase * expr_p
  )
  {
  // Cancel any active stepping
  set_flag(Flag_stepping, false);
  ms_step_type = Step__none;

  // Note context and suspend execution - ms_current_call_p is already set
  ms_exec_state = State_suspended_expr;
  ms_next_invokable_p = caller_p;
  set_next_expression(scope_p, caller_p, expr_p);
  }

//---------------------------------------------------------------------------------------
// Stop/suspend execution at specified invoked context (method, coroutine, etc.).
//
// #Notes
//   Usually SkRemoteRuntimeBase::ms_client_p->on_breakpoint_hit() or on_break_expression() is
//   called immediately following to notify the IDE.
//   Sets ms_next_expr so it can be accessed after this call.
//
// #Modifiers static
// #See Also
//   break_invokable(), SkRemoteRuntimeBase::on_breakpoint_hit(),
//   SkRemoteRuntimeBase::on_break_expression()
//   
// #Author(s) Conan Reis
void SkDebug::break_invokable(SkInvokedBase * invoked_p)
  {
  SkInvokedBase * caller_p = invoked_p->get_caller();

  break_expression(caller_p, caller_p, invoked_p->get_caller_expr());

  SkInvokedContextBase * icontext_p = caller_p->get_scope_context();

  AString str("\n\n", 2u);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optionally print out callstack
  if (ms_pref_flags & PrefFlag_break_print_callstack)
    {
    // $Revisit - CReis Need to send expr_p for additional debug info since not all
    // expressions get an SkInvokedExpression wrapper.
    append_callstack_string(&str, icontext_p);
    str.append("\n\n", 2u);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optionally print out local variables
  if (ms_pref_flags & PrefFlag_break_print_locals)
    {
    append_locals_string(&str, caller_p, icontext_p);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (ms_pref_flags & PrefFlag__break_print_mask)
    {
    print(str, SkLocale_all, SkDPrintType_trace);
    }
  }

//---------------------------------------------------------------------------------------
// Enable/disable debug preference
// Arg         reference - see ePrefFlag
// Arg         enable - indicates whether specified preference should be enabled (true)
//             or disabled (false)
// See:        is_preference()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::enable_preference(
  ePrefFlag preference,
  bool      enable // = true
  )
  {
  if (enable)
    {
    ms_pref_flags |= preference;
    }
  else
    {
    ms_pref_flags &= ~preference;
    }
  }

//---------------------------------------------------------------------------------------
// Invalidates next member expression info and registers the fact that the
//             runtime execution state is running.
// See:        set_next_expression()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::invalidate_next_expression()
  {
  ms_exec_state = State_running;
  ms_next_expr.invalidate();
  }

//---------------------------------------------------------------------------------------
// Sets next member expression info and registers the fact that the runtime
//             execution state has entered expression debugging mode.
// See:        set_next_expression(scope_p, caller_p, expr_p), invalidate_next_expression()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::set_next_expression(const SkMemberExpression & expr_info)
  {
  ms_next_expr  = expr_info;
  ms_exec_state = State_suspended_expr;
  }

//---------------------------------------------------------------------------------------
// Sets next member expression info and registers the fact that the runtime
//             execution state has entered expression debugging mode.
// See:        set_next_expression(expr_info), invalidate_next_expression()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::set_next_expression(
  SkObjectBase * scope_p,
  SkInvokedBase * caller_p,
  SkExpressionBase * expr_p
  )
  {
  ms_next_expr.set_context(scope_p, caller_p, expr_p);
  }

//---------------------------------------------------------------------------------------
// Sets type of stepwise debugging to start
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::step(eStep step_type)
  {
  SkInvokedBase *        next_invoked_p  = ms_next_invokable_p;
  SkInvokedContextBase * next_icontext_p = next_invoked_p ? next_invoked_p->get_caller_context() : nullptr;

  SK_MAD_ASSERTX(ms_step_icontext_p.is_valid(), "ms_step_icontext_p must be set at this point.");

  ms_step_topmost_caller_p = ms_step_icontext_p->get_topmost_caller();
  
  switch (step_type)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Step_into:
      // Stop on the next expression that has the same invoked context (method/coroutine)
      // unless it is stale in which case ignore it.
      ms_step_icontext_p = next_icontext_p;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Step_over:
      // Keep same context
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case Step_out:
      {
      // Caller of the step_icontext is the next best guess
      SkInvokedBase * caller_p = ms_step_icontext_p->get_caller();
      ms_step_icontext_p = caller_p ? caller_p->get_caller_context() : nullptr;
      }
      break;
        
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Step_next does not need to store any context.
    default:
      break;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  set_flag(Flag_stepping, step_type != Step__none);
  ms_next_expr.invalidate();
  ms_step_type  = step_type;
  ms_exec_state = State_running;
  }

//---------------------------------------------------------------------------------------
// Add breakpoint with known values - generally from remote IDE
// Returns:    new breakpoint
// Arg         bp_info - member identifying info
// Arg         table_idx - index in debug table
// Arg         enabled - indicates whether the breakpoint is enabled or not
// Author(s):   Conan Reis
SkBreakPoint * SkDebug::breakpoint_append(
  const SkMemberExpression & bp_info,
  uint32_t table_idx,
  bool enabled // = true
  )
  {
  // If breakpoint in a demand loaded class - ensure loaded and lock in memory
  bp_info.get_class()->ensure_loaded_debug();

  SkBreakPoint * bp_p = SK_NEW(SkBreakPoint)(bp_info, table_idx, enabled);

  ms_breakpoints.append(*bp_p);
  ms_breakpoint_table.ensure_length_null(table_idx + 1u);
  ms_breakpoint_table.get_array()[table_idx] = bp_p;

  bp_p->reaquire_expr();

  return bp_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkBreakPoint * SkDebug::breakpoint_append_absent(
  const SkMemberInfo & member_info,
  SkExpressionBase *   break_expr_p,
  bool *               appended_p // = nullptr
  )
  {
  if (appended_p)
    {
    *appended_p = false;
    }

  if (break_expr_p == nullptr)
    {
    return nullptr;
    }

  SkBreakPoint * bp_p = breakpoint_get_by_expr(*break_expr_p);

  if (bp_p == nullptr)
    {

    if (appended_p)
      {
      *appended_p = true;
      }

    bp_p = SK_NEW(SkBreakPoint)(member_info, break_expr_p);
    ms_breakpoints.append(*bp_p);
    bp_p->m_table_idx = ms_breakpoint_table.append_at_null(*bp_p);
    bp_p->reaquire_expr();
    }

  return bp_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkBreakPoint * SkDebug::breakpoint_append_absent(
  const SkMemberInfo & member_info,
  uint32_t             source_idx,
  bool *               appended_p // = nullptr
  )
  {
  return breakpoint_append_absent(
    member_info,
    member_info.find_expr_on_pos(source_idx, SkExprFind_all),
    appended_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkDebug::breakpoint_remove(const SkBreakPoint & bp)
  {
  SkBreakPoint * bp_p = ms_breakpoints.pop(bp);

  if (bp_p)
    {
    ms_breakpoint_table.null(bp_p->m_table_idx);
    bp_p->release_expr();
    delete bp_p;

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkDebug::breakpoint_remove_by_expr(const SkExpressionBase & expr)
  {
  uint32_t idx = expr.m_debug_info & SkDebugInfo::Flag_debug_idx__mask;

  if (idx != SkDebugInfo::Flag_debug_idx__none)
    {
    SkBreakPoint ** bps_pp = ms_breakpoint_table.get_array();
    SkBreakPoint *  bp_p   = bps_pp[idx];

    bps_pp[idx] = nullptr;
    ms_breakpoints.remove(*bp_p);
    bp_p->release_expr();
    delete bp_p;

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Enables all existing breakpoints
// Author(s):   Conan Reis
void SkDebug::breakpoint_enable_all()
  {
  ms_breakpoints.apply_method(&SkBreakPoint::enable_set);
  }

//---------------------------------------------------------------------------------------
// Disables (though keeps in place) all existing breakpoints
// Author(s):   Conan Reis
void SkDebug::breakpoint_disable_all()
  {
  ms_breakpoints.apply_method(&SkBreakPoint::enable_clear);
  }

//---------------------------------------------------------------------------------------
// Removes binding to associated expression for all existing breakpoints.
// Used before expression structures are recreated during compile, etc.
// 
// See:        breakpoint_acquire_all()
// Author(s):  Conan Reis
void SkDebug::breakpoint_release_all()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - released %u\n", ms_breakpoints.get_length());

  ms_breakpoints.apply_method(&SkBreakPoint::release_expr);
  }

//---------------------------------------------------------------------------------------
// Re-binds associated expression for all existing breakpoints.
// Used when after expression structures are recreated during compile, etc.
// 
// See:        breakpoint_release_all()
// Author(s):  Conan Reis
void SkDebug::breakpoint_acquire_all()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - acquired %u\n", ms_breakpoints.get_length());

  ms_breakpoints.apply_method(&SkBreakPoint::acquire_expr);
  }

//---------------------------------------------------------------------------------------
// Removes/frees any existing breakpoints
// Author(s):   Conan Reis
void SkDebug::breakpoint_remove_all()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " - removed %u\n", ms_breakpoints.get_length());

  ms_breakpoint_table.remove_all();
  ms_breakpoints.apply_method(&SkBreakPoint::release_expr);
  ms_breakpoints.free_all();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkDebug::breakpoint_is_on_class(const SkClass & ssclass)
  {
  // $Vital - CReis Test
  SkMemberInfo member_info;

  member_info.m_member_id.set_scope(&ssclass);

  SkMemberExpression fake_member_expr(member_info);

  return ms_breakpoints.find(fake_member_expr);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkDebug::breakpoint_is_on_member(const SkMemberInfo & member_info)
  {
  SkMemberExpression fake_member_expr(member_info);

  return ms_breakpoints.find(fake_member_expr);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
bool SkDebug::breakpoint_is_on_expr(const SkExpressionBase & expr)
  {
  return ((expr.m_debug_info & SkDebugInfo::Flag_debug_idx__mask) != SkDebugInfo::Flag_debug_idx__none);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkDebug::breakpoint_get_all_by_member(
  tSkBreakPoints *     bps_p,
  const SkMemberInfo & member_info
  )
  {
  SkMemberExpression fake_member_expr(member_info);

  ms_breakpoints.get_all(bps_p, fake_member_expr);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkBreakPoint * SkDebug::breakpoint_get_by_expr(const SkExpressionBase & expr)
  {
  uint32_t idx = expr.m_debug_info & SkDebugInfo::Flag_debug_idx__mask;

  return (idx != SkDebugInfo::Flag_debug_idx__none)
    ? ms_breakpoint_table.get_array()[idx]
    : nullptr;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkBreakPoint * SkDebug::breakpoint_get_at_idx(uint32_t table_idx)
  {
  return (table_idx < SkDebugInfo::Flag_debug_idx__none && table_idx < ms_breakpoint_table.get_length())
    ? ms_breakpoint_table.get_at(table_idx)
    : nullptr;
  }

#endif  // (SKOOKUM & SK_DEBUG)


#if defined(SKDEBUG_COMMON)

//---------------------------------------------------------------------------------------
// Called by runtime prior to an expression is about to be invoked that has
//             an enabled breakpoint on it.
// See:        SKDEBUG_HOOK_EXPR() and other hook functions and macros.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_expression(
  SkExpressionBase *      expr_p,
  SkObjectBase *          scope_p,
  SkInvokedBase *         caller_p,
  SkInvokedBase *         branch_caller_p,
  eHookContext            hook_context
  )
  {
  #if (SKOOKUM & SK_DEBUG)

    // If remote runtime client is nullptr then this is a script running on the IDE so ignore
    // debug flags in local scripts.
    // $Revisit - CReis In the future allow debugging of local scripts - for debugging IDE macros etc.
    if (SkRemoteRuntimeBase::ms_client_p == nullptr)
      {
      return;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Test for breakpoint set on expression - note that a breakpoint trumps stepwise
    // debugging.
    if (expr_p->m_debug_info & SkDebugInfo::Flag_debug_enabled)
      {
      break_expression(scope_p, caller_p, expr_p);

      // Build a callstack
      ARefPtr<SkCallStack> callstack_p = get_callstack(
        caller_p,
        branch_caller_p,
        &get_next_expression(), 
        hook_context == HookContext_current ? SkInvokeInfo__callstack_def : (SkInvokeInfo__callstack_def | SkInvokeInfo_peek));

      // Append break expression to lowest callstack level
      #ifdef A_MAD_CHECK
        callstack_p->m_stack[callstack_p->m_current_level_idx]->m_label.append(a_str_format(" (%d @ 0x%p)", (int32_t)expr_p->get_type(), (void*)expr_p));
      #endif

      // Set context for next step
      ms_step_icontext_p = branch_caller_p 
        ? branch_caller_p->get_caller_context() 
        : ((hook_context == HookContext_current ? caller_p : caller_p->get_caller())->get_caller_context());

      // Get breakpoint info
      SkBreakPoint * bp_p = breakpoint_get_by_expr(*expr_p);
      if (bp_p)
        {
        // Have runtime communicate to IDE that it has hit a breakpoint
        SkRemoteRuntimeBase::ms_client_p->on_breakpoint_hit(*bp_p, callstack_p, scope_p, caller_p);
        }
      else
        {
        // Oops this should never happen - but when it does, just silently ignore it 
        // and strip the breakpoint info from the expression
        SkBreakPoint::release_expr(expr_p);
        }
      }
    else
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Test for step-wise debugging
      if (ms_flags & Flag_stepping)
        {
        bool hit_step_break = true;

        // Test if step conditions met
        if (ms_step_type != Step_next)
          {
          // Context from previous step
          SkInvokedContextBase * step_icontext_p = ms_step_icontext_p;
          if (step_icontext_p)
            {
            // Check if scope matches desired icontext, or if the current caller is a caller of the icontext (i.e. we stepped out)
            hit_step_break = (step_icontext_p == scope_p
              || step_icontext_p->is_caller(branch_caller_p ? branch_caller_p : (hook_context == HookContext_current ? caller_p : caller_p->get_caller())));
            }
          else
            {
            // Desired scope is gone, check if we are within the same call tree
            SkInvokedBase * top_most_caller_p = ms_step_topmost_caller_p;
            if (top_most_caller_p)
              {
              // Check if top caller matches up
              hit_step_break = (top_most_caller_p == (branch_caller_p ? branch_caller_p : caller_p)->get_topmost_caller());
              }
            else
              {
              // Otherwise, stop by default
              hit_step_break = true;
              }
            }
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Was a step point hit?
        if (hit_step_break)
          {
          // Break at expression
          break_expression(scope_p, caller_p, expr_p);

          // Build a callstack
          ARefPtr<SkCallStack> callstack_p = get_callstack(
            caller_p,
            branch_caller_p,
            &get_next_expression(),
            hook_context == HookContext_current ? SkInvokeInfo__callstack_def : (SkInvokeInfo__callstack_def | SkInvokeInfo_peek));

          // Append break expression to lowest callstack level
          #ifdef A_MAD_CHECK
            callstack_p->m_stack[callstack_p->m_current_level_idx]->m_label.append(a_str_format(" (%d @ 0x%p)", (int32_t)expr_p->get_type(), (void*)expr_p));
          #endif

          // Set context for next step
          ms_step_icontext_p = branch_caller_p
            ? branch_caller_p->get_caller_context()
            : ((hook_context == HookContext_current ? caller_p : caller_p->get_caller())->get_caller_context());

          // Have runtime communicate to IDE that it is at a break
          SkRemoteRuntimeBase::ms_client_p->on_break_expression(ms_next_expr, callstack_p);
          }
        }
      }

  #endif

  #ifdef SKDEBUG_HOOKS

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Generic expression hook run after debugger tests
    if (ms_hook_expr_f)
      {
      (*ms_hook_expr_f)(expr_p, scope_p, caller_p);
      }

  #endif
  }

#endif  // defined(SKDEBUG_COMMON)


#if defined(SKDEBUG_HOOKS)

//---------------------------------------------------------------------------------------
// Adds the specified Skookum execution hook to the list of available hooks.
//             If a hook with the same name already exists its settings are replaced.
// Arg         hook - hook to append (its contents are copied)
// Arg         auto_enable - if true it will be enabled immediately
// See:        remove_hook(), enable_hook()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::append_hook(
  const Hook & hook,
  bool         auto_enable // = true
  )
  {
  Hook * hook_p = ms_hooks.get(hook.get_name());

  if (hook_p == nullptr)
    {
    hook_p = SK_NEW(SkDebug::Hook)(hook);
    ms_hooks.append(*hook_p);
    }
  else
    {
    // Disable the existing hook first
    enable_hook(hook_p, false);

    // Copy the new settings over
    *hook_p = hook;
    }

  SK_ASSERTX((hook_p->m_hook_script_entry_f && hook_p->m_hook_script_exit_f)
    || ((hook_p->m_hook_script_entry_f == nullptr) && (hook_p->m_hook_script_exit_f == nullptr)),
    "SkookumScript Debug Hook - either m_hook_script_entry_f & m_hook_script_exit_f\n"
    "must both be set or they both must be nullptr!");

  if (auto_enable)
    {
    enable_hook(hook_p, true);
    }
  }

//---------------------------------------------------------------------------------------
// Copies specified hook settings to a new hook with a new name that is not
//             initially enabled.
// Returns:    reference to new hook
// Arg         hook_name - name of hook to enable or disable
// Arg         new_name - name of new hook or existing hook to copy settings to
// See:        append_hook(), remove_hook()
// Modifiers:   static
// Author(s):   Conan Reis
const SkDebug::Hook * SkDebug::copy_hook(
  const ASymbol & hook_name,
  const AString & new_name
  )
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return nullptr;
    }

  Hook new_hook(*hook_p);

  new_hook.m_hook_name = new_name;
  new_hook.m_name      = ASymbol::create(new_name);

  append_hook(new_hook, false);

  return ms_hooks.get(new_hook.m_name);
  }

//---------------------------------------------------------------------------------------
// Enables/disables specified registered hook.
// Arg         hook_p - hook to enable or disable
// Arg         enable - true to enable or false to disable
// See:        append_hook(), remove_hook()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::enable_hook(
  Hook * hook_p,
  bool   enable // = true
  )
  {
  if (hook_p->is_enabled() != enable)
    {
    print(a_str_format(
      "\n\nSkookumScript execution hook '%s': %s\n\n",
      hook_p->m_hook_name.as_cstr(),
      enable ? "enabled" : "disabled"));

    if (enable)
      {
      if (hook_p->m_hook_method_f)
        {
        ms_hook_methods.append(hook_p);
        }

      if (hook_p->m_hook_coroutine_f)
        {
        ms_hook_coroutines.append(hook_p);
        }

      if (hook_p->m_hook_script_entry_f)
        {
        ms_hook_origins.append(hook_p);
        }

      hook_p->m_flags |= Hook::Flag_enabled;
      }
    else
      {
      if (hook_p->m_hook_method_f)
        {
        ms_hook_methods.remove(hook_p);
        }

      if (hook_p->m_hook_coroutine_f)
        {
        ms_hook_coroutines.remove(hook_p);
        }

      if (hook_p->m_hook_script_entry_f)
        {
        ms_hook_origins.remove(hook_p);
        }

      hook_p->m_flags &= ~Hook::Flag_enabled;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Enables/disables specified hook.
// Arg         hook_name - name of hook to enable or disable
// Arg         enable - true to enable or false to disable
// See:        append_hook(), remove_hook()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::enable_hook(
  const ASymbol & hook_name,
  bool            enable // = true
  )
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return;
    }

  enable_hook(hook_p, enable);
  }

//---------------------------------------------------------------------------------------
// Set scope/receiver actor name hook condition - only run hooks when
//             scope/receiver is an actor with the specified name.
//             [The enable state of the hook remains unchanged.]
// Arg         hook_name - name of hook to modify condition
// Arg         actor_name - name of actor to trigger on (or ASymbol::ms_null to clear this
//             condition)
// See:        other hook_condition_*() methods
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_condition_scope_actor(
  const ASymbol & hook_name,
  const ASymbol & actor_name
  )
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return;
    }

  hook_p->m_scope_actor_name = actor_name;
  hook_p->update_flags();
  }

//---------------------------------------------------------------------------------------
// Set invoked method/coroutine caller hook condition - only run hooks when
//             this particular invoked method/coroutine is in the call stack - i.e. only
//             run when a call is in the same "thread" as this call.
//             Once the invoked method/coroutine goes out of scope this hook will auto-
//             disable since it will never activate.
//             [The enable state of the hook remains unchanged.]
// Arg         hook_name - name of hook to modify condition
// Arg         caller_p - invoked method or coroutine to look in the call stack for
// See:        other hook_condition_*() methods
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_condition_invoked_caller(
  const ASymbol &        hook_name,
  SkInvokedContextBase * caller_p
  )
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return;
    }

  hook_p->m_invoked_caller_p = caller_p;
  hook_p->update_flags();
  }

//---------------------------------------------------------------------------------------
// Set updater class hook condition - only run hooks when updater (generally
//             the originating script of a command) of a method/coroutine is a mind of
//             the specified class (or subclass if subclass_check is true).
//             [The enable state of the hook remains unchanged.]
// Arg         hook_name - name of hook to modify condition
// Arg         class_p - pointer to class to trigger on when it is the updater (or nullptr to
//             clear this condition)
// Arg         subclass_check - if true the subclasses of class_p are checked as well as
//             class_p and if false only class_p is checked.
// See:        other hook_condition_*() methods
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_condition_updater_class(
  const ASymbol & hook_name,
  SkClass *       class_p,
  bool            subclass_check
  )
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return;
    }

  // Look at any mind - clear hook
  if (class_p == SkBrain::ms_mind_class_p)
    {
    class_p = nullptr;
    }

  hook_p->m_updater_class_p        = class_p;
  hook_p->m_updater_subclass_check = subclass_check;
  hook_p->update_flags();
  }

//---------------------------------------------------------------------------------------
// Removes specified hook.
// Arg         hook_name - name of hook to remove
// See:        append_hook(), enable_hook()
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::remove_hook(const ASymbol & hook_name)
  {
  Hook * hook_p = ms_hooks.pop(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return;
    }

  enable_hook(hook_p, false);
  }

//---------------------------------------------------------------------------------------
// Returns true if specified hook is enabled or false if it is not enabled.
// Returns:    true if enabled false if not
// Arg         hook_name - name of hook to look for
// See:        enable_hook()
// Modifiers:   static
// Author(s):   Conan Reis
bool SkDebug::is_hook_enabled(const ASymbol & hook_name)
  {
  Hook * hook_p = ms_hooks.get(hook_name);

  if (hook_p == nullptr)
    {
    A_ERRORX(a_cstr_format("A Skookum execution hook named '%s' does not exist!", hook_name.as_cstr_dbg()));

    return false;
    }

  return hook_p->is_enabled();
  }

//---------------------------------------------------------------------------------------
// Add 3 example execution hooks for tracing
// 
// Modifiers:  static
// Author(s):  Conan Reis
void SkDebug::hook_examples()
  {
  // Trace coroutines
  Hook trace_hook("TraceCoroutines");
  trace_hook.m_hook_coroutine_f    = hook_trace_coroutine;
  append_hook(trace_hook, false);

  // Trace routines - both coroutines and methods
  // The hook is copied when it is appended so the same structure can be reused.
  trace_hook.set_name("TraceRoutines");
  trace_hook.m_hook_method_f       = hook_trace_method;
  append_hook(trace_hook, false);

  // Trace routines and whenever the system enters or exits the scripting system
  trace_hook.set_name("TraceFull");
  trace_hook.m_hook_script_entry_f = hook_trace_script_entry;
  trace_hook.m_hook_script_exit_f  = hook_trace_script_exit;
  append_hook(trace_hook, false);
  }

//---------------------------------------------------------------------------------------
// Simple hook example - trace method
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_trace_method(SkInvokedMethod * imethod_p)
  {
  AString str(imethod_p->as_invoke_string(SkInvokeInfo__callstack_def | SkInvokeInfo_depth));

  str.append('\n');

  print(str);
  }

//---------------------------------------------------------------------------------------
// Simple hook example - trace coroutine
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_trace_coroutine(SkInvokedCoroutine * icoro_p)
  {
  // Only call hook on the first update
  if (icoro_p->m_update_count == 0u)
    {
    AString str(icoro_p->as_invoke_string(SkInvokeInfo__callstack_def | SkInvokeInfo_depth));

    str.append('\n');

    print(str);
    }
  }

//---------------------------------------------------------------------------------------
// Simple hook example - trace script system entry
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_trace_script_entry(const ASymbol & origin_id)
  {
  AString str("Script system entered at: ", 26u);

  str.append(origin_id.as_cstr_dbg());
  str.append('\n');
  print(str);
  }

//---------------------------------------------------------------------------------------
// Simple hook example - trace script system exit
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_trace_script_exit(const ASymbol & origin_id)
  {
  AString str("Script system exited from: ", 27u);

  str.append(origin_id.as_cstr_dbg());
  str.append('\n');
  print(str);
  }

//---------------------------------------------------------------------------------------
// Called whenever a method is about to be invoked - calls any registered
//             method invocation hooks.
// See:        SKDEBUG_HOOK_METHOD() and other hook functions and macros.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_method(SkInvokedMethod * imethod_p)
  {
  // Call registered Skookum method hooks.
  if (ms_hook_methods.is_filled())
    {
    Hook *       hook_next_p;
    Hook *       hook_p = ms_hook_methods.get_first();
    const Hook * end_p  = ms_hook_methods.get_sentinel();

    do 
      {
      // Get next before calling hook in case hook disables itself
      hook_next_p = ms_hook_methods.get_next(hook_p);

      if (hook_p->is_conditions_met(imethod_p))
        {
        hook_p->m_hook_method_f(imethod_p);
        }

      hook_p = hook_next_p;
      }
    while (hook_p != end_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever a coroutine is about to be invoked - calls any registered
//             coroutine invocation hooks.
// See:        SKDEBUG_HOOK_COROUTINE() and other hook functions and macros.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_coroutine(SkInvokedCoroutine * icoro_p)
  {
  // Call registered Skookum coroutine hooks.
  if (ms_hook_coroutines.is_filled())
    {
    Hook *       hook_next_p;
    Hook *       hook_p = ms_hook_coroutines.get_first();
    const Hook * end_p  = ms_hook_coroutines.get_sentinel();

    do 
      {
      // Get next before calling hook in case hook disables itself
      hook_next_p = ms_hook_coroutines.get_next(hook_p);

      if (hook_p->is_conditions_met(icoro_p))
        {
        hook_p->m_hook_coroutine_f(icoro_p);
        }

      hook_p = hook_next_p;
      }
    while (hook_p != end_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever the Skookum scripting system is about to be entered to
//             execute some script code - calls any registered script entry hooks.
// See:        SKDEBUG_HOOK_SCRIPT_ENTRY() and other hook functions and macros.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_script_origin_push(const ASymbol & origin_id)
  {
  // Push new script origin on stack
  ms_hook_script_origin_idx++;
  ms_hook_script_origin_stack[ms_hook_script_origin_idx] = origin_id;

  // Call registered Skookum script system entry hooks.
  if (ms_hook_origins.is_filled())
    {
    const Hook * end_p  = ms_hook_origins.get_sentinel();
    Hook *       hook_p = ms_hook_origins.get_first();

    do 
      {
      hook_p->m_hook_script_entry_f(origin_id);
      hook_p = ms_hook_origins.get_next(hook_p);
      }
    while (hook_p != end_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever the Skookum scripting system is about to be exited after
//             having executed some script code - calls any registered script exit hooks.
// See:        SKDEBUG_HOOK_SCRIPT_EXIT() and other hook functions and macros.
// Modifiers:   static
// Author(s):   Conan Reis
void SkDebug::hook_script_origin_pop()
  {
  // Call registered Skookum script system exit hooks.
  if (ms_hook_origins.is_filled())
    {
    const ASymbol & origin_id = get_hook_script_origin();
    const Hook *    end_p     = ms_hook_origins.get_sentinel();
    Hook *          hook_p    = ms_hook_origins.get_first();

    do 
      {
      hook_p->m_hook_script_exit_f(origin_id);
      hook_p = ms_hook_origins.get_next(hook_p);
      }
    while (hook_p != end_p);
    }

  // Pop old script origin back on to stack
  ms_hook_script_origin_idx--;
  }

#endif // SKDEBUG_HOOKS


//---------------------------------------------------------------------------------------
// Default expression debug function
// Arg         expr_p - expression about to be invoked.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.
// See:        set_hook_expr(), breakpoint_get_by_expr()
// Author(s):   Conan Reis
void SkDebug::breakpoint_hit_embedded_def(
  SkExpressionBase * expr_p,
  SkObjectBase *     scope_p,
  SkInvokedBase *    caller_p
  )
  {
  // Default is to do a C++ breakpoint

  #if (SKOOKUM & SK_DEBUG) 
    // Could do a C++ break
    ADebug::print(get_context_string(
      "\nHit Skookum scripted expression tagged with a breakpoint!\n",
      scope_p,
      caller_p));
  
    A_BREAK();
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params break({Object} objs_as_strs)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_break(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if (SKOOKUM & SK_DEBUG)
    AString message;
    const SkInstanceList & objs = scope_p->get_arg<SkList>(SkArg_1);

    if (objs.get_length())
      {
      objs.append_elems_as_strings(&message, scope_p);
      }
    else
      {
      message = "Debug->Continue to resume...";
      }

    if (ms_scripted_break_f)
      {
      // $Note - CReis If the SkookumScript IDE is embedded it is set to SkConsole::debug_scripted_break()
      (ms_scripted_break_f)(message, scope_p);
      }
    else
      {
      // If remote runtime client is nullptr then this is a script running on the IDE
      if (SkRemoteRuntimeBase::ms_client_p && SkRemoteRuntimeBase::ms_client_p->is_connected())
        {
        SkDebug::print_agog(
          a_str_format("\nSkookum scripted break: %s\n", message.as_cstr()),
          SkLocale_all,
          SkDPrintType_trace);

        // Break in script a call point
        break_invokable(scope_p);

        // Build a callstack (builds it a second time but not time critical here)
        ARefPtr<SkCallStack> callstack_p = get_callstack(scope_p->get_caller(), nullptr);

        // Have runtime communicate to IDE that it is at a break
        SkRemoteRuntimeBase::ms_client_p->on_break_expression(ms_next_expr, callstack_p);
        }
      else
        {
        // Local to IDE or IDE not connected
        SK_ERROR_INVOKED(a_str_format("\nSkookum scripted break: %s\n[Should be safe to ignore.]\n", message.as_cstr()));
        }
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params callstack()
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_callstack(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if (SKOOKUM & SK_DEBUG)
    print_callstack(scope_p, SkInvokeInfo_skip_this | SkInvokeInfo__callstack_def);
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params callstack_str() String
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_callstack_str(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    AString str;
    
    #if (SKOOKUM & SK_DEBUG)
      append_callstack_string(&str, scope_p, SkInvokeInfo_skip_this | SkInvokeInfo__callstack_def);
    #endif

    *result_pp = SkString::new_instance(str);
    }
  }

//---------------------------------------------------------------------------------------
// Skoo Params copy_hook(Symbol hook_name, String new_name)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_copy_hook(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SKDEBUG_HOOKS)
    copy_hook(
      scope_p->get_arg<SkSymbol>(SkArg_1),
      scope_p->get_arg<SkString>(SkArg_2));
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params enable_hook(Symbol hook_name, Boolean enable: true)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_enable_hook(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SKDEBUG_HOOKS)
    enable_hook(
      scope_p->get_arg<SkSymbol>(SkArg_1),
      scope_p->get_arg<SkBoolean>(SkArg_2));
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params hook_condition_scope_actor(Symbol hook_name, Symbol actor_name)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_hook_condition_scope_actor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SKDEBUG_HOOKS)
    hook_condition_scope_actor(
      scope_p->get_arg<SkSymbol>(SkArg_1),
      scope_p->get_arg<SkSymbol>(SkArg_2));
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params hook_condition_scope_actor(Symbol hook_name, InvokedContextBase caller)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_hook_condition_invoked_caller(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SKDEBUG_HOOKS)
    hook_condition_invoked_caller(
      scope_p->get_arg<SkSymbol>(SkArg_1),
      scope_p->get_arg_data<SkInvokedContextBase>(SkArg_2));
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params hook_condition_scope_updater_class(Symbol hook_name, <Actor> updater_class, Boolean subclass_check)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_hook_condition_updater_class(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SKDEBUG_HOOKS)
    hook_condition_updater_class(
      scope_p->get_arg<SkSymbol>(SkArg_1),
      scope_p->get_arg_data<SkClass>(SkArg_2),
      scope_p->get_arg<SkBoolean>(SkArg_3));
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params is_engine_present() Boolean
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_engine_presentQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(ms_engine_present_b);
    }
  }

//---------------------------------------------------------------------------------------
// Skoo Params hook_names() String
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_hook_names(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    AString str;

    #if defined(SKDEBUG_HOOKS)
      bool    enabled;
      Hook ** hooks_pp     = ms_hooks.get_array();
      Hook ** hooks_end_pp = hooks_pp + ms_hooks.get_length();

      AString * hook_name_p;
      uint32_t  str_length = 0u;

      while (hooks_pp < hooks_end_pp)
        {
        enabled = (*hooks_pp)->is_enabled();
        hook_name_p = &(*hooks_pp)->m_hook_name;
        str_length += hook_name_p->get_length() + (enabled ? 6u : 5u);
        str.ensure_size(str_length);
        
        if (enabled)
          {
          // Add asterisk to indicate that it is enabled.
          str.append("*'", 2u);
          }
        else
          {
          str.append('\'');
          }

        str.append(*hook_name_p);

        hooks_pp++;

        if (hooks_pp < hooks_end_pp)
          {
          str.append("', ", 3u);
          }
        else
          {
          str.append('\'');
          }
        }
    #endif

    *result_pp = SkString::new_instance(str);
    }
  }

//---------------------------------------------------------------------------------------
// Skoo Params is_hook_present(Symbol hook_name) Boolean
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_hook_enabledQ(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkBoolean::new_instance(
      #if defined(SKDEBUG_HOOKS)
        is_hook_enabled(scope_p->get_arg<SkSymbol>(SkArg_1))
      #else
        false
      #endif
      );
    }
  }

//---------------------------------------------------------------------------------------
// Skoo Params print_memory_execution()
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):  Conan Reis
static void SkDebug_mthdc_print_memory_execution(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  tSkInteger execution_bytes = 0;

  #if defined(SK_KEEP_DPRINT)
    execution_bytes = tSkInteger(SkDebug::print_memory_runtime());
  #endif
  
  if (result_pp)
    {
    *result_pp = SkInteger::new_instance(execution_bytes);
    }
  }

//---------------------------------------------------------------------------------------
// Skoo Params print({Object} objs_as_strs)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_print(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SK_KEEP_DPRINT)
    if (!ms_suppress_prints)
      {
      AString str;
      const SkInstanceList & objs = scope_p->get_arg<SkList>(SkArg_1);

      objs.append_elems_as_strings(&str, scope_p);

      print_agog(str, SkLocale_all, SkDPrintType_standard);
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params println({Object} objs_as_strs)
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_println(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  #if defined(SK_KEEP_DPRINT)
    if (!ms_suppress_prints)
      {
      AString str;
      const SkInstanceList & objs = scope_p->get_arg<SkList>(SkArg_1);

      objs.append_elems_as_strings(&str, scope_p);
      str.append('\n');

      print_agog(str, SkLocale_all, SkDPrintType_standard);
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Skoo Params sym_to_str(Symbol sym) String
// [See script file.]
// Notes:      This is a class method.
// C++ Args    See tSkMethodFunc or tSkMethodMthd in SkookumScript/SkMethod.hpp
// Author(s):   Conan Reis
void SkDebug::mthdc_sym_to_str(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  // Do nothing if result not desired
  if (result_pp)
    {
    *result_pp = SkString::new_instance(scope_p->get_arg<SkSymbol>(SkArg_1).as_str_dbg());
    }
  }

//---------------------------------------------------------------------------------------
//  Registers the atomic classes, coroutines, etc.
// Notes:       This method is called by Brain::initialize_post_load()
// Modifiers:    static
// Author(s):    Conan Reis
void SkDebug::register_bindings()
  {
  // Class Methods
  SkBrain::ms_debug_class_p->register_method_func("break",                         mthdc_break,                          SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("callstack",                     mthdc_callstack,                      SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("callstack_str",                 mthdc_callstack_str,                  SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("copy_hook",                     mthdc_copy_hook,                      SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("enable_hook",                   mthdc_enable_hook,                    SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("engine_present?",               mthdc_engine_presentQ,                SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("hook_names",                    mthdc_hook_names,                     SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("hook_condition_scope_actor",    mthdc_hook_condition_scope_actor,     SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("hook_condition_invoked_caller", mthdc_hook_condition_invoked_caller,  SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("hook_condition_updater_class",  mthdc_hook_condition_updater_class,   SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("hook_enabled?",                 mthdc_hook_enabledQ,                  SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("print_memory_execution",        SkDebug_mthdc_print_memory_execution, SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("print",                         mthdc_print,                          SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("println",                       mthdc_println,                        SkBindFlag_class_no_rebind);
  SkBrain::ms_debug_class_p->register_method_func("sym_to_str",                    mthdc_sym_to_str,                     SkBindFlag_class_no_rebind);

  // $Note - CReis Just putting this here as a hokey way to ensure that info() is not
  // optimized out.
  if (SkBrain::ms_debug_class_p == nullptr)
    {
    print_info();
    }
  }
