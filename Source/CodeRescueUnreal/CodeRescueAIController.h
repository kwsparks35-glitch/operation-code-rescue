#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CodeRescueAIController.generated.h"

class ACodeZombieActor;
class ACodeRescueCharacter;

/** Zombie AI state machine.
 *
 *  Drives ACodeZombieActor via state transitions:
 *    Patrol → Investigate (on player detection) → Chase → Attack → Stagger (on damage)
 *
 *  Uses navmesh-aware movement when runtime navigation exists, then falls
 *  back to direct character movement so generated levels stay playable while
 *  Recast data is still building.
 *  Line trace visibility checks every 0.5 seconds for perception updates.
 */
UENUM(BlueprintType)
enum class EZombieAIState : uint8
{
    Patrol = 0,
    Investigate = 1,
    Chase = 2,
    Attack = 3,
    Stagger = 4
};

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACodeRescueAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Enter stagger state from external damage handler (called by ACodeZombieActor::ApplyRescueDamage). */
    UFUNCTION(BlueprintCallable, Category="Zombie AI")
    void EnterStagger();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    /** Current AI state. */
    EZombieAIState CurrentState = EZombieAIState::Patrol;

    /** Possessed zombie reference. */
    UPROPERTY()
    ACodeZombieActor* ZombieCharacter = nullptr;

    /** Player reference for chase/attack logic. */
    UPROPERTY()
    ACodeRescueCharacter* PlayerCharacter = nullptr;

    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    bool bHasLastKnownPlayerLocation = false;
    float LostSightGraceSeconds = 0.0f;

    /** Last time we checked visibility (throttle to 0.5s). */
    float LastVisibilityCheckTime = 0.0f;

    /** Stagger recovery timer. */
    float StaggerRecoveryTime = 0.0f;
    static constexpr float STAGGER_DURATION = 0.4f;

    /** Patrol waypoint update frequency (seconds). */
    static constexpr float PATROL_UPDATE_INTERVAL = 4.0f;
    float PatrolUpdateTimer = 0.0f;

    /** Patrol radius (UU). */
    static constexpr float PATROL_RADIUS = 800.0f;

    // State machine methods
    void UpdateState(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateInvestigate(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateStagger(float DeltaTime);

    /** True while the player is standing in a tagged protected terminal/safehouse zone. */
    bool IsPlayerInProtectedLearningZone() const;

    /** Line trace visibility check. Returns true if player is visible from zombie. */
    bool IsPlayerVisible() const;

    bool CanDetectPlayerBySight(float DistanceToPlayer) const;
    bool CanDetectPlayerByNoise(float DistanceToPlayer) const;
    float GetSightDetectionRange() const;

    /** Pick a random waypoint within PatrolRadius of start location. */
    FVector GetRandomPatrolWaypoint() const;

    /** True when the runtime-generated world has usable navigation data. */
    bool HasUsableNavigationData() const;

    /** Keep zombies moving even before dynamic navmesh data is ready. */
    void MoveDirectlyToward(const FVector& TargetLocation, float AcceptanceRadius);

    /** Try navmesh movement first, then degrade to direct movement. */
    bool TryMoveToLocationWithFallback(const FVector& TargetLocation, float AcceptanceRadius);
    bool TryMoveToActorWithFallback(AActor* TargetActor, float AcceptanceRadius);
};
