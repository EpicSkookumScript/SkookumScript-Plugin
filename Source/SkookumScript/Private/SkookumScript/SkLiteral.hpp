// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Literal class for standard literals (Boolean, Character, Integer, Real,
//             String & Symbol) and identifier for special objects (Class,  nil, this,
//             this_class & this_code)
//             List literal class for list objects fully defined in code
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkUserData.hpp>

//=======================================================================================
// Defines
//=======================================================================================

// If defined duplicate string literals share the same memory
//#define SKLITERAL_STRING_POOL

// If this is defined duplicate string literals are tracked for memory usage
//#define SKLITERAL_STRING_TRACK_DUPES


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
struct AStringRef;
class  SkMetaClass;
class  SkMethodCallBase;


//---------------------------------------------------------------------------------------
// Literal Class - wrapper around objects that are fully defined in-place and created
// explicitly in code without specifying standard instantiation and constructor call.  It
// includes objects for Boolean, Character, Integer, Real, String, Symbol and also as an
// identifier for Class, nil, this, this_class and this_code.
// 
// See:       SkLiteralList, SkIdentifierLocal
// Author(s): Conan Reis
class SkLiteral : public SkExpressionBase
  {
  public:

  // Nested Structures

    // Types of literals
    // Note, the values of these ids are used in the binary form of literals, so use caution
    // when modifying them.
    enum eType
      {
      Type_boolean,
      Type_integer,
      Type_real,
      Type_string,
      Type_symbol,

      // These should be more correctly categorized as identifiers, but making them a literal
      // allows a bit of initial optimization to occur.
      Type__class,       // A reference to a Class [AMetaClass]
      Type__nil,         // The single instance of the 'None' class - nil
      Type__this,        // The current owner instance which is at the topmost scope
      Type__this_class,  // The class of the current owner instance which is at the topmost scope
      Type__this_code,   // The invoked code instance (method or coroutine)
      Type__this_mind    // The updater mind for the current routine 
      };

    typedef SkUserData<1> UserData;

  // Common Methods

    SK_NEW_OPERATORS(SkLiteral);

    SkLiteral(eType kind, const void * data_p);
    explicit SkLiteral(eType kind)                 : m_kind(kind)              { memset(&m_data, 0, sizeof(m_data)); }
    explicit SkLiteral(tSkBoolean value)           : m_kind(Type_boolean)      { m_data.set(value); }
    explicit SkLiteral(tSkInteger value)           : m_kind(Type_integer)      { m_data.set(value); }
    explicit SkLiteral(tSkReal value)              : m_kind(Type_real)         { m_data.set(value); }
    explicit SkLiteral(const SkMetaClass & mclass) : m_kind(Type__class)       { m_data.set(&mclass); }
    explicit SkLiteral(const AString & str);
    explicit SkLiteral(const ASymbol & sym);
    virtual ~SkLiteral() override;

  // Converter Methods

    const AString * as_literal_string() const               { return m_data.as<AString>(); }
    const ASymbol * as_literal_symbol() const               { return m_data.as<ASymbol>(); }

    #if (SKOOKUM & SK_COMPILED_IN)
      SkLiteral(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override  { return this; }
      virtual eSkSideEffect            get_side_effect() const override                { return SkSideEffect_none; }
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Methods

    eType                   get_kind() const           { return m_kind; }
    virtual eSkExprType     get_type() const override;
    const UserData &        get_data() const           { return m_data; }
    virtual bool            is_nil() const override    { return (m_kind == Type__nil); }
    virtual bool            is_debug_class() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    void                    set(eType type, const UserData & data)  { m_kind = type; m_data = data; }
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  // Class Methods

    #ifdef SKLITERAL_STRING_POOL
      static AStringRef * get_pooled_str(const AString & str);
      static uint32_t     get_pooled_str_idx(const AString & str);
      static AStringRef * get_pooled_str_at(uint32_t idx);

      #ifdef SKLITERAL_STRING_TRACK_DUPES
        static void get_pooled_string_info(uint32_t * extra_bytes_p, uint32_t * extra_refs_p);
      #endif
    #endif

  protected:

  // Data Members

    // Used to store information for literal - differs for each type of literal.
    UserData m_data;

    // Kind of literal stored
    eType m_kind;
    // $Note - CReis This could have been a virtual method and then there would be a subclass for
    // each literal type, but this is simpler to implement.  [Perhaps it will be split up
    // in a future pass.]

  };  // SkLiteral


//---------------------------------------------------------------------------------------
// Notes      Literal for lists that are explicitly defined in-place in code.
// See Also   SkLiteral, SkIdentifierLocal
// Author(s)  Conan Reis
class SkLiteralList : public SkExpressionBase
  {
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkLiteralList);

    SkLiteralList(SkClass * list_type_p, SkMethodCallBase* ctor_p = nullptr) : m_list_type_p(list_type_p), m_ctor_p(ctor_p) {}
    virtual ~SkLiteralList() override;

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_IN)
      SkLiteralList(const void ** binary_pp);
    #endif


    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp) const override;
      virtual uint32_t as_binary_length() const override;
    #endif


    #if (SKOOKUM & SK_CODE_IN)
      virtual const SkExpressionBase * find_expr_last_no_side_effect() const override  { return this; }
      virtual eSkSideEffect            get_side_effect() const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Debugging Methods

    #if (SKOOKUM & SK_DEBUG)
      virtual SkExpressionBase * find_expr_by_pos(uint pos, eSkExprFind type = SkExprFind_all) const override;
      virtual eAIterateResult    iterate_expressions(SkApplyExpressionBase * apply_expr_p, const SkInvokableBase * invokable_p = nullptr) override;
    #endif


  // Methods

    virtual eSkExprType     get_type() const override;
    virtual SkInvokedBase * invoke(SkObjectBase * scope_p, SkInvokedBase * caller_p = nullptr, SkInstance ** result_pp = nullptr) const override;
    virtual void            track_memory(AMemoryStats * mem_stats_p) const override;

  protected:

  // Data Members

    // Most of the time any list literal will be of type List, but it *can* be subclassed
    // and have a custom constructor, etc.
    // $Note - CReis [Typeinfo] If item type is needed for lists at runtime, a SkTypedClass
    // will also need to be stored.
    ARefPtr<SkClass> m_list_type_p;

    // Constructor method call
    // If nullptr then call default constructor
    // $Revisit - CReis Should be offset or pointer rather than lookup
    SkMethodCallBase * m_ctor_p;

    // Item/Element expressions
    APCompactArray<SkExpressionBase> m_item_exprs;

  };  // SkLiteralList


//=======================================================================================
// Inline Methods
//=======================================================================================

