#include "CodeRescueDeathWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
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
    void MirrorDeathThemeFromSettings(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return;
        }

        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    FString FormatDeathRunTime(float RunSeconds)
    {
        const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(RunSeconds));
        return FString::Printf(TEXT("%02d:%02d:%02d"), TotalSeconds / 3600, (TotalSeconds / 60) % 60, TotalSeconds % 60);
    }

    FString BuildDeathLanguageSummary(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return TEXT("Language run unavailable | Resume slot unavailable");
        }

        return FString::Printf(
            TEXT("Active language: %s | Start-screen resume: %s\nLanguage progress: %s"),
            *GI->GetLanguageName(),
            *UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage),
            *GI->GetLanguageProgressSummary());
    }

    FString BuildDeathStatsText(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return TEXT("Run progress before death is unavailable.");
        }

        return FString::Printf(
            TEXT("Run progress before death:\n  Survivors rescued: %d\n  Terminals solved: %d\n  Zombies neutralized: %d\n  Coding score: %d\n  Research points: %d\n  Run time: %s | Deaths: %d | Headshots: %d"),
            GI->SurvivorsRescued,
            GI->TerminalsSolved,
            GI->ZombiesNeutralized,
            GI->CodingScore,
            GI->ResearchPoints,
            *FormatDeathRunTime(GI->RunSeconds),
            GI->DeathCount,
            GI->HeadshotCount);
    }

    FString BuildDeathActionStatus(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return TEXT("Recovery checkpoint unavailable. Use restart or quit from this death screen.");
        }

        return FString::Printf(
            TEXT("Playable recovery checkpoint: %s | Resume replays from the current city entry with %s only. Save + Quit keeps this checkpoint for the start screen."),
            *UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage),
            *GI->GetLanguageName());
    }
}

void UCodeRescueDeathWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueDeathWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueDeathWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorDeathThemeFromSettings(GI);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DeathRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("DeathBlur"));
    Blur->SetBlurStrength((GI && GI->bReducedMotion) ? 4.0f : 14.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BlurSlot->SetOffsets(FMargin(0, 0, 0, 0));

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeDeathBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.050f, 0.006f, 0.004f, 0.58f));
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BackdropSlot->SetOffsets(FMargin(0));

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeDeathPanelFrame"));
    CodeRescueUI::StylePanel(Panel, CodeRescueUI::Surface::Panel());
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.20f, 0.12f, 0.80f, 0.86f));
    PanelSlot->SetOffsets(FMargin(0));

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("DeathEndStateScroll"));
    Panel->SetContent(Scroll);

    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DeathPanel"));
    Scroll->AddChild(Box);

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathTitle"));
    TitleText->SetText(FText::FromString(TEXT("YOU WERE OVERRUN")));
    TitleText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::DangerBright());
    Box->AddChildToVerticalBox(TitleText);

    SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathSubtitle"));
    SubtitleText->SetText(FText::FromString(TEXT("The horde reached you. Your run is paused.")));
    SubtitleText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(SubtitleText, CodeRescueUI::EType::Body, CodeRescueUI::Color::TextSecondary());
    UVerticalBoxSlot* SubSlot = Box->AddChildToVerticalBox(SubtitleText);
    SubSlot->SetPadding(FMargin(0, 8, 0, 10));

    SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathLanguageSummary"));
    SummaryText->SetText(FText::FromString(BuildDeathLanguageSummary(GI)));
    SummaryText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreenBright());
    UVerticalBoxSlot* SummarySlot = Box->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0, 0, 0, 12));

    StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathStats"));
    StatsText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(StatsText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextPrimary());
    StatsText->SetText(FText::FromString(BuildDeathStatsText(GI)));
    UVerticalBoxSlot* StatsSlot = Box->AddChildToVerticalBox(StatsText);
    StatsSlot->SetPadding(FMargin(0, 0, 0, 16));

    ActionStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathActionStatus"));
    ActionStatusText->SetAutoWrapText(true);
    ActionStatusText->SetText(FText::FromString(BuildDeathActionStatus(GI)));
    CodeRescueUI::StyleText(ActionStatusText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentEmber());
    UVerticalBoxSlot* StatusSlot = Box->AddChildToVerticalBox(ActionStatusText);
    StatusSlot->SetPadding(FMargin(0, 0, 0, 12));

    auto MakeButton = [&](const FName& WidgetName, const FString& Label) -> UButton*
    {
        UButton* B = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);
        CodeRescueUI::StyleSecondaryButton(B);
        UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*(WidgetName.ToString() + TEXT("_Label"))));
        T->SetText(FText::FromString(Label));
        T->SetAutoWrapText(true);
        CodeRescueUI::StyleText(T, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::AccentEmber());
        B->AddChild(T);
        UVerticalBoxSlot* S = Box->AddChildToVerticalBox(B);
        S->SetPadding(FMargin(0, 4, 0, 4));
        return B;
    };

    RestartFromSaveButton = MakeButton(TEXT("DeathRestartFromSave"), TEXT("RESUME FROM LANGUAGE SAVE"));
    RestartFromSaveButton->OnClicked.AddDynamic(this, &UCodeRescueDeathWidget::OnRestartFromSaveClicked);

    RestartFreshButton = MakeButton(TEXT("DeathRestartFresh"), TEXT("START FRESH LANGUAGE RUN (delete this save)"));
    RestartFreshButton->OnClicked.AddDynamic(this, &UCodeRescueDeathWidget::OnRestartFreshClicked);

    SaveAndQuitButton = MakeButton(TEXT("DeathSaveAndQuit"), TEXT("SAVE THIS LANGUAGE RUN AND QUIT"));
    SaveAndQuitButton->OnClicked.AddDynamic(this, &UCodeRescueDeathWidget::OnSaveAndQuitClicked);

    QuitButton = MakeButton(TEXT("DeathQuit"), TEXT("QUIT TO DESKTOP"));
    QuitButton->OnClicked.AddDynamic(this, &UCodeRescueDeathWidget::OnQuitClicked);

    // Lock input + pause world while showing the death screen. Mirrors the
    // pattern in UCodeRescueVictoryWidget so the bUIOpen flag stays consistent.
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

void UCodeRescueDeathWidget::SetActionStatus(const FString& Status)
{
    if (ActionStatusText)
    {
        ActionStatusText->SetText(FText::FromString(Status));
    }
}

void UCodeRescueDeathWidget::OnRestartFromSaveClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        const ECodingLanguage Language = GI->SelectedLanguage;
        const bool bLoaded = GI->ResumeLanguageRun(Language);
        const FString SlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language);
        SetActionStatus(FString::Printf(
            TEXT("Loading %s recovery checkpoint from %s."),
            *GI->GetLanguageName(),
            *SlotName));
        UE_LOG(LogTemp, Display,
            TEXT("[CodeRescueDeathFlow] resume selected language=%s slot=%s loaded=%s"),
            *GI->GetLanguageName(),
            *SlotName,
            bLoaded ? TEXT("true") : TEXT("false"));
    }

    UGameplayStatics::SetGamePaused(GetWorld(), false);
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

void UCodeRescueDeathWidget::OnRestartFreshClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        SetActionStatus(FString::Printf(
            TEXT("Deleting %s run save and restarting this language track fresh."),
            *GI->GetLanguageName()));
        UE_LOG(LogTemp, Display,
            TEXT("[CodeRescueDeathFlow] fresh restart selected language=%s slot=%s"),
            *GI->GetLanguageName(),
            *UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage));
        GI->DeletePersistentRun();
    }
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

void UCodeRescueDeathWidget::OnSaveAndQuitClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SaveDeathRecoveryCheckpoint(false);
        const FString SlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
        SetActionStatus(FString::Printf(
            TEXT("Saved playable %s recovery checkpoint to %s. The start screen can resume it."),
            *GI->GetLanguageName(),
            *SlotName));
        UE_LOG(LogTemp, Display,
            TEXT("[CodeRescueDeathFlow] save-and-quit selected language=%s slot=%s"),
            *GI->GetLanguageName(),
            *SlotName);
    }
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCodeRescueDeathWidget::OnQuitClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        UE_LOG(LogTemp, Display,
            TEXT("[CodeRescueDeathFlow] quit without additional save selected language=%s slot=%s"),
            *GI->GetLanguageName(),
            *UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage));
    }
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCodeRescueDeathWidget::NativeDestruct()
{
    // Safety: clear UI lock + unpause in case the widget tears down via an
    // unexpected path (level reload from another button, etc.).
    ACodeRescueCharacter::SetUIOpen(false);
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, false);
    }
    Super::NativeDestruct();
}
