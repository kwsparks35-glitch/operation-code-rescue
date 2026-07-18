#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BarricadeActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;

/**
 * ABarricadeActor — placeable temporary cover.
 *
 * Spawned by `ACodeRescueCharacter::PlaceBarricade()` (B-key). Costs 5 scrap.
 * Lives for `Lifetime` seconds, blocking pawn collision in a 200u doorway,
 * then destroys itself.
 *
 * The barricade has its own simple Health that zombies can wear down by
 * attacking it (treat it as a damageable obstacle), but in v1 zombies just
 * pathfind around it via the existing nav mesh.
 */
UCLASS()
class CODERESCUEUNREAL_API ABarricadeActor : public AActor
{
    GENERATED_BODY()

public:
    ABarricadeActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade")
    float Lifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade")
    float Health = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade")
    float MaxHealth = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.0", ClampMax="120.0"))
    float MinDamageToChip = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.0", ClampMax="2500.0"))
    float ImpactDamageSpeedThreshold = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.0", ClampMax="0.5"))
    float ImpactDamageScale = 0.026f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0", ClampMax="20"))
    int32 DebrisCount = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.5", ClampMax="45.0"))
    float DebrisLifetime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.0", ClampMax="200000.0"))
    float DebrisImpulseStrength = 42000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Barricade|Destruction", meta=(ClampMin="0.25", ClampMax="8.0"))
    float DebrisSleepDisableDelay = 2.75f;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category="Barricade|Destruction")
    void TakeBarricadeDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpulseDirection, AActor* DamageSource);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY() UStaticMeshComponent* Body = nullptr;
    UPROPERTY() UMaterialInstanceDynamic* BodyMaterial = nullptr;
    FTimerHandle ExpireTimer;
    bool bBroken = false;
    float LastImpactDamageTime = -100.0f;

    void Expire();
    void UpdateDamageStateVisual();
    void BreakApart(const FVector& ImpactPoint, const FVector& ImpulseDirection, AActor* DamageSource);
    void SpawnDebrisChunk(int32 Index, const FVector& ImpactPoint, const FVector& ImpulseDirection);
    void ScheduleDebrisSleepDisable(class AStaticMeshActor* Debris, UStaticMeshComponent* DebrisMesh);

    UFUNCTION()
    void OnBarricadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
