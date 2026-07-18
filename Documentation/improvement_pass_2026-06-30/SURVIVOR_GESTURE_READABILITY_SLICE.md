# Survivor Gesture Readability Slice

This slice continues the June 25 `CHARACTER_ANIMATION_DEEPDIVE` guidance by giving rescued characters visible emotional motion in the live game. The survivor archetype roster already made each survivor specific through role, need, rescue value, dossier text, and selected-language save state; this pass adds a runtime-safe gesture layer so locked and rescued states are not only text.

## Implementation

- Enabled lightweight ticking on `ASurvivorActor` for visual-only gesture updates.
- Added `bEnableSurvivorGestureReadability`, idle scale, rescue gesture duration, and locked gesture duration tuning.
- Added base-pose caching for the professional skeletal survivor body, primitive fallback head, and rescue light.
- Added `UpdateSurvivorGesture()` to create idle life pose, locked-route refusal motion, rescue confirmation lift, and rescue-light pulse.
- Added `TriggerLockedGesture()` when a survivor refuses rescue because the selected-language terminal is unsolved.
- Added `TriggerRescueGesture()` plus `ScheduleRescueFadeOut()` so successful rescue stays nonblocking but no longer hides the survivor instantly.

## Player Result

Before rescue, a survivor has subtle life motion. If the player tries to rescue too early, the survivor visibly reacts while the subtitle names the missing coding objective. When the player succeeds, the survivor gives a short confirmation pose and light pulse before the existing extraction presentation, save update, journal update, and companion handoff continue.

## Validation

Added `Content/CodeRescueData/survivor_gesture_readability_manifest.tsv` and `Scripts/verify_survivor_gesture_readability_slice_pass.py`, then wired the verifier into local CI and full QA. Updated the creative inclusion plan, survivor archetype manifest, human QA checklist, visual regression targets, and progress log so this survivor animation-readability layer remains reviewable.

Manual QA should approach a locked survivor, attempt rescue before solving the terminal, solve the selected-language terminal, then rescue the survivor. Confirm idle motion, locked refusal motion, rescue gesture/light pulse, immediate collision disable, delayed fade-out, save persistence, and unchanged start-screen language resume behavior.
