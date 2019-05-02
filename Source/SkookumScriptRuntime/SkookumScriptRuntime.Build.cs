// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================


using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;
using Tools.DotNETCommon;

namespace UnrealBuildTool.Rules
{
  public class SkookumScriptRuntime : ModuleRules
  {
    public SkookumScriptRuntime(ReadOnlyTargetRules Target) : base(Target)
    {

      // SkUEBindings.cpp takes a long time to compile due to auto-generated engine bindings
      // Set to true when actively working on this plugin, false otherwise
      bFasterWithoutUnity = false;

      // Tell build system we're not using PCHs
      PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
      
      // Expose bindings and Actor includes to self
      PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Bindings"));
      PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Bindings/Engine"));
      PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Bindings/VectorMath"));

      // Add public dependencies that you statically link with here ...
      PublicDependencyModuleNames.AddRange(
        new string[]
          {
            "Core",
            "CoreUObject",
            "Engine"
          }
        );

      // ... add private dependencies that you statically link with here ...
      PrivateDependencyModuleNames.AddRange(
        new string[]
          {
            "Sockets",
            "HTTP",
            "Networking",
            "NetworkReplayStreaming",
            "Projects",
            "SourceControl",
          }
        );

      if (Target.bBuildEditor)
      {
        PrivateDependencyModuleNames.Add("UnrealEd");
        PrivateDependencyModuleNames.Add("MainFrame");
        PrivateDependencyModuleNames.Add("KismetCompiler");
        PrivateDependencyModuleNames.Add("SourceControl");
      }

      // Load SkookumScript.ini and add any ScriptSupportedModules specified to the list of PrivateDependencyModuleNames
      PublicDependencyModuleNames.AddRange(GetSkookumScriptModuleNames(Path.Combine(ModuleDirectory, "../.."), false));

      // Add any modules that your module loads dynamically here ...
      //DynamicallyLoadedModuleNames.AddRange(new string[] {});

      // Whenever SkookumScript.ini changes, this build script should be re-evaluated
      ExternalDependencies.Add("../../Config/SkookumScript.ini");
    }

    // Load SkookumScript.ini and return any ScriptSupportedModules specified
    public static List<string> GetSkookumScriptModuleNames(string PluginOrProjectRootDirectory, bool AddSkookumScriptRuntime = true)
    {
      List<string> moduleList = null;

      // Load SkookumScript.ini and get ScriptSupportedModules
      string iniFilePath = Path.Combine(PluginOrProjectRootDirectory, "Config/SkookumScript.ini");
      if (File.Exists(iniFilePath))
      {
        ConfigFile iniFile = new ConfigFile(new FileReference(iniFilePath), ConfigLineAction.Add);
        var skookumConfig = new ConfigHierarchy(new ConfigFile[] { iniFile });
        skookumConfig.GetArray("CommonSettings", "ScriptSupportedModules", out moduleList);
      }

      if (moduleList == null)
        {
        moduleList = new List<string>();
        }

      // Add additional modules needed for SkookumScript to function
      moduleList.Add("AgogCore");
      moduleList.Add("SkookumScript");
      if (AddSkookumScriptRuntime)
      {
        moduleList.Add("SkookumScriptRuntime");
      }

      return moduleList;
    }
  }
}
