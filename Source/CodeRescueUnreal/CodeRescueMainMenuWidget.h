#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueTypes.h"
#include "CodeRescueMainMenuWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * #38 — Main menu shown by `AMainMenuGameMode` on the splash map.
 *
 * Buttons:
 *   - New Game     → wipe save, OpenLevel("Entry")
 *   - Continue     → load save, OpenLevel("Entry")
 *   - Sandbox Mode → OpenLevel("Entry")  (generated packaged play space)
 *   - Settings     → spawn UCodeRescueSettingsWidget
 *   - Credits      → swap to a credits sub-widget (text scroll)
 *   - Quit
 *
 * The widget owns the input mode while it's up — full UI focus, mouse on.
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual bool NativeSupportsKeyboardFocus() const override;

    void SetLaunchLanguageOnly(bool bInLaunchLanguageOnly);

    /** 2026-07-01: focus-free keyboard driving for the launch selector (polled like WASD). */
    static UCodeRescueMainMenuWidget* GetActiveLaunchMenu() { return ActiveLaunchMenu.Get(); }
    void DriveCycleLanguage(int32 Delta) { CycleSelectedLanguage(Delta); }
    void DriveSelectLanguageIndex(int32 Index) { SetSelectedLanguage(LanguageForMenuIndex(Index)); }
    void DriveDeploySelected();
    virtual void NativeDestruct() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu")
    FName CampaignMapName = TEXT("Entry");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu")
    FName SandboxMapName = TEXT("Entry");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu")
    bool bLaunchLanguageOnly = false;

protected:
    UFUNCTION() void OnNewGameClicked();
    UFUNCTION() void OnContinueClicked();
    UFUNCTION() void OnSandboxClicked();
    UFUNCTION() void OnSettingsClicked();
    UFUNCTION() void OnTutorialClicked();
    UFUNCTION() void OnCreditsClicked();
    UFUNCTION() void OnQuitClicked();
    UFUNCTION() void OnJavaLanguageClicked();
    UFUNCTION() void OnCLanguageClicked();
    UFUNCTION() void OnCPlusLanguageClicked();
    UFUNCTION() void OnCppLanguageClicked();
    UFUNCTION() void OnPythonLanguageClicked();
    UFUNCTION() void OnMATLABLanguageClicked();
    UFUNCTION() void OnResumeJavaClicked();
    UFUNCTION() void OnResumeCClicked();
    UFUNCTION() void OnResumeCPlusClicked();
    UFUNCTION() void OnResumeCppClicked();
    UFUNCTION() void OnResumePythonClicked();
    UFUNCTION() void OnResumeMATLABClicked();

private:
    static TWeakObjectPtr<UCodeRescueMainMenuWidget> ActiveLaunchMenu;

    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    void SetSelectedLanguage(ECodingLanguage Language);
    void StartLanguageRun(ECodingLanguage Language);
    void ResumeLanguageRun(ECodingLanguage Language);
    /** Hands input back to the game (GameOnly mode, no cursor, UI-open flag
     *  cleared) before any deploy travel — see 2026-07-06 movement-lock fix. */
    void RestoreGameInputBeforeTravel();
    void RefreshLanguageText();
    void CycleSelectedLanguage(int32 Delta);
    ECodingLanguage LanguageForMenuIndex(int32 Index) const;
    int32 MenuIndexForLanguage(ECodingLanguage Language) const;

    UPROPERTY() UTextBlock* TitleText = nullptr;
    UPROPERTY() UTextBlock* TaglineText = nullptr;
    UPROPERTY() UTextBlock* LanguageText = nullptr;
    UPROPERTY() UTextBlock* FirstSessionRoutePreviewText = nullptr;
    UPROPERTY() UButton* NewGameBtn = nullptr;
    UPROPERTY() UButton* ContinueBtn = nullptr;
    UPROPERTY() UButton* SandboxBtn = nullptr;
    UPROPERTY() UButton* SettingsBtn = nullptr;
    UPROPERTY() UButton* TutorialBtn = nullptr;
    UPROPERTY() UButton* CreditsBtn = nullptr;
    UPROPERTY() UButton* QuitBtn = nullptr;
    UPROPERTY() UButton* JavaLanguageBtn = nullptr;
    UPROPERTY() UButton* CLanguageBtn = nullptr;
    UPROPERTY() UButton* CPlusLanguageBtn = nullptr;
    UPROPERTY() UButton* CppLanguageBtn = nullptr;
    UPROPERTY() UButton* PythonLanguageBtn = nullptr;
    UPROPERTY() UButton* MATLABLanguageBtn = nullptr;
    UPROPERTY() UButton* ResumeJavaBtn = nullptr;
    UPROPERTY() UButton* ResumeCBtn = nullptr;
    UPROPERTY() UButton* ResumeCPlusBtn = nullptr;
    UPROPERTY() UButton* ResumeCppBtn = nullptr;
    UPROPERTY() UButton* ResumePythonBtn = nullptr;
    UPROPERTY() UButton* ResumeMATLABBtn = nullptr;
    UPROPERTY() class UTextBlock* CreditsScroll = nullptr;
};
