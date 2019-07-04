// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Common stuff for AgogCore
// Notes:          Simple types, macros, etc. for ease of use, readability, and
//              type checking.
//              *** ALL code should use these types, etc. wherever appropriate ***
//              IMPORTANT:  Aspects of this file may be platform specific
//=======================================================================================

#pragma once

//=======================================================================================
// Includes  (Additional include bottom of file)
//=======================================================================================

// If AGOG_EXT_INCLUDE is defined then include AgogExtHook.hpp for custom defines such
// as platform types, asserts, user breaks etc.
//#if defined(AGOG_EXT_INCLUDE) && !defined(AGOG_EXT_IGNORE)
  #include <AgogCore/_AgogExtHook.hpp>
//#endif

#include <stdint.h>  // For sized integer types, minimums and maximums: int##_t, uint##_t, INT##_MAX, UINT##_MAX
#include <limits.h>  // integer minimums and maximums: INT_MAX, UINT_MAX
#include <float.h>   // float minimums and maximums: FLT_DIG, FLT_MAX, FLT_EPSILON, DBL_DIG
#include <stddef.h>  // size_t on OS X
#include <new>       // alloca
#include <memory.h>  // memcpy

// Additional includes at end of file


//=======================================================================================
// Global Macros / Defines
//=======================================================================================

#define A_COPYRIGHT_TEXT  "Copyright 1998-2019 Epic Games, Inc. All Rights Reserved."

//---------------------------------------------------------------------------------------
// DLL API
#ifdef AGOGCORE_API // Use AGOGCORE_API passed down from UE4 build system if present
  // AGOGCORE_API is either DLLIMPORT, DLLEXPORT or empty - map to AgogCore macros
  #ifndef DLLIMPORT
    #define DLLIMPORT A_DLLIMPORT
    #define DLLEXPORT A_DLLEXPORT
  #endif
  #define A_API AGOGCORE_API
#elif defined(A_IS_DLL) // otherwise, DLL linkage control via A_IS_DLL & A_IS_BUILDING_SELF
  // Further down, we'll set A_DLLIMPORT and A_DLLEXPORT to platform-specific values
  #ifdef A_IS_BUILDING_SELF
    #define A_API A_DLLEXPORT
  #else
    #define A_API A_DLLIMPORT
  #endif
#else // AgogCore is a static library
  #define A_API
#endif

//---------------------------------------------------------------------------------------
// Platform Defines
#if !defined(A_PLAT_PC32) && !defined(A_PLAT_PC64) \
  && !defined(A_PLAT_X360) && !defined(A_PLAT_X_ONE) \
  && !defined(A_PLAT_PS3) && !defined(A_PLAT_PS4) \
  && !defined(A_PLAT_WIIU) \
  && !defined(A_PLAT_iOS) && !defined(A_PLAT_tvOS) && !defined(A_PLAT_OSX) \
  && !defined(A_PLAT_ANDROID) && !defined(A_PLAT_LINUX64)
  #ifdef _WIN32
    // Assume it is a Windows PC platform (either 32-bit or 64-bit)
    #ifdef _WIN64
      // Assume 64-bit PC platform
      #define A_PLAT_PC64
    #else
      // Assume 32-bit PC platform
      #define A_PLAT_PC32
    #endif
  #else
    #error No known platform type is defined (A_PLAT_PC32, A_PLAT_PC64, A_PLAT_PS3, A_PLAT_PS4, A_PLAT_X360, A_PLAT_X_ONE, A_PLAT_WIIU, A_PLAT_iOS, A_PLAT_tvOS, A_PLAT_OSX or A_PLAT_LINUX64)!
  #endif
#else
#endif


//---------------------------------------------------------------------------------------
// Microsoft PC 64-bit
#ifdef A_PLAT_PC64

  #define A_PLAT_STR_ID   "PC64"
  #define A_PLAT_STR_DESC "Windows PC 64-bit"
  #define A_PLAT_PC
  #define A_BITS64

#endif


//---------------------------------------------------------------------------------------
// Microsoft PC 32-bit
#ifdef A_PLAT_PC32

  #define A_PLAT_STR_ID   "PC32"
  #define A_PLAT_STR_DESC "Windows PC 32-bit"
  #define A_PLAT_PC
    
#endif


//---------------------------------------------------------------------------------------
// PC (Microsoft Windows) Platform - common to both A_PLAT_PC32 and A_PLAT_PC64
#ifdef A_PLAT_PC

  #define AGOG_LITTLE_ENDIAN_HOST   1    // PC is little endian

  // Microsoft Developer Studio specific compiler pragmas
  #if defined(_MSC_VER)
  
    // Level 4 warnings that are acceptable
    #pragma warning( disable : 4097 ) // synonym used
    #pragma warning( disable : 4100 ) // unreferenced formal parameter
    #pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union (for MMSystem.h)
    #pragma warning( disable : 4238 ) // nonstandard extension used, class rvalue as lvalue
    #pragma warning( disable : 4275 ) // non dll-interface class used as base for dll-interface class
    #pragma warning( disable : 4251 ) // class 'x' (member of 'y') needs to have dll-interface to be used by clients of class 'y'
    #pragma warning( disable : 4348 ) // potential redefinition of template default parameter
    #pragma warning( disable : 4355 ) // 'this' : used in base member initializer list
    #pragma warning( disable : 4786 ) // Disable warning message for long identifiers
    #pragma warning( disable : 4800 ) // Forcing value to bool 'true' or 'false' (performance warning)
    #pragma warning( disable : 4996 ) // Used depreciated function - used since declaring _CRT_SECURE_NO_WARNINGS seems to have no effect
  
    #ifdef __INTEL_COMPILER
      #pragma warning( disable : 981 )   // operands are evaluated in unspecified order
      #pragma warning( disable : 383 )   // value copied to temporary, reference to temporary used
    #else
      #pragma warning( disable : 4616 )  // pragma warning :  warning number '1011' out of range, must be between '4001' and '4999'
    #endif
  
    // Generate intrinsics (inline assembly) for the following functions.
    // $Revisit - CReis There are more intrinsics to add, but they need to be combined with appropriate
    // compiler options - look into this further.
    //#pragma intrinsic(_strset, abs, fabs, memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)
  
    //#pragma optimize()
  
  
    #if defined(_M_IX86) && _M_IX86 < 600
  
      #define A_NO_SSE
      #define a_prefetch(ptr)
  
    #else

      #define a_prefetch(ptr) _mm_prefetch((char*)(ptr), _MM_HINT_T0)

    #endif
  
    #define A_BREAK()   __debugbreak()

    // DLL linkage specification
    #define A_DLLIMPORT __declspec(dllimport)
    #define A_DLLEXPORT __declspec(dllexport)
    #define A_FORCEINLINE __forceinline

  #endif  // _MSC_VER

  #include <malloc.h>   // alloca

#endif  // A_PLAT_PC


//---------------------------------------------------------------------------------------
// Sony PlayStation 4 (Orbis) Platform
#ifdef A_PLAT_PS4

  #define A_PLAT_STR_ID   "PS4"
  #define A_PLAT_STR_DESC "Sony PlayStation 4"
  #define A_BITS64
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian
  #define __FUNCSIG__ __func__

  #define NO_AGOG_PLACEMENT_NEW
  #define A_NO_GLOBAL_EXCEPTION_CATCH
 
  // Use old POSIX call convention rather than new ISO convention
  #define _vsnprintf   vsnprintf
  #define _snprintf    snprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS
  
  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline

#endif


//---------------------------------------------------------------------------------------
// Sony PlayStation 3 Platform
#ifdef A_PLAT_PS3

  #define A_PLAT_STR_ID   "PS3"
  #define A_PLAT_STR_DESC "Sony PlayStation 3"
  #define A_NO_SSE
  #define AGOG_ALIGNMENT32
  #define AGOG_LITTLE_ENDIAN_HOST   0    // Big endian

  // $Revisit - CReis Look into __func__ and other macros instead for GCC compiler.
  #define __FUNCSIG__  __PRETTY_FUNCTION__

  // Use old POSIX call convention rather than new ISO convention
  #define _vsnprintf   vsnprintf
  #define _snprintf    snprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline

#endif


//---------------------------------------------------------------------------------------
// Microsoft Xbox One (Durango) Platform
#ifdef A_PLAT_X_ONE

  #include <xmmintrin.h>

  #define A_PLAT_STR_ID   "X_ONE"
  #define A_PLAT_STR_DESC "Microsoft Xbox One"
  #define A_BITS64
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian

  //   Microsoft Developer Studio specific compiler pragmas
  #if defined(_MSC_VER)

    // Level 4 warnings that are acceptable
    #pragma warning( disable : 4800 ) // Forcing value to bool 'true' or 'false' (performance warning)
    #pragma warning( disable : 4996 ) // Used depreciated function - used since declaring _CRT_SECURE_NO_WARNINGS seems to have no effect
  
    #define A_BREAK()   __debugbreak()

  #endif  // _MSC_VER

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE __forceinline

#endif


//---------------------------------------------------------------------------------------
// Microsoft XBox 360 Platform
#ifdef A_PLAT_X360

  #define A_PLAT_STR_ID   "X360"
  #define A_PLAT_STR_DESC "Microsoft Xbox 360"
  #define A_NO_SSE
  #define AGOG_ALIGNMENT32
  #define AGOG_LITTLE_ENDIAN_HOST   0    // Big endian

  // Load given memory location into L1 cache
  #define a_prefetch(ptr)

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE __forceinline

#endif


//---------------------------------------------------------------------------------------
// Apple iOS (iPhone/iPod/iPad) Platform
#ifdef A_PLAT_iOS

  #define A_PLAT_STR_ID   "iOS"
  #define A_PLAT_STR_DESC "Apple iOS"
  #define A_NO_SSE
  #define AGOG_ALIGNMENT32
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian

  #define NO_AGOG_PLACEMENT_NEW
  #define A_NO_GLOBAL_EXCEPTION_CATCH

  // $Revisit - CReis Look into using __func__ and __PRETTY_FUNCTION__ instead
  #define __FUNCSIG__  __FUNCTION__

  #define A_BREAK()   __builtin_trap()

  // Use old POSIX call convention rather than new ISO convention
  #define _gcvt       gcvt
  #define _stricmp    strcasecmp
  #define _strnicmp   strncasecmp
  #define _snprintf   snprintf
  #define _vsnprintf  vsnprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  #pragma clang diagnostic ignored "-Wundefined-bool-conversion"

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline __attribute__ ((always_inline))

#endif

//---------------------------------------------------------------------------------------
// Apple tvOS (Apple TV) Platform
#ifdef A_PLAT_tvOS

  #define A_PLAT_STR_ID   "tvOS"
  #define A_PLAT_STR_DESC "Apple tvOS"
  #define A_NO_SSE
  #define AGOG_ALIGNMENT32
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian

  #define NO_AGOG_PLACEMENT_NEW
  #define A_NO_GLOBAL_EXCEPTION_CATCH

  // $Revisit - CReis Look into using __func__ and __PRETTY_FUNCTION__ instead
  #define __FUNCSIG__  __FUNCTION__

  #define A_BREAK()   __builtin_trap()

  // Use old POSIX call convention rather than new ISO convention
  #define _gcvt       gcvt
  #define _stricmp    strcasecmp
  #define _strnicmp   strncasecmp
  #define _snprintf   snprintf
  #define _vsnprintf  vsnprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  #pragma clang diagnostic ignored "-Wundefined-bool-conversion"

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline __attribute__ ((always_inline))

#endif

//---------------------------------------------------------------------------------------
// Apple OS X Platform
#ifdef A_PLAT_OSX

  #define A_PLAT_STR_ID   "OSX"
  #define A_PLAT_STR_DESC "Apple OS X"
  #define A_BITS64
  #define A_NO_SSE // $Revisit MBreyer - off for now since intrinsics are different with Clang
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian
  #define NO_AGOG_PLACEMENT_NEW

  #define __FUNCSIG__ __PRETTY_FUNCTION__

  #define A_BREAK()   asm("int $3")

  // Use old POSIX call convention rather than new ISO convention
  #define _gcvt       gcvt
  #define _stricmp    strcasecmp
  #define _strnicmp   strncasecmp
  #define _snprintf   snprintf
  #define _vsnprintf  vsnprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline __attribute__ ((always_inline))

#endif


//---------------------------------------------------------------------------------------
// Linux Platform 64-bit
#ifdef A_PLAT_LINUX64

  #define A_PLAT_STR_ID   "Linux64"
  #define A_PLAT_STR_DESC "Linux 64-bit"
  #define A_BITS64
  #define A_NO_SSE // $Revisit MBreyer - off for now since intrinsics are different with GCC
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian
  #define NO_AGOG_PLACEMENT_NEW

  #define __FUNCSIG__ __PRETTY_FUNCTION__

  #define A_BREAK()   asm("int $3")
  
  // Use old POSIX call convention rather than new ISO convention
  #define _gcvt       gcvt
  #define _stricmp    strcasecmp
  #define _strnicmp   strncasecmp
  #define _snprintf   snprintf
  #define _vsnprintf  vsnprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  // DLL linkage specification
  #define A_DLLEXPORT __attribute__((visibility("default")))
  #define A_DLLIMPORT __attribute__((visibility("default")))
  #define A_FORCEINLINE inline __attribute__ ((always_inline))

#endif


//---------------------------------------------------------------------------------------
// Android Platform 64-bit
#ifdef A_PLAT_ANDROID

  #define A_PLAT_STR_ID   "Android"
  #define A_PLAT_STR_DESC "Android"

  #if defined(__x86_64__) || defined(__ia64__) || defined(__ARM_ARCH_ISA_A64)
    #define A_BITS64
  #endif

  #define A_NO_SSE
  
  #if !defined(__ARMEB__)
    #define AGOG_LITTLE_ENDIAN_HOST 1
  #else
    #define AGOG_LITTLE_ENDIAN_HOST 0
  #endif

  #define NO_AGOG_PLACEMENT_NEW
  #define A_NO_GLOBAL_EXCEPTION_CATCH

  #define __FUNCSIG__ __PRETTY_FUNCTION__

  #define A_BREAK()   __builtin_trap()
  
  // Use old POSIX call convention rather than new ISO convention
  #define _gcvt       gcvt
  #define _stricmp    strcasecmp
  #define _strnicmp   strncasecmp
  #define _snprintf   snprintf
  #define _vsnprintf  vsnprintf

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // Indicate that _itoa(), _ultoa(), and _gcvt() are not defined
  #define A_NO_NUM2STR_FUNCS

  // DLL linkage specification
  #define A_DLLEXPORT __attribute__((visibility("default")))
  #define A_DLLIMPORT __attribute__((visibility("default")))
  #define A_FORCEINLINE inline __attribute__ ((always_inline))

#endif


//---------------------------------------------------------------------------------------
// Nintendo WiiU Platform
#ifdef A_PLAT_WIIU

  #define A_PLAT_STR_ID   "WiiU"
  #define A_PLAT_STR_DESC "Nintendo WiiU"
  #define AGOG_LITTLE_ENDIAN_HOST   1    // Little endian

  // Load given memory location into L1 cache
  #define a_prefetch(ptr) __builtin_prefetch(ptr)

  // DLL linkage specification
  #define A_DLLIMPORT
  #define A_DLLEXPORT
  #define A_FORCEINLINE inline

#endif


//---------------------------------------------------------------------------------------
// Generic 32/64-bit defines
#ifdef A_BITS64

  #define A_BITS_ID    "64"
  #define A_BITS_STR   "64-bit"

#else

  // Assume 32-bits if 64-bits not defined.
  #define A_BITS32
  #define A_BITS_ID    "32"
  #define A_BITS_STR   "32-bit"

#endif

//#pragma message("Using platform - " A_PLAT_STR_DESC)


//---------------------------------------------------------------------------------------
// Inlining related defines
//
// If 'A_INL_IN_CPP' is defined, let the compiler treat inlined code normally and place
// *.inl files in *.cpp files in a non-inlined form.  This makes it so that changes to
// *.inl files will only need to recompile their associated *.cpp files and it makes
// recompiles faster in general.  This creates slower, but more easily debuggable code
// with faster compile turnaround times.
//
// If 'A_INL_IN_CPP' is not defined, enable full inlining and place *.inl files in *.hpp
// files in an inlined form.  Changes to *.inl files will need to recompile any files
// that include the *.hpp files associated with the *.inl files.  This makes for the
// fastest running code, but it is harder to debug and has the longest compile times.
//
// Any functions or methods that are in *.inl files must be preceded with the 'A_INLINE'
// define which is used like the 'inline' keyword.
//
//   Default:  By default if '_DEBUG' is defined, 'A_INL_IN_CPP' is defined.
//   non-inlined:  When '_DEBUG' is not defined and non-inlining is desired, just define
//     'A_INL_IN_CPP' as a compiler preprocessor directive.
//   inlined:  If 'A_INL_IN_HPP' is defined as a compiler preprocessor
//     directive, 'A_INL_IN_CPP' will not be defined even if '_DEBUG' is defined.
#if defined(A_UNOPTIMIZED) && !defined(A_INL_IN_HPP) && !defined(A_INL_IN_CPP)
  #define A_INL_IN_CPP
#endif

#ifdef A_INL_IN_CPP

  // Don't use inlines - Include *.inl files into *.cpp files

  #define A_INLINE

  // MS Dev specific
  #if defined(_MSC_VER)
    #pragma warning( disable : 4710 )  // Function not inlined
  #endif

#else

  // Use inlines - Include *.inl files into *.hpp files

  #define A_INL_IN_HPP  // Note that test for inclusion in hpp should be absence of A_INL_IN_CPP
  #define A_INLINE      inline

  // MS Dev specific
  #if defined(_MSC_VER)
    #pragma inline_depth(255)          // No limits on inline depth.

    // Prevents the compiler from second guessing the inlining of functions marked as inline
    // $Revisit - CReis The C++ Standard Library forbids macroizing keywords.
    //#define inline __forceinline
  #endif

#endif

// A_FORCEINLINE_OPTIMIZED means to force inline unless it's an unoptimized build
#ifdef A_UNOPTIMIZED
  #define A_FORCEINLINE_OPTIMIZED
#else
  #define A_FORCEINLINE_OPTIMIZED A_FORCEINLINE
#endif

#ifdef _MSC_VER
  #pragma warning( disable : 4514 ) // unreferenced inline function has been removed
#endif


//---------------------------------------------------------------------------------------
// Macro Functions

// Swaps two 32-bit values without the need of a third temporary variable
#define A_SWAP32(_value1, _value2) {*(uint32_t *)&_value1 ^= *(uint32_t *)&_value2; *(uint32_t *)&_value2 ^= *(uint32_t *)&_value1; *(uint32_t *)&_value1 ^= *(uint32_t *)&_value2;}

// Converts sized signed integer to AEquate_less(-1), AEquate_equal(0), AEquate_greater(1)
#define A_INT32_AS_EQUATE(_num)  static_cast<eAEquate>(((_num) > 0) ? 1 : (_num) >> 31)
#define A_INT64_AS_EQUATE(_num)  static_cast<eAEquate>(((_num) > 0) ? 1 : (_num) >> 63)
#ifdef A_BITS64
  // Converts system sized integer to AEquate_less(-1), AEquate_equal(0), AEquate_greater(1)
  #define A_INT_AS_EQUATE(_num)  A_INT64_AS_EQUATE(_num)
#else
  #define A_INT_AS_EQUATE(_num)  A_INT32_AS_EQUATE(_num)
#endif


// Determine number of elements in an array
#define A_COUNT_OF(_array) (sizeof(_array) / sizeof(_array[0]))

// Some worker defines unlikely to be used elsewhere
#define A__INT_LITERAL_TO_STR(_val) # _val
#define A_INT_LITERAL_TO_STR(_val)  A__INT_LITERAL_TO_STR(_val)

// To be used when __LINE__ is needed as a string rather than an integer
#define A_LINE_STR A_INT_LITERAL_TO_STR(__LINE__)

// Use this when writing to the IDE output window to make a line that will load up the
// appropriate file and line when it is double clicked on.
#define A_SOURCE_STR __FILE__ "(" A_LINE_STR ") : "

#define A_SOURCE_FUNC_STR  A_SOURCE_STR __FUNCSIG__

// This acts as an intention comment for a non-terminating (i.e. infinite) loop.
#define A_LOOP_INFINITE for (;;)



//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class AString;
class AErrorOutputBase;

//---------------------------------------------------------------------------------------
// Number Type Shorthand
//   - also see stdint.h uint8_t, uint32_t, int32_t, etc.
// Note that pointer storage should use uintptr_t, intptr_t, ptrdiff_t - which are
// included via stdint.h (and other headers).
// 
// $Note - May need to be in their own namespace to avoid collisions with other libraries.

#ifndef A_UINT
  // Standard/unsized unsigned integer - has no t/_t to match int
  typedef unsigned int uint;

  #define A_UINT
#endif


// Floating point shorthand.
typedef float   f32;  // 3.4E +/-   38  (7 digits)
typedef double  f64;  // 1.7E +/-  308 (15 digits)


//=======================================================================================
// Constants
//=======================================================================================

//---------------------------------------------------------------------------------------
// Common default/special operation/self-documenting integer constants
// - Defaults used when 0, 1, -1, 42, etc. will not do.

const int      ADef_int     = INT_MAX;
const uint     ADef_uint    = UINT_MAX;
const int8_t   ADef_int8    = INT8_MAX;
const uint8_t  ADef_uint8   = UINT8_MAX;
const int16_t  ADef_int16   = INT16_MAX;
const uint16_t ADef_uint16  = UINT16_MAX;
const int32_t  ADef_int32   = INT32_MAX;
const uint32_t ADef_uint32  = UINT32_MAX;
const int64_t  ADef_int64   = INT64_MAX;
const uint64_t ADef_uint64  = UINT64_MAX;


// $Note - Most of the major lengths/counts/sizes (arrays, strings, etc.) are in 32-bits.

// Substitute with the length of the object or the length remaining after any supplied
// starting position.
const uint32_t ALength_remainder = ADef_uint32;

// Substitute with the length of the supplied arguments - for example with a supplied
// C-String determine its length by finding a null character.
const uint32_t ALength_calculate = ADef_uint32;

// The length of the buffer is included in the first few byte(s) of the buffer.
const uint32_t ALength_in_header = ADef_uint32;

// If searching some sort of collection where there is the possibility of multiple
// matching elements, this indicates that the last matching element is desired
// - i.e. the matching element that is closest to the end of the collection.
const uint32_t AMatch_last = ADef_uint32;

// If searching some sort of collection where there is the possibility of multiple
// matching elements, this indicates that the matching element that is first *found* is
// desired.  This could be any matching element in the collection it effectively means
// ignore element order.  This type of a search can be faster than specifying that
// matching element order is important.
const uint32_t AMatch_first_found = 0u;

const uint32_t ADef_no_change = ADef_uint32;  // Indicates that no change should be made


//---------------------------------------------------------------------------------------
// Common default/special operation float constants
// - Defaults used when 0.0, 1.0, -1.0, 42.0, etc. will not do.

// $Revisit - CReis Could add float defaults
// const f32 ADef_f32 = nan;
// const f64 ADef_f64 = nan;


//---------------------------------------------------------------------------------------
// Misc. Common Constants
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
enum eAFlag
  {
  AFlag_off       = 0x0,
  AFlag_on        = 1 << 0,
  AFlag__toggle   = 1 << 1
  };

//---------------------------------------------------------------------------------------
// Used for a Boolean flag/value over a collection of elements where they could all have
// it set, they could all have it not set or they could be mixed with some set and some
// not.
enum eAGroupFlag
  {
  AGroupFlag_uninitialized = 0,
  AGroupFlag_present       = 1 << 0,
  AGroupFlag_absent        = 1 << 1,
  AGroupFlag_mixed         = AGroupFlag_present | AGroupFlag_absent  // Some in group with flag set and flag cleared
  };

//---------------------------------------------------------------------------------------
enum eAConfirm
  {
  AConfirm_abort = -1,
  AConfirm_no    =  0,
  AConfirm_yes   =  1
  };

//---------------------------------------------------------------------------------------
enum eAProgess
  {
  AProgess_skip,
  AProgess_queued,
  AProgess_processing,
  AProgess_processed
  };

//---------------------------------------------------------------------------------------
// Comparison result value - also see a_compare() below and A_INT*_AS_EQUATE() macros above.
enum eAEquate
  {
  AEquate_less    = -1,
  AEquate_equal   = 0,
  AEquate_greater = 1
  };

//---------------------------------------------------------------------------------------
// Used to specify lifetime of an argument or task.
// Commonly used with memory buffers like strings and serialization.
enum eATerm
  {
  // transient/temporary/volatile - will not last through task at hand must use
  // immediately or make copy
  ATerm_short = 0,

  // persistent/reserved/non-volatile - can use for lifetime of task at hand
  ATerm_long  = 1
  };

// Used to specify whether something should be remembered/archived in some sort of a
// history/memory or not.  [Used in the place of bool to give more obvious context.]
enum eAHistory
  {
  AHistory_forget   = 0,   // Doomed to repeat it
  AHistory_remember = 1
  };

//---------------------------------------------------------------------------------------
// Specifies whether a user should be prompted or no prompt and proceed with action.
// [Used in the place of `bool` to give more obvious context.]
enum eAPrompt
  {
  APrompt_none,  // Ask user if action should proceed
  APrompt_user   // Proceed with action without prompting user
  };

//---------------------------------------------------------------------------------------
// Indicates whether an iteration or operation did full span or action or quit early.
// [Often used in the place of bool to give more obvious context.]
enum eAIterateResult
  {
  AIterateResult_entire     = 0,  // [implicitly coercible to false]
  AIterateResult_early_exit = 1   // [implicitly coercible to true]
  };

//---------------------------------------------------------------------------------------
enum eAHorizAlign
  {
  AHorizAlign_left      = 0,
  AHorizAlign_right     = 1,
  AHorizAlign_centered  = 2,
  AHorizAlign_justified = 3
  };

//---------------------------------------------------------------------------------------
enum eAVertAlign
  {
  AVertAlign_top      = 0,
  AVertAlign_bottom   = 1,
  AVertAlign_centered = 2
  };

//---------------------------------------------------------------------------------------
// Flags for iterating through a hierarchy
enum eAHierarchy
  {
  // Stop iterating through a hierarchy
  AHierarchy__stop    = 0,
  // Apply operation to current object in hierarchy
  AHierarchy_current  = 1 << 0,
  // Apply operation recursively sub-objects in hierarchy
  AHierarchy_recurse  = 1 << 1,
  // Iterate over current object and recurse through sub-objects
  AHierarchy__all     = AHierarchy_current | AHierarchy_recurse
  };

//---------------------------------------------------------------------------------------
// Used to control information / logging / tracing output.
// If some levels are skipped for a particular function try to use relative operators <>
// to ensure that all verbosity levels may be used.
// Also note that Verbosity_none = 0 may be used as a "false" and that greater amounts of
// verbosity are ordinally greater in value so AVerbosity_full > AVerbosity_critical.
enum eAVerbosity
  {
  AVerbosity_none = 0,  // Display nothing
  AVerbosity_critical,  // Only display info if something important occurs
  AVerbosity_brief,     // Display major/high-level information
  AVerbosity_full       // Display both major/minor high/low-level info in all its glory
  };

//---------------------------------------------------------------------------------------
// What measure to take after an error
enum eAErrAction
  {
  // Go to recovery area (throw / catch) and:

  AErrAction_quit,        // Quit application
  AErrAction_retry,       // Retry last command
  AErrAction_continue,    // Skip current command and continue with next command

                          // Go to next line of code - usually unstable

  AErrAction_ignore,      // Ignore and go to next line of code (and hope that app is still stable)
  AErrAction_ignore_all,  // Ignore and go to next line of code and do not perform this test in the future

                          // Modifier Flags

  AErrAction__debug_break = 0x100,

  AErrAction__action_mask = 0x0FF
  };

//---------------------------------------------------------------------------------------
// Enum used in constructors to indicate they should not initialize anything
// User for hot-swapping the vtable of a class without changing its data
enum eALeaveMemoryUnchanged { ALeaveMemoryUnchanged = 0 };

//=======================================================================================
// Global Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Interface for AgogCore to interact with its app
class AAppInfoCore
  {
  public:

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for ADatum objects  
    virtual uint32_t get_pool_init_datum() const { return 256; }
    virtual uint32_t get_pool_incr_datum() const { return 64;  }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for AStringRef objects
    virtual uint32_t get_pool_init_string_ref() const { return 40960; }
    virtual uint32_t get_pool_incr_string_ref() const { return 256; }

    //---------------------------------------------------------------------------------------
    // Initial pool size and increment amount for ASymbolRef objects
    virtual uint32_t get_pool_init_symbol_ref() const { return 2048; }
    virtual uint32_t get_pool_incr_symbol_ref() const { return 256; }

    //---------------------------------------------------------------------------------------
    // Memory allocation
    virtual void *   malloc(size_t size, const char * debug_name_p) = 0;
    virtual void     free(void * mem_p) = 0;
    virtual uint32_t request_byte_size(uint32_t size_requested) = 0; // Convert needed byte size to byte size for allocation
    virtual bool     is_using_fixed_size_pools() = 0; // If _any_ memory can potentially come from a memory pool (for debug display only)

    //---------------------------------------------------------------------------------------
    // Prints supplied C-string to debug console
    // which can be a debugger window, standard out, something custom in the app, etc.
    virtual void debug_print(const char * cstr_p) = 0;

    //---------------------------------------------------------------------------------------
    // Called whenever an error occurs but *before* a choice has been made as to how it
    // should be resolved.  It optionally creates an error output object that will have its
    // determine_choice() called if 'nested' is false.
    // 
    // Returns:
    //   an AErrorOutputBase object to call determine_choice() on or nullptr if a default
    //   resolve error choice is to be made without prompting the user with output to the
    //   debug output window.
    //   
    // Params:
    //   nested:
    //     Indicates whether the error is nested inside another error - i.e. an additional
    //     error happened before a prior error was fully resolved (while unwinding the stack
    //     on a 'continue' exception throw for example). `determine_choice()` will *not* be
    //     called if 'nested' is true.
    virtual AErrorOutputBase * on_error_pre(bool nested) = 0;

    //---------------------------------------------------------------------------------------
    // Called whenever an error occurs and *after* a choice has been made as to how it should
    // be resolved.
    // 
    // Params:
    //   action: the action that will be taken to attempt resolve the error.
    virtual void on_error_post(eAErrAction action) = 0;

    //---------------------------------------------------------------------------------------
    // Called if 'Quit' is chosen during error.
    virtual void on_error_quit() = 0;

};

//---------------------------------------------------------------------------------------
// Convenience default implementation of AAppInfoCore
class AAppInfoCoreDefault : public AAppInfoCore
  {

  virtual void *             malloc(size_t size, const char * debug_name_p) override;
  virtual void               free(void * mem_p) override;
  virtual uint32_t           request_byte_size(uint32_t size_requested) override;
  virtual bool               is_using_fixed_size_pools() override;
  virtual void               debug_print(const char * cstr_p) override;
  virtual AErrorOutputBase * on_error_pre(bool nested) override;
  virtual void               on_error_post(eAErrAction action) override;
  virtual void               on_error_quit() override;

  };

namespace AgogCore
  {

  // Static (global) initialization/deinitialization of AgogCore
  A_API void  initialize(AAppInfoCore * app_info_p);
  A_API void  deinitialize();

  A_API AAppInfoCore *  get_app_info();
  A_API void            set_app_info(AAppInfoCore * app_info_p);

  }

//---------------------------------------------------------------------------------------
// Compares two types and returns the result as an eAEquate
// 
// Returns: AEquate_less, AEquate_equal or AEquate_greater
template<class _Type>
inline eAEquate a_compare(_Type lhs, _Type rhs)
  {
  return (lhs < rhs) ? AEquate_less : eAEquate(1 - static_cast<int>(lhs == rhs));
  }

//---------------------------------------------------------------------------------------
// Common C-string format functions - use instead of sprintf() and related functions or
// even better use the AString class if possible.  See the comments in the cpp file for
// more info.

A_API char *  a_cstr_format(const char * format_cstr_p, ...);
A_API AString a_str_format(const char * format_cstr_p, ...);

//---------------------------------------------------------------------------------------
// Memory allocation declarations 

#define a_align_up(x,align)       (((x)+((align)-1)) & (-((int)(align))))
#define a_stack_allocate(count,T) (T*)alloca((count)*sizeof(T))

//---------------------------------------------------------------------------------------
inline void * operator new(size_t size, const char * desc_cstr_p)
  {
  return AgogCore::get_app_info()->malloc(size, desc_cstr_p);
  }
    
//---------------------------------------------------------------------------------------
inline void * operator new[](size_t size, const char * desc_cstr_p)
  {
  return AgogCore::get_app_info()->malloc(size, desc_cstr_p);
  }

//---------------------------------------------------------------------------------------
inline void operator delete(void * buffer_p, const char * desc_cstr_p)
  {
  AAppInfoCore * app_info_p = AgogCore::get_app_info();
  // To minimize users' inconvenience, we allow memory to leak unless A_FUSSY_CHECK is set (see AgogCore::get_app_info())
  if (app_info_p)
    {
    app_info_p->free(buffer_p);
    }
  }

//---------------------------------------------------------------------------------------
inline void operator delete[](void * buffer_p, const char * desc_cstr_p)
  {
  AAppInfoCore * app_info_p = AgogCore::get_app_info();
  // To minimize users' inconvenience, we allow memory to leak unless A_FUSSY_CHECK is set (see AgogCore::get_app_info())
  if (app_info_p)
    {
    app_info_p->free(buffer_p);
    }
  }

// Define placement new
#if !defined(__PLACEMENT_NEW_INLINE) && !defined(NO_AGOG_PLACEMENT_NEW)
#define __PLACEMENT_NEW_INLINE

//---------------------------------------------------------------------------------------
// Allows the creation of objects in place using supplied memory
// 
// Params:
//   size:     [ignored] 'memory_p' is assumed to be large enough
//   buffer_p: memory to create instance in
//   
// Examples:
//   AString str_p = new (memory_p) AString("In place AString");
//   
// Notes:
//   This function should be completely optimized out by the compiler and simply become
//   `buffer_p`.
//   
// See: placement delete below
inline void * operator new(size_t, void * buffer_p)
  {
  return buffer_p;
  }

//---------------------------------------------------------------------------------------
// This is the placement delete to mirror the placement new.  It serves no purpose other
// than to prevent a compiler warning since all operator new functions expect a matching
// operator delete function.
// 
// Params:
//   obj_p - [ignored] pointer to object
//   memory_p - [ignored] memory instance was created in
//   
// Examples:
//   str_p->~AString();
//   
// Notes: This function should be completely optimized out by the compiler.
// See:   placement new above
inline void operator delete(void *, void *)
  {
  }

#endif  // __PLACEMENT_NEW_INLINE

//=======================================================================================
// Includes (Special Order)
//=======================================================================================

#include <AgogCore/ADebug.hpp>  // For all debugging related stuff.

