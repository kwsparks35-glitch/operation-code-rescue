# Signed-In Account World Pass - 2026-05-20

## Goal

After the desktop Epic/Unreal account was signed in, continue development in a
way that uses the local account-linked state safely: local Launcher project
registration, locally downloaded Fab/Vault assets, and repeatable editor/demo
entry points.

## Account-Linked Local State Found

- Epic/Unreal app support folders are present under:
  `/Users/labcomputer/Library/Application Support/Epic`
- Fab settings are present under:
  `/Users/labcomputer/Library/Application Support/Epic/FabPlugins/settings_v1.json`
- Local Fab/Vault cache is present under:
  `/Users/Shared/UnrealEngine/Launcher/VaultCache/FabLibrary`
- The project is linked into the normal Unreal project browser path:
  `/Users/labcomputer/Documents/Unreal Projects/CodeRescueUnreal`
- The requested local Unreal support folder exists:
  `/Users/labcomputer/UnrealEngine`

I did not copy new Marketplace/Fab packages out of the Vault Cache blindly.
Instead, this pass uses assets that are already imported into this project
under `Content/` and documents the account handoff path.

## New World Development Completed

- Added `SpawnAccountLinkedAssetShowcase(...)`.
- Every generated city now includes a `FAB / VAULT CONTENT BAY` review area.
- The bay uses the locally imported ModernBridges and Parallax Night Building
  content already present in the project:
  - one authored modern bridge span,
  - three parallax building towers,
  - Vault-cache-style intake crates,
  - a MetaHuman-ready intake marker.
- Actors spawned by this bay are tagged `FabShowcase` and
  `AccountLinkedAsset` for easier editor searching.

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## New Character/Mission Context Completed

- Added `SpawnSurvivorReliefCamp(...)`.
- Each unrescued survivor now gets a small relief camp instead of only a marker:
  - camp ground mat,
  - hazard rail,
  - briefing table,
  - chairs,
  - supply shelf,
  - triage cot,
  - medical cross,
  - civilian profile sign tied to the survivor name and required lesson.
- Camp helper actors are registered with the survivor and are cleared when the
  survivor is rescued.

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## Review Entry Points

- Open the signed-in Unreal Editor project:
  `Open_CodeRescue_In_Unreal_Editor.command`
- Open the playable demo:
  `Run_Character_World_Demo.command`
- In the playable demo, press `T` to move through the objective route. Review:
  - the survivor relief camp near the rescue target,
  - the support hub around the four friendly NPCs,
  - the Fab/Vault content bay in the east side of each generated city,
  - the objective pads and route strips.

## Verification

Run from the project folder:

```bash
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
"$ENGINE_ROOT/Build/BatchFiles/Mac/Build.sh" CodeRescueUnrealEditor Mac Development -Project="$(pwd)/CodeRescueUnreal.uproject" -WaitMutex
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -run=pythonscript -script="$(pwd)/Scripts/verify_character_world_assets.py" -unattended -NoSound -NullRHI -NoLoadStartupPackages -log
```

Latest local validation after this pass:

- `CodeRescueUnrealEditor Mac Development`: build succeeded.
- `Scripts/verify_character_world_assets.py`: succeeded with 0 errors.
- Headless `-game -NullRHI` launch smoke: exited cleanly with code 0.
- Remaining warning: optional `SM_postapo_bridge_001` is not visible to the
  asset registry; runtime bridge selection already falls back to available
  ModernBridges meshes.
