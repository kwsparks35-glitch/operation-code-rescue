# Verification and Mac Package Record

Date: 2026-07-09

This record describes the earlier production-world/V4 archive and remains for
traceability. The current bundle version, V5 acceptance matrix, and final
packaged evidence are in `FIRST_LEVEL_V5_FINAL_TEST_AND_PACKAGE.md`.

## Build Results

### Editor module

Command:

```text
./Recompile_Module.command
```

Result: PASS. UnrealBuildTool compiled and linked
`UnrealEditor-CodeRescueUnreal.dylib` with no compile errors.

### Final Mac package

Command:

```text
./Package_Mac_App.command
```

Result: PASS. The final clean BuildCookRun staged the complete UE data tree,
included the first-level V4 and wound-decal dependencies, signed the app, and
archived:

```text
PackagedMac/Mac/CodeRescueUnreal.app
```

The final archive mounted 1,764 IoStore packages. Bundle version:
`51494982.0.194`.

## Focused Automated Tests

The following tests passed after the final source changes:

- `verify_production_presentation_camera_pass_2026_07_09.py`
- `verify_launch_language_start_screen_save_pass.py`
- `verify_menu_movement_v3art_2026_07_06.py`
- `verify_gameplay_fixes_2026_07_07.py`
- `verify_distinct_weapon_presentation_slice_pass.py`
- `verify_jeep_surface_vehicle_physics_slice_pass.py`
- `verify_creative_physics_world_slice_pass.py`
- `verify_first_level_combat_experience_pass_2026_07_09.py`
- `verify_gameplay_fixes_2026_07_07.py`
- collision-channel, combat-juice, death-physics, physical-animation,
  quick-slot armory, and menu/movement/V3-art acceptance verifiers

The production verifier confirmed 19 regenerated CityKitV3 GLBs, five
regenerated WeaponsV3 GLBs, matching Unreal static-mesh assets, launch selector
ownership, cross-street layout, production character rigs, camera occlusion,
and all three review images.

The first-level verifier confirmed 41 requirement contracts plus its final
acceptance result. Coverage includes aim and jump input, live armory rendering,
17-entry selection/equip behavior, exact hit geometry, localized wounds,
grounded death and corpse fading, cook inclusion, nine Blender sources, nine
Unreal meshes, first-level-only world gating, and both deterministic runtime
audit hooks.

## Unreal Commandlet Tests

`verify_camera_perspectives_and_character_roster.py` was updated from obsolete
roofline camera lengths to the current street-canyon contract, then run through
`UnrealEditor-Cmd`. It loaded the roster and cycled all six perspectives three
times. Result: PASS, 0 errors, 0 warnings.

`verify_character_world_assets.py` was also run through `UnrealEditor-Cmd` with
NullRHI. It loaded and validated the production character, animation, zombie,
building, bridge, prop, and world assets. Result: PASS, 0 errors, 0 warnings.

`Scripts/inspect_first_level_v4_meshes_unreal.py` loaded all nine V4 assets in
`UnrealEditor-Cmd` and verified render triangles, LODs, materials, and sane
bounds. Result: PASS, 9/9 assets. Evidence:
`TestLogs/FirstLevelV4MeshAudit.log`.

## Editor Visual Acceptance

The editor game was launched at 1280x720 with:

```text
-VisualReviewStart -ProductionReviewCapture -NoRadioVoice -NoSound
```

The deterministic capture showed:

- full player silhouette and level horizon;
- unobstructed center intersection;
- no overhead cloud plate;
- no objective cube towers or pads;
- no briefing wall in front of the camera;
- sidewalks, crosswalks, road details, bus shelter, vehicles, buildings,
  streetlights, signs, trees, and support characters visible together.

## Packaged Launch-Gate Test

The unmodified packaged launch path was held for more than 15 seconds. Runtime
evidence confirmed all six language stations and the launch widget initialized.
There were no `[LaunchGate]` errors, no `FAILSAFE`, no production-world spawn,
and no automatic language deployment. The process exited normally on request.

This test initially exposed the Slate weak-pointer ownership split. The
GameMode-owned resolver was implemented, the app was rebuilt, and the same test
was repeated to obtain the passing result above.

## Packaged Gameplay Test

The final archive was launched with the deterministic production-review flags.
The packaged screenshot was written to the app container and copied to:

```text
Documentation/improvement_pass_2026-07-09/Renders/packaged_production_arrival.png
```

Relevant runtime ledger:

```text
[CityBlockV3] v3 street kit ACTIVE
[Streetscape] 01 New York, NY: 54 spawned, 0 failed
[CityBlockV3] 01 New York, NY: 38 spawned, 0 failed
[FirstLevelCombatArtV4] ... spawned 4 authored structures/cover
[ProductionSky] ... geometric cloud plates omitted
[ProductionRoute] ... prototype pads and pillars omitted
[ProductionWorld] ... curated=1 development_showcases=0
[ProductionPresentation] ... review actors=99 arrival blockers=0
[CharacterPresentation] Player uses production Manny locomotion rig
[FirstLevelAim] runtime pose copy configured for SKM_Manny
[HeldWeapon] body weapon 'PistolV3' attached via hand_R
```

The current archive passed both packaged smoke modes, then completed a real
17-entry armory cycle and the integrated jump/bite/two-shot/wound/death/corpse/
fade audit. The package returned exit code 0 for every run. Detailed evidence
and exact COMPLETE PASS lines are recorded in
`FIRST_LEVEL_TEST_AND_PACKAGE.md` and `TestLogs/`.

## Package Integrity

Command:

```text
python3 Scripts/verify_package_integrity_pass.py
```

Result: local package integrity PASS. The verifier recorded bundle identifier
`com.operationcoderescue.CodeRescueUnreal`, local readiness `true`, and size
`2049.1 MB` in `Saved/Release/package_integrity_latest.json`.

Command:

```text
codesign --verify --deep --strict --verbose=2 \
  PackagedMac/Mac/CodeRescueUnreal.app
```

Result: valid on disk and satisfies its Designated Requirement.

## External Release Note

The package uses local ad-hoc signing. Gatekeeper distribution requires an
Apple Developer ID certificate and notarization credentials; that external
credential step remains pending and is correctly reported as
`external_ready=false` by the integrity verifier.
