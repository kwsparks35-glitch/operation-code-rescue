#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class ACodeRescueCharacter;

UENUM(BlueprintType)
enum class EPickupKind : uint8
{
    Ammo    UMETA(DisplayName = "Ammo"),
    Medkit  UMETA(DisplayName = "Medkit"),
    Flare   UMETA(DisplayName = "Flare"),
    Smoke   UMETA(DisplayName = "Smoke"),
    Stim    UMETA(DisplayName = "Stim"),
    Scrap   UMETA(DisplayName = "Scrap"),
    ArmorPlate UMETA(DisplayName = "Armor Plate"),
    RadioScanner UMETA(DisplayName = "Radio Scanner"),
    FlashlightBattery UMETA(DisplayName = "Flashlight Battery"),
    AmmoPouch UMETA(DisplayName = "Ammo Pouch"),
    BypassKit UMETA(DisplayName = "Bypass Kit")
};

/** Simple collectible. Player walks into the sphere trigger; resource is
 *  granted on the character; the actor self-destroys. Restocks ammo,
 *  medkits, throwables, scrap, or armor plates — quantity is configurable
 *  per-pickup. */
UCLASS()
class CODERESCUEUNREAL_API APickupActor : public AActor
{
    GENERATED_BODY()

public:
    APickupActor();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    EPickupKind Kind = EPickupKind::Ammo;

    /** How much ammo (when Kind == Ammo) or how many medkit charges
     *  (when Kind == Medkit) this pickup grants. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    int32 Amount = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    bool bSnapToGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup", meta=(ClampMin="8.0", ClampMax="120.0"))
    float GroundClearance = 42.0f;

    /** Slow turntable motion keeps the physical face symbol readable from any
     * approach without bobbing the grounded package. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup", meta=(ClampMin="0.0", ClampMax="90.0"))
    float PresentationSpinDegreesPerSecond = 18.0f;

    UFUNCTION(BlueprintCallable, Category="Pickup")
    bool Collect(ACodeRescueCharacter* Character);

    /** Re-resolves the icon-first Blender package after Kind is assigned.
     * Spawn callers set Kind immediately after SpawnActor returns, which is
     * after BeginPlay for non-deferred spawns, so this is also run next tick. */
    UFUNCTION(BlueprintCallable, Category="Pickup")
    void RefreshPresentation();

    UFUNCTION(BlueprintPure, Category="Pickup")
    bool IsAuthoredPresentationReady() const { return bAuthoredPresentationReady; }

    UFUNCTION(BlueprintPure, Category="Pickup")
    FString GetPresentationStyleToken() const { return PresentationStyleToken; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                          bool bFromSweep, const FHitResult& SweepResult);

private:
    void SnapToGround();
    const TCHAR* ResolvePresentationAssetName() const;
    const TCHAR* ResolvePresentationStyleToken() const;

    UPROPERTY()
    UStaticMeshComponent* MeshComp;

    UPROPERTY()
    USphereComponent* TriggerComp;

    UPROPERTY()
    UPointLightComponent* GlowComp;

    FString PresentationStyleToken;
    bool bAuthoredPresentationReady = false;
};
