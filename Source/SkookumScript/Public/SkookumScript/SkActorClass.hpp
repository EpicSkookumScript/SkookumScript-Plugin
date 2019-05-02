// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for actor class descriptors and actor class objects
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkActor.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Some shorthand for sorted arrays
typedef APSortedLogical<SkActor, ASymbol>  tSkActors;


//---------------------------------------------------------------------------------------
// Notes      Class structure for all SkookumScript classes derived from and including
//            'Actor'.
// Subclasses 
// See Also   
// UsesLibs   AgogCore/AgogCore.lib
// InLibs     SkookumLib.lib
// Examples:    
// Author(s)  Conan Reis
class SK_API SkActorClass : public SkClass
  {
  // Accesses protected elements
  friend class SkParser;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkActorClass);

    explicit SkActorClass(const ASymbol & name = ASymbol::get_null(), SkClass * superclass_p = nullptr, uint32_t flags = Flag__default_actor, uint32_t annotation_flags = 0);
    virtual ~SkActorClass() override;

  // Methods

    virtual bool is_builtin_actor_class() const override;
    virtual void track_memory(AMemoryStats * mem_stats_p, bool skip_demand_loaded) const override;

    // Serialization Methods

      virtual SkInstance * object_id_lookup(const SkBindName & name, SkInvokedBase * caller_p = nullptr) const override;

      #if (SKOOKUM & SK_CODE_IN)
        //virtual SkClass * object_id_validate(SkObjectID * obj_id_p) const;
      #endif

      virtual bool demand_unload() override;

      //#if (SKOOKUM & SK_COMPILED_OUT)
      //  virtual void     as_binary(void ** binary_pp) const;
      //  virtual uint32_t as_binary_length() const;
      //#endif


      //#if (SKOOKUM & SK_COMPILED_IN)
      //  virtual void assign_binary(const void ** binary_pp, bool append_super_members = true);
      //#endif

    // Instance Methods

      void                 append_instance(const SkActor & actor);
      SkActor *            find_instance(const ASymbol & name) const    { return m_instances.get(name); }
      SkActor *            get_first_instance() const                   { return m_instances.get_first(); }
      const tSkActors &    get_instances() const                        { return m_instances; }
      void                 remove_instance(const SkActor & actor, eATerm term = ATerm_long);
      virtual SkInstance * new_instance() override;

  // Class Methods

  protected:

  // Data Members

    // Instances of this class (including derived class instances) for quick class based
    // look-ups.
    // $Revisit - CReis This might be better as something that doesn't need a large
    // contiguous chunk of memory.  Could also not store derived instances and only do a
    // sub-class look-up if needed - this would potentially slow things down.
    tSkActors m_instances;

  };  // SkActorClass


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Storage specialization - SkActorClass stored indirectly as pointer in SkUserData rather than whole structure
template<> inline SkActorClass * SkUserDataBase::as<SkActorClass>() const                { return *as_stored<SkActorClass*>(); }
template<> inline void           SkUserDataBase::set(SkActorClass const * const & value) { *as_stored<const SkActorClass*>() = value; }
template<> inline void           SkUserDataBase::set(SkActorClass       * const & value) { *as_stored<SkActorClass*>() = value; }

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkActorClass.inl>
#endif
