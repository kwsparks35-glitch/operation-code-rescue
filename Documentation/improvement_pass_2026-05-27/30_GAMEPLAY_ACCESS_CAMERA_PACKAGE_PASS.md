# 2026-05-27 Gameplay Access, Camera, And Package Pass

## Purpose

This pass rebuilt the current local Operation Code Rescue work into a fresh Mac
demo app and addressed the playability complaints from the previous packaged
build:

- Characters and enemies now face and pursue the player directly instead of
  drifting sideways.
- The outside city wall/barrier presentation was removed from the generated
  entry flow. Each stop now opens directly into the actual playable level route.
- Camera switching was hardened with direct key bindings and polling fallbacks
  so the player can change view reliably.
- Building footprint and height multipliers were reduced to make streets,
  props, characters, and buildings feel more human-scale and survival-horror
  focused.
- The follow-up tactical arsenal/runtime pass was folded into the current
  package, so this handoff now points at the newest May 27 build rather than the
  earlier May 27 package.
- Demo and handoff notes now point at the current packaged Mac app, not an older
  archive.

## Gameplay Changes

- `ACodeZombieActor` now refreshes character-movement settings at spawn and
  variant initialization, orients to movement, and faces the current movement
  target.
- Zombie mesh presentation now applies a forward-facing yaw correction so the
  visible body no longer appears to run sideways when the capsule is pursuing
  the player.
- `ACodeRescueAIController` now switches from patrol/investigate to chase as
  soon as the player is in activation range, and its direct movement fallback
  rotates the pawn toward the player before applying movement input.
- Boss-spawned adds, horde zombies, dog pups, and regular GameMode spawns all
  refresh the same movement settings after speed/range tuning.
- Companion follow movement also rotates toward the follow vector so friendly
  movement reads as intentional.

## Level Access And World Proportion Changes

- Removed the generated perimeter gate rails and the global outside safety
  ground barrier.
- Replaced tall entry-gate objects with low open-route beacons, lanterns, and a
  green route stripe tagged as `AlwaysOpenLevelEntry` and
  `NoExteriorWallBarrier`.
- Updated player-start documentation to state that the player begins inside the
  playable level, with no enclosing exterior wall or gate.
- Reduced generated architecture scale from oversized skyline proportions to a
  compact, tense street scale suited to the original coding-rescue survival
  horror direction.
- HUD and playtest wording now refer to the active level/objective route instead
  of an active city gate.

## Camera Changes

- Added public `CycleCameraPerspective()` and `GetActiveGameplayCamera()` access
  points.
- Bound `C`, `V`, and `Gamepad_RightShoulder` to camera cycling.
- Added direct polling fallbacks for `C`, `V`, `Gamepad_RightShoulder`, and
  numeric camera selects `5` through `0`.
- Camera changes explicitly activate/deactivate the correct camera component,
  reset the player-controller view target, and clear ignored move/look input.
- Weapon traces, throws, melee, and HUD crosshair traces now use the active
  gameplay camera instead of assuming first-person view.

## Fresh Mac Demo App

Fresh package:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package size: `1.9G`

Package timestamp from `stat`: `May 27 12:01:58 2026`

Cooked package integrity includes:

```text
Contents/MacOS/CodeRescueUnreal
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.pak
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.ucas
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.utoc
Contents/UE/CodeRescueUnreal/Content/Paks/global.ucas
Contents/UE/CodeRescueUnreal/Content/Paks/global.utoc
```

## Validation

Passed:

- `python3 Scripts/verify_may27_gameplay_access_pass.py`
- `python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py`
- `python3 Scripts/verify_fab_import_and_entry_access.py`
- `python3 Scripts/verify_fab_unreal_mcp_porting.py`
- `python3 Scripts/verify_audit_implementation_closure.py`
- `python3 -m py_compile ../MCP_Server_Development/fab_unreal_macos_mcp/server.py Scripts/verify_may27_tactical_arsenal_mcp_runtime.py Scripts/verify_fab_unreal_mcp_porting.py`
- `./Recompile_Module.command`
- `Scripts/verify_curriculum_validator_shapes.py` commandlet
- `Scripts/verify_camera_perspectives_and_character_roster.py` commandlet
- `Scripts/verify_character_world_assets.py` commandlet
- `Scripts/verify_runtime_step_smoke_contracts.py` commandlet
- `Scripts/verify_production_track_completion.py` commandlet
- `./Package_Mac_App.command`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- `git diff --check`

Packaged smoke status:

- Null smoke launched the packaged app with `-NullRHI -nosound`, loaded
  `/Engine/Maps/Entry`, and exited cleanly.
- Render smoke launched the packaged app with Metal/CoreAudio, loaded
  `/Engine/Maps/Entry`, and exited cleanly.
- The log scanner allows the current immediate-quit navigation/crowd diagnostics
  and the unattended macOS CoreAudio sample-rate query warning. No errors,
  fatals, linker warnings, load errors, stale asset references, or missing
  object warnings were reported in the packaged smoke logs.

Resolved QA caveat:

- `./Run_Full_QA_Audit.command` previously stopped at
  `verify_curriculum_validator_shapes.py` because a local validator subprocess
  could hang. The runtime code now enforces an 8-second validation subprocess
  timeout, terminates stalled process trees, and falls back from unresponsive
  local MATLAB batch mode to the in-engine MATLAB-compatible validator for the
  session.
- After that runtime fix, `verify_curriculum_validator_shapes.py` passed as an
  Unreal commandlet, so the earlier full-audit stall is no longer open for the
  current package.
