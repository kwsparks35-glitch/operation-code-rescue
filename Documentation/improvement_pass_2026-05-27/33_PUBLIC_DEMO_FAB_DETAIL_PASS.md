# May 27 Public Demo Fab Detail Pass

## Purpose

This pass continues the push from prototype toward a reviewable public-demo
presentation by adding denser, more deliberate in-world detail using the
project's currently available local assets and Fab-derived content. The focus is
the playable route: the game should look more authored the moment the player
enters a level, without adding new exterior barriers or hiding the mission flow.

## Gameplay incorporation

- Added `SpawnPublicDemoFabDetailLayer` to every generated campaign city after
  the existing production completion layer and before entry-corridor collision
  cleanup.
- Added street-level public-demo composition: wet ground, readable route paint,
  parallax storefronts, glass windows, door frames, wall lamps, and practical
  lighting.
- Added a ModernBridges Fab hero overpass with concrete pylons, emergency
  lighting, tactical cover, and scale-reference tags.
- Added coding mission-room dressing around the terminal: tech floor, lab
  backwall, animated monitors, debug table, shelf, and blue practical light.
- Added survivor-room dressing: worn wood floor, couch, ceiling lamp, supply
  boxes, relief symbol, medkit pickup, and warm practical light.
- Added a local Fab/design coverage gallery showing current local design roots:
  Modern Bridges, Parallax City, Dog Zombie, Urban Zombie, Groom/Hair, and
  Starter Props.
- Added threat-foreshadowing detail near the deeper route: statue, warning
  spikes, red lighting, and boss-route polish tags.
- Added gameplay-useful pickups inside the detailed spaces so the polish also
  supports playability.

## MCP work

- Advanced the local Fab/Unreal macOS MCP server to version 0.4.0.
- Added `PUBLIC_DEMO_FAB_DETAIL_TRACKS`.
- Added the `public_demo_fab_detail_plan` MCP tool.
- Added the `unreal://project/current/public-demo-fab-detail-plan` resource.
- Included the public-demo detail plan in project scans, generated asset plans,
  and MCP self-test output.

## Local manifest

- `Content/CodeRescueData/public_demo_fab_detail_manifest.tsv` documents the
  public-demo detail systems, live gameplay hooks, local asset sources,
  validation path, and Mac/Fab caveats.

## Honesty boundary

This pass does not claim to download missing Fab, Marketplace, MetaHuman, Maya,
or Houdini assets. It uses currently local project content and exposes the
in-game replacement points where user-owned materialized assets can be promoted
after validation.

## Mac demo app rebuild and handoff status

Fresh package for review:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

- Size: `1.9G`
- Timestamp from `stat`: `May 27 12:48:45 2026`
- Null smoke log: `Saved/Logs/PackagedSmoke_null.log`
- Render smoke log: `Saved/Logs/PackagedSmoke_render.log`

Packaged runtime markers confirmed:

- `[CodeRescueUnrealSystems] 01 New York, NY integrated MetaHuman-ready cast,
  Maya/Houdini DCC intake, PCG/Houdini city cells, Chaos physics props, AI
  director nodes, mission kits, and Sequencer/ControlRig/IK/Groom hooks.`
- `[CodeRescuePublicDemoQuality] 01 New York, NY integrated public-demo detail,
  expanded local Fab set pieces, authored storefronts, combat cover,
  mission-room dressing, survivor room polish, and threat foreshadowing.`
- `[CodeRescueEntryAccess] 01 New York, NY cleared 78 static blockers from the
  spawn entry corridor.`

Passed validation:

```bash
python3 Scripts/verify_may27_public_demo_fab_detail_pass.py
python3 Scripts/verify_fab_unreal_mcp_porting.py
python3 Scripts/verify_may27_unreal_systems_character_world_pass.py
python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py
python3 -m py_compile ../MCP_Server_Development/fab_unreal_macos_mcp/server.py Scripts/verify_may27_public_demo_fab_detail_pass.py Scripts/verify_fab_unreal_mcp_porting.py Scripts/verify_may27_unreal_systems_character_world_pass.py Scripts/verify_may27_tactical_arsenal_mcp_runtime.py
python3 ../MCP_Server_Development/fab_unreal_macos_mcp/server.py --self-test
python3 ../MCP_Server_Development/fab_unreal_macos_mcp/server.py --audit --write-report
./Recompile_Module.command
Scripts/verify_runtime_step_smoke_contracts.py commandlet
Scripts/verify_character_world_assets.py commandlet
Scripts/verify_camera_perspectives_and_character_roster.py commandlet
Scripts/verify_production_track_completion.py commandlet
Scripts/verify_curriculum_validator_shapes.py commandlet
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
git diff --check
```

Packaged null and render smoke both loaded `/Engine/Maps/Entry` and exited
cleanly. The scanner allowed only the known unattended Mac CoreAudio
sample-rate query warning in render mode plus the existing immediate-quit
navigation/crowd diagnostics.
