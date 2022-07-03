import shutil
import subprocess
import distutils.dir_util
import sys, getopt
import os
from git import Repo
import json
import BuildParams

BuildCommand = [
    BuildParams.RunUAT,
    r"BuildPlugin",
    r"-Plugin=" + BuildParams.SkPlugin,
    r"-iwyu",
    r"-noubtmakefiles",
    r"-Package=" + BuildParams.OutFolder,
    r"-CreateSubFolder",
    r"-TargetPlatforms=Android+Win64",
    r"-Rocket"
]

def GetBinaryBuildID(file):
  with open(os.path.join(BuildParams.BinaryWin64, file), 'r') as f:
    data = json.load(f)
    return data['BuildId']

def SetSourceBuildID(file):
  # Set UE4Editor.modules and UnrealHeaderTool.modules
  build_num = GetBinaryBuildID(file)

  with open(os.path.join(BuildParams.OutWin64, file), 'r+') as f:
    data = json.load(f)
    data['BuildId'] = build_num
    f.seek(0)
    json.dump(data, f, indent=4)
    f.truncate()

def GetCommitSHAStr():
  repo = Repo(BuildParams.SkPluginPath)
  print("Building for active branch: " + str(repo.active_branch) + " from " + BuildParams.SkPluginPath)
  commits = list(repo.iter_commits(max_count=1))
  for commit in commits:
    return str(commit.hexsha[:7])

def GetUE4VerStr():
  # Get UE4 version #
  with open(os.path.join(BuildParams.BinaryUE4, "Engine", "Build", "Build.version"), 'r') as f:
    data = json.load(f)
    return str(data['MajorVersion']) + "." + str(data['MinorVersion']) + "." + str(data['PatchVersion'])

def SetVersionName():
  # modify SkookumScript.uplugin and set VersionName to:
  # [UE4 Version]-[First 7 digits of commit sha]
  ver_name = GetUE4VerStr() + "-" + GetCommitSHAStr()
  
  with open(os.path.join(BuildParams.OutFolder, "SkookumScript.uplugin"), 'r+') as f:
    data = json.load(f)
    data['VersionName'] = ver_name
    data['Version'] += 1
    f.seek(0)
    json.dump(data, f, indent=4)
    f.truncate()

ZipCommand = [
  "7z",
  "a",
  "-r",
  "SkookumScript-" + GetUE4VerStr() + "-" + GetCommitSHAStr() + ".7z",
  os.path.join(BuildParams.OutFolder, "*")
]

def Zip():
  ver_name = GetUE4VerStr() + "-" + GetCommitSHAStr()
  subprocess.call(ZipCommand)

def CheckSkPluginsExist():
  exists_in_binary = os.path.isdir(os.path.join(BuildParams.SourceUE4, "Engine", "Plugins", "Runtime", "SkookumScript")) or \
                   os.path.isdir(os.path.join(BuildParams.SourceUE4, "Engine", "Plugins", "SkookumScript"))
  exists_in_source = os.path.isdir(os.path.join(BuildParams.BinaryUE4, "Engine", "Plugins", "Runtime", "SkookumScript")) or \
                   os.path.isdir(os.path.join(BuildParams.BinaryUE4, "Engine", "Plugins", "SkookumScript"))
  return exists_in_binary or exists_in_source

def CheckIniFileExists():
  return os.path.isfile(os.path.join(BuildParams.SkPluginPath, "SkookumIDE", "SkookumIDE-user.ini"))

def CheckIDEMissing():
  return not os.path.isdir(os.path.join(BuildParams.SkPluginPath, "SkookumIDE", "Engine"))

def Build():
  if CheckSkPluginsExist():
    print("Build Failed: SkookumScript plugin already exists in Binary and/or Source engine. Remove it before building.")
    return -1

  if CheckIniFileExists():
    print("Build Failed: SkookumIDE-user.ini file exists in SkookumIDE folder.")
    return -1

  if CheckIDEMissing():
    print("Build Failed: SkookumIDE/Engine folder is missing")
    return -1

  print(subprocess.list2cmdline(BuildCommand))
  buildStatus = subprocess.call(BuildCommand, cwd=BuildParams.UE4CWD)
      
  if buildStatus > 0:
      print("Build Failed")
      return -1
  else:
    print("Setting BuildId...")
    SetSourceBuildID("UE4Editor.modules")
    SetSourceBuildID("UnrealHeaderTool.modules")

    print("Setting VersionName")
    SetVersionName()

    print("Archiving")
    Zip()

    print("Build Success")
    return 0

Build()