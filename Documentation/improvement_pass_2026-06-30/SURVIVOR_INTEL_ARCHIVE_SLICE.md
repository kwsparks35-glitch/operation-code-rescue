# Survivor Intel Archive Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: asks for clearer coding reward payoff, stronger learning feedback, and resume-safe progression.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: emphasizes readable city routes, survivor stakes, and coding-to-world cause and effect.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: requires package-safe, reviewable implementation records for release readiness.

## Implementation

The live `SURVIVOR INTEL DOSSIER` now has a save-backed companion readout: `SURVIVOR INTEL ARCHIVE`.

Added:

- SaveGame and GameInstance fields for the latest uploaded survivor intel terminal, survivor, city route, selected language, status, compact summary, score, and state flag.
- `UCodeRescueGameInstance::RecordSurvivorIntelDossier()` to write the archive into the active selected-language save.
- `UCodeRescueGameInstance::GetSurvivorIntelArchiveSummary()` to expose a journal-safe summary after resume.
- Terminal success and bypass-kit paths that call `RecordSurvivorIntelArchiveForTerminal()` so both clean validation and assisted field repair write the same durable route intel.
- Rescue-state refresh in `MarkSurvivorRescued()` so the matching archived survivor changes from `ROUTE OPEN` to `RESCUED` before the existing rescue checkpoint save.
- `SurvivorIntelArchiveText` in the objective journal, displayed below the current mission dossier.

## Playability Impact

The player can solve or bypass a terminal in one coding language, close the game, return to the start screen, choose that language's Resume action, and still find the latest uploaded survivor contact, route, score, save slot, and next step in the journal. The archive is language-slot data, so Java, Python, C++, C, C+, and MATLAB runs keep separate rescue intel instead of leaking progress between tracks.

## Accessibility

The archive stays inside the scalable objective journal surface. It uses auto-wrapped text, explicit status labels, high-contrast-aware styling, and saved text summaries so route intel is not dependent on color, sound, motion, or terminal output history.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/inventory_map_journal_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Documentation/improvement_pass_2026-06-30/CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

Added:

- `Content/CodeRescueData/survivor_intel_archive_manifest.tsv`
- `Scripts/verify_survivor_intel_archive_slice_pass.py`

## Validation

The verifier checks save fields, GameInstance reset/save/load behavior, terminal clean-solve and assisted-bypass upload hooks, journal construction and refresh, rescue status refresh, the archive manifest, creative-development ledger coverage, QA wiring, and progress documentation.

## Human QA Notes

Start a selected-language run, solve or bypass the active terminal, open `J`, and confirm `SURVIVOR INTEL ARCHIVE` names the selected language, save slot, terminal, contact, route, score, and upload summary. Save/quit, return to the start screen, resume the same language, and confirm the archive is unchanged. Rescue the matching survivor, save/relaunch, and confirm the archive status changes to `RESCUED` with the extraction/debrief update.
