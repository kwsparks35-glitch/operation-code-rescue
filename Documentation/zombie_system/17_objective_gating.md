# Item 17 — Objective progression / gating

**Status:** DONE — survivors soft-gate behind their zone's main
terminal. The bonus terminals (item 15) remain ungated.

## What landed

`ASurvivorActor` gained a new property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Gating")
FString RequiredTerminalId;
```

`Rescue()` checks it:

```cpp
if (!RequiredTerminalId.IsEmpty()
    && GI && !GI->SolvedTerminalIds.Contains(RequiredTerminalId))
{
    GEngine->AddOnScreenDebugMessage(...,
        FString::Printf(TEXT("%s won't move until you finish: %s"),
                        *SurvivorName, *RequiredTerminalId));
    return; // refuses rescue, does NOT mark/save
}
```

`ACodeRescueGameMode::SpawnWorld` populates the field per spawn:

| Survivor index | Zone | RequiredTerminalId |
|---|---|---|
| 0 | Anchorage | `generator_sum` |
| 1 | Seattle | `chapel_lock` |
| 2 | Tokyo (1st) | `generator_sum_final` |
| 3 | Tokyo (2nd) | `""` (ungated bonus) |

## Why "soft" gating

The hint message uses `AddOnScreenDebugMessage` (4 sec, yellow), not a
modal popup. Two reasons:

1. The player might be running for their life — interrupting them with
   a modal "you must solve X first" is hostile.
2. Some players will figure out the connection from the hint and feel
   smart; others will go solve the terminal first; either way the soft
   nudge is enough.

## Files touched

- `Source/CodeRescueUnreal/SurvivorActor.h` — `RequiredTerminalId` field
- `Source/CodeRescueUnreal/SurvivorActor.cpp` — gating check in `Rescue()`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` —
  `SurvivorRequiredTerminals` array + per-spawn assignment

## How to verify

1. PIE on a fresh save.
2. Walk to Anchorage's survivor BEFORE solving the Anchorage main
   terminal. Press E to attempt rescue.
3. Yellow on-screen hint: `Survivor 1 won't move until you finish:
   generator_sum`. Survivor remains in place.
4. Go to the Anchorage main terminal, solve `generator_sum`.
5. Walk back. Now the survivor accepts rescue.
6. The Tokyo bonus survivor (index 3) has empty `RequiredTerminalId`
   and accepts rescue at any time.

## Win condition still works

The win condition is `TerminalsSolved >= 3 && SurvivorsRescued >= 4`
(in `ACodeRescueGameMode::CheckVictoryCondition`). With gating: you
must solve all three main terminals to rescue the gated survivors,
which pushes you to engage the coding loop instead of speed-running
survivor pickups. The bonus survivor still counts toward the 4 rescue
requirement.

## Open follow-ups

If a level designer wants more nuanced gating (e.g. "rescue requires
solving any 2 of these 4 terminals"), extend the field to
`TArray<FString> RequiredTerminalIds` and the count threshold to a new
`int32 RequiredCount = 1`. Current scalar form is the simplest thing
that works.
