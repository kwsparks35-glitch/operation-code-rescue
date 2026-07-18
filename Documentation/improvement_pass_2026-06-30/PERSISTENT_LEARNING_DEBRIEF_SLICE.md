# Persistent Learning Debrief Slice

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: deepen curriculum feedback and make coding success teach the player why a solution worked.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keep playability improvements auditable, package-safe, and clear after save/resume.
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`: routes the terminal post-solve after-action debrief through persistent selected-language review.

## Runtime Implementation

- Added selected-language save fields for the latest learning debrief:
  `LastLearningDebriefChallengeId`, `LastLearningDebriefConcept`,
  `LastLearningDebriefLanguage`, `LastLearningDebriefScore`,
  `LastLearningDebriefSummary`, and `bHasLearningDebriefState`.
- Added `UCodeRescueGameInstance::RecordLearningDebrief()` and
  `GetLearningDebriefJournalSummary()` so terminal outcomes can be saved and the
  journal can render a compact readout.
- Clean terminal solves now save the generated `POST-SOLVE DEBRIEF`.
- Bypass-kit terminal solves now save an `ASSISTED LEARNING DEBRIEF`, preserving
  route progress while making clear that clean-solve rewards are disabled.
- The objective journal now includes `LearningDebriefReadoutText`, a
  `LAST LEARNING DEBRIEF` section near the language-save summary.

## Player-Facing Result

The terminal no longer owns the only copy of the learning takeaway. After a
successful validation or bypass, the player can close the terminal, open the
journal, save/quit, relaunch through the start-screen language Resume action,
and still see the latest challenge, concept, score, selected-language track,
save slot, and practice guidance.

## Data And QA Records

- Added `Content/CodeRescueData/persistent_learning_debrief_manifest.tsv`.
- Updated curriculum feedback, inventory/journal, accessibility,
  first-ten-minutes onboarding, visual-regression, human-QA, and creative
  inclusion records.
- Added `Scripts/verify_persistent_learning_debrief_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and
  `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier: `python3 Scripts/verify_persistent_learning_debrief_slice_pass.py`
- Adjacent verifiers:
  `python3 Scripts/verify_terminal_post_solve_debrief_slice_pass.py`,
  `python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py`,
  `python3 Scripts/verify_selected_language_terminal_flow_slice_pass.py`, and
  `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- Compile gate: `./Recompile_Module.command < /dev/null`
- Package/smoke gate remains covered by full QA and local CI readiness.

## Human QA Notes

Start a selected-language run, solve a terminal cleanly, close the terminal, and
open the journal. Confirm `LAST LEARNING DEBRIEF` names the language, save
slot, challenge, concept, score, and post-solve takeaway. Repeat with a bypass
kit and confirm the assisted debrief is clear. Save, close, relaunch from the
same language Resume action on the start screen, open the journal again, and
confirm the same learning debrief is still present.
