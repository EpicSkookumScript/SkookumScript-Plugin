// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Class descriptor for invokable/callable objects with parameters.
//             [$Revisit - CReis Should only be needed by parser.]
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APSorted.hpp>
#include <SkookumScript/SkContextClassBase.hpp>
#include <SkookumScript/SkParameters.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// SkookumScript Invokable Class - a class (like Closure) and additional info specifying
// parameters for an invoke/call.
//
// #Notes
//
//   closure-class  = ['_' | '|'] parameters
//   parameters     = parameter-list [ws class-desc]
//   parameter-list = '(' ws [send-params ws] [';' ws return-params ws] ')'
//
// #Author(s) Conan Reis
class SkInvokableClass : public SkContextClassBase
  {
  friend class SkBrain;      // Accesses protected elements
  friend class SkClass;      // Accesses protected elements

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkInvokableClass);

    SkInvokableClass(SkClass * class_p, SkParameters * params_p, eSkInvokeTime invoke_type)  : SkContextClassBase(class_p), m_params_p(params_p), m_invoke_type(invoke_type) {}
    SkInvokableClass(const SkInvokableClass & iclass)             : SkContextClassBase(iclass.m_class_p), m_params_p(iclass.m_params_p), m_invoke_type(iclass.m_invoke_type) {}
    ~SkInvokableClass()                                           { clear(); }

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_OUT)
      void              as_binary(void ** binary_pp) const;
      uint32_t          as_binary_length() const;
      virtual void      as_binary_ref(void ** binary_pp) const override;
      virtual uint32_t  as_binary_ref_typed_length() const override;
    #endif


    #if (SKOOKUM & SK_COMPILED_IN)
      SkInvokableClass(const void ** binary_pp)                   { assign_binary(binary_pp); }
      void assign_binary(const void ** binary_pp);

      static SkInvokableClass * from_binary_ref(const void ** binary_pp);
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Comparison Methods

    eAEquate compare(const SkInvokableClass & iclass) const;
    bool     operator==(const SkInvokableClass & iclass) const;
    bool     operator!=(const SkInvokableClass & iclass) const;
    bool     operator<(const SkInvokableClass & iclass) const;
    bool     operator<=(const SkInvokableClass & iclass) const;
    bool     operator>(const SkInvokableClass & iclass) const;
    bool     operator>=(const SkInvokableClass & iclass) const;
    uint32_t generate_crc32() const;

  // Methods

    void           clear();
    bool           is_immediate() const                           { return m_invoke_type == SkInvokeTime_immediate; }
    eSkInvokeTime  get_invoke_type() const                        { return m_invoke_type; }
    SkParameters * get_parameters() const                         { return m_params_p; }

    // Overriding from SkClassUnaryBase, SkClassDescBase, ARefCountMix<>

      virtual void on_no_references() override;

      virtual SkClassDescBase *  as_finalized_generic(const SkClassDescBase & scope_type) const override;
      virtual SkClassUnaryBase * find_common_type(const SkClassDescBase & cls) const override;
      virtual eSkClassType       get_class_type() const override;
      virtual bool               is_class_type(const SkClassDescBase * type_p) const override;
      virtual bool               is_generic() const override;
      void                       track_memory(AMemoryStats * mem_stats_p) const;

  // Class Methods

    static APSortedLogicalFree<SkInvokableClass> & get_classes()  { return ms_shared_classes; }

    static SkInvokableClass * get_or_create(const SkInvokableClass & iclass);
    static SkInvokableClass * get_or_create(SkClass * class_p, SkParameters * params_p, eSkInvokeTime invoke_type);

    static void shared_pre_empty();
    static void shared_empty()                                    { ms_shared_classes.free_all(); }  
    static bool shared_ensure_references();
    static void shared_track_memory(AMemoryStats * mem_stats_p);

  protected:

  // Internal Methods

    SkInvokableClass()                                            : m_params_p(nullptr) {}  // Called by SkBrain

  // Data Members

    // Parameters used to invoke the particular instance of the primary class
    ARefPtr<SkParameters> m_params_p;

    eSkInvokeTime m_invoke_type;

  // Class Data Members

    // Typed Class objects that are shared amongst various data-structures
    static APSortedLogicalFree<SkInvokableClass> ms_shared_classes;

  };  // SkInvokableClass


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkInvokableClass.inl>
#endif

