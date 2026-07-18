# Procedural Secondary-Motion Signal Slice

This pass continues the June 25 creative-development backlog by adding visible, runtime secondary motion to the generated rescue world. The `CHARACTER_ANIMATION_DEEPDIVE` calls out cloth, hair, and dangling gear as grounding details for characters and cinematics, while the `WORLD_DEVELOPMENT_DEEPDIVE` asks for authored hero beats, stronger wayfinding, and less static procedural space. This slice creates a cook-safe bridge: procedural cloth-ready rescue signals that move in the wind today and can later be replaced by Chaos Cloth or groom-driven authored assets.

## Implemented Actor

Added `ASecondaryMotionSignalActor` with a small procedural rig:

- mast
- crossbar
- two cloth/banner panels
- dangling cable

The actor ticks at runtime and uses sinusoidal wind motion to rotate the banner panels and cable independently. It exposes `WindAmplitudeDegrees`, `WindSpeed`, `FlutterPhase`, `SignalTint`, and `ConfigureSignal` so city layers can tune color and motion per landmark.

The actor is tagged:

- `SecondaryMotionSignal`
- `ProceduralClothFallback`
- `ChaosClothReadyFallback`
- `CharacterAnimationDeepDive`
- `WorldDevelopmentDeepDive`

## World Integration

Added `SpawnSecondaryMotionSignalLayer` to `ACodeRescueGameMode` and call it from `SpawnCampaignCity` after the helipad and jeep are created. Each loaded campaign city now gets moving rescue signals at:

- the coding safehouse
- the evac helipad
- the route staging point
- the active survivor camp / survivor relief camp, when the survivor is not already rescued

The survivor-camp signal is also registered as a survivor helper actor, so it disappears with the survivor-specific helper cluster after rescue. All signals are also registered through `RegisterStreamedActor`, so city streaming cleanup removes them normally.

## Why This Matters

This gives the game a small but visible layer of motion beyond characters, vehicles, physics props, and lights. The player now sees moving rescue cloth/cable signals at major wayfinding points, which makes the generated city feel less frozen and supports the survival-rescue identity without depending on new imported cloth assets.

## Verification

Added `Scripts/verify_secondary_motion_signal_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the actor rig, runtime tick motion, tuning API, cloth-ready tags, city-spawn integration, landmark tags, streamed cleanup, survivor-helper cleanup, documentation, progress logging, and QA wiring.

## Boundaries

This is not a final Chaos Cloth asset pass. It does not author cloth simulations, character hair, grooms, Control Rig secondary motion, or cinematic cloth interaction. It provides a playable, visible, reviewable placeholder that establishes where and why those final assets should land.
