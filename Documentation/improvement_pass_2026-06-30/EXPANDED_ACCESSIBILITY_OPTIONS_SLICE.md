# Expanded Accessibility Options Slice

This pass closes the P2 expanded accessibility options row from the June 25 improvement guidance as an auditable runtime contract. The controls already existed across several focused accessibility slices; this slice ties them together so future review can confirm the settings surface, persistence fields, live refresh hooks, and QA coverage as one complete player-facing accessibility options set.

## Runtime Coverage

- `UCodeRescueSettingsWidget` exposes subtitles, subtitle size, UI text size, high contrast, color vision, reduced motion, simplified input hints, aim assist, accessibility reset, audio/gameplay/accessibility readouts, and control-profile export.
- `UCodeRescueGameInstance` and `UCodeRescueSaveGame` persist subtitles, color vision, subtitle scale, UI text scale, high contrast, reduced motion, simplified hints, aim assist, audio mix, and control-profile export metadata.
- `OnApplyClicked()` pushes saved accessibility state into `CodeRescueUI::Theme()`, refreshes active subtitles and damage feedback, refreshes active world color grading, persists the selected-language run, and updates readout text.
- `Content/CodeRescueData/expanded_accessibility_options_manifest.tsv` lists every option, where the player finds it, what field persists it, what runtime effect it has, and which verifier owns it.

## Source Guidance

- `TOP_50_RECOMMENDATIONS_2026-06-25.md` calls for accessibility settings to drive the UI, including high contrast, reduced motion, text scaling, HUD health readability, and later mix/accessibility work.
- `UX_OVERHAUL_GUIDE.md` says accessibility is built into the shared theme and should mirror saved settings for high contrast, reduced motion, and text scale.

## Boundaries

This slice predates the dedicated mono mix and visualized-sound passes. The current game now has persisted per-volume audio mix, text-first captions, saved visualized sound cues, and package-safe mono audio behavior for project-owned runtime cues; authored Sound Class/Submix assets can still extend the same saved settings later.

## Validation

- `python3 Scripts/verify_expanded_accessibility_options_slice_pass.py`
- Adjacent focused verifiers: settings audio, color vision live refresh, subtitle live refresh, damage feedback accessibility, HUD vitals accessibility, minimap route readability, objective journal accessibility, onboarding input glyph, and control remap profile export.
- Manual QA remains required for sensory comfort at the extremes of subtitle scale, UI text scale, color vision, reduced motion, and aim assist.
