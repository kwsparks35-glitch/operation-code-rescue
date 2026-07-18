# Zombie Death Physics And Hit Readability Slice

## Purpose

This pass continues the June 25 creative-development backlog by making zombie combat read as animation plus physics instead of animation-only cleanup. The player now gets immediate nonfatal hit feedback, and a neutralized zombie leaves a short-lived physical body response that can be verified in ordinary play.

## Guidance Used

- `GAME_PHYSICS_DEEPDIVE`: keep living characters on `CharacterMovementComponent`, but use ragdoll, PhysicsAssets, collision profiles, impulses, and controlled corpse lifetimes for death readability.
- `CHARACTER_ANIMATION_DEEPDIVE`: preserve hit-react and death montages while allowing ragdoll to take over when the skeletal mesh has a valid physics setup.
- `WORLD_DEVELOPMENT_DEEPDIVE`: make combat spaces and rescue routes feel authored through readable cause/effect, especially when the player clears threats near objectives.

## Implementation

`ACodeZombieActor` now exposes tunable combat-readability and death-physics settings:

- `HitReactionImpulseStrength`
- `bEnableDeathRagdoll`
- `bEnablePrimitiveCorpsePhysics`
- `RagdollImpulseStrength`
- `PrimitiveCorpseImpulseStrength`
- `RagdollCorpseLifetime`

Nonfatal hits call `ApplyHitReadabilityImpulse`, which gives the zombie a small direction-aware nudge and temporarily brightens the infection glow. The existing hit-react montage remains active, so professional asset packs still contribute authored animation while the fallback path remains playable and visible.

Deaths now call `TryActivateDeathRagdoll` first. When a professional skeletal zombie has a `PhysicsAsset`, the actor disables gameplay collision, detaches the mesh, switches to the `Ragdoll` collision profile, enables body simulation, wakes the rigid bodies, and applies a hit-zone-weighted impulse. Active ragdolls are capped through `CodeRescueMaxActiveRagdollCorpses` so the feature stays practical in larger city encounters.

If a skeletal ragdoll is unavailable, `ActivatePrimitiveDeathPhysics` detaches the cube body and sphere head, switches them to the `PhysicsActor` profile, enables simulation, and applies linear and angular impulses. This keeps the procedural fallback enemies from disappearing without physical feedback and gives QA a reliable way to verify the slice even before every art asset has a PhysicsAsset.

## Save And Encounter Safety

The neutralization contract remains unchanged. `ApplyRescueDamage` still destroys the objective marker, calls `MarkZombieNeutralized`, and immediately saves through `SavePersistentRun` before the corpse lifetime begins. Elite boomer death behavior still fires before generic cleanup, and the original death montage remains as the fallback path whenever physics is not activated.

## Verification

Added `Scripts/verify_zombie_death_physics_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the new tuning properties, hit feedback, ragdoll activation, primitive fallback physics, save/marker preservation, boomer preservation, montage fallback, documentation, and QA wiring.

## Remaining Work

This slice establishes the runtime path, but it does not author new PhysicsAssets, Control Rigs, or AnimBPs. Future art passes should assign PhysicsAssets to every imported zombie, tune per-variant physical-material responses, add authored hit/death montages for each variant, and extend this readable combat feedback into survivor-defense and boss encounters.
