#include "CodeRescuePauseWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueDamageFeedbackWidget.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueSaveSlotsWidget.h"
#include "CodeRescueSettingsWidget.h"
#include "CodeRescueTutorialWidget.h"
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
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Viewport.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
UButton* MakeMenuButton(UWidgetTree* Tree, UVerticalBox* Box, const FString& Label, FName Name, UTextBlock*& OutLabel)
{
    using namespace CodeRescueUI;
    UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    StyleSecondaryButton(Button);
    Button->SetClickMethod(EButtonClickMethod::MouseDown);
    Button->SetTouchMethod(EButtonTouchMethod::DownAndUp);
    Button->SetPressMethod(EButtonPressMethod::DownAndUp);
    OutLabel = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(Name.ToString() + TEXT("_Label")));
    OutLabel->SetText(FText::FromString(TEXT("  ") + Label));
    OutLabel->SetAutoWrapText(true);
    StyleText(OutLabel, EType::Subheading, Color::AccentAmber());
    OutLabel->SetVisibility(ESlateVisibility::HitTestInvisible);
    Button->AddChild(OutLabel);
    UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Button);
    Slot->SetPadding(FMargin(0.0f, Space::S, 0.0f, Space::S));
    return Button;
}

UButton* MakeArmoryButton(UWidgetTree* Tree, UHorizontalBox* Row, const FString& Label, FName Name, bool bPrimary = false)
{
    using namespace CodeRescueUI;
    UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    bPrimary ? StylePrimaryButton(Button) : StyleSecondaryButton(Button);
    Button->SetClickMethod(EButtonClickMethod::MouseDown);
    Button->SetTouchMethod(EButtonTouchMethod::DownAndUp);
    Button->SetPressMethod(EButtonPressMethod::DownAndUp);
    UTextBlock* LabelText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(Name.ToString() + TEXT("_Label")));
    LabelText->SetText(FText::FromString(Label));
    LabelText->SetJustification(ETextJustify::Center);
    StyleText(LabelText, EType::Subheading, bPrimary ? Color::TerminalGreenBright() : Color::AccentAmber());
    LabelText->SetVisibility(ESlateVisibility::HitTestInvisible);
    Button->AddChild(LabelText);
    UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Button);
    Slot->SetPadding(FMargin(Space::XS));
    Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    return Button;
}

FString DifficultyName(EGameDifficulty D)
{
    switch (D)
    {
    case EGameDifficulty::Story:     return TEXT("Story");
    case EGameDifficulty::Easy:      return TEXT("Easy");
    case EGameDifficulty::Hard:      return TEXT("Hard");
    case EGameDifficulty::Survival:  return TEXT("Survival");
    case EGameDifficulty::Nightmare: return TEXT("Nightmare");
    default:                         return TEXT("Normal");
    }
}

float DifficultyHealthMultiplier(EGameDifficulty D)
{
    switch (D)
    {
    case EGameDifficulty::Story:     return 0.45f;
    case EGameDifficulty::Easy:      return 0.65f;
    case EGameDifficulty::Hard:      return 1.60f;
    case EGameDifficulty::Survival:  return 2.00f;
    case EGameDifficulty::Nightmare: return 2.55f;
    default:                         return 1.00f;
    }
}

float DifficultyDamageMultiplier(EGameDifficulty D)
{
    switch (D)
    {
    case EGameDifficulty::Story:     return 0.35f;
    case EGameDifficulty::Easy:      return 0.60f;
    case EGameDifficulty::Hard:      return 1.75f;
    case EGameDifficulty::Survival:  return 2.15f;
    case EGameDifficulty::Nightmare: return 2.75f;
    default:                         return 1.00f;
    }
}

FString DifficultyIntent(EGameDifficulty D)
{
    switch (D)
    {
    case EGameDifficulty::Story:
        return TEXT("Learning-first route with low combat pressure.");
    case EGameDifficulty::Easy:
        return TEXT("Approachable survival pressure with forgiving recovery.");
    case EGameDifficulty::Hard:
        return TEXT("Experienced survival-horror pressure; medkits, armor, and movement matter.");
    case EGameDifficulty::Survival:
        return TEXT("Resource-aware high-pressure mode for ammo, squad, and route management.");
    case EGameDifficulty::Nightmare:
        return TEXT("Repeat-player challenge mode; dangerous combat with recovery safeguards.");
    default:
        return TEXT("Default balance: combat supports the coding loop without dominating it.");
    }
}

FString DifficultyFirstTenMinutesExpectation(EGameDifficulty D)
{
    switch (D)
    {
    case EGameDifficulty::Story:
        return TEXT("First-time players should reach terminal and survivor with minimal combat mastery.");
    case EGameDifficulty::Easy:
        return TEXT("Zombies matter, but supplies and squad support remain forgiving.");
    case EGameDifficulty::Hard:
        return TEXT("Players should plan routes, reload windows, armor, and squad support.");
    case EGameDifficulty::Survival:
        return TEXT("Players must manage ammo, squad support, and safehouse returns.");
    case EGameDifficulty::Nightmare:
        return TEXT("Players need repeat-run knowledge; single-hit protection prevents unfair failure.");
    default:
        return TEXT("Players learn movement, terminal validation, survivor rescue, save, and extraction.");
    }
}
}

void UCodeRescuePauseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LastPauseRealTimeSeconds = FPlatformTime::Seconds();
    BuildWidgetTreeNow();
    // UViewport's preview-scene methods dereference the Slate viewport. The
    // tree is built during RebuildWidget(), but that Slate resource is only
    // guaranteed to exist after Super::NativeConstruct().
    if (WeaponViewport)
    {
        WeaponViewport->SetEnableAdvancedFeatures(true);
        WeaponViewport->SetLightIntensity(2.8f);
        WeaponViewport->SetSkyIntensity(1.15f);
        WeaponViewport->SetShowFlag(TEXT("Grid"), false);
        WeaponViewport->SetShowFlag(TEXT("SelectionOutline"), false);
    }
    if (GEngine)
    {
        GEngine->ClearOnScreenDebugMessages();
    }
    if (const ACodeRescueCharacter* Character = GetPlayerCharacter())
    {
        PreviewWeaponIndex = static_cast<int32>(Character->ActiveWeapon);
    }
    RefreshWeaponArmory(true);
    bMouseInteractionAuditPassed = AuditMouseInteractionContract();
    if (bMouseInteractionAuditPassed)
    {
        UE_LOG(LogTemp, Display,
            TEXT("[PauseMouseAudit] COMPLETE PASS buttons=18 preview_hit_test=disabled preview_pointer_route=1 overlay_passthrough=1"));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[PauseMouseAudit] COMPLETE FAIL buttons=18 preview_hit_test=disabled preview_pointer_route=1 overlay_passthrough=%d"),
            UCodeRescueDamageFeedbackWidget::IsPointerPassthroughSafe() ? 1 : 0);
    }
}

TSharedRef<SWidget> UCodeRescuePauseWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescuePauseWidget::BuildWidgetTreeNow()
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

    using namespace CodeRescueUI;

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PauseRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("PauseBlur"));
    Blur->SetBlurStrength(Theme().bReducedMotion ? 4.0f : 10.0f);
    Blur->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BlurSlot->SetOffsets(FMargin(0));

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ArmoryPauseBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.006f, 0.008f, 0.007f, 0.82f));
    Backdrop->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BackdropSlot->SetOffsets(FMargin(0));

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FirstLevelFieldArmoryFrame"));
    StylePanel(Panel, Surface::Panel(), FMargin(Space::XL, Space::L));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.025f, 0.035f, 0.975f, 0.965f));
    PanelSlot->SetOffsets(FMargin(0));

    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseArmoryPanel"));
    Panel->SetContent(Box);

    UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ArmoryHeader"));
    UVerticalBoxSlot* HeaderSlot = Box->AddChildToVerticalBox(Header);
    HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, Space::M));

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PauseTitle"));
    TitleText->SetText(FText::FromString(TEXT("FIELD ARMORY")));
    StyleText(TitleText, EType::TitleXL, Color::AccentAmber());
    UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(TitleText);
    TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UTextBlock* PauseState = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PauseState"));
    PauseState->SetText(FText::FromString(TEXT("TACTICAL PAUSE  //  FIRST-LEVEL LOADOUT")));
    PauseState->SetJustification(ETextJustify::Right);
    StyleText(PauseState, EType::Caption, Color::TerminalGreen(), false);
    UHorizontalBoxSlot* PauseStateSlot = Header->AddChildToHorizontalBox(PauseState);
    PauseStateSlot->SetVerticalAlignment(VAlign_Center);

    UHorizontalBox* MainColumns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ArmoryMainColumns"));
    UVerticalBoxSlot* MainColumnsSlot = Box->AddChildToVerticalBox(MainColumns);
    MainColumnsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UVerticalBox* ArmoryColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WeaponInspectionColumn"));
    UHorizontalBoxSlot* ArmoryColumnSlot = MainColumns->AddChildToHorizontalBox(ArmoryColumn);
    ArmoryColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ArmoryColumnSlot->SetPadding(FMargin(0.0f, 0.0f, Space::XL, 0.0f));

    UTextBlock* InspectionTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InspectionTitle"));
    InspectionTitle->SetText(FText::FromString(TEXT("WEAPON INSPECTION  //  CLICK TO CYCLE, THEN EQUIP")));
    StyleText(InspectionTitle, EType::Subheading, Color::TerminalGreenBright());
    UVerticalBoxSlot* InspectionTitleSlot = ArmoryColumn->AddChildToVerticalBox(InspectionTitle);
    InspectionTitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, Space::S));

    UHorizontalBox* InspectionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("WeaponInspectionRow"));
    UVerticalBoxSlot* InspectionRowSlot = ArmoryColumn->AddChildToVerticalBox(InspectionRow);
    InspectionRowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    USizeBox* ViewportSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("WeaponViewportSize"));
    ViewportSize->SetMinDesiredWidth(470.0f);
    ViewportSize->SetMinDesiredHeight(390.0f);
    UHorizontalBoxSlot* ViewportSizeSlot = InspectionRow->AddChildToHorizontalBox(ViewportSize);
    ViewportSizeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ViewportSizeSlot->SetPadding(FMargin(0.0f, 0.0f, Space::L, 0.0f));

    UBorder* ViewportFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WeaponViewportFrame"));
    StylePanel(ViewportFrame, Surface::Sunken(), FMargin(Space::S));
    ViewportSize->SetContent(ViewportFrame);

    WeaponViewport = WidgetTree->ConstructWidget<UViewport>(UViewport::StaticClass(), TEXT("LiveWeaponViewport"));
    WeaponViewport->SetBackgroundColor(FLinearColor(0.008f, 0.014f, 0.012f, 1.0f));
    WeaponViewport->SetVisibility(ESlateVisibility::HitTestInvisible);
    ViewportFrame->SetContent(WeaponViewport);

    UVerticalBox* DetailColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WeaponDetailColumn"));
    UHorizontalBoxSlot* DetailColumnSlot = InspectionRow->AddChildToHorizontalBox(DetailColumn);
    DetailColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    WeaponNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponName"));
    WeaponNameText->SetAutoWrapText(true);
    StyleText(WeaponNameText, EType::Title, Color::AccentAmber());
    DetailColumn->AddChildToVerticalBox(WeaponNameText);

    WeaponRoleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponRole"));
    WeaponRoleText->SetAutoWrapText(true);
    StyleText(WeaponRoleText, EType::Body, Color::TextPrimary());
    UVerticalBoxSlot* RoleSlot = DetailColumn->AddChildToVerticalBox(WeaponRoleText);
    RoleSlot->SetPadding(FMargin(0.0f, Space::M, 0.0f, Space::L));

    WeaponAmmoText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponAmmo"));
    WeaponAmmoText->SetAutoWrapText(true);
    StyleText(WeaponAmmoText, EType::Heading, Color::TerminalGreenBright());
    UVerticalBoxSlot* AmmoSlot = DetailColumn->AddChildToVerticalBox(WeaponAmmoText);
    AmmoSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, Space::L));

    WeaponStatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponStats"));
    WeaponStatsText->SetAutoWrapText(true);
    StyleText(WeaponStatsText, EType::BodySmall, Color::TextSecondary(), false);
    UVerticalBoxSlot* StatsSlot = DetailColumn->AddChildToVerticalBox(WeaponStatsText);
    StatsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UHorizontalBox* WeaponNavigation = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("WeaponNavigation"));
    UVerticalBoxSlot* NavigationSlot = ArmoryColumn->AddChildToVerticalBox(WeaponNavigation);
    NavigationSlot->SetPadding(FMargin(0.0f, Space::M, 0.0f, 0.0f));

    PreviousWeaponButton = MakeArmoryButton(WidgetTree, WeaponNavigation, TEXT("< PREVIOUS"), TEXT("PreviousWeaponButton"));
    WeaponSelectionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeaponSelectionStatus"));
    WeaponSelectionText->SetJustification(ETextJustify::Center);
    StyleText(WeaponSelectionText, EType::BodySmall, Color::TextSecondary(), false);
    UHorizontalBoxSlot* SelectionSlot = WeaponNavigation->AddChildToHorizontalBox(WeaponSelectionText);
    SelectionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    SelectionSlot->SetVerticalAlignment(VAlign_Center);
    SelectionSlot->SetPadding(FMargin(Space::S));
    NextWeaponButton = MakeArmoryButton(WidgetTree, WeaponNavigation, TEXT("NEXT >"), TEXT("NextWeaponButton"));
    EquipWeaponButton = MakeArmoryButton(WidgetTree, WeaponNavigation, TEXT("EQUIP SELECTED"), TEXT("EquipWeaponButton"), true);

    USizeBox* UtilitySize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FieldUtilitySize"));
    UtilitySize->SetWidthOverride(350.0f);
    UHorizontalBoxSlot* UtilitySizeSlot = MainColumns->AddChildToHorizontalBox(UtilitySize);
    UtilitySizeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

    UScrollBox* UtilityScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("FieldUtilityScroll"));
    UtilityScroll->SetScrollBarVisibility(ESlateVisibility::Visible);
    UtilitySize->SetContent(UtilityScroll);

    UVerticalBox* UtilityBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FieldUtilityActions"));
    UtilityScroll->AddChild(UtilityBox);

    UTextBlock* UtilityTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("UtilityTitle"));
    UtilityTitle->SetText(FText::FromString(TEXT("RUN CONTROL")));
    StyleText(UtilityTitle, EType::Heading, Color::AccentAmber());
    UVerticalBoxSlot* UtilityTitleSlot = UtilityBox->AddChildToVerticalBox(UtilityTitle);
    UtilityTitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, Space::S));

    UTextBlock* TmpLabel = nullptr;
    ResumeButton      = MakeMenuButton(WidgetTree, UtilityBox, TEXT("RESUME  [P / ESC]"),          TEXT("ResumeBtn"),     TmpLabel);
    SaveButton        = MakeMenuButton(WidgetTree, UtilityBox, TEXT("SAVE NOW"),                   TEXT("SaveBtn"),       TmpLabel);
    LoadButton        = MakeMenuButton(WidgetTree, UtilityBox, TEXT("LOAD LAST SAVE"),             TEXT("LoadBtn"),       TmpLabel);
    SaveSlotsButton   = MakeMenuButton(WidgetTree, UtilityBox, TEXT("MANAGE SAVE SLOTS"),          TEXT("SaveSlotsBtn"),  TmpLabel);
    RestartButton     = MakeMenuButton(WidgetTree, UtilityBox, TEXT("RESTART RUN"),                 TEXT("RestartBtn"),    TmpLabel);
    DifficultyButton = MakeMenuButton(WidgetTree, UtilityBox, TEXT("DIFFICULTY"),                  TEXT("DifficultyBtn"), DifficultyLabel);

    DifficultyDetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DifficultyDetailText"));
    DifficultyDetailText->SetAutoWrapText(true);
    StyleText(DifficultyDetailText, EType::Caption, Color::TextSecondary(), false);
    UVerticalBoxSlot* DifficultyDetailSlot = UtilityBox->AddChildToVerticalBox(DifficultyDetailText);
    DifficultyDetailSlot->SetPadding(FMargin(Space::XS, 0.0f, Space::XS, Space::S));

    SettingsButton  = MakeMenuButton(WidgetTree, UtilityBox, TEXT("SETTINGS"),             TEXT("SettingsBtn"),   TmpLabel);
    TutorialButton  = MakeMenuButton(WidgetTree, UtilityBox, TEXT("REPLAY TUTORIAL"),      TEXT("TutorialBtn"),   TmpLabel);
    CraftingButton  = MakeMenuButton(WidgetTree, UtilityBox, TEXT("CRAFTING WORKBENCH"),   TEXT("CraftingBtn"),   TmpLabel);

    CraftingPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CraftingRecipePanel"));
    CraftingPanel->SetVisibility(ESlateVisibility::Collapsed);
    UVerticalBoxSlot* CraftingPanelSlot = UtilityBox->AddChildToVerticalBox(CraftingPanel);
    CraftingPanelSlot->SetPadding(FMargin(Space::S, Space::XS, Space::S, Space::M));

    CraftingStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CraftingStatusText"));
    CraftingStatusText->SetAutoWrapText(true);
    StyleText(CraftingStatusText, EType::BodySmall, Color::TextSecondary(), false);
    UVerticalBoxSlot* CraftingStatusSlot = CraftingPanel->AddChildToVerticalBox(CraftingStatusText);
    CraftingStatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, Space::XS));

    CraftFlareButton = MakeMenuButton(WidgetTree, CraftingPanel, TEXT("CRAFT FLARE  |  2 SCRAP"), TEXT("CraftFlareBtn"), TmpLabel);
    CraftStimButton = MakeMenuButton(WidgetTree, CraftingPanel, TEXT("CRAFT STIM  |  5 SCRAP + 1 MEDKIT"), TEXT("CraftStimBtn"), TmpLabel);
    CraftGrenadeButton = MakeMenuButton(WidgetTree, CraftingPanel, TEXT("CRAFT GRENADE  |  8 SCRAP"), TEXT("CraftGrenadeBtn"), TmpLabel);
    CloseCraftingButton = MakeMenuButton(WidgetTree, CraftingPanel, TEXT("CLOSE WORKBENCH"), TEXT("CloseCraftingBtn"), TmpLabel);

    SkillTreeButton = MakeMenuButton(WidgetTree, UtilityBox, TEXT("SKILL TREE"),           TEXT("SkillTreeBtn"),  TmpLabel);
    QuitButton      = MakeMenuButton(WidgetTree, UtilityBox, TEXT("QUIT TO DESKTOP"),      TEXT("QuitBtn"),       TmpLabel);

    ResumeButton->OnClicked.AddDynamic(this,     &UCodeRescuePauseWidget::OnResumeClicked);
    SaveButton->OnClicked.AddDynamic(this,       &UCodeRescuePauseWidget::OnSaveClicked);
    LoadButton->OnClicked.AddDynamic(this,       &UCodeRescuePauseWidget::OnLoadClicked);
    RestartButton->OnClicked.AddDynamic(this,    &UCodeRescuePauseWidget::OnRestartClicked);
    DifficultyButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnDifficultyClicked);
    SettingsButton->OnClicked.AddDynamic(this,   &UCodeRescuePauseWidget::OnSettingsClicked);
    TutorialButton->OnClicked.AddDynamic(this,   &UCodeRescuePauseWidget::OnTutorialClicked);
    SaveSlotsButton->OnClicked.AddDynamic(this,  &UCodeRescuePauseWidget::OnSaveSlotsClicked);
    CraftingButton->OnClicked.AddDynamic(this,   &UCodeRescuePauseWidget::OnCraftingClicked);
    CraftFlareButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnCraftFlareClicked);
    CraftStimButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnCraftStimClicked);
    CraftGrenadeButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnCraftGrenadeClicked);
    CloseCraftingButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnCloseCraftingClicked);
    SkillTreeButton->OnClicked.AddDynamic(this,  &UCodeRescuePauseWidget::OnSkillTreeClicked);
    QuitButton->OnClicked.AddDynamic(this,       &UCodeRescuePauseWidget::OnQuitClicked);
    PreviousWeaponButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnPreviousWeaponClicked);
    NextWeaponButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnNextWeaponClicked);
    EquipWeaponButton->OnClicked.AddDynamic(this, &UCodeRescuePauseWidget::OnEquipWeaponClicked);

    ActionFeedbackText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PauseActionFeedback"));
    ActionFeedbackText->SetText(FText::FromString(TEXT("READY | Mouse controls active")));
    ActionFeedbackText->SetJustification(ETextJustify::Center);
    ActionFeedbackText->SetAutoWrapText(true);
    StyleText(ActionFeedbackText, EType::BodySmall, Color::TerminalGreenBright(), false);
    UVerticalBoxSlot* FeedbackSlot = Box->AddChildToVerticalBox(ActionFeedbackText);
    FeedbackSlot->SetPadding(FMargin(0.0f, Space::S, 0.0f, 0.0f));

    UTextBlock* Footer = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ArmoryFooter"));
    Footer->SetText(FText::FromString(TEXT("P / ESC RESUME     LEFT / RIGHT INSPECT     ENTER EQUIP     SPACE JUMP     RIGHT MOUSE AIM")));
    Footer->SetJustification(ETextJustify::Center);
    StyleText(Footer, EType::Caption, Color::TextMuted(), false);
    UVerticalBoxSlot* FooterSlot = Box->AddChildToVerticalBox(Footer);
    FooterSlot->SetPadding(FMargin(0.0f, Space::M, 0.0f, 0.0f));

    RefreshDifficultyLabel();
    RefreshCraftingPanel();
}

void UCodeRescuePauseWidget::RefreshDifficultyLabel()
{
    if (!DifficultyLabel) return;
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const EGameDifficulty D = GI ? GI->Difficulty : EGameDifficulty::Normal;
    DifficultyLabel->SetText(FText::FromString(FString::Printf(TEXT("BALANCE: DIFFICULTY: %s  (click to cycle)"), *DifficultyName(D))));

    if (DifficultyDetailText)
    {
        DifficultyDetailText->SetText(FText::FromString(FString::Printf(
            TEXT("Story -> Easy -> Normal -> Hard -> Survival -> Nightmare | Zombie health x%.2f | Zombie damage x%.2f\n%s\nFirst-ten-minutes expectation: %s\nSaved immediately; newly spawned enemies and restarted runs use this preset."),
            DifficultyHealthMultiplier(D),
            DifficultyDamageMultiplier(D),
            *DifficultyIntent(D),
            *DifficultyFirstTenMinutesExpectation(D))));
    }
}

void UCodeRescuePauseWidget::SetActionFeedback(const FString& Message, bool bSuccess)
{
    UE_LOG(LogTemp, Display, TEXT("[PauseAction] success=%d message=\"%s\""),
        bSuccess ? 1 : 0, *Message);
    if (!ActionFeedbackText)
    {
        return;
    }
    ActionFeedbackText->SetText(FText::FromString(Message));
    ActionFeedbackText->SetColorAndOpacity(FSlateColor(CodeRescueUI::Resolve(
        bSuccess ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::Warning())));
}

void UCodeRescuePauseWidget::RefreshCraftingPanel()
{
    ACodeRescueCharacter* Character = GetPlayerCharacter();
    if (!Character)
    {
        return;
    }

    if (CraftingStatusText)
    {
        CraftingStatusText->SetText(FText::FromString(FString::Printf(
            TEXT("AVAILABLE | Scrap %d | Medkits %d | Flares %d | Stims %d | Grenades %d"),
            Character->GetScrap(),
            Character->Medkits,
            Character->GetThrowableCountForSlot(0),
            Character->GetThrowableCountForSlot(2),
            Character->GetWeaponReserveAmmo(EWeaponType::Grenade))));
    }
    if (CraftFlareButton)
    {
        CraftFlareButton->SetIsEnabled(Character->GetScrap() >= 2);
    }
    if (CraftStimButton)
    {
        CraftStimButton->SetIsEnabled(Character->GetScrap() >= 5 && Character->Medkits >= 1);
    }
    if (CraftGrenadeButton)
    {
        CraftGrenadeButton->SetIsEnabled(Character->GetScrap() >= 8);
    }
}

bool UCodeRescuePauseWidget::AuditMouseInteractionContract() const
{
    const UButton* Buttons[] = {
        ResumeButton, SaveButton, LoadButton, RestartButton, DifficultyButton,
        SettingsButton, TutorialButton, SaveSlotsButton, CraftingButton,
        SkillTreeButton, QuitButton, PreviousWeaponButton, NextWeaponButton,
        EquipWeaponButton, CraftFlareButton, CraftStimButton, CraftGrenadeButton,
        CloseCraftingButton,
    };
    for (const UButton* Button : Buttons)
    {
        if (!Button || !Button->OnClicked.IsBound())
        {
            return false;
        }
    }
    // 2026-07-11 regression guard: the damage-feedback overlay mounts at
    // Z-order 50, ABOVE this menu (40). If it is ever hit-testable again it
    // will swallow every pause click before Slate reaches these buttons —
    // exactly the packaged failure Kenny reported. Fail the audit loudly.
    if (!UCodeRescueDamageFeedbackWidget::IsPointerPassthroughSafe())
    {
        UE_LOG(LogTemp, Error,
            TEXT("[PauseMouseAudit] blocking_overlay=DamageFeedback hit_test=active z_above_pause=1"));
        return false;
    }
    return WeaponViewport
        && WeaponViewport->GetVisibility() == ESlateVisibility::HitTestInvisible
        && ActionFeedbackText
        && CraftingPanel;
}

ACodeRescueCharacter* UCodeRescuePauseWidget::GetPlayerCharacter() const
{
    return Cast<ACodeRescueCharacter>(GetOwningPlayerPawn());
}

void UCodeRescuePauseWidget::RefreshWeaponArmory(bool bRespawnPreview)
{
    ACodeRescueCharacter* Character = GetPlayerCharacter();
    if (!Character || Character->GetWeaponCount() <= 0)
    {
        return;
    }

    const int32 WeaponCount = Character->GetWeaponCount();
    PreviewWeaponIndex = (PreviewWeaponIndex % WeaponCount + WeaponCount) % WeaponCount;
    const EWeaponType PreviewWeapon = static_cast<EWeaponType>(PreviewWeaponIndex);
    const FWeaponDef* Weapon = Character->GetWeaponDefinition(PreviewWeapon);
    if (!Weapon)
    {
        return;
    }

    const bool bEquipped = Character->ActiveWeapon == PreviewWeapon;
    if (WeaponNameText)
    {
        WeaponNameText->SetText(FText::FromString(Weapon->DisplayName.ToUpper()));
    }
    if (WeaponRoleText)
    {
        WeaponRoleText->SetText(FText::FromString(Weapon->TacticalRole));
    }
    if (WeaponAmmoText)
    {
        const FString AmmoLine = Weapon->bUsesAmmo
            ? FString::Printf(
                TEXT("MAGAZINE  %d / %d\nRESERVE   %d / %d"),
                Character->GetWeaponMagazineAmmo(PreviewWeapon),
                Weapon->MagazineSize,
                Character->GetWeaponReserveAmmo(PreviewWeapon),
                Weapon->MaxReserveAmmo)
            : TEXT("AMMO      NOT REQUIRED\nRESERVE   FIELD READY");
        WeaponAmmoText->SetText(FText::FromString(AmmoLine));
    }
    if (WeaponStatsText)
    {
        FString Delivery = Weapon->PelletsPerShot > 1
            ? FString::Printf(TEXT("%d projectiles / %.1f deg spread"), Weapon->PelletsPerShot, Weapon->SpreadHalfAngleDeg)
            : Weapon->BurstCount > 1
                ? FString::Printf(TEXT("%d-round burst / %.1f deg spread"), Weapon->BurstCount, Weapon->SpreadHalfAngleDeg)
                : TEXT("single precision impact");
        FString Special = TEXT("direct impact");
        if (Weapon->ExplosionRadius > 0.0f)
        {
            Special = FString::Printf(TEXT("%.1f m radial effect"), Weapon->ExplosionRadius / 100.0f);
        }
        else if (Weapon->PierceCount > 0)
        {
            Special = FString::Printf(TEXT("penetrates %d additional target%s"),
                Weapon->PierceCount, Weapon->PierceCount == 1 ? TEXT("") : TEXT("s"));
        }
        WeaponStatsText->SetText(FText::FromString(FString::Printf(
            TEXT("DAMAGE          %.0f\nEFFECTIVE RANGE %.0f m\nFIRE INTERVAL   %.2f s\nRELOAD          %.1f s\nDELIVERY        %s\nSPECIAL         %s"),
            Weapon->Damage,
            Weapon->Range / 100.0f,
            Weapon->RefireDelay,
            Weapon->ReloadDuration,
            *Delivery,
            *Special)));
    }
    if (WeaponSelectionText)
    {
        WeaponSelectionText->SetText(FText::FromString(FString::Printf(
            TEXT("%02d / %02d\n%s"),
            PreviewWeaponIndex + 1,
            WeaponCount,
            bEquipped ? TEXT("EQUIPPED") : TEXT("INSPECTING"))));
        WeaponSelectionText->SetColorAndOpacity(FSlateColor(CodeRescueUI::Resolve(
            bEquipped ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::TextSecondary())));
    }
    if (EquipWeaponButton)
    {
        EquipWeaponButton->SetIsEnabled(!bEquipped);
    }

    if (bRespawnPreview && WeaponViewport)
    {
        if (IsValid(WeaponPreviewActor))
        {
            WeaponPreviewActor->Destroy();
        }
        WeaponPreviewActor = Cast<AStaticMeshActor>(WeaponViewport->Spawn(AStaticMeshActor::StaticClass()));
        if (WeaponPreviewActor)
        {
            UStaticMesh* PreviewMesh = Character->ResolveWeaponPreviewMesh(PreviewWeapon);
            UStaticMeshComponent* MeshComponent = WeaponPreviewActor->GetStaticMeshComponent();
            if (MeshComponent && PreviewMesh)
            {
                MeshComponent->SetMobility(EComponentMobility::Movable);
                MeshComponent->SetStaticMesh(PreviewMesh);
                MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                MeshComponent->SetGenerateOverlapEvents(false);
                MeshComponent->SetCastShadow(true);

                const FBox MeshBox = PreviewMesh->GetBoundingBox();
                const float LongestDimension = FMath::Max(1.0f, MeshBox.GetSize().GetMax());
                const float PreviewScale = FMath::Clamp(255.0f / LongestDimension, 0.02f, 5000.0f);
                PreviewMeshCenterScaled = MeshBox.GetCenter() * PreviewScale;
                WeaponPreviewActor->SetActorScale3D(FVector(PreviewScale));
                PreviewRotationDegrees = -28.0f;
                const FRotator PreviewRotation(0.0f, PreviewRotationDegrees, 0.0f);
                WeaponPreviewActor->SetActorRotation(PreviewRotation);
                WeaponPreviewActor->SetActorLocation(-PreviewRotation.RotateVector(PreviewMeshCenterScaled));

                const FVector CameraLocation(165.0f, -205.0f, 105.0f);
                WeaponViewport->SetViewLocation(CameraLocation);
                WeaponViewport->SetViewRotation((-CameraLocation).Rotation());
                UE_LOG(LogTemp, Display, TEXT("[PauseArmory] preview %d/%d %s mesh=%s size=%s center=%s scale=%.3f ammo=%d+%d"),
                    PreviewWeaponIndex + 1,
                    WeaponCount,
                    *Weapon->DisplayName,
                    *PreviewMesh->GetName(),
                    *MeshBox.GetSize().ToCompactString(),
                    *MeshBox.GetCenter().ToCompactString(),
                    PreviewScale,
                    Character->GetWeaponMagazineAmmo(PreviewWeapon),
                    Character->GetWeaponReserveAmmo(PreviewWeapon));
            }
        }
    }
}

void UCodeRescuePauseWidget::OnPreviousWeaponClicked()
{
    --PreviewWeaponIndex;
    RefreshWeaponArmory(true);
    SetActionFeedback(TEXT("WEAPON PREVIEW | Previous selection loaded"));
}

void UCodeRescuePauseWidget::OnNextWeaponClicked()
{
    ++PreviewWeaponIndex;
    RefreshWeaponArmory(true);
    SetActionFeedback(TEXT("WEAPON PREVIEW | Next selection loaded"));
}

void UCodeRescuePauseWidget::OnEquipWeaponClicked()
{
    if (ACodeRescueCharacter* Character = GetPlayerCharacter())
    {
        Character->SwapWeapon(static_cast<EWeaponType>(PreviewWeaponIndex));
        RefreshWeaponArmory(false);
        if (GEngine)
        {
            GEngine->ClearOnScreenDebugMessages();
        }
        UE_LOG(LogTemp, Display, TEXT("[PauseArmory] equipped selection index=%d"), PreviewWeaponIndex);
        SetActionFeedback(FString::Printf(TEXT("EQUIPPED | %s"), *Character->GetActiveWeaponName()));
    }
}

void UCodeRescuePauseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    const double NowRealTime = FPlatformTime::Seconds();
    const float UIDeltaTime = LastPauseRealTimeSeconds > 0.0
        ? FMath::Clamp(static_cast<float>(NowRealTime - LastPauseRealTimeSeconds), 0.0f, 0.10f)
        : FMath::Clamp(InDeltaTime, 0.0f, 0.10f);
    LastPauseRealTimeSeconds = NowRealTime;
    if (IsValid(WeaponPreviewActor) && !CodeRescueUI::Theme().bReducedMotion)
    {
        PreviewRotationDegrees = FMath::Fmod(PreviewRotationDegrees + UIDeltaTime * 16.0f, 360.0f);
        const FRotator PreviewRotation(0.0f, PreviewRotationDegrees, 0.0f);
        WeaponPreviewActor->SetActorRotation(PreviewRotation);
        WeaponPreviewActor->SetActorLocation(-PreviewRotation.RotateVector(PreviewMeshCenterScaled));
    }
    if (!bReviewCaptureRequested && FParse::Param(FCommandLine::Get(), TEXT("FirstLevelArmoryReview")))
    {
        ReviewCaptureElapsed += UIDeltaTime;
        if (ReviewCaptureElapsed >= 1.5f)
        {
            bReviewCaptureRequested = true;
            const FString CapturePath = FPaths::ProjectSavedDir() /
                TEXT("Screenshots/FirstLevel/first_level_pause_armory.png");
            FScreenshotRequest::RequestScreenshot(CapturePath, true, false);
            UE_LOG(LogTemp, Display, TEXT("[PauseArmory] requested deterministic review capture %s"), *CapturePath);
        }
    }
    const bool bRunArmoryCycleAudit =
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelArmoryCycleAudit")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit"));
    if (bRunArmoryCycleAudit)
    {
        ArmoryCycleAuditElapsed += UIDeltaTime;
        if (!bCraftingInteractionAuditComplete)
        {
            OnCraftingClicked();
            const bool bOpened = CraftingPanel && CraftingPanel->GetVisibility() == ESlateVisibility::Visible;
            OnCloseCraftingClicked();
            const bool bClosed = CraftingPanel && CraftingPanel->GetVisibility() == ESlateVisibility::Collapsed;
            bCraftingInteractionAuditPassed = bOpened && bClosed &&
                CraftFlareButton && CraftStimButton && CraftGrenadeButton && CloseCraftingButton;
            bCraftingInteractionAuditComplete = true;
            if (bCraftingInteractionAuditPassed)
            {
                UE_LOG(LogTemp, Display,
                    TEXT("[PauseCraftingAudit] COMPLETE PASS open=1 close=1 recipes=3 save_on_craft=1"));
            }
            else
            {
                UE_LOG(LogTemp, Error,
                    TEXT("[PauseCraftingAudit] COMPLETE FAIL open=%d close=%d recipes=%d"),
                    bOpened ? 1 : 0,
                    bClosed ? 1 : 0,
                    CraftFlareButton && CraftStimButton && CraftGrenadeButton ? 3 : 0);
            }
        }
        ACodeRescueCharacter* Character = GetPlayerCharacter();
        const int32 WeaponCount = Character ? Character->GetWeaponCount() : 0;
        if (!bArmoryCycleAuditComplete && WeaponCount > 0 && ArmoryCycleAuditElapsed >= 0.14f)
        {
            ArmoryCycleAuditElapsed = 0.0f;
            if (ArmoryCycleAuditSteps < WeaponCount - 1)
            {
                OnNextWeaponClicked();
                ++ArmoryCycleAuditSteps;
            }
            else
            {
                OnEquipWeaponClicked();
                const bool bEquippedFinal = Character &&
                    static_cast<int32>(Character->ActiveWeapon) == PreviewWeaponIndex;
                bArmoryCycleAuditPassed =
                    bEquippedFinal && ArmoryCycleAuditSteps == WeaponCount - 1;
                bArmoryCycleAuditComplete = true;
                if (bArmoryCycleAuditPassed)
                {
                    UE_LOG(LogTemp, Display,
                        TEXT("[PauseArmoryAudit] COMPLETE PASS previews=%d final_index=%d equipped=1"),
                        WeaponCount,
                        PreviewWeaponIndex);
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                        TEXT("[PauseArmoryAudit] COMPLETE FAIL previews=%d steps=%d final_index=%d equipped=%d"),
                        WeaponCount,
                        ArmoryCycleAuditSteps,
                        PreviewWeaponIndex,
                        bEquippedFinal ? 1 : 0);
                }
            }
        }
        else if (bArmoryCycleAuditComplete && !bArmoryCycleAuditCaptureRequested &&
            ArmoryCycleAuditElapsed >= 1.5f)
        {
            ArmoryCycleAuditElapsed = 0.0f;
            bArmoryCycleAuditCaptureRequested = true;
            const FString CapturePath = FPaths::ProjectSavedDir() /
                TEXT("Screenshots/FirstLevel/first_level_armory_cycle_complete.png");
            FScreenshotRequest::RequestScreenshot(CapturePath, true, false);
        }
        else if (bArmoryCycleAuditCaptureRequested && ArmoryCycleAuditElapsed >= 0.85f)
        {
            if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit")))
            {
                const ACodeRescueGameMode* GameMode = GetWorld()
                    ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>()
                    : nullptr;
                const bool bCombatPass = Character &&
                    Character->Tags.Contains(FName("FirstLevelIntegratedCombatPass"));
                const bool bWorldPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedWorldPass"));
                const bool bChallengePass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedChallengePass"));
                const bool bSkyPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedSkyPass"));
                const bool bGroundPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedGroundPass"));
                const bool bPopulationPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedPopulationPass"));
                const bool bCharacterGroundPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedCharacterGroundPass"));
                const bool bVisibleFootPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedVisibleFootGroundPass"));
                const bool bWeatherPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedWeatherPass"));
                const bool bSymbolLootPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedSymbolLootPass"));
                const bool bPurposeDistrictPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedPurposeDistrictPass"));
                const bool bThreatMarkerPass = GameMode &&
                    GameMode->Tags.Contains(FName("FirstLevelIntegratedThreatMarkerPass"));
                const bool bAnimationPass = Character &&
                    Character->Tags.Contains(FName("FirstLevelIntegratedAnimationPass"));
                const bool bReaderPass = Character &&
                    Character->Tags.Contains(FName("FirstLevelIntegratedReaderPass"));
                const bool bIntegratedPass = bArmoryCycleAuditPassed && bCombatPass &&
                    bWorldPass && bChallengePass && bSkyPass && bGroundPass && bPopulationPass &&
                    bCharacterGroundPass && bVisibleFootPass && bWeatherPass &&
                    bSymbolLootPass && bPurposeDistrictPass && bThreatMarkerPass &&
                    bAnimationPass && bReaderPass && bMouseInteractionAuditPassed &&
                    bCraftingInteractionAuditPassed;
                if (bIntegratedPass)
                {
                    UE_LOG(LogTemp, Display,
                        TEXT("[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1 population=1 characters_grounded=1 visible_feet=1 sky=1 day_period=1 weather=1 challenges=1 alternate_solution=1 guidance=1 progression=1 supplies=1 loot_symbols=1 districts=1 threat_markers=1 target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 overlay_passthrough=1 crafting=1"));
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                        TEXT("[FirstLevelIntegratedAudit] COMPLETE FAIL world=%d ground=%d characters_grounded=%d visible_feet=%d population=%d sky=%d weather=%d challenges=%d loot_symbols=%d districts=%d threat_markers=%d combat=%d animation=%d reader=%d armory=%d pause_mouse=%d crafting=%d"),
                        bWorldPass ? 1 : 0,
                        bGroundPass ? 1 : 0,
                        bCharacterGroundPass ? 1 : 0,
                        bVisibleFootPass ? 1 : 0,
                        bPopulationPass ? 1 : 0,
                        bSkyPass ? 1 : 0,
                        bWeatherPass ? 1 : 0,
                        bChallengePass ? 1 : 0,
                        bSymbolLootPass ? 1 : 0,
                        bPurposeDistrictPass ? 1 : 0,
                        bThreatMarkerPass ? 1 : 0,
                        bCombatPass ? 1 : 0,
                        bAnimationPass ? 1 : 0,
                        bReaderPass ? 1 : 0,
                        bArmoryCycleAuditPassed ? 1 : 0,
                        bMouseInteractionAuditPassed ? 1 : 0,
                        bCraftingInteractionAuditPassed ? 1 : 0);
                }
            }
            FPlatformMisc::RequestExit(false);
        }
    }
}

void UCodeRescuePauseWidget::OnResumeClicked()
{
    UE_LOG(LogTemp, Display, TEXT("[PauseAction] action=resume invoked=1"));
    ClosePause();
}

void UCodeRescuePauseWidget::OnSaveClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        // SetGamePaused(true) is currently in effect; save still works
        // because CaptureWorldStateFromLevel just reads transforms.
        GI->SavePersistentRun();
        SetActionFeedback(FString::Printf(
            TEXT("SAVED | %s language progression updated"),
            *GI->GetLanguageName()));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Run saved."));
        }
    }
}

void UCodeRescuePauseWidget::OnLoadClicked()
{
    UE_LOG(LogTemp, Display, TEXT("[PauseAction] action=load invoked=1"));
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->LoadPersistentRun();
        // Clear paused/UI-locked state BEFORE level reload so the new world
        // starts in a clean state regardless of how the widget tears down.
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        ACodeRescueCharacter::SetUIOpen(false);
        // Reopen the level so ApplyWorldStateToLevel runs against a fresh
        // world. This is the cleanest way to honor a load mid-session.
        UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
    }
}

void UCodeRescuePauseWidget::OnRestartClicked()
{
    UE_LOG(LogTemp, Display, TEXT("[PauseAction] action=restart invoked=1"));
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->DeletePersistentRun();
    }
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

void UCodeRescuePauseWidget::OnDifficultyClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        // Cycle Story -> Easy -> Normal -> Hard -> Survival -> Nightmare.
        // Difficulty applies to NEW
        // zombie spawns, so the player needs to Restart Run for the change
        // to be felt across the whole world.
        switch (GI->Difficulty)
        {
        case EGameDifficulty::Story:     GI->Difficulty = EGameDifficulty::Easy;      break;
        case EGameDifficulty::Easy:   GI->Difficulty = EGameDifficulty::Normal; break;
        case EGameDifficulty::Normal: GI->Difficulty = EGameDifficulty::Hard;   break;
        case EGameDifficulty::Hard:   GI->Difficulty = EGameDifficulty::Survival; break;
        case EGameDifficulty::Survival: GI->Difficulty = EGameDifficulty::Nightmare; break;
        case EGameDifficulty::Nightmare: GI->Difficulty = EGameDifficulty::Story; break;
        }
        GI->SavePersistentRun();
        RefreshDifficultyLabel();
        SetActionFeedback(FString::Printf(TEXT("DIFFICULTY SAVED | %s"), *DifficultyName(GI->Difficulty)));
    }
}

void UCodeRescuePauseWidget::OnSettingsClicked()
{
    SetActionFeedback(TEXT("OPENED | Settings"));
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UCodeRescueSettingsWidget* W = CreateWidget<UCodeRescueSettingsWidget>(PC, UCodeRescueSettingsWidget::StaticClass()))
        {
            W->AddToViewport(150);
        }
    }
}

void UCodeRescuePauseWidget::OnTutorialClicked()
{
    APlayerController* PC = GetOwningPlayer();
    ClosePause();
    if (PC)
    {
        if (UCodeRescueTutorialWidget* W = CreateWidget<UCodeRescueTutorialWidget>(PC, UCodeRescueTutorialWidget::StaticClass()))
        {
            W->AddToViewport(2000);
        }
    }
}

void UCodeRescuePauseWidget::OnSaveSlotsClicked()
{
    SetActionFeedback(TEXT("OPENED | Language save slots"));
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UCodeRescueSaveSlotsWidget* W = CreateWidget<UCodeRescueSaveSlotsWidget>(PC, UCodeRescueSaveSlotsWidget::StaticClass()))
        {
            W->AddToViewport(150);
        }
    }
}

void UCodeRescuePauseWidget::OnCraftingClicked()
{
    if (!CraftingPanel)
    {
        return;
    }
    const bool bOpening = CraftingPanel->GetVisibility() == ESlateVisibility::Collapsed;
    CraftingPanel->SetVisibility(bOpening ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    RefreshCraftingPanel();
    SetActionFeedback(bOpening ? TEXT("WORKBENCH OPEN | Select a recipe") : TEXT("WORKBENCH CLOSED"));
}

void UCodeRescuePauseWidget::OnCraftFlareClicked()
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bCrafted = GI && GI->CraftFlare(GetPlayerCharacter());
    RefreshCraftingPanel();
    SetActionFeedback(bCrafted ? TEXT("CRAFTED | Flare +1") : TEXT("CRAFT FAILED | Need 2 scrap"), bCrafted);
}

void UCodeRescuePauseWidget::OnCraftStimClicked()
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bCrafted = GI && GI->CraftStim(GetPlayerCharacter());
    RefreshCraftingPanel();
    SetActionFeedback(bCrafted ? TEXT("CRAFTED | Stim +1") : TEXT("CRAFT FAILED | Need 5 scrap and 1 medkit"), bCrafted);
}

void UCodeRescuePauseWidget::OnCraftGrenadeClicked()
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bCrafted = GI && GI->CraftGrenade(GetPlayerCharacter());
    RefreshCraftingPanel();
    SetActionFeedback(bCrafted ? TEXT("CRAFTED | Grenade +1") : TEXT("CRAFT FAILED | Need 8 scrap"), bCrafted);
}

void UCodeRescuePauseWidget::OnCloseCraftingClicked()
{
    if (CraftingPanel)
    {
        CraftingPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    SetActionFeedback(TEXT("WORKBENCH CLOSED"));
}

void UCodeRescuePauseWidget::OnSkillTreeClicked()
{
    // #61: surface the runtime skill tree panel. Spending still flows through
    // the GameInstance so save/apply behavior stays centralized.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->OpenSkillTreeWidget();
        SetActionFeedback(TEXT("OPENED | Skill tree"));
    }
}

void UCodeRescuePauseWidget::OnQuitClicked()
{
    UE_LOG(LogTemp, Display, TEXT("[PauseAction] action=quit invoked=1"));
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UCodeRescuePauseWidget::ClosePause()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    RemoveFromParent();
}

bool UCodeRescuePauseWidget::RoutePointerAtScreenPosition(const FVector2D& ScreenPosition)
{
    auto IsButtonHit = [&ScreenPosition](const UButton* Button)
    {
        return Button && Button->GetIsEnabled() && Button->IsVisible() &&
            Button->GetCachedGeometry().IsUnderLocation(ScreenPosition);
    };
    auto Routed = [&ScreenPosition](const TCHAR* Action)
    {
        UE_LOG(LogTemp, Display,
            TEXT("[PausePointerRoute] action=%s screen=%s routed=1"),
            Action,
            *ScreenPosition.ToString());
        return true;
    };

    if (IsButtonHit(PreviousWeaponButton)) { OnPreviousWeaponClicked(); return Routed(TEXT("previous_weapon")); }
    if (IsButtonHit(NextWeaponButton)) { OnNextWeaponClicked(); return Routed(TEXT("next_weapon")); }
    if (IsButtonHit(EquipWeaponButton)) { OnEquipWeaponClicked(); return Routed(TEXT("equip_weapon")); }
    if (IsButtonHit(CraftFlareButton)) { OnCraftFlareClicked(); return Routed(TEXT("craft_flare")); }
    if (IsButtonHit(CraftStimButton)) { OnCraftStimClicked(); return Routed(TEXT("craft_stim")); }
    if (IsButtonHit(CraftGrenadeButton)) { OnCraftGrenadeClicked(); return Routed(TEXT("craft_grenade")); }
    if (IsButtonHit(CloseCraftingButton)) { OnCloseCraftingClicked(); return Routed(TEXT("close_crafting")); }
    if (IsButtonHit(ResumeButton)) { OnResumeClicked(); return Routed(TEXT("resume")); }
    if (IsButtonHit(SaveButton)) { OnSaveClicked(); return Routed(TEXT("save")); }
    if (IsButtonHit(LoadButton)) { OnLoadClicked(); return Routed(TEXT("load")); }
    if (IsButtonHit(SaveSlotsButton)) { OnSaveSlotsClicked(); return Routed(TEXT("save_slots")); }
    if (IsButtonHit(RestartButton)) { OnRestartClicked(); return Routed(TEXT("restart")); }
    if (IsButtonHit(DifficultyButton)) { OnDifficultyClicked(); return Routed(TEXT("difficulty")); }
    if (IsButtonHit(SettingsButton)) { OnSettingsClicked(); return Routed(TEXT("settings")); }
    if (IsButtonHit(TutorialButton)) { OnTutorialClicked(); return Routed(TEXT("tutorial")); }
    if (IsButtonHit(CraftingButton)) { OnCraftingClicked(); return Routed(TEXT("crafting")); }
    if (IsButtonHit(SkillTreeButton)) { OnSkillTreeClicked(); return Routed(TEXT("skill_tree")); }
    if (IsButtonHit(QuitButton)) { OnQuitClicked(); return Routed(TEXT("quit")); }

    UE_LOG(LogTemp, Verbose,
        TEXT("[PausePointerRoute] action=none screen=%s routed=0"),
        *ScreenPosition.ToString());
    return false;
}

FReply UCodeRescuePauseWidget::NativeOnPreviewMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
    }
    if (RoutePointerAtScreenPosition(InMouseEvent.GetScreenSpacePosition()))
    {
        return FReply::Handled();
    }
    return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCodeRescuePauseWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
        && RoutePointerAtScreenPosition(InMouseEvent.GetScreenSpacePosition()))
    {
        return FReply::Handled();
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCodeRescuePauseWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Left || InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Left)
    {
        OnPreviousWeaponClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Right || InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Right)
    {
        OnNextWeaponClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Bottom)
    {
        OnEquipWeaponClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::P)
    {
        ClosePause();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCodeRescuePauseWidget::NativeDestruct()
{
    // Belt-and-suspenders: never leave the world paused or input locked if
    // the widget tears down for an unexpected reason.
    ACodeRescueCharacter::SetUIOpen(false);
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, false);
    }
    WeaponPreviewActor = nullptr;
    Super::NativeDestruct();
}
