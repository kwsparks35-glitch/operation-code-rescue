#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveFocusBeaconActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class CODERESCUEUNREAL_API AObjectiveFocusBeaconActor : public AActor
{
    GENERATED_BODY()

public:
    AObjectiveFocusBeaconActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    int32 CityIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString CityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString TerminalId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString SurvivorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString TerminalTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString MissionConcept;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FString LandmarkName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FVector EntryLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FVector TerminalLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FVector SurvivorLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FVector ExtractionLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FLinearColor TerminalColor = FLinearColor(0.04f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FLinearColor SurvivorColor = FLinearColor(1.0f, 0.86f, 0.10f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    FLinearColor ExtractionColor = FLinearColor(0.70f, 0.92f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective Beacon")
    bool bReducedMotion = false;

    UFUNCTION(BlueprintCallable, Category="Objective Beacon")
    void ConfigureObjectiveBeacon(
        int32 InCityIndex,
        const FString& InCityName,
        const FString& InTerminalId,
        const FString& InSurvivorName,
        const FString& InTerminalTitle,
        const FString& InMissionConcept,
        const FString& InLandmarkName,
        const FVector& InEntryLocation,
        const FVector& InTerminalLocation,
        const FVector& InSurvivorLocation,
        const FVector& InExtractionLocation,
        const FLinearColor& InTerminalColor,
        const FLinearColor& InSurvivorColor,
        const FLinearColor& InExtractionColor,
        bool bInReducedMotion);

    UFUNCTION(BlueprintImplementableEvent, Category="Objective Beacon")
    void OnObjectiveBeaconPhaseChanged(int32 ObjectivePhase);

    /** Current step of this city's rescue loop: 1=terminal, 2=survivor, 3=extraction (INDEX_NONE when idle). */
    UFUNCTION(BlueprintCallable, Category="Objective Beacon")
    int32 GetCurrentObjectivePhase() { return ResolveObjectivePhase(); }

    /** World location of the current step's target; actor location when idle. Used by T step-travel. */
    UFUNCTION(BlueprintCallable, Category="Objective Beacon")
    FVector GetCurrentPhaseTargetLocation()
    {
        const int32 Phase = ResolveObjectivePhase();
        return Phase == INDEX_NONE ? GetActorLocation() : ResolvePhaseTargetLocation(Phase);
    }

private:
    UPROPERTY()
    USceneComponent* SceneRoot = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BaseRing = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BeaconColumn = nullptr;

    UPROPERTY()
    UStaticMeshComponent* DirectionArrow = nullptr;

    UPROPERTY()
    UStaticMeshComponent* PulseCore = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RadioScanRing = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RadioSweepArm = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RescueBeaconHalo = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RadioPingA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RadioPingB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* StepNodeA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* StepNodeB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* StepNodeC = nullptr;

    UPROPERTY()
    UPointLightComponent* BeaconLight = nullptr;

    UPROPERTY()
    UTextRenderComponent* ObjectiveLabel = nullptr;

    UPROPERTY()
    UTextRenderComponent* RadioScanLabel = nullptr;

    float MotionTime = 0.0f;
    int32 CurrentObjectivePhase = INDEX_NONE;
    bool bBeaconVisible = false;
    FLinearColor CurrentTint = FLinearColor::White;

    int32 ResolveObjectivePhase();
    FVector ResolvePhaseTargetLocation(int32 ObjectivePhase) const;
    /** 2026-07-11 final-station fix: during the terminal phase the beacon must
     *  track the ACTUAL next-unsolved coding station, not the fixed configured
     *  offset (which sat ~23 m from station 10 and stranded the player). */
    FVector ResolveActiveTerminalTargetLocation() const;
    FString BuildPhaseLabel(int32 ObjectivePhase);
    FString BuildRadioScanLine(int32 ObjectivePhase);
    void RefreshPhaseVisuals(int32 ObjectivePhase);
    void SetBeaconVisible(bool bVisible);
    void ConfigureBeaconComponent(UStaticMeshComponent* Component);
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
};
