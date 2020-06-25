import os

# Where the build will be copied when it is completed, make sure this is
# outside of the UE4 project.
OutFolder = r"e:\SkBuilds\FinalBuild"

# The full path to the binary UE4 installation that this will be built for
BinaryUE4 = r"C:\Epic Games\UE_4.25"

# The full path to the source UE4 installation that we will build with
SourceUE4 = r"C:\UnrealEngine"

# The full path to the SkookumScript plugin, this should not be placed inside
# the engine folder.
SkPluginPath = r"e:\SkBuilds\SkookumScript\SkookumScript-4.25"

###
### Don't edit below this line
###
RunUAT = os.path.join(SourceUE4, "Engine", "Build", "BatchFiles", "RunUAT.bat")
SourceWin64 = os.path.join(SourceUE4, "Engine", "Binaries", "Win64")
BinaryWin64 = os.path.join(BinaryUE4, "Engine", "Binaries", "Win64")
OutWin64 = os.path.join(OutFolder, "Binaries", "Win64")
SkPlugin = os.path.join(SkPluginPath, "SkookumScript.uplugin")
UE4CWD = os.path.join(SourceUE4, "Engine", "Build", "BatchFiles")