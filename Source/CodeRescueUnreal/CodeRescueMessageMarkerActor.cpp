// CodeRescueMessageMarkerActor.cpp -- see header for design intent and the Mac-compile DoD note.

#include "CodeRescueMessageMarkerActor.h"
#include "CodeRescueMessageReaderWidget.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ACodeRescueMessageMarkerActor::ACodeRescueMessageMarkerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MessageMarkerRoot"));
    SetRootComponent(SceneRoot);

    Plate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MessageMarkerPlate"));
    Plate->SetupAttachment(SceneRoot);
    Plate->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    Plate->SetRelativeScale3D(FVector(0.04f, 1.15f, 0.42f));
    Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Plate->SetCastShadow(false);

    IdLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MessageMarkerId"));
    IdLabel->SetupAttachment(SceneRoot);
    IdLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
    IdLabel->SetHorizontalAlignment(EHTA_Center);
    IdLabel->SetVerticalAlignment(EVRTA_TextCenter);
    IdLabel->SetWorldSize(34.0f);
    IdLabel->SetText(FText::FromString(TEXT("MSG")));
    IdLabel->SetTextRenderColor(FColor(90, 216, 255));
    IdLabel->SetCastShadow(false);

    ReadPrompt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MessageMarkerPrompt"));
    ReadPrompt->SetupAttachment(SceneRoot);
    ReadPrompt->SetRelativeLocation(FVector(0.0f, 0.0f, -34.0f));
    ReadPrompt->SetHorizontalAlignment(EHTA_Center);
    ReadPrompt->SetVerticalAlignment(EVRTA_TextCenter);
    ReadPrompt->SetWorldSize(20.0f);
    ReadPrompt->SetText(FText::FromString(TEXT("READ  [E]")));
    ReadPrompt->SetTextRenderColor(FColor(220, 240, 255));
    ReadPrompt->SetCastShadow(false);
    ReadPrompt->SetVisibility(false);

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("MessageMarkerGlow"));
    Glow->SetupAttachment(SceneRoot);
    Glow->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    Glow->SetIntensity(1400.0f);
    Glow->SetAttenuationRadius(650.0f);
    Glow->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Plate->SetStaticMesh(CubeMesh.Object);
    }

    Tags.Add(FName("MessageMarker"));
    Tags.Add(FName("WorldInfoSymbol"));
    Tags.Add(FName("WorldTextDeclutter"));
}

void ACodeRescueMessageMarkerActor::ConfigureMessage(const FString& InMessageId, const FString& InTitle,
                                                     const FString& InBody, const FLinearColor& InAccent, bool bInReducedMotion)
{
    MessageId = InMessageId;
    Title = InTitle;
    Body = InBody;
    Accent = InAccent;
    bReducedMotion = bInReducedMotion;

    if (IdLabel) { IdLabel->SetText(FText::FromString(MessageId)); }
    ApplyAccent();
}

void ACodeRescueMessageMarkerActor::OpenMessageReader()
{
    UCodeRescueMessageReaderWidget::OpenReader(this, MessageId, Title, Body);
}

void ACodeRescueMessageMarkerActor::ApplyAccent()
{
    const FColor C = Accent.ToFColor(true);
    if (IdLabel) { IdLabel->SetTextRenderColor(C); }
    if (Glow)    { Glow->SetLightColor(Accent); }
}

void ACodeRescueMessageMarkerActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    MotionTime += DeltaSeconds;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    const float Dist = Player
        ? FVector::Dist(Player->GetActorLocation(), GetActorLocation())
        : TNumericLimits<float>::Max();

    // 2026-07-16 freeze pass (`sample` profile of Kenny's LA save): the
    // facing turn + bob push a full transform update through the TextRender
    // children EVERY frame, and a late-campaign city carries hundreds of
    // markers — a measurable slice of the stalled frame. Beyond readable
    // range the marker holds still; the beam/plate stay visible.
    bMarkerAnimationCulled = Dist > 7000.0f;
    if (bMarkerAnimationCulled)
    {
        return;
    }

    // Face the local player so the id + prompt stay readable, and show the prompt only when near.
    if (Player)
    {
        const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
        const FRotator Face = ToPlayer.Rotation();
        SetActorRotation(FRotator(0.0f, Face.Yaw, 0.0f));

        const bool bNear = Dist <= PromptRange;
        if (ReadPrompt && ReadPrompt->IsVisible() != bNear)
        {
            ReadPrompt->SetVisibility(bNear);
        }
        if (Glow)
        {
            Glow->SetIntensity(bNear ? 2600.0f : 1200.0f);
        }
    }

    // Gentle bob so the marker reads as an interactive beacon, damped under reduced motion.
    const float Amp = bReducedMotion ? 1.5f : 6.0f;
    const float Speed = bReducedMotion ? 1.0f : 2.4f;
    const float Bob = FMath::Sin(MotionTime * Speed) * Amp;
    if (SceneRoot)
    {
        SceneRoot->SetRelativeLocation(FVector(0.0f, 0.0f, Bob));
    }
}
