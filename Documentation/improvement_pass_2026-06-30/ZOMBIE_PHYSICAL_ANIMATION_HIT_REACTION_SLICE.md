# Zombie Physical Animation Hit Reaction Slice

Date: 2026-06-30

## Source Guidance

This slice continues the `GAME_PHYSICS_DEEPDIVE` Phase 2 recommendation to use `UPhysicalAnimationComponent` for living zombie hit flinches, while preserving the `CHARACTER_ANIMATION_DEEPDIVE` guidance that authored montages and additive readability should remain the animation baseline.

## Implementation Summary

`ACodeZombieActor` now owns a `ZombiePhysicalHitReaction` component and exposes a small tuning surface:

- `bEnablePhysicalHitReaction`
- `PhysicalHitReactionRootBone`
- `PhysicalHitReactionBlendWeight`
- `PhysicalHitReactionDuration`
- `PhysicalHitReactionImpulseStrength`

At spawn time the component is bound and tagged, but the physical-animation drive is not applied yet. When a nonfatal hit lands on a promoted skeletal zombie with a valid `PhysicsAsset` body root, `TriggerPhysicalAnimationHitReaction` applies the physical-animation drive, enables short-lived simulation below the resolved root body, applies an impulse at a head/body/limb-aware impact body, and tags the actor with `ZombiePhysicalAnimationHitReaction`, `PhysicalAnimationHitReactionRuntime`, and `HitReactionPhysicsBlend`.

## Safety Boundaries

This pass does not replace the existing readable fallback stack. `ApplyHitReadabilityImpulse` still launches the character capsule slightly, flashes the infection glow, drives `TriggerHitReactionMotionCue`, and plays `HitReactMontage` when available. If a skeletal mesh, PhysicsAsset, or matching PhysicsAsset body is missing, the actor keeps the existing `PhysicalHitReactionFallback` path instead of attempting physical animation.

`UpdatePhysicalAnimationHitReaction` fades `SetStrengthMultiplyer` and `SetAllBodiesBelowPhysicsBlendWeight` back to zero, disables temporary body simulation, and returns the living skeletal mesh to no-collision. `DisableGameplayCollisionForDeath` resets any active physical hit reaction before death ragdoll or primitive corpse physics takes over, so the death-physics slice remains authoritative for corpses.

## Review Artifacts

Added `Content/CodeRescueData/zombie_physical_animation_hit_reaction_manifest.tsv` and updated the creative inclusion plan, enemy readability manifest, physics promotion contract, human QA checklist, and visual regression targets.

## Verification

Added `Scripts/verify_zombie_physical_animation_hit_reaction_slice_pass.py` and wired it into:

- `Run_Local_CI_Readiness.command`
- `Run_Full_QA_Audit.command`

Expected validation:

- Python verifier compilation
- `python3 Scripts/verify_zombie_physical_animation_hit_reaction_slice_pass.py`
- adjacent zombie death physics, zombie motion readability, and physics promotion verifiers
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Manual QA

Fight a promoted skeletal zombie outside the protected learning zone and land a nonfatal body shot, headshot, and limb shot. The zombie should keep pursuing through the capsule movement path while the skeletal body briefly reacts through a blended physical hit cue. Then kill the zombie and confirm the existing ragdoll or primitive corpse physics path still owns the death response and the selected-language save still records the neutralized zombie.
