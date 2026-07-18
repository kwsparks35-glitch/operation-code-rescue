# July 11 Regression Repair Pass

Date: 2026-07-11

This pass resolves the eight regressions reported after the July 10 package:
overlapping narration, elevated or inaccessible ground regions, low terminal
contrast, missing zombies, rejection of valid code, an inescapable code-accepted
screen, an unanimated player, and unresponsive pause controls. The work was
compiled, tested in the editor, clean-cooked, packaged, tested again from the
packaged executable with the real language save, and archived as bundle version
`51494982.0.198`.

## Final Result

The final packaged acceptance marker is:

```text
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1 population=1 sky=1 day_period=1 challenges=1 alternate_solution=1 progression=1 supplies=1 target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 crafting=1
```

The real-save recovery run also reports:

```text
[RadioVoiceArbiter] previous_voice_stopped=1
[CityZombiePopulation] COMPLETE PASS city=1 renewable_alive=124 ... saved_renewable_deaths_ignored=130
[CampaignGroundRecoveryAudit] COMPLETE PASS city=1 recovered_relative_z=92.00 height=1 bounds=1 ground=1 population=1
```

## Documents

- `REGRESSION_REPAIR_IMPLEMENTATION_AND_TEST_REPORT.md`: implementation,
  root-cause analysis, test evidence, and remaining external release work.
- `TestLogs/`: editor, packaged, challenge, recovery, terminal, package, and
  warning-scan evidence.
- `Screenshots/`: terminal contrast, armory, two-arm aim/combat, grounded corpse,
  and corpse-fade review captures.
- `Release/`: current package-integrity and release manifests.

## Package

- App: `PackagedMac/Mac/CodeRescueUnreal.app`
- Bundle ID: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.198`
- Size: 2,051.6 MB
- Local code signature: valid
- Local package integrity: ready
- Developer ID signing and notarization: pending external credentials

## Part 2 — Afternoon packaged-report fix pass (same day)

Kenny's playtest of `.198` surfaced four remaining issues: floating
characters, dead pause-menu clicks, over-literal terminal validation, and an
unenterable final coding station. All four were root-caused, fixed, and
re-verified end-to-end; the shipped app is now bundle version
**`51494982.0.199`**. Full record:
`PACKAGED_REPORT_FIXES_FLOATING_PAUSE_VALIDATION_STATION10.md`, with evidence
in `TestLogs/*FixPass*`. The integrated acceptance marker gained
`characters_grounded=1`, `guidance=1`, and `overlay_passthrough=1`, and the
alternate-solution audit now pins eight non-canonical correct solutions
(including both of Kenny's rejected submissions, verbatim).

## Same day, part 3 — Art + Physics pass (CharactersV3 / WeaponsV4 / authored ragdoll)

Kenny asked for the characters, weapons, and character/world physics to be
rebuilt in Blender for a more stimulating, inviting game. Delivered: five
CharactersV3 (incl. the NEW ZombieRunnerV3 sprinter) with keyed
Attack/Flinch/Death collapses and survivor rescue Waves, five higher-detail
WeaponsV4 with emissive sights, 16-body/15-constraint ragdoll physics assets
built analytically per character (the auto-fitter only managed 2 bodies), and
the C++ that puts an authored share of the horde (cr.AuthoredZombieShare=35)
in the new bodies with ragdoll + physical hit reactions restored
(cr.AuthoredBodyPhysics kill switch). Verified: import validation PASS,
watchdog 129/0 REAL, editor resume soak (20 authored zombies, 13 corpse
lifecycles through the .200 crash path, 0 asserts, 0 material warnings), and a
packaged resume smoke on the repackaged app (11/11 authored physics kept,
5+ min, 0 fatals). Full record: `ART_PHYSICS_V3_WEAPONS_V4_PASS.md`.

## Same day, part 4 — Hero soldier, doors, wind, ground unification

Kenny asked (with two of his own screenshots) for his character built as a
handsome soldier to an exact spec, environment physics (wind, enterable
doors), and a fix for the ground-level mismatch — all verified together in a
single test run. Delivered: SurvivorKennyV4 hero body (5'10", 220 lb build,
strawberry-blonde crop + full red beard, blue eyes, US flag patch; 7 Blender
iteration rounds) riding a new presentation layer over the audited Manny
locomotion; real swinging double doors in every first-level doorway
([E] toggle, auto-close, block zombies) + safehouse interiors; an ambient
gusting wind system for trees/bushes (cr.WindStrength); and
UnifyFirstLevelGroundTops — walkable-top ground unification (driving-surface
top spread 18.0 -> 0.40 uu, curbs preserved, blinding plazas toned) with the
access audit upgraded to gate on it. THE single integrated run:
`[FirstLevelIntegratedAudit] COMPLETE PASS` (all 21 tokens) in the editor AND
in the repackaged PackagedMac app; watchdog 129/0 REAL. Full record:
`HERO_WORLD_PHYSICS_PASS4.md`; logs in `TestLogs/IntegratedV4_*` and
`TestLogs/PackagedIntegrated_Pass4_*`.

---

## Pass 5 (2026-07-16) — perspectives, ADS/scope zoom, grenade ballistics, IMPORT-SCALE ROOT CAUSE

Kenny's packaged-app report (3 screenshots): unhelpful perspectives (player a
dark speck / floating weapon), remaining ground misalignment, "see down the
barrel / into the scope" aiming with user-controlled 1x–50x magnification and
aim-before-fire, and grenade launch-angle + landing-area preview with a real
explosion instead of the wireframe debug dome.

Headline: **every invisible-character mystery traced to ONE line — character
FBX imports in the automation context land at 1/100 scale; both import
pipelines now set `import_uniform_scale=100.0`.** Second bomb defused: the
"first-person arms" were a full SKM_Manny with its head inside the FP lens,
masked for weeks by that same scale bug — FP is weapon-only until a real arms
rig ships. Also shipped: 6-perspective camera retune; ADS system (Z-cycled
1x/2x/5x/10x/20x/50x, per-weapon sight-line poses, scope view ≥5x, sharp
optics via DoF/motion-blur kill, single-pose-writer consolidation); grenade
ballistics (capsule-overlap sky-detonation fixed, deterministic gravity flight
matching the 22-dot arc preview + blast-circle outline, physical explosion:
burst VFX/typed light flash/scorch/camera kick); plaza tone 0.055→0.085.
Verification: 26-stage `-CodeRescuePerspectiveReview` harness (screenshot
staging rule: arrange on even stages, photograph on odd — RequestScreenshot
captures at END of frame), watchdog VERDICT: PASS (3 stale pins migrated),
single integrated run **COMPLETE PASS (all 21 tokens)**, repackaged + verified.
Full record: `PERSPECTIVES_ADS_GRENADE_PASS5.md`.

---

## Hotfix (2026-07-16 late) — late-campaign freeze / dead movement

Kenny: movement almost non-functional, game completely frozen. Reproduced on
his real save (resume into city 03 Chicago): game thread spun INSIDE a frame
at 400% CPU. `sample` profile → `IsLocationInsideProtectedLearningZone`
iterated EVERY world actor per call, called per-AI/per-zombie/per-player-tick
= O(agents × world) per frame on dense saves. Fix: cached zone snapshot (4 s
TTL, per-world) + distance-culled marker facing/bob ticks + permanent
`[ResumeHealth]` FPS heartbeat under the resume harness. Packaged verify:
93–104 FPS steady on his save, editor+packaged integrated audits 21/21,
watchdog PASS. The fresh-level audit had passed on the frozen binary — dense
resumed saves are now part of the verification chain. Full record:
`FREEZE_HOTFIX_PROTECTED_ZONE_SCAN.md`.

---

## Pass 6 (2026-07-17) — thermal scope view, control cleanup, ALL-city ground, Cmd+Shift+4

Kenny's report (2 photos + 2 videos incl. a thermal-scope reference):
movement "still" slow (video predated the freeze fix — now PROVEN healthy:
`-CodeRescueMovementProbe` walks the pawn on his real save, packaged:
commanded 900 = measured 900 uu/s @ 60–120 FPS); the "strange object" =
the anatomical bite wound attached to the HIDDEN driver body (now rides the
visible hero, snug, 0.42 scale, visibility-propagated); ground unifier ran
ONLY in city 0 — his Chicago never unified ("skipped — no driving surfaces
registered") — now every city registers + edge-plane arena tests + wider
blinding-slab toning (verified live: Chicago road 28.4→4.8, 8 slabs toned);
keybinding de-dupe (one key per action; Z zoom/scanner double-bind fixed,
scanner→K; hints updated); Cmd+Shift+4/F12 in-game screenshot straight into
Screenshots_for_Correction with on-screen confirmation; and the reference
aiming system: hold-aim from ANY camera steps into a full circular THERMAL
scope view (black surround, luminous rim, mil-dot crosshair, ZOOM: Nx,
cool-sensor tint w/ hot bloom) at 1x–50x, releasing lowers the weapon and
restores the camera. Verification: watchdog PASS, editor + packaged
integrated audits 21/21, packaged resume probe on Kenny's save. Full
record: `SCOPE_CONTROLS_GROUND_PASS6.md`.

---

## Pass 7 (2026-07-17) — the 12%-speed trap, sandbox screenshots, trackpad fire

Kenny: game STILL extremely slow / screenshots claim success but no files /
cannot fire while aiming on the trackpad. Root causes, all proven on his
machine (M4 Pro): (1) frame rate was NEVER the problem (117–120 FPS
fullscreen native on his save) — **F10 photo mode set global time dilation
to 0.12 and hid the HUD with no indicator**; F10 sits beside the F12
screenshot key. Binding removed, BeginPlay force-restores dilation,
verifier pin migrated. (2) **The packaged app is App-Sandboxed** — every
"lost" screenshot was written inside its container while Desktop writes
were denied; capture now goes through the engine pipeline, tries the direct
move, and on denial renames in place surfaced via the
`Screenshots_for_Correction/InGame_Captures` symlink; confirmation prints
only after the file verifiably exists (his four lost captures recovered).
(3) Trackpads deliver one button at a time → **aim is now a toggle/hold
hybrid**: quick right-click latches sights (click fires, click-again
lowers), long hold stays momentary. Packaged probe on his save:
latch→fire→lower all pass, screenshot delivered=1 on disk, 120 FPS,
vel=900. Watchdog PASS, editor+packaged audits 21/21. Full record:
`SLOWMO_TRAP_SCREENSHOT_SANDBOX_AIM_LATCH_PASS7.md`.

---

## Pass 8 (2026-07-17) - full perimeter ground + held-weapon shadow

Kenny's eight screenshots exposed two shared-system defects. The canonical
mission floor ended 24-26 m inside the arena walls, revealing the recovery
floor 6.11 m below around the right edge and matching perimeter strips; the
third-person weapon also inherited the hero hand rig's import scale, becoming
building-sized and casting a street-wide shadow. The campaign floor now derives
from the arena bounds (`10950 x 9550` half-extents beneath `10800 x 9400` wall
lines), and a 36-point collision audit requires `9/9` support on the right edge
while rejecting the catch floor. Held weapons now use absolute, per-family
real-world scale, zero hand offset, a scale-safe capsule fallback, and no
cosmetic shadow casting; the integrated combat gate verifies size, attachment,
offset, and `casts_shadow=0`. Three right-edge visual captures plus rifle views
show the corrected geometry, city 2 repeats the `36/36` result, and editor and
packaged integrated runs both pass all systems. Repackaged app:
`51494982.0.209`, 2055.0 MB, strict local code signature valid; full record:
`PERIMETER_GROUND_WEAPON_SHADOW_PASS8.md`.

---

## Pass 9 (2026-07-17) - grounded characters, symbol loot, purposeful districts, weather

Kenny's six screenshots showed living characters suspended above the floor,
generic cube rewards, unclear/enclosure-heavy regions, and no readable weather
influence. The shared grounding path now aligns visible skeletal feet after
animation settles (`134` characters, `0` floating, `129/129` visible feet);
six Blender package families provide dual-sided physical ammo/medical/armor/
tech/salvage/utility symbols (`26/26` authored and grounded); and 124 giant
enclosure blocks are replaced by attached, nonblocking threat rings. The first
level now has an open-space-validated logistics depot with usable stock, a
weather relay, and a quarantine checkpoint, while a 120-second wind/rain/fog
cycle adds authored debris/rain, traction, foliage wind, atmosphere, and AI
visibility effects. Editor and packaged five-shot reviews pass, both integrated
runs pass all 29 subsystem tokens, and the normal packaged launch still stops
at the six-language selector. Repackaged app: `51494982.0.210`, 2059.6 MB,
strict local signature valid; full record:
`../improvement_pass_2026-07-17/WORLD_LOOT_WEATHER_GROUNDING_PASS9.md`.
