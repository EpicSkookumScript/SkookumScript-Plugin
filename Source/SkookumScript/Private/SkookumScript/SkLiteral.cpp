// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Literal class for standard literals (Boolean, Character, Integer, Real, String & Symbol)
// and identifier for special objects (Class,  nil, this, this_class, this_code, this_mind)
// List literal class for list objects fully defined in code
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkLiteral.hpp>
#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AString.hpp>
#include <AgogCore/AStringRef.hpp>

#include <SkookumScript/SkBoolean.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkInvokedBase.hpp>
#include <SkookumScript/SkList.hpp>
#include <SkookumScript/SkMethodCall.hpp>
#include <SkookumScript/SkMind.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumScript/SkReal.hpp>
#include <SkookumScript/SkInteger.hpp>


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
  {
  class ACompareStrRef
  {
  public:
  // Class Methods

    // Returns true if elements are equal
    static bool equals(const AStringRef & lhs, const AStringRef & rhs)
      {
      return lhs.compare(rhs) == AEquate_equal;
      }

    // Returns 0 if equal, < 0 if lhs is less than rhs, and > 0 if lhs is greater than rhs
    static ptrdiff_t comparison(const AStringRef & lhs, const AStringRef & rhs)
      {
      return ptrdiff_t(lhs.compare(rhs));
      }
  };

  #if defined(SKLITERAL_STRING_POOL) || defined(SKLITERAL_STRING_TRACK_DUPES)
    uint32_t g_pooled_string_extra_count = 0u;
    uint32_t g_pooled_string_extra_bytes = 0u;

    APSorted<AStringRef, AStringRef, ACompareStrRef> g_pooled_strings;
  #endif
  };


//=======================================================================================
// SkLiteral Method Definitions
//=======================================================================================

#if defined(SKLITERAL_STRING_POOL)

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AStringRef * SkLiteral::get_pooled_str(const AString & str)
  {
  uint32_t     idx;
  AStringRef * str_ref_p = str.get_str_ref();
  bool         found     = g_pooled_strings.find(*str.get_str_ref(), AMatch_first_found, &idx);

  if (found)
    {
    #ifdef SKLITERAL_STRING_TRACK_DUPES
      g_pooled_string_extra_count++;
      g_pooled_string_extra_bytes += str_ref_p->m_length + 1u;
    #endif

    return g_pooled_strings.get_array()[idx];
    }

  str_ref_p->m_ref_count++;
  g_pooled_strings.insert(*str_ref_p, idx);

  return str_ref_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
uint32_t SkLiteral::get_pooled_str_idx(const AString & str)
  {
  uint32_t idx = UINT32_MAX;

  g_pooled_strings.find(*str.get_str_ref(), AMatch_first_found, &idx);

  return idx;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AStringRef * SkLiteral::get_pooled_str_at(uint32_t idx)
  {
  return g_pooled_strings.get_array()[idx];
  }


#ifdef SKLITERAL_STRING_TRACK_DUPES

//---------------------------------------------------------------------------------------
void SkLiteral::get_pooled_string_info(uint32_t * extra_bytes_p, uint32_t * extra_refs_p)
  {
  *extra_refs_p  = g_pooled_string_extra_count;
  *extra_bytes_p = g_pooled_string_extra_bytes;
  }

#endif // SKLITERAL_STRING_TRACK_DUPES

#endif // SKLITERAL_STRING_POOL


//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         kind - type of literal to store
// Arg         data_p - pointer to literal data - differs for each type of literal.
// Author(s):   Conan Reis
SkLiteral::SkLiteral(
  eType        kind,
  const void * data_p // = nullptr
  ) :
  m_kind(kind)
  {
  switch (kind)
    {
    case Type_string:
      new (&m_data) AString(*reinterpret_cast<const AString *>(data_p));
      break;

    case Type_symbol:
      new (&m_data) ASymbol(*reinterpret_cast<const ASymbol *>(data_p));
      break;

    case Type__class:
      m_data.set((SkMetaClass *)data_p);
      break;

    case Type__nil:
    case Type__this:
    case Type__this_class:
    case Type__this_code:
    case Type__this_mind:
      memset(&m_data, 0, sizeof(m_data));
      break;

    //case Type_boolean:
    //case Type_integer:
    //case Type_real:
    default:
      m_data = *(UserData *)data_p;
    }
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         str - string to store
// Author(s):   Conan Reis
SkLiteral::SkLiteral(const AString & str) :
  m_kind(Type_string)
  {
  #if defined(SKLITERAL_STRING_POOL)
    new (&m_data) AString(get_pooled_str(str));
  #else
    new (&m_data) AString(str);
  #endif
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         sym - symbol to store
// Author(s):   Conan Reis
SkLiteral::SkLiteral(const ASymbol & sym) :
  m_kind(Type_symbol)
  {
  new (&m_data) ASymbol(sym);
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkLiteral::~SkLiteral()
  {
  switch (m_kind)
    {
    case Type_symbol:
      reinterpret_cast<ASymbol *>(&m_data)->~ASymbol();
      break;

    #if !defined(SKLITERAL_STRING_POOL)
      case Type_string:
        reinterpret_cast<AString *>(&m_data)->~AString();
        break;
    #endif
        
    default: break; // Make Clang happy
    }
  }

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_literal
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkLiteral::get_type() const
  {
  return SkExprType_literal;
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               1 byte  - literal kind
//               if Type_string
//                 4 bytes - string length
//                 n bytes - string characters
//               if Type_boolean, Type_integer, Type_real, Type_symbol, or Type__class
//                 4 bytes - literal info
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkLiteral::SkLiteral(const void ** binary_pp)
  {
  // 1 byte - literal kind
  m_kind = static_cast<eType>(A_BYTE_STREAM_UI8_INC(binary_pp));

  // n bytes - literal binary
  switch (m_kind)
    {
    case Type_symbol:
      new (&m_data) ASymbol(ASymbol::create_from_binary(binary_pp));
      break;

    case Type_string:
      #if defined(SKLITERAL_STRING_POOL)
        new (&m_data) AString(get_pooled_str_at(A_BYTE_STREAM_UI32_INC(binary_pp)));
      #else
        new (&m_data) AString(binary_pp);
      #endif
      break;

    case Type__class:
      m_data.set(&SkClass::from_binary_ref(binary_pp)->get_metaclass());
      break;

    case Type__nil:
    case Type__this:
    case Type__this_class:
    case Type__this_code:
    case Type__this_mind:
      // The id is all that is needed.
      break;

    default: // Type_integer, Type_real, or Type_boolean
      // 4 bytes - Simple data
      static_assert(sizeof(tSkInteger) <= 4 && sizeof(tSkReal) <= 4 && sizeof(tSkBoolean) <= 4, "must hold for the serialization to work");
      m_data.set<uint32_t>(A_BYTE_STREAM_UI32_INC(binary_pp));
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


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
//               1 byte  - literal kind
//               if Type_string
//                 4 bytes - string length
//                 n bytes - string characters
//               if Type_boolean, Type_integer, Type_real, Type_symbol, or Type__class
//                 4 bytes - literal info
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteral::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkLiteral::as_binary_length());

  uint8_t ** data_pp = (uint8_t **)binary_pp;

  // 1 byte - expression type
  **data_pp = static_cast<uint8_t>(m_kind);
  (*data_pp)++;

  switch (m_kind)
    {
    case Type_symbol:
      m_data.as<ASymbol>()->as_binary(binary_pp);
      break;

    case Type_string:
      {
      #ifdef SKLITERAL_STRING_POOL
        uint32_t str_idx = get_pooled_str_idx(*reinterpret_cast<const AString *>(&m_data));
        A_BYTE_STREAM_OUT32(binary_pp, &str_idx);
      #else
        m_data.as<AString>()->as_binary(binary_pp);
      #endif
      break;
      }

    case Type__class:
      m_data.as<SkMetaClass>()->as_binary_ref(binary_pp);
      break;

    case Type__nil:
    case Type__this:
    case Type__this_class:
    case Type__this_code:
    case Type__this_mind:
      // The id is all that is needed.
      break;

    default:  // Type_boolean, Type_integer, or Type_real
      // 4 bytes - value
      static_assert(sizeof(tSkInteger) <= 4 && sizeof(tSkReal) <= 4 && sizeof(tSkBoolean) <= 4, "must hold for the serialization to work");
      A_BYTE_STREAM_OUT32(binary_pp, m_data.as<uint32_t>());
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               1 byte  - literal kind
//               if Type_string
//                 4 bytes - string length
//                 n bytes - string characters
//               if Type_boolean, Type_integer, Type_real, Type_symbol, or Type__class
//                 4 bytes - literal info
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkLiteral::as_binary_length() const
  {
  switch (m_kind)
    {
    #ifndef SKLITERAL_STRING_POOL
      case Type_string:
        return 5u + m_data.as<AString>()->get_length();
    #endif

    case Type__nil:
    case Type__this:
    case Type__this_class:
    case Type__this_code:
    case Type__this_mind:
      return 1u;

    default: // Type_boolean, Type_integer, Type_real, Type_symbol, Type__class:
      static_assert(SkClass::Binary_ref_size == 4u, "This code assumes that SkClass::Binary_ref_size is 4.");
      return 5u;
    }
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
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkLiteral::as_code() const
  {
  switch (m_kind)
    {
    case Type_boolean:
      return *m_data.as<tSkBoolean>() ? "true" : "false";

    case Type_integer:
      return AString::ctor_int(*m_data.as<tSkInteger>());

    case Type_real:
      return AString::ctor_float(*m_data.as<tSkReal>());

    case Type_string:
      {
      AString esc_str(SkString::to_escape_string(*m_data.as<AString>()));
      AString str(nullptr, 3u + esc_str.get_length(), 0u);

      str.append('"');
      str.append(esc_str);
      str.append('"');
      return str;
      }

    case Type_symbol:
      {
      AString sym_str(m_data.as<ASymbol>()->as_str_dbg());
      AString str(nullptr, 4u + sym_str.get_length(), 0u);

      // $Revisit - CReis Should convert non-printable characters (i.e. newline) to their
      // escape character equivalents (i.e. '\n').
      str.append('\'');
      str.append(sym_str);
      str.append('\'');
      return str;
      }


    // These should be more correctly categorized as identifiers, but making them a
    // literal allows a bit of initial optimization to occur.

    case Type__class:
      return m_data.as<SkMetaClass>()->get_class_info()->get_name_str_dbg();

    case Type__this:
      return "this";

    case Type__this_class:
      return "this_class";

    case Type__this_code:
      return "this_code";

    case Type__this_mind:
      return "this_mind";

    default:  // Type__nil
      return "nil";
    }
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Determine if this literal represents the Debug class
bool SkLiteral::is_debug_class() const
  {
  return m_kind == SkLiteral::Type__class
      && m_data.as<SkMetaClass>() == &SkBrain::ms_debug_class_p->get_metaclass();
  }

//---------------------------------------------------------------------------------------
// Evaluates literal expression and returns an instance version of itself
// Returns:    true - indicating that the expression has completed its evaluation and
//             that there is a resulting instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Notes:      No caller object is needed since SkLiteral always returns the result
//             immediately.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkLiteral::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // If no result is desired then don't bother doing any evaluation
  if (result_pp)
    {
    switch (m_kind)
      {
      case Type_boolean:
        *result_pp = SkBoolean::new_instance(*m_data.as<tSkBoolean>());
        break;

      case Type_integer:
        *result_pp = SkInteger::new_instance(*m_data.as<tSkInteger>());
        break;

      case Type_real:
        *result_pp = SkReal::new_instance(*m_data.as<tSkReal>());
        break;

      case Type_string:
        *result_pp = SkString::new_instance(*m_data.as<AString>());
        break;

      case Type_symbol:
        *result_pp = SkSymbol::new_instance(*m_data.as<ASymbol>());
        break;


      // These should be more correctly categorized as identifiers, but making them a
      // literal allows a bit of initial optimization to occur.

      case Type__class:      // A Class
        // MetaClass objects do not need to be referenced/dereferenced
        *result_pp = m_data.as<SkMetaClass>();
        break;

      case Type__nil:        // The single instance of the 'None' class - nil
        // The nil object does not need to be referenced/dereferenced
        *result_pp = SkBrain::ms_nil_p;
        break;

      case Type__this:       // The current owner instance which is at the topmost scope
        {
        SkInstance * this_p = scope_p->get_topmost_scope();

        this_p->reference();
        *result_pp = this_p;
        break;
        }

      case Type__this_class: // The class of the current owner instance which is at the topmost scope
        // MetaClass objects do not need to be referenced/dereferenced
        *result_pp = &scope_p->get_topmost_scope()->get_class()->get_metaclass();
        break;

      case Type__this_code:  // The invoked code instance (method or coroutine)
        {
        SkInvokedContextBase * icontext_p = scope_p->get_scope_context();
        // new_instance() auto calls reference() and nil does not need to be referenced/dereferenced
        *result_pp = icontext_p ? icontext_p->as_new_instance() : SkBrain::ms_nil_p;
        break;
        }

      case Type__this_mind:  // The updater mind for the current routine
        {
        SkInstance * mind_p = nullptr;

        if (caller_p)
          {
          mind_p = caller_p->get_updater();
          }

        if (mind_p == nullptr)
          {
          mind_p = SkookumScript::get_master_mind();

          #ifdef SK_RUNTIME_RECOVER
            // Master mind can be null if runtime not running
            if (mind_p == nullptr)
              {
              mind_p = SkBrain::ms_nil_p;
              }
          #endif
          }

        mind_p->reference();

        *result_pp = mind_p;
        break;
        }
      }
    }
  
  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteral::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkLiteral, SkDebugInfo_size_used));

  // Breakdown literals by type
  const char * type_p   = nullptr;
  uint32_t   dyn_needed = 0u;
  uint32_t   dyn_actual = 0u;

  switch (m_kind)
    {
    case Type_boolean:
      type_p = "SkLiteral.Boolean";
      break;

    case Type_integer:
      type_p = "SkLiteral.Integer";
      break;

    case Type_real:
      type_p = "SkLiteral.Real";
      break;

    case Type_string:
      type_p = "SkLiteral.String";

      #ifndef SKLITERAL_STRING_POOL
        {
        const AString * str_p = as_literal_string();

        dyn_needed = str_p->get_length() + 1u;
        dyn_actual = str_p->get_size();
        }
      #endif
      break;

    case Type_symbol:
      type_p = "SkLiteral.Symbol";
      break;

    case Type__class:
      type_p = "SkLiteral.Class";
      break;

    case Type__nil:
      type_p = "SkLiteral.nil";
      break;

    case Type__this:
      type_p = "SkLiteral.this";
      break;

    case Type__this_class:
      type_p = "SkLiteral.this_class";
      break;

    case Type__this_code:
      type_p = "SkLiteral.this_code";
      break;

    case Type__this_mind:
      type_p = "SkLiteral.this_mind";
      break;
    }

  mem_stats_p->track_memory(type_p, 0u, 0u, dyn_needed, dyn_actual);
  }


//=======================================================================================
// SkLiteralList Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
SkLiteralList::~SkLiteralList()
  {
  delete m_ctor_p;
  m_item_exprs.free_all();
  }


#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Constructor from binary info
// Returns:    itself
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary() or a similar mechanism.
// See:        as_binary()
// Notes:      Binary composition:
//               4 bytes  - class name id 
//               1 byte   - call type (tSkInvoke_...)
//              [n bytes] - constructor method call
//               2 bytes  - item count
//               n bytes  - expression typed binary  }- repeating
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Author(s):   Conan Reis
SkLiteralList::SkLiteralList(const void ** binary_pp)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 4 bytes  - class name id 
  m_list_type_p = SkClass::from_binary_ref(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 1 byte    - call type (tSkInvoke_...)
  // [n bytes] - constructor method call
  m_ctor_p = static_cast<SkMethodCallBase *>(SkInvokeBase::from_binary_typed_new(binary_pp));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - item count
  uint32_t length = A_BYTE_STREAM_UI16_INC(binary_pp);

  m_item_exprs.set_size(length);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // n bytes - expression typed binary  }- repeating
  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + length;

  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    *expr_pp = SkExpressionBase::from_binary_typed_new(binary_pp);
    }
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to
//             recreate this list literal and increments the memory address to just past
//             the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit all the binary data.  Use the get_binary_length()
//             method to determine the size needed prior to passing binary_pp to this
//             method.
// See:        as_binary_length()
// Notes:      Used in combination with as_binary_length().
//
//             Binary composition:
//               4 bytes  - class name id 
//               1 byte   - call type (tSkInvoke_...)
//              [n bytes] - constructor method call
//               2 bytes  - item count
//               n bytes  - expression typed binary  }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteralList::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkLiteralList::as_binary_length());

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 4 bytes  - class name id 
  m_list_type_p->as_binary_ref(binary_pp);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 1 byte   - call type (tSkInvoke_...)
  **(uint8_t **)binary_pp = static_cast<uint8_t>(m_ctor_p ? SkInvokeType_method_on_instance : SkInvokeType__invalid);
  (*(uint8_t **)binary_pp)++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // [n bytes] - constructor method call
  if (m_ctor_p)
    {
    m_ctor_p->as_binary(binary_pp);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 2 bytes - item count
  // $Note - CReis I'm assuming that 65535 items is a sufficient max for a list literal.
  uint32_t length = m_item_exprs.get_length();

  **(uint16_t **)binary_pp = uint16_t(length);
  (*(uint16_t **)binary_pp)++;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // n bytes - expression typed binary  }- repeating
  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + length;

  // Iterate through item expressions and invoke then to obtain item instances
  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    (*expr_pp)->as_binary_typed(binary_pp);
    }
  }

//---------------------------------------------------------------------------------------
// Returns length of binary version of itself in bytes.
// Returns:    length of binary version of itself in bytes
// See:        as_binary()
// Notes:      Used in combination with as_binary()
//
//             Binary composition:
//               4 bytes  - class name id 
//               1 byte   - call type (tSkInvoke_...)
//              [n bytes] - constructor method call
//               2 bytes  - item count
//               n bytes  - expression typed binary  }- repeating
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
uint32_t SkLiteralList::as_binary_length() const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // class ref + call type + item count
  uint32_t binary_length = SkClass::Binary_ref_size + 3u;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // [n bytes] - constructor method call
  if (m_ctor_p)
    {
    binary_length += m_ctor_p->as_binary_length();
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // n bytes - expression typed binary  }- repeating
  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_item_exprs.get_length();

  // Iterate through item expressions and invoke then to obtain item instances
  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    binary_length += (*expr_pp)->as_binary_typed_length();
    }

  return binary_length;
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


#if (SKOOKUM & SK_CODE_IN)

//---------------------------------------------------------------------------------------
// #Description
//   Indicates whether this expression (or any sub-expression) has any potential side
//   effects when used as a stand alone statement - i.e. not used as an argument/receiver
//   or returned as the last expression in a code block.
//
// #Modifiers virtual - override for custom behaviour
// #See Also  find_expr_last_no_side_effect()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // #Returns See eSkSideEffect
  eSkSideEffect
SkLiteralList::get_side_effect() const
  {
  if (m_ctor_p)
    {
    return SkSideEffect_secondary;
    }

  uint32_t count = m_item_exprs.get_length();

  if (count)
    {
    // Iterate through item expressions
    SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
    SkExpressionBase ** expr_end_pp = expr_pp + count;

    while (expr_pp < expr_end_pp)
      {
      if ((*expr_pp)->get_side_effect())
        {
        return SkSideEffect_secondary; 
        }

      expr_pp++;
      }
    }

  return SkSideEffect_none;
  }

#endif


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent.  This is
//             essentially a disassembly of the internal data-structures to source code.
// Returns:    Source code string version of itself
// See:        as_binary()
// Notes:      The code generated may not look exactly like the original source - for
//             example any comments will not be retained, but it should parse equivalently.
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
AString SkLiteralList::as_code() const
  {
  AString   str(nullptr, 64u, 0u);
  SkClass * list_type_p = m_list_type_p;

  if (m_ctor_p || (list_type_p != SkBrain::ms_list_class_p))
    {
    str.append(list_type_p->as_code());
    }

  if (m_ctor_p)
    {
    str.append(m_ctor_p->get_args().is_empty()
      ? m_ctor_p->get_name_str_dbg()
      : m_ctor_p->as_code());
    }

  str.append('{');

  // Iterate through item expressions
  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_item_exprs.get_length();

  while (expr_pp < expr_end_pp)
    {
    str.append((*expr_pp)->as_code());
    expr_pp++;

    if (expr_pp < expr_end_pp)
      {
      str.append(", ", 2u);
      }
    }

  str.append('}');

  // $Revisit - CReis If the list contains long items or if the item code contains
  // more than one line then the items could each be placed on their own line to make
  // the string more presentable.

  return str;
  }

#endif // defined(SK_AS_STRINGS)


#if (SKOOKUM & SK_DEBUG)

//---------------------------------------------------------------------------------------
// Determines if this expression or the first sub-expression that it contains
//             was located at or follows at the character index position provided and
//             returns it.
// Returns:    The first expression that starts or follows the given position or nullptr if
//             no such expression was found.
// Arg         pos - code file/string character index position
// Notes:      Some sub-expressions may have the same starting character position as
//             their containing expression - in these cases only the containing
//             expression is returned.
// Modifiers:   virtual
// Author(s):   Conan Reis
SkExpressionBase * SkLiteralList::find_expr_by_pos(
  uint        pos,
  eSkExprFind type // = SkExprFind_all
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check constructor call sub-items for position
  SkExpressionBase * expr_p;

  if (m_ctor_p)
    {
    // Check for breakpoint as list is created and before items are evaluated.
    // Return for any find type
    if ((m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
      {
      return const_cast<SkLiteralList *>(this);
      }

    expr_p = m_ctor_p->find_expr_by_pos(pos, type);

    if (expr_p)
      {
      return expr_p;
      }
    }
  else
    {
    // Return only if all find types are desired
    if ((type == SkExprFind_all) && (m_source_idx != SkExpr_char_pos_invalid) && (m_source_idx >= pos))
      {
      return const_cast<SkLiteralList *>(this);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check list sub-items for position
  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_item_exprs.get_length();

  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    expr_p = (*expr_pp)->find_expr_by_pos(pos, type);

    if (expr_p)
      {
      return expr_p;
      }
    }

  // Code character position not located within this list literal
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
SkLiteralList::iterate_expressions(
  // Calls apply_expr() on each expression - see SkApplyExpressionBase
  SkApplyExpressionBase * apply_expr_p, 
  // Optional invokable (method, coroutine) where this expression originates or nullptr.
  const SkInvokableBase * invokable_p // = nullptr
  )
  {
  if (apply_expr_p->apply_expr(this, invokable_p)
    || (m_ctor_p && m_ctor_p->iterate_expressions(apply_expr_p, invokable_p)))
    {
    return AIterateResult_early_exit;
    }

  SkExpressionBase ** expr_pp     = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + m_item_exprs.get_length();

  for (; expr_pp < expr_end_pp; expr_pp++)
    {
    if ((*expr_pp)->iterate_expressions(apply_expr_p, invokable_p))
      {
      return AIterateResult_early_exit;
      }
    }

  return AIterateResult_entire;
  }

#endif // (SKOOKUM & SK_DEBUG)


//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of
//             expressions when it is only known that an instance is of type
//             SkookumScript/SkExpressionBase.
// Returns:    SkExprType_literal
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkLiteralList::get_type() const
  {
  return SkExprType_literal_list;
  }

//---------------------------------------------------------------------------------------
// Evaluates literal list expression and returns an instance version of
//             itself.
// Returns:    true - indicating that the expression has completed its evaluation and
//             that there is a resulting instance.
// Arg         scope_p - scope for data/method/etc. look-ups
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.  (Default nullptr)
// Arg         result_pp - pointer to a pointer to store the instance resulting from the
//             invocation of this expression.  If it is nullptr, then the result does not
//             need to be returned and only side-effects are desired.  (Default nullptr)
// See:        invoke_now()
// Notes:      No caller object is needed since SkLiteral always returns the result
//             immediately.
// Modifiers:   virtual (overriding pure from SkExpressionBase)
// Author(s):   Conan Reis
SkInvokedBase * SkLiteralList::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create list and append item objects to it

  // $Note - CReis Even if no result is desired still evaluate the expressions in case
  // there are any side effects.  Also note that even when the list is not returned, all
  // the items co-exist before they are dereferenced since that behaviour would occur
  // with a list.

  // Create list instance (of correct type)
  SkInstanceList * list_p;
  SkClass *        list_type_p = m_list_type_p;
  SkInstance *     new_p       = list_type_p->new_instance();
  uint32_t         length      = m_item_exprs.get_length();

  if (m_ctor_p)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Call constructor method if one is used

    // Call any debugger handlers if there is a constructor
    // Debugging - store current expression in global rather than passing as argument since
    // only used for debug.
    SKDEBUG_ICALL_STORE_GEXPR(this);

    // $Note - CReis Constructors do not return a value so use List constructor result_pp
    // parameter as a flag to indicate that the SkList structure has already been created
    // by a List literal.
    SkInstance * result_flag_p;

    // Call to SKDEBUG_HOOK_EXPR() made in invoked_call()
    // Constructor methods always return immediately
    m_ctor_p->invoke_call(new_p, scope_p, caller_p, &result_flag_p);
    list_p = &new_p->as<SkList>();
    list_p->ensure_size(length);
    }
  else
    {
    if (list_type_p != SkBrain::ms_list_class_p)
      {
      // Call default constructor
      new_p->call_default_constructor();
      list_p = &new_p->as<SkList>();
      list_p->ensure_size(length);
      }
    else
      {
      // Make simple instance if List and no custom constructor called
      static_cast<SkList *>(new_p)->construct(length);
      list_p = &new_p->as<SkList>();
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through item expressions and invoke then to obtain item instances
  SkExpressionBase ** expr_pp = m_item_exprs.get_array();
  SkExpressionBase ** expr_end_pp = expr_pp + length;
  SkInstance **       items_pp = list_p->get_array();

  for (; expr_pp < expr_end_pp; expr_pp++, items_pp++)
    {
    *items_pp = (*expr_pp)->invoke_now(scope_p, caller_p);
    }
  list_p->get_instances().set_length_unsafe(length);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean up
  if (result_pp)
    {
    *result_pp = new_p;
    }
  else
    {
    new_p->dereference();
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Tracks memory used by this object and its sub-objects
// See:        SkDebug, AMemoryStats
// Modifiers:   virtual - overridden from SkExpressionBase
// Author(s):   Conan Reis
void SkLiteralList::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(
    SKMEMORY_ARGS(SkLiteralList, SkDebugInfo_size_used),
    m_item_exprs.get_length() * sizeof(void *),
    m_item_exprs.track_memory(mem_stats_p));

  if (m_ctor_p)
    {
    m_ctor_p->track_memory(mem_stats_p);
    }
  }
