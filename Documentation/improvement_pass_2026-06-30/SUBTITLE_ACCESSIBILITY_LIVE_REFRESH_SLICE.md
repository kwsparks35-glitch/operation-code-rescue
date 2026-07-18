# Subtitle Accessibility Live Refresh Slice

This slice continues the June 25 accessibility and release-readiness backlog by making active subtitles respond immediately when the player applies subtitle settings. It supports the threat-audio caption work by ensuring those captions respect the saved subtitle toggle, subtitle size, and high-contrast preference without waiting for the HUD to be rebuilt.

## Implementation

- Added `UCodeRescueSubtitlesWidget::RefreshAccessibilityState()` so Settings Apply can refresh the active subtitle overlay.
- Added a stable `BaseSubtitleFont` to avoid compounding font size every time subtitle scale is reapplied.
- Added `ApplyAccessibilityStateFromSettings()` to re-read `bSubtitlesEnabled`, `SubtitleScale`, and `bHighContrastHUD` from `UCodeRescueGameInstance`.
- Reapplies subtitle font size from the saved 0.75x to 1.75x scale while the overlay is active.
- Live refresh clears the subtitle queue and visible subtitle line immediately when subtitles are disabled.
- Calls the refresh path from `UCodeRescueSettingsWidget::OnApplyClicked()` after settings are written and the shared UI theme is updated.
- Updated `Content/CodeRescueData/accessibility_settings_manifest.tsv` with a `SubtitleLiveRefresh` row for future review.

## Player Impact

Players can open Settings, resize subtitles, enable high contrast, or disable subtitles, press Apply, and see the active subtitle overlay update immediately. This makes combat captions, dispatch lines, and accessibility changes feel consistent during live play instead of requiring a menu reload or level restart.

## Verification

Added `Scripts/verify_subtitle_accessibility_live_refresh_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the active refresh API, stable base font, saved setting reads, disabled-subtitle queue clearing, Settings Apply integration, manifest row, progress entry, documentation, and QA wiring.

## Remaining QA

Human playtest should confirm subtitle scale legibility at 0.75x, 1.0x, and 1.75x over bright and dark scenes, plus confirm that disabling subtitles during an active line clears it immediately.
