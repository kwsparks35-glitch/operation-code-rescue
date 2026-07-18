# Reactive Threat Audio Music Slice

This pass implements a package-safe reactive-audio bridge from `TOP_50_RECOMMENDATIONS` recommendation 43, spatial and reactive audio, and the `WORLD_DEVELOPMENT_DEEPDIVE` guidance for readable district threat tone. The game already had a persistent audio mix, runtime music component, hostile threat captions, protected learning zones, and direct-pursuit zombies; this slice connects those systems so music pressure rises around nearby active zombies and dampens inside coding-safe space.

## Player-Facing Changes

- Nearby living zombies now drive a smoothed threat intensity value on the player.
- Music volume is scaled by the saved `MusicVolume` setting plus a restrained reactive scalar from 0.82x to 1.22x.
- Threat states are labeled as `calm`, `watch`, `pursuit`, `critical`, or `safehouse muted`.
- Protected learning zones clamp the reactive pressure to near-calm so coding spaces do not feel like active combat rooms.
- State changes push sparse `[Audio]` captions with nearest threat distance and contact count, keeping the audio change text-backed.

## Implementation

The sampler lives in `ACodeRescueCharacter::UpdateReactiveThreatAudio`.

- Added player tuning for enabling reactive threat audio, scan range, critical range, and update interval.
- Added runtime state for the last sample time, caption time, smoothed intensity, and last state label.
- Scans `ACodeZombieActor` instances with `TActorIterator`, ignores invalid or dead zombies, and scores distance-weighted pressure.
- Calls `ACodeRescueGameMode::IsLocationInsideProtectedLearningZone` to damp combat music inside protected learning spaces.
- Calls `UCodeRescueGameInstance::UpdateReactiveThreatMusic` with the smoothed intensity and label.
- Adds QA tags: `ReactiveThreatAudioRuntime`, `ReactiveThreatMusicDirector`, `Top50Recommendation43ReactiveAudio`, and `WorldDevelopmentAudioGuidance`.

The music bridge lives in `UCodeRescueGameInstance`.

- Added `bReactiveThreatMusicEnabled`, `ReactiveThreatMusicIntensity`, and `ReactiveThreatMusicState`.
- Added `GetReactiveThreatMusicScalar`, `UpdateReactiveThreatMusic`, `GetReactiveThreatMusicSummary`, and `RefreshReactiveThreatMusicVolume`.
- Updated `ApplyAudioMixSettings` and `PlayMusic` so reactive music respects saved `MusicVolume` and the existing music component.
- Extended `GetAudioMixSummary` with the current threat-music state for settings and QA review.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/reactive_threat_audio_music_manifest.tsv`.
- Added creative-development, audio-coverage, visual-regression, human-QA, and accessibility entries for reactive threat audio.
- Wired `Scripts/verify_reactive_threat_audio_music_slice_pass.py` into local CI and full QA.
- Logged this slice in `progress.md`.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_reactive_threat_audio_music_slice_pass.py`
- `python3 Scripts/verify_reactive_threat_audio_music_slice_pass.py`
- `python3 Scripts/verify_threat_audio_captions_slice_pass.py`
- `python3 Scripts/verify_settings_audio_accessibility_slice_pass.py`
- `python3 Scripts/verify_city_radio_bark_cadence_slice_pass.py`
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Start a selected-language run, leave the protected coding space, approach one zombie at long range, close to pursuit range, enter critical range, and then step back into the protected safehouse boundary. The music pressure should rise and fall with the hostile distance, captions should name the audio state, and the protected learning space should read as damped without removing the start-screen language selection or language-specific save/resume flow.

## Remaining Art Hooks

This is a procedural fallback. Future authored combat stems, spatial music layers, district ambience, stingers, MetaSound graphs, or Wwise/FMOD events can replace the scalar while keeping `UpdateReactiveThreatMusic` as the audited gameplay bridge.
