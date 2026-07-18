# Pass 2026-07-04 Part 3 — Pedagogy Slice (items 25, 27, 32, 33, 46)

Continuing straight down the top-50 spine with the standing rules: everything
tested, everything documented, and the pass ends with a repackage so
`PackagedMac/Mac/CodeRescueUnreal.app` is immediately playable.

## Item 25 — Curriculum 36 → 60 (tiers 6–10 at full 6-language depth)

`Scripts/build_curriculum_tiers6_10_2026_07_04.py` authors and merges 24 new
entries; the distribution is now t1:3 t2:6 t3:3 t4:6 t5:6 **t6:6 t7:6 t8:6 t9:6
t10:6** (+6 legacy language stubs) = 60. Every new entry ships:

- all six starters (python/java/c/cplus/cpp/matlab) generated from a typed
  signature spec (int/float/bool/str/int-array), so the R1 generic harness can
  derive the function and EXECUTE the tests in every language;
- visible + hidden tests, micro-lesson, worked example, prompt, misconceptions,
  strategies, common mistakes, world_effect and post_solve — full teach →
  apply → reinforce loop, not stubs.

Concept coverage by tier: functions/params/return (clamp, unit-rate, mean,
boolean window), recursion (countdown-sum, factorial, fibonacci, power-of-two
halving), maps/sets (anagram, first-duplicate, distinct-count, mode, char
histogram), search/sort (linear search contract, sortedness invariant,
second-largest, count-below-threshold), and complexity capstones (closed-form
vs loop, pair-sum with a seen-set, longest run, one-pass best-gain, pipeline
refactor reverse-words).

**Testing (the important part):** the builder EXECUTES every visible+hidden
test against a Python reference solution before it will merge — all **76 tests
pass**; it also lints each starter for generic-harness compatibility (function
name extractable, literals classifiable) mirroring the C++ gates. Re-running
the script is idempotent (entries keyed by id).

One supporting engine fix: the generic C harness now renders boolean literals
as `1/0` (C starters use int flags; no stdbool dependency in the sandbox TU).

## Item 27 — Interactive predict-the-output drill

The old static "predict before coding" sentence is now an interactive drill in
the terminal (`PredictionDrillRow`): it shows the selected challenge's OWN
first visible test ("PREDICT IT: for input 13, 4 this returns …") with three
choice buttons — the correct output plus two shape-aware distractors (flipped
booleans, off-by-one numbers, reversed arrays/strings, first-element slices).
Choice slots are assigned deterministically per challenge id so replays can't
memorize "always B". Answering locks the row, gives immediate feedback in the
diagnostics strip, and records to the concept telemetry stream as
`<challenge-id>#predict` — so prediction accuracy is analyzable separately
from code attempts. Solved terminals collapse the row.

## Item 32 — Concept mastery meter (journal)

The objective journal now renders `CONCEPT MASTERY (solves / attempts)` — up to
eight bars built from the SAVED `ConceptProgress` array (`##########----`
style, percent + counts), sorted by practice volume, with an explicit
empty-state line before any attempts. Players finally SEE which concepts are
solid and which need reps; it refreshes every time the journal opens.

## Item 33 — Reflective debrief prompt

Every successful (non-practice) validation now appends one rotating
metacognitive prompt to the solve output — "REFLECT (say it out loud or jot
it): …" — five research-flavored variants (key idea, earliest-catching test,
hint-for-a-teammate, scaling pressure, transfer). Deterministic rotation by
challenge hash + session attempts keeps repeats fresh.

## Item 46 — Minimap beacon glyphs

`ACodeRescueBeaconMarkerActor` instances (the beaming-symbol POIs from part 1)
now draw on the minimap as their own violet dot family, so beacons are
navigable from the map, not just by line of sight.

## Verification

- `verify_pedagogy_slice_pass_2026_07_04b.py` — ALL CHECKS PASSED (curriculum
  counts + per-tier depth + 6-starter completeness + drill/meter/debrief/
  minimap hooks).
- Full watchdog after the slice: **160 run / 146 pass / 14 env-only /
  0 pattern-stale / 0 REAL.**
- `Recompile_Module.command` → Result: Succeeded (16s, first try).
- Packaged build + shipped-pak playtest evidence: `08_PACKAGED_VERIFY_PART3.md`.
