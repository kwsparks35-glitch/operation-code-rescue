# Learning Vertical Slice — 2026-07-01

Implements the pedagogy recommendations from `Operation_Code_Rescue_Maximizing_Learning_2026-07-01.pdf`
as real, additive code + data. The first pass was isolated; this pass completes the runtime wiring for
one concept node while keeping the legacy terminal/gameplay path as fallback if the data pack cannot load.

## What shipped

### 1. Curriculum content pack (data) — `Content/CodeRescueData/curriculum_database.json`
- Grew from 6 concept stubs to **36 entries across a 10-tier concept graph** (variables → conditionals →
  loops → arrays → strings → functions → recursion → dictionaries → search/sort → capstones).
- Each new challenge carries: `micro_lesson`, `worked_example`, `prompt`, per-language `starter`,
  `visible_tests`, `hidden_tests`, `misconceptions`, `world_effect`, `post_solve`, plus the fields the
  existing loader already reads. Backward compatible — the legacy terminal challenge still works if the
  data-driven loader cannot resolve a matching node.
- `world_effect` binds the learner's **output** to the rescue fiction (intrinsic integration).
- The tier-4 filter node now has **five playable challenges for every launch language** (`Java`, `C`,
  `C+`, `C++`, `Python`, `MATLAB`) using `language: "All"` plus explicit language starters. This proves
  one concept node end-to-end before attempting the full 465-city curriculum expansion.
- The tier-2 boolean/conditionals node now has **five playable challenges for every launch language**
  using validator-compatible `shouldUnlock` / `should_unlock` signatures. This fixes the old `canOpen`
  data mismatch and gives lock terminals the same data-driven teaching depth as filter terminals.
- The tier-5 strings/indexing node now has **five playable reverse challenges for every launch language**
  using validator-compatible `reverseString` / `reverse_string` signatures, including empty, one-character,
  mixed digit, and case-sensitive boundary coverage.
- The tier-2, tier-4, and tier-5 runtime rows now declare a curriculum `validator` key
  (`boolean_lock`, `even_filter`, `reverse_string`) so terminal selection and validation can scale from
  metadata instead of per-node C++ branches.

### 2. Learning library (C++, additive) — `Source/CodeRescueUnreal/CodeRescueLearning.{h,cpp}`
`UCodeRescueLearningLibrary` (a BlueprintFunctionLibrary) + structs. Blueprint-callable:
- `LoadChallenges` / `SelectChallengeForCity(Language, Tier, CityIndex)` — data-driven selection beyond
  the old 8-value enum, deterministic per city.
- `BuildTeachPayload` — micro-lesson + worked example + prompt + language-resolved starter (teach step).
- `BuildPostSolve` — "why it works" + hidden-test reveal (reinforce step).
- `ShouldOfferScaffold` / `BuildScaffold` — never wall-block: guided fill-in after N failed attempts.
- `GetWorldEffect` — the fiction the solution drives.
- `IsExternalValidationEnabled` — reports whether the real compiler path is on.
- `RecordAttempt` / `SummarizeConcept` — learning telemetry to `Saved/ClaudeLearning/telemetry.jsonl`.

### 3. Runtime terminal wiring — `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`
- Terminal open now selects the data-driven tier-4 filter node for even/filter challenges, the
  tier-2 boolean node for lock challenges, and the tier-5 reverse node for reverse/string challenges.
- Selection now flows through one normalized validator-key path. Legacy terminal ids map to a runtime
  key, curriculum rows provide their own `validator`, and compatible rows are selected by key plus
  selected-language starter support.
- The checklist panel shows the teach -> apply payload: concept line, micro-lesson, prompt, visible test,
  validation mode, telemetry summary, world effect, and scaffold rule.
- For data-driven nodes, the code editor starts from a language-correct exercise skeleton instead of a
  pre-solved answer while preserving the existing starter fallback if the data pack cannot load.
- Validation now receives a runtime `FChallengeSpec` synthesized from the selected curriculum row. The
  selected row's visible and hidden tests are copied into `FChallengeSpec::TestCases`; Java, C, C+, C++,
  Python, and MATLAB external validators generate assertions from those tests for the three wired
  validator families. The in-engine fallback remains static-analysis based and reports the declared
  curriculum test count/test pack.
- Every validation attempt records learning telemetry through `RecordAttempt`.
- Successful validation appends `BuildPostSolve`; repeated repair attempts append `BuildScaffold` after
  the never-wall-block threshold.

### 4. Intrinsic world-effect (C++, cook-safe) — `Source/CodeRescueUnreal/CodeRescueSolveEffectActor.{h,cpp}`
`ACodeRescueSolveEffectActor` — a primitive-only, reduced-motion-aware burst spawned on solve. Its
intensity is parameterized by `OutputMagnitude` (e.g. how many units the player's filter returned), so
the **player's answer drives the world**, not merely the boolean solve. `OnSolveEffectStarted` is a
Blueprint hook for later authored Niagara / sound / camera.

### 5. Solved-route world response — `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `RevealSolvedTerminalRescueRoute` now spawns `ACodeRescueSolveEffectActor` on successful validation
  and configures it with the data-driven `world_effect`, selected-language accent color, reduced-motion
  state, and output magnitude estimated from the active challenge's visible test output.
- Filter solves add a reviewable 3D `DataDrivenFilterNode` layer: kept/rejected lanes, ordered convoy
  units, empty-output feedback, mentor pointing pose proxy, survivor boarding pose proxies, and
  nonblocking physics-safe rails tagged for future art/animation passes.
- Lock solves add a reviewable 3D `DataDrivenBooleanNode` layer: truth-table lamps, paired required
  switch pylons, an opening airlock slab, mentor truth-table pose proxy, survivor exit pose proxy, and
  nonblocking physics-safe rails tagged for future art/animation passes.
- Reverse solves add a reviewable 3D `DataDrivenStringNode` layer: input/output glyph tiles, last-to-first
  transfer beams, a rolling vault door slab, mentor last-to-first pose proxy, survivor unlock pose proxy,
  and nonblocking physics-safe rails tagged for future art/animation passes.

Optional (recommended): flip `CodeRescue.AllowExternalCodeValidation=1` for a **local teaching build**
so real compile/run feedback is live (keep it off for public distribution).

## Honest status / Definition of Done

- **Done here:** data pack (valid, verified), runtime terminal wiring, telemetry, adaptive scaffold,
  post-solve reinforcement, solved-route intrinsic effect spawn, filter/boolean/reverse-node world art/animation/physics
  placeholders, validator-key terminal generalization, curriculum-declared external test harness bridge,
  static verifier (`Scripts/verify_learning_vertical_slice_pass.py`), docs.
- **Mac/editor gate still required:** compile the module and playtest one filter city end-to-end. Per
  `GOVERNANCE_PROTOCOL.md`, the slice is not "done" until it
  compiles, the packaged smoke is clean, and it is committed.
- **Needs an artist/editor (out of code scope):** bespoke terminal-typing animation, rigged survivor/
  zombie meshes, sculpted world art. The effect actor is a readable programmer-art placeholder with a
  Blueprint hook so authored FX can replace it without code changes.

## Verify
```
python3 Scripts/verify_learning_vertical_slice_pass.py
python3 Scripts/claude_oversight_watchdog.py
```
