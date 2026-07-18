#pragma once

// 2026-07-11 pass 4 (environment physics): ambient WIND. Trees and bushes
// near the player sway with gusting, per-actor-phased motion. Registration is
// by mesh-path match ("/Nature/" kit pieces) or the explicit "WindFoliage"
// tag; registered actors are flipped to Movable once and rotated about their
// captured base rotation every tick. Tunable/disable-able: cr.WindStrength.

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueWindSway.generated.h"

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueWindSwayManager : public AActor
{
    GENERATED_BODY()

public:
    ACodeRescueWindSwayManager();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    struct FSwayTarget
    {
        TWeakObjectPtr<AActor> Actor;
        FRotator BaseRotation = FRotator::ZeroRotator;
        float Phase = 0.0f;
        float Amplitude = 1.5f;    // degrees at gust strength 1
    };

    void RefreshRegistry();

    TArray<FSwayTarget> Targets;
    FTimerHandle RefreshTimer;
};
