#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueSkillTreeWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UVerticalBox;

/** Runtime-authored skill tree panel opened from the pause menu. */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueSkillTreeWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

protected:
    UFUNCTION() void OnCloseClicked();
    UFUNCTION() void OnSkill0Clicked();
    UFUNCTION() void OnSkill1Clicked();
    UFUNCTION() void OnSkill2Clicked();
    UFUNCTION() void OnSkill3Clicked();
    UFUNCTION() void OnSkill4Clicked();
    UFUNCTION() void OnSkill5Clicked();
    UFUNCTION() void OnSkill6Clicked();
    UFUNCTION() void OnSkill7Clicked();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UTextBlock* TitleText = nullptr;
    UPROPERTY() UTextBlock* PointsText = nullptr;
    UPROPERTY() UTextBlock* SummaryText = nullptr;
    UPROPERTY() UTextBlock* FeedbackText = nullptr;
    UPROPERTY() UBorder* PanelFrame = nullptr;
    UPROPERTY() UVerticalBox* NodeList = nullptr;
    UPROPERTY() UButton* CloseButton = nullptr;
    UPROPERTY() TArray<UButton*> SkillButtons;
    UPROPERTY() TArray<UTextBlock*> SkillLabels;
    FLinearColor FeedbackColor = FLinearColor(0.94f, 0.72f, 0.30f, 1.0f);

    void Refresh();
    void TryUnlockNode(int32 NodeIndex);
    void Close();
    void SetFeedback(const FString& Message, const FLinearColor& Color);
    static FString SkillName(int32 NodeIndex);
    static FString SkillDescription(int32 NodeIndex);
};
