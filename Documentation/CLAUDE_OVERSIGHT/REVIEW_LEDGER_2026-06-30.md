# Review Ledger — 2026-06-30 (Claude oversight pass)

First independent review under the checks-and-balances protocol. All findings come from running
Codex's own verifiers and inspecting the source/data; nothing was taken on faith.

## Snapshot

| Metric | Value |
|---|---|
| C++ source files | 104 |
| C++ lines | ~37,800 |
| `verify_*.py` scripts | 62 (was 60 when this review began — loop added 2 mid-review) |
| Slice docs dated 2026-06-30 | 23 |
| `progress.md` dated entries | 82 (back to 2026-05-24) |
| Packaged Mac app | rebuilt today 08:35; headless smoke clean at 08:36 |

## Watchdog result (`claude_oversight_watchdog.py`)

- 62 verifiers run · **48 pass** · **10 environment-only** · **4 real regressions**.
- Environment-only failures need the Mac engine (`unreal` module), a CLI arg, or the Fab MCP
  server. They are expected off-Mac and are **not** defects.

### Real regressions (all are stale verifiers left behind by drift — invisible to the curated CI)

1. `verify_june18_launch_grounding_symbol_pickup_pass.py` — expects `SelectLanguageAndLaunch`,
   which **no longer exists in Source** (today's onboarding slice replaced it with per-language
   `On<Lang>LanguageClicked` / `StartLanguageRun`). Confirmed: 0 occurrences in `Source/`.
2. `verify_may27_safe_learning_city_controls_pass.py` — `CodeRescueMainMenuWidget.cpp` no longer
   contains the old menu tokens (`LEARN C+`, `CHOOSE YOUR TRAINING LANGUAGE`,
   `SelectLanguageAndLaunch(ECodingLanguage Language)`). Same refactor.
3. `verify_may27_gameplay_access_pass.py` — `CodeRescueAIController.cpp` no longer contains the
   exact `MoveDirectlyToward(PlayerCharacter->GetActorLocation()` token the verifier pins.
4. `verify_settings_color_vision_slice_pass.py` — expects a `progress.md` entry for the color-
   vision settings slice that shipped in the last ~30 minutes; the doc/log entry is not yet
   written. Likely a mid-flight slice.

**Interpretation:** none of these necessarily means broken *gameplay*. They mean older
guarantees rotted when newer slices moved on, and because the quick CI list does not run them,
nobody noticed. This is exactly the drift the protocol's G1/G2 gates exist to stop.

## Build evidence (positive)

`Saved/Logs/PackagedSmoke_render.log` (08:36 today): packaged Mac app boots, loads the `Entry`
map, runs, and exits cleanly (`LogExit: Game engine shut down` → `LogExit: Exiting.`), no Fatal
lines. This is a **real, building, runnable project** — not vaporware.

## Process risks

- **744 uncommitted files, 0 commits today**, last commit `f532ba6` (2026-05-02). Weeks of slice
  work sit in the working tree with no checkpoints. Violates G3. Highest-priority process fix.
- **Unbounded loop.** Two new slices shipped during this review. The original prompt ("complete
  all work not yet completed") has no terminal state. Violates G5.
- **Static-only proof.** ~90% of verification is string presence (426 `check(`, 155 `check_all`,
  104 `.exists()`). Behavior is only checked by the Mac headless smoke (boot+exit) and the
  optional local toolchain validator.
- **Shell ≫ curriculum.** `curriculum_database.json` holds **6 concept stubs**; the default-safe
  validator recognizes **~8 challenge archetypes** by keyword. That teaching core backs a
  465-city × 6-language campaign. Real compile/run exists but is **off by default**
  (`CodeRescue.AllowExternalCodeValidation=0`). Violates the spirit of G4.

## Action items for Codex (clear before adding new scope)

- [ ] Fix or retire the 4 stale verifiers (G1). Prefer updating them to the new symbols.
- [ ] Replace the curated local-CI list with `claude_oversight_watchdog.py` (G2).
- [ ] Commit the working tree, then commit per slice going forward (G3).
- [ ] Pivot the next passes from shell features to curriculum depth (G4): add real challenges +
      visible/hidden tests, not more beacons/telegraphs.
