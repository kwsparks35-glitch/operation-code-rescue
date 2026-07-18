#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CompanionActor.generated.h"

class USoundBase;
class UPointLightComponent;
class UStaticMeshComponent;

/**
 * Friendly rescue-team member that follows the player in formation, avoids
 * physically blocking the player, and contributes light support fire.
 */
UCLASS()
class CODERESCUEUNREAL_API ACompanionActor : public ACharacter
{
    GENERATED_BODY()

public:
    ACompanionActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    int32 MagazineAmmo = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    int32 MagazineSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float FollowOffset = 600.0f;

    /** Companion's display name; used in subtitle on death. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    FString DisplayName = TEXT("Survivor Companion");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    FString RoleLabel = TEXT("Rifle Support");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    FString MechanicalIdentity = TEXT("Steady support fire");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    FString BarkStyle = TEXT("Concise overwatch callouts");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    FLinearColor RoleAccentColor = FLinearColor(0.28f, 0.58f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float LateralFollowOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float PersonalSpaceRadius = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
    float FormationSpacingScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Combat")
    float CombatDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Combat")
    float RefireDelay = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Support")
    bool bMedicSupport = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Support")
    float MedicHealAmount = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Support")
    float MedicPulseCooldown = 12.0f;

    /** Enables a lightweight visual-only companion gesture layer for readable
     * idle, formation, order, support-fire, and medic-pulse states. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability")
    bool bEnableCompanionGestureReadability = true;

    /** Scales small additive mesh/light motion without changing the capsule or
     * movement target. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability", meta=(ClampMin="0.0", ClampMax="3.0"))
    float CompanionIdleGestureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability", meta=(ClampMin="0.1", ClampMax="2.5"))
    float SupportFireGestureDuration = 0.42f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability", meta=(ClampMin="0.1", ClampMax="3.0"))
    float MedicPulseGestureDuration = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability", meta=(ClampMin="0.1", ClampMax="2.5"))
    float OrderGestureDuration = 0.72f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Animation Readability", meta=(ClampMin="0.1", ClampMax="2.5"))
    float DamageGestureDuration = 0.55f;

    UFUNCTION(BlueprintCallable)
    void TakeCompanionDamage(float Amount);

    UFUNCTION(BlueprintCallable)
    bool IsOperational() const { return !bIsDead && Health > 0.0f; }

    /** Persists a deferred visible-foot correction into the gesture base. */
    bool RefreshGroundedVisualPose();

    UFUNCTION(BlueprintCallable)
    float GetMedicPulseReadySeconds() const;

    UFUNCTION(BlueprintCallable)
    bool TryManualMedicPulse(class ACodeRescueCharacter* Player, FString& OutMessage);

    UFUNCTION(BlueprintCallable)
    void RegroupNearPlayer(class APawn* PlayerPawn, int32 FormationIndex);

    UFUNCTION(BlueprintCallable)
    void ApplyFormationSpacingScale(float NewScale);

    UFUNCTION(BlueprintCallable)
    void ConfigureSquadPersonality(
        const FString& InDisplayName,
        const FString& InRoleLabel,
        const FString& InMechanicalIdentity,
        const FString& InBarkStyle,
        const FLinearColor& InRoleAccentColor);

    UFUNCTION(BlueprintPure)
    FString GetHudCallsign() const;

    UFUNCTION(BlueprintPure)
    FString GetRoleStatusLabel() const;

    UFUNCTION(BlueprintPure)
    FString GetOrderResponseBark(const FString& OrderLabel) const;

    UFUNCTION(BlueprintCallable)
    void PushRoleOrderBark(const FString& OrderLabel, float DurationSeconds = 2.6f);

    UFUNCTION(BlueprintCallable)
    void SetHoldPosition(const FVector& NewHoldLocation, const FRotator& NewHoldRotation);

    UFUNCTION(BlueprintCallable)
    void ClearHoldPosition();

    UFUNCTION(BlueprintCallable)
    bool IsHoldingPosition() const { return bHoldPosition; }

private:
    float TimeSinceShot = 99.0f;
    float TimeSinceReload = 0.0f;
    float TimeSinceSupportPulse = 99.0f;
    float TimeSinceRoleBark = 99.0f;
    bool bIsDead = false;
    bool bCorpseFadeActive = false;
    float CorpseFadeElapsed = 0.0f;
    float CorpseFadeDuration = 3.0f;
    FVector CorpseFadeStartLocation = FVector::ZeroVector;
    bool bHoldPosition = false;
    FVector HoldLocation = FVector::ZeroVector;
    FRotator HoldRotation = FRotator::ZeroRotator;
    bool bHasCapturedBaseFormation = false;
    float BaseFollowOffset = 600.0f;
    float BaseLateralFollowOffset = 0.0f;
    float BasePersonalSpaceRadius = 260.0f;
    bool bCompanionGestureBasePoseCached = false;
    float CompanionGesturePhase = 0.0f;
    float SupportFireGestureTimer = 0.0f;
    float MedicPulseGestureTimer = 0.0f;
    float OrderGestureTimer = 0.0f;
    float DamageGestureTimer = 0.0f;
    FTransform MeshGestureBaseTransform;
    FTransform RoleSignalLightBaseTransform;
    float RoleSignalLightBaseIntensity = 850.0f;

    UPROPERTY()
    UPointLightComponent* RoleSignalLight = nullptr;

    UPROPERTY()
    UStaticMeshComponent* ResponderPack = nullptr;

    void TryFireAtNearbyZombie(class APawn* PlayerPawn);
    void TrySupportPlayer(class APawn* PlayerPawn);
    void CacheCompanionGestureBasePose(bool bTagForAudit = false);
    void UpdateCompanionGesture(float DeltaSeconds);
    void TriggerSupportFireGesture();
    void TriggerMedicPulseGesture();
    void TriggerOrderGesture();
    void TriggerDamageGesture();
    void RefreshRoleSignalLight();
    void BeginCorpseFade();
    void UpdateCorpseFade(float DeltaSeconds);
};
