#pragma once

#include "CoreMinimal.h"
#include "CodeZombieActor.h"
#include "BossZombieActor.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/**
 * #51 — Per-zone boss. Three phases scaling by city tier.
 *   Phase 1: standard chase + melee (Health > 66%)
 *   Phase 2: sprint+regen (33% < Health <= 66%) — MoveSpeed * 1.5, regens 5/s
 *   Phase 3: spawn-adds (Health <= 33%) — every 3s spawns 2 small zombies
 *
 * Spawned by ACodeRescueGameMode::SpawnBossForCity at city center +500u.
 * Drops a permanent stat-upgrade voucher on death (sets a SaveGame flag).
 */
UCLASS()
class CODERESCUEUNREAL_API ABossZombieActor : public ACodeZombieActor
{
    GENERATED_BODY()

public:
    ABossZombieActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    int32 CurrentPhase = 1;
    float TimeSinceAddSpawn = 0.0f;
    float MaxBossHealth = 600.0f;
    int32 SpawnedAddSerial = 0;

    UPROPERTY(EditAnywhere, Category="Boss")
    int32 MaxActiveAdds = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Combat Readability", meta=(ClampMin="0.5", ClampMax="8.0"))
    float PhaseTelegraphDuration = 3.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Combat Readability", meta=(ClampMin="120.0", ClampMax="1600.0"))
    float PhaseTelegraphRadius = 560.0f;

    UFUNCTION(BlueprintImplementableEvent, Category="Boss|Combat Readability")
    void OnBossPhaseTelegraphStarted(int32 Phase);

    TArray<TWeakObjectPtr<ACodeZombieActor>> ActiveAdds;

    UPROPERTY()
    UStaticMeshComponent* PhaseTelegraphRing = nullptr;

    UPROPERTY()
    UStaticMeshComponent* PhaseTelegraphCore = nullptr;

    UPROPERTY()
    UStaticMeshComponent* PhaseTelegraphSweep = nullptr;

    UPROPERTY()
    UStaticMeshComponent* PhaseTelegraphAddBeaconA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* PhaseTelegraphAddBeaconB = nullptr;

    UPROPERTY()
    UPointLightComponent* PhaseTelegraphLight = nullptr;

    float PhaseTelegraphElapsed = 0.0f;
    float PhaseTelegraphTimeRemaining = 0.0f;
    int32 PhaseTelegraphActivePhase = 1;
    bool bPhaseTelegraphReducedMotion = false;
    FLinearColor PhaseTelegraphColor = FLinearColor(1.0f, 0.46f, 0.02f);

    void EnterPhase(int32 Phase);
    int32 CountLivingAdds();
    void StartPhaseTelegraph(int32 Phase);
    void UpdatePhaseTelegraph(float DeltaSeconds);
    void ApplyPhaseTelegraphVisibility(bool bVisible);
    void ConfigurePhaseTelegraphComponent(UStaticMeshComponent* Component);
    void ApplyPhaseTelegraphTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
};
