# Challenge Replay Journal Slice

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: calls for stronger curriculum feedback, replay hooks, and visible/hidden test learning support.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps playable improvements auditable, save-safe, and package-safe.
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`: routes terminal after-action debrief work through persistent selected-language review.

## Runtime Implementation

- Added `UCodeRescueGameInstance::GetChallengeReplayJournalSummary()`.
- The summary is derived from the already saved learning debrief state:
  `LastLearningDebriefChallengeId`, concept, language, score, compact summary,
  and state flag.
- The replay brief detects clean solve versus bypass-kit-assisted route open and
  labels the replay state accordingly.
- Added challenge-family replay helpers for lock, reverse, palindrome,
  fizzbuzz, even filter, linked-list traversal, binary search, and accumulator
  lessons.
- Added `ChallengeReplayBriefText` to `UCodeRescueObjectiveJournalWidget`
  between `LAST LEARNING DEBRIEF` and the route map.

## Player-Facing Result

After solving or bypassing a terminal, the player can close the terminal, open
the journal with `J`, save/quit, relaunch through the same start-screen language
Resume action, and still see the start-screen language Resume contract:

- selected language track, saved challenge, concept, score, and save slot
- clean-solve or assisted-route replay status
- visible validation goal
- hidden-test replay note
- one next practice action before the next live terminal

This turns the campaign's replay-from-journal promise into a durable field
brief without reopening solved terminals or weakening mission progression.

## Data And QA Records

- Added `Content/CodeRescueData/challenge_replay_journal_manifest.tsv`.
- Updated curriculum feedback, inventory/journal, accessibility,
  first-ten-minutes onboarding, visual-regression, human-QA, and creative
  inclusion records.
- Added `Scripts/verify_challenge_replay_journal_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and
  `Run_Local_CI_Readiness.command`.

## Validation

- Static verifier:
  `python3 Scripts/verify_challenge_replay_journal_slice_pass.py`
- Adjacent verifiers:
  `python3 Scripts/verify_persistent_learning_debrief_slice_pass.py`,
  `python3 Scripts/verify_terminal_post_solve_debrief_slice_pass.py`,
  `python3 Scripts/verify_inventory_map_journal_polish_slice_pass.py`, and
  `python3 Scripts/verify_launch_language_start_screen_save_pass.py`
- Compile gate:
  `./Recompile_Module.command < /dev/null`

## Human QA Notes

Solve a terminal cleanly, close it, and open `J`; confirm `CHALLENGE REPLAY
BRIEF` names the language, challenge, concept, score, visible goal, hidden-test
replay note, and practice action. Repeat with a bypass-kit solve and confirm the
assisted replay status is clear. Save/quit, resume the same language from the
start screen, and confirm the replay brief survives with the same saved state.
