#!/usr/bin/env python3
"""
Static verifier for the June 12 squad command/status continuation pass.

This locks the next implementation slice:
- compact squad health/status pips,
- N manual medic call,
- O squad hold/follow order,
- hold-aware companion behavior,
- roadmap and handoff documentation.
"""

from __future__ import annotations

from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC = PROJECT_ROOT / "Documentation"


def read(path: Path) -> str:
    if not path.exists():
        raise RuntimeError(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require(path: Path, tokens: list[str]) -> None:
    content = read(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def main() -> int:
    character_h = SRC / "CodeRescueCharacter.h"
    character_cpp = SRC / "CodeRescueCharacter.cpp"
    companion_h = SRC / "CompanionActor.h"
    companion_cpp = SRC / "CompanionActor.cpp"
    hud_cpp = SRC / "CodeRescueHUDWidget.cpp"
    audit = PROJECT_ROOT / "Run_Full_QA_Audit.command"
    roadmap_doc = DOC / "improvement_pass_2026-06-12/42_NEXT_100_SQUAD_COMMAND_STATUS_ROADMAP.md"

    require(character_h, [
        "GetSquadOrderLabel",
        "GetLastSquadOrderWorldTime",
        "GetLastManualMedicCallWorldTime",
        "CallSquadMedic",
        "ToggleSquadHoldPosition",
        "bSquadHoldPosition",
        "LastManualMedicCallWorldTime",
    ])
    require(character_cpp, [
        "EKeys::N",
        "EKeys::O",
        "CallSquadMedic",
        "ToggleSquadHoldPosition",
        "Medic called after %s hit",
        "Squad order: HOLD",
        "Squad order: FOLLOW",
        "SetHoldPosition",
        "ClearHoldPosition",
    ])
    require(companion_h, [
        "TryManualMedicPulse",
        "SetHoldPosition",
        "ClearHoldPosition",
        "IsHoldingPosition",
        "bHoldPosition",
        "HoldLocation",
    ])
    require(companion_cpp, [
        "Manual medic pulse applied",
        "Medic kit recharging %.0fs",
        "const FVector FormationTarget = bHoldPosition",
        "? HoldLocation",
        "Movement->StopMovementImmediately",
        "SetHoldPosition",
        "ClearHoldPosition",
    ])
    require(hud_cpp, [
        "Y/U/N/O squad",
        "HP %s",
        "N MEDIC",
        "MEDIC CALLED",
        "O %s",
        "ORDER %s",
        "GetSquadOrderLabel",
    ])
    require(audit, [
        "verify_june12_squad_command_status_pass.py",
    ])
    require(roadmap_doc, [
        "Next 100 Recommended Improvements",
        "1. Compact companion health/status pips",
        "2. Manual medic-call command",
        "3. Squad hold/follow command",
        "100. Release readiness gate",
    ])

    print("[verify-june12-squad-command-status-pass] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
