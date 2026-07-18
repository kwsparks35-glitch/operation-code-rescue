#include "CodeRescueWindSway.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

static TAutoConsoleVariable<float> CVarWindStrength(
    TEXT("cr.WindStrength"), 1.0f,
    TEXT("Global foliage wind-sway multiplier (0 disables the ambient wind)."));

ACodeRescueWindSwayManager::ACodeRescueWindSwayManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f;   // 30 Hz is plenty for sway
}

void ACodeRescueWindSwayManager::BeginPlay()
{
    Super::BeginPlay();
    RefreshRegistry();
    GetWorldTimerManager().SetTimer(RefreshTimer, this,
        &ACodeRescueWindSwayManager::RefreshRegistry, 3.0f, true);
}

void ACodeRescueWindSwayManager::RefreshRegistry()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    const APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    const FVector Center = Player ? Player->GetActorLocation() : FVector::ZeroVector;
    constexpr float RadiusSq = 12000.0f * 12000.0f;
    constexpr int32 MaxTargets = 96;

    // keep existing valid entries that are still in range
    TArray<FSwayTarget> Kept;
    TSet<const AActor*> Known;
    for (const FSwayTarget& T : Targets)
    {
        if (const AActor* A = T.Actor.Get())
        {
            if (FVector::DistSquared(A->GetActorLocation(), Center) <= RadiusSq)
            {
                Kept.Add(T);
                Known.Add(A);
            }
        }
    }

    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (Kept.Num() >= MaxTargets)
        {
            break;
        }
        AStaticMeshActor* Actor = *It;
        if (Known.Contains(Actor) ||
            FVector::DistSquared(Actor->GetActorLocation(), Center) > RadiusSq)
        {
            continue;
        }
        const UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
        const UStaticMesh* Asset = Mesh ? Mesh->GetStaticMesh() : nullptr;
        bool bFoliage = Actor->Tags.Contains(FName("WindFoliage"));
        bool bBush = false;
        if (!bFoliage && Asset)
        {
            const FString Path = Asset->GetPathName();
            if (Path.Contains(TEXT("/Nature/")) || Path.Contains(TEXT("SM_Tree")) ||
                Path.Contains(TEXT("SM_Bush")))
            {
                bFoliage = true;
                bBush = Path.Contains(TEXT("Bush"));
            }
        }
        if (!bFoliage)
        {
            continue;
        }
        if (Actor->GetStaticMeshComponent() &&
            Actor->GetStaticMeshComponent()->Mobility != EComponentMobility::Movable)
        {
            Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        }
        FSwayTarget T;
        T.Actor = Actor;
        T.BaseRotation = Actor->GetActorRotation();
        T.Phase = static_cast<float>(Actor->GetUniqueID() % 628) * 0.01f;
        T.Amplitude = bBush ? 2.6f : 1.5f;
        Kept.Add(T);
        Actor->Tags.AddUnique(FName("WindFoliageActive"));
    }
    Targets = MoveTemp(Kept);
}

void ACodeRescueWindSwayManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float Strength = FMath::Clamp(CVarWindStrength.GetValueOnGameThread(), 0.0f, 4.0f);
    if (Strength <= KINDA_SMALL_NUMBER || Targets.Num() == 0 || !GetWorld())
    {
        return;
    }
    const float T = GetWorld()->GetTimeSeconds();
    // gust envelope: slow beats layered so the wind "breathes"
    const float Gust = FMath::Clamp(
        0.55f + 0.45f * FMath::Sin(T * 0.29f) + 0.28f * FMath::Sin(T * 0.117f + 1.7f),
        0.05f, 1.4f);

    for (const FSwayTarget& Target : Targets)
    {
        AActor* Actor = Target.Actor.Get();
        if (!Actor)
        {
            continue;
        }
        const float Amp = Target.Amplitude * Gust * Strength;
        const float Pitch = Amp * FMath::Sin(T * 1.55f + Target.Phase);
        const float Roll = 0.6f * Amp * FMath::Sin(T * 1.13f + Target.Phase * 1.7f);
        FRotator NewRot = Target.BaseRotation;
        NewRot.Pitch += Pitch;
        NewRot.Roll += Roll;
        Actor->SetActorRotation(NewRot);
    }
}
