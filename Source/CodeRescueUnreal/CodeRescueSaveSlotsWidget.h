#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueSaveSlotsWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UVerticalBox;

/**
 * UCodeRescueSaveSlotsWidget — manage three manual language-run backups.
 *
 * Each row: slot name, backup summary, [Save Backup] [Load Backup] [Delete]
 * buttons.
 *
 * Slots are encoded as save-game files named `OperationCodeRescue_Slot{N}`,
 * where N is 0..2. These slots are now treated as manual backups of the
 * currently selected coding-language run. Successful save/load operations
 * always restore the GameInstance's SaveSlotName to
 * `OperationCodeRescue_Language_<Track>` so the start screen remains the
 * authoritative resume surface for every language.
 *
 * Mounted from the pause widget. Modal: SetUIOpen(true) on enter, false on
 * close. Hooks the autosave-indicator code path (SaveStampNow on the GI)
 * so HUD can show a brief "Saving..." pip.
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueSaveSlotsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

protected:
    UFUNCTION() void OnCloseClicked();
    UFUNCTION() void OnSaveSlot0Clicked();
    UFUNCTION() void OnSaveSlot1Clicked();
    UFUNCTION() void OnSaveSlot2Clicked();
    UFUNCTION() void OnLoadSlot0Clicked();
    UFUNCTION() void OnLoadSlot1Clicked();
    UFUNCTION() void OnLoadSlot2Clicked();
    UFUNCTION() void OnDeleteSlot0Clicked();
    UFUNCTION() void OnDeleteSlot1Clicked();
    UFUNCTION() void OnDeleteSlot2Clicked();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY() UVerticalBox* SlotsList = nullptr;
    UPROPERTY() UButton* CloseButton = nullptr;
    UPROPERTY() UTextBlock* TitleText = nullptr;
    UPROPERTY() UTextBlock* SummaryText = nullptr;
    UPROPERTY() UTextBlock* FeedbackText = nullptr;
    UPROPERTY() UBorder* PanelFrame = nullptr;

    FLinearColor FeedbackColor = FLinearColor(0.94f, 0.72f, 0.30f, 1.0f);

    void Refresh();
    void Close();

    void DoSave(int32 SlotIndex);
    void DoLoad(int32 SlotIndex);
    void DoDelete(int32 SlotIndex);
    void SetFeedback(const FString& Message, const FLinearColor& Color);

    static FString MakeSlotName(int32 SlotIndex);
};
