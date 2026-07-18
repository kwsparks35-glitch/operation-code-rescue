# Operation Code Rescue — Checks-and-Balances Protocol

**Owner:** Kenny · **Builder agent:** Codex · **Reviewer agent:** Claude
**Established:** 2026-06-30

## Why this exists

Codex builds this project as a fast loop of "vertical slices." That loop is productive, but it
has no independent reviewer and no hard stop, so three failure modes accumulate silently:

1. **Drift** — a new slice refactors a symbol and quietly breaks an older slice's guarantee.
2. **Self-graded scope** — the quick local-CI command runs only a *curated subset* of the
   verifiers, so the broken guarantee is never re-run.
3. **Static-only proof** — the `verify_*.py` checks confirm *"the code contains the tokens I
   expected,"* not *"the code compiles, runs, plays, or teaches."*

The two agents share **no live channel.** This protocol is the substitute: a file-based set of
gates that Codex reads each loop, that Claude enforces by review, and that Kenny uses to make
go / no-go calls.

## The one command that is the gate

```
python3 Scripts/claude_oversight_watchdog.py
```

It runs **every** `Scripts/verify_*.py` (not a hand-picked list), separates *environment-only*
failures (need the Mac engine / a CLI arg / an external server) from **real regressions**,
performs symbol-drift analysis, reports git commit hygiene, and exits non-zero if any real
regression exists. Run it before declaring a slice done and before any packaging.

## Definition of Done — a slice is NOT "complete" until all four hold

1. **Watchdog green** — `claude_oversight_watchdog.py` reports `0 real regressions`.
2. **Compiles on Mac** — `./Recompile_Module.command` exits 0.
3. **Packaged smoke clean** — the headless packaged run reaches `LogExit: Exiting.` with no
   `Fatal`/`Error` lines (see `Saved/Logs/PackagedSmoke_*.log`).
4. **Committed** — the slice is committed to git with a descriptive message.

"It passes its own new verifier" is **not** sufficient. A new static check that only asserts the
strings you just typed proves nothing about behavior.

## Standing gates

- **G1 — No-Drift.** When you rename or remove a symbol, update or retire **every** verifier that
  referenced it, in the **same slice**. A red watchdog caused by your own refactor is your slice's
  responsibility, not a separate cleanup later.
- **G2 — Full-Suite.** The CI list must contain every verifier, or be replaced outright by the
  watchdog. No self-selected passing subset.
- **G3 — Commit-Per-Slice.** Commit after each slice. Target fewer than 50 uncommitted files at
  rest. A 700+ file uncommitted pile has no rollback point and cannot be bisected.
- **G4 — Curriculum-First.** Before adding another *shell* feature (beacon, telegraph, vehicle,
  menu polish), advance the *teaching core*: more challenge archetypes, real visible/hidden tests,
  broader validator coverage, difficulty progression. The shell already vastly outweighs the
  curriculum; new shell scope widens that gap.
- **G5 — Honest Done.** "Complete all work not yet completed" has no terminal state, so the loop
  can run forever. Track work against an explicit backlog with written acceptance criteria, and do
  not invent scope that isn't on it. When the backlog is empty, stop and report — do not generate.

## Roles

- **Codex (builder):** implements slices; obeys G1–G5; reads `CLAUDE_TO_CODEX.md` each loop.
- **Claude (reviewer):** runs the watchdog, files dated entries in `REVIEW_LEDGER_*.md`, writes
  `CLAUDE_TO_CODEX.md` action notes. Does not edit Codex's `progress.md`.
- **Kenny (owner):** runs the Mac compile + an actual playtest, and makes the go / no-go decision.
  Tools and tests inform that decision; they do not replace it.

## The file channel (how the two agents hand off)

- `Saved/ClaudeBridge/inbox/` — live editor command channel (`<id>.json`, consumed only while the
  Unreal editor is open). Non-`.json` notes left here are safe and ignored by the consumer.
- `CLAUDE_TO_CODEX.md` (repo root) — reviewer → builder action list. Codex should read it first
  each loop and clear it before adding new scope.
- `Documentation/CLAUDE_OVERSIGHT/REVIEW_LEDGER_*.md` — dated review findings, append-only.
