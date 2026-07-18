#!/usr/bin/env python3
"""Static verifier for the human-scale route access slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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


game_mode_h = read(SRC / "CodeRescueGameMode.h")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "human_scale_route_access_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
world_contract = read(DATA / "world_promotion_validation_contract.tsv")
runtime_logs = read(PROJECT_ROOT / "Scripts/verify_runtime_log_contracts.py")
profile_layers = read(PROJECT_ROOT / "Scripts/profile_city_layers_static.py")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "HUMAN_SCALE_ROUTE_ACCESS_SLICE.md")
world_doc = read(SOURCE_DOC / "WORLD_DEVELOPMENT_DEEPDIVE.md")
physics_doc = read(SOURCE_DOC / "GAME_PHYSICS_DEEPDIVE.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_human_scale_route_access_slice_pass.py")


check_all(
    game_mode_h,
    [
        "SpawnUniversalEntryAccessLayer",
        "SpawnPurposeClarityLayer",
        "EnsureEntryAccessCorridorClear",
    ],
    "game mode header must expose route access and architecture clarity hooks",
)

check_all(
    game_mode_cpp,
    [
        "void ACodeRescueGameMode::EnsureEntryAccessCorridorClear",
        "FAccessClearanceZone",
        "entry",
        "armory",
        "safehouse",
        "launch language marker",
        "terminal",
        "survivor",
        "helipad",
        "SetSimulatePhysics(false)",
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetCollisionResponseToAllChannels(ECR_Ignore)",
        "UniversalEntryAccess",
        "EntryCorridorCollisionCleared",
        "[CodeRescueEntryAccess]",
        "void ACodeRescueGameMode::SpawnPurposeClarityLayer",
        "NAVIGATION LEGEND",
        "OBJECTIVE 2\\nPROTECTED CODING SAFEHOUSE",
        "OBJECTIVE 4\\nEXTRACTION",
        "PurposeCodedArchitecture",
        "ArchitectureClarityPass",
        "CriticalPathNonBlockingArchitecture",
        "DistancePointToSegment2D",
        "[CodeRescueArchitectureClarity]",
        "SpawnPurposeClarityLayer(Mission, CityIndex, Origin, CityLabel);",
        "EnsureEntryAccessCorridorClear(CityIndex, Origin, CityLabel);",
    ],
    "game mode implementation must clear access zones and label the critical route",
)

check_all(
    manifest,
    [
        "Entry access clearance",
        "Purpose-coded route labels",
        "Critical path nonblocking cleanup",
        "Objective route beacon",
        "World promotion gate",
        "Runtime log contract",
        "UniversalEntryAccess",
        "CriticalPathNonBlockingArchitecture",
        "verify_human_scale_route_access_slice_pass.py",
    ],
    "human-scale route access manifest must document runtime surfaces and validation",
)

check_all(
    creative_plan,
    [
        "human-scale doors windows stairs and cover",
        "verify_human_scale_route_access_slice_pass.py",
        "verify_june19_playability_readability_fix_pass.py",
        "verify_world_promotion_validation_contract_pass.py",
        "packaged render smoke",
    ],
    "creative plan must route the P0 human-scale row through the new verifier",
)

check_all(
    human_qa,
    [
        "Route Access",
        "entry, armory, safehouse, language marker, terminal, survivor, and helipad",
        "nonblocking",
    ],
    "human QA checklist must include route access review",
)

check_all(
    visual_targets,
    [
        "HumanScaleRouteAccess",
        "entry, armory, safehouse, terminal, survivor, and helipad route pads",
        "nonblocking critical route",
    ],
    "visual regression targets must include human-scale route access",
)

check_all(
    world_contract,
    [
        "Human-scale collision and accessibility",
        "doors, stairs, cover",
        "walkable collision",
        "AI navigation clarity",
        "recovery-route accessibility",
        "verify_world_promotion_validation_unreal.py",
    ],
    "world promotion contract must gate future imported route geometry",
)

check_all(
    runtime_logs,
    [
        "[CodeRescueEntryAccess]",
        # 2026-07-17 pin migration: pass-6 control cleanup made Backspace the single
        # recovery key (F8 removed); the guidance marker text changed with it.
        "Backspace recovery guidance",
    ],
    "runtime log contracts must include access and recovery evidence",
)

check_all(
    profile_layers,
    [
        "CoreRoute",
        "EnsureEntryAccessCorridorClear",
        "EntryAccess",
    ],
    "static layer profiler must include the entry access route",
)

check("verify_human_scale_route_access_slice_pass.py" in full_qa,
      "full QA must run the human-scale route access verifier")
check("verify_human_scale_route_access_slice_pass.py" in local_ci,
      "local CI must run the human-scale route access verifier")

check_all(
    slice_doc,
    [
        "Human-Scale Route Access Slice",
        "Runtime Coverage",
        "Source Guidance",
        "Boundaries",
        "Validation",
        "EnsureEntryAccessCorridorClear",
        "SpawnPurposeClarityLayer",
    ],
    "slice documentation must explain runtime coverage, boundaries, and validation",
)

check_all(
    world_doc,
    [
        "terminal \"safe rooms\"",
        "clear **paths**",
        "designed critical path",
        "cover density and escape routes",
    ],
    "world-development source guidance must include safe rooms, paths, and route readability",
)

check_all(
    physics_doc,
    [
        "Static city geometry",
        "simple collision",
        "Cover",
        "Barricades",
    ],
    "physics source guidance must include collision and cover discipline",
)

check_all(
    progress,
    [
        "Human-scale route access slice",
        "human_scale_route_access_manifest.tsv",
        "verify_human_scale_route_access_slice_pass.py",
    ],
    "progress log must record the human-scale route access slice",
)

check("verify_human_scale_route_access_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_human_scale_route_access_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_human_scale_route_access_slice_pass] PASS: human-scale route access verified")
