#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FriendlyNPCActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class USceneComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UAnimInstance;

/**
 * #67 / #68 — Ambient friendly NPCs that aren't part of the rescue chain.
 *
 * Each city gets a small handful of these so it feels populated rather
 * than empty between objectives. Each role has a single Interact effect
 * the player can use once per day-cycle:
 *
 *   Engineer   → grants +1 Scrap once per visit (ambient resource trickle)
 *   Medic      → restores +25 Health if you're below max (no medkit cost)
 *   Scientist  → grants +1 ResearchPoint (skill-tree currency)
 *   Trader     → exchange 5 Scrap for 1 ResearchPoint (one-time per visit)
 *
 * Visually they're built from procedural blocks plus a colored point light
 * tinted by role. When art-pipeline lands, swap the procedural body for
 * the same MetaHuman path used by ASurvivorActor by setting
 * ProfessionalNPCMesh on a Blueprint subclass.
 *
 * The interact-cooldown resets when the day/night cycle flips bIsNight,
 * so the player can naturally re-visit them across day cycles.
 */
UENUM(BlueprintType)
enum class EFriendlyNPCRole : uint8
{
    Engineer    = 0   UMETA(DisplayName = "Engineer (free scrap)"),
    Medic       = 1   UMETA(DisplayName = "Medic (free heal)"),
    Scientist   = 2   UMETA(DisplayName = "Scientist (free research)"),
    Trader      = 3   UMETA(DisplayName = "Trader (5 scrap -> 1 research)")
};

UCLASS()
class CODERESCUEUNREAL_API AFriendlyNPCActor : public AActor
{
    GENERATED_BODY()

public:
    AFriendlyNPCActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
    EFriendlyNPCRole NPCRole = EFriendlyNPCRole::Engineer;

    /** Display name shown in floating label and in subtitles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
    FString NPCName = TEXT("Civilian");

    /** Single line of dialogue spoken (subtitled) on Interact. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
    FString GreetingLine = TEXT("Stay sharp out there.");

    /** Index of the city this NPC belongs to. Used for the cooldown reset
     *  and for save persistence per city. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
    int32 CityIndex = 0;

    /** Optional skeletal-mesh override. When set, the procedural cube body
     *  is hidden and the skeletal mesh is shown instead. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    USkeletalMesh* ProfessionalNPCMesh = nullptr;

    /** Optional animation Blueprint for the professional NPC mesh. If unset,
     *  BeginPlay picks the matching Manny/Quinn locomotion AnimBP from the
     *  locally staged mannequin assets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Professional Assets")
    TSubclassOf<UAnimInstance> ProfessionalNPCAnimClass;

    /** Enables the lightweight visual-only gesture layer used until final
     * authored support-NPC animation sets are imported. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation Readability")
    bool bEnableServiceGestureReadability = true;

    /** Scales ambient idle motion on the role badge, prop, head, and
     * professional skeletal mesh without moving the collision body. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation Readability", meta=(ClampMin="0.0", ClampMax="3.0"))
    float ServiceIdleGestureScale = 1.0f;

    /** Duration for the visible successful-service acknowledgment pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation Readability", meta=(ClampMin="0.1", ClampMax="3.0"))
    float ServiceGrantGestureDuration = 0.95f;

    /** Duration for cooldown or unmet-precondition refusal gestures. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation Readability", meta=(ClampMin="0.1", ClampMax="3.0"))
    float ServiceDeniedGestureDuration = 0.65f;

    /** Player-facing interact entry point. Returns true if the perk was
     *  granted; false if cooldown blocked or the role-specific precondition
     *  failed (e.g. Trader visit with insufficient scrap). */
    UFUNCTION(BlueprintCallable, Category="NPC")
    bool Interact(class APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NPC")
    FString GetServiceId() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NPC")
    FString GetRoleDisplayName() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NPC")
    FString GetServiceSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NPC")
    FString GetInteractionPrompt() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="NPC")
    bool IsServiceOnCooldown() const;

    UFUNCTION(BlueprintCallable, Category="NPC")
    void ApplySavedServiceState();

    /** Called by GameMode when day flips to night, so each NPC's perk
     *  becomes available again at sunrise. */
    UFUNCTION(BlueprintCallable, Category="NPC")
    void ResetDailyPerk();

private:
    void ApplyRoleVisualIdentity();
    void SetComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale = 0.0f);
    void CacheServiceGestureBasePose(bool bTagForAudit = false);
    void UpdateServiceGesture(float DeltaSeconds);
    void TriggerServiceGrantGesture();
    void TriggerServiceDeniedGesture();

    UPROPERTY() UStaticMeshComponent* Body = nullptr;
    UPROPERTY() UStaticMeshComponent* Head = nullptr;
    UPROPERTY() UStaticMeshComponent* RoleBadge = nullptr;
    UPROPERTY() UStaticMeshComponent* RoleProp = nullptr;
    UPROPERTY() UStaticMeshComponent* RoleIconA = nullptr;
    UPROPERTY() UStaticMeshComponent* RoleIconB = nullptr;
    UPROPERTY() USkeletalMeshComponent* SkeletalBody = nullptr;
    UPROPERTY() UPointLightComponent* RoleLight = nullptr;
    UPROPERTY() USceneComponent* Root = nullptr;

    bool bPerkUsedThisDay = false;
    bool bServiceGestureBasePoseCached = false;
    float ServiceGesturePhase = 0.0f;
    float ServiceGrantGestureTimer = 0.0f;
    float ServiceDeniedGestureTimer = 0.0f;
    FTransform SkeletalGestureBaseTransform;
    FTransform HeadGestureBaseTransform;
    FTransform RoleBadgeGestureBaseTransform;
    FTransform RolePropGestureBaseTransform;
    FTransform RoleIconAGestureBaseTransform;
    FTransform RoleIconBGestureBaseTransform;
    FTransform RoleLightGestureBaseTransform;
    float RoleLightGestureBaseIntensity = 3200.0f;
};
