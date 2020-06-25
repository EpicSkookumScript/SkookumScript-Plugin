// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Class for managing functions exposed to Blueprint graphs 
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "UObject/WeakObjectPtr.h"
#include "UObject/Class.h"

#include <AgogCore/AFunctionArg.hpp>
#include <AgogCore/APArray.hpp>
#include <AgogCore/AVArray.hpp>
#include <AgogCore/ASymbol.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkInvokableBase.hpp>
#include <SkookumScript/SkMethod.hpp>
#include <SkookumScript/SkParameters.hpp>

class SkClass;
class SkClassDescBase;
class SkInstance;
class SkInvokedMethod;
struct FFrame;

typedef AFunctionArgBase<UClass *>            tSkUEOnClassUpdatedFunc;
typedef AFunctionArgBase2<UFunction *, bool>  tSkUEOnFunctionUpdatedFunc;
typedef AFunctionArgBase<UClass *>            tSkUEOnFunctionRemovedFromClassFunc;

//---------------------------------------------------------------------------------------
// Class for managing functions exposed to Blueprint graphs
class SkUEReflectionManager
  {
  public:
                 SkUEReflectionManager();
                ~SkUEReflectionManager();

    static SkUEReflectionManager * get() { return ms_singleton_p; }

    void         clear(tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f);
    bool         sync_all_from_sk(tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f);
    bool         sync_class_from_sk(SkClass * sk_class_p, tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f);
    bool         sync_all_to_ue(tSkUEOnFunctionUpdatedFunc * on_function_updated_f, bool is_final);

    static bool  does_class_need_instance_property(SkClass * sk_class_p);
    static bool  add_instance_property_to_class(UClass * ue_class_p, SkClass * sk_class_p);

    static bool  can_ue_property_be_reflected(FProperty * ue_property_p);

    static bool  is_skookum_reflected_call(UFunction * function_p);
    static bool  is_skookum_reflected_event(UFunction * function_p);

    void         invoke_k2_delegate(const FScriptDelegate & script_delegate, const SkParameters * sk_params_p, SkInvokedMethod * scope_p, SkInstance ** result_pp);
    void         invoke_k2_delegate(const FMulticastScriptDelegate & script_delegate, const SkParameters * sk_params_p, SkInvokedMethod * scope_p, SkInstance ** result_pp);

  protected:

    // We place this magic number in the rep offset to be able to tell if a UFunction is an Sk event
    // Potential false positive is ok since we are using it only to select which graph nodes to update
    enum { EventMagicRepOffset = 0xBEEF };

    // If the type is contained in a container such as a list
    enum eContainerType : uint8_t
      {
      ContainerType_scalar, // No container, just value itself
      ContainerType_array,  // Array/list
      };

    // To keep track of a property/parameter's name, size and type
    struct TypedName : ANamed
      {
      SkClass *       m_sk_class_p;
      ASymbol         m_sk_class_name;
      uint32_t        m_byte_size;
      eContainerType  m_container_type;

      TypedName(const ASymbol & name, SkClassDescBase * sk_type_p);

      void set_byte_size(FProperty * ue_property_p);
      };

    struct ReflectedCallParam;
    struct ReflectedParamStorer;
    struct ReflectedEventParam;

    typedef SkInstance *  (*tK2ParamFetcher)(FFrame & stack, const ReflectedCallParam & value_type);
    typedef SkInstance *  (*tK2ValueFetcher)(const void * value_p, const TypedName & value_type);
    typedef void          (*tK2ValueAssigner)(SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    typedef uint32_t      (*tSkValueStorer)(void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);

    enum eReflectedFunctionType : uint8_t
      {
      ReflectedFunctionType_call,   // Call from Blueprints into Sk
      ReflectedFunctionType_event,  // Call from Sk into Blueprints
      };

    // Keep track of a binding between Blueprints and Sk
    struct ReflectedFunction : ANamed
      {
      SkInvokableBase *         m_sk_invokable_p;
      TWeakObjectPtr<UFunction> m_ue_function_p;
      eReflectedFunctionType    m_type;
      uint8_t                   m_num_params;
      bool                      m_has_out_params;
      bool                      m_is_class_member;    // Copy of m_sk_invokable_p->is_class_member() in case m_sk_invokable_p goes bad
      uint16_t                  m_ue_params_size;     // Byte size of all parameters combined on UE4 stack
      bool                      m_is_ue_function_built;
      bool                      m_marked_for_delete_class;
      bool                      m_marked_for_delete_all;

      ReflectedFunction(eReflectedFunctionType type, SkInvokableBase * sk_invokable_p, uint32_t num_params)
        : ANamed(sk_invokable_p->get_name())
        , m_sk_invokable_p(sk_invokable_p)
        , m_ue_function_p(nullptr) // Yet unknown
        , m_type(type)
        , m_num_params(num_params)
        , m_has_out_params(false) // Yet unknown
        , m_is_class_member(sk_invokable_p->is_class_member())
        , m_ue_params_size(0) // Yet unknown
        , m_is_ue_function_built(false)
        , m_marked_for_delete_class(false)
        , m_marked_for_delete_all(false)
        {}

      //void rebind_sk_invokable();

      };

    typedef APArrayFree<ReflectedFunction, ASymbol> tReflectedFunctions;

    // Parameter being passed into Sk from Blueprints
    struct ReflectedCallParam : TypedName
      {
      tK2ParamFetcher m_outer_fetcher_p;  // Fetches the parameter from the stack
      tK2ValueFetcher m_inner_fetcher_p;  // Fetches an item from a container parameter

      ReflectedCallParam(const ASymbol & name, SkClassDescBase * sk_type_p) : TypedName(name, sk_type_p), m_outer_fetcher_p(nullptr), m_inner_fetcher_p(nullptr) {}
      };

    // Parameter being passed into Sk from Blueprints
    struct ReflectedParamStorer : TypedName
      {
      tSkValueStorer m_outer_storer_p;
      tSkValueStorer m_inner_storer_p;

      ReflectedParamStorer(const ASymbol & name, SkClassDescBase * sk_type_p) : TypedName(name, sk_type_p), m_outer_storer_p(nullptr), m_inner_storer_p(nullptr) {}
      };

    // Function binding (call from Blueprints into Sk)
    struct ReflectedCall : public ReflectedFunction
      {
      ReflectedParamStorer  m_result;

      ReflectedCall(SkInvokableBase * sk_invokable_p, uint32_t num_params, SkClassDescBase * sk_result_type_p)
        : ReflectedFunction(ReflectedFunctionType_call, sk_invokable_p, num_params)
        , m_result(ASymbol::ms_null, sk_result_type_p)
        {}

      // The parameter entries are stored behind this structure in memory
      ReflectedCallParam *       get_param_array()       { return (ReflectedCallParam *)(this + 1); }
      const ReflectedCallParam * get_param_array() const { return (const ReflectedCallParam *)(this + 1); }
      };

    // Parameter being passed into Blueprints into Sk
    struct ReflectedEventParam : ReflectedParamStorer
      {
      tK2ValueAssigner  m_outer_assigner_p;
      tK2ValueFetcher   m_inner_fetcher_p;
      uint32_t          m_offset;

      ReflectedEventParam(const ASymbol & name, SkClassDescBase * sk_type_p)
        : ReflectedParamStorer(name, sk_type_p)
        , m_outer_assigner_p(nullptr)
        , m_inner_fetcher_p(nullptr)
        , m_offset(0) {}
      };

    // Event binding (call from Sk into Blueprints)
    struct ReflectedEvent : public ReflectedFunction
      {
      mutable TWeakObjectPtr<UFunction> m_ue_function_to_invoke_p; // The copy of our method we actually can invoke

      ReflectedEvent(SkMethodBase * sk_method_p, uint32_t num_params)
        : ReflectedFunction(ReflectedFunctionType_event, sk_method_p, num_params)
        {}

      // The parameter entries are stored behind this structure in memory
      ReflectedEventParam *       get_param_array()       { return (ReflectedEventParam *)(this + 1); }
      const ReflectedEventParam * get_param_array() const { return (const ReflectedEventParam *)(this + 1); }
      };

    // Delegate binding (call from Sk into Blueprints)
    struct ReflectedDelegate
      {
      const SkParameters *  m_sk_params_p;    // Unique pointer to Sk parameters, also used for lookup
      uint8_t               m_num_params;
      bool                  m_has_out_params;
      uint16_t              m_ue_params_size; // Byte size of all parameters combined on UE4 stack

      ReflectedDelegate(const SkParameters * sk_params_p, uint32_t num_params, bool has_out_params, uint32_t ue_params_size)
        : m_sk_params_p(sk_params_p)
        , m_num_params(num_params)
        , m_has_out_params(has_out_params)
        , m_ue_params_size(ue_params_size)
        {}

      // So we can use SkParameters as lookup key
      operator const SkParameters * () const { return m_sk_params_p; }

      // The parameter entries are stored behind this structure in memory
      ReflectedEventParam *       get_param_array() { return (ReflectedEventParam *)(this + 1); }
      const ReflectedEventParam * get_param_array() const { return (const ReflectedEventParam *)(this + 1); }
      };

    typedef APSortedLogicalFree<ReflectedDelegate, const SkParameters *> tReflectedDelegates;

    // Collection of helper functions to translate between Sk and K2 for a particular type
    struct ReflectedAccessors
      {
      tK2ParamFetcher  m_k2_param_fetcher_p;
      tK2ValueFetcher  m_k2_value_fetcher_p;
      tK2ValueAssigner m_k2_value_assigner_p;
      tSkValueStorer   m_sk_value_storer_p;
      };

    // An Sk parameter or raw instance data member reflected to UE4 as FProperty
    struct ReflectedProperty : ANamed
      {
      FProperty *                 m_ue_property_p;
      const ReflectedAccessors *  m_outer_p; // Always set
      const ReflectedAccessors *  m_inner_p; // Only set if this property is a container (e.g. an array)

      ReflectedProperty() : m_ue_property_p(nullptr), m_outer_p(nullptr), m_inner_p(nullptr) {}
      };

    typedef APSortedLogicalFree<ReflectedProperty, ASymbol> tReflectedProperties;

    // AVArray can only be declared with a struct/class as template argument
    struct FunctionIndex
      {
      uint16_t  m_idx;

      FunctionIndex(uint32_t idx) : m_idx((uint16_t)idx) {}
      };

    // An Sk class reflected to UE4
    struct ReflectedClass : ANamed
      {
      TWeakObjectPtr<UClass>  m_ue_static_class_p;  // UE4 equivalent of the class (might be a parent class if the actual class is Blueprint generated)
      AVArray<FunctionIndex>  m_functions;          // Indices of reflected member routines
      tReflectedProperties    m_properties;         // Reflected (raw) data members
      bool                    m_store_sk_instance;  // This class must have a USkookumScriptInstanceProperty attached to it

      ReflectedClass(ASymbol name) : ANamed(name), m_store_sk_instance(false) {}
      };

    typedef APSortedLogicalFree<ReflectedClass, ASymbol> tReflectedClasses;

    static void         exec_sk_method(UObject * context_p, FFrame & stack, void * const result_p, SkClass * class_scope_p, SkInstance * this_p);
    static void         exec_sk_class_method(UObject * context_p, FFrame & stack, void * const result_p);
    static void         exec_sk_instance_method(UObject * context_p, FFrame & stack, void * const result_p);
    static void         exec_sk_coroutine(UObject * context_p, FFrame & stack, void * const result_p);

    template<typename _EventType, typename _LambdaType>
    static void         invoke_k2_event(_EventType * reflected_event_p, SkInvokedMethod * scope_p, SkInstance ** result_pp, _LambdaType && invoker);

    static void         mthd_invoke_k2_event(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void         mthd_struct_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void         mthd_struct_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void         mthd_struct_dtor(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void         mthd_struct_op_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp);

    bool                sync_class_from_sk_recursively(SkClass * sk_class_p, tSkUEOnFunctionRemovedFromClassFunc * on_function_removed_from_class_f);
    bool                try_add_reflected_function(SkInvokableBase * sk_invokable_p);
    bool                try_update_reflected_function(SkInvokableBase * sk_invokable_p, ReflectedClass ** out_reflected_class_pp, int32_t * out_function_index_p);
    bool                add_reflected_call(SkInvokableBase * sk_invokable_p);
    bool                add_reflected_event(SkMethodBase * sk_method_p);
    ReflectedDelegate * add_reflected_delegate(const SkParameters * sk_params_p, UFunction * ue_function_p);
    bool                expose_reflected_function(uint32_t i, tSkUEOnFunctionUpdatedFunc * on_function_updated_f, bool is_final);
    int32_t             store_reflected_function(ReflectedFunction * reflected_function_p, ReflectedClass * reflected_class_p, int32_t function_index_to_use);
    void                delete_reflected_function(uint32_t function_index);
    static UFunction *  find_ue_function(SkInvokableBase * sk_invokable_p);
    static UFunction *  reflect_ue_function(SkInvokableBase * sk_invokable_p, ReflectedProperty * out_param_info_array_p);
    static bool         reflect_ue_params(const SkParameters & sk_params, UFunction * ue_function_p, ReflectedProperty * out_param_info_array_p);
    static bool         reflect_ue_property(FProperty * ue_property_p, ReflectedProperty * out_info_p = nullptr);
    UFunction *         build_ue_function(UClass * ue_class_p, SkInvokableBase * sk_invokable_p, eReflectedFunctionType binding_type, uint32_t binding_index, ReflectedProperty * out_param_info_array_p, bool is_final);
    FProperty *         build_ue_param(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UFunction * ue_function_p, ReflectedProperty * out_info_p, bool is_final);
    FProperty *         build_ue_property(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UObject * ue_outer_p, ReflectedProperty * out_info_p, bool is_final);
    static void         bind_event_method(SkMethodBase * sk_method_p);
    void                on_unknown_type(const ASymbol & sk_name, SkClassDescBase * sk_type_p, UObject * ue_outer_p);
    
    template<class _TypedName>
    static bool         have_identical_signatures(const tSkParamList & param_list, const _TypedName * param_array_p);

    template<class _TypedName>
    static void         rebind_params_to_sk(const tSkParamList & param_list, _TypedName * param_array_p);

    static SkInstance * fetch_k2_param_boolean         (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_integer         (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_real            (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_string          (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_vector2         (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_vector3         (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_vector4         (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_rotation_angles (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_transform       (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_struct_val      (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_struct_ref      (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_entity          (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_enum            (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_array           (FFrame & stack, const ReflectedCallParam & value_type);
    static SkInstance * fetch_k2_param_name            (FFrame & stack, const ReflectedCallParam & value_type);

    static SkInstance * fetch_k2_value_boolean         (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_integer         (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_real            (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_string          (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_vector2         (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_vector3         (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_vector4         (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_rotation_angles (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_transform       (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_struct_val      (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_struct_ref      (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_entity          (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_enum            (const void * value_p, const TypedName & value_type);
    static SkInstance * fetch_k2_value_name            (const void * value_p, const TypedName & value_type);

    static void         assign_k2_value_boolean         (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_integer         (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_real            (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_string          (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_vector2         (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_vector3         (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_vector4         (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_rotation_angles (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_transform       (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_struct_val      (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_struct_ref      (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_entity          (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_enum            (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_array           (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);
    static void         assign_k2_value_name            (SkInstance * dest_p, const void * value_p, const ReflectedEventParam & value_type);

    static uint32_t     store_sk_value_boolean         (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_integer         (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_real            (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_string          (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_vector2         (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_vector3         (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_vector4         (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_rotation_angles (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_transform       (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_struct_val      (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_struct_ref      (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_entity          (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_enum            (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_array           (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);
    static uint32_t     store_sk_value_name            (void * dest_p, SkInstance * value_p, const ReflectedParamStorer & value_type);

    static const ReflectedAccessors ms_accessors_boolean;
    static const ReflectedAccessors ms_accessors_integer;
    static const ReflectedAccessors ms_accessors_real;
    static const ReflectedAccessors ms_accessors_string;
    static const ReflectedAccessors ms_accessors_vector2;
    static const ReflectedAccessors ms_accessors_vector3;
    static const ReflectedAccessors ms_accessors_vector4;
    static const ReflectedAccessors ms_accessors_rotation_angles;
    static const ReflectedAccessors ms_accessors_transform;
    static const ReflectedAccessors ms_accessors_struct_val;
    static const ReflectedAccessors ms_accessors_struct_ref;
    static const ReflectedAccessors ms_accessors_entity;
    static const ReflectedAccessors ms_accessors_enum;
    static const ReflectedAccessors ms_accessors_array;
    static const ReflectedAccessors ms_accessors_name;

    tReflectedFunctions   m_reflected_functions;
    tReflectedClasses     m_reflected_classes;
    tReflectedDelegates   m_reflected_delegates;

    ASymbol               m_result_name;

    UPackage *            m_module_package_p;

    static SkUEReflectionManager * ms_singleton_p; // Hack, make it easy to access for callbacks
        
    static UScriptStruct *  ms_struct_vector2_p;
    static UScriptStruct *  ms_struct_vector3_p;
    static UScriptStruct *  ms_struct_vector4_p;
    static UScriptStruct *  ms_struct_rotation_angles_p;
    static UScriptStruct *  ms_struct_transform_p;

  };
