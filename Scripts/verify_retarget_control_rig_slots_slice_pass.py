#!/usr/bin/env python3
"""Static verifier for the retarget/control-rig runtime slot slice."""

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


helper_h = read(SRC / "CodeRescueRetargetRig.h")
helper_cpp = read(SRC / "CodeRescueRetargetRig.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
friendly_cpp = read(SRC / "FriendlyNPCActor.cpp")
companion_cpp = read(SRC / "CompanionActor.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
boss_cpp = read(SRC / "BossZombieActor.cpp")
manifest = read(DATA / "retarget_control_rig_slots_manifest.tsv")
animation_manifest = read(DATA / "animation_coverage_manifest.tsv")
character_contract = read(DATA / "character_promotion_validation_contract.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "RETARGET_CONTROL_RIG_SLOTS_SLICE.md")
animation_budget_doc = read(DOC / "ANIMATION_BUDGET_RUNTIME_SLICE.md")
character_deep_dive = read(SOURCE_DOC / "CHARACTER_ANIMATION_DEEPDIVE.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_retarget_control_rig_slots_slice_pass.py")

check_all(
    helper_h,
    [
        "ECodeRescueRetargetRigProfile",
        "PlayerOperator",
        "FirstPersonArms",
        "SurvivorHero",
        "FriendlyNPC",
        "CompanionHero",
        "ZombieCrowd",
        "BossWarden",
        "ApplyRuntimeRetargetRigSlots",
    ],
    "retarget helper header must expose every runtime profile",
)
check_all(
    helper_cpp,
    [
        "IKRetargetRuntimeSlot",
        "ControlRigRuntimeSlot",
        "RetargetControlRigRuntimeContract",
        "MayaCharacterCleanupTarget",
        "RetargetProfile_PlayerOperator",
        "RetargetProfile_FirstPersonArms",
        "RetargetProfile_SurvivorHero",
        "RetargetProfile_FriendlyNPC",
        "RetargetProfile_CompanionHero",
        "RetargetProfile_ZombieCrowd",
        "RetargetProfile_BossWarden",
        "ControlRigFullBodySlot",
        "ControlRigArmsSlot",
        "ControlRigFacialSlot",
        "FootIKReviewSlot",
        "WeaponIKSocketSlot",
        "ZombieAttackRetargetSlot",
        "ControlRigBossRevealSlot",
        "BossPhaseMontageSlot",
        "RagdollPhysicsAssetReviewSlot",
    ],
    "retarget helper implementation must add common and profile-specific tags",
)

for label, source, profile, component in [
    ("player body", character_cpp, "ECodeRescueRetargetRigProfile::PlayerOperator", "GetMesh()"),
    ("first-person arms", character_cpp, "ECodeRescueRetargetRigProfile::FirstPersonArms", "FirstPersonArmsMesh"),
    ("survivor hero", survivor_cpp, "ECodeRescueRetargetRigProfile::SurvivorHero", "SkeletalBody"),
    ("friendly NPC", friendly_cpp, "ECodeRescueRetargetRigProfile::FriendlyNPC", "SkeletalBody"),
    ("companion hero", companion_cpp, "ECodeRescueRetargetRigProfile::CompanionHero", "GetMesh()"),
    ("zombie crowd", zombie_cpp, "ECodeRescueRetargetRigProfile::ZombieCrowd", "GetMesh()"),
    ("boss warden", boss_cpp, "ECodeRescueRetargetRigProfile::BossWarden", "GetMesh()"),
]:
    check("CodeRescueRetargetRig.h" in source, f"{label} source must include retarget helper")
    check("ApplyRuntimeRetargetRigSlots" in source, f"{label} source must call retarget helper")
    check(profile in source, f"{label} source must apply {profile}")
    check(component in source, f"{label} source must apply retarget slots to {component}")

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
        "IKRetargetRuntimeSlot",
        "ControlRigFullBodySlot",
        "ControlRigArmsSlot",
        "ControlRigFacialSlot",
        "ControlRigBossRevealSlot",
        "verify_retarget_control_rig_slots_slice_pass.py",
        "runtime_slot_ready",
    ],
    "retarget manifest must cover every runtime owner and validation route",
)
check_all(
    animation_manifest,
    [
        "player_operator",
        "first_person_arms",
        "survivor_hero",
        "friendly_npc",
        "companion_hero",
        "zombie_crowd",
        "boss_warden",
        "CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots",
        "ik_control_rig_runtime_slot_ready",
    ],
    "animation coverage manifest must expose retarget slot rows",
)
check_all(
    character_contract,
    [
        "Runtime retarget and Control Rig slots",
        "CodeRescueRetargetRig runtime slots",
        "verify_retarget_control_rig_slots_slice_pass.py",
    ],
    "character promotion contract must include retarget/control-rig slot rules",
)
check_all(
    creative_plan,
    [
        "IK retargeting and Control Rig slots",
        "verify_retarget_control_rig_slots_slice_pass.py plus verify_animation_budget_runtime_slice_pass.py",
        "manual animation review",
    ],
    "creative plan must route the P1 retarget row through the new verifier",
)
check_all(
    human_qa,
    [
        "RetargetControlRigSlots",
        "IK/Control Rig slot tags",
        "authored IK Rig, IK Retargeter, Control Rig, and Maya cleanup work",
    ],
    "human QA checklist must include retarget/control-rig slot review",
)
check_all(
    visual_targets,
    [
        "RetargetControlRigSlots",
        "player body, first-person arms, survivor, friendly NPC, companion, zombie, and boss skeletal runtime slots",
        "Retarget and Control Rig readiness",
    ],
    "visual regression targets must include retarget/control-rig review",
)
check("verify_retarget_control_rig_slots_slice_pass.py" in full_qa,
      "full QA must run the retarget/control-rig verifier")
check("verify_retarget_control_rig_slots_slice_pass.py" in local_ci,
      "local CI must run the retarget/control-rig verifier")
check_all(
    slice_doc,
    [
        "Retarget Control Rig Slots Slice",
        "CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots",
        "IKRetargetRuntimeSlot",
        "ControlRigRuntimeSlot",
        "PlayerOperator",
        "FirstPersonArms",
        "SurvivorHero",
        "FriendlyNPC",
        "CompanionHero",
        "ZombieCrowd",
        "BossWarden",
        "Validation",
        "Manual Review",
        "Boundaries",
    ],
    "slice documentation must explain implementation, validation, and boundaries",
)
check_all(
    animation_budget_doc,
    [
        "future IK/Control Rig content",
        "remaining character-animation roadmap",
    ],
    "animation budget doc should remain aligned with this retarget slice",
)
check_all(
    character_deep_dive,
    [
        "IK Rig",
        "IK Retargeter",
        "Control Rig",
        "retargeting",
    ],
    "character deep dive must contain the retarget/control-rig guidance",
)
check_all(
    progress,
    [
        "Retarget Control Rig slots slice",
        "CodeRescueRetargetRig",
        "verify_retarget_control_rig_slots_slice_pass.py",
    ],
    "progress log must record the retarget/control-rig slice",
)
check("verify_retarget_control_rig_slots_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_retarget_control_rig_slots_slice_pass] FAIL")
    for err in errors:
        print(f" - {err}")
    sys.exit(1)

print("[verify_retarget_control_rig_slots_slice_pass] PASS: retarget/control-rig runtime slots verified")
