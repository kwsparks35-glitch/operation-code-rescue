#include "CodeRescueUITheme.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"

namespace CodeRescueUI
{
    FLinearColor Resolve(const FLinearColor& In)
    {
        if (!Theme().bHighContrast)
        {
            return In;
        }

        // High-contrast: widen luminance separation so foreground text pops off
        // dark survival-horror panels, and force chrome toward near-opaque.
        FLinearColor Out = In;
        const float Luma = 0.2126f * In.R + 0.7152f * In.G + 0.0722f * In.B;
        if (Luma >= 0.40f)
        {
            Out.R = FMath::Min(1.0f, In.R * 1.18f + 0.06f);
            Out.G = FMath::Min(1.0f, In.G * 1.18f + 0.06f);
            Out.B = FMath::Min(1.0f, In.B * 1.18f + 0.06f);
        }
        else
        {
            Out.R = In.R * 0.55f;
            Out.G = In.G * 0.55f;
            Out.B = In.B * 0.55f;
        }
        Out.A = FMath::Max(In.A, 0.96f);
        return Out;
    }

    void ApplyType(UTextBlock* Text, EType T)
    {
        if (!Text)
        {
            return;
        }
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = ScaledSize(T);
        Text->SetFont(Font);
    }

    void StyleText(UTextBlock* Text, EType T, const FLinearColor& InColor, bool bShadow)
    {
        if (!Text)
        {
            return;
        }
        ApplyType(Text, T);
        Text->SetColorAndOpacity(FSlateColor(Resolve(InColor)));
        if (bShadow)
        {
            // Larger headings get a slightly heavier shadow for legibility in fog.
            const float Drop = (T == EType::Display || T == EType::TitleXL) ? 3.0f
                             : (T == EType::Title || T == EType::Heading) ? 2.0f : 1.0f;
            Text->SetShadowOffset(FVector2D(Drop, Drop));
            Text->SetShadowColorAndOpacity(Color::Shadow());
        }
    }

    void StylePanel(UBorder* Border, const FLinearColor& Fill)
    {
        StylePanel(Border, Fill, FMargin(Space::XL, Space::L));
    }

    void StylePanel(UBorder* Border, const FLinearColor& Fill, const FMargin& Padding)
    {
        if (!Border)
        {
            return;
        }
        Border->SetBrushColor(Resolve(Fill));
        Border->SetPadding(Padding);
    }

    void StylePrimaryButton(UButton* Button)
    {
        if (!Button)
        {
            return;
        }
        // Primary = warmer raised fill; the child label carries the accent color.
        // Content tint stays white so the label renders at its true color (button
        // SetColorAndOpacity multiplies into child widgets).
        Button->SetBackgroundColor(Resolve(Surface::ButtonHover()));
        Button->SetColorAndOpacity(FLinearColor::White);
    }

    void StyleSecondaryButton(UButton* Button)
    {
        if (!Button)
        {
            return;
        }
        // Secondary = subtle dark fill; the child label carries the color.
        Button->SetBackgroundColor(Resolve(Surface::ButtonFill()));
        Button->SetColorAndOpacity(FLinearColor::White);
    }
}
