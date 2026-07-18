#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueObjectiveJournalWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UScrollBox;
class UBorder;

/** Read-only objective overlay toggled by the J key. Lists the major-city
 *  campaign stops and marks each as complete / active / unlocked / locked
 *  from persisted terminal and survivor progress. Does NOT lock input. */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueObjectiveJournalWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;

    /** Re-pull current progress from the GameInstance and rebuild the rows. */
    UFUNCTION(BlueprintCallable)
    void RefreshJournal();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY()
    UVerticalBox* RowBox;

    UPROPERTY()
    UScrollBox* MissionScrollBox;

    UPROPERTY()
    UTextBlock* TitleText;

    UPROPERTY()
    UTextBlock* SummaryText;

    UPROPERTY()
    UTextBlock* LanguageSaveText;

    UPROPERTY()
    UTextBlock* LanguageProfileRecapText;

    /** 2026-07-04 (top-50 item 32): per-concept mastery meter rendered from saved
     *  concept progress — players SEE which concepts are solid and which need reps. */
    UPROPERTY()
    UTextBlock* ConceptMasteryText = nullptr;

    UPROPERTY()
    UTextBlock* LearningDebriefText;

    UPROPERTY()
    UTextBlock* ChallengeReplayText;

    UPROPERTY()
    UTextBlock* FailSafeObjectiveBoardText;

    UPROPERTY()
    UTextBlock* RouteMapText;

    UPROPERTY()
    UTextBlock* InventoryText;

    UPROPERTY()
    UTextBlock* IntelText;

    UPROPERTY()
    UTextBlock* IntelArchiveText;

    UPROPERTY()
    UBorder* PanelBorder;
};
