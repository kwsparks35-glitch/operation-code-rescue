# Terminal Practice Run Slice

This pass implements the campaign flow-plan promise for a `practice-only terminal option`. It gives players a safe rehearsal path inside the selected-language terminal before they commit a live validation attempt that saves progress, uploads survivor intel, and opens the rescue route.

## Player-Facing Changes

- Added `PRACTICE RUN [Ctrl+P]` beside the terminal validation controls.
- Practice runs use the same selected-language challenge validator as live validation.
- Practice output begins with `PRACTICE RUN - NO SAVE ADVANCE`.
- Passing practice output includes `PRACTICE-ONLY DEBRIEF`.
- Practice output includes `Practice Run Lock`, naming the selected language, unchanged language save slot, and future start-screen resume path.
- Practice runs keep `VALIDATE CODE` available and do not mark the code editor read-only.

## Save And Progress Safety

Practice runs deliberately do not:

- mark the terminal solved
- reveal the survivor route
- write terminal attempts or solves
- award `ResearchPoints`
- award coding score bonuses
- update learning attempts, streaks, perfect solves, or no-hint counters
- upload survivor intel archive entries
- write the selected-language save profile

Only `VALIDATE CODE` commits those outcomes.

## Implementation

Runtime work lives in `UCodeTerminalWidget`.

- `OnValidateClicked()` now calls `RunValidation(false)`.
- `OnPracticeClicked()` calls `RunValidation(true)`.
- `RunValidation(bool bPracticeOnly)` owns the shared validation path and gates save mutation, rewards, route reveal, survivor intel archive writes, and read-only solved-state behavior behind the non-practice branch.
- `PracticeRunButton` is created as `PracticeRunButton` with the label `PRACTICE RUN [Ctrl+P]`.
- `NativeOnKeyDown()` maps Ctrl+P or Command+P to `OnPracticeClicked()`.
- The ready output explains that practice checks code without saving or opening the route.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/terminal_practice_run_manifest.tsv`.
- Updated `Content/CodeRescueData/curriculum_feedback_manifest.tsv`.
- Updated `Content/CodeRescueData/selected_language_terminal_flow_manifest.tsv`.
- Updated accessibility, visual regression, human QA, first-ten-minutes, creative inclusion, and implementation-ledger records.
- Wired `Scripts/verify_terminal_practice_run_slice_pass.py` into full QA and local CI readiness.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_terminal_practice_run_slice_pass.py`
- `python3 Scripts/verify_terminal_practice_run_slice_pass.py`
- `python3 Scripts/verify_selected_language_terminal_flow_slice_pass.py`
- `python3 Scripts/verify_terminal_post_solve_debrief_slice_pass.py`
- `python3 Scripts/verify_persistent_learning_debrief_slice_pass.py`
- `python3 Scripts/verify_creative_development_implementation_ledger.py`
- module recompile
- scoped `git diff --check`

## Human QA Notes

Open a selected-language terminal, enter a correct solution, press `PRACTICE RUN [Ctrl+P]`, and confirm no route opens, no save updates, no rewards appear, and `VALIDATE CODE` remains enabled. Then press `VALIDATE CODE` and confirm the route opens, the selected-language save updates, and the normal post-solve debrief plus survivor intel archive write occur.
