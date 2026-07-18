#include "CodeRescueHUDWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "CodeRescueMinimapWidget.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CompanionActor.h"
#include "BossZombieActor.h"
#include "CodeZombieActor.h"
#include "CodingTerminalActor.h"
#include "SurvivorActor.h"
#include "PickupActor.h"
#include "LanguageStationActor.h"
#include "FriendlyNPCActor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Camera/CameraComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace
{
// 2026-07-11 FINAL-STATION FIX (Kenny: "cannot enter the last coding
// challenge; the game keeps directing me to the designated area"): the
// coding-phase objective used a FIXED fully-scaled offset while the ten
// stations are laid out with UNSCALED inner-grid offsets on a scaled hub —
// the marker landed ~23 m from station 10 (worst for the back-corner final
// station), on empty ground. Guidance now targets the ACTUAL next-unsolved
// terminal actor; the fixed offset remains only as a fallback while the city
// is still spawning.
FVector ResolveNextStationWorldTarget(UWorld* World, const FString& NextChallengeId, const FVector& Fallback)
{
    if (World && !NextChallengeId.IsEmpty())
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
    return Fallback;
}

FString DirectionLabelFromPlayer(const ACodeRescueCharacter* Character, const FVector& Target)
{
    if (!Character)
    {
        return TEXT("unknown");
    }

    FVector ToTarget = Target - Character->GetActorLocation();
    ToTarget.Z = 0.0f;
    if (ToTarget.SizeSquared() <= KINDA_SMALL_NUMBER)
    {
        return TEXT("here");
    }

    const FVector TargetDir = ToTarget.GetSafeNormal();
    const float ForwardDot = FVector::DotProduct(Character->GetActorForwardVector(), TargetDir);
    const float RightDot = FVector::DotProduct(Character->GetActorRightVector(), TargetDir);

    if (ForwardDot > 0.72f) return TEXT("ahead");
    if (ForwardDot < -0.72f) return TEXT("behind");
    return RightDot >= 0.0f ? TEXT("right") : TEXT("left");
}

struct FCodeRescueThreatHudInfo
{
    bool bHasThreat = false;
    bool bIsBoss = false;
    bool bIsElite = false;
    bool bStandardPursuit = false;
    float DistanceMeters = -1.0f;
    FString DirectionLabel = TEXT("unknown");
    FString RoleLabel;
    FString VariantLabel = TEXT("infected");
    FString PursuitLabel;
    FString UrgencyLabel = TEXT("TRACKING");
    FLinearColor Accent = FLinearColor(0.92f, 0.84f, 0.62f, 1.0f);
};

constexpr float HeadshotStandardDurationSeconds = 0.75f;
constexpr float HeadshotReducedMotionDurationSeconds = 1.25f;
constexpr float HeadshotBaseY = -88.0f;

FString HudVitalStateLabel(float Fraction)
{
    if (Fraction < 0.25f)
    {
        return TEXT("CRITICAL");
    }
    if (Fraction < 0.55f)
    {
        return TEXT("LOW");
    }
    return TEXT("STABLE");
}

FLinearColor HudHealthFillColor(float HealthPct, const UCodeRescueGameInstance* GI)
{
    const bool bHighContrast = GI && GI->bHighContrastHUD;
    if (HealthPct < 0.25f)
    {
        return CodeRescueUI::Resolve(bHighContrast
            ? FLinearColor(1.0f, 0.08f, 0.03f, 1.0f)
            : CodeRescueUI::Color::DangerBright());
    }
    if (HealthPct < 0.55f)
    {
        return CodeRescueUI::Resolve(bHighContrast
            ? FLinearColor(1.0f, 0.92f, 0.12f, 1.0f)
            : CodeRescueUI::Color::Warning());
    }
    return CodeRescueUI::Resolve(bHighContrast
        ? FLinearColor(0.72f, 1.0f, 0.44f, 1.0f)
        : CodeRescueUI::Color::TerminalGreen());
}

FLinearColor HudHealthLabelColor(float HealthPct, const UCodeRescueGameInstance* GI)
{
    const bool bHighContrast = GI && GI->bHighContrastHUD;
    if (HealthPct < 0.25f)
    {
        return bHighContrast
            ? FLinearColor(1.0f, 0.42f, 0.30f, 1.0f)
            : CodeRescueUI::Color::DangerBright();
    }
    if (HealthPct < 0.55f)
    {
        return bHighContrast
            ? FLinearColor(1.0f, 0.96f, 0.32f, 1.0f)
            : CodeRescueUI::Color::Warning();
    }
    return bHighContrast
        ? FLinearColor(0.86f, 1.0f, 0.62f, 1.0f)
        : CodeRescueUI::Color::TerminalGreenBright();
}

FLinearColor HudStaminaFillColor(float StaminaPct, const UCodeRescueGameInstance* GI)
{
    const bool bHighContrast = GI && GI->bHighContrastHUD;
    if (StaminaPct < 0.25f)
    {
        return CodeRescueUI::Resolve(bHighContrast
            ? FLinearColor(1.0f, 0.48f, 0.12f, 1.0f)
            : CodeRescueUI::Color::AccentEmber());
    }
    if (StaminaPct < 0.55f)
    {
        return CodeRescueUI::Resolve(bHighContrast
            ? FLinearColor(1.0f, 0.90f, 0.22f, 1.0f)
            : CodeRescueUI::Color::AccentAmber());
    }
    return CodeRescueUI::Resolve(bHighContrast
        ? FLinearColor(0.50f, 0.86f, 1.0f, 1.0f)
        : CodeRescueUI::Color::Stamina());
}

FString ZombieVariantHudLabel(EZombieVariant Variant)
{
    switch (Variant)
    {
    case EZombieVariant::DogZombie: return TEXT("dog infected");
    case EZombieVariant::UrbanZombie4: return TEXT("urban infected");
    case EZombieVariant::BusinessSuit: return TEXT("business infected");
    case EZombieVariant::BloatedFemale: return TEXT("bloated infected");
    case EZombieVariant::NurseFemale: return TEXT("nurse infected");
    case EZombieVariant::BaseMesh: return TEXT("base-mesh infected");
    case EZombieVariant::EliteSpitter: return TEXT("spitter");
    case EZombieVariant::EliteCharger: return TEXT("charger");
    case EZombieVariant::EliteBoomer: return TEXT("boomer");
    default: return TEXT("infected");
    }
}

FString ZombieRoleHudLabel(ECodeRescueZombieEncounterRole Role)
{
    switch (Role)
    {
    case ECodeRescueZombieEncounterRole::Anchor: return TEXT("anchor ");
    case ECodeRescueZombieEncounterRole::Flanker: return TEXT("flanker ");
    case ECodeRescueZombieEncounterRole::Pressure: return TEXT("pressure ");
    case ECodeRescueZombieEncounterRole::Sentinel: return TEXT("sentinel ");
    default: return FString();
    }
}

bool IsEliteZombieVariant(EZombieVariant Variant)
{
    return Variant == EZombieVariant::EliteSpitter
        || Variant == EZombieVariant::EliteCharger
        || Variant == EZombieVariant::EliteBoomer;
}

void FinalizeThreatHudInfo(FCodeRescueThreatHudInfo& Info)
{
    if (!Info.bHasThreat)
    {
        return;
    }

    if (Info.DistanceMeters <= 8.0f)
    {
        Info.UrgencyLabel = TEXT("MELEE");
        Info.Accent = FLinearColor(1.0f, 0.18f, 0.12f, 1.0f);
    }
    else if (Info.DistanceMeters <= 18.0f)
    {
        Info.UrgencyLabel = TEXT("CLOSE");
        Info.Accent = FLinearColor(0.98f, 0.38f, 0.16f, 1.0f);
    }
    else if (Info.bIsBoss && Info.DistanceMeters <= 55.0f)
    {
        Info.UrgencyLabel = TEXT("BOSS");
        Info.Accent = FLinearColor(1.0f, 0.30f, 0.12f, 1.0f);
    }
    else if (Info.bIsElite && Info.DistanceMeters <= 42.0f)
    {
        Info.UrgencyLabel = TEXT("ELITE");
        Info.Accent = FLinearColor(1.0f, 0.62f, 0.20f, 1.0f);
    }
    else if (Info.bStandardPursuit && Info.DistanceMeters <= 28.0f)
    {
        Info.UrgencyLabel = TEXT("PURSUIT");
        Info.Accent = FLinearColor(1.0f, 0.52f, 0.22f, 1.0f);
    }
    else
    {
        Info.UrgencyLabel = TEXT("TRACKING");
        Info.Accent = FLinearColor(0.92f, 0.84f, 0.62f, 1.0f);
    }
}

FCodeRescueThreatHudInfo GetNearestHudThreat(UWorld* World, const ACodeRescueCharacter* Character)
{
    FCodeRescueThreatHudInfo Result;
    if (!World || !Character)
    {
        return Result;
    }

    const FVector PlayerLocation = Character->GetActorLocation();
    float BestDistanceSq = TNumericLimits<float>::Max();

    auto ConsiderThreat = [&](const ACodeZombieActor* Threat, bool bBossThreat)
    {
        if (!IsValid(Threat) || Threat->Health <= 0.0f)
        {
            return;
        }

        const float DistanceSq = FVector::DistSquared(PlayerLocation, Threat->GetActorLocation());
        const bool bPriorityBoss = bBossThreat && (!Result.bIsBoss || DistanceSq < BestDistanceSq * 1.35f);
        if (DistanceSq >= BestDistanceSq && !bPriorityBoss)
        {
            return;
        }

        BestDistanceSq = DistanceSq;
        Result.bHasThreat = true;
        Result.bIsBoss = bBossThreat;
        Result.bIsElite = bBossThreat || IsEliteZombieVariant(Threat->Variant);
        const float DistanceUU = FMath::Sqrt(DistanceSq);
        Result.DistanceMeters = DistanceUU / 100.0f;
        Result.DirectionLabel = DirectionLabelFromPlayer(Character, Threat->GetActorLocation());
        Result.RoleLabel = bBossThreat ? TEXT("boss ") : ZombieRoleHudLabel(Threat->EncounterRole);
        Result.VariantLabel = bBossThreat ? TEXT("boss infected") : ZombieVariantHudLabel(Threat->Variant);
        Result.bStandardPursuit = !bBossThreat && Threat->bStandardDirectPursuitEnabled;
        if (Result.bStandardPursuit)
        {
            if (Result.RoleLabel.IsEmpty())
            {
                Result.RoleLabel = TEXT("standard ");
            }
            Result.PursuitLabel = Threat->GetStandardPursuitStateSummary(DistanceUU);
        }
    };

    for (TActorIterator<ABossZombieActor> It(World); It; ++It)
    {
        ConsiderThreat(*It, true);
    }

    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        const ACodeZombieActor* Zombie = *It;
        if (Cast<ABossZombieActor>(Zombie))
        {
            continue;
        }
        ConsiderThreat(Zombie, false);
    }

    FinalizeThreatHudInfo(Result);
    return Result;
}

float GetNearestLivingBossDistanceMeters(UWorld* World, const FVector& PlayerLocation)
{
    if (!World)
    {
        return -1.0f;
    }

    float BestDistanceSq = TNumericLimits<float>::Max();
    for (TActorIterator<ABossZombieActor> It(World); It; ++It)
    {
        const ABossZombieActor* Boss = *It;
        if (!IsValid(Boss) || Boss->Health <= 0.0f)
        {
            continue;
        }

        BestDistanceSq = FMath::Min(BestDistanceSq, FVector::DistSquared(PlayerLocation, Boss->GetActorLocation()));
    }

    return BestDistanceSq < TNumericLimits<float>::Max()
        ? FMath::Sqrt(BestDistanceSq) / 100.0f
        : -1.0f;
}

const FCodeRescueMissionProgress* FindMissionProgress(const UCodeRescueGameInstance* GI, const FString& MissionId)
{
    if (!GI)
    {
        return nullptr;
    }

    for (const FCodeRescueMissionProgress& Progress : GI->MissionProgress)
    {
        if (Progress.MissionId == MissionId)
        {
            return &Progress;
        }
    }
    return nullptr;
}
}

void UCodeRescueHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueHUDWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueHUDWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;

    // Sync the shared UI design system with saved accessibility settings so the
    // HUD (and any themed widget opened during play) honors high contrast,
    // reduced motion, and text scaling from the first frame.
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUDRoot"));
    WidgetTree->RootWidget = Root;

    UBorder* TopVignette = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeSurvivalHUDTopVignette"));
    TopVignette->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.28f));
    TopVignetteBorder = TopVignette;
    UCanvasPanelSlot* TopVignetteSlot = Root->AddChildToCanvas(TopVignette);
    TopVignetteSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 0.0f));
    TopVignetteSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 235.0f));

    UBorder* BottomVignette = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeSurvivalHUDBottomVignette"));
    BottomVignette->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.34f));
    BottomVignetteBorder = BottomVignette;
    UCanvasPanelSlot* BottomVignetteSlot = Root->AddChildToCanvas(BottomVignette);
    BottomVignetteSlot->SetAnchors(FAnchors(0.0f, 1.0f, 1.0f, 1.0f));
    BottomVignetteSlot->SetOffsets(FMargin(0.0f, -142.0f, 0.0f, 0.0f));

    UBorder* StatusPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeSurvivalHUDStatusPanel"));
    StatusPanel->SetBrushColor(FLinearColor(0.030f, 0.028f, 0.023f, 0.78f));
    StatusPanelBorder = StatusPanel;
    UCanvasPanelSlot* StatusPanelSlot = Root->AddChildToCanvas(StatusPanel);
    StatusPanelSlot->SetAnchors(FAnchors(0, 0, 0, 0));
    StatusPanelSlot->SetPosition(FVector2D(16, 16));
    StatusPanelSlot->SetSize(FVector2D(1040, 170));

    StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
    StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.84f, 0.72f, 1.0f)));
    // Stronger drop shadow + larger font so the status line stays readable
    // over bright sky, dark streets, or busy geometry alike.
    StatusText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    StatusText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
    {
        FSlateFontInfo StatusFont = StatusText->GetFont();
        StatusFont.Size = 20;
        StatusText->SetFont(StatusFont);
    }
    StatusText->SetText(FText::FromString(TEXT("Operation Code Rescue")));
    UCanvasPanelSlot* StatusSlot = Root->AddChildToCanvas(StatusText);
    StatusSlot->SetAnchors(FAnchors(0, 0, 0, 0));
    StatusSlot->SetPosition(FVector2D(24, 24));
    StatusSlot->SetSize(FVector2D(1200, 190));

    UBorder* ObjectivePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeSurvivalHUDObjectivePanel"));
    ObjectivePanel->SetBrushColor(FLinearColor(0.050f, 0.040f, 0.028f, 0.72f));
    ObjectivePanelBorder = ObjectivePanel;
    UCanvasPanelSlot* ObjectivePanelSlot = Root->AddChildToCanvas(ObjectivePanel);
    ObjectivePanelSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
    ObjectivePanelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    ObjectivePanelSlot->SetPosition(FVector2D(0.0f, 204.0f));
    ObjectivePanelSlot->SetSize(FVector2D(790.0f, 76.0f));

    ObjectiveFocusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ObjectiveFocusText"));
    ObjectiveFocusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.78f, 0.46f, 1.0f)));
    ObjectiveFocusText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ObjectiveFocusText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
    {
        FSlateFontInfo ObjectiveFont = ObjectiveFocusText->GetFont();
        ObjectiveFont.Size = 24;
        ObjectiveFocusText->SetFont(ObjectiveFont);
    }
    ObjectiveFocusText->SetText(FText::FromString(TEXT("")));
    UCanvasPanelSlot* ObjectiveSlot = Root->AddChildToCanvas(ObjectiveFocusText);
    ObjectiveSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
    ObjectiveSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    ObjectiveSlot->SetPosition(FVector2D(0.0f, 210.0f));
    ObjectiveSlot->SetSize(FVector2D(760.0f, 68.0f));

    ThreatCompassText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ThreatCompassText"));
    ThreatCompassText->SetText(FText::FromString(TEXT("THREAT COMPASS  clear")));
    ThreatCompassText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.84f, 0.62f, 1.0f)));
    ThreatCompassText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    ThreatCompassText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
    {
        FSlateFontInfo ThreatFont = ThreatCompassText->GetFont();
        ThreatFont.Size = 18;
        ThreatCompassText->SetFont(ThreatFont);
    }
    UCanvasPanelSlot* ThreatCompassSlot = Root->AddChildToCanvas(ThreatCompassText);
    ThreatCompassSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
    ThreatCompassSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    ThreatCompassSlot->SetPosition(FVector2D(0.0f, 286.0f));
    ThreatCompassSlot->SetSize(FVector2D(760.0f, 32.0f));

    SoundCueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VisualizedSoundCueText"));
    SoundCueText->SetText(FText::FromString(TEXT("")));
    SoundCueText->SetAutoWrapText(true);
    SoundCueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.94f, 1.0f, 1.0f)));
    SoundCueText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    SoundCueText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
    {
        FSlateFontInfo SoundCueFont = SoundCueText->GetFont();
        SoundCueFont.Size = 16;
        SoundCueText->SetFont(SoundCueFont);
    }
    UCanvasPanelSlot* SoundCueSlot = Root->AddChildToCanvas(SoundCueText);
    SoundCueSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
    SoundCueSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    SoundCueSlot->SetPosition(FVector2D(0.0f, 322.0f));
    SoundCueSlot->SetSize(FVector2D(680.0f, 42.0f));

    ObjectiveToastText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ObjectiveRouteToastText"));
    ObjectiveToastText->SetText(FText::FromString(TEXT("")));
    ObjectiveToastText->SetAutoWrapText(true);
    ObjectiveToastText->SetJustification(ETextJustify::Center);
    ObjectiveToastText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.96f, 1.0f, 0.0f)));
    ObjectiveToastText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ObjectiveToastText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.94f));
    CodeRescueUI::StyleText(ObjectiveToastText, CodeRescueUI::EType::Subheading, FLinearColor(0.78f, 0.96f, 1.0f, 1.0f));
    UCanvasPanelSlot* ToastSlot = Root->AddChildToCanvas(ObjectiveToastText);
    ToastSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    ToastSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    ToastSlot->SetPosition(FVector2D(0.0f, -226.0f));
    ToastSlot->SetSize(FVector2D(780.0f, 54.0f));

    CrosshairText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Crosshair"));
    CrosshairText->SetText(FText::FromString(TEXT("+")));
    CrosshairText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.96f, 0.68f, 1.0f)));
    // Larger, shadowed crosshair so aim is clear against any background.
    {
        FSlateFontInfo CrossFont = CrosshairText->GetFont();
        CrossFont.Size = 34;
        CrosshairText->SetFont(CrossFont);
    }
    CrosshairText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    CrosshairText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
    UCanvasPanelSlot* CrossSlot = Root->AddChildToCanvas(CrosshairText);
    CrossSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    CrossSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    CrossSlot->SetPosition(FVector2D(0, 0));
    CrossSlot->SetSize(FVector2D(64, 64));

    AimLockText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AimLockText"));
    AimLockText->SetText(FText::GetEmpty());
    AimLockText->SetJustification(ETextJustify::Center);
    AimLockText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.72f, 0.20f, 1.0f)));
    AimLockText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    AimLockText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.94f));
    {
        FSlateFontInfo LockFont = AimLockText->GetFont();
        LockFont.Size = 16;
        AimLockText->SetFont(LockFont);
    }
    UCanvasPanelSlot* AimLockSlot = Root->AddChildToCanvas(AimLockText);
    AimLockSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    AimLockSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    AimLockSlot->SetPosition(FVector2D(0.0f, 46.0f));
    AimLockSlot->SetSize(FVector2D(420.0f, 30.0f));

    InteractionPromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InteractPrompt"));
    InteractionPromptText->SetText(FText::FromString(TEXT("")));
    InteractionPromptText->SetColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.82f, 0.48f, 1.0f)));
    InteractionPromptText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    InteractionPromptText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
    {
        FSlateFontInfo PromptFont = InteractionPromptText->GetFont();
        PromptFont.Size = 22;
        InteractionPromptText->SetFont(PromptFont);
    }
    UCanvasPanelSlot* PromptSlot = Root->AddChildToCanvas(InteractionPromptText);
    PromptSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    PromptSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    // Slightly below the crosshair.
    PromptSlot->SetPosition(FVector2D(0, 36));
    PromptSlot->SetSize(FVector2D(420, 36));

    HeadshotFeedbackText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HeadshotFeedback"));
    HeadshotFeedbackText->SetText(FText::FromString(TEXT("")));
    HeadshotFeedbackText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.64f, 0.34f, 1.0f)));
    HeadshotFeedbackText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    HeadshotFeedbackText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
    {
        FSlateFontInfo HeadshotFont = HeadshotFeedbackText->GetFont();
        HeadshotFont.Size = 30;
        HeadshotFeedbackText->SetFont(HeadshotFont);
    }
    HeadshotFeedbackSlot = Root->AddChildToCanvas(HeadshotFeedbackText);
    HeadshotFeedbackSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    HeadshotFeedbackSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    HeadshotFeedbackSlot->SetPosition(FVector2D(0.0f, -88.0f));
    HeadshotFeedbackSlot->SetSize(FVector2D(420.0f, 46.0f));

    // #6 — Minimap, top-right corner. CreateWidget() takes the world so the
    // child widget shares our outer game instance for actor iteration.
    Minimap = CreateWidget<UCodeRescueMinimapWidget>(GetWorld(), UCodeRescueMinimapWidget::StaticClass());
    if (Minimap)
    {
        UCanvasPanelSlot* MapSlot = Root->AddChildToCanvas(Minimap);
        MapSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        MapSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        MapSlot->SetPosition(FVector2D(-12.0f, 12.0f));
        MapSlot->SetSize(FVector2D(220.0f, 220.0f));
    }

    UBorder* NavigationPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeNavigationPanel"));
    NavigationPanel->SetBrushColor(FLinearColor(0.015f, 0.040f, 0.050f, 0.84f));
    {
        UCanvasPanelSlot* NavPanelSlot = Root->AddChildToCanvas(NavigationPanel);
        NavPanelSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        NavPanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        NavPanelSlot->SetPosition(FVector2D(-12.0f, 238.0f));
        NavPanelSlot->SetSize(FVector2D(330.0f, 96.0f));
    }

    NavigationStripText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NavigationStripText"));
    NavigationStripText->SetText(FText::FromString(TEXT("NAVIGATION")));
    NavigationStripText->SetAutoWrapText(true);
    NavigationStripText->SetColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.96f, 1.0f, 1.0f)));
    NavigationStripText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    NavigationStripText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
    {
        FSlateFontInfo NavFont = NavigationStripText->GetFont();
        NavFont.Size = 17;
        NavigationStripText->SetFont(NavFont);
        UCanvasPanelSlot* NavSlot = Root->AddChildToCanvas(NavigationStripText);
        NavSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        NavSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        NavSlot->SetPosition(FVector2D(-22.0f, 246.0f));
        NavSlot->SetSize(FVector2D(310.0f, 84.0f));
    }

    UBorder* FieldChecklistPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeFieldChecklistPanel"));
    FieldChecklistPanel->SetBrushColor(FLinearColor(0.018f, 0.034f, 0.032f, 0.82f));
    {
        UCanvasPanelSlot* ChecklistPanelSlot = Root->AddChildToCanvas(FieldChecklistPanel);
        ChecklistPanelSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        ChecklistPanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        ChecklistPanelSlot->SetPosition(FVector2D(-12.0f, 342.0f));
        ChecklistPanelSlot->SetSize(FVector2D(330.0f, 132.0f));
    }

    FieldChecklistText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(),
        TEXT("FirstTenMinutesFieldChecklistText"));
    FieldChecklistText->SetText(FText::FromString(TEXT("FIRST TEN MINUTES FIELD CHECKLIST")));
    FieldChecklistText->SetAutoWrapText(true);
    FieldChecklistText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 1.0f, 0.80f, 1.0f)));
    FieldChecklistText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    FieldChecklistText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.94f));
    {
        FSlateFontInfo ChecklistFont = FieldChecklistText->GetFont();
        ChecklistFont.Size = 14;
        FieldChecklistText->SetFont(ChecklistFont);
        UCanvasPanelSlot* ChecklistSlot = Root->AddChildToCanvas(FieldChecklistText);
        ChecklistSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        ChecklistSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        ChecklistSlot->SetPosition(FVector2D(-22.0f, 350.0f));
        ChecklistSlot->SetSize(FVector2D(310.0f, 116.0f));
    }

    UBorder* BottomReadoutPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeSurvivalHUDBottomReadoutPanel"));
    BottomReadoutPanel->SetBrushColor(FLinearColor(0.030f, 0.028f, 0.023f, 0.76f));
    {
        UCanvasPanelSlot* ReadoutPanelSlot = Root->AddChildToCanvas(BottomReadoutPanel);
        ReadoutPanelSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        ReadoutPanelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        ReadoutPanelSlot->SetPosition(FVector2D(16.0f, -22.0f));
        ReadoutPanelSlot->SetSize(FVector2D(1180.0f, 152.0f));
    }

    // #42 — second-line readout for new mechanics, anchored bottom-left.
    SecondLineText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SecondLineText"));
    SecondLineText->SetText(FText::FromString(TEXT("")));
    SecondLineText->SetAutoWrapText(true);
    SecondLineText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.84f, 0.74f, 1.0f)));
    SecondLineText->SetShadowOffset(FVector2D(1, 1));
    {
        UCanvasPanelSlot* SlSlot = Root->AddChildToCanvas(SecondLineText);
        SlSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        SlSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        SlSlot->SetPosition(FVector2D(24.0f, -24.0f));
        SlSlot->SetSize(FVector2D(1140.0f, 54.0f));
    }

    HealthLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayerHealthLabel"));
    HealthLabelText->SetText(FText::FromString(TEXT("PLAYER HEALTH")));
    CodeRescueUI::StyleText(
        HealthLabelText,
        CodeRescueUI::EType::Subheading,
        HudHealthLabelColor(1.0f, GetGameInstance<UCodeRescueGameInstance>()));
    {
        UCanvasPanelSlot* HealthLabelSlot = Root->AddChildToCanvas(HealthLabelText);
        HealthLabelSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        HealthLabelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        HealthLabelSlot->SetPosition(FVector2D(24.0f, -146.0f));
        HealthLabelSlot->SetSize(FVector2D(640.0f, 36.0f));
    }

    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(HudHealthFillColor(1.0f, GetGameInstance<UCodeRescueGameInstance>()));
    {
        UCanvasPanelSlot* HealthSlot = Root->AddChildToCanvas(HealthBar);
        HealthSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        HealthSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        HealthSlot->SetPosition(FVector2D(24.0f, -120.0f));
        HealthSlot->SetSize(FVector2D(360.0f, 24.0f));
    }

    StaminaBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("StaminaBar"));
    StaminaBar->SetPercent(1.0f);
    StaminaBar->SetFillColorAndOpacity(HudStaminaFillColor(1.0f, GetGameInstance<UCodeRescueGameInstance>()));
    {
        UCanvasPanelSlot* StaminaSlot = Root->AddChildToCanvas(StaminaBar);
        StaminaSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        StaminaSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        StaminaSlot->SetPosition(FVector2D(24.0f, -88.0f));
        StaminaSlot->SetSize(FVector2D(360.0f, 14.0f));
    }

    TacticalReadoutText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TacticalReadoutText"));
    TacticalReadoutText->SetText(FText::FromString(TEXT("")));
    TacticalReadoutText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.84f, 0.62f, 1.0f)));
    TacticalReadoutText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    TacticalReadoutText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f));
    {
        FSlateFontInfo TacticalFont = TacticalReadoutText->GetFont();
        TacticalFont.Size = 18;
        TacticalReadoutText->SetFont(TacticalFont);
        UCanvasPanelSlot* TacticalSlot = Root->AddChildToCanvas(TacticalReadoutText);
        TacticalSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        TacticalSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        TacticalSlot->SetPosition(FVector2D(402.0f, -78.0f));
        TacticalSlot->SetSize(FVector2D(720.0f, 36.0f));
    }

    WeaponStripText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponStripText"));
    WeaponStripText->SetText(FText::FromString(TEXT("WEAPON SLOT")));
    WeaponStripText->SetAutoWrapText(true);
    WeaponStripText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.88f, 0.56f, 1.0f)));
    WeaponStripText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    WeaponStripText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
    {
        FSlateFontInfo WeaponFont = WeaponStripText->GetFont();
        WeaponFont.Size = 13;
        WeaponStripText->SetFont(WeaponFont);
        UCanvasPanelSlot* WeaponSlot = Root->AddChildToCanvas(WeaponStripText);
        WeaponSlot->SetAnchors(FAnchors(0.0f, 1.0f, 1.0f, 1.0f));
        WeaponSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        WeaponSlot->SetPosition(FVector2D(402.0f, -188.0f));
        WeaponSlot->SetSize(FVector2D(860.0f, 52.0f));
    }

    ReloadStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReloadStatusText"));
    ReloadStatusText->SetText(FText::FromString(TEXT("")));
    ReloadStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.58f, 0.30f, 1.0f)));
    ReloadStatusText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    ReloadStatusText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f));
    {
        FSlateFontInfo ReloadFont = ReloadStatusText->GetFont();
        ReloadFont.Size = 20;
        ReloadStatusText->SetFont(ReloadFont);
        UCanvasPanelSlot* ReloadSlot = Root->AddChildToCanvas(ReloadStatusText);
        ReloadSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        ReloadSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        ReloadSlot->SetPosition(FVector2D(402.0f, -116.0f));
        ReloadSlot->SetSize(FVector2D(760.0f, 34.0f));
    }

    SquadStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SquadStatusText"));
    SquadStatusText->SetText(FText::FromString(TEXT("")));
    SquadStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.92f, 1.0f, 1.0f)));
    SquadStatusText->SetShadowOffset(FVector2D(1.5f, 1.5f));
    SquadStatusText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
    SquadStatusText->SetAutoWrapText(true);
    {
        FSlateFontInfo SquadFont = SquadStatusText->GetFont();
        SquadFont.Size = 15;
        SquadStatusText->SetFont(SquadFont);
        UCanvasPanelSlot* SquadSlot = Root->AddChildToCanvas(SquadStatusText);
        SquadSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        SquadSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        SquadSlot->SetPosition(FVector2D(402.0f, -146.0f));
        SquadSlot->SetSize(FVector2D(860.0f, 48.0f));
    }

    DamageAlertText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DamageAlertText"));
    DamageAlertText->SetText(FText::FromString(TEXT("")));
    DamageAlertText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.30f, 0.18f, 1.0f)));
    DamageAlertText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    DamageAlertText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
    {
        FSlateFontInfo DamageFont = DamageAlertText->GetFont();
        DamageFont.Size = 24;
        DamageAlertText->SetFont(DamageFont);
        UCanvasPanelSlot* DamageSlot = Root->AddChildToCanvas(DamageAlertText);
        DamageSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
        DamageSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        DamageSlot->SetPosition(FVector2D(0.0f, -132.0f));
        DamageSlot->SetSize(FVector2D(780.0f, 42.0f));
    }

    // #44 — subtitles overlay. Mounted as a child so it shares HUD lifetime.
    if (UCodeRescueSubtitlesWidget* Subs = CreateWidget<UCodeRescueSubtitlesWidget>(GetWorld(), UCodeRescueSubtitlesWidget::StaticClass()))
    {
        UCanvasPanelSlot* SubSlot = Root->AddChildToCanvas(Subs);
        SubSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        SubSlot->SetOffsets(FMargin(0));
    }

    // #19 wiring — autosave pip just below the minimap.
    AutosaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AutosaveText"));
    AutosaveText->SetText(FText::FromString(TEXT("")));
    AutosaveText->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 1.0f, 0.4f, 0.9f)));
    AutosaveText->SetShadowOffset(FVector2D(1, 1));
    {
        UCanvasPanelSlot* ASlot = Root->AddChildToCanvas(AutosaveText);
        ASlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        ASlot->SetAlignment(FVector2D(1.0f, 0.0f));
        ASlot->SetPosition(FVector2D(-12.0f, 240.0f));
        ASlot->SetSize(FVector2D(220.0f, 28.0f));
    }

    // ---- 2026-07-01 HUD diet (Kenny): only the absolute necessities stay on screen.
    // Everything collapsed here remains fully populated and is surfaced through the
    // objective journal (J) and pause menu (P) instead of competing with play.
    if (bMinimalHUD)
    {
        for (UWidget* Verbose : TArray<UWidget*>{ FieldChecklistText, ThreatCompassText,
             SoundCueText, SecondLineText, TacticalReadoutText, SquadStatusText,
             WeaponStripText, ReloadStatusText, NavigationStripText, ObjectiveToastText,
             AutosaveText })
        {
            if (Verbose)
            {
                Verbose->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
        if (Minimap)
        {
            Minimap->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UCodeRescueHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    const bool bModalUIOwnsInput = ACodeRescueCharacter::IsUIOpen();
    SetRenderOpacity(bModalUIOwnsInput ? 0.0f : 1.0f);
    if (bModalUIOwnsInput)
    {
        return;
    }
    RefreshHUD();
}

int32 UCodeRescueHUDWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
        OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    const ACodeRescueCharacter* Character =
        Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Character || !Character->IsScopeViewActive())
    {
        return MaxLayer;
    }

    // ---- full-screen circular scope (Kenny's sighting reference video) ----
    // The blackout is FOUR solid boxes surrounding the scope's bounding
    // square plus ONE modest annulus that rounds the square hole into a
    // circle. (Cycle-16 lesson: a single mega-thick polyline ring leaves
    // wedge gaps between segments — the screen showed radial streaks — and
    // its chord-flat inner edges shrink the glass.)
    const FVector2D Size = AllottedGeometry.GetLocalSize();
    if (Size.X < 8.0f || Size.Y < 8.0f)
    {
        return MaxLayer;
    }
    const FVector2D Center = Size * 0.5f;
    const float ScopeRadius = FMath::Min(Size.X, Size.Y) * 0.44f;
    const int32 ScopeLayer = MaxLayer + 1;
    const FSlateBrush* FillBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    const FLinearColor MaskColor(0.0f, 0.0f, 0.0f, 1.0f);
    const float BoxEdge = ScopeRadius * 1.60f;   // boxes start inside the outer band overlap
    auto DrawMaskBox = [&](const FVector2D& TopLeft, const FVector2D& BoxSize)
    {
        if (BoxSize.X > 0.5f && BoxSize.Y > 0.5f)
        {
            FSlateDrawElement::MakeBox(OutDrawElements, ScopeLayer,
                AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(TopLeft)),
                FillBrush, ESlateDrawEffect::None, MaskColor);
        }
    };
    DrawMaskBox(FVector2D(0.0f, 0.0f), FVector2D(Center.X - BoxEdge, Size.Y));                          // left
    DrawMaskBox(FVector2D(Center.X + BoxEdge, 0.0f), FVector2D(Size.X - Center.X - BoxEdge, Size.Y));   // right
    DrawMaskBox(FVector2D(Center.X - BoxEdge, 0.0f), FVector2D(BoxEdge * 2.0f, Center.Y - BoxEdge));    // top
    DrawMaskBox(FVector2D(Center.X - BoxEdge, Center.Y + BoxEdge), FVector2D(BoxEdge * 2.0f, Size.Y - Center.Y - BoxEdge)); // bottom

    // Cycle-18: a single mega-thick NON-antialiased polyline renders as
    // disjoint per-segment quads (radial streak gaps). Overlapping THIN
    // antialiased bands join cleanly and tile the ring zone solid.
    const int32 SegmentCount = 160;
    const float BandThickness = ScopeRadius * 0.24f;
    for (float BandRadiusScale : { 1.10f, 1.28f, 1.46f, 1.64f, 1.80f })
    {
        const float BandRadius = ScopeRadius * BandRadiusScale;
        TArray<FVector2D> Ring;
        Ring.Reserve(SegmentCount + 3);
        // overshoot two segments past the wrap so the start/end joint has no
        // seam (cycle-19: a dashed sliver showed on the +X axis)
        for (int32 Index = 0; Index <= SegmentCount + 2; ++Index)
        {
            const float Angle = (2.0f * PI * Index) / SegmentCount;
            Ring.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * BandRadius);
        }
        FSlateDrawElement::MakeLines(OutDrawElements, ScopeLayer,
            AllottedGeometry.ToPaintGeometry(), Ring, ESlateDrawEffect::None,
            MaskColor, true, BandThickness);
    }

    // thin luminous rim so the glass edge reads
    TArray<FVector2D> Rim;
    Rim.Reserve(SegmentCount + 1);
    for (int32 Index = 0; Index <= SegmentCount; ++Index)
    {
        const float Angle = (2.0f * PI * Index) / SegmentCount;
        Rim.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * ScopeRadius);
    }
    FSlateDrawElement::MakeLines(OutDrawElements, ScopeLayer + 1,
        AllottedGeometry.ToPaintGeometry(), Rim, ESlateDrawEffect::None,
        FLinearColor(0.62f, 0.70f, 0.78f, 0.9f), true, 3.0f);

    // fine crosshair spanning the glass + mil dots down each axis
    const FLinearColor ReticleColor(0.92f, 0.96f, 1.0f, 0.92f);
    const TArray<FVector2D> HairH = {
        Center - FVector2D(ScopeRadius, 0.0f), Center + FVector2D(ScopeRadius, 0.0f) };
    const TArray<FVector2D> HairV = {
        Center - FVector2D(0.0f, ScopeRadius), Center + FVector2D(0.0f, ScopeRadius) };
    FSlateDrawElement::MakeLines(OutDrawElements, ScopeLayer + 2,
        AllottedGeometry.ToPaintGeometry(), HairH, ESlateDrawEffect::None, ReticleColor, true, 1.4f);
    FSlateDrawElement::MakeLines(OutDrawElements, ScopeLayer + 2,
        AllottedGeometry.ToPaintGeometry(), HairV, ESlateDrawEffect::None, ReticleColor, true, 1.4f);
    for (int32 Dot = 1; Dot <= 4; ++Dot)
    {
        const float Offset = ScopeRadius * 0.2f * Dot;
        for (const FVector2D& Axis : { FVector2D(1.0f, 0.0f), FVector2D(-1.0f, 0.0f),
                                       FVector2D(0.0f, 1.0f), FVector2D(0.0f, -1.0f) })
        {
            const FVector2D DotCenter = Center + Axis * Offset;
            const FVector2D Perp(Axis.Y, Axis.X);
            const TArray<FVector2D> Tick = { DotCenter - Perp * 4.0f, DotCenter + Perp * 4.0f };
            FSlateDrawElement::MakeLines(OutDrawElements, ScopeLayer + 2,
                AllottedGeometry.ToPaintGeometry(), Tick, ESlateDrawEffect::None, ReticleColor, true, 2.2f);
        }
    }

    return ScopeLayer + 3;
}

void UCodeRescueHUDWidget::RefreshHeadshotFeedback(float SinceHeadshot, const UCodeRescueGameInstance* GI)
{
    if (!HeadshotFeedbackText)
    {
        return;
    }

    const bool bReducedMotion = GI && GI->bReducedMotion;
    const bool bHighContrast = GI && GI->bHighContrastHUD;
    const float Duration = bReducedMotion
        ? HeadshotReducedMotionDurationSeconds
        : HeadshotStandardDurationSeconds;
    if (SinceHeadshot < 0.0f || SinceHeadshot >= Duration)
    {
        HeadshotFeedbackText->SetText(FText::FromString(TEXT("")));
        HeadshotFeedbackText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.64f, 0.34f, 0.0f)));
        if (HeadshotFeedbackSlot)
        {
            HeadshotFeedbackSlot->SetPosition(FVector2D(0.0f, HeadshotBaseY));
        }
        return;
    }

    const float Normalized = FMath::Clamp(SinceHeadshot / Duration, 0.0f, 1.0f);
    const float Alpha = bReducedMotion
        ? 1.0f
        : FMath::Clamp(1.0f - Normalized, 0.0f, 1.0f);
    const FLinearColor FeedbackColor = bHighContrast
        ? FLinearColor(1.0f, 0.96f, 0.14f, Alpha)
        : FLinearColor(0.96f, 0.64f, 0.34f, Alpha);

    FSlateFontInfo HeadshotFont = HeadshotFeedbackText->GetFont();
    HeadshotFont.Size = bReducedMotion
        ? 28
        : FMath::RoundToInt(FMath::Lerp(34.0f, 28.0f, Normalized));
    HeadshotFeedbackText->SetFont(HeadshotFont);
    HeadshotFeedbackText->SetText(FText::FromString(bReducedMotion ? TEXT("PRECISION HIT") : TEXT("HEADSHOT")));
    HeadshotFeedbackText->SetColorAndOpacity(FSlateColor(FeedbackColor));

    if (HeadshotFeedbackSlot)
    {
        const float MotionY = bReducedMotion
            ? HeadshotBaseY
            : FMath::Lerp(-76.0f, -112.0f, Normalized);
        HeadshotFeedbackSlot->SetPosition(FVector2D(0.0f, MotionY));
    }
}

void UCodeRescueHUDWidget::TriggerObjectiveRouteToast(const FString& Message, const FLinearColor& Color, float DurationSeconds)
{
    UWorld* World = GetWorld();
    if (!World || Message.IsEmpty())
    {
        return;
    }

    ObjectiveToastMessage = Message;
    ObjectiveToastColor = Color;
    ObjectiveToastStartSeconds = World->TimeSeconds;
    ObjectiveToastDurationSeconds = FMath::Max(0.5f, DurationSeconds);
}

void UCodeRescueHUDWidget::RefreshObjectiveRouteToast(const UCodeRescueGameInstance* GI, UWorld* World)
{
    if (!ObjectiveToastText || !GI || !World)
    {
        return;
    }

    const int32 SolvedCount = GI->SolvedTerminalIds.Num();
    const int32 RescuedCount = GI->RescuedSurvivorNames.Num();
    const int32 CurrentScore = GI->CodingScore;
    const float SaveStamp = GI->LastSaveWallSeconds;
    if (!bObjectiveToastStateSeeded)
    {
        bObjectiveToastStateSeeded = true;
        LastObservedSolvedTerminalCount = SolvedCount;
        LastObservedRescuedSurvivorCount = RescuedCount;
        LastObservedCodingScore = CurrentScore;
        LastObservedSaveWallSeconds = SaveStamp;
        ObjectiveToastText->SetText(FText::FromString(TEXT("")));
        return;
    }

    bool bRouteToastTriggered = false;
    if (SolvedCount > LastObservedSolvedTerminalCount)
    {
        const int32 ScoreDelta = FMath::Max(0, CurrentScore - LastObservedCodingScore);
        TriggerObjectiveRouteToast(
            FString::Printf(
                TEXT("OBJECTIVE UPDATED | Terminal solved | Survivor route open | +%d code score"),
                ScoreDelta),
            GI->bHighContrastHUD
                ? FLinearColor(1.0f, 0.96f, 0.18f, 1.0f)
                : FLinearColor(0.96f, 0.74f, 0.34f, 1.0f),
            4.8f);
        bRouteToastTriggered = true;
    }
    else if (RescuedCount > LastObservedRescuedSurvivorCount)
    {
        TriggerObjectiveRouteToast(
            TEXT("OBJECTIVE UPDATED | Survivor rescued | Extraction ready | Language save refreshed"),
            GI->bHighContrastHUD
                ? FLinearColor(0.74f, 1.0f, 0.92f, 1.0f)
                : FLinearColor(0.52f, 0.94f, 1.0f, 1.0f),
            5.2f);
        bRouteToastTriggered = true;
    }
    else if (SaveStamp > LastObservedSaveWallSeconds + KINDA_SMALL_NUMBER
        && (World->TimeSeconds - SaveStamp) >= 0.0f
        && (World->TimeSeconds - SaveStamp) < 1.5f)
    {
        TriggerObjectiveRouteToast(
            FString::Printf(
                TEXT("CHECKPOINT SAVED | %s run can resume from the start screen"),
                *GI->GetLanguageName()),
            GI->bHighContrastHUD
                ? FLinearColor(0.82f, 1.0f, 0.62f, 1.0f)
                : FLinearColor(0.50f, 1.0f, 0.52f, 1.0f),
            3.6f);
    }

    LastObservedSolvedTerminalCount = SolvedCount;
    LastObservedRescuedSurvivorCount = RescuedCount;
    LastObservedCodingScore = CurrentScore;
    LastObservedSaveWallSeconds = SaveStamp;

    const float ToastElapsed = World->TimeSeconds - ObjectiveToastStartSeconds;
    if (ObjectiveToastMessage.IsEmpty() || ToastElapsed < 0.0f || ToastElapsed >= ObjectiveToastDurationSeconds)
    {
        ObjectiveToastText->SetText(FText::FromString(TEXT("")));
        ObjectiveToastText->SetColorAndOpacity(FSlateColor(FLinearColor(ObjectiveToastColor.R, ObjectiveToastColor.G, ObjectiveToastColor.B, 0.0f)));
        return;
    }

    const float FadeWindow = GI->bReducedMotion ? 0.0f : 0.85f;
    const float Alpha = FadeWindow <= 0.0f
        ? 1.0f
        : FMath::Clamp(FMath::Min(ToastElapsed / FadeWindow, (ObjectiveToastDurationSeconds - ToastElapsed) / FadeWindow), 0.0f, 1.0f);
    ObjectiveToastText->SetText(FText::FromString(ObjectiveToastMessage));
    ObjectiveToastText->SetColorAndOpacity(FSlateColor(FLinearColor(ObjectiveToastColor.R, ObjectiveToastColor.G, ObjectiveToastColor.B, Alpha)));

    if (bRouteToastTriggered && GI->bVisualizeSoundCues && SoundCueText)
    {
        SoundCueText->SetColorAndOpacity(FSlateColor(GI->bHighContrastHUD
            ? FLinearColor(0.86f, 1.0f, 1.0f, 1.0f)
            : FLinearColor(0.72f, 0.98f, 1.0f, 1.0f)));
    }
}

void UCodeRescueHUDWidget::RefreshHUD()
{
    ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    UWorld* World = GetWorld();
    if (!StatusText || !Character || !GI || !World) return;
    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();

    const FCodeRescueThreatHudInfo ThreatInfo = GetNearestHudThreat(World, Character);
    if (GI->bHighContrastHUD)
    {
        StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        if (SecondLineText)
        {
            SecondLineText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.96f, 0.62f, 1.0f)));
        }
        if (ObjectiveFocusText)
        {
            ObjectiveFocusText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.92f, 0.36f, 1.0f)));
        }
        if (ThreatCompassText)
        {
            ThreatCompassText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.96f, 0.42f, 1.0f)));
        }
        if (SoundCueText)
        {
            SoundCueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 1.0f, 1.0f, 1.0f)));
        }
    }
    const int32 RequiredCities = FCodeRescueCampaign::GetMissionCount();
    if (!bHasObservedPlayerLocation)
    {
        bHasObservedPlayerLocation = true;
        LastObservedPlayerLocation = Character->GetActorLocation();
        IdleGuidanceSeconds = 0.0f;
    }
    else if (FVector::DistSquared2D(LastObservedPlayerLocation, Character->GetActorLocation()) > FMath::Square(24.0f))
    {
        LastObservedPlayerLocation = Character->GetActorLocation();
        IdleGuidanceSeconds = 0.0f;
    }
    else
    {
        IdleGuidanceSeconds += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
    }

    const int32 ActiveReserveAmmo = Character->GetActiveWeaponReserveAmmo();
    const FString ControlsLine = GI->bSimplifiedInputHints
        ? TEXT("WASD move | LMB shoot | RMB aim | E interact | Q medkit | L light | K scan | J journal | Esc pause")
        : TEXT("WASD move | LMB/Space/F shoot | 1-0 select, Wheel/[ ] cycle weapons | F1-F6 views | C/V camera | L flashlight | Z scanner | E interact | Q medkit | Y/U/N/O squad | J journal");
    // 2026-07-01 HUD diet (Kenny): in minimal mode the top strip is ONE compact vitals line -
    // health / armor / ammo / medkits / language. Campaign counters, guidance, field kit, and the
    // full controls sheet move to the objective journal (J) and pause menu (P), so play stays clean.
    FString Text;
    if (bMinimalHUD)
    {
        // 2026-07-07 (Kenny: "I still do not see the option to cycle between
        // different weapons"): minimal HUD never NAMED the held weapon, so
        // swaps were invisible. The vitals strip now leads with the weapon
        // and its cycle keys.
        Text = FString::Printf(
            TEXT("%s  (wheel or [ ] to cycle)\nHP %.0f/%.0f  Armor %d/%d  Ammo %d/%d  Kits %d/%d  %s"),
            *Character->GetActiveWeaponName(),
            Character->Health, Character->MaxHealth,
            Character->GetArmorPlates(), Character->GetMaxArmorPlates(),
            Character->Ammo, Character->MaxAmmo,
            Character->Medkits, Character->MaxMedkits,
            *GI->GetLanguageName());
    }
    else
    {
        Text = FString::Printf(
            TEXT("Health: %.0f / %.0f   Armor: %d / %d   Ammo Pool: %d / %d   Active Reserve: %d   Medkits: %d / %d   Language: %s   Difficulty: %s\nCities Graduated: %d / %d   Survivors: %d / %d   Zombies Neutralized: %d   Coding Score: %d\n%s\n%s\n%s"),
            Character->Health, Character->MaxHealth,
            Character->GetArmorPlates(), Character->GetMaxArmorPlates(),
            Character->Ammo, Character->MaxAmmo, ActiveReserveAmmo,
            Character->Medkits, Character->MaxMedkits,
            *GI->GetLanguageName(), *GI->GetDifficultyDisplayName(),
            FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI), RequiredCities,
            GI->RescuedSurvivorNames.Num(), RequiredCities,
            GI->ZombiesNeutralized, GI->CodingScore,
            *Character->GetOpenWorldGuidanceText(),
            *Character->GetFieldKitSummary(), *ControlsLine);
    }
    StatusText->SetText(FText::FromString(Text));

    if (ObjectiveFocusText)
    {
        const int32 ActiveCityIndex = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
        const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(ActiveCityIndex);
        FString ObjectiveText = TEXT("Campaign Complete | Extraction ready");
        if (Mission)
        {
            const FVector Origin = FCodeRescueCampaign::GetCityOrigin(ActiveCityIndex);
            FVector Target = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(-3820.0f, -3180.0f, 90.0f));
            FString Verb = TEXT("Reach city entry");
            FString CoachLine;
            const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, ActiveCityIndex);
            const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, ActiveCityIndex);
            const bool bSurvivorRescued = GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
            const FString NextChallengeId = FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(GI, ActiveCityIndex);
            const FCodeRescueMissionProgress* Progress = FindMissionProgress(GI, NextChallengeId);
            const float BossDistanceMeters = GetNearestLivingBossDistanceMeters(GetWorld(), Character->GetActorLocation());
            if (!bTerminalSolved)
            {
                Target = ResolveNextStationWorldTarget(GetWorld(), NextChallengeId,
                    Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(-2860.0f, -2930.0f, 90.0f)));
                if (Progress && Progress->Attempts > 0 && !Progress->bCompleted)
                {
                    Verb = FString::Printf(TEXT("Repair coding station %d/%d"), CompletedChallenges + 1, FCodeRescueCampaign::RequiredChallengesPerCity);
                    CoachLine = FString::Printf(TEXT("Protected concourse active - use the failed check as a clue for %s."), *GI->GetLanguageName());
                }
                else
                {
                    Verb = FString::Printf(
                        TEXT("Complete %s coding station %d/%d"),
                        *GI->GetLanguageName(),
                        CompletedChallenges + 1,
                        FCodeRescueCampaign::RequiredChallengesPerCity);
                    CoachLine = TEXT("Coding pauses combat; every first-time pass saves and drops supplies.");
                }
            }
            else if (!bSurvivorRescued)
            {
                Target = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(2850.0f, 1500.0f, 90.0f));
                Verb = TEXT("Terminal passed - rescue survivor team");
                CoachLine = TEXT("The code route is open; find the cyan survivor marker.");
            }
            else
            {
                Target = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(-3700.0f, 2850.0f, 90.0f));
                Verb = TEXT("Extraction ready - debrief and graduate");
                CoachLine = TEXT("Rescue complete; use extraction or press T to return to the level route.");
            }

            if (BossDistanceMeters >= 0.0f && BossDistanceMeters < 55.0f && !bSurvivorRescued)
            {
                CoachLine = TEXT("Optional boss nearby - keep distance unless you are ready.");
            }
            else if (IdleGuidanceSeconds > 18.0f)
            {
                CoachLine = TEXT("Idle tip: follow colored route strips, use J for the journal, or press T for the active objective.");
            }

            const float DistanceMeters = FVector::Dist(Character->GetActorLocation(), Target) / 100.0f;
            if (bMinimalHUD)
            {
                // One short objective line: verb + distance/direction. No city echo, no coach paragraph.
                ObjectiveText = FString::Printf(TEXT("%s  |  %.0fm %s"),
                    *Verb, DistanceMeters, *DirectionLabelFromPlayer(Character, Target));
            }
            else
            {
                ObjectiveText = FString::Printf(
                    TEXT("%s | %s, %s | %.0fm %s%s%s"),
                    *Verb, *Mission->CityName, *Mission->StateName, DistanceMeters,
                    *DirectionLabelFromPlayer(Character, Target),
                    CoachLine.IsEmpty() ? TEXT("") : TEXT("\n"), *CoachLine);
            }
        }
        ObjectiveFocusText->SetText(FText::FromString(ObjectiveText));
    }

    if (NavigationStripText)
    {
        const int32 ActiveCityIndex = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
        const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(ActiveCityIndex);
        FString NavText = TEXT("NAVIGATION\nCampaign complete\nExtraction/debrief ready");
        if (Mission)
        {
            const FVector Origin = FCodeRescueCampaign::GetCityOrigin(ActiveCityIndex);
            const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, ActiveCityIndex);
            const FVector StationFallback = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(-2860.0f, -2930.0f, 90.0f));
            FVector Target = ResolveNextStationWorldTarget(GetWorld(),
                FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(GI, ActiveCityIndex), StationFallback);
            FString Phase = FString::Printf(TEXT("2 CODING %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity);
            if (!FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, ActiveCityIndex))
            {
                Phase = FString::Printf(TEXT("2 CODING %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity);
            }
            else if (!GI->RescuedSurvivorNames.Contains(Mission->SurvivorName))
            {
                Target = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(2850.0f, 1500.0f, 90.0f));
                Phase = TEXT("3 SURVIVOR");
            }
            else
            {
                Target = Origin + FCodeRescueCampaign::ScaleCityOffset(FVector(2400.0f, 2400.0f, 90.0f));
                Phase = TEXT("4 EXTRACTION");
            }

            const float DistanceMeters = FVector::Dist(Character->GetActorLocation(), Target) / 100.0f;
            NavText = FString::Printf(
                TEXT("NAVIGATION\n%s\n%.0fm %s\nFollow glowing floor arrows or press T"),
                *Phase,
                DistanceMeters,
                *DirectionLabelFromPlayer(Character, Target).ToUpper());
        }
        NavigationStripText->SetText(FText::FromString(NavText));
    }

    if (FieldChecklistText)
    {
        const int32 ActiveCityIndex = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
        const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(ActiveCityIndex);
        FString ChecklistText = FString::Printf(
            TEXT("FIRST TEN MINUTES FIELD CHECKLIST\nTrack: %s only | Campaign complete\nRoute: protected terminal -> survivor marker -> extraction\nKeys: J journal | P/Esc save | start-screen Resume ready"),
            *GI->GetLanguageName());

        if (Mission)
        {
            const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, ActiveCityIndex);
            const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, ActiveCityIndex);
            const bool bSurvivorRescued = GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
            const FString SaveSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
            const FString SaveState = GI->DoesLanguageSaveExist(GI->SelectedLanguage)
                ? TEXT("start-screen Resume ready")
                : TEXT("autosaves after progress");
            FString Phase = FString::Printf(TEXT("protected coding concourse %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity);
            FString NextAction = FString::Printf(TEXT("Complete station %d of %d | E open | VALIDATE CODE"), CompletedChallenges + 1, FCodeRescueCampaign::RequiredChallengesPerCity);
            FString Recovery = TEXT("T route | Backspace recover | J journal");

            if (bTerminalSolved && !bSurvivorRescued)
            {
                Phase = TEXT("survivor marker");
                NextAction = FString::Printf(TEXT("Follow survivor marker to %s"), *Mission->SurvivorName);
                Recovery = TEXT("T route | J survivor intel | P/Esc save");
            }
            else if (bTerminalSolved && bSurvivorRescued)
            {
                Phase = TEXT("extraction");
                NextAction = TEXT("Return to helipad extraction/debrief");
                Recovery = TEXT("T route | J recap | P/Esc save");
            }

            ChecklistText = FString::Printf(
                TEXT("FIRST TEN MINUTES FIELD CHECKLIST\nTrack: %s only | Save: %s | %s\nRoute: protected terminal -> survivor marker -> extraction\nPhase: %s | %s\nKeys: %s"),
                *GI->GetLanguageName(),
                *SaveSlot,
                *SaveState,
                *Phase,
                *NextAction,
                *Recovery);
        }

        FieldChecklistText->SetText(FText::FromString(ChecklistText));
        FieldChecklistText->SetColorAndOpacity(FSlateColor(GI->bHighContrastHUD
            ? FLinearColor(0.92f, 1.0f, 0.62f, 1.0f)
            : FLinearColor(0.72f, 1.0f, 0.80f, 1.0f)));
    }

    if (ThreatCompassText)
    {
        if (ThreatInfo.bHasThreat)
        {
            const FString PursuitSegment = ThreatInfo.bStandardPursuit
                ? FString::Printf(TEXT(" | %s"), *ThreatInfo.PursuitLabel)
                : FString();
            ThreatCompassText->SetText(FText::FromString(FString::Printf(
                TEXT("THREAT COMPASS  %s | %s%s%s | %.0fm %s"),
                *ThreatInfo.UrgencyLabel,
                *ThreatInfo.RoleLabel,
                *ThreatInfo.VariantLabel,
                *PursuitSegment,
                ThreatInfo.DistanceMeters,
                *ThreatInfo.DirectionLabel.ToUpper())));
            ThreatCompassText->SetColorAndOpacity(FSlateColor(
                GI->bHighContrastHUD
                    ? FLinearColor(1.0f, 0.96f, 0.42f, 1.0f)
                    : ThreatInfo.Accent));
        }
        else
        {
            ThreatCompassText->SetText(FText::FromString(TEXT("THREAT COMPASS  clear")));
            ThreatCompassText->SetColorAndOpacity(FSlateColor(
                GI->bHighContrastHUD
                    ? FLinearColor::White
                    : FLinearColor(0.68f, 0.96f, 1.0f, 1.0f)));
        }
    }

    if (SoundCueText)
    {
        if (GI->bVisualizeSoundCues)
        {
            const float ThreatIntensity = FMath::Clamp(GI->ReactiveThreatMusicIntensity, 0.0f, 1.0f);
            const float AmbientIntensity = FMath::Clamp(GI->CityAmbientZoneIntensity, 0.0f, 1.0f);
            const FString ThreatState = GI->ReactiveThreatMusicState.IsEmpty()
                ? FString(TEXT("calm"))
                : GI->ReactiveThreatMusicState;
            const FString AmbientLabel = GI->CityAmbientZoneLabel.IsEmpty()
                ? FString(TEXT("city street"))
                : GI->CityAmbientZoneLabel;
            const FString SubtitleState = GI->bSubtitlesEnabled ? TEXT("captions on") : TEXT("captions off");
            SoundCueText->SetText(FText::FromString(FString::Printf(
                TEXT("SOUND CUES  threat %s %.0f%% | ambient %s %.0f%% | %s"),
                *ThreatState,
                ThreatIntensity * 100.0f,
                *AmbientLabel,
                AmbientIntensity * 100.0f,
                *SubtitleState)));
            const FLinearColor CueColor = GI->bHighContrastHUD
                ? FLinearColor(0.78f, 1.0f, 1.0f, 1.0f)
                : FMath::Lerp(FLinearColor(0.70f, 0.94f, 1.0f, 1.0f), ThreatInfo.Accent, ThreatIntensity);
            SoundCueText->SetColorAndOpacity(FSlateColor(CueColor));
        }
        else
        {
            SoundCueText->SetText(FText::FromString(TEXT("")));
        }
    }

    RefreshObjectiveRouteToast(GI, World);

    // Crosshair hover: forward-trace ~600 units, color the crosshair and
    // show the right interaction prompt depending on what's there. Cheap
    // single-line trace per tick, fine to run every frame.
    if (!CrosshairText || !World)
    {
        return;
    }

    UCameraComponent* Cam = Character->GetActiveGameplayCamera();
    const FVector Start = Cam ? Cam->GetComponentLocation() : Character->GetActorLocation();
    const FVector Dir   = Cam ? Cam->GetForwardVector()     : Character->GetActorForwardVector();
    const FVector End   = Start + Dir * Character->GetInteractionTraceDistance();

    FCollisionQueryParams Params(SCENE_QUERY_STAT(HUDCrosshairTrace), false, Character);
    FHitResult Hit;
    const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    FLinearColor CrossColor(0.78f, 0.96f, 0.68f, 1.0f);
    FString Prompt;

    if (bHit)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            if (Cast<ACodeZombieActor>(HitActor))
            {
                CrossColor = FLinearColor(1.0f, 0.20f, 0.16f, 1.0f);
            }
            else if (ACodingTerminalActor* T = Cast<ACodingTerminalActor>(HitActor))
            {
                CrossColor = FLinearColor(0.96f, 0.78f, 0.42f, 1.0f);
                Prompt     = TEXT("[E] open coding terminal");
            }
            else if (ASurvivorActor* Survivor = Cast<ASurvivorActor>(HitActor))
            {
                CrossColor = Survivor->ArchetypeAccentColor;
                Prompt     = Survivor->GetInteractionPrompt();
            }
            else if (Cast<APickupActor>(HitActor))
            {
                CrossColor = FLinearColor(0.62f, 0.92f, 0.48f, 1.0f);
                Prompt     = TEXT("[E] pick up supplies");
            }
            else if (AFriendlyNPCActor* NPC = Cast<AFriendlyNPCActor>(HitActor))
            {
                CrossColor = FLinearColor(0.95f, 0.74f, 0.38f, 1.0f);
                Prompt     = NPC->GetInteractionPrompt();
            }
            else if (Cast<ALanguageStationActor>(HitActor))
            {
                CrossColor = FLinearColor(0.72f, 0.56f, 0.92f, 1.0f);
                Prompt     = TEXT("language locked at launch");
            }
            else if (HitActor->Tags.Contains(FName("Helipad")))
            {
                CrossColor = FLinearColor(0.42f, 0.68f, 0.88f, 1.0f);
                Prompt     = TEXT("[E] open fast travel");
            }
            else if (HitActor->Tags.Contains(FName("Jeep")))
            {
                CrossColor = FLinearColor(0.52f, 0.92f, 0.56f, 1.0f);
                Prompt     = TEXT("[E] mount jeep");
            }
            else if (HitActor->Tags.Contains(FName("CodeRescueDoor")))
            {
                CrossColor = FLinearColor(0.92f, 0.80f, 0.38f, 1.0f);
                Prompt     = TEXT("[E] open / close door");
            }
        }
    }

    if (Character->IsAimTargetLocked())
    {
        CrossColor = FLinearColor(1.0f, 0.54f, 0.10f, 1.0f);
        CrosshairText->SetText(FText::FromString(TEXT("[X]")));
        if (AimLockText)
        {
            AimLockText->SetText(FText::FromString(Character->GetAimTargetLockSummary()));
        }
    }
    else
    {
        // pass 5: while aiming, the crosshair reads as a scope reticle with the
        // active magnification so the player always knows their zoom level.
        // 2026-07-17: in the full circular scope view the PAINTED reticle owns
        // the center — the text becomes the ZOOM readout parked below the
        // glass, like the reference sight. Center guidance texts step aside
        // so the sight picture stays clean.
        if (Character->IsScopeViewActive())
        {
            CrosshairText->SetText(FText::FromString(
                FString::Printf(TEXT("ZOOM: %s"), *Character->GetScopeZoomLabel())));
            CrosshairText->SetRenderTranslation(FVector2D(0.0f, 260.0f));
            for (UTextBlock* CenterText : { InteractionPromptText, ObjectiveFocusText,
                                            ObjectiveToastText, NavigationStripText,
                                            FieldChecklistText, ThreatCompassText, StatusText })
            {
                if (CenterText)
                {
                    CenterText->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
            for (UBorder* Panel : { TopVignetteBorder, BottomVignetteBorder,
                                    StatusPanelBorder, ObjectivePanelBorder })
            {
                if (Panel)
                {
                    Panel->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
        }
        else
        {
            if (Character->IsADSActive())
            {
                const FString ZoomLabel = Character->GetScopeZoomLabel();
                CrosshairText->SetText(FText::FromString(
                    ZoomLabel == TEXT("1x")
                        ? FString(TEXT("[ + ]"))
                        : FString::Printf(TEXT("[ + ]  %s"), *ZoomLabel)));
            }
            else
            {
                CrosshairText->SetText(FText::FromString(TEXT("+")));
            }
            CrosshairText->SetRenderTranslation(FVector2D::ZeroVector);
            // restore the guidance texts + panels the scope view stepped aside
            // (navigation strip + toast stay collapsed under minimal HUD)
            for (UTextBlock* CenterText : { InteractionPromptText, ObjectiveFocusText,
                                            FieldChecklistText, ThreatCompassText, StatusText })
            {
                if (CenterText)
                {
                    CenterText->SetVisibility(ESlateVisibility::HitTestInvisible);
                }
            }
            for (UBorder* Panel : { TopVignetteBorder, BottomVignetteBorder,
                                    StatusPanelBorder, ObjectivePanelBorder })
            {
                if (Panel)
                {
                    Panel->SetVisibility(ESlateVisibility::HitTestInvisible);
                }
            }
            if (!bMinimalHUD)
            {
                for (UTextBlock* StripText : { NavigationStripText, ObjectiveToastText })
                {
                    if (StripText)
                    {
                        StripText->SetVisibility(ESlateVisibility::HitTestInvisible);
                    }
                }
            }
        }
        if (AimLockText)
        {
            AimLockText->SetText(FText::GetEmpty());
        }
    }
    CrosshairText->SetColorAndOpacity(FSlateColor(CrossColor));
    if (InteractionPromptText)
    {
        if (Prompt.IsEmpty() && IdleGuidanceSeconds > 8.0f)
        {
            Prompt = TEXT("No [E] target in reach - follow the top objective");
        }
        InteractionPromptText->SetText(FText::FromString(Prompt));
    }

    // #42 — second-line readout for new mechanics.
    if (SecondLineText)
    {
        const FString WeaponName = Character->GetActiveWeaponName();
        const FString WeaponRole = Character->GetActiveWeaponTacticalRole();
        static const TCHAR* ThrowableSlotNames[] = { TEXT("Flare"), TEXT("Smoke"), TEXT("Stim") };
        const int32 ThrowableSlot = FMath::Clamp(Character->GetActiveThrowableSlot(), 0, 2);
        const int32 ThrowableCount = Character->GetThrowableCountForSlot(ThrowableSlot);
        const int32 SuccessRate = GI->TotalValidationAttempts > 0
            ? FMath::RoundToInt((100.0f * GI->SuccessfulValidationAttempts) / static_cast<float>(GI->TotalValidationAttempts))
            : 0;
        auto CountAt = [](const TArray<int32>& Counters, int32 Idx) -> int32
        {
            return Counters.IsValidIndex(Idx) ? Counters[Idx] : 0;
        };
        const int32 SelectedLanguageIndex = FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, 5);
        const FString OperatorCallsign = GI->OperatorCallsign.IsEmpty()
            ? FString(TEXT("Rhea Calder"))
            : GI->OperatorCallsign;
        const FString OperatorRole = GI->OperatorRoleTitle.IsEmpty()
            ? FString(TEXT("Rescue Operator"))
            : GI->OperatorRoleTitle;
        SecondLineText->SetText(FText::FromString(FString::Printf(
            TEXT("Operator: %s (%s)   Weapon: %s (%d/%d + %d)   Role: %s   Throwable: %s x%d   Armor: %d/%d   Scan: %d/%d   Light: %d/%d   Bypass: %d/%d   Pouch +%d   Scrap: %d   Research: %d   Mode: %s Academy\nLearning: %s | Attempts %d | Success %d%% | Streak %d/%d | No-hint %d | Perfect %d | Track %s %d/%d"),
            *OperatorCallsign,
            *OperatorRole,
            *WeaponName,
            Character->MagazineAmmo,
            Character->GetActiveWeaponMagazineSize(),
            Character->GetActiveWeaponReserveAmmo(),
            *WeaponRole,
            ThrowableSlotNames[ThrowableSlot], ThrowableCount,
            Character->GetArmorPlates(), Character->GetMaxArmorPlates(),
            Character->GetRadioScannerCharges(), Character->GetMaxRadioScannerCharges(),
            Character->GetFlashlightBatteries(), Character->GetMaxFlashlightBatteries(),
            Character->GetBypassKits(), Character->GetMaxBypassKits(),
            Character->GetAmmoPouchCapacityBonus(),
            Character->GetScrap(), GI->ResearchPoints, *GI->GetLanguageName(),
            *GI->GetLearningMasteryTitle(),
            GI->TotalValidationAttempts,
            SuccessRate,
            GI->CurrentLearningStreak,
            GI->BestLearningStreak,
            GI->NoHintSolveCount,
            GI->PerfectSolveCount,
            *GI->GetLanguageName(),
            CountAt(GI->LanguageSolveCounts, SelectedLanguageIndex),
            CountAt(GI->LanguageAttemptCounts, SelectedLanguageIndex))));
    }

    if (WeaponStripText)
    {
        const int32 ActiveWeaponIndex = static_cast<int32>(Character->ActiveWeapon);
        const int32 ActiveSlot = ActiveWeaponIndex + 1;
        const FString ThrowableName = Character->GetActiveThrowableSlot() == 0
            ? TEXT("Flare")
            : Character->GetActiveThrowableSlot() == 1
                ? TEXT("Smoke")
                : TEXT("Stim");
        const int32 ThrowableCount = Character->GetThrowableCountForSlot(Character->GetActiveThrowableSlot());
        WeaponStripText->SetText(FText::FromString(FString::Printf(
            TEXT("%s\nWEAPON SLOT | ACTIVE %d/%d %s | Weapons %d total | Ammo %d/%d + %d | Wheel/[ ] all %d | X %s x%d | L %d | Z %d | Q Medkit x%d"),
            *Character->GetWeaponQuickSlotSummary(),
            ActiveSlot,
            FMath::Max(1, Character->GetWeaponCount()),
            *Character->GetActiveWeaponName(),
            FMath::Max(1, Character->GetWeaponCount()),
            Character->MagazineAmmo,
            Character->GetActiveWeaponMagazineSize(),
            Character->GetActiveWeaponReserveAmmo(),
            FMath::Max(1, Character->GetWeaponCount()),
            *ThrowableName,
            ThrowableCount,
            Character->GetFlashlightBatteries(),
            Character->GetRadioScannerCharges(),
            Character->Medkits)));
    }

    if (HealthBar)
    {
        const float HealthPct = Character->MaxHealth > 0.0f
            ? FMath::Clamp(Character->Health / Character->MaxHealth, 0.0f, 1.0f)
            : 0.0f;
        HealthBar->SetPercent(HealthPct);
        HealthBar->SetFillColorAndOpacity(HudHealthFillColor(HealthPct, GI));
    }

    if (HealthLabelText)
    {
        const float HealthPct = Character->MaxHealth > 0.0f
            ? FMath::Clamp(Character->Health / Character->MaxHealth, 0.0f, 1.0f)
            : 0.0f;
        HealthLabelText->SetText(FText::FromString(FString::Printf(
            TEXT("PLAYER HEALTH  %.0f / %.0f   %.0f%%   %s"),
            Character->Health,
            Character->MaxHealth,
            HealthPct * 100.0f,
            *HudVitalStateLabel(HealthPct))));
        CodeRescueUI::StyleText(
            HealthLabelText,
            CodeRescueUI::EType::Subheading,
            HudHealthLabelColor(HealthPct, GI));
    }

    if (DamageAlertText)
    {
        const float SinceDamage = World->TimeSeconds - Character->GetLastDamageWorldTime();
        if (SinceDamage >= 0.0f && SinceDamage < 2.8f)
        {
            const FString DistanceText = Character->GetLastDamageSourceDistanceMeters() >= 0.0f
                ? FString::Printf(TEXT(" | %.0fm"), Character->GetLastDamageSourceDistanceMeters())
                : FString();
            const FString MitigationText = Character->GetLastDamageMitigationText();
            const FString MitigationSuffix = MitigationText.Equals(TEXT("none"))
                ? FString()
                : FString::Printf(TEXT(" | %s"), *MitigationText.ToUpper());
            DamageAlertText->SetText(FText::FromString(FString::Printf(
                TEXT("ATTACKED FROM %s | %.0f dmg | %s%s%s"),
                *Character->GetLastDamageLocationText().ToUpper(),
                Character->GetLastDamageAmount(),
                *Character->GetLastDamageSourceText(),
                *DistanceText,
                *MitigationSuffix)));
            const float Alpha = FMath::Clamp(1.0f - SinceDamage / 2.8f, 0.15f, 1.0f);
            const FLinearColor DamageAlertColor = GI->bHighContrastHUD
                ? FLinearColor(1.0f, 0.94f, 0.16f, Alpha)
                : FLinearColor(1.0f, 0.30f, 0.18f, Alpha);
            DamageAlertText->SetColorAndOpacity(FSlateColor(DamageAlertColor));
        }
        else
        {
            DamageAlertText->SetText(FText::FromString(TEXT("")));
        }
    }

    if (SquadStatusText)
    {
        int32 OperationalCount = 0;
        int32 MedicCount = 0;
        bool bMedicNearby = false;
        float MedicReadySeconds = -1.0f;
        TArray<FString> SquadHealthPips;
        TArray<FString> SquadRoleReadouts;
        for (TActorIterator<ACompanionActor> It(World); It; ++It)
        {
            const ACompanionActor* Companion = *It;
            if (!IsValid(Companion) || !Companion->IsOperational())
            {
                continue;
            }

            ++OperationalCount;
            const float HealthPct = Companion->MaxHealth > 0.0f
                ? FMath::Clamp(Companion->Health / Companion->MaxHealth, 0.0f, 1.0f)
                : 0.0f;
            SquadHealthPips.Add(FString::Printf(
                TEXT("%s %.0f%%"),
                *Companion->GetHudCallsign(),
                HealthPct * 100.0f));
            if (SquadRoleReadouts.Num() < 3)
            {
                SquadRoleReadouts.Add(Companion->GetRoleStatusLabel());
            }
            if (Companion->bMedicSupport)
            {
                ++MedicCount;
                const float DistanceSq = FVector::DistSquared(Character->GetActorLocation(), Companion->GetActorLocation());
                bMedicNearby |= DistanceSq <= FMath::Square(1200.0f);
                const float ReadySeconds = Companion->GetMedicPulseReadySeconds();
                MedicReadySeconds = MedicReadySeconds < 0.0f ? ReadySeconds : FMath::Min(MedicReadySeconds, ReadySeconds);
            }
        }

        if (OperationalCount > 0)
        {
            FString MedicText = TEXT("MEDIC UNASSIGNED");
            if (MedicCount > 0)
            {
                MedicText = MedicReadySeconds <= 0.0f
                    ? FString::Printf(TEXT("MEDIC %s READY"), bMedicNearby ? TEXT("NEARBY") : TEXT("AWAY"))
                    : FString::Printf(TEXT("MEDIC %s %.0fs"), bMedicNearby ? TEXT("NEARBY") : TEXT("AWAY"), MedicReadySeconds);
            }
            const float SinceRegroup = World->TimeSeconds - Character->GetLastSquadRegroupWorldTime();
            const FString RegroupText = SinceRegroup >= 0.0f && SinceRegroup < 3.5f
                ? FString::Printf(TEXT("REGROUPED %d"), Character->GetLastSquadRegroupCount())
                : TEXT("Y REGROUP");
            const float SinceFormation = World->TimeSeconds - Character->GetLastSquadFormationWorldTime();
            const FString FormationText = SinceFormation >= 0.0f && SinceFormation < 3.5f
                ? FString::Printf(TEXT("FORMATION %s"), *Character->GetSquadFormationLabel())
                : FString::Printf(TEXT("U %s"), *Character->GetSquadFormationLabel());
            const float SinceOrder = World->TimeSeconds - Character->GetLastSquadOrderWorldTime();
            const FString OrderText = SinceOrder >= 0.0f && SinceOrder < 3.5f
                ? FString::Printf(TEXT("ORDER %s"), *Character->GetSquadOrderLabel())
                : FString::Printf(TEXT("O %s"), *Character->GetSquadOrderLabel());
            const float SinceMedicCall = World->TimeSeconds - Character->GetLastManualMedicCallWorldTime();
            const FString MedicCallText = SinceMedicCall >= 0.0f && SinceMedicCall < 3.5f
                ? TEXT("MEDIC CALLED")
                : TEXT("N MEDIC");
            const FString SquadHealthText = FString::Join(SquadHealthPips, TEXT(" "));
            if (OperationalCount > SquadRoleReadouts.Num())
            {
                SquadRoleReadouts.Add(FString::Printf(TEXT("+%d active"), OperationalCount - SquadRoleReadouts.Num()));
            }
            const FString SquadRoleText = FString::Join(SquadRoleReadouts, TEXT(" | "));
            SquadStatusText->SetText(FText::FromString(FString::Printf(
                TEXT("RESCUE TEAM  %d ACTIVE   |   HP %s   |   %s\nROLES %s   |   SUPPORT FIRE ONLINE   |   %s   |   %s   |   %s   |   %s"),
                OperationalCount,
                *SquadHealthText,
                *MedicText,
                *SquadRoleText,
                *RegroupText,
                *FormationText,
                *OrderText,
                *MedicCallText)));
            SquadStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.92f, 1.0f, 1.0f)));
        }
        else
        {
            SquadStatusText->SetText(FText::FromString(TEXT("")));
        }
    }

    if (StaminaBar)
    {
        const float StaminaPct = Character->MaxStamina > 0.0f
            ? FMath::Clamp(Character->Stamina / Character->MaxStamina, 0.0f, 1.0f)
            : 0.0f;
        StaminaBar->SetPercent(StaminaPct);
        StaminaBar->SetFillColorAndOpacity(HudStaminaFillColor(StaminaPct, GI));
    }

    if (HeadshotFeedbackText)
    {
        const float SinceHeadshot = World->TimeSeconds - Character->GetLastHeadshotWorldTime();
        RefreshHeadshotFeedback(SinceHeadshot, GI);
    }

    if (ReloadStatusText || TacticalReadoutText)
    {
        const int32 CurrentMagazineSize = Character->GetActiveWeaponMagazineSize();
        const bool bLowMagazine = Character->MagazineAmmo <= FMath::Max(2, CurrentMagazineSize / 4);
        const bool bLowHealth = Character->Health <= Character->MaxHealth * 0.30f;
        const bool bThreatClose = ThreatInfo.bHasThreat && ThreatInfo.DistanceMeters <= 18.0f;
        const bool bBossPressure = ThreatInfo.bIsBoss && ThreatInfo.DistanceMeters <= 55.0f;
        const bool bElitePressure = ThreatInfo.bIsElite && ThreatInfo.DistanceMeters <= 42.0f;
        const bool bStandardPursuitPressure = ThreatInfo.bStandardPursuit && ThreatInfo.DistanceMeters <= 28.0f;

        FString Alert;
        if (Character->bIsReloading)
        {
            Alert = TEXT("RELOADING");
        }
        else if (bLowHealth)
        {
            Alert = TEXT("LOW HEALTH - use Q for medkit or find medic");
        }
        else if (bBossPressure)
        {
            Alert = FString::Printf(TEXT("BOSS PRESSURE - %.0fm %s"), ThreatInfo.DistanceMeters, *ThreatInfo.DirectionLabel.ToUpper());
        }
        else if (bElitePressure)
        {
            Alert = FString::Printf(TEXT("ELITE PRESSURE - %s %.0fm %s"), *ThreatInfo.VariantLabel.ToUpper(), ThreatInfo.DistanceMeters, *ThreatInfo.DirectionLabel.ToUpper());
        }
        else if (bStandardPursuitPressure)
        {
            Alert = FString::Printf(TEXT("PURSUIT PRESSURE - %s %.0fm %s"), *ThreatInfo.PursuitLabel.ToUpper(), ThreatInfo.DistanceMeters, *ThreatInfo.DirectionLabel.ToUpper());
        }
        else if (bLowMagazine)
        {
            Alert = TEXT("LOW MAGAZINE - press R to reload");
        }
        else if (bThreatClose)
        {
            Alert = TEXT("HOSTILE CLOSE");
        }

        if (ReloadStatusText)
        {
            ReloadStatusText->SetText(FText::FromString(Alert));
            ReloadStatusText->SetColorAndOpacity(FSlateColor(
                bLowHealth || bThreatClose
                    ? FLinearColor(0.95f, 0.22f, 0.14f, 1.0f)
                    : FLinearColor(0.96f, 0.68f, 0.32f, 1.0f)));
        }

        if (TacticalReadoutText)
        {
            const FString ThreatText = ThreatInfo.bHasThreat
                ? FString::Printf(
                    TEXT("Threat %s: %s%s %.0fm %s%s%s"),
                    *ThreatInfo.UrgencyLabel,
                    *ThreatInfo.RoleLabel,
                    *ThreatInfo.VariantLabel,
                    ThreatInfo.DistanceMeters,
                    *ThreatInfo.DirectionLabel,
                    ThreatInfo.bStandardPursuit ? TEXT(" - ") : TEXT(""),
                    ThreatInfo.bStandardPursuit ? *ThreatInfo.PursuitLabel : TEXT(""))
                : TEXT("Threat clear");
            const float EmergencyMedkitReadySeconds = Character->GetEmergencyMedkitReadySeconds();
            const FString EmergencyMedkitText = EmergencyMedkitReadySeconds < 0.0f
                ? TEXT("Auto medkit off")
                : EmergencyMedkitReadySeconds <= 0.0f
                    ? TEXT("Auto medkit ready")
                    : FString::Printf(TEXT("Auto medkit %.0fs"), EmergencyMedkitReadySeconds);
            TacticalReadoutText->SetText(FText::FromString(FString::Printf(
                TEXT("Stamina %.0f%%   Stealth %s %.0f%%   Armor %d/%d   Mag %d/%d   Reserve %d   Scan %d   Light %d   Bypass %d   %s   %s"),
                Character->MaxStamina > 0.0f ? (Character->Stamina / Character->MaxStamina) * 100.0f : 0.0f,
                *Character->GetStealthStateSummary(),
                Character->GetStealthNoiseLevel() * 100.0f,
                Character->GetArmorPlates(),
                Character->GetMaxArmorPlates(),
                Character->MagazineAmmo,
                CurrentMagazineSize,
                Character->GetActiveWeaponReserveAmmo(),
                Character->GetRadioScannerCharges(),
                Character->GetFlashlightBatteries(),
                Character->GetBypassKits(),
                *ThreatText,
                *EmergencyMedkitText)));
            TacticalReadoutText->SetColorAndOpacity(FSlateColor(
                bThreatClose || bBossPressure || bElitePressure || bStandardPursuitPressure
                    ? ThreatInfo.Accent
                    : FLinearColor(0.92f, 0.84f, 0.62f, 1.0f)));
        }
    }

    // #19 wiring — autosave pip. Shows for 1.0s after every save.
    if (AutosaveText && GI)
    {
        const float Elapsed = World->TimeSeconds - GI->LastSaveWallSeconds;
        if (Elapsed >= 0.0f && Elapsed < 1.0f)
        {
            AutosaveText->SetText(FText::FromString(TEXT("[Saving...]")));
            const float Alpha = FMath::Clamp(1.0f - Elapsed, 0.0f, 1.0f);
            AutosaveText->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 1.0f, 0.4f, Alpha)));
        }
        else
        {
            AutosaveText->SetText(FText::FromString(TEXT("")));
        }
    }
}
