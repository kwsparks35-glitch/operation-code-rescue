#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueTutorialWidget.generated.h"

class UTextBlock;
class UButton;
class UHorizontalBox;

/**
 * UCodeRescueTutorialWidget — first-launch onboarding overlay.
 *
 * Shown by ACodeRescueGameMode::BeginPlay if the SaveGame's
 * bHasShownTutorial flag is false. Displays a sequence of pages
 * teaching WASD/look/E/click/J. Player advances pages with Space
 * or [Next] button. Final page closes and writes the flag.
 *
 * Modal: takes input via SetUIOpen(true) so polled gameplay keys
 * don't fire while the tutorial is up.
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueTutorialWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // 2026-07-01 (round 4): packaged-build UMG focus does not deliver key events reliably, so the
    // tutorial is advanced/dismissed by the pawn's polled-key path (the same focus-free mechanism
    // as the launch-language gate). The pawn queries IsShowing() each tick and calls these drivers.
    static bool IsShowing();
    static void DriveAdvance();
    static void DriveDismiss();

protected:
    UFUNCTION() void OnNextClicked();
    UFUNCTION() void OnSkipClicked();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    // Weak handle to the one on-screen tutorial so the pawn can poll-drive it without Slate focus.
    static TWeakObjectPtr<UCodeRescueTutorialWidget> ActiveInstance;

    UPROPERTY() UTextBlock* PageText = nullptr;
    UPROPERTY() UTextBlock* PageNumberText = nullptr;
    UPROPERTY() UTextBlock* PhaseStripText = nullptr;
    UPROPERTY() UTextBlock* LanguageSaveText = nullptr;
    UPROPERTY() UTextBlock* InputHintModeText = nullptr;
    UPROPERTY() UHorizontalBox* ActionCardBox = nullptr;
    UPROPERTY() UTextBlock* SkipButtonLabel = nullptr;
    UPROPERTY() UButton* NextButton = nullptr;
    UPROPERTY() UButton* SkipButton = nullptr;

    int32 CurrentPage = 0;
    bool bSkipConfirmArmed = false;

    void ShowPage(int32 Index);
    void RefreshActionCardsForPage(int32 Index);
    void AddActionCard(const FString& KeyGlyph, const FString& ActionLabel, const FLinearColor& Tint);
    FString BuildLanguageSaveLine();
    FString BuildPhaseStripLine(int32 Index) const;
    void Finish();
};
