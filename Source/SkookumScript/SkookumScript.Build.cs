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

    switch (Target.Platform)
    {
      case UnrealTargetPlatform.Win64:
        platformName = "Win64";
        platPathSuffixes.Add(Path.Combine(platformName, Target.WindowsPlatform.Compiler == WindowsCompiler.VisualStudio2019 ? "VS2019" : "VS2017"));
        break;
      case UnrealTargetPlatform.Mac:
        platformName = "Mac";
        platPathSuffixes.Add(platformName);
        useDebugCRT = true;
        break;
      case UnrealTargetPlatform.Linux:
        platformName = "Linux";
        platPathSuffixes.Add(platformName);
        useDebugCRT = true;
        //UEBuildConfiguration.bForceEnableExceptions = true;
        break;
      case UnrealTargetPlatform.IOS:
        platformName = "IOS";
        platPathSuffixes.Add(platformName);
        useDebugCRT = true;
        break;
      case UnrealTargetPlatform.TVOS:
        platformName = "TVOS";
        platPathSuffixes.Add(platformName);
        useDebugCRT = true;
        break;
      case UnrealTargetPlatform.Android:
        platformName = "Android";
        platPathSuffixes.Add(Path.Combine(platformName, "ARM"));
        platPathSuffixes.Add(Path.Combine(platformName, "ARM64"));
        platPathSuffixes.Add(Path.Combine(platformName, "x86"));
        platPathSuffixes.Add(Path.Combine(platformName, "x64"));
        useDebugCRT = true;
        break;
      case UnrealTargetPlatform.XboxOne:
        platformName = "XONE";
        platPathSuffixes.Add(platformName);
        break;
      case UnrealTargetPlatform.PS4:
        platformName = "PS4";
        platPathSuffixes.Add(platformName);
        break;
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
    PublicDependencyModuleNames.Add("AgogCore");

    PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
  }    
}
