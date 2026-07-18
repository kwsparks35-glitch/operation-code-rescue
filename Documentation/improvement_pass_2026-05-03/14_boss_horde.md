# Item 14 — Boss / horde encounter on terminal solve

## What changed
When the player successfully solves a city's coding terminal,
`UCodeTerminalWidget::OnValidateClicked` now calls
`ACodeRescueGameMode::TriggerBossHorde(TerminalLocation, CityIndex)` after
recording the solve.

`TriggerBossHorde` spawns `max(8, ZombieMaxCount + 4)` zombies in a ring
1100 units around the terminal. Horde zombies use ID range 100000+ so they
don't collide with the city's saved-neutralized list (they're transient
encounter content, not persisted).

Stat boosts vs. baseline city zombies:
- Health × 1.25
- Damage × 1.15
- Move speed × 1.20
- Activation range raised so they all aggro immediately

A red guide-text marker reading **"INCOMING HORDE — DEFEND THE EXTRACTION"**
is dropped above the terminal.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.h` — public method declaration.
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — implementation.
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp` — trigger call after
  `RecordTerminalSolved` + `IncrementTerminalSolveCount`.

## Design decisions
- Horde zombies are not persisted to the SaveGame: if you save mid-horde and
  reload, the horde simply doesn't respawn. This is intentional — the horde
  is a per-encounter spice, not save-state.
- Horde zombie count is data-driven via `ZombieMaxCount`, so the difficulty
  knob the GameMode already exposes also tunes the horde.

## Known limitations
- No "wave clear → unlock survivor" gate yet. The horde is purely additive.
  To gate the survivor extraction, add a `bHordeCleared` field on
  `ACodingTerminalActor` and check it in `ASurvivorActor::Rescue()`.
- Horde zombies share the city's `Accent` color via SpawnBlock marker but
  don't read the variant table — they're always the procedural fallback.

## Follow-up work
- Wave gating on the Survivor rescue.
- Visual: spawn a Niagara emitter at horde-spawn points for "they came from
  there" telegraphing.
- Hard-mode option: 2nd wave at +30s for cities past tier 5.
