# Settings Audio Accessibility Slice

This slice continues the June 25 creative-development backlog by making the settings menu more reviewable, persistent, and useful during active play. It supports the `TOP_50_RECOMMENDATIONS` accessibility/readability guidance, the `OPERATION_CODE_RESCUE_RELEASE_DOSSIER` first-session readiness goals, and the existing `accessibility_settings_manifest.tsv` and `audio_coverage_manifest.tsv` tracking data.

## Implementation

- Added persistent `MasterVolume`, `SfxVolume`, and `MusicVolume` fields to `UCodeRescueSaveGame`.
- Added matching runtime fields and helpers to `UCodeRescueGameInstance`: `GetAudioMixSummary`, `GetSfxVolumeScalar`, `GetMusicVolumeScalar`, and `ApplyAudioMixSettings`.
- Restored saved audio values when loading a run and saved them with the rest of the language/run profile.
- Routed runtime music through the saved music scalar, including menu, city, and boss music playback.
- Routed core SFX cue playback through the saved SFX scalar, including player weapon cues, zombie attack/death/growl cues, survivor voice cues, cooked radio briefings, and city ambient beds.
- Expanded `UCodeRescueSettingsWidget` with live audio, gameplay, and accessibility readouts so slider/toggle changes are visible before Apply.
- Added `Reset Accessibility Defaults`, which queues safe subtitle, contrast, motion, hints, and aim-assist defaults without silently changing the saved run until Apply.

## Player Impact

Players can now return to the game and see their saved audio mix reflected in the settings menu instead of every audio slider reopening at full volume. The settings screen also explains the queued state of the major playability settings in one place: volume, FOV, sensitivity, aim assist, subtitles, high contrast, reduced motion, and simplified input hints.

## Verification

Added `Scripts/verify_settings_audio_accessibility_slice_pass.py`, wired into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks persistent save fields, game-instance helpers, settings readouts, reset behavior, SFX/music routing, manifest coverage, progress logging, and QA wiring.

## Remaining Work

This is a runtime and UI integration pass, not final authored audio mixing. Future audio work should still create dedicated sound classes or sound mixes for weapon, creature, voice, music, and ambience buses, then retarget these same saved values to those authored assets.
