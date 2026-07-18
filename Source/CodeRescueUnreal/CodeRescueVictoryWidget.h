#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueVictoryWidget.generated.h"

class UButton;
class UTextBlock;

/** Modal end-of-mission screen shown when the player completes the win
 *  condition (all coding terminals solved AND all survivors rescued).
 *  Displays the run summary and offers Restart Run / Quit. */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueVictoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UTextBlock* TitleText;
    UPROPERTY() UTextBlock* SummaryText;
    UPROPERTY() UTextBlock* StatsText;
    UPROPERTY() UButton* RestartButton;
    UPROPERTY() UButton* QuitButton;

    UFUNCTION() void OnRestartClicked();
    UFUNCTION() void OnQuitClicked();
};
