#!/usr/bin/env python3
"""
Standalone verification for the Fab/Unreal macOS MCP implementation.

Run from the project root:

    python3 Scripts/verify_fab_unreal_mcp_porting.py
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = PROJECT_ROOT.parent
SERVER = WORKSPACE_ROOT / "MCP_Server_Development/fab_unreal_macos_mcp/server.py"
PLAN = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"
WORLD_QUEUE = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_world_generation_queue.tsv"
UNREAL_VALIDATOR = PROJECT_ROOT / "Scripts/mcp_fab_unreal_import_validate.py"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-05-24/28_MACOS_FAB_UNREAL_MCP_ASSET_PORTING.md"


def fail(message: str) -> None:
    raise RuntimeError(message)


def read_json(path: Path) -> dict:
    if not path.exists():
        fail(f"missing {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def require_tokens(path: Path, tokens: list[str]) -> None:
    if not path.exists():
        fail(f"missing {path}")
    text = path.read_text(encoding="utf-8")
    missing = [token for token in tokens if token not in text]
    if missing:
        fail(f"{path} missing tokens: {', '.join(missing)}")


def run_server_self_test() -> dict:
    if not SERVER.exists():
        fail(f"missing server {SERVER}")
    completed = subprocess.run(
        [sys.executable, str(SERVER), "--self-test"],
        cwd=str(WORKSPACE_ROOT),
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        fail(f"server self-test failed: {completed.stderr}")
    return json.loads(completed.stdout)


def verify_uproject() -> None:
    uproject = read_json(PROJECT_ROOT / "CodeRescueUnreal.uproject")
    plugins = {plugin["Name"]: plugin.get("Enabled", False) for plugin in uproject.get("Plugins", [])}
    for required in ("PythonScriptPlugin", "EditorScriptingUtilities"):
        if not plugins.get(required):
            fail(f"{required} is not enabled in CodeRescueUnreal.uproject")


def verify_plan() -> None:
    plan = read_json(PLAN)
    if plan.get("generated_by") != "fab-unreal-macos-mcp":
        fail("asset plan generated_by mismatch")
    if plan.get("target", {}).get("platform") != "Mac":
        fail("asset plan target platform is not Mac")
    items = plan.get("items", [])
    if not items:
        fail("asset plan has no items")
    verdicts = {entry.get("analysis", {}).get("verdict") for entry in items}
    if not {
        "portable_after_retarget",
        "portable",
        "portable_after_rebuild",
        "not_portable",
        "manual_review_required",
    } & verdicts:
        fail("asset plan has no meaningful compatibility verdicts")
    present = [entry for entry in items if entry.get("project_presence", {}).get("present")]
    if not present:
        fail("asset plan did not detect any Fab-derived content already present in the project")
    matrix = plan.get("unreal_constituent_access_matrix", [])
    if len(matrix) < 7:
        fail("asset plan is missing the Unreal constituent access matrix")
    matrix_text = json.dumps(matrix)
    for token in (
        "MetaHuman Character Design",
        "MetaHuman for Maya and Houdini DCC Handoff",
        "Chaos Interactive and Async Physics",
        "AI for NPC",
        "Extended Standard Libraries",
        "Quest and Mission Kits",
    ):
        if token not in matrix_text:
            fail(f"Unreal constituent matrix missing {token}")
    character_world_plan = plan.get("unreal_character_world_development_plan", [])
    if len(character_world_plan) < 8:
        fail("asset plan is missing the Unreal character/world development plan")
    character_world_text = json.dumps(character_world_plan)
    for token in (
        "Novel MetaHuman-ready Playable and Survivor Cast",
        "Maya and Houdini DCC Intake",
        "Houdini and PCG World Design",
        "Chaos Interactive and Async Physics",
        "NPC AI and Encounter Director",
        "Continuous Playability QA",
    ):
        if token not in character_world_text:
            fail(f"Unreal character/world plan missing {token}")
    public_demo_plan = plan.get("public_demo_fab_detail_plan", [])
    if len(public_demo_plan) < 5:
        fail("asset plan is missing the public-demo Fab detail plan")
    public_demo_text = json.dumps(public_demo_plan)
    for token in (
        "Street-level Public Demo Composition",
        "ModernBridges Fab Hero Set Piece",
        "Mission-room and Survivor-room Dressing",
        "Local Fab/Design Coverage Gallery",
        "Threat Foreshadowing and Combat Readability",
    ):
        if token not in public_demo_text:
            fail(f"public-demo Fab detail plan missing {token}")


def verify_world_queue() -> None:
    if not WORLD_QUEUE.exists():
        fail(f"missing {WORLD_QUEUE}")
    lines = WORLD_QUEUE.read_text(encoding="utf-8").splitlines()
    if len(lines) < 2:
        fail("world-generation queue has no data rows")
    header = lines[0].split("\t")
    for required in ("title", "verdict", "world_role", "gameplay_use"):
        if required not in header:
            fail(f"world-generation queue missing column {required}")


def main() -> int:
    self_test = run_server_self_test()
    if self_test.get("status") != "ok":
        fail("server self-test did not return ok")
    if self_test.get("unreal_constituent_access_matrix_count", 0) < 7:
        fail("server self-test did not expose the Unreal constituent matrix")
    if self_test.get("unreal_character_world_development_plan_count", 0) < 8:
        fail("server self-test did not expose the Unreal character/world development plan")
    if self_test.get("public_demo_fab_detail_plan_count", 0) < 5:
        fail("server self-test did not expose the public-demo Fab detail plan")
    verify_uproject()
    verify_plan()
    verify_world_queue()
    require_tokens(
        UNREAL_VALIDATOR,
        ["AssetRegistryHelpers", "fab_unreal_mcp_asset_plan.json", "unreal_asset_validation_report.json"],
    )
    require_tokens(
        DOC,
        ["dry-run-first", "binary-only", "Fab/Vault", "UE 5.7", "world-generation queue"],
    )
    print("[verify-fab-unreal-mcp] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
