# Fixed Timestep Physics Contract Slice

Date: 2026-06-30

## Source Guidance

- `GAME_PHYSICS_DEEPDIVE.md`: recommends synchronous substepping first, a fixed timestep foundation, and deliberate async-physics validation only after the stable path is proven.
- `TOP_50_RECOMMENDATIONS_2026-06-25.md`: recommendation 25 calls for determinism and fixed tick so physics feel is frame-rate independent across Macs.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: keeps packaged Mac smoke validation and clear future-review records as release-readiness gates.

## Implementation

- Made the project physics config explicit: sync substepping is enabled at `0.016667` seconds, `MaxPhysicsDeltaTime` is capped at `0.033333`, `MaxSubsteps` remains `6`, and async physics/substepping stay off until a deliberate Mac validation pass.
- Added `CodeRescuePhysicsStability` as the shared runtime contract for gameplay physics bodies.
- The shared helper applies damping floors, custom sleep threshold/stabilization multipliers, max depenetration velocity, optional CCD, mass overrides when requested, and QA tags.
- Wired the contract into throwables, throwable pulse targets, barricade bodies, barricade debris, zombie physical hit reactions, zombie ragdolls, primitive corpse fallback parts, the Jeep fallback body, and GameMode-spawned training/systems/stress physics props.
- Added `Content/CodeRescueData/fixed_timestep_physics_contract_manifest.tsv` and updated creative inclusion, physics promotion, visual-regression, human-QA, full-QA, local-CI, and progress surfaces.

## Player Impact

- Throwables and radial impulses have a more stable substep budget, which helps flare/smoke/stim arcs and impact nudges feel consistent at different frame rates.
- Zombie hit reactions and death physics now share a common solver-facing stability profile instead of depending on whichever damping settings a mesh happened to inherit.
- Destructible cover debris still retires quickly, but the live debris window is less likely to jitter or tunnel through route geometry.
- The Jeep remains a fallback pawn until Chaos Vehicles promotion, but its body is now explicitly marked for fixed-step review.

## Files Changed

- `Config/DefaultEngine.ini`
- `Source/CodeRescueUnreal/CodeRescuePhysicsStability.h`
- `Source/CodeRescueUnreal/CodeRescuePhysicsStability.cpp`
- `Source/CodeRescueUnreal/ThrowableActor.cpp`
- `Source/CodeRescueUnreal/BarricadeActor.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
- `Source/CodeRescueUnreal/JeepActor.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Content/CodeRescueData/fixed_timestep_physics_contract_manifest.tsv`
- `Scripts/verify_fixed_timestep_physics_contract_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Validation

- Static verifier: `python3 Scripts/verify_fixed_timestep_physics_contract_slice_pass.py`
- Adjacent verifiers: collision channel gameplay contract, creative physics world, physics promotion contract, throwable surface impact, destructible cover, zombie death physics, zombie physical-animation hit reaction, Jeep surface-aware vehicle physics, and runtime step smoke contracts.
- Build/package/smoke should be run because this touches runtime physics component setup and project physics settings.

## Human QA Notes

- Throw flares and smoke at physics targets and barricades in the physics yard; impacts should nudge props without long jitter or tunneling.
- Kill zombies with body/head/limb hits and confirm promoted ragdoll or fallback corpse motion still reads while caps and corpse retirement remain intact.
- Break barricades in the physics ambush and confirm debris settles or disables without trapping the player route.
- Drive the Jeep fallback over marked surfaces and confirm the existing surface-aware cues still function while the actor carries the fixed-step review tag.
