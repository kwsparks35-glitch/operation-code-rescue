#!/usr/bin/env python3
"""Static verifier for the runtime skeletal animation budget slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

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


helper_h = read(SRC / "CodeRescueAnimationBudget.h")
helper_cpp = read(SRC / "CodeRescueAnimationBudget.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
friendly_cpp = read(SRC / "FriendlyNPCActor.cpp")
companion_cpp = read(SRC / "CompanionActor.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ANIMATION_BUDGET_RUNTIME_SLICE.md")

check_all(
    helper_h,
    [
        "ECodeRescueAnimationBudgetProfile",
        "PlayerBody",
        "FirstPersonArms",
        "HeroNPC",
        "CrowdZombie",
        "ApplySkeletalMeshBudget",
    ],
    "animation budget helper header must expose all runtime profiles",
)
check_all(
    helper_cpp,
    [
        "bComponentUseFixedSkelBounds",
        "SetBoundsScale",
        "VisibilityBasedAnimTickOption",
        "AlwaysTickPoseAndRefreshBones",
        "OnlyTickMontagesWhenNotRendered",
        "OnlyTickPoseWhenRendered",
        "bEnableUpdateRateOptimizations",
        "PrimaryComponentTick.TickInterval",
        "ComponentTags.AddUnique",
        "AnimationBudget_Runtime",
        "AnimationBudget_PlayerBody",
        "AnimationBudget_FirstPersonArms",
        "AnimationBudget_HeroNPC",
        "AnimationBudget_CrowdZombie",
        "CharacterAnimationDeepDive",
    ],
    "animation budget helper must apply visibility ticking, URO, bounds, tick interval, and audit tags",
)

for label, source, profile in [
    ("player character", character_cpp, "ECodeRescueAnimationBudgetProfile::PlayerBody"),
    ("first-person arms", character_cpp, "ECodeRescueAnimationBudgetProfile::FirstPersonArms"),
    ("zombie", zombie_cpp, "ECodeRescueAnimationBudgetProfile::CrowdZombie"),
    ("survivor", survivor_cpp, "ECodeRescueAnimationBudgetProfile::HeroNPC"),
    ("friendly NPC", friendly_cpp, "ECodeRescueAnimationBudgetProfile::HeroNPC"),
    ("companion", companion_cpp, "ECodeRescueAnimationBudgetProfile::HeroNPC"),
]:
    check("CodeRescueAnimationBudget.h" in source,
          f"{label} source must include the animation budget helper")
    check(profile in source,
          f"{label} source must apply the expected animation budget profile")

check("verify_animation_budget_runtime_slice_pass.py" in full_qa,
      "full QA must run the animation budget runtime verifier")
check("verify_animation_budget_runtime_slice_pass.py" in local_ci,
      "local CI must run the animation budget runtime verifier")
check("Runtime skeletal animation budget slice" in progress,
      "progress log must document the runtime skeletal animation budget slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "update-rate optimization",
        "VisibilityBasedAnimTickOption",
        "PlayerBody",
        "FirstPersonArms",
        "HeroNPC",
        "CrowdZombie",
        "Control Rig",
        "IK",
    ],
    "slice doc must map the runtime budget work to the character animation guidance and limits",
)

if errors:
    for error in errors:
        print(f"[verify_animation_budget_runtime_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_animation_budget_runtime_slice_pass] PASS: runtime skeletal animation budget slice verified")
