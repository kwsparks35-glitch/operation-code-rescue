# Operation Code Rescue Unreal — macOS Build Guide

This is an Unreal Engine C++ source project for a first-person post-apocalyptic coding rescue game.

## What this project currently includes

- First-person navigation using WASD + mouse look
- Left mouse shooting
- Interact with survivors, language stations, and coding terminals using **E**
- Java, C, Python, and MATLAB coding modes
- MATLAB desktop launch support from in-game MATLAB terminals
- MATLAB batch validation support through your installed MATLAB
- 465-mission campaign scope: ranks 1-342 are the U.S. major-city segment and
  ranks 343-465 are the global city extension
- Procedurally generated zones inspired by:
  - Anchorage Medical District
  - Seattle Harbor Evac Line
  - Tokyo Metro Shelter Route
- Zombies, survivors, language stations, terminals, lighting, fog, and HUD
- No binary `.umap` dependency: the world is generated from C++ at runtime in the engine entry map

## Requirements

1. macOS
2. Unreal Engine 5.x installed through Epic Games Launcher
3. Xcode command line tools installed
4. MATLAB installed for MATLAB mode
5. Java / clang / python3 available for those respective validation modes

## Quick start

1. Double-click `Generate_Xcode_Project.command`.
2. When Unreal opens, allow it to build missing modules if prompted.
3. Press **Play**.

## Controls

- WASD: move
- Mouse: look
- Left Mouse: shoot
- E: interact
- Q: medkit

## Language selection

At spawn, interact with one of the language stations:

- JAVA
- C
- PYTHON
- MATLAB

Then approach a glowing coding terminal and press **E**.

## MATLAB behavior

For MATLAB mode, the game tries:

1. `MATLAB_BIN`
2. `MATLABROOT/bin/matlab`
3. `/Applications/MATLAB_R*.app/bin/matlab`
4. `matlab` from PATH

Trusted local QA must still launch with `-AllowExternalCodeValidation` before
MATLAB, Java, C/C++, or Python subprocess validators run. When that opt-in is
enabled, PATH-only MATLAB installs are probed with `matlab -batch "exit"` and
validated through `/usr/bin/env matlab -batch ...`.

If MATLAB does not launch, set one of these environment variables before launching Unreal:

```zsh
export MATLAB_BIN="/Applications/MATLAB_R2025b.app/bin/matlab"
```

or:

```zsh
export MATLABROOT="/Applications/MATLAB_R2025b.app"
```

Adjust the version name to match your installed MATLAB version.

## Package as a Mac app

Double-click:

```text
Package_Mac_App.command
```

The output will appear in:

```text
PackagedMac/
```

Current Mac bundle identifier:

```text
com.operationcoderescue.CodeRescueUnreal
```

For the current non-human continued-improvement evidence pass, run:

```text
Run_NoHuman_Next20_Improvement.command
```

It refreshes control-profile, static performance, and top-twenty
recommendation evidence under `Saved/`.

For the second no-human continued-improvement pass, run:

```text
Run_NoHuman_Next20_Round2_Improvement.command
```

It refreshes the release dashboard, input/curriculum/localization audits,
visual-readability metrics, source-control slice summary, and round-two
recommendation evidence under `Saved/Release/`.

## Important note

This is a real Unreal C++ playable prototype, not a finished commercial-quality AAA FPS. It is intended as a strong foundation that can be expanded with professional art assets, animations, MetaHumans, Niagara effects, and level design.

## Pro asset/system upgrade added

This version expands the original Unreal prototype with the next production layer:

- optional animated zombie skeletal mesh slots
- optional survivor skeletal mesh slots
- Niagara VFX hooks for muzzle flashes, impacts, infection, death, and rescue beacons
- persistent save-game scaffolding
- deeper curriculum JSON database for Java, C, Python, MATLAB, and debugging strategies
- asset-import folders under `Content/CodeRescueAssets/`
- documentation for adding professional assets and curriculum content

See:

```text
Documentation/PRO_ASSET_AND_SYSTEM_UPGRADE.md
Documentation/CURRICULUM_AUTHORING_GUIDE.md
Documentation/MISSION_SAVE_SYSTEM_NOTES.md
Content/CodeRescueData/curriculum_database.json
```

## Professional assets

This package does not include paid/proprietary Marketplace, Fab, or MetaHuman assets. Import those on your Mac through Unreal/Fab according to their license, then assign them to the exposed Blueprint/C++ properties.

The prototype still works with primitive fallback geometry if no professional assets are assigned.

## UE 5.7 Build Fix Notes

This package updates both Unreal target files for UE 5.7:

- `Source/CodeRescueUnrealEditor.Target.cs`
- `Source/CodeRescueUnreal.Target.cs`

The target files now use:

```csharp
DefaultBuildSettings = BuildSettingsVersion.V6;
IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
```

This resolves the build error where UnrealBuildTool reported that `CodeRescueUnrealEditor` modifies `UndefinedIdentifierWarningLevel: Off != Error`.

If the editor still uses an old partial build receipt, delete these folders before reopening the `.uproject`:

- `Binaries`
- `Intermediate`
- `.vs` if present
- `Saved` if the issue persists

Then reopen `CodeRescueUnreal.uproject` and allow Unreal to rebuild modules.
