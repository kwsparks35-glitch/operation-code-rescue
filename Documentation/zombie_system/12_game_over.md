# Item 12 — Game-over / death state

**Status:** DONE — modal death widget shipped; pauses world, captures
input, offers Restart-from-Save / Restart-Fresh / Quit.

## What landed

Two new files:

- `Source/CodeRescueUnreal/CodeRescueDeathWidget.h`
- `Source/CodeRescueUnreal/CodeRescueDeathWidget.cpp`

The widget is modeled on the existing `UCodeRescueVictoryWidget` so the
look/feel and input-lock pattern stay consistent. It builds a
canvas-with-blur layout and three buttons:

| Button | Behavior |
|---|---|
| **RESTART FROM LAST SAVE** | Unpauses, clears UI lock, calls `OpenLevel(currentLevel)`. The save is loaded automatically on the next BeginPlay because `UCodeRescueGameInstance::Init()` calls `LoadPersistentRun`. |
| **RESTART FRESH (delete save)** | Calls `GI->DeletePersistentRun()` then reopens the level. Wipes terminal/survivor/zombie progress — same as Victory's Restart. |
| **QUIT TO DESKTOP** | `UKismetSystemLibrary::QuitGame`. |

`ACodeRescueCharacter::ApplyDamage` now spawns the widget when Health
hits 0:

```cpp
if (Health <= 0.0f)
{
    UGameplayStatics::SetGamePaused(GetWorld(), true);
    UClass* WidgetClass = DeathWidgetClass
        ? DeathWidgetClass.Get()
        : static_cast<UClass*>(UCodeRescueDeathWidget::StaticClass());
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UUserWidget* W = CreateWidget<UUserWidget>(PC, WidgetClass))
        {
            W->AddToViewport(1000);
        }
    }
}
```

The early-out at the top of `ApplyDamage` (`if (Health <= 0) return;`)
prevents zombies stacking on the corpse from re-spawning the widget every
hit.

`ACodeRescueCharacter::DeathWidgetClass` (`TSubclassOf<UUserWidget>`)
exposes a Blueprint override slot — set it on the BP defaults if you
author a richer death-screen UI later. Falls back to the C++ class.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueDeathWidget.{h,cpp}` (new)
- `Source/CodeRescueUnreal/CodeRescueCharacter.h` — `DeathWidgetClass`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` —
  ApplyDamage rewrite; `#include "CodeRescueDeathWidget.h"`

## How to verify

1. PIE.
2. Stand still and let zombies hit you. Each hit prints the existing
   "Damage taken: X | Health: Y" debug line.
3. When Health reaches 0:
   - World pauses (zombies stop, particles freeze).
   - Mouse cursor appears.
   - Death widget shows with title, stats, and three buttons.
4. Click **RESTART FROM LAST SAVE** — level reopens; you spawn at the
   last saved transform with the same terminals-solved / survivors-rescued
   counts.
5. Or click **RESTART FRESH** — level reopens with everything zeroed.

## Note on input safety

The widget calls `ACodeRescueCharacter::SetUIOpen(true)` so the
polled-key gameplay loop pauses (consistent with how the
terminal/journal/pause widgets work). On `NativeDestruct` the same flag
is cleared, plus the world is unpaused — defensive code for the case
where the widget tears down via an unexpected path.
