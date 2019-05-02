// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// Binding classes for UE4 
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "SkookumScriptBehaviorComponent.h"

#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Actor.h"

#include <SkookumScript/SkClassBindingBase.hpp>
#include <SkookumScript/SkString.hpp>
#include <SkookumScript/SkList.hpp>

//---------------------------------------------------------------------------------------

class FSkookumScriptRuntimeGenerator;

//---------------------------------------------------------------------------------------
// Helper class providing useful global variables and static methods
class SKOOKUMSCRIPTRUNTIME_API SkUEClassBindingHelper
  {
  public:

    enum
      {
      Raw_data_info_offset_shift    = 0,      // Byte offset inside structure where this property is stored
      Raw_data_info_offset_mask     = 0xFFFF, // Stored as 16 bit
      Raw_data_info_type_shift      = 16,     // See Raw_data_type_... below
      Raw_data_info_type_mask       = 0xFFFF, // See Raw_data_type_... below
      Raw_data_info_elem_type_shift = 32,
      Raw_data_info_elem_type_mask  = 0xFFFF,

      Raw_data_type_size_shift  = 0,      // Size of this data type
      Raw_data_type_size_mask   = 0x3FF,
      Raw_data_type_extra_shift = 10,     // Extra type-specific information stored here
      Raw_data_type_extra_mask  = 0x3F,
      };

    static UWorld *        get_world(); // Get tha world
    static void            set_world(UWorld * world_p);

  #if WITH_EDITORONLY_DATA
    static void            reset_dynamic_class_mappings();
    static bool            is_static_class_registered(UClass * ue_class_p);
    static bool            is_static_struct_registered(UStruct * ue_struct_p);
    static bool            is_static_enum_registered(UEnum * ue_enum_p);
  #endif
    static void            reset_static_class_mappings(uint32_t reserve);
    static void            reset_static_struct_mappings(uint32_t reserve);
    static void            reset_static_enum_mappings(uint32_t reserve);
    static void            add_slack_to_static_class_mappings(uint32_t slack);
    static void            add_slack_to_static_struct_mappings(uint32_t slack);
    static void            add_slack_to_static_enum_mappings(uint32_t slack);
    static void            forget_sk_classes_in_all_mappings();
    static void            register_static_class(UClass * ue_class_p);
    static void            register_static_struct(UStruct * ue_struct_p);
    static void            register_static_enum(UEnum * ue_enum_p);
    static void            add_static_class_mapping(SkClass * sk_class_p, UClass * ue_class_p);
    static void            add_static_struct_mapping(SkClass * sk_class_p, UStruct * ue_struct_p);
    static void            add_static_enum_mapping(SkClass * sk_class_p, UEnum * ue_enum_p);

    static SkClass *       get_sk_class_from_ue_class(UClass * ue_class_p);
    static UClass *        get_ue_class_from_sk_class(SkClass * sk_class_p);
    static SkClass *       get_sk_class_from_ue_struct(UScriptStruct * ue_struct_p);
    static UScriptStruct * get_ue_struct_from_sk_class(SkClass * sk_class_p);
    static UStruct *       get_ue_struct_or_class_from_sk_class(SkClass * sk_class_p);
    static UClass *        get_static_ue_class_from_sk_class_super(SkClassDescBase * sk_class_p);
    static SkClass *       find_most_derived_super_class_known_to_ue(SkClass * sk_class_p);
    static SkClass *       find_most_derived_super_class_known_to_ue(SkClass * sk_class_p, UClass ** out_ue_class_pp);
    static SkClass *       find_most_derived_super_class_known_to_sk(UClass * ue_class_p);
    static SkClass *       get_object_class(UObject * obj_p, UClass * def_ue_class_p = nullptr, SkClass * def_sk_class_p = nullptr); // Determine SkookumScript class from UClass
    static SkInstance *    get_embedded_instance(UObject * obj_p, SkClass * sk_class_p);
    static SkInstance *    get_embedded_instance(AActor * actor_p, SkClass * sk_class_p);

    static FString         get_ue_class_name_sans_c(UClass * ue_class_p);

    static tSkRawDataInfo  compute_raw_data_info(UProperty * ue_var_p);
    static bool            resolve_raw_data_static(SkClass * sk_class_p);
    static void            resolve_raw_data(SkClass * sk_class_p, UStruct * ue_struct_or_class_p);
    static void            resolve_raw_data_struct(SkClass * sk_class_p, const TCHAR * ue_struct_name_p);
    static bool            resolve_raw_data_funcs(SkClass * sk_class_p, UStruct * ue_struct_or_class_p = nullptr);

    static void *          get_raw_pointer_entity(SkInstance * obj_p);
    static void *          get_raw_pointer_null(SkInstance * obj_p);
    static SkInstance *    access_raw_data_entity(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_boolean(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_integer(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_string(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_enum(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_color(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_list(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);
    static SkInstance *    access_raw_data_user_struct(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);

    template<class _BindingClass>
    static SkInstance *    access_raw_data_struct(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p);

    template<class _BindingClass, typename _DataType>
    static SkInstance *    new_instance(const _DataType & value) { return _BindingClass::new_instance(value); }

    template<class _BindingClass, typename _DataType, typename _CastType = _DataType>
    static void            set_from_instance(_DataType * out_value_p, SkInstance * instance_p) { *out_value_p = (_CastType)instance_p->as<_BindingClass>(); }

    template<class _BindingClass, typename _DataType>
    static void            initialize_list_from_array(SkInstanceList * out_instance_list_p, const TArray<_DataType> & array);

    template<class _BindingClass, typename _DataType>
    static void            initialize_empty_list_from_array(SkInstanceList * out_instance_list_p, const TArray<_DataType> & array);

    template<class _BindingClass, typename _DataType>
    static SkInstance *    list_from_array(const TArray<_DataType> & array);

    template<class _BindingClass, typename _DataType, typename _CastType = _DataType>
    static void            initialize_array_from_list(TArray<_DataType> * out_array_p, const SkInstanceList & list);

  protected:

    // A few handy symbol id constants
    enum
      {
      ASymbolId_Entity          = 0x0984415e,
      ASymbolId_EntityClass     = 0xbfff707c,
      ASymbolId_GameEntity      = 0xfed2d79d,
      ASymbolId_Vector2         = 0x29ca61a5,
      ASymbolId_Vector3         = 0x5ecd5133,
      ASymbolId_Rotation        = 0xd00afaa7,
      ASymbolId_RotationAngles  = 0x81d74f22,
      ASymbolId_UStruct         = 0xe07d6ad5,
      };

    static const FName NAME_Entity;

    // Copy of bool property with public members so we can extract its private data
    struct HackedBoolProperty : UProperty
      {
      /** Size of the bitfield/bool property. Equal to ElementSize but used to check if the property has been properly initialized (0-8, where 0 means uninitialized). */
      uint8 FieldSize;
      /** Offset from the member variable to the byte of the property (0-7). */
      uint8 ByteOffset;
      /** Mask of the byte byte with the property value. */
      uint8 ByteMask;
      /** Mask of the field with the property value. Either equal to ByteMask or 255 in case of 'bool' type. */
      uint8 FieldMask;
      };

    // Copy of TArray so we access protected methods
    struct HackedTArray : TArray<uint8>
      {
      public:
        // Set size and leave array uninitialized
        FORCEINLINE void resize_uninitialized(int32 num_elements, int32 num_bytes_per_element)
          {
          if (num_elements > ArrayMax)
            {
            resize_to(num_elements, num_bytes_per_element);
            }
          SetNumUninitialized(num_elements);
          }

      protected:
        FORCENOINLINE void resize_to(int32 new_max, int32 num_bytes_per_element);
      };

    // HACK to utilize 4 bytes of unused space behind a uint32
    struct HackedUint32Mem
      {
      enum { Magic = 0x5c3a };

      uint32_t  m_uint32;
      uint16_t  m_magic;
      uint16_t  m_data_idx;

      uint32_t  get_data_idx() const             { return m_magic == Magic ? m_data_idx : 0; }
      void      set_data_idx(uint32_t data_idx)  { m_magic = Magic; m_data_idx = (uint16_t)data_idx; }
      };

    // A version of SkInstance that does not call the destructor
    class SkRawDataInstance : public SkInstance
      {
      public:

        // Special constructor meant for just replacing vtable
        // Leaves class & user data unchanged
        SkRawDataInstance(eALeaveMemoryUnchanged) : SkInstance(ALeaveMemoryUnchanged) {}

        virtual void on_no_references() override
          {
          // Change virtual table for this instance back to SkInstance
          new (this) SkInstance(ALeaveMemoryUnchanged);
          // Important: Do not call destructor here
          // Put it back on the pool for reuse
          SkInstance::delete_this();
          }
      };


  static UClass *        find_ue_class_from_sk_class(SkClass * sk_class_p);
  static UScriptStruct * find_ue_struct_from_sk_class(SkClass * sk_class_p);
  static SkClass *       find_sk_class_from_ue_class(UClass * ue_class_p);
  static SkClass *       find_sk_class_from_ue_struct(UStruct * ue_struct_p);
  static SkClass *       find_sk_class_from_ue_enum(UEnum * ue_enum_p);
  static uint32_t        get_sk_class_idx_from_ue_class(UClass * ue_class_p);
  static void            set_sk_class_idx_on_ue_class(UClass * ue_class_p, uint32_t sk_class_idx);
  static UStruct *       get_ue_struct_ptr_from_sk_class(SkClass * sk_class_p);
  static void            set_ue_struct_ptr_on_sk_class(SkClass * sk_class_p, UStruct * ue_struct_p);

  #if WITH_EDITORONLY_DATA
    static UClass *      add_dynamic_class_mapping(SkClassDescBase * sk_class_desc_p);
    static SkClass *     add_dynamic_class_mapping(UBlueprint * blueprint_p);
  #endif
    static UClass *      add_static_class_mapping(SkClassDescBase * sk_class_desc_p);
    static SkClass *     add_static_class_mapping(UClass * ue_class_p);

    static TMap<UClass*, SkClass*>                            ms_static_class_map_u2s; // Maps UClasses to their respective SkClasses
    static TMap<SkClassDescBase*, UClass*>                    ms_static_class_map_s2u; // Maps SkClasses to their respective UClasses
    static TMap<UStruct*, SkClass*>                           ms_static_struct_map_u2s; // Maps UStructs to their respective SkClasses
    static TMap<SkClassDescBase*, UStruct*>                   ms_static_struct_map_s2u; // Maps SkClasses to their respective UStructs
    static TMap<UEnum*, SkClass*>                             ms_static_enum_map_u2s; // Maps UEnums to their respective SkClasses

  #if WITH_EDITORONLY_DATA
    static TMap<UBlueprint*, SkClass*>                        ms_dynamic_class_map_u2s; // Maps Blueprints to their respective SkClasses
    static TMap<SkClassDescBase*, TWeakObjectPtr<UBlueprint>> ms_dynamic_class_map_s2u; // Maps SkClasses to their respective Blueprints
  #endif

    static int32_t      get_world_data_idx();
    static int32_t      ms_world_data_idx;

  };

//---------------------------------------------------------------------------------------
// Template specializations
template<> inline SkInstance * SkUEClassBindingHelper::new_instance<SkString, FString>(const FString & value) { return SkString::new_instance(AString(TCHAR_TO_WCHAR(*value), value.Len())); }
template<> inline void         SkUEClassBindingHelper::set_from_instance<SkString, FString>(FString * out_value_p, SkInstance * instance_p) { *out_value_p = FString(instance_p->as<SkString>().as_cstr()); }

//---------------------------------------------------------------------------------------
// Customized version of the UE weak pointer

template<class _UObjectType>
class SkUEWeakObjectPtr
  {
  public:
    SkUEWeakObjectPtr() {}
    SkUEWeakObjectPtr(_UObjectType * obj_p) : m_ptr(obj_p) {}

    bool is_valid() const               { return m_ptr.IsValid(); }
    _UObjectType * get_obj() const      { return m_ptr.Get(); }
    operator _UObjectType * () const    { return m_ptr.Get(); } // Cast myself to UObject pointer so it can be directly assigned to UObject pointer
    _UObjectType * operator -> () const { return m_ptr.Get(); }

    void operator = (const _UObjectType * obj_p)                    { m_ptr = obj_p; }
    void operator = (const SkUEWeakObjectPtr<_UObjectType> & other) { m_ptr = other.m_ptr; }

  protected:
    TWeakObjectPtr<_UObjectType>  m_ptr;
  };

//---------------------------------------------------------------------------------------
// Binding class encapsulating a (weak pointer to a) UObject
template<class _BindingClass, class _UObjectType>
class SkUEClassBindingEntity : public SkClassBindingBase<_BindingClass, SkUEWeakObjectPtr<_UObjectType>>
  {
  public:

    typedef SkUEWeakObjectPtr<_UObjectType>                     tDataType;
    typedef SkClassBindingAbstract<_BindingClass>               tBindingAbstract;
    typedef SkClassBindingBase<_BindingClass, tDataType>        tBindingBase;
    typedef SkUEClassBindingEntity<_BindingClass, _UObjectType> tBindingEntity;

    // Don't generate these per class - inherit copy constructor and assignment operator from SkUEEntity
    enum { Binding_has_ctor      = false }; // If to generate constructor
    enum { Binding_has_ctor_copy = false }; // If to generate copy constructor
    enum { Binding_has_assign    = false }; // If to generate assignment operator
    enum { Binding_has_dtor      = false }; // If to generate destructor

    // Class Data

    static UClass * ms_uclass_p; // Pointer to the UClass belonging to this binding

    // Class Methods

    //---------------------------------------------------------------------------------------
    // Allocate and initialize a new instance of this SkookumScript type with given sub class
    static SkInstance * new_instance(_UObjectType * obj_p, SkClass * sk_class_p)
      {
      SkInstance * instance_p = SkUEClassBindingHelper::get_embedded_instance(obj_p, sk_class_p);
      if (instance_p)
        {
        instance_p->reference();
        }
      else
        {
        instance_p = sk_class_p->new_instance();
        instance_p->construct<tBindingBase>(obj_p);
        }
      return instance_p;
      }

    //---------------------------------------------------------------------------------------
    // Allocate and initialize a new instance of this SkookumScript type
    // We override this so we can properly determine the actual class of the SkInstance
    // which may be a sub class of tBindingAbstract::ms_class_p 
    static SkInstance * new_instance(_UObjectType * obj_p, UClass * def_ue_class_p = nullptr, SkClass * def_sk_class_p = nullptr)
      {
      SK_ASSERTX(!def_ue_class_p || def_ue_class_p->IsChildOf(tBindingEntity::ms_uclass_p), "If you pass in def_uclass_p, it must be the same as or a super class of ms_uclass_p.");
      SK_ASSERTX(!def_sk_class_p || def_sk_class_p->is_class(*tBindingAbstract::ms_class_p), "If you pass in def_class_p, it must be the same as or a super class of ms_class_p.");
      SkClass * sk_class_p = SkUEClassBindingHelper::get_object_class(obj_p, def_ue_class_p ? def_ue_class_p : ms_uclass_p, def_sk_class_p ? def_sk_class_p : tBindingAbstract::ms_class_p);
      return new_instance(obj_p, sk_class_p);
      }

  protected:

    // Make method bindings known to SkookumScript
    static void register_bindings()
      {
      // Bind basic methods
      tBindingBase::register_bindings();

      // Bind raw pointer callback function
      tBindingAbstract::ms_class_p->register_raw_pointer_func(&SkUEClassBindingHelper::get_raw_pointer_entity);

      // Bind raw accessor callback function
      tBindingAbstract::ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_entity);
      }

    // Convenience methods - initialize Sk class and bind methods
    static void register_bindings(ASymbol class_name)         { tBindingAbstract::initialize_class(class_name); register_bindings(); }
    static void register_bindings(const char * class_name_p)  { tBindingAbstract::initialize_class(class_name_p); register_bindings(); }
    static void register_bindings(uint32_t class_name_id)     { tBindingAbstract::initialize_class(class_name_id); register_bindings(); }

  };

//---------------------------------------------------------------------------------------
// Binding class encapsulating a (weak pointer to an) AActor
template<class _BindingClass, class _AActorType>
class SkUEClassBindingActor : public SkUEClassBindingEntity<_BindingClass, _AActorType>
  {
  public:

    typedef SkClassBindingAbstract<_BindingClass>               tBindingAbstract;
    typedef SkUEClassBindingEntity<_BindingClass, _AActorType>  tBindingEntity;

    // Class Methods

    //---------------------------------------------------------------------------------------
    // Allocate and initialize a new instance of this SkookumScript type
    // We override this so we can properly determine the actual class of the SkInstance
    // which may be a sub class of tBindingAbstract::ms_class_p 
    // The actor may also contain its own SkInstance inside its SkookumScriptClassDataComponent
    static SkInstance * new_instance(_AActorType * actor_p, UClass * def_ue_class_p = nullptr, SkClass * def_sk_class_p = nullptr)
      {
      return tBindingEntity::new_instance(actor_p, def_ue_class_p ? def_ue_class_p : tBindingEntity::ms_uclass_p, def_sk_class_p ? def_sk_class_p : tBindingAbstract::ms_class_p);
      }

  };

//---------------------------------------------------------------------------------------
// Class binding for UStruct
template<class _BindingClass, typename _DataType>
class SkUEClassBindingStruct : public SkClassBindingBase<_BindingClass, _DataType>
  {
  public:

    typedef SkClassBindingAbstract<_BindingClass>             tBindingAbstract;
    typedef SkClassBindingBase<_BindingClass, _DataType>      tBindingBase;
    typedef SkUEClassBindingStruct<_BindingClass, _DataType>  tBindingStruct;

    static UStruct * ms_ustruct_p; // Pointer to the UStruct belonging to this binding

  protected:

    // Make method bindings known to SkookumScript
    static void register_bindings()
      {
      // Bind basic methods
      tBindingBase::register_bindings();

      // Bind raw accessor callback function
      tBindingAbstract::ms_class_p->register_raw_accessor_func(&SkUEClassBindingHelper::access_raw_data_struct<_BindingClass>);
      }

    // Convenience methods - initialize Sk class and bind methods
    static void register_bindings(ASymbol class_name)        { tBindingAbstract::initialize_class(class_name); register_bindings(); }
    static void register_bindings(const char * class_name_p) { tBindingAbstract::initialize_class(class_name_p); register_bindings(); }
    static void register_bindings(uint32_t class_name_id)    { tBindingAbstract::initialize_class(class_name_id); register_bindings(); }

  };

//---------------------------------------------------------------------------------------
// Class binding for UStruct with plain old data (assign/copy with memcpy)
template<class _BindingClass, typename _DataType>
class SkUEClassBindingStructPod : public SkUEClassBindingStruct<_BindingClass,_DataType>
  {
  public:

  #ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
  #endif

    // Copy constructor and assignment use memcpy
    static void mthd_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp) { ::memcpy(&scope_p->this_as<_BindingClass>(), &scope_p->get_arg<_BindingClass>(SkArg_1), sizeof(typename _BindingClass::tDataType)); }
    static void mthd_op_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp) { ::memcpy(&scope_p->this_as<_BindingClass>(), &scope_p->get_arg<_BindingClass>(SkArg_1), sizeof(typename _BindingClass::tDataType)); }

  #ifdef __clang__
    #pragma clang diagnostic pop
  #endif
  };

//---------------------------------------------------------------------------------------
// Class binding for types with a constructor that takes an EForceInit argument
template<class _BindingClass, typename _DataType>
class SkClassBindingSimpleForceInit : public SkClassBindingBase<_BindingClass, _DataType>
  {
  public:
    // Constructor initializes with ForceInitToZero
    static void mthd_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp) { scope_p->get_this()->construct<_BindingClass>(ForceInitToZero); }
  };

//=======================================================================================
// Class Data Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Pointer to the UClass belonging to this binding
template<class _BindingClass, class _UObjectType>
UClass * SkUEClassBindingEntity<_BindingClass, _UObjectType>::ms_uclass_p = nullptr;

//---------------------------------------------------------------------------------------
// Pointer to the UStruct belonging to this binding
template<class _BindingClass, typename _DataType>
UStruct * SkUEClassBindingStruct<_BindingClass, _DataType>::ms_ustruct_p = nullptr;

//=======================================================================================
// Inline Function Definitions
//=======================================================================================

#if WITH_EDITORONLY_DATA

//---------------------------------------------------------------------------------------

inline bool SkUEClassBindingHelper::is_static_class_registered(UClass * ue_class_p)
  {
  return !ue_class_p->UObject::IsA<UBlueprintGeneratedClass>() && get_sk_class_from_ue_class(ue_class_p);
  }

//---------------------------------------------------------------------------------------

inline bool SkUEClassBindingHelper::is_static_struct_registered(UStruct * ue_struct_p)
  {
  // This is just used for script generation, therefore a slower lookup is ok here
  return !!find_sk_class_from_ue_struct(ue_struct_p);
  }

//---------------------------------------------------------------------------------------

inline bool SkUEClassBindingHelper::is_static_enum_registered(UEnum * ue_enum_p)
  {
  // This is just used for script generation, therefore a slower lookup is ok here
  return !!find_sk_class_from_ue_enum(ue_enum_p);
  }

#endif

//---------------------------------------------------------------------------------------

inline SkClass * SkUEClassBindingHelper::get_sk_class_from_ue_class(UClass * ue_class_p)
  {
  uint32_t sk_class_idx = get_sk_class_idx_from_ue_class(ue_class_p);
  if (sk_class_idx)
    {
    const tSkClasses & sk_classes = SkBrain::get_classes();
    if (sk_class_idx < sk_classes.get_length())
      {
      SkClass * sk_class_p = sk_classes[sk_class_idx];

      #if (SKOOKUM & SK_DEBUG)
        // In debug builds (which allow live updating), make sure the return pointer matches, otherwise fall through and re-resolve
        if (get_ue_struct_ptr_from_sk_class(sk_class_p) == ue_class_p)
          {
          return sk_class_p;
          }
      #else
        // If no live update allowed, the index is good forever
        #ifdef A_MAD_CHECK
          // If extra checking is on though, make sure it really is
          UStruct * ue_struct_p = get_ue_struct_ptr_from_sk_class(sk_class_p); 
          A_VERIFYX(!ue_struct_p || ue_class_p == ue_struct_p, a_str_format("Mapped classes don't match for '%s' ('%S' != '%S')", sk_class_p->get_name_cstr(), ue_struct_p ? *ue_struct_p->GetName() : TEXT("?"), *ue_class_p->GetName()));
        #endif
        return sk_class_p;
      #endif
      }
    }
  return find_sk_class_from_ue_class(ue_class_p);
  }

//---------------------------------------------------------------------------------------
// Find our (static!) UE counterpart
// If there is no direct match, crawl up the class hierarchy until we find a match
inline UClass * SkUEClassBindingHelper::get_static_ue_class_from_sk_class_super(SkClassDescBase * sk_class_p)
  {
  SkClass * sk_ue_class_p = sk_class_p->get_key_class();
  while (sk_ue_class_p->get_annotation_flags() & SkAnnotation_reflected_data)
    {
    sk_ue_class_p = sk_ue_class_p->get_superclass();
    }
  UClass * ue_class_p;
  do 
  {
    ue_class_p = get_ue_class_from_sk_class(sk_ue_class_p);
  } while (!ue_class_p && (sk_ue_class_p = sk_ue_class_p->get_superclass()) != nullptr);
  return ue_class_p;
  }

//---------------------------------------------------------------------------------------

inline UClass * SkUEClassBindingHelper::get_ue_class_from_sk_class(SkClass * sk_class_p)
  {
  UClass * ue_class_p = static_cast<UClass *>(get_ue_struct_ptr_from_sk_class(sk_class_p));  
  SK_ASSERTX(!ue_class_p || ue_class_p->UObject::IsA<UClass>(), a_str_format("Requested UClass '%S' is not a UClass!", *ue_class_p->GetName()));

  if (!ue_class_p && sk_class_p->is_entity_class())
    {
    ue_class_p = find_ue_class_from_sk_class(sk_class_p);
    }

  return ue_class_p;
  }

//---------------------------------------------------------------------------------------

inline SkClass * SkUEClassBindingHelper::get_sk_class_from_ue_struct(UScriptStruct * ue_struct_p)
  {
  // No fast lookup right now as this is not called at runtime
  return find_sk_class_from_ue_struct(ue_struct_p);
  }

//---------------------------------------------------------------------------------------

inline UScriptStruct * SkUEClassBindingHelper::get_ue_struct_from_sk_class(SkClass * sk_class_p)
  {
  UScriptStruct * ue_struct_p = static_cast<UScriptStruct *>(get_ue_struct_ptr_from_sk_class(sk_class_p));
  SK_ASSERTX(!ue_struct_p || ue_struct_p->IsA<UScriptStruct>(), a_str_format("Requested UScriptStruct '%S' is not a UScriptStruct!", *ue_struct_p->GetName()));

  if (!ue_struct_p)
    {
    ue_struct_p = find_ue_struct_from_sk_class(sk_class_p);
    }

  return ue_struct_p;
  }

//---------------------------------------------------------------------------------------

inline UStruct * SkUEClassBindingHelper::get_ue_struct_or_class_from_sk_class(SkClass * sk_class_p)
  {
  UStruct * ue_struct_or_class_p = get_ue_struct_ptr_from_sk_class(sk_class_p);

  if (!ue_struct_or_class_p)
    {
    ue_struct_or_class_p = sk_class_p->is_entity_class()
      ? static_cast<UStruct *>(find_ue_class_from_sk_class(sk_class_p))
      : static_cast<UStruct *>(find_ue_struct_from_sk_class(sk_class_p));
    }

  return ue_struct_or_class_p;
  }

//---------------------------------------------------------------------------------------
// Given a SkookumScript class, find most derived SkookumScript class known to UE4
inline SkClass * SkUEClassBindingHelper::find_most_derived_super_class_known_to_ue(SkClass * sk_class_p)
  {
  for (; sk_class_p; sk_class_p = sk_class_p->get_superclass())
    {
    if (get_ue_class_from_sk_class(sk_class_p)) break;
    }
  return sk_class_p;
  }

//---------------------------------------------------------------------------------------
// Given a SkookumScript class, find most derived SkookumScript class known to UE4
inline SkClass * SkUEClassBindingHelper::find_most_derived_super_class_known_to_ue(SkClass * sk_class_p, UClass ** out_ue_class_pp)
  {
  UClass * ue_class_p = nullptr;
  for (; sk_class_p; sk_class_p = sk_class_p->get_superclass())
    {
    ue_class_p = get_ue_class_from_sk_class(sk_class_p);
    if (ue_class_p) break;
    }
  *out_ue_class_pp = ue_class_p;
  return sk_class_p;
  }

//---------------------------------------------------------------------------------------
// Given a UE4 class, find most derived UE4 class known to SkookumScript and return its SkookumScript class
inline SkClass * SkUEClassBindingHelper::find_most_derived_super_class_known_to_sk(UClass * ue_class_p)
  {
  SkClass * sk_class_p;
  for (sk_class_p = nullptr; !sk_class_p && ue_class_p; ue_class_p = ue_class_p->GetSuperClass())
    {
    sk_class_p = get_sk_class_from_ue_class(ue_class_p);
    }
  return sk_class_p;
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass>
SkInstance * SkUEClassBindingHelper::access_raw_data_struct(void * obj_p, tSkRawDataInfo raw_data_info, SkClassDescBase * data_type_p, SkInstance * value_p)
  {
  uint32_t byte_offset = (raw_data_info >> Raw_data_info_offset_shift) & Raw_data_info_offset_mask;
  SK_ASSERTX(((raw_data_info >> (Raw_data_info_type_shift + Raw_data_type_size_shift)) & Raw_data_type_size_mask) == sizeof(typename _BindingClass::tDataType), "Size of data type and data member must match!");

  typename _BindingClass::tDataType * data_p = (typename _BindingClass::tDataType *)((uint8_t*)obj_p + byte_offset);

  // Set or get?
  if (value_p)
    {
    // Set value
    *data_p = value_p->as<_BindingClass>();
    return nullptr;
    }

  // Get value
  return _BindingClass::new_instance(*data_p);
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
void SkUEClassBindingHelper::initialize_empty_list_from_array(SkInstanceList * out_instance_list_p, const TArray<_DataType> & array)
  {
  APArray<SkInstance> & list_instances = out_instance_list_p->get_instances();
  SK_ASSERTX(list_instances.is_empty(), "initialize_empty_list_from_array() called with non-empty list!");
  list_instances.ensure_size(array.Num());
  for (auto & item : array)
    {
    list_instances.append(*new_instance<_BindingClass, _DataType>(item));
    }
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
inline void SkUEClassBindingHelper::initialize_list_from_array(SkInstanceList * out_instance_list_p, const TArray<_DataType> & array)
  {
  out_instance_list_p->empty();
  initialize_empty_list_from_array<_BindingClass, _DataType>(out_instance_list_p, array);
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
inline SkInstance * SkUEClassBindingHelper::list_from_array(const TArray<_DataType> & array)
  {
  SkInstance * instance_p = SkList::new_instance(array.Num());
  initialize_empty_list_from_array<_BindingClass, _DataType>(&instance_p->as<SkList>(), array);
  return instance_p;
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType, typename _CastType>
void SkUEClassBindingHelper::initialize_array_from_list(TArray<_DataType> * out_array_p, const SkInstanceList & list)
  {
  APArray<SkInstance> & instances = list.get_instances();
  uint32                length    = instances.get_length();

  if (length == 0u)
    {
    return;
    }

  out_array_p->AddDefaulted(length);

  _DataType *   out_items_p      = out_array_p->GetData();
  SkInstance ** instances_pp     = instances.get_array();
  SkInstance ** instances_end_pp = instances_pp + length;

  while (instances_pp < instances_end_pp)
    {
    set_from_instance<_BindingClass, _DataType, _CastType>(out_items_p, *instances_pp);
    out_items_p++;
    instances_pp++;
    }
  }


//---------------------------------------------------------------------------------------
// Returns index into master class array or 0 if unset
// HACK! This exploits unused memory between member variables
inline uint32_t SkUEClassBindingHelper::get_sk_class_idx_from_ue_class(UClass * ue_class_p)
  {
  //static_assert(offsetof(UClass, ClassWithin) - offsetof(UClass, ClassCastFlags) >= sizeof(HackedUint32Mem), "Not enough storage space for sk_class_idx!");
  //return reinterpret_cast<const HackedUint32Mem *>(&ue_class_p->ClassCastFlags)->get_data_idx();
  return 0;
  }

//---------------------------------------------------------------------------------------
// Sets index into master class array or 0 if unset
// HACK! This exploits unused memory between member variables
inline void SkUEClassBindingHelper::set_sk_class_idx_on_ue_class(UClass * ue_class_p, uint32_t sk_class_idx)
  {
  //reinterpret_cast<HackedUint32Mem *>(&ue_class_p->ClassCastFlags)->set_data_idx(sk_class_idx);
  }

//---------------------------------------------------------------------------------------
// Fetch struct pointer from Sk class user data
inline UStruct * SkUEClassBindingHelper::get_ue_struct_ptr_from_sk_class(SkClass * sk_class_p)
  {
  // Use a weak pointer since classes can be dynamically loaded and unloaded (e.g. during map load)
  return sk_class_p->get_user_data<TWeakObjectPtr<UStruct>>().Get();
  }

//---------------------------------------------------------------------------------------
// Set struct pointer to Sk class user data
inline void SkUEClassBindingHelper::set_ue_struct_ptr_on_sk_class(SkClass * sk_class_p, UStruct * ue_struct_p)
  {
  sk_class_p->set_user_data<TWeakObjectPtr<UStruct>>(ue_struct_p);
  }



