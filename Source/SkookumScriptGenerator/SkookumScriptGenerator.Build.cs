// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
  public class SkookumScriptGenerator : ModuleRules
  {
    public SkookumScriptGenerator(ReadOnlyTargetRules Target) : base(Target)
    {			
      // Tell build system we're not using PCHs
      PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

      PublicIncludePaths.AddRange(
        new string[] {					
          "Programs/UnrealHeaderTool/Public",
          // ... add other public include paths required here ...
        }
        );

      PrivateIncludePaths.AddRange(
        new string[] {
          // "Developer/SkookumScriptGenerator/Private",
          // ... add other private include paths required here ...
        }
        );

      PublicDependencyModuleNames.AddRange(
        new string[]
        {
          "Core",
          "CoreUObject",
          // ... add other public dependencies that you statically link with here ...
        }
        );

      PrivateDependencyModuleNames.AddRange(
        new string[]
        {
          // ... add private dependencies that you statically link with here ...
        }
        );

      DynamicallyLoadedModuleNames.AddRange(
        new string[]
        {
          // ... add any modules that your module loads dynamically here ...
        }
        );
    }
  }
}