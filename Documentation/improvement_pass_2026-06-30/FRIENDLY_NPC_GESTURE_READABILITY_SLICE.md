# Friendly NPC Gesture Readability Slice

This slice continues the June 25 `CHARACTER_ANIMATION_DEEPDIVE` guidance for the safehouse support cast. The existing friendly NPC service slice already made Engineer, Medic, Scientist, and Trader interactions save-backed inside the selected-language profile; this pass adds visible runtime reactions so the support hub feels staffed rather than decorative.

## Implementation

- Enabled lightweight ticking on `AFriendlyNPCActor` for visual-only service gestures.
- Added `bEnableServiceGestureReadability`, idle scale, successful-service duration, and denied-service duration tuning.
- Added base-pose caching for the professional skeletal NPC body, primitive fallback head, role badge, role prop, role icons, and role light.
- Added `UpdateServiceGesture()` for ambient idle motion, cooldown dimming, success acknowledgment pulses, and denial head-shake/prop-hold motion.
- Added `TriggerServiceGrantGesture()` after a role benefit is granted and saved to the selected-language profile.
- Added `TriggerServiceDeniedGesture()` for cooldown, full-health Medic, and insufficient-scrap Trader interactions.

## Player Result

The Civilian Support Hub now gives immediate visual feedback. Support NPCs subtly idle while waiting, pulse their badge/prop/light when a service succeeds, and visibly refuse when the player has already used a daily service or does not meet a role precondition. The layer does not move the actor root or the primitive collision body, so prompts, blocking, save persistence, and day-night cooldown reset behavior remain unchanged.

## Validation

Added `Content/CodeRescueData/friendly_npc_gesture_readability_manifest.tsv` and `Scripts/verify_friendly_npc_gesture_readability_slice_pass.py`, then wired the verifier into local CI and full QA. Updated the creative inclusion plan, human QA checklist, visual regression targets, and progress log so the safehouse support animation-readability layer remains reviewable.

Manual QA should enter a selected-language run, inspect all four support roles, use an available Engineer or Scientist service, try a used service again, try Medic while healthy, and try Trader with insufficient scrap. Confirm idle motion, success pulse, refusal pose, unchanged HUD service prompts, selected-language cooldown persistence, and unchanged start-screen language resume behavior.
