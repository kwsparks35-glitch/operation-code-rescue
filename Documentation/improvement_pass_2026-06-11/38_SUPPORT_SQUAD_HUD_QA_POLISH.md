# Support Squad, HUD, and QA Polish Pass

Date: 2026-06-11

## Scope

This was a steady code and QA improvement pass on top of the June 1 packaged
build. It did not rebuild a new distributable Mac package. The current package
path remains:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

## Improvements

- Made rescue-team companions player-safe by keeping their world collision
  active while making their capsules ignore pawn and camera collision. This
  prevents the five-person squad from physically trapping or blocking the
  player.
- Enabled companion RVO avoidance and added a personal-space movement response
  so teammates step away when they drift too close to the player.
- Added a rescue-team HUD status line showing active squad count, medic
  nearby/away state, medic cooldown readiness, and support-fire availability.
- Exposed companion operational state and medic cooldown helpers for HUD and
  future QA hooks.
- Added `Scripts/verify_june01_rescue_survivability_pass.py` to lock in the
  current survivability, attack-direction, rescue-squad, non-blocking
  companion, squad-HUD, access-cleanup, and documentation contracts.
- Wired the new verifier into `Run_Full_QA_Audit.command`.
- Updated the older May 27 safe-learning verifier to expect the current
  `MaxEnemyDamagePerHitFraction = 0.16f` balance instead of the retired
  `0.34f` value.

## Validation

Passed:

```bash
python3 Scripts/verify_june01_rescue_survivability_pass.py
python3 Scripts/verify_may27_safe_learning_city_controls_pass.py
./Recompile_Module.command
./Run_Full_QA_Audit.command
```

The full QA audit rebuilt the editor module, ran static verifiers including the
new June rescue/survivability verifier, ran Unreal commandlets for campaign,
curriculum, production-track, character/world, camera/roster, and runtime-step
contracts, then completed headless runtime smoke and log scanning.

Allowed smoke diagnostics remained limited to:

- Immediate-quit navigation dirty-area warning.
- Immediate-quit crowd-following RecastNavMesh warning.

No new crash, compiler, physics, map-load, access-cleanup, or smoke-scan
regressions were found.

## Manual Playtest Focus

- Walk through doorways, terminal spaces, safehouse routes, survivor spaces,
  and helipad access points with the full squad following.
- Confirm the squad no longer blocks the player when reversing or strafing in
  narrow spaces.
- Confirm the new rescue-team HUD line stays readable and does not overlap the
  health, reload, tactical, or weapon readouts at 1280x720 and native display
  sizes.
- Confirm the medic status line matches play: nearby/away state and cooldown
  behavior should feel understandable during combat.
