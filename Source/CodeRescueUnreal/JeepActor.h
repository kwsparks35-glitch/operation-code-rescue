#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "JeepActor.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class UPointLightComponent;
class UFloatingPawnMovement;

/**
 * #52 — Drivable jeep. Mounted via Interact at a helipad. Player possess
 * swaps in this pawn; WASD drives forward/back/turn at 3x ground speed,
 * E dismounts and respawns the player at the jeep's location.
 *
 * Uses UFloatingPawnMovement for Mac-friendly basic vehicle behavior
 * (no full PhysX wheel sim). Good enough for "fast traversal across the
 * 50x city" — the design intent.
 */
UCLASS()
class CODERESCUEUNREAL_API AJeepActor : public APawn
{
    GENERATED_BODY()

public:
    AJeepActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep")
    float MaxJeepSpeed = 2700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep")
    float TurnRateDegPerSec = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep|Surface Physics", meta=(ClampMin="0.02", ClampMax="1.0"))
    float SurfaceProbeInterval = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep|Surface Physics", meta=(ClampMin="80.0", ClampMax="900.0"))
    float SurfaceProbeDistance = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep|Surface Physics", meta=(ClampMin="0.0", ClampMax="20.0"))
    float HighTractionLateralDamping = 5.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jeep|Surface Physics", meta=(ClampMin="0.0", ClampMax="20.0"))
    float LowTractionLateralDamping = 0.9f;

    /** Called by player Interact. */
    UFUNCTION(BlueprintCallable, Category="Jeep")
    void Mount(class APlayerController* PC, class APawn* PreviousPawn);

    UFUNCTION(BlueprintCallable, Category="Jeep")
    void Dismount();

private:
    UPROPERTY() UStaticMeshComponent* Body = nullptr;
    UPROPERTY() UCameraComponent* Camera = nullptr;
    UPROPERTY() UPointLightComponent* SurfaceCueLight = nullptr;
    UPROPERTY() UFloatingPawnMovement* Movement = nullptr;
    UPROPERTY() class APawn* CachedPreviousPawn = nullptr;

    EPhysicalSurface CurrentGroundSurface = SurfaceType_Default;
    float CurrentTraction = 0.92f;
    float CurrentSpeedScale = 1.0f;
    float CurrentTurnScale = 1.0f;
    float LastSurfaceProbeTime = -100.0f;
    float BaseAcceleration = 4500.0f;
    float BaseDeceleration = 6000.0f;

    void UpdateGroundSurface();
    EPhysicalSurface ResolveGroundSurface(const FHitResult& Hit) const;
    void ApplySurfaceTuning(float DeltaSeconds, float ForwardInput, float TurnInput);
    float GetSurfaceTraction(EPhysicalSurface SurfaceType) const;
    float GetSurfaceSpeedScale(EPhysicalSurface SurfaceType) const;
    float GetSurfaceTurnScale(EPhysicalSurface SurfaceType) const;
    FLinearColor GetSurfaceCueColor(EPhysicalSurface SurfaceType) const;
    const TCHAR* DescribeSurface(EPhysicalSurface SurfaceType) const;
};
