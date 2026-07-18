# HUD Vitals Theme Accessibility Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS_2026-06-25.md`, recommendation 4: continue the HUD theme rollout, drive health bar color by health fraction, and keep reduced-motion headshot handling.
- `UX_OVERHAUL_GUIDE.md`: move HUD vitals toward shared color/type tokens, including themed health and stamina fills.

## Implementation

- Added reviewable HUD helper functions:
  - `HudVitalStateLabel()`
  - `HudHealthFillColor()`
  - `HudHealthLabelColor()`
  - `HudStaminaFillColor()`
- Replaced local health and stamina color literals in the refresh path with those helpers.
- Health now reports `STABLE`, `LOW`, or `CRITICAL` next to the numeric health readout.
- Health label styling now uses `CodeRescueUI::StyleText()` so saved text scale affects the live HUD.
- `RefreshHUD()` mirrors high contrast, reduced motion, and subtitle/text scale back into the shared theme every frame.
- Stamina now uses the shared stamina token for healthy state, with amber/ember depletion states and high-contrast variants.

## Player Impact

- Health and stamina states are easier to read under pressure because color and text now communicate the same state.
- High Contrast HUD has a concrete effect on core combat vitals, not just status text.
- Larger text settings scale the health label through the same design system used by the other updated UI surfaces.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/safe_learning_city_controls_manifest.tsv`
- `Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_hud_vitals_theme_accessibility_slice_pass.py`
- Adjacent HUD verifiers: headshot reduced motion, damage feedback accessibility, threat compass HUD, demo readiness.
- Compile/package/smoke should be run because this changes HUD widget code.

## Human QA Notes

- Start a language run, take controlled damage, and confirm health changes from `STABLE` to `LOW` to `CRITICAL`.
- Sprint until stamina depletes and confirm the stamina fill moves through blue, amber, and ember states.
- Enable High Contrast HUD and confirm both bars and the health label become visibly brighter.
- Increase Subtitle/Text size and confirm the health label remains readable and does not overlap the health bar.
