#include "CityFastTravelWidget.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
void MirrorFastTravelThemeFromSettings(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return;
    }

    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
}

UTextBlock* MakeFastTravelLabel(
    UWidgetTree* Tree,
    const FString& Text,
    CodeRescueUI::EType Type,
    const FLinearColor& Color,
    bool bShadow = true)
{
    UTextBlock* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Label->SetText(FText::FromString(Text));
    Label->SetAutoWrapText(true);
    CodeRescueUI::StyleText(Label, Type, Color, bShadow);
    return Label;
}

void StyleFastTravelButton(UButton* Button, bool bPrimary)
{
    if (!Button)
    {
        return;
    }

    if (bPrimary)
    {
        CodeRescueUI::StylePrimaryButton(Button);
    }
    else
    {
        CodeRescueUI::StyleSecondaryButton(Button);
    }
}

FLinearColor FastTravelDebriefColor(bool bExtractionReady, const FLinearColor& ExtractionAccentColor)
{
    if (!bExtractionReady)
    {
        return CodeRescueUI::Color::TextSecondary();
    }

    const FLinearColor AccentBlend = ExtractionAccentColor * 0.62f + CodeRescueUI::Color::TerminalGreenBright() * 0.38f;
    return CodeRescueUI::Resolve(AccentBlend);
}

FString BuildFastTravelSummary(const UCodeRescueGameInstance* GI, bool bExtractionReady)
{
    const FString LanguageText = GI ? GI->GetLanguageName() : TEXT("selected language");
    const FString SaveSlot = GI
        ? UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)
        : TEXT("OperationCodeRescue_Language_<Track>");
    const int32 SolvedCount = GI ? GI->SolvedTerminalIds.Num() : 0;
    const int32 RescueCount = GI ? GI->RescuedSurvivorNames.Num() : 0;
    const FString ModeText = bExtractionReady ? TEXT("extraction debrief") : TEXT("cleared-city redeploy");

    return FString::Printf(
        TEXT("Active language: %s | Start-screen resume slot: %s\nRoute mode: %s | Cleared terminals: %d | Rescued teams: %d\nSelecting a destination teleports the player and saves this language run after arrival."),
        *LanguageText,
        *SaveSlot,
        *ModeText,
        SolvedCount,
        RescueCount);
}

FString BuildDestinationLabel(const UCodeRescueGameInstance* GI, int32 CityIndex, const FCodeRescueCityMission* Mission)
{
    if (!Mission)
    {
        return FString::Printf(TEXT("REDEPLOY | City %d\nTerminal solved. Arrival saves the active language run."), CityIndex + 1);
    }

    const FString LanguageText = GI ? GI->GetLanguageName() : TEXT("selected language");
    const bool bCityComplete = GI && FCodeRescueCampaign::IsCityCompleted(GI, CityIndex);
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const FString CompletionText = bCityComplete
        ? TEXT("10/10 coding clearance + survivor extracted")
        : FString::Printf(TEXT("coding clearance %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity);
    const FString FocusText = Mission->CurriculumFocus.IsEmpty() ? TEXT("field review") : Mission->CurriculumFocus;
    return FString::Printf(
        TEXT("REDEPLOY | %s\n%s | %s\nArrival saves the %s run after fast travel."),
        *FCodeRescueCampaign::GetMissionLabel(CityIndex),
        *CompletionText,
        *FocusText,
        *LanguageText);
}

UButton* MakeFastTravelActionButton(
    UWidgetTree* Tree,
    const FString& Label,
    bool bPrimary,
    const FLinearColor& LabelColor,
    FName Name = NAME_None)
{
    UButton* Button = Name.IsNone()
        ? Tree->ConstructWidget<UButton>(UButton::StaticClass())
        : Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    StyleFastTravelButton(Button, bPrimary);
    UTextBlock* Text = MakeFastTravelLabel(
        Tree,
        TEXT("  ") + Label,
        CodeRescueUI::EType::BodySmall,
        LabelColor);
    Button->AddChild(Text);
    return Button;
}
}

void UCityFastTravelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCityFastTravelWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCityFastTravelWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorFastTravelThemeFromSettings(GI);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("FastTravelRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("FastTravelBlur"));
    Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 1.0f : 4.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BlurSlot->SetOffsets(FMargin(0.0f));

    PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EvacRoutePanel"));
    CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Panel(), FMargin(18.0f, 16.0f));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelFrame);
    PanelSlot->SetAnchors(FAnchors(0.18f, 0.09f, 0.82f, 0.90f));
    PanelSlot->SetOffsets(FMargin(0.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EvacRouteStack"));
    PanelFrame->SetContent(Stack);

    TitleText = MakeFastTravelLabel(
        WidgetTree,
        bSourceExtractionReady
            ? TEXT("EVAC HELI - extraction debrief")
            : TEXT("EVAC HELI - pick a destination city"),
        CodeRescueUI::EType::Title,
        CodeRescueUI::Color::AccentAmber());
    Stack->AddChildToVerticalBox(TitleText);

    DebriefText = MakeFastTravelLabel(
        WidgetTree,
        BuildExtractionDebriefText(GI),
        CodeRescueUI::EType::Body,
        FastTravelDebriefColor(bSourceExtractionReady, ExtractionAccentColor),
        false);
    UVerticalBoxSlot* DebriefStackSlot = Stack->AddChildToVerticalBox(DebriefText);
    DebriefStackSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 6.0f));

    SummaryText = MakeFastTravelLabel(
        WidgetTree,
        BuildFastTravelSummary(GI, bSourceExtractionReady),
        CodeRescueUI::EType::BodySmall,
        CodeRescueUI::Color::TextSecondary(),
        false);
    UVerticalBoxSlot* SummarySlot = Stack->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("EvacRouteScroll"));
    UVerticalBoxSlot* ScrollSlot = Stack->AddChildToVerticalBox(Scroll);
    ScrollSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    DestinationsList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Destinations"));
    Scroll->AddChild(DestinationsList);

    CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseBtn"));
    CodeRescueUI::StyleSecondaryButton(CloseButton);
    CloseButton->OnClicked.AddDynamic(this, &UCityFastTravelWidget::OnCloseClicked);
    UTextBlock* CloseLabel = MakeFastTravelLabel(
        WidgetTree,
        TEXT("Cancel"),
        CodeRescueUI::EType::BodySmall,
        CodeRescueUI::Color::AccentAmber());
    CloseLabel->SetJustification(ETextJustify::Center);
    CloseButton->AddChild(CloseLabel);
    Stack->AddChildToVerticalBox(CloseButton);

    if (!GI)
    {
        return;
    }

    if (bSourceExtractionReady)
    {
        ContinueDestinationCityIndex = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
        if (ContinueDestinationCityIndex >= FCodeRescueCampaign::GetMissionCount())
        {
            ContinueDestinationCityIndex = INDEX_NONE;
        }

        if (ContinueDestinationCityIndex != INDEX_NONE)
        {
            const FCodeRescueCityMission* NextMission = FCodeRescueCampaign::GetMission(ContinueDestinationCityIndex);
            ContinueButton = MakeFastTravelActionButton(
                WidgetTree,
                FString::Printf(
                    TEXT("NEXT OPERATION\nContinue operation: %s\nKeeps the %s language route moving."),
                    NextMission ? *FCodeRescueCampaign::GetMissionLabel(ContinueDestinationCityIndex) : TEXT("next city"),
                    *GI->GetLanguageName()),
                true,
                CodeRescueUI::Color::TerminalGreenBright(),
                FName(TEXT("ContinueExtractionButton")));
            ContinueButton->OnClicked.AddDynamic(this, &UCityFastTravelWidget::OnContinueExtractionClicked);
            UVerticalBoxSlot* ContinueSlot = DestinationsList->AddChildToVerticalBox(ContinueButton);
            if (ContinueSlot)
            {
                ContinueSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, CodeRescueUI::Space::L));
            }
        }
    }

    const int32 NumCities = FCodeRescueCampaign::GetMissionCount();
    int32 AddedDestinations = 0;
    for (int32 i = 0; i < NumCities; ++i)
    {
        const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(i);
        if (!Mission || !FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, i))
        {
            continue;
        }

        UButton* DestButton = MakeFastTravelActionButton(
            WidgetTree,
            BuildDestinationLabel(GI, i, Mission),
            false,
            FCodeRescueCampaign::IsCityCompleted(GI, i)
                ? CodeRescueUI::Color::TerminalGreenBright()
                : CodeRescueUI::Color::AccentAmber());
        DestButton->OnClicked.AddDynamic(this, &UCityFastTravelWidget::OnDestinationClicked);
        UVerticalBoxSlot* RowSlot = DestinationsList->AddChildToVerticalBox(DestButton);
        if (RowSlot)
        {
            RowSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS));
        }
        ButtonToCityIndex.Add(DestButton, i);
        ++AddedDestinations;
    }

    if (AddedDestinations == 0)
    {
        UTextBlock* Empty = MakeFastTravelLabel(
            WidgetTree,
            TEXT("NO CLEARED DESTINATIONS\nSolve a coding terminal first; cleared cities appear here for redeploy."),
            CodeRescueUI::EType::BodySmall,
            CodeRescueUI::Color::Warning(),
            false);
        DestinationsList->AddChildToVerticalBox(Empty);
    }

    ACodeRescueCharacter::SetUIOpen(true);
}

void UCityFastTravelWidget::ConfigureOpeningHelipadContext(
    int32 InSourceCityIndex,
    const FString& InSourceCityLabel,
    bool bInExtractionReady,
    const FString& InSurvivorName,
    const FLinearColor& InAccentColor)
{
    SourceCityIndex = InSourceCityIndex;
    SourceCityLabel = InSourceCityLabel;
    bSourceExtractionReady = bInExtractionReady;
    ExtractionSurvivorName = InSurvivorName;
    ExtractionAccentColor = InAccentColor;
}

void UCityFastTravelWidget::NativeDestruct()
{
    ACodeRescueCharacter::SetUIOpen(false);
    Super::NativeDestruct();
}

void UCityFastTravelWidget::OnCloseClicked()
{
    Close();
}

void UCityFastTravelWidget::OnDestinationClicked()
{
    // Walk the map to find which button fired. Slate doesn't pass sender data
    // by default for dynamic delegates, so we identify by hover state /
    // active-focus widget — but the simplest correct approach here is to
    // match all buttons whose IsHovered() is currently true (the click that
    // just fired implies hover). Falls back to the first solved city if
    // detection fails.
    int32 DestIndex = INDEX_NONE;
    for (const TPair<UButton*, int32>& Pair : ButtonToCityIndex)
    {
        if (Pair.Key && Pair.Key->IsHovered())
        {
            DestIndex = Pair.Value;
            break;
        }
    }

    if (DestIndex == INDEX_NONE && ButtonToCityIndex.Num() > 0)
    {
        for (const TPair<UButton*, int32>& Pair : ButtonToCityIndex)
        {
            DestIndex = Pair.Value;
            break;
        }
    }

    if (DestIndex != INDEX_NONE)
    {
        TeleportPlayerToCity(DestIndex);
    }
    Close();
}

void UCityFastTravelWidget::OnContinueExtractionClicked()
{
    if (ContinueDestinationCityIndex != INDEX_NONE)
    {
        TeleportPlayerToCity(ContinueDestinationCityIndex);
    }
    Close();
}

FString UCityFastTravelWidget::BuildExtractionDebriefText(const UCodeRescueGameInstance* GI) const
{
    if (!bSourceExtractionReady)
    {
        return TEXT("Cleared and solved cities are available for field redeployment.");
    }

    const FString CityText = SourceCityLabel.IsEmpty()
        ? FCodeRescueCampaign::GetMissionLabel(SourceCityIndex)
        : SourceCityLabel;
    const FString SurvivorText = ExtractionSurvivorName.IsEmpty()
        ? TEXT("survivor")
        : ExtractionSurvivorName;
    const FString LanguageText = GI ? GI->GetLanguageName() : TEXT("selected language");

    return FString::Printf(
        TEXT("Extraction confirmed: %s rescued from %s.\n%s progression is saved. Choose the next operation or redeploy to a cleared city."),
        *SurvivorText,
        *CityText,
        *LanguageText);
}

void UCityFastTravelWidget::TeleportPlayerToCity(int32 CityIndex)
{
    UWorld* World = GetWorld();
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!Pawn || !PC) return;

    const FVector Dest = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    Pawn->TeleportTo(Dest, Pawn->GetActorRotation());

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SavePersistentRun();
    }

    if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
    {
        CamMgr->StartCameraFade(1.0f, 0.0f, 0.4f, FLinearColor::Black, false, true);
    }
}

void UCityFastTravelWidget::Close()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
    }
    RemoveFromParent();
}
