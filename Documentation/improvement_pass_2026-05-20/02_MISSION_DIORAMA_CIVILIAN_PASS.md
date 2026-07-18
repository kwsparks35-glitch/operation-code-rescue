# Mission Diorama and Civilian Presence Pass - 2026-05-20

## Goal

Continue replacing ambiguous generated space with readable mission staging:
places that look like people use them, teach in them, debug in them, and defend
them.

## Character/People Work Completed

- Added `SpawnDecorativeCivilian(...)` to the runtime city generator.
- The helper spawns non-interactive Manny/Quinn mannequin civilians with the
  same local skeletal meshes and animation Blueprints used by the friendly NPCs.
- Decorative civilians are deliberately not rescue actors and do not touch save
  state. They are world storytelling figures only.
- Each decorative civilian gets a small colored halo so they can be found while
  reviewing the generated scene.

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## World/Mission Work Completed

- Added `SpawnMissionDioramas(...)`.
- Every generated city now includes three mission-space dioramas along the
  existing objective route:
  - **Field Classroom** beside the language stations, with lesson board,
    benches, and civilian learners.
  - **Debug Field Lab** around the coding terminal, with server racks, screens,
    cable runs, a table, and an analyst civilian.
  - **Quarantine Line** before the optional warden/boss objective, with metal
    floor, gate frame, watch towers, barricades, and warning lights.
- These dioramas sit on top of the existing numbered route pads rather than
  replacing them, so the mission path remains easy to follow.

## Review Notes

Open:

```text
Run_Character_World_Demo.command
```

Then press `T` to jump along the objective route:

- At `1 SELECT LANGUAGE`, review the Field Classroom.
- At `2 SOLVE TERMINAL`, review the Debug Field Lab.
- At `4 OPTIONAL`, review the Quarantine Line and warden staging.

## Verification

Updated:

```text
Scripts/verify_character_world_assets.py
```

The verifier now checks the additional diorama dependencies:

- `SM_DoorFrame`
- `SM_PillarFrame`
- `M_Wood_Floor_Walnut_Worn`
- `M_Tech_Checker_Dot`
- `M_Metal_Rust`

Latest validation after this pass:

- `CodeRescueUnrealEditor Mac Development`: build succeeded.
- `Scripts/verify_character_world_assets.py`: succeeded with 0 errors.
- Headless `-game -NullRHI` launch smoke: exited cleanly with code 0.
- Remaining warning: optional `SM_postapo_bridge_001` is not visible to the
  asset registry; runtime bridge selection already falls back to available
  ModernBridges meshes.
