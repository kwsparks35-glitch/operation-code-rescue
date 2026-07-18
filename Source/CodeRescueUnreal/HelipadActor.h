#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelipadActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UUserWidget;

/**
 * AHelipadActor — interaction landing pad for fast-travel between cities.
 *
 * One spawned per city in ACodeRescueGameMode::SpawnWorld. CityIndex
 * matches FCodeRescueCampaign mission index (0..341). Player presses E
 * while looking at the helipad to open UCityFastTravelWidget; selecting
 * a destination teleports the player to that city's player-start.
 *
 * Visual: large flat cylinder + glowing point light. Static-mesh
 * placeholder; swap for an authored helipad mesh later.
 */
UCLASS()
class CODERESCUEUNREAL_API AHelipadActor : public AActor
{
    GENERATED_BODY()

public:
    AHelipadActor();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helipad")
    int32 CityIndex = 0;

    /** Display name shown in the fast-travel widget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helipad")
    FString CityLabel;

    /** Optional override for the fast-travel widget. Defaults to the C++
     *  UCityFastTravelWidget class if left unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helipad")
    TSubclassOf<UUserWidget> FastTravelWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category="Helipad|Extraction")
    bool bExtractionReady = false;

    UPROPERTY(BlueprintReadOnly, Category="Helipad|Extraction")
    FString ExtractionSurvivorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helipad|Extraction")
    FLinearColor ExtractionAccentColor = FLinearColor(0.36f, 1.0f, 0.42f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helipad|Extraction")
    bool bReducedMotion = false;

    /** Called by the player's Interact() when the trace lands on this actor. */
    UFUNCTION(BlueprintCallable, Category="Helipad")
    void OpenFastTravelMenu();

    UFUNCTION(BlueprintCallable, Category="Helipad|Extraction")
    void SetExtractionReady(const FString& SurvivorName, const FLinearColor& AccentColor, bool bInReducedMotion);

protected:
    virtual void BeginPlay() override;

private:
    void ApplyExtractionVisualState();
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
    void ConfigureExtractionComponent(UStaticMeshComponent* Component);

    UPROPERTY()
    UStaticMeshComponent* PadMesh = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ExtractionColumn = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ExtractionSweepA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ExtractionSweepB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ExtractionBeacon = nullptr;

    UPROPERTY()
    UPointLightComponent* GlowLight = nullptr;

    UPROPERTY()
    UUserWidget* ActiveFastTravelWidget = nullptr;

    float ExtractionPulseTime = 0.0f;
};
