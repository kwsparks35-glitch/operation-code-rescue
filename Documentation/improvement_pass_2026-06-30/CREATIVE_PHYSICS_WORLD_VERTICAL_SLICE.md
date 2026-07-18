# Creative Physics/World Vertical Slice - 2026-06-30

## Purpose

This pass begins the full creative development backlog requested after the launch-language and terminal-language work. It implements a focused, playable vertical slice from the June 25 documentation set instead of merely adding planning notes: player throwables now launch with real physics, emit kind-specific utility pulses, interact with movable props, and are introduced through safehouse/world training spaces.

## Source Guidance

- `GAME_PHYSICS_DEEPDIVE C21-C23`: establish collision/physics foundations, make throwables real physics objects, and expose projectile/impulse interaction targets.
- `WORLD_DEVELOPMENT_DEEPDIVE`: move beyond invisible systems by creating readable authored gameplay spaces, safehouse storytelling, wayfinding, and environmental teaching.
- `TOP_50_RECOMMENDATIONS`: prioritize high-impact gameplay readability and a small playable fidelity slice before attempting broad asset replacement.

## Implemented Systems

### Throwable Physics

- `AThrowableActor` now exposes tunable physics properties: launch impulse, upward boost, utility pulse delay/radius/strength, and flare/smoke pulse damage.
- `ACodeRescueCharacter::ThrowActive` now uses `SpawnActorDeferred<AThrowableActor>` so the selected kind is assigned before `BeginPlay`. This fixes the prior smoke/stim issue where `BeginPlay` could process them as default flares.
- The throw path consumes flare/smoke/stim inventory only after the deferred actor is successfully spawned.
- `LaunchThrowable` applies real linear/angular impulse to the simulated mesh body and logs through `[CodeRescuePhysicsVerticalSlice]`.
- Flare and smoke now fire a delayed utility pulse:
  - overlaps nearby dynamic/physics bodies,
  - applies `AddRadialImpulse` with linear falloff,
  - nudges nearby zombies with `LaunchCharacter`,
  - applies small torso damage through `ACodeZombieActor::ApplyRescueDamage`.
- Existing identity remains intact: flare still registers in `StaticActiveLures`; smoke remains lower-light gray utility; stim still restores stamina/health after its throw arc.

### World/Training Integration

- `SpawnPhysicsTraversalYard` now includes a tagged `THROWABLE PHYSICS LANE`.
- The lane spawns seven movable `ThrowablePhysicsTarget` props with `PhysicsActor` collision, simulated physics, mass overrides, damping, and review tags:
  - `ThrowablePhysicsTarget`
  - `PhysicsDeepDiveC23`
  - `RadialImpulseTrainingProp`
- The safehouse now includes a visible utility bench with flare/smoke/stim props and instructional text: `X SLOT: flare lures, smoke staggers, stim restores`.
- These additions keep the start-screen/language-selection flow untouched and appear only inside active gameplay cities.

### Physics/Collision Foundation

- `Config/DefaultEngine.ini` now enables Chaos substepping:
  - `bSubstepping=True`
  - `MaxSubstepDeltaTime=0.016667`
  - `MaxSubsteps=6`
- The config now reserves named channels for the broader C21 collision plan:
  - `PlayerPawn`
  - `ZombiePawn`
  - `CoverObject`
  - `PickupObject`
  - `WeaponTrace`
  - `AISightTrace`
  - `InteractionTrace`

## Regression Coverage

- Added `Scripts/verify_creative_physics_world_slice_pass.py`.
- Wired the verifier into:
  - `Run_Full_QA_Audit.command`
  - `Run_Local_CI_Readiness.command`
- The verifier checks the deferred throwable spawn path, launch impulse API, radial pulse implementation, zombie pulse integration, physics-yard targets, safehouse utility bench, config physics/collision settings, documentation, and progress-log entry.

## Files Changed

- `Source/CodeRescueUnreal/ThrowableActor.h`
- `Source/CodeRescueUnreal/ThrowableActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Config/DefaultEngine.ini`
- `Scripts/verify_creative_physics_world_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Remaining Deep-Dive Work

This slice does not claim completion of every attached-document inclusion. Remaining major work from the deep dives still includes ragdolls, Chaos destruction, vehicle physics promotion, physical materials/surface reactions, authored-map or PCG replacement workflows, skeletal character/AnimBP asset wiring, animation montages, IK/Control Rig, broader AI encounter direction, and final release polish. The practical next slice should build on this one by adding surface-specific throwable impact feedback, ragdoll-on-death for zombies, and a small authored combat encounter that uses the new physics lane props.
