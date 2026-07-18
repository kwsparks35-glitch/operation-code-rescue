# IK Foot Grounding Review Slice

Date: 2026-06-30

## Source Guidance

- `CHARACTER_ANIMATION_DEEPDIVE.md`: calls out UE 5.7 IK Rig, IK Retargeter, Control Rig, foot-to-ground contact, and pelvis/pose cleanup as the bridge from fallback characters to commercial-quality locomotion.
- `TOP_50_RECOMMENDATIONS.pdf`: prioritizes IK retargeting, locomotion Animation Blueprints, readable hit/death motion, and companion/survivor personality.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps cooked Mac playability and auditable promotion gates ahead of final authored assets.

## Implementation

- Extended `CodeRescueRetargetRig` with `ApplyFootGroundingReview`, called by `ApplyRuntimeRetargetRigSlots`.
- Added `FootGroundingRuntimeContract`, `FootIKGroundingReview`, `FootPlantTraceReady`, `PelvisOffsetReview`, `ControlRigFootContactReady`, and `RetargetFootContactPoseReview` tags to every full-body skeletal profile.
- Added profile-specific grounding tags for player operator, survivor hero, friendly NPC, companion hero, zombie crowd, and boss warden.
- Explicitly excluded first-person arms with `FootGroundingExcluded_FirstPersonArms`, preserving the separate hand/weapon IK path.
- Added a visible `FOOT IK` station to the runtime DCC/Maya review bay so visual QA can inspect the requirement inside the packaged game.

## Player Impact

- This slice does not pretend final IK assets already exist. It makes the requirement unavoidable: any promoted full-body character must preserve grounded feet, pelvis offset review, and Control Rig foot contact.
- Future survivor rescue gestures, companion formations, zombie shambles, and boss phase attacks now have a shared runtime promotion contract instead of per-actor notes.
- The first-person arms path remains cleanly separated from full-body locomotion, avoiding accidental foot-IK assumptions on weapon-only meshes.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueRetargetRig.h`
- `Source/CodeRescueUnreal/CodeRescueRetargetRig.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Content/CodeRescueData/ik_foot_grounding_review_manifest.tsv`
- `Content/CodeRescueData/retarget_control_rig_slots_manifest.tsv`
- `Content/CodeRescueData/animation_coverage_manifest.tsv`
- `Content/CodeRescueData/character_promotion_validation_contract.tsv`
- `Content/CodeRescueData/maya_character_cleanup_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Scripts/verify_ik_foot_grounding_review_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Validation

- Static verifier: `python3 Scripts/verify_ik_foot_grounding_review_slice_pass.py`
- Adjacent verifiers: retarget/control-rig slots, animation budget runtime, Maya cleanup, player first-person animation, companion gesture readability, survivor rescue dialogue handoff, zombie motion readability, and boss phase telegraph.
- Compile/package/smoke should be run because this changes runtime C++ helper tags and the generated world review bay.

## Human QA Notes

- Inspect the DCC review bay and confirm the `FOOT IK` station appears beside the Maya cleanup stations.
- Inspect player body, survivor, friendly NPC, companion, zombie, and boss skeletal actors for `FootGroundingRuntimeContract`, `FootIKGroundingReview`, `ControlRigFootContactReady`, `FootPlantTraceReady`, and `PelvisOffsetReview`.
- Inspect first-person arms and confirm they carry `FootGroundingExcluded_FirstPersonArms`.
- During future authored IK/Control Rig promotion, reject any full-body mesh whose feet slide, float, sink, or lose pelvis offset review on uneven city surfaces.
