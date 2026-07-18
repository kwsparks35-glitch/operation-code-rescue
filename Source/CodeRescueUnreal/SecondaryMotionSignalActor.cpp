#include "SecondaryMotionSignalActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASecondaryMotionSignalActor::ASecondaryMotionSignalActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("SecondaryMotionRoot"));
    RootComponent = Root;

    Mast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignalMast"));
    Mast->SetupAttachment(Root);
    Mast->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));
    Mast->SetRelativeScale3D(FVector(0.07f, 0.07f, 2.9f));

    Crossbar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignalCrossbar"));
    Crossbar->SetupAttachment(Root);
    Crossbar->SetRelativeLocation(FVector(0.0f, 0.0f, 292.0f));
    Crossbar->SetRelativeScale3D(FVector(0.08f, 1.15f, 0.06f));

    BannerA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignalBannerA"));
    BannerA->SetupAttachment(Root);
    BannerA->SetRelativeLocation(FVector(0.0f, 52.0f, 228.0f));
    BannerA->SetRelativeScale3D(FVector(0.035f, 0.78f, 0.52f));

    BannerB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignalBannerB"));
    BannerB->SetupAttachment(Root);
    BannerB->SetRelativeLocation(FVector(0.0f, -52.0f, 224.0f));
    BannerB->SetRelativeScale3D(FVector(0.035f, 0.66f, 0.44f));

    Cable = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignalCable"));
    Cable->SetupAttachment(Root);
    Cable->SetRelativeLocation(FVector(0.0f, 0.0f, 176.0f));
    Cable->SetRelativeScale3D(FVector(0.022f, 0.022f, 2.1f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        for (UStaticMeshComponent* Component : { Mast, Crossbar, BannerA, BannerB, Cable })
        {
            if (Component)
            {
                Component->SetStaticMesh(CubeMesh.Object);
            }
        }
    }

    for (UStaticMeshComponent* Component : { Mast, Crossbar, BannerA, BannerB, Cable })
    {
        if (!Component)
        {
            continue;
        }
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetMobility(EComponentMobility::Movable);
        Component->ComponentTags.AddUnique(FName("SecondaryMotionSignal"));
        Component->ComponentTags.AddUnique(FName("ProceduralClothFallback"));
    }

    Tags.AddUnique(FName("SecondaryMotionSignal"));
    Tags.AddUnique(FName("ProceduralClothFallback"));
    Tags.AddUnique(FName("ChaosClothReadyFallback"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
}

void ASecondaryMotionSignalActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyTint();
}

void ASecondaryMotionSignalActor::ConfigureSignal(const FLinearColor& InTint, float InPhase, float InAmplitudeDegrees, float InWindSpeed)
{
    SignalTint = InTint;
    FlutterPhase = InPhase;
    WindAmplitudeDegrees = FMath::Clamp(InAmplitudeDegrees, 0.0f, 45.0f);
    WindSpeed = FMath::Clamp(InWindSpeed, 0.0f, 12.0f);
    ApplyTint();
}

void ASecondaryMotionSignalActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MotionTime += DeltaSeconds;
    const float Base = MotionTime * WindSpeed + FlutterPhase;
    const float FlutterA = FMath::Sin(Base) * WindAmplitudeDegrees;
    const float FlutterB = FMath::Sin(Base * 1.27f + 1.9f) * WindAmplitudeDegrees * 0.72f;
    const float CableSwing = FMath::Sin(Base * 0.82f + 0.6f) * WindAmplitudeDegrees * 0.25f;

    if (BannerA)
    {
        BannerA->SetRelativeRotation(FRotator(FlutterA * 0.18f, FlutterA, FlutterA * 0.42f));
    }
    if (BannerB)
    {
        BannerB->SetRelativeRotation(FRotator(FlutterB * 0.22f, -FlutterB * 0.75f, FlutterB * 0.55f));
    }
    if (Cable)
    {
        Cable->SetRelativeRotation(FRotator(CableSwing, CableSwing * 0.35f, 0.0f));
    }
}

void ASecondaryMotionSignalActor::ApplyTint()
{
    ApplyComponentTint(Mast, FLinearColor(0.12f, 0.12f, 0.13f), 0.05f);
    ApplyComponentTint(Crossbar, FLinearColor(0.16f, 0.16f, 0.17f), 0.08f);
    ApplyComponentTint(Cable, FLinearColor(0.06f, 0.06f, 0.065f), 0.02f);
    ApplyComponentTint(BannerA, SignalTint, 0.75f);
    ApplyComponentTint(BannerB, SignalTint * 0.55f + FLinearColor(0.04f, 0.05f, 0.06f), 0.45f);
}

void ASecondaryMotionSignalActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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
