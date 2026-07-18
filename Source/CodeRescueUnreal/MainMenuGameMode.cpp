#include "MainMenuGameMode.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultPawnClass = nullptr;   // no pawn on the menu map
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    UClass* WidgetClass = MenuWidgetClass.Get();
    if (!WidgetClass)
    {
        WidgetClass = UCodeRescueMainMenuWidget::StaticClass();
    }
    if (UUserWidget* W = CreateWidget<UUserWidget>(PC, WidgetClass))
    {
        W->AddToViewport(2000);
    }

    // #64: kick off menu music. No-op if MenuMusic soft ref is unbound.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->PlayMenuMusic();
    }
}
