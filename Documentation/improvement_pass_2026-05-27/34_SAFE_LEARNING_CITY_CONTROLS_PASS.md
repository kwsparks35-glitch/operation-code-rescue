# May 27 Safe Learning, City, Controls, and Health Pass

## Purpose

This pass addresses the current playability complaint directly: coding
challenges should be learning moments, not unavoidable death traps. The rescue
loop is now split into protected coding, intel reward, and then city rescue.

## Gameplay changes

- Main campaign terminals now spawn inside a protected coding safehouse tagged
  `ProtectedCodingChallengeZone`, `NoZombieLearningZone`, `SafeTerminalLab`,
  `SelectedLanguageOnly`, and `LearningWithoutDeathRisk`.
- The hidden binary-search bonus terminal was moved from the far combat city to
  a protected safehouse annex.
- Opening a coding terminal pauses gameplay; closing it unpauses gameplay.
- Terminal success now reports an intel reward that directs the player toward
  survivor rescue after the lesson.
- Regular zombie spawns are clamped to the combat/rescue district and away from
  the safe learning district.

## Curriculum changes

- The main menu now lets the player select one deployment language before
  starting a run.
- Terminals use that selected language track instead of mixing all languages
  into every level.
- The framework now includes Java, C, Python, MATLAB, C+, and C++.
- C+ and C++ share a clang++-backed validator path and language-specific
  starter/signature text for the existing coding missions.

## City and presentation changes

- Every generated campaign city gets an additional urban identity layer with
  street-grid composition, sidewalks, lane paint, storefront windows, district
  signs, civic signs, and local street/city labels.
- Mission route text now points first to protected coding, then survivor rescue.
- The generated zombie district remains part of the rescue path but no longer
  occupies the coding area.

## Controls, gear, health, and death flow

- Camera perspective controls now use `C`/`V`/gamepad shoulder for cycling and
  `F1` through `F6` for direct perspective selection.
- Weapon quick slots now use `1` through `0`, while wheel/bracket cycling still
  reaches the wider arsenal.
- The HUD now shows a player health gauge.
- Enemy damage per hit is capped, and a healthy player is protected against a
  single zombie hit instantly ending the run.
- The death screen now offers play again from last save, play again fresh,
  save and quit, and quit to desktop.

## Review files

- `Content/CodeRescueData/safe_learning_city_controls_manifest.tsv`
- `Scripts/verify_may27_safe_learning_city_controls_pass.py`

## Mac demo app rebuild

Fresh package for review:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

- Size: `1.9G`
- Timestamp from `stat`: `May 27 13:57:28 2026`
- Null smoke log: `Saved/Logs/PackagedSmoke_null.log`
- Render smoke log: `Saved/Logs/PackagedSmoke_render.log`

## Validation

Passed:

```bash
python3 Scripts/verify_may27_safe_learning_city_controls_pass.py
python3 Scripts/verify_may27_gameplay_access_pass.py
python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py
python3 Scripts/verify_may27_public_demo_fab_detail_pass.py
python3 Scripts/verify_may27_unreal_systems_character_world_pass.py
./Recompile_Module.command
Scripts/verify_runtime_step_smoke_contracts.py
Scripts/verify_camera_perspectives_and_character_roster.py
Scripts/verify_character_world_assets.py
Scripts/verify_curriculum_validator_shapes.py
Scripts/verify_production_track_completion.py
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
git diff --check
```

Packaged smoke notes:

- Null and render smoke both loaded `/Engine/Maps/Entry`, spawned the New York
  demo city, and exited cleanly.
- Both packaged logs confirmed `[CodeRescueSafeLearning]`,
  `[CodeRescueUnrealSystems]`, `[CodeRescuePublicDemoQuality]`, and
  `[CodeRescueEntryAccess]`.
- The rendered pass allowed only the known unattended macOS CoreAudio
  sample-rate query warning and the known immediate-quit navigation/crowd
  diagnostics.
