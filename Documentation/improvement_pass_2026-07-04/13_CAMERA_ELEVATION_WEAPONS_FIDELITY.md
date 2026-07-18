# 13 — Camera Containment, Elevation, Weapon Visibility, Fire FX, Fidelity (2026-07-07, pass 8)

Kenny's five-point report, addressed through SEVENTEEN compile-playtest
cycles in one session. Two long-standing ghosts were finally unmasked.

## The two unmasked ghosts

**The "cyan lattice dome"** that had been swallowing the camera around the
safehouse since pass 5 — hunted across beacons, sky domes, concept-art
layers and the armory display — was ultimately a **PERSISTENT
`DrawDebugSphere`** (radius 260, thickness 18, lifetime -1) drawn over every
terminal, sitting right beside the comment "without leaning on
DrawDebugSphere". A movement parallax test plus the new `cr.DumpNearbyActors`
console command (logs class/mesh/bounds/tags of everything near the player)
closed the case. Deleted — the emissive terminal halo carries the job.

**The magenta wire web** across night skies was a **persistent city-scale
`DrawDebugBox`** (160m, thickness 22) labeled "editor-only" — but Kenny plays
DEVELOPMENT builds where ENABLE_DRAW_DEBUG is on. Deleted; the low guide
rails already mark the zone.

Lesson recorded: **persistent debug draws are invisible to actor-based
forensics** (they're not actors) and DO ship to Development players.

## 1. Camera aligned with the player's environment

- Top-down and isometric now live INSIDE the street canyon (1150 / 1250 arm)
  instead of above the roofline — no more "camera outside the building
  showing roofs".
- Boom-camera pitch clamped (-28°..+38°) — pitching hard down used to park
  the camera inside the raised entry platform (live: full-screen unlit
  geometry).
- Spawn framing: after the footing check, the pawn now faces the direction
  with the most open camera room (four-way `ECC_Camera` probe, logged
  `[SpawnClear] facing yaw ...`).
- `ApplyCameraPerspective()` now runs at BeginPlay — a fresh spawn previously
  never applied ANY perspective, leaving weapon models at their constructor
  visibility (hidden): **this is why Kenny never saw a weapon at spawn**.

## 2. Elevation / characters standing ON regions

- Zombies, survivors and friendly NPCs all ground-snap at BeginPlay
  (trace-down + capsule/bounds placement) — no more floating or buried
  characters on layered surfaces.
- Player `MaxStepHeight` 45→65 + walkable angle 50° — region slab lips read
  as curbs, not walls.
- `[AutoGround]` pass at the end of every city spawn — BOUNDED (gap
  60–420uu only) after the first, naive version dragged window strips and
  signage down into the street and entombed the spawn (live regression,
  same-day fix). Architecture above 420uu is never touched.
- Kenny's confirmation that T evacuates stuck states = the pass-7 escalating
  watchdog + teleport depenetration working as designed.

## 3. Weapons visibly held + visible cycling

- Body weapon scaled 1.35x and pushed clear of the fist (true-scale pistols
  vanished inside the chunky v2 glove).
- Minimal HUD now LEADS with the weapon: "Balanced Handgun (wheel or [ ] to
  cycle)" — the old vitals strip never named the weapon, so swaps looked
  like nothing happened.
- Wheel/bracket cycling verified live repeatedly (toast + subtitle + HUD all
  update); `[HeldWeapon] body weapon 'PistolV3' attached via hand_R`
  confirmed from the shipped pak.

## 4. Fire feedback

- The thick red 0.65s `DrawDebugLine` per shot is gone. Now: a hair-thin
  warm tracer (0.07s) from just ahead of the muzzle to the actual impact,
  plus a small warm glint at the hit point. The aim-assist confirm sphere
  (thick red) became a slim brief amber ring.

## 5. Fidelity groundwork (start of the "beyond 8-bit" push)

- Readability clamps on the key light: night pulls 55% toward desaturated
  moon-blue, day toward warm white — city climate stays as tint, never a
  saturated flood.
- NEW moon key light (cool blue, -35°, night-only): true midnight used to be
  pitch black because the sun points UP at night.
- Streetlights now cast REAL warm sodium pools (7 movable point lights per
  city, shadows off) — night streets read noir instead of void.
- Mood fog thinned (0.011 density, 1600 start) for mid-range clarity.
- v3 PBR kit + mood post from passes 6–7 remain the base; full texture/
  normal-map authoring and character re-sculpt queued as the next major art
  pass (documented for scope honesty: procedural-color meshes + better
  lighting is where this build lands).

## Verification

- 17 compile+playtest cycles; final acceptance frame: daylight street,
  lit building rows, grounded kneeling survivor on the crosswalk, lamp
  pools, weapon-led HUD, level horizon.
- `verify_gameplay_fixes_2026_07_07.py` updated to the day's final
  contracts — ALL CHECKS PASSED.
- Watchdog: **150 pass / 0 REAL — VERDICT: PASS.**
- Repackaged + soaked (see addendum) with `[AutoGround]`, `[CanopyTrim]`,
  `[PadCanopy]`, `[HeldWeapon]`, `[SpawnClear]` markers live from the pak.

## New ops tooling

- `cr.DumpNearbyActors [radius]` — class/mesh/bounds/tags of everything near
  the player; the tool that unmasked the armory lattice and cleared actor
  suspects until only debug draws remained.
- `GetAll <Class> <UPROPERTY>` works from the in-game console for quick
  state reads (TimeOfDay, bIsNight, light intensity); non-UPROPERTY values
  print nothing.
- `viewmode unlit` instantly separates "world is dark" from "camera is
  inside geometry".
