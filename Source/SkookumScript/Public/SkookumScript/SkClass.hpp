// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structures for class descriptors and class objects
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/APArray.hpp>
#include <SkookumScript/SkClassDescBase.hpp>
#include <SkookumScript/SkDataInstance.hpp>
#include <SkookumScript/SkMethod.hpp>
#include <SkookumScript/SkCoroutine.hpp>  // Needs: tSkCoroutineFunc and tSkCoroutineMthd


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
struct SkApplyExpressionBase; 
class  SkBrain;
class  SkClassDescBase;
class  SkInvokedMethod;


// Short-hand for arrays
typedef APSorted<SkClassUnaryBase, SkClassUnaryBase, SkClassUnaryBase> tSkSortedTypes;
typedef APSortedLogical<SkClass, ASymbol>            tSkClasses;
typedef APSortedLogical<SkMethodBase, ASymbol>       tSkMethodTable;
typedef APSortedLogical<SkCoroutineBase, ASymbol>    tSkCoroutines;
typedef APSortedLogical<SkTypedName, ASymbol>        tSkTypedNames;
typedef APArrayLogical<SkTypedName, ASymbol>         tSkTypedNameArray;
typedef APArrayLogical<SkTypedNameRaw, ASymbol>      tSkTypedNameRawArray;
//typedef APSortedLogical<SkTypedNameIndexed, ASymbol> tSkTypedNamesIndexed;
typedef APArrayLogical<SkInvokableBase, ASymbol>     tSkVTable; // Virtual method table

//---------------------------------------------------------------------------------------
// Record of a routine that was live updated
// Keeps the previous expression tree around so invoked expressions can be patched up
struct SkRoutineUpdateRecord
  {
  SK_NEW_OPERATORS(SkRoutineUpdateRecord);

  SkInvokableBase *     m_routine_p;          // Current version of routine, nullptr if it was deleted
  SkInvokableBase *     m_previous_routine_p; // If it was deleted, this is the old routine
  ARefPtr<SkParameters> m_previous_params_p;
  uint16_t              m_previous_invoked_data_array_size;
  uint32_t              m_previous_annotation_flags;
  SkExpressionBase *    m_previous_custom_expr_p;

  SkRoutineUpdateRecord() 
    : m_routine_p(nullptr)
    , m_previous_routine_p(nullptr)
    , m_previous_invoked_data_array_size(0)
    , m_previous_annotation_flags(0)
    , m_previous_custom_expr_p(nullptr) 
    {}

  ~SkRoutineUpdateRecord();

  };

//---------------------------------------------------------------------------------------
// Record of a class that was live updated
struct SkClassUpdateRecord : public ANamed
  {
  SK_NEW_OPERATORS(SkClassUpdateRecord);

  APArrayFree<SkRoutineUpdateRecord>  m_updated_routines;

  SkClassUpdateRecord(const ASymbol & class_name) : ANamed(class_name) {}
  ~SkClassUpdateRecord() {}
  };
                                                                
//---------------------------------------------------------------------------------------
// Record of a program that was live updated
struct SkProgramUpdateRecord
  {
  SK_NEW_OPERATORS(SkProgramUpdateRecord);

  APSortedLogicalFree<SkClassUpdateRecord, ASymbol>  m_updated_classes;
  
  SkClassUpdateRecord * get_or_create_class_update_record(const ASymbol & class_name);  
  };

//---------------------------------------------------------------------------------------
// Helper struct when finding inaccessible raw members
struct SkRawMemberRecord
  {
  SK_NEW_OPERATORS(SkRawMemberRecord);

  SkClass *         m_class_p;
  SkTypedNameRaw *  m_raw_member_p;
  
  SkRawMemberRecord(SkClass * class_p, SkTypedNameRaw * raw_member_p) : m_class_p(class_p), m_raw_member_p(raw_member_p) {}
  };

//---------------------------------------------------------------------------------------
// Used when binding a member to a class:
//   SkClass::register_coroutine_func(), SkClass::register_coroutine_mthd(),
//   SkClass::register_method_func(), SkClass::register_method_mthd()
//   
// Enum flags are preferred over bool since it is more descriptive at the point of call.
enum eSkBindFlag
  {
  // Descriptive enumerations - use when passing as a full argument:
  // [Listed before flag enumerations to ensure that they are used as descriptors in the
  // debugger when this enum type has the associated value.]
  
    // Instance member with rebinding disallowed.
    SkBindFlag_instance_no_rebind   = 0x0,  // [Placed as first enum with 0 to ensure that it appears as description in debugger when value is 0.]

    // Class member with rebinding disallowed.
    SkBindFlag_class_no_rebind      = 1 << 0,

    // Instance member with rebinding allowed
    SkBindFlag_instance_rebind      = 1 << 1,

    // Bind as a class member rather than an instance member
    SkBindFlag_class_rebind         = (1 << 0) | (1 << 1),

    // Default binding - instance member and disallow previously bound member to be rebound
    SkBindFlag_default              = SkBindFlag_instance_no_rebind,


  // Flag enumerations - use when dealing with the individual flags:

    // No binding flags set
    SkBindFlag__none          = 0x0,

    // Bind as a class member rather than an instance member
    SkBindFlag__class         = 1 << 0,

    // Bind as an instance member rather than a class member.  [Just used as hint since flag not set.]
    SkBindFlag__instance      = 0x0,

    // Allow previously bound member to be rebound
    SkBindFlag__rebind        = 1 << 1,

    // This method/coroutine will append a user data argument to the arg data list when invoked
    SkBindFlag__arg_user_data = 1 << 2,

    // Disallow previously bound member to be rebound - assert if attempt is made to bind
    // a member that has already been bound.  [Just used as hint since flag not set.]
    SkBindFlag__no_rebind     = 0x0,
  };

//---------------------------------------------------------------------------------------
// Function to resolve the m_raw_data_info member of all raw data members of this class
typedef bool (*tSkRawResolveFunc)(SkClass * class_p);

//---------------------------------------------------------------------------------------
// Function to get/set raw data of this class type
// If value_p is given, it acts as a setter and sets the given value and returns nullptr
// If value_p is nullptr, it acts as a getter and returns a new instance with the value of the member
typedef SkInstance * (*tSkRawAccessorFunc)(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);

//---------------------------------------------------------------------------------------
// Function to get a pointer to the raw data from an instance of this class type
typedef void * (*tSkRawPointerFunc)(SkInstance * obj_p);

//---------------------------------------------------------------------------------------
// Used to lookup named instance objects via object ID expressions
typedef SkInstance * (*tSkObjectIdLookupFunc)(const SkBindName & name);

//---------------------------------------------------------------------------------------
// SkookumScript MetaClass - i.e. a class treated as an instance using class scope (only
// class methods/data can be used).  It is a wrapper around a `SkClass` object so that a
// class can be treated as an first-order instance object.
//
//   ```
//   meta-class = '<' class-name '>'
//   ```
// $Revisit - CReis Might be good idea to have it be derived from `SkMind` instead of
// `SkInstanceUnreffed`. This would allow instances to use their class as a default
// updater mind rather than the single "master mind".
// 
// Author(s)  Conan Reis
class SK_API SkMetaClass : public SkClassUnaryBase, public SkInstanceUnreffed
  {
  friend class SkClass;       // Accesses protected elements
  friend class SkClassUnion;  // Accesses protected elements

  public:

  // Unhide Inherited Methods

    // Methods in this class with the same name as methods in its superclasses are 'hidden'
    // (even if they do not have the same parameters), this makes them visible again.
    // These using directives must precede the new methods in this class.
    using SkInstanceUnreffed::method_call;


  // Common Methods

    SK_NEW_OPERATORS(SkMetaClass);

    explicit SkMetaClass(SkClass * class_info_p = nullptr);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void as_binary_ref(void ** binary_pp) const override;
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
      virtual AString get_scope_desc() const override;
    #endif


  // Comparison Methods

    eAEquate compare(const SkMetaClass & mclass) const;
    uint32_t generate_crc32() const;

  // Methods

    SkClass * get_class_info() const  { return m_class_info_p; }

    // Overriding from SkClassUnaryBase & SkClassDescBase

      virtual SkClassDescBase *  as_finalized_generic(const SkClassDescBase & scope_type) const override;
      SkClassUnaryBase *         find_common_class(const SkClass & cls) const;
      virtual SkClassUnaryBase * find_common_type(const SkClassDescBase & cls) const override;
      virtual eSkClassType       get_class_type() const override;
      virtual SkTypedName *      get_data_type(const ASymbol & data_name, eSkScope * scope_p = nullptr, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const override;
      virtual SkClass *          get_key_class() const override;
      virtual const ASymbol &    get_key_class_name() const override;
      virtual SkMetaClass &      get_metaclass() const override;
      virtual bool               is_generic() const override;
      virtual bool               is_metaclass() const override; 
      virtual bool               is_class_type(const SkClassDescBase * type_p) const override;

      // Method Member Methods

        virtual void           append_method(SkMethodBase * method_p, bool * has_signature_changed_p = nullptr) override;
        virtual SkMethodBase * find_method(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual SkMethodBase * find_method_inherited(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual bool           is_method_inherited_valid(const ASymbol & method_name) const override;
        virtual bool           is_method_valid(const ASymbol & method_name) const override;

      // Coroutine Member Methods

        virtual void              append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p = nullptr) override;
        virtual SkCoroutineBase * find_coroutine_inherited(const ASymbol & coroutine_name) const override  { return nullptr; }
        virtual bool              is_coroutine_registered(const ASymbol & coroutine_name) const override  { return false; }
        virtual bool              is_coroutine_valid(const ASymbol & coroutine_name) const override       { return false; }

      // Data Member Methods

        virtual SkTypedName *    append_data_member(const ASymbol & name, SkClassDescBase * type_p) override;
        virtual SkTypedNameRaw * append_data_member_raw(const ASymbol & name, SkClassDescBase * type_p, const AString & bind_name) override;

    // Overriding from SkInstance

      virtual void method_call(const ASymbol & method_name, SkInstance ** args_pp, uint32_t arg_count, SkInstance ** result_pp = nullptr, SkInvokedBase * caller_p = nullptr) override;

      #if defined(SK_AS_STRINGS)
        virtual AString as_string_debug() const override;
      #endif

      virtual SkInstance *  get_data_by_name(const ASymbol & name) const override;
      virtual bool          set_data_by_name(const ASymbol & name, SkInstance * data_p) override;

    // Overriding from SkObjectBase

      virtual eSkObjectType get_obj_type() const override       { return SkObjectType_meta_class; }


  protected:

  // Data Members

    // The class that this instance represents/is wrapped around
    // $Revisit - CReis [Optimization] Consider using m_class_p from SkInstance instead since it will
    // always point to "Class" and not add any useful info.
    SkClass * m_class_info_p;

    // $Revisit - CReis [Optimization] m_ref_count and m_user_data inherited from SkInstance are not used.
    // A SkInstanceBase class could be created that does not have these data members.

  };  // SkMetaClass

//---------------------------------------------------------------------------------------
// Specialization - also ensures that `SkInvokedBase::get_arg_data<SkMetaClass>(--)` etc. work properly
// 
// See: SkInstance::as<SkClass>
template<> inline SkMetaClass * SkInstance::as_data<SkMetaClass>() const { return static_cast<SkMetaClass *>(const_cast<SkInstance *>(this)); }

//---------------------------------------------------------------------------------------
// Specialization - also ensures that `SkInvokedBase::get_arg_data<SkClass>(--)` etc. work properly
// 
// See: SkInstance::as<SkMetaClass>()
template<> inline SkClass * SkInstance::as_data<SkClass>() const  { return static_cast<const SkMetaClass *>(this)->get_class_info(); }


//---------------------------------------------------------------------------------------
// SkookumScript Class - i.e. an instance of a class using class instance scope (both
// instance methods/data and class methods/data can be used).
//
//   class = class-name
//
// Subclasses: SkActorClass
// Author(s):  Conan Reis
class SK_API SkClass : public SkClassUnaryBase, public ANamed
  {
  // Accesses protected elements
  friend class SkBrain;
  friend class SkClassUnion;
  friend class SkContextClassBase;
  friend class SkInvokableClass;
  friend class SkMetaClass;
  friend class SkParser;
  friend class SkTypedClass;
  friend class SkCompiler; // HACK while it exists!

  public:

  // Nested Structures

    // AEx<SkClass> exception id values
    enum
      {
      ErrId_nonexistent_method_regd = AErrId_last,  // No such method has been registered
      ErrId_duplicate_coroutine,                    // Coroutine with same name already registered
      ErrId_nonexistent_coroutine,                  // No such coroutine exists
      };

    enum eSubclass
      {
      Subclass_recurse,  // Apply recursively to subclasses
      Subclass_defer     // Do not apply to subclasses - defer it at a later time as a bulk operation
      };

    // Class flags - stored in m_flags
    enum eFlag
      {
      Flag_none             = 0,
      Flag_loaded           = 1 << 0,  // Class has all members loaded
      Flag_demand_load      = 1 << 1,  // Class only loads members when asked to do so
      Flag_demand_load_lock = 1 << 2,  // Once loaded do not allow it to be unloaded
      Flag_demand_unload    = 1 << 3,  // Deferred unload - unload when next possible to do so
      
      Flag_raw_resolved     = 1 << 4,  // Set when all raw members of this class have been resolved

      Flag_is_mind          = 1 << 5,  // For fast lookup if this class is derived from SkMind
      Flag_is_actor         = 1 << 6,  // For fast lookup if this class is derived from the (custom or built-in) actor class
      Flag_is_entity        = 1 << 7,  // For fast lookup if this class is derived from the (custom or built-in) actor class
      Flag_is_component     = 1 << 8,  // For fast lookup if this class is derived from a component (custom for each engine)

      // Object ID flags - look-up/validate for this class - i.e. Class@'name'

        // Validation flags - use masks below
        // These are inherited setting that propagate to all subclasses
          Flag_object_id_parse_any   = 1 << 10,
          Flag_object_id_parse_list  = 1 << 11,
          Flag_object_id_parse_defer = 1 << 12,

        // Object ID validation setting (masks):
          // Accept none during compile
          Flag__id_valid_none  = Flag_none,  // Inherit setting from superclass or no ObjectIDs
          // Accept any during compile
          Flag__id_valid_any   = Flag_object_id_parse_any,
          // Validate using list during compile (include list as compile dependency)
          Flag__id_valid_parse = Flag_object_id_parse_list,
          // Validate using list during compile if it exists (parse) - otherwise accept any during compile and validate using list in separate pass/run (defer)
          Flag__id_valid_exist = Flag_object_id_parse_any | Flag_object_id_parse_list,
          // Accept any during compile and validate using list in separate pass/run
          Flag__id_valid_defer = Flag_object_id_parse_any | Flag_object_id_parse_list | Flag_object_id_parse_defer,

          Flag__id_valid_mask = Flag_object_id_parse_any | Flag_object_id_parse_list | Flag_object_id_parse_defer,

      // Defaults and masks
        Flag__default         = Flag_none,
        Flag__default_actor   = Flag__default | Flag_is_actor,
        Flag__demand_loaded   = Flag_loaded | Flag_demand_load,
        Flag__mask_binary     = Flag_demand_load | Flag__id_valid_mask
      };

  // Public Types

    struct MethodInitializerFunc
      {
      const char *    m_method_name_p;
      tSkMethodFunc   m_atomic_f;
      };

    struct MethodInitializerFuncId
      {
      uint32_t        m_method_name_id;
      tSkMethodFunc   m_atomic_f;
      };
    
    struct MethodInitializerMthd
      {
      const char *    m_method_name_p;
      tSkMethodMthd   m_atomic_m;
      };

    struct MethodInitializerMthdId
      {
      uint32_t        m_method_name_id;
      tSkMethodMthd   m_atomic_m;
      };
    
    struct CoroutineInitializerFunc
      {
      const char *      m_coroutine_name_p;
      tSkCoroutineFunc  m_atomic_f;
      };

    struct CoroutineInitializerFuncId
      {
      uint32_t          m_coroutine_name_id;
      tSkCoroutineFunc  m_atomic_f;
      };

    struct CoroutineInitializerMthd
      {
      const char *      m_coroutine_name_p;
      tSkCoroutineMthd  m_atomic_m;
      };

    struct CoroutineInitializerMthdId
      {
      uint32_t          m_coroutine_name_id;
      tSkCoroutineMthd  m_atomic_m;
      };

    // Public Class Data

    #if (SKOOKUM & SK_DEBUG)
      // Reparse Member Info - placed here to ensure that it is
      // available for as many build configurations as possible.

      static struct ReparseInfo
        {
        bool m_is_active;

        tSkTypedNameArray     m_data;
        tSkTypedNameArray     m_class_data;
        tSkTypedNameRawArray  m_data_raw;

        APSorted<SkMethodBase, SkQualifier, SkQualifierCompareName>    m_methods;
        APSorted<SkMethodBase, SkQualifier, SkQualifierCompareName>    m_class_methods;
        APSorted<SkCoroutineBase, SkQualifier, SkQualifierCompareName> m_coroutines;

        ReparseInfo() : m_is_active(false) {}
        ~ReparseInfo() { free_all_compact(); }

        void begin();
        void end();
        void free_all();
        void free_all_compact();

        } ms_reparse_info;

    #endif

    // Initialization

    static void initialize();
    static void deinitialize();

  // Common Methods

    SK_NEW_OPERATORS(SkClass);

    explicit SkClass(const ASymbol & name, SkClass * superclass_p = nullptr, uint32_t flags = Flag__default, uint32_t annotation_flags = 0);
    ~SkClass();

    void  clear_meta_info();
    void  clear_members();
    void  clear_members_compact();

  // Converter Methods

    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif


  // Comparison Methods

    eAEquate compare(const SkClass & ssclass) const;      // Hierarchy sort
    eAEquate compare_ids(const SkClass & ssclass) const;  // Standard sort

  // Serialization Methods

    void         enable_demand_load(bool demand_load = true)            { if (demand_load) { m_flags |= Flag_demand_load; } else { m_flags &= ~Flag_demand_load; } }
    virtual bool demand_unload();
    SkClass *    get_demand_loaded_root() const;
    bool         is_demand_loaded() const                               { return (m_flags & Flag_demand_load) || (m_superclass_p && m_superclass_p->is_demand_loaded()); }
    bool         is_demand_loaded_root() const                          { return (m_flags & Flag_demand_load) != 0; }
    bool         is_loaded() const                                      { return (m_flags & Flag_loaded) != 0u; }
    bool         is_load_locked() const                                 { return (m_flags & Flag_demand_load_lock) != 0u; }
    bool         is_unload_deferred() const                             { return (m_flags & Flag_demand_unload) != 0u; }
    void         lock_load(bool lock = true)                            { if (lock) { m_flags |= Flag_demand_load_lock; } else { m_flags &= ~Flag_demand_load_lock; } }
    void         set_flag_load(bool loaded = true);
    void         set_loaded()                                           { set_flag_load(); }

    static void          register_raw_resolve_func(tSkRawResolveFunc raw_resolve_f);
    void                 register_raw_accessor_func(tSkRawAccessorFunc raw_member_accessor_f);
    void                 register_raw_pointer_func(tSkRawPointerFunc raw_pointer_func_f);
    tSkRawAccessorFunc   get_raw_accessor_func() const  { return m_raw_member_accessor_f; }
    tSkRawAccessorFunc   get_raw_accessor_func_inherited() const;
    tSkRawPointerFunc    get_raw_pointer_func() const   { return m_raw_pointer_f; }
    tSkRawPointerFunc    get_raw_pointer_func_inherited() const;
    SkInstance *         new_instance_from_raw_data(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p) const;
    void                 assign_raw_data(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p) const;
    void *               get_raw_pointer(SkInstance * obj_p) const;

    #if (SKOOKUM & SK_COMPILED_OUT)
      virtual void     as_binary(void ** binary_pp, bool include_routines = true) const;
      virtual uint32_t as_binary_length(bool include_routines = true) const;
      void             as_binary_recurse(void ** binary_pp, bool skip_demand_loaded) const;
      uint32_t         as_binary_recurse_length(bool skip_demand_loaded) const;
      void             as_binary_group(void ** binary_pp, bool skip_demand_loaded) const;
      uint32_t         as_binary_group_length(bool skip_demand_loaded) const;
      void             as_binary_placeholder_recurse(void ** binary_pp) const;
      uint32_t         as_binary_placeholder_recurse_length();
      virtual void     as_binary_ref(void ** binary_pp) const override;
    #endif


    #if (SKOOKUM & SK_COMPILED_IN)
      void             assign_binary(const void ** binary_pp, bool include_routines = true);
      static SkClass * from_binary_ref(const void ** binary_pp);
      static void      from_binary_group(const void ** binary_pp);
      void             append_instance_method(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p = nullptr);
      void             append_class_method(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p = nullptr);
      void             append_coroutine(const void ** binary_pp, SkRoutineUpdateRecord * update_record_p = nullptr);
    #endif

    #if (SKOOKUM & SK_DEBUG)
      void        ensure_loaded_debug();
      void        reparse_begin(bool include_routines);
      void        reparse_end();

      uint32_t        count_expressions_recurse(eAHierarchy hierarchy = AHierarchy__all);
      eAIterateResult iterate_expressions(SkApplyExpressionBase * apply_expr_p);
      eAIterateResult iterate_expressions_recurse(SkApplyExpressionBase * apply_expr_p, eAHierarchy hierarchy = AHierarchy__all);
    #endif

  // Methods

    virtual SkMetaClass & get_metaclass() const override;

    uint32_t     get_flags() const                                      { return m_flags; }
    uint32_t     get_annotation_flags() const                           { return m_annotation_flags; }
    void         set_annotation_flags(uint32_t flags)                   { m_annotation_flags = flags; }

    const SkBindName & get_bind_name() const                            { return m_bind_name; }
    void               set_bind_name(const AString & bind_name)         { m_bind_name = bind_name; }

    virtual void track_memory(AMemoryStats * mem_stats_p, bool skip_demand_loaded) const;
    virtual void track_memory(AMemoryStats * mem_stats_p) const         { track_memory(mem_stats_p, true); }
    void         track_memory_recursive(AMemoryStats * mem_stats_p, bool skip_demand_loaded) const;


    // Validated Object ID methods

      bool                 is_object_id_lookup() const;
      void                 set_object_id_lookup_func(tSkObjectIdLookupFunc object_id_lookup_f);
      virtual SkInstance * object_id_lookup(const SkBindName & name, SkInvokedBase * caller_p = nullptr) const;
      void                 object_id_lookup_clear_cache()                 { m_object_id_lookup_p = nullptr; }

      #if (SKOOKUM & SK_CODE_IN)
        void              clear_object_id_valid_list();
        uint32_t          get_object_id_validate() const;
        ASymbolTable *    get_object_id_valid_list() const                { return m_object_ids_p; }
        ASymbolTable *    get_object_id_valid_list_merge();
        void              set_object_id_validate(uint32_t validate_mask = Flag__id_valid_any);
        virtual SkClass * object_id_validate(const SkBindName & name, bool validate_deferred = true) const;

        #if (SKOOKUM & SK_DEBUG)
          uint32_t        object_id_validate_recurse(bool validate_deferred, uint32_t * obj_id_count_p = nullptr);
        #endif
      #endif


    // Hierarchy Methods

      void                       append_subclass(SkClass * subclass_p);
      void                       build_vtables_recurse(bool force_new);
      SkClass *                  find_common_class(const SkClass & cls) const;
      virtual SkClassUnaryBase * find_common_type(const SkClassDescBase & cls) const override;
      uint32_t                   get_class_recurse_count(bool skip_demand_loaded) const;
      tSkClasses &               get_subclasses()                       { return m_subclasses; }
      const tSkClasses &         get_subclasses() const                 { return m_subclasses; }
      void                       get_subclasses_all(tSkClasses * classes_p) const;
      SkClass *                  get_superclass() const                 { return m_superclass_p; }
      void                       get_superclasses_all(tSkClasses * classes_p) const;
      uint32_t                   get_superclass_depth() const;
      SkClass *                  get_class_depth_at(uint32_t depth) const;
      AString                    get_class_path_str(int32_t scripts_path_depth) const;
      bool                       is_actor_class() const                 { return (m_flags & Flag_is_actor) != 0; }
      bool                       is_mind_class() const                  { return (m_flags & Flag_is_mind) != 0; }
      bool                       is_entity_class() const                { return (m_flags & Flag_is_entity) != 0; }
      bool                       is_component_class() const             { return (m_flags & Flag_is_component) != 0; }
      bool                       is_class(const SkClass & cls) const;
      bool                       is_class(uint32_t name_id) const;
      bool                       is_subclass(const SkClass & superclass) const;
      bool                       is_superclass(const SkClass & subclass) const;
      bool                       is_scope_qualifier(SkClassDescBase * recv_type_p) const;
      bool                       is_deleted() const;
      eAIterateResult            iterate_recurse(AFunctionArgRtnBase<SkClass *, eAIterateResult> * apply_class_p, eAHierarchy hierarchy = AHierarchy__all);
      void                       iterate_recurse(AFunctionArgBase<SkClass *> * apply_class_p, eAHierarchy hierarchy = AHierarchy__all);
      SkClass *                  next_class(SkClass * root_p) const;
      SkClass *                  next_sibling() const;
      void                       remove_subclass(SkClass * subclass_p);


    // Method Methods

      // Instance & Class Methods

        virtual void           append_method(SkMethodBase * method_p, bool * has_signature_changed_p = nullptr) override;
        virtual SkMethodBase * find_method(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual SkMethodBase * find_method_inherited(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        SkMethodBase *         find_method_inherited_receiver(const ASymbol & method_name, SkInstance ** receiver_pp, SkInvokedBase * caller_p) const;
        SkInvokableBase *      get_invokable_from_vtable(eSkScope scope, int16_t vtable_index) const;
        SkInvokableBase *      get_invokable_from_vtable_i(int16_t vtable_index) const;
        SkInvokableBase *      get_invokable_from_vtable_c(int16_t vtable_index) const;
        int16_t                find_invokable_in_vtable_i(const ASymbol & name) const;
        int16_t                find_invokable_in_vtable_c(const ASymbol & name) const;
        virtual bool           is_method_valid(const ASymbol & method_name) const override;
        virtual bool           is_method_inherited_valid(const ASymbol & method_name) const override;
        void                   register_method_func(const ASymbol & method_name, tSkMethodFunc atomic_f, eSkBindFlag flags = SkBindFlag_default);
        void                   register_method_func(const char * method_name_p, tSkMethodFunc atomic_f, eSkBindFlag flags = SkBindFlag_default);
        void                   register_method_mthd(const ASymbol & method_name, tSkMethodMthd atomic_m, eSkBindFlag flags = SkBindFlag_default);
        void                   register_method_mthd(const char * method_name_p, tSkMethodMthd atomic_m, eSkBindFlag flags = SkBindFlag_default);
        void                   register_method_func_bulk(const MethodInitializerFunc   * bindings_p, uint32_t count, eSkBindFlag flags);
        void                   register_method_func_bulk(const MethodInitializerFuncId * bindings_p, uint32_t count, eSkBindFlag flags);
        void                   register_method_mthd_bulk(const MethodInitializerMthd   * bindings_p, uint32_t count, eSkBindFlag flags);
        void                   register_method_mthd_bulk(const MethodInitializerMthdId * bindings_p, uint32_t count, eSkBindFlag flags);

      // Instance Methods

        void                   append_instance_method(SkMethodBase * method_p, bool * has_signature_changed_p = nullptr);
        SkMethodBase *         get_instance_destructor_inherited() const                     { return m_destructor_p; }
        SkMethodBase *         find_instance_method(const ASymbol & method_name) const       { return m_methods.get(method_name); }
        SkMethodBase *         find_instance_method_inherited(const ASymbol & method_name) const;
        SkMethodBase *         find_instance_method_overridden(const ASymbol & method_name) const;
        SkMethodBase *         find_instance_method_scoped_inherited(const SkQualifier & method_qual) const;
        bool                   remove_instance_method(const ASymbol & method_name)           { return m_methods.free(method_name); }
        bool                   unlink_instance_method(const ASymbol & method_name)           { return m_methods.remove(method_name); }
        const tSkMethodTable & get_instance_methods() const                                  { return m_methods; }
        bool                   is_instance_method_valid(const ASymbol & method_name) const   { return (m_methods.get(method_name) != nullptr); }
        virtual SkInstance *   new_instance();
        SkInstance *           new_mind_instance();

      // Class Methods

        void                   append_class_method(SkMethodBase * method_p, bool * has_signature_changed_p = nullptr);
        SkMethodBase *         find_class_method(const ASymbol & method_name) const                 { return m_class_methods.get(method_name); }
        SkMethodBase *         find_class_method_inherited(const ASymbol & method_name, bool * is_class_member_p = nullptr) const;
        SkMethodBase *         find_class_method_overridden(const ASymbol & method_name) const;
        bool                   remove_class_method(const ASymbol & method_name)                     { return m_class_methods.free(method_name); }
        bool                   unlink_class_method(const ASymbol & method_name)                     { return m_class_methods.remove(method_name); }
        const tSkMethodTable & get_class_methods() const                                            { return m_class_methods; }
        void                   invoke_class_ctor();
        void                   invoke_class_ctor_recurse();
        void                   invoke_class_dtor_recurse();
        bool                   is_class_method_valid(const ASymbol & method_name) const             { return (m_class_methods.get(method_name) != nullptr); }
        bool                   is_class_method_inherited_valid(const ASymbol & method_name) const   { return (find_class_method_inherited(method_name) != nullptr); }

    // Coroutine Methods

      virtual void              append_coroutine(SkCoroutineBase * coroutine_p, bool * has_signature_changed_p = nullptr) override;
      SkCoroutineBase *         find_coroutine(const ASymbol & coroutine_name) const       { return m_coroutines.get(coroutine_name); }
      virtual SkCoroutineBase * find_coroutine_inherited(const ASymbol & coroutine_name) const override;
      SkCoroutineBase *         find_coroutine_overridden(const ASymbol & coroutine_name) const;
      SkCoroutineBase *         find_coroutine_scoped_inherited(const SkQualifier & coroutine_qual) const;
      virtual bool              is_coroutine_registered(const ASymbol & coroutine_name) const override;
      virtual bool              is_coroutine_valid(const ASymbol & coroutine_name) const override  { return m_coroutines.find(coroutine_name); }
      void                      register_coroutine_func(const ASymbol & coroutine_name, tSkCoroutineFunc update_f, eSkBindFlag flags = SkBindFlag_default);
      void                      register_coroutine_func(const char * coroutine_name_p, tSkCoroutineFunc update_f, eSkBindFlag flags = SkBindFlag_default);
      void                      register_coroutine_mthd(const ASymbol & coroutine_name, tSkCoroutineMthd update_m, eSkBindFlag flags = SkBindFlag_default);
      void                      register_coroutine_mthd(const char * coroutine_name_p, tSkCoroutineMthd update_m, eSkBindFlag flags = SkBindFlag_default);
      void                      register_coroutine_func_bulk(const CoroutineInitializerFunc   * bindings_p, uint32_t count, eSkBindFlag flags);
      void                      register_coroutine_func_bulk(const CoroutineInitializerFuncId * bindings_p, uint32_t count, eSkBindFlag flags);
      void                      register_coroutine_mthd_bulk(const CoroutineInitializerMthd   * bindings_p, uint32_t count, eSkBindFlag flags);
      void                      register_coroutine_mthd_bulk(const CoroutineInitializerMthdId * bindings_p, uint32_t count, eSkBindFlag flags);
      bool                      remove_coroutine(const ASymbol & coroutine_name)          { return m_coroutines.free(coroutine_name); }
      bool                      unlink_coroutine(const ASymbol & coroutine_name)          { return m_coroutines.remove(coroutine_name); }
      const tSkCoroutines &     get_coroutines() const                                   { return m_coroutines; }

    // Data Methods

      virtual SkTypedName *     append_data_member(const ASymbol & name, SkClassDescBase * type_p) override;
      virtual SkTypedNameRaw *  append_data_member_raw(const ASymbol & name, SkClassDescBase * type_p, const AString & bind_name) override;
      virtual SkTypedName *     get_data_type(const ASymbol & data_name, eSkScope * scope_p = nullptr, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const override;
      SkTypedName *             get_instance_data_type(uint32_t data_idx, SkClass ** data_owner_class_pp = nullptr) const;
      SkTypedName *             get_instance_data_type(const ASymbol & data_name, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const;
      SkTypedNameRaw *          get_instance_data_type_raw(const ASymbol & data_name, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const;
      SkTypedName *             get_class_data_type(const ASymbol & data_name, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const;
      uint32_t                  get_inherited_instance_data_count() const;
      bool                      is_raw_data_resolved() const           { return (m_flags & Flag_raw_resolved) != 0; }
      void                      set_raw_data_resolved(bool is_resolved);
      bool                      resolve_raw_data(const ASymbol & name, tSkRawDataInfo raw_data_info);
      bool                      resolve_raw_data(const char * name_p, tSkRawDataInfo raw_data_info);
      void                      resolve_raw_data();
      void                      resolve_raw_data_recurse();

      // Instance Methods

        SkTypedName *                 append_instance_data(const ASymbol & name, SkClassDescBase * type_p, bool increment_total_data_count);
        const tSkTypedNameArray &     get_instance_data() const        { return m_data; }
        uint32_t                      get_total_data_count() const     { return m_total_data_count; }

        SkTypedNameRaw *              append_instance_data_raw(const ASymbol & name, SkClassDescBase * type_p, const AString & bind_name);
        const tSkTypedNameRawArray &  get_instance_data_raw() const { return m_data_raw; }
        tSkTypedNameRawArray &        get_instance_data_raw_for_resolving() { return m_data_raw; }
        uint32_t                      compute_total_raw_data_count() const;

        void                          remove_instance_data_all();
        uint32_t                      generate_crc32_instance_data() const;

        #if (SKOOKUM & SK_COMPILED_IN)
          SkClass * find_instance_data_scope(const ASymbol & name, ePath check_path = Path_super_sub);
        #endif

      // Class Methods

        SkTypedName *             append_class_data(const ASymbol & name, SkClassDescBase * type_p, bool increment_total_data_count);
        void                      fill_class_data_names(APSortedLogical<ASymbol> * target_p);
        const tSkTypedNameArray & get_class_data() const             { return m_class_data; }
        uint32_t                  get_total_class_data_count() const { return m_total_class_data_count; }
        void                      remove_class_data_all();
        uint32_t                  generate_crc32_class_data() const;

        const SkInstanceList &    get_class_data_values() const                                             { return m_class_data_values; }
        SkInstance *              get_class_data_value_by_idx(uint32_t data_idx) const                      { return m_class_data_values[data_idx]; }
        void                      set_class_data_value_by_idx(uint32_t data_idx, SkInstance * obj_p)        { m_class_data_values.set_at(data_idx, *obj_p); }
        void                      set_class_data_value_by_idx_no_ref(uint32_t data_idx, SkInstance * obj_p) { m_class_data_values.set_at_no_ref(data_idx, *obj_p); }

        #if (SKOOKUM & SK_COMPILED_IN)
          SkClass * find_class_data_scope(const ASymbol & name, ePath check_path = Path_super_sub);
        #endif

    // Overriding from SkClassDescBase

      virtual SkClassDescBase * as_finalized_generic(const SkClassDescBase & scope_type) const override;
      virtual eSkClassType      get_class_type() const override;
      virtual SkClassDescBase * get_item_type() const override;
      virtual const ASymbol &   get_key_class_name() const override;
      virtual bool              is_class_type(const SkClassDescBase * type_p) const override;
      virtual bool              is_generic() const override;
      virtual bool              is_method_registered(const ASymbol & method_name, bool allow_placeholder) const override;

    // Overriding from SkClassUnaryBase

      virtual SkClass * get_key_class() const override;

    // User data

      template<typename T> const T &  get_user_data() const               { return reinterpret_cast<const T &>(m_user_data); }
      template<typename T> void       set_user_data(const T & user_data)  { static_assert(sizeof(T) <= sizeof(m_user_data), "User data does not fit."); reinterpret_cast<SkPOD32<T> &>(m_user_data) = reinterpret_cast<const SkPOD32<T> &>(user_data); }
      uint32_t                        get_user_data_int() const           { return m_user_data_int; }
      void                            set_user_data_int(uint32_t value)   { m_user_data_int = value; }
      void                            set_user_data_int_recursively(uint32_t value);

    // Sanity checking

    #if (SKOOKUM & SK_DEBUG)
      virtual void reference() const override          { ++m_ref_count; }
      virtual void dereference() override              { --m_ref_count; }    
      virtual void dereference_delay() const override  { --m_ref_count; }    
    #endif

  protected:

  // Internal Methods

    void         recurse_modify_total_data_count(int32_t delta);
    void         recurse_modify_total_class_data_count(int32_t delta);
    void         recurse_class_ctor();
    void         recurse_class_dtor();
    void         recurse_replace_vtable_entry_i(int16_t vtable_index, SkInvokableBase * old_entry_p, SkInvokableBase * new_entry_p);
    void         recurse_replace_vtable_entry_c(int16_t vtable_index, SkInvokableBase * old_entry_p, SkInvokableBase * new_entry_p);
    void         demand_unload_recurse();
    void         set_destructor(SkMethodBase * destructor_p);
    void         build_vtables(bool force_new);

  // Internal Class Methods

    // SkookumScript Atomic Methods

      #if (SKOOKUM & SK_DEBUG)
        virtual void find_unregistered_atomics(APArray<SkInvokableBase> * atomics_p);
        virtual void find_inaccessible_raw_members(APArray<SkRawMemberRecord> * raw_members_p);
      #endif


  // Data Members

    // Class Flags - see eFlags
    uint32_t m_flags;

    // Class annotations
    uint32_t m_annotation_flags;

    // Parent of this class [Single inheritance only - at least in first pass]
    SkClass * m_superclass_p;

    // Child classes of this class
    tSkClasses m_subclasses;

    // Name used for associating this class with engine classes
    SkBindName m_bind_name;

    // Function to resolve the raw data info of all raw data members of a given class
    static tSkRawResolveFunc ms_raw_resolve_f;

    // Function to set/get raw data members of this class type
    tSkRawAccessorFunc m_raw_member_accessor_f;

    // Function to get raw data pointer from an instance of this class type
    tSkRawPointerFunc m_raw_pointer_f;

    // Object ID Info

      // Used to lookup named instance objects via object ID expressions. If set to nullptr
      // use Sk class method object_id_find(NameType name) <ThisClass_|None> instead.
      // Used in object_id_lookup()
      tSkObjectIdLookupFunc m_object_id_lookup_f;

      // Cached Sk class method object_id_find(NameType name) <ThisClass_|None> or nullptr.
      // Used in object_id_lookup()
      mutable SkMethodBase * m_object_id_lookup_p;

      // Used to validate object IDs for this class.
      // $Revisit - CReis Since there are few classes that use object IDs it might be an idea
      // to store only the symbol tables for classes that actually use them rather than
      // taking up 4/8 bytes for every class whether they use object IDs or not.
      ASymbolTable * m_object_ids_p;

    // Instance Member Info

      // Data Members (name and class type) for instances of this class - added via this
      // class directly.  Each data member name must be unique - no superclass nor subclass
      // may have it and there should be no class data member with the same name either
      // - this is enforced by the compiler.
      // m_data_table references this and inherited members for constructing instances of
      // this class.
      tSkTypedNameArray m_data;

      // Data Members (name, class type and raw storage information) for engine-native instances of this class
      // Same naming rules apply as for m_data
      // These are created by the parser and then at load time initialized with the proper storage info by the engine
      tSkTypedNameRawArray m_data_raw;

      // Shortcut of total count of all m_data members of this class and superclasses combined
      uint32_t m_total_data_count;

      // Methods of this class - added via this class directly.
      tSkMethodTable m_methods;

      // Shortcut to instance destructor
      SkMethodBase * m_destructor_p;

      // List of available coroutines
      tSkCoroutines m_coroutines;

     // Class Member Info

      // Class Data Members (name, class type and data) for this class - added via this
      // class directly.  Each class data member name must be unique - no superclass nor
      // subclass/ may have it and there should be no instance data member with the same
      // name either - this is enforced by the compiler.
      tSkTypedNameArray m_class_data;

      // Where the runtime values of the class data are actually stored
      SkInstanceList m_class_data_values;

      // Shortcut of total count of all m_class_data members of this class and superclasses combined
      uint32_t m_total_class_data_count;

      // Class Methods of this class - added via this class directly.
      tSkMethodTable m_class_methods;

    // "Virtual function table"

      // Instance methods and coroutines of this class 
      // including inherited ones from superclasses for quick look-ups
      tSkVTable m_vtable_i;
      // Class methods of this class
      tSkVTable m_vtable_c;

    // Wrappers - for quick access and no need to allocate memory

      // Metaclass wrapper for this class
      SkMetaClass m_metaclass;

    // User Data

      uint64_t m_user_data;
      uint32_t m_user_data_int;

    // Sanity checking

    #if (SKOOKUM & SK_DEBUG)
      mutable uint32_t m_ref_count;
    #endif

  };  // SkClass


//---------------------------------------------------------------------------------------
// Notes      SkookumScript Union between 2 or more classes - i.e. instance classes and
//            meta classes (SkClassUnaryBase -> SkClass & SkMetaClass).  Used as a type
//            descriptor to denote an instance that could be one of several different
//            types.  The class type "None" (nil) in particular is often paired with
//            other types.
//
//              class-union    = '<' class-unary {'|' class-unary}1+ '>'
//              class-unary    = class-instance | meta-class
//              class-instance = class | list-class
//              class          = class-name
//              list-class     = List ['{' ws [class-desc ws] '}']
//              meta-class     = '<' class-name '>'
//
// ToDo
//   - merge superclasses with subclasses (and same classes)
//   - disallow "unions" that merge down to one class - use MetaClass instead
//   - sort list of unioned classes alphabetically
//   - have global sorted list of already created unions
//   - derive from ARefCount and have all references to a SkDescBase increment and decrement
//     - potentially make a SkClassDescRef that does this automatically
//
// Author(s)  Conan Reis
class SK_API SkClassUnion : public SkClassDescBase, public ARefCountMix<SkClassUnion>
  {
  // Accesses protected elements
  friend class SkBrain;       
  friend class SkClass;
  friend class SkTypedClass;
  friend class SkInvokableClass;
  friend class SkMetaClass;

  public:

  // Common Methods

    SK_NEW_OPERATORS(SkClassUnion);

    SkClassUnion()                                                : m_common_class_p(nullptr) {}
    explicit SkClassUnion(const SkClassUnaryBase & initial_class) : m_common_class_p(const_cast<SkClassUnaryBase *>(&initial_class)) { initial_class.reference(); initial_class.reference(); m_union.append(initial_class); }
    explicit SkClassUnion(const SkClassDescBase & initial_class)  : m_common_class_p(nullptr) { merge_class(initial_class); }
    SkClassUnion(const SkClassUnion & class_union);
    virtual ~SkClassUnion() override;
    
    SkClassUnion & operator=(const SkClassUnion & class_union);

  // Converter Methods

    #if (SKOOKUM & SK_COMPILED_OUT)
      void         as_binary(void ** binary_pp) const;
      uint32_t     as_binary_length() const;
      virtual void as_binary_ref(void ** binary_pp) const override;
      virtual uint32_t as_binary_ref_typed_length() const override;
    #endif


    #if (SKOOKUM & SK_COMPILED_IN)
      SkClassUnion(const void ** binary_pp)                       : m_common_class_p(nullptr) { assign_binary(binary_pp); }
      void                  assign_binary(const void ** binary_pp);
      static SkClassUnion * from_binary_ref(const void ** binary_pp);
    #endif


    #if defined(SK_AS_STRINGS)
      virtual AString as_code() const override;
    #endif

  // Comparison Methods

    eAEquate compare(const SkClassUnion & class_union) const;
    bool     operator==(const SkClassUnion & class_union) const  { return compare(class_union) == AEquate_equal; }
    bool     operator<(const SkClassUnion & class_union) const   { return compare(class_union) == AEquate_less; }
    bool     operator>(const SkClassUnion & class_union) const   { return compare(class_union) == AEquate_greater; }
    bool     operator!=(const SkClassUnion & class_union) const  { return compare(class_union) != AEquate_equal; }
    bool     operator<=(const SkClassUnion & class_union) const  { return compare(class_union) != AEquate_greater; }
    bool     operator>=(const SkClassUnion & class_union) const  { return compare(class_union) != AEquate_less; }
    uint32_t generate_crc32() const;

  // Methods

    // Type-checking Methods

      virtual const SkClassUnaryBase * as_unary_class() const override         { return m_common_class_p; }
      virtual SkClassUnaryBase *       as_unary_class() override               { return m_common_class_p; }
      SkClassUnaryBase *               get_common_class() const       { return m_common_class_p; }
      void                             set_common_class(SkClassUnaryBase * class_p);
      virtual bool                     is_builtin_actor_class() const override;
      bool                             is_class_maybe(const SkClassDescBase * type_p) const;
      bool                             is_valid_param_for(const SkClassDescBase * arg_type_p) const;
      virtual bool                     is_metaclass() const override; 
      bool                             is_trivial() const             { return (m_union.get_length() <= 1u); }
      void                             merge_class(const SkClassUnaryBase & new_class);
      void                             merge_class(const SkClassDescBase & new_class);

    // Overriding from SkClassUnaryBase, SkClassDescBase, ARefCountMix<>

      virtual void reference() const override;
      virtual void dereference() override;
      virtual void dereference_delay() const override;
      void         on_no_references();

      virtual SkClassDescBase *  as_finalized_generic(const SkClassDescBase & scope_type) const override;
      virtual SkClassUnaryBase * find_common_type(const SkClassDescBase & cls) const override;
      virtual eSkClassType       get_class_type() const override;
      virtual SkTypedName *      get_data_type(const ASymbol & data_name, eSkScope * scope_p = nullptr, uint32_t * data_idx_p = nullptr, SkClass ** data_owner_class_pp = nullptr) const override;
      virtual SkClassDescBase *  get_item_type() const override;
      virtual SkClass *          get_key_class() const override;
      virtual const ASymbol &    get_key_class_name() const override;
      virtual SkMetaClass &      get_metaclass() const override;
      virtual bool               is_class_type(const SkClassDescBase * type_p) const override;
      virtual bool               is_generic() const override;

      // Method Member Methods

        virtual SkMethodBase * find_method(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual SkMethodBase * find_method_inherited(const ASymbol & method_name, bool * is_class_member_p = nullptr) const override;
        virtual bool           is_method_inherited_valid(const ASymbol & method_name) const override;

      // Coroutine Member Methods

        virtual SkCoroutineBase * find_coroutine_inherited(const ASymbol & coroutine_name) const override;

  // Class Methods

    static SkClassUnion *    get_or_create(const SkClassUnion & class_union);
    static SkClassDescBase * get_merge(const SkClassDescBase & class1, const SkClassDescBase & class2);
    static SkClassDescBase * get_merge(const APArrayBase<SkClassDescBase> & classes, bool object_on_empty_b = true);
    static SkClassDescBase * get_reduced(const SkClassUnion & class_union, const SkClassUnaryBase & class_to_remove);
    static void              shared_empty()               { ms_shared_unions.free_all(); }  
    static bool              shared_ensure_references();
    static void              shared_track_memory(AMemoryStats * mem_stats_p);

  protected:

  // Internal Methods

    void clear();

  // Data Members

    // [Cached] Most specific superclass common to all classes in this union.
    SkClassUnaryBase * m_common_class_p;

    // Sorted list of classes in this union - there will always be more than one and they
    // will never be direct descendants of one another - i.e. none of the classes are
    // super-classes or sub-classes to any of the other classes in the union.
    tSkSortedTypes m_union;

  // Class Data Members

    // Class union objects that are shared amongst various data-structures
    static APSortedLogicalFree<SkClassUnion> ms_shared_unions;

  };  // SkClassUnion


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Storage specialization - SkMetaClass stored indirectly as pointer in SkUserData rather than whole structure
template<> inline SkMetaClass * SkUserDataBase::as<SkMetaClass>() const                { return *as_stored<SkMetaClass*>(); }
template<> inline void          SkUserDataBase::set(SkMetaClass const * const & value) { *as_stored<const SkMetaClass*>() = value; }
template<> inline void          SkUserDataBase::set(SkMetaClass       * const & value) { *as_stored<SkMetaClass*>() = value; }


#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkClass.inl>
#endif
