REM This is used to build SkookumScript libraries in preperation for packing for the marketplace.
REM If you are a normal consumer of SkookumScript then you will never need to run this.

@echo off
setlocal

REM === Set some useful folder shortcuts ===
pushd "%~dp0.."
set "MASTER_PLUGIN_DIR=%~dp0
popd

REM === Build the binaries ===
"C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe" %MASTER_PLUGIN_DIR%\Build\SkookumBinaries_Win_msbuild_VS2015.proj
if %errorlevel% neq 0 goto AbortError

REM === Copy DLLs to Binaries folder ===
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\SkookumScript\Lib\Win64\VS2015" "%MASTER_PLUGIN_DIR%\Binaries\Win64" SkookumScript-Win64-*.dll
if errorlevel 8 goto AbortError
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\AgogCore\Lib\Win64\VS2015" "%MASTER_PLUGIN_DIR%\Binaries\Win64" AgogCore-Win64-*.dll
if errorlevel 8 goto AbortError
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\SkookumScript\Lib\Android\ARM" "%MASTER_PLUGIN_DIR%\Binaries\Android\arm" *.a
if errorlevel 8 goto AbortError
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\SkookumScript\Lib\Android\ARM64" "%MASTER_PLUGIN_DIR%\Binaries\Android\arm64" *.a
if errorlevel 8 goto AbortError
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\AgogCore\Lib\Android\ARM" "%MASTER_PLUGIN_DIR%\Binaries\Android\arm" *.a
if errorlevel 8 goto AbortError
robocopy /MIR /NFL /NDL /NJH /NJS /A-:RASH "%MASTER_PLUGIN_DIR%\Source\AgogCore\Lib\Android\ARM64" "%MASTER_PLUGIN_DIR%\Binaries\Android\arm64" *.a
if errorlevel 8 goto AbortError

echo ***
echo *** SUCCESS!
echo ***

exit /B 0

:AbortError
echo ***
echo *** ERRORS ENCOUNTERED!
echo ***

exit /B 20

