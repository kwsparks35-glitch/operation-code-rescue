#include "CodeRescueSaveSlotsWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSaveGame.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
FString SaveSlotsLanguageName(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java: return TEXT("Java");
    case ECodingLanguage::C: return TEXT("C");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    case ECodingLanguage::CPlus: return TEXT("C+");
    case ECodingLanguage::Cpp: return TEXT("C++");
    default: return TEXT("Unknown");
    }
}

void MirrorSaveSlotsThemeFromSettings(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return;
    }

    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
}

UTextBlock* MakeSaveSlotsLabel(
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

void StyleSaveSlotsButton(UButton* Button, bool bPrimary, bool bDanger, bool bEnabled)
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

    if (bDanger)
    {
        const FLinearColor DangerFill = CodeRescueUI::Theme().bHighContrast
            ? FLinearColor(0.22f, 0.035f, 0.025f, 0.98f)
            : FLinearColor(0.14f, 0.045f, 0.035f, 0.96f);
        Button->SetBackgroundColor(CodeRescueUI::Resolve(DangerFill));
    }
    if (!bEnabled)
    {
        Button->SetBackgroundColor(CodeRescueUI::Resolve(FLinearColor(0.038f, 0.036f, 0.034f, 0.88f)));
    }

    Button->SetColorAndOpacity(FLinearColor::White);
    Button->SetIsEnabled(bEnabled);
}

FString MakeBackupSlotName(int32 SlotIndex)
{
    return FString::Printf(TEXT("OperationCodeRescue_Slot%d"), SlotIndex);
}

FString DescribeBackupSlot(int32 SlotIndex)
{
    const FString SlotName = MakeBackupSlotName(SlotIndex);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return FString::Printf(
            TEXT("BACKUP %d | EMPTY\nNo manual backup written yet. Save Backup copies the active language run here."),
            SlotIndex + 1);
    }

    const UCodeRescueSaveGame* Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!Save)
    {
        return FString::Printf(
            TEXT("BACKUP %d | UNREADABLE\nA save file exists but could not be decoded."),
            SlotIndex + 1);
    }

    const FString WorldState = Save->bHasWorldState ? TEXT("world state captured") : TEXT("profile-only state");
    return FString::Printf(
        TEXT("BACKUP %d | SAVED %s RUN\nTerminals %d | Survivors %d | Score %d | RP %d | %s"),
        SlotIndex + 1,
        *SaveSlotsLanguageName(Save->LastSelectedLanguage),
        Save->TerminalsSolved,
        Save->SurvivorsRescued,
        Save->CodingScore,
        Save->ResearchPoints,
        *WorldState);
}

UButton* MakeSaveSlotsActionButton(
    UWidgetTree* Tree,
    UHorizontalBox* Row,
    const FString& Label,
    bool bPrimary,
    bool bDanger,
    bool bEnabled)
{
    UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
    StyleSaveSlotsButton(Button, bPrimary, bDanger, bEnabled);

    const FLinearColor LabelColor = !bEnabled
        ? CodeRescueUI::Color::TextMuted()
        : bDanger
            ? CodeRescueUI::Color::DangerBright()
            : bPrimary
                ? CodeRescueUI::Color::TerminalGreenBright()
                : CodeRescueUI::Color::AccentAmber();
    UTextBlock* Text = MakeSaveSlotsLabel(
        Tree,
        FString::Printf(TEXT("  %s  "), *Label),
        CodeRescueUI::EType::BodySmall,
        LabelColor);
    Text->SetJustification(ETextJustify::Center);
    Button->AddChild(Text);

    UHorizontalBoxSlot* ButtonSlot = Row->AddChildToHorizontalBox(Button);
    ButtonSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, 0.0f, 0.0f));
    ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    return Button;
}
}

FString UCodeRescueSaveSlotsWidget::MakeSlotName(int32 SlotIndex)
{
    return MakeBackupSlotName(SlotIndex);
}

void UCodeRescueSaveSlotsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueSaveSlotsWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueSaveSlotsWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    MirrorSaveSlotsThemeFromSettings(GetGameInstance<UCodeRescueGameInstance>());

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SaveRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("SaveSlotsBlur"));
    Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 1.0f : 4.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BlurSlot->SetOffsets(FMargin(0.0f));

    PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LanguageSaveBackupPanel"));
    CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Panel(), FMargin(18.0f, 16.0f));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelFrame);
    PanelSlot->SetAnchors(FAnchors(0.18f, 0.12f, 0.82f, 0.88f));
    PanelSlot->SetOffsets(FMargin(0.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LanguageSaveBackupStack"));
    PanelFrame->SetContent(Stack);

    TitleText = MakeSaveSlotsLabel(
        WidgetTree,
        TEXT("LANGUAGE SAVE BACKUPS"),
        CodeRescueUI::EType::Title,
        CodeRescueUI::Color::AccentAmber());
    Stack->AddChildToVerticalBox(TitleText);

    SummaryText = MakeSaveSlotsLabel(
        WidgetTree,
        TEXT(""),
        CodeRescueUI::EType::BodySmall,
        CodeRescueUI::Color::TextSecondary(),
        false);
    UVerticalBoxSlot* SummarySlot = Stack->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 8.0f));

    FeedbackText = MakeSaveSlotsLabel(
        WidgetTree,
        TEXT("Save Backup copies the active language run; Load Backup promotes a backup into that language resume save."),
        CodeRescueUI::EType::Caption,
        CodeRescueUI::Color::AccentAmber(),
        false);
    Stack->AddChildToVerticalBox(FeedbackText);

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("LanguageSaveBackupScroll"));
    UVerticalBoxSlot* ScrollSlot = Stack->AddChildToVerticalBox(Scroll);
    ScrollSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 10.0f));
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    SlotsList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LanguageSaveBackupList"));
    Scroll->AddChild(SlotsList);

    CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StyleSecondaryButton(CloseButton);
    CloseButton->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnCloseClicked);
    UTextBlock* CloseLabel = MakeSaveSlotsLabel(
        WidgetTree,
        TEXT("Close"),
        CodeRescueUI::EType::BodySmall,
        CodeRescueUI::Color::AccentAmber());
    CloseLabel->SetJustification(ETextJustify::Center);
    CloseButton->AddChild(CloseLabel);
    Stack->AddChildToVerticalBox(CloseButton);

    ACodeRescueCharacter::SetUIOpen(true);
    SetFeedback(
        TEXT("Manual backups are available; the start-screen language resume save remains authoritative."),
        CodeRescueUI::Color::AccentAmber());
    Refresh();
}

void UCodeRescueSaveSlotsWidget::NativeDestruct()
{
    ACodeRescueCharacter::SetUIOpen(false);
    Super::NativeDestruct();
}

void UCodeRescueSaveSlotsWidget::Refresh()
{
    if (!SlotsList) return;
    SlotsList->ClearChildren();

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorSaveSlotsThemeFromSettings(GI);

    if (PanelFrame)
    {
        CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Panel(), FMargin(18.0f, 16.0f));
    }
    if (TitleText)
    {
        CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    }
    if (SummaryText)
    {
        const FString LanguageName = GI ? GI->GetLanguageName() : TEXT("Unavailable");
        const FString LanguageSlot = GI
            ? UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)
            : TEXT("OperationCodeRescue_Language_<Track>");
        const FString LearningLine = GI ? GI->GetLearningProgressSummary() : TEXT("Learning progress unavailable");
        SummaryText->SetText(FText::FromString(FString::Printf(
            TEXT("Active language run: %s | Start-screen resume slot: %s\nManual backups copy this run; loading a backup writes it back into that language resume slot.\n%s"),
            *LanguageName,
            *LanguageSlot,
            *LearningLine)));
        CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
    }
    if (FeedbackText)
    {
        CodeRescueUI::StyleText(FeedbackText, CodeRescueUI::EType::Caption, FeedbackColor, false);
    }

    for (int32 SlotIdx = 0; SlotIdx < 3; ++SlotIdx)
    {
        const FString Name = MakeSlotName(SlotIdx);
        const bool bExists = UGameplayStatics::DoesSaveGameExist(Name, 0);

        UBorder* RowFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        CodeRescueUI::StylePanel(RowFrame, CodeRescueUI::Surface::Sunken(), FMargin(10.0f, 8.0f));
        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        RowFrame->SetContent(Row);

        UTextBlock* Label = MakeSaveSlotsLabel(
            WidgetTree,
            DescribeBackupSlot(SlotIdx),
            CodeRescueUI::EType::BodySmall,
            bExists ? CodeRescueUI::Color::TextPrimary() : CodeRescueUI::Color::TextSecondary(),
            false);
        UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
        LabelSlot->SetPadding(FMargin(0.0f, 0.0f, CodeRescueUI::Space::S, 0.0f));
        LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

        {
            UButton* B = MakeSaveSlotsActionButton(WidgetTree, Row, TEXT("Save Backup"), true, false, true);
            if (SlotIdx == 0) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnSaveSlot0Clicked);
            if (SlotIdx == 1) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnSaveSlot1Clicked);
            if (SlotIdx == 2) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnSaveSlot2Clicked);
        }
        {
            UButton* B = MakeSaveSlotsActionButton(WidgetTree, Row, TEXT("Load Backup"), false, false, bExists);
            if (SlotIdx == 0) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnLoadSlot0Clicked);
            if (SlotIdx == 1) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnLoadSlot1Clicked);
            if (SlotIdx == 2) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnLoadSlot2Clicked);
        }
        {
            UButton* B = MakeSaveSlotsActionButton(WidgetTree, Row, TEXT("Delete"), false, true, bExists);
            if (SlotIdx == 0) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnDeleteSlot0Clicked);
            if (SlotIdx == 1) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnDeleteSlot1Clicked);
            if (SlotIdx == 2) B->OnClicked.AddDynamic(this, &UCodeRescueSaveSlotsWidget::OnDeleteSlot2Clicked);
        }

        UVerticalBoxSlot* RowSlot = SlotsList->AddChildToVerticalBox(RowFrame);
        RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, CodeRescueUI::Space::S));
    }
}

void UCodeRescueSaveSlotsWidget::DoSave(int32 SlotIndex)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        SetFeedback(TEXT("Save data is unavailable."), CodeRescueUI::Color::DangerBright());
        return;
    }

    const FString BackupSlot = MakeSlotName(SlotIndex);
    const FString LanguageSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
    GI->SaveSlotName = BackupSlot;
    const bool bBackupSaved = GI->SavePersistentRun();
    GI->SaveSlotName = LanguageSlot;
    const bool bLanguageSaved = bBackupSaved && GI->SavePersistentRun();

    if (bBackupSaved && bLanguageSaved)
    {
        SetFeedback(
            FString::Printf(
                TEXT("Saved Backup %d and refreshed the %s start-screen resume save."),
                SlotIndex + 1,
                *GI->GetLanguageName()),
            CodeRescueUI::Color::TerminalGreenBright());
    }
    else
    {
        SetFeedback(
            FString::Printf(TEXT("Backup %d could not be saved; active language slot restored."), SlotIndex + 1),
            CodeRescueUI::Color::DangerBright());
    }
    Refresh();
}

void UCodeRescueSaveSlotsWidget::DoLoad(int32 SlotIndex)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        SetFeedback(TEXT("Save data is unavailable."), CodeRescueUI::Color::DangerBright());
        return;
    }

    const FString BackupSlot = MakeSlotName(SlotIndex);
    UCodeRescueSaveGame* BackupSave = Cast<UCodeRescueSaveGame>(UGameplayStatics::LoadGameFromSlot(BackupSlot, 0));
    if (!BackupSave)
    {
        const FString LanguageSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
        GI->SaveSlotName = LanguageSlot;
        SetFeedback(
            FString::Printf(TEXT("Backup %d is empty or unreadable; active language save unchanged."), SlotIndex + 1),
            CodeRescueUI::Color::Warning());
        Refresh();
        return;
    }

    GI->SaveSlotName = BackupSlot;
    const bool bLoaded = GI->LoadPersistentRun();
    const FString LanguageSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
    GI->SaveSlotName = LanguageSlot;
    const bool bPromoted = bLoaded && UGameplayStatics::SaveGameToSlot(BackupSave, LanguageSlot, 0);
    if (bPromoted)
    {
        if (UWorld* W = GetWorld())
        {
            GI->LastSaveWallSeconds = W->GetTimeSeconds();
        }
        SetFeedback(
            FString::Printf(
                TEXT("Loaded Backup %d into the %s start-screen resume save."),
                SlotIndex + 1,
                *GI->GetLanguageName()),
            CodeRescueUI::Color::TerminalGreenBright());
    }
    else
    {
        SetFeedback(
            FString::Printf(TEXT("Backup %d could not be promoted into a language resume save."), SlotIndex + 1),
            CodeRescueUI::Color::DangerBright());
    }
    Refresh();
}

void UCodeRescueSaveSlotsWidget::DoDelete(int32 SlotIndex)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const FString Name = MakeSlotName(SlotIndex);
    const bool bDeleted = UGameplayStatics::DeleteGameInSlot(Name, 0);
    if (GI)
    {
        GI->SaveSlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
    }
    if (bDeleted)
    {
        SetFeedback(
            FString::Printf(TEXT("Deleted Backup %d; active language resume save untouched."), SlotIndex + 1),
            CodeRescueUI::Color::AccentAmber());
    }
    else
    {
        SetFeedback(
            FString::Printf(TEXT("Backup %d was already empty; active language resume save untouched."), SlotIndex + 1),
            CodeRescueUI::Color::Warning());
    }
    Refresh();
}

void UCodeRescueSaveSlotsWidget::OnCloseClicked()
{
    Close();
}

void UCodeRescueSaveSlotsWidget::OnSaveSlot0Clicked() { DoSave(0); }
void UCodeRescueSaveSlotsWidget::OnSaveSlot1Clicked() { DoSave(1); }
void UCodeRescueSaveSlotsWidget::OnSaveSlot2Clicked() { DoSave(2); }
void UCodeRescueSaveSlotsWidget::OnLoadSlot0Clicked() { DoLoad(0); }
void UCodeRescueSaveSlotsWidget::OnLoadSlot1Clicked() { DoLoad(1); }
void UCodeRescueSaveSlotsWidget::OnLoadSlot2Clicked() { DoLoad(2); }
void UCodeRescueSaveSlotsWidget::OnDeleteSlot0Clicked() { DoDelete(0); }
void UCodeRescueSaveSlotsWidget::OnDeleteSlot1Clicked() { DoDelete(1); }
void UCodeRescueSaveSlotsWidget::OnDeleteSlot2Clicked() { DoDelete(2); }

void UCodeRescueSaveSlotsWidget::SetFeedback(const FString& Message, const FLinearColor& Color)
{
    FeedbackColor = Color;
    if (!FeedbackText)
    {
        return;
    }

    FeedbackText->SetText(FText::FromString(Message));
    CodeRescueUI::StyleText(FeedbackText, CodeRescueUI::EType::Caption, FeedbackColor, false);
}

void UCodeRescueSaveSlotsWidget::Close()
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
