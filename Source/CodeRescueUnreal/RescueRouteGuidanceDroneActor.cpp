#include "RescueRouteGuidanceDroneActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARescueRouteGuidanceDroneActor::ARescueRouteGuidanceDroneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RouteGuidanceDroneRoot"));
    RootComponent = Root;

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteGuidanceDroneBody"));
    Body->SetupAttachment(Root);
    Body->SetRelativeScale3D(FVector(0.36f, 0.24f, 0.18f));

    Nose = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteGuidanceDroneNose"));
    Nose->SetupAttachment(Root);
    Nose->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f));
    Nose->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.12f));

    RotorArmA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteGuidanceDroneRotorArmA"));
    RotorArmA->SetupAttachment(Root);
    RotorArmA->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
    RotorArmA->SetRelativeScale3D(FVector(0.92f, 0.045f, 0.025f));

    RotorArmB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteGuidanceDroneRotorArmB"));
    RotorArmB->SetupAttachment(Root);
    RotorArmB->SetRelativeLocation(FVector(0.0f, 0.0f, 23.0f));
    RotorArmB->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    RotorArmB->SetRelativeScale3D(FVector(0.82f, 0.04f, 0.022f));

    SignalPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteGuidanceDroneSignalPanel"));
    SignalPanel->SetupAttachment(Root);
    SignalPanel->SetRelativeLocation(FVector(-24.0f, 0.0f, -30.0f));
    SignalPanel->SetRelativeScale3D(FVector(0.08f, 0.48f, 0.20f));

    GuidanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RouteGuidanceDroneLight"));
    GuidanceLight->SetupAttachment(Root);
    GuidanceLight->SetRelativeLocation(FVector(0.0f, 0.0f, -16.0f));
    GuidanceLight->SetIntensity(5200.0f);
    GuidanceLight->SetAttenuationRadius(820.0f);
    GuidanceLight->SetCastShadows(false);
    GuidanceLight->SetLightColor(GuidanceTint);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    if (CubeMesh.Succeeded())
    {
        for (UStaticMeshComponent* Component : { Body, RotorArmA, RotorArmB, SignalPanel })
        {
            if (Component)
            {
                Component->SetStaticMesh(CubeMesh.Object);
            }
        }
    }
    if (SphereMesh.Succeeded() && Nose)
    {
        Nose->SetStaticMesh(SphereMesh.Object);
    }

    for (UStaticMeshComponent* Component : { Body, Nose, RotorArmA, RotorArmB, SignalPanel })
    {
        TagGuidanceComponent(Component);
    }

    Tags.AddUnique(FName("RescueRouteGuidanceDrone"));
    Tags.AddUnique(FName("AnimatedWayfinding"));
    Tags.AddUnique(FName("CodingToRescueWorldResponse"));
    Tags.AddUnique(FName("TerminalSolvedRouteVisible"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
}

void ARescueRouteGuidanceDroneActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyTint();
}

void ARescueRouteGuidanceDroneActor::ConfigureDrone(
    const FVector& InRouteStartWorld,
    const FVector& InRouteEndWorld,
    const FLinearColor& InTint,
    float InPhase,
    bool bInReducedMotion)
{
    RouteStartWorld = InRouteStartWorld;
    RouteEndWorld = InRouteEndWorld;
    GuidanceTint = InTint;
    Phase = InPhase;
    bReducedMotion = bInReducedMotion;
    BobAmplitude = bReducedMotion ? 8.0f : 28.0f;
    PatrolSpeed = bReducedMotion ? 0.18f : 0.52f;
    ApplyTint();
}

void ARescueRouteGuidanceDroneActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MotionTime += DeltaSeconds;
    const float Oscillation = FMath::Sin(MotionTime * PatrolSpeed + Phase);
    const float Alpha = 0.5f + 0.5f * Oscillation;
    const FVector RouteLocation = FMath::Lerp(RouteStartWorld, RouteEndWorld, Alpha);
    const float Bob = FMath::Sin(MotionTime * (bReducedMotion ? 1.4f : 3.4f) + Phase * 0.7f) * BobAmplitude;

    SetActorLocation(RouteLocation + FVector(0.0f, 0.0f, HoverHeight + Bob));

    const FVector RouteDelta = RouteEndWorld - RouteStartWorld;
    const float RouteYaw = FMath::RadiansToDegrees(FMath::Atan2(RouteDelta.Y, RouteDelta.X));
    SetActorRotation(FRotator(0.0f, RouteYaw, 0.0f));

    const float MotionScale = bReducedMotion ? 0.35f : 1.0f;
    if (RotorArmA)
    {
        RotorArmA->SetRelativeRotation(FRotator(0.0f, MotionTime * 640.0f * MotionScale, 0.0f));
    }
    if (RotorArmB)
    {
        RotorArmB->SetRelativeRotation(FRotator(0.0f, 90.0f - MotionTime * 520.0f * MotionScale, 0.0f));
    }
    if (SignalPanel)
    {
        const float PanelSwing = FMath::Sin(MotionTime * 2.1f + Phase) * (bReducedMotion ? 3.0f : 11.0f);
        SignalPanel->SetRelativeRotation(FRotator(PanelSwing, 0.0f, PanelSwing * 0.35f));
    }
    if (GuidanceLight)
    {
        const float Pulse = 0.76f + 0.24f * FMath::Sin(MotionTime * (bReducedMotion ? 1.6f : 5.1f) + Phase);
        GuidanceLight->SetIntensity(3600.0f + 4200.0f * Pulse);
    }
}

void ARescueRouteGuidanceDroneActor::ApplyTint()
{
    ApplyComponentTint(Body, FLinearColor(0.045f, 0.055f, 0.060f) + GuidanceTint * 0.12f, 0.08f);
    ApplyComponentTint(Nose, GuidanceTint * 0.78f + FLinearColor::White * 0.22f, 1.45f);
    ApplyComponentTint(RotorArmA, GuidanceTint * 0.45f + FLinearColor(0.04f, 0.05f, 0.055f), 0.65f);
    ApplyComponentTint(RotorArmB, GuidanceTint * 0.35f + FLinearColor(0.04f, 0.05f, 0.055f), 0.48f);
    ApplyComponentTint(SignalPanel, GuidanceTint, 1.75f);

    if (GuidanceLight)
    {
        GuidanceLight->SetLightColor(GuidanceTint);
    }
}

void ARescueRouteGuidanceDroneActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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

void ARescueRouteGuidanceDroneActor::TagGuidanceComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->ComponentTags.AddUnique(FName("RescueRouteGuidanceDrone"));
    Component->ComponentTags.AddUnique(FName("AnimatedWayfinding"));
    Component->ComponentTags.AddUnique(FName("TerminalSolvedRouteVisible"));
}
