REM This is used to build the SkookumScript plugin in preperation for packing for the marketplace.
REM If you are a normal consumer of SkookumScript then you will never need to run this.
REM Notes:
REM Assumptions:
REM  1. SkookumIDE is compiled and exists in the SkookumIDE folder
REM  2. RUN_UAT points to a source version of UE4 (binary will not work)
REM  3. OUT_PATH points to a location outside of the source plugin
REM
REM This uses the UE4 build system to package the SkookumScript Plugin and dump it into an output path.
REM The source version of UE4 is required to perform the build. If the end goal is to use the built
REM plugin with a specific engine version, the BuildId found in Binaries\Win64\UE4Editor.modules will
REM need to be modified to match the BuildId for the target engine version.

@echo off
setlocal

REM The full path to RunUAT.bat
set RUN_UAT="C:/Epic Games/UnrealEngine/Engine/Build/BatchFiles/RunUAT.bat"

REM The full path to the SkookumScript.uplugin file
set SK_PLUGIN="C:/Epic Games/UnrealEngine/Engine/Plugins/Runtime/SkookumScript/SkookumScript.uplugin"

REM Where to put the final build
set OUT_PATH="E:/SkPackage"

REM Set all target platforms with + as delimiter
set PLATFORMS="Win64"

%RUN_UAT% BuildPlugin -Plugin=%SK_PLUGIN% -Package=%OUT_PATH% -CreateSubFolder -TargetPlatforms=%PLATFORMS% -Rocket
exit /B 0