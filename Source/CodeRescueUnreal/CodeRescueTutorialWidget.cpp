#include "CodeRescueTutorialWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
struct FTutorialActionCard
{
    FString KeyGlyph;
    FString ActionLabel;
    FLinearColor Tint;
};

const TArray<FString>& GetTutorialPages()
{
    static const TArray<FString> Pages = {
        TEXT("OPERATION CODE RESCUE\n\nYou are an evac specialist learning to code under pressure.\nEvery city is a lesson space: choose a language, solve a\nprogramming challenge, rescue people, and graduate forward.\n\nPress [Space] or [Next] to continue."),
        TEXT("THE FIVE-STEP RESCUE LOOP\n\n1. Choose one deployment language before launch\n2. Start at the orientation plaza\n3. Solve the protected safehouse terminal\n4. Use the intel to rescue the survivor team\n5. Extract, debrief, and move to the next city"),
        TEXT("MOVEMENT AND SAFETY\n\nWASD - move\nMouse / arrow keys - look\nLeft Shift - sprint while stamina lasts\nSpace - jump\nC/V - cycle camera views\nF1-F6 - direct camera views\n\nFollow colored route strips if you feel lost."),
        TEXT("INTERACTION\n\nE - open terminals, rescue survivors, board helipads,\ntalk to NPCs, and pick up supplies\n\n1-0 - weapon quick slots\nJ - campaign journal\nP or Escape - pause, settings, saves, and safe exit"),
        TEXT("LANGUAGE CHOICE\n\nThe rescue goal stays the same, but syntax changes.\nJava, C, C+, C++, Python, and MATLAB each train the same\nconcept through a different coding habit.\n\nPick the language you want before deployment."),
        TEXT("TERMINAL VALIDATION\n\nVisible tests show examples you can inspect.\nHidden tests check whether your solution handles edge cases.\n\nA failed test is not failure as a player. It is a repair clue."),
        TEXT("COMBAT BASICS\n\nLeft Mouse / Space / F - fire\nR - reload\nQ - medkit if available\n\nHeadshots hit harder. Optional bosses are bonus threats:\nfinish the lesson and rescue route first."),
        TEXT("REWARDS AND MASTERY\n\nHints are allowed when you need them.\nNo-hint solves, first-try solves, streaks, and perfect scores\nbuild mastery, unlock rewards, and make the city feel safer.")
    };
    return Pages;
}

TArray<FTutorialActionCard> GetTutorialActionCards(int32 PageIndex, bool bSimplified)
{
    using namespace CodeRescueUI;

    TArray<FTutorialActionCard> Cards;
    Cards.Reserve(4);

    const FLinearColor Amber = Color::AccentAmber();
    const FLinearColor Green = Color::TerminalGreenBright();
    const FLinearColor Blue = Color::Stamina();
    const FLinearColor Red = Color::DangerBright();
    const FLinearColor Purple(0.82f, 0.58f, 1.0f, 1.0f);

    switch (PageIndex)
    {
    case 0:
        Cards.Add({ TEXT("LANG"), TEXT("Pick one track before deployment"), Amber });
        Cards.Add({ TEXT("SAVE"), TEXT("Progress writes to that language"), Green });
        Cards.Add({ TEXT("CITY"), TEXT("Solve terminals to open rescue routes"), Blue });
        break;
    case 1:
        Cards.Add({ TEXT("1"), TEXT("Language"), Amber });
        Cards.Add({ TEXT("3"), TEXT("Terminal"), Green });
        Cards.Add({ TEXT("5"), TEXT("Extraction"), Blue });
        if (!bSimplified)
        {
            Cards.Add({ TEXT("J"), TEXT("Journal tracks every city"), Purple });
        }
        break;
    case 2:
        Cards.Add({ TEXT("WASD"), TEXT("Move"), Green });
        Cards.Add({ TEXT("MOUSE"), TEXT("Look"), Blue });
        Cards.Add({ TEXT("SHIFT"), TEXT("Sprint"), Amber });
        if (!bSimplified)
        {
            Cards.Add({ TEXT("C/V"), TEXT("Camera views"), Purple });
        }
        break;
    case 3:
        Cards.Add({ TEXT("E"), TEXT("Interact"), Green });
        Cards.Add({ TEXT("J"), TEXT("Journal"), Amber });
        Cards.Add({ TEXT("P/ESC"), TEXT("Pause and save"), Blue });
        if (!bSimplified)
        {
            Cards.Add({ TEXT("T"), TEXT("Return to active route"), Purple });
        }
        break;
    case 4:
        Cards.Add({ TEXT("JAVA"), TEXT("Typed method contracts"), Amber });
        Cards.Add({ TEXT("C / C+"), TEXT("Pointers, counts, and careful buffers"), Blue });
        Cards.Add({ TEXT("C++"), TEXT("Vectors, strings, and typed returns"), Purple });
        Cards.Add({ TEXT("PY / MATLAB"), TEXT("Readable helpers and array thinking"), Green });
        break;
    case 5:
        Cards.Add({ TEXT("RUN"), TEXT("Submit visible and hidden tests"), Green });
        Cards.Add({ TEXT("HINT"), TEXT("Spend research points when stuck"), Amber });
        Cards.Add({ TEXT("TRACE"), TEXT("Compare first failed check"), Blue });
        if (!bSimplified)
        {
            Cards.Add({ TEXT("SAVE"), TEXT("Successful solves persist"), Purple });
        }
        break;
    case 6:
        Cards.Add({ TEXT("LMB"), TEXT("Fire"), Red });
        Cards.Add({ TEXT("R"), TEXT("Reload"), Amber });
        Cards.Add({ TEXT("Q"), TEXT("Medkit"), Green });
        if (!bSimplified)
        {
            Cards.Add({ TEXT("1-0"), TEXT("Weapon slots"), Blue });
        }
        break;
    case 7:
        Cards.Add({ TEXT("STREAK"), TEXT("First-try and no-hint solves score higher"), Amber });
        Cards.Add({ TEXT("SKILL"), TEXT("Rewards feed progression"), Green });
        Cards.Add({ TEXT("DEBRIEF"), TEXT("Extraction opens next-city travel"), Blue });
        break;
    default:
        break;
    }

    return Cards;
}
}

// Weak handle to the single on-screen tutorial. Poll-driven by the pawn (packaged-build UMG focus
// does not reliably deliver key events), so keep it in sync with construct/destruct.
TWeakObjectPtr<UCodeRescueTutorialWidget> UCodeRescueTutorialWidget::ActiveInstance;

bool UCodeRescueTutorialWidget::IsShowing()
{
    return ActiveInstance.IsValid();
}

void UCodeRescueTutorialWidget::DriveAdvance()
{
    if (UCodeRescueTutorialWidget* T = ActiveInstance.Get())
    {
        T->OnNextClicked();
    }
}

void UCodeRescueTutorialWidget::DriveDismiss()
{
    if (UCodeRescueTutorialWidget* T = ActiveInstance.Get())
    {
        // Deliberate keyboard skip closes immediately (no confirm arm needed like the button path).
        T->Finish();
    }
}

void UCodeRescueTutorialWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueTutorialWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueTutorialWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TutorialRoot"));
    WidgetTree->RootWidget = Root;

    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    Bg->SetBrushColor(CodeRescueUI::Resolve(FLinearColor(0.012f, 0.014f, 0.012f, 0.92f)));
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BgSlot->SetOffsets(FMargin(0));

    PhaseStripText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TutorialPhaseStrip"));
    PhaseStripText->SetAutoWrapText(true);
    PhaseStripText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(PhaseStripText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TerminalGreen(), false);
    UCanvasPanelSlot* PhaseSlot = Root->AddChildToCanvas(PhaseStripText);
    PhaseSlot->SetAnchors(FAnchors(0.5f, 0.12f, 0.5f, 0.12f));
    PhaseSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    PhaseSlot->SetSize(FVector2D(980.0f, 42.0f));

    LanguageSaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TutorialLanguageSave"));
    LanguageSaveText->SetAutoWrapText(true);
    LanguageSaveText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(LanguageSaveText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentAmber());
    UCanvasPanelSlot* SaveSlot = Root->AddChildToCanvas(LanguageSaveText);
    SaveSlot->SetAnchors(FAnchors(0.5f, 0.18f, 0.5f, 0.18f));
    SaveSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    SaveSlot->SetSize(FVector2D(980.0f, 54.0f));

    PageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    PageText->SetAutoWrapText(true);
    PageText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(PageText, CodeRescueUI::EType::Body, CodeRescueUI::Color::TextPrimary());
    UCanvasPanelSlot* TextSlot = Root->AddChildToCanvas(PageText);
    TextSlot->SetAnchors(FAnchors(0.5f, 0.27f, 0.5f, 0.27f));
    TextSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    TextSlot->SetSize(FVector2D(960.0f, 310.0f));

    ActionCardBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TutorialActionCards"));
    UCanvasPanelSlot* CardSlot = Root->AddChildToCanvas(ActionCardBox);
    CardSlot->SetAnchors(FAnchors(0.5f, 0.68f, 0.5f, 0.68f));
    CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    CardSlot->SetSize(FVector2D(980.0f, 122.0f));

    InputHintModeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TutorialInputHintMode"));
    InputHintModeText->SetAutoWrapText(true);
    InputHintModeText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(InputHintModeText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary(), false);
    UCanvasPanelSlot* HintSlot = Root->AddChildToCanvas(InputHintModeText);
    HintSlot->SetAnchors(FAnchors(0.5f, 0.79f, 0.5f, 0.79f));
    HintSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    HintSlot->SetSize(FVector2D(980.0f, 34.0f));

    PageNumberText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    PageNumberText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(PageNumberText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextMuted(), false);
    UCanvasPanelSlot* PnSlot = Root->AddChildToCanvas(PageNumberText);
    PnSlot->SetAnchors(FAnchors(0.5f, 0.85f, 0.5f, 0.85f));
    PnSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    PnSlot->SetSize(FVector2D(200.0f, 30.0f));

    NextButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StylePrimaryButton(NextButton);
    NextButton->OnClicked.AddDynamic(this, &UCodeRescueTutorialWidget::OnNextClicked);
    UCanvasPanelSlot* NbSlot = Root->AddChildToCanvas(NextButton);
    NbSlot->SetAnchors(FAnchors(0.5f, 0.92f, 0.5f, 0.92f));
    NbSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    NbSlot->SetSize(FVector2D(200.0f, 50.0f));
    {
        UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        L->SetText(FText::FromString(TEXT("Next [Space]")));
        L->SetJustification(ETextJustify::Center);
        CodeRescueUI::StyleText(L, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentAmber());
        NextButton->AddChild(L);
    }

    SkipButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StyleSecondaryButton(SkipButton);
    SkipButton->OnClicked.AddDynamic(this, &UCodeRescueTutorialWidget::OnSkipClicked);
    UCanvasPanelSlot* SkipSlot = Root->AddChildToCanvas(SkipButton);
    SkipSlot->SetAnchors(FAnchors(0.95f, 0.05f, 0.95f, 0.05f));
    SkipSlot->SetAlignment(FVector2D(1.0f, 0.0f));
    SkipSlot->SetSize(FVector2D(120.0f, 36.0f));
    {
        SkipButtonLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        SkipButtonLabel->SetText(FText::FromString(TEXT("Skip")));
        SkipButtonLabel->SetJustification(ETextJustify::Center);
        CodeRescueUI::StyleText(SkipButtonLabel, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
        SkipButton->AddChild(SkipButtonLabel);
    }

    ACodeRescueCharacter::SetUIOpen(true);
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        // 2026-07-01 (round 5, playtest-proven): GameOnly, NOT UIOnly. The pawn's polled-key
        // path drives this overlay (Space/Enter advance, Esc/Backspace skip), and
        // PC->WasInputKeyJustPressed only sees keys while the PlayerController owns input.
        // UIOnly routed Space to Slate and starved the poll, so the tutorial never advanced.
        // This mirrors the launch-language gate, which uses GameOnly for the same reason.
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
    }
    ActiveInstance = this;
    ShowPage(0);
}

void UCodeRescueTutorialWidget::NativeDestruct()
{
    if (ActiveInstance.Get() == this)
    {
        ActiveInstance = nullptr;
    }
    ACodeRescueCharacter::SetUIOpen(false);
    Super::NativeDestruct();
}

FReply UCodeRescueTutorialWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::SpaceBar || InKeyEvent.GetKey() == EKeys::Enter)
    {
        OnNextClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        OnSkipClicked();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCodeRescueTutorialWidget::ShowPage(int32 Index)
{
    const TArray<FString>& Pages = GetTutorialPages();
    if (!Pages.IsValidIndex(Index))
    {
        Finish();
        return;
    }
    CurrentPage = Index;
    bSkipConfirmArmed = false;
    if (SkipButtonLabel)
    {
        SkipButtonLabel->SetText(FText::FromString(TEXT("Skip")));
    }
    if (PageText) PageText->SetText(FText::FromString(Pages[Index]));
    if (PhaseStripText)
    {
        PhaseStripText->SetText(FText::FromString(BuildPhaseStripLine(Index)));
    }
    if (LanguageSaveText)
    {
        LanguageSaveText->SetText(FText::FromString(BuildLanguageSaveLine()));
    }
    if (InputHintModeText)
    {
        const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
        InputHintModeText->SetText(FText::FromString(
            GI && GI->bSimplifiedInputHints
                ? TEXT("SIMPLIFIED HINTS: showing the core action prompts for this page")
                : TEXT("FULL INPUT GLYPHS: showing primary keys plus route, camera, save, and mastery prompts")));
    }
    if (PageNumberText)
    {
        PageNumberText->SetText(FText::FromString(FString::Printf(TEXT("Page %d / %d"), Index + 1, Pages.Num())));
    }
    RefreshActionCardsForPage(Index);
}

void UCodeRescueTutorialWidget::RefreshActionCardsForPage(int32 Index)
{
    if (!ActionCardBox)
    {
        return;
    }

    ActionCardBox->ClearChildren();

    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bSimplified = GI && GI->bSimplifiedInputHints;
    for (const FTutorialActionCard& Card : GetTutorialActionCards(Index, bSimplified))
    {
        AddActionCard(Card.KeyGlyph, Card.ActionLabel, Card.Tint);
    }
}

void UCodeRescueTutorialWidget::AddActionCard(const FString& KeyGlyph, const FString& ActionLabel, const FLinearColor& Tint)
{
    if (!ActionCardBox)
    {
        return;
    }

    const int32 CardIndex = ActionCardBox->GetChildrenCount();
    UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("TutorialActionCard_%d"), CardIndex));
    FLinearColor Fill = CodeRescueUI::Surface::Raised();
    Fill.R = FMath::Min(1.0f, Fill.R + Tint.R * 0.045f);
    Fill.G = FMath::Min(1.0f, Fill.G + Tint.G * 0.045f);
    Fill.B = FMath::Min(1.0f, Fill.B + Tint.B * 0.045f);
    CodeRescueUI::StylePanel(Card, Fill, FMargin(10.0f, 9.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("TutorialActionCardStack_%d"), CardIndex));
    Card->SetContent(Stack);

    UTextBlock* Glyph = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("TutorialActionGlyph_%d"), CardIndex));
    Glyph->SetText(FText::FromString(KeyGlyph));
    Glyph->SetJustification(ETextJustify::Center);
    Glyph->SetAutoWrapText(true);
    CodeRescueUI::StyleText(Glyph, CodeRescueUI::EType::Heading, Tint);
    UVerticalBoxSlot* GlyphSlot = Stack->AddChildToVerticalBox(Glyph);
    GlyphSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));

    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("TutorialActionLabel_%d"), CardIndex));
    Label->SetText(FText::FromString(ActionLabel));
    Label->SetJustification(ETextJustify::Center);
    Label->SetAutoWrapText(true);
    CodeRescueUI::StyleText(Label, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextPrimary(), false);
    UVerticalBoxSlot* LabelSlot = Stack->AddChildToVerticalBox(Label);
    LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UHorizontalBoxSlot* CardSlot = ActionCardBox->AddChildToHorizontalBox(Card);
    CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    CardSlot->SetPadding(FMargin(6.0f, 0.0f, 6.0f, 0.0f));
}

FString UCodeRescueTutorialWidget::BuildLanguageSaveLine()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        const FString LanguageName = GI->GetLanguageName();
        const FString SlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
        const FString SaveState = GI->DoesLanguageSaveExist(GI->SelectedLanguage)
            ? TEXT("resume data found")
            : TEXT("fresh language run ready");
        return FString::Printf(
            TEXT("ACTIVE LANGUAGE SAVE: %s  |  %s  |  %s"),
            *LanguageName,
            *SlotName,
            *SaveState);
    }

    return TEXT("ACTIVE LANGUAGE SAVE: choose a language on the start screen before deployment");
}

FString UCodeRescueTutorialWidget::BuildPhaseStripLine(int32 Index) const
{
    switch (Index)
    {
    case 0:
        return TEXT("FIRST TEN MINUTES: LANGUAGE -> ORIENT -> TERMINAL -> RESCUE -> EXTRACT");
    case 1:
        return TEXT("RESCUE LOOP: choose language, solve terminal, rescue survivor, extract");
    case 2:
        return TEXT("MOVEMENT: move, look, sprint, recover route");
    case 3:
        return TEXT("INTERACTION: terminals, survivors, supplies, saves");
    case 4:
        return TEXT("LANGUAGE TRACK: one selected coding platform drives every terminal");
    case 5:
        return TEXT("VALIDATION: visible tests teach, hidden tests verify edge cases");
    case 6:
        return TEXT("SAFETY: keep distance, reload, heal, finish the lesson first");
    case 7:
        return TEXT("MASTERY: clean solves improve rewards, debriefs open redeployment");
    default:
        return TEXT("FIRST TEN MINUTES");
    }
}

void UCodeRescueTutorialWidget::OnNextClicked()
{
    const TArray<FString>& Pages = GetTutorialPages();
    if (CurrentPage + 1 >= Pages.Num())
    {
        Finish();
        return;
    }
    ShowPage(CurrentPage + 1);
}

void UCodeRescueTutorialWidget::OnSkipClicked()
{
    if (!bSkipConfirmArmed)
    {
        bSkipConfirmArmed = true;
        if (SkipButtonLabel)
        {
            SkipButtonLabel->SetText(FText::FromString(TEXT("Skip?")));
        }
        if (PageNumberText)
        {
            PageNumberText->SetText(FText::FromString(TEXT("Press Skip/Escape again to close tutorial")));
        }
        return;
    }
    Finish();
}

void UCodeRescueTutorialWidget::Finish()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->bHasShownTutorial = true;
        GI->SavePersistentRun();
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            FInputModeGameOnly Mode;
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = false;
        }
        else
        {
            FInputModeUIOnly Mode;
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = true;
        }
    }
    RemoveFromParent();
}
