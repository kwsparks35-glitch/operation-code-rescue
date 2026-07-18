# Adaptive Encounter Director Pressure Slice

Date: 2026-06-30

## Source Guidance

This slice continues the June 25 implementation pass from `WORLD_DEVELOPMENT_DEEPDIVE`, `GAME_PHYSICS_DEEPDIVE`, `CHARACTER_ANIMATION_DEEPDIVE`, `TOP_50_RECOMMENDATIONS`, and `OPERATION_CODE_RESCUE_RELEASE_DOSSIER`. The goal is to make the encounter director feel authored, readable, and reviewable while still using the current C++ fallback systems before final Behavior Tree, StateTree, EQS, Control Rig, and authored asset promotion.

## What Changed

`ACodeRescueGameMode::SpawnEncounterDirectorLayer` now reads the active save/objective state before placing the survivor-route encounter. It checks whether the mission terminal is already solved, whether the resumed player state is low on health, ammo, medkits, or armor, and which difficulty is active.

The director now adds:

- `EncounterDirectorAdaptivePressure` and `ObjectiveStateAwareEncounter` tags to the deck, markers, cover, pickups, and directed zombies.
- Route-state tags: `EncounterDirectorRouteLockedPressure` before the terminal is solved and `EncounterDirectorRouteOpenPressure` after the route is open.
- Resource-state tags: `EncounterDirectorResourceRelief` for low-resource resumes and `EncounterDirectorStandardResources` otherwise.
- A visible `DIRECTOR STATE` board naming objective state, resource state, and difficulty.
- Relief pickups tagged `EncounterDirectorReliefMedkit`, `EncounterDirectorReliefAmmo`, and optional `EncounterDirectorReliefStim` when the player reloads into the encounter with low resources.
- `AdaptiveEncounterScale`, `AdaptiveSpeedScale`, and `AdaptiveActivationScale` tuning for directed zombie health, damage, speed, and activation range.

## Player-Facing Behavior

Before the terminal is solved, the survivor-route director reads as a lighter route-locked warning pocket. After the terminal is solved, the same encounter reads as a stronger route-open pressure pocket. If a player resumes with low health, low ammo, or nearly no recovery resources, the director adds a small relief cache without removing the role markers or combat loop.

## Review Hooks

Reviewers can find this work through:

- `Content/CodeRescueData/encounter_director_adaptive_pressure_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/verify_adaptive_encounter_director_pressure_slice_pass.py`

Runtime logs now include `[CodeRescueEncounterDirector]` with `objective`, `adaptive_pressure`, `relief`, and `difficulty` values.

## Boundaries

This slice does not replace the current C++ fallback with Behavior Tree, StateTree, EQS, or fully authored animation assets. It creates a stable pressure-budget contract and visible playtest surface so those future assets have clear gameplay behavior to preserve.

## Validation

Run:

```zsh
python3 Scripts/verify_adaptive_encounter_director_pressure_slice_pass.py
python3 Scripts/verify_encounter_director_ai_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```
