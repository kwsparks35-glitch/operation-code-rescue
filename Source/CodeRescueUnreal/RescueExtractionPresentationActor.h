#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RescueExtractionPresentationActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CODERESCUEUNREAL_API ARescueExtractionPresentationActor : public AActor
{
    GENERATED_BODY()

public:
    ARescueExtractionPresentationActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation")
    FString PresentedSurvivorName = TEXT("Civilian Survivor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation")
    int32 PresentedCityIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation")
    FLinearColor AccentColor = FLinearColor(1.0f, 0.82f, 0.18f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation", meta=(ClampMin="0.75", ClampMax="12.0"))
    float DurationSeconds = 4.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation")
    bool bReducedMotion = false;

    /** Optional future asset slot. The C++ fallback below is fully playable even when unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Rescue Presentation", meta=(AllowedClasses="/Script/LevelSequence.LevelSequence"))
    TSoftObjectPtr<UObject> OptionalSequencerBeatAsset;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Rescue Presentation")
    void ConfigurePresentation(const FString& InSurvivorName, int32 InCityIndex, const FLinearColor& InAccentColor, bool bInReducedMotion);

    UFUNCTION(BlueprintImplementableEvent, Category="Code Rescue|Rescue Presentation")
    void OnRescuePresentationStarted(const FString& InSurvivorName, int32 InCityIndex);

private:
    void ApplyVisualTints();
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
    void TagPresentationComponent(UStaticMeshComponent* Component);

    UPROPERTY()
    USceneComponent* Root = nullptr;

    UPROPERTY()
    UStaticMeshComponent* LandingDisc = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RescueBeam = nullptr;

    UPROPERTY()
    UStaticMeshComponent* SweepArmA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* SweepArmB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* LiftMarker = nullptr;

    UPROPERTY()
    UStaticMeshComponent* OrbitBeaconA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* OrbitBeaconB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* OrbitBeaconC = nullptr;

    UPROPERTY()
    UPointLightComponent* RescueKeyLight = nullptr;

    UPROPERTY()
    UPointLightComponent* RescueFillLight = nullptr;

    float ElapsedSeconds = 0.0f;
};
