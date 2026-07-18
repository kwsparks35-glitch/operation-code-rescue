# Mono Audio Accessibility Slice

Date: 2026-06-30

## Guidance Source

- `TOP_50_RECOMMENDATIONS` item 44 calls for real audio mix controls plus mono and visualized-sound accessibility options.
- The visualized-sound slice now covers the readable HUD side of that recommendation. This slice completes the package-safe mono side for project-owned runtime cues.

## Implementation

- Added a saved `bMonoAudio` accessibility setting to `UCodeRescueSaveGame` and `UCodeRescueGameInstance`.
- Added `UCodeRescueGameInstance::GetMonoAudioSummary()` and extended audio/accessibility summaries so Settings and QA can see whether mono mode is active.
- Extended Settings with a `Mono Audio` checkbox, cached preview readouts, Apply persistence, and reset-default behavior.
- Mono Audio forces `bVisualizeSoundCues` on. This keeps threat, ambient, and caption state visible whenever directional audio is flattened.
- Added `GetMonoSafeSoundLocation` routing in player weapon cues, zombie attack/death cues, and survivor bark/rescue VO so project-owned `PlaySoundAtLocation` calls center at the player while mono mode is enabled.
- Added `ACodeZombieActor::ApplyMonoAudioAccessibility()` so the persistent growl component disables spatialization in mono mode.
- Added `ACodeRescueGameMode::RefreshMonoAudioSpatialization()` so Settings Apply immediately refreshes active ambient sounds and zombie growl components without requiring a restart.
- Updated city ambient spawning so newly spawned ambient beds honor the current mono state.

## Accessibility Contract

- Stereo remains the default.
- Mono Audio is an opt-in saved accessibility mode for the selected-language run.
- In mono mode, project-owned positional gameplay cues are centered rather than relying on left/right placement.
- Active runtime ambience and zombie growls disable spatialization when Settings are applied.
- Visual sound cues stay enabled with mono mode so directional and state awareness is never audio-only.

## Scope Note

This project does not currently ship authored Sound Class/Submix assets for a platform-level channel downmix. The implemented mono mode is therefore the reliable runtime layer inside the game code we own: it centers project-owned positional cues, refreshes active spatial components, and pairs mono listening with visual sound indicators. If final Sound Class/Submix assets are authored later, they can replace or augment this contract without changing the saved `bMonoAudio` setting or Settings UI.

## Review Artifacts

- `Content/CodeRescueData/mono_audio_accessibility_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/audio_coverage_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Scripts/verify_mono_audio_accessibility_slice_pass.py`

## Validation Plan

- Compile and run the mono audio verifier.
- Re-run adjacent visualized sound cues, settings/audio, ambient-zone, and reactive-threat audio verifiers.
- Recompile the module, package the Mac app, and run null/render packaged smokes.
- Perform a manual Settings pass by enabling Mono Audio, confirming Visualize Sound Cues stays on, and checking weapon, zombie, survivor, growl, and ambient cue behavior in a selected-language run.

## Validation Performed

- `python3 -m py_compile Scripts/verify_mono_audio_accessibility_slice_pass.py`
- `python3 Scripts/verify_mono_audio_accessibility_slice_pass.py`
- `python3 Scripts/verify_visualized_sound_cues_accessibility_slice_pass.py`
- `python3 Scripts/verify_settings_audio_accessibility_slice_pass.py`
- `python3 Scripts/verify_city_ambient_zone_audio_slice_pass.py`
- `python3 Scripts/verify_reactive_threat_audio_music_slice_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

Packaged null smoke passed with allowed navigation dirty-area and crowd-manager warnings. Packaged render smoke passed with allowed CoreAudio sample-rate, navigation dirty-area, and crowd-manager warnings.
