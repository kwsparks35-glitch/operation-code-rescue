#include "RescueExtractionPresentationActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARescueExtractionPresentationActor::ARescueExtractionPresentationActor()
{
    PrimaryActorTick.bCanEverTick = true;
    SetLifeSpan(DurationSeconds + 0.75f);

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RescueExtractionPresentationRoot"));
    RootComponent = Root;

    LandingDisc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionLandingDisc"));
    LandingDisc->SetupAttachment(Root);
    LandingDisc->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
    LandingDisc->SetRelativeScale3D(FVector(2.9f, 2.9f, 0.035f));

    RescueBeam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionRescueBeam"));
    RescueBeam->SetupAttachment(Root);
    RescueBeam->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
    RescueBeam->SetRelativeScale3D(FVector(0.42f, 0.42f, 5.0f));

    SweepArmA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionSweepArmA"));
    SweepArmA->SetupAttachment(Root);
    SweepArmA->SetRelativeLocation(FVector(0.0f, 0.0f, 38.0f));
    SweepArmA->SetRelativeScale3D(FVector(3.4f, 0.045f, 0.045f));

    SweepArmB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionSweepArmB"));
    SweepArmB->SetupAttachment(Root);
    SweepArmB->SetRelativeLocation(FVector(0.0f, 0.0f, 42.0f));
    SweepArmB->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    SweepArmB->SetRelativeScale3D(FVector(3.05f, 0.04f, 0.04f));

    LiftMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionLiftMarker"));
    LiftMarker->SetupAttachment(Root);
    LiftMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 142.0f));
    LiftMarker->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.42f));

    OrbitBeaconA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionOrbitBeaconA"));
    OrbitBeaconA->SetupAttachment(Root);
    OrbitBeaconA->SetRelativeLocation(FVector(210.0f, 0.0f, 82.0f));
    OrbitBeaconA->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.16f));

    OrbitBeaconB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionOrbitBeaconB"));
    OrbitBeaconB->SetupAttachment(Root);
    OrbitBeaconB->SetRelativeLocation(FVector(-105.0f, 182.0f, 94.0f));
    OrbitBeaconB->SetRelativeScale3D(FVector(0.14f, 0.14f, 0.14f));

    OrbitBeaconC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionOrbitBeaconC"));
    OrbitBeaconC->SetupAttachment(Root);
    OrbitBeaconC->SetRelativeLocation(FVector(-105.0f, -182.0f, 88.0f));
    OrbitBeaconC->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.12f));

    RescueKeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ExtractionKeyLight"));
    RescueKeyLight->SetupAttachment(Root);
    RescueKeyLight->SetRelativeLocation(FVector(0.0f, 0.0f, 265.0f));
    RescueKeyLight->SetIntensity(18000.0f);
    RescueKeyLight->SetAttenuationRadius(1300.0f);
    RescueKeyLight->SetLightColor(AccentColor);

    RescueFillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ExtractionFillLight"));
    RescueFillLight->SetupAttachment(Root);
    RescueFillLight->SetRelativeLocation(FVector(-180.0f, 180.0f, 120.0f));
    RescueFillLight->SetIntensity(4200.0f);
    RescueFillLight->SetAttenuationRadius(850.0f);
    RescueFillLight->SetLightColor(FLinearColor(0.08f, 0.85f, 1.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    if (CylinderMesh.Succeeded())
    {
        if (LandingDisc)
        {
            LandingDisc->SetStaticMesh(CylinderMesh.Object);
        }
        if (RescueBeam)
        {
            RescueBeam->SetStaticMesh(CylinderMesh.Object);
        }
    }
    if (CubeMesh.Succeeded())
    {
        if (SweepArmA)
        {
            SweepArmA->SetStaticMesh(CubeMesh.Object);
        }
        if (SweepArmB)
        {
            SweepArmB->SetStaticMesh(CubeMesh.Object);
        }
    }
    if (SphereMesh.Succeeded())
    {
        for (UStaticMeshComponent* Component : { LiftMarker, OrbitBeaconA, OrbitBeaconB, OrbitBeaconC })
        {
            if (Component)
            {
                Component->SetStaticMesh(SphereMesh.Object);
            }
        }
    }

    for (UStaticMeshComponent* Component : { LandingDisc, RescueBeam, SweepArmA, SweepArmB, LiftMarker, OrbitBeaconA, OrbitBeaconB, OrbitBeaconC })
    {
        TagPresentationComponent(Component);
    }

    Tags.AddUnique(FName("RescueExtractionPresentation"));
    Tags.AddUnique(FName("SequencerReadyFallback"));
    Tags.AddUnique(FName("ControlRigReadyFallback"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("ReleaseDossier"));
}

void ARescueExtractionPresentationActor::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(DurationSeconds + 0.75f);
    ApplyVisualTints();
    OnRescuePresentationStarted(PresentedSurvivorName, PresentedCityIndex);

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueRescuePresentation] city=%d survivor='%s' started runtime extraction beat with Sequencer/ControlRig-ready fallback%s."),
        PresentedCityIndex,
        *PresentedSurvivorName,
        bReducedMotion ? TEXT(" (reduced motion)") : TEXT(""));
}

void ARescueExtractionPresentationActor::ConfigurePresentation(
    const FString& InSurvivorName,
    int32 InCityIndex,
    const FLinearColor& InAccentColor,
    bool bInReducedMotion)
{
    PresentedSurvivorName = InSurvivorName;
    PresentedCityIndex = InCityIndex;
    AccentColor = InAccentColor;
    bReducedMotion = bInReducedMotion;
    ApplyVisualTints();
}

void ARescueExtractionPresentationActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ElapsedSeconds += DeltaSeconds;
    const float Duration = FMath::Max(DurationSeconds, 0.1f);
    const float T = FMath::Clamp(ElapsedSeconds / Duration, 0.0f, 1.0f);
    const float Intro = FMath::Clamp(T * 4.0f, 0.0f, 1.0f);
    const float Outro = FMath::Clamp((1.0f - T) * 4.0f, 0.0f, 1.0f);
    const float Envelope = FMath::Sin(T * PI);
    const float Pulse = 0.72f + 0.28f * FMath::Sin(ElapsedSeconds * (bReducedMotion ? 2.2f : 7.6f));
    const float MotionScale = bReducedMotion ? 0.22f : 1.0f;

    if (LandingDisc)
    {
        const float DiscScale = 1.0f + Envelope * 0.16f;
        LandingDisc->SetRelativeScale3D(FVector(2.9f * DiscScale, 2.9f * DiscScale, 0.035f));
    }
    if (RescueBeam)
    {
        const float BeamHeight = FMath::Lerp(1.35f, 5.25f, Intro) * FMath::Max(Outro, 0.12f);
        RescueBeam->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f + BeamHeight * 50.0f));
        RescueBeam->SetRelativeScale3D(FVector(0.30f + Envelope * 0.24f, 0.30f + Envelope * 0.24f, BeamHeight));
    }
    if (SweepArmA)
    {
        SweepArmA->SetRelativeRotation(FRotator(0.0f, ElapsedSeconds * 76.0f * MotionScale, 0.0f));
        SweepArmA->SetRelativeScale3D(FVector(3.0f + Envelope * 0.6f, 0.045f, 0.045f));
    }
    if (SweepArmB)
    {
        SweepArmB->SetRelativeRotation(FRotator(0.0f, 90.0f - ElapsedSeconds * 54.0f * MotionScale, 0.0f));
        SweepArmB->SetRelativeScale3D(FVector(2.65f + Envelope * 0.46f, 0.04f, 0.04f));
    }
    if (LiftMarker)
    {
        LiftMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 105.0f + 178.0f * Intro + 22.0f * Envelope));
        const float MarkerScale = 0.25f + 0.28f * Intro + 0.08f * Pulse;
        LiftMarker->SetRelativeScale3D(FVector(MarkerScale));
    }

    const float OrbitSpeed = ElapsedSeconds * 1.95f * MotionScale;
    const float Radius = 218.0f + Envelope * 30.0f;
    if (OrbitBeaconA)
    {
        OrbitBeaconA->SetRelativeLocation(FVector(FMath::Cos(OrbitSpeed) * Radius, FMath::Sin(OrbitSpeed) * Radius, 86.0f + 20.0f * Envelope));
    }
    if (OrbitBeaconB)
    {
        OrbitBeaconB->SetRelativeLocation(FVector(FMath::Cos(OrbitSpeed + 2.094f) * Radius, FMath::Sin(OrbitSpeed + 2.094f) * Radius, 98.0f + 16.0f * Pulse));
    }
    if (OrbitBeaconC)
    {
        OrbitBeaconC->SetRelativeLocation(FVector(FMath::Cos(OrbitSpeed + 4.188f) * Radius, FMath::Sin(OrbitSpeed + 4.188f) * Radius, 92.0f + 14.0f * Envelope));
    }

    if (RescueKeyLight)
    {
        RescueKeyLight->SetIntensity((9000.0f + 10500.0f * Envelope) * FMath::Max(Outro, 0.1f));
    }
    if (RescueFillLight)
    {
        RescueFillLight->SetIntensity((2400.0f + 3600.0f * Pulse) * FMath::Max(Outro, 0.1f));
    }

    if (ElapsedSeconds >= DurationSeconds)
    {
        Destroy();
    }
}

void ARescueExtractionPresentationActor::ApplyVisualTints()
{
    ApplyComponentTint(LandingDisc, AccentColor * 0.55f + FLinearColor(0.03f, 0.06f, 0.08f), 0.75f);
    ApplyComponentTint(RescueBeam, AccentColor * 0.62f + FLinearColor(0.08f, 0.85f, 1.0f) * 0.38f, 1.25f);
    ApplyComponentTint(SweepArmA, AccentColor, 1.8f);
    ApplyComponentTint(SweepArmB, FLinearColor(0.08f, 0.85f, 1.0f), 1.4f);
    ApplyComponentTint(LiftMarker, AccentColor * 0.76f + FLinearColor::White * 0.24f, 1.55f);
    ApplyComponentTint(OrbitBeaconA, AccentColor, 1.6f);
    ApplyComponentTint(OrbitBeaconB, FLinearColor(0.08f, 0.85f, 1.0f), 1.35f);
    ApplyComponentTint(OrbitBeaconC, AccentColor * 0.45f + FLinearColor(0.08f, 0.85f, 1.0f) * 0.55f, 1.25f);

    if (RescueKeyLight)
    {
        RescueKeyLight->SetLightColor(AccentColor);
    }
    if (RescueFillLight)
    {
        RescueFillLight->SetLightColor(FLinearColor(0.08f, 0.85f, 1.0f));
    }
}

void ARescueExtractionPresentationActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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

void ARescueExtractionPresentationActor::TagPresentationComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->ComponentTags.AddUnique(FName("RescueExtractionPresentation"));
    Component->ComponentTags.AddUnique(FName("SequencerReadyFallback"));
    Component->ComponentTags.AddUnique(FName("ControlRigReadyFallback"));
}
