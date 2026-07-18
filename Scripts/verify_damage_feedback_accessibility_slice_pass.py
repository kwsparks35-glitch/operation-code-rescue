#!/usr/bin/env python3
"""Static verifier for the damage feedback accessibility slice."""

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


damage_h = read(SRC / "CodeRescueDamageFeedbackWidget.h")
damage_cpp = read(SRC / "CodeRescueDamageFeedbackWidget.cpp")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
enemy_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/enemy_readability_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "DAMAGE_FEEDBACK_ACCESSIBILITY_SLICE.md")

construct_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::NativeConstruct")
destruct_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::NativeDestruct")
tick_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::NativeTick")
refresh_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState")
apply_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::ApplyAccessibilityStateFromSettings")
vignette_body = function_body(damage_cpp, "FLinearColor UCodeRescueDamageFeedbackWidget::GetVignetteColor")
chevron_body = function_body(damage_cpp, "FLinearColor UCodeRescueDamageFeedbackWidget::GetChevronColor")
duration_body = function_body(damage_cpp, "float UCodeRescueDamageFeedbackWidget::GetDirectionalFlashDuration")
resize_body = function_body(damage_cpp, "void UCodeRescueDamageFeedbackWidget::ResizeDirectionalChevron")
settings_apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")
hud_refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    damage_h,
    [
        "virtual void NativeDestruct() override",
        "static void RefreshAccessibilityState()",
        "ApplyAccessibilityStateFromSettings",
        "GetVignetteColor",
        "GetChevronColor",
        "GetDirectionalFlashDuration",
        "ResizeDirectionalChevron",
        "bDamageFeedbackHighContrast",
        "bDamageFeedbackReducedMotion",
        "ReducedMotionDirectionalFlashDuration",
        "ActiveInstance",
    ],
    "damage feedback header must expose accessibility refresh and state helpers",
)
check_all(
    damage_cpp,
    [
        "#include \"CodeRescueGameInstance.h\"",
        "UCodeRescueDamageFeedbackWidget::ActiveInstance = nullptr",
    ],
    "damage feedback implementation must include game instance settings and active widget storage",
)
check_all(
    construct_body,
    [
        "ActiveInstance = this",
        "ApplyAccessibilityStateFromSettings()",
    ],
    "construct path must register and apply accessibility state",
)
check_all(
    destruct_body,
    [
        "ActiveInstance == this",
        "ActiveInstance = nullptr",
    ],
    "destruct path must clear the active damage overlay pointer",
)
check_all(
    tick_body,
    [
        "ApplyAccessibilityStateFromSettings()",
        "bDamageFeedbackReducedMotion",
        "GetVignetteColor(Alpha)",
        "GetDirectionalFlashDuration()",
        "GetChevronColor(0.0f)",
        "bDamageFeedbackReducedMotion",
        "? 0.92f",
    ],
    "tick path must use live accessibility state for vignette and chevron flashes",
)
check_all(
    refresh_body,
    [
        "ActiveInstance",
        "ActiveInstance->ApplyAccessibilityStateFromSettings()",
    ],
    "static refresh hook must apply state to the live overlay",
)
check_all(
    apply_body,
    [
        "GI->bHighContrastHUD",
        "GI->bReducedMotion",
        "CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD",
        "CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion",
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()",
        "ResizeDirectionalChevron(DirN, true)",
        "ResizeDirectionalChevron(DirE, false)",
    ],
    "damage overlay must read saved accessibility state and resize chevrons",
)
check_all(
    vignette_body,
    [
        "bDamageFeedbackHighContrast",
        "FLinearColor(1.0f, 0.82f, 0.06f",
        "FLinearColor(0.6f, 0.0f, 0.0f",
    ],
    "vignette helper must provide standard and high-contrast palettes",
)
check_all(
    chevron_body,
    [
        "bDamageFeedbackHighContrast",
        "FLinearColor(1.0f, 0.92f, 0.08f",
        "FLinearColor(1.0f, 0.05f, 0.05f",
    ],
    "chevron helper must provide standard and high-contrast palettes",
)
check_all(
    duration_body,
    [
        "bDamageFeedbackReducedMotion",
        "ReducedMotionDirectionalFlashDuration",
        "DirectionalFlashDuration",
    ],
    "duration helper must switch timing in reduced motion",
)
check_all(
    resize_body,
    [
        "UCanvasPanelSlot",
        "HighContrastSize",
        "FVector2D(196.0f, 28.0f)",
        "FVector2D(28.0f, 196.0f)",
    ],
    "resize helper must enlarge directional cues for high contrast",
)
check_all(
    settings_cpp,
    [
        "#include \"CodeRescueDamageFeedbackWidget.h\"",
        "UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState()",
    ],
    "settings implementation must include and call the damage feedback refresh hook",
)
check_all(
    settings_apply_body,
    [
        "CodeRescueUI::Theme().bHighContrast = bCachedHighContrast",
        "CodeRescueUI::Theme().bReducedMotion = bCachedReducedMotion",
        "UCodeRescueSubtitlesWidget::RefreshAccessibilityState()",
        "UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState()",
    ],
    "Settings Apply must refresh live accessibility overlays together",
)
check_all(
    hud_refresh_body,
    [
        "GI->bHighContrastHUD",
        "DamageAlertColor",
        "FLinearColor(1.0f, 0.94f, 0.16f, Alpha)",
        "FLinearColor(1.0f, 0.30f, 0.18f, Alpha)",
    ],
    "HUD damage alert must use high-contrast and standard colors",
)
check_all(
    access_manifest,
    [
        "DamageFeedbackAccessibility",
        "bHighContrastHUD + bReducedMotion",
        "active damage overlay",
        "steady reduced-motion hit flashes",
    ],
    "accessibility manifest must document damage feedback behavior",
)
check_all(
    enemy_manifest,
    [
        "DamageFeedback",
        "accessible hit overlay",
        "high-contrast chevrons",
        "reduced-motion steady flashes",
    ],
    "enemy readability manifest must document accessible damage feedback",
)
check("verify_damage_feedback_accessibility_slice_pass.py" in full_qa,
      "full QA must run the damage feedback accessibility verifier")
check("verify_damage_feedback_accessibility_slice_pass.py" in local_ci,
      "local CI must run the damage feedback accessibility verifier")
check("Damage feedback accessibility slice" in progress,
      "progress log must document the damage feedback accessibility slice")
check_all(
    slice_doc,
    [
        "Damage Feedback Accessibility Slice",
        "RefreshAccessibilityState",
        "ApplyAccessibilityStateFromSettings",
        "high-contrast amber",
        "ReducedMotionDirectionalFlashDuration",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_damage_feedback_accessibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_damage_feedback_accessibility_slice_pass] PASS: damage feedback accessibility verified")
