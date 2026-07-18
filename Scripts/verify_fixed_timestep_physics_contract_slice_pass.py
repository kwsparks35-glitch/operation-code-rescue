#!/usr/bin/env python3
"""Static verifier for the fixed timestep physics contract slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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


config = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
helper_h = read(SRC / "CodeRescuePhysicsStability.h")
helper_cpp = read(SRC / "CodeRescuePhysicsStability.cpp")
throwable = read(SRC / "ThrowableActor.cpp")
barricade = read(SRC / "BarricadeActor.cpp")
zombie = read(SRC / "CodeZombieActor.cpp")
jeep = read(SRC / "JeepActor.cpp")
game_mode = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "fixed_timestep_physics_contract_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
physics_contract = read(DATA / "physics_promotion_validation_contract.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "FIXED_TIMESTEP_PHYSICS_CONTRACT_SLICE.md")

check_all(
    config,
    [
        "[/Script/Engine.PhysicsSettings]",
        "MinPhysicsDeltaTime=0.000000",
        "MaxPhysicsDeltaTime=0.033333",
        "bSubstepping=True",
        "bSubsteppingAsync=False",
        "bTickPhysicsAsync=False",
        "AsyncFixedTimeStepSize=0.016667",
        "MaxSubstepDeltaTime=0.016667",
        "MaxSubsteps=6",
        "InitialAverageFrameRate=0.016667",
    ],
    "physics config must expose the sync fixed-step/substep contract",
)
check("bTickPhysicsAsync=True" not in config and "bSubsteppingAsync=True" not in config,
      "async physics must stay disabled until a deliberate validation pass")

check_all(
    helper_h,
    [
        "namespace CodeRescuePhysicsStability",
        "FixedStepSeconds = 1.0f / 60.0f",
        "MaxPhysicsDeltaSeconds = 1.0f / 30.0f",
        "MaxSubstepCount = 6",
        "DefaultMaxDepenetrationVelocity = 900.0f",
        "ApplyRuntimeBodyContract",
        "GetRuntimeContractSummary",
    ],
    "helper header must define the shared fixed-step contract",
)
check_all(
    helper_cpp,
    [
        "SetMaxDepenetrationVelocity",
        "SetUseCCD",
        "SleepFamily = ESleepFamily::Custom",
        "CustomSleepThresholdMultiplier",
        "StabilizationThresholdMultiplier",
        "FixedStepPhysicsBody",
        "ChaosSubstepStability",
        "PhysicsDeterminismReview",
        "FixedStepPhysicsContract",
        "SubsteppedPhysicsRuntime",
        "Top50Recommendation25",
    ],
    "helper implementation must apply solver-facing stability settings and QA tags",
)

for label, source, tokens in [
    (
        "throwables",
        throwable,
        [
            '#include "CodeRescuePhysicsStability.h"',
            "ThrowableFixedStepBody",
            "ThrowableSubstepLaunch",
            "ThrowablePulseFixedStepTarget",
            "true);",
        ],
    ),
    (
        "barricades",
        barricade,
        [
            '#include "CodeRescuePhysicsStability.h"',
            "CoverFixedStepBody",
            "CoverDebrisFixedStepBody",
            "ScheduleDebrisSleepDisable",
        ],
    ),
    (
        "zombies",
        zombie,
        [
            '#include "CodeRescuePhysicsStability.h"',
            "ZombiePhysicalHitReactionFixedStepBody",
            "ZombieRagdollFixedStepBody",
            "ZombiePrimitiveCorpseFixedStepBody",
        ],
    ),
    (
        "jeep",
        jeep,
        [
            '#include "CodeRescuePhysicsStability.h"',
            "JeepFallbackFixedStepBody",
            "ChaosVehicleReadyFallback",
        ],
    ),
    (
        "GameMode physics props",
        game_mode,
        [
            '#include "CodeRescuePhysicsStability.h"',
            "PhysicsTrainingFixedStepBody",
            "SystemsReviewFixedStepBody",
            "StressRigFixedStepBody",
        ],
    ),
]:
    check_all(source, tokens, f"{label} must opt into the shared fixed-step contract")

check_all(
    manifest,
    [
        "Physics settings",
        "Shared stabilizer",
        "Throwable bodies",
        "Zombie physics",
        "Cover and debris",
        "Vehicle and world props",
        "verify_fixed_timestep_physics_contract_slice_pass.py",
    ],
    "manifest must describe fixed-step physics coverage",
)
check("determinism and fixed timestep physics contract" in creative_plan,
      "creative inclusion plan must include the fixed timestep contract")
check("FixedTimestepPhysicsContract" in visual_targets,
      "visual regression targets must include the fixed timestep physics contract")
check("FixedTimestepPhysicsContract" in human_qa,
      "human QA checklist must include the fixed timestep physics contract")
check("Fixed timestep physics contract" in physics_contract,
      "physics promotion contract must include the fixed timestep physics contract")
check("verify_fixed_timestep_physics_contract_slice_pass.py" in full_qa,
      "full QA must run the fixed timestep physics verifier")
check("verify_fixed_timestep_physics_contract_slice_pass.py" in local_ci,
      "local CI must run the fixed timestep physics verifier")
check("Fixed timestep physics contract slice" in progress,
      "progress log must document the fixed timestep physics slice")
check("GAME_PHYSICS_DEEPDIVE" in doc and "recommendation 25" in doc,
      "slice doc must map the work to the physics guidance and Top 50 recommendation")

if errors:
    for error in errors:
        print(f"[verify_fixed_timestep_physics_contract_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_fixed_timestep_physics_contract_slice_pass] PASS: fixed timestep physics contract verified")
