#!/usr/bin/env python3
"""
Static verifier for the May 27 public-demo Fab/detail world-polish pass.
"""

from __future__ import annotations

import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = PROJECT_ROOT.parent
MODE_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h"
MODE_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp"
MCP_SERVER = WORKSPACE_ROOT / "MCP_Server_Development/fab_unreal_macos_mcp/server.py"
MANIFEST = PROJECT_ROOT / "Content/CodeRescueData/public_demo_fab_detail_manifest.tsv"
PLAN = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-05-27/33_PUBLIC_DEMO_FAB_DETAIL_PASS.md"


def fail(message: str) -> None:
    raise RuntimeError(message)


def text(path: Path) -> str:
    if not path.exists():
        fail(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require_tokens(path: Path, tokens: list[str]) -> None:
    content = text(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        fail(f"{path} missing tokens: {', '.join(missing)}")


def verify_gameplay_layer() -> None:
    require_tokens(MODE_H, ["SpawnPublicDemoFabDetailLayer"])
    require_tokens(
        MODE_CPP,
        [
            "SpawnPublicDemoFabDetailLayer",
            "PublicDemoProductionDetail",
            "RetailQualityWorldPass",
            "CompetitivePricePresentation",
            "PlayableSetDressing",
            "LocalFabAssetIntegration",
            "FabDesignInclusion",
            "ModernBridgesFabAsset",
            "ParallaxNightBuildingUse",
            "FabShowcaseExpanded",
            "MissionRoomPolish",
            "SurvivorRoomPolish",
            "CombatCoverReadable",
            "SurvivalHorrorThreatForeshadow",
            "PremiumRouteAmmo",
            "PremiumSafeRoomMedkit",
            "[CodeRescuePublicDemoQuality]",
            "LoadCodeRescueBridgeMesh",
            "LoadCodeRescueCityBuildingMesh",
            "SpawnPremiumPickup",
            "AddSlowRotation",
        ],
    )
    mode_text = text(MODE_CPP)
    if mode_text.find("SpawnProductionTrackCompletionLayer(Mission") > mode_text.find("SpawnPublicDemoFabDetailLayer(Mission"):
        fail("public-demo detail layer should spawn after production completion layer")
    if mode_text.find("SpawnPublicDemoFabDetailLayer(Mission") > mode_text.find("EnsureEntryAccessCorridorClear(CityIndex"):
        fail("public-demo detail layer should spawn before entry corridor cleanup")


def verify_mcp_plan() -> None:
    require_tokens(
        MCP_SERVER,
        [
            'SERVER_VERSION = "0.4.0"',
            "PUBLIC_DEMO_FAB_DETAIL_TRACKS",
            "public_demo_fab_detail_plan",
            "tool_public_demo_fab_detail_plan",
            "unreal://project/current/public-demo-fab-detail-plan",
            "polish_public_demo_with_local_fab_assets",
            "Street-level Public Demo Composition",
            "ModernBridges Fab Hero Set Piece",
            "Mission-room and Survivor-room Dressing",
            "Local Fab/Design Coverage Gallery",
            "Threat Foreshadowing and Combat Readability",
        ],
    )
    if PLAN.exists():
        data = json.loads(PLAN.read_text(encoding="utf-8"))
        plan = data.get("public_demo_fab_detail_plan", [])
        if len(plan) < 5:
            fail("existing MCP asset plan is missing the public-demo Fab detail plan")
        plan_text = json.dumps(plan)
        for token in ("ModernBridges", "Parallax", "Mission-room", "Threat Foreshadowing"):
            if token not in plan_text:
                fail(f"public-demo Fab detail plan missing {token}")


def verify_manifest_and_docs() -> None:
    require_tokens(
        MANIFEST,
        [
            "Street-level Public Demo Composition",
            "ModernBridges Fab Hero Set Piece",
            "Mission-room Dressing",
            "Survivor-room Dressing",
            "Local Fab and Design Coverage Gallery",
            "Threat Foreshadowing and Combat Readability",
            "Useful Gear Visual Cue",
        ],
    )
    require_tokens(
        DOC,
        [
            "Public Demo Fab Detail Pass",
            "SpawnPublicDemoFabDetailLayer",
            "ModernBridges Fab hero overpass",
            "local Fab/design coverage gallery",
            "PUBLIC_DEMO_FAB_DETAIL_TRACKS",
            "public_demo_fab_detail_plan",
            "Honesty boundary",
        ],
    )


def main() -> int:
    verify_gameplay_layer()
    verify_mcp_plan()
    verify_manifest_and_docs()
    print("[verify-may27-public-demo-fab-detail] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
