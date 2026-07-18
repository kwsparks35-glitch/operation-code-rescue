#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueDamageFeedbackWidget.generated.h"

class UImage;
class UBorder;

/**
 * UCodeRescueDamageFeedbackWidget — full-screen damage feedback overlay.
 *
 * Three components:
 *   1. Screen-edge blood vignette — opacity = (1 - HealthFrac). Always visible
 *      at low health, fades out when healed.
 *   2. Four directional hit indicators (N / E / S / W chevrons) — flash red
 *      for 0.6s when the player takes damage from that direction.
 *   3. Low-health desaturation cue (slight red border tint when below 25%).
 *
 * Mounted by ACodeRescueCharacter::BeginPlay alongside the regular HUD.
 * Driven by the character calling NotifyDamageFromDirection(WorldDir) and
 * an internal Tick that polls health.
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueDamageFeedbackWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Called by the character when ApplyDamage fires. Direction is in world
     *  space (where the damage came from, relative to the player). */
    UFUNCTION(BlueprintCallable, Category="Damage Feedback")
    void NotifyDamageFromDirection(const FVector& WorldDirectionFromAttacker);

    /** Called by Settings Apply so the active overlay picks up accessibility
     *  changes immediately, without waiting for another damage event. */
    static void RefreshAccessibilityState();

    /** 2026-07-11 pause-click contract: true when no overlay instance exists
     *  or the live instance is fully hit-test invisible. The pause menu's
     *  mouse audit fails hard if this overlay could swallow pointer events
     *  again (it mounts above the pause surface in Z-order). */
    static bool IsPointerPassthroughSafe();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UImage* Vignette = nullptr;
    UPROPERTY() UBorder* DirN = nullptr;
    UPROPERTY() UBorder* DirE = nullptr;
    UPROPERTY() UBorder* DirS = nullptr;
    UPROPERTY() UBorder* DirW = nullptr;

    void ApplyAccessibilityStateFromSettings();
    FLinearColor GetVignetteColor(float Alpha) const;
    FLinearColor GetChevronColor(float Alpha) const;
    float GetDirectionalFlashDuration() const;
    void ResizeDirectionalChevron(UBorder* Chevron, bool bHorizontal) const;

    float DirNFlashTime = -99.0f;
    float DirEFlashTime = -99.0f;
    float DirSFlashTime = -99.0f;
    float DirWFlashTime = -99.0f;

    bool bDamageFeedbackHighContrast = false;
    bool bDamageFeedbackReducedMotion = false;

    static constexpr float DirectionalFlashDuration = 0.6f;
    static constexpr float ReducedMotionDirectionalFlashDuration = 0.95f;
    static UCodeRescueDamageFeedbackWidget* ActiveInstance;
};
