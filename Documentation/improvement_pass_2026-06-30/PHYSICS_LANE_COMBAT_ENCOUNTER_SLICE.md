# Physics Lane Combat Encounter Slice

## Purpose

This pass turns the physics traversal yard from a training set into a small authored combat pocket. It ties together the recent throwable impulse work, surface-impact feedback, zombie hit/death readability, and world-development guidance around readable encounter spaces.

## Guidance Used

- `GAME_PHYSICS_DEEPDIVE`: make physics props part of combat, use stable collision profiles, keep simulated props constrained to intentional spaces, and verify that impulses are meaningful in play.
- `WORLD_DEVELOPMENT_DEEPDIVE`: vary cover, escape routes, reward placement, and authored-feeling objective pockets instead of relying only on procedural city spawns.
- `CHARACTER_ANIMATION_DEEPDIVE`: make zombie combat readable through hit feedback, death readability, and visible threat markers while later asset-specific animation work continues.

## Implementation

`SpawnPhysicsTraversalYard` now creates a `PHYSICS AMBUSH DRILL` pocket just beyond the throwable physics lane. The encounter includes:

- five simulated impact props tagged `PhysicsLaneCombatProp`, `ThrowableImpactCoverProp`, and `SurfaceImpactCombatTraining`,
- concrete/metal/wood surface tags so the new throwable surface resolver is visible in the same space,
- three readable cover blocks tagged as part of the encounter,
- a smoke cache, flare cache, and ammo cache to teach the intended `X slot` utility loop,
- three low-intensity zombies tagged `PhysicsLaneCombatZombie` and `ZombieDeathPhysicsReadabilityTarget`.

The encounter actors are tagged `PhysicsLaneCombatEncounter`, `AuthoredCombatEncounter`, and `UsesThrowablePhysicsLane` for future review, editor capture, or automated checks.

## Save And Sandbox Behavior

The enemy wave is gated behind `!bSandboxMode`, preserving sandbox practice as a noncombat training space. Each encounter zombie receives a stable save ID under `CodeRescueHordeZombieIdBase + CityIndex * 1000 + 650 + i`, checks `NeutralizedZombieIds` before spawning, records its variant, and therefore stays neutralized after the player clears it and saves.

## Verification

Added `Scripts/verify_physics_lane_combat_encounter_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks encounter tags, simulated combat props, surface tags, utility pickups, sandbox gating, stable save IDs, variant initialization, zombie markers, documentation, and QA wiring.

## Remaining Work

This is an authored procedural pocket, not a fully art-authored arena. Future passes should replace the primitive cover and props with final modular meshes, add encounter-specific audio stingers, tune zombie counts through human playtest, and connect the encounter to survivor defense or extraction objectives.
