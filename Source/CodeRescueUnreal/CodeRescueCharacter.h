#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputCoreTypes.h"
#include "CodeRescueTypes.h"
#include "CodeRescueCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class UPoseableMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UPointLightComponent;
class UCodeTerminalWidget;
class UUserWidget;
class UNiagaraSystem;
class ACodingTerminalActor;
class ACodeZombieActor;

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueCharacter : public ACharacter
{
    GENERATED_BODY()

    friend class UCodeRescueGameInstance;

public:
    ACodeRescueCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Landed(const FHitResult& Hit) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats")
    float Health = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats")
    float MaxHealth = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats")
    int32 Ammo = 150;
    /** Magazine-based ammo system: current bullets in current magazine. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="1", ClampMax="60"))
    int32 MagazineSize = 30;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Combat")
    int32 MagazineAmmo = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="0.1", ClampMax="10.0"))
    float ReloadDuration = 2.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Combat")
    bool bIsReloading = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats")
    int32 Medkits = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats", meta=(ClampMin="1"))
    int32 MaxAmmo = 300;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats", meta=(ClampMin="0"))
    int32 MaxMedkits = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 ArmorPlates = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 MaxArmorPlates = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0.0", ClampMax="0.95"))
    float ArmorDamageReduction = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 RadioScannerCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 MaxRadioScannerCharges = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 FlashlightBatteries = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 MaxFlashlightBatteries = 6;

    // 2026-07-02: small starting bypass allowance. Lets a stuck player open the survivor route on an
    // early terminal (accessibility net) and makes the terminal->horde loop reachable without first
    // hunting a bypass-kit pickup. Tunable; earned pickups still top it back up to MaxBypassKits.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 BypassKits = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 MaxBypassKits = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Tactical Gear")
    int32 AmmoPouchCapacityBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Tactical Gear", meta=(ClampMin="0"))
    int32 MaxAmmoPouchCapacityBonus = 240;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Tactical Gear")
    bool bFieldFlashlightActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stats")
    bool bClampSuppliesToMaximum = true;

    // 900 cm/s (~9 m/s) — a brisk run that suits the compact 2x-scale city.
    // Was 9000 to cope with the old, broken 50x city span.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement", meta=(ClampMin="150.0", ClampMax="30000.0"))
    float WalkSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement", meta=(ClampMin="128.0", ClampMax="50000.0"))
    float BrakingDeceleration = 18000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement", meta=(ClampMin="10.0", ClampMax="360.0"))
    float DirectKeyboardTurnRate = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement", meta=(ClampMin="10.0", ClampMax="240.0"))
    float DirectKeyboardLookRate = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics", meta=(ClampMin="500.0", ClampMax="5000.0"))
    float SoftLandingSpeed = 1400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics", meta=(ClampMin="0.1", ClampMax="25.0"))
    float FallDamagePer100Speed = 2.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics")
    bool bEnableTrainingLandingAssist = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics", meta=(ClampMin="0.0", ClampMax="500.0"))
    float EnemyHitKnockbackHorizontal = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics", meta=(ClampMin="0.0", ClampMax="500.0"))
    float EnemyHitKnockbackVertical = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Movement|Physics", meta=(ClampMin="0.0", ClampMax="2.0"))
    float DamageMercyWindowSeconds = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Safety", meta=(ClampMin="0.05", ClampMax="1.0"))
    float MaxEnemyDamagePerHitFraction = 0.16f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Safety")
    bool bPreventSingleHitEnemyDeaths = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Safety")
    bool bAutoUseEmergencyMedkit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Safety", meta=(ClampMin="0.05", ClampMax="0.50"))
    float EmergencyMedkitHealthFraction = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Safety", meta=(ClampMin="0.0", ClampMax="120.0"))
    float EmergencyMedkitCooldownSeconds = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Interaction", meta=(ClampMin="300.0", ClampMax="30000.0"))
    float InteractionTraceDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Interaction", meta=(ClampMin="300.0", ClampMax="50000.0"))
    float InteractionAssistRadius = 9000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UCodeTerminalWidget> TerminalWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    /** Optional Blueprint override classes for the journal and pause menu.
     *  Default to the C++ classes if unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UUserWidget> ObjectiveJournalWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    /** Modal lose-screen widget shown when Health hits 0. Falls back to the
     *  C++ class UCodeRescueDeathWidget if unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UUserWidget> DeathWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|VFX")
    UNiagaraSystem* MuzzleFlashVFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|VFX")
    UNiagaraSystem* BulletImpactVFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    class USoundBase* FireCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    class USoundBase* HitConfirmCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    class USoundBase* DryFireCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    bool bEnableReactiveThreatAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio", meta=(ClampMin="500.0", ClampMax="12000.0"))
    float ReactiveThreatAudioRange = 5400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio", meta=(ClampMin="100.0", ClampMax="4000.0"))
    float ReactiveThreatAudioCriticalRange = 1250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio", meta=(ClampMin="0.05", ClampMax="2.0"))
    float ReactiveThreatAudioUpdateInterval = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    bool bEnableCityAmbientZoneDirector = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio", meta=(ClampMin="0.20", ClampMax="5.0"))
    float CityAmbientZoneUpdateInterval = 1.25f;

    /** Minimum seconds between Fire() invocations. Prevents the polled-key
     *  shoot from eating an entire mag in one frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="0.05", ClampMax="2.0"))
    float FireRefireDelay = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="250.0", ClampMax="100000.0"))
    float WeaponRange = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="1.0", ClampMax="500.0"))
    float DirectHitDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="0.0", ClampMax="500.0"))
    float AssistedHitDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="250.0", ClampMax="4500.0"))
    float AssistedHitRadius = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat", meta=(ClampMin="1.0", ClampMax="8.0"))
    float AssistedHitMaxAngleDegrees = 6.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat")
    bool bEnableAssistedHit = true;

    /** Holding aim acquires one visible hostile inside this distance. The
     * lock redirects the real weapon trace; it never applies remote damage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat|Target Lock", meta=(ClampMin="500.0", ClampMax="6000.0"))
    float TargetLockMaxDistance = 4200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat|Target Lock", meta=(ClampMin="4.0", ClampMax="35.0"))
    float TargetLockAcquireAngleDegrees = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat|Target Lock", meta=(ClampMin="8.0", ClampMax="55.0"))
    float TargetLockBreakAngleDegrees = 36.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat|Target Lock", meta=(ClampMin="1.0", ClampMax="30.0"))
    float TargetLockCameraTurnSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice")
    bool bEnableCombatJuice = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="5.0"))
    float CombatJuiceFireKickPitch = 0.56f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="3.0"))
    float CombatJuiceFireKickYaw = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="0.20"))
    float CombatJuiceHitStopSeconds = 0.045f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="3.0"))
    float CombatJuiceHitConfirmKick = 0.42f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="6.0"))
    float CombatJuiceDamageKick = 1.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Combat Juice", meta=(ClampMin="0.0", ClampMax="2.0"))
    float CombatJuiceReloadSettleKick = 0.34f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Healing", meta=(ClampMin="1.0", ClampMax="500.0"))
    float MedkitHealAmount = 75.0f;
    /** Stamina system for sprint movement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stamina", meta=(ClampMin="10.0", ClampMax="500.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Stamina")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stamina", meta=(ClampMin="1.0", ClampMax="5.0"))
    float SprintSpeedMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stamina", meta=(ClampMin="5.0", ClampMax="50.0"))
    float StaminaDrainRate = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stamina", meta=(ClampMin="1.0", ClampMax="50.0"))
    float StaminaRegenRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stamina", meta=(ClampMin="0.1", ClampMax="5.0"))
    float JumpStaminaCost = 15.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Stamina")
    bool bIsSprinting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="0.0", ClampMax="1.0"))
    float StealthNoiseLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="0.25", ClampMax="8.0"))
    float StealthNoiseDecayRate = 1.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="300.0", ClampMax="10000.0"))
    float QuietMovementNoiseRadius = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="500.0", ClampMax="14000.0"))
    float SprintNoiseRadius = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="500.0", ClampMax="18000.0"))
    float WeaponNoiseRadius = 5200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Stealth", meta=(ClampMin="300.0", ClampMax="10000.0"))
    float UtilityNoiseRadius = 1500.0f;

    UFUNCTION(BlueprintCallable, Category="Code Rescue")
    void ApplyDamage(float DamageAmount, AActor* DamageSource = nullptr);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Stats")
    int32 AddAmmo(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Stats")
    int32 AddMedkits(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddFlares(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddSmokes(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddStims(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddArmorPlates(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddRadioScannerCharges(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddFlashlightBatteries(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddBypassKits(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    int32 AddAmmoPouch(int32 CapacityBonus);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    bool TrySpendBypassKit(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    void ToggleFlashlight();

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Tactical Gear")
    void UseRadioScanner();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetArmorPlates() const { return ArmorPlates; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetMaxArmorPlates() const { return MaxArmorPlates; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetRadioScannerCharges() const { return RadioScannerCharges; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetMaxRadioScannerCharges() const { return MaxRadioScannerCharges; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetFlashlightBatteries() const { return FlashlightBatteries; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetMaxFlashlightBatteries() const { return MaxFlashlightBatteries; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetBypassKits() const { return BypassKits; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetMaxBypassKits() const { return MaxBypassKits; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    int32 GetAmmoPouchCapacityBonus() const { return AmmoPouchCapacityBonus; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Tactical Gear")
    FString GetFieldKitSummary() const;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Stealth")
    void ReportStealthNoise(float NoiseLevel, float NoiseRadius, const FString& Reason);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Stealth")
    float GetStealthNoiseLevel() const { return StealthNoiseLevel; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Stealth")
    float GetStealthNoiseRadius() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Stealth")
    FString GetStealthStateSummary() const;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Stats")
    void RestorePlayerResources(float SavedHealth, int32 SavedAmmo, int32 SavedMedkits);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Stats")
    void RestorePlayerResourcesDetailed(
        float SavedHealth,
        int32 SavedAmmo,
        int32 SavedMedkits,
        int32 SavedArmorPlates,
        int32 SavedFlares,
        int32 SavedSmokes,
        int32 SavedStims,
        int32 SavedScrap,
        int32 SavedRadioScannerCharges,
        int32 SavedFlashlightBatteries,
        int32 SavedBypassKits,
        int32 SavedAmmoPouchCapacityBonus);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Interaction")
    float GetInteractionTraceDistance() const { return InteractionTraceDistance; }

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Guidance")
    FString GetOpenWorldGuidanceText() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Identity")
    FString GetOperatorIdentitySummary() const;

    UFUNCTION(BlueprintCallable, Exec, Category="Code Rescue|Safety")
    void RecoverToCityArena();

    /** Global flag: true while a modal UI (terminal, menu) owns input.
     *  PollDirectKeys early-returns when set, so polled W/A/S/D/E/F/Space/T
     *  cannot leak into gameplay while the player is typing in the terminal. */
    static void SetUIOpen(bool bOpen) { bUIOpen = bOpen; }
    static bool IsUIOpen() { return bUIOpen; }

    UFUNCTION(BlueprintCallable, Exec, Category="Code Rescue|Camera")
    void SelectCameraPerspective(int32 NewPerspective);

    UFUNCTION(BlueprintCallable, Exec, Category="Code Rescue|Camera")
    void CycleCameraPerspective();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Camera")
    int32 GetCameraPerspectiveIndex() const { return CameraPerspective; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Camera")
    FString GetCameraPerspectiveLabel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Camera")
    UCameraComponent* GetActiveGameplayCamera() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat|Target Lock")
    bool IsAimTargetLocked() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat|Target Lock")
    ACodeZombieActor* GetLockedAimTarget() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat|Target Lock")
    FString GetAimTargetLockSummary() const;

    /** Read-write objective index so the save system can restore it. */
    int32 GetObjectiveIndex() const { return ObjectiveIndex; }
    void SetObjectiveIndex(int32 NewIndex) { ObjectiveIndex = NewIndex; }


    UFUNCTION(BlueprintCallable, Category="Code Rescue|Combat")
    void Reload();

    UFUNCTION()
    void OnReloadComplete();

    /** Hit zone classification from bone name. */
    EHitZone ClassifyHitZone(const FName& BoneName) const;

    // ---- #26 weapon system -------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Weapons")
    TArray<FWeaponDef> WeaponLoadout;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Weapons")
    EWeaponType ActiveWeapon = EWeaponType::Pistol;

    /** Current per-weapon magazine ammo (parallel to WeaponLoadout). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Weapons")
    TArray<int32> WeaponMagazines;

    /** Current per-weapon reserve ammo (parallel to WeaponLoadout). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Weapons")
    TArray<int32> WeaponReserveAmmo;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Weapons")
    void SwapWeapon(EWeaponType NewWeapon);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Weapons")
    void CycleWeapon(int32 Direction);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Weapons")
    void CycleWeaponNext();

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Weapons")
    void CycleWeaponPrevious();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    int32 GetActiveWeaponReserveAmmo() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    int32 GetWeaponCount() const { return WeaponLoadout.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    FString GetActiveWeaponName() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    FString GetActiveWeaponTacticalRole() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    int32 GetActiveWeaponMagazineSize() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Weapons")
    FString GetWeaponQuickSlotSummary() const;

    /** Resolve the authored mesh used by both the in-world weapon and the
     * pause-armory preview. Every loadout entry returns a visible model. */
    UStaticMesh* ResolveWeaponPreviewMesh(EWeaponType Weapon) const;

    /** C++ armory helpers; return nullptr/zero for an invalid loadout slot. */
    const FWeaponDef* GetWeaponDefinition(EWeaponType Weapon) const;
    int32 GetWeaponMagazineAmmo(EWeaponType Weapon) const;
    int32 GetWeaponReserveAmmo(EWeaponType Weapon) const;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Weapons")
    void RestoreWeaponQuickSlotState(
        EWeaponType SavedActiveWeapon,
        const TArray<int32>& SavedMagazines,
        const TArray<int32>& SavedReserveAmmo);

    /** Init the loadout to the full demo arsenal. Called from BeginPlay if
     *  WeaponLoadout is missing any of the current default weapon slots. */
    void InitDefaultWeaponLoadout();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Throwables")
    int32 GetActiveThrowableSlot() const { return ActiveThrowableSlot; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Throwables")
    int32 GetThrowableCountForSlot(int32 SlotIndex) const
    {
        return (SlotIndex == 0) ? FlareCount :
               (SlotIndex == 1) ? SmokeCount :
               (SlotIndex == 2) ? StimCount : 0;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Scrap")
    int32 GetScrap() const { return Scrap; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    float GetLastHeadshotWorldTime() const { return LastHeadshotTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    float GetLastDamageWorldTime() const { return LastDamageWorldTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    FString GetLastDamageLocationText() const { return LastDamageLocationText; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    FString GetLastDamageSourceText() const { return LastDamageSourceText; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    float GetLastDamageAmount() const { return LastDamageAmount; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    float GetLastDamageSourceDistanceMeters() const { return LastDamageSourceDistanceMeters; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    FString GetLastDamageMitigationText() const { return LastDamageMitigationText; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    float GetLastSquadRegroupWorldTime() const { return LastSquadRegroupWorldTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    int32 GetLastSquadRegroupCount() const { return LastSquadRegroupCount; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    FString GetSquadFormationLabel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    float GetSquadFormationSpacingScale() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    float GetLastSquadFormationWorldTime() const { return LastSquadFormationWorldTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    FString GetSquadOrderLabel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    float GetLastSquadOrderWorldTime() const { return LastSquadOrderWorldTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    bool IsSquadHoldingPosition() const { return bSquadHoldPosition; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Squad")
    float GetLastManualMedicCallWorldTime() const { return LastManualMedicCallWorldTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Code Rescue|Combat")
    float GetEmergencyMedkitReadySeconds() const;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Scrap")
    int32 GrantScrap(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Scrap")
    bool TrySpendScrap(int32 Amount);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Camera")
    UCameraComponent* FirstPersonCamera;

    /** Spring arm + camera for the third-person perspectives. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Camera")
    UCameraComponent* ThirdPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Animation")
    USkeletalMeshComponent* FirstPersonArmsMesh;

    /** Runtime copy of the locomotion pose. The copy receives a procedural
     * two-arm aiming pose without replacing the production locomotion AnimBP. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Animation")
    UPoseableMeshComponent* AimingPresentationMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Weapons")
    UStaticMeshComponent* FirstPersonWeaponSilhouette;

    /** 2026-07-07: held-weapon model on the body's right hand — visible from
     *  every third-person camera (the FP silhouette is owner-only + camera
     *  parented, so it never showed on the character). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Weapons")
    UStaticMeshComponent* ThirdPersonWeaponMesh = nullptr;

    /** Debounce so polled + bound weapon-cycle inputs can't double-fire. */
    float WeaponCycleCooldown = 0.0f;

    /** 2026-07-04 (top-50 item 50): PHOTO MODE — F10 hides every viewport widget and
     *  slows time to 12% for clean captures (F12 remains the engine screenshot key).
     *  Original widget visibilities are restored exactly on exit. */
    bool bPhotoModeActive = false;
    TArray<TPair<TWeakObjectPtr<UUserWidget>, ESlateVisibility>> PhotoModeHiddenWidgets;
    void TogglePhotoMode();

    /** 2026-07-04: v2 authored player body (SurvivorKenny) + facial morph driver + FP weapon art. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Face")
    class UCodeRescueFacialExpressionComponent* FacialExpression = nullptr;
    UPROPERTY() class UAnimSequence* V2IdleAnim = nullptr;
    UPROPERTY() class UAnimSequence* V2WalkAnim = nullptr;
    UPROPERTY() class UAnimSequence* V2RunAnim = nullptr;
    bool bUsingV2Body = false;
    int32 V2BodyAnimState = -1;

    /** 2026-07-11 pass 4: HERO third-person presentation (SurvivorKennyV4 —
     *  5'10", red beard, blue eyes). The Manny production rig keeps DRIVING
     *  locomotion + the aim/landing presentation (audit contract); this
     *  visible skeletal body PRESENTS the hero in non-FP perspectives. */
    UPROPERTY() class USkeletalMeshComponent* HeroPresentationMesh = nullptr;
    UPROPERTY() class UAnimSequence* HeroIdleAnim = nullptr;
    UPROPERTY() class UAnimSequence* HeroWalkAnim = nullptr;
    UPROPERTY() class UAnimSequence* HeroRunAnim = nullptr;
    bool bHeroPresentationConfigured = false;
    int32 HeroBodyAnimState = -1;

public:
    /** 2026-07-16 pass 5: aim-down-sights + scope magnification.
     *  RMB HOLD = aim (weapon centers on the bore, FOV narrows). Z cycles the
     *  scope ladder 1x/2x/5x/10x/20x/50x on scoped weapons. LMB fires — so the
     *  player aims FIRST, with full crosshair/scope/lock assistance, and only
     *  then chooses to shoot. */
    bool IsADSActive() const { return bADSActive; }
    // 2026-07-17: TRUE while the player is looking THROUGH the optic of a
    // scope-capable weapon (full-screen circular scope + reticle + thermal
    // tint drawn by the HUD; the weapon body hides). Non-scope weapons keep
    // the down-the-barrel viewmodel instead.
    bool IsScopeViewActive() const;
    float GetCurrentScopeZoom() const;
    FString GetScopeZoomLabel() const;
    void CycleScopeZoom();
    bool WeaponSupportsScopeZoom(EWeaponType Weapon) const;
    bool WeaponIsGrenadeFamily(EWeaponType Weapon) const;

    /** Grenade payload detonation callback (from AThrowableActor at the REAL
     *  landing point after the fuse). Applies gameplay area effect + the
     *  physical explosion presentation for the grenade type. */
    void DetonateGrenadePayload(const FVector& Location, uint8 WeaponTypeRaw);

private:
    bool bADSActive = false;
    int32 ScopeZoomIndex = 0;
    float ADSBlend = 0.0f;
    // 2026-07-17: aim from any camera — remember where the player was
    // looking from so releasing aim hands the same perspective back.
    int32 PreAimCameraPerspective = 0;
    bool bRestorePerspectiveOnAimEnd = false;
    void TakeGameplayScreenshot();   // Cmd+Shift+4 / F12 (Kenny's report workflow)
    // 2026-07-17 trackpad reality (Kenny: two-finger press = right click, so
    // holding aim makes a left-click FIRE physically impossible on the pad):
    // a QUICK right-click LATCHES the sights up, the next click lowers them;
    // a LONG hold stays momentary (mouse users keep hold-to-aim). Wall-clock
    // timed so slow-motion effects can't distort the click window.
    void OnAimPressed();
    void OnAimReleased();
    double AimPressWallTime = 0.0;
    bool bAimToggleLatched = false;
    float BaseFirstPersonFOV = 90.0f;
    float ADSLookScale = 1.0f;
    void UpdateADSPresentation(float DeltaSeconds);
    void PlayExplosionPresentation(const FVector& Location, const struct FWeaponDef& WeaponDef);
    bool ComputeGrenadeLaunch(FVector& OutStart, FVector& OutVelocity) const;
    void SpawnGrenadeProjectile(const struct FWeaponDef& WeaponDef);
    void UpdateGrenadeArcPreview(float DeltaSeconds);
    UPROPERTY() TArray<class UStaticMeshComponent*> GrenadeArcDots;
    UPROPERTY() class UStaticMeshComponent* GrenadeLandingRing = nullptr;
    // blast-radius OUTLINE: rim segments arranged on the predicted blast
    // circle (a filled disc read as "the world turned orange" in review).
    UPROPERTY() TArray<class UStaticMeshComponent*> GrenadeRingSegments;
    bool bGrenadePreviewVisible = false;

    /** -CodeRescuePerspectiveReview: staged self-driving visual review —
     *  cycles every camera perspective, ADS zoom ladder, and a live grenade
     *  arc + detonation, screenshotting each stage, then exits. */
    void StartPerspectiveReviewHarness();
    void AdvancePerspectiveReview();
    int32 PerspectiveReviewStage = -1;
    FTimerHandle PerspectiveReviewTimer;
    FTimerHandle ResumeHealthPulseTimer;   // 2026-07-16: -CodeRescueAutoResumeLanguage heartbeat
    int32 ResumeHealthPulseCount = 0;

protected:   // restore the surrounding section's access level (UHT contract)

    /** Explicit production animation set. This avoids depending on a sample
     *  AnimBP whose movement variables are not driven by this custom pawn. */
    UPROPERTY() class UAnimSequence* MannyIdleAnim = nullptr;
    UPROPERTY() class UAnimSequence* MannyWalkAnim = nullptr;
    UPROPERTY() class UAnimSequence* MannyRunAnim = nullptr;
    UPROPERTY() class UAnimSequence* MannyJumpAnim = nullptr;
    UPROPERTY() class UAnimSequence* MannyFallAnim = nullptr;
    UPROPERTY() class UAnimSequence* MannyLandAnim = nullptr;
    bool bUsingAuthoredMannyAnimation = false;
    int32 MannyAnimationState = -1;
    uint8 MannyObservedAnimationStateMask = 0;
    float MannyLandingPresentationRemaining = 0.0f;
    /** Swap the visible first-person weapon model to match ActiveWeapon (authored RawArt meshes). */
    void RefreshFirstPersonWeapon();
    /** Idle/Walk/Run single-node animation switching for the v2 body, by ground speed. */
    void UpdateV2BodyLocomotion(float DeltaSeconds);
    /** Idle, walk, run, jump, fall, and land animation controller for Manny. */
    void UpdateAuthoredMannyAnimation(float DeltaSeconds);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Tactical Gear")
    UPointLightComponent* FieldFlashlight;

    /** 0 first-person, 1 third-person, 2 tactical, 3 top-down, 4 isometric, 5 side/2.5D. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Camera")
    int32 CameraPerspective = 0;

    /** Cycle every active gameplay perspective. Bound to C. */
    void CyclePerspective();
    void ApplyCameraPerspective();
    void SelectFirstPersonPerspective();
    void SelectThirdPersonPerspective();
    void SelectTacticalPerspective();
    void SelectTopDownPerspective();
    void SelectIsometricPerspective();
    void SelectSidePerspective();
    bool IsFixedCameraPerspective() const;
    FVector GetPerspectiveMoveForwardVector() const;
    FVector GetPerspectiveMoveRightVector() const;
    void UpdateFirstPersonArms(float DeltaSeconds);
    void ConfigureAimingPresentationMesh();
    void UpdateAutoTargetLock(float DeltaSeconds);
    void UpdateWeaponAimPresentation(float DeltaSeconds);
    FVector GetAimTargetPoint(const ACodeZombieActor* Target) const;
    bool IsAimTargetCandidate(const ACodeZombieActor* Target, float MaxAngleDegrees, float MaxDistance) const;
    ACodeZombieActor* FindBestAimTarget(float MaxAngleDegrees, float MaxDistance, float MaxReticleDistance = 0.0f) const;
    void UpdateFirstPersonWeaponPresentation(float DeltaSeconds);
    float GetCombatJuiceMotionScale() const;
    void TriggerCombatJuiceFireCue(const FWeaponDef* WeaponDef, bool bMeleeOrDryFire);
    void TriggerCombatJuiceHitConfirm(const FVector& ImpactPoint, EHitZone HitZone, bool bGameplayHit);
    void TriggerCombatJuiceReloadStageCue(float StageAlpha, bool bComplete);
    void TriggerCombatJuiceDamageCue(float EffectiveDamage, AActor* DamageSource);
    void UpdateCombatJuice(float DeltaSeconds);
    void UpdateReactiveThreatAudio(float DeltaSeconds);
    void UpdateCityAmbientZoneAudio(float DeltaSeconds);
    void UpdateStealthNoise(float DeltaSeconds);

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);

    void Fire();
    void TryJump();
    void StopJumpAction();
    void BeginAim();
    void EndAim();
    void Interact();
    void UseMedkit();

    /** No-arg wrappers so BindKey can target weapon swaps directly
     *  (SwapWeapon itself takes an EWeaponType argument). */
    void SwapToPistol();
    void SwapToShotgun();
    void SwapToRifle();
    void SwapToGrenade();
    void SelectWeaponSlot1();
    void SelectWeaponSlot2();
    void SelectWeaponSlot3();
    void SelectWeaponSlot4();
    void SelectWeaponSlot5();
    void SelectWeaponSlot6();
    void SelectWeaponSlot7();
    void SelectWeaponSlot8();
    void SelectWeaponSlot9();
    void SelectWeaponSlot10();
    void SwapToWeaponSlot(int32 SlotIndex);

    /** #27 — emergency melee swing when both magazine + reserve are empty.
     *  0.6s cooldown, 200u arc, 80 damage. Saves the player from softlock. */
    void MeleeAttack();
    float TimeSinceLastMelee = 99.0f;
    void EnsureWeaponStateInitialized();
    void SyncActiveWeaponStateFromLoadout();
    void RefreshLegacyAmmoFromWeaponReserves();
    int32 AddAmmoToWeaponIndex(int32 WeaponIdx, int32 Amount);
    int32 AddBoundedResource(int32& Value, int32 Amount, int32 MaxValue);
    int32 ApplyAreaWeaponEffect(const FVector& ImpactPoint, const FWeaponDef& WeaponDef, const FString& EffectLabel);
    EHitZone ClassifyImpactPoint(const class ACodeZombieActor* Zombie, const FHitResult& Hit) const;
    bool HasClearWeaponPath(const FVector& Start, const FVector& End, const AActor* IntendedTarget) const;

    // ---- #28 throwables ---------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Throwables")
    int32 FlareCount = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Throwables")
    int32 SmokeCount = 2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Throwables")
    int32 StimCount = 2;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|Throwables")
    int32 ActiveThrowableSlot = 0;     // 0 = flare, 1 = smoke, 2 = stim
    void ThrowActive();
    void CycleThrowable();

    // ---- #30 barricades + scrap -------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Scrap")
    int32 Scrap = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Scrap")
    int32 BarricadeScrapCost = 5;
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Scrap") int32 AddScrap(int32 Amount);
    void PlaceBarricade();

    void PollDirectKeys(float DeltaSeconds);
    /** #39 — gamepad polling. Reads stick axes + face buttons; complements
     *  PollDirectKeys. Same UI-gate honored. */
    void PollGamepad(float DeltaSeconds);
    void ShowMissionHelp();
    void TeleportToNextObjective();
    void ToggleObjectiveJournal();
    void TogglePauseMenu();
    void RegroupRescueTeam();
    void CycleSquadFormation();
    void CallSquadMedic();
    void ToggleSquadHoldPosition();

    bool TraceForward(FHitResult& Hit, float Distance) const;
    AActor* FindNearestInteractable(float Radius) const;
    void ApplyRuntimeTuning();
    void UpdateArenaSafety(float DeltaSeconds);
    void StartFirstLevelCombatRuntimeAudit();

private:
    float DirectKeyCooldown = 0.0f;

    /** 2026-07-06 movement-lock fixes: watchdog that self-heals a pawn whose
     *  movement input is held while velocity stays zero (wedged/stale state),
     *  and grace timer for the launch-gate-without-menu failsafe. */
    void UpdateStuckMovementWatchdog(float DeltaSeconds);
    /** 2026-07-07: hide the body when the boom collapses onto the pawn so the
     *  player sees the room, not the back of their own head. */
    void UpdateCameraProximityFade();
    bool bCameraProximityHidden = false;
    /** Independently detects visible architecture between the pawn and the
     *  active boom camera, including decorative meshes with collision disabled. */
    void UpdateCameraOcclusion(float DeltaSeconds);
    float CameraOcclusionAccumulator = 0.0f;
    TArray<TWeakObjectPtr<AActor>> CameraOcclusionHiddenActors;
    /** 2026-07-07: one-shot post-spawn footing check — relocates the pawn if
     *  the spawn point is embedded in world geometry. */
    void EnsureSpawnClearance(float DeltaSeconds);
    bool bSpawnClearanceDone = false;
    float SpawnClearanceDelay = 0.0f;
    float StuckMovementSeconds = 0.0f;
    float LanguageGateNoMenuSeconds = 0.0f;
    /** 2026-07-11 launch-crash harness: -CodeRescueAutoResumeLanguage=<name>
     *  drives the exact packaged resume path (gate -> ResumeLanguageRun ->
     *  OpenLevel) with no keyboard, so real-save launch regressions can be
     *  reproduced and gated headlessly. One-shot per session. */
    bool bLaunchAutoResumeConsumed = false;
    /** Consecutive watchdog fires without free movement — drives escalation. */
    int32 StuckRescueEscalation = 0;
    /** 2026-07-07: depenetrate/relocate a teleport destination so the pawn
     *  can never be placed inside geometry. Returns false if no clear spot. */
    bool AdjustTeleportDestination(FVector& InOutDestination) const;

    UPROPERTY(Transient)
    int64 AppliedSkillTreeUnlockedMask = 0;

    /** Tracks time since last Fire() so FireRefireDelay can gate it. */
    float TimeSinceLastFire = 99.0f;

    /** World-time of the last shot. The fire-rate gate uses this instead of
     *  the Tick-incremented counter above, so firing keeps working even if
     *  the actor Tick is ever impaired. -100 = "never fired yet". */
    float LastFireWorldTime = -100.0f;
    int32 ObjectiveIndex = 0;
    float BoundMoveForwardValue = 0.0f;
    float BoundMoveRightValue = 0.0f;
    float BoundTurnValue = 0.0f;
    float BoundLookUpValue = 0.0f;
    float HighestFallingDownSpeed = 0.0f;
    float LastDamageWorldTime = -100.0f;
    float LastCameraInputWorldTime = -100.0f;
    float FirstPersonArmsSwayTime = 0.0f;
    float AimPresentationAlpha = 0.0f;
    float AimHoldTimer = 0.0f;
    bool bAimInputHeld = false;
    bool bAimPresentationConfigured = false;
    TWeakObjectPtr<ACodeZombieActor> LockedAimTarget;
    float LastTargetLockChangeWorldTime = -100.0f;
    float FirstPersonWeaponPresentationTime = 0.0f;
    float LastWeaponPresentationFireWorldTime = -100.0f;
    float LastWeaponPresentationReloadWorldTime = -100.0f;
    EWeaponType LastPresentedWeapon = EWeaponType::Pistol;
    bool bWeaponPresentationProfileInitialized = false;
    float LastCombatJuiceFireWorldTime = -100.0f;
    float LastCombatJuiceHitConfirmWorldTime = -100.0f;
    float LastCombatJuiceReloadStageWorldTime = -100.0f;
    float LastCombatJuiceDamageWorldTime = -100.0f;
    float LastCombatJuiceHitStopDuration = 0.0f;
    float LastCombatJuiceHitStopScale = 0.0f;
    bool bLastCombatJuiceHeadshot = false;
    float LastReactiveThreatAudioWorldTime = -100.0f;
    float LastReactiveThreatAudioCaptionWorldTime = -100.0f;
    float ReactiveThreatAudioSmoothedIntensity = 0.0f;
    FString LastReactiveThreatAudioState = TEXT("calm");
    float LastCityAmbientZoneWorldTime = -100.0f;
    float LastCityAmbientZoneCaptionWorldTime = -100.0f;
    FString LastCityAmbientZoneLabel = TEXT("entry approach");
    float LastStealthNoiseWorldTime = -100.0f;
    float LastStealthNoiseRadius = 0.0f;
    FString LastStealthNoiseReason = TEXT("quiet");

    /** Live journal widget (J key). Null when the journal is closed. */
    UPROPERTY()
    UUserWidget* ActiveJournalWidget = nullptr;

    /** Live pause menu widget (P key). Null when the menu is closed. */
    UPROPERTY()
    UUserWidget* ActivePauseWidget = nullptr;

    /** Live damage-feedback overlay (#21 wiring). */
    UPROPERTY()
    class UCodeRescueDamageFeedbackWidget* DamageFeedbackWidget = nullptr;

    /** Timer handle for reload completion callback. */
    FTimerHandle ReloadTimerHandle;

    /** Tracks time since last headshot for HUD feedback. */
    float LastHeadshotTime = -99.0f;

    FString LastDamageLocationText = TEXT("clear");
    FString LastDamageSourceText = TEXT("none");
    float LastDamageAmount = 0.0f;
    float LastDamageSourceDistanceMeters = -1.0f;
    FString LastDamageMitigationText = TEXT("none");
    float LastSquadRegroupWorldTime = -100.0f;
    int32 LastSquadRegroupCount = 0;
    float LastSquadFormationWorldTime = -100.0f;
    int32 SquadFormationMode = 1;
    bool bSquadHoldPosition = false;
    float LastSquadOrderWorldTime = -100.0f;
    int32 LastSquadOrderCount = 0;
    float LastManualMedicCallWorldTime = -100.0f;
    bool bLastManualMedicCallSucceeded = false;
    float LastEmergencyMedkitWorldTime = -100.0f;
    float LastCriticalHealthCalloutWorldTime = -100.0f;
    FVector LastSafeArenaLocation = FVector::ZeroVector;
    int32 LastSafeArenaCityIndex = INDEX_NONE;
    bool bHasLastSafeArenaLocation = false;
    float LastArenaSafetyRescueWorldTime = -100.0f;
    float LastBiteWoundWorldTime = -100.0f;
    int32 SpawnedBiteWoundCount = 0;

    TWeakObjectPtr<ACodeZombieActor> FirstLevelCombatAuditTarget;
    int32 FirstLevelCombatAuditTraceHits = 0;
    bool bFirstLevelCombatAuditJumpPassed = false;
    bool bFirstLevelCombatAuditBitePassed = false;
    bool bFirstLevelCombatAuditCorpsePassed = false;
    bool bFirstLevelCombatAuditFadePassed = false;
    bool bFirstLevelCombatAuditTargetLockPassed = false;
    bool bFirstLevelCombatAuditMissLocalityPassed = false;
    FVector FirstLevelCombatAuditCorpseLocation = FVector::ZeroVector;
    FVector FirstLevelCombatAuditCorpseScale = FVector::OneVector;

    void SpawnAnatomicalBiteWound(AActor* DamageSource);

    /** Backing storage for SetUIOpen / IsUIOpen. Static because the terminal
     *  widget toggles this without holding a character pointer. Single-player
     *  game, so static is safe. */
    static bool bUIOpen;
};
