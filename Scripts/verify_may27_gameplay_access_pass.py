#!/usr/bin/env python3
"""
Static verifier for the May 27 gameplay/accessibility rebuild.

Checks the user-facing fixes from this pass:
- enemies move/facetarget directly toward the player instead of pathing sideways,
- camera view switching has bound and polled fallbacks,
- exterior city gate/rail barriers and global outside floor are gone,
- generated buildings use compact human-scale proportions.
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


def forbid(path: Path, tokens: list[str]) -> None:
    content = read(path)
    present = [token for token in tokens if token in content]
    if present:
        raise RuntimeError(f"{path} still contains forbidden tokens: {', '.join(present)}")


def main() -> int:
    ai = SRC / "CodeRescueAIController.cpp"
    zombie = SRC / "CodeZombieActor.cpp"
    character = SRC / "CodeRescueCharacter.cpp"
    character_h = SRC / "CodeRescueCharacter.h"
    hud = SRC / "CodeRescueHUDWidget.cpp"
    game_mode = SRC / "CodeRescueGameMode.cpp"
    campaign = SRC / "CodeRescueCampaign.cpp"

    require(ai, [
        "CurrentState = EZombieAIState::Chase;",
        "StandardPursuitMoveDirectlyTowardPlayer",
        "MoveDirectlyToward(ZombieCharacter->ResolveEncounterMoveTarget(TargetLocation), ZombieCharacter->AttackRange * 0.8f)",
        "MoveDirectlyToward(ZombieCharacter->ResolveEncounterMoveTarget(PlayerCharacter->GetActorLocation()), ZombieCharacter->AttackRange * 0.7f)",
        "ZombieCharacter->SetActorRotation(Direction.Rotation())",
    ])
    forbid(ai, ["TryMoveToActorWithFallback(PlayerCharacter"])

    require(zombie, [
        "RefreshMovementSettings()",
        "Movement->bOrientRotationToMovement = true",
        "FRotator(0.0f, -90.0f, 0.0f)",
        "FaceMovementTarget(PlayerPawn->GetActorLocation(), DeltaSeconds)",
    ])

    require(character_h, [
        "void CycleCameraPerspective()",
        "UCameraComponent* GetActiveGameplayCamera() const",
        "LastCameraInputWorldTime",
    ])
    require(character, [
        "EKeys::V",
        "EKeys::Gamepad_RightShoulder",
        "PC->WasInputKeyJustPressed(EKeys::C)",
        "GetActiveGameplayCamera()",
        "PC->SetViewTargetWithBlend(this, 0.05f)",
    ])
    require(hud, ["Character->GetActiveGameplayCamera()"])

    require(game_mode, [
        "constexpr float FootprintScale = 4.8f",
        "constexpr float HeightScale = 5.7f",
        "AlwaysOpenLevelEntry",
        "NoExteriorWallBarrier",
        "Universal Entry Open Route Stripe",
    ])
    forbid(game_mode, [
        "North Gate Rail",
        "South Gate Rail",
        "West Gate Rail",
        "East Gate Rail",
        "First View Rescue Gate Header",
        "Universal Entry Open Header",
        "National Campaign Safety Ground",
    ])
    require(campaign, ["no enclosing exterior wall"])

    print("[verify-may27-gameplay-access] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
