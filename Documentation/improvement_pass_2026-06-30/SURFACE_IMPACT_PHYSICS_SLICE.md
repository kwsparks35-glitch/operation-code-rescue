# Surface Impact Physics Slice

## Purpose

This pass continues the creative physics/world backlog by making thrown utility items react differently when they hit different surfaces. The goal is to make the current procedural training yard feel less generic now, while leaving a clean path for later authored physical-material assets.

## Guidance Used

- `GAME_PHYSICS_DEEPDIVE`: define a compact surface set early, read `Hit.PhysMaterial` when available, and key impact feedback off Concrete, Metal, Wood, Glass, Flesh, and Dirt.
- `WORLD_DEVELOPMENT_DEEPDIVE`: make playable spaces communicate cause/effect through readable environmental response.
- Previous throwable physics slice: utility throwables already launch with Chaos impulses and pulse nearby physics props.

## Implementation

`Config/DefaultEngine.ini` now defines the compact physical-surface set:

- Concrete
- Metal
- Wood
- Glass
- Flesh
- Dirt

`AThrowableActor` now subscribes to `OnComponentHit`, asks moving collision to return physical materials, and routes impacts through `OnThrowableImpact`. The resolver prefers `Hit.PhysMaterial` via `UGameplayStatics::GetSurfaceType`, then falls back to authored actor/component tags such as `SurfaceConcrete`, `SurfaceMetal`, and `SurfaceWood`.

Each resolved surface receives a distinct readable response:

- surface-colored glow flash,
- impact-speed-gated feedback so resting contacts do not spam,
- cooldown throttling,
- surface-scaled impulse against hit physics props,
- bounce impulse back onto the throwable,
- QA tags such as `SurfaceImpactFeedback`, `PhysicalMaterialSurfaceReaction`, and `SurfaceImpact_Metal`,
- a `CodeRescueSurfaceImpact` runtime log line.

## World Integration

`SpawnPhysicsTraversalYard` now tags the existing floor, ramps, cover, and movable throwable targets as surface-impact training objects. The throwable physics lane now includes a readable `SURFACE IMPACT RANGE` label explaining that concrete, metal, and wood react differently. This makes the feature testable in the current generated city without requiring new imported assets.

## Verification

Added `Scripts/verify_surface_impact_physics_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the physical-surface config, throwable impact event subscription, surface resolver, per-surface helper functions, impact impulse/glow/tags/logging, in-world training props, documentation, and QA wiring.

## Remaining Work

The current slice uses code-level light/impulse feedback and authored tags, not final Niagara decals or sound cues. Future art/audio passes should create actual `UPhysicalMaterial` assets for imported meshes, add surface-specific SFX/VFX, use complex traces for skeletal/PhysicsAsset surface reads where needed, and extend the same surface response table to bullets, melee, vehicles, and destructible cover.
