# Rescue Team, Survivability, Access Cleanup, and Fresh Package Pass

Date: 2026-06-01

## Package

Fresh Mac package:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

```text
Size: 1.9G
Timestamp: May 31 19:59:46 AKDT / June 1 03:59:46 UTC 2026
```

## Gameplay Updates

- Added a larger player health HUD readout with current health, max health,
  and percentage so survivability is visible during combat.
- Added a directional attack alert near the crosshair. When the player is hit,
  the HUD reports the attack direction, source, damage amount, and approximate
  source distance.
- Increased player resilience: higher base health, larger medkit and armor
  capacity, stronger armor mitigation, a longer repeated-hit mercy window,
  lower repeated-hit damage, and a single-hit lethal guard that prevents a
  healthy player from dying to one enemy contact.
- Added a five-member rescue support squad in every generated city:
  medic, engineer, rifle support, scout, and heavy rescue. Squad members spawn
  near the entry rally point, follow in a staggered formation, assist in combat,
  and the medic can pulse-heal the player when nearby and below half health.
- Kept the existing full arsenal cycling path active: `1`-`0` quick slots,
  mouse wheel, bracket keys, and gamepad shoulder cycling.
- Expanded access cleanup after final city set dressing so the entry, armory,
  safehouse, language plaza, terminal, survivor area, and helipad are cleared
  of blocking static collision.
- Updated the access cleanup to freeze simulated physics components before
  disabling blocker collision, preventing Chaos physics configuration warnings.

## Regressions Found and Fixed

- Full QA initially failed because generated language-track text did not
  include the required `cross-training` token. The campaign language-track
  generator now includes that wording for the Next 100 verifier.
- The first expanded access-cleanup pass created blocking `LogPhysics`
  warnings by disabling collision on simulated Chaos cover props before
  disabling simulation. The cleanup now disables simulation first, then removes
  blocker collision.

## Validation

Passed:

```bash
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Relevant logs:

```text
Saved/Logs/HeadlessFullQASmoke.log
Saved/Logs/PackagedSmoke_null.log
Saved/Logs/PackagedSmoke_render.log
```

The final full QA run passed static verifiers, the Next 100 implementation
verifier, curriculum validator shape checks for Java/C/Python/MATLAB/C+/C++,
production-track commandlets, runtime-step smoke commandlets, headless runtime
smoke, package build/cook/stage/archive, packaged null smoke, and packaged
render smoke.

The final smoke scanner allowed only the known unattended macOS CoreAudio
sample-rate query warning in render mode, the known immediate-quit navigation
dirty-area diagnostic, and the known immediate-quit crowd-following
RecastNavMesh diagnostic. No blocking physics, crash, map-load, packaging, or
gameplay marker failures remained.

## Follow-Up Playtest Focus

- Run a human visual playtest before external sharing.
- Verify the rescue squad formation does not crowd the player in narrow
  interiors.
- Confirm the new health and attack-alert HUD text remains readable on the
  smallest supported Mac display resolution.
- Continue balance tuning after human playtest data, especially medic heal
  cadence and early-city enemy density.
