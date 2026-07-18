# Survivor Intel Dossier Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: calls for stronger survivor intel payoff and clearer objective feedback after coding.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: emphasizes coding-to-world cause and effect, readable routes, and city-specific mission identity.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: favors package-safe implementation with clear verification records.

## Implementation

`UCodeRescueObjectiveJournalWidget` now includes a compact `SURVIVOR INTEL DOSSIER` card directly under the route summary.

Added:

- `IntelText` to persist the dossier widget.
- `FindJournalMissionProgress()` to show attempts and best score for the active terminal.
- `SurvivorIntelStatusLabel()` with `LOCKED`, `ROUTE OPEN`, and `RESCUED` states.
- `SurvivorIntelNextStep()` to connect the dossier to the immediate route action.
- `BuildSurvivorIntelDossier()` to assemble survivor contact, city rank, location, lesson payoff, validation record, selected-language run, and next step.

## Playability Impact

After a terminal solve, the journal now makes the rescue payoff visible outside the terminal output. The player can press `J` and see which survivor team is active, where the route points, what lesson opened that route, how validation went, which language run is being advanced, and whether the next step is coding, survivor rescue, or extraction.

## Accessibility

The dossier stays inside the existing objective journal accessibility surface. It uses wrapped text, shared type scaling, reduced-motion journal blur, and high-contrast-aware color states so `LOCKED`, `ROUTE OPEN`, and `RESCUED` remain readable without relying on color alone.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

Added:

- `Scripts/verify_survivor_intel_dossier_slice_pass.py`

## Validation

The verifier checks the dossier widget storage, construction, helper functions, terminal/survivor-progress binding, mission-progress readout, selected-language progress line, manifest coverage, documentation, progress log, and QA wiring.

## Human QA Notes

Open the journal before solving the active terminal and confirm `SURVIVOR INTEL DOSSIER` reads `LOCKED`. Solve the terminal, reopen the journal, and confirm it changes to `ROUTE OPEN` with survivor contact, location, lesson payoff, validation line, language run, and next step. Rescue the survivor, reopen the journal, and confirm the dossier changes to `RESCUED` and points toward extraction/debrief.
