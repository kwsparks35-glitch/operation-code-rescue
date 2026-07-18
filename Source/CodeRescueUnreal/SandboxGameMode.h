#pragma once

#include "CoreMinimal.h"
#include "CodeRescueGameMode.h"
#include "SandboxGameMode.generated.h"

/**
 * #33 — Sandbox game mode for stress-free practice.
 *
 * Subclass of `ACodeRescueGameMode` that flips `bSandboxMode = true` in its
 * constructor. The base SpawnWorld checks this flag and skips zombie + horde
 * spawns, keeping the launch-selected language marker, terminals, and pickups intact.
 *
 * Set as the GameMode override on a separate entry map (e.g.
 * `Maps/Sandbox.umap`) so the player can choose between Sandbox and the
 * Campaign from the main menu (item 38).
 */
UCLASS()
class CODERESCUEUNREAL_API ASandboxGameMode : public ACodeRescueGameMode
{
    GENERATED_BODY()

public:
    ASandboxGameMode();
};
