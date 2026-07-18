#!/usr/bin/env python3
"""
Static verifier for the June rescue-team, survivability, and access polish.

The goal is to keep the current player-facing promises from regressing:
- the player has a visible, resilient health model,
- hit direction/source data reaches the HUD,
- the rescue team spawns as a five-role squad,
- companions do not physically block the player,
- the HUD reports squad status,
- access cleanup freezes simulated props before disabling collision.
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
    hud_h = SRC / "CodeRescueHUDWidget.h"
    hud_cpp = SRC / "CodeRescueHUDWidget.cpp"
    mode_h = SRC / "CodeRescueGameMode.h"
    mode_cpp = SRC / "CodeRescueGameMode.cpp"
    save_h = SRC / "CodeRescueSaveGame.h"
    gi_cpp = SRC / "CodeRescueGameInstance.cpp"
    campaign_cpp = SRC / "CodeRescueCampaign.cpp"
    qa_checklist = DOC / "QA_PLAYTEST_CHECKLIST.md"
    pass_doc = DOC / "improvement_pass_2026-06-01/37_RESCUE_TEAM_SURVIVABILITY_PACKAGE_PASS.md"

    require(character_h, [
        "float Health = 250.0f",
        "float MaxHealth = 250.0f",
        "int32 Medkits = 10",
        "int32 MaxMedkits = 16",
        "int32 ArmorPlates = 4",
        "int32 MaxArmorPlates = 6",
        "ArmorDamageReduction = 0.55f",
        "DamageMercyWindowSeconds = 0.85f",
        "MaxEnemyDamagePerHitFraction = 0.16f",
        "bPreventSingleHitEnemyDeaths = true",
        "GetLastDamageLocationText",
        "GetLastDamageSourceText",
        "GetLastDamageAmount",
        "GetLastDamageSourceDistanceMeters",
        "GetLastSquadRegroupWorldTime",
        "GetLastSquadRegroupCount",
        "LastDamageLocationText",
        "RegroupRescueTeam",
    ])
    require(character_cpp, [
        "#include \"CompanionActor.h\"",
        "DescribeAttackerDirection",
        "DescribeDamageSource",
        "EKeys::Y",
        "RegroupRescueTeam",
        "TActorIterator<ACompanionActor>",
        "RegroupNearPlayer",
        "Squad regrouped: %d",
        "DamageAmount * 0.25f",
        "MaxHealth * MaxEnemyDamagePerHitFraction",
        "MaxHealth * 0.14f",
        "LastDamageLocationText = DescribeAttackerDirection",
        "LastDamageSourceDistanceMeters",
        "Damage taken from %s",
    ])
    require(companion_h, [
        "PersonalSpaceRadius",
        "IsOperational",
        "GetMedicPulseReadySeconds",
        "RegroupNearPlayer",
        "bMedicSupport",
        "RoleLabel",
    ])
    require(companion_cpp, [
        "SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore)",
        "SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore)",
        "bUseRVOAvoidance = true",
        "AvoidanceConsiderationRadius",
        "PersonalSpaceRadius",
        "StopMovementImmediately",
        "ETeleportType::TeleportPhysics",
        "TrySupportPlayer",
        "Medic pulse applied",
    ])
    require(hud_h, ["SquadStatusText", "DamageAlertText", "HealthLabelText"])
    require(hud_cpp, [
        "SquadStatusText = WidgetTree->ConstructWidget<UTextBlock>",
        "RESCUE TEAM  %d ACTIVE",
        "SUPPORT FIRE ONLINE",
        "MEDIC %s READY",
        "Y REGROUP",
        "REGROUPED %d",
        "ATTACKED FROM %s",
        "PLAYER HEALTH  %.0f / %.0f",
        "TActorIterator<ACompanionActor>",
    ])
    require(mode_h, ["SpawnRescueSupportTeamForCity", "EnsureEntryAccessCorridorClear"])
    require(mode_cpp, [
        "SpawnRescueSupportTeamForCity",
        "Mira Hale",
        "Tomas Ives",
        "Ada Cross",
        "Noor Vance",
        "Briggs Vale",
        "Medic",
        "Engineer",
        "Rifle Support",
        "Scout",
        "Heavy Rescue",
        "safehouse",
        "launch language marker",
        "terminal",
        "helipad",
        "SetSimulatePhysics(false)",
        "froze %d physics components",
    ])
    require(save_h, ["bHasCompanion"])
    require(gi_cpp, ["Save->bHasCompanion = bHasCompanion", "bHasCompanion = Save->bHasCompanion"])
    require(campaign_cpp, ["launch track note"])
    require(qa_checklist, [
        "five-member rescue support squad",
        "attack alert states the hit direction",
        "entry, armory, safehouse, launch language marker, terminal, survivor, and helipad access points",
        "full weapon roster",
    ])
    require(pass_doc, [
        "Rescue Team, Survivability, Access Cleanup, and Fresh Package Pass",
        "directional attack alert",
        "five-member rescue support squad",
        "Regressions Found and Fixed",
    ])

    print("[verify-june01-rescue-survivability] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
