# Survivor Rescue Dialogue Handoff Slice

Date: 2026-06-30

## Source Guidance

- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: calls for stronger survivor identity, rescue readability, and future-friendly handoff points for authored character moments.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: emphasizes clear coding-to-world cause and effect, city-specific landmarks, and readable extraction routes.
- `TOP_50_RECOMMENDATIONS.pdf`: asks for more obvious reward feedback, better moment-to-moment player guidance, and stronger rescue-loop communication.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: prioritizes package-safe implementation, save continuity, and documented validation.

## Implementation

`ASurvivorActor::Rescue()` now resolves the active `FCodeRescueCityMission` once at the start of the interaction and uses it to drive all survivor rescue subtitles.

Added helper builders:

- `BuildSurvivorLockedRouteLine()` tells the player which terminal, concept, selected language, and landmark still block the rescue.
- `BuildSurvivorRescueLine()` replaces the generic thank-you line with a mission-aware payoff that names the selected language solve, concept, landmark, novel gameplay detail, and survivor story.
- `BuildExtractionDispatchLine()` queues a dispatch handoff after save persistence, confirming the survivor, city, live helipad route, selected-language save update, and `RESCUED` journal dossier state.
- `BuildCompanionHandoffLine()` makes the first survivor companion join line refer to the active route instead of using a generic combat bark.

The existing rescue loop remains intact. Successful rescue still plays the optional voice cue, spawns `ARescueExtractionPresentationActor`, marks the matching `AHelipadActor` extraction-ready, calls `GI->MarkSurvivorRescued()`, saves through `GI->SavePersistentRun()`, spawns the first companion only once, hides the rescued survivor, and disables collision.

## Playability Impact

The player now hears a complete route story:

1. Before solving the terminal, the survivor explains that the route is locked and names the active language task.
2. After solving and rescuing, the survivor confirms that the selected-language fix opened the city landmark.
3. Dispatch confirms extraction, save continuity, and the journal state so the player understands that the start-screen language run has progressed.

This closes the loop between the terminal post-solve debrief, the objective journal survivor dossier, the visible extraction beacon, and the persistent language save.

## Accessibility

The work uses the existing subtitle queue, so lines respect `bSubtitlesEnabled`, `SubtitleScale`, and high-contrast subtitle styling. The dispatch line is text-first and does not rely on color, animation, or audio assets being available.

## Data / Audit Updates

Updated:

- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/novel_character_world_design_manifest.tsv`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

Added:

- `Scripts/verify_survivor_rescue_dialogue_handoff_slice_pass.py`

## Validation

The verifier checks mission-aware dialogue helpers, blocked-route copy, successful rescue copy, dispatch copy, first-companion copy, rescue-loop preservation, manifest coverage, QA wiring, documentation, and progress logging.

## Human QA Notes

Attempt to rescue the active survivor before solving the terminal and confirm the subtitle names the blocked terminal, concept, language, and landmark. Solve the terminal and rescue the survivor, then confirm the survivor line names the selected-language fix and city landmark, dispatch confirms helipad route/save/dossier state, the extraction beacon remains visible, and the journal dossier reads `RESCUED`.
