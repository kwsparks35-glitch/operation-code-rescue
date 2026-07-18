#include "CodeRescueObjectiveJournalWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

namespace
{
void MirrorJournalThemeFromSettings(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return;
    }

    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
}

FLinearColor JournalStateColor(bool bDone, bool bActive, bool bUnlocked, bool bHighContrast)
{
    if (bDone)
    {
        return bHighContrast
            ? FLinearColor(0.72f, 1.0f, 0.68f, 1.0f)
            : FLinearColor(0.55f, 0.95f, 0.55f, 1.0f);
    }
    if (bActive)
    {
        return bHighContrast
            ? FLinearColor(1.0f, 0.96f, 0.18f, 1.0f)
            : FLinearColor(1.0f, 0.82f, 0.28f, 1.0f);
    }
    if (bUnlocked)
    {
        return bHighContrast
            ? FLinearColor::White
            : CodeRescueUI::Color::TextPrimary();
    }
    return bHighContrast
        ? FLinearColor(0.80f, 0.80f, 0.88f, 1.0f)
        : CodeRescueUI::Color::TextMuted();
}

FString JournalStateLabel(bool bDone, bool bActive, bool bUnlocked)
{
    if (bDone) return TEXT("DONE");
    if (bActive) return TEXT("ACTIVE");
    if (bUnlocked) return TEXT("OPEN");
    return TEXT("LOCKED");
}

const FCodeRescueMissionProgress* FindJournalMissionProgress(const UCodeRescueGameInstance* GI, const FString& MissionId)
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

FString SurvivorIntelStatusLabel(bool bTerminalSolved, bool bSurvivorRescued)
{
    if (bSurvivorRescued)
    {
        return TEXT("RESCUED - extraction debrief ready");
    }
    if (bTerminalSolved)
    {
        return TEXT("ROUTE OPEN - survivor marker broadcasting");
    }
    return TEXT("LOCKED - solve terminal to decrypt");
}

FString SurvivorIntelNextStep(bool bTerminalSolved, bool bSurvivorRescued)
{
    if (bSurvivorRescued)
    {
        return TEXT("move to extraction, then review the debrief and next operation");
    }
    if (bTerminalSolved)
    {
        return TEXT("follow the cyan survivor marker and keep squad support close");
    }
    return TEXT("enter the protected coding safehouse and validate the terminal");
}

FString BuildLanguageSaveContinuityLine(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return TEXT("LANGUAGE SAVE\nTrack: unavailable | Save slot: profile not loaded");
    }

    const FString LanguageName = GI->GetLanguageName();
    const bool bSaveExists = GI->DoesLanguageSaveExist(GI->SelectedLanguage);
    return FString::Printf(
        TEXT("LANGUAGE SAVE\nTrack: %s only | Slot: %s | %s\nStart screen remains available; choose Resume %s to continue this saved language run."),
        *LanguageName,
        *GI->SaveSlotName,
        bSaveExists ? TEXT("save present") : TEXT("new run - save when ready"),
        *LanguageName);
}

FString BuildRouteMapReadout(
    const FCodeRescueCityMission* Mission,
    const UCodeRescueGameInstance* GI,
    int32 CityIndex,
    int32 CompletedCount,
    int32 TotalMissions)
{
    if (!Mission)
    {
        return FString::Printf(
            TEXT("ROUTE MAP\nCampaign complete | Cities graduated %d / %d\nAll terminal, survivor, and extraction routes are clear for this language run."),
            CompletedCount,
            TotalMissions);
    }

    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bSurvivorRescued = GI && GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
    const FString TerminalState = FString::Printf(
        TEXT("coding clearance %d/%d"),
        CompletedChallenges,
        FCodeRescueCampaign::RequiredChallengesPerCity);
    const FString SurvivorState = bSurvivorRescued ? TEXT("survivor rescued") : TEXT("survivor intel pending");
    const FString NextMarker = !bTerminalSolved
        ? FString::Printf(TEXT("protected %s coding terminal"), GI ? *GI->GetLanguageName() : TEXT("selected-language"))
        : (!bSurvivorRescued ? TEXT("cyan survivor marker") : TEXT("helipad extraction/debrief"));
    const FString CaseFileLine = GI
        ? GI->GetCaseFileCollectionSummary()
        : FString(TEXT("Case files: profile unavailable"));

    return FString::Printf(
        TEXT("ROUTE MAP\n%03d %s, %s | %s | %s\nIntel: %s | %s | %s\nNext marker: %s"),
        Mission->Rank,
        *Mission->CityName,
        *Mission->StateName,
        *Mission->RegionName,
        *Mission->LandmarkName,
        *TerminalState,
        *SurvivorState,
        *CaseFileLine,
        *NextMarker);
}

FString BuildInventoryReadout(const ACodeRescueCharacter* Character, const UCodeRescueGameInstance* GI)
{
    if (!Character)
    {
        return TEXT("FIELD INVENTORY\nOpen during active play to inspect weapon, ammo, throwables, armor, scanner, flashlight, bypass kits, scrap, and research.");
    }

    return FString::Printf(
        TEXT("FIELD INVENTORY\nWeapon: %s | Ammo %d/%d + %d | Role: %s | Weapons %d slot(s)\nGear: armor %d/%d | medkits %d/%d | scanner %d/%d | light %d/%d | bypass %d/%d | flares %d smokes %d stims %d | pouch +%d | scrap %d | research %d"),
        *Character->GetActiveWeaponName(),
        Character->MagazineAmmo,
        Character->GetActiveWeaponMagazineSize(),
        Character->GetActiveWeaponReserveAmmo(),
        *Character->GetActiveWeaponTacticalRole(),
        FMath::Max(1, Character->GetWeaponCount()),
        Character->GetArmorPlates(),
        Character->GetMaxArmorPlates(),
        Character->Medkits,
        Character->MaxMedkits,
        Character->GetRadioScannerCharges(),
        Character->GetMaxRadioScannerCharges(),
        Character->GetFlashlightBatteries(),
        Character->GetMaxFlashlightBatteries(),
        Character->GetBypassKits(),
        Character->GetMaxBypassKits(),
        Character->GetThrowableCountForSlot(0),
        Character->GetThrowableCountForSlot(1),
        Character->GetThrowableCountForSlot(2),
        Character->GetAmmoPouchCapacityBonus(),
        Character->GetScrap(),
        GI ? GI->ResearchPoints : 0);
}

FString BuildSurvivorIntelDossier(
    const FCodeRescueCityMission* Mission,
    const UCodeRescueGameInstance* GI,
    int32 CityIndex)
{
    if (!Mission)
    {
        return TEXT("SURVIVOR INTEL DOSSIER\nStatus: campaign complete\nAll survivor routes are clear; extraction/debrief review is ready.");
    }

    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bSurvivorRescued = GI && GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
    const FCodeRescueMissionProgress* Progress = FindJournalMissionProgress(GI, Mission->TerminalId);
    const FString ValidationLine = Progress
        ? FString::Printf(TEXT("Validation: %d attempt(s), best score %d, terminal %s"),
            Progress->Attempts,
            Progress->BestScore,
            Progress->bCompleted ? TEXT("complete") : TEXT("in repair"))
        : TEXT("Validation: no attempts recorded yet");
    const FString LanguageLine = GI
        ? FString::Printf(TEXT("Language run: %s | %s"), *GI->GetLanguageName(), *GI->GetLanguageProgressSummary())
        : TEXT("Language run: profile unavailable");
    const FCodeRescueSurvivorArchetypeProfile Archetype = FCodeRescueCampaign::GetSurvivorArchetypeProfile(*Mission);

    return FString::Printf(
        TEXT("SURVIVOR INTEL DOSSIER\nStatus: %s | Coding clearance %d/%d\nContact: %s | %03d %s, %s\nRole: %s [%s]\nNeed: %s\nRescue value: %s\nLocation: %s / %s\nLesson payoff: %s\nDossier: %s\n%s\n%s\nNext step: %s"),
        *SurvivorIntelStatusLabel(bTerminalSolved, bSurvivorRescued),
        CompletedChallenges,
        FCodeRescueCampaign::RequiredChallengesPerCity,
        *Mission->SurvivorName,
        Mission->Rank,
        *Mission->CityName,
        *Mission->StateName,
        *Archetype.Title,
        *Archetype.IconLabel,
        *Archetype.FieldNeed,
        *Archetype.RescueSkill,
        *Mission->LandmarkName,
        *Mission->DistrictStyle,
        *Mission->NovelGameplayDetail,
        *Archetype.DossierHook,
        *ValidationLine,
        *LanguageLine,
        *SurvivorIntelNextStep(bTerminalSolved, bSurvivorRescued));
}
}

void UCodeRescueObjectiveJournalWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueObjectiveJournalWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueObjectiveJournalWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;

    MirrorJournalThemeFromSettings(GetGameInstance<UCodeRescueGameInstance>());

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("JournalRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("JournalBlur"));
    Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 1.0f : 3.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0.54f, 0.08f, 0.98f, 0.88f));
    BlurSlot->SetOffsets(FMargin(0, 0, 0, 0));

    PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("JournalPanelBorder"));
    CodeRescueUI::StylePanel(PanelBorder, CodeRescueUI::Surface::Panel(), FMargin(16.0f, 14.0f));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelBorder);
    PanelSlot->SetAnchors(FAnchors(0.55f, 0.09f, 0.97f, 0.86f));
    PanelSlot->SetOffsets(FMargin(0, 0, 0, 0));

    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("JournalPanel"));
    PanelBorder->SetContent(Box);

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("JournalTitle"));
    TitleText->SetText(FText::FromString(TEXT("OPERATION ROUTE JOURNAL")));
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Heading, CodeRescueUI::Color::AccentAmber());
    Box->AddChildToVerticalBox(TitleText);

    SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("JournalSummary"));
    SummaryText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary());
    UVerticalBoxSlot* SummarySlot = Box->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0, 6, 0, 8));

    LanguageSaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LanguageSaveContinuityText"));
    LanguageSaveText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(LanguageSaveText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentAmber(), false);
    UVerticalBoxSlot* LanguageSlot = Box->AddChildToVerticalBox(LanguageSaveText);
    LanguageSlot->SetPadding(FMargin(0, 0, 0, 7));

    LanguageProfileRecapText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LanguageProfileRecapText"));
    LanguageProfileRecapText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(LanguageProfileRecapText, CodeRescueUI::EType::BodySmall, FLinearColor(0.78f, 0.86f, 1.0f, 1.0f), false);
    UVerticalBoxSlot* ProfileSlot = Box->AddChildToVerticalBox(LanguageProfileRecapText);
    ProfileSlot->SetPadding(FMargin(0, 0, 0, 7));

    // 2026-07-04 (item 32): concept mastery meter — text bars, refreshed on open.
    ConceptMasteryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ConceptMasteryMeterText"));
    ConceptMasteryText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(ConceptMasteryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreenBright(), false);
    UVerticalBoxSlot* MasterySlot = Box->AddChildToVerticalBox(ConceptMasteryText);
    MasterySlot->SetPadding(FMargin(0, 0, 0, 7));

    LearningDebriefText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LearningDebriefReadoutText"));
    LearningDebriefText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(LearningDebriefText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreen(), false);
    UVerticalBoxSlot* DebriefSlot = Box->AddChildToVerticalBox(LearningDebriefText);
    DebriefSlot->SetPadding(FMargin(0, 0, 0, 7));

    ChallengeReplayText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ChallengeReplayBriefText"));
    ChallengeReplayText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(ChallengeReplayText, CodeRescueUI::EType::BodySmall, FLinearColor(0.44f, 0.86f, 0.96f, 1.0f), false);
    UVerticalBoxSlot* ReplaySlot = Box->AddChildToVerticalBox(ChallengeReplayText);
    ReplaySlot->SetPadding(FMargin(0, 0, 0, 7));

    FailSafeObjectiveBoardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FailSafeObjectiveBoardText"));
    FailSafeObjectiveBoardText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(FailSafeObjectiveBoardText, CodeRescueUI::EType::BodySmall, FLinearColor(1.0f, 0.93f, 0.52f, 1.0f), false);
    UVerticalBoxSlot* FailSafeSlot = Box->AddChildToVerticalBox(FailSafeObjectiveBoardText);
    FailSafeSlot->SetPadding(FMargin(0, 0, 0, 7));

    RouteMapText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RouteMapReadoutText"));
    RouteMapText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(RouteMapText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
    UVerticalBoxSlot* RouteSlot = Box->AddChildToVerticalBox(RouteMapText);
    RouteSlot->SetPadding(FMargin(0, 0, 0, 7));

    InventoryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FieldInventoryReadoutText"));
    InventoryText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(InventoryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextPrimary(), false);
    UVerticalBoxSlot* InventorySlot = Box->AddChildToVerticalBox(InventoryText);
    InventorySlot->SetPadding(FMargin(0, 0, 0, 7));

    IntelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SurvivorIntelDossierText"));
    IntelText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(IntelText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreen(), false);
    UVerticalBoxSlot* IntelSlot = Box->AddChildToVerticalBox(IntelText);
    IntelSlot->SetPadding(FMargin(0, 0, 0, 8));

    IntelArchiveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SurvivorIntelArchiveText"));
    IntelArchiveText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(IntelArchiveText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
    UVerticalBoxSlot* IntelArchiveSlot = Box->AddChildToVerticalBox(IntelArchiveText);
    IntelArchiveSlot->SetPadding(FMargin(0, 0, 0, 8));

    MissionScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("MissionScrollBox"));
    UVerticalBoxSlot* ScrollSlot = Box->AddChildToVerticalBox(MissionScrollBox);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0, 4, 0, 0));

    RowBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("JournalRows"));
    MissionScrollBox->AddChild(RowBox);

    RefreshJournal();
}

void UCodeRescueObjectiveJournalWidget::RefreshJournal()
{
    if (!RowBox)
    {
        return;
    }

    RowBox->ClearChildren();

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorJournalThemeFromSettings(GI);

    // 2026-07-04 (item 32): render the per-concept mastery meter from saved progress.
    if (ConceptMasteryText && GI)
    {
        if (GI->ConceptProgress.Num() == 0)
        {
            ConceptMasteryText->SetText(FText::FromString(
                TEXT("CONCEPT MASTERY: no attempts logged yet - solve a terminal to start the meter.")));
        }
        else
        {
            TArray<FCodeRescueConceptProgress> Sorted = GI->ConceptProgress;
            Sorted.Sort([](const FCodeRescueConceptProgress& A, const FCodeRescueConceptProgress& B)
            {
                return (A.SuccessfulValidations + A.FailedValidations)
                     > (B.SuccessfulValidations + B.FailedValidations);
            });
            FString Meter = TEXT("CONCEPT MASTERY (solves / attempts):");
            const int32 MaxRows = FMath::Min(8, Sorted.Num());
            for (int32 Row = 0; Row < MaxRows; ++Row)
            {
                const FCodeRescueConceptProgress& P = Sorted[Row];
                const int32 Attempts = P.SuccessfulValidations + P.FailedValidations;
                const float Ratio = Attempts > 0
                    ? static_cast<float>(P.SuccessfulValidations) / static_cast<float>(Attempts) : 0.0f;
                const int32 Filled = FMath::Clamp(FMath::RoundToInt(Ratio * 10.0f), 0, 10);
                FString Bar;
                for (int32 Seg = 0; Seg < 10; ++Seg)
                {
                    Bar += (Seg < Filled) ? TEXT("#") : TEXT("-");
                }
                Meter += FString::Printf(TEXT("\n  %-28s [%s] %d%%  (%d/%d)"),
                    *P.ConceptId.Left(28), *Bar, FMath::RoundToInt(Ratio * 100.0f),
                    P.SuccessfulValidations, Attempts);
            }
            if (Sorted.Num() > MaxRows)
            {
                Meter += FString::Printf(TEXT("\n  ... and %d more concepts in telemetry."), Sorted.Num() - MaxRows);
            }
            ConceptMasteryText->SetText(FText::FromString(Meter));
        }
    }

    const int32 FirstIncomplete = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    const FCodeRescueCityMission* ActiveMission = Missions.IsValidIndex(FirstIncomplete) ? &Missions[FirstIncomplete] : nullptr;
    const ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

    int32 CompletedCount = 0;
    for (int32 i = 0; i < Missions.Num(); ++i)
    {
        if (FCodeRescueCampaign::IsCityCompleted(GI, i))
        {
            ++CompletedCount;
        }
    }

    if (PanelBorder)
    {
        CodeRescueUI::StylePanel(PanelBorder, CodeRescueUI::Surface::Panel(), FMargin(16.0f, 14.0f));
    }
    if (TitleText)
    {
        CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Heading, CodeRescueUI::Color::AccentAmber());
    }
    if (SummaryText)
    {
        FString Phase = TEXT("Campaign complete - extraction/debrief ready");
        if (ActiveMission)
        {
            const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, FirstIncomplete);
            const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, FirstIncomplete);
            const bool bSurvivorRescued = GI && GI->RescuedSurvivorNames.Contains(ActiveMission->SurvivorName);
            if (!bTerminalSolved)
            {
                Phase = FString::Printf(
                    TEXT("Complete coding concourse %d/%d"),
                    CompletedChallenges,
                    FCodeRescueCampaign::RequiredChallengesPerCity);
            }
            else if (!bSurvivorRescued)
            {
                Phase = FString::Printf(TEXT("Rescue %s"), *ActiveMission->SurvivorName);
            }
            else
            {
                Phase = TEXT("Extract and debrief");
            }
        }

        const FString ActiveLine = ActiveMission
            ? FString::Printf(TEXT("Active: %03d. %s, %s | %s"),
                ActiveMission->Rank,
                *ActiveMission->CityName,
                *ActiveMission->StateName,
                *Phase)
            : FString(TEXT("Campaign complete | All rescue routes clear"));
        const FString LanguageLine = GI
            ? FString::Printf(TEXT("Language: %s | Progress: %d / %d | %s"),
                *GI->GetLanguageName(),
                CompletedCount,
                Missions.Num(),
                *GI->GetAccessibilitySummary())
            : FString::Printf(TEXT("Language: not selected | Progress: %d / %d"), CompletedCount, Missions.Num());
        const FString CaseFileLine = GI
            ? GI->GetCaseFileCollectionSummary()
            : FString(TEXT("Case files: profile unavailable"));
        SummaryText->SetText(FText::FromString(ActiveLine + TEXT("\n") + LanguageLine + TEXT("\n") + CaseFileLine));
        SummaryText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary());
    }
    if (LanguageSaveText)
    {
        LanguageSaveText->SetText(FText::FromString(BuildLanguageSaveContinuityLine(GI)));
        LanguageSaveText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            LanguageSaveText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor(1.0f, 0.96f, 0.22f, 1.0f) : CodeRescueUI::Color::AccentAmber(),
            false);
    }
    if (LanguageProfileRecapText)
    {
        LanguageProfileRecapText->SetText(FText::FromString(GI
            ? GI->GetLanguageProfileRecapSummary()
            : FString(TEXT("LANGUAGE PROFILE RECAP\nProfile unavailable. Resume a selected-language run to review stage recap, profile stats, recommendation, and save-slot preview."))));
        LanguageProfileRecapText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            LanguageProfileRecapText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor(0.82f, 0.90f, 1.0f, 1.0f) : FLinearColor(0.78f, 0.86f, 1.0f, 1.0f),
            false);
    }
    if (LearningDebriefText)
    {
        LearningDebriefText->SetText(FText::FromString(GI
            ? GI->GetLearningDebriefJournalSummary()
            : FString(TEXT("LAST LEARNING DEBRIEF\nProfile unavailable. Resume a selected-language run to review the latest terminal learning takeaway."))));
        LearningDebriefText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            LearningDebriefText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor(0.72f, 1.0f, 0.68f, 1.0f) : CodeRescueUI::Color::TerminalGreen(),
            false);
    }
    if (ChallengeReplayText)
    {
        ChallengeReplayText->SetText(FText::FromString(GI
            ? GI->GetChallengeReplayJournalSummary()
            : FString(TEXT("CHALLENGE REPLAY BRIEF\nProfile unavailable. Resume a selected-language run to review saved visible goals, hidden-test replay notes, and practice actions."))));
        ChallengeReplayText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            ChallengeReplayText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor(0.58f, 0.96f, 1.0f, 1.0f) : FLinearColor(0.44f, 0.86f, 0.96f, 1.0f),
            false);
    }
    if (FailSafeObjectiveBoardText)
    {
        FailSafeObjectiveBoardText->SetText(FText::FromString(GI
            ? GI->GetFailSafeObjectiveBoardSummary()
            : FString(TEXT("FAIL-SAFE OBJECTIVE BOARD\nProfile unavailable. Resume a selected-language run to review active route, return markers, recovery controls, and start-screen Resume state."))));
        FailSafeObjectiveBoardText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            FailSafeObjectiveBoardText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor(1.0f, 0.98f, 0.42f, 1.0f) : FLinearColor(1.0f, 0.93f, 0.52f, 1.0f),
            false);
    }
    if (RouteMapText)
    {
        RouteMapText->SetText(FText::FromString(BuildRouteMapReadout(ActiveMission, GI, FirstIncomplete, CompletedCount, Missions.Num())));
        RouteMapText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            RouteMapText,
            CodeRescueUI::EType::BodySmall,
            CodeRescueUI::Color::TextSecondary(),
            false);
    }
    if (InventoryText)
    {
        InventoryText->SetText(FText::FromString(BuildInventoryReadout(Character, GI)));
        InventoryText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            InventoryText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor::White : CodeRescueUI::Color::TextPrimary(),
            false);
    }
    if (IntelText)
    {
        const bool bTerminalSolved = ActiveMission && FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, FirstIncomplete);
        const bool bSurvivorRescued = ActiveMission && GI && GI->RescuedSurvivorNames.Contains(ActiveMission->SurvivorName);
        IntelText->SetText(FText::FromString(BuildSurvivorIntelDossier(ActiveMission, GI, FirstIncomplete)));
        IntelText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            IntelText,
            CodeRescueUI::EType::BodySmall,
            JournalStateColor(bSurvivorRescued, bTerminalSolved, ActiveMission != nullptr, GI && GI->bHighContrastHUD),
            false);
    }
    if (IntelArchiveText)
    {
        IntelArchiveText->SetText(FText::FromString(GI
            ? GI->GetSurvivorIntelArchiveSummary()
            : FString(TEXT("SURVIVOR INTEL ARCHIVE\nProfile unavailable. Resume a selected-language run to review the latest uploaded survivor intel."))));
        IntelArchiveText->SetAutoWrapText(true);
        CodeRescueUI::StyleText(
            IntelArchiveText,
            CodeRescueUI::EType::BodySmall,
            GI && GI->bHighContrastHUD ? FLinearColor::White : CodeRescueUI::Color::TextSecondary(),
            false);
    }

    UTextBlock* ActiveRow = nullptr;
    for (int32 i = 0; i < Missions.Num(); ++i)
    {
        const FCodeRescueCityMission& Mission = Missions[i];
        const bool bDone = FCodeRescueCampaign::IsCityCompleted(GI, i);
        const bool bActive = i == FirstIncomplete;
        const bool bUnlocked = FCodeRescueCampaign::IsCityUnlocked(GI, i);

        UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("Row_%d"), i));
        const FString StateLabel = JournalStateLabel(bDone, bActive, bUnlocked);
        const FString PhaseLabel = bDone
            ? TEXT("Terminal cleared + survivor rescued")
            : bActive
                ? (FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, i)
                    ? TEXT("Survivor / extraction route")
                    : TEXT("Coding terminal route"))
                : bUnlocked
                    ? TEXT("Unlocked next route")
                    : TEXT("Locked until prior city graduation");

        Row->SetText(FText::FromString(FString::Printf(TEXT("[%s] %03d. %s, %s | Tier %d | %s | %s | %s"),
            *StateLabel,
            Mission.Rank,
            *Mission.CityName,
            *Mission.StateName,
            Mission.DifficultyTier,
            *Mission.RegionName,
            *Mission.TerminalTitle,
            *PhaseLabel)));
        Row->SetAutoWrapText(true);
        Row->SetColorAndOpacity(FSlateColor(JournalStateColor(bDone, bActive, bUnlocked, GI && GI->bHighContrastHUD)));
        Row->SetShadowOffset(FVector2D(1.0f, 1.0f));
        Row->SetShadowColorAndOpacity(CodeRescueUI::Color::Shadow());
        FSlateFontInfo RowFont = Row->GetFont();
        RowFont.Size = CodeRescueUI::ScaledSize(CodeRescueUI::EType::BodySmall);
        Row->SetFont(RowFont);
        UVerticalBoxSlot* RowSlot = RowBox->AddChildToVerticalBox(Row);
        RowSlot->SetPadding(FMargin(0, 1, 0, 1));
        if (bActive)
        {
            ActiveRow = Row;
        }
    }

    if (MissionScrollBox && ActiveRow)
    {
        MissionScrollBox->ScrollWidgetIntoView(ActiveRow);
    }
}
