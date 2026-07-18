#!/usr/bin/env python3
"""Static verifier for the physics promotion validation slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
EDITOR_SRC = PROJECT_ROOT / "Source/CodeRescueUnrealEditor"
RUNTIME_SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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


validator_h = read(EDITOR_SRC / "Public/CodeRescuePhysicsPromotionValidator.h")
validator_cpp = read(EDITOR_SRC / "Private/CodeRescuePhysicsPromotionValidator.cpp")
barricade_h = read(RUNTIME_SRC / "BarricadeActor.h")
barricade_cpp = read(RUNTIME_SRC / "BarricadeActor.cpp")
unreal_smoke = read(PROJECT_ROOT / "Scripts/verify_physics_promotion_validation_unreal.py")
static_self = read(PROJECT_ROOT / "Scripts/verify_physics_promotion_validation_contract_pass.py")
editor_contract = read(PROJECT_ROOT / "Content/CodeRescueData/editor_data_validation_contract.tsv")
physics_contract = read(PROJECT_ROOT / "Content/CodeRescueData/physics_promotion_validation_contract.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
mac_budget = read(PROJECT_ROOT / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "PHYSICS_PROMOTION_VALIDATION_SLICE.md")
physics_doc = read(SOURCE_DOC_DIR / "GAME_PHYSICS_DEEPDIVE.md")
asset_budget_doc = read(DOC_DIR / "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md")
destructible_doc = read(DOC_DIR / "DESTRUCTIBLE_COVER_PHYSICS_SLICE.md")
zombie_physics_doc = read(DOC_DIR / "ZOMBIE_DEATH_PHYSICS_SLICE.md")

check_all(
    validator_h,
    [
        "UCodeRescuePhysicsPromotionValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
    ],
    "physics validator header must expose a native UEditorValidatorBase subclass",
)
check_all(
    validator_cpp,
    [
        "UPhysicsAsset",
        "SkeletalBodySetups",
        "ConstraintSetup",
        "GetElementCount",
        "GeometryCollection",
        "Runtime zombie ragdoll Physics Assets",
        "AssetMessage",
        "AssetPasses",
        "EDataValidationResult::Invalid",
    ],
    "physics validator implementation must enforce Physics Asset and Geometry Collection promotion rules",
)
check_all(
    barricade_h + barricade_cpp,
    [
        "DebrisSleepDisableDelay",
        "ScheduleDebrisSleepDisable",
        "PutRigidBodyToSleep",
        "SetPhysicsLinearVelocity(FVector::ZeroVector)",
        "SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector)",
        "SetSimulatePhysics(false)",
        "SetCollisionEnabled(ECollisionEnabled::QueryOnly)",
        "ChaosDebrisSleepDisableFallback",
        "ChaosDebrisSleepDisabled",
        "MacPhysicsBudgetReviewGate",
    ],
    "barricade debris must sleep and disable after its readable burst window",
)
check_all(
    unreal_smoke,
    [
        "CodeRescuePhysicsPromotionValidator",
        "/Game/CodeRescueAssets/DT_ZombieVariants",
        "code_rescue_physics_promotion_validation.json",
        "physics_asset_body_count",
        "physics_asset_constraint_count",
        "ragdoll_promotion_ready",
        "GeometryCollection",
        "DebrisSleepDisableDelay",
        "source_checks",
    ],
    "Unreal smoke must validate promoted physics assets and write a report",
)
check_all(
    editor_contract,
    [
        "UCodeRescuePhysicsPromotionValidator",
        "Promoted Physics Assets, zombie ragdoll candidates, destructible cover, and Geometry Collections",
        "Scripts/verify_physics_promotion_validation_unreal.py",
    ],
    "editor Data Validation contract must list the current physics validator",
)
check("FuturePhysicsRuntimeValidator" not in editor_contract,
      "physics validator should no longer be listed as a future-only surface")
check_all(
    physics_contract,
    [
        "Zombie ragdoll Physics Assets",
        "Promoted zombie DataTable physics",
        "Destructible cover debris",
        "Geometry Collection promotion",
        "Throwable radial impulse props",
        "Jeep Chaos vehicle path",
        "Mac active physics budget",
        "UCodeRescuePhysicsPromotionValidator",
        "verify_physics_promotion_validation_unreal.py",
    ],
    "physics promotion manifest must cover current and future physics surfaces",
)
check_all(
    creative_plan,
    [
        "interactive barricades and cover",
        "verify_runtime_step_smoke_contracts.py plus verify_physics_promotion_validation_unreal.py",
    ],
    "creative plan must route interactive physics through the physics promotion smoke",
)
check_all(
    human_qa,
    [
        "physics promotion validation",
        "physics asset and debris retirement gates",
    ],
    "human QA checklist must expose physics promotion validation",
)
check_all(
    performance_budget,
    [
        "PhysicsPromotionValidation",
        "Physics Assets, destructible debris, throwables, ragdolls, and future Geometry Collections",
        "active-body retirement",
    ],
    "performance budget must include the physics promotion validation row",
)
check_all(
    mac_budget,
    [
        "Physics and destruction props",
        "verify_mac_asset_import_budget_gate_slice_pass.py plus verify_physics_promotion_validation_contract_pass.py",
    ],
    "Mac budget gate must route physics props through this validation slice",
)
check("verify_physics_promotion_validation_contract_pass.py" in full_qa,
      "full QA must run the static physics promotion verifier")
check("verify_physics_promotion_validation_unreal.py" in full_qa,
      "full QA must run the Unreal-side physics promotion smoke")
check("verify_physics_promotion_validation_contract_pass.py" in local_ci,
      "local CI must run the static physics promotion verifier")
check_all(
    slice_doc,
    [
        "Physics Promotion Validation Slice",
        "UCodeRescuePhysicsPromotionValidator",
        "CodeRescuePhysicsPromotionValidator",
        "code_rescue_physics_promotion_validation.json",
        "DebrisSleepDisableDelay",
        "Physics Asset",
        "Geometry Collection",
        "Sleep/Disable",
        "GAME_PHYSICS_DEEPDIVE",
    ],
    "slice documentation must explain the physics validator and runtime debris retirement",
)
check_all(
    physics_doc,
    [
        "UE Data Validation",
        "Physics Asset",
        "Geometry Collection",
        "fixed seed",
        "Sleep/Disable Fields",
        "simultaneous active ragdolls",
    ],
    "physics deep dive must contain the validation and budget guidance this slice implements",
)
check_all(
    asset_budget_doc,
    [
        "physics and destruction props",
        "simple collision preference",
        "piece-count limits",
        "sleep/disable behavior",
    ],
    "Mac asset budget gate must remain aligned with the physics validation slice",
)
check_all(
    destructible_doc,
    [
        "Destructible Cover Physics Slice",
        "ChaosDestructionFallback",
    ],
    "destructible cover slice must remain available for this validation layer",
)
check_all(
    zombie_physics_doc,
    [
        "Zombie Death Physics",
        "PhysicsAsset",
        "CodeRescueMaxActiveRagdollCorpses",
    ],
    "zombie death physics slice must remain available for this validation layer",
)
check_all(
    progress,
    [
        "Physics promotion validation slice",
        "UCodeRescuePhysicsPromotionValidator",
        "verify_physics_promotion_validation_unreal.py",
        "code_rescue_physics_promotion_validation.json",
    ],
    "progress log must record the physics promotion validation slice",
)
check("verify_physics_promotion_validation_contract_pass.py" in static_self,
      "static verifier should be self-identifying")

if errors:
    for error in errors:
        print(f"[verify_physics_promotion_validation_contract_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_physics_promotion_validation_contract_pass] PASS")
