# Save Slots Language Backup UX Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS_2026-06-25.md`, recommendation 46: make progression and save surfaces clearer and more rewarding.
- Launch-language save contract: every playable run belongs to one selected coding language, and the start screen remains the authoritative place to start fresh or resume that language.

## Implementation

- Reframed `CodeRescueSaveSlotsWidget` as a manual backup overlay for the active coding-language run instead of a competing profile selector.
- Preserved compatibility with the existing backup files named `OperationCodeRescue_Slot0`, `OperationCodeRescue_Slot1`, and `OperationCodeRescue_Slot2`.
- Added a themed `LANGUAGE SAVE BACKUPS` panel with reduced blur, scalable text, high-contrast-aware colors, active language summary, start-screen resume slot name, and per-backup state summaries.
- Renamed row actions to `Save Backup`, `Load Backup`, and `Delete` so the player understands these are optional snapshots.
- Updated Save Backup to write the manual backup and then refresh `OperationCodeRescue_Language_<Track>`.
- Updated Load Backup to load the backup, then copy that backup into the loaded run's language resume slot so the start screen can resume it later.
- Updated Delete so it only removes the manual backup and restores `SaveSlotName` to the active language slot.

## Player Impact

- Players can create pause-menu backups without losing the per-language resume options on the start screen.
- Loading an older backup intentionally promotes it into the matching language resume save, making the next launch predictable.
- Empty, saved, and unreadable backup states are visible before the player presses a destructive or state-changing action.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueSaveSlotsWidget.h`
- `Source/CodeRescueUnreal/CodeRescueSaveSlotsWidget.cpp`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Scripts/verify_save_slots_language_backup_ux_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_save_slots_language_backup_ux_slice_pass.py`
- Adjacent contract verifiers: launch language start-screen save, save compatibility, onboarding input glyph, and settings accessibility.
- Compile/package/smoke should be run because this changes a runtime UMG widget and save/load behavior.

## Human QA Notes

- Start a new language run from the launch screen, pause, and open the save slots overlay.
- Confirm the panel title says `LANGUAGE SAVE BACKUPS`, the summary names the active language, and each row says `BACKUP N`.
- Save Backup 1, quit, relaunch, and confirm the start screen still offers the matching language resume save.
- Load Backup 1 and confirm the feedback says it loaded into the matching start-screen resume save.
- Delete Backup 1 and confirm the active language resume option remains available on the start screen.
