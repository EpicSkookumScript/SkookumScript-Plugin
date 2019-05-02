// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Object class with extra context abstract base class
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ARefCount.hpp>
#include <SkookumScript/SkClassDescBase.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// Object class descriptor that has an additional context/type info applied to it as a
// component or sub elements as well as a traditional primary class.
//
//   context-class = list-class | closure-class
//
// #Author(s) Conan Reis
class SkContextClassBase : public SkClassUnaryBase, public ARefCountMix<SkContextClassBase>
  {
  public:

  // Common Methods

    SK_NEW_OPERATORS(SkContextClassBase);

    explicit SkContextClassBase(SkClass * class_p)         : m_class_p(class_p) {}
    SkContextClassBase(const SkContextClassBase & tclass)  : m_class_p(tclass.m_class_p) {}

  // Methods

    // Overriding from SkClassUnaryBase, SkClassDescBase, ARefCountMix<>

      virtual void reference() const override;
      virtual void dereference() override;
      virtual void dereference_delay() const override;
      virtual void on_no_references() = 0;

      virtual SkTypedName *     get_data_type(const ASymbol & data_name, eSkScope * scope_p = nullptr, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const override;
      virtual SkClass *         get_key_class() const override;
      virtual const ASymbol &   get_key_class_name() const override;
      virtual SkMetaClass &     get_metaclass() const override;

      // Method Member Methods

        virtual void           append_method(SkMethodBase * method_p, bool * has_signature_changed_p = nullptr) override;
        virtual SkMethodBase * find_method(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual SkMethodBase * find_method_inherited(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual bool           is_method_inherited_valid(const ASymbol & method_name) const override;
        virtual bool           is_method_valid(const ASymbol & method_name) const override;

      // Method Member Methods

        virtual void              append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p = nullptr) override;
        virtual SkCoroutineBase * find_coroutine_inherited(const ASymbol & coroutine_name) const override;
        virtual bool              is_coroutine_valid(const ASymbol & coroutine_name) const override;
        virtual bool              is_coroutine_registered(const ASymbol & coroutine_name) const override;

      // Data Member Methods

        virtual SkTypedName *    append_data_member(const ASymbol & name, SkClassDescBase * type_p) override;
        virtual SkTypedNameRaw * append_data_member_raw(const ASymbol & name, SkClassDescBase * type_p, const AString & bind_name) override;

  protected:

  // Internal Methods

    SkContextClassBase() {}

  // Data Members

    // This is the primary class that is being wrapped around.  For example, given the
    // typed class List{String} the primary class would be List.
    SkClass * m_class_p;

  };  // SkContextClassBase


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkContextClassBase.inl>
#endif


