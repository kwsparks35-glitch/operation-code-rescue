#!/usr/bin/env python3
"""Static closure checks for the comprehensive audit implementation pass."""

from __future__ import annotations

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8", errors="ignore")


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def main() -> int:
    errors: list[str] = []
    character = read("Source/CodeRescueUnreal/CodeRescueCharacter.cpp")
    game_mode = read("Source/CodeRescueUnreal/CodeRescueGameMode.cpp")
    game_mode_h = read("Source/CodeRescueUnreal/CodeRescueGameMode.h")
    input_ini = read("Config/DefaultInput.ini")
    assets = read("Scripts/verify_character_world_assets.py")
    roster = read("Scripts/verify_camera_perspectives_and_character_roster.py")
    validator = read("Scripts/verify_curriculum_validator_shapes.py")
    scanner = read("Scripts/scan_audit_warnings.py")
    runtime_step = read("Scripts/verify_runtime_step_smoke_contracts.py")
    qa = read("Run_Full_QA_Audit.command")

    require("Objective remains unsolved" in character, "terminal failure path must keep objectives unsolved", errors)
    require("completed with the crash-safe fallback" not in character, "terminal fallback must not auto-complete objectives", errors)
    require("Terminal->MarkSolved();" not in character[character.find("CreateWidget<UCodeTerminalWidget>"): character.find("if (APickupActor* Pickup")], "terminal widget-failure branch must not call MarkSolved", errors)
    require("bEnabledLegacyMappingDeprecationWarnings=False" in input_ini, "legacy input deprecation warning noise should be disabled", errors)

    for forbidden_label in ("TEXT(\"DEBUG\")", "DEBUG FIELD LAB", "DEBUG COVER", "DEBUG LADDER", "VISUAL DEBUGGER"):
        require(forbidden_label not in game_mode, f"player-facing debug label remains: {forbidden_label}", errors)

    authored_start = game_mode.find("void ACodeRescueGameMode::SpawnAuthoredPropsForCity")
    authored_end = game_mode.find("void ACodeRescueGameMode::SpawnPerZonePostProcessVolume", authored_start)
    authored_body = game_mode[authored_start:authored_end]
    require("InspectableAuthoredMesh" in game_mode, "authored prop pass should tag mesh-backed props for inspection", errors)
    require("SpawnStaticMeshProp(" in authored_body, "authored prop pass should prefer static meshes", errors)
    require("imported/static-mesh props" in game_mode_h, "GameMode header should document mesh-backed prop behavior", errors)

    require("ZombieFemale_NurseOutfit" in assets, "character/world asset verifier should use current nurse mesh object", errors)
    require("load_asset" in assets and "OBJECT NAME MISMATCH" in assets, "character/world asset verifier should load and compare object names", errors)
    require("ZombieFemale_NurseOutfit" in roster, "camera/roster verifier should use current nurse mesh object", errors)
    require("load_asset" in roster and "asset object mismatch" in roster, "camera/roster verifier should load and compare object names", errors)
    require("running {case_index}/{total_cases}" in validator, "curriculum validator should print per-case progress", errors)

    require("BLOCKING_PATTERNS" in scanner and "ALLOWED_WARNING_FRAGMENTS" in scanner, "log warning scanner should define blocker and allowlist patterns", errors)
    require("add_movement_input" in runtime_step and "mark_solved" in runtime_step, "runtime step smoke should exercise movement and terminal contracts", errors)
    require("verify_graduated_campaign_world.py" in qa, "full QA command should run graduated campaign verifier", errors)
    require("verify_curriculum_validator_shapes.py" in qa, "full QA command should run curriculum validator", errors)
    require("scan_audit_warnings.py" in qa, "full QA command should run log scanner", errors)
    require("verify_runtime_step_smoke_contracts.py" in qa, "full QA command should run runtime step smoke contracts", errors)

    if errors:
        for error in errors:
            print(f"[cr-audit-closure] ERROR {error}", file=sys.stderr)
        print(f"[cr-audit-closure] failed with {len(errors)} error(s)", file=sys.stderr)
        return 1

    print("[cr-audit-closure] Success - audit implementation closure checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
