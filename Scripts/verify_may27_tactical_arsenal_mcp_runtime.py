#!/usr/bin/env python3
"""
Static verifier for the May 27 tactical arsenal, runtime-timeout, armory,
and Unreal constituent MCP pass.
"""

from __future__ import annotations

import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = PROJECT_ROOT.parent
TYPES_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueTypes.h"
CHAR_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.h"
CHAR_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.cpp"
HUD_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp"
MODE_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h"
MODE_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp"
RUNNER_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRunnerLibrary.cpp"
MCP_SERVER = WORKSPACE_ROOT / "MCP_Server_Development/fab_unreal_macos_mcp/server.py"
PLAN = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-05-27/31_TACTICAL_ARSENAL_MCP_RUNTIME_PASS.md"


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


def verify_weapon_system() -> None:
    require_tokens(
        TYPES_H,
        [
            "CombatKnife",
            "HeavyHandgun",
            "BurstHandgun",
            "TacticalShotgun",
            "AutoShotgun",
            "SMG",
            "PrecisionRifle",
            "SemiAutoRifle",
            "Magnum",
            "BoltLauncher",
            "RocketLauncher",
            "IncendiaryGrenade",
            "FlashGrenade",
            "TacticalRole",
            "StartingReserveAmmo",
            "MaxReserveAmmo",
            "BurstCount",
            "PierceCount",
        ],
    )
    require_tokens(
        CHAR_H,
        [
            "WeaponReserveAmmo",
            "CycleWeaponNext",
            "CycleWeaponPrevious",
            "GetActiveWeaponReserveAmmo",
            "ApplyAreaWeaponEffect",
        ],
    )
    require_tokens(
        CHAR_CPP,
        [
            "GCodeRescueDefaultWeaponCount",
            "Balanced Handgun",
            "Rocket Launcher",
            "Flash Grenade",
            "MouseScrollDown",
            "MouseScrollUp",
            "Gamepad_LeftShoulder",
            "ApplyAreaWeaponEffect",
            "ExplosionRadius",
            "PierceCount",
            "GetActiveWeaponReserveAmmo",
            "RefreshLegacyAmmoFromWeaponReserves",
        ],
    )
    require_tokens(
        HUD_CPP,
        [
            "Ammo Pool",
            "Active Reserve",
            "Wheel/[ ] all %d",
            "Role:",
            "Weapons %d",
        ],
    )


def verify_armory_world_layer() -> None:
    require_tokens(MODE_H, ["SpawnTacticalArmoryLayer"])
    require_tokens(
        MODE_CPP,
        [
            "SpawnTacticalArmoryLayer",
            "TacticalArmoryAllWeaponsAvailable",
            "SurvivalHorrorWeaponArchetype",
            "ImmediateGearSelection",
            "TACTICAL ARMORY",
            "all weapons unlocked",
            "Armory Ammo Crate",
            "Armory Medical Case",
        ],
    )
    mode_text = text(MODE_CPP)
    if mode_text.find("SpawnUniversalEntryAccessLayer(Mission") > mode_text.find("SpawnTacticalArmoryLayer(Mission"):
        fail("tactical armory should spawn after universal entry access")


def verify_runtime_timeout() -> None:
    require_tokens(
        RUNNER_CPP,
        [
            "GCodeValidationProcessTimeoutSeconds",
            "CreateProc",
            "IsProcRunning",
            "TerminateProc",
            "Code validation timed out",
            "WaitForProc",
            "MATLAB batch mode timed out locally",
            "ValidateInEngine",
        ],
    )


def verify_mcp_matrix() -> None:
    require_tokens(
        MCP_SERVER,
        [
            "SERVER_VERSION = \"0.4.0\"",
            "UNREAL_CONSTITUENT_CAPABILITIES",
            "unreal_constituent_access_matrix",
            "MetaHuman Character Design",
            "MetaHuman for Maya and Houdini DCC Handoff",
            "Chaos Interactive and Async Physics",
            "AI for NPC and Enemy Characters",
            "Extended Standard Libraries and Plugins",
            "Quest and Mission Kits",
            "unreal_constituent_matrix",
        ],
    )
    if PLAN.exists():
        plan = json.loads(PLAN.read_text(encoding="utf-8"))
        matrix = plan.get("unreal_constituent_access_matrix", [])
        if matrix and len(matrix) < 7:
            fail("existing Fab MCP plan has a partial constituent matrix")


def verify_docs() -> None:
    require_tokens(
        DOC,
        [
            "May 27 Tactical Arsenal",
            "all weapons are immediately available",
            "per-weapon reserve ammo",
            "runtime validator timeout",
            "Unreal constituent access matrix",
            "Mac demo app rebuild",
        ],
    )


def main() -> int:
    verify_weapon_system()
    verify_armory_world_layer()
    verify_runtime_timeout()
    verify_mcp_matrix()
    verify_docs()
    print("[verify-may27-tactical-arsenal-mcp-runtime] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
