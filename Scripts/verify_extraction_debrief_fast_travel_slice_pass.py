#!/usr/bin/env python3
"""Static verifier for the extraction debrief fast-travel slice."""

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
helipad_cpp = read(SRC / "HelipadActor.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "EXTRACTION_DEBRIEF_FAST_TRAVEL_SLICE.md")

construct_body = function_body(widget_cpp, "void UCityFastTravelWidget::NativeConstruct")
configure_body = function_body(widget_cpp, "void UCityFastTravelWidget::ConfigureOpeningHelipadContext")
continue_body = function_body(widget_cpp, "void UCityFastTravelWidget::OnContinueExtractionClicked")
debrief_body = function_body(widget_cpp, "FString UCityFastTravelWidget::BuildExtractionDebriefText")
teleport_body = function_body(widget_cpp, "void UCityFastTravelWidget::TeleportPlayerToCity")
open_body = function_body(helipad_cpp, "void AHelipadActor::OpenFastTravelMenu")

check_all(
    widget_h,
    [
        "ConfigureOpeningHelipadContext",
        "OnContinueExtractionClicked",
        "ContinueButton",
        "DebriefText",
        "SourceCityIndex",
        "ContinueDestinationCityIndex",
        "ExtractionSurvivorName",
        "bSourceExtractionReady",
        "BuildExtractionDebriefText",
    ],
    "fast-travel widget header must expose extraction debrief state",
)
check_all(
    construct_body,
    [
        "EVAC HELI - extraction debrief",
        "ExtractionDebrief",
        "BuildExtractionDebriefText(GI)",
        "FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI)",
        "ContinueExtractionButton",
        "OnContinueExtractionClicked",
        "Continue operation",
    ],
    "fast-travel construct must build debrief text and continue action",
)
check_all(
    configure_body,
    [
        "SourceCityIndex = InSourceCityIndex",
        "SourceCityLabel = InSourceCityLabel",
        "bSourceExtractionReady = bInExtractionReady",
        "ExtractionSurvivorName = InSurvivorName",
        "ExtractionAccentColor = InAccentColor",
    ],
    "helipad context must be stored before widget construction",
)
check_all(
    continue_body,
    [
        "ContinueDestinationCityIndex != INDEX_NONE",
        "TeleportPlayerToCity(ContinueDestinationCityIndex)",
        "Close()",
    ],
    "continue extraction click must teleport and close",
)
check_all(
    debrief_body,
    [
        "Extraction confirmed",
        "GI->GetLanguageName()",
        "progression is saved",
        "FCodeRescueCampaign::GetMissionLabel(SourceCityIndex)",
    ],
    "debrief text must mention rescue, city, active language save, and fallback labels",
)
check_all(
    teleport_body,
    [
        "FCodeRescueCampaign::GetPlayerStartLocation(CityIndex)",
        "Pawn->TeleportTo",
        "GI->SavePersistentRun()",
        "StartCameraFade",
    ],
    "fast travel teleport must save the active language run after moving",
)
check_all(
    open_body,
    [
        "Cast<UCityFastTravelWidget>(ActiveFastTravelWidget)",
        "ConfigureOpeningHelipadContext",
        "CityIndex",
        "CityLabel",
        "bExtractionReady",
        "ExtractionSurvivorName",
        "ExtractionAccentColor",
    ],
    "helipad must pass extraction context into the fast-travel widget",
)
check("verify_extraction_debrief_fast_travel_slice_pass.py" in full_qa,
      "full QA must run the extraction debrief fast-travel verifier")
check("verify_extraction_debrief_fast_travel_slice_pass.py" in local_ci,
      "local CI must run the extraction debrief fast-travel verifier")
check("Extraction debrief fast-travel slice" in progress,
      "progress log must document the extraction debrief fast-travel slice")
check_all(
    slice_doc,
    [
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "extraction debrief",
        "helipad",
        "fast travel",
        "next city",
        "language save",
    ],
    "slice doc must map the extraction debrief fast-travel work to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_extraction_debrief_fast_travel_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_extraction_debrief_fast_travel_slice_pass] PASS: extraction debrief fast-travel slice verified")
