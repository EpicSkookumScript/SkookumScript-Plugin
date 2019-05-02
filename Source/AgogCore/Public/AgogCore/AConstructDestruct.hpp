// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Constructor and Destructor class definition header
// Notes:          All of the methods are inline, so there is no cpp file.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================
      
//---------------------------------------------------------------------------------------
// This class is used to give a sort of constructor and destructor for classes or global
// initializations/deinitializations.  It is used by making a class with one (or more) of
// its class data members (i.e. static) a AConstructDestruct class.  It is passed two
// static zero argument functions.  The first (the "class constructor) is invoked
// immediately upon construction of the AConstructDestruct class at program start up
// - even before WinMain(), so be careful in any assumptions about what is and what
// is not yet initialized, and the second (the "class destructor") is invoked when the
// AConstructDestruct class is destructed at the application exit.
//
// There is no technical reason that prevents this class from being used non-statically,
// but there is also no benefit for it either.  Due to this and also due to simplicity,
// AConstructDestruct works with function pointers rather than function objects derived
// from the AFunctionBase class.
class AConstructDestruct
  {
  public:
  // Common methods

    AConstructDestruct(void (*constructor_p)() = nullptr, void (*destructor_p)() = nullptr);
    ~AConstructDestruct();

  // Accessor Methods

    void set_destructor(void (*destructor_p)());

  protected:
  // Data Members

    void (*m_class_destructor_p)();

  // Forbidden methods

    // Since the constructor function pointer is not stored, there is not enough info to copy
    AConstructDestruct(const AConstructDestruct & cc)             {}
    AConstructDestruct & operator=(const AConstructDestruct & cc) { return *this; }
  };


//---------------------------------------------------------------------------------------
// Simple wrapper to set a type in its destructor. Useful to guarantee setting a value
// regardless of the return point of a method.
// 
// #Examples
// 
//   // ASetOnReturn<> streamlines returns:
//   void do_stuff1(int * idx_p)
//     {
//     int pos = 0;
//     ASetOnReturn<int> set_idx(&pos, idx_p);
//     
//     if (!test1(&pos))
//       {
//       return;
//       }
//     
//     do_things1(&pos);
//     
//     if (!test2(&pos))
//       {
//       return;
//       }
//     
//     do_things2(&pos);
//     
//     if (!test3(&pos))
//       {
//       return;
//       }
//     
//     do_things3(&pos);
//     }  
//       
// 
//   // Instead of this:
//   void do_stuff2(int * idx_p)
//     {
//     int pos = 0;
//     
//     if (!test1(&pos))
//       {
//       if (idx_p)
//         {
//         *idx_p = pos;
//         }
//         
//       return;
//       }
//     
//     do_things1(&pos);
//     
//     if (!test2(&pos))
//       {
//       if (idx_p)
//         {
//         *idx_p = pos;
//         }
//         
//       return;
//       }
//     
//     do_things2(&pos);
//     
//     if (!test3(&pos))
//       {
//       if (idx_p)
//         {
//         *idx_p = pos;
//         }
//         
//       return;
//       }
//     
//     do_things3(&pos);
//     
//     if (idx_p)
//       {
//       *idx_p = pos;
//       }
//     }  
// 
// 
//   // And helps prevent deep nesting like this:
//   void do_stuff2(int * idx_p)
//     {
//     int pos = 0;
//     
//     if (test1(&pos))
//       {
//       do_things1(&pos);
//     
//       if (test2(&pos))
//         {
//         do_things2(&pos);
//     
//         if (test3(&pos))
//           {
//           do_things3(&pos);
//           }
//         }
//       }
//       
//     if (idx_p)
//       {
//       *idx_p = pos;
//       }
//     }  
template<class _Type>
class ASetOnReturn
  {
  public:
    _Type * m_source_p;
    _Type * m_dest_p;

    ASetOnReturn(_Type * source_p, _Type * dest_p): m_source_p(source_p), m_dest_p(dest_p)
      {}

    ~ASetOnReturn()
      {
      if (m_source_p && m_dest_p)
        {
        *m_dest_p = *m_source_p;
        }
      }

  };


//---------------------------------------------------------------------------------------
// Prints out constructor message when constructed and destructor message when destructed.
// Useful to note when scopes are entered or exited or when certain objects are created
// or destructed.
// If you only want this to be run when debugging and A_EXTRA_CHECK is enabled use the
// A_DSCOPE_LOG() macro.
class AScopeLogger
  {
  public:
    const char * m_dtor_msg_p;

    AScopeLogger(const char * ctor_msg_p, const char * dtor_msg_p) :
      m_dtor_msg_p(dtor_msg_p)
      {
      if (ctor_msg_p)
        {
        ADebug::print(ctor_msg_p);
        }
      }

    ~AScopeLogger()
      {
      if (m_dtor_msg_p)
        {
        ADebug::print(m_dtor_msg_p);
        m_dtor_msg_p = nullptr;
        }
      }

  };


#ifdef A_EXTRA_CHECK  // Checked build
  #define A_DSCOPE_LOG(_var_name, _ctor_msg_p, _dtor_msg_p)  AScopeLogger _var_name(_ctor_msg_p, _dtor_msg_p)
#else                 // Non-checked build
  #define A_DSCOPE_LOG(_var_name, _ctor_msg_p, _dtor_msg_p)  int _var_name
#endif

#ifdef A_EXTRA_CHECK  // Checked build
  #define A_DSINGLETON_GUARD  \
    static AScopeLogger _singleton_guard(A_SOURCE_FUNC_STR " - ctor\n", A_SOURCE_FUNC_STR " - dtor\n");  \
    if (_singleton_guard.m_dtor_msg_p == nullptr) { A_ERRORX(A_SOURCE_FUNC_STR " used after cleaned up!") }
#else                 // Non-checked build
  #define A_DSINGLETON_GUARD  (void(0))
#endif



//=======================================================================================
// Inline Functions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Default Constructor
// Returns:    itself
// Arg         constructor_p - class constructor / initialization code to be invoked
//             immediately or to ignore if nullptr.  (Default nullptr)
// Arg         destructor_p - class destructor / deinitialization code to invoke at the
//             destruction of this instance or to ignore if nullptr.  (Default nullptr)
// Author(s):   Conan Reis
inline AConstructDestruct::AConstructDestruct(
  void (*constructor_p)(), // = nullptr
  void (*destructor_p)()   // = nullptr
  ) :
  m_class_destructor_p(destructor_p)
  {
  if (constructor_p)
    {
    (*constructor_p)();
    }
  }

//---------------------------------------------------------------------------------------
// Destructor - invokes the class destructor / deinitialization code if any.
// Author(s):   Conan Reis
inline AConstructDestruct::~AConstructDestruct()
  {
  if (m_class_destructor_p)
    {
    (*m_class_destructor_p)();
    }
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Sets or clears the class destructor / deinitialization function pointer.
// Arg         destructor_p - class destructor / deinitialization code to invoke at the
//             destruction of this instance or to ignore if nullptr.
// Author(s):   Conan Reis
inline void AConstructDestruct::set_destructor(void (*destructor_p)())
  {
  m_class_destructor_p = destructor_p;
  }
