#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueTypes.h"
#include "CodeRescueSettingsWidget.generated.h"

class USlider;
class UButton;
class UCheckBox;
class UTextBlock;

/**
 * UCodeRescueSettingsWidget — modal settings menu opened from the pause widget.
 *
 * Sliders for: master volume, SFX volume, music volume, mouse sensitivity, FOV.
 * Toggles for: fullscreen, vsync.
 * Persisted via UGameUserSettings::ApplySettings() so they survive across runs.
 *
 * Audio sliders write to the master sound class via UGameplayStatics — that
 * keeps the widget self-contained and doesn't require defining a Sound Mix
 * snapshot on the Blueprint side. (When/if a sound mix is authored, this
 * widget can be retargeted by changing the SetSoundMixClassOverride call.)
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

protected:
    UFUNCTION() void OnMasterVolumeChanged(float Value);
    UFUNCTION() void OnSfxVolumeChanged(float Value);
    UFUNCTION() void OnMusicVolumeChanged(float Value);
    UFUNCTION() void OnMouseSensitivityChanged(float Value);
    UFUNCTION() void OnFovChanged(float Value);
    UFUNCTION() void OnSubtitleScaleChanged(float Value);
    UFUNCTION() void OnUITextScaleChanged(float Value);
    UFUNCTION() void OnAimAssistChanged(float Value);
    UFUNCTION() void OnFullscreenChanged(bool bChecked);
    UFUNCTION() void OnVsyncChanged(bool bChecked);
    UFUNCTION() void OnSubtitlesChanged(bool bChecked);
    UFUNCTION() void OnHighContrastChanged(bool bChecked);
    UFUNCTION() void OnColorblindModeClicked();
    UFUNCTION() void OnReducedMotionChanged(bool bChecked);
    UFUNCTION() void OnSimplifiedHintsChanged(bool bChecked);
    UFUNCTION() void OnMonoAudioChanged(bool bChecked);
    UFUNCTION() void OnVisualizeSoundCuesChanged(bool bChecked);
    UFUNCTION() void OnResetAccessibilityClicked();
    UFUNCTION() void OnExportControlsClicked();
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCloseClicked();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() USlider* MasterSlider = nullptr;
    UPROPERTY() USlider* SfxSlider = nullptr;
    UPROPERTY() USlider* MusicSlider = nullptr;
    UPROPERTY() USlider* SensitivitySlider = nullptr;
    UPROPERTY() USlider* FovSlider = nullptr;
    UPROPERTY() USlider* SubtitleScaleSlider = nullptr;
    UPROPERTY() USlider* UITextScaleSlider = nullptr;
    UPROPERTY() USlider* AimAssistSlider = nullptr;
    UPROPERTY() UCheckBox* FullscreenCheck = nullptr;
    UPROPERTY() UCheckBox* VsyncCheck = nullptr;
    UPROPERTY() UCheckBox* SubtitlesCheck = nullptr;
    UPROPERTY() UCheckBox* HighContrastCheck = nullptr;
    UPROPERTY() UButton* ColorblindModeButton = nullptr;
    UPROPERTY() UTextBlock* ColorblindModeButtonText = nullptr;
    UPROPERTY() UCheckBox* ReducedMotionCheck = nullptr;
    UPROPERTY() UCheckBox* SimplifiedHintsCheck = nullptr;
    UPROPERTY() UCheckBox* MonoAudioCheck = nullptr;
    UPROPERTY() UCheckBox* VisualizeSoundCuesCheck = nullptr;
    UPROPERTY() UTextBlock* AudioReadoutText = nullptr;
    UPROPERTY() UTextBlock* GameplayReadoutText = nullptr;
    UPROPERTY() UTextBlock* AccessibilityReadoutText = nullptr;
    UPROPERTY() UTextBlock* ControlProfileText = nullptr;
    UPROPERTY() UTextBlock* FeedbackText = nullptr;
    UPROPERTY() UButton* ResetAccessibilityButton = nullptr;
    UPROPERTY() UButton* ExportControlsButton = nullptr;
    UPROPERTY() UButton* ApplyButton = nullptr;
    UPROPERTY() UButton* CloseButton = nullptr;
    UPROPERTY() UTextBlock* TitleText = nullptr;

    /** Cached current values, applied only when the player presses Apply. */
    float CachedMaster = 1.0f;
    float CachedSfx = 1.0f;
    float CachedMusic = 1.0f;
    float CachedSensitivity = 1.0f;
    float CachedFov = 90.0f;
    float CachedSubtitleScale = 1.0f;
    float CachedUITextScale = 1.0f;
    float CachedAimAssistScale = 1.0f;
    bool bCachedFullscreen = false;
    bool bCachedVsync = true;
    bool bCachedSubtitles = true;
    bool bCachedHighContrast = false;
    EColorblindMode CachedColorblindMode = EColorblindMode::None;
    bool bCachedReducedMotion = false;
    bool bCachedSimplifiedHints = false;
    bool bCachedMonoAudio = false;
    bool bCachedVisualizeSoundCues = true;

    void RefreshReadouts();
    void RefreshColorblindButtonLabel();
    void SyncCachedControls();
    void Close();
};
