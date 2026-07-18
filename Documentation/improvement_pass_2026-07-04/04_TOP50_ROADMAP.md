# Top-50 Roadmap (2026-07-04) — pedagogy value × game improvement

Ranked by (learning impact × player-visible impact ÷ effort). Items 1–22 SHIPPED in
this pass; 23–50 are specced next steps. Pedagogy items lead the queue per the
2026-06-30 oversight finding (rich shell vs thin teaching core) and the 07-01
Learning Vertical follow-ups (R1/R2).

## Shipped this pass (1–22)

1. DONE — Beacon marker system: every non-essential multi-word world label → beaming
   symbol above its target (glyph + colored light beam), full text on E.
2. DONE — Single-word labels keep their word; control prompts untouched.
3. DONE — v2 player character SurvivorKenny: field jacket, chest rig, gloves, hair,
   real face; replaces the silver mannequin (with graceful fallback).
4. DONE — v2 female survivor SurvivorMaya (medic shell, armband, ponytail) for rescue
   targets + medic/scientist NPCs.
5. DONE — v2 zombies (ShamblerV2 hunched/dragging, BruteV2 massive) with wounds,
   torn clothing, necrotic palettes.
6. DONE — Facial expression system: 7 morph targets per character + driver component
   with envelopes and autonomous blinking.
7. DONE — Emotion triggers: terminal solve → Smile; damage → Grimace; horde → Alarm.
8. DONE — Five authored weapon models (rifle/pistol/shotgun/machete/wrench).
9. DONE — Visible first-person weapon that follows the active weapon selection.
10. DONE — Player body Idle/Walk/Run switching by ground speed.
11. DONE — NPC/survivor idle animations + blinking (no more statue T-poses).
12. DONE — Roads with lane paint down the arrival street.
13. DONE — Sidewalks with curbs; crosswalk at the extraction end.
14. DONE — Abandoned vehicles (wrecked sedan w/ flat tire, delivery van, police
    cruiser w/ emissive lightbar) as solid cover.
15. DONE — Street greenery: oaks, dead trees, bushes.
16. DONE — Stop sign + traffic lights with emissive lamps.
17. DONE — Night star dome (420 stars) that follows the player.
18. DONE — Moon with maria, night-window visibility tied to TimeOfDay.
19. DONE — All kit props now collide (no walk-through lamp posts/kiosks/arches).
20. DONE — Complex-as-simple collision on every imported mesh spawned solid.
21. DONE — Ground-snap traces on all streetscape placement + `cr.AuditWorldSolidity
    [fix]` QA command for floaters/no-collision reporting.
22. DONE — Oversight watchdog PATTERN-STALE triage (38 of 39 "regressions" were
    stale verifier expectations, not lost features).

## Next up (23–50)

23. Data-driven validation (R1): execute each challenge's OWN visible/hidden tests
    from curriculum_database.json instead of ~8 archetype checks. THE unlock for
    curriculum scale. (pedagogy, L)
24. Generic terminal node (R2): wire all 10 concept-graph tiers, not 3. (pedagogy, M)
25. Curriculum 36 → 60 challenges: tiers 6–10 at full 6-language depth. (pedagogy, L)
26. Parsons-problem terminal mode: drag/reorder given lines — lower entry ramp for
    novices, proven pedagogy for syntax load. (pedagogy, M)
27. Predict-the-output drills before run (choice UI reuses reward-choice rows).
    (pedagogy, S)
28. Bug-hunt challenges: given defective code, find+fix the bug (uses the existing
    diagnostics pane). (pedagogy, M)
29. Spaced-repetition scheduler: weak concepts (from telemetry) resurface in later
    cities automatically. (pedagogy, M)
30. Error-message literacy: real compiler stderr shown with a guided decoding strip
    (what failed / where / why / try). (pedagogy, M)
31. Worked-example fading across each tier: full example → skeleton → blank.
    (pedagogy, S — data-side once 23 lands)
32. Per-concept mastery meter in HUD/journal from SummarizeConcept telemetry.
    (pedagogy, S)
33. One-line reflective debrief prompt after each solve (metacognition). (pedagogy, S)
34. Companion "rubber duck": before hints, the companion asks the player to state
    their plan in one sentence. (pedagogy, S)
35. Code-reading rooms: collectible annotated snippets as world lore. (pedagogy, M)
36. Adaptive pressure: hint cadence + horde intensity tied to mastery curve.
    (pedagogy/game, M)
37. Token-palette input mode (tap-to-compose) for accessibility + controller play.
    (pedagogy/UX, L)
38. Pseudocode starter track for absolute beginners (7th "language"). (pedagogy, M)
39. Session report export for teachers/parents (PDF from telemetry.jsonl). (meta, M)
40. In-game glossary: tap any highlighted term in lessons/errors. (pedagogy, M)
41. Melee swing animation + weapons visible on the character's back. (game, M)
42. Zombie hit-reaction morphs (JawOpen/flinch on hit) + ambient v2 zombie variants
    mixed into hordes. (game, S)
43. Enterable jeep visual upgrade using the vehicle kit language. (game, M)
44. Weather-matched sky: cloud layer material states tied to the weather system.
    (game, M)
45. Street decals: grime + guidance trails leading toward active terminals. (game, S)
46. Minimap glyphs matched to beacon categories. (UX, S)
47. Save-slot thumbnails (HighResShot on save). (UX, S)
48. Instanced static meshes (ISM) for streetscape rows — perf headroom for bigger
    dressing. (tech, M)
49. Beacon accessibility: colorblind-safe palette variants + patterned beams (shape
    coding already present via glyphs). (UX/a11y, S)
50. Photo mode (pause + free camera + HUD hide) — cheap delight, great for sharing
    progress with the class. (game, S)

S/M/L = small/medium/large effort. Recommended execution order for the next pass:
23 → 24 → 27 → 31 → 32 → 26 → 29 → 30 (pedagogy spine), interleaved with 42/45/46
as visual breathers.
