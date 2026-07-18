#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "CodeRescueSubtitlesWidget.generated.h"

class UTextBlock;

/**
 * #44 — bottom-third subtitle overlay.
 *
 * Single static instance; callers push subtitle lines via
 * UCodeRescueSubtitlesWidget::Push("...", 4.0f). Each line stays visible
 * for its declared Duration, then fades out and is replaced by the next
 * queued entry (if any).
 *
 * Mounted by UCodeRescueHUDWidget::NativeConstruct alongside the HUD
 * itself. Honors GameUserSettings → bSubtitlesEnabled (item 45 setting).
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueSubtitlesWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Static push API. Finds the active subtitle widget (registered in
     *  NativeConstruct) and queues the line. Safe to call from anywhere. */
    static void Push(const FString& Line, float Duration = 4.0f);

    /** Re-read saved subtitle accessibility settings on the active overlay. */
    static void RefreshAccessibilityState();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UTextBlock* LineText = nullptr;

    FSlateFontInfo BaseSubtitleFont;
    bool bHasBaseSubtitleFont = false;

    struct FSubtitleEntry
    {
        FString Line;
        float Remaining = 0.0f;
    };

    TArray<FSubtitleEntry> Queue;

    static TWeakObjectPtr<UCodeRescueSubtitlesWidget> ActiveInstanceWeak;   // 2026-07-04: renamed so Push()'s conventional ActiveInstance local doesn't shadow
    static TArray<FSubtitleEntry> PendingQueue;

    void ApplyAccessibilityStateFromSettings(bool bClearWhenDisabled);
    bool AreSubtitlesEnabledInSettings() const;
};
