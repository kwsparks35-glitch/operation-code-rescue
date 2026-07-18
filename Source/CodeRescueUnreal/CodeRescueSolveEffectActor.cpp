// CodeRescueSolveEffectActor.cpp  -- see header for design intent and the Mac-compile DoD note.

#include "CodeRescueSolveEffectActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr int32 MaxPulseNodes = 6;
}

ACodeRescueSolveEffectActor::ACodeRescueSolveEffectActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SolveEffectRoot"));
    SetRootComponent(SceneRoot);

    BaseRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SolveEffectBaseRing"));
    BaseRing->SetupAttachment(SceneRoot);
    BaseRing->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
    BaseRing->SetRelativeScale3D(FVector(2.6f, 2.6f, 0.04f));

    RouteColumn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SolveEffectRouteColumn"));
    RouteColumn->SetupAttachment(SceneRoot);
    RouteColumn->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    RouteColumn->SetRelativeScale3D(FVector(0.16f, 0.16f, 2.6f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    if (CylinderMesh.Succeeded())
    {
        BaseRing->SetStaticMesh(CylinderMesh.Object);
        RouteColumn->SetStaticMesh(CylinderMesh.Object);
    }

    // One pulse node per unit of the player's OUTPUT (up to a cook-safe cap) -- intrinsic integration.
    for (int32 i = 0; i < MaxPulseNodes; ++i)
    {
        UStaticMeshComponent* Node = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("SolveEffectPulseNode_%d"), i));
        Node->SetupAttachment(SceneRoot);
        Node->SetRelativeLocation(FVector(0.0f, 90.0f + i * 70.0f, 60.0f));
        Node->SetRelativeScale3D(FVector(0.22f));
        if (SphereMesh.Succeeded()) { Node->SetStaticMesh(SphereMesh.Object); }
        Node->SetVisibility(false, true);
        PulseNodes.Add(Node);
    }

    EffectLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SolveEffectLight"));
    EffectLight->SetupAttachment(SceneRoot);
    EffectLight->SetRelativeLocation(FVector(0.0f, 0.0f, 260.0f));
    EffectLight->SetIntensity(0.0f);
    EffectLight->SetAttenuationRadius(1800.0f);
    EffectLight->SetCastShadows(false);

    EffectLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SolveEffectLabel"));
    EffectLabel->SetupAttachment(SceneRoot);
    EffectLabel->SetRelativeLocation(FVector(0.0f, -220.0f, 430.0f));
    EffectLabel->SetHorizontalAlignment(EHTA_Center);
    EffectLabel->SetVerticalAlignment(EVRTA_TextCenter);
    EffectLabel->SetWorldSize(58.0f);
    EffectLabel->SetTextRenderColor(FColor::Green);
    EffectLabel->SetText(FText::FromString(TEXT("ROUTE RESTORED")));

    Tags.AddUnique(FName("CodeRescueSolveEffect"));
    Tags.AddUnique(FName("IntrinsicIntegration"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
}

void ACodeRescueSolveEffectActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyAccent();
}

void ACodeRescueSolveEffectActor::ConfigureSolveEffect(const FString& InWorldEffectText, const FLinearColor& InAccentColor,
                                                       int32 InOutputMagnitude, bool bInReducedMotion, float InLifetimeSeconds)
{
    WorldEffectText = InWorldEffectText;
    AccentColor = InAccentColor;
    OutputMagnitude = FMath::Max(0, InOutputMagnitude);
    bReducedMotion = bInReducedMotion;
    EffectLifetimeSeconds = FMath::Max(1.0f, InLifetimeSeconds);

    ApplyAccent();

    const int32 Lit = FMath::Clamp(OutputMagnitude, 0, PulseNodes.Num());
    for (int32 i = 0; i < PulseNodes.Num(); ++i)
    {
        if (PulseNodes[i]) { PulseNodes[i]->SetVisibility(i < Lit, true); }
    }

    OnSolveEffectStarted(OutputMagnitude);
    SetLifeSpan(EffectLifetimeSeconds);
}

void ACodeRescueSolveEffectActor::ApplyAccent()
{
    if (EffectLight)
    {
        EffectLight->SetLightColor(AccentColor);
        EffectLight->SetIntensity(bReducedMotion ? 4200.0f : 6400.0f);
    }
    if (EffectLabel)
    {
        EffectLabel->SetTextRenderColor(AccentColor.ToFColor(true));
        if (!WorldEffectText.IsEmpty())
        {
            EffectLabel->SetText(FText::FromString(WorldEffectText));
        }
    }
}

void ACodeRescueSolveEffectActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    MotionTime += DeltaSeconds;

    const float MotionScale = bReducedMotion ? 0.25f : 1.0f;
    const float Pulse = 0.6f + 0.4f * FMath::Sin(MotionTime * (bReducedMotion ? 1.2f : 5.4f));

    if (BaseRing)
    {
        const float Ring = 2.4f + Pulse * 0.6f;
        BaseRing->SetRelativeScale3D(FVector(Ring, Ring, 0.04f));
        BaseRing->SetRelativeRotation(FRotator(0.0f, MotionTime * 40.0f * MotionScale, 0.0f));
    }
    if (RouteColumn)
    {
        RouteColumn->SetRelativeScale3D(FVector(0.15f + Pulse * 0.05f, 0.15f + Pulse * 0.05f, 2.4f + Pulse * 0.9f));
    }
    for (int32 i = 0; i < PulseNodes.Num(); ++i)
    {
        if (!PulseNodes[i] || !PulseNodes[i]->IsVisible()) { continue; }
        const float NodePulse = 0.18f + 0.10f * (0.5f + 0.5f * FMath::Sin(MotionTime * (bReducedMotion ? 1.1f : 4.6f) + i * 0.7f));
        PulseNodes[i]->SetRelativeScale3D(FVector(NodePulse));
        const float Bob = FMath::Sin(MotionTime * (bReducedMotion ? 0.8f : 3.0f) + i * 0.6f) * (bReducedMotion ? 3.0f : 14.0f);
        PulseNodes[i]->SetRelativeLocation(FVector(0.0f, 90.0f + i * 70.0f, 60.0f + Bob));
    }
    if (EffectLight)
    {
        const float Base = bReducedMotion ? 4200.0f : 6400.0f;
        EffectLight->SetIntensity(Base * (0.75f + 0.25f * Pulse));
    }
}
