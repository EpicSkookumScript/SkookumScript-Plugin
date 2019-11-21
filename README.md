# SkookumScript Plugin
[![SkookumScript UE4 Plugin Intro hood](https://i.vimeocdn.com/filter/overlay?src0=https%3A%2F%2Fi.vimeocdn.com%2Fvideo%2F527429587_640.jpg&src1=http%3A%2F%2Ff.vimeocdn.com%2Fp%2Fimages%2Fcrawler_play.png)](https://vimeo.com/133828708 "SkookumScript UE4 Plugin Intro - Click to Watch!")

SkookumScript is the superpowered scripting solution and cutting-edge command console for UE4. It is powerful and feature-rich, yet simple and easy to learn. It has native game concepts and deep integration with UE4 Blueprints and C++. SkookumScript allows the live creation of sophisticated gameplay with surprisingly few lines of code. It has been lovingly crafted over more than a decade, [battle-tested on major game titles](http://skookumscript.com/about/#sleeping-dogs) and supports all platforms. SkookumScript is made for games.

![SkookumScript IDE Screenshots](http://skookumscript.com/images/galleries/Screens.png)

## Build Instructions

### SkookumIDE (Required)
To use the source version of SkookumScript, you will also need he SkookumIDE binaries. The easiest way to acquire these is to download the [latest release](https://github.com/EpicSkookumScript/SkookumScript-Plugin/releases) and overwrite the `SkookumIDE` folder in this repo with the one from the release. You can also build the IDE from source by following [these instructions](https://github.com/EpicSkookumScript/SkookumIDE). Note that to access the SkookumIDE repo, you need to [have access to the Unreal Engine 4 repository](https://www.unrealengine.com/en-US/ue4-on-github).

### Clone the repo
#### Binary Engine
Place the SkookumScript plugin into your project's plugin folder. Example: `MyProject/Plugins/SkookumScript`.

#### Source Engine
You can choose whether to place the SkookumScript plugin into the engine's `Runtime/Plugins` folder or in your project's plugin folder.

### Build
1. Build the SkookumIDE
2. Copy the built SkookumIDE folder from the SkookumIDE project folder `Engine/Plugins/SkookumScript/SkookumIDE` to the SkookumScript Plugin folder `Plugins/SkookumScript`. Note that this will overwrite the existing `SkookumIDE` folder.
3. Regenerate project files. If you placed the plugin into the engine folder then run `GenerateProjectFiles.bat` otherwise right-click your `.uproject` and select *Generate Visual Studio Project Files*.
4. Build your project/engine as usual.
