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
  commits = list(repo.iter_commits('master', max_count=1))
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

def Build():
  print subprocess.list2cmdline(BuildCommand)
  buildStatus = subprocess.call(BuildCommand, cwd=BuildParams.UE4CWD)
      
  if buildStatus > 0:
      print "Build Failed"
  else:
    print "Setting BuildId..."
    SetSourceBuildID("UE4Editor.modules")
    SetSourceBuildID("UnrealHeaderTool.modules")

    print "Setting VersionName"
    SetVersionName()

    print "Archiving"
    Zip()

    print "Build Success"

Build()