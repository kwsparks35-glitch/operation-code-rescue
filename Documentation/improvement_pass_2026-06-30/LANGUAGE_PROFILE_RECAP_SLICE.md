# Language Profile Recap Slice

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: calls for profile stats, review
  recommendations, stage recap, and save-slot previews in the learning loop.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps save/resume behavior
  auditable and player-readable.
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`: routes the
  terminal after-action debrief inclusion through persistent selected-language
  journal review.

## Runtime Implementation

- Added `UCodeRescueGameInstance::GetLanguageProfileRecapSummary()`.
- The recap derives from existing selected-language save fields rather than
  creating a new save schema:
  language solve/attempt/no-hint counters, mastery title, streaks,
  Research Points, score, difficulty, run scoreboard, and start-screen save-slot
  state.
- Added `BuildLanguageProfileReviewRecommendation()` so the journal recommends
  different next actions for no attempts, no solves, low success rate, low
  no-hint rate, perfect-solve/streak growth, campaign completion, and normal
  advancement.
- Added `LanguageProfileRecapText` to `UCodeRescueObjectiveJournalWidget`
  directly beneath the language save continuity line.

## Player-Facing Result

The journal now fulfills the campaign flow-plan promise for `profile stats`,
`stage recap`, `review recommendation`, and `save-slot preview`. The player can
open `J` during a fresh run, after a terminal solve, after rescue, or after
start-screen Resume and see:

- selected language, mastery title, and difficulty
- language solves, attempts, success rate, no-hint solves, perfect solves,
  streaks, Research Points, and score
- run stats for survivor rescues, zombie neutralizations, headshots, deaths,
  and play time
- active curriculum stage, city, tier, or campaign-complete recap
- review recommendation and selected-language save-slot preview

## Data And QA Records

- Added `Content/CodeRescueData/language_profile_recap_manifest.tsv`.
- Updated curriculum feedback, inventory/journal, accessibility,
  first-ten-minutes onboarding, visual-regression, human-QA, and creative
  inclusion records.
- Added `Scripts/verify_language_profile_recap_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and
  `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier:
  `python3 Scripts/verify_language_profile_recap_slice_pass.py`
- Adjacent verifiers:
  `python3 Scripts/verify_challenge_replay_journal_slice_pass.py`,
  `python3 Scripts/verify_persistent_learning_debrief_slice_pass.py`,
  `python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py`,
  `python3 Scripts/verify_objective_journal_accessibility_slice_pass.py`, and
  `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- Compile gate:
  `./Recompile_Module.command < /dev/null`

## Human QA Notes

Start a selected-language run, open `J`, and confirm `LANGUAGE PROFILE RECAP`
shows a new-run save-slot preview. Solve a terminal and confirm profile stats,
stage recap, and review recommendation update. Save/quit, resume the same
language from the start screen, and confirm the recap still names the selected
language, stage, recommendation, and save slot.
