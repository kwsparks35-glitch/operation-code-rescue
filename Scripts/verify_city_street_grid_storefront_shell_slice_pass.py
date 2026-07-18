#!/usr/bin/env python3
"""Static verifier for the city street-grid and storefront-shell slice."""

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


gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/city_street_grid_storefront_shell_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "CITY_STREET_GRID_STOREFRONT_SHELL_SLICE.md")

urban_fn = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnMajorCityUrbanIdentityLayer")

check_all(
    urban_fn,
    [
        "StreetGridCityComposition",
        "CityStreetGridStorefrontShell",
        "ReadableCityStreetGrid",
        "NotOpenClutterField",
        "NoAccessBlocker",
        "WorldDevelopment",
    ],
    "urban layer must tag the city street grid/storefront shell surface",
)
check_all(
    urban_fn,
    [
        "TagCrosswalkShell",
        "StreetGridCrosswalkReadable",
        "HumanScaleCurbCrossing",
        "RouteClearStreetShell",
        "Street Grid Crosswalk Stripe",
        "CROSSWALK",
    ],
    "urban layer must spawn readable, nonblocking crosswalk and curb cues",
)
check_all(
    urban_fn,
    [
        "TagStorefrontShell",
        "StorefrontShellGroundFloor",
        "ModularStorefrontShell",
        "ParallaxStorefrontReady",
        "ImportedWorldAssetPromotionTarget",
        "Storefront Door Recess",
        "Storefront Ground Window",
        "Storefront Sign Band",
        "Storefront Awning",
        "STREET LEVEL",
    ],
    "urban layer must spawn tagged ground-floor storefront shell pieces",
)
check_all(
    urban_fn,
    [
        "CLINIC",
        "RADIO REPAIR",
        "MARKET",
        "PHARMACY",
        "TRANSIT INFO",
        "HARDWARE",
        "COMMUNITY KITCHEN",
        "SAFE ROUTE MAP",
    ],
    "storefront role labels must make street-level public uses readable",
)
check_all(
    urban_fn,
    [
        "Downtown Block",
        "Civic Core",
        "Transit Spine",
        "Medical District",
        "Survivor Search District",
        "CITY LANDSCAPE PASS",
        "CITY STREET GRID + STOREFRONT SHELL",
        "roads, sidewalks, crosswalks, dense facades, ground-floor shops",
    ],
    "district and review signage must expose the P0 city/world slice",
)
check_all(
    manifest,
    [
        "Street grid composition",
        "Readable crosswalks",
        "Ground-floor storefront shell",
        "District readability",
        "World promotion tags",
        "Review signage",
    ],
    "manifest must document city street/storefront surfaces",
)
check_all(
    plan,
    [
        "city street grid and storefront shell",
        "verify_city_street_grid_storefront_shell_slice_pass.py",
        "verify_character_world_assets.py",
        "manual city street/storefront review",
    ],
    "creative inclusion plan must route the P0 street/storefront row through this verifier",
)
check_all(
    qa,
    [
        "CityStreetGridStorefrontShell",
        "crosswalks",
        "shop doors",
        "shop windows",
        "store-role labels",
    ],
    "human QA checklist must include city street/storefront review",
)
check_all(
    visual,
    [
        "CityStreetGridStorefrontShell",
        "CITY STREET GRID + STOREFRONT SHELL",
        "crosswalk stripes",
        "shop doors",
        "awnings",
    ],
    "visual regression targets must include city street/storefront screenshots",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_city_street_grid_storefront_shell_slice_pass.py"],
    "local CI and full QA must run the city street/storefront verifier",
)
check_all(
    progress + doc,
    [
        "City street grid and storefront shell slice",
        "CityStreetGridStorefrontShell",
        "StorefrontShellGroundFloor",
        "verify_city_street_grid_storefront_shell_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_city_street_grid_storefront_shell_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_city_street_grid_storefront_shell_slice_pass] OK")
