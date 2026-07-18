#!/usr/bin/env python3
"""Static verifier for the zombie motion readability slice."""

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


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/zombie_motion_readability_manifest.tsv")
enemy_readability = read(PROJECT_ROOT / "Content/CodeRescueData/enemy_readability_manifest.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "ZOMBIE_MOTION_READABILITY_SLICE.md")

tick_body = function_body(zombie_cpp, "void ACodeZombieActor::Tick")
damage_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyRescueDamage")
motion_body = function_body(zombie_cpp, "void ACodeZombieActor::UpdateMotionReadability")
cache_body = function_body(zombie_cpp, "void ACodeZombieActor::CacheMotionReadabilityBasePose")
reset_body = function_body(zombie_cpp, "void ACodeZombieActor::ResetMotionReadabilityPose")

check_all(
    zombie_h,
    [
        "bEnableMotionReadability",
        "MotionReadabilitySwayScale",
        "HitReactionPoseDuration",
        "AttackLungePoseDuration",
        "MotionReadabilityPhase",
        "HitReactionPoseTimer",
        "AttackLungePoseTimer",
        "CacheMotionReadabilityBasePose",
        "ResetMotionReadabilityPose",
        "TriggerAttackMotionCue",
        "TriggerHitReactionMotionCue",
        "UpdateMotionReadability",
    ],
    "zombie header must declare runtime motion readability tuning, state, and helpers",
)
check_all(
    cache_body,
    [
        "SkeletalMotionBaseLocation",
        "PrimitiveBodyMotionBaseLocation",
        "PrimitiveHeadMotionBaseLocation",
        "GlowMotionBaseLocation",
        "ZombieMotionReadabilityComponent",
        "AdditivePoseReadabilityRuntime",
        "ZombieMotionReadabilityRuntime",
    ],
    "base pose cache must preserve skeletal, primitive, and glow transforms with audit tags",
)
check_all(
    reset_body,
    [
        "IsSimulatingPhysics",
        "SetRelativeLocationAndRotation",
        "SetRelativeScale3D",
        "Glow->SetRelativeLocation",
    ],
    "reset path must restore cached poses without fighting physics",
)
check_all(
    motion_body,
    [
        "GetVelocity().Size2D()",
        "bTelegraphingAttack",
        "bProtectedLearningHold",
        "MotionReadabilityPhase",
        "ZombieLocomotionSway",
        "ZombieAttackWindupPose",
        "ZombieAttackLungePose",
        "ZombieHitReactPoseFallback",
        "ZombieProtectedZoneHoldPose",
        "SetRelativeLocation",
        "SetRelativeRotation",
    ],
    "motion update must drive chase, windup, lunge, hit, and protected-hold visual states",
)
check_all(
    tick_body,
    [
        "ResetMotionReadabilityPose();",
        "UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, false, true);",
        "UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, false, false);",
        "UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, bTelegraphingAttack, false);",
        "TriggerAttackMotionCue();",
    ],
    "tick must wire motion readability into protected hold, idle/tracking, windup, and attack commit",
)
check_all(
    damage_body,
    [
        "ApplyHitReadabilityImpulse",
        "TriggerHitReactionMotionCue",
        "HitReactMontage",
        "TryActivateDeathRagdoll",
        "ActivatePrimitiveDeathPhysics",
    ],
    "damage path must combine additive hit reaction with existing montage and death physics behavior",
)
check_all(
    plan,
    [
        "standard direct-pursuit zombies",
        "verify_zombie_motion_readability_slice_pass.py",
        "zombie family variants",
    ],
    "creative inclusion plan must route enemy rows through the zombie motion verifier",
)
check_all(
    manifest,
    [
        "Additive zombie pose layer",
        "Locomotion sway",
        "Attack windup and lunge",
        "Hit reaction pose",
        "Protected learning hold",
        "Death and physics compatibility",
    ],
    "manifest must document every zombie motion readability surface",
)
check_all(
    enemy_readability,
    [
        "ZombieMotionReadability",
        "UpdateMotionReadability",
        "TriggerAttackMotionCue",
        "TriggerHitReactionMotionCue",
    ],
    "enemy readability manifest must list the new motion readability signal",
)
check_all(
    qa,
    [
        "ZombieMotionReadability",
        "chase sway",
        "attack windup",
        "lunge",
        "hit recoil",
        "protected learning hold",
    ],
    "human QA checklist must include zombie motion readability review",
)
check_all(
    visual,
    [
        "ZombieMotionReadability",
        "chase sway",
        "attack windup pose",
        "attack lunge pose",
        "hit recoil",
    ],
    "visual regression targets must include zombie motion readability screenshots",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_zombie_motion_readability_slice_pass.py"],
    "local CI and full QA must run the zombie motion readability verifier",
)
check_all(
    progress + doc,
    [
        "Zombie motion readability slice",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "verify_zombie_motion_readability_slice_pass.py",
        "ZombieMotionReadabilityRuntime",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_zombie_motion_readability_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_zombie_motion_readability_slice_pass] OK")
