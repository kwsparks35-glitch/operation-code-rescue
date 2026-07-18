# Headshot Feedback Reduced Motion Slice

## Purpose

The June 25 UX guidance specifically calls out the HUD headshot pop as part of the remaining HUD-theme rollout: precision feedback should feel satisfying, but it should not force motion on players who enable Reduced Motion. This slice gives the headshot feedback a clear accessibility branch while keeping the standard-mode pop/fade reward.

## Implementation

- Added `HeadshotFeedbackSlot` to `UCodeRescueHUDWidget` so the HUD can move the precision feedback in standard motion mode and keep it fixed in reduced-motion mode.
- Added `RefreshHeadshotFeedback()` to keep headshot timing, text, color, font size, and slot motion out of the main `RefreshHUD()` body.
- Added `HeadshotStandardDurationSeconds`, `HeadshotReducedMotionDurationSeconds`, and `HeadshotBaseY` constants for reviewable timing and layout.
- Standard motion now shows `HEADSHOT` with a short size pop, upward movement, and fade.
- Reduced Motion now shows `PRECISION HIT` for a longer fixed duration, with no slot movement and no fade-dependent readability.
- High Contrast HUD now uses bright amber precision-hit text so the reward cue remains readable in the same visual language as damage alerts and threat HUD cues.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` and `Content/CodeRescueData/enemy_readability_manifest.tsv` with the new precision-hit coverage.

## Player Impact

Players still get a clear reward cue for precise shots, but Reduced Motion replaces the moving popup with a stable readout. High Contrast HUD makes that readout bright and readable without relying only on the standard warm-orange color.

## Verification

- Added `Scripts/verify_headshot_feedback_reduced_motion_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command`.
- Wired the verifier into `Run_Local_CI_Readiness.command`.
- C++ compile, package, and packaged smoke validation should be run because this changes HUD widget code.

## Human QA Notes

- In standard motion, land a headshot and confirm `HEADSHOT` briefly grows/moves/fades.
- Enable Reduced Motion, land a headshot, and confirm `PRECISION HIT` stays fixed long enough to read.
- Enable High Contrast HUD and confirm the precision-hit text switches to bright amber.
