#!/usr/bin/env python3
"""Static verifier for the HUD vitals theme/accessibility slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
safe_learning_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/safe_learning_city_controls_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "HUD_VITALS_THEME_ACCESSIBILITY_SLICE.md")

construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
health_fill_body = function_body(hud_cpp, "FLinearColor HudHealthFillColor")
health_label_body = function_body(hud_cpp, "FLinearColor HudHealthLabelColor")
stamina_fill_body = function_body(hud_cpp, "FLinearColor HudStaminaFillColor")
state_body = function_body(hud_cpp, "FString HudVitalStateLabel")

check_all(
    hud_cpp,
    [
        "HudVitalStateLabel",
        "HudHealthFillColor",
        "HudHealthLabelColor",
        "HudStaminaFillColor",
        "CodeRescueUI::Resolve",
        "CodeRescueUI::Color::TerminalGreen()",
        "CodeRescueUI::Color::TerminalGreenBright()",
        "CodeRescueUI::Color::Stamina()",
    ],
    "HUD implementation must define themed helper functions for vitals",
)
check_all(
    state_body,
    [
        "TEXT(\"CRITICAL\")",
        "TEXT(\"LOW\")",
        "TEXT(\"STABLE\")",
        "0.25f",
        "0.55f",
    ],
    "state helper must expose explicit health state labels",
)
check_all(
    health_fill_body,
    [
        "GI && GI->bHighContrastHUD",
        "CodeRescueUI::Color::DangerBright()",
        "CodeRescueUI::Color::Warning()",
        "CodeRescueUI::Color::TerminalGreen()",
        "FLinearColor(1.0f, 0.92f, 0.12f, 1.0f)",
    ],
    "health fill helper must provide standard and high-contrast green/amber/red states",
)
check_all(
    health_label_body,
    [
        "GI && GI->bHighContrastHUD",
        "CodeRescueUI::Color::DangerBright()",
        "CodeRescueUI::Color::Warning()",
        "CodeRescueUI::Color::TerminalGreenBright()",
    ],
    "health label helper must provide readable standard and high-contrast text colors",
)
check_all(
    stamina_fill_body,
    [
        "GI && GI->bHighContrastHUD",
        "CodeRescueUI::Color::AccentEmber()",
        "CodeRescueUI::Color::AccentAmber()",
        "CodeRescueUI::Color::Stamina()",
        "FLinearColor(0.50f, 0.86f, 1.0f, 1.0f)",
    ],
    "stamina fill helper must use the shared stamina token and depletion states",
)
check_all(
    construct_body,
    [
        "CodeRescueUI::StyleText(",
        "HudHealthLabelColor(1.0f",
        "HealthLabelSlot->SetSize(FVector2D(640.0f, 36.0f))",
        "HealthBar->SetFillColorAndOpacity(HudHealthFillColor(1.0f",
        "StaminaBar->SetFillColorAndOpacity(HudStaminaFillColor(1.0f",
    ],
    "construct path must initialize health/stamina with themed scalable vitals",
)
check_all(
    refresh_body,
    [
        "CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD",
        "CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion",
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()",
        "HealthBar->SetFillColorAndOpacity(HudHealthFillColor(HealthPct, GI))",
        "TEXT(\"PLAYER HEALTH  %.0f / %.0f   %.0f%%   %s\")",
        "HudVitalStateLabel(HealthPct)",
        "HudHealthLabelColor(HealthPct, GI)",
        "StaminaBar->SetFillColorAndOpacity(HudStaminaFillColor(StaminaPct, GI))",
    ],
    "refresh path must mirror accessibility settings and apply helper-driven vitals",
)
check_all(
    access_manifest,
    [
        "HUDVitalsAccessibility",
        "bHighContrastHUD + UITextScale",
        "STABLE/LOW/CRITICAL",
    ],
    "accessibility manifest must document HUD vitals accessibility coverage",
)
check_all(
    visual_manifest,
    [
        "HUDVitals",
        "Health bar green/amber/red ramp",
        "stamina blue/amber/ember ramp",
    ],
    "visual regression targets must include HUD vitals",
)
check_all(
    safe_learning_manifest,
    [
        "health_gauge",
        "themed health/stamina vitals",
        "STABLE/LOW/CRITICAL",
    ],
    "safe-learning controls manifest must describe the upgraded health gauge",
)
check("verify_hud_vitals_theme_accessibility_slice_pass.py" in full_qa,
      "full QA must run the HUD vitals verifier")
check("verify_hud_vitals_theme_accessibility_slice_pass.py" in local_ci,
      "local CI must run the HUD vitals verifier")
check("HUD vitals theme accessibility slice" in progress,
      "progress log must document the HUD vitals slice")
check_all(
    slice_doc,
    [
        "HUD Vitals Theme Accessibility Slice",
        "HudHealthFillColor",
        "HudStaminaFillColor",
        "STABLE",
        "LOW",
        "CRITICAL",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_hud_vitals_theme_accessibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_hud_vitals_theme_accessibility_slice_pass] PASS: HUD vitals theme/accessibility verified")
