# Terminal Post-Solve Debrief Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: asks for deeper curriculum feedback and post-solve explanations.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasizes playability improvements that remain auditable and package-safe.
- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`: tracks `PostSolveExplanation`, `FailedChecks`, `TestCounts`, `HintEconomy`, and selected-language progress.

## Implementation

`UCodeTerminalWidget::OnValidateClicked()` now appends structured after-action learning text to the existing validation result.

Added helper coverage:

- `GetConceptProofForChallenge()` names why the passed checks prove the concept.
- `GetLanguageTransferForChallenge()` maps the concept back to the selected language.
- `GetNextPracticeRepForChallenge()` gives the next learning rep after a solve.
- `BuildPostSolveAfterActionDebrief()` emits the `POST-SOLVE DEBRIEF` block with concept proof, selected-language transfer, survivor-route follow-up, save continuity, next practice, rewards, and language progress.
- `BuildRepairDebrief()` emits the `REPAIR DEBRIEF` block with active failed check, language tactic, next validation move, and safehouse pause reminder.

The legacy `Intel Reward: survivor whereabouts uploaded` copy remains intact so existing QA contracts still identify successful terminal unlocks.

## Playability Impact

Correct solves now tell the player what worked, why it matters, how it transfers into the locked language run, where the survivor route opened, and that the active language save profile was updated. Failed solves now keep the feedback actionable by naming one repair target and one next validation move while reminding the player that combat remains paused inside the coding safehouse.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

Added:

- `Scripts/verify_terminal_post_solve_debrief_slice_pass.py`

## Validation

The verifier checks that terminal validation includes the post-solve and repair debrief helpers, keeps the survivor intel reward/profile output, documents the feature in curriculum/onboarding/visual/human-QA/creative manifests, and is wired into full QA plus local CI.

## Human QA Notes

Open a terminal from any selected language run. Submit one incorrect solution and confirm `REPAIR DEBRIEF` names the active target, language tactic, next validation move, and safehouse pause state. Submit a correct solution and confirm `POST-SOLVE DEBRIEF` names the concept proof, language transfer, survivor-route follow-up, save continuity, next practice, reward state, and language progress without hiding the original survivor intel reward.
