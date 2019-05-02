// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ARandom class defintion module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AgogCore.hpp> // Always include AgogCore first (as some builds require a designated precompiled header)
#include <AgogCore/ARandom.hpp>
#ifdef A_INL_IN_CPP
  #include <AgogCore/ARandom.inl>
#endif


//=======================================================================================
// Class Data
//=======================================================================================

// Common random number generator - defined in AgogCore/AgogCore.cpp since it its initialization
// order may be important
//ARandom ARandom::ms_gen;
