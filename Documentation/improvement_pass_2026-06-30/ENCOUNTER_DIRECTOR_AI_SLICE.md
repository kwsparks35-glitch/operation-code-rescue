# Encounter Director AI Slice

Date: 2026-06-30

## Source Guidance

This slice continues the June 25 creative-development implementation using these documents as the active guideline set:

- `WORLD_DEVELOPMENT_DEEPDIVE` for authored encounter spaces, readable route pressure, and designed gameplay pockets rather than purely decorative city fill.
- `CHARACTER_ANIMATION_DEEPDIVE` for readable enemy intention, clear hit/death continuity, and variant-aware zombie presentation.
- `GAME_PHYSICS_DEEPDIVE` for destructible cover, physical encounter props, and combat spaces that expose physics choices during play.
- `TOP_50_RECOMMENDATIONS` and `OPERATION_CODE_RESCUE_RELEASE_DOSSIER` for turning the educational rescue loop into repeatable, shippable, reviewable gameplay beats.

## Implementation Summary

The new encounter director pass adds explicit zombie roles to `ACodeZombieActor`:

- `Anchor`: holds an authored point until the player enters the pressure pocket.
- `Flanker`: targets an offset lane beside and behind the player instead of stacking in a straight chase line.
- `Pressure`: advances directly to keep the encounter legible and urgent.
- `Sentinel`: guards the survivor-side handoff until the player crosses into the rescue zone.

In verifier language, the implemented roles are anchor, flanker, pressure, and sentinel.

`ACodeRescueAIController` now asks each zombie for a directed movement target during patrol, chase, and attack states. Zombies without a directive continue using the old player-chase behavior, so existing city, horde, boss, and language-breach encounters remain compatible.

## World Integration

`ACodeRescueGameMode::SpawnEncounterDirectorLayer` now runs for each unrescued survivor city. It places:

- a readable `DIRECTED ENCOUNTER` staging deck near the survivor route,
- role-color lane markers and beacons,
- three health-based destructible barricades tagged as encounter director cover,
- a smoke cache, armor cache, and ammo cache for counterplay,
- four save-aware directed zombies using stable neutralization IDs under `CodeRescueHordeZombieIdBase + CityIndex * 1000 + 800 + i`.

The encounter is skipped in sandbox mode, and neutralized directed zombies stay neutralized on reload through the same `NeutralizedZombieIds`, `RecordZombieVariant`, and `SavePersistentRun` contracts used elsewhere.

## Review Hooks

Runtime/source review tags added by this slice include:

- `EncounterDirectorLayer`
- `AIDirectedEncounter`
- `AuthoredEncounterDirector`
- `EncounterDirectedZombie`
- `EncounterRole_Anchor`
- `EncounterRole_Flanker`
- `EncounterRole_Pressure`
- `EncounterRole_Sentinel`
- `EncounterDirectorZombie`
- `EncounterDirectorCover`
- `EncounterDirectorReward`

These tags are intentionally explicit so future QA passes, editor captures, or asset audits can find the authored director content without relying on actor names alone.

## Boundaries

This slice does not replace the full future Animation Blueprint, Control Rig, IK, or fully authored crowd-behavior work described in `CHARACTER_ANIMATION_DEEPDIVE`. It creates a playable, source-verifiable director layer that works with the current C++ zombie controller and the variant asset table while leaving room for later animation-asset promotion.

## Verification

Added `Scripts/verify_encounter_director_ai_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the role enum and tuning hooks, AI-controller movement integration, world-spawned director layer, sandbox/save behavior, variant initialization, documentation, and progress-log coverage.
