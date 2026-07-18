#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueTypes.h"
#include "CodeTerminalWidget.generated.h"

class UTextBlock;
class UMultiLineEditableTextBox;
class UButton;
class UBorder;
class UScrollBox;
class ACodingTerminalActor;

UCLASS()
class CODERESCUEUNREAL_API UCodeTerminalWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UFUNCTION(BlueprintCallable)
    void InitializeTerminal(ACodingTerminalActor* InTerminal);

    /** Canonical complete solution used by deterministic curriculum audits.
     *  The playable editor receives the same six-language implementation. */
    static FString GetCanonicalReferenceSolution(const FString& ChallengeId, ECodingLanguage Language);

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY()
    ACodingTerminalActor* TerminalActor;

    UPROPERTY()
    UTextBlock* TitleText;

    UPROPERTY()
    UTextBlock* TerminalStatusText;

    UPROPERTY()
    UTextBlock* BriefText;

    UPROPERTY()
    UTextBlock* LanguageLockText;

    /** Banner showing whether the external compiler/runtime for the selected
     *  language was found, plus install hint if not. Sourced from
     *  UCodeRunnerLibrary::GetLanguageDependencyMessage. */
    UPROPERTY()
    UTextBlock* DependencyBanner;

    UPROPERTY()
    UTextBlock* LearningStatusText;

    UPROPERTY()
    UTextBlock* ChecklistText;

    UPROPERTY()
    UMultiLineEditableTextBox* CodeBox;

    UPROPERTY()
    UBorder* PanelFrame;

    /** 2026-07-06 first-level completion: the whole terminal column lives in a
     *  ScrollBox so the action buttons can NEVER be pushed off-screen by long
     *  diagnostics/briefs at short window heights (playtest found a stuck state
     *  at 1310x780 where CLOSE TERMINAL was unreachable after a solve). */
    UPROPERTY()
    UScrollBox* TerminalScroll;

    UPROPERTY()
    UBorder* CodeEditorFrame;

    UPROPERTY()
    UBorder* OutputFrame;

    UPROPERTY()
    UTextBlock* DiagnosticsHeaderText;

    UPROPERTY()
    UTextBlock* OutputText;

    UPROPERTY()
    UButton* ValidateButton;

    UPROPERTY()
    UButton* PracticeButton;

    UPROPERTY()
    UButton* ResetButton;

    UPROPERTY()
    UButton* MATLABButton;

    UPROPERTY()
    UButton* CloseButton;

    UFUNCTION()
    void OnValidateClicked();

    UFUNCTION()
    void OnPracticeClicked();

    UFUNCTION()
    void OnResetStarterClicked();

    UFUNCTION()
    void OnMATLABClicked();

    UFUNCTION()
    void OnCloseClicked();

    /** #31 — H-key reveals the next pseudocode hint for the active challenge.
     *  Costs 1 ResearchPoint per hint. */
    UFUNCTION()
    void OnHintClicked();

    UFUNCTION()
    void OnBypassClicked();

    UFUNCTION()
    void OnRewardResearchClicked();

    UFUNCTION()
    void OnRewardFieldKitClicked();

    UFUNCTION()
    void OnRewardCraftingClicked();

    UPROPERTY()
    UButton* HintButton = nullptr;

    UPROPERTY()
    UButton* BypassButton = nullptr;

    UPROPERTY()
    UButton* RewardResearchButton = nullptr;

    UPROPERTY()
    UButton* RewardFieldKitButton = nullptr;

    UPROPERTY()
    UButton* RewardCraftingButton = nullptr;

    UPROPERTY()
    UTextBlock* HintText = nullptr;

    // 2026-07-04 (top-50 item 27): interactive predict-the-output drill. Before
    // writing code, the player commits to a prediction for the challenge's own
    // first visible test — active retrieval beats passive reading.
    UPROPERTY()
    class UHorizontalBox* PredictionDrillRow = nullptr;
    UPROPERTY()
    UTextBlock* PredictionQuestionText = nullptr;
    UPROPERTY()
    UButton* PredictChoiceButtonA = nullptr;
    UPROPERTY()
    UButton* PredictChoiceButtonB = nullptr;
    UPROPERTY()
    UButton* PredictChoiceButtonC = nullptr;
    UPROPERTY()
    UTextBlock* PredictChoiceLabelA = nullptr;
    UPROPERTY()
    UTextBlock* PredictChoiceLabelB = nullptr;
    UPROPERTY()
    UTextBlock* PredictChoiceLabelC = nullptr;

    UFUNCTION()
    void OnPredictChoiceA();
    UFUNCTION()
    void OnPredictChoiceB();
    UFUNCTION()
    void OnPredictChoiceC();

    int32 PredictionCorrectIndex = -1;
    bool bPredictionAnswered = false;
    FString PredictionConcept;
    FString PredictionChallengeId;

    void SetupPredictionDrill();
    void AnswerPrediction(int32 ChoiceIndex);

    int32 HintsRevealed = 0;
    bool bUsedHintThisAttempt = false;
    int32 SessionAttemptCount = 0;
    int32 PracticeRunCount = 0;
    int32 SessionBestScore = 0;
    int32 LastScore = 0;
    int32 ConsecutiveFailureCount = 0;
    FString LastFailedCheck;

    void RefreshText();
    void ResetToStarterCode();
    void SetDiagnosticsState(const FString& Label, const FLinearColor& Color);
    void RunValidation(bool bPracticeOnly);
    void ClaimRewardChoice(const FString& ChoiceId);
    void UpdateRewardChoiceButtons();
};
