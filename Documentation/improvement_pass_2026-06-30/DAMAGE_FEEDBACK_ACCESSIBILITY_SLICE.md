# Damage Feedback Accessibility Slice

## Purpose

The June 25 playability and accessibility guidance repeatedly calls for combat feedback that is readable without relying on a single sense, color channel, or fast motion cue. The existing damage system already recorded attacker direction, damage amount, source type, and distance, but the fullscreen hit overlay still used standard red-only flashes and did not refresh immediately when the player changed accessibility settings.

## Implementation

- Added `UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState()` and an active-instance pointer so Settings Apply can refresh the live damage overlay immediately.
- Added `ApplyAccessibilityStateFromSettings()` to mirror `bHighContrastHUD`, `bReducedMotion`, and subtitle text scale from `UCodeRescueGameInstance` into the shared UI theme while the overlay is active.
- Added high-contrast amber vignette and chevron colors through `GetVignetteColor()` and `GetChevronColor()` so hit direction remains readable for players who cannot reliably parse red-only danger flashes.
- Added reduced-motion hit flashes through `ReducedMotionDirectionalFlashDuration` and `GetDirectionalFlashDuration()`: reduced-motion mode holds a steady directional cue instead of fading through a fast flash.
- Enlarged directional chevrons in high-contrast mode through `ResizeDirectionalChevron()` while preserving the original compact sizes in standard mode.
- Updated `UCodeRescueSettingsWidget::OnApplyClicked()` to call both subtitle and damage-feedback refresh hooks after saving accessibility state.
- Updated the HUD damage alert to use a high-contrast amber text color when `bHighContrastHUD` is enabled.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` and `Content/CodeRescueData/enemy_readability_manifest.tsv` with the new damage feedback accessibility coverage.

## Player Impact

Damage feedback now has three synchronized channels: directional chevrons, a written HUD warning with direction/source/distance, and subtitle-backed critical health callouts. High-contrast mode turns the hit overlay and HUD warning into bright amber indicators, while reduced-motion mode keeps hit direction visible without relying on a fast fade or critical-health pulse.

## Verification

- Added `Scripts/verify_damage_feedback_accessibility_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command`.
- Wired the verifier into `Run_Local_CI_Readiness.command`.
- Compile, package, and packaged smoke validation should be run after this slice because it changes C++ widget code.

## Human QA Notes

- Enable High Contrast HUD, trigger zombie damage from front/right/rear/left, and confirm the chevron is larger and amber.
- Enable Reduced Motion, take repeated hits, and confirm the hit direction holds steady rather than rapidly fading.
- Toggle settings during a live run, press Apply, and confirm the active overlay changes immediately without needing to restart or wait for a new widget.
