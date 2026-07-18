# Runtime Skeletal Animation Budget Slice

This pass continues the June 25 `CHARACTER_ANIMATION_DEEPDIVE` work by adding a runtime skeletal animation budget policy for the characters that now use imported mannequin and zombie meshes. The deep-dive called out explicit performance budgets, offscreen animation throttling, reduced distant work, and animation update-rate optimization as requirements before the game can scale its character roster in dense rescue spaces.

## Implemented Runtime Policy

Added `CodeRescueAnimationBudget.h/.cpp` with a shared `CodeRescueAnimationBudget::ApplySkeletalMeshBudget` helper. The helper configures each skeletal mesh with fixed skeletal bounds, a modest bounds scale, `VisibilityBasedAnimTickOption`, `bEnableUpdateRateOptimizations`, tick interval policy, and audit tags on both the component and owning actor.

The current profiles are:

- `PlayerBody`: full first/third-person responsiveness with `AlwaysTickPoseAndRefreshBones` and no URO.
- `FirstPersonArms`: full first-person responsiveness with enlarged bounds for close camera framing and no URO.
- `HeroNPC`: survivor, friendly NPC, and companion presentation with `OnlyTickMontagesWhenNotRendered` plus update-rate optimization.
- `CrowdZombie`: zombie crowd presentation with `OnlyTickPoseWhenRendered`, update-rate optimization, and a 30 Hz component tick interval cap.

## Actor Integration

The budget helper is now applied in the constructors of:

- `ACodeRescueCharacter` for the inherited player body mesh and the new `FirstPersonArmsMesh`.
- `ACodeZombieActor` for the inherited zombie skeletal mesh, including imported Fab zombie variants.
- `ASurvivorActor` for the professional survivor skeletal body.
- `AFriendlyNPCActor` for role-based friendly NPC skeletal bodies.
- `ACompanionActor` for rescued companion and rescue-team support actors.

Each path adds `AnimationBudget_Runtime`, `CharacterAnimationDeepDive`, and a profile-specific tag such as `AnimationBudget_CrowdZombie` for later profiling, screenshots, and static review.

## Why This Matters

The project is moving from primitive body shapes toward skeletal actors that can run AnimBPs, montages, ragdoll, and future IK/Control Rig content. Without a budget policy, every spawned character risks paying full animation cost even when hidden, distant, or part of a crowd. This slice gives the game a production-style runtime default before more animation assets are authored.

## Verification

Added `Scripts/verify_animation_budget_runtime_slice_pass.py` and wired it into both:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the shared helper, all four profiles, URO and visibility tick options, component/actor tags, actor integration points, documentation, progress logging, and QA wiring.

## Boundaries

This is not a final Control Rig, IK Rig, IK Retargeter, motion matching, facial animation, or authored LOD asset pass. It establishes the runtime budget surface those later assets need to land safely. The remaining character-animation roadmap still includes final first-person arms clips, retargeted zombie locomotion and attack sets, Control Rig cinematics, IK foot placement, facial animation decisions, and authored LOD chains for every imported character pack.
