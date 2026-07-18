#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeRescueMinimapWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;

/**
 * Minimap widget for Operation Code Rescue.
 *
 * Renders a themed top-down 2D mini-radar of the player's surroundings
 * showing Points-Of-Interest (POIs) — terminals, survivors, launch language
 * markers, pickups, zombies — as colored and size-coded dots with a compact
 * legend and nearest-objective route cue. Dots are recreated from scratch
 * every refresh so we don't have to track pre-allocated widget pointers per
 * actor.
 *
 * Hooked from UCodeRescueHUDWidget — call RefreshMinimap() each frame
 * (it's lightweight; the worst case is ~60 actors visited per frame).
 *
 * Category scheme:
 *   - Player (always centered):   white, largest
 *   - Coding terminal (yellow)
 *   - Survivor (cyan)
 *   - Launch language marker (amber, if present)
 *   - Pickup (green, small)
 *   - Zombie (red)
 *
 * View radius defaults to 40000 units to make the 50x cities legible at
 * a glance — anything beyond ViewRadius is hidden, anything inside is
 * placed proportional to its 2D offset from the player.
 */
UCLASS()
class CODERESCUEUNREAL_API UCodeRescueMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void RefreshMinimap();

    /** World-space radius (UU) of the minimap's coverage. Anything farther
     *  than this from the player is not drawn. Default tuned for the 50x city. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap")
    float ViewRadius = 40000.0f;

    /** Pixel size of the minimap widget footprint (square). Set to 220 to
     *  match the HUD layout. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap")
    float MinimapSizePx = 220.0f;

    /** Pixel size of the inner radar plot after title/legend/readout space. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap")
    float MapAreaSizePx = 128.0f;

    /** Diameter (px) of each POI dot. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap")
    float DotSizePx = 6.0f;

private:
    void BuildWidgetTreeNow();
    bool bWidgetTreeBuilt = false;

    UPROPERTY()
    UCanvasPanel* RootCanvas = nullptr;

    UPROPERTY()
    UBorder* PanelFrame = nullptr;

    UPROPERTY()
    UCanvasPanel* DotCanvas = nullptr;

    UPROPERTY()
    UTextBlock* TitleText = nullptr;

    UPROPERTY()
    UTextBlock* CompassText = nullptr;

    UPROPERTY()
    UTextBlock* SummaryText = nullptr;

    UPROPERTY()
    UTextBlock* RouteCueText = nullptr;

    UPROPERTY()
    UTextBlock* LegendText = nullptr;

    /** Spawn (or re-use) a single colored dot at the given normalized 2D offset.
     *  RelOffset components are in [-1, +1]; (0,0) is center / the player. */
    void DrawDot(const FVector2D& RelOffset, const FLinearColor& Color, float SizePx);

    float RefreshAccumulatorSeconds = 0.0f;

    /** Tracks every dot widget added this frame so they can be cleaned out
     *  on the next refresh. We rebuild from scratch every frame for simplicity. */
    UPROPERTY()
    TArray<UImage*> ActiveDots;
};
