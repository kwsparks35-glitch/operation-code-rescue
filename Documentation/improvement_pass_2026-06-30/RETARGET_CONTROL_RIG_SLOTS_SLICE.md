# Retarget Control Rig Slots Slice

This pass implements the P1 character request for IK retargeting and Control Rig slots in a package-safe runtime form. The June 25 character-animation guidance calls for IK Rig, IK Retargeter, Control Rig, Maya cleanup, and montage-ready character assets, but the game still needs to run cleanly before every final authored asset exists. This slice establishes the integration contract those assets will use while preserving the current Manny, Quinn, zombie-pack, and procedural fallback presentations.

## Runtime Helper

Added `CodeRescueRetargetRig.h/.cpp` with `CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots()`.

The helper adds shared tags to the live skeletal mesh component and owning actor:

- `IKRetargetRuntimeSlot`
- `ControlRigRuntimeSlot`
- `RetargetControlRigRuntimeContract`
- `MayaCharacterCleanupTarget`
- `CharacterAnimationDeepDive`

It also adds profile-specific tags for player body, first-person arms, survivor heroes, friendly NPCs, companions, zombie crowds, and boss wardens. These tags make screenshots, QA inspection, and future editor validators able to distinguish full-body rigs, arms-only rigs, facial slots, foot-IK review, weapon socket review, zombie attack retargeting, boss reveal Control Rig, boss phase montages, and ragdoll physics review.

## Actor Integration

The runtime slots are now applied to:

- `ACodeRescueCharacter::GetMesh()` as `PlayerOperator`
- `ACodeRescueCharacter::FirstPersonArmsMesh` as `FirstPersonArms`
- `ASurvivorActor::ProfessionalSkeletalBody` as `SurvivorHero`
- `AFriendlyNPCActor::SkeletalNPCBody` as `FriendlyNPC`
- `ACompanionActor::GetMesh()` as `CompanionHero`
- `ACodeZombieActor::GetMesh()` as `ZombieCrowd`
- `ABossZombieActor::GetMesh()` as `BossWarden`

The existing fallback meshes and AnimBPs remain active. This keeps the current playable build stable while making each skeletal path ready for future authored IK Rig, IK Retargeter, Control Rig, montage, and Maya export promotion.

## Review Artifacts

Added `Content/CodeRescueData/retarget_control_rig_slots_manifest.tsv` as the source-of-truth slot table. It names each profile, runtime owner, skeletal component, source skeleton family, fallback mesh, fallback AnimBP, profile tags, promotion target, and validation route.

Updated `Content/CodeRescueData/animation_coverage_manifest.tsv`, `character_promotion_validation_contract.tsv`, `creative_development_inclusion_plan.tsv`, `human_qa_signoff_checklist.tsv`, and `visual_regression_targets.tsv` so this slice appears in the normal creative-development, QA, and review trail.

## Validation

Added `Scripts/verify_retarget_control_rig_slots_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

Expected validation:

- Python verifier compilation
- `python3 Scripts/verify_retarget_control_rig_slots_slice_pass.py`
- adjacent animation-budget and camera/character roster verifiers
- module recompile
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Manual Review

Spawn into the default city and inspect the player body, first-person arms, survivor, friendly NPC safehouse cast, companion, standard zombie, and boss/warden actors. Each should retain readable fallback animation or silhouette while carrying the retarget/control-rig profile tags that identify the final authored asset lane.

## Boundaries

This does not author final `.ikrig`, `.ikretargeter`, Control Rig Blueprint, Maya scene, facial rig, or montage assets. It converts the request into a live runtime contract and review surface so those authored assets can be promoted without ambiguity or hidden one-off wiring.
