// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

using System.IO;
using System.Net;
using System.Collections.Generic;
using UnrealBuildTool;


public class AgogCore : ModuleRules
{
  public AgogCore(ReadOnlyTargetRules Target) : base(Target)
  {
    bRequiresImplementModule = false;

    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    // Ignore warnings about hokey code in windows.h
    bEnableUndefinedIdentifierWarnings = false;

    // If full source is present, build module from source, otherwise link with binary library
    Type = ModuleType.CPlusPlus;

    // Enable fussy level of checking (Agog Labs internal)
    ExternalDependencies.Add("enable-mad-check.txt");
    var bMadCheck = File.Exists(Path.Combine(ModuleDirectory, "enable-mad-check.txt"));
    if (bMadCheck)
    {
      PublicDefinitions.Add("A_MAD_CHECK");
    }

    // Add user define if exists (Agog Labs internal)
    ExternalDependencies.Add("mad-define.txt");

    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        "CoreUObject",
        "Engine",
      }
     );

    var userDefineFilePath = Path.Combine(ModuleDirectory, "mad-define.txt");
    if (File.Exists(userDefineFilePath))
    {
      var userDefine = File.ReadAllText(userDefineFilePath).Trim();
      if (userDefine.Length > 0)
      {
        PublicDefinitions.Add(userDefine);
      }
    }

    List<string> platPathSuffixes = new List<string>();

    string platformName = "";
    bool useDebugCRT = Target.bDebugBuildsActuallyUseDebugCRT;

    if(Target.Platform == UnrealTargetPlatform.Win64)
    {
        platformName = "Win64";
        platPathSuffixes.Add(Path.Combine(platformName, Target.WindowsPlatform.Compiler == WindowsCompiler.VisualStudio2019 ? "VS2019" : "VS2017"));
    }
    else if(Target.Platform == UnrealTargetPlatform.Mac)
    {
        platformName = "Mac";
        platPathSuffixes.Add(platformName);
        useDebugCRT = true;
        PublicDefinitions.Add("A_PLAT_OSX");
    }
    else if(Target.Platform == UnrealTargetPlatform.Linux)
    {
      platformName = "Linux";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
      PublicDefinitions.Add("A_PLAT_LINUX64");
      //UEBuildConfiguration.bForceEnableExceptions = true;
    }
    else if(Target.Platform == UnrealTargetPlatform.IOS)
    {
      platformName = "IOS";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
      PublicDefinitions.Add("A_PLAT_iOS");
    }
    else if(Target.Platform == UnrealTargetPlatform.TVOS)
    {
      platformName = "TVOS";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
      PublicDefinitions.Add("A_PLAT_tvOS");
    }
    else if(Target.Platform == UnrealTargetPlatform.Android)
    {
      platformName = "Android";
      platPathSuffixes.Add(Path.Combine(platformName, "ARM"));
      platPathSuffixes.Add(Path.Combine(platformName, "ARM64"));
      platPathSuffixes.Add(Path.Combine(platformName, "x86"));
      platPathSuffixes.Add(Path.Combine(platformName, "x64"));
      useDebugCRT = true;
      PublicDefinitions.Add("A_PLAT_ANDROID");
    }
    else if(Target.Platform == UnrealTargetPlatform.XboxOne)
    {
      platformName = "XONE";
      PublicDefinitions.Add("A_PLAT_X_ONE");
    }
    else if(Target.Platform == UnrealTargetPlatform.PS4)
    { 
      platformName = "PS4";
      PublicDefinitions.Add("A_PLAT_PS4");
    }
    else if (Target.Platform == UnrealTargetPlatform.Switch)
    {
      platformName = "SWITCH";
      PublicDefinitions.Add("A_PLAT_SWITCH");
    }

    // NOTE: All modules inside the SkookumScript plugin folder must use the exact same definitions!
    switch (Target.Configuration)
    {
      case UnrealTargetConfiguration.Debug:
      case UnrealTargetConfiguration.DebugGame:
        PublicDefinitions.Add("A_EXTRA_CHECK=1");
        PublicDefinitions.Add("A_UNOPTIMIZED=1");
        break;

      case UnrealTargetConfiguration.Development:
      case UnrealTargetConfiguration.Test:
        PublicDefinitions.Add("A_EXTRA_CHECK=1");
        break;

      case UnrealTargetConfiguration.Shipping:
        PublicDefinitions.Add("A_SYMBOL_STR_DB=1");
        PublicDefinitions.Add("A_NO_SYMBOL_REF_LINK=1");
        break;
    }

    // Determine if monolithic build
    var bIsMonolithic = (Target.LinkType == TargetLinkType.Monolithic);

    if (!bIsMonolithic)
    {
      PublicDefinitions.Add("A_IS_DLL");
    }

    // Public include paths
    PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

    // We're building SkookumScript from source - not much else needed
    PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
  }
}
