#include "CodeRescueSubtitlesWidget.h"
#include "CodeRescueGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

TWeakObjectPtr<UCodeRescueSubtitlesWidget> UCodeRescueSubtitlesWidget::ActiveInstanceWeak = nullptr;
TArray<UCodeRescueSubtitlesWidget::FSubtitleEntry> UCodeRescueSubtitlesWidget::PendingQueue;

void UCodeRescueSubtitlesWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
}

void UCodeRescueSubtitlesWidget::NativeDestruct()
{
    // 2026-07-01 crash fix: clear the shared instance so static Push() never dereferences a
    // widget from a torn-down world (SIGSEGV during city spawn after level travel).
    if (ActiveInstanceWeak.Get() == this)
    {
        ActiveInstanceWeak = nullptr;
    }
    Super::NativeDestruct();
}

TSharedRef<SWidget> UCodeRescueSubtitlesWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeRescueSubtitlesWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    ActiveInstanceWeak = this;

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SubsRoot"));
    WidgetTree->RootWidget = Root;

    LineText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubsLine"));
    LineText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));
    LineText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
    LineText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    LineText->SetText(FText::FromString(TEXT("")));
    LineText->SetJustification(ETextJustify::Center);
    BaseSubtitleFont = LineText->GetFont();
    BaseSubtitleFont.Size = FMath::Max(22, BaseSubtitleFont.Size);
    bHasBaseSubtitleFont = true;
    ApplyAccessibilityStateFromSettings(true);
    if (PendingQueue.Num() > 0)
    {
        if (AreSubtitlesEnabledInSettings())
        {
            Queue.Append(PendingQueue);
        }
        PendingQueue.Reset();
    }

    UCanvasPanelSlot* LSlot = Root->AddChildToCanvas(LineText);
    LSlot->SetAnchors(FAnchors(0.5f, 0.78f, 0.5f, 0.78f));
    LSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    LSlot->SetSize(FVector2D(1200.0f, 80.0f));
}

void UCodeRescueSubtitlesWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ApplyAccessibilityStateFromSettings(true);
    if (Queue.Num() == 0)
    {
        if (LineText) LineText->SetText(FText::FromString(TEXT("")));
        return;
    }
    Queue[0].Remaining -= InDeltaTime;
    if (Queue[0].Remaining <= 0.0f)
    {
        Queue.RemoveAt(0);
        if (LineText) LineText->SetText(FText::FromString(Queue.Num() > 0 ? Queue[0].Line : FString()));
    }
    else if (LineText)
    {
        LineText->SetText(FText::FromString(Queue[0].Line));
    }
}

void UCodeRescueSubtitlesWidget::Push(const FString& Line, float Duration)
{
    const float SafeDuration = FMath::Max(1.0f, Duration);
    // 2026-07-04: local raw pointer deliberately named ActiveInstance — matches the
    // project-wide static-instance push convention (and its slice verifier).
    UCodeRescueSubtitlesWidget* ActiveInstance = UCodeRescueSubtitlesWidget::ActiveInstanceWeak.Get();
    if (!ActiveInstance)
    {
        PendingQueue.Add({ Line, SafeDuration });
        constexpr int32 MaxPendingSubtitleLines = 8;
        if (PendingQueue.Num() > MaxPendingSubtitleLines)
        {
            PendingQueue.RemoveAt(0, PendingQueue.Num() - MaxPendingSubtitleLines);
        }
        return;
    }
    ActiveInstance->ApplyAccessibilityStateFromSettings(false);
    // Honor the accessibility toggle: when subtitles are off, no-op silently.
    if (UWorld* W = ActiveInstance->GetWorld())
    {
        if (UCodeRescueGameInstance* GI = W->GetGameInstance<UCodeRescueGameInstance>())
        {
            if (!GI->bSubtitlesEnabled) return;
        }
    }
    ActiveInstance->Queue.Add({ Line, SafeDuration });
}

void UCodeRescueSubtitlesWidget::RefreshAccessibilityState()
{
    if (UCodeRescueSubtitlesWidget* Inst = ActiveInstanceWeak.Get())
    {
        Inst->ApplyAccessibilityStateFromSettings(true);
    }
}

void UCodeRescueSubtitlesWidget::ApplyAccessibilityStateFromSettings(bool bClearWhenDisabled)
{
    if (!LineText)
    {
        return;
    }

    float SubtitleScale = 1.0f;
    bool bSubtitlesEnabled = true;
    bool bHighContrast = false;
    if (UWorld* W = GetWorld())
    {
        if (UCodeRescueGameInstance* GI = W->GetGameInstance<UCodeRescueGameInstance>())
        {
            SubtitleScale = FMath::Clamp(GI->SubtitleScale, 0.75f, 1.75f);
            bSubtitlesEnabled = GI->bSubtitlesEnabled;
            bHighContrast = GI->bHighContrastHUD;
        }
    }

    if (!bHasBaseSubtitleFont)
    {
        BaseSubtitleFont = LineText->GetFont();
        BaseSubtitleFont.Size = FMath::Max(22, BaseSubtitleFont.Size);
        bHasBaseSubtitleFont = true;
    }

    FSlateFontInfo SubtitleFont = BaseSubtitleFont;
    SubtitleFont.Size = FMath::RoundToInt(BaseSubtitleFont.Size * SubtitleScale);
    LineText->SetFont(SubtitleFont);
    LineText->SetColorAndOpacity(FSlateColor(bHighContrast
        ? FLinearColor(1.0f, 1.0f, 0.72f, 1.0f)
        : FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

    if (!bSubtitlesEnabled && bClearWhenDisabled)
    {
        Queue.Reset();
        LineText->SetText(FText::FromString(TEXT("")));
    }
}

bool UCodeRescueSubtitlesWidget::AreSubtitlesEnabledInSettings() const
{
    if (UWorld* W = GetWorld())
    {
        if (UCodeRescueGameInstance* GI = W->GetGameInstance<UCodeRescueGameInstance>())
        {
            return GI->bSubtitlesEnabled;
        }
    }
    return true;
}
