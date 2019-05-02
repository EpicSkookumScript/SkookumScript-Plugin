// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AVec2i structure declaration header
// Notes:          Simple 2D integer vector structure for user interface positions / areas.
//              Declared here in AgogCore rather than AgogGUI since 2D integer
//              co-ordinates are common across platforms.
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AMath.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

struct  tagPOINT;
typedef tagPOINT POINT;    // Forward declaration so windows header does not need to be included


//---------------------------------------------------------------------------------------
// UI 2D Vector
struct A_API AVec2i
  {
  // Data Members

    int m_x;
    int m_y;

  // Class Data Members

    static AVec2i ms_zero;

  // Common Methods

    AVec2i()                    : m_x(0), m_y(0) {}
    AVec2i(int x, int y)        : m_x(x), m_y(y) {}
    AVec2i(const AVec2i & vec)  : m_x(vec.m_x), m_y(vec.m_y) {}

  // Assignment Methods

    AVec2i & operator=(const AVec2i & vec)        { m_x = vec.m_x; m_y = vec.m_y; return *this; }
    AVec2i & make_zero()                          { m_x = 0; m_y = 0; return *this; }

  // Comparison Methods

    bool operator==(const AVec2i & vec) const     { return (m_x == vec.m_x) && (m_y == vec.m_y); }
    bool operator!=(const AVec2i & vec) const     { return (m_x != vec.m_x) || (m_y != vec.m_y); }

  // Methods

    AVec2i & operator+=(const AVec2i & vec)       { m_x += vec.m_x; m_y += vec.m_y; return *this; }
    AVec2i & operator+=(int expand)               { m_x += expand; m_y += expand; return *this; }
    AVec2i & operator-=(const AVec2i & vec)       { m_x -= vec.m_x; m_y -= vec.m_y; return *this; }
    AVec2i & operator-=(int reduce)               { m_x -= reduce; m_y -= reduce; return *this; }
    AVec2i & operator*=(f32 scale)                { m_x = a_round(f32(m_x) * scale); m_y = a_round(f32(m_y) * scale); return *this; }
    AVec2i & operator/=(f32 inv_scale)            { m_x = a_round(f32(m_x) / inv_scale); m_y = a_round(f32(m_y) / inv_scale); return *this; }
    AVec2i   operator+(const AVec2i & vec) const  { return AVec2i(m_x + vec.m_x, m_y + vec.m_y); }
    AVec2i   operator-(const AVec2i & vec) const  { return AVec2i(m_x - vec.m_x, m_y - vec.m_y); }
    AVec2i   operator*(f32 scale) const           { return AVec2i(a_round(f32(m_x) * scale), a_round(f32(m_y) * scale)); }
    AVec2i   operator/(f32 inv_scale) const       { return AVec2i(a_round(f32(m_x) / inv_scale), a_round(f32(m_y) / inv_scale)); }

    AVec2i   operator-() const                    { return AVec2i(-m_x, -m_y); }
    AVec2i & negate()                             { m_x = -m_x; m_y = -m_y; return *this; }  

    int  get_area() const                         { return m_x * m_y; }
    f32  get_length_sqr() const                   { return f32((m_x * m_x) + (m_y * m_y)); }
    f32  get_length() const                       { return a_sqrt(f32((m_x * m_x) + (m_y * m_y))); }
    bool is_zero() const                          { return (m_x == 0) && (m_y == 0); }
  };


f32 a_dist(const AVec2i & va, const AVec2i & vb);
f32 a_dist_sqr(const AVec2i & va, const AVec2i & vb);


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Gets the distance between va and vb
// Returns:    distance between va and vb
// Author(s):   Conan Reis
inline f32 a_dist(const AVec2i & va, const AVec2i & vb)
  {
  int dx = va.m_x - vb.m_x;
  int dy = va.m_y - vb.m_y;

  return a_sqrt(f32((dx * dx) + (dy * dy)));
  }

//---------------------------------------------------------------------------------------
// Gets the squared distance between va and vb
// Returns:    squared distance between va and vb
// Author(s):   Conan Reis
inline f32 a_dist_sqr(const AVec2i & va, const AVec2i & vb)
  {
  int dx = va.m_x - vb.m_x;
  int dy = va.m_y - vb.m_y;

  return f32((dx * dx) + (dy * dy));
  }
