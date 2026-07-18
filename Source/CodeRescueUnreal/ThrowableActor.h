#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ThrowableActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UNiagaraSystem;
class USphereComponent;
class UPrimitiveComponent;

/** #28 — throwable types. Flare distracts zombies; smoke breaks LoS;
 *  stim refills player stamina. */
UENUM(BlueprintType)
enum class EThrowableKind : uint8
{
    Flare = 0,
    Smoke = 1,
    Stim  = 2
};

/**
 * AThrowableActor — base class for player-thrown utility items.
 *
 * Spawned by `ACodeRescueCharacter::ThrowActive()` at the camera location
 * with a forward velocity. Lives for `Lifetime` seconds, doing kind-specific
 * work each tick or via timer. On expiration, destroys itself.
 *
 * The AI controller polls a global registry (`StaticActiveLures`) for
 * Flare-kind throwables to redirect chase targets. See item 28 doc for
 * the AI integration details.
 */
UCLASS()
class CODERESCUEUNREAL_API AThrowableActor : public AActor
{
    GENERATED_BODY()

public:
    AThrowableActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
    EThrowableKind Kind = EThrowableKind::Flare;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
    float Lifetime = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="100.0", ClampMax="6000.0"))
    float ThrowImpulseStrength = 1850.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="0.0", ClampMax="1800.0"))
    float ThrowUpwardImpulse = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="0.0", ClampMax="5.0"))
    float UtilityPulseDelay = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="100.0", ClampMax="5000.0"))
    float UtilityPulseRadius = 720.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="0.0", ClampMax="250000.0"))
    float UtilityPulseImpulseStrength = 68000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Surface Impact", meta=(ClampMin="0.0", ClampMax="500000.0"))
    float SurfaceImpactImpulseStrength = 36000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Surface Impact", meta=(ClampMin="0.0", ClampMax="2500.0"))
    float MinSurfaceImpactSpeed = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Surface Impact", meta=(ClampMin="0.0", ClampMax="2.0"))
    float SurfaceImpactCooldown = 0.14f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="0.0", ClampMax="200.0"))
    float FlarePulseDamage = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Physics", meta=(ClampMin="0.0", ClampMax="200.0"))
    float SmokePulseDamage = 16.0f;

    /** AI lure radius — zombies within this distance prefer the throwable
     *  over the player as their chase target. Flare-only. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
    float LureRadius = 2500.0f;

    UFUNCTION(BlueprintCallable, Category="Throwable|Physics")
    void LaunchThrowable(const FVector& Direction, float StrengthScale = 1.0f);

    /** 2026-07-16 pass 5: GRENADE PAYLOAD mode — the actor flies as a real
     *  physics projectile (deterministic initial velocity so the aim-arc
     *  prediction matches exactly), then detonates after FuseSeconds via the
     *  owning character (which applies the area effect + explosion
     *  presentation at the ACTUAL landing point). */
    void ConfigureGrenadePayload(class ACodeRescueCharacter* InInstigator,
        uint8 InPayloadWeapon, float InFuseSeconds, const FVector& InitialVelocity);

    /** Exposed so the aim preview can read the exact ballistic constants. */
    static constexpr float GrenadeProjectileRadius = 9.0f;

    /** All currently-active flare lures. Polled by AI controllers each tick.
     *  Static + raw pointers because lures are short-lived single-player. */
    static TArray<TWeakObjectPtr<AThrowableActor>> StaticActiveLures;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
    UPROPERTY() UStaticMeshComponent* Body = nullptr;
    UPROPERTY() UPointLightComponent* GlowLight = nullptr;

    void Expire();
    void DetonateGrenade();
    bool bGrenadePayload = false;
    uint8 PayloadWeapon = 0;
    TWeakObjectPtr<class ACodeRescueCharacter> PayloadInstigator;
    FTimerHandle GrenadeFuseTimer;
    FTimerHandle ExpireTimer;
    FTimerHandle UtilityPulseTimer;
    bool bUtilityPulseFired = false;
    float LastSurfaceImpactTime = -100.0f;

    UFUNCTION()
    void OnThrowableImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    EPhysicalSurface ResolveImpactSurface(const FHitResult& Hit, AActor* OtherActor, UPrimitiveComponent* OtherComp) const;
    FLinearColor GetSurfaceImpactColor(EPhysicalSurface SurfaceType) const;
    float GetSurfaceImpactImpulseScale(EPhysicalSurface SurfaceType) const;
    const TCHAR* DescribeImpactSurface(EPhysicalSurface SurfaceType) const;
    FName GetSurfaceImpactTag(EPhysicalSurface SurfaceType) const;
    void FireUtilityPulse();
    void ApplyZombieUtilityPulse(float DamageAmount);
    void ApplyStimEffect();
};
