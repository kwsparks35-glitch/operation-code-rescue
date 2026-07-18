# 11 — Launch Menu + Movement Lock + v3 Art Overhaul (2026-07-06, pass 6)

Kenny's report after playing the packaged build:
1. *"the language selection screen is too large to see all available
   options/text"*
2. *"the user's character is completely locked stationary… even with the use
   of the 'T' function"*
3. *"characters and world development, general aesthetics, weapon inclusions…
   (lack of) vehicles/sidewalks/completed structures… develop all of these
   such that they actually look realistic"*

## Forensics (from Kenny's own session logs + live repro)

His two packaged sessions (14:01, 14:07 — both ~2 min, both quit) plus a live
repro in the editor `-game` build established:

- **Menu overflow REPRODUCED**: the launch panel was a fixed 760x620 canvas
  slot at 34% viewport height. Its content (16-line route preview + 6 track
  rows) overflows 620px at EVERY resolution — C++ was half-clipped and
  Python/MATLAB fully invisible at 1310x780, with no scrollbar. **This also
  explains the mysterious "C+ track" in his logs: C+ was the last fully
  visible button, so that's what he clicked.**
- **Menu buttons ate clicks**: two clicks on a fully visible NEW C+ RUN did
  nothing in the live repro (decorative text/borders in the hit-test path;
  packaged Slate taint history compounds it).
- **The freeze**: his run-2 log shows recovery teleporting him TWICE to the
  same clear-ground spot while movement stayed dead — input polls partially
  alive (bound-E Interact fired) but WASD/T dead. The language-gate return in
  `PollDirectKeys` suppresses ALL polled movement and `TeleportToNextObjective`
  deliberately refuses while the gate is active; menu-tainted packaged input
  can leave exactly this state. (My playtests had never exercised the real
  menu path — every prior session used `-CodeRescueBypassLaunchLanguageMenu`.
  Lesson recorded.)

## Fixes (defense in depth — a permanent freeze is now structurally impossible)

**Menu (`CodeRescueMainMenuWidget.cpp`)**
- Launch panel anchors 0.28→0.97 vertical stretch (adapts to ANY window);
  non-launch menu panel stretched similarly.
- Whole menu column in a ScrollBox; track buttons come FIRST, the route
  preview + save roster text sits below and scrolls; keyboard hint line added
  ("1-6 picks, ENTER deploys").
- Every decorative layer (fog band, preview text, selected-language text) is
  `HitTestInvisible` — clicks can only land on buttons.
- `RestoreGameInputBeforeTravel()` (GameOnly input mode, cursor off, ignore
  flags cleared, static UI-open flag cleared) called from ALL FIVE deploy
  paths + the pawn-poll deploy in the character.

**Movement failsafes (`CodeRescueCharacter.cpp`)**
- `[LaunchGate] FAILSAFE`: if the language gate is active >3s with NO launch
  menu on screen (a contradiction), the selection auto-commits and input
  unlocks — whatever path dropped the commit.
- `[MoveWatchdog]`: movement input held ~3s with zero velocity → re-assert
  MOVE_Walking + 32uu depenetration nudge + on-screen notice. Self-heals
  wedges, stale movement modes, spawn penetration — any cause.
- Static `bUIOpen` reset in the player pawn's BeginPlay (it survives
  OpenLevel by design; a fresh world must never boot input-locked).

**v3 art (24 assets, `Scripts/BlenderArt/build_world_art_v3.py` + `build_weapons_v3.py`)**
- 4 CLOSED buildings (brick/concrete/stucco/brownstone): inset window grids
  with lit/dark glass, sills, storefronts w/ awnings + doors, fire escapes,
  cornices, parapets, rooftop AC/vents/antennas.
- Street system: worn asphalt road w/ dashed paint, intersection w/
  crosswalks, sidewalk w/ curb + expansion joints, streetlight (emissive
  head), traffic light, hydrant, trash cluster, bus stop (glass + bench),
  power pole, jersey barrier.
- 5 vehicles w/ real wheels (tire+rim), glass, emissive head/taillights,
  plates, mirrors: clean sedan, crashed sedan (crumpled hood, flat tire,
  open door), police cruiser (light bar, door band), delivery van, pickup.
- 5 weapons w/ real silhouettes: pistol (slide serrations, raked grip),
  pump shotgun (mag tube, wood furniture), AR-pattern rifle (rails, curved
  mag, flash hider), suppressed SMG (folded wire stock), crossbow (limbs,
  string, scope). Blued steel / polymer / worn wood materials.
- ALL materials are principled-BSDF → UE imports real MaterialInstances with
  roughness/metallic/emission — the single biggest realism jump over v2's
  flat colors. Palette is muted/weathered (survival-horror, no primaries).
- Visual QA: lineup render `RawArt/CityKitV3/_review_lineup.png` (brick tone
  darkened after first review).

**Wiring (`CodeRescueGameMode.cpp`)**
- Streetscape roads/sidewalks/vehicles upgraded to v3 in place (v2 fallback
  if import missing — logged `[CityBlockV3] v3 street kit ACTIVE/not imported`).
- NEW `SpawnCityBlockV3Layer`: 18 buildings forming street walls on both
  sides (deterministic per-city variety), staggered streetlights, hydrant,
  trash, bus stop, power poles, barriers, extra parked/wrecked vehicles.
- NEW `SpawnCityMoodLayer`: ExponentialHeightFog (near-neutral cold gray —
  honors the 07-02 de-teal) + unbound filmic post volume (bloom 0.55,
  vignette 0.42, grain 0.16, slight contrast/desat, cool shadow gamma).
- FP weapons: `WeaponMeshPathV3` map (pistol/shotgun/rifle/SMG/crossbow by
  EWeaponType) with v2 fallback.

## Ops lessons (paid for in time today)

- **UE 5.7 Interchange imports GLBs at `<dest>/<SourceName>/…` — one level
  DEEPER than the legacy importer.** All v3 paths use the doubled segment
  (`CityKitV3/<Name>/<Name>/StaticMeshes/<Name>`).
- Interchange parallel-imports collide on shared material names ("Multiple
  import tasks are importing the same asset") — import sequentially and
  verify via `list_assets`, not blind `does_asset_exist` on assumed paths.
- A packaged app launched from a shell on THIS Mac can hang eternally black
  at frame 1 (`RHIBlockUntilGPUIdle`, zero MTLCompilerService activity) while
  Finder-launches by the seated user paint instantly — treat the editor
  `-game` binary as the reliable remote-playtest vehicle.
- `pkill` the editor then VERIFY with `pgrep` — one kill silently failed and
  the surviving editor auto-reimported assets underneath the session.
- Naming: helper functions on the character must NOT start with `Tick` —
  five verifiers extract the Tick body via `find("…::Tick")` and matched
  `TickStuckMovementWatchdog` instead (renamed `UpdateStuckMovementWatchdog`).

## Verification

- `Scripts/verify_menu_movement_v3art_2026_07_06.py` — 23/23 PASS.
- Watchdog: **148 pass / 0 REAL — VERDICT: PASS.**
- Live repro playtest (editor `-game`, real menu path, no bypass): menu
  overflow + dead buttons reproduced pre-fix; deploy via Enter; movement
  verified working in-world pre-art-wiring.
- Packaged rebuild + soak: see ledger addendum below (this file is written
  during the cook; Kenny is standing by to play the fixed build — his
  playtest is the final gate for the visual pass).
