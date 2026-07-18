# Character, World, and Mission Demo Pass - 2026-05-20

## Goal

Replace the remaining blocky/ambiguous reads in the generated runtime city with
more recognizable people, textured environmental detail, and explicit in-world
mission direction.

This pass uses only locally available project assets. I did not need an Epic
account login for these changes because the mannequin, StarterContent,
building, bridge, groom, and zombie assets are already present under
`Content/`.

## Character Development

- Friendly NPCs now auto-load real local mannequin meshes by role:
  - Engineer and Trader use `SKM_Manny` plus `ABP_Manny`.
  - Medic and Scientist use `SKM_Quinn` plus `ABP_Quinn`.
- Friendly NPCs now also carry role-specific props/badges so the player can
  tell at a glance who repairs, heals, researches, or trades.
- Decorative Manny/Quinn civilians now appear in classroom/debug-lab mission
  spaces as non-interactive world storytelling figures.
- Decorative civilians now carry recognition badges, shoulder sashes, presence
  halos, and optional name/role labels.
- Every generated city now has a Civilian Cast court near the entry route with
  three named characters: Civic Guide, Signal Scout, and Rescue Liaison.
- Survivor teams still use the local Quinn mesh/AnimBP fallback, but their
  mesh transform was corrected so the character sits at ground level instead
  of hovering above the rescue marker.
- If any professional mesh is missing, the previous cube/sphere body still
  remains as a fallback so the city can load safely.

Files:

- `Source/CodeRescueUnreal/FriendlyNPCActor.h`
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp`
- `Source/CodeRescueUnreal/SurvivorActor.cpp`

## World Environment Development

- Added `SpawnCityLandscapeDetails(...)` to the runtime city generator.
- Every streamed city now gets:
  - textured terrain underlay from StarterContent materials,
  - concrete/cobblestone evac roads,
  - waterline material for coastal/lake/river cities,
  - deterministic rock clusters,
  - deterministic bush clusters outside desert cities,
  - route lamps and a small civilian rest-stop prop.
- Every city also receives a concrete civilian support hub around the four
  friendly NPCs with workstations, canopy props, light strips, and role signage.
- Every generated city now receives a Fab/Vault content bay that showcases the
  locally imported ModernBridges span and Parallax Night Building meshes already
  present in the project.
- Every generated city now receives mission-space dioramas:
  - a Field Classroom near language selection,
  - a Debug Field Lab around the coding terminal,
  - a Quarantine Line before the optional warden fight.
- Every generated city now has colored objective viewframes with pillars,
  header beams, suspended lamps, and compact route labels to focus player
  attention through the mission path.
- The Field Classroom now has window-wall framing; the Debug Field Lab now has
  observation glass.
- Unrescued survivor teams now get a relief camp with briefing table, chairs,
  supply shelf, cot, medical cross, and a civilian profile sign tied to the
  required lesson.
- The new environmental layer sits on top of the existing large safety floor,
  Parallax building meshes, ModernBridges spans, set-pieces, post-process, fog,
  and day/night lighting.

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## Mission Objective Development

- Added `SpawnMissionObjectiveRoute(...)` to make the mission loop readable in
  the actual play space.
- Every city now has numbered pads and beacons:
  - `0 START` at the entry pad
  - `1 SELECT LANGUAGE`
  - `2 SOLVE TERMINAL`
  - `3 RESCUE TEAM`
  - `4 OPTIONAL` boss/warden fight
- Pads are connected by textured route strips using the StarterContent tech
  panel material, so players can follow the objective chain without guessing.
- A mission board is placed near the entry corridor with the current terminal
  title and the main sequence.

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

## Demonstration

Double-click:

```text
Run_Character_World_Demo.command
```

Then review:

- At spawn: the numbered start pad and mission board.
- Near the start route: inspect the Civilian Cast court and its named guide,
  scout, and rescue liaison.
- Press `T`: jump along objectives and inspect the language pad, terminal pad,
  survivor pad, and optional boss pad.
- At the language objective: inspect the Field Classroom and mannequin
  learners.
- At the terminal objective: inspect the Debug Field Lab, server racks, screens,
  cable runs, and analyst.
- At the language plaza: four named NPCs should render as Manny/Quinn-style
  people instead of block figures, with role props and workstation kiosks.
- At the survivor objective: inspect the relief camp and civilian profile sign.
- On the east side of the city: inspect the Fab/Vault content bay with the
  authored bridge span, parallax towers, intake crates, and MetaHuman-ready pad.
- Around the city: look for textured terrain, evac roads, waterlines in
  coastal/lake/river cities, rocks, bushes, lamps, building meshes, bridges,
  and set-piece landmarks.
- Press `F12`: saves a screenshot to the project's `Saved/Screenshots/`
  folder.

## Verification

Updated:

```text
Scripts/verify_character_world_assets.py
```

The verification script now checks the new mannequin, animation, terrain,
road, objective-pad, rock, and bush dependencies in addition to the existing
zombie/building/bridge assets.

Latest local validation:

- `CodeRescueUnrealEditor Mac Development`: build succeeded.
- `Scripts/verify_character_world_assets.py`: succeeded with 0 errors.
- Headless `-game -NullRHI` launch smoke: exited cleanly with code 0.
- Remaining warning: optional `SM_postapo_bridge_001` package path is not
  exposed by the asset registry; runtime bridge selection already falls back to
  the available modern bridge meshes.
