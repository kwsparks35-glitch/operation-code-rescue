# Character Recognition and World Composition Pass - 2026-05-20

## Goal

Continue developing recognizable in-game people and more compelling world
composition. The emphasis is not cosmetic beauty for its own sake; it is
readability, attention, and immersion inside a coding-rescue survival world.

## Character Recognition Completed

- Expanded `SpawnDecorativeCivilian(...)` so mannequin civilians now carry:
  - a colored chest badge,
  - a colored shoulder sash,
  - a small presence halo,
  - an optional floating display label.
- Added `SpawnCharacterIdentityCourt(...)`.
- Every generated city now has an entry-side **Civilian Cast** court with three
  named world-story characters:
  - Civic Guide
  - Signal Scout
  - Rescue Liaison
- Names rotate deterministically by city index so the cast feels local to each
  city while remaining stable across reloads.
- Classroom and debug-lab civilians now have direct in-game labels:
  - `Nova / Learner`
  - `Kai / Learner`
  - `Dr. Vale / Analyst`

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## World Composition Completed

- Added `SpawnWorldCompositionLayer(...)`.
- Every generated city now gets objective viewframes around the main route:
  - Entry
  - Learn
  - Debug
  - Rescue
  - Warden
- Each viewframe uses colored pillars, a header beam, a banner wash, a suspended
  lamp, and a compact objective label. These create attention anchors without
  changing collision-critical gameplay.
- Field Classroom received window-wall framing.
- Debug Field Lab received observation glass.

## Review Notes

Open:

```text
Run_Character_World_Demo.command
```

Then inspect:

- Start area: **Civilian Cast** court with three labeled mannequin characters.
- Objective pads: colored viewframes and suspended lights.
- Language area: classroom with labeled learners and window framing.
- Coding terminal: debug lab with observation glass and labeled analyst.

## Verification

Updated:

```text
Scripts/verify_character_world_assets.py
```

The verifier now checks:

- `SM_Lamp_Ceiling`
- `Wall_Window_400x300`
- `SM_GlassWindow`
- `M_Concrete_Poured`

Latest validation after this pass:

- `CodeRescueUnrealEditor Mac Development`: build succeeded.
- `Scripts/verify_character_world_assets.py`: succeeded with 0 errors.
- Headless `-game -NullRHI` launch smoke: exited cleanly with code 0.
- Remaining warning: optional `SM_postapo_bridge_001` is not visible to the
  asset registry; runtime bridge selection already falls back to available
  ModernBridges meshes.
