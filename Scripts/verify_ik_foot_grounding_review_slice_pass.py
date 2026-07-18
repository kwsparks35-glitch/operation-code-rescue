#!/usr/bin/env python3
"""Static verifier for the IK foot grounding review slice."""

from pathlib import Path
import sys

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

errors: list[str] = []


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        errors.append(f"missing file: {path.relative_to(PROJECT_ROOT)}")
        return ""


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(haystack: str, needles: list[str], message: str) -> None:
    missing = [needle for needle in needles if needle not in haystack]
    if missing:
        errors.append(f"{message}; missing {missing}")


helper_h = read(SRC / "CodeRescueRetargetRig.h")
helper_cpp = read(SRC / "CodeRescueRetargetRig.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "ik_foot_grounding_review_manifest.tsv")
retarget_manifest = read(DATA / "retarget_control_rig_slots_manifest.tsv")
animation_manifest = read(DATA / "animation_coverage_manifest.tsv")
character_contract = read(DATA / "character_promotion_validation_contract.tsv")
maya_manifest = read(DATA / "maya_character_cleanup_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "IK_FOOT_GROUNDING_REVIEW_SLICE.md")
deep_dive = read(SOURCE_DOC / "CHARACTER_ANIMATION_DEEPDIVE.md")
progress = read(PROJECT_ROOT / "progress.md")

check_all(
    helper_h,
    [
        "ApplyFootGroundingReview",
        "ECodeRescueRetargetRigProfile",
    ],
    "retarget helper header must expose the foot grounding review helper",
)

check_all(
    helper_cpp,
    [
        "void ApplyFootGroundingReview",
        "ApplyFootGroundingReview(Mesh, Profile, OwnerForTags)",
        "FootGroundingExcluded_FirstPersonArms",
        "FootGroundingRuntimeContract",
        "FootIKGroundingReview",
        "FootPlantTraceReady",
        "PelvisOffsetReview",
        "ControlRigFootContactReady",
        "RetargetFootContactPoseReview",
        "PlayerFootGroundingReview",
        "SurvivorFootGroundingReview",
        "FriendlyNPCFootGroundingReview",
        "CompanionFootGroundingReview",
        "ZombieFootGroundingReview",
        "BossFootGroundingReview",
        "WeaponStanceFootPlantReview",
        "RescueGestureFootPlantReview",
        "ServiceGestureFootPlantReview",
        "FormationLocomotionFootPlantReview",
        "ShambleFootPlantReview",
        "HeavyPhaseFootPlantReview",
    ],
    "retarget helper implementation must tag full-body foot grounding profiles and exclude first-person arms",
)

check_all(
    gamemode_cpp,
    [
        "FOOT IK",
        "plant + pelvis",
        "FootIKGroundingReview",
        "MayaCharacterCleanupRuntimeContract",
        "DccRetargetValidationQueue",
    ],
    "runtime DCC review bay must expose a visible FOOT IK station",
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
        "FootGroundingRuntimeContract",
        "FootGroundingExcluded_FirstPersonArms",
        "ControlRigFootContactReady",
        "PelvisOffsetReview",
        "verify_ik_foot_grounding_review_slice_pass.py",
    ],
    "foot grounding manifest must cover every skeletal profile and validation route",
)

check_all(
    retarget_manifest + animation_manifest + character_contract + maya_manifest,
    [
        "FootGroundingRuntimeContract",
        "FootGroundingExcluded_FirstPersonArms",
        "ApplyFootGroundingReview",
        "foot_grounding_contract_ready",
        "Runtime retarget and Control Rig slots plus foot grounding",
        "Foot grounding review bay",
    ],
    "retarget, animation coverage, character promotion, and Maya manifests must include foot grounding",
)

check_all(
    creative_plan + visual_targets + human_qa,
    [
        "verify_ik_foot_grounding_review_slice_pass.py",
        "IKFootGroundingReview",
        "FootGroundingRuntimeContract",
        "FootGroundingExcluded_FirstPersonArms",
        "ControlRigFootContactReady",
    ],
    "creative plan, visual targets, and human QA must route foot grounding review",
)

check("verify_ik_foot_grounding_review_slice_pass.py" in full_qa,
      "full QA must run the IK foot grounding verifier")
check("verify_ik_foot_grounding_review_slice_pass.py" in local_ci,
      "local CI must run the IK foot grounding verifier")

check_all(
    slice_doc,
    [
        "IK Foot Grounding Review Slice",
        "ApplyFootGroundingReview",
        "FootGroundingRuntimeContract",
        "FootIKGroundingReview",
        "FootGroundingExcluded_FirstPersonArms",
        "FOOT IK",
        "verify_ik_foot_grounding_review_slice_pass.py",
    ],
    "slice documentation must describe implementation, player impact, and validation",
)

check_all(
    deep_dive,
    [
        "IK Rig",
        "IK Retargeter",
        "Control Rig",
        "foot-to-ground contact",
    ],
    "character animation deep dive must contain foot-grounding guidance",
)

check_all(
    progress,
    [
        "IK foot grounding review slice",
        "ApplyFootGroundingReview",
        "verify_ik_foot_grounding_review_slice_pass.py",
    ],
    "progress log must document the IK foot grounding review slice",
)

if errors:
    print("[verify_ik_foot_grounding_review_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_ik_foot_grounding_review_slice_pass] PASS: IK foot grounding review verified")
