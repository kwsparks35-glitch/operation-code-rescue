#include "CodeRescueSkillTreeWidget.h"
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

namespace
{
constexpr int32 SkillTreeNodeCount = 8;
constexpr int32 SkillTreeNodeCost = 2;

void MirrorSkillTreeThemeFromSettings(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return;
    }

    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
}

UTextBlock* MakeSkillTreeLabel(
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

UButton* MakeSkillTreeButton(UWidgetTree* Tree, UVerticalBox* Parent, const FName& Name, UTextBlock*& OutLabel)
{
    UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    Button->SetBackgroundColor(CodeRescueUI::Resolve(CodeRescueUI::Surface::ButtonFill()));
    Button->SetColorAndOpacity(FLinearColor::White);

    OutLabel = MakeSkillTreeLabel(Tree, TEXT(""), CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextPrimary());
    Button->AddChild(OutLabel);

    UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Button);
    Slot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 5.0f));
    return Button;
}

int32 CountUnlockedSkills(const UCodeRescueGameInstance* GI)
{
    int32 Count = 0;
    for (int32 NodeIndex = 0; NodeIndex < SkillTreeNodeCount; ++NodeIndex)
    {
        Count += (GI && GI->IsSkillUnlocked(NodeIndex)) ? 1 : 0;
    }
    return Count;
}

FString SkillCategory(int32 NodeIndex)
{
    static const TCHAR* Categories[] = {
        TEXT("Mobility"),
        TEXT("Weapons"),
        TEXT("Weapons"),
        TEXT("Field Gear"),
        TEXT("Field Gear"),
        TEXT("Field Medicine"),
        TEXT("Survivability"),
        TEXT("Engineering"),
    };
    return (NodeIndex >= 0 && NodeIndex < UE_ARRAY_COUNT(Categories)) ? Categories[NodeIndex] : TEXT("General");
}

FString SkillStatusLabel(bool bUnlocked, bool bCanAfford, int32 RP)
{
    if (bUnlocked)
    {
        return TEXT("UNLOCKED");
    }
    if (bCanAfford)
    {
        return FString::Printf(TEXT("READY - spend %d RP"), SkillTreeNodeCost);
    }

    const int32 Needed = FMath::Max(0, SkillTreeNodeCost - RP);
    return FString::Printf(TEXT("LOCKED - need %d more RP"), Needed);
}

FLinearColor SkillNodeFill(bool bUnlocked, bool bCanAfford)
{
    if (bUnlocked)
    {
        return CodeRescueUI::Theme().bHighContrast
            ? FLinearColor(0.02f, 0.16f, 0.07f, 0.98f)
            : FLinearColor(0.05f, 0.13f, 0.07f, 0.96f);
    }
    if (bCanAfford)
    {
        return CodeRescueUI::Theme().bHighContrast
            ? FLinearColor(0.22f, 0.14f, 0.03f, 0.98f)
            : FLinearColor(0.16f, 0.11f, 0.05f, 0.96f);
    }
    return CodeRescueUI::Theme().bHighContrast
        ? FLinearColor(0.032f, 0.032f, 0.030f, 0.98f)
        : CodeRescueUI::Surface::ButtonFill();
}

FLinearColor SkillNodeTextColor(bool bUnlocked, bool bCanAfford)
{
    if (bUnlocked)
    {
        return CodeRescueUI::Color::TerminalGreenBright();
    }
    if (bCanAfford)
    {
        return CodeRescueUI::Color::AccentAmber();
    }
    return CodeRescueUI::Color::TextSecondary();
}

FString SkillTreeMilestoneLine(const UCodeRescueGameInstance* GI, int32 UnlockedCount)
{
    if (!GI)
    {
        return TEXT("Progression data unavailable.");
    }
    if (UnlockedCount >= SkillTreeNodeCount)
    {
        return TEXT("All field upgrades active in this saved language run.");
    }

    const int32 RemainingRP = FMath::Max(0, SkillTreeNodeCost - GI->ResearchPoints);
    if (GI->ResearchPoints >= SkillTreeNodeCost)
    {
        return TEXT("Next step: select any READY node to apply it immediately.");
    }
    return FString::Printf(TEXT("Next step: solve terminals or rescue survivors for %d more RP."), RemainingRP);
}
}

void UCodeRescueSkillTreeWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueSkillTreeWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueSkillTreeWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    MirrorSkillTreeThemeFromSettings(GetGameInstance<UCodeRescueGameInstance>());

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SkillTreeRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("SkillTreeBlur"));
    Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 1.0f : 4.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BlurSlot->SetOffsets(FMargin(0.0f));

    PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SkillTreePanel"));
    CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Panel(), FMargin(18.0f, 16.0f));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelFrame);
    PanelSlot->SetAnchors(FAnchors(0.24f, 0.08f, 0.76f, 0.92f));
    PanelSlot->SetOffsets(FMargin(0.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SkillTreeStack"));
    PanelFrame->SetContent(Stack);

    TitleText = MakeSkillTreeLabel(WidgetTree, TEXT("FIELD SKILL TREE"), CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    Stack->AddChildToVerticalBox(TitleText);

    PointsText = MakeSkillTreeLabel(WidgetTree, TEXT(""), CodeRescueUI::EType::Subheading, CodeRescueUI::Color::TerminalGreenBright());
    Stack->AddChildToVerticalBox(PointsText);

    SummaryText = MakeSkillTreeLabel(WidgetTree, TEXT(""), CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
    UVerticalBoxSlot* SummarySlot = Stack->AddChildToVerticalBox(SummaryText);
    SummarySlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 6.0f));

    FeedbackText = MakeSkillTreeLabel(WidgetTree, TEXT("Select a READY node to spend Research Points."), CodeRescueUI::EType::Caption, CodeRescueUI::Color::AccentAmber(), false);
    Stack->AddChildToVerticalBox(FeedbackText);

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SkillTreeScroll"));
    UVerticalBoxSlot* ScrollSlot = Stack->AddChildToVerticalBox(Scroll);
    ScrollSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 8.0f));
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    NodeList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SkillTreeNodes"));
    Scroll->AddChild(NodeList);

    SkillButtons.Reset();
    SkillLabels.Reset();
    for (int32 NodeIndex = 0; NodeIndex < SkillTreeNodeCount; ++NodeIndex)
    {
        UTextBlock* NodeLabel = nullptr;
        UButton* Button = MakeSkillTreeButton(
            WidgetTree,
            NodeList,
            FName(*FString::Printf(TEXT("SkillNode%d"), NodeIndex)),
            NodeLabel);
        SkillButtons.Add(Button);
        SkillLabels.Add(NodeLabel);
    }

    SkillButtons[0]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill0Clicked);
    SkillButtons[1]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill1Clicked);
    SkillButtons[2]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill2Clicked);
    SkillButtons[3]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill3Clicked);
    SkillButtons[4]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill4Clicked);
    SkillButtons[5]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill5Clicked);
    SkillButtons[6]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill6Clicked);
    SkillButtons[7]->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnSkill7Clicked);

    CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SkillTreeClose"));
    CodeRescueUI::StyleSecondaryButton(CloseButton);
    CloseButton->OnClicked.AddDynamic(this, &UCodeRescueSkillTreeWidget::OnCloseClicked);
    UTextBlock* CloseLabel = MakeSkillTreeLabel(WidgetTree, TEXT("Close"), CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentAmber());
    CloseLabel->SetJustification(ETextJustify::Center);
    CloseButton->AddChild(CloseLabel);
    Stack->AddChildToVerticalBox(CloseButton);

    ACodeRescueCharacter::SetUIOpen(true);
    SetFeedback(TEXT("Select a READY node to spend Research Points; unlocks save to the active language run."), CodeRescueUI::Color::AccentAmber());
    Refresh();
}

void UCodeRescueSkillTreeWidget::NativeDestruct()
{
    ACodeRescueCharacter::SetUIOpen(false);
    Super::NativeDestruct();
}

FString UCodeRescueSkillTreeWidget::SkillName(int32 NodeIndex)
{
    static const TCHAR* Names[] = {
        TEXT("Endurance Training"),
        TEXT("Rapid Reload"),
        TEXT("Extended Pistol Magazine"),
        TEXT("Flare Reserve"),
        TEXT("Smoke Reserve"),
        TEXT("Stim Reserve"),
        TEXT("Field Conditioning"),
        TEXT("Efficient Barricades"),
    };
    return (NodeIndex >= 0 && NodeIndex < UE_ARRAY_COUNT(Names)) ? Names[NodeIndex] : TEXT("Unknown Skill");
}

FString UCodeRescueSkillTreeWidget::SkillDescription(int32 NodeIndex)
{
    static const TCHAR* Descriptions[] = {
        TEXT("+20 max stamina"),
        TEXT("-25% reload time across the weapon loadout"),
        TEXT("+6 Balanced Handgun magazine capacity"),
        TEXT("Start every applied run state with at least 4 flares"),
        TEXT("Start every applied run state with at least 3 smoke charges"),
        TEXT("Start every applied run state with at least 3 stims"),
        TEXT("+25 max health and an immediate health bump"),
        TEXT("-1 scrap cost for barricades"),
    };
    return (NodeIndex >= 0 && NodeIndex < UE_ARRAY_COUNT(Descriptions)) ? Descriptions[NodeIndex] : TEXT("");
}

void UCodeRescueSkillTreeWidget::Refresh()
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorSkillTreeThemeFromSettings(GI);

    if (PanelFrame)
    {
        CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Panel(), FMargin(18.0f, 16.0f));
    }
    if (TitleText)
    {
        CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    }

    const int32 RP = GI ? GI->ResearchPoints : 0;
    const int32 UnlockedCount = CountUnlockedSkills(GI);
    if (PointsText)
    {
        PointsText->SetText(FText::FromString(FString::Printf(
            TEXT("Research Points %d | Skills %d/%d | Cost %d RP"),
            RP,
            UnlockedCount,
            SkillTreeNodeCount,
            SkillTreeNodeCost)));
        CodeRescueUI::StyleText(PointsText, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::TerminalGreenBright());
    }

    if (SummaryText)
    {
        const FString LanguageName = GI ? GI->GetLanguageName() : TEXT("selected language");
        const FString SaveSlot = GI ? GI->SaveSlotName : TEXT("active save slot");
        const FString LearningLine = GI ? GI->GetLearningProgressSummary() : TEXT("Learning progress unavailable");
        const FString LanguageLine = GI ? GI->GetLanguageProgressSummary() : TEXT("Language progress unavailable");
        SummaryText->SetText(FText::FromString(FString::Printf(
            TEXT("%s run | %s | %s\nSave: unlocks persist immediately to %s\n%s"),
            *LanguageName,
            *LanguageLine,
            *LearningLine,
            *SaveSlot,
            *SkillTreeMilestoneLine(GI, UnlockedCount))));
        CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TextSecondary(), false);
    }

    if (FeedbackText)
    {
        CodeRescueUI::StyleText(FeedbackText, CodeRescueUI::EType::Caption, FeedbackColor, false);
    }

    for (int32 NodeIndex = 0; NodeIndex < SkillLabels.Num(); ++NodeIndex)
    {
        const bool bUnlocked = GI && GI->IsSkillUnlocked(NodeIndex);
        const bool bCanAfford = GI && RP >= SkillTreeNodeCost && !bUnlocked;
        if (SkillLabels[NodeIndex])
        {
            SkillLabels[NodeIndex]->SetText(FText::FromString(FString::Printf(
                TEXT("%s | %s | %s\nOutcome: %s\nProgression: applies now and remains saved to this language run."),
                *SkillStatusLabel(bUnlocked, bCanAfford, RP),
                *SkillCategory(NodeIndex),
                *SkillName(NodeIndex),
                *SkillDescription(NodeIndex))));
            CodeRescueUI::StyleText(SkillLabels[NodeIndex], CodeRescueUI::EType::BodySmall, SkillNodeTextColor(bUnlocked, bCanAfford));
        }
        if (SkillButtons.IsValidIndex(NodeIndex) && SkillButtons[NodeIndex])
        {
            SkillButtons[NodeIndex]->SetBackgroundColor(CodeRescueUI::Resolve(SkillNodeFill(bUnlocked, bCanAfford)));
            SkillButtons[NodeIndex]->SetColorAndOpacity(FLinearColor::White);
            SkillButtons[NodeIndex]->SetIsEnabled(!bUnlocked);
        }
    }
}

void UCodeRescueSkillTreeWidget::TryUnlockNode(int32 NodeIndex)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        SetFeedback(TEXT("Skill tree progression data is unavailable."), CodeRescueUI::Color::DangerBright());
        return;
    }

    if (GI->IsSkillUnlocked(NodeIndex))
    {
        SetFeedback(FString::Printf(TEXT("Already unlocked: %s."), *SkillName(NodeIndex)), CodeRescueUI::Color::TerminalGreenBright());
    }
    else if (GI->ResearchPoints < SkillTreeNodeCost)
    {
        const int32 Needed = SkillTreeNodeCost - GI->ResearchPoints;
        SetFeedback(
            FString::Printf(TEXT("Need %d more Research Point%s before unlocking %s."),
                Needed,
                Needed == 1 ? TEXT("") : TEXT("s"),
                *SkillName(NodeIndex)),
            CodeRescueUI::Color::Warning());
    }
    else if (GI->TryUnlockSkill(NodeIndex))
    {
        SetFeedback(
            FString::Printf(TEXT("Unlocked: %s. Saved to the %s run."), *SkillName(NodeIndex), *GI->GetLanguageName()),
            CodeRescueUI::Color::TerminalGreenBright());
    }
    else
    {
        SetFeedback(TEXT("Skill could not be unlocked."), CodeRescueUI::Color::DangerBright());
    }
    Refresh();
}

void UCodeRescueSkillTreeWidget::SetFeedback(const FString& Message, const FLinearColor& Color)
{
    if (!FeedbackText)
    {
        return;
    }
    FeedbackText->SetText(FText::FromString(Message));
    FeedbackColor = Color;
    CodeRescueUI::StyleText(FeedbackText, CodeRescueUI::EType::Caption, Color, false);
}

void UCodeRescueSkillTreeWidget::Close()
{
    RemoveFromParent();
}

void UCodeRescueSkillTreeWidget::OnCloseClicked() { Close(); }
void UCodeRescueSkillTreeWidget::OnSkill0Clicked() { TryUnlockNode(0); }
void UCodeRescueSkillTreeWidget::OnSkill1Clicked() { TryUnlockNode(1); }
void UCodeRescueSkillTreeWidget::OnSkill2Clicked() { TryUnlockNode(2); }
void UCodeRescueSkillTreeWidget::OnSkill3Clicked() { TryUnlockNode(3); }
void UCodeRescueSkillTreeWidget::OnSkill4Clicked() { TryUnlockNode(4); }
void UCodeRescueSkillTreeWidget::OnSkill5Clicked() { TryUnlockNode(5); }
void UCodeRescueSkillTreeWidget::OnSkill6Clicked() { TryUnlockNode(6); }
void UCodeRescueSkillTreeWidget::OnSkill7Clicked() { TryUnlockNode(7); }
