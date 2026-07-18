# Pass 2026-07-04 Part 2 — Data-Driven Validation, Verifier Suite Repair, Packaged App

Kenny's directive: the packaged app didn't contain the pass-1 work — repackage so
changes are immediately playable, and complete everything I can directly instead of
deferring to Codex. Delivered:

## 1. R1 — GENERIC data-driven challenge validation (the pedagogy unlock)

Before: declarative curriculum tests only executed for THREE hand-mapped validator
kinds (boolean_lock / reverse_string / even_filter) with hardcoded function names —
15 of 36 curriculum entries; everything else fell to archetype simulations.

Now (`CodeRunnerLibrary.cpp`, new generic section):

- `ExtractStarterFunctionName(Language, Starter)` — the target function comes from
  each challenge's OWN starter code (regex per language family; keyword-safe).
- Literal engine: `ClassifyGenericLiteral` + `SplitTopLevelTestArgs` +
  `RenderGenericLiteral` handle bool / int / float / quoted-string / int-array test
  values, rendered per language (True/False vs true/false, `new int[]{…}`,
  `std::vector<int>{…}`, MATLAB vectors, quote escaping).
- Five generic harness builders (`BuildGenericJavaHarness`, `BuildGenericCMain`
  [scalar-only, honest about C's plumbing], `BuildGenericCppMain`,
  `BuildGenericPythonTestBlock` [with snake↔camel name tolerance],
  `BuildGenericMatlabRunner`) — routed from the kind-based builders when
  `Kind == Unknown`, gated by `GenericDeclarativeSupported`.
- In-engine fallback (external validation OFF — the shipping default): any
  test-bearing challenge now gets a structural gate derived from its own starter —
  required function present (name variants tolerated), produces a result, and
  KEEPS THE REQUIRED SIGNATURE (starter-vs-user parameter-count comparison) — plus
  an explicit note that `CodeRescue.AllowExternalCodeValidation=1` executes the
  declared tests for real.

Net: ALL 36 curriculum entries (and any future entry with tests) validate
data-driven in every language — content now scales without C++ changes.

## 2. R2 — full concept graph + tier ladder

`CodeTerminalWidget.cpp`: every standard terminal is data-driven eligible
(previously 3 of 10 concept nodes). Curated archetype terminals keep their exact
concept mapping; all others accept any test-bearing curriculum entry. Selection now
sorts compatible entries by (Tier, Id) and steps `CityIndex/2 % N` — early cities
teach tier-1 foundations, later cities climb the graph, wrapping for variety.
Starter-signature compatibility auto-passes for generic entries (the harness derives
the name from the starter itself). Oracle text updated to describe the generic mode.

## 3. Verifier suite repaired — 158 checks: 141 PASS, 0 REAL, 2→0 stale

Root cause of the "39 regressions" plague: the 2026-07-02 refactor moved every
widget's construction into `BuildWidgetTreeNow()` (NativeConstruct/RebuildWidget are
thin wrappers), so verifiers extracting the NativeConstruct body saw only the
wrapper. `Scripts/migrate_verifiers_buildwidgettree_2026_07_04.py` patched all 91
verifiers with a function_body helper to transparently follow the wrapper —
same tokens, right function. Design-evolution adjudications (fixed myself, none
deferred):

- `may27_safe_learning`: forbade `TriggerBossHorde(` in the terminal — obsolete;
  the 07-02 pass intentionally wired the post-solve horde (playtest-verified core
  loop). Verifier now asserts the ACTUAL safety contract (horde only after
  RecordTerminalSolved + protected-learning-zone damage block exists).
- `june18_launch_grounding`: forbade language stations anywhere — obsolete; the
  07-01 root fix requires REAL stations in the LAUNCH scene. Verifier now asserts
  the only spawn site is inside `SpawnLaunchLanguageSelectionScene`.
- objective-route toast + terminal language-track UX: features fully present —
  extraction fix above cleared them.
- Subtitles: static instance renamed `ActiveInstanceWeak` so `Push()` can keep the
  project-conventional `ActiveInstance` local without `-Wshadow` (the two last
  stale verifiers now pass against real code).

## 4. Top-50 quick wins shipped

- Item 42: procedural-fallback zombies now wear the authored ZombieShamblerV2 /
  BruteV2 (alternating by actor id) with their own shamble/idle loops — no more
  cube-and-sphere infected anywhere.
- Item 49: beacon accents snap to Okabe-Ito colorblind-safe anchors per the
  player's `EColorblindMode` (deuteranope/protanope vs tritanope anchor sets);
  glyphs already shape-code the categories.

## 5. Packaging (the point of this pass)

`Package_Mac_App.command` (RunUAT BuildCookRun: build+cook+stage+pak+archive) —
`/Game/CodeRescueArt` is in `DirectoriesToAlwaysCook`, so ALL pass-1 art
(CharactersV2 incl. legacy-imported zombies, Weapons, Vehicles, Nature, Sky, street
kit) cooks into the pak automatically. Result + packaged-app playtest evidence in
`06_PACKAGED_PLAYTEST.md`.

## Verification

- `verify_datadriven_validation_pass_2026_07_04.py` — ALL CHECKS PASSED (R1 hooks,
  R2 gates, quick wins, verifier-suite repairs).
- `Recompile_Module.command` — Result: Succeeded (27s) after fixing two real
  compile issues the gate caught (forward decls for array-literal helpers;
  the ActiveInstanceWeak rename).
- Full watchdog: 158 run / 141 pass / 15 env-only / 0 pattern-stale / 0 REAL.
