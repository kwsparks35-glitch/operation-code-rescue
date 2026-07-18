#!/usr/bin/env python3
"""Static verifier for the Maya character cleanup slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


helper_cpp = read(SRC / "CodeRescueRetargetRig.cpp")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "maya_character_cleanup_manifest.tsv")
animation_manifest = read(DATA / "animation_coverage_manifest.tsv")
character_contract = read(DATA / "character_promotion_validation_contract.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "MAYA_CHARACTER_CLEANUP_SLICE.md")
retarget_doc = read(DOC / "RETARGET_CONTROL_RIG_SLOTS_SLICE.md")
character_deep_dive = read(SOURCE_DOC / "CHARACTER_ANIMATION_DEEPDIVE.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_maya_character_cleanup_slice_pass.py")


check_all(
    helper_cpp,
    [
        "MayaCharacterCleanupRuntimeContract",
        "MayaFbxExportReview",
        "MayaSkeletonNamingReview",
        "MayaBindPoseOriginReview",
        "MayaAnimationTakeCleanupReview",
        "MayaSocketAuthoringReview",
        "MayaLODMaterialBudgetReview",
        "MayaPhysicsAssetReview",
        "MayaWeaponSocketCleanup",
        "MayaCameraSocketCleanup",
        "MayaFirstPersonArmsCleanup",
        "MayaGripPoseCleanup",
        "MayaFacialRigCleanup",
        "MayaWardrobeSocketCleanup",
        "MayaRoleGestureCleanup",
        "MayaFormationLocomotionCleanup",
        "MayaCommandGestureCleanup",
        "MayaZombieAttackCleanup",
        "MayaRagdollBodyCleanup",
        "MayaBossMontageCleanup",
        "MayaCinematicRevealCleanup",
    ],
    "retarget helper must tag every live skeletal path with Maya cleanup gates",
)

check_all(
    game_mode_cpp,
    [
        "MAYA CHARACTER CLEANUP RECIPE",
        "MayaCharacterCleanup",
        "MayaCleanupRecipeBoard",
        "MayaRetargetRoundtripReview",
        "MayaPromotionEvidenceReady",
        "BIND POSE",
        "SKELETON NAMES",
        "SOCKETS",
        "ANIM TAKES",
        "LOD MATERIAL",
        "PHYSICS ASSET",
        "FBX EXPORT",
        "PROMOTION",
    ],
    "game mode must expose a visible Maya cleanup review lane",
)

check_all(
    manifest,
    [
        "PlayerOperator",
        "FirstPersonArms",
        "SurvivorHero",
        "FriendlyNPC",
        "CompanionHero",
        "ZombieCrowd",
        "BossWarden",
        "DCC cleanup review bay",
        "MayaCharacterCleanupRuntimeContract",
        "MayaBindPoseOriginReview",
        "MayaSkeletonNamingReview",
        "MayaSocketAuthoringReview",
        "MayaAnimationTakeCleanupReview",
        "MayaLODMaterialBudgetReview",
        "MayaPhysicsAssetReview",
        "MayaPromotionEvidenceReady",
        "verify_maya_character_cleanup_slice_pass.py",
        "cleanup_contract_ready",
        "cleanup_contract_visible",
    ],
    "Maya cleanup manifest must cover every runtime profile and review bay station",
)

check_all(
    animation_manifest,
    [
        "maya_player_operator",
        "maya_first_person_arms",
        "maya_survivor_hero",
        "maya_friendly_npc",
        "maya_companion_hero",
        "maya_zombie_crowd",
        "maya_boss_warden",
        "maya_cleanup_profile",
        "maya_cleanup_contract_ready",
    ],
    "animation coverage manifest must include Maya cleanup rows",
)

check_all(
    character_contract,
    [
        "Maya character cleanup manifest",
        "bind-pose/origin review",
        "socket authoring review",
        "animation take cleanup",
        "physics asset review",
        "verify_maya_character_cleanup_slice_pass.py",
    ],
    "character promotion contract must include the Maya cleanup rules",
)

check_all(
    creative_plan,
    [
        "Maya character cleanup",
        "verify_maya_character_cleanup_slice_pass.py",
        "manual animation/socket review",
    ],
    "creative plan must route the P1 Maya row through the cleanup verifier",
)

check_all(
    human_qa,
    [
        "MayaCharacterCleanup",
        "bind pose, skeleton names, sockets, animation takes, LOD/material, physics asset, FBX export, and promotion stations",
        "Every promoted character path has clear Maya cleanup evidence",
    ],
    "human QA checklist must include Maya cleanup review",
)

check_all(
    visual_targets,
    [
        "MayaCharacterCleanup",
        "Maya cleanup recipe board",
        "Character cleanup should read as a concrete Maya-to-Unreal promotion gate",
    ],
    "visual regression targets must include the Maya cleanup surface",
)

check("verify_maya_character_cleanup_slice_pass.py" in full_qa,
      "full QA must run the Maya cleanup verifier")
check("verify_maya_character_cleanup_slice_pass.py" in local_ci,
      "local CI must run the Maya cleanup verifier")

check_all(
    slice_doc,
    [
        "Maya Character Cleanup Slice",
        "CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots",
        "MayaCharacterCleanupRuntimeContract",
        "MayaFbxExportReview",
        "MayaSocketAuthoringReview",
        "MayaPromotionEvidenceReady",
        "Validation",
        "Boundaries",
    ],
    "slice documentation must explain implementation, validation, and boundaries",
)

check_all(
    retarget_doc,
    [
        "Maya cleanup",
        "runtime contract",
    ],
    "retarget slot documentation should remain aligned with Maya cleanup",
)

check_all(
    character_deep_dive,
    [
        "FBX",
        "Armature",
        "Only Deform Bones",
        "Control Rig",
        "IK Retargeter",
    ],
    "character deep dive must contain the DCC/FBX animation guidance",
)

check_all(
    progress,
    [
        "Maya character cleanup slice",
        "MayaCharacterCleanupRuntimeContract",
        "verify_maya_character_cleanup_slice_pass.py",
    ],
    "progress log must record the Maya cleanup slice",
)

check("verify_maya_character_cleanup_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_maya_character_cleanup_slice_pass] FAIL")
    for err in errors:
        print(f" - {err}")
    sys.exit(1)

print("[verify_maya_character_cleanup_slice_pass] PASS: Maya character cleanup contract verified")
