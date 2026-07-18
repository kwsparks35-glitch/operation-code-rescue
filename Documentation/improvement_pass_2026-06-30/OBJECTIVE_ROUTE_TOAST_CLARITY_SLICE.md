# Objective Route Toast Clarity Slice

Date: 2026-06-30

## Source Guidance

- `TOP_50_RECOMMENDATIONS.pdf`: prioritizes clear objective markers, route guidance, and immediate feedback when player progress changes.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: asks the route through city, survivor, and extraction spaces to be readable without relying on memory or hidden state.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps selected-language progression and start-screen resume behavior visible as a release-readiness contract.

## Implementation

- Added `ObjectiveRouteToastText` to `UCodeRescueHUDWidget` as a centered, text-first route acknowledgement layer above the crosshair.
- Added `TriggerObjectiveRouteToast` and `RefreshObjectiveRouteToast` helpers so the HUD observes changes in solved terminal count, rescued survivor count, coding score, and save timestamp.
- Seeded the observed progress state on the first HUD refresh so resumed saves do not replay old terminal, survivor, or checkpoint notices.
- Added terminal solve messaging: `OBJECTIVE UPDATED | Terminal solved | Survivor route open | +N code score`.
- Added survivor rescue messaging: `OBJECTIVE UPDATED | Survivor rescued | Extraction ready | Language save refreshed`.
- Added checkpoint messaging: `CHECKPOINT SAVED | <language> run can resume from the start screen`.
- Honored accessibility state by using high-contrast color alternatives, reduced-motion stable alpha, shared UI text styling, and a small visualized-sound-cue brightness handoff when objective route state changes.

## Player Impact

- Players get immediate confirmation that a code success opened the survivor route instead of needing to infer that from the journal or minimap alone.
- Survivor rescue now clearly acknowledges extraction readiness and selected-language save continuity.
- Manual or automatic save moments now reinforce that the active coding-language run can be resumed from the always-present start screen.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Content/CodeRescueData/objective_route_toast_clarity_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Scripts/verify_objective_route_toast_clarity_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- Static verifier: `python3 Scripts/verify_objective_route_toast_clarity_slice_pass.py`
- Adjacent verifiers: objective focus beacon, minimap route readability, HUD vitals accessibility, terminal post-solve debrief, survivor rescue dialogue handoff, inventory/map/journal polish, and save compatibility.
- Compile/package/smoke should be run because this changes a runtime HUD widget.

## Human QA Notes

- Start a selected-language run and confirm no old toast appears on the first HUD frame after resume.
- Solve a terminal and confirm the route toast names terminal solved, survivor route open, and code-score gain.
- Rescue the survivor and confirm the route toast names survivor rescued, extraction ready, and language save refresh.
- Trigger a save and confirm the route toast names the active language run and start-screen resume path.
- Toggle High Contrast HUD, Reduced Motion, Visualize Sound Cues, and UI Text Size, then confirm the toast remains readable without replacing the persistent objective, minimap, or sound-cue lines.
