#!/usr/bin/env python3
"""Static verifier for the minimap route readability slice."""

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


widget_h = read(SRC / "CodeRescueMinimapWidget.h")
widget_cpp = read(SRC / "CodeRescueMinimapWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
onboarding = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MINIMAP_ROUTE_READABILITY_SLICE.md")

construct_body = function_body(widget_cpp, "void UCodeRescueMinimapWidget::NativeConstruct")
tick_body = function_body(widget_cpp, "void UCodeRescueMinimapWidget::NativeTick")
refresh_body = function_body(widget_cpp, "void UCodeRescueMinimapWidget::RefreshMinimap")
draw_body = function_body(widget_cpp, "void UCodeRescueMinimapWidget::DrawDot")
summary_body = function_body(widget_cpp, "FString MakeMinimapSummary")
route_body = function_body(widget_cpp, "FString MakeRouteCue")

check_all(
    widget_h,
    [
        "class UBorder",
        "UCanvasPanel* RootCanvas",
        "UBorder* PanelFrame",
        "UTextBlock* TitleText",
        "UTextBlock* SummaryText",
        "UTextBlock* RouteCueText",
        "UTextBlock* LegendText",
        "MapAreaSizePx",
        "RefreshAccumulatorSeconds",
        "void DrawDot(const FVector2D& RelOffset, const FLinearColor& Color, float SizePx)",
    ],
    "minimap header must expose themed frame, readable text fields, stable plot size, and size-coded dots",
)
check_all(
    widget_cpp,
    [
        "#include \"CodeRescueGameInstance.h\"",
        "#include \"CodeRescueUITheme.h\"",
        "#include \"Components/Border.h\"",
        "MirrorMinimapThemeFromSettings",
        "MinimapDotColor",
        "DirectionFromDelta",
        "MakeMinimapSummary",
        "MakeRouteCue",
        "CodeRescueUI::Theme().bHighContrast",
        "CodeRescueUI::Theme().bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "minimap implementation must use shared UI theme, settings mirroring, route helpers, and high-contrast dot helpers",
)
check_all(
    construct_body,
    [
        "MinimapRootCanvas",
        "MinimapThemedPanel",
        "CodeRescueUI::StylePanel",
        "MinimapDotCanvas",
        "NAV MAP",
        "MinimapSummary",
        "MinimapRouteCue",
        "P YOU  T CODE  S RESCUE  L LANG  ! THREAT",
        "RefreshMinimap()",
    ],
    "construct path must build the themed NAV MAP frame, scan summary, route cue, legend, and dot layer",
)
check_all(
    tick_body,
    [
        "GI->bReducedMotion",
        "? 0.25f : 0.10f",
        "RefreshAccumulatorSeconds",
        "RefreshMinimap()",
    ],
    "tick path must honor reduced-motion refresh cadence",
)
check_all(
    draw_body,
    [
        "MapAreaSizePx",
        "ClampedDotSize",
        "DotSlot->SetSize(FVector2D(ClampedDotSize, ClampedDotSize))",
    ],
    "dot drawing must use the inner plot size and per-category dot sizes",
)
check_all(
    summary_body,
    [
        "GI->GetLanguageName()",
        "TEXT(\"%s | T%d S%d L%d !%d | %.0fm\")",
        "ViewRadius / 100.0f",
    ],
    "summary helper must show active language, visible category counts, and range",
)
check_all(
    route_body,
    [
        "Nearest: no objective in scan range",
        "Nearest %s: %.0fm %s",
        "DirectionFromDelta(NearestDelta)",
    ],
    "route helper must provide a text-first nearest objective cue",
)
check_all(
    refresh_body,
    [
        "MirrorMinimapThemeFromSettings(GI)",
        "CodeRescueUI::StyleText(TitleText",
        "CodeRescueUI::StyleText(SummaryText",
        "CodeRescueUI::StyleText(RouteCueText",
        "CodeRescueUI::StyleText(LegendText",
        "Route scan offline",
        "TEXT(\"CODE\")",
        "TEXT(\"RESCUE\")",
        "TEXT(\"LANG\")",
        "TEXT(\"THREAT\")",
        "DotSizePx + 4.0f",
        "DotSizePx + 2.5f",
        "MakeMinimapSummary(",
        "MakeRouteCue(NearestObjectiveKind, NearestObjectiveDelta)",
    ],
    "refresh path must restyle the map, handle offline state, draw size-coded categories, and update route text",
)
check_all(
    access_manifest,
    [
        "MinimapRouteAccessibility",
        "HUD minimap route scanner",
        "nearest-objective route cue",
    ],
    "accessibility manifest must document minimap route readability coverage",
)
check_all(
    visual_manifest,
    [
        "MinimapRouteMap",
        "NAV MAP frame",
        "not an unlabeled cluster of dots",
    ],
    "visual regression targets must include the minimap route scanner",
)
check_all(
    onboarding,
    [
        "NAV MAP nearest cue",
        "active language route scan",
        "minimap legend",
    ],
    "first ten minutes onboarding must teach the minimap route scanner",
)
check("verify_minimap_route_readability_slice_pass.py" in full_qa,
      "full QA must run the minimap route readability verifier")
check("verify_minimap_route_readability_slice_pass.py" in local_ci,
      "local CI must run the minimap route readability verifier")
check("Minimap route readability slice" in progress,
      "progress log must document the minimap route readability slice")
check_all(
    slice_doc,
    [
        "Minimap Route Readability Slice",
        "CodeRescueMinimapWidget",
        "NAV MAP",
        "MinimapSummary",
        "MinimapRouteCue",
        "MakeMinimapSummary",
        "MakeRouteCue",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_minimap_route_readability_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_minimap_route_readability_slice_pass] PASS: minimap route readability verified")
