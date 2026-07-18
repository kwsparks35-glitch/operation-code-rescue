#include "CodeRescueVictoryWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueLeaderboards.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
    void MirrorVictoryThemeFromSettings(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return;
        }

        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    FString FormatVictoryRunTime(float RunSeconds)
    {
        const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(RunSeconds));
        return FString::Printf(TEXT("%02d:%02d:%02d"), TotalSeconds / 3600, (TotalSeconds / 60) % 60, TotalSeconds % 60);
    }

    FString BuildVictoryLanguageSummary(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return TEXT("Language run unavailable | Resume slot unavailable");
        }

        return FString::Printf(
            TEXT("Completed language: %s | Start-screen resume: %s\nLanguage progress: %s"),
            *GI->GetLanguageName(),
            *UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage),
            *GI->GetLanguageProgressSummary());
    }

    FString BuildVictoryStatsText(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return TEXT("Run summary unavailable.");
        }

        return FString::Printf(
            TEXT("Survivors rescued: %d\nTerminals solved: %d\nZombies neutralized: %d\nCoding score: %d\nResearch points remaining: %d\nRun time: %s | Deaths: %d | Headshots: %d"),
            GI->SurvivorsRescued,
            GI->TerminalsSolved,
            GI->ZombiesNeutralized,
            GI->CodingScore,
            GI->ResearchPoints,
            *FormatVictoryRunTime(GI->RunSeconds),
            GI->DeathCount,
            GI->HeadshotCount);
    }
}

void UCodeRescueVictoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueVictoryWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueVictoryWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorVictoryThemeFromSettings(GI);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("VictoryRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("VictoryBlur"));
    Blur->SetBlurStrength((GI && GI->bReducedMotion) ? 3.0f : 10.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BlurSlot->SetOffsets(FMargin(0, 0, 0, 0));

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeVictoryBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.012f, 0.014f, 0.012f, 0.68f));
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BackdropSlot->SetOffsets(FMargin(0));

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeVictoryPanelFrame"));
    CodeRescueUI::StylePanel(Panel, CodeRescueUI::Surface::Panel());
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.20f, 0.12f, 0.80f, 0.86f));
    PanelSlot->SetOffsets(FMargin(0));

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("VictoryEndStateScroll"));
    Panel->SetContent(Scroll);

    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VictoryPanel"));
    Scroll->AddChild(Box);

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryTitle"));
    TitleText->SetText(FText::FromString(TEXT("EXTRACTION COMPLETE\nOperation Code Rescue")));
    TitleText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    Box->AddChildToVerticalBox(TitleText);

    SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryLanguageSummary"));
    SummaryText->SetText(FText::FromString(BuildVictoryLanguageSummary(GI)));
    SummaryText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreenBright());
    UVerticalBoxSlot* SummarySlot = Box->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0, 10, 0, 12));

    StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryStats"));
    StatsText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(StatsText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextPrimary());
    StatsText->SetText(FText::FromString(BuildVictoryStatsText(GI)));
    if (GI)
    {
        // #66: submit run results to the local leaderboards (top-10 per kind).
        // Player name defaults to "Operative" when no profile name is set.
        const FString PlayerName = TEXT("Operative");
        // Fastest five-city clear: lower-is-better metric. Stored as the negated
        // run-second count so the descending Score sort still puts faster runs
        // at the top; PrettyPrint negates back when displaying. Only submit if
        // the player has finished at least 5 cities (TerminalSolveCount >= 5).
        if (GI->TerminalSolveCount >= 5 && GI->RunSeconds > 0.0f)
        {
            const int64 NegatedSeconds = static_cast<int64>(-FMath::RoundToFloat(GI->RunSeconds));
            UCodeRescueLeaderboards::Submit(ELeaderboardKind::FastestFiveCity, PlayerName, NegatedSeconds);
        }
        UCodeRescueLeaderboards::Submit(ELeaderboardKind::MostRescues,   PlayerName, GI->RescueCount);
        UCodeRescueLeaderboards::Submit(ELeaderboardKind::MostHeadshots, PlayerName, GI->HeadshotCount);
        // LongestNoResupply isn't tracked yet — submit total kill count as a
        // safe stand-in so the leaderboard table populates with each victory.
        UCodeRescueLeaderboards::Submit(ELeaderboardKind::LongestNoResupply, PlayerName, GI->KillCount);

        // Preserve the completed language run for the start-screen resume list.
        GI->SavePersistentRun();
    }
    UVerticalBoxSlot* StatsSlot = Box->AddChildToVerticalBox(StatsText);
    StatsSlot->SetPadding(FMargin(0, 16, 0, 16));

    RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("VictoryRestart"));
    CodeRescueUI::StyleSecondaryButton(RestartButton);
    UTextBlock* RestartLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryRestart_Label"));
    RestartLabel->SetText(FText::FromString(TEXT("START NEW LANGUAGE RUN")));
    RestartLabel->SetAutoWrapText(true);
    CodeRescueUI::StyleText(RestartLabel, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::AccentAmber());
    RestartButton->AddChild(RestartLabel);
    RestartButton->OnClicked.AddDynamic(this, &UCodeRescueVictoryWidget::OnRestartClicked);
    Box->AddChildToVerticalBox(RestartButton);

    QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("VictoryQuit"));
    CodeRescueUI::StyleSecondaryButton(QuitButton);
    UTextBlock* QuitLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryQuit_Label"));
    QuitLabel->SetText(FText::FromString(TEXT("SAVE COMPLETION AND QUIT")));
    QuitLabel->SetAutoWrapText(true);
    CodeRescueUI::StyleText(QuitLabel, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::AccentEmber());
    QuitButton->AddChild(QuitLabel);
    QuitButton->OnClicked.AddDynamic(this, &UCodeRescueVictoryWidget::OnQuitClicked);
    Box->AddChildToVerticalBox(QuitButton);

    // Lock input + pause world while showing the end screen.
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly UIOnly;
        UIOnly.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(UIOnly);
        PC->bShowMouseCursor = true;
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
    }
    ACodeRescueCharacter::SetUIOpen(true);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void UCodeRescueVictoryWidget::OnRestartClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->DeletePersistentRun();
    }
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

void UCodeRescueVictoryWidget::OnQuitClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SavePersistentRun();
    }
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCodeRescueVictoryWidget::NativeDestruct()
{
    // Safety: clear UI lock + unpause if widget tears down via an unexpected
    // path (level reload from another button, etc.).
    ACodeRescueCharacter::SetUIOpen(false);
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, false);
    }
    Super::NativeDestruct();
}
