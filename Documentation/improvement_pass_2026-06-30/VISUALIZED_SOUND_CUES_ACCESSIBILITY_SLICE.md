# Visualized Sound Cues Accessibility Slice

Date: 2026-06-30

## Guidance Source

- `TOP_50_RECOMMENDATIONS` item 44 calls for audio mix accessibility work, including a "visualize sounds" option.
- The existing reactive-threat music and city ambient-zone slices already expose package-safe runtime state for threat pressure and zone ambience. This slice makes those states visible in the HUD so critical awareness is not audio-only.

## Implementation

- Added a saved `bVisualizeSoundCues` accessibility setting to `UCodeRescueSaveGame` and `UCodeRescueGameInstance`.
- Added `UCodeRescueGameInstance::GetVisualizedSoundCueSummary()` so the active sound-cue state is Blueprint-callable and reviewable next to the audio mix summary.
- Extended the Settings menu with a `Visualize Sound Cues` checkbox, cached preview state, reset-default behavior, and Apply persistence.
- Added `SoundCueText` to `UCodeRescueHUDWidget`, mounted below the threat compass as a compact runtime line:
  - threat music state and intensity,
  - city ambient zone label and intensity,
  - current caption/subtitle state.
- The HUD branch clears the readout when the setting is disabled and uses high-contrast colors when `bHighContrastHUD` is enabled.

## Accessibility Contract

- Threat music remains helpful, but threat pressure is mirrored as text through the HUD.
- Ambient-zone audio remains helpful, but the current zone tone is mirrored as text through the HUD.
- Subtitle state is visible in the same line so players can confirm whether caption fallback is active.
- The feature persists through the same selected-language save/load path as the rest of the accessibility settings, preserving the start-screen language selection and resume behavior.

## Review Artifacts

- `Content/CodeRescueData/visualized_sound_cues_accessibility_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/audio_coverage_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Scripts/verify_visualized_sound_cues_accessibility_slice_pass.py`

## Validation Plan

- Compile and run the visualized sound cues verifier.
- Re-run adjacent settings/audio/ambient/reactive verifiers.
- Recompile the module, package the Mac app, and run null/render packaged smokes.
- Perform a manual HUD pass by toggling Visualize Sound Cues, entering a calm street, approaching a zombie, entering a protected safehouse, and walking to a survivor/extraction route.
