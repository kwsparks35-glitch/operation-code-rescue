// CodeRescueMessageReaderWidget.h
//
// A separate, scrollable reader screen for a single world message. World clutter is replaced by
// compact hover markers (ACodeRescueMessageMarkerActor); when the player reads one, its full
// paragraph opens here in a calm, scrollable panel so the words no longer compete for attention.
//
// Singleton-style: OpenReader() closes any existing reader and shows the new one. Self-contained
// (builds its own widget tree in C++), so it needs no Blueprint asset.
//
// Authored without an on-device UE compile; Mac compile + playtest is the Definition-of-Done gate.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueMessageReaderWidget.generated.h"

class UBorder;
class UButton;
class UScrollBox;
class UTextBlock;
class UVerticalBox;

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueMessageReaderWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    /** Open (or replace) the reader with one message. Handles input mode + cursor. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Messages")
    static void OpenReader(UObject* WorldContext, const FString& MessageId, const FString& Title, const FString& Body);

    /** Close the active reader and restore game input. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Messages")
    static void CloseActiveReader();

    /** True while the reader owns the modal input stack. */
    static bool IsReaderOpen();

    void SetMessage(const FString& InMessageId, const FString& InTitle, const FString& InBody);

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    void Close();
    UFUNCTION() void OnCloseClicked();

    UPROPERTY() UBorder* RootBorder = nullptr;
    UPROPERTY() UVerticalBox* Column = nullptr;
    UPROPERTY() UTextBlock* IdText = nullptr;
    UPROPERTY() UTextBlock* TitleText = nullptr;
    UPROPERTY() UScrollBox* BodyScroll = nullptr;
    UPROPERTY() UTextBlock* BodyText = nullptr;
    UPROPERTY() UTextBlock* HintText = nullptr;
    UPROPERTY() UButton* CloseButton = nullptr;

    FString MessageId;
    FString Title;
    FString Body;

    static TWeakObjectPtr<UCodeRescueMessageReaderWidget> ActiveReader;
};
