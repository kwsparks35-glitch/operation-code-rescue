#include "BossRevealPresentationActor.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABossRevealPresentationActor::ABossRevealPresentationActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BossRevealRoot"));
    SetRootComponent(SceneRoot);

    ArenaRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealArenaRing"));
    ArenaRing->SetupAttachment(SceneRoot);
    ArenaRing->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
    ArenaRing->SetRelativeScale3D(FVector(7.0f, 7.0f, 0.055f));

    ThreatGateA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealThreatGateA"));
    ThreatGateA->SetupAttachment(SceneRoot);
    ThreatGateA->SetRelativeLocation(FVector(-260.0f, 0.0f, 230.0f));
    ThreatGateA->SetRelativeScale3D(FVector(0.08f, 3.9f, 4.4f));

    ThreatGateB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealThreatGateB"));
    ThreatGateB->SetupAttachment(SceneRoot);
    ThreatGateB->SetRelativeLocation(FVector(260.0f, 0.0f, 230.0f));
    ThreatGateB->SetRelativeScale3D(FVector(0.08f, 3.9f, 4.4f));

    SweepA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealSweepA"));
    SweepA->SetupAttachment(SceneRoot);
    SweepA->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
    SweepA->SetRelativeScale3D(FVector(5.2f, 0.07f, 0.035f));

    SweepB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealSweepB"));
    SweepB->SetupAttachment(SceneRoot);
    SweepB->SetRelativeLocation(FVector(0.0f, 0.0f, 92.0f));
    SweepB->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    SweepB->SetRelativeScale3D(FVector(4.2f, 0.06f, 0.035f));

    BossCrown = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealCrown"));
    BossCrown->SetupAttachment(SceneRoot);
    BossCrown->SetRelativeLocation(FVector(0.0f, 0.0f, 535.0f));
    BossCrown->SetRelativeScale3D(FVector(0.62f));

    BeaconA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealBeaconA"));
    BeaconA->SetupAttachment(SceneRoot);
    BeaconA->SetRelativeLocation(FVector(460.0f, 0.0f, 170.0f));
    BeaconA->SetRelativeScale3D(FVector(0.28f));

    BeaconB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealBeaconB"));
    BeaconB->SetupAttachment(SceneRoot);
    BeaconB->SetRelativeLocation(FVector(-230.0f, 398.0f, 170.0f));
    BeaconB->SetRelativeScale3D(FVector(0.28f));

    BeaconC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossRevealBeaconC"));
    BeaconC->SetupAttachment(SceneRoot);
    BeaconC->SetRelativeLocation(FVector(-230.0f, -398.0f, 170.0f));
    BeaconC->SetRelativeScale3D(FVector(0.28f));

    WarningLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BossRevealWarningLight"));
    WarningLight->SetupAttachment(SceneRoot);
    WarningLight->SetRelativeLocation(FVector(0.0f, 0.0f, 360.0f));
    WarningLight->SetLightColor(WarningColor);
    WarningLight->SetIntensity(0.0f);
    WarningLight->SetAttenuationRadius(2500.0f);
    WarningLight->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CylinderMesh.Succeeded() && ArenaRing)
    {
        ArenaRing->SetStaticMesh(CylinderMesh.Object);
    }
    if (CubeMesh.Succeeded())
    {
        ThreatGateA->SetStaticMesh(CubeMesh.Object);
        ThreatGateB->SetStaticMesh(CubeMesh.Object);
        SweepA->SetStaticMesh(CubeMesh.Object);
        SweepB->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        BossCrown->SetStaticMesh(SphereMesh.Object);
        BeaconA->SetStaticMesh(SphereMesh.Object);
        BeaconB->SetStaticMesh(SphereMesh.Object);
        BeaconC->SetStaticMesh(SphereMesh.Object);
    }

    for (UStaticMeshComponent* Component : { ArenaRing, ThreatGateA, ThreatGateB, SweepA, SweepB, BossCrown, BeaconA, BeaconB, BeaconC })
    {
        ConfigureRevealComponent(Component);
    }

    Tags.AddUnique(FName("BossRevealPresentation"));
    Tags.AddUnique(FName("SequencerReadyFallback"));
    Tags.AddUnique(FName("SequencerIntroBossRevealBlocking"));
    Tags.AddUnique(FName("SequencerBossRevealBeat"));
    Tags.AddUnique(FName("CinematicCameraBlockingReady"));
    Tags.AddUnique(FName("ControlRigReadyFallback"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("ReleaseDossier"));
}

void ABossRevealPresentationActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyRevealVisualState(false);
}

void ABossRevealPresentationActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bRevealStarted)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn && FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation()) <= FMath::Square(TriggerRadius))
        {
            BeginReveal();
        }
        return;
    }

    RevealElapsed += DeltaSeconds;
    const float MotionScale = bReducedMotion ? 0.24f : 1.0f;
    const float Pulse = 0.62f + 0.38f * FMath::Sin(RevealElapsed * (bReducedMotion ? 1.25f : 6.0f));

    if (BossActor)
    {
        SetActorLocation(BossActor->GetActorLocation() + FVector(0.0f, 0.0f, 22.0f));
    }
    if (ArenaRing)
    {
        const float RingScale = 6.2f + Pulse * 1.2f;
        ArenaRing->SetRelativeScale3D(FVector(RingScale, RingScale, 0.055f));
    }
    if (ThreatGateA)
    {
        ThreatGateA->SetRelativeLocation(FVector(-260.0f - Pulse * 52.0f, 0.0f, 230.0f));
    }
    if (ThreatGateB)
    {
        ThreatGateB->SetRelativeLocation(FVector(260.0f + Pulse * 52.0f, 0.0f, 230.0f));
    }
    if (SweepA)
    {
        SweepA->SetRelativeRotation(FRotator(0.0f, RevealElapsed * 80.0f * MotionScale, 0.0f));
    }
    if (SweepB)
    {
        SweepB->SetRelativeRotation(FRotator(0.0f, 90.0f - RevealElapsed * 54.0f * MotionScale, 0.0f));
    }
    if (BossCrown)
    {
        const float CrownBob = FMath::Sin(RevealElapsed * (bReducedMotion ? 0.9f : 2.6f)) * (bReducedMotion ? 4.0f : 30.0f);
        BossCrown->SetRelativeLocation(FVector(0.0f, 0.0f, 535.0f + CrownBob));
        BossCrown->SetRelativeScale3D(FVector(0.54f + Pulse * 0.16f));
    }

    const float BeaconSpin = RevealElapsed * 66.0f * MotionScale;
    if (BeaconA)
    {
        BeaconA->SetRelativeLocation(FRotator(0.0f, BeaconSpin, 0.0f).RotateVector(FVector(460.0f, 0.0f, 170.0f)));
    }
    if (BeaconB)
    {
        BeaconB->SetRelativeLocation(FRotator(0.0f, BeaconSpin + 120.0f, 0.0f).RotateVector(FVector(460.0f, 0.0f, 170.0f)));
    }
    if (BeaconC)
    {
        BeaconC->SetRelativeLocation(FRotator(0.0f, BeaconSpin + 240.0f, 0.0f).RotateVector(FVector(460.0f, 0.0f, 170.0f)));
    }
    if (WarningLight)
    {
        WarningLight->SetIntensity(9000.0f + Pulse * 15000.0f);
    }

    if (RevealElapsed >= DurationSeconds)
    {
        Destroy();
    }
}

void ABossRevealPresentationActor::ConfigureReveal(
    AActor* InBossActor,
    int32 InCityIndex,
    const FString& InCityName,
    const FString& InBossTitle,
    const FLinearColor& InWarningColor,
    bool bInReducedMotion)
{
    BossActor = InBossActor;
    CityIndex = InCityIndex;
    CityName = InCityName;
    BossTitle = InBossTitle;
    WarningColor = InWarningColor;
    bReducedMotion = bInReducedMotion;

    if (BossActor)
    {
        SetActorLocation(BossActor->GetActorLocation() + FVector(0.0f, 0.0f, 22.0f));
    }

    ApplyComponentTint(ArenaRing, WarningColor * 0.52f + FLinearColor(1.0f, 0.78f, 0.08f) * 0.48f, 1.35f);
    ApplyComponentTint(ThreatGateA, WarningColor, 1.75f);
    ApplyComponentTint(ThreatGateB, WarningColor, 1.75f);
    ApplyComponentTint(SweepA, FLinearColor(1.0f, 0.06f, 0.2f), 1.85f);
    ApplyComponentTint(SweepB, FLinearColor(1.0f, 0.78f, 0.08f), 1.55f);
    ApplyComponentTint(BossCrown, WarningColor * 0.68f + FLinearColor::White * 0.32f, 2.1f);
    ApplyComponentTint(BeaconA, WarningColor, 1.9f);
    ApplyComponentTint(BeaconB, FLinearColor(1.0f, 0.78f, 0.08f), 1.65f);
    ApplyComponentTint(BeaconC, WarningColor * 0.5f + FLinearColor(0.7f, 0.0f, 1.0f) * 0.5f, 1.8f);

    if (WarningLight)
    {
        WarningLight->SetLightColor(WarningColor);
    }
}

void ABossRevealPresentationActor::BeginReveal()
{
    bRevealStarted = true;
    RevealElapsed = 0.0f;
    ApplyRevealVisualState(true);

    Tags.AddUnique(FName("BossRevealTriggered"));
    Tags.AddUnique(FName("SequencerReadyFallback"));
    Tags.AddUnique(FName("SequencerBossRevealBeat"));
    Tags.AddUnique(FName("CinematicCameraBlockingReady"));
    Tags.AddUnique(FName("ControlRigReadyFallback"));

    OnBossRevealStarted(BossActor);

    if (GEngine)
    {
        const FString DisplayTitle = BossTitle.IsEmpty() ? TEXT("Boss revealed") : BossTitle;
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, DisplayTitle);
    }
}

void ABossRevealPresentationActor::ApplyRevealVisualState(bool bVisible)
{
    for (UStaticMeshComponent* Component : { ArenaRing, ThreatGateA, ThreatGateB, SweepA, SweepB, BossCrown, BeaconA, BeaconB, BeaconC })
    {
        if (Component)
        {
            Component->SetVisibility(bVisible, true);
        }
    }

    if (WarningLight)
    {
        WarningLight->SetIntensity(bVisible ? 12000.0f : 0.0f);
    }
}

void ABossRevealPresentationActor::ConfigureRevealComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->SetVisibility(false, true);
    Component->ComponentTags.AddUnique(FName("BossRevealPresentation"));
    Component->ComponentTags.AddUnique(FName("SequencerReadyFallback"));
    Component->ComponentTags.AddUnique(FName("SequencerIntroBossRevealBlocking"));
    Component->ComponentTags.AddUnique(FName("SequencerBossRevealBeat"));
    Component->ComponentTags.AddUnique(FName("CinematicCameraBlockingReady"));
    Component->ComponentTags.AddUnique(FName("ControlRigReadyFallback"));
}

void ABossRevealPresentationActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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
