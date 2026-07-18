#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossRevealPresentationActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CODERESCUEUNREAL_API ABossRevealPresentationActor : public AActor
{
    GENERATED_BODY()

public:
    ABossRevealPresentationActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    float TriggerRadius = 4600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    float DurationSeconds = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    bool bReducedMotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    int32 CityIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    FString BossTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    FString CityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    FLinearColor WarningColor = FLinearColor(1.0f, 0.04f, 0.18f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss Reveal")
    TSoftObjectPtr<UObject> OptionalSequencerRevealAsset;

    UFUNCTION(BlueprintCallable, Category="Boss Reveal")
    void ConfigureReveal(
        AActor* InBossActor,
        int32 InCityIndex,
        const FString& InCityName,
        const FString& InBossTitle,
        const FLinearColor& InWarningColor,
        bool bInReducedMotion);

    UFUNCTION(BlueprintImplementableEvent, Category="Boss Reveal")
    void OnBossRevealStarted(AActor* RevealedBoss);

private:
    UPROPERTY()
    USceneComponent* SceneRoot = nullptr;

    UPROPERTY()
    AActor* BossActor = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ArenaRing = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ThreatGateA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ThreatGateB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* SweepA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* SweepB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BossCrown = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BeaconA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BeaconB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BeaconC = nullptr;

    UPROPERTY()
    UPointLightComponent* WarningLight = nullptr;

    bool bRevealStarted = false;
    float RevealElapsed = 0.0f;

    void BeginReveal();
    void ApplyRevealVisualState(bool bVisible);
    void ConfigureRevealComponent(UStaticMeshComponent* Component);
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
};
