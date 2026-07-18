#!/usr/bin/env python3
"""
Static verifier for the June 12 next-100 improvement kickoff pass.

This locks in the first implemented items from the new roadmap:
- squad formation cycling on U,
- formation-aware regroup spacing,
- HUD discoverability for expanded Y/U/N/O squad controls,
- emergency auto-medkit resilience,
- documentation of the next 100 recommended improvements.
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
    roadmap_doc = DOC / "improvement_pass_2026-06-12/40_NEXT_100_IMPROVEMENT_ROADMAP_AND_IMPLEMENTATION.md"

    require(character_h, [
        "bAutoUseEmergencyMedkit",
        "EmergencyMedkitHealthFraction",
        "EmergencyMedkitCooldownSeconds",
        "GetSquadFormationLabel",
        "GetSquadFormationSpacingScale",
        "GetEmergencyMedkitReadySeconds",
        "CycleSquadFormation",
        "LastEmergencyMedkitWorldTime",
        "LastCriticalHealthCalloutWorldTime",
    ])
    require(character_cpp, [
        "EKeys::U",
        "CycleSquadFormation",
        "Squad formation: %s spacing applied",
        "ApplyFormationSpacingScale",
        "Emergency medkit deployed after %s hit",
        "Critical health after %s hit",
        "GetEmergencyMedkitReadySeconds() <= 0.0f",
    ])
    require(companion_h, [
        "FormationSpacingScale",
        "ApplyFormationSpacingScale",
        "BaseFollowOffset",
        "BaseLateralFollowOffset",
    ])
    require(companion_cpp, [
        "FormationSpacingScale = FMath::Clamp",
        "FollowOffset = BaseFollowOffset * FormationSpacingScale",
        "LateralFollowOffset = BaseLateralFollowOffset * FormationSpacingScale",
        "PersonalSpaceRadius = FMath::Clamp",
        "Movement->AvoidanceConsiderationRadius",
    ])
    require(hud_cpp, [
        "Y/U/N/O squad",
        "FORMATION %s",
        "U %s",
        "Auto medkit ready",
        "Auto medkit %.0fs",
    ])
    require(roadmap_doc, [
        "Next 100 Recommended Improvements",
        "Implemented in this pass",
        "1. Squad formation cycling",
        "100. External playtest package rubric",
    ])

    print("[verify-june12-next100-improvement-pass] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
