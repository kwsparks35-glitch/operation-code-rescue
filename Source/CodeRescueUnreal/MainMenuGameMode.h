#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UUserWidget;

/**
 * #38 — Game mode for the splash map. Spawns the main menu widget on
 * BeginPlay and parks the player in UI mode. Has no zombies, no terminals,
 * no tick cost — just the widget.
 *
 * Set as the GameModeOverride on `Maps/MainMenu.umap` (or the default
 * Entry map). DefaultEngine.ini has `GameDefaultMap` and `GlobalDefaultGameMode`
 * settings if you want this to be the launch target.
 */
UCLASS()
class CODERESCUEUNREAL_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu")
    TSubclassOf<UUserWidget> MenuWidgetClass;

protected:
    virtual void BeginPlay() override;
};
