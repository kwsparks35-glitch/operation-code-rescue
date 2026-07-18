# #66 — Victory widget submits to the leaderboards

`UCodeRescueVictoryWidget::NativeConstruct` now writes to all four
`ELeaderboardKind` lists when the win screen first appears.

| Kind                          | Score                                  |
| ----------------------------- | -------------------------------------- |
| `FastestFiveCity`             | `-floor(GI->RunSeconds)` (negated so   |
|                               | descending sort puts faster runs at   |
|                               | the top); only submitted if           |
|                               | `TerminalSolveCount >= 5`             |
| `MostRescues`                 | `GI->RescueCount`                     |
| `MostHeadshots`               | `GI->HeadshotCount`                   |
| `LongestNoResupply`           | `GI->KillCount` (placeholder until a  |
|                               | dedicated streak counter exists)      |

Player name defaults to `"Operative"`. A real profile-name pass can
slot in here once the settings menu surfaces a name field.

## Why negate FastestFiveCity score
`UCodeRescueLeaderboards` sorts entries descending by `Score`. Real
"fastest" semantics want ascending. Negating the seconds at submit time
plus negating again at display time gives the right order without
adding a per-kind comparator.

## Files
- `Source/CodeRescueUnreal/CodeRescueVictoryWidget.cpp` — new include
  + four `Submit` calls inside the existing GI block.
