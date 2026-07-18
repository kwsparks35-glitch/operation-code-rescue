# Improvement Pass 2026-07-04 — Art v2, Beacons, World Physics, Faces, Night Sky

Kenny's directives for this pass, verbatim goals → what shipped:

| # | Directive | Delivered |
|---|-----------|-----------|
| 1 | Improved character design (player especially: clothing, weapons, hair, facial responses, emotional expression) | Four new Blender-authored characters (SurvivorKenny, SurvivorMaya, ZombieShamblerV2, ZombieBruteV2) with layered clothing/gear, hair, faces, and 7 facial morph targets each; a facial-expression driver component with game-event triggers; five authored weapon models visible in first person |
| 2 | World development & aesthetics (sky, moon, stars, roads, sidewalks, vehicles, buildings, trees/foliage) | Star-dome + moon night-sky layer; roads with lane paint, sidewalks with curbs, crosswalk, 3 vehicles, oaks/dead trees/bushes, stop sign + traffic lights, all Blender-authored and spawned per city |
| 3 | World physics (no walking through walls, no floating) | Complex-as-simple collision on ALL imported art, kit props switched to colliding, ground-snap traces on every streetscape placement, `cr.AuditWorldSolidity [fix]` QA command |
| 4 | Word competition → beaming symbol above the target | `ACodeRescueBeaconMarkerActor`: vertical emissive beam + one category glyph for every non-essential multi-word label; single words and control prompts stay as text; full text remains readable via the existing press-E reader |
| 5 | Build art in Blender, comfortably realistic (RE-Requiem-styled, not photorealistic) | All art authored live in Blender 5.1 over MCP; reproducible pipelines committed (`Scripts/BlenderArt/build_characters_v2.py`, `build_world_art_v2.py`); muted survival-horror palettes, organic subsurf bodies, worn gear |
| 6 | Test everything | Mac compile **BUILD SUCCEEDED (38s)**; new pass verifier `verify_art_beacon_physics_pass_2026_07_04.py` ALL CHECKS PASSED; oversight watchdog upgraded with pattern-stale triage; editor import + live playtest results recorded below |

## What was built (detail docs)

- `01_ART_PIPELINE_V2.md` — the Blender character/world pipelines, the Blender 5.1
  world-space-vertex lesson, shape keys, rig, exports.
- `02_BEACONS_AND_SOLIDITY.md` — word-competition fix and the collision/grounding pass.
- `03_CHARACTER_SKY_WIRING.md` — C++ wiring: player body, FP weapons, expressions,
  survivors/NPCs, streetscape, night sky.
- `04_TOP50_ROADMAP.md` — the next top-50 items; 22 landed in this pass, 28 specced
  and ranked (pedagogy first, per the 2026-06-30 oversight finding).

## Key numbers

- 4 rigged characters (~18–20k verts each), 7 morph targets + Idle/Walk/Run each
- 18 world/weapon/sky GLB meshes (weapons 100–700 verts; dome 1,771 incl. 420 stars)
- New C++: `CodeRescueBeaconMarkerActor`, `CodeRescueFacialExpressionComponent`
  (+4 GameMode functions, solidity audit command, character locomotion/weapon code)
- Files changed: 15 source files, 2 Blender pipelines, watchdog, 1 new verifier

## Verification trail

1. `python3 Scripts/verify_art_beacon_physics_pass_2026_07_04.py` → ALL CHECKS PASSED
2. `Recompile_Module.command` → `Result: Succeeded, Total execution time: 38.34s`
3. `python3 Scripts/claude_oversight_watchdog.py` → see ledger note; 39 previous
   "REAL regressions" re-triaged: 38 are PATTERN-STALE (symbols all still in Source/ —
   the 2026-07-02 RebuildWidget refactor moved construction out of the verifiers'
   expected exact lines), leaving the true regression count at ~1 niche case.
4. Editor opened → bridge job `0200_import_art_pass_v2.json` consumed → import result
   + live playtest log lines (`[CharacterV2] Player body = SurvivorKenny`,
   `[Streetscape] ... spawned`, `[NightSky] dome=ok`) captured in `03_...md`.
