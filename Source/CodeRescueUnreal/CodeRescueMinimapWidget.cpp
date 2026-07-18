#include "CodeRescueMinimapWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "CodingTerminalActor.h"
#include "SurvivorActor.h"
#include "LanguageStationActor.h"
#include "PickupActor.h"
#include "CodeZombieActor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "CodeRescueBeaconMarkerActor.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    void MirrorMinimapThemeFromSettings(const UCodeRescueGameInstance* GI)
    {
        if (!GI)
        {
            return;
        }

        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    FLinearColor MinimapDotColor(const FLinearColor& Standard, const FLinearColor& HighContrast)
    {
        return CodeRescueUI::Theme().bHighContrast ? HighContrast : Standard;
    }

    FString DirectionFromDelta(const FVector& Delta)
    {
        if (Delta.SizeSquared2D() <= KINDA_SMALL_NUMBER)
        {
            return TEXT("HERE");
        }

        static const TCHAR* Directions[] =
        {
            TEXT("N"), TEXT("NE"), TEXT("E"), TEXT("SE"),
            TEXT("S"), TEXT("SW"), TEXT("W"), TEXT("NW")
        };
        const float Degrees = FMath::Fmod(FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X)) + 360.0f, 360.0f);
        const int32 DirectionIndex = FMath::FloorToInt((Degrees + 22.5f) / 45.0f) % 8;
        return Directions[DirectionIndex];
    }

    FString MakeMinimapSummary(
        const UCodeRescueGameInstance* GI,
        int32 TerminalCount,
        int32 SurvivorCount,
        int32 LanguageMarkerCount,
        int32 ZombieCount,
        float ViewRadius)
    {
        const FString LanguageName = GI ? GI->GetLanguageName() : FString(TEXT("Java"));
        return FString::Printf(
            TEXT("%s | T%d S%d L%d !%d | %.0fm"),
            *LanguageName,
            TerminalCount,
            SurvivorCount,
            LanguageMarkerCount,
            ZombieCount,
            FMath::Max(1.0f, ViewRadius / 100.0f));
    }

    FString MakeRouteCue(const FString& NearestKind, const FVector& NearestDelta)
    {
        if (NearestKind.IsEmpty())
        {
            return TEXT("Nearest: no objective in scan range");
        }

        const FString Direction = DirectionFromDelta(NearestDelta);
        return FString::Printf(
            TEXT("Nearest %s: %.0fm %s"),
            *NearestKind,
            FMath::Max(1.0f, NearestDelta.Size2D() / 100.0f),
            *Direction);
    }
}

void UCodeRescueMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueMinimapWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueMinimapWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;

    MirrorMinimapThemeFromSettings(GetGameInstance<UCodeRescueGameInstance>());

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapRootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MinimapThemedPanel"));
    CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Sunken(), FMargin(6.0f));
    UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelFrame);
    PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    PanelSlot->SetAlignment(FVector2D::ZeroVector);
    PanelSlot->SetPosition(FVector2D::ZeroVector);
    PanelSlot->SetSize(FVector2D(MinimapSizePx, MinimapSizePx));
    PanelSlot->SetZOrder(0);

    DotCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapDotCanvas"));
    UCanvasPanelSlot* DotLayerSlot = RootCanvas->AddChildToCanvas(DotCanvas);
    DotLayerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    DotLayerSlot->SetAlignment(FVector2D::ZeroVector);
    DotLayerSlot->SetPosition(FVector2D((MinimapSizePx - MapAreaSizePx) * 0.5f, 52.0f));
    DotLayerSlot->SetSize(FVector2D(MapAreaSizePx, MapAreaSizePx));
    DotLayerSlot->SetZOrder(1);

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapTitle"));
    TitleText->SetText(FText::FromString(TEXT("NAV MAP")));
    TitleText->SetAutoWrapText(false);
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::AccentAmber());
    UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleText);
    TitleSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    TitleSlot->SetAlignment(FVector2D::ZeroVector);
    TitleSlot->SetPosition(FVector2D(8.0f, 5.0f));
    TitleSlot->SetSize(FVector2D(122.0f, 24.0f));
    TitleSlot->SetZOrder(2);

    // Compass label in the top-center of the minimap so the player knows
    // which way is "up" on the map (always world-north, not facing-up).
    CompassText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapCompass"));
    CompassText->SetText(FText::FromString(TEXT("N")));
    CompassText->SetJustification(ETextJustify::Right);
    CodeRescueUI::StyleText(CompassText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextPrimary());
    UCanvasPanelSlot* CompassSlot = RootCanvas->AddChildToCanvas(CompassText);
    CompassSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
    CompassSlot->SetAlignment(FVector2D(1.0f, 0.0f));
    CompassSlot->SetPosition(FVector2D(-8.0f, 5.0f));
    CompassSlot->SetSize(FVector2D(42.0f, 24.0f));
    CompassSlot->SetZOrder(2);

    SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapSummary"));
    SummaryText->SetText(FText::FromString(TEXT("Route scan initializing")));
    SummaryText->SetAutoWrapText(false);
    SummaryText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary());
    UCanvasPanelSlot* SummarySlot = RootCanvas->AddChildToCanvas(SummaryText);
    SummarySlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    SummarySlot->SetAlignment(FVector2D::ZeroVector);
    SummarySlot->SetPosition(FVector2D(8.0f, 28.0f));
    SummarySlot->SetSize(FVector2D(204.0f, 22.0f));
    SummarySlot->SetZOrder(2);

    RouteCueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapRouteCue"));
    RouteCueText->SetText(FText::FromString(TEXT("Nearest: scanning")));
    RouteCueText->SetAutoWrapText(false);
    RouteCueText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(RouteCueText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TerminalGreenBright());
    UCanvasPanelSlot* RouteSlot = RootCanvas->AddChildToCanvas(RouteCueText);
    RouteSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    RouteSlot->SetAlignment(FVector2D::ZeroVector);
    RouteSlot->SetPosition(FVector2D(8.0f, 180.0f));
    RouteSlot->SetSize(FVector2D(204.0f, 20.0f));
    RouteSlot->SetZOrder(2);

    LegendText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapLegend"));
    LegendText->SetText(FText::FromString(TEXT("P YOU  T CODE  S RESCUE  L LANG  ! THREAT")));
    LegendText->SetAutoWrapText(true);
    LegendText->SetJustification(ETextJustify::Center);
    CodeRescueUI::StyleText(LegendText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextMuted());
    UCanvasPanelSlot* LegendSlot = RootCanvas->AddChildToCanvas(LegendText);
    LegendSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
    LegendSlot->SetAlignment(FVector2D::ZeroVector);
    LegendSlot->SetPosition(FVector2D(8.0f, 200.0f));
    LegendSlot->SetSize(FVector2D(204.0f, 20.0f));
    LegendSlot->SetZOrder(2);

    RefreshMinimap();
}

void UCodeRescueMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const float RefreshInterval = (GI && GI->bReducedMotion) ? 0.25f : 0.10f;
    RefreshAccumulatorSeconds += InDeltaTime;
    if (RefreshAccumulatorSeconds >= RefreshInterval)
    {
        RefreshAccumulatorSeconds = 0.0f;
        RefreshMinimap();
    }
}

void UCodeRescueMinimapWidget::DrawDot(const FVector2D& RelOffset, const FLinearColor& Color, float SizePx)
{
    if (!DotCanvas) return;

    // Map normalized [-1, +1] -> pixel coordinates centered on the canvas.
    const float ClampedDotSize = FMath::Max(3.0f, SizePx);
    const float HalfSize = MapAreaSizePx * 0.5f;
    const FVector2D PixelPos(
        HalfSize + RelOffset.X * HalfSize - ClampedDotSize * 0.5f,
        HalfSize + RelOffset.Y * HalfSize - ClampedDotSize * 0.5f);

    UImage* Dot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    Dot->SetColorAndOpacity(Color);
    UCanvasPanelSlot* DotSlot = DotCanvas->AddChildToCanvas(Dot);
    DotSlot->SetAnchors(FAnchors(0, 0, 0, 0));
    DotSlot->SetAlignment(FVector2D(0, 0));
    DotSlot->SetPosition(PixelPos);
    DotSlot->SetSize(FVector2D(ClampedDotSize, ClampedDotSize));

    ActiveDots.Add(Dot);
}

void UCodeRescueMinimapWidget::RefreshMinimap()
{
    UWorld* World = GetWorld();
    if (!World || !RootCanvas || !DotCanvas)
    {
        return;
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    MirrorMinimapThemeFromSettings(GI);
    CodeRescueUI::StylePanel(PanelFrame, CodeRescueUI::Surface::Sunken(), FMargin(6.0f));
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::AccentAmber());
    CodeRescueUI::StyleText(CompassText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextPrimary());
    CodeRescueUI::StyleText(SummaryText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary());
    CodeRescueUI::StyleText(RouteCueText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TerminalGreenBright());
    CodeRescueUI::StyleText(LegendText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextMuted());

    // 1. Tear down dots from the previous frame.
    for (UImage* Dot : ActiveDots)
    {
        if (Dot)
        {
            DotCanvas->RemoveChild(Dot);
        }
    }
    ActiveDots.Reset();

    // 2. Find the player.
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        if (SummaryText)
        {
            SummaryText->SetText(FText::FromString(TEXT("Route scan offline")));
        }
        if (RouteCueText)
        {
            RouteCueText->SetText(FText::FromString(TEXT("Nearest: awaiting player signal")));
        }
        return;
    }
    const FVector PlayerLoc = PlayerPawn->GetActorLocation();

    int32 TerminalCount = 0;
    int32 SurvivorCount = 0;
    int32 LanguageMarkerCount = 0;
    int32 PickupCount = 0;
    int32 ZombieCount = 0;
    float NearestObjectiveDistSq = TNumericLimits<float>::Max();
    FString NearestObjectiveKind;
    FVector NearestObjectiveDelta = FVector::ZeroVector;

    // 3. Always draw the player at the center.
    DrawDot(
        FVector2D::ZeroVector,
        MinimapDotColor(FLinearColor::White, FLinearColor::White),
        DotSizePx + 4.0f);

    // 4. Iterate POIs by class and draw dots inside ViewRadius.
    auto AddPOI = [
        this,
        &PlayerLoc,
        &NearestObjectiveDistSq,
        &NearestObjectiveKind,
        &NearestObjectiveDelta](
            AActor* Actor,
            const FLinearColor& Color,
            float SizePx,
            const TCHAR* ObjectiveKind,
            bool bRouteObjective,
            int32& VisibleCount)
    {
        if (!Actor) return;
        const FVector Delta = Actor->GetActorLocation() - PlayerLoc;
        const float DistSq = Delta.SizeSquared2D();
        if (DistSq > ViewRadius * ViewRadius) return;
        ++VisibleCount;

        // Project onto the horizontal plane: x = east, y = north (UE convention
        // is +X = forward in world; for a north-up minimap we flip Y).
        FVector2D Rel(Delta.X / ViewRadius, -Delta.Y / ViewRadius);
        DrawDot(Rel, Color, SizePx);

        if (bRouteObjective && DistSq < NearestObjectiveDistSq)
        {
            NearestObjectiveDistSq = DistSq;
            NearestObjectiveKind = ObjectiveKind;
            NearestObjectiveDelta = Delta;
        }
    };

    for (TActorIterator<ACodingTerminalActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.96f, 0.82f, 0.18f, 1.0f), FLinearColor(1.0f, 1.0f, 0.10f, 1.0f)),
            DotSizePx + 1.5f,
            TEXT("CODE"),
            true,
            TerminalCount);
    }
    for (TActorIterator<ASurvivorActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.22f, 0.92f, 0.95f, 1.0f), FLinearColor(0.32f, 1.0f, 1.0f, 1.0f)),
            DotSizePx + 2.0f,
            TEXT("RESCUE"),
            true,
            SurvivorCount);
    }
    for (TActorIterator<ALanguageStationActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.96f, 0.58f, 0.20f, 1.0f), FLinearColor(1.0f, 0.72f, 0.15f, 1.0f)),
            DotSizePx + 2.5f,
            TEXT("LANG"),
            true,
            LanguageMarkerCount);
    }
    // 2026-07-04 (top-50 item 46): beacon markers (the beaming-symbol POIs) appear
    // on the minimap in their own violet family so players can navigate to
    // readable messages/points of interest from the map, not just by line of sight.
    int32 BeaconCount = 0;
    for (TActorIterator<ACodeRescueBeaconMarkerActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.72f, 0.40f, 0.98f, 1.0f), FLinearColor(0.85f, 0.55f, 1.0f, 1.0f)),
            FMath::Max(4.0f, DotSizePx),
            TEXT("BEACON"),
            false,
            BeaconCount);
    }
    for (TActorIterator<APickupActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.36f, 0.88f, 0.34f, 1.0f), FLinearColor(0.45f, 1.0f, 0.38f, 1.0f)),
            FMath::Max(4.0f, DotSizePx - 0.5f),
            TEXT("AID"),
            false,
            PickupCount);
    }
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        AddPOI(
            *It,
            MinimapDotColor(FLinearColor(0.95f, 0.22f, 0.18f, 1.0f), FLinearColor(1.0f, 0.20f, 0.12f, 1.0f)),
            DotSizePx + 0.5f,
            TEXT("THREAT"),
            false,
            ZombieCount);
    }

    if (SummaryText)
    {
        SummaryText->SetText(FText::FromString(MakeMinimapSummary(
            GI,
            TerminalCount,
            SurvivorCount,
            LanguageMarkerCount,
            ZombieCount,
            ViewRadius)));
    }
    if (RouteCueText)
    {
        RouteCueText->SetText(FText::FromString(MakeRouteCue(NearestObjectiveKind, NearestObjectiveDelta)));
    }
}
