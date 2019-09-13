// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

using System.IO;
using System.Net;
using System.Diagnostics;
using System.Collections.Generic;
using UnrealBuildTool;


public class SkookumScript : ModuleRules
{
  public SkookumScript(ReadOnlyTargetRules Target) : base(Target)
  {
    bRequiresImplementModule = false;

    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    // Ignore warnings about hokey code in windows.h
    bEnableUndefinedIdentifierWarnings = false;

    // Force to full source now that we are open source
    Type = ModuleType.CPlusPlus;
    
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
    }
    else if(Target.Platform == UnrealTargetPlatform.Linux)
    {
      platformName = "Linux";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
      //UEBuildConfiguration.bForceEnableExceptions = true;
    }
    else if(Target.Platform == UnrealTargetPlatform.IOS)
    {
      platformName = "IOS";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
    }
    else if(Target.Platform == UnrealTargetPlatform.TVOS)
    {
      platformName = "TVOS";
      platPathSuffixes.Add(platformName);
      useDebugCRT = true;
    }
    else if(Target.Platform == UnrealTargetPlatform.Android)
    {
      platformName = "Android";
      platPathSuffixes.Add(Path.Combine(platformName, "ARM"));
      platPathSuffixes.Add(Path.Combine(platformName, "ARM64"));
      platPathSuffixes.Add(Path.Combine(platformName, "x86"));
      platPathSuffixes.Add(Path.Combine(platformName, "x64"));
      useDebugCRT = true;
    }
    else if(Target.Platform == UnrealTargetPlatform.XboxOne)
    {
      platformName = "XONE";
      platPathSuffixes.Add(platformName);
    }
    else if(Target.Platform == UnrealTargetPlatform.PS4)
    {
      platformName = "PS4";
      platPathSuffixes.Add(platformName);
    }

    // NOTE: All modules inside the SkookumScript plugin folder must use the exact same definitions!
    switch (Target.Configuration)
    {
      case UnrealTargetConfiguration.Debug:
      case UnrealTargetConfiguration.DebugGame:
        PublicDefinitions.Add("SKOOKUM=31");
        break;

      case UnrealTargetConfiguration.Development:
      case UnrealTargetConfiguration.Test:
        PublicDefinitions.Add("SKOOKUM=31");
        break;

      case UnrealTargetConfiguration.Shipping:
        PublicDefinitions.Add("SKOOKUM=8");
        break;
    }

    // Determine if monolithic build
    var bIsMonolithic = (Target.LinkType == TargetLinkType.Monolithic);

    if (!bIsMonolithic)
    {
      PublicDefinitions.Add("SK_IS_DLL");
    }

    // Public include paths
    PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

    // Public dependencies
    PublicDependencyModuleNames.AddRange(new string[] { "AgogCore", "Core" });
    PrivateDependencyModuleNames.Add("Core");

    PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
  }    
}
