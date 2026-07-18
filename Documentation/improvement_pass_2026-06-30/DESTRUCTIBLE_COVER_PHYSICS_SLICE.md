# Destructible Cover Physics Slice

Date: 2026-06-30

## Purpose

This slice implements a playable destruction/readability layer from the June 25 guidance, especially the `GAME_PHYSICS_DEEPDIVE`, `WORLD_DEVELOPMENT_DEEPDIVE`, and `CHARACTER_ANIMATION_DEEPDIVE` recommendations around Chaos-driven feedback, physical cover, authored encounter spaces, and readable zombie attacks.

## What changed

- `ABarricadeActor` is now real destructible cover instead of a temporary blocking cube with unused health.
- Barricades expose `TakeBarricadeDamage`, accept standard Unreal `TakeDamage`, react to physics impacts, tint through healthy/cracked/critical damage states, and break into short-lived simulated debris chunks.
- Debris chunks use the existing cube mesh fallback, `PhysicsActor` collision, Chaos simulation, impact/angular impulses, wood surface tags, and explicit documentation tags such as `ChaosDestructionFallback`, `ChaosReadableDestruction`, and `DestructibleCoverDebris`.
- Player firearms, melee, and area-effect weapons now damage barricades.
- Throwables now damage barricades on direct collision and through utility pulses, linking this work to the surface-impact and throwable physics slices.
- Zombies now trace toward the player; if a barricade blocks the route and the zombie is close enough, it plays the existing attack readability cue and attacks the barricade instead of passively pathing around it.
- The authored physics-lane encounter now includes a visible destructible-cover drill with three long-lived breakable wooden barricades.

## Player-facing behavior

- Press `B` to place a barricade, then allow zombies to reach it or damage it with shots, melee, or throwables.
- In the physics ambush drill, the player can shoot barricades, throw utility items into them, or kite infected into them to see the new breakage loop.
- Damaged barricades become visually warmer and brighter as health falls, then burst into physical wood chunks when destroyed.

## Validation

The slice is covered by `Scripts/verify_destructible_cover_physics_slice_pass.py`, which checks:

- destructible cover health/damage/debris hooks;
- player melee, firearm, and area-effect integration;
- throwable impact and pulse integration;
- zombie barricade attack routing;
- authored physics-lane training placement;
- QA script wiring and this documentation.

Run:

```zsh
python3 Scripts/verify_destructible_cover_physics_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
```
