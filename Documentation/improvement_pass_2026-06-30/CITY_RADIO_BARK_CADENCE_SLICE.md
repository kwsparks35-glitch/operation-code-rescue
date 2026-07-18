# City Radio Bark Cadence Slice

This pass extends the P1 audio request for city radio and survivor barks, following the `WORLD_DEVELOPMENT_DEEPDIVE`, `TOP_50_RECOMMENDATIONS`, and `OPERATION_CODE_RESCUE_RELEASE_DOSSIER` guidance for readable world feedback that remains package-safe. The game already generated city radio briefings and survivor rescue subtitles, but early radio lines could be lost if they fired before the subtitle widget existed. This slice makes the subtitle system tolerant of early world-load bark timing and adds a contextual radio relay line for every loaded city.

## Player-Facing Changes

- City radio briefings now survive HUD construction timing because subtitle pushes made before the overlay exists are buffered.
- Each loaded city now queues a `[Radio Relay]` line after the main briefing.
- The relay line names the city/state, selected coding language, current route phase, terminal, survivor, landmark, and next step.
- The line changes with save state: terminal locked, survivor route open, or extraction beacon live.
- Existing survivor rescue barks remain intact and now share the same safer subtitle delivery path.

## Implementation

- `UCodeRescueSubtitlesWidget` now owns a static `PendingQueue`.
- `Push()` buffers up to eight early subtitle lines if `ActiveInstance` is not available yet.
- `NativeConstruct()` flushes pending lines only when subtitles are enabled, then existing subtitle scale and high-contrast styling apply normally.
- `BuildRadioRouteCadenceLine()` reads the active mission and `UCodeRescueGameInstance` save state to build a route-aware dispatch line.
- `SpeakRadioBriefing()` queues both the original mission briefing and the new route cadence line before trying cooked audio or macOS system voice.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/city_radio_bark_cadence_manifest.tsv`.
- Updated the creative-development inclusion plan so the P1 audio row now routes through `verify_city_radio_bark_cadence_slice_pass.py`, the survivor handoff verifier, the audio manifest, packaged smoke, and manual audio/subtitle review.
- Added visual regression, human QA, accessibility, audio manifest, and onboarding coverage.
- Wired the new verifier into full QA and local CI.

## Validation

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_city_radio_bark_cadence_slice_pass.py`
- `python3 Scripts/verify_city_radio_bark_cadence_slice_pass.py`
- existing survivor rescue dialogue handoff verifier
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Start a fresh run and confirm the first city radio briefing appears even if the HUD is still initializing. Then test three save states: before terminal solve, after terminal solve, and after survivor rescue. The relay line should tell the player the current route phase and next step without requiring audio playback.

## Future Audio Hooks

The current implementation is text-first and package-safe. Future audio production can add cooked radio stingers, survivor voice sets, spatial safehouse radios, and city-specific dispatch actors while keeping the same subtitle delivery and save-state cadence contract.
