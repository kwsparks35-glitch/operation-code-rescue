# Maya Character Cleanup Slice

This pass implements the P1 pipeline request for Maya character cleanup as a package-safe gameplay and validation contract. The June 25 character-animation guidance asks the project to prepare imported characters, sockets, retargeting, animations, physics assets, and Mac-safe runtime presentation before final authored assets replace stable fallbacks. This slice makes that handoff explicit for every live skeletal path while keeping the current playable build intact.

## Runtime Integration

- `CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots()` now adds a shared `MayaCharacterCleanupRuntimeContract` to every runtime skeletal path.
- Common cleanup tags cover the required promotion gates:
  - `MayaFbxExportReview`
  - `MayaSkeletonNamingReview`
  - `MayaBindPoseOriginReview`
  - `MayaAnimationTakeCleanupReview`
  - `MayaSocketAuthoringReview`
  - `MayaLODMaterialBudgetReview`
  - `MayaPhysicsAssetReview`
- Profile-specific tags identify the practical cleanup work for player body, first-person arms, survivor, friendly NPC, companion, zombie crowd, and boss warden assets.
- `SpawnUnrealSystemsCharacterWorldLayer()` now adds a visible Maya cleanup lane in the DCC review bay. The lane has a recipe board plus stations for bind pose, skeleton naming, sockets, animation takes, LOD/material budget, physics asset, FBX export, and promotion evidence.
- The cleanup lane is tagged with `MayaCharacterCleanup`, `MayaCleanupRecipeBoard`, `MayaRetargetRoundtripReview`, `MayaPromotionEvidenceReady`, and the same review tags used by live skeletal actors.

## Data And QA

- Added `Content/CodeRescueData/maya_character_cleanup_manifest.tsv`.
- Expanded `Content/CodeRescueData/animation_coverage_manifest.tsv` with Maya cleanup profiles for every runtime skeletal owner.
- Updated the character promotion contract so Maya/FBX promotion requires bind-pose/origin review, skeleton naming review, socket authoring review, animation take cleanup, LOD/material budget evidence, physics asset review when relevant, and packaged promotion evidence.
- Updated the creative-development inclusion plan, human QA checklist, and visual regression targets to include the Maya cleanup verifier and manual animation/socket review.
- Wired `Scripts/verify_maya_character_cleanup_slice_pass.py` into both local CI and full QA.

## Validation

The slice verifier checks:

- cleanup tags in `CodeRescueRetargetRig`
- visible cleanup stations in `SpawnUnrealSystemsCharacterWorldLayer`
- the new manifest, animation coverage rows, character promotion contract, creative plan, human QA checklist, and visual regression target
- CI wiring in `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`
- documentation and progress-log coverage

Full validation should run the new static verifier, retarget/control-rig verifier, character promotion contract verifier, character promotion Unreal commandlet, module recompile, Mac packaging, packaged null smoke, and packaged render smoke.

## Boundaries

This pass does not author final `.ma`, `.mb`, `.fbx`, `.ikrig`, `.ikretargeter`, Control Rig Blueprint, Physics Asset, or animation montage files. It establishes the runtime and review contract those assets must satisfy before they replace the current fallback meshes and AnimBPs.
