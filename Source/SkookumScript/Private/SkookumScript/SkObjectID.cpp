// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// SkookumScript Validated Object IDs
//=======================================================================================



//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkObjectId.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkObjectId.inl>
#endif

#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkCode.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkObjectBase.hpp>


//=======================================================================================
// SkObjectID Method Definitions
//=======================================================================================


#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to recreate this
// Object ID expression and increments the memory address to just past the last byte
// written.
// 
// Params:
//   binary_pp:
//     Pointer to address to fill and increment. Its size *must* be large enough to fit
//     all the binary data.  Use the `get_binary_length()` method to determine the size
//     needed prior to passing `binary_pp` to this method.
//     
// Notes:
//   Used in combination with `as_binary_length()`.
//
//             Binary composition:
//               n bytes - identifier name string
//               4 bytes - class name id
//               1 byte  - flags
//
// Modifiers:   virtual from SkExpressionBase
// See:       as_binary_length()
// Author(s):   Conan Reis
void SkObjectID::as_binary(void ** binary_pp) const
  {
  A_SCOPED_BINARY_SIZE_SANITY_CHECK(binary_pp, SkObjectID::as_binary_length());

  // n bytes - bind name string
  m_bind_name.as_binary(binary_pp);

  // 4 bytes - class name id
  m_class_p->as_binary_ref(binary_pp);

  // 1 byte - flags (only bother storing first 8 flags)
  uint8_t flags_byte = uint8_t(m_flags);
  A_BYTE_STREAM_OUT8(binary_pp, &flags_byte);
  }

//---------------------------------------------------------------------------------------
// Calculates length of binary version of itself.
// 
// Returns:    length of binary version of itself in bytes
// 
// Notes:
//   Used in combination with `as_binary()`.
//
//             Binary composition:
//       n bytes - identifier name string
//               4 bytes - class name id
//               1 byte  - flags
//       
// Modifiers:   virtual from SkExpressionBase
// See:       as_binary()
// Author(s):   Conan Reis
uint32_t SkObjectID::as_binary_length() const
  {
  return 1u + SkClass::Binary_ref_size + m_bind_name.as_binary_length();
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from data structures to code strings
#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Converts this expression into its source code string equivalent. This is essentially a
// disassembly of the internal data-structures to source code.
// 
// Returns:    Source code string version of itself
// 
// Notes:
//   The code generated may not look exactly like the original source - for example the
//   class could have been inferred.
//   
// See:        as_binary()
// Author(s):   Conan Reis
AString SkObjectID::as_code(const AString & name, SkClass * class_p, eVariant variant)
  {
  AString str(
    class_p->get_name_str_dbg(),
    ((variant != Variant_reference) ? 3u : 4u) + name.get_length());

  switch (variant)
    {
    case Variant_reference:
      str.append("@'", 2u);
      break;

    case Variant_possible_ref:
    str.append("@?'", 3u);
      break;

    case Variant_identifier:
      str.append("@#'", 3u);
      break;
    }

  str.append(name);
  str.append('\'');

  // $Revisit - CReis Could indicate whether the object is cached or not.

  return str;
  }

#endif // defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// This method is used to differentiate between different types of expressions when it
// is only known that an instance is of type SkookumScript/SkExpressionBase.
// 
// Returns:    SkExprType_identifier_local
// Modifiers:   virtual from SkExpressionBase
// Author(s):   Conan Reis
eSkExprType SkObjectID::get_type() const
  {
  return SkExprType_object_id;
  }

//---------------------------------------------------------------------------------------
// Evaluates Object ID expression and returns the object instance, nil or the name
// identifier as appropriate.
// 
// Returns:
//   true - indicating that the expression has completed its evaluation and that there is
//   a resulting instance.
//   
// Params:  
//   scope_p: scope for data/method/etc. look-ups
//   caller_p:
//     object that called/invoked this expression and that may await a result.  If it is
//     nullptr, then there is no object that needs to be returned to and notified when
//     this invocation is complete.
//   result_pp:
//     pointer to a pointer to store the instance resulting from the invocation of this
//     expression.  If it is nullptr, then the result does not need to be returned and
//     only side-effects are desired.
//     
// Notes:
//   No caller object is needed since SkObjectID always returns the result immediately.
//   
// Modifiers:  virtual (overriding pure from SkExpressionBase)
// See:        invoke_now()
// Author(s):   Conan Reis
SkInvokedBase * SkObjectID::invoke(
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p, // = nullptr
  SkInstance **   result_pp // = nullptr
  ) const
  {
  // Do the hook regardless of whether look-up is done
  SKDEBUG_HOOK_EXPR(this, scope_p, caller_p, nullptr, SkDebug::HookContext_current);

  // If no result is desired then don't bother doing any evaluation
  if (result_pp)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if it is a validated identifier name
    if (m_flags & Flag_identifier)
      {
      *result_pp = m_bind_name.new_instance();
      return nullptr;
      }

    // Only do look-up if not already cached
    SkInstance * obj_p = m_obj_p;

    // Since we are storing just a weak reference to the SkInstance (i.e. it never gets dereffed)
    // don't count m_obj_p as a reference!
    if (obj_p)
      {
      obj_p->reference();
      }
    else
      {
      obj_p   = m_class_p->object_id_lookup(m_bind_name, caller_p);
      m_obj_p = obj_p;
      }

    if (obj_p == nullptr)
      {
      // Object not found return nil as a result
      obj_p = SkBrain::ms_nil_p;

      // Give runtime assert if nil is not supposed to be a valid outcome
      // - i.e. @? is not used
      SK_ASSERT_INFO(
        m_flags & Flag_possible,
        a_str_format(
          "Unable to find named object using %s!\n"
          "It is either a name (or class) typo, the object isn't created yet, it was created but already removed from the system, or some other error.\n"
          "If not being present in the system is a valid possibility use @? which allows nil as a result.\n\n"
          "[Ignore returns nil as result.]",
          as_code().as_cstr()),
        *this);
      }

    *result_pp = obj_p;
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------

void SkObjectID::track_memory(AMemoryStats * mem_stats_p) const
  {
  mem_stats_p->track_memory(SKMEMORY_ARGS(SkObjectID, SkDebugInfo_size_used));
  }

