# Field Checklist HUD Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: calls for first-time-user onboarding, consistent input guidance, and always-readable objective clarity.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: asks the city route to remain understandable as the player moves from coding space to survivor route to extraction.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps selected-language save continuity and release-readiness QA visible in normal play.

## Implementation

- Added `FieldChecklistText` to `UCodeRescueHUDWidget` as `FirstTenMinutesFieldChecklistText`, positioned beside the existing minimap/navigation stack.
- Added a compact `FIRST TEN MINUTES FIELD CHECKLIST` readout during `RefreshHUD()`.
- The checklist names the active language track, language-specific save slot, and whether the selected run is `start-screen Resume ready` or still waiting for progress autosave.
- The checklist keeps the first-route shape visible as `protected terminal -> survivor marker -> extraction`.
- The phase line changes from protected terminal to survivor marker to extraction based on `SolvedTerminalIds` and `RescuedSurvivorNames`.
- The key line changes by phase and includes `E`, `Ctrl+P`, `T`, `Backspace/F8`, `J`, and `P/Esc` where relevant.
- High Contrast HUD swaps to a brighter checklist color, while auto-wrap keeps the text readable in the compact panel.

## Player Impact

- After selecting or resuming a coding language, the player keeps a small, persistent field checklist without reopening the tutorial.
- The live HUD now ties selected-language save continuity, route phase, and recovery controls together during the first playable minutes.
- The checklist complements objective toasts, minimap, radio scan, and journal guidance instead of replacing them.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Content/CodeRescueData/field_checklist_hud_manifest.tsv`
- `Content/CodeRescueData/objective_route_toast_clarity_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Scripts/verify_field_checklist_hud_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_field_checklist_hud_slice_pass.py`
- Adjacent verifiers: objective route toast clarity, minimap route readability, onboarding input glyph, launch language start-screen save, and creative implementation ledger.
- Compile/package/smoke should be run because this changes a runtime HUD widget.

## Human QA Notes

Start a selected-language run from the language start screen, then inspect the checklist before terminal solve, after terminal solve, after survivor rescue, and after save/resume. Confirm the checklist stays readable, does not overlap the minimap/navigation stack, names the correct phase and save state, and keeps first-route recovery keys visible.
