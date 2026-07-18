#include "CodeRescueMainMenuWidget.h"
#include "CodeRescueGameInstance.h"
#include "InputCoreTypes.h"

TWeakObjectPtr<UCodeRescueMainMenuWidget> UCodeRescueMainMenuWidget::ActiveLaunchMenu = nullptr;
#include "CodeRescueCharacter.h"
#include "CodeRescueSettingsWidget.h"
#include "CodeRescueTutorialWidget.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
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
#include "Kismet/KismetSystemLibrary.h"

namespace
{
FString MenuLanguageName(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java: return TEXT("Java");
    case ECodingLanguage::C: return TEXT("C");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    case ECodingLanguage::CPlus: return TEXT("C+");
    case ECodingLanguage::Cpp: return TEXT("C++");
    default: return TEXT("Java");
    }
}

UButton* MakeMenuButton(UWidgetTree* Tree, UVerticalBox* Box, const FString& Label)
{
    using namespace CodeRescueUI;
    UButton* B = Tree->ConstructWidget<UButton>(UButton::StaticClass());
    StyleSecondaryButton(B);
    UTextBlock* L = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    L->SetText(FText::FromString(TEXT("  ") + Label));
    L->SetAutoWrapText(true);
    L->SetJustification(ETextJustify::Left);
    StyleText(L, EType::Subheading, Color::AccentAmber());
    B->AddChild(L);
    UVerticalBoxSlot* S = Box->AddChildToVerticalBox(B);
    S->SetPadding(FMargin(0.0f, Space::S, 0.0f, Space::S));
    return B;
}

UButton* MakeLanguageRowButton(UWidgetTree* Tree, UHorizontalBox* Row, const FString& Label, bool bEnabled)
{
    using namespace CodeRescueUI;
    UButton* B = Tree->ConstructWidget<UButton>(UButton::StaticClass());
    StyleSecondaryButton(B);
    B->SetIsEnabled(bEnabled);
    UTextBlock* L = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    L->SetText(FText::FromString(TEXT("  ") + Label));
    L->SetAutoWrapText(true);
    L->SetJustification(ETextJustify::Left);
    StyleText(L, EType::Body, bEnabled ? Color::AccentAmber() : Color::TextSecondary());
    B->AddChild(L);
    UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(B);
    Slot->SetPadding(FMargin(Space::XS, Space::XS, Space::XS, Space::XS));
    Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    return B;
}

UHorizontalBox* MakeLanguageActionRow(UWidgetTree* Tree, UVerticalBox* Box)
{
    UHorizontalBox* Row = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Row);
    RowSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 2.0f));
    return Row;
}

FString NewLanguageRunLabel(ECodingLanguage Language)
{
    return FString::Printf(TEXT("NEW %s RUN\nStart a fresh %s-only rescue."), *MenuLanguageName(Language), *MenuLanguageName(Language));
}

FString ResumeLanguageRunLabel(const UCodeRescueGameInstance* GI, ECodingLanguage Language)
{
    const FString Name = MenuLanguageName(Language);
    if (GI && GI->DoesLanguageSaveExist(Language))
    {
        return FString::Printf(TEXT("RESUME %s SAVE\n%s"), *Name, *GI->GetLanguageSaveSummary(Language));
    }
    return FString::Printf(TEXT("RESUME %s SAVE\nNo save yet"), *Name);
}
}

void UCodeRescueMainMenuWidget::SetLaunchLanguageOnly(bool bInLaunchLanguageOnly)
{
    if (bInLaunchLanguageOnly)
    {
        ActiveLaunchMenu = this;
    }
    bLaunchLanguageOnly = bInLaunchLanguageOnly;
}

void UCodeRescueMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (bLaunchLanguageOnly)
    {
        ActiveLaunchMenu = this;
    }
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueMainMenuWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: the tree must exist BEFORE Slate assembly.
    // Building it in NativeConstruct (which fires after TakeWidget) rendered every screen empty.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueMainMenuWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MenuRoot"));
    WidgetTree->RootWidget = Root;

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeMainMenuBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.012f, 0.014f, 0.012f, 1.0f));
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BackdropSlot->SetOffsets(FMargin(0));

    UBorder* LowFog = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeLowFogBand"));
    LowFog->SetBrushColor(FLinearColor(0.20f, 0.15f, 0.09f, 0.22f));
    // 2026-07-06: decorative layers must NEVER intercept clicks meant for the
    // track buttons (live repro: clicks on visible buttons did nothing).
    LowFog->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* FogSlot = Root->AddChildToCanvas(LowFog);
    FogSlot->SetAnchors(FAnchors(0.0f, 0.70f, 1.0f, 1.0f));
    FogSlot->SetOffsets(FMargin(0));

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    TitleText->SetText(FText::FromString(
        bLaunchLanguageOnly ? TEXT("CHOOSE CODING LANGUAGE") : TEXT("OPERATION CODE RESCUE")));
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Display, CodeRescueUI::Color::AccentAmber());
    UCanvasPanelSlot* TSlot = Root->AddChildToCanvas(TitleText);
    TSlot->SetAnchors(FAnchors(0.08f, 0.10f, 0.92f, 0.10f));
    TSlot->SetAlignment(FVector2D(0.0f, 0.0f));
    TSlot->SetSize(FVector2D(920.0f, 76.0f));

    TaglineText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    TaglineText->SetText(FText::FromString(
        bLaunchLanguageOnly ? TEXT("Select one track to deploy into the rescue zone.") : TEXT("Read the code. Hold the line. Rescue who you can.")));
    CodeRescueUI::StyleText(TaglineText, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::TextSecondary());
    UCanvasPanelSlot* TgSlot = Root->AddChildToCanvas(TaglineText);
    TgSlot->SetAnchors(FAnchors(0.08f, 0.19f, 0.92f, 0.19f));
    TgSlot->SetAlignment(FVector2D(0.0f, 0.0f));
    TgSlot->SetSize(FVector2D(920.0f, 36.0f));

    UTextBlock* MoodText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BespokeMainMenuMoodText"));
    MoodText->SetText(FText::FromString(
        bLaunchLanguageOnly ? TEXT("FIELD TERMINAL // LANGUAGE SELECTION") : TEXT("FIELD TERMINAL // CHOOSE A LANGUAGE TO DEPLOY")));
    CodeRescueUI::StyleText(MoodText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TerminalGreen(), false);
    UCanvasPanelSlot* MoodSlot = Root->AddChildToCanvas(MoodText);
    MoodSlot->SetAnchors(FAnchors(0.08f, 0.25f, 0.92f, 0.25f));
    MoodSlot->SetSize(FVector2D(760.0f, 28.0f));

    UBorder* MenuPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeMainMenuPanel"));
    CodeRescueUI::StylePanel(MenuPanel, CodeRescueUI::Surface::Panel());
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(MenuPanel);
    // 2026-07-06 first-boot fix (live repro): the launch panel was a FIXED
    // 760x620 box anchored at 34% height. Its content (16-line route preview +
    // 6 track rows) overflows 620px at EVERY resolution, so the later track
    // buttons (C++/Python/MATLAB) rendered clipped or off-window with no
    // scrollbar — "language selection screen is too large to see all
    // available options". The panel now stretches vertically with the window
    // (28%..97%) and its content scrolls, buttons first.
    if (bLaunchLanguageOnly)
    {
        PanelSlot->SetAnchors(FAnchors(0.5f, 0.28f, 0.5f, 0.97f));
        PanelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
        PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 780.0f, 0.0f));
    }
    else
    {
        PanelSlot->SetAnchors(FAnchors(0.08f, 0.30f, 0.08f, 0.96f));
        PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
        PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 460.0f, 0.0f));
    }

    // Whole menu column scrolls as a final safety at very short windows.
    UScrollBox* MenuScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("MenuColumnScroll"));
    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    MenuScroll->AddChild(Box);
    MenuPanel->SetContent(MenuScroll);
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    if (!bLaunchLanguageOnly)
    {
        NewGameBtn  = MakeMenuButton(WidgetTree, Box, TEXT("START SELECTED LANGUAGE"));
        ContinueBtn = MakeMenuButton(WidgetTree, Box, TEXT("CONTINUE SAVED RUN"));
        SandboxBtn  = MakeMenuButton(WidgetTree, Box, TEXT("SANDBOX: SELECTED LANGUAGE"));
    }

    if (bLaunchLanguageOnly)
    {
        // Buttons FIRST — they can never be pushed out of sight by text again.
        UTextBlock* KeyboardHint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LaunchKeyboardHint"));
        KeyboardHint->SetText(FText::FromString(TEXT("CLICK a track — or press 1-6 to pick, ENTER to deploy. Scroll down for the route preview.")));
        KeyboardHint->SetAutoWrapText(true);
        CodeRescueUI::StyleText(KeyboardHint, CodeRescueUI::EType::Caption, CodeRescueUI::Color::Warning(), false);
        KeyboardHint->SetVisibility(ESlateVisibility::HitTestInvisible);
        UVerticalBoxSlot* HintSlot = Box->AddChildToVerticalBox(KeyboardHint);
        HintSlot->SetPadding(FMargin(0, 0, 0, 6));
    }

    if (bLaunchLanguageOnly)
    {
        UHorizontalBox* JavaRow = MakeLanguageActionRow(WidgetTree, Box);
        JavaLanguageBtn = MakeLanguageRowButton(WidgetTree, JavaRow, NewLanguageRunLabel(ECodingLanguage::Java), true);
        ResumeJavaBtn = MakeLanguageRowButton(WidgetTree, JavaRow, ResumeLanguageRunLabel(GI, ECodingLanguage::Java), GI && GI->DoesLanguageSaveExist(ECodingLanguage::Java));

        UHorizontalBox* CRow = MakeLanguageActionRow(WidgetTree, Box);
        CLanguageBtn = MakeLanguageRowButton(WidgetTree, CRow, NewLanguageRunLabel(ECodingLanguage::C), true);
        ResumeCBtn = MakeLanguageRowButton(WidgetTree, CRow, ResumeLanguageRunLabel(GI, ECodingLanguage::C), GI && GI->DoesLanguageSaveExist(ECodingLanguage::C));

        UHorizontalBox* CPlusRow = MakeLanguageActionRow(WidgetTree, Box);
        CPlusLanguageBtn = MakeLanguageRowButton(WidgetTree, CPlusRow, NewLanguageRunLabel(ECodingLanguage::CPlus), true);
        ResumeCPlusBtn = MakeLanguageRowButton(WidgetTree, CPlusRow, ResumeLanguageRunLabel(GI, ECodingLanguage::CPlus), GI && GI->DoesLanguageSaveExist(ECodingLanguage::CPlus));

        UHorizontalBox* CppRow = MakeLanguageActionRow(WidgetTree, Box);
        CppLanguageBtn = MakeLanguageRowButton(WidgetTree, CppRow, NewLanguageRunLabel(ECodingLanguage::Cpp), true);
        ResumeCppBtn = MakeLanguageRowButton(WidgetTree, CppRow, ResumeLanguageRunLabel(GI, ECodingLanguage::Cpp), GI && GI->DoesLanguageSaveExist(ECodingLanguage::Cpp));

        UHorizontalBox* PythonRow = MakeLanguageActionRow(WidgetTree, Box);
        PythonLanguageBtn = MakeLanguageRowButton(WidgetTree, PythonRow, NewLanguageRunLabel(ECodingLanguage::Python), true);
        ResumePythonBtn = MakeLanguageRowButton(WidgetTree, PythonRow, ResumeLanguageRunLabel(GI, ECodingLanguage::Python), GI && GI->DoesLanguageSaveExist(ECodingLanguage::Python));

        UHorizontalBox* MATLABRow = MakeLanguageActionRow(WidgetTree, Box);
        MATLABLanguageBtn = MakeLanguageRowButton(WidgetTree, MATLABRow, NewLanguageRunLabel(ECodingLanguage::MATLAB), true);
        ResumeMATLABBtn = MakeLanguageRowButton(WidgetTree, MATLABRow, ResumeLanguageRunLabel(GI, ECodingLanguage::MATLAB), GI && GI->DoesLanguageSaveExist(ECodingLanguage::MATLAB));
    }
    else
    {
        JavaLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT JAVA"));
        CLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT C"));
        CPlusLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT C+"));
        CppLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT C++"));
        PythonLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT PYTHON"));
        MATLABLanguageBtn = MakeMenuButton(WidgetTree, Box, TEXT("SELECT MATLAB"));
    }

    // Informational text lives BELOW the actionable rows and never eats
    // clicks (HitTestInvisible). At short windows it scrolls; buttons don't.
    LanguageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectedLanguageText"));
    LanguageText->SetAutoWrapText(true);
    LanguageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.92f, 0.76f, 1.0f)));
    LanguageText->SetVisibility(ESlateVisibility::HitTestInvisible);
    {
        FSlateFontInfo LangFont = LanguageText->GetFont();
        LangFont.Size = 15;
        LanguageText->SetFont(LangFont);
    }
    UVerticalBoxSlot* LangTextSlot = Box->AddChildToVerticalBox(LanguageText);
    LangTextSlot->SetPadding(FMargin(0, 10, 0, 4));

    if (bLaunchLanguageOnly)
    {
        FirstSessionRoutePreviewText = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("FirstSessionRoutePreviewText"));
        FirstSessionRoutePreviewText->SetAutoWrapText(true);
        FirstSessionRoutePreviewText->SetJustification(ETextJustify::Left);
        CodeRescueUI::StyleText(
            FirstSessionRoutePreviewText,
            CodeRescueUI::EType::Caption,
            CodeRescueUI::Color::TerminalGreen(),
            false);
        FirstSessionRoutePreviewText->SetVisibility(ESlateVisibility::HitTestInvisible);
        UVerticalBoxSlot* PreviewSlot = Box->AddChildToVerticalBox(FirstSessionRoutePreviewText);
        PreviewSlot->SetPadding(FMargin(0, 6, 0, 8));
    }

    if (!bLaunchLanguageOnly)
    {
        SettingsBtn = MakeMenuButton(WidgetTree, Box, TEXT("SETTINGS"));
        TutorialBtn = MakeMenuButton(WidgetTree, Box, TEXT("REPLAY TUTORIAL"));
        CreditsBtn  = MakeMenuButton(WidgetTree, Box, TEXT("CREDITS"));
        QuitBtn     = MakeMenuButton(WidgetTree, Box, TEXT("QUIT"));

        NewGameBtn->OnClicked.AddDynamic(this,  &UCodeRescueMainMenuWidget::OnNewGameClicked);
        ContinueBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnContinueClicked);
        SandboxBtn->OnClicked.AddDynamic(this,  &UCodeRescueMainMenuWidget::OnSandboxClicked);
        SettingsBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnSettingsClicked);
        TutorialBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnTutorialClicked);
        CreditsBtn->OnClicked.AddDynamic(this,  &UCodeRescueMainMenuWidget::OnCreditsClicked);
        QuitBtn->OnClicked.AddDynamic(this,     &UCodeRescueMainMenuWidget::OnQuitClicked);
    }
    JavaLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnJavaLanguageClicked);
    CLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnCLanguageClicked);
    CPlusLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnCPlusLanguageClicked);
    CppLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnCppLanguageClicked);
    PythonLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnPythonLanguageClicked);
    MATLABLanguageBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnMATLABLanguageClicked);
    if (bLaunchLanguageOnly)
    {
        ResumeJavaBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumeJavaClicked);
        ResumeCBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumeCClicked);
        ResumeCPlusBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumeCPlusClicked);
        ResumeCppBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumeCppClicked);
        ResumePythonBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumePythonClicked);
        ResumeMATLABBtn->OnClicked.AddDynamic(this, &UCodeRescueMainMenuWidget::OnResumeMATLABClicked);
    }

    RefreshLanguageText();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = true;
        SetKeyboardFocus();
    }

    if (bLaunchLanguageOnly)
    {
        UE_LOG(LogTemp, Display, TEXT("[CodeRescueLaunchLanguageMenu] Launch-only language widget ready: Java, C, C+, C++, Python, MATLAB."));
    }
}

void UCodeRescueMainMenuWidget::OnNewGameClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->StartFreshLanguageRun(GI->SelectedLanguage);
    }
    RestoreGameInputBeforeTravel();
    UGameplayStatics::OpenLevel(GetWorld(), CampaignMapName);
}

void UCodeRescueMainMenuWidget::OnContinueClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->ResumeLanguageRun(GI->SelectedLanguage);
    }
    RestoreGameInputBeforeTravel();
    UGameplayStatics::OpenLevel(GetWorld(), CampaignMapName);
}

void UCodeRescueMainMenuWidget::OnSandboxClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->bHasSelectedLaunchLanguageThisSession = true;
    }
    RestoreGameInputBeforeTravel();
    UGameplayStatics::OpenLevel(GetWorld(), SandboxMapName);
}

void UCodeRescueMainMenuWidget::OnSettingsClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UCodeRescueSettingsWidget* W = CreateWidget<UCodeRescueSettingsWidget>(PC, UCodeRescueSettingsWidget::StaticClass()))
        {
            W->AddToViewport(150);
        }
    }
}

void UCodeRescueMainMenuWidget::OnTutorialClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UCodeRescueTutorialWidget* W = CreateWidget<UCodeRescueTutorialWidget>(PC, UCodeRescueTutorialWidget::StaticClass()))
        {
            W->AddToViewport(2000);
        }
    }
}

void UCodeRescueMainMenuWidget::OnCreditsClicked()
{
    // Tag the tagline as a scrolling credits roll. Replacing widget contents
    // in-place is simpler than spawning a separate widget.
    if (TaglineText)
    {
        TaglineText->SetText(FText::FromString(
            TEXT("CODE & DESIGN — Kenny Sparks\n")
            TEXT("CURRICULUM — Operation Code Rescue\n")
            TEXT("ENGINE — Unreal Engine 5.7\n")
            TEXT("ZOMBIE PACKS — Yarrawah, Andryuha1981, RamsterZ, PxItiger, rivai\n")
            TEXT("BUILT WITH HELP FROM — Anthropic Claude")));
    }
}

void UCodeRescueMainMenuWidget::OnQuitClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCodeRescueMainMenuWidget::SetSelectedLanguage(ECodingLanguage Language)
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SelectedLanguage = Language;
        GI->SaveSlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language);
    }
    RefreshLanguageText();
}

bool UCodeRescueMainMenuWidget::NativeSupportsKeyboardFocus() const
{
    return true;
}

ECodingLanguage UCodeRescueMainMenuWidget::LanguageForMenuIndex(int32 Index) const
{
    static const ECodingLanguage Order[6] = {
        ECodingLanguage::Java, ECodingLanguage::C, ECodingLanguage::CPlus,
        ECodingLanguage::Cpp, ECodingLanguage::Python, ECodingLanguage::MATLAB };
    const int32 Clamped = ((Index % 6) + 6) % 6;
    return Order[Clamped];
}

int32 UCodeRescueMainMenuWidget::MenuIndexForLanguage(ECodingLanguage Language) const
{
    static const ECodingLanguage Order[6] = {
        ECodingLanguage::Java, ECodingLanguage::C, ECodingLanguage::CPlus,
        ECodingLanguage::Cpp, ECodingLanguage::Python, ECodingLanguage::MATLAB };
    for (int32 i = 0; i < 6; ++i)
    {
        if (Order[i] == Language)
        {
            return i;
        }
    }
    return 0;
}

void UCodeRescueMainMenuWidget::CycleSelectedLanguage(int32 Delta)
{
    ECodingLanguage Current = ECodingLanguage::Java;
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        Current = GI->SelectedLanguage;
    }
    SetSelectedLanguage(LanguageForMenuIndex(MenuIndexForLanguage(Current) + Delta));
}

FReply UCodeRescueMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    // Enter (or gamepad A) confirms the currently selected language and starts that focused run.
    if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
    {
        ECodingLanguage Selected = ECodingLanguage::Java;
        if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            Selected = GI->SelectedLanguage;
        }
        StartLanguageRun(Selected);
        return FReply::Handled();
    }

    // Arrow keys / d-pad move the highlighted language; Refresh shows the new selection.
    if (Key == EKeys::Up || Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_DPad_Left)
    {
        CycleSelectedLanguage(-1);
        return FReply::Handled();
    }
    if (Key == EKeys::Down || Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_DPad_Right)
    {
        CycleSelectedLanguage(1);
        return FReply::Handled();
    }

    // Number keys 1..6 select a language directly (still confirmed with Enter).
    const FKey NumberKeys[6] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };
    for (int32 i = 0; i < 6; ++i)
    {
        if (Key == NumberKeys[i])
        {
            SetSelectedLanguage(LanguageForMenuIndex(i));
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCodeRescueMainMenuWidget::RestoreGameInputBeforeTravel()
{
    // 2026-07-06: the menu takes FInputModeUIOnly + cursor on construct. Every
    // deploy path must hand input back to the GAME before the level travels,
    // or the reloaded world starts with menu-tainted input (a prime suspect in
    // the packaged "character completely locked" report). Also clears the
    // static UI-open flag, which survives OpenLevel.
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }
    ACodeRescueCharacter::SetUIOpen(false);
    UE_LOG(LogTemp, Display, TEXT("[LaunchGate] deploy: game input restored before travel"));
}

void UCodeRescueMainMenuWidget::StartLanguageRun(ECodingLanguage Language)
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->StartFreshLanguageRun(Language);
    }
    RefreshLanguageText();
    RestoreGameInputBeforeTravel();
    UGameplayStatics::OpenLevel(GetWorld(), CampaignMapName);
}

void UCodeRescueMainMenuWidget::ResumeLanguageRun(ECodingLanguage Language)
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->ResumeLanguageRun(Language);
    }
    RefreshLanguageText();
    RestoreGameInputBeforeTravel();
    UGameplayStatics::OpenLevel(GetWorld(), CampaignMapName);
}

void UCodeRescueMainMenuWidget::RefreshLanguageText()
{
    if (!LanguageText)
    {
        return;
    }

    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const ECodingLanguage Language = GI ? GI->SelectedLanguage : ECodingLanguage::Java;
    const FString Body = bLaunchLanguageOnly
        ? TEXT("Choose NEW to start a fresh language-locked run, or RESUME to continue that language's saved progress. This screen appears every time the game starts. Preview uses the current/default profile; each row deploys only the clicked language.")
        : FString::Printf(TEXT("SELECTED TRAINING LANGUAGE: %s\nStart and continue use the %s save profile."),
            *MenuLanguageName(Language),
            *MenuLanguageName(Language));
    LanguageText->SetText(FText::FromString(Body));

    if (FirstSessionRoutePreviewText)
    {
        const FString Preview = GI
            ? GI->GetFirstSessionRoutePreviewSummary(Language)
                + TEXT("\n")
                + GI->GetLaunchLanguageSaveRosterSummary()
            : TEXT("FIRST-SESSION ROUTE PREVIEW\nTrack: Java only | Slot: OperationCodeRescue_Language_Java | No save yet\nFirst route: protected terminal -> survivor marker -> extraction\nStart-screen choice remains first: NEW starts a clean profile; RESUME reloads only this language save.\nLANGUAGE SAVE ROSTER\nJava: NEW RUN READY | C: NEW RUN READY | C+: NEW RUN READY\nC++: NEW RUN READY | Python: NEW RUN READY | MATLAB: NEW RUN READY");
        FirstSessionRoutePreviewText->SetText(FText::FromString(Preview));
    }
}

void UCodeRescueMainMenuWidget::OnJavaLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::Java) : SetSelectedLanguage(ECodingLanguage::Java); }
void UCodeRescueMainMenuWidget::OnCLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::C) : SetSelectedLanguage(ECodingLanguage::C); }
void UCodeRescueMainMenuWidget::OnCPlusLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::CPlus) : SetSelectedLanguage(ECodingLanguage::CPlus); }
void UCodeRescueMainMenuWidget::OnCppLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::Cpp) : SetSelectedLanguage(ECodingLanguage::Cpp); }
void UCodeRescueMainMenuWidget::OnPythonLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::Python) : SetSelectedLanguage(ECodingLanguage::Python); }
void UCodeRescueMainMenuWidget::OnMATLABLanguageClicked() { bLaunchLanguageOnly ? StartLanguageRun(ECodingLanguage::MATLAB) : SetSelectedLanguage(ECodingLanguage::MATLAB); }
void UCodeRescueMainMenuWidget::OnResumeJavaClicked() { ResumeLanguageRun(ECodingLanguage::Java); }
void UCodeRescueMainMenuWidget::OnResumeCClicked() { ResumeLanguageRun(ECodingLanguage::C); }
void UCodeRescueMainMenuWidget::OnResumeCPlusClicked() { ResumeLanguageRun(ECodingLanguage::CPlus); }
void UCodeRescueMainMenuWidget::OnResumeCppClicked() { ResumeLanguageRun(ECodingLanguage::Cpp); }
void UCodeRescueMainMenuWidget::OnResumePythonClicked() { ResumeLanguageRun(ECodingLanguage::Python); }
void UCodeRescueMainMenuWidget::OnResumeMATLABClicked() { ResumeLanguageRun(ECodingLanguage::MATLAB); }


void UCodeRescueMainMenuWidget::DriveDeploySelected()
{
    ECodingLanguage Selected = ECodingLanguage::Java;
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        Selected = GI->SelectedLanguage;
    }
    StartLanguageRun(Selected);
}

void UCodeRescueMainMenuWidget::NativeDestruct()
{
    if (ActiveLaunchMenu.Get() == this)
    {
        ActiveLaunchMenu = nullptr;
    }
    Super::NativeDestruct();
}
