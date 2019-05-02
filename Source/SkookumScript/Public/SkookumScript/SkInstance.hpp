// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for simplest type of object in language - instance of a
// class without data members
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ARefCount.hpp>
#include <AgogCore/AString.hpp>
#include <SkookumScript/SkObjectBase.hpp>
#include <SkookumScript/SkUserData.hpp>

//=======================================================================================
// Global Macros / Defines
//=======================================================================================

// Trace instance reference counting
//#define SK_INSTANCE_REF_DBG


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkBrain;
class SkClass;
class SkList;
class SkCoroutineCall;
class SkExpressionBase;
class SkMethodCallBase;
class SkInvokedBase;
class SkInvokedCoroutine;
class SkInvokedMethod;
struct SkClosureInvokeInfo;


//---------------------------------------------------------------------------------------
// Flags used when converting an SkInstance to a code string
enum eSkCodeFlag
  {
  // If set and a new line character is encountered then replace with escaped new line
  // and also split string literal into multiple lines for better readability:
  //   This:
  //     "First line
  //      Second line"
  //      
  //   if true becomes:
  //     "First line\n" +
  //     "Second line"
  //     
  //   if false becomes:
  //     "First line\nSecond line"
  SkCodeFlag_break_lines   = 1 << 0,

  // Append the class type after the code string. Only used when a String() method is
  // called and ignored for simple types where the type is obvious.
  SkCodeFlag_include_type  = 1 << 1,

  SkCodeFlag__none    = 0,
  SkCodeFlag__default = SkCodeFlag__none
  };

// Used with abort commands
enum eSkNotify
  {
  SkNotify_ignore   = 0x0,  // Caller is not notified
  SkNotify_fail     = 0x1,  // Caller notified that this sub-expression did not successfully complete
  SkNotify_success  = 0x3   // Caller notified that this sub-expression successfully completed
  };

// Used with abort commands - also see eSkNotify 
enum eSkNotifyChild
  {
  SkNotifyChild_detach  = 0x0,  // Child/sub calls ignore the fact that their caller is being aborted
  SkNotifyChild_abort   = 0x1   // Abort child/sub calls
  };

//---------------------------------------------------------------------------------------
// SkookumScript user/reusable instance object / class instance - simplest object without
// any data members and with reference counting and user data
// 
// Subclasses: SkBoolean, SkClosure, SkDataInstance(SkActor), SkInstanceUnreffed(SkMetaClass)
// Author(s):  Conan Reis
class SK_API SkInstance : public SkObjectBase, public ARefCountMix<SkInstance>
  {
  // Accesses protected elements
  friend class SkBrain;
  friend class SkClosure;

  public:

  // Methods

    SK_NEW_OPERATORS(SkInstance);

    SkInstance();
    SkInstance(eALeaveMemoryUnchanged);
    virtual ~SkInstance() override;

    virtual bool is_metaclass() const;
    SkClass *    get_class() const;
    SkClass *    get_key_class() const;
    void         set_class(SkClass * class_p);

    virtual void delete_this();

    virtual void as_code_append(AString * str_p, uint32_t code_flags = SkCodeFlag__none, SkInvokedBase * caller_p = nullptr) const;
    AString      as_code(uint32_t code_flags = SkCodeFlag__none, SkInvokedBase * caller_p = nullptr) const  { AString str; as_code_append(&str, code_flags, caller_p); return str; }
    void         as_string_invoke_append(AString * str_p, SkInvokedBase * caller_p = nullptr) const;
    AString      as_watch_value() const { return as_code(); }

    #if defined(SK_AS_STRINGS)
      virtual AString         as_string_debug() const;
      virtual const ASymbol & get_name_debug() const                            { return ASymbol::ms_null; }
    #endif

    // !!DEPRECATED!!
    template <typename _UserType> _UserType * as_data() const                   { return m_user_data.as<_UserType>(); }
    template <typename _UserType> void        set_data(_UserType const & value) { return m_user_data.set(value); }


    //---------------------------------------------------------------------------------------
    // Allocate (from pool) and construct a new SkInstance of using _BindingClass
    template <class _BindingClass, typename... _ParamClasses>
    static SkInstance * new_instance(const _ParamClasses & ... args)
      {
      return _BindingClass::new_instance(args...);
      }

    //---------------------------------------------------------------------------------------
    // Allocate memory for internal data (if necessary) and invoke constructor
    template <class _BindingClass, typename... _ParamClasses>
    typename _BindingClass::tDataType & construct(const _ParamClasses & ... constructor_args)
      {
      return static_cast<_BindingClass*>(this)->construct(constructor_args...);
      }

    //---------------------------------------------------------------------------------------
    // Invoke destructor of internal data and free memory (if necessary)
    template <class _BindingClass>
    void destruct()
      {
      static_cast<_BindingClass*>(this)->destruct();
      }

    //---------------------------------------------------------------------------------------
    // Get const reference to internal data
    template <class _BindingClass>
    const typename _BindingClass::tDataType & as() const
      {
      return static_cast<const _BindingClass*>(this)->get_data();
      }

    //---------------------------------------------------------------------------------------
    // Get writable reference to internal data
    template <class _BindingClass>
    typename _BindingClass::tDataType & as()
      {
      return static_cast<_BindingClass*>(this)->get_data();
      }

  // Method Invocation

    //---------------------------------------------------------------------------------------
    // This calls this instance's SkookumScript default constructor method !() according to
    // its class type.
    // 
    // Examples:
    //   Generally called in the constructor of a custom object derived from the SkInstance
    //   class.
    //   
    // Notes:
    //   This method allows for the delayed initialization of an object - i.e. after the
    //   object has already been created.
    void call_default_constructor();

    //---------------------------------------------------------------------------------------
    // Calls this instance's SkookumScript destructor method !!() according to its class type.
    // 
    // Examples: Generally called whenever the instance has no references to it.
    void call_destructor();

    //---------------------------------------------------------------------------------------
    // Evaluates the method with 0 or more arguments and returns immediately
    // 
    // Params:
    //   method_name: name of method to call
    //   args_pp:
    //     Optional pointers to object instances to use as arguments - each one present
    //     should have its reference count incremented and each defaulted/skipped argument
    //     should be a `nullptr` element. If `arg_count` is 0 this is ignored
    //   arg_count:
    //     number of arguments to use in `args_pp`. If it is 0 then no arguments are passed
    //     and `args_pp` is ignored.
    //   caller_p:
    //     object that called/invoked this expression and that may await a result.  If it is
    //     nullptr, then there is no object that needs to be notified when this invocation is
    //     complete.
    //   result_pp:
    //     Pointer to a pointer to store the instance resulting from the invocation of this
    //     expression.  If it is nullptr, then the result does not need to be returned and
    //     only side-effects are desired.
    //     
    // Notes:
    //   This is a convenience method to use instead of SkMethodCall<>::invoke_call() - if more arguments
    //   or control is desired, then use SkMethodCall<>::invoke_call()
    //   
    // See:
    //   method_call(), method_query(), SkMethodCall<>::invoke_call(), call_destructor(),
    //   call_default_constructor()
    virtual void method_call(
      const ASymbol & method_name,
      SkInstance **   args_pp,
      uint32_t        arg_count,
      SkInstance **   result_pp = nullptr,
      SkInvokedBase * caller_p = nullptr
      );

    //---------------------------------------------------------------------------------------
    // Evaluates the method with 0/1 arguments and returns immediately
    // 
    // Params:
    //   method_name: name of method to call
    //   arg_p:
    //     Optional pointer to object instance to use as an argument.  If it is present it
    //     should have its reference count incremented.  If it is nullptr, then no arguments
    //     are passed.
    //   caller_p:
    //     object that called/invoked this expression and that may await a result.  If it is
    //     nullptr, then there is no object that needs to be notified when this invocation is
    //     complete.
    //   result_pp:
    //     Pointer to a pointer to store the instance resulting from the invocation of this
    //     expression.  If it is nullptr, then the result does not need to be returned and
    //     only side-effects are desired.
    //     
    // Notes:
    //   This is a convenience method to use instead of `method_call(name, args_pp, --)`
    //   - if more arguments or control is desired, then use it instead
    //   
    // See:
    //   method_call(), method_query(), SkMethodCall<>::invoke_call(), call_destructor(),
    //   call_default_constructor()
    void method_call(
      const ASymbol & method_name,
      SkInstance *    arg_p = nullptr,
      SkInstance **   result_pp = nullptr,
      SkInvokedBase * caller_p = nullptr
      );

    //---------------------------------------------------------------------------------------
    // Evaluates the method with 0/1 arguments and returns a Boolean `true` or
    // `false` result immediately.
    // 
    // Returns: the result of the method call as `true` or `false`.
    // 
    // Params:
    //   method_name: name of method to call
    //   arg_p:
    //     Optional argument to be passed to method.  If it is nullptr, then no arguments are
    //     passed.
    //   caller_p:
    //     Object that called/invoked this expression and that may await a result.  If it is
    //     `nullptr`, then there is no object that needs to be notified when this invocation
    //     is complete.
    // 
    // Notes:
    //   This is a convenience method to use instead of `method_call(name, args_pp, --)`
    //   - if more arguments or control is desired, then use it instead
    // 
    // See: method_query(), SkMethodCall<>::invoke_call(), call_destructor(), call_default_constructor()
    bool method_query(
      const ASymbol & method_name,
      SkInstance *    arg_p    = nullptr,
      SkInvokedBase * caller_p = nullptr
      );

    // Coroutine related

    // Instances do not update coroutines though their subclasses `SkMind` do.
    void abort_coroutines_on_this(eSkNotify notify_caller = SkNotify_fail);

    //---------------------------------------------------------------------------------------
    // Evaluates the coroutine call with 0 or more arguments.
    // 
    // Returns:
    //   `nullptr` if the coroutine completed immediately or an invoked coroutine if the
    //   coroutine has a deferred completion.
    //   
    // Params:  
    //   coroutine_name:
    //     name of the coroutine to call if it exists for this object. If the specified
    //     coroutine does not exist for this object it will assert if `(SKOOKUM & SK_DEBUG)`
    //     is set.
    //   args_pp:
    //     Optional pointers to object instances to use as arguments - each one present should
    //     have its reference count incremented and each defaulted/skipped argument should be
    //     a `nullptr` element.  If arg_count is 0 this is ignored
    //   arg_count:
    //     number of arguments to use in args_pp.  If it is 0 then no arguments are passed and
    //     args_pp is ignored.
    //   immediate:
    //     if true the coroutine is invoked immediately (it may not be completed, but it will
    //     be *started* immediately), if false the coroutine is scheduled for invocation on
    //     the next update.
    //   update_interval:
    //     Specifies how often the coroutine should be updated in seconds.
    //     (Default SkCall_interval_always)
    //   caller_p:
    //     object that called/invoked this expression and that may await a result - call its
    //     `pending_deferred()` method with the result of this method as necessary.  If it is
    //     `nullptr`, then there is no object that needs to be notified when this invocation
    //     is complete.
    //   updater_p:
    //    Mind object that will update this invoked coroutine as needed - generally same
    //    updater as the caller.  If nullptr the caller's updater is used and if the caller is
    //    nullptr scope_p is used.
    //             
    // See:       coroutine_invoke()
    // Author(s): Conan Reis
    SkInvokedCoroutine * coroutine_call(const ASymbol & coroutine_name, SkInstance ** args_pp, uint32_t arg_count, bool immediate = true, f32 update_interval = SkCall_interval_always, SkInvokedBase * caller_p = nullptr, SkMind * updater_p = nullptr);

    //---------------------------------------------------------------------------------------
    // Evaluates the coroutine call with 0/1 arguments.
    // 
    // Returns:
    //   `nullptr` if the coroutine completed immediately or an invoked coroutine if the
    //   coroutine has a deferred completion.
    //   
    // Params:  
    //   coroutine_name:
    //     name of the coroutine to call if it exists for this object. If the specified
    //     coroutine does not exist for this object it will assert if `(SKOOKUM & SK_DEBUG)`
    //     is set.
    //   arg_p:
    //     pointer to an object to use as an argument to the coroutine. If it is nullptr then
    //     no argument is passed.
    //   immediate:
    //     if true the coroutine is invoked immediately (it may not be completed, but it will
    //     be *started* immediately), if false the coroutine is scheduled for invocation on
    //     the next update.
    //   update_interval:
    //     Specifies how often the coroutine should be updated in seconds.
    //     (Default SkCall_interval_always)
    //   caller_p:
    //     object that called/invoked this expression and that may await a result - call its
    //     `pending_deferred()` method with the result of this method as necessary.  If it is
    //     `nullptr`, then there is no object that needs to be notified when this invocation
    //     is complete.
    //   updater_p:
    //    Mind object that will update this invoked coroutine as needed - generally same
    //    updater as the caller.  If nullptr the caller's updater is used and if the caller is
    //    nullptr scope_p is used.
    //             
    // See:       coroutine_invoke()
    // Author(s): Conan Reis
    SkInvokedCoroutine * coroutine_call(const ASymbol & coroutine_name, SkInstance * arg_p = nullptr, bool immediate = true, f32 update_interval = SkCall_interval_always, SkInvokedBase * caller_p = nullptr, SkMind * updater_p = nullptr);

    //---------------------------------------------------------------------------------------
    // Invoke the object represented by this SkInstance (e.g. if this SkInstance is an SkClosure)
    virtual SkInvokedBase * invoke(
      SkObjectBase * scope_p,
      SkInvokedBase * caller_p,
      SkInstance ** result_pp,
      const SkClosureInvokeInfo & invoke_info,
      const SkExpressionBase * invoking_expr_p) const;

    //---------------------------------------------------------------------------------------
    // Invoke the object represented by this SkInstance (e.g. if this SkInstance is an SkClosure)
    virtual void invoke_as_method(
      SkObjectBase * scope_p,
      SkInvokedBase * caller_p,
      SkInstance ** result_pp,
      const SkClosureInvokeInfo & invoke_info,
      const SkExpressionBase * invoking_expr_p) const;

  // Overriding from SkObjectBase

    virtual SkInstance *  as_new_instance() const override                            { reference(); return const_cast<SkInstance *>(this); }
    virtual eSkObjectType get_obj_type() const override                               { return SkObjectType_instance; }
    virtual SkInstance *  get_topmost_scope() const override;
    virtual SkInstance *  get_data_by_name(const ASymbol & name) const;
    virtual bool          set_data_by_name(const ASymbol & name, SkInstance * data_p);

  // Reference count related

    virtual bool is_ref_counted() const                                               { return true; }
    virtual void on_no_references();

    #ifdef SK_INSTANCE_REF_DBG
      void reference() const;
      void reference(uint32_t increment_by) const;
    #endif

  // Class Methods

    static void initialize_post_load();

    template<typename _UserType>
    static SkInstance * new_instance(SkClass * class_p, const _UserType & user_data);
    static SkInstance * new_instance(SkClass * class_p);
    static SkInstance * new_instance(SkClass * class_p, const void * user_data_p, uint32_t byte_size);
    static SkInstance * new_instance_val(SkClass * class_p, const void * user_data_p, uint32_t byte_size);
    static SkInstance * new_instance_ref(SkClass * class_p, const void * user_data_p, uint32_t byte_size);
    static SkInstance * new_instance_uninitialized(SkClass * class_p, uint32_t byte_size, void ** user_data_pp);
    static SkInstance * new_instance_uninitialized_val(SkClass * class_p, uint32_t byte_size, void ** user_data_pp);
    static SkInstance * new_instance_uninitialized_ref(SkClass * class_p, uint32_t byte_size, void ** user_data_pp);

    static AObjReusePool<SkInstance> & get_pool();

    static bool   is_data_stored_by_val(uint32_t byte_size) { return byte_size <= sizeof(tUserData); }

    // Raw data access
    // Expert terrain!
    void *        get_raw_pointer(uint32_t byte_size) { return byte_size <= sizeof(tUserData) ? &m_user_data : *((void **)&m_user_data); }
    void *        get_raw_pointer_val() { return &m_user_data; }
    void *        get_raw_pointer_ref() { return *((void **)&m_user_data); }
    static void * get_raw_pointer_val(SkInstance * obj_p) { return &obj_p->m_user_data; }
    static void * get_raw_pointer_ref(SkInstance * obj_p) { return *((void **)&obj_p->m_user_data); }

    //---------------------------------------------------------------------------------------
    // Allocate/deallocate memory for internal data
    void * allocate_raw(uint32_t byte_size)   { return byte_size <= sizeof(m_user_data) ? &m_user_data : (*((void **)&m_user_data) = AgogCore::get_app_info()->malloc(byte_size, "SkUserData")); }
    void   deallocate_raw(uint32_t byte_size) { if (byte_size > sizeof(m_user_data)) { AgogCore::get_app_info()->free(*((void **)&m_user_data)); }}

  protected:

  // User data type definition

    typedef SkUserData<2> tUserData;

  // Internal Methods

    friend class AObjReusePool<SkInstance>;  // So it can be accessed only by pool_new()

    SkInstance(SkClass * class_p);
    SkInstance(SkClass * class_p, const tUserData & user_data);

    SkInstance ** get_pool_unused_next() { return (SkInstance **)&m_user_data.m_data.m_uintptr; } // Area in this class where to store the pointer to the next unused object when not in use

  // Data Members

    // Class this is an instance of - stores methods and other info shared by instances
    SkClass * m_class_p;

    // $Note - CReis Could allow a redirection pointer - to allow direct hook-ins to already
    // existing data structures.  This would make a wasted 4 bytes for each type.

    // $Note - CReis [Memory]
    // The address of m_user_data can be used with a placement new of a data structure
    // up to pointer size * 2 bytes.
    // This includes most smart pointers like AIdxPtr<> and AIdPtr<>
    tUserData  m_user_data;

    // The global pool of SkInstances
    static AObjReusePool<SkInstance> ms_pool;

  };  // SkInstance



// Used to set an effectively *infinite* reference count so that the instance never gets
// "garbage collected" - in combination with overriding on_no_referenced() just to make sure.
const uint32_t SkInstanceUnreffed_infinite_ref_count = 1000000u;


//---------------------------------------------------------------------------------------
// Notes      SkookumScript instance object / class instance - that ignores reference
//            counting and has user data no data members.
//            
//            Mainly used as a hack for nil so that it does not need to be reference
//            counted - though it has the unnecessary overhead of the user data and
//            reference count variables.  [Mechanism to ignore reference counting is a bit
//            hacky though it saves splitting apart SkInstance into 2 or more classes and
//            rewriting all the code that references it.]
// Subclasses SkMetaClass
// Author(s)  Conan Reis
class SK_API SkInstanceUnreffed : public SkInstance
  {
  public:

  // Methods

    SK_NEW_OPERATORS(SkInstanceUnreffed);
    SkInstanceUnreffed();
    SkInstanceUnreffed(SkClass * class_p);

    // Overriding from SkInstance (overriding from ARefCountMix<>)

      virtual bool is_ref_counted() const override                                               { return false; }
      virtual void on_no_references() override;

  };


//=======================================================================================
// Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Retrieves an instance object from the dynamic pool and initializes it for use.
// This method should be used instead of 'new' because it prevents unnecessary
// allocations by reusing previously allocated objects.
// Returns:    a dynamic SkInstance
// See:        pool_delete() 
// Notes:      To 'deallocate' an object that was retrieved with this method, use
//             'pool_delete()' rather than 'delete'.
// Modifiers:   static
// Author(s):   Markus Breyer
template <typename _UserType>
inline SkInstance * SkInstance::new_instance(SkClass * class_p, const _UserType & user_data)
  {
  SkInstance * instance_p = new_instance(class_p);
  
  instance_p->set_data(user_data);

  return instance_p;
  }

#ifndef SK_IS_DLL

//---------------------------------------------------------------------------------------
// Get the global pool of SkInstances
A_FORCEINLINE AObjReusePool<SkInstance> & SkInstance::get_pool()
  {
  return ms_pool;
  }

#endif

#ifndef A_INL_IN_CPP
  #include <SkookumScript/SkInstance.inl>
#endif

