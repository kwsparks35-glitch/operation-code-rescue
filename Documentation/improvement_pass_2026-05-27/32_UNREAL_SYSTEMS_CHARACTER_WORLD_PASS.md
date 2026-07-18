# May 27 Unreal Systems Character and World Pass

Current packaged review is superseded by
`Documentation/improvement_pass_2026-05-27/33_PUBLIC_DEMO_FAB_DETAIL_PASS.md`,
which rebuilds the Mac app again after adding the public-demo Fab/detail polish
layer.

## Purpose

This pass begins direct in-game incorporation of the broader Unreal systems
pipeline for Operation Code Rescue. The goal is to make MetaHuman-ready
characters, Maya/Houdini handoff, PCG/Houdini city design, Chaos physics, NPC
AI, quest/mission kits, and cinematic rigging systems visible and testable in
the regular gameplay environment rather than leaving them as external notes.

## Gameplay incorporation

- Added `SpawnUnrealSystemsCharacterWorldLayer` to every generated campaign city.
- Added novel character slots for Rhea Calder, Mika Stone, Noor Vance, Jules
  Ardent, and Ilan Cross using live `AFriendlyNPCActor` instances.
- Tagged the new cast with `MetaHumanReadyCharacterDesign`,
  `MayaHoudiniDccHandoff`, `SequencerControlRigIKGroomReady`,
  `ControlRigFacialSlot`, `IKRetargeterReady`, `GroomCardFallbackReady`, and
  `QuestMissionKitReady` so future authored assets have concrete replacement
  targets.
- Added DCC intake staging for local Maya/Houdini/FBX/USD/Alembic exports.
- Added PCG/Houdini city review cells tagged `HoudiniProceduralWorldDesign`,
  `PCGWorldPartitionCell`, `WorldPartitionReady`, and `PCGRouteSplineReady`.
- Added Chaos/async-physics-ready movable cover props with simulation enabled
  and projectile target tags.
- Added AI encounter director nodes tagged `NPCBehaviorTreeReady`,
  `StateTreeEQSReady`, `AIPatrolRouteNode`, `AICoverSignal`, and
  `EnemyEncounterDirector`.
- Added quest/mission kit boards tied to the active city terminal, survivor,
  and cinematic rescue beat.

## MCP work

- Advanced the local Fab/Unreal macOS MCP server to version 0.3.0.
- Added `UNREAL_CHARACTER_WORLD_DEVELOPMENT_TRACKS`.
- Added the `unreal_character_world_development_plan` MCP tool.
- Added the `unreal://project/current/character-world-development-plan`
  resource.
- Included the development plan in project scans and generated asset plans.

## Local manifests

- `Content/CodeRescueData/unreal_systems_character_world_manifest.tsv`
  documents each Unreal system, its gameplay hook, local inputs, validation
  path, and Mac caveat.
- `Content/CodeRescueData/novel_character_world_design_manifest.tsv`
  documents the current novel cast/world concepts and where they appear in
  game.

## Runtime correction

- Corrected the in-engine MATLAB-compatible reverse validator so shipped
  `fliplr(...)` solutions are recognized during curriculum commandlet QA.
- This was found during the comprehensive playability run, rebuilt into the
  module, and revalidated before packaging.

## Honesty boundary

This pass does not claim to download licensed MetaHuman, Maya, Houdini, Fab, or
Marketplace assets. It creates in-game slots, manifests, MCP hooks, and
validation targets so local, user-owned exports can be incorporated safely.

## Mac demo app rebuild and handoff status

Fresh package for review:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

- Size: `1.9G`
- Timestamp from `stat`: `May 27 12:27:16 2026`
- Null smoke log: `Saved/Logs/PackagedSmoke_null.log`
- Render smoke log: `Saved/Logs/PackagedSmoke_render.log`

Packaged runtime markers confirmed:

- `[CodeRescueUnrealSystems] 01 New York, NY integrated MetaHuman-ready cast,
  Maya/Houdini DCC intake, PCG/Houdini city cells, Chaos physics props, AI
  director nodes, mission kits, and Sequencer/ControlRig/IK/Groom hooks.`
- `[CodeRescueEntryAccess] 01 New York, NY cleared 78 static blockers from the
  spawn entry corridor.`

Passed validation:

```bash
python3 Scripts/verify_may27_unreal_systems_character_world_pass.py
python3 Scripts/verify_fab_unreal_mcp_porting.py
python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py
python3 Scripts/verify_may27_gameplay_access_pass.py
python3 Scripts/verify_fab_import_and_entry_access.py
python3 Scripts/verify_audit_implementation_closure.py
python3 -m py_compile ../MCP_Server_Development/fab_unreal_macos_mcp/server.py Scripts/verify_may27_unreal_systems_character_world_pass.py Scripts/verify_fab_unreal_mcp_porting.py Scripts/verify_may27_tactical_arsenal_mcp_runtime.py
python3 ../MCP_Server_Development/fab_unreal_macos_mcp/server.py --self-test
python3 ../MCP_Server_Development/fab_unreal_macos_mcp/server.py --audit --write-report
./Recompile_Module.command
Scripts/verify_curriculum_validator_shapes.py commandlet
Scripts/verify_runtime_step_smoke_contracts.py commandlet
Scripts/verify_camera_perspectives_and_character_roster.py commandlet
Scripts/verify_character_world_assets.py commandlet
Scripts/verify_production_track_completion.py commandlet
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
git diff --check
```

Packaged null and render smoke both loaded `/Engine/Maps/Entry` and exited
cleanly. The scanner allowed only the known unattended Mac audio sample-rate
warning in render mode and the existing immediate-quit navigation/crowd
diagnostics.
