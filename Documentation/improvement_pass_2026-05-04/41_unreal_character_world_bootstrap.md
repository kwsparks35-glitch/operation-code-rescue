# Unreal Character + World Bootstrap — 2026-05-04

## Goal

Prepare the current character, zombie, UI, persistence, and world-generation work so the project can open cleanly in the Unreal Editor and continue authoring world content from a compiling editor target.

## Build result

- Target built: `CodeRescueUnrealEditor Mac Development`
- Project: `CodeRescueUnreal.uproject`
- Result: succeeded after integration repairs
- Command:

```bash
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)" && "$ENGINE_ROOT/Build/BatchFiles/Mac/Build.sh" CodeRescueUnrealEditor Mac Development -Project="$(pwd)/CodeRescueUnreal.uproject" -WaitMutex
```

## Character work now compiling

- Player character systems compile with weapon loadouts, magazine reloads, stamina, gamepad support, throwables, barricades, scrap, damage feedback, medkits, terminal interactions, objective/journal/pause UI, and save resource restoration.
- Zombie actors compile as `ACharacter`-based enemies with character movement, AI possession, elite variants, hit-zone damage, death effects, boomer spawn behavior, and persistent neutralized-state tracking.
- Survivor actors compile with rescue persistence, rescue counts, story/radio text, and optional professional mesh/audio assignment.
- The local crafting and skill-tree hooks in `UCodeRescueGameInstance` now link successfully, so Blueprint/menu calls no longer point at missing C++ symbols.

## World-generation work now compiling

- The GameMode world-spawn path compiles with the expanded city layout, city entry access changes, helipads, authored props, radio briefings, weather emitters, secret terminals, day/night cycle, sandbox mode, and persistent objective-state replay.
- Save-game state application is public to the world-generation path, so spawned terminals, survivors, zombies, and player resources can be restored after the generated city content appears.
- Custom challenge JSON loading now maps imported `brief` text into `FChallengeSpec::MissionBrief`, matching the terminal UI and generated mission data model.

## Integration repairs made in this pass

- Fixed UnrealHeaderTool include ordering in `CodeZombieActor.h`.
- Renamed `ApplyDamage`'s reflected `Instigator` parameter to `DamageSource` to avoid shadowing `AActor::Instigator`.
- Exposed save/progress/world-state GameInstance functions that are intentionally called from GameMode, widgets, actors, and terminal validation.
- Replaced unsupported `UButton::OnClicked.AddLambda` slot bindings with reflected save/load/delete slot handlers.
- Added missing Unreal includes for progress bars, capsule components, character movement, actor iterators, and audio device access.
- Implemented/linked the declared crafting and skill-tree GameInstance API surface.
- Added a narrow friend relationship from `UCodeRescueGameInstance` to `ACodeRescueCharacter` for progression systems that mutate protected character resources.

## Character and world asset incorporation

2026-05-04 follow-up pass: local character and world assets are now wired into
the runtime generator instead of only existing as loose Content folders.

- `ACodeRescueGameMode` now has runtime fallback paths for Parallax Night
  Building meshes and ModernBridges meshes.
- The generated city skyline prefers real Parallax building static meshes and
  falls back to cube buildings only if those assets are unavailable.
- `SpawnAuthoredPropsForCity` now places an open-access bridge near the city
  entry corridor, showcase authored building clusters, and harbor bridge spans
  for port-themed cities. The access bridge is tagged `OpenCityAccess` and has
  collision disabled so it cannot recreate the earlier blocked-entry problem.
- Zombie variant selection now falls back to built-in rows when
  `/Game/CodeRescueAssets/DT_ZombieVariants` is missing or empty. The built-in
  rows reference DogZombie, UrbanZombie4, YI Modular Zombie M04/F01,
  ZombieFemale Nurse, and the base Zombie mesh.
- `ACodeZombieActor::InitializeFromVariant` now reapplies professional visuals
  immediately after loading the mesh. This covers Unreal's runtime spawn timing
  where a spawned actor may already have run `BeginPlay` before GameMode assigns
  its variant row.
- `ASurvivorActor` now defaults to the local Quinn skeletal mesh and AnimBP
  when no designer-assigned survivor mesh is present, so survivor teams render
  as real character meshes by default.

## Asset verification

Added:

```text
Scripts/verify_character_world_assets.py
```

Run it with:

```bash
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -run=pythonscript -script="$(pwd)/Scripts/verify_character_world_assets.py" -unattended -NoSound -NullRHI -NoLoadStartupPackages -log
```

Latest verification result:

- `CodeRescueUnrealEditor Mac Development`: build succeeded.
- `verify_character_world_assets.py`: succeeded with 0 errors.
- Expected warning: `SM_postapo_bridge_001` is optional because the file exists
  on disk but is not currently exposed by Unreal's asset registry under the
  expected package path. Runtime bridge loading already falls back to the
  registered modern bridge meshes.

## Editor handoff

2026-05-04 recheck: the missing UE 5.7 Engine Content was restored and the
project now opens in the GUI editor. The asset registry completed its scan
and reported 9,766 uncontrolled assets, including the newly available groom,
bridge, parallax-building, StarterContent, zombie, and MetaHuman-adjacent
asset folders.

The editor was launched successfully with:

```bash
open -a "$(Scripts/find_unreal_mac.sh)/Binaries/Mac/UnrealEditor.app" --args "$(pwd)/CodeRescueUnreal.uproject" -log -NoSplash
```

The earlier Engine installation/content issue is resolved. The project-side
editor target builds successfully and the GUI editor can now engage with
local assets.

Open the project in Unreal Editor and continue by authoring/validating:

- generated city layout scale and traversal,
- navmesh coverage through city entry routes,
- character Blueprint mesh/AnimBP assignments,
- Survivor and zombie asset placement,
- `Maps/MainMenu.umap` and `Maps/Sandbox.umap`,
- Blueprint asset references for weather, radio, ambient audio, and professional character assets.
