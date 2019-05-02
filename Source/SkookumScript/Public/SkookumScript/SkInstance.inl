// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript C++ library.
//
// Data structure for simplest type of object in language - instance of a
//             class without data members
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AObjReusePool.hpp>
#include <SkookumScript/SkDebug.hpp>


//=======================================================================================
// SkInstance Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default constructor - does not initialize all data members for speed.
// 
// Notes:  
//   Should probably use `new_pool()` instead.
//   ***Don't call this unless data members are manually initialized***
//   Used by get_pool().
//   
// Author(s):   Conan Reis
A_INLINE SkInstance::SkInstance()
  {
  }

//---------------------------------------------------------------------------------------
// Constructor for hot swapping the vtable - does not initialize _anything_ except vtable
A_INLINE SkInstance::SkInstance(eALeaveMemoryUnchanged)
  : ARefCountMix<SkInstance>(ALeaveMemoryUnchanged)
  {
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         class_p - Class type for this instance
// Author(s):   Conan Reis
A_INLINE SkInstance::SkInstance(
  SkClass * class_p // = nullptr
  ) :
  m_class_p(class_p)
  {
  m_ptr_id = ++ms_ptr_id_prev;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         class_p - Class type for this instance
// Arg         user_data - 32 bits of user info
// Author(s):   Conan Reis
A_INLINE SkInstance::SkInstance(
  SkClass * class_p,
  const tUserData & user_data
  ) :
  m_class_p(class_p),
  m_user_data(user_data)
  {
  m_ptr_id = ++ms_ptr_id_prev;
  }

//---------------------------------------------------------------------------------------
// Destructor
// Modifiers:   virtual - overridden from SkObjectBase
// Author(s):   Conan Reis
A_INLINE SkInstance::~SkInstance()
  {
  m_ptr_id = AIdPtr_null;
  }

//---------------------------------------------------------------------------------------
// Returns the class of the instance
// Returns:    class of the instance (may be a SkClass or a SkActorClass)
// Author(s):   Conan Reis
A_INLINE SkClass * SkInstance::get_class() const
  {
  return m_class_p;
  }

//---------------------------------------------------------------------------------------
// Sets the class of the instance
A_INLINE void SkInstance::set_class(SkClass * class_p)
  {
  m_class_p = class_p;
  }

//---------------------------------------------------------------------------------------
// Returns the 'this' using this object as the scope.
// Returns:    itself
// Modifiers:   virtual from SkObjectBase
// Author(s):   Conan Reis
A_INLINE SkInstance * SkInstance::get_topmost_scope() const
  {
  return const_cast<SkInstance *>(this);
  }

//---------------------------------------------------------------------------------------
// Returns true if class type, false if not
// Modifiers:   virtual
// Author(s):   Conan Reis
A_INLINE bool SkInstance::is_metaclass() const
  {
  return false;
  }

//---------------------------------------------------------------------------------------
// Cleans up the instance.  Calls its destructor (if it has one) and adds it
//             to the reuse pool.
// Modifiers:   virtual - overridden from ARefCountMix<>
// Author(s):   Conan Reis
A_INLINE void SkInstance::on_no_references()
  {
  // Call its destructor - if it has one
  call_destructor();
  delete_this();
  }

//---------------------------------------------------------------------------------------
//  Frees up an instance and puts it into the dynamic pool ready for its next
//              use.  This should be used instead of 'delete' because it prevents
//              unnecessary deallocations by saving previously allocated objects.
// Arg          str_ref_p - pointer to instance to free up and put into the dynamic pool.
// Examples:    called by dereference()
// See:         pool_new(), dereference()
// Notes:       To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:    virtual
// Author(s):    Conan Reis
A_INLINE void SkInstance::delete_this()
  {
  //A_DPRINT("SkInstance::pool_delete('%s', 0x%p) - id: %u\n", m_class_p->get_name_cstr(), m_user_data, m_ptr_id);

  m_ptr_id = AIdPtr_null;

  get_pool().recycle(this);
  }

//---------------------------------------------------------------------------------------
A_INLINE void SkInstance::method_call(
  const ASymbol & method_name,
  SkInstance *    arg_p,     // = nullptr
  SkInstance **   result_pp, // = nullptr
  SkInvokedBase * caller_p   // = nullptr
  )
  {
  method_call(method_name, &arg_p, arg_p ? 1u : 0u, result_pp, caller_p);
  }

//---------------------------------------------------------------------------------------
A_INLINE SkInvokedCoroutine * SkInstance::coroutine_call(
  const ASymbol & coroutine_name,
  SkInstance *    arg_p,           // = nullptr
  bool            immediate,       // = true
  f32             update_interval, // = SkCall_interval_always
  SkInvokedBase * caller_p,        // = nullptr
  SkMind *        updater_p        // = nullptr
  )
  {
  return coroutine_call(
    coroutine_name, &arg_p, arg_p ? 1u : 0u, immediate, update_interval, caller_p, updater_p);
  }

//---------------------------------------------------------------------------------------
// Retrieves an instance object from the dynamic pool and initializes it for
//             use.  This method should be used instead of 'new' because it prevents
//             unnecessary allocations by reusing previously allocated objects.
// Returns:    a dynamic SkInstance
// See:        pool_delete() 
// Notes:      To 'deallocate' an object that was retrieved with this method, use
//             'pool_delete()' rather than 'delete'.
// Modifiers:   static
// Author(s):   Conan Reis
A_INLINE SkInstance * SkInstance::new_instance(SkClass * class_p)
  {
  SkInstance * instance_p = get_pool().allocate();

  instance_p->m_class_p   = class_p;
  instance_p->m_ref_count = 1u;
  instance_p->m_ptr_id    = ++ms_ptr_id_prev;

  //A_DPRINT("SkInstance::new_instance('%s') - id: %u\n", class_p->get_name_cstr(), instance_p->m_ptr_id);

  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type 
// (stored either by value or reference depending on byte size)
A_INLINE SkInstance * SkInstance::new_instance_uninitialized(SkClass * class_p, uint32_t byte_size, void ** user_data_pp)
  {
  SkInstance * instance_p = new_instance(class_p);
  void * user_data_p = &instance_p->m_user_data;
  if (byte_size > sizeof(tUserData))
    {
    user_data_p = AgogCore::get_app_info()->malloc(byte_size, "SkUserData");
    *((void **)&instance_p->m_user_data) = user_data_p;
    }
  *user_data_pp = user_data_p;
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type (stored inside the instance)
A_INLINE SkInstance * SkInstance::new_instance_uninitialized_val(SkClass * class_p, uint32_t byte_size, void ** user_data_pp)
  {
  SK_ASSERTX(byte_size <= sizeof(tUserData), "Data must fit inside m_user_data!");
  SkInstance * instance_p = new_instance(class_p);
  *user_data_pp = &instance_p->m_user_data;
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type (allocated from heap and stored by reference)
A_INLINE SkInstance * SkInstance::new_instance_uninitialized_ref(SkClass * class_p, uint32_t byte_size, void ** user_data_pp)
  {
  SK_ASSERTX(byte_size > sizeof(tUserData), "Use new_instance_val instead!");
  SkInstance * instance_p = new_instance(class_p);
  void * user_data_p = AgogCore::get_app_info()->malloc(byte_size, "SkUserData");
  *((void **)&instance_p->m_user_data) = user_data_p;
  *user_data_pp = user_data_p;
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type with given data 
// (stored either by value or reference depending on byte size)
A_INLINE SkInstance * SkInstance::new_instance(SkClass * class_p, const void * user_data_p, uint32_t byte_size)
  {
  void * new_user_data_p;
  SkInstance * instance_p = new_instance_uninitialized(class_p, byte_size, &new_user_data_p);
  ::memcpy(new_user_data_p, user_data_p, byte_size);
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type with given data (stored inside the instance)
A_INLINE SkInstance * SkInstance::new_instance_val(SkClass * class_p, const void * user_data_p, uint32_t byte_size)
  {
  void * new_user_data_p;
  SkInstance * instance_p = new_instance_uninitialized_val(class_p, byte_size, &new_user_data_p);
  ::memcpy(new_user_data_p, user_data_p, byte_size);
  return instance_p;
  }

//---------------------------------------------------------------------------------------
// Allocate (from pool) and construct a new SkInstance of given type with given data (allocated from heap and stored by reference)
A_INLINE SkInstance * SkInstance::new_instance_ref(SkClass * class_p, const void * user_data_p, uint32_t byte_size)
  {
  void * new_user_data_p;
  SkInstance * instance_p = new_instance_uninitialized_ref(class_p, byte_size, &new_user_data_p);
  ::memcpy(new_user_data_p, user_data_p, byte_size);
  return instance_p;
  }

//=======================================================================================
// SkInstanceUnreffed Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Default constructor - does not initialize all data members for speed
// Notes:      ***Don't call this unless data members are manually initialized***
//             Used by get_pool().
// Returns:    itself
// Author(s):   Conan Reis
A_INLINE SkInstanceUnreffed::SkInstanceUnreffed()
  {
  // Set ref count to essentially infinite - and on_no_references() modifiied just in case.
  m_ref_count = SkInstanceUnreffed_infinite_ref_count;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         class_p - Class type for this instance
// Author(s):   Conan Reis
A_INLINE SkInstanceUnreffed::SkInstanceUnreffed(
  SkClass * class_p // = nullptr
  ) :
  SkInstance(class_p)
  {
  // Set ref count to essentially infinite - and on_no_references() modifiied just in case.
  m_ref_count = SkInstanceUnreffed_infinite_ref_count;
  }

//---------------------------------------------------------------------------------------
// Would cleans up the instance though just resets reference count to a high
//             number since it ignores referencing/dereferencing.
// Modifiers:   virtual - overridden from SkInstance (which overrode ARefCountMix)
// Author(s):   Conan Reis
A_INLINE void SkInstanceUnreffed::on_no_references()
  {
  // Wow a million decrements!  Just reset it.
  m_ref_count = SkInstanceUnreffed_infinite_ref_count;
  }
