# Boss Phase Telegraph Slice

Date: 2026-06-30

## Source Guidance

- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: called for boss animation/readability beats that can later be replaced by authored montage, Control Rig, or VFX work.
- `GAME_PHYSICS_DEEPDIVE.pdf`: emphasized readable combat state changes, fair impact windows, and nonblocking collision behavior.
- `TOP_50_RECOMMENDATIONS.pdf`: called out enemy variety, clear enemy telegraphs, boss presentation, and accessible combat polish.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: requires reviewable, package-safe work that can be audited and smoke-tested.

## Implementation

Extended `ABossZombieActor` with a runtime boss phase telegraph rig built from engine primitives:

- phase ring
- overhead phase core
- rotating warning sweep
- phase 3 add-spawn beacons
- warning point light

The rig is attached directly to the boss capsule and uses no collision or overlap events, so it does not interfere with movement, melee range, player navigation, or spawned adds.

## Gameplay Behavior

When the boss crosses into phase 2, `EnterPhase()` now starts an orange sprint/regen telegraph while preserving the existing speed increase and regeneration behavior.

When the boss crosses into phase 3, the telegraph shifts to a stronger red warning with add-spawn beacons before the existing add-spawn behavior begins filling the arena. This makes the phase escalation readable in packaged builds instead of relying only on temporary debug text.

## Authored Asset Handoff

`OnBossPhaseTelegraphStarted(int32 Phase)` is exposed as a Blueprint event so future authored animation, Niagara, sound, camera, or Control Rig work can layer onto the same phase boundary without changing the combat logic.

## Accessibility

The boss caches `UCodeRescueGameInstance::bReducedMotion` during `BeginPlay()`. The reduced motion path keeps the ring, color, light, and phase silhouettes readable while damping spin, orbit, bobbing, and pulse speed.

## Verification

Added `Scripts/verify_boss_phase_telegraph_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- telegraph tuning and component fields
- cook-safe primitive construction
- reduced-motion support
- phase 2 and phase 3 integration
- nonblocking component setup
- Blueprint handoff event
- documentation and QA wiring
