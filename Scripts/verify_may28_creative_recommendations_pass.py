#!/usr/bin/env python3
"""
Static verifier for the May 28 creative recommendations implementation pass.

This verifies that the broad recommendation plan has moved into gameplay
surfaces, functional tactical pickups, asset-intake tracking, and review docs.
"""

from __future__ import annotations

from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-05-28/36_CREATIVE_RECOMMENDATIONS_IMPLEMENTATION_PASS.md"


def read(path: Path) -> str:
    if not path.exists():
        raise RuntimeError(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require(path: Path, tokens: list[str]) -> None:
    content = read(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def verify_tsv(path: Path, min_rows: int, required_tokens: list[str]) -> None:
    content = read(path)
    rows = [line for line in content.splitlines() if line.strip()]
    if len(rows) - 1 < min_rows:
        raise RuntimeError(f"{path} has {len(rows) - 1} rows; expected at least {min_rows}")
    missing = [token for token in required_tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def main() -> int:
    mode_h = SRC / "CodeRescueGameMode.h"
    mode_cpp = SRC / "CodeRescueGameMode.cpp"
    pickup_h = SRC / "PickupActor.h"
    pickup_cpp = SRC / "PickupActor.cpp"
    character_h = SRC / "CodeRescueCharacter.h"
    character_cpp = SRC / "CodeRescueCharacter.cpp"
    hud_cpp = SRC / "CodeRescueHUDWidget.cpp"
    scanner = PROJECT_ROOT / "Scripts/scan_may28_active_asset_downloads.py"
    intake_tsv = DATA / "active_download_asset_intake_2026_05_28.tsv"
    implementation_tsv = DATA / "creative_recommendations_implementation_manifest.tsv"

    require(mode_h, ["SpawnCreativeRecommendationImplementationLayer"])
    require(
        mode_cpp,
        [
            "SpawnCreativeRecommendationImplementationLayer",
            "May28CreativeRecommendationImplementation",
            "FullRecommendationImplementationPass",
            "DownloadedAssetIntakeReview",
            "ActiveFabDownloadStaging",
            "MetaHumanMhpkgStaged",
            "LicenseSafePromotionGate",
            "PlayableRescueOperatorReady",
            "MetaHumanReplacementSlot",
            "CurriculumRoomConceptArt",
            "SurvivorIntelRewardChain",
            "TacticalGearFunctionalPickup",
            "EPickupKind::Flare",
            "EPickupKind::Smoke",
            "EPickupKind::Stim",
            "EPickupKind::Scrap",
            "EPickupKind::ArmorPlate",
            "MajorCityDistrictKit",
            "InteriorMissionSpaceReady",
            "HumanScaleBuildingProportion",
            "AIEncounterDirectorRuntimeHook",
            "AsyncPhysicsPromotionGate",
            "SetSimulatePhysics(true)",
            "[CodeRescueCreativeImplementation]",
        ],
    )
    require(
        pickup_h,
        [
            "Flare   UMETA",
            "Smoke   UMETA",
            "Stim    UMETA",
            "Scrap   UMETA",
            "ArmorPlate UMETA",
        ],
    )
    require(
        pickup_cpp,
        [
            "PickupKindLabel",
            "PickupKindColor",
            "Character->AddFlares",
            "Character->AddSmokes",
            "Character->AddStims",
            "Character->GrantScrap",
            "Character->AddArmorPlates",
        ],
    )
    require(
        character_h,
        [
            "ArmorPlates",
            "MaxArmorPlates",
            "ArmorDamageReduction",
            "AddFlares",
            "AddSmokes",
            "AddStims",
            "AddArmorPlates",
            "GetArmorPlates",
            "AddBoundedResource",
        ],
    )
    require(
        character_cpp,
        [
            "ArmorDamageReduction = FMath::Clamp",
            "int32 ACodeRescueCharacter::AddFlares",
            "int32 ACodeRescueCharacter::AddSmokes",
            "int32 ACodeRescueCharacter::AddStims",
            "int32 ACodeRescueCharacter::AddArmorPlates",
            "bArmorPlateAbsorbed",
            "MitigationNotes.Add(TEXT(\"armor plate\"))",
        ],
    )
    require(hud_cpp, ["Armor: %d / %d", "Armor: %d/%d", "Armor %d/%d"])
    require(
        scanner,
        [
            "FAB_ROOT",
            "METAHUMAN_ROOT",
            "active_download_asset_intake_2026_05_28.tsv",
            "manual_mhpkg_or_dcc_materialization_required",
            "plugin_source_mac_review_required",
        ],
    )
    verify_tsv(
        intake_tsv,
        20,
        [
            "MetaHuman",
            "ASYNC",
            "Quest",
            "Zombie",
            "Modern",
            "Project Content",
        ],
    )
    verify_tsv(
        implementation_tsv,
        20,
        [
            "functional_tactical_pickups",
            "download_intake_gate",
            "playable_cast_promotion_stage",
            "curriculum_concept_rooms",
            "comprehensive_stress_test_rig",
        ],
    )
    require(
        DOC,
        [
            "Creative Recommendations Implementation Pass",
            "functional tactical pickups",
            "SpawnCreativeRecommendationImplementationLayer",
            "active_download_asset_intake_2026_05_28.tsv",
            "Honesty boundary",
            "Stress Test Plan",
        ],
    )
    print("[verify-may28-creative-recommendations] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
