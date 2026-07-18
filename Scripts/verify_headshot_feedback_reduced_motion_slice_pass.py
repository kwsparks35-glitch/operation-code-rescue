#!/usr/bin/env python3
"""Static verifier for the headshot feedback reduced-motion slice."""

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


hud_h = read(SRC / "CodeRescueHUDWidget.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
enemy_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/enemy_readability_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "HEADSHOT_FEEDBACK_REDUCED_MOTION_SLICE.md")

construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
headshot_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHeadshotFeedback")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    hud_h,
    [
        "class UCanvasPanelSlot",
        "class UCodeRescueGameInstance",
        "UCanvasPanelSlot* HeadshotFeedbackSlot",
        "RefreshHeadshotFeedback(float SinceHeadshot, const UCodeRescueGameInstance* GI)",
    ],
    "HUD header must expose headshot slot storage and refresh helper",
)
check_all(
    hud_cpp,
    [
        "HeadshotStandardDurationSeconds",
        "HeadshotReducedMotionDurationSeconds",
        "HeadshotBaseY",
    ],
    "HUD implementation must define reviewable headshot timing/layout constants",
)
check_all(
    construct_body,
    [
        "HeadshotFeedbackSlot = Root->AddChildToCanvas(HeadshotFeedbackText)",
        "HeadshotFeedbackSlot->SetPosition(FVector2D(0.0f, -88.0f))",
        "HeadshotFeedbackSlot->SetSize(FVector2D(420.0f, 46.0f))",
    ],
    "HUD construct path must retain the headshot feedback slot",
)
check_all(
    headshot_body,
    [
        "const bool bReducedMotion = GI && GI->bReducedMotion",
        "const bool bHighContrast = GI && GI->bHighContrastHUD",
        "HeadshotReducedMotionDurationSeconds",
        "HeadshotStandardDurationSeconds",
        "TEXT(\"PRECISION HIT\")",
        "TEXT(\"HEADSHOT\")",
        "FLinearColor(1.0f, 0.96f, 0.14f, Alpha)",
        "FMath::Lerp(34.0f, 28.0f, Normalized)",
        "? HeadshotBaseY",
        "FMath::Lerp(-76.0f, -112.0f, Normalized)",
    ],
    "headshot helper must branch for reduced motion, high contrast, and standard pop/fade",
)
check_all(
    refresh_body,
    [
        "Character->GetLastHeadshotWorldTime()",
        "RefreshHeadshotFeedback(SinceHeadshot, GI)",
    ],
    "RefreshHUD must delegate headshot rendering to the accessibility helper",
)
check_all(
    access_manifest,
    [
        "HeadshotReducedMotion",
        "bReducedMotion + bHighContrastHUD",
        "longer fixed precision-hit readout",
    ],
    "accessibility manifest must document headshot reduced-motion behavior",
)
check_all(
    enemy_manifest,
    [
        "PrecisionHitFeedback",
        "RefreshHeadshotFeedback",
        "without forced motion",
    ],
    "enemy readability manifest must document precision-hit feedback",
)
check("verify_headshot_feedback_reduced_motion_slice_pass.py" in full_qa,
      "full QA must run the headshot feedback verifier")
check("verify_headshot_feedback_reduced_motion_slice_pass.py" in local_ci,
      "local CI must run the headshot feedback verifier")
check("Headshot feedback reduced-motion slice" in progress,
      "progress log must document the headshot reduced-motion slice")
check_all(
    slice_doc,
    [
        "Headshot Feedback Reduced Motion Slice",
        "RefreshHeadshotFeedback",
        "PRECISION HIT",
        "High Contrast HUD",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_headshot_feedback_reduced_motion_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_headshot_feedback_reduced_motion_slice_pass] PASS: headshot feedback reduced motion verified")
