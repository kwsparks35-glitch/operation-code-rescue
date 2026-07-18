# City Ambient Zone Audio Slice

This pass implements a package-safe ambient-zone director from the `WORLD_DEVELOPMENT_DEEPDIVE` guidance for `ZoneAmbientCues` and the `TOP_50_RECOMMENDATIONS` spatial/reactive audio and mix-accessibility recommendations. The game already has city mission metadata, objective-space anchors, protected learning zones, selected-language progression, subtitles, and a saved audio mix; this slice creates a runtime bridge that names the intended ambient bed for the current city space without requiring final audio stems.

## Player-Facing Changes

- The player now has an ambient-zone sampler that recognizes entry approach, protected coding lab, safehouse exterior, overrun street, transit corridor, civic block, survivor route, survivor camp, and extraction pad contexts.
- The active ambient zone is stored on `UCodeRescueGameInstance` for settings, QA, and future audio asset binding.
- Zone changes push sparse `[Ambient]` captions naming the city, zone, intended bed key, and selected language track.
- Survivor and extraction zone tone responds to selected-language progression: locked/search/dormant states shift to route-open/ready once the terminal and rescue state support it.
- The start screen and language-specific save/resume flow remain untouched; this reads existing save state only.

## Implementation

The sampler lives in `ACodeRescueCharacter::UpdateCityAmbientZoneAudio`.

- Added `bEnableCityAmbientZoneDirector` and `CityAmbientZoneUpdateInterval`.
- Added last-sample, last-caption, and last-zone runtime state.
- Samples the nearest city through `FindClosestObjectiveIndex`, `FCodeRescueCampaign::GetMission`, `GetCityOrigin`, and mission origin-relative anchors.
- Uses `ACodeRescueGameMode::IsLocationInsideProtectedLearningZone` for protected coding-space classification.
- Reads `SolvedTerminalIds` and `RescuedSurvivorNames` to choose route-open, search, dormant, and extraction-ready ambience labels.
- Calls `UCodeRescueGameInstance::UpdateCityAmbientZone` with the zone label, bed key, and intended intensity.
- Adds QA tags: `CityAmbientZoneDirectorRuntime`, `WorldDevelopmentZoneAmbientCues`, `Top50Recommendation43SpatialAudio`, and `Top50Recommendation44AudioAccessibility`.

The state bridge lives in `UCodeRescueGameInstance`.

- Added `bCityAmbientZoneDirectorEnabled`, `CityAmbientZoneLabel`, `CityAmbientZoneBed`, and `CityAmbientZoneIntensity`.
- Added `UpdateCityAmbientZone` and `GetCityAmbientZoneSummary`.
- Extended `GetAudioMixSummary` with the current ambient zone label so the settings/readout path can expose ambience state beside master, SFX, music, and threat music.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/city_ambient_zone_audio_manifest.tsv`.
- Added creative-development, audio-coverage, visual-regression, human-QA, and accessibility entries for the city ambient zone director.
- Wired `Scripts/verify_city_ambient_zone_audio_slice_pass.py` into local CI and full QA.
- Logged this slice in `progress.md`.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_city_ambient_zone_audio_slice_pass.py`
- `python3 Scripts/verify_city_ambient_zone_audio_slice_pass.py`
- `python3 Scripts/verify_reactive_threat_audio_music_slice_pass.py`
- `python3 Scripts/verify_settings_audio_accessibility_slice_pass.py`
- `python3 Scripts/verify_city_radio_bark_cadence_slice_pass.py`
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Start a selected-language run and walk from entry approach to protected coding lab, safehouse exterior, city street, survivor route, survivor camp, and extraction pad before and after solving/rescuing. `[Ambient]` captions and the audio summary should name the zone and bed hook, while language selection, selected-language terminal validation, and future start-screen resume behavior remain unchanged.

## Remaining Art Hooks

This is a procedural fallback and hook contract. Future authored ambience can attach Sound Cues, MetaSound graphs, Submix sends, attenuation, and occlusion to the stable `CityAmbientZoneBed` keys instead of rediscovering zone logic. Dedicated follow-up slices now provide a HUD visualized-sound overlay and package-safe mono behavior for project-owned runtime cues.
