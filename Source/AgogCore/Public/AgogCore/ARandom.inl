// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARandom class A_INLINE definitions
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <time.h>        // Uses:  time
#include <AgogCore/ADatum.hpp>


//=======================================================================================
// Local Constants
//=======================================================================================

// Linear congruential constants - as suggested by Knuth and H. W. Lewis respectively.
const uint32_t ARandom_mult = 1664525UL;
const uint32_t ARandom_incr = 1013904223UL;


// IEEE floating point constants - different for DEC VAXes and other representations!!!

// 1.0f for the exponent to make the value between 1.0 and 2.0
const uint32_t ARandom_float_one = 0x3f800000UL;
// 2.0f for the exponent to make the value between 2.0 and 4.0
const uint32_t ARandom_float_two = 0x40000000UL;

// A shift is used to remove the exponent part rather than performing a bitwise and (&)
// with 0x7fffffUL since it is better to use the high-order 23 bits of the integer rather
// than the low-order 23 bits.
const uint32_t ARandom_mantissa_shift = 9UL;


//=======================================================================================
// Inline Functions
//=======================================================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Common Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Default constructor.  Sets seed value or uses default made from system
//              time.  Any ARandom with the same seed will end up generating the same
//              sequence of numbers.  This allows repeatable results with random numbers
//              and a means to describe a "randomly generated" yet predictable object with
//              just the seed value.
// Returns:     itself
// Arg          time_seed - the seed for the random number generator to use.  It creates
//              a seed based on the system time as default.  (Default time_seed())
// Examples:    ARandom rand;
// Author(s):    Conan Reis
A_INLINE ARandom::ARandom(
  uint32_t seed // = time_seed()
  ) :
  m_seed(seed)
  {
  }

//---------------------------------------------------------------------------------------
//  Copy constructor.  It copies the seed value from another ARandom.
// Returns:     itself
// Arg          rnd - the ARandom to copy
// Examples:    ARandom rand1;
//              ARandom rand2(rand1);
// See:         set_seed(), get_seed(), operator=()
// Author(s):    Conan Reis
A_INLINE ARandom::ARandom(const ARandom & rnd) :
  m_seed(rnd.m_seed)
  {
  }

//---------------------------------------------------------------------------------------
//  Assignment operator.  Copies existing seed from another ARandom.
// Returns:     reference to itself to allow for stringization
//              rand1 = rand2 = rand3;
// Arg          rnd - the ARandom to copy
// Examples:    rand1 = rand2;
// See:         set_seed(), get_seed() 
// Author(s):    Conan Reis
A_INLINE ARandom & ARandom::operator=(const ARandom & rnd)
  {
  m_seed = rnd.m_seed;
  return *this;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Accessor Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Set seed value.  Any ARandom with the same seed will end up generating the
//              same sequence of numbers.  This allows repeatable results with random
//              numbers and a means to describe a "randomly generated" yet predictable
//              object with just the seed value.
// Arg          time_seed - the seed for the random number generator to use.  It creates
//              a seed based on the system time as default.  (Default time_seed())
// Examples:    random.set_seed(1234567);
// See:         get_seed()
// Author(s):    Conan Reis
A_INLINE void ARandom::set_seed(
  uint32_t seed // = time_seed()
  )
  {
  m_seed = seed;
  }

//---------------------------------------------------------------------------------------
//  Gets the current seed value of the ARandom instance.  Any ARandom with the
//              same seed will end up generating the same sequence of numbers.  This
//              allows repeatable results with random numbers and a means to describe a
//              "randomly generated" yet predictable object with just the seed value.
// Returns:     the integer seed
// Examples:    uint32_t seed = random.get_seed();
// See:         set_seed()
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::get_seed() const
  {
  return m_seed;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modifying Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and UINT32_MAX (2^32 - 1) with
//              a uniform distribution - i.e. each number in the range is just as likely
//              as any other.
// Returns:     a pseudo-random number between 0 and (2^32 - 1)
// Examples:    uint32_t num = random.uniform_ui();
// See:         operator(), uniform(), triangle(), normal(), up_slope(), down_slope(),
//              thorn(), nose(), and the f32 versions of the same methods
// Notes:       The uniform distribution generator is faster than the algorithms for
//              other distributions - this 'no limit' version is in particular the
//              fastest.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//                0       UINT32_MAX
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::uniform_ui()
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value
    
  return m_seed;
  }

//---------------------------------------------------------------------------------------
//  Returns true/false with 50/50 chance
// Returns:     true/false
// Examples:    bool turn_left = random.coin_toss();
// Author(s):    Markus Breyer
A_INLINE bool ARandom::coin_toss()
  {
  return (int32_t(uniform_ui()) < 0);
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and limit-1 with a uniform
//              distribution - i.e. each number in the range is just as likely as any
//              other.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random(100);  // 0-99
// See:         uniform(), uniform_ui(), triangle(), normal(), up_slope(),
//              down_slope(), thorn(), nose(), and the f32 versions of the same methods
// Notes:       This method is identical to uniform().  The uniform distribution
//              generator is faster than the algorithms for other distributions.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//                0       limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::operator() (uint32_t limit)
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value
    
  // This is an optimized (and overflow preventing) method of saying
  // "seed * limit / 2^32", but it limits the limit to a maximum of only 2^16 - 1.
  // It also uses the high-order bits of the seed which generate better numbers.
  return ((m_seed >> 16) * limit) >> 16;
  }

  // Old method
  //m_seed = (m_seed * 5709421UL) + 1UL;  // Get next seed value
  //return uint32_t((m_seed >> 16UL) % limit);

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and limit-1 with a uniform
//              distribution - i.e. each number in the range is just as likely as any
//              other.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.uniform(100);  // 0-99
// See:         operator(), uniform_ui(), triangle(), normal(), up_slope(),
//              down_slope(), thorn(), nose(), and the f32 versions of the same methods
// Notes:       The uniform distribution generator is faster than the algorithms for
//              other distributions.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//                0       limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::uniform(uint32_t limit)
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  // This is an optimized (and overflow preventing) method of saying
  // "seed * limit / 2^32", but it limits the limit to a maximum of only 2^16 - 1.
  // It also uses the high-order bits of the seed which generate better numbers.
  return ((m_seed >> 16) * limit) >> 16;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a
//              triangle shaped distribution - i.e. more likely to get numbers in the
//              middle of the range.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.triangle(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), normal(), up_slope(),
//              down_slope(), thorn(), nose(), and the f32 versions of the same methods
// Notes:       It is relatively efficient for a non-uniform distribution.
//              It has a distribution that looks like:
//                |   /\   |
//                |  /  \  |
//                | /    \ |
//                |/      \|
//                0     limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::triangle(uint32_t limit)
  {
  // Equivalent: return (uniform(limit) + uniform(limit) + 1u) >> 1;
  
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = m_seed >> 16;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  return (((m_seed >> 16) + temp) * limit) >> 17;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a normal
//              distribution - i.e. more likely to get numbers in the middle of the
//              range.  Normal disributions occur frequently in nature.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.normal(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), triangle(), up_slope(),
//              down_slope(), thorn(), nose(), and the f32 versions of the same methods
// Notes:       It has a distribution that looks like:
//                |           |
//                |    _-_    |
//                |   /   \   |
//                |_ -     - _|
//                0         limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::normal(uint32_t limit)
  {
  // Equivalent: return (uniform(limit) + uniform(limit) + uniform(limit) + 1u) / 3u;
  
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = m_seed >> 16;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  temp += m_seed >> 16;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  return (((m_seed >> 16) + temp) * limit) / 196608u;  // 196608u = (2^16) * 3
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a
//              positive sloping distribution - i.e. more likely to get numbers at the
//              end of the range.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.up_slope(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), triangle(), normal(),
//              down_slope(), thorn(), nose(), and the f32 versions of the same methods
// Notes:       It has a distribution that looks like:
//                |      /|
//                |    /  |
//                |  /    |
//                |/      |
//                0    limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::up_slope(uint32_t limit)
  {
  // Equivalent: uint32_t rand_num = triangle(limit << 1);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = m_seed >> 16;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t rand_num = (((m_seed >> 16) + temp) * limit) >> 16;

  return (rand_num < limit)
    ? rand_num
    : (limit << 1) - rand_num - 1u;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a
//              negative sloping distribution - i.e. more likely to get numbers at the
//              beginning of the range.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.down_slope(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), triangle(), normal(), up_slope(),
//              thorn(), nose(), and the f32 versions of the same methods
// Notes:       It has a distribution that looks like:
//                |\      |
//                |  \    |
//                |    \  |
//                |      \|
//                0    limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::down_slope(uint32_t limit)
  {
  // Equivalent: uint32_t rand_num = triangle(limit << 1);
  
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = m_seed >> 16;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t rand_num = (((m_seed >> 16) + temp) * limit) >> 16;
  
  return (rand_num < limit)
    ? limit - rand_num - 1u
    : rand_num - limit;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a thorn
//              shaped distribution - i.e. more likely to get numbers in the middle of 
//              the range, starting with a convex slope and ending with a concave slope.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.thorn(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), triangle(), normal(), up_slope(),
//              down_slope(), nose(), and the f32 versions of the same methods
// Notes:       It has a distribution that looks like:
//                |         |
//                |   /|    |
//                | /   \   |
//                |/      -_|
//                0      limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::thorn(uint32_t limit)
  {
  // $Revisit - CReis [Efficiency] - Consider mathematical reduction.
  return (uniform(limit) + down_slope(limit) + 1u) >> 1;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0 and (limit - 1) with a nose
//              shaped distribution [kooky name, I know] - i.e. more likely to get
//              numbers in the beginning of the range, starting with a convex slope,
//              rounding out, and ending with a concave slope.
// Returns:     a pseudo-random number between 0 and (limit - 1)
// Arg          limit - the upper range - 1 of the number to generate.  It must not
//              exceed (2^16 - 1) = UINT16_MAX = 65535.  If a larger integer range is
//              desired, use uniform_ui() which returns a random number up to
//              UINT32_MAX or use one of the floating point generation routines and
//              multiply the result by the desired range.
// Examples:    uint32_t num = random.nose(100);  // 0-99
// See:         operator(), uniform(), uniform_ui(), triangle(), normal(), up_slope(),
//              down_slope(), thorn(), and the f32 versions of the same methods
// Notes:       It has a distribution that looks like:
//                |       |
//                | --    |
//                |/  \   |
//                ||    -_|
//                0     limit-1
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::nose(uint32_t limit)
  {
  // $Revisit - CReis [Efficiency] - Consider mathematical reduction.
  return (down_slope(limit) + down_slope(limit) + 1u) >> 1;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a uniform
//              distribution - i.e. each number in the range is just as likely as any
//              other.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random();
// See:         uniform(), triangle(), normal(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       The uniform distribution generator is faster than the algorithms for
//              other distributions.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//               0.0f       1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::operator() ()
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return ((*reinterpret_cast<f32 *>(&temp)) - 1.0f);
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a uniform
//              distribution - i.e. each number in the range is just as likely as any
//              other.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.uniform();
// See:         operator(), triangle(), normal(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       The uniform distribution generator is faster than the algorithms for
//              other distributions.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//               0.0f       1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::uniform()
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return ((*reinterpret_cast<f32 *>(&temp)) - 1.0f);
  }

//---------------------------------------------------------------------------------------
// Generates a pseudo-random number between min_val and max_val with a uniform
//             distribution - i.e. each number in the range is just as likely as any
//             other.
// Returns:    a pseudo-random number between min_val and max_val
// Examples:   ARandom random;
//             f32  num = random.uniform();
// See:        operator(), triangle(), normal(), up_slope(), down_slope(), thorn(),
//             nose(), and the integer versions of the same methods
// Notes:      The uniform distribution generator is faster than the algorithms for
//             other distributions.
//             It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//             min_val    max_val
// Author(s):   Conan Reis
A_INLINE f32 ARandom::uniform_range(f32 min_val, f32 max_val)
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return min_val + (((*reinterpret_cast<f32 *>(&temp)) - 1.0f) * (max_val - min_val));
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between -1.0f and +1.0f with a uniform
//              distribution - i.e. each number in the range is just as likely as any
//              other.
// Returns:     a pseudo-random number between -1.0f and +1.0f
// Examples:    ARandom random;
//              f32  num = random.uniform_symm();
// See:         operator(), triangle(), normal(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       The uniform distribution generator is faster than the algorithms for
//              other distributions.
//              It has a distribution that looks like:
//                |          |
//                |__________|
//                |          |
//                |          |
//              -1.0f       1.0f
// Author(s):    Markus Breyer
A_INLINE f32 ARandom::uniform_symm()
  {
  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp = ARandom_float_two | (m_seed >> ARandom_mantissa_shift);

  return ((*reinterpret_cast<f32 *>(&temp)) - 3.0f);
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a triangle
//              shaped distribution - i.e. more likely to get numbers in the middle of
//              the range.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.triangle();
// See:         operator(), uniform(), normal(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       It is relatively efficient for a non-constant distribution.
//              It has a distribution that looks like:
//                |   /\   |
//                |  /  \  |
//                | /    \ |
//                |/      \|
//               0.0f     1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::triangle()
  {
  // Equivalent: return (uniform() + uniform()) / 2.0f;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp1 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp2 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return (*reinterpret_cast<f32 *>(&temp1) + *reinterpret_cast<f32 *>(&temp2) - 2.0f) * 0.5f;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between -1.0f and 1.0f with a triangle
//              shaped distribution - i.e. more likely to get numbers in the middle of
//              the range.
// Returns:     a pseudo-random number between -1.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.triangle();
// See:         operator(), uniform(), normal(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       It is relatively efficient for a non-constant distribution.
//              It has a distribution that looks like:
//                |   /\   |
//                |  /  \  |
//                | /    \ |
//                |/      \|
//              -1.0f     1.0f
// Author(s):    Conan Reis, Markus Breyer
A_INLINE f32 ARandom::triangle_symm()
  {
  // Equivalent: return (uniform() + uniform() - 1.0f);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp1 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp2 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return (*reinterpret_cast<f32 *>(&temp1) + *reinterpret_cast<f32 *>(&temp2) - 3.0f);
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a normal
//              distribution - i.e. more likely to get numbers in the middle of the range.
//              Normal distributions occur frequently in nature.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.normal();
// See:         operator(), uniform(), triangle(), up_slope(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       It has a distribution that looks like:
//                |           |
//                |    _-_    |
//                |   /   \   |
//                |_ -     - _|
//               0.0f        1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::normal()
  {
  // Equivalent: return (uniform() + uniform() + uniform()) / 3.0f;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp1 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp2 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp3 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  return (*reinterpret_cast<f32 *>(&temp1) + *reinterpret_cast<f32 *>(&temp2) + *reinterpret_cast<f32 *>(&temp3) - 3.0f) / 3.0f;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a
//              positive sloping distribution - i.e. more likely to get numbers at the
//              end of the range.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.up_slope();
// See:         operator(), uniform(), triangle(), normal(), down_slope(), thorn(),
//              nose(), and the integer versions of the same methods
// Notes:       It has a distribution that looks like:
//                |      /|
//                |    /  |
//                |  /    |
//                |/      |
//               0.0f    1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::up_slope()
  {
  // Equivalent: f32 rand_num = triangle() * 2.0f;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp1 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp2    = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);
  f32      rand_num = (*reinterpret_cast<f32 *>(&temp1) + *reinterpret_cast<f32 *>(&temp2) - 2.0f);

  return (rand_num < 1.0f)
    ? rand_num
    : 2.0f - rand_num;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a negative
//              sloping distribution - i.e. more likely to get numbers at the beginning
//              of the range.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.down_slope();
// See:         operator(), uniform(), triangle(), normal(), up_slope(), thorn(), nose(),
//              and the integer versions of the same methods 
// Notes:       It has a distribution that looks like:
//                |\      |
//                |  \    |
//                |    \  |
//                |      \|
//               0.0f    1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::down_slope()
  {
  // Equivalent: f32 rand_num = triangle() * 2.0f;

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp1 = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);

  m_seed = (m_seed * ARandom_mult) + ARandom_incr;  // Get next seed value

  uint32_t temp2    = ARandom_float_one | (m_seed >> ARandom_mantissa_shift);
  f32      rand_num = (*reinterpret_cast<f32 *>(&temp1) + *reinterpret_cast<f32 *>(&temp2) - 2.0f);

  return (rand_num < 1.0f)
    ? 1.0f - rand_num
    : rand_num - 1.0f;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a thorn
//              shaped distribution - i.e. more likely to get numbers in the middle of
//              the range, starting with a convex slope and ending with a concave slope.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.thorn();
// See:         operator(), uniform(), triangle(), normal(), up_slope(), down_slope(),
//              nose(), and the integer versions of the same methods
// Notes:       It has a distribution that looks like:
//                |         |
//                |   /|    |
//                | /   \   |
//                |/      -_|
//               0.0f      1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::thorn()
  {
  // $Revisit - CReis [Efficiency] - Consider mathematical reduction.
  return (uniform() + down_slope()) * 0.5f;
  }

//---------------------------------------------------------------------------------------
//  Generates a pseudo-random number between 0.0f and 1.0f with a nose
//              shaped distribution [kooky name, I know] - i.e. more likely to get
//              numbers in the beginning of the range, starting with a convex slope,
//              rounding out, and ending with a concave slope.
// Returns:     a pseudo-random number between 0.0f and 1.0f
// Examples:    ARandom random;
//              f32  num = random.nose();
// See:         operator(), uniform(), triangle(), normal(), up_slope(), down_slope(),
//              thorn(), and the integer versions of the same methods
// Notes:       It has a distribution that looks like:
//                |       |
//                | --    |
//                |/  \   |
//                ||    -_|
//               0.0f    1.0f
// Author(s):    Conan Reis
A_INLINE f32 ARandom::nose()
  {
  // $Revisit - CReis [Efficiency] - Consider mathematical reduction.
  return (down_slope() + down_slope()) * 0.5f;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Internal Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
//  Gets seed value based on system millisecond clock.
// Returns:     seed value based on system millisecond clock.
// See:         set_seed()
// Modifiers:    protected
// Author(s):    Conan Reis
A_INLINE uint32_t ARandom::time_seed()
  {
  // $Revisit - CReis [Efficiency] - Will timeGetTime() work here?
  // Using the second iteration of the seed since time() is based on seconds and does not
  // have much initial variation.
  return (uint32_t(::time(nullptr)) * ARandom_mult) + ARandom_incr;
  }



