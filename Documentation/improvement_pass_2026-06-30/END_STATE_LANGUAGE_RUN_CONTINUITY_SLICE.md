# End State Language Run Continuity Slice

Date: 2026-06-30

## Source Guidance

- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasizes release-readiness, save reliability, and clear player-facing state.
- `UX_OVERHAUL_GUIDE.md`: calls for shared visual language and readable end-to-end UI flow.
- `TOP_50_RECOMMENDATIONS.pdf`: stresses clearer post-objective feedback and recovery paths.

## Implementation

- Upgraded `UCodeRescueDeathWidget` and `UCodeRescueVictoryWidget` to mirror saved high-contrast, reduced-motion, and subtitle/text-scale settings before building their overlays.
- Added scrollable end-state panels so large accessibility text can still expose title, summary, stats, and actions at 720p.
- Added language-run summaries to both screens, including the active/completed language, `MakeLanguageSaveSlotName()` start-screen resume slot, and `GetLanguageProgressSummary()`.
- Expanded death stats with research points, run time, death count, and headshots while preserving existing survivor, terminal, zombie, and score details.
- Expanded victory stats with research points, run time, death count, and headshots while preserving existing leaderboard submission behavior.
- Renamed end-state actions so buttons explicitly communicate language-run effects: resume from language save, start fresh language run, save this language run and quit, and save completion and quit.
- Added `SavePersistentRun()` when victory is constructed and before victory quit so completed language progress remains available from the future start screen.

## Player Impact

- Death and victory no longer feel like generic modal overlays; they now explain what happens to the selected coding-language run.
- Players can see the exact language resume slot before quitting, restarting, or starting fresh.
- Reduced motion and large-text users get calmer blur and scrollable summaries instead of clipped end-state content.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueDeathWidget.h`
- `Source/CodeRescueUnreal/CodeRescueDeathWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueVictoryWidget.h`
- `Source/CodeRescueUnreal/CodeRescueVictoryWidget.cpp`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/safe_learning_city_controls_manifest.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Scripts/verify_end_state_language_run_continuity_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_end_state_language_run_continuity_slice_pass.py`
- Adjacent verifier: `python3 Scripts/verify_save_slots_language_backup_ux_slice_pass.py`
- Compile/package/smoke should be run because this changes runtime modal widgets and save timing.

## Human QA Notes

- Force player death and confirm the screen shows `YOU WERE OVERRUN`, active language, start-screen resume slot, expanded stats, and language-run action labels.
- Complete a city/campaign victory state and confirm the screen shows `EXTRACTION COMPLETE`, completed language, start-screen resume slot, expanded stats, and `SAVE COMPLETION AND QUIT`.
- Toggle High Contrast HUD, Subtitle/Text size, and Reduced Motion before opening each end-state screen; confirm the panel remains readable and scrollable.
- Use `SAVE THIS LANGUAGE RUN AND QUIT` on death, relaunch, and confirm the start screen offers the same language resume.
- Use `SAVE COMPLETION AND QUIT` on victory, relaunch, and confirm the completed language run remains available.
