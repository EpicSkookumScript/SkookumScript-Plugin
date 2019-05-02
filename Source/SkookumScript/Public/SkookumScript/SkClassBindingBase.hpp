// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Base class for binding mechanism 
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumScript/SkClassBindingAbstract.hpp>
#include <SkookumScript/SkInstance.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable : 4127) // Some functions below contain constant conditionals on purpose
#endif

//---------------------------------------------------------------------------------------
// Default class binding for "simple" types = scalars and structs
// This simple class assumes that this data type "owns" or "contains"
// a C++ type/structure with the actual data
// Initializes and stores a number of helpful methods and data structures
//
template<class _BindingClass, typename _DataType>
class SkClassBindingBase : public SkClassBindingAbstract<_BindingClass>, public SkInstance // Derive from SkInstance to gain access to its internals
  {
  public:

    // Types

    typedef _DataType                                     tDataType;
    typedef SkClassBindingAbstract<_BindingClass>         tBindingAbstract;
    typedef SkClassBindingBase<_BindingClass, _DataType>  tBindingBase;
    typedef SkInstance::tUserData                         tUserData;

    // Constants

    enum { Binding_has_ctor      = true }; // If to generate constructor
    enum { Binding_has_ctor_copy = true }; // If to generate copy constructor
    enum { Binding_has_assign    = true }; // If to generate assignment operator
    enum { Binding_has_dtor      = true }; // If to generate destructor

    // "Virtual" Methods - all class bindings are expected to expose these five methods

    //---------------------------------------------------------------------------------------
    // Allocate and initialize a new instance of this SkookumScript type
    template <typename... tParamClasses>
    static SkInstance * new_instance(const tParamClasses & ... args);

    //---------------------------------------------------------------------------------------
    // Initialize an existing instance of this SkookumScript type
    template <typename... tParamClasses>
    _DataType & construct(const tParamClasses & ... constructor_args);

    //---------------------------------------------------------------------------------------
    // Deinitialize an existing instance of this SkookumScript type
    void destruct();

    //---------------------------------------------------------------------------------------
    // Provide access to the data stored inside this instance
    const _DataType & get_data() const;
          _DataType & get_data();

    // Default automatic implementations of basic methods
    static void mthd_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthd_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthd_op_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp);
    static void mthd_dtor(SkInvokedMethod * scope_p, SkInstance ** result_pp);

  protected:

    // Protected constructor so a sub class can construct us
    SkClassBindingBase() : SkInstance(_BindingClass::get_class()) {}

    // Make method bindings known to SkookumScript
    static void register_bindings();

    // Convenience methods - initialize Sk class and bind methods
    static void register_bindings(ASymbol class_name)         { tBindingAbstract::initialize_class(class_name); register_bindings(); }
    static void register_bindings(const char * class_name_p)  { tBindingAbstract::initialize_class(class_name_p); register_bindings(); }
    static void register_bindings(uint32_t class_name_id)     { tBindingAbstract::initialize_class(class_name_id); register_bindings(); }

  };

//=======================================================================================
// Class Method Implementations
//=======================================================================================

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
template<typename... tParamClasses>
inline SkInstance * SkClassBindingBase<_BindingClass, _DataType>::new_instance(const tParamClasses & ... constructor_args)
  {
  SkInstance * instance_p = _BindingClass::get_class()->new_instance();

  static_cast<_BindingClass *>(instance_p)->construct(constructor_args...);

  return instance_p;
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
template<typename... tParamClasses>
inline _DataType & SkClassBindingBase<_BindingClass, _DataType>::construct(const tParamClasses & ... constructor_args)
  {
  if (sizeof(_DataType) <= sizeof(m_user_data))
    {
    return *new((void*)&m_user_data) _DataType(constructor_args...);
    }
  else
    {
    return *(*((_DataType **)&m_user_data) = AMemory::new_object<_DataType>("SkUserData", constructor_args...));
    }
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
inline void SkClassBindingBase<_BindingClass, _DataType>::destruct()
  {
  if (sizeof(_DataType) <= sizeof(m_user_data))
    {
    ((_DataType *)&m_user_data)->~_DataType();
    }
  else
    {
    AMemory::delete_object(*((_DataType **)&m_user_data));
    }
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
inline const _DataType & SkClassBindingBase<_BindingClass, _DataType>::get_data() const
  {
  if (sizeof(_DataType) <= sizeof(m_user_data))
    {
    return *((const _DataType *)&m_user_data);
    }
  else
    {
    return **((const _DataType **)&m_user_data);
    }
  }

//---------------------------------------------------------------------------------------

template<class _BindingClass, typename _DataType>
inline _DataType & SkClassBindingBase<_BindingClass, _DataType>::get_data()
  {
  if (sizeof(_DataType) <= sizeof(m_user_data))
    {
    return *((_DataType *)&m_user_data);
    }
  else
    {
    return **((_DataType **)&m_user_data);
    }
  }

//---------------------------------------------------------------------------------------
// Default constructor - invokes the data type's default constructor
template<class _BindingClass, typename _DataType>
void SkClassBindingBase<_BindingClass, _DataType>::mthd_ctor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  scope_p->get_this()->construct<_BindingClass>();
  }

//---------------------------------------------------------------------------------------
// Copy constructor - invokes the data type's copy constructor
template<class _BindingClass, typename _DataType>
void SkClassBindingBase<_BindingClass, _DataType>::mthd_ctor_copy(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  scope_p->get_this()->construct<_BindingClass>(scope_p->get_arg<_BindingClass>(SkArg_1));
  }

//---------------------------------------------------------------------------------------
// Destructor - invokes the data type's destructor
template<class _BindingClass, typename _DataType>
void SkClassBindingBase<_BindingClass, _DataType>::mthd_dtor(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  scope_p->get_this()->destruct<_BindingClass>();
  }

//---------------------------------------------------------------------------------------
// Assignment operator - invokes the data type's assignment operator
template<class _BindingClass, typename _DataType>
void SkClassBindingBase<_BindingClass, _DataType>::mthd_op_assign(SkInvokedMethod * scope_p, SkInstance ** result_pp)
  {
  SkInstance * this_p = scope_p->get_this();

  this_p->as<_BindingClass>() = scope_p->get_arg<_BindingClass>(SkArg_1);

  // Return this if result desired
  if (result_pp)
    {
    this_p->reference();
    *result_pp = this_p;
    }
  }

//---------------------------------------------------------------------------------------
// Make method bindings known to SkookumScript
template<class _BindingClass, typename _DataType>
void SkClassBindingBase<_BindingClass, _DataType>::register_bindings()
  {
  // Bind raw pointer callback function
  _BindingClass::get_class()->register_raw_pointer_func(sizeof(_DataType) <= sizeof(SkInstance::tUserData) 
    ? static_cast<tSkRawPointerFunc>(&SkInstance::get_raw_pointer_val)
    : static_cast<tSkRawPointerFunc>(&SkInstance::get_raw_pointer_ref));

  // Bind basic methods
  static_assert(_BindingClass::Binding_has_ctor || sizeof(_DataType) <= sizeof(tUserData), "If _DataType does not fit inside m_user_data, it will be allocated from the heap, hence there must be a constructor to allocate the memory.");
  if (_BindingClass::Binding_has_ctor)
    {
    _BindingClass::get_class()->register_method_func(ASymbolX_ctor, _BindingClass::mthd_ctor, SkBindFlag_instance_no_rebind);
    }
  if (_BindingClass::Binding_has_ctor_copy)
    {
    _BindingClass::get_class()->register_method_func(ASymbolX_ctor_copy, _BindingClass::mthd_ctor_copy, SkBindFlag_instance_no_rebind);
    }
  if (_BindingClass::Binding_has_assign)
    {
    _BindingClass::get_class()->register_method_func(ASymbol_assign, _BindingClass::mthd_op_assign, SkBindFlag_instance_no_rebind);
    }
  static_assert(_BindingClass::Binding_has_dtor || sizeof(_DataType) <= sizeof(tUserData), "If _DataType does not fit inside m_user_data, it will be allocated from the heap, hence there must be a destructor to free the memory.");
  if (_BindingClass::Binding_has_dtor)
    {
    _BindingClass::get_class()->register_method_func(ASymbolX_dtor, _BindingClass::mthd_dtor, SkBindFlag_instance_no_rebind);
    }
  }


#ifdef _MSC_VER
  #pragma warning(pop)
#endif
