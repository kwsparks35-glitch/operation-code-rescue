# Settings Color Vision Live Refresh Slice

This slice completes the player-facing color-vision path started in `SETTINGS_COLOR_VISION_SLICE.md`. The selector already saved `Standard`, `Deuteranope`, `Protanope`, and `Tritanope`; this pass makes the selected mode affect active world grading immediately when the player presses Apply.

## Implementation

- Extracted `ACodeRescueGameMode::ConfigurePerZonePostProcessVolume` from the per-zone spawn path so city grade and color-vision correction are reusable.
- Tagged spawned post-process volumes with `CodeRescueZonePostProcess`, `CodeRescueColorVisionRefresh`, `CodeRescueCityIndex_*`, and `CodeRescueGrade_*` metadata.
- Added `ACodeRescueGameMode::RefreshActiveColorVisionPostProcess(EColorblindMode)` to reapply the saved correction to active streamed city volumes.
- Wired `UCodeRescueSettingsWidget::OnApplyClicked` to call the refresh after writing `UCodeRescueGameInstance::ColorblindMode`.
- Updated Settings feedback so the player can see how many active world grade volumes were refreshed.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` so the control records hot-refresh behavior rather than future-only world grading.

## Player Impact

Players can now change color-vision mode from Settings and see the active city palette update immediately. The same mode still persists through the save-game path and applies naturally to future city loads.

## Verification

Added `Scripts/verify_settings_color_vision_live_refresh_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the GameMode API, post-process metadata tags, refactored grading helper, Settings Apply call, player-facing feedback, manifest wording, progress entry, and documentation.

## Remaining QA

Human visual validation should still review Standard, Deuteranope, Protanope, and Tritanope against representative cool, overcast, neon, desert, gulf, and neutral city palettes.
