#include "HelipadActor.h"
#include "CityFastTravelWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueCollisionChannels.h"
#include "Blueprint/UserWidget.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AHelipadActor::AHelipadActor()
{
    PrimaryActorTick.bCanEverTick = true;

    PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
    SetRootComponent(PadMesh);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        PadMesh->SetStaticMesh(CylinderMesh.Object);
    }
    // Wide flat disk: 6m radius, ~30cm tall. Cylinder mesh is 1m^3 default.
    PadMesh->SetWorldScale3D(FVector(6.0f, 6.0f, 0.3f));
    PadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PadMesh->SetCollisionObjectType(ECC_WorldStatic);
    PadMesh->SetCollisionResponseToAllChannels(ECR_Block);
    PadMesh->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    PadMesh->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));

    ExtractionColumn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionReadyColumn"));
    ExtractionColumn->SetupAttachment(PadMesh);
    ExtractionColumn->SetRelativeLocation(FVector(0.0f, 0.0f, 330.0f));
    ExtractionColumn->SetRelativeScale3D(FVector(0.18f, 0.18f, 4.8f));

    ExtractionSweepA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionReadySweepA"));
    ExtractionSweepA->SetupAttachment(PadMesh);
    ExtractionSweepA->SetRelativeLocation(FVector(0.0f, 0.0f, 118.0f));
    ExtractionSweepA->SetRelativeScale3D(FVector(4.2f, 0.055f, 0.04f));

    ExtractionSweepB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionReadySweepB"));
    ExtractionSweepB->SetupAttachment(PadMesh);
    ExtractionSweepB->SetRelativeLocation(FVector(0.0f, 0.0f, 126.0f));
    ExtractionSweepB->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    ExtractionSweepB->SetRelativeScale3D(FVector(3.7f, 0.05f, 0.035f));

    ExtractionBeacon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionReadyBeacon"));
    ExtractionBeacon->SetupAttachment(PadMesh);
    ExtractionBeacon->SetRelativeLocation(FVector(0.0f, 0.0f, 545.0f));
    ExtractionBeacon->SetRelativeScale3D(FVector(0.36f, 0.36f, 0.36f));

    GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
    GlowLight->SetupAttachment(PadMesh);
    GlowLight->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    GlowLight->SetIntensity(8000.0f);
    GlowLight->SetLightColor(FLinearColor(0.2f, 0.6f, 1.0f));
    GlowLight->SetAttenuationRadius(2000.0f);
    GlowLight->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CylinderMesh.Succeeded() && ExtractionColumn)
    {
        ExtractionColumn->SetStaticMesh(CylinderMesh.Object);
    }
    if (CubeMesh.Succeeded())
    {
        if (ExtractionSweepA)
        {
            ExtractionSweepA->SetStaticMesh(CubeMesh.Object);
        }
        if (ExtractionSweepB)
        {
            ExtractionSweepB->SetStaticMesh(CubeMesh.Object);
        }
    }
    if (SphereMesh.Succeeded() && ExtractionBeacon)
    {
        ExtractionBeacon->SetStaticMesh(SphereMesh.Object);
    }

    for (UStaticMeshComponent* Component : { ExtractionColumn, ExtractionSweepA, ExtractionSweepB, ExtractionBeacon })
    {
        ConfigureExtractionComponent(Component);
    }

    Tags.AddUnique(FName("Helipad"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
}

void AHelipadActor::BeginPlay()
{
    Super::BeginPlay();

    if (CityLabel.IsEmpty())
    {
        CityLabel = FString::Printf(TEXT("Helipad %d"), CityIndex);
    }

    ApplyExtractionVisualState();
}

void AHelipadActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bExtractionReady)
    {
        return;
    }

    ExtractionPulseTime += DeltaSeconds;
    const float MotionScale = bReducedMotion ? 0.22f : 1.0f;
    const float Pulse = 0.68f + 0.32f * FMath::Sin(ExtractionPulseTime * (bReducedMotion ? 1.5f : 5.2f));

    if (ExtractionColumn)
    {
        const float ColumnScale = 4.3f + Pulse * 0.9f;
        ExtractionColumn->SetRelativeScale3D(FVector(0.18f + Pulse * 0.035f, 0.18f + Pulse * 0.035f, ColumnScale));
    }
    if (ExtractionSweepA)
    {
        ExtractionSweepA->SetRelativeRotation(FRotator(0.0f, ExtractionPulseTime * 72.0f * MotionScale, 0.0f));
    }
    if (ExtractionSweepB)
    {
        ExtractionSweepB->SetRelativeRotation(FRotator(0.0f, 90.0f - ExtractionPulseTime * 48.0f * MotionScale, 0.0f));
    }
    if (ExtractionBeacon)
    {
        const float Bob = FMath::Sin(ExtractionPulseTime * (bReducedMotion ? 1.2f : 3.0f)) * (bReducedMotion ? 6.0f : 28.0f);
        const float BeaconScale = 0.34f + Pulse * 0.08f;
        ExtractionBeacon->SetRelativeLocation(FVector(0.0f, 0.0f, 545.0f + Bob));
        ExtractionBeacon->SetRelativeScale3D(FVector(BeaconScale));
    }
    if (GlowLight)
    {
        GlowLight->SetIntensity(9800.0f + Pulse * 9000.0f);
    }
}

void AHelipadActor::OpenFastTravelMenu()
{
    if (ActiveFastTravelWidget)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    TSubclassOf<UUserWidget> WidgetClass = FastTravelWidgetClass.Get();
    if (!WidgetClass)
    {
        WidgetClass = UCityFastTravelWidget::StaticClass();
    }

    ActiveFastTravelWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (ActiveFastTravelWidget)
    {
        if (UCityFastTravelWidget* FastTravelWidget = Cast<UCityFastTravelWidget>(ActiveFastTravelWidget))
        {
            FastTravelWidget->ConfigureOpeningHelipadContext(
                CityIndex,
                CityLabel,
                bExtractionReady,
                ExtractionSurvivorName,
                ExtractionAccentColor);
        }
        ActiveFastTravelWidget->AddToViewport(100);
        ACodeRescueCharacter::SetUIOpen(true);
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(ActiveFastTravelWidget->TakeWidget());
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = true;
    }
}

void AHelipadActor::SetExtractionReady(const FString& SurvivorName, const FLinearColor& AccentColor, bool bInReducedMotion)
{
    bExtractionReady = true;
    ExtractionSurvivorName = SurvivorName;
    ExtractionAccentColor = AccentColor;
    bReducedMotion = bInReducedMotion;

    Tags.AddUnique(FName("ExtractionReadyHelipad"));
    Tags.AddUnique(FName("RescueLoopClosure"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("ReleaseDossier"));

    ApplyExtractionVisualState();
}

void AHelipadActor::ApplyExtractionVisualState()
{
    const bool bVisible = bExtractionReady;
    for (UStaticMeshComponent* Component : { ExtractionColumn, ExtractionSweepA, ExtractionSweepB, ExtractionBeacon })
    {
        if (Component)
        {
            Component->SetVisibility(bVisible, true);
        }
    }

    if (GlowLight)
    {
        GlowLight->SetLightColor(bExtractionReady ? ExtractionAccentColor : FLinearColor(0.2f, 0.6f, 1.0f));
        GlowLight->SetIntensity(bExtractionReady ? 12800.0f : 8000.0f);
        GlowLight->SetAttenuationRadius(bExtractionReady ? 2800.0f : 2000.0f);
    }

    ApplyComponentTint(ExtractionColumn, ExtractionAccentColor * 0.52f + FLinearColor(0.08f, 0.95f, 1.0f) * 0.48f, 1.25f);
    ApplyComponentTint(ExtractionSweepA, ExtractionAccentColor, 1.65f);
    ApplyComponentTint(ExtractionSweepB, FLinearColor(0.08f, 0.95f, 1.0f), 1.35f);
    ApplyComponentTint(ExtractionBeacon, ExtractionAccentColor * 0.74f + FLinearColor::White * 0.26f, 1.8f);
}

void AHelipadActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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

void AHelipadActor::ConfigureExtractionComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->SetVisibility(false, true);
    Component->ComponentTags.AddUnique(FName("ExtractionReadyHelipad"));
    Component->ComponentTags.AddUnique(FName("RescueLoopClosure"));
}
