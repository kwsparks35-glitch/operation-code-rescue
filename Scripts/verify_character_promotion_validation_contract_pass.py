#!/usr/bin/env python3
"""Static verifier for the character promotion validation slice."""

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


validator_h = read(EDITOR_SRC / "Public/CodeRescueZombieVariantTableValidator.h")
validator_cpp = read(EDITOR_SRC / "Private/CodeRescueZombieVariantTableValidator.cpp")
unreal_smoke = read(PROJECT_ROOT / "Scripts/verify_character_promotion_validation_unreal.py")
static_self = read(PROJECT_ROOT / "Scripts/verify_character_promotion_validation_contract_pass.py")
editor_contract = read(PROJECT_ROOT / "Content/CodeRescueData/editor_data_validation_contract.tsv")
character_contract = read(PROJECT_ROOT / "Content/CodeRescueData/character_promotion_validation_contract.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CHARACTER_PROMOTION_VALIDATION_SLICE.md")
editor_slice_doc = read(DOC_DIR / "EDITOR_DATA_VALIDATION_CONTRACT_SLICE.md")
character_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")
hair_doc = read(DOC_DIR / "MAC_HAIR_CARD_COMPATIBILITY_SLICE.md")
asset_budget_doc = read(DOC_DIR / "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md")

check_all(
    validator_h,
    [
        "UCodeRescueZombieVariantTableValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
    ],
    "character validator header must expose a native UEditorValidatorBase subclass",
)
check_all(
    validator_cpp,
    [
        "FZombieVariantRow",
        "UDataTable",
        "GetRowStruct() == FZombieVariantRow::StaticStruct()",
        "Default",
        "BaseMesh",
        "SkeletalMesh",
        "AnimBPClass",
        "HealthMultiplier",
        "DamageMultiplier",
        "SpeedMultiplier",
        "MeshScale",
        "ZoneWeights",
        "/Game/Grooms",
        "AssetMessage",
        "AssetPasses",
        "EDataValidationResult::Invalid",
    ],
    "character validator implementation must enforce zombie variant promotion rules",
)
check_all(
    unreal_smoke,
    [
        "CodeRescueZombieVariantTableValidator",
        "/Game/CodeRescueAssets/DT_ZombieVariants",
        "code_rescue_character_promotion_validation.json",
        "promoted_row_count",
        "fallback_rows",
        "SkeletalMesh",
        "AnimBPClass",
        "/Game/Grooms",
        "DataTableFunctionLibrary.export_data_table_to_json_string",
    ],
    "Unreal smoke must load the live zombie DataTable and write a promotion report",
)
check_all(
    editor_contract,
    [
        "UCodeRescueZombieVariantTableValidator",
        "Promoted zombie variant DataTables",
        "Default and BaseMesh remain explicit fallback exceptions",
        "Scripts/verify_character_promotion_validation_unreal.py",
    ],
    "editor Data Validation contract must list the current character validator",
)
check("FutureCharacterRuntimeValidator" not in editor_contract,
      "character validator should no longer be listed as a future-only surface")
check_all(
    character_contract,
    [
        "Zombie variant DataTable",
        "/Game/CodeRescueAssets/DT_ZombieVariants",
        "Zombie fallback exceptions",
        "Survivor MetaHuman import",
        "Friendly NPC cast promotion",
        "Player rescue operator promotion",
        "Character Mac compatibility",
        "UCodeRescueZombieVariantTableValidator",
        "verify_character_promotion_validation_unreal.py",
        "verify_mac_hair_card_compatibility_slice_pass.py",
    ],
    "character promotion contract manifest must cover current and future character surfaces",
)
check_all(
    creative_plan,
    [
        "zombie family variants",
        "verify_character_world_assets.py plus verify_character_promotion_validation_unreal.py",
        "Maya character cleanup",
        "verify_character_promotion_validation_unreal.py plus retarget tests",
    ],
    "creative plan must route zombie variants and Maya cleanup through character promotion validation",
)
check_all(
    human_qa,
    [
        "zombie variant promotion validation",
        "promoted zombie meshes, and locomotion AnimBPs",
        "character promotion validation",
        "promoted asset manifests",
        "zombie variant rows",
    ],
    "human QA checklist must expose character promotion validation",
)
check("verify_character_promotion_validation_contract_pass.py" in full_qa,
      "full QA must run the static character promotion verifier")
check("verify_character_promotion_validation_unreal.py" in full_qa,
      "full QA must run the Unreal-side character promotion smoke")
check("verify_character_promotion_validation_contract_pass.py" in local_ci,
      "local CI must run the static character promotion verifier")
check_all(
    slice_doc,
    [
        "Character Promotion Validation Slice",
        "UCodeRescueZombieVariantTableValidator",
        "UEditorValidatorBase",
        "FZombieVariantRow",
        "DT_ZombieVariants",
        "code_rescue_character_promotion_validation.json",
        "Default",
        "BaseMesh",
        "Mac-safe hair boundaries",
    ],
    "slice documentation must explain the zombie variant promotion validator",
)
check_all(
    editor_slice_doc,
    [
        "UCodeRescueZombieVariantTableValidator",
        "UCodeRescueMacCompatibilityValidator",
    ],
    "editor Data Validation slice should still describe the expanded live validator lane",
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
    "character deep dive must contain the character Data Validation guidance this slice implements",
)
check_all(
    hair_doc,
    [
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
    ],
    "hair compatibility slice must remain available for character promotion review",
)
check_all(
    asset_budget_doc,
    [
        "crowd zombie skeletal meshes",
        "LOD0-to-LOD3",
        "animation URO",
        "Data Validation",
    ],
    "asset budget slice must remain aligned with zombie character promotion budgets",
)
check_all(
    progress,
    [
        "Character promotion validation slice",
        "UCodeRescueZombieVariantTableValidator",
        "verify_character_promotion_validation_unreal.py",
        "code_rescue_character_promotion_validation.json",
    ],
    "progress log must record the character promotion validation slice",
)
check("verify_character_promotion_validation_contract_pass.py" in static_self,
      "static verifier should be self-identifying")

if errors:
    for error in errors:
        print(f"[verify_character_promotion_validation_contract_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_character_promotion_validation_contract_pass] PASS")
