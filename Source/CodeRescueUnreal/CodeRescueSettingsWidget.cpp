#include "CodeRescueSettingsWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueDamageFeedbackWidget.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "AudioDevice.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
USlider* MakeSlider(UWidgetTree* Tree, float Min, float Max, float Default)
{
    USlider* S = Tree->ConstructWidget<USlider>(USlider::StaticClass());
    S->SetMinValue(Min);
    S->SetMaxValue(Max);
    S->SetValue(Default);
    return S;
}

UTextBlock* MakeLabel(UWidgetTree* Tree, const FString& Text)
{
    UTextBlock* L = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    L->SetText(FText::FromString(Text));
    L->SetAutoWrapText(true);
    CodeRescueUI::StyleText(L, CodeRescueUI::EType::Body, CodeRescueUI::Color::TextPrimary());
    return L;
}

FString ColorblindModeName(EColorblindMode Mode)
{
    switch (Mode)
    {
    case EColorblindMode::Deuteranope: return TEXT("Deuteranope");
    case EColorblindMode::Protanope: return TEXT("Protanope");
    case EColorblindMode::Tritanope: return TEXT("Tritanope");
    default: return TEXT("Standard");
    }
}

EColorblindMode NextColorblindMode(EColorblindMode Mode)
{
    switch (Mode)
    {
    case EColorblindMode::None: return EColorblindMode::Deuteranope;
    case EColorblindMode::Deuteranope: return EColorblindMode::Protanope;
    case EColorblindMode::Protanope: return EColorblindMode::Tritanope;
    default: return EColorblindMode::None;
    }
}
}

void UCodeRescueSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueSettingsWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueSettingsWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SettingsRoot"));
    WidgetTree->RootWidget = Root;

    // Readable dark backdrop so settings text never fights the world/menu behind it.
    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SettingsBackdrop"));
    CodeRescueUI::StylePanel(Backdrop, FLinearColor(0.012f, 0.014f, 0.012f, 0.92f), FMargin(0.0f));
    {
        UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
        BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        BackdropSlot->SetOffsets(FMargin(0.0f));
    }

    TitleText = MakeLabel(WidgetTree, TEXT("SETTINGS"));
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    UCanvasPanelSlot* TitleSlot = Root->AddChildToCanvas(TitleText);
    TitleSlot->SetAnchors(FAnchors(0.5f, 0.05f, 0.5f, 0.05f));
    TitleSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    TitleSlot->SetSize(FVector2D(400.0f, 60.0f));

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SettingsScroll"));
    UCanvasPanelSlot* StackSlot = Root->AddChildToCanvas(Scroll);
    StackSlot->SetAnchors(FAnchors(0.5f, 0.14f, 0.5f, 0.92f));
    StackSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    StackSlot->SetSize(FVector2D(760.0f, 600.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsStack"));
    Scroll->AddChild(Stack);

    // Pull current settings to seed the UI.
    UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
    if (GUS)
    {
        bCachedFullscreen = (GUS->GetFullscreenMode() == EWindowMode::Fullscreen);
        bCachedVsync = GUS->IsVSyncEnabled();
    }
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        bCachedSubtitles = GI->bSubtitlesEnabled;
        bCachedHighContrast = GI->bHighContrastHUD;
        CachedColorblindMode = GI->ColorblindMode;
        bCachedReducedMotion = GI->bReducedMotion;
        bCachedSimplifiedHints = GI->bSimplifiedInputHints;
        bCachedMonoAudio = GI->bMonoAudio;
        bCachedVisualizeSoundCues = GI->bVisualizeSoundCues;
        if (bCachedMonoAudio)
        {
            bCachedVisualizeSoundCues = true;
        }
        CachedSubtitleScale = FMath::Clamp(GI->SubtitleScale, 0.75f, 1.75f);
        CachedUITextScale = GI->GetUITextScale();
        CachedAimAssistScale = FMath::Clamp(GI->AimAssistScale, 0.0f, 2.0f);
        CachedMaster = FMath::Clamp(GI->MasterVolume, 0.0f, 1.0f);
        CachedSfx = FMath::Clamp(GI->SfxVolume, 0.0f, 1.0f);
        CachedMusic = FMath::Clamp(GI->MusicVolume, 0.0f, 1.0f);

        // Mirror saved accessibility state into the shared UI design system so
        // every themed screen honors high contrast and text scale on open.
        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    auto AddRow = [&](const FString& LabelStr, UWidget* Control)
    {
        UTextBlock* L = MakeLabel(WidgetTree, LabelStr);
        Stack->AddChildToVerticalBox(L);
        Stack->AddChildToVerticalBox(Control);
    };

    MasterSlider = MakeSlider(WidgetTree, 0.0f, 1.0f, CachedMaster);
    MasterSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnMasterVolumeChanged);
    AddRow(TEXT("Master Volume"), MasterSlider);

    SfxSlider = MakeSlider(WidgetTree, 0.0f, 1.0f, CachedSfx);
    SfxSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnSfxVolumeChanged);
    AddRow(TEXT("SFX Volume"), SfxSlider);

    MusicSlider = MakeSlider(WidgetTree, 0.0f, 1.0f, CachedMusic);
    MusicSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnMusicVolumeChanged);
    AddRow(TEXT("Music Volume"), MusicSlider);

    SensitivitySlider = MakeSlider(WidgetTree, 0.25f, 3.0f, CachedSensitivity);
    SensitivitySlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnMouseSensitivityChanged);
    AddRow(TEXT("Mouse Sensitivity"), SensitivitySlider);

    FovSlider = MakeSlider(WidgetTree, 60.0f, 110.0f, CachedFov);
    FovSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnFovChanged);
    AddRow(TEXT("Field of View"), FovSlider);

    SubtitleScaleSlider = MakeSlider(WidgetTree, 0.75f, 1.75f, CachedSubtitleScale);
    SubtitleScaleSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnSubtitleScaleChanged);
    AddRow(TEXT("Subtitle Size"), SubtitleScaleSlider);

    UITextScaleSlider = MakeSlider(WidgetTree, 0.80f, 1.75f, CachedUITextScale);
    UITextScaleSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnUITextScaleChanged);
    AddRow(TEXT("UI Text Size"), UITextScaleSlider);

    AimAssistSlider = MakeSlider(WidgetTree, 0.0f, 2.0f, CachedAimAssistScale);
    AimAssistSlider->OnValueChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnAimAssistChanged);
    AddRow(TEXT("Aim Assist Strength"), AimAssistSlider);

    FullscreenCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    FullscreenCheck->SetIsChecked(bCachedFullscreen);
    FullscreenCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnFullscreenChanged);
    AddRow(TEXT("Fullscreen"), FullscreenCheck);

    VsyncCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    VsyncCheck->SetIsChecked(bCachedVsync);
    VsyncCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnVsyncChanged);
    AddRow(TEXT("VSync"), VsyncCheck);

    SubtitlesCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    SubtitlesCheck->SetIsChecked(bCachedSubtitles);
    SubtitlesCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnSubtitlesChanged);
    AddRow(TEXT("Subtitles"), SubtitlesCheck);

    HighContrastCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    HighContrastCheck->SetIsChecked(bCachedHighContrast);
    HighContrastCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnHighContrastChanged);
    AddRow(TEXT("High Contrast HUD"), HighContrastCheck);

    ColorblindModeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StyleSecondaryButton(ColorblindModeButton);
    ColorblindModeButton->OnClicked.AddDynamic(this, &UCodeRescueSettingsWidget::OnColorblindModeClicked);
    ColorblindModeButtonText = MakeLabel(WidgetTree, TEXT(""));
    ColorblindModeButton->AddChild(ColorblindModeButtonText);
    AddRow(TEXT("Color Vision Mode"), ColorblindModeButton);

    ReducedMotionCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    ReducedMotionCheck->SetIsChecked(bCachedReducedMotion);
    ReducedMotionCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnReducedMotionChanged);
    AddRow(TEXT("Reduced Motion"), ReducedMotionCheck);

    SimplifiedHintsCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    SimplifiedHintsCheck->SetIsChecked(bCachedSimplifiedHints);
    SimplifiedHintsCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnSimplifiedHintsChanged);
    AddRow(TEXT("Simplified Input Hints"), SimplifiedHintsCheck);

    MonoAudioCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    MonoAudioCheck->SetIsChecked(bCachedMonoAudio);
    MonoAudioCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnMonoAudioChanged);
    AddRow(TEXT("Mono Audio"), MonoAudioCheck);

    VisualizeSoundCuesCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
    VisualizeSoundCuesCheck->SetIsChecked(bCachedVisualizeSoundCues);
    VisualizeSoundCuesCheck->OnCheckStateChanged.AddDynamic(this, &UCodeRescueSettingsWidget::OnVisualizeSoundCuesChanged);
    AddRow(TEXT("Visualize Sound Cues"), VisualizeSoundCuesCheck);

    AudioReadoutText = MakeLabel(WidgetTree, TEXT(""));
    CodeRescueUI::StyleText(AudioReadoutText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary());
    Stack->AddChildToVerticalBox(AudioReadoutText);

    GameplayReadoutText = MakeLabel(WidgetTree, TEXT(""));
    CodeRescueUI::StyleText(GameplayReadoutText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary());
    Stack->AddChildToVerticalBox(GameplayReadoutText);

    AccessibilityReadoutText = MakeLabel(WidgetTree, TEXT(""));
    CodeRescueUI::StyleText(AccessibilityReadoutText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary());
    Stack->AddChildToVerticalBox(AccessibilityReadoutText);

    ResetAccessibilityButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StyleSecondaryButton(ResetAccessibilityButton);
    ResetAccessibilityButton->OnClicked.AddDynamic(this, &UCodeRescueSettingsWidget::OnResetAccessibilityClicked);
    {
        UTextBlock* L = MakeLabel(WidgetTree, TEXT("Reset Accessibility Defaults"));
        ResetAccessibilityButton->AddChild(L);
    }
    Stack->AddChildToVerticalBox(ResetAccessibilityButton);

    ControlProfileText = MakeLabel(WidgetTree, TEXT("Control Profile: Default | Config axes + audited direct bindings"));
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        ControlProfileText->SetText(FText::FromString(GI->GetControlProfileSummary()));
    }
    Stack->AddChildToVerticalBox(ControlProfileText);

    ExportControlsButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    ExportControlsButton->OnClicked.AddDynamic(this, &UCodeRescueSettingsWidget::OnExportControlsClicked);
    {
        UTextBlock* L = MakeLabel(WidgetTree, TEXT("Export Control Profile"));
        ExportControlsButton->AddChild(L);
    }
    Stack->AddChildToVerticalBox(ExportControlsButton);

    FeedbackText = MakeLabel(WidgetTree, TEXT(""));
    Stack->AddChildToVerticalBox(FeedbackText);

    ApplyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StylePrimaryButton(ApplyButton);
    ApplyButton->OnClicked.AddDynamic(this, &UCodeRescueSettingsWidget::OnApplyClicked);
    {
        UTextBlock* L = MakeLabel(WidgetTree, TEXT("Apply"));
        ApplyButton->AddChild(L);
    }
    Stack->AddChildToVerticalBox(ApplyButton);

    CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    CodeRescueUI::StyleSecondaryButton(CloseButton);
    CloseButton->OnClicked.AddDynamic(this, &UCodeRescueSettingsWidget::OnCloseClicked);
    {
        UTextBlock* L = MakeLabel(WidgetTree, TEXT("Close"));
        CloseButton->AddChild(L);
    }
    Stack->AddChildToVerticalBox(CloseButton);

    RefreshColorblindButtonLabel();
    RefreshReadouts();
    ACodeRescueCharacter::SetUIOpen(true);
}

void UCodeRescueSettingsWidget::NativeDestruct()
{
    ACodeRescueCharacter::SetUIOpen(false);
    Super::NativeDestruct();
}

void UCodeRescueSettingsWidget::OnMasterVolumeChanged(float Value)         { CachedMaster = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnSfxVolumeChanged(float Value)            { CachedSfx = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnMusicVolumeChanged(float Value)          { CachedMusic = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnMouseSensitivityChanged(float Value)     { CachedSensitivity = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnFovChanged(float Value)                  { CachedFov = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnSubtitleScaleChanged(float Value)        { CachedSubtitleScale = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnUITextScaleChanged(float Value)          { CachedUITextScale = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnAimAssistChanged(float Value)            { CachedAimAssistScale = Value; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnFullscreenChanged(bool bChecked)         { bCachedFullscreen = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnVsyncChanged(bool bChecked)              { bCachedVsync = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnSubtitlesChanged(bool bChecked)          { bCachedSubtitles = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnHighContrastChanged(bool bChecked)       { bCachedHighContrast = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnColorblindModeClicked()
{
    CachedColorblindMode = NextColorblindMode(CachedColorblindMode);
    RefreshColorblindButtonLabel();
    RefreshReadouts();
}
void UCodeRescueSettingsWidget::OnReducedMotionChanged(bool bChecked)      { bCachedReducedMotion = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnSimplifiedHintsChanged(bool bChecked)    { bCachedSimplifiedHints = bChecked; RefreshReadouts(); }
void UCodeRescueSettingsWidget::OnMonoAudioChanged(bool bChecked)
{
    bCachedMonoAudio = bChecked;
    if (bCachedMonoAudio)
    {
        bCachedVisualizeSoundCues = true;
        if (VisualizeSoundCuesCheck)
        {
            VisualizeSoundCuesCheck->SetIsChecked(true);
        }
    }
    RefreshReadouts();
}
void UCodeRescueSettingsWidget::OnVisualizeSoundCuesChanged(bool bChecked)
{
    bCachedVisualizeSoundCues = bChecked;
    if (bCachedMonoAudio)
    {
        bCachedVisualizeSoundCues = true;
        if (VisualizeSoundCuesCheck)
        {
            VisualizeSoundCuesCheck->SetIsChecked(true);
        }
    }
    RefreshReadouts();
}

void UCodeRescueSettingsWidget::OnResetAccessibilityClicked()
{
    bCachedSubtitles = true;
    CachedSubtitleScale = 1.0f;
    CachedUITextScale = 1.0f;
    bCachedHighContrast = false;
    CachedColorblindMode = EColorblindMode::None;
    bCachedReducedMotion = false;
    bCachedSimplifiedHints = false;
    bCachedMonoAudio = false;
    bCachedVisualizeSoundCues = true;
    CachedAimAssistScale = 1.0f;
    SyncCachedControls();
    RefreshReadouts();
    if (FeedbackText)
    {
        FeedbackText->SetText(FText::FromString(TEXT("Accessibility defaults queued. Apply to save.")));
    }
}

void UCodeRescueSettingsWidget::RefreshReadouts()
{
    if (AudioReadoutText)
    {
        AudioReadoutText->SetText(FText::FromString(FString::Printf(
            TEXT("Audio mix | Master %d%% | SFX %d%% | Music %d%% | Mono %s"),
            FMath::RoundToInt(FMath::Clamp(CachedMaster, 0.0f, 1.0f) * 100.0f),
            FMath::RoundToInt(FMath::Clamp(CachedSfx, 0.0f, 1.0f) * 100.0f),
            FMath::RoundToInt(FMath::Clamp(CachedMusic, 0.0f, 1.0f) * 100.0f),
            bCachedMonoAudio ? TEXT("on") : TEXT("off"))));
    }
    if (GameplayReadoutText)
    {
        GameplayReadoutText->SetText(FText::FromString(FString::Printf(
            TEXT("Display and aim | FOV %.0f deg | Mouse %.2fx | Aim assist %.2fx | %s | VSync %s"),
            FMath::Clamp(CachedFov, 60.0f, 110.0f),
            FMath::Clamp(CachedSensitivity, 0.25f, 3.0f),
            FMath::Clamp(CachedAimAssistScale, 0.0f, 2.0f),
            bCachedFullscreen ? TEXT("Fullscreen") : TEXT("Windowed"),
            bCachedVsync ? TEXT("on") : TEXT("off"))));
    }
    if (AccessibilityReadoutText)
    {
        AccessibilityReadoutText->SetText(FText::FromString(FString::Printf(
            TEXT("Accessibility | Subtitles %s %.2fx | UI %.2fx | Contrast %s | Motion %s | Mono %s | Sound cues %s | Hints %s"),
            bCachedSubtitles ? TEXT("on") : TEXT("off"),
            FMath::Clamp(CachedSubtitleScale, 0.75f, 1.75f),
            FMath::Clamp(CachedUITextScale, 0.80f, 1.75f),
            *FString::Printf(TEXT("%s / %s"),
                bCachedHighContrast ? TEXT("high") : TEXT("standard"),
                *ColorblindModeName(CachedColorblindMode)),
            bCachedReducedMotion ? TEXT("reduced") : TEXT("standard"),
            bCachedMonoAudio ? TEXT("on") : TEXT("off"),
            bCachedVisualizeSoundCues ? TEXT("visible") : TEXT("hidden"),
            bCachedSimplifiedHints ? TEXT("simplified") : TEXT("full"))));
    }
}

void UCodeRescueSettingsWidget::RefreshColorblindButtonLabel()
{
    if (ColorblindModeButtonText)
    {
        ColorblindModeButtonText->SetText(FText::FromString(FString::Printf(
            TEXT("Color Vision: %s (cycle)"),
            *ColorblindModeName(CachedColorblindMode))));
    }
}

void UCodeRescueSettingsWidget::SyncCachedControls()
{
    if (MasterSlider) MasterSlider->SetValue(FMath::Clamp(CachedMaster, 0.0f, 1.0f));
    if (SfxSlider) SfxSlider->SetValue(FMath::Clamp(CachedSfx, 0.0f, 1.0f));
    if (MusicSlider) MusicSlider->SetValue(FMath::Clamp(CachedMusic, 0.0f, 1.0f));
    if (SensitivitySlider) SensitivitySlider->SetValue(FMath::Clamp(CachedSensitivity, 0.25f, 3.0f));
    if (FovSlider) FovSlider->SetValue(FMath::Clamp(CachedFov, 60.0f, 110.0f));
    if (SubtitleScaleSlider) SubtitleScaleSlider->SetValue(FMath::Clamp(CachedSubtitleScale, 0.75f, 1.75f));
    if (UITextScaleSlider) UITextScaleSlider->SetValue(FMath::Clamp(CachedUITextScale, 0.80f, 1.75f));
    if (AimAssistSlider) AimAssistSlider->SetValue(FMath::Clamp(CachedAimAssistScale, 0.0f, 2.0f));
    if (FullscreenCheck) FullscreenCheck->SetIsChecked(bCachedFullscreen);
    if (VsyncCheck) VsyncCheck->SetIsChecked(bCachedVsync);
    if (SubtitlesCheck) SubtitlesCheck->SetIsChecked(bCachedSubtitles);
    if (HighContrastCheck) HighContrastCheck->SetIsChecked(bCachedHighContrast);
    RefreshColorblindButtonLabel();
    if (ReducedMotionCheck) ReducedMotionCheck->SetIsChecked(bCachedReducedMotion);
    if (SimplifiedHintsCheck) SimplifiedHintsCheck->SetIsChecked(bCachedSimplifiedHints);
    if (MonoAudioCheck) MonoAudioCheck->SetIsChecked(bCachedMonoAudio);
    if (VisualizeSoundCuesCheck) VisualizeSoundCuesCheck->SetIsChecked(bCachedVisualizeSoundCues);
}

void UCodeRescueSettingsWidget::OnExportControlsClicked()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        const bool bOk = GI->ExportControlProfileReviewFile();
        if (ControlProfileText)
        {
            ControlProfileText->SetText(FText::FromString(GI->GetControlProfileSummary()));
        }
        if (FeedbackText)
        {
            FeedbackText->SetText(FText::FromString(bOk
                ? TEXT("Control profile exported to Saved/Config/ControlProfiles/runtime_controls_profile.json")
                : TEXT("Control profile export failed.")));
        }
    }
}

void UCodeRescueSettingsWidget::OnApplyClicked()
{
    UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
    if (GUS)
    {
        GUS->SetFullscreenMode(bCachedFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
        GUS->SetVSyncEnabled(bCachedVsync);
        GUS->ApplySettings(false);
    }

    // Mouse sensitivity: rescale the player's existing rate properties so the
    // change takes effect immediately for the polled-key path. (We don't bind
    // raw mouse axes; this is a fair approximation.)
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(PC->GetPawn()))
        {
            Player->DirectKeyboardTurnRate = 135.0f * CachedSensitivity;
            Player->DirectKeyboardLookRate = 75.0f * CachedSensitivity;

            if (UCameraComponent* Cam = Player->FindComponentByClass<UCameraComponent>())
            {
                Cam->SetFieldOfView(CachedFov);
            }
        }
    }

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->bSubtitlesEnabled = bCachedSubtitles;
        GI->SubtitleScale = FMath::Clamp(CachedSubtitleScale, 0.75f, 1.75f);
        GI->UITextScale = FMath::Clamp(CachedUITextScale, 0.80f, 1.75f);
        GI->bHighContrastHUD = bCachedHighContrast;
        GI->ColorblindMode = CachedColorblindMode;
        GI->bReducedMotion = bCachedReducedMotion;
        GI->bSimplifiedInputHints = bCachedSimplifiedHints;
        GI->bMonoAudio = bCachedMonoAudio;
        if (bCachedMonoAudio)
        {
            GI->bVisualizeSoundCues = true;
        }
        else
        {
            GI->bVisualizeSoundCues = bCachedVisualizeSoundCues;
        }
        bCachedVisualizeSoundCues = GI->bVisualizeSoundCues;
        GI->AimAssistScale = FMath::Clamp(CachedAimAssistScale, 0.0f, 2.0f);
        GI->MasterVolume = FMath::Clamp(CachedMaster, 0.0f, 1.0f);
        GI->SfxVolume = FMath::Clamp(CachedSfx, 0.0f, 1.0f);
        GI->MusicVolume = FMath::Clamp(CachedMusic, 0.0f, 1.0f);
        GI->ApplyAudioMixSettings();

        int32 RefreshedWorldColorVolumes = 0;
        int32 RefreshedMonoAudioComponents = 0;
        if (ACodeRescueGameMode* GM = Cast<ACodeRescueGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            RefreshedWorldColorVolumes = GM->RefreshActiveColorVisionPostProcess(GI->ColorblindMode);
            RefreshedMonoAudioComponents = GM->RefreshMonoAudioSpatialization(GI->bMonoAudio);
        }

        // Push accessibility state into the shared UI design system so every themed
        // widget (HUD, menus, terminal) immediately honors high contrast, reduced
        // motion, and text scaling without each screen re-reading settings.
        CodeRescueUI::Theme().bHighContrast = bCachedHighContrast;
        CodeRescueUI::Theme().bReducedMotion = bCachedReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
        UCodeRescueSubtitlesWidget::RefreshAccessibilityState();
        UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState();

        GI->SavePersistentRun();
        if (ControlProfileText)
        {
            ControlProfileText->SetText(FText::FromString(GI->GetControlProfileSummary()));
        }
        if (FeedbackText)
        {
            FeedbackText->SetText(FText::FromString(FString::Printf(
                TEXT("Settings applied. %s %s %s Refreshed %d mono-aware audio component%s. Color vision refreshed %d active world grade%s."),
                *GI->GetAudioMixSummary(),
                *GI->GetUITextScaleSummary(),
                *GI->GetMonoAudioSummary(),
                RefreshedMonoAudioComponents,
                RefreshedMonoAudioComponents == 1 ? TEXT("") : TEXT("s"),
                RefreshedWorldColorVolumes,
                RefreshedWorldColorVolumes == 1 ? TEXT("") : TEXT("s"))));
        }
        RefreshReadouts();
    }
}

void UCodeRescueSettingsWidget::OnCloseClicked()
{
    Close();
}

void UCodeRescueSettingsWidget::Close()
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
