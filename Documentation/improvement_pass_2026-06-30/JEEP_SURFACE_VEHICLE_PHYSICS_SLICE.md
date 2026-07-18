# Jeep Surface-Aware Vehicle Physics Slice

Date: 2026-06-30

## Purpose

This slice advances the `GAME_PHYSICS_DEEPDIVE` vehicle recommendation without pretending the project already has a skeletal jeep, wheel bones, wheel blueprints, or a vehicle animation blueprint. The full commercial path remains `ChaosVehiclesPlugin` plus `UChaosWheeledVehicleMovementComponent`; this pass makes the current `UFloatingPawnMovement` jeep a safer, more tactile fallback while those assets are absent.

## What changed

- `AJeepActor` now probes the ground below the vehicle on a short interval and resolves the current physical surface from `Hit.PhysMaterial` or authored surface tags.
- The jeep adjusts max speed, acceleration, braking, turn rate, and lateral drift damping from the resolved surface.
- Concrete keeps full grip; metal/wood reduce turn response; dirt lowers speed and traction; glass/flesh/default surfaces have distinct fallback tuning.
- A small underbody `SurfaceCueLight` shifts color and intensity based on the current surface/speed so the player gets immediate feedback that traction is changing.
- Jeep actors are tagged `VehiclePhysicsFallback`, `ChaosVehicleReadyFallback`, `SurfaceAwareVehicle`, and `GamePhysicsDeepDive`.
- Each spawned staff jeep now gets an authored concrete traction pad and an updated in-world label: `surface-aware traction active`.

## Player-facing behavior

- Mount the jeep with `E` and drive with WASD.
- When the jeep crosses tagged or physical-material surfaces, speed, turn response, and lateral drift change instead of remaining a perfectly floating box.
- The fallback is still intentionally arcade-stable on macOS: it does not require a skeletal vehicle asset, wheel setup, or Chaos vehicle animation graph.

## Remaining full Chaos Vehicles work

To complete the deep-dive's full §8 target, the project still needs:

- a skeletal jeep mesh with correctly named wheel bones;
- a Physics Asset for the jeep body/wheels;
- wheel blueprints parented to `ChaosVehicleWheel`;
- a `WheeledVehiclePawn` / `UChaosWheeledVehicleMovementComponent` implementation;
- an engine torque curve;
- a vehicle Animation Blueprint parented to `VehicleAnimationInstance`;
- macOS substep validation to avoid slope jitter.

This slice keeps those future requirements explicit while improving current playability now.

## Validation

Run:

```zsh
python3 Scripts/verify_jeep_surface_vehicle_physics_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
```
