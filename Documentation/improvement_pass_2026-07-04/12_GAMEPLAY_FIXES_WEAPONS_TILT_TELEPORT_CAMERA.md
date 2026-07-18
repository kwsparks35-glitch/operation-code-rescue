# 12 — Gameplay Fixes: Weapons, Tilt, Teleport, Camera (2026-07-07, pass 7)

Kenny's four-issue report from the packaged build, each REPRODUCED LIVE in the
editor `-game` build, root-caused, fixed, and re-verified live before
repackaging. His session log (container, 10:33–10:43) provided the forensic
baseline: zero `[MoveWatchdog]` fires during a session he described as
"completely unable to move" — which told us the watchdog itself was blind, and
guided fix #3.

## 1. "Cannot select/cycle weapons; weapon visible only in first person"

Diagnosis: weapon SWAPPING always worked (slots 1-0 are polled), but at any
third-person camera nothing changed on screen — the ONLY weapon model in the
game (`FirstPersonWeaponSilhouette`) is parented to the FP camera and
owner-only. Wheel/bracket cycling additionally existed ONLY as BindKey — the
exact event path known to drop in packaged builds. So from Kenny's seat:
switching did nothing visible, cycling did nothing at all.

Fixes (`CodeRescueCharacter.cpp`):
- NEW `ThirdPersonWeaponMesh` component — the held weapon snaps to the v2
  rig's right hand (Blender `hand.R` → UE `hand_R`; mesh-root fallback for the
  mannequin). Forensic marker `[HeldWeapon]` logs the mesh + attach bone;
  boot log confirms: `body weapon 'PistolV3' attached via hand_R`.
- Wheel + `[`/`]` cycling now POLLED like every critical key; a 0.15s
  debounce in `CycleWeapon` dedupes when the bound event also delivers.
- Every swap announces through TWO channels: the debug toast and the
  always-visible subtitle bar ("Equipped: Pump Shotgun (1-0 slots, [ ] or
  mouse wheel to cycle)") — live-verified.
- Visibility contract: FP silhouette only in first person, body weapon in
  every other perspective, both hidden for thrown weapons.

## 2. "Character perpetually at ~15 degrees"

Diagnosis (live screenshot): the BUILDING VERTICALS leaned exactly as the
character did — the CAMERA was rolled, not the character. Residual roll had
crept into the control rotation (shake fallout / teleport rotations) and the
spring arm inherited it, tilting the whole rendered world.

Fixes: `CameraBoom->bInheritRoll = false` (roll refused at the boom, all
third-person cameras) + `PollDirectKeys` strips any residual control-rotation
roll every frame (covers the FP camera, which uses pawn control rotation).
Live-verified: level horizon, upright body from shoulder and tactical cams.

## 3. "T teleports into a placement where the character cannot move"

Three compounding causes found:
- T's step teleport dropped the pawn at `beacon target + fixed offset` with
  NO clearance test — with the safehouse interior and (pre-fix) misplaced
  buildings, that spot was often inside geometry.
- The 07-06 watchdog keyed on the movement component's CONSUMED input, so
  states where input never reaches the component were invisible; and its 3s
  hold threshold reset every time a frustrated player changed keys — Kenny's
  log shows zero fires during his stuck session.
- The spawn pad itself was embedded in geometry (proven live:
  `[Teleport] destination blocked — relocated 220uu` fired on a teleport TO
  the pad).

Fixes:
- `AdjustTeleportDestination()` — engine `FindTeleportSpot` depenetration +
  8-direction ring search (220/420/720uu) with ground snap — wired into the
  step teleport, both city teleports, arena recovery, the watchdog's
  escalated rescue, and the new spawn check. Every relocation logs
  `[Teleport]`.
- `EnsureSpawnClearance()` — one-shot footing check 0.75s after spawn (after
  city layers finish): live-verified `[SpawnClear] spawn point was embedded
  in geometry — relocated ...`.
- Watchdog rebuilt: reads RAW movement keys (blind spot #1 closed), triggers
  at 1.25s (mash-proof), and escalates — nudge → `FindTeleportSpot`
  relocation — with an escalation ladder that resets once the pawn moves
  freely.

## 4. "Walls obstruct the camera; most of the view is obstructed"

Four compounding causes found live:
- **My own bug (pass 6): `SpawnCityBlockV3Layer` used ABSOLUTE Y while X was
  relative** — with the street at Start.Y=-5520, all 18 buildings + furniture
  spawned 19–91m north of their row: ON the roadway and the spawn pad. This
  alone explains "most if not all of the view is obstructed". All placements
  are now `Start.Y + offset` (matching the streetscape layer).
- Fixed cameras (top-down/iso/side) ran with the boom probe DISABLED and sat
  INSIDE building geometry. The probe is now always on; top-down raised to
  2600 (above the ~20.6m v3 roofline); isometric steepened to -64°/2000.
- When the probe collapses the boom (tight interiors, wall contact), the
  screen used to fill with the back of the character's head. NEW
  `UpdateCameraProximityFade()`: within 120uu the body + held weapon hide so
  the player sees the room — the standard third-person treatment.
  Live-verified in the safehouse interior.
- The night-sky star dome rendered as a full-screen cyan shell with black
  quads during the dawn window — a zero-parallax blinder (confirmed by a
  movement parallax test). SHIP-SAFE: dome disabled until rebuilt as a
  proper one-sided skybox; the moon + fog/post carry the night mood.
- Mood fog thinned (density 0.024→0.011, start 900→1600, max opacity
  0.82→0.72): mid-range readability wins; the mood stays.

## Verification ledger

- Live playtest cycle (editor `-game`): movement ✓, T step-teleport with
  `[Teleport]` relocation ✓, `[SpawnClear]` ✓, level horizon + upright body ✓
  (two cameras), weapon wheel-cycling + dual feedback ✓, `[HeldWeapon] ...
  attached via hand_R` ✓, proximity fade in interiors ✓, city renders in
  correct rows ✓, Backspace tutorial dismiss without teleport ✓.
- `Scripts/verify_gameplay_fixes_2026_07_07.py` — 17/17 PASS.
- Watchdog: **150 pass / 0 REAL — VERDICT: PASS.**
- Repackaged + soaked (see addendum below).

## Ops notes

- `timeout` is not a macOS builtin — watchdog runs silently die under it;
  run inline.
- Photo mode (F10) is the fastest way to get daylight + HUD-free body
  inspection during a night-cycle playtest.
- The parallax test (move; compare pattern) instantly separates "nearby
  prop" from "skybox-scale shell" when a mystery mesh fills the screen.
