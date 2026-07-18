# Note for Codex — what Claude shared with Kenny (2026-07-01)

Hi Codex. So you're aware and can align, here is everything I (Claude, acting as reviewer + a
pedagogy implementation pass) have shared with Kenny for game and learning improvement. None of it
competes with your work — it is additive and points the shared effort at the game's core purpose:
teaching students to code.

## Documents shared with Kenny (at the project root, one level above this repo folder)

1. **`Operation_Code_Rescue_Review_2026-06-30.pdf`** — independent review + the checks-and-balances
   system. Key point: the build is real and boots, but "QA passing" is ~90% static string checks;
   run the full watchdog, not a curated subset.
2. **`Operation_Code_Rescue_Maximizing_Learning_2026-07-01.pdf`** — how to maximize learning without
   hurting UX. Core finding: the learning engine is good but starved — 8 concepts drove 465 cities and
   real validation shipped off. Fix = concept breadth + real validation + intrinsic integration.
3. **`Operation_Code_Rescue_Implementation_Report_2026-07-01.pdf`** — what I then built (below).

## What I implemented (additive — please build on it, don't duplicate)

- **`Content/CodeRescueData/curriculum_database.json`** — expanded from 6 stubs to **24 challenges
  across a 10-tier concept graph**, each with micro-lesson, worked example, per-language starter,
  visible/hidden tests, misconceptions, `world_effect`, and post-solve. Your loader already reads the
  shared fields; the richer fields are forward-compatible.
- **`Source/CodeRescueUnreal/CodeRescueLearning.{h,cpp}`** — `UCodeRescueLearningLibrary`:
  teach→apply→reinforce, adaptive scaffold (never wall-block), learning telemetry, data-driven
  challenge selection, and an external-validation-mode helper.
- **`Source/CodeRescueUnreal/CodeRescueSolveEffectActor.{h,cpp}`** — cook-safe effect where the
  player's solution OUTPUT drives the world response (World Deep-Dive §7.1). Blueprint hook for your
  authored FX.
- **`Scripts/verify_learning_vertical_slice_pass.py`** + `Documentation/improvement_pass_2026-07-01/`.

## What would help most next (curriculum-first, per G4)

1. Wire the 4 terminal calls in `UCodeTerminalWidget` (see
   `improvement_pass_2026-07-01/LEARNING_VERTICAL_SLICE.md`) and compile on the Mac.
2. Grow the concept graph further using the schema in `CURRICULUM_EXPANSION_SPEC.md` — more challenges
   per tier per language, with real tests.
3. Turn on `CodeRescue.AllowExternalCodeValidation=1` for a local teaching build so real code is judged.
4. Prefer this learning work over new shell features until the terminal loop is proven on one city.

## Housekeeping

Kenny had the entire working tree committed on 2026-07-01 (your slices + my additions) as a single
checkpoint/rollback point. Going forward, please commit per slice and run
`python3 Scripts/claude_oversight_watchdog.py` before declaring a slice done. Thanks — nice work on
the shell; let's make the teaching match it.
