# Item 15 — Run scoreboard & persistent stats

## What changed
Added six persistent counters tracked across every city:
- `KillCount` — zombies neutralized lifetime (counts horde + persistent both)
- `RescueCount` — survivors rescued lifetime
- `TerminalSolveCount` — coding terminals graduated lifetime
- `RunSeconds` — cumulative gameplay seconds
- `DeathCount` — player deaths lifetime
- `HeadshotCount` — confirmed headshot kills

Each counter has an `IncrementXxx()` method on `UCodeRescueGameInstance`
that increments and immediately persists via `SavePersistentRun()`. There
is also `AccumulateRunSeconds(DeltaSeconds)` for the time tracker (no
auto-save on that one — too chatty).

`GetScoreboardSummary()` formats a multi-line summary string suitable for
pasting into the Victory or Death widget.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueSaveGame.h` — added 6 counters +
  `bHasRunScoreboard` backcompat flag.
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h/.cpp` — fields, increment
  helpers, summary formatter, save/load wiring with backcompat default.
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp` — calls
  `IncrementTerminalSolveCount()` on solve.
- `Source/CodeRescueUnreal/SurvivorActor.cpp` — calls `IncrementRescueCount()`
  on rescue.

## Design decisions
- Counters reset on `ResetRun()` (Restart-Fresh) and accumulate across
  Restart-from-Save. Behavior matches existing `CodingScore` semantics.
- Headshot counter is on `UCodeRescueGameInstance` and the SaveGame; the
  character increments it from `Fire()` when a headshot kills (item 3).
- Scoreboard text is plain Printf rather than a UMG widget so it can be
  pasted into either the Victory widget or the Death widget without
  dragging Slate constructors into both.

## Known limitations
- `RunSeconds` increment hook isn't wired to a tick yet — the field exists
  but nobody calls `AccumulateRunSeconds`. Quick win: have the GameMode
  Victory-check timer call it with 1.0 each tick.
- The Victory and Death widgets currently show their own stats text rather
  than the new summary. They can be retargeted to call `GetScoreboardSummary()`
  in a one-liner.

## Follow-up work
- Hook `RunSeconds` accumulation in `ACodeRescueGameMode::CheckVictoryCondition`
  (it's already on a 1Hz timer).
- Show the scoreboard on the death + victory screens.
- Surface in pause menu as "Stats" tab.
