#!/usr/bin/env python3
"""Static verifier for the editor Data Validation contract slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
EDITOR_SRC = PROJECT_ROOT / "Source/CodeRescueUnrealEditor"
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


uproject = read(PROJECT_ROOT / "CodeRescueUnreal.uproject")
build_cs = read(EDITOR_SRC / "CodeRescueUnrealEditor.Build.cs")
module_cpp = read(EDITOR_SRC / "Private/CodeRescueUnrealEditor.cpp")
validator_h = read(EDITOR_SRC / "Public/CodeRescueAssetManifestValidator.h")
validator_cpp = read(EDITOR_SRC / "Private/CodeRescueAssetManifestValidator.cpp")
unreal_smoke = read(PROJECT_ROOT / "Scripts/verify_code_rescue_data_validation_unreal.py")
contract = read(PROJECT_ROOT / "Content/CodeRescueData/editor_data_validation_contract.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "EDITOR_DATA_VALIDATION_CONTRACT_SLICE.md")
character_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
physics_doc = read(SOURCE_DOC_DIR / "GAME_PHYSICS_DEEPDIVE.md")

check_all(
    uproject,
    [
        '"Name": "CodeRescueUnrealEditor"',
        '"Type": "Editor"',
        '"Name": "CodeRescueUnreal"',
    ],
    "uproject must declare a runtime module plus an editor-only module",
)
check_all(
    build_cs,
    [
        "public class CodeRescueUnrealEditor",
        '"DataValidation"',
        '"CodeRescueUnreal"',
        '"UnrealEd"',
        '"AssetRegistry"',
    ],
    "editor module must depend on DataValidation, the runtime module, and editor support modules",
)
check("IMPLEMENT_MODULE(FDefaultModuleImpl, CodeRescueUnrealEditor)" in module_cpp,
      "editor module must implement a module entrypoint")
check_all(
    validator_h,
    [
        "UCodeRescueAssetManifestValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
    ],
    "validator header must expose a native UEditorValidatorBase subclass using UE 5.7 signatures",
)
check_all(
    validator_cpp,
    [
        "UCodeRescueAssetManifest",
        "Misc/DataValidation.h",
        "CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const",
        "ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)",
        "ZombieSkeletalMesh",
        "SurvivorSkeletalMesh",
        "CityBuildingMeshes",
        "BarricadeMeshes",
        "MuzzleFlashVFX",
        "BulletImpactVFX",
        "FireAndSmokeVFX",
        "InfectionCloudVFX",
        "RadioBriefingSound",
        "ZombieAttackSound",
        "AssetMessage",
        "AssetPasses",
        "EDataValidationResult::Invalid",
    ],
    "validator implementation must enforce promoted Code Rescue asset manifest assignments",
)
check_all(
    unreal_smoke,
    [
        "CodeRescueAssetManifestValidator",
        "CodeRescueAssetManifest",
        "AssetRegistryHelpers",
        "code_rescue_data_validation_contract.json",
        "UEditorValidatorBase",
    ],
    "Unreal Python smoke must confirm validator class registration and write a report",
)
check_all(
    contract,
    [
        "UCodeRescueAssetManifestValidator",
        "UCodeRescueZombieVariantTableValidator",
        "UCodeRescuePhysicsPromotionValidator",
        "UCodeRescueWorldPromotionValidator",
        "UCodeRescueMacCompatibilityValidator",
        "Scripts/verify_mac_compatibility_validation_unreal.py",
        "Scripts/verify_world_promotion_validation_unreal.py",
        "Scripts/verify_physics_promotion_validation_unreal.py",
        "GroomStrandReviewOnlyMac",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "texture-memory review",
        "shader-complexity review",
    ],
    "Data Validation contract manifest must cover current and future imported-asset validator surfaces",
)
check_all(
    creative_plan,
    [
        "DataValidation plus verify_editor_data_validation_contract_pass.py plus verify_world_promotion_validation_unreal.py plus visual review",
        "DataValidation plus verify_editor_data_validation_contract_pass.py plus verify_character_promotion_validation_unreal.py plus retarget tests",
    ],
    "creative plan must wire Houdini and Maya pipeline work to the Data Validation contract verifier",
)
check_all(
    human_qa,
    [
        "editor Data Validation contract",
        "promoted asset manifests",
        "zombie variant rows",
        "physics asset and debris retirement gates",
        "city-module/PCG/streaming gates",
    ],
    "human QA checklist must expose the editor Data Validation contract",
)
check("verify_editor_data_validation_contract_pass.py" in full_qa,
      "full QA must run the static editor Data Validation verifier")
check("verify_code_rescue_data_validation_unreal.py" in full_qa,
      "full QA must run the Unreal-side Data Validation smoke script")
check("verify_editor_data_validation_contract_pass.py" in local_ci,
      "local CI must run the static editor Data Validation verifier")
check_all(
    slice_doc,
    [
        "Editor Data Validation Contract Slice",
        "CodeRescueUnrealEditor",
        "UCodeRescueAssetManifestValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
        "verify_code_rescue_data_validation_unreal.py",
        "Data Validation",
    ],
    "slice documentation must explain the native editor validator and validation boundary",
)
check_all(
    character_doc,
    [
        "Data Validation",
        "UEditorValidatorBase",
        "skeletal mesh is assigned",
        "Animation Blueprint class is set",
        "LODs exist",
    ],
    "character deep dive must contain the Data Validation guidance this slice implements",
)
check_all(
    world_doc,
    [
        "Data Validation",
        "UEditorValidatorBase",
        "performance-budget rules",
        "streaming/memory budgets",
    ],
    "world deep dive must contain the Data Validation guidance this slice implements",
)
check_all(
    physics_doc,
    [
        "UE Data Validation",
        "Physics Asset",
        "Geometry Collection",
        "fixed seed",
    ],
    "physics deep dive must contain the Data Validation guidance this slice implements",
)
check_all(
    progress,
    [
        "Editor Data Validation contract slice",
        "UCodeRescueAssetManifestValidator",
        "verify_code_rescue_data_validation_unreal.py",
    ],
    "progress log must record the editor Data Validation contract slice",
)

if errors:
    for error in errors:
        print(f"[verify_editor_data_validation_contract_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_editor_data_validation_contract_pass] PASS")
