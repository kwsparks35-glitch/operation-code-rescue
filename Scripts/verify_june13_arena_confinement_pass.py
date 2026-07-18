#!/usr/bin/env python3
"""
Static verifier for the June 13 city arena confinement and fall-recovery pass.

This locks the playtest-safety contract:
- every streamed campaign city gets a blocking arena confinement layer,
- the layer includes a catch floor, four perimeter lock walls, visible boundary
  aesthetics, and a runtime log marker,
- entry/access cleanup must not disable the confinement collision,
- the player has automatic fall/out-of-bounds recovery plus Backspace/F8
  manual recovery and guidance.
"""

from __future__ import annotations

from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"


def read(path: Path) -> str:
    if not path.exists():
        raise RuntimeError(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require(path: Path, tokens: list[str]) -> None:
    content = read(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def extract_function_body(content: str, signature: str) -> str:
    start = content.find(signature)
    if start < 0:
        raise RuntimeError(f"missing function signature: {signature}")
    brace = content.find("{", start)
    if brace < 0:
        raise RuntimeError(f"missing function body: {signature}")
    depth = 0
    for index in range(brace, len(content)):
        char = content[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return content[brace + 1:index]
    raise RuntimeError(f"unterminated function body: {signature}")


def main() -> int:
    mode_h = SRC / "CodeRescueGameMode.h"
    mode_cpp = SRC / "CodeRescueGameMode.cpp"
    character_h = SRC / "CodeRescueCharacter.h"
    character_cpp = SRC / "CodeRescueCharacter.cpp"
    audit = PROJECT_ROOT / "Run_Full_QA_Audit.command"

    require(mode_h, [
        "SpawnGameplayArenaConfinementLayer",
    ])

    mode_content = read(mode_cpp)
    require(mode_cpp, [
        "void ACodeRescueGameMode::SpawnGameplayArenaConfinementLayer",
        "GameplayArenaConfinement",
        "CityGameplayBoundary",
        "CityBoundaryAesthetic",
        "FallRecoveryCatchFloor",
        "ArenaLockWall",
        "Gameplay Arena Fall Recovery Catch Floor",
        "%s Gameplay Arena %s Lock Wall",
        "SpawnWall(TEXT(\"North\")",
        "SpawnWall(TEXT(\"South\")",
        "SpawnWall(TEXT(\"West\")",
        "SpawnWall(TEXT(\"East\")",
        "Gameplay Arena Corner Rescue Beacon",
        "Gameplay Arena Boundary Skyline Facade",
        "CITY PERIMETER LOCKED",
        "[CodeRescueArenaConfinement]",
        "SpawnGameplayArenaConfinementLayer(Mission, CityIndex, Origin, CityLabel);",
        "Actor->Tags.Contains(FName(\"GameplayArenaConfinement\"))",
    ])

    floor = "CityLabel + TEXT(\" Mission Floor\")"
    call = "SpawnGameplayArenaConfinementLayer(Mission, CityIndex, Origin, CityLabel);"
    entry = "CityLabel + TEXT(\" Open Entry Pad\")"
    if not (mode_content.index(floor) < mode_content.index(call) < mode_content.index(entry)):
        raise RuntimeError("arena confinement should spawn after the mission floor and before the entry pad")

    confinement_body = extract_function_body(
        mode_content,
        "void ACodeRescueGameMode::SpawnGameplayArenaConfinementLayer",
    )
    if confinement_body.count("SpawnWall(TEXT(") < 4 or "CornerLocals" not in confinement_body:
        raise RuntimeError("expected blocking catch floor, four walls, and corner confinement actors")
    if "SetCollisionResponseToAllChannels(ECR_Block)" not in confinement_body:
        raise RuntimeError("confinement layer must force blocking collision")
    if "NoAccessBlocker" not in confinement_body:
        raise RuntimeError("visual-only boundary dressing should stay tagged as non-blocking")

    require(character_h, [
        "RecoverToCityArena",
        "UpdateArenaSafety",
        "LastSafeArenaLocation",
        "LastArenaSafetyRescueWorldTime",
    ])

    require(character_cpp, [
        "GCodeRescueArenaSafeGroundZ",  # 2026-07-11 refresh: soft-band pushback replaced by catch-floor + recovery contract
        "GCodeRescueArenaFallRecoveryZ",
        "void ACodeRescueCharacter::UpdateArenaSafety",
        "void ACodeRescueCharacter::RecoverToCityArena",
        "EKeys::BackSpace",
        "EKeys::F8",
        "Backspace/F8 unstuck",
        "City arena is locked",
        "SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics)",
        "Movement->StopMovementImmediately()",
        "SetMovementMode(MOVE_Walking)",
        "SavePersistentRun()",
        "[CodeRescueArenaRecovery]",
    ])

    require(audit, [
        "verify_june13_arena_confinement_pass.py",
    ])

    print("[verify-june13-arena-confinement-pass] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
