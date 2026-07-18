#!/usr/bin/env python3
"""Static verifier for the expanded extraction set-pieces slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "expanded_extraction_set_pieces_manifest.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "EXPANDED_EXTRACTION_SET_PIECES_SLICE.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
set_piece_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnExpandedExtractionSetPieceForCity")

check_all(
    gamemode_h,
    [
        "SpawnExpandedExtractionSetPieceForCity",
        "Expanded, city-variant extraction staging around the helipad",
    ],
    "game mode header must declare the expanded extraction set-piece layer",
)

helipad_idx = spawn_city_body.find("SpawnHelipadForCity(CityIndex, Origin, CityLabel)")
expanded_idx = spawn_city_body.find("SpawnExpandedExtractionSetPieceForCity(Mission, CityIndex, Origin, CityLabel)")
check(helipad_idx >= 0, "campaign city spawn must still create the helipad")
check(expanded_idx > helipad_idx >= 0, "expanded extraction set piece must spawn immediately after the helipad")

check_all(
    set_piece_body,
    [
        "ROOFTOP LIFT",
        "CONVOY GATE",
        "HARBOR BOAT",
        "RAIL EVAC",
        "BRIDGE RUN",
        "HELIPAD COMMAND",
        "ExtractionSetPiece_RooftopLift",
        "ExtractionSetPiece_ConvoyGate",
        "ExtractionSetPiece_HarborBoat",
        "ExtractionSetPiece_RailEvac",
        "ExtractionSetPiece_BridgeRun",
        "ExtractionSetPiece_HelipadCommand",
    ],
    "set-piece layer must provide the six requested extraction variants",
)
check_all(
    set_piece_body,
    [
        "ExpandedExtractionSetPiece",
        "ExtractionSetPieceNonBlocking",
        "HelipadClearancePreserved",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "ReleaseDossier",
        "SpawnBlock",
        "SpawnRotatedBlock",
        "SpawnGuideText",
        "false",
        "[CodeRescueExtractionSetPiece]",
    ],
    "set-piece pieces must be cook-safe, tagged, logged, and nonblocking",
)
check_all(
    set_piece_body,
    [
        "Mission.SurvivorName",
        "Mission.LandmarkName",
        "Mission.CurriculumFocus",
        "Mission.CityName",
        "EXTRACTION SET PIECE",
    ],
    "set-piece labels must use mission survivor, landmark, curriculum, and city identity",
)

check_all(
    manifest,
    [
        "RooftopLift",
        "ConvoyGate",
        "HarborBoat",
        "RailEvac",
        "BridgeRun",
        "HelipadCommand",
        "SpawnExpandedExtractionSetPieceForCity",
        "ExtractionSetPieceNonBlocking",
        "HelipadClearancePreserved",
    ],
    "expanded extraction manifest must document every variant and nonblocking contract",
)
check_all(
    creative_plan,
    [
        "expanded extraction set pieces",
        "verify_expanded_extraction_set_pieces_slice_pass.py plus render smoke plus visual capture",
    ],
    "creative plan must move expanded extraction from manual-only to verified implementation",
)
check_all(
    curriculum_manifest,
    [
        "ExpandedExtractionSetPieces",
        "rescued survivor, lesson focus, city route, and landmark",
        "rooftop, convoy, boat, rail, bridge, or helipad-command staging",
    ],
    "curriculum feedback manifest must document the extraction payoff label",
)
check_all(
    onboarding,
    [
        "expanded extraction set piece",
        "rooftop, convoy, boat, rail, bridge, or helipad-command staging",
    ],
    "onboarding must mention readable extraction staging at graduation",
)
check_all(
    visual_manifest,
    [
        "ExpandedExtractionSetPieces",
        "rooftop, convoy, boat, rail, bridge, or helipad-command staging",
    ],
    "visual manifest must include the expanded extraction review target",
)
check_all(
    human_qa,
    [
        "ExpandedExtractionSetPieces",
        "nonblocking",
        "fast-travel menu",
    ],
    "human QA checklist must include traversal and fast-travel checks for extraction staging",
)
check_all(
    accessibility_manifest,
    [
        "ExpandedExtractionSetPieceAccessibility",
        "text-first labels",
        "nonblocking",
    ],
    "accessibility manifest must document text-first nonblocking extraction staging",
)
check_all(
    full_qa + local_ci,
    [
        "verify_expanded_extraction_set_pieces_slice_pass.py",
    ],
    "QA scripts must run the expanded extraction verifier",
)
check_all(
    progress,
    [
        "Expanded extraction set-pieces slice",
        "SpawnExpandedExtractionSetPieceForCity",
        "RooftopLift",
        "ConvoyGate",
        "HarborBoat",
        "RailEvac",
        "BridgeRun",
        "HelipadCommand",
    ],
    "progress log must summarize the expanded extraction implementation",
)
check_all(
    slice_doc,
    [
        "Expanded Extraction Set-Pieces Slice",
        "WORLD_DEVELOPMENT_DEEPDIVE.pdf",
        "SpawnExpandedExtractionSetPieceForCity",
        "RooftopLift",
        "HelipadCommand",
        "HelipadClearancePreserved",
    ],
    "slice documentation must cover source guidance, runtime hook, variants, and clearance contract",
)

if errors:
    print("Expanded extraction set-pieces verifier FAILED:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Expanded extraction set-pieces verifier passed.")
