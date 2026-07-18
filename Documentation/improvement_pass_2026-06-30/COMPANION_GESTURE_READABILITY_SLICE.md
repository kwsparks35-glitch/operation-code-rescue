# Companion Gesture Readability Slice

This slice continues the June 25 `CHARACTER_ANIMATION_DEEPDIVE` and squad-readability work by giving rescue-team companions visual state cues beyond HUD text and subtitles. The previous squad personality pass made Mira, Tomas, Ada, Noor, and Briggs named tactical teammates; this pass makes their idle, formation, order, medic, support-fire, and damage states visible in the game world.

## Implementation

- Added a `CompanionRoleSignalLight` component to `ACompanionActor` and refresh it from each companion's `RoleAccentColor`.
- Added `bEnableCompanionGestureReadability`, idle scale, support-fire, medic-pulse, order, and damage gesture duration tuning.
- Added base-pose caching for the companion skeletal mesh and role signal light.
- Added `UpdateCompanionGesture()` to apply visual-only mesh/light offsets for idle, follow, hold, order acknowledgment, support fire, medic pulse, and damage flinch.
- Added gesture triggers to support-fire hits, automatic and manual medic pulses, regroup, hold, follow, order barks, and companion damage.

## Player Result

Companions now feel like responsive teammates rather than HUD-only helpers. A role-colored signal follows each companion, support fire produces a small recoil/light pulse, medic healing produces a brighter lift, and squad commands produce an acknowledgment pose. The implementation does not move the capsule or rewrite formation logic, so collision, follow spacing, hold/follow orders, support fire, medic cooldowns, and selected-language save behavior stay intact.

## Validation

Added `Content/CodeRescueData/companion_gesture_readability_manifest.tsv` and `Scripts/verify_companion_gesture_readability_slice_pass.py`, then wired the verifier into local CI and full QA. Updated the creative inclusion plan, squad personality manifest, first-ten-minutes onboarding, human QA checklist, visual regression targets, and progress log.

Manual QA should start a selected-language run with the rescue team active, inspect role signal lights, press Y/U/O/N to trigger regroup/formation/hold/follow cues, let a companion fire at a nearby zombie, use N for medic support, and verify no teammate capsule blocks the player or changes the language-run save contract.
