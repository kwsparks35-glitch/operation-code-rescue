# Fail-Safe Objective Board Slice

This pass extends the P1 inventory, map, and journal polish work with a playable recovery surface for players who lose track of the route after selecting a coding language. The board lives inside the existing `J` objective journal, so it does not interrupt the start screen, does not add a new modal flow, and does not change the selected-language save contract.

## Player-Facing Changes

- Added `FAIL-SAFE OBJECTIVE BOARD` to the objective journal.
- The board shows the selected coding language, the language-only save slot, and whether `Start-screen Resume` is available for that profile.
- The board names the active route phase: `protected terminal`, `survivor route`, `extraction/debrief`, or campaign complete.
- The board exposes return markers in text: `T objective jump`, `Backspace/F8 safe recovery`, `J journal`, and `P/Esc pause/save`.
- The board reminds players that the `protected terminal` suppresses combat pressure while learning.
- The board gives a direct next action for terminal repair, survivor routing, extraction/debrief, or campaign-complete review.

## Implementation

Runtime work is split between the save/game-state layer and the journal UI:

- `UCodeRescueGameInstance::GetFailSafeObjectiveBoardSummary()` derives the board from `SelectedLanguage`, `SaveSlotName`, language-specific save existence, `SolvedTerminalIds`, `RescuedSurvivorNames`, and the active `FCodeRescueCityMission`.
- `UCodeRescueObjectiveJournalWidget` now owns a named `FailSafeObjectiveBoardText` block.
- `RefreshJournal()` updates the block from the live `GameInstance` and restyles it through the same high-contrast/text-scale path used by the other journal readouts.
- The board is text-first and save-backed; it stores no new schema fields, so existing language saves continue to load.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/fail_safe_objective_board_manifest.tsv`.
- Updated `Content/CodeRescueData/inventory_map_journal_manifest.tsv`.
- Updated accessibility, visual regression, human QA, and first-ten-minutes review manifests.
- Updated the creative-development inclusion plan so the P1 `inventory map and journal polish` row now references `verify_fail_safe_objective_board_slice_pass.py`.
- Wired `Scripts/verify_fail_safe_objective_board_slice_pass.py` into full QA and local CI readiness.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_fail_safe_objective_board_slice_pass.py`
- `python3 Scripts/verify_fail_safe_objective_board_slice_pass.py`
- `python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py`
- `python3 Scripts/verify_objective_journal_accessibility_slice_pass.py`
- `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- `python3 Scripts/verify_creative_development_implementation_ledger.py`
- module recompile
- scoped `git diff --check`

## Human QA Notes

Open the journal with `J` before solving a terminal, after terminal solve, after survivor rescue, and again after start-screen resume. The board should clearly state the selected language, save slot, resume availability, active route phase, return markers, recovery controls, protected terminal safety, and next action without relying on color, audio, or terminal history.

## Remaining Art Hooks

This slice intentionally delivers the fail-safe board as a readable C++ UI surface. Future UI art can replace the text block with icons, control glyphs, or a framed objective board as long as the named `FailSafeObjectiveBoardText` contract and `GetFailSafeObjectiveBoardSummary()` source of truth remain intact.
