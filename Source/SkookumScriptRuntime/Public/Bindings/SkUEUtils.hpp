// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Unreal Engine Helper Utilities
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

#include <AgogCore/AString.hpp>
#include <AgogCore/ASymbol.hpp>

//=======================================================================================
// Global Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Converts `FName` (similar to `ASymbol`) to `AString`.
// 
// Somewhat dangerous conversion since `FName` does case insensitive comparison and stores
// first string it encounters.
//
// # Author(s): Conan Reis
inline AString FNameToAString(const FName & name)
  {
  FString fstr;
  name.ToString(fstr);

  return AString(*fstr);
  }

//---------------------------------------------------------------------------------------
inline AString FStringToAString(const FString & str)
  {
  // $Revisit - CReis Look into StringCast<>  Engine\Source\Core\Public\Containers\StringConv.h
  return AString(*str);
  }

//---------------------------------------------------------------------------------------
inline ASymbol FStringToASymbol(const FString & str)
  {
  // $Revisit - CReis Look into StringCast<>
  return ASymbol::create(AString(*str));
  }

//---------------------------------------------------------------------------------------
inline FString AStringToFString(const AString & str)
  {
  // $Revisit - CReis Look into StringCast<>
  return FString(str.as_cstr());
  }

//---------------------------------------------------------------------------------------
// Converts `AString` to `FName` (similar to `ASymbol`). If this is the first time this
// string is encountered in the `FName` system it stores in a string table for later
// conversion.
// 
// Somewhat dangerous conversion since `FName` does case insensitive comparison and stores
// first string it encounters. So `length` will match an originally stored `Length`.
//
// # Author(s): Conan Reis
inline FName AStringToFName(const AString & str)
  {
  // $Revisit - CReis Look into StringCast<>
  return FName(str.as_cstr());
  }

//---------------------------------------------------------------------------------------
// Converts `AString` to `FName` (similar to `ASymbol`) if it already exist. If the string
// does not already exist as an `FName` then return `FNAME_Find`.
// 
// Somewhat dangerous conversion since `FName` does case insensitive comparison and stores
// first string it encounters. So `length` will match an originally stored `Length`.
//
// # Author(s): Conan Reis
inline FName AStringToExistingFName(const AString & str)  
  {
  // $Revisit - CReis Look into StringCast<>
  return FName(str.as_cstr(), FNAME_Find);
  }
