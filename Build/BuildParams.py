import os

# Where the build will be copied when it is completed, make sure this is
# outside of the UE4 project.
OutFolder = r"e:\SkookumScript"

# The full path to the binary UE4 installation that this will be built for
BinaryUE4 = r"e:\Epic\UE_4.24"

# The full path to the source UE4 installation that we will build with
SourceUE4 = r"c:\Epic Games\4.24"

# The full path to the SkookumScript plugin
SkPluginPath = r"C:\Epic Games\4.24\Engine\Plugins\Runtime\SkookumScript"

###
### Don't edit below this line
###
RunUAT = os.path.join(SourceUE4, "Engine", "Build", "BatchFiles", "RunUAT.bat")
SourceWin64 = os.path.join(SourceUE4, "Engine", "Binaries", "Win64")
BinaryWin64 = os.path.join(BinaryUE4, "Engine", "Binaries", "Win64")
OutWin64 = os.path.join(OutFolder, "Binaries", "Win64")
SkPlugin = os.path.join(SkPluginPath, "SkookumScript.uplugin")
UE4CWD = os.path.join(SourceUE4, "Engine", "Build", "BatchFiles")