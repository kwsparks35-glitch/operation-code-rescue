#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CityFastTravelWidget.generated.h"

class UVerticalBox;
class UButton;
class UBorder;
class UTextBlock;
class UCodeRescueGameInstance;

/**
 * UCityFastTravelWidget — modal menu opened by AHelipadActor::OpenFastTravelMenu.
 *
 * Lists every campaign city whose terminal has been recorded as solved
 * (UCodeRescueGameInstance::SolvedTerminalIds). Selecting a city teleports
 * the player to that city's player-start (FCodeRescueCampaign helpers) with
 * a brief camera fade. Clicking Cancel returns to gameplay.
 *
 * Pattern matches UCodeTerminalWidget for input-mode handling: SetUIOpen(true)
 * on construct, SetUIOpen(false) + return input to game on close.
 */
UCLASS()
class CODERESCUEUNREAL_API UCityFastTravelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category="Fast Travel")
    void ConfigureOpeningHelipadContext(
        int32 InSourceCityIndex,
        const FString& InSourceCityLabel,
        bool bInExtractionReady,
        const FString& InSurvivorName,
        const FLinearColor& InAccentColor);

protected:
    UFUNCTION()
    void OnCloseClicked();

    UFUNCTION()
    void OnDestinationClicked();

    UFUNCTION()
    void OnContinueExtractionClicked();

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY()
    UVerticalBox* DestinationsList = nullptr;

    UPROPERTY()
    UButton* CloseButton = nullptr;

    UPROPERTY()
    UTextBlock* TitleText = nullptr;

    UPROPERTY()
    UTextBlock* DebriefText = nullptr;

    UPROPERTY()
    UTextBlock* SummaryText = nullptr;

    UPROPERTY()
    UBorder* PanelFrame = nullptr;

    UPROPERTY()
    UButton* ContinueButton = nullptr;

    /** Maps button -> destination city index for click dispatch. */
    UPROPERTY()
    TMap<UButton*, int32> ButtonToCityIndex;

    int32 SourceCityIndex = INDEX_NONE;
    int32 ContinueDestinationCityIndex = INDEX_NONE;
    FString SourceCityLabel;
    FString ExtractionSurvivorName;
    FLinearColor ExtractionAccentColor = FLinearColor(0.36f, 1.0f, 0.42f);
    bool bSourceExtractionReady = false;

    FString BuildExtractionDebriefText(const UCodeRescueGameInstance* GI) const;
    void TeleportPlayerToCity(int32 CityIndex);
    void Close();
};
