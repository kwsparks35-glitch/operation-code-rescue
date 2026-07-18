#!/usr/bin/env python3
"""Static verifier for the regional city kit identity slice."""

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
manifest = read(DATA / "regional_city_kit_identity_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "REGIONAL_CITY_KIT_IDENTITY_SLICE.md")
world_doc = read(PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25/WORLD_DEVELOPMENT_DEEPDIVE.md")
top50_doc = read(PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25/TOP_50_RECOMMENDATIONS_2026-06-25.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
regional_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnRegionalCityKitIdentityLayer")

check_all(
    gamemode_h,
    ["SpawnRegionalCityKitIdentityLayer"],
    "game mode header must declare the regional city kit layer",
)

identity_idx = spawn_city_body.find("SpawnUSCitySpecificIdentityLayer(Mission, CityIndex, Origin, CityLabel)")
regional_idx = spawn_city_body.find("SpawnRegionalCityKitIdentityLayer(Mission, CityIndex, Origin, CityLabel)")
check(identity_idx >= 0, "campaign city spawn must still call SpawnUSCitySpecificIdentityLayer")
check(regional_idx > identity_idx >= 0, "regional kit layer must spawn after city identity layers")

check_all(
    regional_body,
    [
        "BuildUSCityVisualProfile",
        "BuildUSCityRealizationParams",
        "Mission.ArtKitName",
        "Mission.RegionName",
        "Mission.DistrictStyle",
        "Mission.LandmarkName",
        "Profile.DistrictCue",
        "Profile.SignatureCue",
        "Params.TerrainToken",
    ],
    "regional kit layer must be data-driven by mission and visual-profile data",
)
check_all(
    regional_body,
    [
        "REGIONAL KIT ENTRY GATE",
        "LANDMARK VISTA KIT",
        "OBJECTIVE DISTRICT KIT",
        "RegionalCityKitIdentity",
        "MajorCityRegionalKit",
        "RegionalKitReady",
        "LandmarkWayfindingKit",
        "DistrictLevelInstanceStandIn",
        "KitBibleRuntimeCue",
        "RegionalKitSignalLight",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "NoAccessBlocker",
        "APointLight",
        "SpawnBlock",
        "SpawnGuideText",
        "[CodeRescueRegionalCityKits]",
    ],
    "regional kit layer must create tagged anchors, lights, labels, and a runtime marker",
)
check_all(
    regional_body,
    [
        "Coastal Port",
        "Desert Solar Grid",
        "Mountain Relay",
        "Great Lakes Industrial",
        "Capital Command",
        "Rail Yard",
        "Metro Facade Module",
    ],
    "regional kit layer must cover major kit families and a metro fallback",
)

check_all(
    manifest,
    [
        "RegionalCityKitIdentity",
        "RegionalKitEntryGate",
        "LandmarkVistaKit",
        "ObjectiveDistrictKit",
        "RegionalMotifStandIn",
        "SpawnRegionalCityKitIdentityLayer",
    ],
    "regional city kit manifest must document anchors and motif stand-ins",
)
check_all(
    creative_plan,
    [
        "major city regional kits",
        "verify_regional_city_kit_identity_slice_pass.py plus verify_production_track_completion.py plus packaged render smoke plus visual review",
    ],
    "creative plan must route major city regional kits through the new verifier",
)
check_all(
    visual_manifest,
    [
        "RegionalCityKits",
        "regional kit entry gate, landmark vista kit, and objective district kit",
    ],
    "visual regression targets must include the regional city kit review target",
)
check_all(
    human_qa,
    [
        "RegionalCityKits",
        "regional kit entry gate",
        "landmark vista kit",
        "objective district kit",
    ],
    "human QA checklist must include the regional city kit walkthrough",
)
check_all(
    accessibility_manifest,
    [
        "RegionalCityKitAccessibility",
        "text-first kit labels",
        "nonblocking regional kit anchors",
    ],
    "accessibility manifest must document regional kit cues without color dependency",
)
check_all(
    full_qa + local_ci,
    ["verify_regional_city_kit_identity_slice_pass.py"],
    "QA scripts must run the regional city kit identity verifier",
)
check_all(
    progress,
    [
        "Regional city kit identity slice",
        "SpawnRegionalCityKitIdentityLayer",
        "REGIONAL KIT ENTRY GATE",
        "LANDMARK VISTA KIT",
    ],
    "progress log must record the regional city kit identity slice",
)
check_all(
    slice_doc,
    [
        "Regional city kit identity slice",
        "SpawnRegionalCityKitIdentityLayer",
        "Regional Kit Entry Gate",
        "Landmark Vista Kit",
        "Objective District Kit",
        "Validation",
    ],
    "slice documentation must explain implementation and validation",
)
check_all(
    world_doc + top50_doc,
    [
        "modular kit",
        "Per-city identity as data",
        "Landmarks & wayfinding",
    ],
    "June 25 source guidance must remain available for this slice",
)

if errors:
    for error in errors:
        print(f"[verify-regional-city-kit-identity] BLOCKER: {error}", file=sys.stderr)
    raise SystemExit(1)

print("[verify-regional-city-kit-identity] PASS")
