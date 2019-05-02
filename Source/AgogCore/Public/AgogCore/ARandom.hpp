// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARandom class declaration header
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
// Notes    This class is used to simply and efficiently generate pseudo random numbers.
//          It performs its number generation based on a seed value which may be set or
//          stored.  Any ARandom object starting with the same seed will end up generating
//          the same sequence of numbers if the same sequence of queries (method calls)
//          are also made of it.  This allows repeatable results with random numbers and
//          a means to describe a 'randomly generated' yet repeatable and predictable
//          number sequence with just the seed value.
//
//          The ARandom class is primarily geared towards games.  Its implementation is
//          a 'quick and dirty' psuedo-random number generator that uses the linear
//          congruential method.
//
//          [It is specifically adapted from "Numerical Recipes in C: Second Edition" -
//          Copyright 1992, fourth reprint 1997 on p.284-284 using routines ranqd1() for integer
//          and randq2() for floating point number generation.]
//
//          It generates a sequence of integers num1, num2, num3, ... each between 0 and
//          limit - 1 as defined by the recurrence relation:
//
//            seed2 = multiplier * seed1 + increment
//            num   = seed2 % limit
//
//          Knuth suggests that a good value for the multiplier is 1664525.
//          H. W. Lewis suggests that a good value for the increment is 1013904223.
//
//          * Note that the actual algorithm used by this ARandom class is an extensively
//          optimized version of the above algorithm - don't worry, there are no modulo
//          operations used.
//
//          There are better pseudo random generator algorithms that generate a more
//          truly random sequence of numbers and with a greater period before the number
//          sequence repeats, but they are significantly more costly with respect to
//          execution time.  [As compared to randq1() - the integer generator - better
//          generation algorithms range approximately from 6 to 40 times slower.]
//
//          *** This generator depends upon overflow. It will NOT work correctly in an
//          environment where integer arithmetic generates an overflow exception.
//
//          *** DO NOT use rand() from the standard library routines.  It uses the same
//          basic technique, but is less efficient and less flexible.  This ARandom class
//          has been extensively tested for correctness and optimized (via profiling) for
//          both debug and release builds.
// UsesLibs    
// Inlibs   AgogCore/AgogCore.lib
// Author   Conan Reis
class A_API ARandom
  {
  protected:
  // Internal Class Methods

    static uint32_t time_seed();

  public:

  // Public Class data members

  // Common random number generator
  static ARandom ms_gen;

  // Common Methods

    ARandom(uint32_t seed = time_seed());
    ARandom(const ARandom & rnd);
    ARandom & operator=(const ARandom & rnd);

  // Accessor Methods  

    void set_seed(uint32_t seed = time_seed());
    uint32_t get_seed() const;

  // Modifying Methods

    // Large Integer generator (0 - UINT32_MAX)

    uint32_t uniform_ui();

    // Boolean coin toss

    bool coin_toss();

    // Integer generators (0 - limit-1)

    uint32_t operator() (uint32_t limit);
    uint32_t uniform(uint32_t limit);
    uint32_t triangle(uint32_t limit);
    uint32_t normal(uint32_t limit);
    uint32_t up_slope(uint32_t limit);
    uint32_t down_slope(uint32_t limit);
    uint32_t thorn(uint32_t limit);
    uint32_t nose(uint32_t limit);

    // Floating point generators (0.0f - 1.0f)
    // - Symmetric (symm) functions range from -1.0f - 1.0f
    // - Multiply result if a different range is desired

    f32 operator() ();
    f32 uniform();
    f32 uniform_range(f32 min_val, f32 max_val);
    f32 uniform_symm();
    f32 triangle();
    f32 triangle_symm();
    f32 normal();
    f32 up_slope();
    f32 down_slope();
    f32 thorn();
    f32 nose();

  protected:
  // Data Members

    // ARandom number seed
    uint32_t m_seed;

  };  // ARandom


//=======================================================================================
// Inline Methods
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <AgogCore/ARandom.inl>
#endif
