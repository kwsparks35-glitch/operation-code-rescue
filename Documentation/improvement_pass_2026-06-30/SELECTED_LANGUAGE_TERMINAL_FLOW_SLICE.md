# Selected Language Terminal Flow Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: prioritize readable first-play flow, clear coding-language selection, and save continuity.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keep release-facing playability work auditable with direct validation gates.
- Prior start-screen work already introduced language-specific saves; this slice ties that work to terminal validation and reviewer-facing evidence.

## Implemented Work

- Preserved the launch-only start screen where Java, C, C+, C++, Python, and MATLAB each have explicit new-run and resume-save actions before gameplay.
- Preserved `UCodeRescueGameInstance::MakeLanguageSaveSlotName()`, `StartFreshLanguageRun()`, and `ResumeLanguageRun()` as the authority for per-language save isolation.
- Preserved active-play language locking: terminals refresh to `GI->SelectedLanguage`, and language stations cannot switch the player into another track after deployment.
- Added a `Language Run Lock` block to terminal validation output. After failed or passing attempts, the output now names the active language, the exact save profile updated, and the start-screen resume path for that language.
- Hardened `UCodeRunnerLibrary::ValidateInEngine()` so the C reverse-string lesson accepts valid output-buffer solutions that copy `input[length - 1 - i]` into `output[i]` and terminate the buffer.
- Added `Content/CodeRescueData/selected_language_terminal_flow_manifest.tsv` so the flow is reviewable from launch choice through terminal validation and future resume.

## Playability Impact

Players now receive a visible chain of custody for their chosen coding language: choose or resume a language before play, see the locked track inside the terminal, validate only that language's starter code, and receive output confirming the same language save profile was updated. This directly supports parallel language runs and future start-screen resume without letting one language's terminal attempts or saves blur into another.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

Added:

- `Content/CodeRescueData/selected_language_terminal_flow_manifest.tsv`
- `Scripts/verify_selected_language_terminal_flow_slice_pass.py`

## Validation

The verifier checks launch-menu language rows, per-language save helpers, active-play language locking, terminal locked-track copy, the new `Language Run Lock` validation output, C reverse output-buffer fallback support, post-solve/repair continuity, manifest coverage, human/visual QA routing, creative-plan routing, progress documentation, and CI wiring.

## Human QA Notes

Start fresh Java, C, C+, C++, Python, and MATLAB runs from the launch screen. Create or use at least two language saves, relaunch, and confirm the start screen still appears with resume options for those languages. In a terminal, submit one incorrect and one correct attempt, then confirm the output includes `Language Run Lock`, the terminal remains locked to the chosen track, and the resume path reopens the same language save profile.
