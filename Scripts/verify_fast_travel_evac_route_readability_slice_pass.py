#!/usr/bin/env python3
"""Static verifier for the fast-travel evac route readability slice."""

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


widget_h = read(SRC / "CityFastTravelWidget.h")
widget_cpp = read(SRC / "CityFastTravelWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
onboarding = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "FAST_TRAVEL_EVAC_ROUTE_READABILITY_SLICE.md")

construct_body = function_body(widget_cpp, "void UCityFastTravelWidget::NativeConstruct")
summary_body = function_body(widget_cpp, "FString BuildFastTravelSummary")
destination_body = function_body(widget_cpp, "FString BuildDestinationLabel")

check_all(
    widget_h,
    [
        "class UBorder",
        "UTextBlock* SummaryText",
        "UBorder* PanelFrame",
        "ContinueButton",
        "ButtonToCityIndex",
    ],
    "fast travel header must carry themed panel and summary widgets without losing dispatch state",
)
check_all(
    widget_cpp,
    [
        "#include \"CodeRescueUITheme.h\"",
        "#include \"Components/BackgroundBlur.h\"",
        "#include \"Components/Border.h\"",
        "#include \"Components/ScrollBox.h\"",
        "MirrorFastTravelThemeFromSettings",
        "MakeFastTravelLabel",
        "StyleFastTravelButton",
        "FastTravelDebriefColor",
        "BuildFastTravelSummary",
        "BuildDestinationLabel",
        "CodeRescueUI::Theme().bHighContrast",
        "GI->bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "fast travel implementation must use shared theme, accessibility settings, and readable route helpers",
)
check_all(
    construct_body,
    [
        "SetIsFocusable(true)",
        "FastTravelBlur",
        "PanelFrame = WidgetTree->ConstructWidget<UBorder>",
        "EvacRoutePanel",
        "EVAC HELI - extraction debrief",
        "BuildExtractionDebriefText(GI)",
        "SummaryText = MakeFastTravelLabel",
        "BuildFastTravelSummary(GI, bSourceExtractionReady)",
        "EvacRouteScroll",
        "NEXT OPERATION",
        "Continue operation",
        "ContinueExtractionButton",
        "BuildDestinationLabel(GI, i, Mission)",
        "NO CLEARED DESTINATIONS",
    ],
    "construct path must build the themed evac panel, summary, next action, and redeploy rows",
)
check_all(
    summary_body,
    [
        "Active language: %s | Start-screen resume slot: %s",
        "MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "Cleared terminals: %d",
        "Rescued teams: %d",
        "saves this language run after arrival",
    ],
    "route summary must expose language save, progress counts, route mode, and save behavior",
)
check_all(
    destination_body,
    [
        "REDEPLOY | %s",
        "FCodeRescueCampaign::IsCityCompleted(GI, CityIndex)",
        "terminal solved + survivor extracted",
        "Mission->CurriculumFocus",
        "Arrival saves the %s run after fast travel.",
    ],
    "destination labels must explain availability, completion, curriculum focus, and post-travel save",
)
check_all(
    access_manifest,
    [
        "FastTravelEvacRouteAccessibility",
        "extraction debrief text",
        "reduced blur",
    ],
    "accessibility manifest must document fast-travel evac route coverage",
)
check_all(
    visual_manifest,
    [
        "FastTravelEvacRoute",
        "NEXT OPERATION action",
        "not a generic button list",
    ],
    "visual regression targets must include the fast-travel evac route overlay",
)
check_all(
    onboarding,
    [
        "Extraction debrief shows NEXT OPERATION",
        "active language save",
        "cleared-city redeploy choices",
    ],
    "first ten minutes onboarding must document the clearer extraction debrief",
)
check("verify_fast_travel_evac_route_readability_slice_pass.py" in full_qa,
      "full QA must run the fast-travel evac route readability verifier")
check("verify_fast_travel_evac_route_readability_slice_pass.py" in local_ci,
      "local CI must run the fast-travel evac route readability verifier")
check("Fast travel evac route readability slice" in progress,
      "progress log must document the fast-travel evac route readability slice")
check_all(
    slice_doc,
    [
        "Fast Travel Evac Route Readability Slice",
        "CityFastTravelWidget",
        "EvacRoutePanel",
        "BuildFastTravelSummary",
        "NEXT OPERATION",
        "REDEPLOY",
        "active language run",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_fast_travel_evac_route_readability_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_fast_travel_evac_route_readability_slice_pass] PASS: fast-travel evac route readability verified")
