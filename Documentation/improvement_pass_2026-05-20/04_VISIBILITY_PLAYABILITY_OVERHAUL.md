# Improvement Pass 2026-05-20 — Visibility & Playability Overhaul

Author: Claude (Cowork session for Kenny)
Date: 2026-05-20
Build target verified: `CodeRescueUnrealEditor Mac Development`

## Why this pass exists

Before adding anything new, this session did two things prior passes had
deferred: (1) ran a **real Mac compile** of the whole 68-item codebase, and
(2) launched the **actual playable game** and looked at it.

Results of that verification:

- **Compile: PASS.** `Recompile_Module.command` reported `Result: Succeeded`
  / `BUILD SUCCEEDED`. All 85 source files / ~15,700 lines compile and link
  cleanly on the Mac. The 68 coded features are real and buildable.
- **Run: PASS.** `Run_Character_World_Demo.command` launched the game; it
  generates its world, renders, and responds to keyboard/mouse input.
- **Problem found:** the running game was **punishingly dark**, and the
  engine printed an on-screen warning: *"Multiple directional lights are
  competing to be the single one used for forward shading..."* The world,
  characters, and mission route were all but invisible to a player.

That darkness — not a missing feature — was the single biggest thing
standing between this project and being playable. This pass fixes it.

## The 10 improvements in this pass

### World & landscapes

**W1 — One sun instead of two.** `BeginPlay()` spawned a directional light
for the day/night cycle, then `SpawnWorld()` spawned a *second* directional
light for the sky atmosphere. Two suns → the engine's "multiple directional
lights competing" warning, and split control (day/night dimmed one light
while the atmosphere followed the other). `SpawnWorld()` now adopts the
existing `SunLight` and tags it as the atmosphere sun. **Verified in the
live game: the warning is gone.**

**W2 — Night is now playable.** The day/night cycle dimmed the sun to `0.3`
intensity at night — effectively black. Night is now a moonlit dusk: `3.2`
intensity with a cool-blue key light; day stays a warm white `7.0`. This is
an educational game — students must always be able to read the world.

**W3 — Brighter ambient floor.** `SkyLight` intensity raised `1.6 → 3.0` so
interiors, undersides, and the night city don't crush to black.

**W4 — Auto-exposure that brightens dark areas.** Post-process
`AutoExposureMinBrightness` lowered `0.4 → 0.03` (lets the camera adapt up
in dark areas instead of clamping them black), `AutoExposureBias` raised
`0.5 → 1.0`, `MaxBrightness` `2.0 → 2.6`.

**W5 — Opens in daylight.** `TimeOfDay` default changed `0.25 → 0.12` so a
fresh run starts in bright mid-morning instead of racing into night within
the first minute.

### Mission objectives

**M1 — Objective beacon lights.** Each of the five objective stops (START,
SELECT LANGUAGE, SOLVE TERMINAL, RESCUE TEAM, OPTIONAL warden) now spawns a
colored point light, so the mission route is followable in any lighting.
Lights are registered with the city streaming system so they clean up
correctly when the player moves between cities.

### Characters & NPCs

**C1 — Survivors are easier to find.** The survivor rescue beacon is
brighter (`1500 → 3000`) with a longer reach (`420 → 750`). Color stays
cyan to match the HUD crosshair and minimap color-coding.

**C2 — Friendly-NPC hubs feel like refuges.** The friendly-NPC role light
is warmer and wider (`2400 → 3200` intensity, new warm color, `800 → 950`
radius) so the Engineer/Medic/Scientist/Trader plaza reads as a welcoming
safe zone.

### Polish & feel

**P1 — Legible HUD.** The crosshair, status line, and interaction prompt
were small default-font text that disappeared against bright or busy
backgrounds. They now use larger fonts (crosshair 34pt, status 20pt,
prompt 22pt) and stronger drop shadows so the HUD reads on any background.

## Files changed

- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — W1, W2, W3, W4, M1,
  plus two new includes (`Engine/PointLight.h`, `Components/PointLightComponent.h`)
- `Source/CodeRescueUnreal/CodeRescueGameMode.h` — W5
- `Source/CodeRescueUnreal/SurvivorActor.cpp` — C1
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp` — C2
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` — P1

All changes are surgical, additive, and were **recompiled and verified to
build cleanly** (`Result: Succeeded`) after editing.

## Demo verification — what was and wasn't confirmed live

Confirmed in the running game after the rebuild:

- The project compiles and the game launches and plays. ✅
- The game responds to WASD movement and mouse-look. ✅
- **W1 is visually confirmed** — the "multiple directional lights
  competing" warning that appeared before the fix no longer appears. ✅

Not cleanly confirmed this session (honest note):

- A clean *daytime* "after" screenshot could not be captured. The editor
  game window pauses its simulation when it loses focus, so the 240-second
  day/night cycle did not advance to daytime during screenshot-based
  observation, and the player camera repeatedly ended up buried in
  geometry. W2–W5 are coded and compile-verified but their visual result
  is best confirmed by Kenny playing a fresh run (it now opens in daytime).

## Recommended follow-ups (carried into the roadmap)

1. The player camera buries itself in terrain/walls easily — capsule
   collision / camera spring-arm needs a pass (roadmap item).
2. Consider lengthening `DayNightPeriodSeconds` (240s is a very fast cycle)
   or making it a difficulty/comfort setting.
3. Investigate whether the on-screen first-run help text and HUD are both
   showing in a fresh run.
