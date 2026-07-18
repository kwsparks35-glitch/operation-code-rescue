#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivorActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPointLightComponent;
class UNiagaraSystem;
class UAnimInstance;
struct FCodeRescueCityMission;

UCLASS()
class CODERESCUEUNREAL_API ASurvivorActor : public AActor
{
    GENERATED_BODY()

public:
    ASurvivorActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor")
    FString SurvivorName = TEXT("Civilian Survivor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor")
    FString Story = TEXT("Trapped and waiting for evacuation.");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FString ArchetypeTitle = TEXT("Power-Grid Apprentice");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FString ArchetypeIconLabel = TEXT("POWER");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FString ArchetypeFieldNeed = TEXT("three backup-cell readings combined into one route total");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FString ArchetypeRescueSkill = TEXT("can rebalance battery cells and field generators");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FString ArchetypeDossierHook = TEXT("Writes every load reading in pencil before trusting the grid.");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Archetype")
    FLinearColor ArchetypeAccentColor = FLinearColor(1.0f, 0.86f, 0.18f, 1.0f);

    UPROPERTY(BlueprintReadOnly)
    bool bRescued = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor")
    int32 CityIndex = 0;

    /** If non-empty, the survivor refuses rescue until this Challenge.Id has
     *  been solved (and recorded in UCodeRescueGameInstance::SolvedTerminalIds).
     *  Empty = no gating. Set by GameMode at spawn time so the level designer
     *  can wire which terminal each survivor is "behind". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Gating")
    FString RequiredTerminalId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    USkeletalMesh* ProfessionalSurvivorMesh = nullptr;

    /** Optional: animation Blueprint class for the professional survivor mesh.
     *  Set this to e.g. ABP_MetaHuman or the AnimBP that shipped with your
     *  MetaHuman / character pack. When set together with
     *  ProfessionalSurvivorMesh, the procedural cube + sphere body is hidden
     *  and the skeletal mesh runs the assigned animation graph. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    TSubclassOf<UAnimInstance> ProfessionalSurvivorAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|VFX")
    UNiagaraSystem* RescueBeaconVFX = nullptr;

    /** #11 — voice-over played when the survivor is successfully rescued.
     *  Soft ref so the asset can be cooked separately or remain unset until
     *  imported. Falls back to silence if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|Audio")
    TSoftObjectPtr<class USoundBase> RescueVoCue;

    /** #11 — short distress bark played on a 20-30s random idle timer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets|Audio")
    TSoftObjectPtr<class USoundBase> IdleBarkCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Animation Readability")
    bool bEnableSurvivorGestureReadability = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Animation Readability", meta=(ClampMin="0.0", ClampMax="2.0"))
    float SurvivorIdleGestureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Animation Readability", meta=(ClampMin="0.15", ClampMax="3.0"))
    float RescueGestureDuration = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survivor|Animation Readability", meta=(ClampMin="0.15", ClampMax="2.0"))
    float LockedGestureDuration = 0.72f;

    UFUNCTION(BlueprintCallable)
    bool Rescue();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survivor|Archetype")
    FString GetSurvivorArchetypeSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survivor|Archetype")
    FString GetInteractionPrompt() const;

    void ConfigureArchetypeFromMission(const FCodeRescueCityMission& Mission);

    void AddHelperActor(AActor* HelperActor);

private:
    void ClearHelperActors();
    /** Internal: re-arm the idle-bark timer. */
    void ScheduleNextIdleBark();
    void PlayIdleBark();
    void CacheGestureBasePose(bool bForce = false);
    void UpdateSurvivorGesture(float DeltaSeconds);
    void TriggerLockedGesture();
    void TriggerRescueGesture();
    void ScheduleRescueFadeOut();

    FTimerHandle IdleBarkTimer;
    FTimerHandle RescueFadeOutTimer;

    UPROPERTY()
    UStaticMeshComponent* Body;

    /** Spherical "head" mesh attached above the body so the survivor reads
     *  as a figure rather than a single monolithic cube. */
    UPROPERTY()
    UStaticMeshComponent* Head;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalBody;

    UPROPERTY()
    UPointLightComponent* RescueLight;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> HelperActors;

    bool bGestureBasePoseCached = false;
    float GesturePhase = 0.0f;
    float LockedGestureTimer = 0.0f;
    float RescueGestureTimer = 0.0f;

    /** 2026-07-11 v3: authored celebration wave + the idle loop resumed after
     *  it finishes (only set when the survivor wears an authored V3 body). */
    UPROPERTY()
    class UAnimSequence* V3WaveAnim = nullptr;
    UPROPERTY()
    class UAnimSequence* V3IdleAnim = nullptr;
    FTimerHandle WaveResumeTimer;
    FVector SkeletalGestureBaseLocation = FVector::ZeroVector;
    FRotator SkeletalGestureBaseRotation = FRotator::ZeroRotator;
    FVector SkeletalGestureBaseScale = FVector::OneVector;
    FVector HeadGestureBaseLocation = FVector::ZeroVector;
    FRotator HeadGestureBaseRotation = FRotator::ZeroRotator;
    FVector HeadGestureBaseScale = FVector::OneVector;
    FVector LightGestureBaseLocation = FVector::ZeroVector;
    float LightGestureBaseIntensity = 0.0f;
};
