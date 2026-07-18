#include "CodeRescueDamageFeedbackWidget.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCodeRescueDamageFeedbackWidget* UCodeRescueDamageFeedbackWidget::ActiveInstance = nullptr;

void UCodeRescueDamageFeedbackWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

TSharedRef<SWidget> UCodeRescueDamageFeedbackWidget::RebuildWidget()
{
    // 2026-07-01 invisible-UMG fix: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueDamageFeedbackWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;

    ActiveInstance = this;

    // 2026-07-11 PAUSE-CLICK ROOT FIX (Kenny's packaged report: "cannot click
    // any pause selection"): this overlay mounts at Z-order 50 — ABOVE the
    // pause menu's 40 — and UImage/UBorder children default to Visible, i.e.
    // HIT-TESTABLE. The invisible full-screen vignette therefore swallowed
    // every pause-menu click (keyboard kept working, which matched the
    // symptom). A pure visual overlay must NEVER participate in hit testing:
    // force the widget AND every child to HitTestInvisible.
    SetVisibility(ESlateVisibility::HitTestInvisible);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DmgRoot"));
    WidgetTree->RootWidget = Root;
    Root->SetVisibility(ESlateVisibility::HitTestInvisible);

    // Full-screen blood vignette via UImage colored red, low alpha.
    Vignette = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    Vignette->SetColorAndOpacity(FLinearColor(0.6f, 0.0f, 0.0f, 0.0f));
    Vignette->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(Vignette);
    VSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    VSlot->SetOffsets(FMargin(0));

    auto MakeChevron = [&](const FVector2D& Anchors, const FVector2D& Offset, const FVector2D& Size)
    {
        UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        B->SetBrushColor(FLinearColor(1.0f, 0.0f, 0.0f, 0.0f));
        B->SetVisibility(ESlateVisibility::HitTestInvisible);
        UCanvasPanelSlot* S = Root->AddChildToCanvas(B);
        S->SetAnchors(FAnchors(Anchors.X, Anchors.Y, Anchors.X, Anchors.Y));
        S->SetAlignment(FVector2D(0.5f, 0.5f));
        S->SetPosition(Offset);
        S->SetSize(Size);
        return B;
    };

    DirN = MakeChevron(FVector2D(0.5f, 0.5f), FVector2D(0.0f, -200.0f), FVector2D(160.0f, 18.0f));
    DirS = MakeChevron(FVector2D(0.5f, 0.5f), FVector2D(0.0f,  200.0f), FVector2D(160.0f, 18.0f));
    DirE = MakeChevron(FVector2D(0.5f, 0.5f), FVector2D( 200.0f, 0.0f), FVector2D(18.0f, 160.0f));
    DirW = MakeChevron(FVector2D(0.5f, 0.5f), FVector2D(-200.0f, 0.0f), FVector2D(18.0f, 160.0f));

    ApplyAccessibilityStateFromSettings();
}

void UCodeRescueDamageFeedbackWidget::NativeDestruct()
{
    if (ActiveInstance == this)
    {
        ActiveInstance = nullptr;
    }

    Super::NativeDestruct();
}

void UCodeRescueDamageFeedbackWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ApplyAccessibilityStateFromSettings();

    UWorld* World = GetWorld();
    if (!World) return;
    const float Now = World->TimeSeconds;

    // 1. Vignette: scale by (1 - HealthFrac). Below 25% HP, kick the alpha
    //    much higher to telegraph "you are about to die".
    if (Vignette)
    {
        if (ACodeRescueCharacter* P = Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)))
        {
            const float HealthFrac = (P->MaxHealth > 0.0f) ? FMath::Clamp(P->Health / P->MaxHealth, 0.0f, 1.0f) : 1.0f;
            float Alpha = (1.0f - HealthFrac) * 0.5f;
            if (HealthFrac < 0.25f)
            {
                // Critical-HP emphasis. Honor the Reduced Motion accessibility
                // setting by holding a steady elevated alpha instead of pulsing.
                if (bDamageFeedbackReducedMotion)
                {
                    Alpha += 0.25f;
                }
                else
                {
                    Alpha += 0.25f * (0.5f + 0.5f * FMath::Sin(Now * 6.0f));
                }
            }
            Vignette->SetColorAndOpacity(GetVignetteColor(Alpha));
        }
    }

    // 2. Directional flashes: fade alpha based on time since last hit.
    const float FlashDuration = GetDirectionalFlashDuration();
    auto FadeChevron = [this, Now, FlashDuration](UBorder* B, float FlashTime)
    {
        if (!B) return;
        const float Elapsed = Now - FlashTime;
        if (Elapsed >= FlashDuration)
        {
            B->SetBrushColor(GetChevronColor(0.0f));
            return;
        }
        const float A = bDamageFeedbackReducedMotion
            ? 0.92f
            : FMath::Clamp(1.0f - (Elapsed / FlashDuration), 0.0f, 1.0f);
        B->SetBrushColor(GetChevronColor(A));
    };
    FadeChevron(DirN, DirNFlashTime);
    FadeChevron(DirE, DirEFlashTime);
    FadeChevron(DirS, DirSFlashTime);
    FadeChevron(DirW, DirWFlashTime);
}

void UCodeRescueDamageFeedbackWidget::NotifyDamageFromDirection(const FVector& WorldDirectionFromAttacker)
{
    UWorld* World = GetWorld();
    if (!World) return;
    const float Now = World->TimeSeconds;

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Pawn) return;

    // Project the attacker direction into the player's local space so
    // "where it hit me" is in player-relative axes (X=forward, Y=right).
    const FVector LocalDir = Pawn->GetActorRotation().UnrotateVector(WorldDirectionFromAttacker.GetSafeNormal());

    // Pick the dominant axis. North = behind us (player's -X) ... no wait,
    // attacker direction is FROM the attacker, so -LocalDir is where the
    // attacker is. We want chevrons on the screen pointing at the attacker:
    // attacker in front => N, behind => S, right => E, left => W.
    const FVector AttackerLocal = -LocalDir;
    const float AbsX = FMath::Abs(AttackerLocal.X);
    const float AbsY = FMath::Abs(AttackerLocal.Y);
    if (AbsX >= AbsY)
    {
        if (AttackerLocal.X >= 0) DirNFlashTime = Now;
        else                       DirSFlashTime = Now;
    }
    else
    {
        if (AttackerLocal.Y >= 0) DirEFlashTime = Now;
        else                       DirWFlashTime = Now;
    }
}

void UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState()
{
    if (ActiveInstance)
    {
        ActiveInstance->ApplyAccessibilityStateFromSettings();
    }
}

bool UCodeRescueDamageFeedbackWidget::IsPointerPassthroughSafe()
{
    return !ActiveInstance
        || ActiveInstance->GetVisibility() == ESlateVisibility::HitTestInvisible;
}

void UCodeRescueDamageFeedbackWidget::ApplyAccessibilityStateFromSettings()
{
    bDamageFeedbackHighContrast = CodeRescueUI::Theme().bHighContrast;
    bDamageFeedbackReducedMotion = CodeRescueUI::Theme().bReducedMotion;

    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        bDamageFeedbackHighContrast = GI->bHighContrastHUD;
        bDamageFeedbackReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
        CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
        CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
    }

    ResizeDirectionalChevron(DirN, true);
    ResizeDirectionalChevron(DirS, true);
    ResizeDirectionalChevron(DirE, false);
    ResizeDirectionalChevron(DirW, false);
}

FLinearColor UCodeRescueDamageFeedbackWidget::GetVignetteColor(float Alpha) const
{
    const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, bDamageFeedbackHighContrast ? 0.88f : 0.72f);
    if (bDamageFeedbackHighContrast && ClampedAlpha > 0.0f)
    {
        return FLinearColor(1.0f, 0.82f, 0.06f, FMath::Clamp(ClampedAlpha + 0.08f, 0.0f, 0.92f));
    }

    return FLinearColor(0.6f, 0.0f, 0.0f, ClampedAlpha);
}

FLinearColor UCodeRescueDamageFeedbackWidget::GetChevronColor(float Alpha) const
{
    if (Alpha <= 0.0f)
    {
        return FLinearColor(1.0f, bDamageFeedbackHighContrast ? 0.92f : 0.0f, 0.05f, 0.0f);
    }

    if (bDamageFeedbackHighContrast)
    {
        return FLinearColor(1.0f, 0.92f, 0.08f, FMath::Clamp(0.35f + Alpha * 0.65f, 0.0f, 1.0f));
    }

    return FLinearColor(1.0f, 0.05f, 0.05f, FMath::Clamp(Alpha, 0.0f, 1.0f));
}

float UCodeRescueDamageFeedbackWidget::GetDirectionalFlashDuration() const
{
    return bDamageFeedbackReducedMotion
        ? ReducedMotionDirectionalFlashDuration
        : DirectionalFlashDuration;
}

void UCodeRescueDamageFeedbackWidget::ResizeDirectionalChevron(UBorder* Chevron, bool bHorizontal) const
{
    if (!Chevron)
    {
        return;
    }

    UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Chevron->Slot);
    if (!Slot)
    {
        return;
    }

    const FVector2D StandardSize = bHorizontal
        ? FVector2D(160.0f, 18.0f)
        : FVector2D(18.0f, 160.0f);
    const FVector2D HighContrastSize = bHorizontal
        ? FVector2D(196.0f, 28.0f)
        : FVector2D(28.0f, 196.0f);
    Slot->SetSize(bDamageFeedbackHighContrast ? HighContrastSize : StandardSize);
}
