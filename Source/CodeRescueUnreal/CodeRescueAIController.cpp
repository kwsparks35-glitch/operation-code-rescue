#include "CodeRescueAIController.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeZombieActor.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameMode.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

ACodeRescueAIController::ACodeRescueAIController()
{
    PrimaryActorTick.TickInterval = 0.016f; // ~60 Hz
}

void ACodeRescueAIController::BeginPlay()
{
    Super::BeginPlay();

    ZombieCharacter = Cast<ACodeZombieActor>(GetPawn());

    // Find player in world
    for (TActorIterator<ACodeRescueCharacter> It(GetWorld()); It; ++It)
    {
        PlayerCharacter = *It;
        break;
    }
}

void ACodeRescueAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ZombieCharacter = Cast<ACodeZombieActor>(InPawn);
}

void ACodeRescueAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ZombieCharacter || ZombieCharacter->Health <= 0.0f)
    {
        return;
    }

    UpdateState(DeltaTime);
}

void ACodeRescueAIController::UpdateState(float DeltaTime)
{
    if (IsPlayerInProtectedLearningZone())
    {
        StopMovement();
        CurrentState = EZombieAIState::Patrol;
        return;
    }

    switch (CurrentState)
    {
        case EZombieAIState::Patrol:
            UpdatePatrol(DeltaTime);
            break;
        case EZombieAIState::Investigate:
            UpdateInvestigate(DeltaTime);
            break;
        case EZombieAIState::Chase:
            UpdateChase(DeltaTime);
            break;
        case EZombieAIState::Attack:
            UpdateAttack(DeltaTime);
            break;
        case EZombieAIState::Stagger:
            UpdateStagger(DeltaTime);
            break;
    }
}

void ACodeRescueAIController::UpdatePatrol(float DeltaTime)
{
    if (!PlayerCharacter)
    {
        return;
    }

    float DistanceToPlayer = FVector::Dist(ZombieCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation());
    const bool bSightDetected = CanDetectPlayerBySight(DistanceToPlayer);
    const bool bNoiseDetected = CanDetectPlayerByNoise(DistanceToPlayer);

    if (bSightDetected || bNoiseDetected)
    {
        LastKnownPlayerLocation = PlayerCharacter->GetActorLocation();
        bHasLastKnownPlayerLocation = true;
        LostSightGraceSeconds = bNoiseDetected && !bSightDetected ? 2.2f : 3.5f;
        ZombieCharacter->Tags.AddUnique(bNoiseDetected ? FName("StealthNoiseDetected") : FName("StealthSightlineDetected"));
        CurrentState = bSightDetected ? EZombieAIState::Chase : EZombieAIState::Investigate;
        return;
    }

    // Check visibility periodically
    LastVisibilityCheckTime += DeltaTime;
    if (LastVisibilityCheckTime >= 0.5f)
    {
        LastVisibilityCheckTime = 0.0f;
        if (CanDetectPlayerBySight(DistanceToPlayer))
        {
            LastKnownPlayerLocation = PlayerCharacter->GetActorLocation();
            bHasLastKnownPlayerLocation = true;
            LostSightGraceSeconds = 3.5f;
            ZombieCharacter->Tags.AddUnique(FName("StealthSightlineDetected"));
            CurrentState = EZombieAIState::Investigate;
            return;
        }
    }

    // Update patrol waypoint periodically
    PatrolUpdateTimer += DeltaTime;
    if (ZombieCharacter->HasEncounterDirective())
    {
        if (PatrolUpdateTimer >= 1.25f)
        {
            PatrolUpdateTimer = 0.0f;
            TryMoveToLocationWithFallback(ZombieCharacter->ResolveEncounterMoveTarget(PlayerCharacter->GetActorLocation()), 130.0f);
        }
        return;
    }

    if (PatrolUpdateTimer >= PATROL_UPDATE_INTERVAL)
    {
        PatrolUpdateTimer = 0.0f;
        FVector WayPoint = GetRandomPatrolWaypoint();
        TryMoveToLocationWithFallback(WayPoint, 120.0f);
    }
}

void ACodeRescueAIController::UpdateInvestigate(float DeltaTime)
{
    if (!PlayerCharacter)
    {
        CurrentState = EZombieAIState::Patrol;
        return;
    }

    float DistanceToPlayer = FVector::Dist(ZombieCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation());
    const bool bSightDetected = CanDetectPlayerBySight(DistanceToPlayer);
    const bool bNoiseDetected = CanDetectPlayerByNoise(DistanceToPlayer);

    if (bSightDetected)
    {
        LastKnownPlayerLocation = PlayerCharacter->GetActorLocation();
        bHasLastKnownPlayerLocation = true;
        LostSightGraceSeconds = 3.5f;
        CurrentState = EZombieAIState::Chase;
        return;
    }

    if (bNoiseDetected)
    {
        LastKnownPlayerLocation = PlayerCharacter->GetActorLocation();
        bHasLastKnownPlayerLocation = true;
        LostSightGraceSeconds = 2.2f;
        ZombieCharacter->Tags.AddUnique(FName("StealthInvestigateNoise"));
        TryMoveToLocationWithFallback(LastKnownPlayerLocation, 180.0f);
        return;
    }

    LostSightGraceSeconds = FMath::Max(0.0f, LostSightGraceSeconds - DeltaTime);
    if (bHasLastKnownPlayerLocation && LostSightGraceSeconds > 0.0f)
    {
        ZombieCharacter->Tags.AddUnique(FName("StealthInvestigateLastKnownLocation"));
        TryMoveToLocationWithFallback(LastKnownPlayerLocation, 180.0f);
        return;
    }

    CurrentState = EZombieAIState::Patrol;
}

void ACodeRescueAIController::UpdateChase(float DeltaTime)
{
    if (!PlayerCharacter)
    {
        CurrentState = EZombieAIState::Patrol;
        return;
    }

    float DistanceToPlayer = FVector::Dist(ZombieCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation());
    const bool bSightDetected = CanDetectPlayerBySight(DistanceToPlayer);
    const bool bNoiseDetected = CanDetectPlayerByNoise(DistanceToPlayer);

    // If close enough to attack, transition to attack
    if (DistanceToPlayer < ZombieCharacter->AttackRange)
    {
        CurrentState = EZombieAIState::Attack;
        return;
    }

    if (bSightDetected || bNoiseDetected)
    {
        LastKnownPlayerLocation = PlayerCharacter->GetActorLocation();
        bHasLastKnownPlayerLocation = true;
        LostSightGraceSeconds = bSightDetected ? 3.5f : 2.2f;
    }
    else
    {
        LostSightGraceSeconds = FMath::Max(0.0f, LostSightGraceSeconds - DeltaTime);
        if (LostSightGraceSeconds <= 0.0f)
        {
            ZombieCharacter->Tags.AddUnique(FName("StealthAvoidanceLostContact"));
            CurrentState = EZombieAIState::Investigate;
            return;
        }
    }

    if (ZombieCharacter->bStandardDirectPursuitEnabled)
    {
        ZombieCharacter->Tags.AddUnique(FName("StandardPursuitDirectChase"));
        ZombieCharacter->Tags.AddUnique(FName("StandardPursuitMoveDirectlyTowardPlayer"));
    }
    const FVector TargetLocation = (bSightDetected || bNoiseDetected)
        ? PlayerCharacter->GetActorLocation()
        : LastKnownPlayerLocation;
    MoveDirectlyToward(ZombieCharacter->ResolveEncounterMoveTarget(TargetLocation), ZombieCharacter->AttackRange * 0.8f);
}

void ACodeRescueAIController::UpdateAttack(float DeltaTime)
{
    if (!PlayerCharacter)
    {
        CurrentState = EZombieAIState::Patrol;
        return;
    }

    float DistanceToPlayer = FVector::Dist(ZombieCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation());

    // If player is out of range, go back to chase
    if (DistanceToPlayer > ZombieCharacter->AttackRange * 1.5f)
    {
        CurrentState = EZombieAIState::Chase;
        return;
    }

    if (ZombieCharacter->bStandardDirectPursuitEnabled)
    {
        ZombieCharacter->Tags.AddUnique(FName("StandardPursuitAttackHold"));
    }
    MoveDirectlyToward(ZombieCharacter->ResolveEncounterMoveTarget(PlayerCharacter->GetActorLocation()), ZombieCharacter->AttackRange * 0.7f);
    // Delegate to zombie's attack montage logic (already exists in Tick)
    // This state just prevents movement while attacking
}

void ACodeRescueAIController::UpdateStagger(float DeltaTime)
{
    StaggerRecoveryTime -= DeltaTime;

    if (StaggerRecoveryTime <= 0.0f)
    {
        // Return to previous behavior
        CurrentState = EZombieAIState::Chase;
    }
}

bool ACodeRescueAIController::IsPlayerInProtectedLearningZone() const
{
    return PlayerCharacter &&
        ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(PlayerCharacter, PlayerCharacter->GetActorLocation(), 300.0f);
}

bool ACodeRescueAIController::IsPlayerVisible() const
{
    if (!PlayerCharacter || !ZombieCharacter)
    {
        return false;
    }
    if (IsPlayerInProtectedLearningZone())
    {
        return false;
    }

    FVector StartLocation = ZombieCharacter->GetActorLocation();
    FVector EndLocation = PlayerCharacter->GetActorLocation() + FVector(0, 0, PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ZombieCharacter);
    QueryParams.AddIgnoredActor(PlayerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, CodeRescueCollision::AISightTrace, QueryParams);

    // Visible if no hit, or if hit actor is the player
    return !bHit || HitResult.GetActor() == PlayerCharacter;
}

float ACodeRescueAIController::GetSightDetectionRange() const
{
    if (!ZombieCharacter)
    {
        return 0.0f;
    }

    float SightRange = ZombieCharacter->ActivationRange * 0.62f;
    if (PlayerCharacter && PlayerCharacter->bFieldFlashlightActive)
    {
        SightRange = FMath::Max(SightRange, ZombieCharacter->ActivationRange * 0.88f);
    }
    float WeatherVisibilityScale = 1.0f;
    static const IConsoleVariable* WeatherVisibility =
        IConsoleManager::Get().FindConsoleVariable(TEXT("cr.WeatherVisibilityScale"));
    if (WeatherVisibility)
    {
        WeatherVisibilityScale = FMath::Clamp(WeatherVisibility->GetFloat(), 0.50f, 1.0f);
    }
    SightRange *= WeatherVisibilityScale;
    const float MinimumCombatSight = ZombieCharacter->AttackRange * 3.0f;
    const float WeatherLimitedMaximum = FMath::Max(
        MinimumCombatSight, ZombieCharacter->ActivationRange * WeatherVisibilityScale);
    return FMath::Clamp(SightRange, MinimumCombatSight, WeatherLimitedMaximum);
}

bool ACodeRescueAIController::CanDetectPlayerBySight(float DistanceToPlayer) const
{
    if (!PlayerCharacter || !ZombieCharacter || DistanceToPlayer > GetSightDetectionRange())
    {
        return false;
    }
    return IsPlayerVisible();
}

bool ACodeRescueAIController::CanDetectPlayerByNoise(float DistanceToPlayer) const
{
    if (!PlayerCharacter || !ZombieCharacter)
    {
        return false;
    }
    if (IsPlayerInProtectedLearningZone())
    {
        return false;
    }

    const float NoiseRadius = PlayerCharacter->GetStealthNoiseRadius();
    if (NoiseRadius <= 10.0f || DistanceToPlayer > NoiseRadius)
    {
        return false;
    }

    ZombieCharacter->Tags.AddUnique(FName("StealthAvoidanceNoiseListener"));
    return true;
}

FVector ACodeRescueAIController::GetRandomPatrolWaypoint() const
{
    if (!ZombieCharacter)
    {
        return FVector::ZeroVector;
    }

    FVector StartLocation = ZombieCharacter->GetActorLocation();
    FVector RandomDirection = FMath::VRand() * PATROL_RADIUS;
    FVector WayPoint = StartLocation + RandomDirection;

    // Snap to navmesh
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSystem && HasUsableNavigationData())
    {
        FNavLocation OutLocation;
        if (NavSystem->GetRandomPointInNavigableRadius(WayPoint, PATROL_RADIUS, OutLocation))
        {
            WayPoint = OutLocation.Location;
        }
    }

    return WayPoint;
}

bool ACodeRescueAIController::HasUsableNavigationData() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    return NavSystem && NavSystem->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfMissing::DontCreate) != nullptr;
}

void ACodeRescueAIController::MoveDirectlyToward(const FVector& TargetLocation, float AcceptanceRadius)
{
    if (!ZombieCharacter)
    {
        return;
    }

    FVector Delta = TargetLocation - ZombieCharacter->GetActorLocation();
    Delta.Z = 0.0f;

    if (Delta.SizeSquared() <= FMath::Square(AcceptanceRadius))
    {
        StopMovement();
        return;
    }

    const FVector Direction = Delta.GetSafeNormal();
    ZombieCharacter->SetActorRotation(Direction.Rotation());
    ZombieCharacter->AddMovementInput(Direction, 1.0f);
}

bool ACodeRescueAIController::TryMoveToLocationWithFallback(const FVector& TargetLocation, float AcceptanceRadius)
{
    if (!HasUsableNavigationData())
    {
        MoveDirectlyToward(TargetLocation, AcceptanceRadius);
        return false;
    }

    const EPathFollowingRequestResult::Type Result = MoveToLocation(TargetLocation, AcceptanceRadius);
    if (Result == EPathFollowingRequestResult::Failed)
    {
        MoveDirectlyToward(TargetLocation, AcceptanceRadius);
        return false;
    }

    return true;
}

bool ACodeRescueAIController::TryMoveToActorWithFallback(AActor* TargetActor, float AcceptanceRadius)
{
    if (!TargetActor)
    {
        return false;
    }

    if (!HasUsableNavigationData())
    {
        MoveDirectlyToward(TargetActor->GetActorLocation(), AcceptanceRadius);
        return false;
    }

    const EPathFollowingRequestResult::Type Result = MoveToActor(TargetActor, AcceptanceRadius);
    if (Result == EPathFollowingRequestResult::Failed)
    {
        MoveDirectlyToward(TargetActor->GetActorLocation(), AcceptanceRadius);
        return false;
    }

    return true;
}

void ACodeRescueAIController::EnterStagger()
{
    CurrentState = EZombieAIState::Stagger;
    StaggerRecoveryTime = STAGGER_DURATION;
}
