# UX / UI Design-System Overhaul — 2026-06-25

This pass introduces a **centralized UI design system** for Operation Code Rescue and
refactors the highest-visibility screens onto it. The goal is a *radically* more cohesive,
readable, and accessible interface in the project's established **"refined survival-horror"**
direction — without throwing away the existing procedural-UMG-in-C++ approach.

## The problem this solves

Every UMG screen was built procedurally in C++ with **hardcoded, ad-hoc values**: dozens of
slightly different `FLinearColor(...)` literals for the same conceptual color, font sizes set
inline per widget (`19`, `17`, `20`, `28`, `30`, `42`...), shadow offsets duplicated everywhere,
and spacing magic-numbers (`FMargin(0, 7, 0, 7)`). The result was a UI that worked but lacked a
single visual language: amber titles drifted between `(0.86,0.74,0.46)` and `(0.84,0.74,0.48)`,
panels between `(0.040,0.035,0.030)` and `(0.035,0.030,0.025)`, and there was no shared notion of
type scale, spacing rhythm, or how accessibility settings should change the look.

## The solution: `CodeRescueUITheme`

New files:

- `Source/CodeRescueUnreal/CodeRescueUITheme.h`
- `Source/CodeRescueUnreal/CodeRescueUITheme.cpp`

A header-only token layer plus a small `.cpp` of widget-styling helpers. No new module, no
`UCLASS`, no UHT dependency — it compiles as part of the existing module (Unreal auto-compiles
every `.cpp` under `Source/CodeRescueUnreal/`, so no `Build.cs` change is needed).

### Color tokens (`CodeRescueUI::Color` / `::Surface`)

Semantic, not literal. Use the meaning, never a raw color:

| Token | Use |
|---|---|
| `Color::TextPrimary/Secondary/Muted` | warm off-white text hierarchy |
| `Color::AccentAmber` / `AccentEmber` | "field terminal / emergency power" accents, primary actions |
| `Color::TerminalGreen[Bright]` | reserved for the **coding/safe** layer (terminals, success) |
| `Color::Danger[Bright]` / `Health` | reserved for **danger / health loss** — never reused for chrome |
| `Color::Warning` / `Stamina` | caution states / stamina bar |
| `Surface::Panel/Raised/Sunken` | panel fills (near-black warm, never pure #000) |
| `Surface::BorderSubtle/Strong`, `ButtonFill/Hover` | frames and button fills |

### Type scale (`CodeRescueUI::EType`)

One ramp: `Display(46) · TitleXL(34) · Title(26) · Heading(20) · Subheading(18) · Body(16) ·
BodySmall(14) · Caption(12)`. All sizes pass through `ScaledSize()` which multiplies by the
accessibility text-scale and clamps to `[9, 96]`.

### Spacing scale (`CodeRescueUI::Space`)

`XS=4 · S=8 · M=12 · L=16 · XL=24 · XXL=40`. Use these instead of arbitrary margins so vertical
rhythm is consistent across screens.

### Helpers

```cpp
using namespace CodeRescueUI;
StyleText(MyText, EType::Display, Color::AccentAmber());   // font + color + shadow in one call
StylePanel(MyBorder, Surface::Panel());                    // fill + consistent padding
StylePrimaryButton(StartBtn);                              // warm raised fill, white content tint
StyleSecondaryButton(BackBtn);                             // subtle fill; label carries the color
```

> **Why white content tint on buttons?** `UButton::SetColorAndOpacity` multiplies into child
> widgets. Setting the button tint to white lets the child label render at its true themed color
> instead of being darkened by an amber-on-amber multiply.

### Accessibility is built in

`CodeRescueUI::Theme()` is a header-only singleton holding `{ bHighContrast, bReducedMotion,
TextScale }`. It is now **mirrored from the player's saved settings**
(`UCodeRescueGameInstance::bHighContrastHUD / bReducedMotion / UITextScale`) in the settings and
gameplay entry points, while `SubtitleScale` remains dedicated to the subtitle overlay:

1. `UCodeRescueSettingsWidget::NativeConstruct` (seed on open)
2. `UCodeRescueSettingsWidget::OnApplyClicked` (live update when the player changes a setting)
3. `UCodeRescueHUDWidget::NativeConstruct` (so gameplay honors it from the first frame)

Then every `Style*` helper calls `Resolve()`, which — when high contrast is on — widens luminance
separation (brightens light text, darkens dark fills) and forces chrome to near-opaque. Text scale
flows automatically through `ScaledSize()`. **High Contrast and text scaling now have real,
visible effects across the themed UI**, where previously the flags were stored but barely used.

## What was refactored in this pass

| Widget | Change |
|---|---|
| `CodeRescueMainMenuWidget` | title, tagline, mood line, menu panel, all buttons → theme |
| `CodeRescuePauseWidget` | title, panel, all buttons → theme |
| `CodeRescueSettingsWidget` | **added a readable dark backdrop**, title, all labels, Apply/Close buttons → theme; wired settings → `Theme()` |
| `CodeRescueDeathWidget` | title (Danger), subtitle, stats, panel, buttons → theme |
| `CodeRescueVictoryWidget` | title, stats, panel, buttons → theme |
| `CodeRescueDamageFeedbackWidget` | critical-HP vignette now honors **Reduced Motion** (steady vs. pulsing) |
| `CodeRescueHUDWidget` | syncs `Theme()` from settings at construct |

## Rollout plan for the remaining screens (next pass)

The pattern is mechanical and low-risk. For each widget, replace inline literals with `Style*`
calls. Recommended order by player visibility:

1. **`CodeRescueHUDWidget`** (982 lines) — biggest win. Replace per-element color/size literals:
   - `StatusText` → `StyleText(.., Body, Color::TextPrimary())`
   - `ObjectiveFocusText` → `Heading, Color::AccentAmber()`
   - `InteractionPromptText` → `Subheading, Color::AccentAmber()`
   - `HealthBar` fill → drive by health fraction between `Color::Health()` and `Color::TerminalGreen()`
   - `StaminaBar` fill → `Color::Stamina()`
   - alerts (low health / low ammo) → `Color::DangerBright()` / `Color::Warning()`
   - Gate the headshot "pop" animation behind `!Theme().bReducedMotion`.
2. **`CodeTerminalWidget`** (1,146 lines) — the coding surface. Use `Color::TerminalGreen()` for
   prompts/success, `Color::Danger()` for compile errors, monospace body. (See the
   Character-Animation/World PDFs for the diegetic-terminal art direction.)
3. `CodeRescueTutorialWidget`, `CodeRescueObjectiveJournalWidget`, `CodeRescueSkillTreeWidget`,
   `CodeRescueSaveSlotsWidget`, `CityFastTravelWidget`, `CodeRescueMinimapWidget`,
   `CodeRescueSubtitlesWidget` (wire its scale to `Theme().TextScale`).

## Verification

- The theme files are additive and self-contained; existing member variables, event bindings, and
  gameplay logic are untouched — only color/font/spacing/padding calls changed.
- Build on Mac with the existing flow: `Recompile_Module.command` (or open the `.uproject` and let
  it rebuild). Then run `Run_Launch_Menu_Visual_Check.command` and capture screenshots at 1280×720
  and native size; confirm the menu/pause/settings/death/victory screens read cleanly.
- Toggle **High Contrast** and **Subtitle/Text size** in Settings → Apply, and confirm the UI
  visibly responds. Toggle **Reduced Motion** and confirm the critical-HP vignette stops pulsing.

## Design rationale (refined survival-horror)

- **Never pure black.** Backgrounds are warm near-black (`0.012, 0.014, 0.012`) to preserve depth
  and avoid OLED smear; panels are a touch warmer still.
- **Amber = power, green = code, red = danger.** Reserving hues by meaning lets the player read
  state pre-attentively: a red flash always means harm, green always means "safe/solved."
- **One rhythm.** Shared type and spacing scales make dense survival HUDs legible under stress and
  in fog, and keep menus feeling like one product rather than many hand-built screens.
