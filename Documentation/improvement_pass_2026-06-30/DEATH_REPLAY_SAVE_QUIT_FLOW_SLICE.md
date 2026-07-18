# Death Replay Save-And-Quit Flow Slice

## Purpose

This slice completes the P0 death replay and save-and-quit requirement from the June 25 playability guidance. The death screen already exposed the right high-level choices, but the implementation needed a stronger save contract so a player could die, save the selected-language run, relaunch through the start screen, and resume a playable checkpoint in that same language.

## Implementation

- Added `UCodeRescueGameInstance::SaveDeathRecoveryCheckpoint(bool bCountDeath)` and the private `SavePersistentRunInternal(bool bCaptureWorldState)` helper.
- Updated `UCodeRescueGameInstance::IncrementDeathCount()` so death progress persists through `SaveDeathRecoveryCheckpoint(false)` instead of using the regular live-state save path.
- `SaveDeathRecoveryCheckpoint` captures the live world once, then rewrites the saved recovery state to the current city entry pad with a safe rotation, playable health, and minimum recovery resources before serializing.
- Updated `ACodeRescueCharacter::ApplyDamage()` so the first zero-health transition increments the death counter and saves the playable selected-language recovery checkpoint before showing the death overlay.
- Added `DeathActionStatus` text to `UCodeRescueDeathWidget` explaining the recovery checkpoint, active language slot, and start-screen resume behavior.
- Updated death-screen actions:
  - `RESUME FROM LANGUAGE SAVE` now calls `ResumeLanguageRun(GI->SelectedLanguage)` before reopening the level.
  - `SAVE THIS LANGUAGE RUN AND QUIT` now calls `SaveDeathRecoveryCheckpoint(false)` so it never serializes a zero-health pawn.
  - `START FRESH LANGUAGE RUN` keeps deleting only the active language save slot.
  - `QUIT TO DESKTOP` logs the selected language and slot without making an additional live-state save.

## Player Impact

Death is now a replay point instead of a save-corruption risk. The player keeps the selected coding-language track, solved terminals, rescued survivors, defeated zombies, score, learning stats, and death count, but resumes from a playable city-entry checkpoint rather than the exact defeat spot. Save-and-quit from the death screen writes the same language profile that the start screen can offer on a future launch.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueDeathWidget.h`
- `Source/CodeRescueUnreal/CodeRescueDeathWidget.cpp`
- `Content/CodeRescueData/death_replay_save_quit_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/verify_death_replay_save_quit_flow_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- `python3 -m py_compile Scripts/verify_death_replay_save_quit_flow_slice_pass.py`
- `python3 Scripts/verify_death_replay_save_quit_flow_slice_pass.py`
- `python3 Scripts/verify_end_state_language_run_continuity_slice_pass.py`
- `python3 Scripts/verify_health_damage_survivability_slice_pass.py`
- `python3 Scripts/verify_save_compatibility_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

## Human QA Notes

Force a death in a selected language run. Confirm the death screen shows the active language, start-screen resume slot, playable recovery checkpoint text, run stats, and all four actions. Use `RESUME FROM LANGUAGE SAVE` and confirm the same language run reloads from the city entry pad with health above zero. Force death again, choose `SAVE THIS LANGUAGE RUN AND QUIT`, relaunch, and confirm the start screen still appears but offers resume for that saved language profile.
