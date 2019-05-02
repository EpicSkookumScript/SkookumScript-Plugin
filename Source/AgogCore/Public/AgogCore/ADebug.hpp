// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ADebug class declaration header
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AFunctionArgBase.hpp>
#include <AgogCore/AException.hpp>
#include <stdio.h>
#include <stdarg.h>
                
//=======================================================================================
// Global Macros / Defines
//=======================================================================================

// If this define is present, it indicates that exceptions may be used
//#if defined(_CPPUNWIND) || defined(__TEMPLATES__)  // These defines are MSVC specific
//  #define A_EXCEPTION_CAPABLE
//#endif


//---------------------------------------------------------------------------------------
// If 'A_EXTRA_CHECK' is defined, extra checking/validity code will be enabled in the
// compile - range boundaries tests, check for valid arguments, ensure non-nullptr, etc.
//
// Try to use the 'A_EXTRA_CHECK' define rather than the '_DEBUG' define, then extra
// checks can still be performed during non-debug builds if desired.
//
// Also try to use the 'A_NO_EXTRA_CHECK' define rather than the 'NDEBUG' define or even
// better check for the absence of 'A_EXTRA_CHECK'.
//
//   Default:  if '_DEBUG' is defined, 'A_EXTRA_CHECK' is defined.
//   Always Check:  When '_DEBUG' is not defined and extra checking is desired, just
//     define 'A_EXTRA_CHECK' as a compiler preprocessor directive.
//   Never Check:  If 'A_NO_EXTRA_CHECK' is defined as a compiler preprocessor directive,
//     'A_EXTRA_CHECK' will not be defined even if '_DEBUG' is defined.
#if defined(_DEBUG) && !defined(A_NO_EXTRA_CHECK) && !defined(A_EXTRA_CHECK)
  // Enable extra checking
  #define A_EXTRA_CHECK
#endif

#ifndef A_EXTRA_CHECK
  #define A_NO_EXTRA_CHECK
#endif


//---------------------------------------------------------------------------------------
// If 'A_BOUNDS_CHECK' is defined, enable bounds checking on indexable objects - i.e.
//   ensure that index value arguments are within valid index ranges.
//
//   Default:  if 'A_EXTRA_CHECK' is defined, 'A_BOUNDS_CHECK' is defined.
//   Always Check:  When 'A_EXTRA_CHECK' is not defined and extra checking is desired,
//     just define 'A_BOUNDS_CHECK' as a compiler preprocessor directive.
//   Never Check:  If 'A_NO_BOUNDS_CHECK' is defined as a compiler preprocessor
//     directive, 'A_BOUNDS_CHECK' will not be defined even if 'A_EXTRA_CHECK' is defined.
#if defined(A_EXTRA_CHECK) && !defined(A_NO_BOUNDS_CHECK)
  #define A_BOUNDS_CHECK
#endif


//---------------------------------------------------------------------------------------
// Indicates whether exceptions are thrown on errors.  If A_NO_GLOBAL_EXCEPTION_CATCH is
// defined, then a break point is made on an error instead.
#if !defined(A_NO_GLOBAL_EXCEPTION_CATCH)
  #define A_THROW_EXCEPTION
#endif


//---------------------------------------------------------------------------------------
// If 'AEX_CATCH_ALL' is defined, all unknown exceptions are caught after they are thrown.
// An 'unknown' exception is any exception that is not an Agog exception - i.e. not
// derived from AExceptionBase.  Common unknown exceptions are: Access Violations
// (attempt to read from a nullptr address or invalid memory), Data Type Misalignment,
// floating point division by zero, etc.
//
// If the exception catch all is defined, then the application will catch these 'unknown'
// exceptions and try to recover gracefully and keep the app running - usually with a
// prompt to the user that an 'Unknown error' has occurred before continuing.
//
// Some compilers will break in the code where an 'unknown' exception is initially thrown
// which is very handy.  Some compilers will only do this if the exception catch all is
// *not* in place - so you can break at the problem point, but the app will likely not
// recover gracefully.
//
// MSDev 7 and up can set Debug->Exceptions...->Win32 Exceptions->Access Violation(etc)->
// "When the exception is thrown" to "Break in the debugger" so AEX_CATCH_ALL can be
// defined and it will both break at the exception point and then recover.
#define AEX_CATCH_ALL


//---------------------------------------------------------------------------------------
// If 'ADEBUG_COMMANDS' is defined, 'debug' commands are enabled - A_DPRINT(),
// A_DBREAK(), and ADEBUG_CODE().
//
//   Default:  if 'A_EXTRA_CHECK' is defined, 'ADEBUG_COMMANDS' is defined.
//   Always Check:  When 'A_EXTRA_CHECK' is not defined and extra checking is desired,
//     just define 'ADEBUG_COMMANDS' as a compiler preprocessor directive.
//   Never Check:  If 'ADEBUG_NO_COMMANDS' is defined as a compiler preprocessor
//     directive, 'ADEBUG_COMMANDS' will not be defined even if 'A_EXTRA_CHECK' is defined.
#if defined(A_EXTRA_CHECK) && !defined(ADEBUG_NO_COMMANDS)
  #define ADEBUG_COMMANDS
#endif

#ifdef ADEBUG_COMMANDS  // Compile debug commands

  #define ADEBUG_CODE(_code)  _code
  #define A_DBREAK()          A_BREAK()
  // $Revisit - CReis Consider variadic macros with __VA_ARGS__, however by the standard
  // __VA_ARGS__ must have at least one argument.
  #define A_DPRINT            ADebug::print_format

#else  // Don't compile debug code

  // Note, using (void(0)) avoids warning that ; is used without an expression.
  // Also, 1?0: is a mechanism to tell the compiler to optimize out the next statement
  // $Revisit - CReis Microsoft has a compiler intrinsic called __noop specifically
  // designed for this case - investigate and see if it is any better, however these
  // current mechanisms seem more portable (though they look strange).

  #define ADEBUG_CODE(_code)  (void(0))
  #define A_DBREAK()          (void(0))
  #define A_DPRINT            true ? void(0) : ADebug::print_format

#endif


//---------------------------------------------------------------------------------------
// This is a macro for the common arguments for AErrMsg objects.
// The main point of the A_ERR_ARGS macro is to add the source function, file and line
// number where an exception object is constructed.  See the notes on mFuncName,
// m_source_path_p, and m_source_line in the AErrMsg structure found lower this file. 
#define A_ERR_ARGS  __FUNCSIG__, __FILE__, __LINE__



//---------------------------------------------------------------------------------------
#ifdef A_THROW_EXCEPTION
  #define A_ERROR_ACTION(_ExClass, _err_id, _action)   throw AEx<_ExClass>(_err_id, _action);
#else
  #define A_ERROR_ACTION(_ExClass, _err_id, _action)   A_BREAK();
#endif

//---------------------------------------------------------------------------------------
// Macros for throwing exceptions - these should be used rather than using 'throw'
// directly since they can stringize the class name and err_id and they can be #ifdef-ed
// out in various builds if desired - to be replaced with abort() or simply blank code
#define A_ERROR(_error_msg, _err_id, _ExClass) \
  { \
  static bool _test = true; \
  if (_test) \
    { \
    eAErrAction _action; \
    if (ADebug::resolve_error(AErrMsg((_error_msg), nullptr, A_ERR_ARGS, _err_id), &_action, &_test)) \
      A_BREAK(); \
    if (_action != AErrAction_ignore) \
      A_ERROR_ACTION(_ExClass, _err_id, _action) \
    } \
  }

// $Revisit - CReis Since asserts now automatically generate the name of function where
// they occur, go through older code and remove redundant function name strings from
// error messages.

#define A_ERRORX(_error_msg)              A_ERROR(_error_msg, AErrId_generic, ADebug)
#define A_ERROR_OS(_error_msg, _ExClass)  A_ERROR(ADebugOS::get_last_os_error(_error_msg), AErrId_os_error, _ExClass)
#define A_ERROR_MEMORY(_ExClass)          A_ERROR("Unable to allocate memory", AErrId_low_memory, _ExClass)

#ifdef A_EXTRA_CHECK  // Checking build
  #define A_DERROR(_error_msg, _err_id, _ExClass)  A_ERROR(_error_msg, _err_id, _ExClass)
  #define A_DERRORX(_boolean_exp)                  A_ERRORX(_boolean_exp)
  #define A_DERROR_OS(_error_msg, _ExClass)        A_ERROR_OS(_error_msg, _ExClass)
  #define A_DERROR_MEMORY(_ExClass)                A_ERROR_MEMORY(_ExClass)
#else                 // Non-checked build
  #define A_DERROR(_error_msg, _err_id, _ExClass)  (void(0))
  #define A_DERRORX(_boolean_exp)                  (void(0))
  #define A_DERROR_OS(_error_msg, _ExClass)        (void(0))
  #define A_DERROR_MEMORY(_ExClass)                (void(0))
#endif


// Assertions
#define A_VERIFY(_boolean_expr, _error_msg, _err_id, _ExClass) \
  { \
  static bool _test = true; \
  if (_test && !(_boolean_expr)) \
    { \
    eAErrAction _action; \
    if (ADebug::resolve_error(AErrMsg((_error_msg), "Test failed: " #_boolean_expr, A_ERR_ARGS, _err_id), &_action, &_test)) \
      A_BREAK(); \
    if (_action != AErrAction_ignore) \
      A_ERROR_ACTION(_ExClass, _err_id, _action) \
    } \
  }

#define A_VERIFY_NO_THROW(_boolean_expr, _error_msg, _err_id, _ExClass) \
  { \
  static bool _test = true; \
  if (_test && !(_boolean_expr)) \
    { \
    eAErrAction _action; \
    if (ADebug::resolve_error(AErrMsg((_error_msg), "Test failed: " #_boolean_expr, A_ERR_ARGS, _err_id), &_action, &_test) || _action != AErrAction_ignore) \
      A_BREAK(); \
    } \
  }

#define A_VERIFYX(_boolean_exp, _error_msg)                       A_VERIFY(_boolean_exp, _error_msg, AErrId_generic, ADebug)
#define A_VERIFYX_NO_THROW(_boolean_exp, _error_msg)              A_VERIFY_NO_THROW(_boolean_exp, _error_msg, AErrId_generic, ADebug)
#define A_VERIFY_OS(_boolean_exp, _error_msg, _ExClass)           A_VERIFY(_boolean_exp, ADebugOS::get_last_os_error(_error_msg), AErrId_os_error, _ExClass)
#define A_VERIFY_OS_NO_THROW(_boolean_exp, _error_msg, _ExClass)  A_VERIFY_NO_THROW(_boolean_exp, ADebugOS::get_last_os_error(_error_msg), AErrId_os_error, _ExClass)
#define A_VERIFY_MEMORY(_boolean_exp, _ExClass)                   A_VERIFY(_boolean_exp, "Unable to allocate memory", AErrId_low_memory, _ExClass)

#ifdef A_EXTRA_CHECK  // Checked build
  #define A_ASSERT(_boolean_exp, _error_msg, _err_id, _ExClass)  A_VERIFY(_boolean_exp, _error_msg, _err_id, _ExClass)
  #define A_ASSERTX(_boolean_exp, _error_msg)                    A_VERIFY(_boolean_exp, _error_msg, AErrId_generic, ADebug)
  #define A_ASSERT_OS(_boolean_exp, _error_msg, _ExClass)        A_VERIFY_OS(_boolean_exp, _error_msg, _ExClass)
  #define A_ASSERT_MEMORY(_boolean_exp, _ExClass)                A_VERIFY_MEMORY(_boolean_exp, _ExClass)
#else                 // Non-checked build
  #define A_ASSERT(_boolean_exp, _error_msg, _err_id, _ExClass)  (void(0))
  #define A_ASSERTX(_boolean_exp, _error_msg)                   (void(0))
  #define A_ASSERT_OS(_boolean_exp, _error_msg, _ExClass)       (void(0))
  #define A_ASSERT_MEMORY(_boolean_exp, _ExClass)               (void(0))
#endif

#ifdef A_MAD_CHECK  // Fussy checked build
  #define A_MAD_ASSERTX(_boolean_exp, _error_msg)              A_VERIFY(_boolean_exp, _error_msg, AErrId_generic, ADebug)
#else
  #define A_MAD_ASSERTX(_boolean_exp, _error_msg)              (void(0))
#endif

//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class AString;

// Standard levels / choices for errors/exceptions
// $Note - CReis In the future, custom choices should be available.
enum eAErrLevel
  {
  // Exception handled - could log exception, state should be ok afterwards
  // *No notification to user*
  AErrLevel_internal,

  // Potentially ignorable exception - state may be ok or instable afterwards.
  // Ok/Ignore, Ignore All, [x]Break
  AErrLevel_notify,

  // Potentially recoverable exception - state may be ok or instable afterwards.
  // Continue, Retry, Quit, [x]Break
  AErrLevel_warning,
  
  // Potentially recoverable / ignorable exception - state may be ok or instable afterwards.
  // Continue, Retry, Quit, Ignore, Ignore All, [x]Break
  AErrLevel_error,

  // Non-recoverable exception causing program termination.
  // Quit, [x]Break
  AErrLevel_fatal
  };

//---------------------------------------------------------------------------------------
// Describes information (both high and low level) about an exception.
// Used by AErrorOutputBase::determine_choice().
struct A_API AErrMsg
  {
  // Common Methods
    
    // Common constructor

      AErrMsg(const AString & desc_high, eAErrLevel err_level = AErrLevel_error, const char * title_p = nullptr, const char * desc_low_p = nullptr);
      AErrMsg(const char * desc_high_p, eAErrLevel err_level = AErrLevel_error, const char * title_p = nullptr, const char * desc_low_p = nullptr);

    // Sort of a copy constructor, but adds preprocessor information

      AErrMsg(const AErrMsg & err_msg, const char * desc_low_p, const char * func_name_p, const char * source_path_p, uint source_line, uint err_id);

  // Data Members

    // Short title - if empty, a AString based on the exception level is used in
    // AErrorOutputBase::determine_choice()
    const char * m_title_p;

    // High level description - i.e. user-friendly.  It should describe any likely causes
    // of the exception, how it could be prevented in the future, possible support contact,
    // each of the choices available and any actions that should be taken by the user pior
    // to making a selection or following a selection.
    // If m_desc_high is empty, a default AString based on the exception level is used in
    // AErrorOutputBase::determine_choice().
    // Note that a AString does not have to be used to manage the description memory since
    // The AErrMsg object is used immediately before any passed in AString goes out of scope.
    const char * m_desc_high_p;
    
    // Low level description - i.e. possibly useful to the programmer, not already
    // contained in the high level description.
    // Note that a AString does not have to be used to manage the description memory since
    // The AErrMsg object is used immediately before any passed in AString goes out of scope.
    const char * m_desc_low_p;

    // Level of error
    eAErrLevel m_err_level;

    // numeric id identifying the specific exception - usually from an enumerated type
    uint32_t m_err_id;

    // Function name of error origin using the __FUNCSIG__ macro *** MSVC7 specific ***
    // Or other optional low-level description.
    const char * m_func_name_p;

    // Source file location of the exception, usually where the exception is constructed
    // or thrown.  It is usually created using the preprocessor macro __FILE__.
    // If it is nullptr, then the file is not specified and m_source line is ignored.
    // Efficiency note: using __FILE__ numerous times in a given source file will only
    // end up storing one AString in the compiled version of the application rather than
    // a copy each time it is used.  This holds for any AString literal.
    const char * m_source_path_p;

    // Source file line number of the exception, usually where the exception is constructed
    // or thrown.  It is usually created using the preprocessor macro __LINE__.  Only
    // considered to be valid if m_source_path_p is not nullptr.
    uint32_t m_source_line;

  };  // ExInfo

//---------------------------------------------------------------------------------------
// Notes      Used for abstraction of platform output.  Should display an appropriate
//            message based on str and level and return an error action based on user
//            input or a default action. 
// Subclasses AErrPopUp, AErrorDialog
// Author(s)  Conan Reis
class A_API AErrorOutputBase
  {
  public:
  // Common Methods

    virtual ~AErrorOutputBase();

  // Modifying Methods

    virtual bool determine_choice(const AErrMsg & info, eAErrAction * action_p) = 0;

  };  // AErrorOutputBase


// Shorthand
typedef AFunctionArgBase<const AString &> tAPrintFunc;
typedef AFunctionArgBase<AString *>       tAContextFunc;

//---------------------------------------------------------------------------------------
// Performance counter that prints time elapsed since construction in its destructor
#if defined(A_PLAT_PC) && defined(A_EXTRA_CHECK)

  class AScopedDebugPrintPerfCounter
    {
    public:
      AScopedDebugPrintPerfCounter(const char * label_p);
      ~AScopedDebugPrintPerfCounter();

    protected:
      const char * m_label_p;
      uint64_t     m_start_time;
    };

#else

  class AScopedDebugPrintPerfCounter
    {
    public:
      AScopedDebugPrintPerfCounter(const char *) {}
    };

#endif

//---------------------------------------------------------------------------------------
// Author   Conan Reis
class A_API ADebug
  {
  public:

  // Class Methods

    static void    initialize();
    static void    deinitialize();

    static void    info();
    static bool    context_append(AString * str_p);
    static AString context_string();
    static bool    is_nested_error();
    static bool    is_debugging();
    static void    print(const char * cstr_p, bool call_print_funcs_b = true);
    static void    print(const AString & str, bool call_print_funcs_b = true);
    static void    print_format(const char * format_cstr_p, ...);
    static void    print_args(const char * format_cstr_p, va_list args);
    static void    print_std(const AString & str);
    static void    register_context_func(tAContextFunc * context_func_p);
    static void    register_print_func(tAPrintFunc * print_func_p);
    static void    register_print_std();
    static void    unregister_context_func(tAContextFunc * context_func_p);
    static void    unregister_print_func(tAPrintFunc * print_func_p);
    static bool    resolve_error(const AErrMsg & info, eAErrAction * action_p, bool * test_again_p = nullptr);

    // Future
    //   Logging function
    //   Attended / Unattended operation setting

  protected:

  // Internal Class Methods

    static bool determine_choice(const AErrMsg & info, eAErrAction * action_p, bool nested_err);

  // Class Data Members

    // If > 0 then running inside a call to `resolve_error()`.
    // Values greater than 1 indicate level of nested calls to `resolve_error()`.
    static uint32_t ms_resolve_error_depth;

  };  // ADebug
