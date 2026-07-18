# Pass 6 — Thermal scope view, control cleanup, all-city ground, screenshot key
**Date:** 2026-07-17 · **Requested by:** Kenny (2 photos + 2 videos) · **Status:** shipped

Kenny's report: slow movement persists (video); a strange obscuring object
around his character; uneven ground never actually fixed in his play area;
too many redundant buttons; no way to take screenshots in-game
(photographing the screen with his phone); and no real "look down the
weapon/through the scope" aiming — with a reference video showing exactly
what he wants (full circular thermal scope, fine crosshair, ZOOM label,
raise on hold / lower on release).

## 1. Slow movement — measured, resolved, and permanently instrumented

His video timestamp (06:36) predates the freeze hotfix; the frozen build was
the cause. To prove it with numbers instead of assurances, the resume
harness gained `-CodeRescueMovementProbe`: the pawn walks itself forward for
a fixed window while `[ResumeHealth]` pulses log frame/FPS/location/velocity.
**Packaged app on HIS save (city 03 Chicago): commanded 900 uu/s → measured
900 uu/s (5,400 uu covered in the 6 s window), 60–120 FPS.** Movement is
fully functional; `maxwalk=900` on every pulse.

## 2. The strange object — identified and fixed

The pulse screenshots from his save caught it: **the anatomical bite-wound
mesh** (spawned when zombies bite the player) attached to the HIDDEN driver
body's shoulder bone at 0.88 scale, floating 1.5 uu off the bone — a black/
red gore mass hovering beside the visible hero, immune to visibility
toggles. Fixes (`SpawnAnatomicalBiteWound`): attaches to the VISIBLE hero
presentation mesh (hero-rig bone names `upperarm_R`/`chest` first), snug
(+0.5), scale 0.42, and the hero's visibility now propagates to attached
children (so wounds vanish in first person with the body). The objects at
his feet in the video are ordinary street pickups (ammo/flare), not
followers. Result verified in his save: clean silhouette, small readable
wound decal only.

## 3. Uneven ground — he was right; it was never fixed WHERE HE PLAYS

Street surfaces registered for the ground unifier **only in city 0**; every
later city logged `[GroundUnify] skipped — no driving surfaces registered`.
Kenny has been in city 03 Chicago for days. Fixes (`CodeRescueGameMode`):

- street kit registers for the unifier in EVERY city;
- arena-membership tests use the ACTIVE city index (was hardcoded 0) and
  accept edge planes whose bounds touch the arena (origin-only testing
  skipped the perimeter slabs in his outskirt photos);
- blinding-slab toning window widened (|top| < 160, was 60).

Verified live on his save: `RoadStraightV3 top 28.4 → 4.8` snapped in
Chicago, 8 plaza slabs toned (was 0), outskirt smooth/speckled planes meet
flush in the pulse screenshots.

## 4. Control cleanup — one key per action

Removed duplicates: Interact **E** (was E/Enter/Tab/G), Fire **LMB** (was
also F), Reload **R** (was also LCtrl), Help **H** (was also M), Recover
**Backspace** (was also F8), Pause **Esc** (was also P), Camera cycle **C**
(was also V). Fixed the **Z double-bind** (scope zoom + radio scanner fired
together — scanner moved to **K**). All HUD hints, arena guidance, and
fail-safe board strings updated to the new single keys.

## 5. In-game screenshots — Cmd+Shift+4

`Cmd+Shift+4` (and `F12`) now captures the game frame **directly into
`Operation_Code_Rescue/Screenshots_for_Correction/InGame_<timestamp>.png`**
with an on-screen confirmation — the exact folder Kenny already uses for
bug-report photos. Weapon-slot keys ignore presses while Command is held so
the chord can't switch weapons.

## 6. The scope view — Kenny's reference, implemented

- **Raise/lower from any camera**: holding aim in ANY perspective steps into
  the sight view and stores the previous camera; releasing hands it
  straight back (BeginAim/EndAim). A modal UI mid-aim lowers the weapon
  outright (no stuck sights).
- **Scope-capable weapons (rifles, bolt launcher, rocket launcher)**: ADS =
  full-screen **circular scope** at every zoom — solid black surround
  (4 fill boxes + overlapping thin antialiased ring bands; single
  mega-thick polylines render as streaky disjoint quads), luminous rim,
  fine crosshair with mil-dot ticks, and a **ZOOM: Nx** readout. `Z` cycles
  1x/2x/5x/10x/20x/50x.
- **Thermal read**: while scoped, the world desaturates to a cool sensor
  tint with lifted exposure and hot bloom (emissive zombie eyes/muzzle
  flashes glow against the cold background) via camera post-process.
- Sidearms (pistol/shotgun/SMG) keep the down-the-barrel viewmodel.
- HUD panels/status texts that banded across the glass collapse during the
  scope view and restore after; subtitles remain (the reference sight shows
  its subtitles inside the glass too).
- Review harness now aims FROM the side camera (exercises the perspective
  round-trip) and captures **with UI** so review shots show the player's
  actual sight picture.

## Verification chain (single run of everything together)

26-stage perspective review (13 screenshots incl. scope views at 1x–50x,
grenade arc + ground detonation), watchdog **VERDICT: PASS (0 REAL)**,
editor integrated audit **21/21 COMPLETE PASS**, repackage, **packaged**
integrated audit **21/21 COMPLETE PASS**, and the packaged resume probe on
Kenny's real save (movement numbers above). Dev save slots restored after
staging.

## Residuals / notes
- Packaged-build `FScreenshotRequest` writes no file in the headless probe
  (harness pulses capture via the editor instead); Kenny's interactive
  Cmd+Shift+4 uses the game viewport and confirms on-screen with the path.
- Outskirt smooth-vs-gravel TEXTURE difference remains by design (no
  elevation lip); a distant white sliver at the perimeter wall base is on
  the polish list.
