#pragma once

#include "CoreMinimal.h"

/**
 * Operation Code Rescue - centralized UI design system ("Refined Survival-Horror").
 *
 * Single source of truth for color, type scale, spacing, and common widget
 * styling so every procedurally-built UMG screen shares a cohesive, readable,
 * accessible look. Tokens are exposed as inline functions in this header;
 * widget-touching helpers are implemented in CodeRescueUITheme.cpp.
 *
 * Design intent (refined survival-horror):
 *   - Near-black warm backgrounds; never pure #000 (keeps depth, reduces smear).
 *   - Amber/ember accents read as "field terminal / emergency power".
 *   - Phosphor green is reserved for the coding/safe layer (terminals, success).
 *   - Blood red is reserved for danger/health loss; do not reuse for chrome.
 *   - One type scale, one spacing scale, consistent shadows for legibility.
 *
 * Usage:
 *   using namespace CodeRescueUI;
 *   StyleText(TitleText, EType::Display, Color::AccentAmber());
 *   StylePanel(MenuBorder, Surface::Panel());
 *   StylePrimaryButton(StartBtn);
 *
 * Accessibility: call CodeRescueUI::Theme() once at HUD/menu construct to mirror
 * the player's settings (high contrast, reduced motion, text scale); every
 * helper then honors them automatically.
 */

class UTextBlock;
class UButton;
class UBorder;
struct FMargin; // used by reference in StylePanel(); full definition lives in the .cpp

namespace CodeRescueUI
{
    /** Runtime-adjustable accessibility state (mirror from settings/GameInstance). */
    struct FThemeState
    {
        bool bHighContrast = false;
        bool bReducedMotion = false;
        float TextScale = 1.0f; // clamped 0.80 .. 1.75 when applied
    };

    /** Header-only singleton so widgets read/write theme state without a global symbol. */
    inline FThemeState& Theme()
    {
        static FThemeState State;
        return State;
    }

    // ---- Color tokens (linear). Resolve() applies high-contrast at use time. ----
    namespace Color
    {
        inline FLinearColor BackgroundDeep()      { return FLinearColor(0.012f, 0.014f, 0.012f, 1.0f); }
        inline FLinearColor TextPrimary()         { return FLinearColor(0.94f, 0.91f, 0.83f, 1.0f); }
        inline FLinearColor TextSecondary()       { return FLinearColor(0.78f, 0.74f, 0.66f, 1.0f); }
        inline FLinearColor TextMuted()           { return FLinearColor(0.56f, 0.53f, 0.48f, 1.0f); }
        inline FLinearColor AccentAmber()         { return FLinearColor(0.94f, 0.72f, 0.30f, 1.0f); }
        inline FLinearColor AccentEmber()         { return FLinearColor(0.88f, 0.46f, 0.18f, 1.0f); }
        inline FLinearColor TerminalGreen()       { return FLinearColor(0.55f, 0.86f, 0.62f, 1.0f); }
        inline FLinearColor TerminalGreenBright() { return FLinearColor(0.71f, 0.96f, 0.77f, 1.0f); }
        inline FLinearColor Danger()              { return FLinearColor(0.88f, 0.26f, 0.21f, 1.0f); }
        inline FLinearColor DangerBright()        { return FLinearColor(0.97f, 0.36f, 0.29f, 1.0f); }
        inline FLinearColor Warning()             { return FLinearColor(0.95f, 0.75f, 0.31f, 1.0f); }
        inline FLinearColor Health()              { return FLinearColor(0.82f, 0.27f, 0.22f, 1.0f); }
        inline FLinearColor Stamina()             { return FLinearColor(0.36f, 0.66f, 0.86f, 1.0f); }
        inline FLinearColor Shadow()              { return FLinearColor(0.0f, 0.0f, 0.0f, 0.95f); }
    }

    // ---- Surfaces (panel fills / borders / buttons) ----
    namespace Surface
    {
        inline FLinearColor Panel()        { return FLinearColor(0.045f, 0.040f, 0.034f, 0.92f); }
        inline FLinearColor Raised()       { return FLinearColor(0.085f, 0.075f, 0.062f, 0.95f); }
        inline FLinearColor Sunken()       { return FLinearColor(0.020f, 0.020f, 0.018f, 0.94f); }
        inline FLinearColor BorderSubtle() { return FLinearColor(0.16f, 0.13f, 0.10f, 1.0f); }
        inline FLinearColor BorderStrong() { return FLinearColor(0.34f, 0.24f, 0.13f, 1.0f); }
        inline FLinearColor ButtonFill()   { return FLinearColor(0.10f, 0.085f, 0.065f, 0.95f); }
        inline FLinearColor ButtonHover()  { return FLinearColor(0.20f, 0.15f, 0.09f, 0.98f); }
    }

    // ---- Type scale (point sizes, before accessibility text-scale) ----
    enum class EType : uint8
    {
        Display, TitleXL, Title, Heading, Subheading, Body, BodySmall, Caption
    };

    inline int32 BaseSize(EType T)
    {
        switch (T)
        {
        case EType::Display:    return 46;
        case EType::TitleXL:    return 34;
        case EType::Title:      return 26;
        case EType::Heading:    return 20;
        case EType::Subheading: return 18;
        case EType::Body:       return 16;
        case EType::BodySmall:  return 14;
        case EType::Caption:    return 12;
        default:                return 16;
        }
    }

    /** Final point size after applying the accessibility text-scale, clamped sane. */
    inline int32 ScaledSize(EType T)
    {
        const float Clamped = FMath::Clamp(Theme().TextScale, 0.80f, 1.75f);
        return FMath::Clamp(FMath::RoundToInt((float)BaseSize(T) * Clamped), 9, 96);
    }

    // ---- Spacing scale (slate units) ----
    namespace Space
    {
        constexpr float XS  = 4.0f;
        constexpr float S   = 8.0f;
        constexpr float M   = 12.0f;
        constexpr float L   = 16.0f;
        constexpr float XL  = 24.0f;
        constexpr float XXL = 40.0f;
    }

    /** High-contrast-aware color resolve (separates luminance, forces near-opaque). */
    FLinearColor Resolve(const FLinearColor& In);

    // ---- Widget styling helpers (implemented in CodeRescueUITheme.cpp) ----
    void ApplyType(UTextBlock* Text, EType T);
    void StyleText(UTextBlock* Text, EType T, const FLinearColor& InColor, bool bShadow = true);
    void StylePanel(UBorder* Border, const FLinearColor& Fill);
    void StylePanel(UBorder* Border, const FLinearColor& Fill, const FMargin& Padding);
    void StylePrimaryButton(UButton* Button);
    void StyleSecondaryButton(UButton* Button);
}
