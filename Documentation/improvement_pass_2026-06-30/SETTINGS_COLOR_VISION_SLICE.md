# Settings Color Vision Slice

This slice closes a release-readiness gap in the existing accessibility work. `TOP_50_RECOMMENDATIONS` item 12 calls for colorblind validation, and the project already had `EColorblindMode`, save-game persistence, and per-zone post-process correction, but the player could not select the mode from Settings.

## Implementation

- Added a `Color Vision Mode` settings row to `UCodeRescueSettingsWidget`.
- The row cycles through `Standard`, `Deuteranope`, `Protanope`, and `Tritanope`.
- The queued mode appears in the Accessibility readout next to high-contrast state.
- `Reset Accessibility Defaults` now also queues `Standard` color vision.
- `Apply` writes the selected mode into `UCodeRescueGameInstance::ColorblindMode`, which is already saved through `UCodeRescueSaveGame`.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` so the row records a real settings control instead of a hidden runtime field.

## Player Impact

Players can now choose a color-vision mode from the same settings surface as subtitles, high contrast, reduced motion, simplified hints, and aim assist. The selected mode is saved with the run and is available to the existing per-zone post-process grading path when the world creates or reloads graded zones.

## Verification

Added `Scripts/verify_settings_color_vision_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the settings button, mode cycle, readout, reset behavior, Apply/save behavior, manifest entry, documentation, and QA wiring.

## Follow-On Slice

`SETTINGS_COLOR_VISION_LIVE_REFRESH_SLICE.md` closes the implementation gap by hot-refreshing already-spawned post-process volumes from Settings Apply. Human visual validation across all three correction modes on key city palettes remains a release QA task.
