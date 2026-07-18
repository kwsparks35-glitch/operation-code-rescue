#include "ObjectiveFocusBeaconActor.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueGameInstance.h"
#include "CodingTerminalActor.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr int32 ObjectivePhaseTerminal = 1;
constexpr int32 ObjectivePhaseSurvivor = 2;
constexpr int32 ObjectivePhaseExtraction = 3;
}

AObjectiveFocusBeaconActor::AObjectiveFocusBeaconActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ObjectiveFocusBeaconRoot"));
    SetRootComponent(SceneRoot);

    BaseRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconBaseRing"));
    BaseRing->SetupAttachment(SceneRoot);
    BaseRing->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
    BaseRing->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.045f));

    BeaconColumn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconColumn"));
    BeaconColumn->SetupAttachment(SceneRoot);
    BeaconColumn->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    BeaconColumn->SetRelativeScale3D(FVector(0.20f, 0.20f, 3.2f));

    DirectionArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconDirectionArrow"));
    DirectionArrow->SetupAttachment(SceneRoot);
    DirectionArrow->SetRelativeLocation(FVector(0.0f, -150.0f, 360.0f));
    DirectionArrow->SetRelativeScale3D(FVector(1.7f, 0.16f, 0.12f));

    PulseCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconPulseCore"));
    PulseCore->SetupAttachment(SceneRoot);
    PulseCore->SetRelativeLocation(FVector(0.0f, 0.0f, 440.0f));
    PulseCore->SetRelativeScale3D(FVector(0.38f));

    RadioScanRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusRadioScanRing"));
    RadioScanRing->SetupAttachment(SceneRoot);
    RadioScanRing->SetRelativeLocation(FVector(0.0f, 0.0f, 42.0f));
    RadioScanRing->SetRelativeScale3D(FVector(4.1f, 4.1f, 0.026f));

    RadioSweepArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusRadioSweepArm"));
    RadioSweepArm->SetupAttachment(SceneRoot);
    RadioSweepArm->SetRelativeLocation(FVector(0.0f, 0.0f, 86.0f));
    RadioSweepArm->SetRelativeScale3D(FVector(3.65f, 0.045f, 0.035f));

    RescueBeaconHalo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusRescueBeaconHalo"));
    RescueBeaconHalo->SetupAttachment(SceneRoot);
    RescueBeaconHalo->SetRelativeLocation(FVector(0.0f, 0.0f, 606.0f));
    RescueBeaconHalo->SetRelativeScale3D(FVector(2.15f, 2.15f, 0.034f));

    RadioPingA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusRadioPingA"));
    RadioPingA->SetupAttachment(SceneRoot);
    RadioPingA->SetRelativeLocation(FVector(-220.0f, 150.0f, 118.0f));
    RadioPingA->SetRelativeScale3D(FVector(0.18f));

    RadioPingB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusRadioPingB"));
    RadioPingB->SetupAttachment(SceneRoot);
    RadioPingB->SetRelativeLocation(FVector(220.0f, 150.0f, 118.0f));
    RadioPingB->SetRelativeScale3D(FVector(0.18f));

    StepNodeA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconStepNodeA"));
    StepNodeA->SetupAttachment(SceneRoot);
    StepNodeA->SetRelativeLocation(FVector(-120.0f, 126.0f, 76.0f));
    StepNodeA->SetRelativeScale3D(FVector(0.26f));

    StepNodeB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconStepNodeB"));
    StepNodeB->SetupAttachment(SceneRoot);
    StepNodeB->SetRelativeLocation(FVector(0.0f, 146.0f, 76.0f));
    StepNodeB->SetRelativeScale3D(FVector(0.26f));

    StepNodeC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveFocusBeaconStepNodeC"));
    StepNodeC->SetupAttachment(SceneRoot);
    StepNodeC->SetRelativeLocation(FVector(120.0f, 126.0f, 76.0f));
    StepNodeC->SetRelativeScale3D(FVector(0.26f));

    BeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ObjectiveFocusBeaconLight"));
    BeaconLight->SetupAttachment(SceneRoot);
    BeaconLight->SetRelativeLocation(FVector(0.0f, 0.0f, 290.0f));
    BeaconLight->SetLightColor(TerminalColor);
    BeaconLight->SetIntensity(0.0f);
    BeaconLight->SetAttenuationRadius(2100.0f);
    BeaconLight->SetCastShadows(false);

    ObjectiveLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ObjectiveFocusBeaconLabel"));
    ObjectiveLabel->SetupAttachment(SceneRoot);
    ObjectiveLabel->SetRelativeLocation(FVector(0.0f, -260.0f, 520.0f));
    ObjectiveLabel->SetHorizontalAlignment(EHTA_Center);
    ObjectiveLabel->SetVerticalAlignment(EVRTA_TextCenter);
    ObjectiveLabel->SetWorldSize(86.0f);
    ObjectiveLabel->SetTextRenderColor(FColor::Cyan);
    ObjectiveLabel->SetText(FText::FromString(TEXT("CURRENT OBJECTIVE")));
    ObjectiveLabel->SetVisibility(false, true);

    RadioScanLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ObjectiveFocusRadioScanLabel"));
    RadioScanLabel->SetupAttachment(SceneRoot);
    RadioScanLabel->SetRelativeLocation(FVector(0.0f, 272.0f, 472.0f));
    RadioScanLabel->SetHorizontalAlignment(EHTA_Center);
    RadioScanLabel->SetVerticalAlignment(EVRTA_TextCenter);
    RadioScanLabel->SetWorldSize(52.0f);
    RadioScanLabel->SetTextRenderColor(FColor::Cyan);
    RadioScanLabel->SetText(FText::FromString(TEXT("RADIO SCAN")));
    RadioScanLabel->SetVisibility(false, true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CylinderMesh.Succeeded())
    {
        BaseRing->SetStaticMesh(CylinderMesh.Object);
        BeaconColumn->SetStaticMesh(CylinderMesh.Object);
        RadioScanRing->SetStaticMesh(CylinderMesh.Object);
        RescueBeaconHalo->SetStaticMesh(CylinderMesh.Object);
    }
    if (CubeMesh.Succeeded())
    {
        DirectionArrow->SetStaticMesh(CubeMesh.Object);
        RadioSweepArm->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        PulseCore->SetStaticMesh(SphereMesh.Object);
        RadioPingA->SetStaticMesh(SphereMesh.Object);
        RadioPingB->SetStaticMesh(SphereMesh.Object);
        StepNodeA->SetStaticMesh(SphereMesh.Object);
        StepNodeB->SetStaticMesh(SphereMesh.Object);
        StepNodeC->SetStaticMesh(SphereMesh.Object);
    }

    for (UStaticMeshComponent* Component : { BaseRing, BeaconColumn, DirectionArrow, PulseCore, RadioScanRing, RadioSweepArm, RescueBeaconHalo, RadioPingA, RadioPingB, StepNodeA, StepNodeB, StepNodeC })
    {
        ConfigureBeaconComponent(Component);
    }

    Tags.AddUnique(FName("ObjectiveFocusBeacon"));
    Tags.AddUnique(FName("ObjectiveClarity"));
    Tags.AddUnique(FName("RadioScanRescueBeaconEffects"));
    Tags.AddUnique(FName("RescueBeaconEffects"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("ReleaseDossier"));
}

void AObjectiveFocusBeaconActor::BeginPlay()
{
    Super::BeginPlay();
    SetBeaconVisible(false);
}

void AObjectiveFocusBeaconActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MotionTime += DeltaSeconds;

    const int32 ObjectivePhase = ResolveObjectivePhase();
    if (ObjectivePhase == INDEX_NONE)
    {
        SetBeaconVisible(false);
        return;
    }

    if (ObjectivePhase != CurrentObjectivePhase)
    {
        CurrentObjectivePhase = ObjectivePhase;
        RefreshPhaseVisuals(ObjectivePhase);
        OnObjectiveBeaconPhaseChanged(ObjectivePhase);
    }

    SetBeaconVisible(true);

    const FVector Target = ResolvePhaseTargetLocation(ObjectivePhase);
    const float FollowSpeed = bReducedMotion ? 7.0f : 14.0f;
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), Target, DeltaSeconds, FollowSpeed));

    const float MotionScale = bReducedMotion ? 0.25f : 1.0f;
    const float Pulse = 0.66f + 0.34f * FMath::Sin(MotionTime * (bReducedMotion ? 1.15f : 5.8f));
    const float PhaseBoost = ObjectivePhase == ObjectivePhaseExtraction ? 1.18f : 1.0f;

    if (BaseRing)
    {
        const float RingScale = (2.8f + Pulse * 0.72f) * PhaseBoost;
        BaseRing->SetRelativeScale3D(FVector(RingScale, RingScale, 0.045f));
    }
    if (BeaconColumn)
    {
        BeaconColumn->SetRelativeScale3D(FVector(0.18f + Pulse * 0.055f, 0.18f + Pulse * 0.055f, 3.0f + Pulse * 1.2f));
    }
    if (DirectionArrow)
    {
        DirectionArrow->SetRelativeRotation(FRotator(0.0f, MotionTime * 92.0f * MotionScale, 0.0f));
    }
    if (RadioScanRing)
    {
        const float ScanScale = (4.0f + Pulse * (ObjectivePhase == ObjectivePhaseExtraction ? 1.15f : 0.82f)) * PhaseBoost;
        RadioScanRing->SetRelativeScale3D(FVector(ScanScale, ScanScale, 0.026f));
        RadioScanRing->SetRelativeRotation(FRotator(0.0f, MotionTime * 34.0f * MotionScale, 0.0f));
    }
    if (RadioSweepArm)
    {
        RadioSweepArm->SetRelativeRotation(FRotator(0.0f, MotionTime * (ObjectivePhase == ObjectivePhaseExtraction ? 164.0f : 118.0f) * MotionScale, 0.0f));
    }
    if (RescueBeaconHalo)
    {
        const bool bExtractionPhase = ObjectivePhase == ObjectivePhaseExtraction;
        const float HaloScale = bExtractionPhase ? (2.0f + Pulse * 0.78f) : (1.35f + Pulse * 0.18f);
        RescueBeaconHalo->SetRelativeScale3D(FVector(HaloScale, HaloScale, 0.034f));
        RescueBeaconHalo->SetVisibility(bExtractionPhase, true);
    }
    if (PulseCore)
    {
        const float CoreBob = FMath::Sin(MotionTime * (bReducedMotion ? 0.85f : 2.9f)) * (bReducedMotion ? 4.0f : 24.0f);
        PulseCore->SetRelativeLocation(FVector(0.0f, 0.0f, 440.0f + CoreBob));
        PulseCore->SetRelativeScale3D(FVector((0.32f + Pulse * 0.18f) * PhaseBoost));
    }
    if (RadioPingA)
    {
        const float PingScale = 0.16f + 0.10f * (0.5f + 0.5f * FMath::Sin(MotionTime * (bReducedMotion ? 1.1f : 4.7f)));
        RadioPingA->SetRelativeScale3D(FVector(PingScale));
        RadioPingA->SetVisibility(ObjectivePhase >= ObjectivePhaseSurvivor, true);
    }
    if (RadioPingB)
    {
        const float PingScale = 0.16f + 0.11f * (0.5f + 0.5f * FMath::Sin(MotionTime * (bReducedMotion ? 1.0f : 4.1f) + 1.6f));
        RadioPingB->SetRelativeScale3D(FVector(PingScale));
        RadioPingB->SetVisibility(ObjectivePhase >= ObjectivePhaseExtraction, true);
    }

    const float NodePulse = 0.22f + Pulse * 0.12f;
    if (StepNodeA)
    {
        StepNodeA->SetRelativeScale3D(FVector(NodePulse));
        StepNodeA->SetVisibility(ObjectivePhase >= ObjectivePhaseTerminal, true);
    }
    if (StepNodeB)
    {
        StepNodeB->SetRelativeScale3D(FVector(NodePulse));
        StepNodeB->SetVisibility(ObjectivePhase >= ObjectivePhaseSurvivor, true);
    }
    if (StepNodeC)
    {
        StepNodeC->SetRelativeScale3D(FVector(NodePulse));
        StepNodeC->SetVisibility(ObjectivePhase >= ObjectivePhaseExtraction, true);
    }
    if (BeaconLight)
    {
        BeaconLight->SetIntensity((ObjectivePhase == ObjectivePhaseExtraction ? 11800.0f : 8600.0f) + Pulse * 7200.0f);
    }

    if (ObjectiveLabel)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            const FVector ToPlayer = PlayerPawn->GetActorLocation() - ObjectiveLabel->GetComponentLocation();
            if (!ToPlayer.IsNearlyZero())
            {
                ObjectiveLabel->SetWorldRotation(FRotator(0.0f, ToPlayer.Rotation().Yaw + 180.0f, 0.0f));
            }
        }
    }
    if (RadioScanLabel)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            const FVector ToPlayer = PlayerPawn->GetActorLocation() - RadioScanLabel->GetComponentLocation();
            if (!ToPlayer.IsNearlyZero())
            {
                RadioScanLabel->SetWorldRotation(FRotator(0.0f, ToPlayer.Rotation().Yaw + 180.0f, 0.0f));
            }
        }
    }
}

void AObjectiveFocusBeaconActor::ConfigureObjectiveBeacon(
    int32 InCityIndex,
    const FString& InCityName,
    const FString& InTerminalId,
    const FString& InSurvivorName,
    const FString& InTerminalTitle,
    const FString& InMissionConcept,
    const FString& InLandmarkName,
    const FVector& InEntryLocation,
    const FVector& InTerminalLocation,
    const FVector& InSurvivorLocation,
    const FVector& InExtractionLocation,
    const FLinearColor& InTerminalColor,
    const FLinearColor& InSurvivorColor,
    const FLinearColor& InExtractionColor,
    bool bInReducedMotion)
{
    CityIndex = InCityIndex;
    CityName = InCityName;
    TerminalId = InTerminalId;
    SurvivorName = InSurvivorName;
    TerminalTitle = InTerminalTitle;
    MissionConcept = InMissionConcept;
    LandmarkName = InLandmarkName;
    EntryLocation = InEntryLocation;
    TerminalLocation = InTerminalLocation;
    SurvivorLocation = InSurvivorLocation;
    ExtractionLocation = InExtractionLocation;
    TerminalColor = InTerminalColor;
    SurvivorColor = InSurvivorColor;
    ExtractionColor = InExtractionColor;
    bReducedMotion = bInReducedMotion;

    SetActorLocation(TerminalLocation);
    CurrentObjectivePhase = INDEX_NONE;
    MotionTime = 0.0f;
    SetBeaconVisible(false);
}

int32 AObjectiveFocusBeaconActor::ResolveObjectivePhase()
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        return INDEX_NONE;
    }

    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bSurvivorRescued = GI->RescuedSurvivorNames.Contains(SurvivorName);
    const int32 ActiveCityIndex = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);

    if (ActiveCityIndex == CityIndex)
    {
        if (!bTerminalSolved)
        {
            return ObjectivePhaseTerminal;
        }
        if (!bSurvivorRescued)
        {
            return ObjectivePhaseSurvivor;
        }
        return ObjectivePhaseExtraction;
    }

    if (bTerminalSolved && bSurvivorRescued)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            const float NearbyExtractionSq = FVector::DistSquared2D(PlayerPawn->GetActorLocation(), ExtractionLocation);
            if (NearbyExtractionSq <= FMath::Square(7200.0f))
            {
                return ObjectivePhaseExtraction;
            }
        }
    }

    return INDEX_NONE;
}

FVector AObjectiveFocusBeaconActor::ResolvePhaseTargetLocation(int32 ObjectivePhase) const
{
    switch (ObjectivePhase)
    {
    case ObjectivePhaseTerminal:
        // 2026-07-11: track the actual next-unsolved station (fixed offset
        // stranded the player ~23 m from the final back-corner station).
        return ResolveActiveTerminalTargetLocation() + FVector(0.0f, 0.0f, 32.0f);
    case ObjectivePhaseSurvivor:
        return SurvivorLocation + FVector(0.0f, 0.0f, 32.0f);
    case ObjectivePhaseExtraction:
        return ExtractionLocation + FVector(0.0f, 0.0f, 34.0f);
    default:
        return EntryLocation + FVector(0.0f, 0.0f, 32.0f);
    }
}

FVector AObjectiveFocusBeaconActor::ResolveActiveTerminalTargetLocation() const
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    UWorld* World = GetWorld();
    if (GI && World)
    {
        const FString NextChallengeId = FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(GI, CityIndex);
        if (!NextChallengeId.IsEmpty())
        {
            for (TActorIterator<ACodingTerminalActor> It(World); It; ++It)
            {
                const ACodingTerminalActor* Terminal = *It;
                if (IsValid(Terminal) && !Terminal->bSolved && Terminal->Challenge.Id == NextChallengeId)
                {
                    return Terminal->GetActorLocation();
                }
            }
        }
    }
    return TerminalLocation;
}

FString AObjectiveFocusBeaconActor::BuildPhaseLabel(int32 ObjectivePhase)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const FString LanguageName = GI ? GI->GetLanguageName() : TEXT("coding");
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    switch (ObjectivePhase)
    {
    case ObjectivePhaseTerminal:
        return FString::Printf(
            TEXT("CURRENT OBJECTIVE\n%s CODING CONCOURSE %d/%d\n%s"),
            *LanguageName.ToUpper(),
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity,
            *CityName);
    case ObjectivePhaseSurvivor:
        return FString::Printf(TEXT("CURRENT OBJECTIVE\nSURVIVOR PING: RESCUE SURVIVOR\n%s"), *CityName);
    case ObjectivePhaseExtraction:
        return FString::Printf(TEXT("CURRENT OBJECTIVE\nRESCUE BEACON: EXTRACTION READY\n%s"), *CityName);
    default:
        return FString::Printf(TEXT("CURRENT OBJECTIVE\n%s"), *CityName);
    }
}

FString AObjectiveFocusBeaconActor::BuildRadioScanLine(int32 ObjectivePhase)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const FString LanguageName = GI ? GI->GetLanguageName() : FString(TEXT("selected language"));
    const FString TerminalLabel = TerminalTitle.IsEmpty() ? TerminalId : TerminalTitle;
    const FString ConceptLabel = MissionConcept.IsEmpty() ? FString(TEXT("active coding concept")) : MissionConcept;
    const FString LandmarkLabel = LandmarkName.IsEmpty() ? CityName : LandmarkName;
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);

    switch (ObjectivePhase)
    {
    case ObjectivePhaseTerminal:
        return FString::Printf(
            TEXT("RADIO SCAN\n%s coding clearance %d/%d\n%s"),
            *LanguageName,
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity,
            *TerminalLabel);
    case ObjectivePhaseSurvivor:
        return FString::Printf(TEXT("SURVIVOR PING\n%s\n%s"), *SurvivorName, *ConceptLabel);
    case ObjectivePhaseExtraction:
        return FString::Printf(TEXT("RESCUE BEACON\n%s\nExtraction live"), *LandmarkLabel);
    default:
        return FString::Printf(TEXT("RADIO SCAN\n%s"), *CityName);
    }
}

void AObjectiveFocusBeaconActor::RefreshPhaseVisuals(int32 ObjectivePhase)
{
    CurrentTint = ObjectivePhase == ObjectivePhaseExtraction
        ? ExtractionColor
        : ObjectivePhase == ObjectivePhaseSurvivor
            ? SurvivorColor
            : TerminalColor;

    const FLinearColor SecondaryTint = ObjectivePhase == ObjectivePhaseExtraction
        ? FLinearColor(0.36f, 1.0f, 0.42f)
        : ObjectivePhase == ObjectivePhaseSurvivor
            ? FLinearColor(1.0f, 0.70f, 0.04f)
            : FLinearColor(0.22f, 0.94f, 1.0f);

    ApplyComponentTint(BaseRing, CurrentTint, 1.55f);
    ApplyComponentTint(BeaconColumn, CurrentTint, 1.9f);
    ApplyComponentTint(DirectionArrow, SecondaryTint, 2.1f);
    ApplyComponentTint(PulseCore, CurrentTint * 0.72f + FLinearColor::White * 0.28f, 2.45f);
    ApplyComponentTint(RadioScanRing, CurrentTint * 0.62f + SecondaryTint * 0.38f, 1.85f);
    ApplyComponentTint(RadioSweepArm, SecondaryTint, 2.55f);
    ApplyComponentTint(RescueBeaconHalo, ExtractionColor * 0.76f + FLinearColor::White * 0.24f, 3.2f);
    ApplyComponentTint(RadioPingA, SurvivorColor, 2.15f);
    ApplyComponentTint(RadioPingB, ExtractionColor, 2.35f);
    ApplyComponentTint(StepNodeA, TerminalColor, 1.7f);
    ApplyComponentTint(StepNodeB, SurvivorColor, 1.7f);
    ApplyComponentTint(StepNodeC, ExtractionColor, 1.7f);

    if (BeaconLight)
    {
        BeaconLight->SetLightColor(CurrentTint);
    }
    if (ObjectiveLabel)
    {
        ObjectiveLabel->SetText(FText::FromString(BuildPhaseLabel(ObjectivePhase)));
        ObjectiveLabel->SetTextRenderColor(CurrentTint.ToFColor(true));
    }
    if (RadioScanLabel)
    {
        RadioScanLabel->SetText(FText::FromString(BuildRadioScanLine(ObjectivePhase)));
        RadioScanLabel->SetTextRenderColor(SecondaryTint.ToFColor(true));
    }

    Tags.AddUnique(FName("ObjectiveFocusBeaconActive"));
    Tags.AddUnique(FName("RadioScanRescueBeaconActive"));
    Tags.AddUnique(ObjectivePhase == ObjectivePhaseTerminal ? FName("ObjectiveBeaconTerminalPhase") :
        ObjectivePhase == ObjectivePhaseSurvivor ? FName("ObjectiveBeaconSurvivorPhase") :
        FName("ObjectiveBeaconExtractionPhase"));
    Tags.AddUnique(ObjectivePhase == ObjectivePhaseTerminal ? FName("RadioScanTerminalPhase") :
        ObjectivePhase == ObjectivePhaseSurvivor ? FName("SurvivorPingPhase") :
        FName("RescueBeaconExtractionPhase"));
}

void AObjectiveFocusBeaconActor::SetBeaconVisible(bool bVisible)
{
    if (bBeaconVisible == bVisible)
    {
        return;
    }
    bBeaconVisible = bVisible;

    for (UStaticMeshComponent* Component : { BaseRing, BeaconColumn, DirectionArrow, PulseCore, RadioScanRing, RadioSweepArm, RescueBeaconHalo, RadioPingA, RadioPingB, StepNodeA, StepNodeB, StepNodeC })
    {
        if (Component)
        {
            Component->SetVisibility(bVisible, true);
        }
    }
    if (ObjectiveLabel)
    {
        ObjectiveLabel->SetVisibility(bVisible, true);
    }
    if (RadioScanLabel)
    {
        RadioScanLabel->SetVisibility(bVisible, true);
    }
    if (BeaconLight)
    {
        BeaconLight->SetIntensity(bVisible ? 9200.0f : 0.0f);
    }
}

void AObjectiveFocusBeaconActor::ConfigureBeaconComponent(UStaticMeshComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetMobility(EComponentMobility::Movable);
    Component->SetVisibility(false, true);
    Component->ComponentTags.AddUnique(FName("ObjectiveFocusBeacon"));
    Component->ComponentTags.AddUnique(FName("ObjectiveClarity"));
    Component->ComponentTags.AddUnique(FName("RadioScanRescueBeaconEffects"));
    Component->ComponentTags.AddUnique(FName("WorldDevelopmentDeepDive"));
}

void AObjectiveFocusBeaconActor::ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
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
