# UI Text Scale Settings Slice

This pass continues the June 25 accessibility/readability guidance by splitting general interface text scaling away from subtitle scaling. Before this slice, `SubtitleScale` also drove `CodeRescueUI::Theme().TextScale`, so players could not enlarge HUD/menu/terminal/journal text without also changing caption size.

## Runtime Coverage

- Added persisted `UITextScale` state to `UCodeRescueGameInstance` and `UCodeRescueSaveGame`.
- Added `UCodeRescueGameInstance::GetUITextScale()` and `GetUITextScaleSummary()` so every UI surface reads the same clamped 0.80x to 1.75x value.
- Added a `UI Text Size` slider to `UCodeRescueSettingsWidget`, including queued readouts, Apply persistence, reset-to-default handling, and Apply feedback.
- Updated HUD, pause, tutorial, terminal, objective journal, minimap, skill tree, save slots, fast travel, damage feedback, victory, and death screens to set `CodeRescueUI::Theme().TextScale` from `GetUITextScale()`.
- Kept `UCodeRescueSubtitlesWidget` tied to `SubtitleScale`, preserving independent caption sizing and live subtitle refresh behavior.

## Documentation And Data

- Added `Content/CodeRescueData/ui_text_scale_settings_manifest.tsv`.
- Updated expanded accessibility, accessibility settings, creative inclusion, visual regression, human QA, full-QA, local-CI, and progress-log surfaces.
- Updated adjacent verifier expectations that previously treated subtitle scale as the shared UI theme scale.

## Validation

- `python3 Scripts/verify_ui_text_scale_settings_slice_pass.py`
- Adjacent verifiers: expanded accessibility options, settings audio/accessibility, subtitle live refresh, HUD vitals, damage feedback, onboarding input glyph, pause difficulty matrix, minimap route readability, objective journal accessibility, fast travel readability, skill tree clarity, save-slot language backup, end-state continuity, and save compatibility.
- Manual QA should deliberately mismatch Subtitle Size and UI Text Size, then verify captions and themed UI surfaces respond independently before and after selected-language save/resume.
