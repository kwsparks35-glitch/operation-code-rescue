#include "BossZombieActor.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueRetargetRig.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABossZombieActor::ABossZombieActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Bigger, scarier defaults.
    SetActorScale3D(FVector(1.4f, 1.4f, 1.4f));
    Health = 600.0f;
    AttackDamage = 25.0f;
    MoveSpeed = 110.0f;
    AttackRange = 220.0f;
    ActivationRange = 8000.0f;

    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        GetMesh(), ECodeRescueRetargetRigProfile::BossWarden, this);

    PhaseTelegraphRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossPhaseTelegraphRing"));
    PhaseTelegraphRing->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphRing->SetRelativeLocation(FVector(0.0f, 0.0f, -78.0f));
    PhaseTelegraphRing->SetRelativeScale3D(FVector(4.8f, 4.8f, 0.035f));

    PhaseTelegraphCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossPhaseTelegraphCore"));
    PhaseTelegraphCore->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphCore->SetRelativeLocation(FVector(0.0f, 0.0f, 220.0f));
    PhaseTelegraphCore->SetRelativeScale3D(FVector(0.44f));

    PhaseTelegraphSweep = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossPhaseTelegraphSweep"));
    PhaseTelegraphSweep->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphSweep->SetRelativeLocation(FVector(0.0f, 0.0f, 24.0f));
    PhaseTelegraphSweep->SetRelativeScale3D(FVector(5.4f, 0.08f, 0.035f));

    PhaseTelegraphAddBeaconA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossPhaseTelegraphAddBeaconA"));
    PhaseTelegraphAddBeaconA->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphAddBeaconA->SetRelativeLocation(FVector(280.0f, 0.0f, 10.0f));
    PhaseTelegraphAddBeaconA->SetRelativeScale3D(FVector(0.22f));

    PhaseTelegraphAddBeaconB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossPhaseTelegraphAddBeaconB"));
    PhaseTelegraphAddBeaconB->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphAddBeaconB->SetRelativeLocation(FVector(-280.0f, 0.0f, 10.0f));
    PhaseTelegraphAddBeaconB->SetRelativeScale3D(FVector(0.22f));

    PhaseTelegraphLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BossPhaseTelegraphLight"));
    PhaseTelegraphLight->SetupAttachment(GetCapsuleComponent());
    PhaseTelegraphLight->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    PhaseTelegraphLight->SetLightColor(PhaseTelegraphColor);
    PhaseTelegraphLight->SetIntensity(0.0f);
    PhaseTelegraphLight->SetAttenuationRadius(1600.0f);
    PhaseTelegraphLight->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CylinderMesh.Succeeded() && PhaseTelegraphRing)
    {
        PhaseTelegraphRing->SetStaticMesh(CylinderMesh.Object);
    }
    if (CubeMesh.Succeeded() && PhaseTelegraphSweep)
    {
        PhaseTelegraphSweep->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        PhaseTelegraphCore->SetStaticMesh(SphereMesh.Object);
        PhaseTelegraphAddBeaconA->SetStaticMesh(SphereMesh.Object);
        PhaseTelegraphAddBeaconB->SetStaticMesh(SphereMesh.Object);
    }

    for (UStaticMeshComponent* Component : { PhaseTelegraphRing, PhaseTelegraphCore, PhaseTelegraphSweep, PhaseTelegraphAddBeaconA, PhaseTelegraphAddBeaconB })
    {
        ConfigurePhaseTelegraphComponent(Component);
    }

    Tags.AddUnique(FName("BossPhaseTelegraph"));
    Tags.AddUnique(FName("EnemyTelegraphReadability"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
}

void ABossZombieActor::BeginPlay()
{
    Super::BeginPlay();
    MaxBossHealth = FMath::Max(1.0f, Health);
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        bPhaseTelegraphReducedMotion = GI->bReducedMotion;
    }
    ApplyPhaseTelegraphVisibility(false);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}

int32 ABossZombieActor::CountLivingAdds()
{
    ActiveAdds.RemoveAll([](const TWeakObjectPtr<ACodeZombieActor>& Add)
    {
        return !Add.IsValid() || Add->Health <= 0.0f;
    });
    return ActiveAdds.Num();
}

void ABossZombieActor::EnterPhase(int32 Phase)
{
    if (CurrentPhase == Phase) return;
    CurrentPhase = Phase;
    StartPhaseTelegraph(Phase);

    switch (Phase)
    {
    case 2:
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = MoveSpeed * 1.5f;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Orange, TEXT("BOSS PHASE 2 - sprint + regen"));
        }
        break;
    case 3:
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = MoveSpeed * 1.2f;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("BOSS PHASE 3 - spawning adds"));
        }
        break;
    default: break;
    }
}

void ABossZombieActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Health <= 0.0f)
    {
        ApplyPhaseTelegraphVisibility(false);
        return;
    }

    const float Frac = Health / FMath::Max(1.0f, MaxBossHealth);
    if (Frac <= 0.33f) EnterPhase(3);
    else if (Frac <= 0.66f) EnterPhase(2);

    UpdatePhaseTelegraph(DeltaSeconds);

    if (CurrentPhase == 2)
    {
        Health = FMath::Min(MaxBossHealth, Health + 5.0f * DeltaSeconds);   // regen
    }
    else if (CurrentPhase == 3)
    {
        TimeSinceAddSpawn += DeltaSeconds;
        if (TimeSinceAddSpawn >= 3.0f && CountLivingAdds() < MaxActiveAdds)
        {
            TimeSinceAddSpawn = 0.0f;
            const int32 AddsToSpawn = FMath::Min(2, MaxActiveAdds - CountLivingAdds());
            for (int32 i = 0; i < AddsToSpawn; ++i)
            {
                const float Angle = FMath::FRandRange(0.0f, 360.0f);
                const FVector Off(FMath::Cos(FMath::DegreesToRadians(Angle)) * 200.0f,
                                  FMath::Sin(FMath::DegreesToRadians(Angle)) * 200.0f, 60.0f);
                ACodeZombieActor* Add = GetWorld()->SpawnActor<ACodeZombieActor>(ACodeZombieActor::StaticClass(),
                    GetActorLocation() + Off, FRotator::ZeroRotator);
                if (Add)
                {
                    Add->ZombieId = 300000 + FMath::Max(0, ZombieId) * 100 + SpawnedAddSerial++;
                    Add->Health = 25.0f;
                    Add->AttackDamage = 5.0f;
                    Add->MoveSpeed = MoveSpeed * 0.9f;
                    Add->ActivationRange = ActivationRange;
                    Add->Variant = EZombieVariant::Default;
                    Add->Tags.AddUnique(FName("ZombieFamilyVariantRuntime"));
                    Add->Tags.AddUnique(FName("CityZombieFamilyVariant"));
                    Add->Tags.AddUnique(FName("ZombieFamily_Default"));
                    Add->Tags.AddUnique(FName("BossPhaseAddFamily"));
                    Add->Tags.AddUnique(FName("Top50Recommendations"));
                    Add->SetActorScale3D(FVector(0.7f));
                    Add->RefreshMovementSettings();
                    ActiveAdds.Add(Add);
                }
            }
        }
    }
}

void ABossZombieActor::StartPhaseTelegraph(int32 Phase)
{
    if (Phase < 2)
    {
        return;
    }

    PhaseTelegraphActivePhase = Phase;
    PhaseTelegraphElapsed = 0.0f;
    PhaseTelegraphTimeRemaining = FMath::Max(0.5f, PhaseTelegraphDuration);
    PhaseTelegraphColor = (Phase >= 3)
        ? FLinearColor(1.0f, 0.04f, 0.14f)
        : FLinearColor(1.0f, 0.48f, 0.02f);

    const FLinearColor Accent = (Phase >= 3)
        ? FLinearColor(0.78f, 0.0f, 1.0f)
        : FLinearColor(1.0f, 0.86f, 0.08f);

    ApplyPhaseTelegraphTint(PhaseTelegraphRing, PhaseTelegraphColor, 1.55f);
    ApplyPhaseTelegraphTint(PhaseTelegraphCore, PhaseTelegraphColor * 0.72f + FLinearColor::White * 0.28f, 2.4f);
    ApplyPhaseTelegraphTint(PhaseTelegraphSweep, Accent, 1.9f);
    ApplyPhaseTelegraphTint(PhaseTelegraphAddBeaconA, Accent, Phase >= 3 ? 2.2f : 1.25f);
    ApplyPhaseTelegraphTint(PhaseTelegraphAddBeaconB, Accent, Phase >= 3 ? 2.2f : 1.25f);

    if (PhaseTelegraphLight)
    {
        PhaseTelegraphLight->SetLightColor(PhaseTelegraphColor);
    }

    ApplyPhaseTelegraphVisibility(true);
    Tags.AddUnique(FName("BossPhaseTelegraphActive"));
    Tags.AddUnique(Phase >= 3 ? FName("BossPhase3SpawnAddsTelegraph") : FName("BossPhase2RegenSprintTelegraph"));
    Tags.AddUnique(FName("EnemyTelegraphReadability"));
    Tags.AddUnique(FName("Top50Recommendations"));

    OnBossPhaseTelegraphStarted(Phase);
}

void ABossZombieActor::UpdatePhaseTelegraph(float DeltaSeconds)
{
    if (PhaseTelegraphTimeRemaining <= 0.0f)
    {
        ApplyPhaseTelegraphVisibility(false);
        return;
    }

    PhaseTelegraphElapsed += DeltaSeconds;
    PhaseTelegraphTimeRemaining -= DeltaSeconds;

    const float MotionScale = bPhaseTelegraphReducedMotion ? 0.24f : 1.0f;
    const float Pulse = 0.62f + 0.38f * FMath::Sin(PhaseTelegraphElapsed * (bPhaseTelegraphReducedMotion ? 1.35f : 6.6f));
    const float PhaseScale = PhaseTelegraphActivePhase >= 3 ? 1.22f : 1.0f;
    const float RadiusScale = FMath::Max(0.1f, PhaseTelegraphRadius / 560.0f);

    if (PhaseTelegraphRing)
    {
        const float RingScale = (4.2f + Pulse * 1.1f) * PhaseScale * RadiusScale;
        PhaseTelegraphRing->SetRelativeScale3D(FVector(RingScale, RingScale, 0.035f));
    }
    if (PhaseTelegraphCore)
    {
        const float CoreBob = FMath::Sin(PhaseTelegraphElapsed * (bPhaseTelegraphReducedMotion ? 0.9f : 3.1f)) * (bPhaseTelegraphReducedMotion ? 3.0f : 22.0f);
        PhaseTelegraphCore->SetRelativeLocation(FVector(0.0f, 0.0f, 220.0f + CoreBob));
        PhaseTelegraphCore->SetRelativeScale3D(FVector((0.36f + Pulse * 0.18f) * PhaseScale));
    }
    if (PhaseTelegraphSweep)
    {
        PhaseTelegraphSweep->SetRelativeRotation(FRotator(0.0f, PhaseTelegraphElapsed * 118.0f * MotionScale, 0.0f));
        PhaseTelegraphSweep->SetRelativeScale3D(FVector((5.2f + Pulse * 1.0f) * RadiusScale, 0.08f, 0.035f));
    }

    const float BeaconOrbit = PhaseTelegraphElapsed * 72.0f * MotionScale;
    const float BeaconRadius = (PhaseTelegraphActivePhase >= 3 ? 330.0f : 240.0f) * RadiusScale;
    const float BeaconScale = (PhaseTelegraphActivePhase >= 3 ? 0.30f : 0.18f) + Pulse * 0.08f;
    if (PhaseTelegraphAddBeaconA)
    {
        PhaseTelegraphAddBeaconA->SetRelativeLocation(FRotator(0.0f, BeaconOrbit, 0.0f).RotateVector(FVector(BeaconRadius, 0.0f, 12.0f)));
        PhaseTelegraphAddBeaconA->SetRelativeScale3D(FVector(BeaconScale));
        PhaseTelegraphAddBeaconA->SetVisibility(PhaseTelegraphActivePhase >= 3, true);
    }
    if (PhaseTelegraphAddBeaconB)
    {
        PhaseTelegraphAddBeaconB->SetRelativeLocation(FRotator(0.0f, BeaconOrbit + 180.0f, 0.0f).RotateVector(FVector(BeaconRadius, 0.0f, 12.0f)));
        PhaseTelegraphAddBeaconB->SetRelativeScale3D(FVector(BeaconScale));
        PhaseTelegraphAddBeaconB->SetVisibility(PhaseTelegraphActivePhase >= 3, true);
    }
    if (PhaseTelegraphLight)
    {
        PhaseTelegraphLight->SetIntensity((PhaseTelegraphActivePhase >= 3 ? 11500.0f : 7600.0f) + Pulse * 6200.0f);
    }

    if (PhaseTelegraphTimeRemaining <= 0.0f)
    {
        ApplyPhaseTelegraphVisibility(false);
    }
}

void ABossZombieActor::ApplyPhaseTelegraphVisibility(bool bVisible)
{
    for (UStaticMeshComponent* Component : { PhaseTelegraphRing, PhaseTelegraphCore, PhaseTelegraphSweep, PhaseTelegraphAddBeaconA, PhaseTelegraphAddBeaconB })
    {
        if (Component)
        {
            Component->SetVisibility(bVisible, true);
        }
    }
    if (PhaseTelegraphLight)
    {
        PhaseTelegraphLight->SetIntensity(bVisible ? 8000.0f : 0.0f);
    }
}

void ABossZombieActor::ConfigurePhaseTelegraphComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->SetVisibility(false, true);
    Component->ComponentTags.AddUnique(FName("BossPhaseTelegraph"));
    Component->ComponentTags.AddUnique(FName("EnemyTelegraphReadability"));
    Component->ComponentTags.AddUnique(FName("CharacterAnimationDeepDive"));
}

void ABossZombieActor::ApplyPhaseTelegraphTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
{
    if (!Component)
    {
        return;
    }

    UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(0);
    if (!MID)
    {
        return;
    }

    MID->SetVectorParameterValue(TEXT("Color"), Tint);
    MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
    MID->SetVectorParameterValue(TEXT("EmissiveColor"), Tint * EmissiveScale);
}
