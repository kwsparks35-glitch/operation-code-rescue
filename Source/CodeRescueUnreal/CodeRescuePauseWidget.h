#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueTypes.h"
#include "CodeRescuePauseWidget.generated.h"

class UButton;
class UTextBlock;
class UViewport;
class UVerticalBox;
class AStaticMeshActor;
class ACodeRescueCharacter;

/** Modal pause menu opened by P or Esc. Resume / Save Now / Load Last Save /
 *  Restart Run / Quit, plus a difficulty cycle button. While this widget is
 *  active the world is paused (UGameplayStatics::SetGamePaused) and input is
 *  in UI-only mode. */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescuePauseWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnPreviewMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UTextBlock* TitleText;
    UPROPERTY() UTextBlock* DifficultyLabel;
    UPROPERTY() UTextBlock* DifficultyDetailText;
    UPROPERTY() UTextBlock* WeaponNameText;
    UPROPERTY() UTextBlock* WeaponRoleText;
    UPROPERTY() UTextBlock* WeaponAmmoText;
    UPROPERTY() UTextBlock* WeaponStatsText;
    UPROPERTY() UTextBlock* WeaponSelectionText;
    UPROPERTY() UTextBlock* ActionFeedbackText;
    UPROPERTY() UTextBlock* CraftingStatusText;
    UPROPERTY() UViewport* WeaponViewport;
    UPROPERTY() AStaticMeshActor* WeaponPreviewActor;
    UPROPERTY() UButton* PreviousWeaponButton;
    UPROPERTY() UButton* NextWeaponButton;
    UPROPERTY() UButton* EquipWeaponButton;
    UPROPERTY() UButton* ResumeButton;
    UPROPERTY() UButton* SaveButton;
    UPROPERTY() UButton* LoadButton;
    UPROPERTY() UButton* RestartButton;
    UPROPERTY() UButton* DifficultyButton;
    UPROPERTY() UButton* SettingsButton;        // #21 wiring
    UPROPERTY() UButton* TutorialButton;
    UPROPERTY() UButton* SaveSlotsButton;       // #21 wiring
    UPROPERTY() UButton* CraftingButton;        // #61 wiring (improvement pass 2026-05-04 part 3)
    UPROPERTY() UVerticalBox* CraftingPanel;
    UPROPERTY() UButton* CraftFlareButton;
    UPROPERTY() UButton* CraftStimButton;
    UPROPERTY() UButton* CraftGrenadeButton;
    UPROPERTY() UButton* CloseCraftingButton;
    UPROPERTY() UButton* SkillTreeButton;       // #61 wiring (improvement pass 2026-05-04 part 3)
    UPROPERTY() UButton* QuitButton;

    UFUNCTION() void OnResumeClicked();
    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnLoadClicked();
    UFUNCTION() void OnRestartClicked();
    UFUNCTION() void OnDifficultyClicked();
    UFUNCTION() void OnSettingsClicked();       // #21 wiring
    UFUNCTION() void OnTutorialClicked();
    UFUNCTION() void OnSaveSlotsClicked();      // #21 wiring
    UFUNCTION() void OnCraftingClicked();       // #61 wiring
    UFUNCTION() void OnCraftFlareClicked();
    UFUNCTION() void OnCraftStimClicked();
    UFUNCTION() void OnCraftGrenadeClicked();
    UFUNCTION() void OnCloseCraftingClicked();
    UFUNCTION() void OnSkillTreeClicked();      // #61 wiring
    UFUNCTION() void OnQuitClicked();
    UFUNCTION() void OnPreviousWeaponClicked();
    UFUNCTION() void OnNextWeaponClicked();
    UFUNCTION() void OnEquipWeaponClicked();

    void RefreshDifficultyLabel();
    void RefreshWeaponArmory(bool bRespawnPreview);
    void RefreshCraftingPanel();
    void SetActionFeedback(const FString& Message, bool bSuccess = true);
    bool AuditMouseInteractionContract() const;
    bool RoutePointerAtScreenPosition(const FVector2D& ScreenPosition);
    ACodeRescueCharacter* GetPlayerCharacter() const;
    void ClosePause();

    int32 PreviewWeaponIndex = 0;
    float PreviewRotationDegrees = 0.0f;
    FVector PreviewMeshCenterScaled = FVector::ZeroVector;
    float ReviewCaptureElapsed = 0.0f;
    bool bReviewCaptureRequested = false;
    float ArmoryCycleAuditElapsed = 0.0f;
    int32 ArmoryCycleAuditSteps = 0;
    bool bArmoryCycleAuditComplete = false;
    bool bArmoryCycleAuditPassed = false;
    bool bArmoryCycleAuditCaptureRequested = false;
    bool bMouseInteractionAuditPassed = false;
    bool bCraftingInteractionAuditComplete = false;
    bool bCraftingInteractionAuditPassed = false;
    double LastPauseRealTimeSeconds = 0.0;
};
