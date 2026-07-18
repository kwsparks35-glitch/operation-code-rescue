# July 11 Packaged-Report Fix Pass (Part 2)

Date: 2026-07-11 (afternoon — follows the morning regression-repair pass)
Reporter: Kenny, playing the packaged Mac app (bundle 51494982.0.198), with two
terminal screenshots and one gameplay screenshot as evidence.

Four issues were reported, all four root-caused against the exact evidence,
fixed, and re-verified (compile → watchdog verifiers → single integrated
editor run → repackage → packaged verification). This document is the
implementation and root-cause record; test evidence lands in `TestLogs/` and
the results table at the bottom is filled in as each gate completes.

---

## Issue 1 — "Many characters are floating above the ground"

### Root cause

Character grounding was five divergent per-class code paths with no
post-geometry pass:

1. **Ordering bug.** `GroundFloatingMeshes` (the elevation autofix from commit
   `12b5380`) LOWERS walkable slabs by up to ~400 uu — but it runs AFTER
   every character self-snapped in `BeginPlay`
   (`CodeRescueGameMode.cpp` SpawnCityContent: survivors spawn at 16273,
   zombies 16386, NPCs 16446; the flatten ran at 16460). Anything standing on
   a moved slab was stranded in mid-air.
2. **Gravity-less actors.** `ASurvivorActor`, `AFriendlyNPCActor`, and the
   decorative civilians are plain `AActor`s — no capsule, no
   CharacterMovement, no gravity. One missed trace = a permanent float.
   Companions and decorative civilians had NO snap at all (civilians frozen
   at hardcoded spawn offsets of +92/+100).
3. **Weak snaps.** The old per-class BeginPlay snaps traced `ECC_Visibility`
   only, silently no-op'd on a miss, and would happily snap onto
   fall-recovery catch floors or other characters — all problems the player
   recovery path (`RecoverToCityArena`) had already solved for the player.

### Fix

- **`ACodeRescueGameMode::SnapCharacterBaseToGround`** (new, static,
  `CodeRescueGameModeSpawning.cpp`): one robust shared snap — traces
  `ECC_Visibility` then `ECC_WorldStatic`, re-traces through catch floors /
  pawns / other characters (up to 4 rejects), rejects hits below a caller
  `MinGroundZ`, computes the base from capsule half-height (`ACharacter`) or
  the lowest **visible mesh** bounds (AActor characters — lights excluded so
  attenuation spheres can't inflate the base), optional fallback snap to the
  city plane on a total miss.
- **`ACodeRescueGameMode::GroundSpawnedCharacters`** (new): runs at the END
  of city construction — after `GroundFloatingMeshes` and the canopy trims —
  and re-grounds every zombie (incl. boss), survivor, friendly NPC,
  companion, and decorative civilian in the city, then runs a verification
  sweep: any character whose base still hangs > 25 uu above the surface
  under it is counted and logged. Marker:
  `[CharacterGroundingAudit] COMPLETE PASS city=… characters=… regrounded=… floating_after=0`.
  City 0 pass sets the `FirstLevelIntegratedCharacterGroundPass` tag, which
  the integrated audit now REQUIRES (`characters_grounded=1`).
- **Per-class BeginPlay snaps** (zombie / survivor / NPC) replaced with the
  shared helper; **companion** gained a snap (covers runtime spawns:
  reinforcements, rescue companions). Decorative civilians are tagged
  `DecorativeCivilian` at spawn so the pass can find them.

Files: `CodeRescueGameMode.h`, `CodeRescueGameModeSpawning.cpp`,
`CodeRescueGameMode.cpp` (call site), `CodeZombieActor.cpp`,
`SurvivorActor.cpp`, `FriendlyNPCActor.cpp`, `CompanionActor.cpp`.

---

## Issue 2 — "I still cannot click on any of the selections within the pause screen (e.g. Save Now)"

### Root cause

**The damage-feedback overlay was eating the clicks.**
`UCodeRescueDamageFeedbackWidget` (blood vignette + four hit chevrons) is
mounted at **Z-order 50** (`CodeRescueCharacter.cpp:891`); the pause menu
mounts at **Z-order 40** (`CodeRescueCharacter.cpp:5835`). The overlay's
full-screen `UImage` vignette and `UBorder` chevrons never set a visibility
mode, and UMG child widgets default to `Visible` — which is HIT-TESTABLE.
An invisible (alpha 0) but hit-testable full-screen image above the pause
surface consumed every mouse press before Slate could route it to the pause
buttons. This exactly matches the observed symptoms:

- No pause button reacted to clicks (no hover highlight either).
- Keyboard (Esc / P) kept working — key events route by focus, not by
  hit-testing.
- The coding-terminal widget's buttons DID work — it mounts at Z-order
  50/500 LATER than the damage overlay, so it sat above the overlay.
- The July-10 "pause_mouse" audit kept passing — it only verified button
  BINDINGS and the widget's own flags, and the pointer-route test invoked
  the widget's handler directly, bypassing global Slate z-order hit-testing.

### Fix

- The damage overlay and every child (vignette + 4 chevrons + root canvas)
  are now `ESlateVisibility::HitTestInvisible` — a pure visual overlay must
  never participate in hit testing.
- New static contract `UCodeRescueDamageFeedbackWidget::IsPointerPassthroughSafe()`;
  the pause widget's `AuditMouseInteractionContract` now FAILS loudly
  (`[PauseMouseAudit] blocking_overlay=DamageFeedback …`) if the overlay is
  ever hit-testable again. The audit marker gained `overlay_passthrough=1`.

Files: `CodeRescueDamageFeedbackWidget.h/.cpp`, `CodeRescuePauseWidget.cpp`.

---

## Issue 3 — "Terminal interpretations are entirely too literal; accept anything that would function correctly"

### Root cause (matched to both screenshots)

The in-engine static validator (`UCodeRunnerLibrary::ValidateInEngine`)
pattern-matched ONE literal spelling per concept:

- **Palindrome** (screenshot 1): the check "Compares characters via equality
  OR two-pointer indices" required `==` (or `.equals`/`strcmp`/`isequal`) or
  a regex demanding a **postfix** decrement. Kenny's correct solution used
  `!=` with **prefix** `++left; --right;` — rejected.
- **Binary search** (screenshot 2): the check "Computes a midpoint as
  (lo+hi)/2" required the literal `(a+b)/2` shape. Kenny's correct —
  and objectively superior, overflow-safe — `low + (high - low) / 2` was
  rejected.

### Fix — acceptance by functional FAMILY, not spelling

`CodeRunnerLibrary.cpp` `ValidateInEngine` now normalizes to a compact
lowercase form and accepts every common correct family per archetype:

| Archetype | Newly accepted families |
|---|---|
| palindrome | `!=` mismatch tests, `~=`, `.compare`, `.equalsIgnoreCase`; two-pointer with ANY inc/dec spelling (prefix/postfix/`+=1`/`-=1`, any order); reverse-then-compare builtins |
| binary search | overflow-safe `lo+(hi-lo)/2`, `>>1`, `//2`, `len(a)//2` slice recursion, `midpoint/floor/fix/idivide/median` helpers; bound updates with ANY midpoint variable name, half-open `hi = mid`, slice recursion `a[mid+1:]`; `compareTo` comparisons |
| sum | step-wise accumulation (`t = a + b; t += c`), `sum(...)` builtin, any 2+ additions |
| lock | nested-if conjunction, ternary, MATLAB/bitwise `&`; the "unconditionally returns true" anti-check now only fires when NO conditional logic exists |
| reverse | Python `reversed()`, prepend-accumulation (`out = c + out`) |
| fizzbuzz | case-insensitive `'fizz'/'buzz'` in any quote style, `rem()` |
| filter | `for(`/`while(` without a space, `foreach`, `.where(`, bitwise even-tests (`&1`, `bitand`) |
| linked list | recursive traversal (function calls itself) |
| all | `return(x)`, print/printf/cout/disp output, result-variable assignment count as producing output |

Anti-trivial protections (unchanged-starter gate, placeholder-return gate,
linear-scan guard, hardcoded-FizzBuzz guard) are all preserved.

**Regression pinning:** the first-level challenge audit now validates a
table of eight alternate-solution vectors — including Kenny's two exact
rejected submissions verbatim — across the archetypes
(`[FirstLevelAlternateSolutionAudit] COMPLETE PASS vectors=8/8 …`). The
audit fails the whole challenge gate if ANY vector is rejected.

Note for absolute behavioral truth: the game retains the opt-in REAL
execution tier (`-AllowExternalCodeValidation` / CVar
`CodeRescue.AllowExternalCodeValidation=1`) which compiles and runs the
declared tests with clang/clang++/python3. The static tier stays the
default for public-release safety, but is now family-general.

---

## Issue 4 — "Cannot enter the last coding challenge; the game directs me to the area but refuses entry"

### Root cause

**A city-scale coordinate mismatch in guidance — the final station itself was
always fine.** The ten stations spawn on a 5×2 grid of UNSCALED inner
offsets around a scaled hub, but the HUD objective marker
(`CodeRescueHUDWidget.cpp:1055`), the navigation strip, and the in-world
objective beacon (`CodeRescueGameMode.cpp:15212`) all pointed at FIXED,
fully-scaled offsets. At city scale 2.0 the marker landed ~8 m off station 1
but **~23 m from station 10** — the back-corner grid cell — on empty ground.
After stations 1–9 were solved (and hidden), the player followed the marker
to nothing: the crosshair prompt correctly said "No [E] target in reach",
and the 9000-uu proximity-assisted E grabbed the nearby hidden SECRET bonus
terminal instead of the real station 10 standing ~23 m away. Station 10
spawns correctly, opens correctly when actually targeted, and its
`boolean_lock` finale is a valid, solvable challenge in all six languages
(the 60-combination validator audit covers it).

### Fix

- **HUD objective + navigation strip** now resolve the ACTUAL next-unsolved
  `ACodingTerminalActor` by challenge id and target its world location
  (fixed offset retained only as a fallback while the city is spawning).
- **Objective beacon**: `ResolvePhaseTargetLocation(ObjectivePhaseTerminal)`
  re-resolves the next-unsolved terminal actor every tick
  (`ResolveActiveTerminalTargetLocation`), so the beacon walks the player
  station-to-station through all ten, ending AT station 10.
- **E-assist preference**: in `FindNearestInteractable`, `secret_*` bonus
  terminals now carry a 6× distance penalty so the required campaign station
  wins whenever both are in assist range (standing directly at the secret
  terminal still opens it).
- **New guidance-coherence audit** in `RunFirstLevelChallengeAudit`:
  `[ObjectiveGuidanceAudit] COMPLETE PASS next_id=… next_station_resolves=1
  final_station_exists=1 final_station_briefed=1` — asserts the next-unsolved
  id resolves to a physical terminal and that the FINAL station exists with a
  real title + brief. Feeds the challenge gate (`guidance=1`).

Files: `CodeRescueHUDWidget.cpp`, `ObjectiveFocusBeaconActor.h/.cpp`,
`CodeRescueCharacter.cpp`, `CodeRescueGameMode.cpp`.

---

## Verification gates (filled in as they complete)

| Gate | Result |
|---|---|
| UBT compile (CodeRescueUnrealEditor Mac Development) | **PASS** — clean, 0 errors (40s full, 12s incremental) |
| Oversight watchdog (`Scripts/claude_oversight_watchdog.py`) | **PASS** — 129 pass / **0 real regressions** / 27 auto-classified pattern-stale / 14 env-only (`TestLogs/WatchdogFixPass_2026_07_11.json`). 12 stale verifier expectations were refreshed to the current shipped design (see "Verifier refresh" below); one dangling setting (aim-assist slider) was re-wired into the auto target lock for real. |
| Single integrated editor run (all four fixes in one session) | **PASS** — `TestLogs/FirstLevelIntegratedFixPass_2026_07_11.log`: `[CharacterGroundingAudit] … characters=135 regrounded=7 floating_after=0`, `[PauseMouseAudit] … overlay_passthrough=1`, `[FirstLevelAlternateSolutionAudit] … vectors=8/8`, `[ObjectiveGuidanceAudit] PASS`, `[FirstLevelChallengeAudit] … validators=60/60 alternate_solutions=1 guidance=1`, `[FirstLevelIntegratedAudit] COMPLETE PASS` (all 21 tokens), log scan clean (2 allowed warnings only) |
| Repackage (`Package_Mac_App.command`) | **PASS** — fresh paks (Jul 11 15:38), `codesign --verify --deep` valid, bundle version **51494982.0.199** (`TestLogs/PackagingBuildFixPass.log`) |
| Packaged-app verification run | **PASS** — `TestLogs/PackagedFirstLevelIntegratedFixPass_2026_07_11.log`: identical full-audit sweep from the shipped binary (`characters=134 regrounded=7 floating_after=0`, `vectors=8/8`, guidance PASS, `[FirstLevelIntegratedAudit] COMPLETE PASS`, 0 fails); `Smoke_Test_Packaged_App.command render` normal-launch smoke passed; both packaged logs pass the warning scan (allowed diagnostics only) |

### Verifier refresh (watchdog hygiene)

The suite carried 12 REAL-classified failures whose expectations referenced
designs replaced by the (uncommitted) July 9–11 passes: renamed widget names
(`BespokePauseBackdrop` → `ArmoryPauseBackdrop`, pause frame → field armory),
the deliberately removed blocking "CODE ACCEPTED" reader, the SolvedRoute city
tag suffix, radio's stoppable `SpawnSound2D`, the catch-floor confinement
globals, the lowered entry anchor Z, the armory pause labels, the pavilion
rename, the survivor clearance gate, and the target-lock-era combat-juice call
forms. Each verifier was updated to pin the CURRENT shipped design with an
inline `2026-07-11 pattern refresh` note — contract intent unchanged.
One was a genuine dangling behavior, restored in code rather than papered
over: the Settings **aim-assist slider** now scales the auto-target-lock
acquisition cone and reach (0.0 disables lock — accessibility opt-out) in
`ACodeRescueCharacter::UpdateAutoTargetLock`.

## Issue 5 (same evening) — "The game immediately closes; it refuses to open for continued play"

Reported after considerable self-progress on `.199` (New York fully cleared —
all ten stations, survivor, warden — and deep into Los Angeles across three
language tracks). Four macOS crash reports (16:56–17:02) all showed the same
signature: `EXC_BAD_ACCESS` from an engine assertion inside
`UPhysicalAnimationComponent::UpdatePhysicsEngineImp` during world tick.

### Root cause

A physical-animation lifecycle hole, exposed by resuming a REAL mid-combat
save. `ACodeZombieActor::ResetPhysicalAnimationHitReaction` (end of every
hit reaction) and the death paths tear down the mesh's body instances
(`SetCollisionEnabled(NoCollision)`, ragdoll profile swap/detach) while the
`UPhysicalAnimationComponent` REMAINED BOUND with live drive data. The next
time the component's physics-engine update was re-armed, the engine indexed
`SkeletalMeshComponent->Bodies[ChildBodyIdx]` against a shorter/empty Bodies
array → `Assertion failed: (Index >= 0) & (Index < ArrayNum)` → instant exit.

On Kenny's resume, his save put the player inside an active swarm: a
spawn-adjacent zombie died on FRAME 2 (hit reaction + death ragdoll in the
same frame), and the assert fired on FRAME 3 — before the world was even
visible. That is the "immediately closes" experience. Fresh boots never hit
it because nothing dies that early without restored mid-combat state; the
2026-07-04 fix had guarded the mesh-SWAP flavor of this crash but not the
reset/death flavors.

### Why it was invisible to the shell harness at first

Finder-launched, the app reads its container save path
(`~/Library/Containers/com.operationcoderescue.CodeRescueUnreal/…`);
shell-launched test runs read `~/Library/Application Support/Epic/…`, which
was EMPTY — so every automated run booted save-less and clean. The
reproduction required staging his real saves into the shell path.

### Fix (all belt-and-braces, `CodeZombieActor.cpp`)

1. `ResetPhysicalAnimationHitReaction` now fully DETACHES the component
   (`SetSkeletalMeshComponent(nullptr)`) before disabling mesh collision;
   the next hit re-binds through the validated
   `BindPhysicalHitReactionComponent` path after recreating bodies.
2. `DisableGameplayCollisionForDeath` detaches explicitly and stops the
   component's tick — a dying zombie never re-binds.
3. Per-tick failsafe in `UpdatePhysicalAnimationHitReaction`: an active
   reaction with an empty `Bodies` array settles and detaches instead of
   ever reaching the engine assert.

### New reproduction / regression harness

`-CodeRescueAutoResumeLanguage=<Java|C|CPlus|Cpp|Python|MATLAB>` — drives the
EXACT packaged resume path (language gate → `ResumeLanguageRun` → `OpenLevel`)
with no keyboard. With the real save staged this reproduced the crash 100%
(frame-3 assert) before the fix and survives indefinitely after it. The
static contract (`verify_iteration_regressions_pass_2026_07_11.py`, now
25/25) pins the detach sites, the empty-bodies failsafe, and the harness.

### Crash-fix verification

| Gate | Result |
|---|---|
| Editor resume repro, real save (pre-fix) | CRASH reproduced deterministically — frame 3, Array.h:1095 assert (`TestLogs/ResumeCrashRepro_2026_07_11.log`) |
| Editor resume, real save (post-fix) | **PASS** — alive through soak, 377 corpse lifecycles through the previously-fatal path, 0 asserts (`TestLogs/ResumeCrashRepro_AfterFix_2026_07_11.log`) |
| Repackage | **PASS** — bundle **51494982.0.200**, signature valid |
| PACKAGED resume, real save (post-fix) | **PASS** — alive 3+ min mid-swarm, 374 corpse lifecycles, 0 asserts (`TestLogs/PackagedResumeAfterFix_2026_07_11.log`) |
| PACKAGED full integrated audit (new package) | **PASS** — all 21 tokens incl. characters_grounded/guidance/overlay_passthrough (`TestLogs/PackagedIntegratedAfterCrashFix_2026_07_11.log`) |
| Normal-launch smoke | **PASS** |
| Watchdog | **PASS** — 129 pass / 0 real regressions (`TestLogs/WatchdogAfterCrashFix_2026_07_11.json`); the `.198` bundle-version pin was refreshed to a floor check so repackaging can't stale it again |

Kenny's saves were backed up untouched to
`Operation_Code_Rescue/SaveGames_backup_2026-07-11_1720/` before any
investigation; his container saves were never modified.

## New / changed audit markers

- `[CharacterGroundingAudit] COMPLETE PASS … floating_after=0` (new)
- `[PauseMouseAudit] … overlay_passthrough=1` (extended)
- `[FirstLevelAlternateSolutionAudit] COMPLETE PASS vectors=8/8 …` (extended from 1 vector)
- `[ObjectiveGuidanceAudit] COMPLETE PASS …` (new)
- `[FirstLevelChallengeAudit] … alternate_solutions=1 guidance=1 …` (extended)
- `[FirstLevelIntegratedAudit] COMPLETE PASS … characters_grounded=1 … guidance=1 … overlay_passthrough=1 …` (extended)
