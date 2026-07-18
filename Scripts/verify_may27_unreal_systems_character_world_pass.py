#!/usr/bin/env python3
"""
Static verifier for the May 27 Unreal systems, character design, world design,
MCP development-plan, and playability-hook pass.
"""

from __future__ import annotations

import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = PROJECT_ROOT.parent
MODE_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h"
MODE_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp"
MCP_SERVER = WORKSPACE_ROOT / "MCP_Server_Development/fab_unreal_macos_mcp/server.py"
SYSTEMS_MANIFEST = PROJECT_ROOT / "Content/CodeRescueData/unreal_systems_character_world_manifest.tsv"
DESIGN_MANIFEST = PROJECT_ROOT / "Content/CodeRescueData/novel_character_world_design_manifest.tsv"
PLAN = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-05-27/32_UNREAL_SYSTEMS_CHARACTER_WORLD_PASS.md"


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
    require_tokens(MODE_H, ["SpawnUnrealSystemsCharacterWorldLayer"])
    require_tokens(
        MODE_CPP,
        [
            "SpawnUnrealSystemsCharacterWorldLayer",
            "UnrealSystemsCharacterWorld",
            "FullNovelCharacterWorldDesign",
            "ContinuousPlayabilityTestRig",
            "MetaHumanReadyCharacterDesign",
            "MayaHoudiniDccHandoff",
            "HoudiniProceduralWorldDesign",
            "PCGWorldPartitionCell",
            "WorldPartitionReady",
            "PCGRouteSplineReady",
            "ChaosInteractivePhysics",
            "AsyncPhysicsReady",
            "ProjectilePhysicsTarget",
            "NPCBehaviorTreeReady",
            "StateTreeEQSReady",
            "AIPatrolRouteNode",
            "EnemyEncounterDirector",
            "QuestMissionKitReady",
            "MissionObjectiveKit",
            "SequencerControlRigIKGroomReady",
            "ControlRigFacialSlot",
            "IKRetargeterReady",
            "GroomCardFallbackReady",
            "SetSimulatePhysics(true)",
            "[CodeRescueUnrealSystems]",
            "AFriendlyNPCActor",
            "Rhea Calder",
            "Mika Stone",
            "Noor Vance",
            "Jules Ardent",
            "Ilan Cross",
        ],
    )
    mode_text = text(MODE_CPP)
    if mode_text.find("SpawnTacticalArmoryLayer(Mission") > mode_text.find("SpawnUnrealSystemsCharacterWorldLayer(Mission"):
        fail("Unreal systems layer should spawn after tactical armory setup")
    if mode_text.find("SpawnUnrealSystemsCharacterWorldLayer(Mission") > mode_text.find("SpawnCinematicStreetLifeLayer(Mission"):
        fail("Unreal systems layer should be in the core city layer stack before street-life dressing")


def verify_mcp_development_plan() -> None:
    require_tokens(
        MCP_SERVER,
        [
            'SERVER_VERSION = "0.4.0"',
            "UNREAL_CHARACTER_WORLD_DEVELOPMENT_TRACKS",
            "unreal_character_world_development_plan",
            "tool_unreal_character_world_development_plan",
            "unreal://project/current/character-world-development-plan",
            "develop_full_character_world_pipeline",
            "Novel MetaHuman-ready Playable and Survivor Cast",
            "Maya and Houdini DCC Intake",
            "Houdini and PCG World Design",
            "Chaos Interactive and Async Physics",
            "NPC AI and Encounter Director",
            "Sequencer, Control Rig, IK Retargeter, Groom",
        ],
    )
    if PLAN.exists():
        data = json.loads(PLAN.read_text(encoding="utf-8"))
        plan = data.get("unreal_character_world_development_plan", [])
        if plan and len(plan) < 8:
            fail("existing MCP asset plan has a partial character/world development plan")


def verify_manifests_and_docs() -> None:
    require_tokens(
        SYSTEMS_MANIFEST,
        [
            "MetaHuman Character Design",
            "Maya and Houdini DCC Handoff",
            "Houdini and PCG World Design",
            "Chaos Interactive and Async Physics",
            "NPC AI and Encounter Director",
            "Quest and Mission Kits",
            "Sequencer Control Rig IK Groom",
            "Continuous Playability QA",
        ],
    )
    require_tokens(
        DESIGN_MANIFEST,
        [
            "Rhea Calder",
            "Mika Stone",
            "Noor Vance",
            "Jules Ardent",
            "Ilan Cross",
            "The Redline Warden",
            "The Glass Ward",
            "The Broken Grid",
        ],
    )
    require_tokens(
        DOC,
        [
            "Unreal Systems Character and World Pass",
            "SpawnUnrealSystemsCharacterWorldLayer",
            "MetaHuman-ready",
            "Maya/Houdini",
            "Chaos/async-physics-ready",
            "unreal_character_world_development_plan",
            "Honesty boundary",
        ],
    )


def main() -> int:
    verify_gameplay_layer()
    verify_mcp_development_plan()
    verify_manifests_and_docs()
    print("[verify-may27-unreal-systems-character-world] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
