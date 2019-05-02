// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Math Scalar Function Inline Definitions
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <cmath>
#ifndef A_PLAT_PC32
  #include <math.h>
#endif
#if !defined(A_NO_SSE)
  #include <emmintrin.h>
#endif


//=======================================================================================
// Function Definitions
//=======================================================================================

#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Get fpu status
  // Author(s):   Markus Breyer
  static A_INLINE uint16_t a_fpu_get_cw()
    {
    uint16_t cw;

    __asm fnstcw cw;
    return cw;
    }

  //---------------------------------------------------------------------------------------
  // Set fpu status
  // Author(s):   Markus Breyer
  static A_INLINE void a_fpu_set_cw(uint16_t cw)
    {
    __asm fldcw cw;
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE f32 a_sin(f32 rads)
    {
    __asm
      {
      fld     rads
      fsin
      }
    }
  #pragma warning(default : 1011)

#else

  //---------------------------------------------------------------------------------------
  A_INLINE f32 a_sin(f32 rads)
    {
	return sinf(rads);
	}

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE f32 a_cos(f32 rads)
    {
    __asm
      {
      fld     rads
      fcos
      }
    }
  #pragma warning(default : 1011)

#else

  //---------------------------------------------------------------------------------------
  A_INLINE f32 a_cos(f32 rads)
    {
	return cosf(rads);
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Determines the tangent of a radian angle
  // Returns:    tangent value
  // Arg         rads - angle in radians
  // See:        other trigonometry functions
  // Notes:      This seems to generate the same code as the built in tanf() if the
  //             compiler optimizing and intrinsics are enabled.  For MSDEV, this will only
  //             occur if the "fast" floating point model is enabled.
  // Author(s):   Conan Reis
  #pragma warning(disable : 4725) // fptan instruction may be inaccurate on some Pentiums
  A_INLINE f32 a_tan(f32 rads)
    {
    f32 result;
    
    __asm
      {
      fld    rads
      fptan
      fstp   st(0)    // fptan stuffs a 1.0 on the fp stack just for fun, so get rid of it
      fstp   result   // pop result out to ensure clean FPU state - also see note below
      }

    // $Note - CReis If you know your way around assembly, you may think that storing the
    // result and then returning it is redundant since a function returns its result in
    // st(0) which is where the result is copied from in the first place.  So long as the
    // inline assembly remains as a function call you would be correct - however, if the
    // compiler helps you out by inlining this code then the result may not end up in the
    // desired location.  For example if the result of this function is assigned to a
    // variable or passed as an argument other than the first argument then the result
    // sitting in st(0) will not be used and it may interfere with other floating point
    // operations to boot.  Also note that inlining can be done via normal use of inline or
    // A_INLINE, but it may also occur if the compiler feels it may be a good optimization
    // especially if global optimizations are enabled.  If the code is inlined the compiler
    // usually does a good job of removing any redundant seeming code.
    // To make this a little more frightening, the other inline assembly functions that
    // return a float do not seem to have this problem - yet.
    return result;  
    }
  #pragma warning(default : 4725)

#else

  //---------------------------------------------------------------------------------------
  A_INLINE f32 a_tan(f32 rads)
    {
	return tanf(rads);
    }

#endif


#if !defined(A_PLAT_PC32)

  // PC 32-bit version defined as a macro in AMath.hpp

  //---------------------------------------------------------------------------------------
  A_INLINE void a_sin_cos(f32 & out_sin, f32 & out_cos, f32 rads)
    {
    out_sin = sinf(rads);
    out_cos = cosf(rads);
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_float2int(register f32 f)
    {
    int32_t t;
    __asm
      {
      fld   f
      fistp t
      }
    return t;
    }

#elif !defined(A_NO_SSE)

  //---------------------------------------------------------------------------------------
  A_INLINE int a_float2int(register f32 f)
    {
    return _mm_cvtss_si32(_mm_load_ss(&f));
    }

#else

  //---------------------------------------------------------------------------------------
  A_INLINE int a_float2int(f32 f)
    {
    return int(f);
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_round(f32 f)
    {
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw(cw & ~0x0c00);
    int r = a_float2int(f);
    a_fpu_set_cw(cw);
    return r;
    }

#else

  //---------------------------------------------------------------------------------------
  // Author(s):   Conan Reis
  A_INLINE int a_round(f32 f)
    {
    // $Revisit - Consider changing to std::lround()
    return int(f + 0.5f);
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE f32 a_sqrt(f32 radicand)
    {
    A_ASSERTX(radicand >= 0.0f, "Negative radicand");
    __asm
      {
      fld     radicand
      fsqrt
      }
    }
  #pragma warning(default : 1011)

#else

  //---------------------------------------------------------------------------------------
  // Square root
  // Author(s):   Conan Reis
  A_INLINE f32 a_sqrt(f32 radicand)
    {
    return sqrtf(radicand);
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_floor(f32 f)
    {
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw((cw & ~0x0c00) | 0x0400);
    int r = a_float2int(f);
    a_fpu_set_cw(cw);
    return r;
    }

#else

  //---------------------------------------------------------------------------------------
  A_INLINE int a_floor(f32 f)
    {
    return int(floorf(f));
    }

#endif


#ifdef A_PLAT_PC32

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_ceil(f32 f)
    {
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw((cw & ~0x0c00) | 0x0800);
    int r = a_float2int(f);
    a_fpu_set_cw(cw);
    return r;
    }

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_trunc(f32 f)
    {
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw(cw | 0x0c00);
    int r = a_float2int(f);
    a_fpu_set_cw(cw);
    return r;
    }

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE f32 a_floor_fraction(f32 f)
    {
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw((cw & ~0x0c00) | 0x0400);  // rounding mode = floor
    __asm
      {
      fld   f
      fld   st(0)
      frndint
      fsubp st(1),st(0)
      fldcw [cw]
      }
    // missing return statement means - return what's in st(0)
    }
  #pragma warning(default : 1011)

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  A_INLINE int a_floor_fraction(
    f32 & out_frac,
    f32 f
    )
    {
    int32_t r;
    uint16_t cw = a_fpu_get_cw();
    a_fpu_set_cw((cw & ~0x0c00) | 0x0400);  // rounding mode = floor
    __asm
      {
      fld   f
      fld   st(0)
      frndint
      fsubr st(0),st(1)
      fstp  out_frac
      fistp r
      fldcw [cw]
      }
    return (r);
    }

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE uint32_t a_div_modulo(
    uint32_t & modulo,
    uint32_t   numerator,
    uint32_t   denominator
    )
    {
    __asm
      {
      mov   eax, numerator
      xor   edx, edx
      div   denominator
      mov   ecx, modulo
      mov   [ecx], edx
      }
    }
  #pragma warning(default : 1011)

  //---------------------------------------------------------------------------------------
  // Author(s):   Markus Breyer
  #pragma warning(disable : 1011) // missing return statement at end of non-void function
  A_INLINE int32_t a_div_modulo(
    int32_t & modulo,
    int32_t   numerator,
    int32_t   denominator
    )
    {
    __asm
      {
      mov   eax, numerator
      xor   edx, edx
      idiv  denominator
      mov   ecx, modulo
      mov   [ecx], edx
      }
    }
  #pragma warning(default : 1011)

#else

  // $Vital - CReis Need to define non-PC 32-bit versions of:
  //   a_ceil()
  //   a_trunc()
  //   a_floor_fraction()
  //   a_div_modulo()

#endif // A_PLAT_PC32


//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE bool a_is_finite(f32 f)
  {
  union
    {
    f32  f;
    uint32_t u;
    } c;

  c.f = f;

  return (c.u & (0xff << 23)) != (0xff << 23);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE bool a_is_approx_zero(
  f32 f,
  f32 eps // = FLT_EPSILON
  )
  {
  return a_abs(f) <= eps;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE bool a_is_approx_equal(
  f32 fa,
  f32 fb,
  f32 eps  // = FLT_EPSILON
  )
  {
  return a_abs(fa - fb) <= eps;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE bool a_is_ordered_approx(
  f32 valuea,
  f32 valueb,
  f32 valuec,
  f32 epsilon // = FLT_EPSILON
  )
  {
  return (((valuea - epsilon) <= valueb ) && (valueb <= (valuec + epsilon)));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_sqr(f32 f)
  {
  return f * f;
  }

//---------------------------------------------------------------------------------------
// Find the integral part of the base 10 logarithm of the unsigned value -
//             i.e. ceil(log10(value))
// Returns:    integral part of the base 10 logarithm of the unsigned value
// Arg         value - 
// Notes:      Useful to determine the number of decimal digits for a value - 1
// Author(s):   Conan Reis
A_INLINE uint32_t a_log10ceil(uint32_t value)
  {
  uint32_t pow10 = 0u;

  while (value >= 10u)
    {
    value /= 10u;
    pow10++;
    }

  return pow10;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_cube(f32 f)
  {
  return f * f * f;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_cubic_attenuate(f32 t)
  {
  return t * t * (3 - 2*t);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_reciprocal(f32 f)
  {
  A_ASSERTX(f != 0.0f, "Divide by zero");
  return 1.0f / f;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_reciprocal_est(f32 f)
  {
  A_ASSERTX(f != 0.0f, "Divide by zero");
  #ifndef A_NO_SSE
    f32 r;
    _mm_store_ss(&r, _mm_rcp_ss(_mm_set_ss(f)));
    return r;
  #else
    return a_reciprocal(f);
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_sqrt_est(f32 radicand)
  {
  A_ASSERTX(radicand >= 0.0f, "Negative radicand");
  #ifndef A_NO_SSE
    f32 r;
    _mm_store_ss(&r, _mm_sqrt_ss(_mm_set_ss(radicand)));
    return r;
  #else
    return a_sqrt(radicand);
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_rsqrt(f32 radicand)
  {
  return 1.0f / a_sqrt(radicand);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_rsqrt_est(f32 radicand)
  {
  A_ASSERTX(radicand > 0.0f, "Negative or zero radicand");
  #ifndef A_NO_SSE
    f32 r;
    _mm_store_ss(&r, _mm_rsqrt_ss(_mm_set_ss(radicand)));
    return r;
  #else
    return a_rsqrt(radicand);
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_rsqrt(
  f32 radicand,
  f32 factor
  )
  {
  A_ASSERTX(radicand > 0.0f, "Negative or zero radicand");
  #ifndef A_NO_SSE
    return factor * a_rsqrt(radicand);
  #else
    return factor / a_sqrt(radicand);
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_rsqrt_est(
  f32 radicand,
  f32 factor
  )
  {
  A_ASSERTX(radicand > 0.0f, "Negative or zero radicand");
  #ifndef A_NO_SSE
    return factor * a_rsqrt_est(radicand);
  #else
    return factor / a_sqrt_est(radicand);
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_hypot(
  f32 x,
  f32 y
  )
  {
  return a_sqrt(x*x + y*y);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_hypot(
  f32 x,
  f32 y,
  f32 z
  )
  {
  return a_sqrt(x*x + y*y + z*z);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE f32 a_lerp(
  f32 va, f32 vb, f32 t)
  {
  // May not result in vb when t = 1 due to floating-point arithmetic error.
  return va + t * (vb - va);

  // Precise mechanism, which guarantees result = vb when t = 1.
  //return ((1.0f - t) * va) + (t * vb);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
A_INLINE int32_t a_lerp_round(
  int32_t va, int32_t vb, f32 t)
  {
  using namespace std; // To get lround to compile on all platforms
  return va + int32_t(lround(static_cast<double>(t) * (vb - va)));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE bool a_is_pow_2(uint x)
  {
  return (x & (x-1)) == 0;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE uint32_t a_floor_pow_2(uint32_t x)
  {
  x |= x>>1; 
  x |= x>>2; 
  x |= x>>4; 
  x |= x>>8; 
  x |= x>>16; 
  return x - (x>>1);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE uint32_t a_ceil_pow_2(uint32_t x)  
  {
  --x; 
  x |= x>>1; 
  x |= x>>2;
  x |= x>>4;
  x |= x>>8;
  x |= x>>16;
  return x + 1; 
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE int a_floor_log_2(uint x)
  {
  A_ASSERTX(x > 0, "Cannot compute logarithm of zero");
  f32 y = f32(x);
  return (reinterpret_cast<int32_t&>(y) >> 23) - 127;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE int a_floor_log_2(f32 x)
  {
  A_ASSERTX(x > 0.0f, "Cannot compute logarithm of zero or negative numbers");
  return (reinterpret_cast<int32_t&>(x) >> 23) - 127;  
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE int a_ceil_log_2(uint x)
  {
  if (x <= 1)
    return 0;
  f32 y = f32(x - 1); 
  return (reinterpret_cast<int32_t&>(y) >> 23) - 126;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
A_INLINE int a_ceil_log_2(f32 x)
  {
  A_ASSERTX(x > 0.0f, "Cannot compute logarithm of zero or negative numbers");
  int i = reinterpret_cast<int32_t&>(x);
  int r = (i >> 23) - 126; 
  return r - ((i & 0x7fffff)==0);
  }

//---------------------------------------------------------------------------------------
// Approximation of arc sine
// Notes:      With a maximum error of about 2.75e-5 (at x==A_sin_45)
// Author(s):   Markus Breyer
A_INLINE f32 a_asin(f32 sina)
  {
  if (sina >= 1.0f)
    {
    return A_90_deg;
    }

  if (sina <= -1.0f)
    {
    return -A_90_deg;
    }

  f32 sina_save  = sina;
  f32 sina_sqr   = sina * sina;
  bool mirrored  = false;

  if (sina_sqr >= 0.5f)
    {
    mirrored = true;
    sina_sqr = 1.0f - sina_sqr;
    sina     = a_sqrt(sina_sqr);

    if (sina_save < 0.0f)
      {
      sina = -sina;
      }
    }

  f32 result = .1101154144f;

  result *= sina_sqr;
  result -= .0074306085f;
  result *= sina_sqr;
  result += .08819451763f;
  result *= sina_sqr;
  result += .1653592541f;
  result *= sina_sqr;
  result += 1.000035701f;
  result *= sina;

  if (mirrored)
    {
    result = (sina_save < 0.0f ? -A_90_deg : A_90_deg) - result;
    }

  return result;
  }

//---------------------------------------------------------------------------------------
// Approximation of arc cosine
// Notes:      With a maximum error of about 2.75e-5 (at x==A_cos_45)
// Author(s):   Markus Breyer
A_INLINE f32 a_acos(f32 cosa)
  {
  return A_90_deg - a_asin(cosa);
  }

//---------------------------------------------------------------------------------------
// Approximation of four quadrant arc tangent
// Notes:      With a maximum error of about 1.2e-5
// Author(s):   Markus Breyer
A_INLINE f32 a_atan2(
  f32 y,
  f32 x
  )
  {
  bool swapped = false;

  if (a_abs(y) > a_abs(x))
    {
    a_swap(x, y);
    swapped = true;
    }

  // minimax approximation for arctan x with -1<=x<=+1
  // error <= 1.2e-5 - error function crosses zero at PI/4,
  // so that there is no discontinuity at swap points
  f32 t      = y / x;
  f32 t_sqr  = t * t;
  f32 result = .2024574739865927e-1f;

  result *= t_sqr;
  result += -.08396617728624652f;
  result *= t_sqr;
  result += .1793882689804296f;
  result *= t_sqr;
  result += -.3301251915207592f;
  result *= t_sqr;
  result += .9998555155925566f;
  result *= t;

  if (swapped)
    {
    result = A_90_deg - result;
    }

  if (x < 0.0f)
    {
    f32 add = A_180_deg;

    if (result > 0.0f)
      {
      add = -add;
      }
    result += add;
    }

  return result;
  }





