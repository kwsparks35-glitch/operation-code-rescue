# #63 — Spawn Companion on first survivor rescue

`ACompanionActor` (#53) was implemented but never spawned. Hooked into
`ASurvivorActor::Rescue()` so the very first successful rescue spawns a
follower. A new `bHasCompanion` flag on `UCodeRescueGameInstance` gates
this so subsequent rescues don't spawn duplicates.

## Logic

```cpp
if (!GI->bHasCompanion)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    FVector SpawnAt = Player->GetActorLocation() + FVector(-220, -180, 0);
    ACompanionActor* Buddy = World->SpawnActor<ACompanionActor>(...);
    Buddy->DisplayName = SurvivorName;
    GI->bHasCompanion = true;
    UCodeRescueSubtitlesWidget::Push("[Name]: I'll cover you. Let's move!", 4.0f);
}
```

## Why behind+left of the player
So the companion appears in the player's normal field of view when they
turn around — not directly under their feet (collision spike) and not
in front of them (visual jumpscare).

## Reset rules
- `bHasCompanion` is reset on `ResetRun()` (Restart-Fresh).
- It is NOT reset on `LoadPersistentRun()` — if you saved with a
  companion alive, you reload with the flag set; the companion itself
  isn't persisted (transient world content), but the flag prevents the
  next rescue from spawning a duplicate.
- The companion's death subtitle is part of `ACompanionActor` itself
  (see #53).

## Files
- `Source/CodeRescueUnreal/SurvivorActor.cpp` — Rescue() addition,
  `#include "CompanionActor.h"`.
- `Source/CodeRescueUnreal/CodeRescueGameInstance.{h,cpp}` —
  `bHasCompanion` field + reset.
