// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structures for class descriptors and class objects
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/Sk.hpp> // Always include Sk.hpp first (as some builds require a designated precompiled header)
#include <SkookumScript/SkClassDescBase.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumScript/SkClassDescBase.inl>
#endif

#include <AgogCore/ABinaryParse.hpp>
#include <AgogCore/AMath.hpp>
#include <AgogCore/AString.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkTypedClass.hpp>


//=======================================================================================
// SkClassDescBase Method Definitions
//=======================================================================================

bool SkClassDescBase::ms_compounds_use_ref = true;


//=======================================================================================
// SkClassDescBase Method Definitions
//=======================================================================================

// Converters from data structures to compiled binary code
#if (SKOOKUM & SK_COMPILED_OUT)

//---------------------------------------------------------------------------------------
// Fills memory pointed to by binary_pp with the information needed to create
//             a reference to this class descriptor and increments the memory address to
//             just past the last byte written.
// Arg         binary_pp - Pointer to address to fill and increment.  Its size *must* be
//             large enough to fit 5* bytes of binary data.
// See:        from_binary_ref_typed(), from_binary_ref(), as_binary_ref_typed_length()
// Notes:      Binary composition:
//               1 byte  - eSkClassType
//               n bytes - class id [currently all classes use 4 bytes]
//
// Modifiers:   virtual - overridden from SkClassDescBase
// Author(s):   Conan Reis
void SkClassDescBase::as_binary_ref_typed(void ** binary_pp) const
  {
  // 1 byte - eSkClassType
  **(uint8_t **)binary_pp = static_cast<uint8_t>(get_class_type());
  (*(uint8_t **)binary_pp)++;

  // n bytes - class id [currently all classes use 4 bytes]
  as_binary_ref(binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_OUT)


// Converters from compiled binary code to data structures
#if (SKOOKUM & SK_COMPILED_IN)

//---------------------------------------------------------------------------------------
// Creates and returns a class descriptor based on the binary which does not
//             include the class type info.
// Returns:    a class descriptor
// Arg         class_type - type of class stored in binary form
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_ref_typed() or a similar
//             mechanism.
// See:        as_binary_ref_typed(), as_binary_ref_typed_length(), from_binary_ref_typed()
// Notes:      Binary composition:
//               4 bytes - class name id
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkClassDescBase * SkClassDescBase::from_binary_ref(
  eSkClassType  class_type,
  const void ** binary_pp
  )
  {
  switch (class_type)
    {
    case SkClassType_class:
      return SkClass::from_binary_ref(binary_pp);

    case SkClassType_metaclass:
      return &SkClass::from_binary_ref(binary_pp)->get_metaclass();

    case SkClassType_typed_class:
      return SkTypedClass::from_binary_ref(binary_pp);

    case SkClassType_invokable_class:
      return SkInvokableClass::from_binary_ref(binary_pp);

    case SkClassType_class_union:
      return SkClassUnion::from_binary_ref(binary_pp);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Creates and returns an expression created dynamically based on the binary
//             which includes the expression's type info.
// Returns:    a dynamically allocated expression object
// Arg         binary_pp - Pointer to address to read binary serialization info from and
//             to increment - previously filled using as_binary_typed() or a similar
//             mechanism.
// See:        as_binary_typed(), as_binary_typed_length(), as_binary(), as_binary_length()
// Notes:      Binary composition:
//               1 byte  - eSkClassType
//               4 bytes - class name id
//
//             Little error checking is done on the binary info as it assumed that it
//             previously validated upon input.
// Modifiers:   static
// Author(s):   Conan Reis
SkClassDescBase * SkClassDescBase::from_binary_ref_typed(const void ** binary_pp)
  {
  // 1 byte - class type
  eSkClassType class_type = static_cast<eSkClassType>(A_BYTE_STREAM_UI8_INC(binary_pp));

  // 4 bytes - class reference binary
  return from_binary_ref(class_type, binary_pp);
  }

#endif // (SKOOKUM & SK_COMPILED_IN)


#if defined(SK_AS_STRINGS)

//---------------------------------------------------------------------------------------
// Get a description of the class and indicate whether it is using instance
//             or class scope.
// Modifiers:   virtual - override for custom behaviour.
// Author(s):   Conan Reis
AString SkClassDescBase::get_scope_desc() const
  {
  AString str("an instance of the class '");

  str.append(as_code());
  str.append('\'');

  return str;
  }

#endif  // defined(SK_AS_STRINGS)


//---------------------------------------------------------------------------------------
// Comparison function
// Returns:    AEquate_equal, AEquate_less, or AEquate_greater
// Arg         type - other class type to compare against
// Notes:      This method really only makes sense if the classes being compared are
//             ancestor/descendant or descendant/ancestor
// Author(s):   Conan Reis
eAEquate SkClassDescBase::compare(const SkClassDescBase & type) const
  {
  if (this == &type)
    {
    return AEquate_equal;
    }

  eSkClassType class_type = get_class_type();
  int          type_diff  = class_type - type.get_class_type();

  if (type_diff == 0)
    {
    // They are the same type of class
    switch (class_type)
      {
      case SkClassType_class:
        return static_cast<const SkClass *>(this)->compare_ids(*static_cast<const SkClass *>(&type));

      case SkClassType_typed_class:
        return static_cast<const SkTypedClass *>(this)->compare(*static_cast<const SkTypedClass *>(&type));

      case SkClassType_metaclass:
        return static_cast<const SkMetaClass *>(this)->compare(*static_cast<const SkMetaClass *>(&type));

      case SkClassType_invokable_class:
        return static_cast<const SkInvokableClass *>(this)->compare(*static_cast<const SkInvokableClass *>(&type));

      case SkClassType_class_union:
        return static_cast<const SkClassUnion *>(this)->compare(*static_cast<const SkClassUnion *>(&type));
          
      default:
        SK_ERRORX("Unknown class type!");
        break;
      }
    }

  // They are not the same type of class
  return eAEquate(a_sign(type_diff));
  }

//---------------------------------------------------------------------------------------

uint32_t SkClassDescBase::generate_crc32() const
  {
  eSkClassType class_type = get_class_type();

  uint32_t crc;
  switch (class_type)
    {
    case SkClassType_class:
      crc = static_cast<const SkClass *>(this)->get_name_id();
      break;

    case SkClassType_typed_class:
      crc = static_cast<const SkTypedClass *>(this)->generate_crc32();
      break;

    case SkClassType_metaclass:
      crc = static_cast<const SkMetaClass *>(this)->generate_crc32();
      break;

    case SkClassType_invokable_class:
      crc = static_cast<const SkInvokableClass *>(this)->generate_crc32();
      break;

    case SkClassType_class_union:
      crc = static_cast<const SkClassUnion *>(this)->generate_crc32();
      break;

    default: 
      SK_ERRORX("Unknown class type!");
      crc = 0;
      break;
    }

  return AChecksum::generate_crc32_uint8((uint8_t)class_type, crc);   
  }

//---------------------------------------------------------------------------------------
// Determines if method is registered - i.e. it exists and it is not a
//             placeholder.
// Returns:    true if method is registered
// Arg         method_name - name of method to check
// Author(s):   Conan Reis
bool SkClassDescBase::is_method_registered(const ASymbol & method_name, bool allow_placeholder) const
  {
  SkMethodBase * method_p = find_method(method_name);

  return method_p && (allow_placeholder || !method_p->is_placeholder());
  }

//---------------------------------------------------------------------------------------
// Qualifies/narrows the scope of this class descriptor
// Returns:    Qualified/narrowed scope of this class descriptor
// Arg         qual_scope_p - narrower (i.e. superclass) scope or nullptr if this class
//             descriptor should be used.
// See:        SkClass::is_scope_qualifier()
// Author(s):   Conan Reis
SkClassDescBase * SkClassDescBase::qualify(SkClass * qual_scope_p) const
  {
  return (qual_scope_p == nullptr)
    ? const_cast<SkClassDescBase *>(this)
    : (is_metaclass()
      ? (SkClassDescBase *)&qual_scope_p->get_metaclass()
      : qual_scope_p);
  }


//=======================================================================================
// SkClassUnaryBase Method Definitions
//=======================================================================================



