#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueDeathWidget.generated.h"

class UButton;
class UTextBlock;

/** Modal lose-screen shown by ACodeRescueCharacter::ApplyDamage when Health
 *  reaches 0. Pauses the world, captures input, offers Restart from Save /
 *  Restart Fresh / Quit. Modeled on UCodeRescueVictoryWidget — same lifecycle
 *  hooks, same input-lock pattern. */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueDeathWidget : public UUserWidget
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
    UPROPERTY() UTextBlock* SubtitleText;
    UPROPERTY() UTextBlock* SummaryText;
    UPROPERTY() UTextBlock* StatsText;
    UPROPERTY() UTextBlock* ActionStatusText;
    UPROPERTY() UButton* RestartFromSaveButton;
    UPROPERTY() UButton* RestartFreshButton;
    UPROPERTY() UButton* SaveAndQuitButton;
    UPROPERTY() UButton* QuitButton;

    void SetActionStatus(const FString& Status);

    UFUNCTION() void OnRestartFromSaveClicked();
    UFUNCTION() void OnRestartFreshClicked();
    UFUNCTION() void OnSaveAndQuitClicked();
    UFUNCTION() void OnQuitClicked();
};
