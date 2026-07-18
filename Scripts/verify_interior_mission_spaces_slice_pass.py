#!/usr/bin/env python3
"""Static verifier for the interior mission spaces slice."""

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
manifest = read(DATA / "interior_mission_spaces_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "INTERIOR_MISSION_SPACES_SLICE.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
interior_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnInteriorMissionSpacesForCity")

check_all(
    gamemode_h,
    [
        "SpawnInteriorMissionSpacesForCity",
    ],
    "game mode header must declare the interior mission spaces layer",
)

safehouse_idx = spawn_city_body.find("SpawnEnterableCivicSafehouse(Mission, CityIndex, Origin, CityLabel)")
interior_idx = spawn_city_body.find("SpawnInteriorMissionSpacesForCity(Mission, CityIndex, Origin, CityLabel)")
check(safehouse_idx >= 0, "campaign city spawn must still create the enterable civic safehouse")
check(interior_idx > safehouse_idx >= 0, "interior mission spaces must spawn after the civic safehouse")

check_all(
    interior_body,
    [
        "HOSPITAL TRIAGE CLINIC",
        "SCHOOL STUDY SHELTER",
        "CORNER STORE CACHE",
        "TRANSIT OPERATIONS HUB",
        "CIVIC RECORDS ANNEX",
        "InteriorMission_HospitalTriage",
        "InteriorMission_SchoolStudy",
        "InteriorMission_CornerStore",
        "InteriorMission_TransitOps",
        "InteriorMission_CivicRecords",
    ],
    "interior layer must implement the five requested mission-space archetypes",
)
check_all(
    interior_body,
    [
        "InteriorMissionSpace",
        "EnterableMissionInterior",
        "InteriorMissionSpaceReady",
        "HumanScaleBuildingProportion",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "InteriorMissionSupplyPickup",
        "APickupActor",
        "EPickupKind::Medkit",
        "EPickupKind::RadioScanner",
        "EPickupKind::AmmoPouch",
        "EPickupKind::FlashlightBattery",
        "EPickupKind::BypassKit",
        "APointLight",
        "SpawnTexturedBlock",
        "SpawnBlock",
        "SpawnGuideText",
        "[CodeRescueInteriorMissionSpaces]",
    ],
    "interior spaces must be tagged, lit, cook-safe, and gameplay-functional",
)
check_all(
    interior_body,
    [
        "Mission.CurriculumFocus",
        "Mission.SurvivorName",
        "Mission.LandmarkName",
        "Mission.CityName",
        "terminal -> survivor -> helipad",
        "clean-solve bonus disabled",
    ],
    "interior labels must connect curriculum, survivor, landmark, and route context",
)

check_all(
    manifest,
    [
        "HospitalTriageClinic",
        "SchoolStudyShelter",
        "CornerStoreCache",
        "TransitOperationsHub",
        "CivicRecordsAnnex",
        "SpawnInteriorMissionSpacesForCity",
        "EnterableMissionInterior",
        "InteriorMissionSupplyPickup",
    ],
    "interior mission spaces manifest must document every room and pickup contract",
)
check_all(
    creative_plan,
    [
        "interior mission spaces",
        "verify_interior_mission_spaces_slice_pass.py plus packaged render smoke plus visual review",
    ],
    "creative plan must move interior mission spaces from manual-only to verified implementation",
)
check_all(
    curriculum_manifest,
    [
        "InteriorMissionSpaces",
        "hospital, school, store, transit, and civic records interiors",
        "curriculum, survivor, landmark, and route context",
    ],
    "curriculum feedback manifest must document interior mission-space learning context",
)
check_all(
    onboarding,
    [
        "Enter interior mission spaces",
        "hospital, school, store, transit, and civic records",
    ],
    "onboarding must teach players to inspect interior mission spaces",
)
check_all(
    visual_manifest,
    [
        "InteriorMissionSpaces",
        "hospital, school, store, transit, and civic records interiors",
    ],
    "visual manifest must include the interior mission-space review target",
)
check_all(
    human_qa,
    [
        "InteriorMissionSpaces",
        "enter each open-front room",
        "functional pickup",
    ],
    "human QA checklist must include traversal and pickup checks for interiors",
)
check_all(
    accessibility_manifest,
    [
        "InteriorMissionSpaceAccessibility",
        "text-first labels",
        "open-front layouts",
    ],
    "accessibility manifest must document text-first open-front interiors",
)
check_all(
    full_qa + local_ci,
    [
        "verify_interior_mission_spaces_slice_pass.py",
    ],
    "QA scripts must run the interior mission spaces verifier",
)
check_all(
    progress,
    [
        "Interior mission spaces slice",
        "SpawnInteriorMissionSpacesForCity",
        "HOSPITAL TRIAGE CLINIC",
        "TRANSIT OPERATIONS HUB",
    ],
    "progress log must record the interior mission spaces slice",
)
check_all(
    slice_doc,
    [
        "Interior mission spaces slice",
        "SpawnInteriorMissionSpacesForCity",
        "Hospital Triage Clinic",
        "School Study Shelter",
        "Corner Store Cache",
        "Transit Operations Hub",
        "Civic Records Annex",
        "Validation",
    ],
    "slice documentation must explain implementation and validation",
)

if errors:
    for error in errors:
        print(f"[verify-interior-mission-spaces] BLOCKER: {error}", file=sys.stderr)
    raise SystemExit(1)

print("[verify-interior-mission-spaces] PASS")
