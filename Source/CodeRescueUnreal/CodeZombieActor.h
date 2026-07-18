#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CodeRescueTypes.h"
#include "CodeZombieActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPoseableMeshComponent;
class UPointLightComponent;
class UNiagaraSystem;
class UAnimInstance;
class UAnimMontage;
class UAnimSequence;
class UAudioComponent;
class UPhysicalAnimationComponent;
class USoundBase;
class UDataTable;
class UDecalComponent;

UENUM(BlueprintType)
enum class ECodeRescueZombieEncounterRole : uint8
{
    Default = 0,
    Anchor = 1,
    Flanker = 2,
    Pressure = 3,
    Sentinel = 4
};

UCLASS()
class CODERESCUEUNREAL_API ACodeZombieActor : public ACharacter
{
    GENERATED_BODY()

public:
    ACodeZombieActor();
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    /** #20 — distant-zombie tick throttle accumulator. Resets when within
     *  10000 units of the player; while beyond that range, only one tick
     *  out of every ~2 seconds runs the chase + attack logic. */
    float DistantTickAccumulator = 0.0f;

    /** #29 — Elite ability cooldown. Spitter ranged attack / Charger
     *  sprint dash use this. */
    float TimeSinceEliteAbility = 99.0f;

    /** #29 — fire elite-specific behavior on Tick. Returns true if the
     *  caller should skip the normal melee/chase logic this tick. */
    bool TickEliteBehavior(float DeltaSeconds, APawn* PlayerPawn, float DistanceToPlayer);

    /** #29 — Boomer death effect. AoE + spawn small zombies. */
    void OnBoomerDeath();

public:

    /** Stable per-spawn ID assigned by ACodeRescueGameMode::SpawnWorld so the
     *  save system can record exactly which zombies were neutralized and skip
     *  respawning them (or destroy them post-spawn) when the game reloads. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    int32 ZombieId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    float Health = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    float MoveSpeed = 145.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    float AttackRange = 130.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    float AttackDamage = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    float ActivationRange = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Standard Pursuit")
    bool bStandardDirectPursuitEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Standard Pursuit", meta=(ClampMin="0.5", ClampMax="4.0"))
    float StandardPursuitAttackCooldown = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Standard Pursuit", meta=(ClampMin="250.0", ClampMax="4500.0"))
    float StandardPursuitReadabilityRange = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Standard Pursuit", meta=(ClampMin="150.0", ClampMax="2200.0"))
    float StandardPursuitClosePressureRange = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Combat Readability", meta=(ClampMin="1.0", ClampMax="3.0"))
    float AttackTelegraphRangeMultiplier = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Combat Readability", meta=(ClampMin="0.0", ClampMax="1.25"))
    float AttackTelegraphLeadSeconds = 0.38f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Combat Readability", meta=(ClampMin="0.0", ClampMax="600.0"))
    float HitReactionImpulseStrength = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Animation Readability")
    bool bEnableMotionReadability = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Animation Readability", meta=(ClampMin="0.0", ClampMax="2.0"))
    float MotionReadabilitySwayScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Animation Readability", meta=(ClampMin="0.05", ClampMax="1.5"))
    float HitReactionPoseDuration = 0.34f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Animation Readability", meta=(ClampMin="0.05", ClampMax="1.2"))
    float AttackLungePoseDuration = 0.28f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics")
    bool bEnableDeathRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics")
    bool bEnablePrimitiveCorpsePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics", meta=(ClampMin="0.0", ClampMax="150000.0"))
    float RagdollImpulseStrength = 430.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics", meta=(ClampMin="0.0", ClampMax="150000.0"))
    float PrimitiveCorpseImpulseStrength = 310.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics", meta=(ClampMin="0.5", ClampMax="20.0"))
    float RagdollCorpseLifetime = 9.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Death Physics", meta=(ClampMin="0.5", ClampMax="8.0"))
    float CorpseFadeDuration = 2.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Physical Animation")
    bool bEnablePhysicalHitReaction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Physical Animation")
    FName PhysicalHitReactionRootBone = FName(TEXT("spine_01"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Physical Animation", meta=(ClampMin="0.0", ClampMax="1.0"))
    float PhysicalHitReactionBlendWeight = 0.62f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Physical Animation", meta=(ClampMin="0.05", ClampMax="1.0"))
    float PhysicalHitReactionDuration = 0.28f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Physical Animation", meta=(ClampMin="0.0", ClampMax="150000.0"))
    float PhysicalHitReactionImpulseStrength = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    USkeletalMesh* ProfessionalZombieMesh = nullptr;

    /** Optional: animation Blueprint class for the zombie skeletal mesh.
     *  Most FAB zombie packs ship with an AnimBP; assign it here so the
     *  character actually idles, walks, attacks instead of standing in T-pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    TSubclassOf<UAnimInstance> ProfessionalZombieAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|VFX")
    UNiagaraSystem* HitVFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|VFX")
    UNiagaraSystem* DeathVFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|VFX")
    UNiagaraSystem* InfectionAuraVFX = nullptr;

    /** Which marketplace pack this zombie was spawned as. Persisted via the
     *  save system (paired with ZombieId) so reloads reproduce the same lineup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
    EZombieVariant Variant = EZombieVariant::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director")
    ECodeRescueZombieEncounterRole EncounterRole = ECodeRescueZombieEncounterRole::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director")
    bool bHasEncounterDirective = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director")
    FVector EncounterAnchorLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director", meta=(ClampMin="0.0", ClampMax="5000.0"))
    float EncounterLeashRadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director", meta=(ClampMin="0.0", ClampMax="1800.0"))
    float EncounterFlankOffset = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie|Encounter Director", meta=(ClampMin="0.25", ClampMax="3.0"))
    float EncounterDirectivePressureScale = 1.0f;

    /** Optional helper/marker actor spawned by GameMode. Destroyed as soon
     *  as this zombie is neutralized so the objective space stops advertising
     *  a threat that is already gone. */
    UPROPERTY()
    AActor* VisualMarkerActor = nullptr;

    /** Apply mesh + AnimBP + stat multipliers from a data table row. Called by
     *  ACodeRescueGameMode::SpawnWorld immediately after spawn. Safe to call
     *  before BeginPlay; effective values are picked up by the existing
     *  BeginPlay path that switches between primitive and skeletal mesh. */
    UFUNCTION(BlueprintCallable)
    void InitializeFromVariant(EZombieVariant InVariant, const FZombieVariantRow& Row);

    UFUNCTION(BlueprintCallable, Category="Zombie|Movement")
    void RefreshMovementSettings();

    /** Re-aligns visible feet and recaches the additive readability pose so a
     * deferred grounding correction survives subsequent animation ticks. */
    bool RefreshGroundedVisualPose();

    UFUNCTION(BlueprintCallable, Category="Zombie|Encounter Director")
    void ConfigureEncounterDirective(ECodeRescueZombieEncounterRole InRole, const FVector& AnchorLocation, float LeashRadius, float FlankOffset, float PressureScale);

    UFUNCTION(BlueprintCallable, Category="Zombie|Encounter Director")
    FVector ResolveEncounterMoveTarget(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category="Zombie|Encounter Director")
    bool HasEncounterDirective() const { return bHasEncounterDirective; }

    UFUNCTION(BlueprintCallable, Category="Zombie|Standard Pursuit")
    void ApplyStandardDirectPursuitProfile();

    UFUNCTION(BlueprintCallable, Category="Zombie|Standard Pursuit")
    FString GetStandardPursuitStateSummary(float DistanceToPlayerUU) const;

    UFUNCTION(BlueprintCallable)
    void ApplyRescueDamage(float DamageAmount, EHitZone HitZone = EHitZone::Other);

    /** Point-impact combat path. Stores the actual shot direction/location,
     * creates an anatomical wound, then applies the normal zone multiplier. */
    void ApplyRescuePointDamage(
        float DamageAmount,
        EHitZone HitZone,
        const FVector& ImpactPoint,
        const FVector& ShotDirection,
        FName ImpactBone = NAME_None);

    UFUNCTION(BlueprintCallable, Category="Zombie|Audio")
    void ApplyMonoAudioAccessibility(bool bMonoAudioEnabled);

private:
    UPROPERTY()
    UStaticMeshComponent* Body;

    UPROPERTY()
    UStaticMeshComponent* Head;

    UPROPERTY()
    UPointLightComponent* Glow;

    // Note: Skeletal mesh is inherited from ACharacter::GetMesh()

    /** Persistent audio component for ambient growls so we don't spawn a
     *  new one each interval. */
    UPROPERTY()
    UAudioComponent* GrowlAudio = nullptr;

    UPROPERTY()
    UPhysicalAnimationComponent* PhysicalHitReactionComponent = nullptr;

    UPROPERTY()
    UPoseableMeshComponent* FrozenCorpsePose = nullptr;

    /** Cached resolved montages/cues from the variant row. nullptr is fine
     *  and means "skip the corresponding play()". Resolved in
     *  InitializeFromVariant via TSoftObjectPtr::LoadSynchronous(). */
    UPROPERTY()
    UAnimMontage* HitReactMontage = nullptr;

    UPROPERTY()
    UAnimMontage* DeathMontage = nullptr;

    UPROPERTY()
    UAnimMontage* AttackMontage = nullptr;

    UPROPERTY()
    USoundBase* GrowlCue = nullptr;

    UPROPERTY()
    USoundBase* AttackCue = nullptr;

    UPROPERTY()
    USoundBase* DeathCue = nullptr;

    /** Set in ApplyRescueDamage when Health hits 0. Tick early-outs and AI
     *  stops issuing attacks; actual Destroy() is deferred via timer so the
     *  death montage can play out. */
    bool bIsDying = false;
    bool bCorpseFadeActive = false;
    bool bCountedActiveRagdoll = false;
    float CorpseFadeElapsed = 0.0f;
    FVector CorpseFadeActorStart = FVector::ZeroVector;
    FVector CorpseFadeActorScale = FVector::OneVector;
    FVector LastImpactPoint = FVector::ZeroVector;
    FVector LastIncomingShotDirection = FVector::ZeroVector;
    FName LastImpactBone = NAME_None;
    int32 SpawnedWoundCount = 0;
    bool bApplyingPointDamage = false;

    FTimerHandle DeathDestroyTimer;
    FTimerHandle GrowlTimer;

    float TimeSinceAttack = 99.0f;
    float LastThreatCaptionWorldTime = -999.0f;
    float LastStandardPursuitCaptionWorldTime = -999.0f;
    float MotionReadabilityPhase = 0.0f;
    float HitReactionPoseTimer = 0.0f;
    float AttackLungePoseTimer = 0.0f;
    float PhysicalHitReactionTimer = 0.0f;
    bool bPhysicalHitReactionActive = false;
    FName LastStandardPursuitStateTag;
    bool bMotionReadabilityBasePoseCached = false;
    FVector SkeletalMotionBaseLocation = FVector::ZeroVector;
    FRotator SkeletalMotionBaseRotation = FRotator::ZeroRotator;
    FVector SkeletalMotionBaseScale = FVector::OneVector;
    FVector PrimitiveBodyMotionBaseLocation = FVector::ZeroVector;
    FRotator PrimitiveBodyMotionBaseRotation = FRotator::ZeroRotator;
    FVector PrimitiveBodyMotionBaseScale = FVector::OneVector;
    FVector PrimitiveHeadMotionBaseLocation = FVector::ZeroVector;
    FRotator PrimitiveHeadMotionBaseRotation = FRotator::ZeroRotator;
    FVector PrimitiveHeadMotionBaseScale = FVector::OneVector;
    FVector GlowMotionBaseLocation = FVector::ZeroVector;

    /** Helper: schedule the next ambient growl at a random interval. Called
     *  on BeginPlay and recursively from itself via the timer. */
    void ScheduleNextGrowl();
    void PushThreatCaption(const FString& EventLabel, float RadiusUU = 3200.0f, float CooldownSeconds = 3.5f);
    float GetStandardPursuitAttackCooldown() const;
    void UpdateStandardPursuitReadability(float DeltaSeconds, APawn* PlayerPawn, float DistanceToPlayer, bool bTelegraphingAttack);
    void CacheMotionReadabilityBasePose(bool bForce = false);
    void ResetMotionReadabilityPose();
    void TriggerAttackMotionCue();
    void TriggerHitReactionMotionCue(EHitZone HitZone, float FinalDamage);
    void UpdateMotionReadability(float DeltaSeconds, const APawn* PlayerPawn, float DistanceToPlayer, bool bTelegraphingAttack, bool bProtectedLearningHold);

    void ApplyProfessionalVisuals();
    void FaceMovementTarget(const FVector& TargetLocation, float DeltaSeconds);
    void BindPhysicalHitReactionComponent();

    /** 2026-07-04: true when this zombie wears an authored v2 mesh (ShamblerV2/BruteV2).
     *  Their auto-generated physics assets are not physical-animation safe, so the
     *  hit-reaction physical animation stays OFF for them (crash fix: OOB body index
     *  in UPhysicalAnimationComponent::UpdatePhysicsEngineImp on 0-body instances). */
    bool bUsingV2ZombieBody = false;

    /** 2026-07-11 art+physics v3: CharactersV3 bodies ship a deliberate
     *  import-time physics asset. When its bodies match the mesh's bones,
     *  ragdoll death + physical hit reactions are re-enabled for authored
     *  zombies (the 07-04/07-11 detach failsafes stay in force). Kill switch:
     *  cr.AuthoredBodyPhysics=0 restores the legacy cleared-asset behavior. */
    bool bAuthoredBodyPhysicsReady = false;

    /** Authored V3 one-shot clips + the locomotion loop resumed afterwards.
     *  Null on pack meshes / v2 fallback (one-shot helper then no-ops). */
    UPROPERTY() UAnimSequence* AuthoredLoopAnim = nullptr;
    UPROPERTY() UAnimSequence* AuthoredAttackAnim = nullptr;
    UPROPERTY() UAnimSequence* AuthoredFlinchAnim = nullptr;
    UPROPERTY() UAnimSequence* AuthoredDeathAnim = nullptr;
    FTimerHandle AuthoredOneShotTimer;

    /** Play a one-shot clip on the single-node authored body; optionally
     *  schedule the locomotion loop to resume when the clip ends. */
    void PlayAuthoredOneShot(UAnimSequence* Anim, bool bResumeLoop);

    FName ResolvePhysicalHitReactionRootBone() const;
    FName ResolvePhysicalHitReactionImpactBone(EHitZone HitZone) const;
    bool TriggerPhysicalAnimationHitReaction(EHitZone HitZone, float FinalDamage);
    void UpdatePhysicalAnimationHitReaction(float DeltaSeconds);
    void ResetPhysicalAnimationHitReaction();
    void ApplyHitReadabilityImpulse(EHitZone HitZone, float FinalDamage);
    void SpawnLocalizedWound(EHitZone HitZone, const FVector& ImpactPoint, const FVector& ShotDirection, FName ImpactBone);
    void BeginCorpseFade();
    void UpdateCorpseFade(float DeltaSeconds);
    void ReleaseRagdollBudget();
    void DisableGameplayCollisionForDeath();
    bool TryActivateDeathRagdoll(EHitZone HitZone, float FinalDamage);
    bool ActivatePrimitiveDeathPhysics(EHitZone HitZone, float FinalDamage);
    FVector ComputeDeathPhysicsImpulse(EHitZone HitZone, float FinalDamage, float Strength) const;
};
