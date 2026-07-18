# Pass 4 — Hero Soldier, Enterable Doors, Ambient Wind, Ground Unification (2026-07-11, late night)

Kenny's directive (with two reference photos from his own sessions, including
one from the NEW build at 22:06): keep building the world + characters with
Blender, focus on HIS character and the first-level NPC environment, build him
as a handsome soldier to an exact spec, add environment physics (wind in
trees, enterable doors/buildings), FIX the ground-level mismatch he
photographed, verify everything together in a SINGLE test run, repackage,
document.

## 1. SurvivorKennyV4 — the HERO player body

`Scripts/BlenderArt/build_hero_kenny_v4.py` (extends the v3→v2 pipeline).
Spec compliance:

| Spec | Implementation |
|---|---|
| White male (US) | fair warm skin; **US flag patch** (canton + stripes) on the right shoulder |
| Strawberry blonde hair | custom crop shell w/ natural hairline + temples (0.66,0.42,0.20) |
| Full red beard | continuous cheek-line beard shell + mustache band (0.52,0.21,0.09) |
| Blue eyes | iris material (0.15,0.33,0.60) |
| 5'10" | whole rig + mesh scaled 177.8/186 = 0.956 |
| 220 lbs @ ~5% | muscle 1.26 torso, waist pinch −8.5%, lat flare, trap wedges, embedded pec plates, bare muscular forearms (rolled sleeves) |
| Soldier kit | fitted olive tee + plate carrier, dog tags, camo pants, knee pads, thigh holster, emissive headlamp |

**Seven visual iteration rounds** (previews in `RawArt/CharactersV4/previews_v4/`):
round 2 fixed pec/dog-tag/trap integration; rounds 3–6 engineered the beard
(cheek-line diagonal, mustache band, face-carve quantization); round 7 fixed
shell poke-through. Key art lessons recorded in §5.

**Presentation-layer integration (the important design decision):** the Manny
production rig KEEPS driving locomotion, two-arm aim and landing compression —
those are audited contracts (`PlayerAnimationRuntimeAudit`) and good movement
feel. The hero rides a new `HeroPresentationMesh` (skeletal, no collision)
that: replaces every driver body VISUALLY in non-FP perspectives, runs its own
speed-driven Idle/Walk/Run (authored V4 clips), and carries the held weapon
(`hand_R`). First-person view is unchanged. Log marker:
`[CharacterV4] HERO presentation layer active = SurvivorKennyV4`.

Physics: imported with create_physics_asset then rebuilt analytically → 16
bodies / 15 constraints (same builder as the V3 horde).

## 2. Enterable buildings get REAL doors

- Blender `Scripts/BlenderArt/build_world_kit_v4.py` → `SM_Door_Steel`
  (hinge-origin leaf: panels, kick plate, viewport slit, hinges, knob) +
  `SM_Curb_Ramp` (reserve transition piece).
- New `ADoorActor` (`Source/CodeRescueUnreal/DoorActor.{h,cpp}`): smooth
  220°/s swing, **[E] open/close** (tag-dispatch like Helipad/MessageMarker,
  HUD prompt "[E] open / close door"), auto-close 8 s, **blocks zombies and
  gunfire while closed** (BlockAllDynamic + RuntimeWeaponTraceBlocking).
- Placement: **double doors (2 leaves × 1.6 m) in every first-level V5
  building doorway** (market/clinic/café — the three audited doorways), spawn
  collision override AlwaysSpawn (hinges sit at the jambs). The campaign civic
  safehouse (cities 1+) got the same doors PLUS a lived-in interior (cot,
  bedroll, supply table + crate).
- Audit integration: `RunFirstLevelWorldAccessAudit` now snaps doors OPEN
  (`SetDoorOpenInstant`) before the doorway-clearance sweeps and restores them
  closed — doors must be OPENABLE and the opened doorway clear.

## 3. Ambient wind (environment physics)

`ACodeRescueWindSwayManager` (`CodeRescueWindSway.{h,cpp}`), spawned once per
world: registers trees/bushes near the player (mesh-path match on the Nature
kit or `WindFoliage` tag, 96-actor cap, 3 s re-scan), flips them Movable once,
and drives gusting sway — layered slow gust envelope × per-actor phase,
±1.5° trees / ±2.6° bushes at 30 Hz. Tunable/off via `cr.WindStrength`
(default 1.0). Character HAIR wind was evaluated and deferred: the hero wears
a crop + beard (no long hair), and cloth-sim on the 17-bone rigs is not
worth the risk budget tonight — noted as future work.

## 4. Ground unification (Kenny's screenshots)

His photos show two defects: walkable surfaces at DIFFERENT heights meeting in
hard steps, and a blinding white plaza slab that reads as "different ground".
Root cause found: the old audit measured actor PIVOT spread (8 uu) while
walkable TOPS disagreed by up to 18+ uu (thick vs thin slabs), and nothing
enforced agreement between the street kit, painted plazas, and decor layers.

`UnifyFirstLevelGroundTops()` (GameMode; runs 1.2 s after every world spawn
AND before the access audit; idempotent):

1. **Street kit phase**: road-tile top median = the datum. Every driving
   surface (roads, crosswalks, painted street blocks) snaps its TOP to
   datum (+0.4 layer epsilon); sidewalks snap to their own group median —
   curbs are street design, not defects.
2. **Slab sweep**: every other large thin slab in the city-0 arena snaps its
   top to the datum (0.75–60 uu deltas); >60 uu offsets are intentional
   set-pieces, logged loudly (4 in the first level, e.g. the −611 uu
   underpass piece).
3. **Plaza toning**: large thin Concrete-textured slabs get a dim
   asphalt-grey MID (Roughness 0.92) — kills the blinding white in his night
   shot. (2 slabs toned, including the 900×700 entry plaza.)

Audit upgraded: `driving_TOP_spread` (≤ 8 uu gate) now measures what the
player perceives as "the ground": **0.40 uu after unification** (was 18.00).
Pivot-spread gate relaxed to ≤18.5 uu because sidewalk curbs legitimately
dominate it.

## 5. Blender / pipeline lessons (new)

1. **Vertex-deletion carving erodes one face-ring past every boundary** — the
   beard's entire front vanished. Carve shells by FACE CENTERS, then prune
   loose verts (`shell_from_head` rewrite).
2. Radial inflation of a shell SAGS over the v2 pipeline's FLAT face plane —
   the head poked through the beard. Push front-region verts forward
   (`front_push`) after radial inflation.
3. A carved "lip window" at this quantization reads as a hole, not a mouth —
   full beard coverage + a small proud lip part reads better.
4. `-run=pythonscript` still cannot import FBX; the full-editor
   `-ExecutePythonScript` flow from pass 3 was reused for the hero drop.
5. The integrated harness needs `-VisualReviewStart -Unattended` to bypass the
   launch language gate (recovered from the .200 test log's command line).

## 6. The SINGLE integrated test run

`Scripts/run_v4_integrated_test.sh` → the game's own
`-FirstLevelIntegratedAcceptanceAudit` harness with all pass-3 + pass-4
systems live (log preserved at
`TestLogs/IntegratedV4_AllSystems_2026_07_11.log`):

```
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1
population=1 characters_grounded=1 sky=1 day_period=1 challenges=1
alternate_solution=1 guidance=1 progression=1 supplies=1 target_lock=1
combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1
overlay_passthrough=1 crafting=1
```

Supporting evidence in the same run: hero presentation active; 6 door leaves
functional (opened for the clearance sweeps, restored); driving_TOP_spread
0.40; 2 plazas toned; wind manager spawned; V3 authored zombies with 16/16
physics bodies; 0 fatals, 0 asserts, 0 LogMaterial warnings. It took four
integration cycles to get here — the failures each cycle (Manny animation
contract vs hero body, door-leaf vs clearance sweep, crosswalk offsets,
double-processing between unify phases) are exactly what the single-run
requirement exists to catch.

## 7. Packaged verification

(see commit + TestLogs)

## 8. Knobs / rollback

- `cr.WindStrength 0` — wind off.
- Doors: delete `FirstLevelAccessDoorLeaf`-tagged actors or set AutoCloseSeconds.
- Ground unification: tag any actor `GroundUnifyExempt` to pin its height;
  the pass only ever moves slabs ≤ 60 uu and logs every adjustment.
- Hero: remove/rename the CharactersV4 folder → Manny visuals return automatically.
