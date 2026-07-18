#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UCanvasPanelSlot;
class UCodeRescueGameInstance;
class UWorld;

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    // 2026-07-17: paints the full-screen circular scope mask + fine reticle
    // while the player looks through a magnified optic (Kenny's thermal-sight
    // reference video) — everything outside the objective circle goes black.
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    UFUNCTION(BlueprintCallable)
    void RefreshHUD();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    /** 2026-07-01 HUD diet: keep only play-critical readouts on screen; detail lives in J/P menus. */
    bool bMinimalHUD = true;

    UPROPERTY()
    UTextBlock* StatusText;

    UPROPERTY()
    UTextBlock* ObjectiveFocusText;

    UPROPERTY()
    UTextBlock* NavigationStripText;

    UPROPERTY()
    UTextBlock* FieldChecklistText;

    UPROPERTY()
    UTextBlock* ThreatCompassText;

    UPROPERTY()
    UTextBlock* SoundCueText;

    UPROPERTY()
    UTextBlock* ObjectiveToastText = nullptr;

    UPROPERTY()
    UTextBlock* CrosshairText;

    /** 2026-07-17 scope view: translucent HUD panels that would band across
        the scope glass — collapsed while sighted in, restored after. */
    UPROPERTY() class UBorder* TopVignetteBorder = nullptr;
    UPROPERTY() class UBorder* BottomVignetteBorder = nullptr;
    UPROPERTY() class UBorder* StatusPanelBorder = nullptr;
    UPROPERTY() class UBorder* ObjectivePanelBorder = nullptr;

    UPROPERTY()
    UTextBlock* AimLockText;

    /** Centered, slightly below the crosshair. Empty unless the player is
     *  looking at an interactable within reach. */
    UPROPERTY()
    UTextBlock* InteractionPromptText;

    UPROPERTY()
    UTextBlock* AmmoText;

    UPROPERTY()
    UTextBlock* HealthLabelText;

    UPROPERTY()
    UProgressBar* HealthBar;

    UPROPERTY()
    UProgressBar* StaminaBar;

    UPROPERTY()
    UTextBlock* ReloadStatusText;

    UPROPERTY()
    UTextBlock* WeaponStripText;

    UPROPERTY()
    UTextBlock* HeadshotFeedbackText;

    UPROPERTY()
    UCanvasPanelSlot* HeadshotFeedbackSlot = nullptr;

    UPROPERTY()
    UTextBlock* DamageAlertText;

    /** Minimap mounted top-right. Created in NativeConstruct. (#6) */
    UPROPERTY()
    class UCodeRescueMinimapWidget* Minimap = nullptr;

    /** Autosave indicator (#19 wiring). Shown briefly when GI->LastSaveWallSeconds
     *  was just stamped. */
    UPROPERTY()
    UTextBlock* AutosaveText = nullptr;

    /** #42 — second-line readout for new mechanics: weapon name, throwable
     *  counts, scrap, ResearchPoints, and learning mastery streak. */
    UPROPERTY()
    UTextBlock* SecondLineText = nullptr;

    /** Immediate tactical readout: stamina, reload state, nearest hostile,
     *  low-health/low-ammo alerts, and headshot feedback. */
    UPROPERTY()
    UTextBlock* TacticalReadoutText = nullptr;

    UPROPERTY()
    UTextBlock* SquadStatusText = nullptr;

    FVector LastObservedPlayerLocation = FVector::ZeroVector;
    float IdleGuidanceSeconds = 0.0f;
    bool bHasObservedPlayerLocation = false;
    bool bObjectiveToastStateSeeded = false;
    int32 LastObservedSolvedTerminalCount = 0;
    int32 LastObservedRescuedSurvivorCount = 0;
    int32 LastObservedCodingScore = 0;
    float LastObservedSaveWallSeconds = -99.0f;
    FString ObjectiveToastMessage;
    FLinearColor ObjectiveToastColor = FLinearColor(0.78f, 0.96f, 1.0f, 1.0f);
    float ObjectiveToastStartSeconds = -99.0f;
    float ObjectiveToastDurationSeconds = 0.0f;

    void RefreshHeadshotFeedback(float SinceHeadshot, const UCodeRescueGameInstance* GI);
    void TriggerObjectiveRouteToast(const FString& Message, const FLinearColor& Color, float DurationSeconds);
    void RefreshObjectiveRouteToast(const UCodeRescueGameInstance* GI, UWorld* World);
};
