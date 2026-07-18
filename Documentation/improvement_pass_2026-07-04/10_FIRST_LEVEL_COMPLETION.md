# 10 — First-Level Completion Pass (2026-07-05 → 07-06)

Kenny's directive: *"emphasize your attention specifically toward the absolute
completion of the first level, including all relevant pedagogical influence,
aesthetic and physics development, and all coding challenges… do not stop your
work until everything has been perfected, at least, within this first level.
Once complete, please repackage the app."*

Method: **live playtest of city 01 (New York, Java track) drove every fix.**
Two full playtest cycles were run in the editor `-game` build at 1310x780 —
one to find defects on the solved path, one on a wiped save to verify the
fixes against the true first-time-player experience. All engine-code claims
below were observed on screen or in the runtime log, not inferred.

---

## What the first level now is (verified end-to-end)

City 01 serves `t1-ration-split` ("Split the ration crates", Java:
`leftoverRations(total, people)`) through the full teach→predict→apply loop:

1. Spawn on the entry pad (camera clear), tutorial (8 pages, skippable).
2. Guidance trail (cyan strips) leads to the protected coding safehouse.
3. Terminal opens with a coherent screen: city title, fiction, stage line,
   **THIS TERMINAL'S CODING TASK**, locked-track banner, in-engine-validator
   notice, lesson node, starter code already in the editor.
4. PREDICT-IT drill (own first visible test: `13, 4 → ?`): answering
   correctly locks the prediction ("now prove it in code"); telemetry tagged
   `#predict`.
5. Validation is honest: the unchanged starter **fails** ("REPAIR | failed
   checks detected", score 60, tests 0/3); a real solution **passes** (score
   100, "S - First Try", tests 3/3, all structural + anti-trivial checks).
6. Solve consequences fire and were log-verified:
   `[CodeRescueCodingWorldResponse] … revealed solved rescue route.`, horde
   rush at the solved terminal (PhysAnimBind retry guard engaged cleanly, no
   crash), Smile morph, achievement "Hello World!", +2 ResearchPoints, REFLECT
   debrief, reward choice unlocked, save profile written (verified persisting
   across a relaunch: "Terminal passed - rescue survivor team").

## Defects found by the playtest and fixed

### 1. Validation hole — unchanged starter passed (CodeRunnerLibrary.cpp)
The in-engine structural gate accepted the untouched starter (`return 0;`).
Added two anti-trivial checks to the generic branch of `ValidateInEngine`:
whitespace-normalized user code must differ from the starter, and
placeholder-only returns (`return 0;`, `return false;`, `out=0;`…) fail.
**Live-verified both directions** (starter → FAIL; real solve → PASS with the
new checks in the PASSED list).

### 2. Spawn camera collapse (CodeRescueCampaign.cpp)
`GetPlayerStartLocation` pad moved (-3820,-3180) → **(-3170,-2760)**; the old
pad sat close enough to the safehouse wall to collapse the spring arm into
the player's head on frame 1. (`verify_fab_import_and_entry_access.py`
updated to the new pad coordinate.)

### 3. Terminal incoherence (CodeTerminalWidget.cpp `RefreshText`)
Header/title advertised the legacy archetype (`totalPower`) while the lesson
node taught ration-split. `ActiveLearning` is now hoisted and the title, brief
task block, and checklist all derive from the SERVED lesson.

### 4. Code editor squeezed to zero height (`BuildWidgetTreeNow`)
Long briefs collapsed the Fill-sized editor. IDE layout: reading stack lives
in a ScrollBox capped at 280px (`ReadingCap`); editor is Automatic-sized with
a 210px floor (`CodeEditorMinHeight`). Lesson: **a Fill slot in a UVerticalBox
collapses to 0 when Auto siblings exceed the panel; MinDesiredHeight only
affects Automatic slots.**

### 5. Unreachable buttons after a solve — stuck state (07-06)
At 1310x780 a passing validation grew the diagnostics block until COLLECT
REWARD / CLOSE TERMINAL were **pushed below the window edge with no way to
reach them** (Escape also dead, see #6). Three-part fix:
- whole terminal column now lives in `TerminalScroll` (UScrollBox) — every
  control reachable at ANY resolution, inert at fullscreen;
- diagnostics scroll inside a 230px cap (`DiagnosticsScroll` /
  `DiagnosticsMaxHeight`) so results never shove the buttons around;
- `RunValidation` ends with `ScrollWidgetIntoView(OutputFrame)` so the score
  card + buttons land in view.
**Re-verified live on the fresh save: every button visible pre-solve at
1310x780; negative validate auto-scrolled results into frame with the capped
diagnostics showing its own scrollbar.**

### 6. Escape-to-close was dead until the student clicked the widget
`NativeOnKeyDown` had the Escape→close handler, but the widget never took
keyboard focus, so the event never reached it. `SetKeyboardFocus()` now runs
in `NativeConstruct`, at the end of `RunValidation`, and after
`AnswerPrediction` (buttons steal focus on click). Toast already tells the
player "press Esc to close".

### 7. Backspace tutorial-dismiss teleported the player (CodeRescueCharacter.cpp)
The polled tutorial path (Tick) dismisses on Backspace, but the
`BindKey(BackSpace → RecoverToCityArena)` delegate fired independently for
the SAME keypress — a first-time player skipping the tutorial was silently
arena-recovered as their very first input (observed live: "Arena recovery:
returned to 01. New York…" the moment the tutorial closed).
`RecoverToCityArena` now no-ops while `UCodeRescueTutorialWidget::IsShowing()`.

### 8. Recovery point pinches the camera (CodeRescueCharacter.cpp)
The "last safe" breadcrumb can be dropped while hugging geometry; recovering
onto it collapses the spring arm (observed repeatedly). The destination is
now probed at chest height along the four cardinals; anything blocking within
170uu slides the point away (≤240uu), logged as
`[CodeRescueArenaRecovery] destination nudged …`.

### 9. Curriculum-wide lint (Scripts/lint_curriculum_full_2026_07_05.py — NEW)
All 60 entries: unique ids, tiered pedagogy fields present, test literals
parseable, arity consistent, per-language starter extractable, per-language
executability classified (C scalar-only → structural gate). Fixed
`t2-triage-tag` unquoted string outputs. **ALL CHECKS PASSED.**

## Verification ledger

- `Scripts/verify_first_level_completion_2026_07_06.py` — 17/17 PASS.
- Full watchdog: **147 pass / 0 stale / 0 REAL — VERDICT: PASS** (one
  pattern-stale verifier updated: frozen spawn coordinate → new pad).
- Playtest cycle 1 (solved path): predict correct → starter REJECTED →
  solution PASSED (3/3, S-grade) → rescue route + horde + achievement +
  rewards, all on screen or in log.
- Playtest cycle 2 (wiped save, post-fix build): fresh tutorial → terminal
  coherent → predict locked → starter REJECTED with capped scrolling
  diagnostics and reachable buttons at 1310x780.
- Packaged rebuild: `Package_Mac_App.command` → ExitCode=0. Headless packaged
  soak (`-nullrhi -nosound -unattended`, bypass flag): **4m14s alive, 0
  appError/Assertion/Fatal, 0 LogMaterial warnings**, shipped-pak markers all
  fired — `[CityKit] 27/0`, `[Streetscape] 41/0`, `[CodeRescueSafeLearning]
  01 New York Java`, `[CharacterV2] SurvivorKenny`, `[NightSky] ok`,
  `[CodeRescueEliteWardenMiniBoss]` — and Kenny's real container save loaded
  intact (boss_defeated state restored).
  NOTE: the soak ran headless because Kenny was actively using the Mac at the
  time; a windowed spot-check of the packaged app is a nice-to-have next
  session. `[GuidanceTrail]` didn't appear in this particular boot (untouched
  this pass; fired from the pak in build #5; likely save-state-dependent
  spawn ordering) — worth one glance next windowed run.

## Known cosmetic residuals (non-blocking)

- "Validator:" line renders empty for generic entries in the lesson node.
- Distant teal-glass panels + entry-plaza brightness (from 07-02 notes).
- The isometric/top-down cameras make manual navigation awkward for a
  computer-use driver (not a player-facing issue).
