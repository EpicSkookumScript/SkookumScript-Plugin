// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Math Common Constants & Scalar Function Declarations
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp>
#if defined(A_PLAT_PC) && !defined(A_NO_SSE)
  #include <xmmintrin.h>
#endif


//=======================================================================================
// Constants
//=======================================================================================

// Other common constants are in AgogCore/AgogCore.hpp

const f32 A_sqrt_eps_f32  = 3.452669831e-04f;
const f32 A_sqrt_min_f32  = 1.084202172e-19f;
const f32 A_sqrt_max_f32  = 1.844674352e+19f;
const f32 A_pi            = 3.1415926535897932384626433832795f;
const f32 A_2pi           = (2.0f * A_pi);
const f32 A_half_pi       = (0.5f * A_pi);
const f32 A_sqrt_2        = 1.4142135623730950488016887242097f;
const f32 A_sqrt_3        = 1.7320508075688772935274463415059f;
const f32 A_inv_sqrt_2    = 0.70710678118654752440084436210485f;
const f32 A_inv_sqrt_3    = 0.57735026918962576450914878050196f;
const f32 A_half_sqrt_2   = 0.70710678118654752440084436210485f;
const f32 A_half_sqrt_3   = 0.86602540378443864676372317075294f;

// Also known as divine proportion, golden mean, or golden section - related to the Fibonacci Sequence
const f32 A_golden_ratio  = 1.6180339887498948482045868343656381f;  // = (1 + sqrt(5)) / 2

const f32 A_deg_rad       = (A_pi / 180.0f);
const f32 A_rad_deg       = (180.0f / A_pi);

const f32 A_0_deg       = 0.0f;
const f32 A_1_deg       = .0174532925199432958f;
const f32 A_2_deg       = .0349065850398865915f;
const f32 A_3_deg       = .0523598775598298873f;
const f32 A_4_deg       = .0698131700797731831f;
const f32 A_5_deg       = .0872664625997164788f;
const f32 A_6_deg       = .1047197551196597746f;
const f32 A_7_deg       = .1221730476396030704f;
const f32 A_8_deg       = .1396263401595463662f;
const f32 A_9_deg       = .1570796326794896619f;
const f32 A_10_deg      = .1745329251994329577f;
const f32 A_11_25_deg   = .19634954084936207740391521146f;
const f32 A_15_deg      = .2617993877991494365f;
const f32 A_20_deg      = .3490658503988659154f;
const f32 A_22_5_deg    = .392699081698724154807830423f;
const f32 A_25_deg      = .4363323129985823942f;
const f32 A_30_deg      = .5235987755982988731f;
const f32 A_35_deg      = .6108652381980153519f;
const f32 A_40_deg      = .6981317007977318308f;
const f32 A_45_deg      = .7853981633974483096f;          // 0.25 pi
const f32 A_50_deg      = .8726646259971647885f;
const f32 A_55_deg      = .9599310885968812673f;
const f32 A_60_deg      = 1.0471975511965977462f;
const f32 A_65_deg      = 1.1344640137963142250f;
const f32 A_70_deg      = 1.2217304763960307039f;
const f32 A_75_deg      = 1.3089969389957471827f;
const f32 A_80_deg      = 1.3962634015954636615f;
const f32 A_85_deg      = 1.4835298641951801404f;
const f32 A_90_deg      = 1.5707963267948966192f;         // 0.5 pi
const f32 A_95_deg      = 1.6580627893946130981f;
const f32 A_100_deg     = 1.7453292519943295769f;
const f32 A_105_deg     = 1.8325957145940460558f;
const f32 A_110_deg     = 1.9198621771937625346f;
const f32 A_115_deg     = 2.0071286397934790135f;
const f32 A_120_deg     = 2.0943951023931954923f;
const f32 A_125_deg     = 2.1816615649929119712f;
const f32 A_130_deg     = 2.2689280275926284500f;
const f32 A_135_deg     = 2.3561944901923449288f;         // 0.75 pi
const f32 A_140_deg     = 2.4434609527920614077f;
const f32 A_145_deg     = 2.5307274153917778865f;
const f32 A_150_deg     = 2.6179938779914943654f;
const f32 A_155_deg     = 2.7052603405912108442f;
const f32 A_160_deg     = 2.7925268031909273231f;
const f32 A_165_deg     = 2.8797932657906438019f;
const f32 A_170_deg     = 2.9670597283903602808f;
const f32 A_175_deg     = 3.0543261909900767596f;
const f32 A_180_deg     = 3.1415926535897932385f;         // 1 pi
const f32 A_190_deg     = 3.3161255787892261962f;
const f32 A_200_deg     = 3.4906585039886591538f;
const f32 A_210_deg     = 3.6651914291880921116f;
const f32 A_220_deg     = 3.8397243543875250692f;
const f32 A_225_deg     = 3.92699081698724154807830423f;  // 1.25 pi
const f32 A_230_deg     = 4.0142572795869580269f;
const f32 A_240_deg     = 4.1887902047863909846f;
const f32 A_250_deg     = 4.3633231299858239423f;
const f32 A_260_deg     = 4.5378560551852569000f;
const f32 A_270_deg     = 4.7123889803846898577f;         // 1.5 pi
const f32 A_280_deg     = 4.8869219055841228154f;
const f32 A_290_deg     = 5.0614548307835557731f;
const f32 A_300_deg     = 5.2359877559829887308f;
const f32 A_310_deg     = 5.4105206811824216885f;
const f32 A_315_deg     = 5.49778714378213816730962592f;  // 1.75 pi
const f32 A_320_deg     = 5.5850536063818546462f;
const f32 A_330_deg     = 5.7595865315812876038f;
const f32 A_340_deg     = 5.9341194567807205615f;
const f32 A_350_deg     = 6.1086523819801535192f;
const f32 A_360_deg     = 6.2831853071795864769f;         // 2 pi
const f32 A_370_deg     = 6.4577182323790194346f;
const f32 A_380_deg     = 6.6322511575784523923f;
const f32 A_390_deg     = 6.8067840827778853500f;
const f32 A_400_deg     = 6.9813170079773183077f;
const f32 A_410_deg     = 7.1558499331767512654f;
const f32 A_420_deg     = 7.3303828583761842231f;
const f32 A_430_deg     = 7.5049157835756171808f;
const f32 A_440_deg     = 7.6794487087750501384f;
const f32 A_450_deg     = 7.8539816339744830962f;
const f32 A_460_deg     = 8.0285145591739160539f;
const f32 A_470_deg     = 8.2030474843733490115f;
const f32 A_480_deg     = 8.3775804095727819692f;
const f32 A_490_deg     = 8.5521133347722149269f;
const f32 A_500_deg     = 8.7266462599716478846f;
const f32 A_510_deg     = 8.9011791851710808423f;
const f32 A_520_deg     = 9.0757121103705138000f;
const f32 A_530_deg     = 9.2502450355699467577f;
const f32 A_540_deg     = 9.4247779607693797154f;
const f32 A_550_deg     = 9.5993108859688126731f;
const f32 A_560_deg     = 9.7738438111682456308f;
const f32 A_570_deg     = 9.9483767363676785885f;
const f32 A_580_deg     = 10.1229096615671115461f;
const f32 A_590_deg     = 10.2974425867665445038f;
const f32 A_600_deg     = 10.4719755119659774615f;
const f32 A_610_deg     = 10.6465084371654104192f;
const f32 A_620_deg     = 10.8210413623648433769f;
const f32 A_630_deg     = 10.9955742875642763346f;
const f32 A_640_deg     = 11.1701072127637092923f;
const f32 A_650_deg     = 11.3446401379631422500f;
const f32 A_660_deg     = 11.5191730631625752077f;
const f32 A_670_deg     = 11.6937059883620081654f;
const f32 A_680_deg     = 11.8682389135614411231f;
const f32 A_690_deg     = 12.0427718387608740808f;
const f32 A_700_deg     = 12.2173047639603070385f;
const f32 A_710_deg     = 12.3918376891597399961f;
const f32 A_720_deg     = 12.5663706143591729538f;

const f32 A_sin_0_deg   = 0.0f;
const f32 A_sin_1_deg   = .01745240643728351282f;
const f32 A_sin_2_deg   = .03489949670250097165f;
const f32 A_sin_3_deg   = .05233595624294383272f;
const f32 A_sin_4_deg   = .06975647374412530078f;
const f32 A_sin_5_deg   = .08715574274765817356f;
const f32 A_sin_6_deg   = .10452846326765347140f;
const f32 A_sin_7_deg   = .12186934340514748112f;
const f32 A_sin_8_deg   = .13917310096006544411f;
const f32 A_sin_9_deg   = .15643446504023086901f;
const f32 A_sin_10_deg  = .17364817766693034886f;
const f32 A_sin_15_deg  = .25881904510252076235f;
const f32 A_sin_20_deg  = .34202014332566873305f;
const f32 A_sin_25_deg  = .42261826174069943620f;
const f32 A_sin_30_deg  = .50000000000000000000f;
const f32 A_sin_35_deg  = .57357643635104609611f;
const f32 A_sin_40_deg  = .64278760968653932632f;
const f32 A_sin_45_deg  = .70710678118654752440f;
const f32 A_sin_50_deg  = .76604444311897803521f;
const f32 A_sin_55_deg  = .81915204428899178970f;
const f32 A_sin_60_deg  = .86602540378443864675f;
const f32 A_sin_65_deg  = .90630778703664996324f;
const f32 A_sin_70_deg  = .93969262078590838407f;
const f32 A_sin_75_deg  = .96592582628906828675f;
const f32 A_sin_80_deg  = .98480775301220805936f;
const f32 A_sin_85_deg  = .99619469809174553230f;
const f32 A_sin_90_deg  = 1.0f;
const f32 A_sin_95_deg  = A_sin_85_deg;
const f32 A_sin_100_deg = A_sin_80_deg;
const f32 A_sin_105_deg = A_sin_75_deg;
const f32 A_sin_110_deg = A_sin_70_deg;
const f32 A_sin_115_deg = A_sin_65_deg;
const f32 A_sin_120_deg = A_sin_60_deg;
const f32 A_sin_125_deg = A_sin_55_deg;
const f32 A_sin_130_deg = A_sin_50_deg;
const f32 A_sin_135_deg = A_sin_45_deg;
const f32 A_sin_140_deg = A_sin_40_deg;
const f32 A_sin_145_deg = A_sin_35_deg;
const f32 A_sin_150_deg = A_sin_30_deg;
const f32 A_sin_155_deg = A_sin_25_deg;
const f32 A_sin_160_deg = A_sin_20_deg;
const f32 A_sin_165_deg = A_sin_15_deg;
const f32 A_sin_170_deg = A_sin_10_deg;
const f32 A_sin_175_deg = A_sin_5_deg;
const f32 A_sin_180_deg = 0.0f;
const f32 A_sin_185_deg = -A_sin_5_deg;
const f32 A_sin_190_deg	= -A_sin_10_deg;
const f32 A_sin_195_deg	= -A_sin_15_deg;
const f32 A_sin_200_deg	= -A_sin_20_deg;
const f32 A_sin_205_deg	= -A_sin_25_deg;
const f32 A_sin_210_deg	= -A_sin_30_deg;
const f32 A_sin_215_deg	= -A_sin_35_deg;
const f32 A_sin_220_deg	= -A_sin_40_deg;
const f32 A_sin_225_deg	= -A_sin_45_deg;
const f32 A_sin_230_deg	= -A_sin_50_deg;
const f32 A_sin_235_deg	= -A_sin_55_deg;
const f32 A_sin_240_deg	= -A_sin_60_deg;
const f32 A_sin_245_deg	= -A_sin_65_deg;
const f32 A_sin_250_deg	= -A_sin_70_deg;
const f32 A_sin_255_deg	= -A_sin_75_deg;
const f32 A_sin_260_deg	= -A_sin_80_deg;
const f32 A_sin_265_deg	= -A_sin_85_deg;
const f32 A_sin_270_deg	= -1.0f;
const f32 A_sin_275_deg	= -A_sin_85_deg;
const f32 A_sin_280_deg	= -A_sin_80_deg;
const f32 A_sin_285_deg	= -A_sin_75_deg;
const f32 A_sin_290_deg	= -A_sin_70_deg;
const f32 A_sin_295_deg	= -A_sin_65_deg;
const f32 A_sin_300_deg	= -A_sin_60_deg;
const f32 A_sin_305_deg	= -A_sin_55_deg;
const f32 A_sin_310_deg	= -A_sin_50_deg;
const f32 A_sin_315_deg	= -A_sin_45_deg;
const f32 A_sin_320_deg	= -A_sin_40_deg;
const f32 A_sin_325_deg	= -A_sin_35_deg;
const f32 A_sin_330_deg	= -A_sin_30_deg;
const f32 A_sin_335_deg	= -A_sin_25_deg;
const f32 A_sin_340_deg	= -A_sin_20_deg;
const f32 A_sin_345_deg	= -A_sin_15_deg;
const f32 A_sin_350_deg	= -A_sin_10_deg;
const f32 A_sin_355_deg	= -A_sin_5_deg;
const f32 A_sin_360_deg	= 0.0f;

const f32 A_cos_0_deg   = 1.0f;
const f32 A_cos_1_deg   = .99984769515639123916f;
const f32 A_cos_2_deg   = .99939082701909573001f;
const f32 A_cos_3_deg   = .99862953475457387378f;
const f32 A_cos_4_deg   = .99756405025982424761f;
const f32 A_cos_5_deg   = .99619469809174553229f;
const f32 A_cos_6_deg   = .99452189536827333692f;
const f32 A_cos_7_deg   = .99254615164132203498f;
const f32 A_cos_8_deg   = .99026806874157031508f;
const f32 A_cos_9_deg   = .98768834059513772619f;
const f32 A_cos_10_deg  = .98480775301220805937f;
const f32 A_cos_15_deg  = .96592582628906828675f;
const f32 A_cos_20_deg  = .93969262078590838405f;
const f32 A_cos_25_deg  = .90630778703664996324f;
const f32 A_cos_30_deg  = .86602540378443864675f;
const f32 A_cos_35_deg  = .81915204428899178969f;
const f32 A_cos_40_deg  = .76604444311897803520f;
const f32 A_cos_45_deg  = .70710678118654752440f;
const f32 A_cos_50_deg  = .64278760968653932631f;
const f32 A_cos_55_deg  = .57357643635104609609f;
const f32 A_cos_60_deg  = .50000000000000000000f;
const f32 A_cos_65_deg  = .42261826174069943619f;
const f32 A_cos_70_deg  = .34202014332566873299f;
const f32 A_cos_75_deg  = .25881904510252076234f;
const f32 A_cos_80_deg  = .17364817766693034889f;
const f32 A_cos_85_deg  = .087155742747658173543f;
const f32 A_cos_90_deg  = 0.0f;
const f32 A_cos_95_deg  = -A_cos_85_deg;
const f32 A_cos_100_deg = -A_cos_80_deg;
const f32 A_cos_105_deg = -A_cos_75_deg;
const f32 A_cos_110_deg = -A_cos_70_deg;
const f32 A_cos_115_deg = -A_cos_65_deg;
const f32 A_cos_120_deg = -A_cos_60_deg;
const f32 A_cos_125_deg = -A_cos_55_deg;
const f32 A_cos_130_deg = -A_cos_50_deg;
const f32 A_cos_135_deg = -A_cos_45_deg;
const f32 A_cos_140_deg = -A_cos_40_deg;
const f32 A_cos_145_deg = -A_cos_35_deg;
const f32 A_cos_150_deg = -A_cos_30_deg;
const f32 A_cos_155_deg = -A_cos_25_deg;
const f32 A_cos_160_deg = -A_cos_20_deg;
const f32 A_cos_165_deg = -A_cos_15_deg;
const f32 A_cos_170_deg = -A_cos_10_deg;
const f32 A_cos_175_deg = -A_cos_5_deg;
const f32 A_cos_180_deg = -1.0f;
const f32 A_cos_185_deg = -A_cos_5_deg;
const f32 A_cos_190_deg = -A_cos_10_deg;
const f32 A_cos_195_deg = -A_cos_15_deg;
const f32 A_cos_200_deg = -A_cos_20_deg;
const f32 A_cos_205_deg = -A_cos_25_deg;
const f32 A_cos_210_deg = -A_cos_30_deg;
const f32 A_cos_215_deg = -A_cos_35_deg;
const f32 A_cos_220_deg = -A_cos_40_deg;
const f32 A_cos_225_deg = -A_cos_45_deg;
const f32 A_cos_230_deg = -A_cos_50_deg;
const f32 A_cos_235_deg = -A_cos_55_deg;
const f32 A_cos_240_deg = -A_cos_60_deg;
const f32 A_cos_245_deg = -A_cos_65_deg;
const f32 A_cos_250_deg = -A_cos_70_deg;
const f32 A_cos_255_deg = -A_cos_75_deg;
const f32 A_cos_260_deg = -A_cos_80_deg;
const f32 A_cos_265_deg = -A_cos_85_deg;
const f32 A_cos_270_deg = 0.0f;
const f32 A_cos_275_deg = A_cos_85_deg;
const f32 A_cos_280_deg = A_cos_80_deg;
const f32 A_cos_285_deg = A_cos_75_deg;
const f32 A_cos_290_deg = A_cos_70_deg;
const f32 A_cos_295_deg = A_cos_65_deg;
const f32 A_cos_300_deg = A_cos_60_deg;
const f32 A_cos_305_deg = A_cos_55_deg;
const f32 A_cos_310_deg = A_cos_50_deg;
const f32 A_cos_315_deg = A_cos_45_deg;
const f32 A_cos_320_deg = A_cos_40_deg;
const f32 A_cos_325_deg = A_cos_35_deg;
const f32 A_cos_330_deg = A_cos_30_deg;
const f32 A_cos_335_deg = A_cos_25_deg;
const f32 A_cos_340_deg = A_cos_20_deg;
const f32 A_cos_345_deg = A_cos_15_deg;
const f32 A_cos_350_deg = A_cos_10_deg;
const f32 A_cos_355_deg = A_cos_5_deg;
const f32 A_cos_360_deg = 1.0f;

const f32 A_tan_0_deg   = 0.0f;
const f32 A_tan_1_deg   = .01745506492821758577f;
const f32 A_tan_2_deg   = .03492076949174773050f;
const f32 A_tan_3_deg   = .05240777928304120404f;
const f32 A_tan_4_deg   = .06992681194351041367f;
const f32 A_tan_5_deg   = .08748866352592400522f;
const f32 A_tan_6_deg   = .10510423526567646252f;
const f32 A_tan_7_deg   = .12278456090290459114f;
const f32 A_tan_8_deg   = .14054083470239144683f;
const f32 A_tan_9_deg   = .15838444032453629384f;
const f32 A_tan_10_deg  = .17632698070846497348f;
const f32 A_tan_15_deg  = .26794919243112270647f;
const f32 A_tan_20_deg  = .36397023426620236136f;
const f32 A_tan_25_deg  = .46630765815499859284f;
const f32 A_tan_30_deg  = .57735026918962576449f;
const f32 A_tan_35_deg  = .70020753820970977945f;
const f32 A_tan_40_deg  = .83909963117728001176f;
const f32 A_tan_45_deg  = 1.0f;
const f32 A_tan_50_deg  = 1.1917535925942099587f;
const f32 A_tan_55_deg  = 1.4281480067421145022f;
const f32 A_tan_60_deg  = 1.7320508075688772935f;
const f32 A_tan_65_deg  = 2.1445069205095586164f;
const f32 A_tan_70_deg  = 2.7474774194546222792f;
const f32 A_tan_75_deg  = 3.7320508075688772936f;
const f32 A_tan_80_deg  = 5.6712818196177095297f;
const f32 A_tan_85_deg  = 11.430052302761343069f;

//---------------------------------------------------------------------------------------
// Used to convert angles into more discrete values.  All the directions can be used
// including diagonals or just the cardinal directions of up, right, down, and left.
// Note that the cardinal directions can be used like flags.
// See a_angle_to_yaw(), a_yaw_to_angle()
enum eAYaw
  {
  AYaw_none       = 0,  // Centered
  AYaw_up         = 1 << 0,
  AYaw_up_right   = AYaw_up | (1 << 1),
  AYaw_right      = 1 << 1,
  AYaw_down_right = (1 << 2) | AYaw_right,
  AYaw_down       = 1 << 2,
  AYaw_down_left  = AYaw_down | (1 << 3),
  AYaw_left       = 1 << 3,
  AYaw_up_left    = AYaw_up | AYaw_left
  };


//=======================================================================================
// Global Functions
//=======================================================================================

A_API bool     a_is_finite(       f32 f);
A_API bool     a_is_approx_zero(  f32 f, f32 eps = FLT_EPSILON);
A_API bool     a_is_approx_equal( f32 fa, f32 fb, f32 eps = FLT_EPSILON);

A_API int      a_round(           f32 f);
A_API int      a_floor(           f32 f);
A_API int      a_ceil(            f32 f);
A_API int      a_trunc(           f32 f);
A_API int      a_float2int(       f32 f);
A_API f32      a_floor_fraction(  f32 f);
A_API int      a_floor_fraction(f32 & out_frac, f32 f);
A_API uint32_t a_log10ceil(  uint32_t value);

A_API uint32_t a_div_modulo(  uint32_t & modulo, uint32_t numerator, uint32_t denominator);
A_API int32_t  a_div_modulo(   int32_t & modulo, int32_t numerator, int32_t denominator);

A_API f32     a_sqr(             f32 f);
A_API f32     a_cube(            f32 f);
A_API f32     a_cubic_attenuate( f32 t);
A_API f32     a_reciprocal(      f32 f);
A_API f32     a_reciprocal_est(  f32 f);
A_API f32     a_sqrt(            f32 radicand);
A_API f32     a_sqrt_est(        f32 radicand);
A_API f32     a_rsqrt(           f32 radicand);
A_API f32     a_rsqrt_est(       f32 radicand);
A_API f32     a_rsqrt(           f32 radicand, f32 factor);
A_API f32     a_rsqrt_est(       f32 radicand, f32 factor);
A_API f32     a_hypot(           f32 x, f32 y);
A_API f32     a_hypot(           f32 x, f32 y, f32 z);
A_API f32     a_lerp(            f32 va, f32 vb, f32 t);
A_API int32_t a_lerp_round(      int32_t va, int32_t vb, f32 t);

A_API bool  a_is_pow_2(        uint x);
A_API uint  a_floor_pow_2(     uint32_t x);
A_API uint  a_ceil_pow_2(      uint32_t x);
A_API int   a_floor_log_2(     uint x);
A_API int   a_floor_log_2(     f32 x);
A_API int   a_ceil_log_2(      uint x);
A_API int   a_ceil_log_2(      f32 x);

A_API f32   a_sin(             f32 rads);
A_API f32   a_cos(             f32 rads);
A_API void  a_sin_cos(         f32 & out_sin, f32 & out_cos, f32 rads);
A_API f32   a_tan(             f32 rads);
A_API f32   a_asin(            f32 sina);
A_API f32   a_acos(            f32 cosa);
A_API f32   a_atan2(           f32 y, f32 x);

A_API eAYaw a_angle_to_yaw(f32 rads);
A_API f32   a_yaw_to_angle(eAYaw yaw);

template<class _Type>    _Type    a_abs(_Type a);
template<class _Type>    _Type    a_sign(_Type value);
template<class _Type>    _Type    a_min(_Type a, _Type b);
template<class _Type>    _Type    a_max(_Type a, _Type b);
template<class _Type>    bool     a_is_ordered(_Type valuea, _Type valueb, _Type valuec);
bool                              a_is_ordered_approx(f32 valuea, f32 valueb, f32 valuec, f32 epsilon = FLT_EPSILON);
template<class _Type>    _Type    a_clamp(_Type x, _Type min, _Type max);
template<class _Type>    void     a_swap(_Type & a, _Type & b);
template<class _IntType> bool     a_is_bit_on(uint32_t bit, _IntType int_val);
template<class _IntType> _IntType a_hypot_approx(_IntType x, _IntType y);


//=======================================================================================
// Inlines
//=======================================================================================

//---------------------------------------------------------------------------------------
#ifdef A_PLAT_PC32

  // Author(s):   Markus Breyer
  #define a_sin_cos(out_sin, out_cos, rads) \
    { \
    register f32 a = rads; \
    __asm fld a \
    __asm fsincos \
    __asm fstp dword ptr[out_cos] \
    __asm fstp dword ptr[out_sin] \
    }

  // Alternate method - has some problems?
  //inline void a_sin_cos(f32 & out_sin, f32 & out_cos, f32 rads)
  //  {
  //  __asm
  //    {
  //    fld     rads
  //    fsincos
  //    mov     eax, out_cos
  //    fstp    [eax]
  //    mov     eax, out_sin
  //    fstp    [eax]
  //    }
  //  }

#endif

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
template<class _Type>
inline _Type a_abs(_Type a)
  {
  if (a < 0)
    {
    a = -a;
    }
  return a;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _Type>
inline _Type a_sign(_Type value)
  {
  return ((value >= 0) ? static_cast<_Type>(1) : static_cast<_Type>(-1));
  }


#ifdef A_PLAT_PC32
  // Microsoft Specific

  #pragma warning(disable : 1011) // missing return statement at end of non-void function

  // Specialization of a_abs() for f32.
  template<>
  inline f32 a_abs<f32>(f32 a)
    {
    __asm
      {
      fld   a
      fabs
      }
    }

  #pragma warning(default : 1011) // missing return statement at end of non-void function

#else

  // Specialization of a_abs() for f32.
  template<>
  inline f32 a_abs<f32>(f32 a)
    {
    *(long*)&a &= 0x7fffffff;
    return a;
    }

#endif  // _MSC_VER


//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
template<class _Type>
inline _Type a_min(
  _Type a,
  _Type b
  )
  {
  if (a < b)
    {
    b = a;
    }
  return b;
  }

#ifndef A_NO_SSE

  // Specialization for f32 when SSE present
  template<>
  inline f32 a_min<f32>(
    f32 a,
    f32 b
    )
    {
    f32 r;
    _mm_store_ss(&r, _mm_min_ss(_mm_set_ss(a),_mm_set_ss(b)));
    return r;
    }

#endif


//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
template<class _Type>
inline _Type a_max(_Type a, _Type b)
  {
  if (b > a)
    {
    a = b;
    }
  return a;
  }

#ifndef A_NO_SSE

  // Specialization for f32 when SSE present
  template<>
  inline f32 a_max<f32>(
    f32 a,
    f32 b
    )
    {
    f32 r;
    _mm_store_ss(&r, _mm_max_ss(_mm_set_ss(a),_mm_set_ss(b)));
    return r;
    }

#endif

//---------------------------------------------------------------------------------------
// Is value1 <= value2 <= value3
// Author(s):   Conan Reis
template<class _Type>
inline bool a_is_ordered(
  _Type valuea,
  _Type valueb,
  _Type valuec
  )
  {
  return ((valuea <= valueb ) && (valueb <= valuec));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
template<class _Type>
inline _Type a_clamp(
  _Type x,
  _Type min,
  _Type max
  )
  {
  if (x >= min)
    {
    min = x;
    }

  if (min <= max)
    {
    max = min;
    }

  return max;
  }

#ifndef A_NO_SSE

  // Specialization for f32 when SSE present
  template<>
    inline f32 a_clamp<f32>(
    f32 x,
    f32 min,
    f32 max
    )
    {
    f32 r;

    _mm_store_ss(&r, _mm_min_ss(_mm_max_ss(_mm_set_ss(x), _mm_set_ss(min)), _mm_set_ss(max)));

    return r;
    }

#endif


//---------------------------------------------------------------------------------------
// Author(s):   Markus Breyer
template<class _Type>
inline void a_swap(
  _Type & a,
  _Type & b
  )
  {
  _Type t = a;

  a = b;
  b = t;
  }

//---------------------------------------------------------------------------------------
// Approximate integral hypotenuse with overestimates never > 9/8 + 1 bit
// Notes:      Class _Type must be an integral type.
// Author(s):   Conan Reis
template<class _IntType>
inline _IntType a_hypot_approx(
  _IntType x,
  _IntType y
  )
  {
  return (x + y - (((x > y) ? y : x) >> 1));
  }

//---------------------------------------------------------------------------------------
// Is bit on (=1) at specified bit position in int_val
// Author(s):   Conan Reis
template<class _IntType>
inline bool a_is_bit_on(
  uint32_t bit,
  _IntType int_val
  )
  {
  return ((int_val >> bit) & 1u);
  }


#ifndef A_INL_IN_CPP
  #include <AgogCore/AMath.inl>
#endif
