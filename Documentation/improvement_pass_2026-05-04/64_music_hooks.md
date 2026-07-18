# #64 — Music hooks (menu / city / horde stinger)

`UCodeRescueGameInstance` (#43) defined `PlayMenuMusic()`,
`PlayCityMusic()`, `PlayHordeStinger()` and the underlying
cross-fade `PlayMusic()`, but no call sites invoked them. Wired three
spots:

| Hook                              | Caller                                  |
| --------------------------------- | --------------------------------------- |
| `PlayMenuMusic()`                 | `AMainMenuGameMode::BeginPlay()`        |
| `PlayCityMusic()`                 | `ACodeRescueGameMode::BeginPlay()` after `SpawnWorld()` |
| `PlayHordeStinger()`              | End of `ACodeRescueGameMode::TriggerBossHorde()` |

All three are no-ops if the corresponding soft cue (`MenuMusic`,
`AmbientCityMusic`, `BossHordeStinger`) hasn't been bound yet — the
audio component creation and `SetSound(nullptr)` short-circuits cleanly.
That means importing or binding music is a separate Blueprint-only step;
no further C++ work needed.

## Files
- `Source/CodeRescueUnreal/MainMenuGameMode.cpp` — added GI include +
  PlayMenuMusic call after viewport mount.
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — PlayCityMusic
  after SpawnWorld; PlayHordeStinger at end of TriggerBossHorde.
