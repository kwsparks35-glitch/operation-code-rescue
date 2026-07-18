#!/usr/bin/env python3
"""Static verifier for the world promotion validation slice."""

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


validator_h = read(EDITOR_SRC / "Public/CodeRescueWorldPromotionValidator.h")
validator_cpp = read(EDITOR_SRC / "Private/CodeRescueWorldPromotionValidator.cpp")
game_mode_h = read(RUNTIME_SRC / "CodeRescueGameMode.h")
game_mode_cpp = read(RUNTIME_SRC / "CodeRescueGameMode.cpp")
spawning_cpp = read(RUNTIME_SRC / "CodeRescueGameModeSpawning.cpp")
unreal_smoke = read(PROJECT_ROOT / "Scripts/verify_world_promotion_validation_unreal.py")
static_self = read(PROJECT_ROOT / "Scripts/verify_world_promotion_validation_contract_pass.py")
editor_contract = read(PROJECT_ROOT / "Content/CodeRescueData/editor_data_validation_contract.tsv")
world_contract = read(PROJECT_ROOT / "Content/CodeRescueData/world_promotion_validation_contract.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
mac_budget = read(PROJECT_ROOT / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "WORLD_PROMOTION_VALIDATION_SLICE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
mac_asset_doc = read(DOC_DIR / "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md")
feature_doc = read(DOC_DIR / "MAC_FEATURE_CAPABILITY_GATE_SLICE.md")
editor_doc = read(DOC_DIR / "EDITOR_DATA_VALIDATION_CONTRACT_SLICE.md")

runtime_source = "\n".join([game_mode_h, game_mode_cpp, spawning_cpp])

check_all(
    validator_h,
    [
        "UCodeRescueWorldPromotionValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
    ],
    "world validator header must expose a native UEditorValidatorBase subclass",
)
check_all(
    validator_cpp,
    [
        "UStaticMesh",
        "GetNumLODs",
        "GetStaticMaterials",
        "AggGeom.GetElementCount",
        "CTF_UseComplexAsSimple",
        "PackedLevelActor",
        "WorldPartition",
        "DataLayer",
        "PCG",
        "Runtime-promoted city modules",
        "simple collision",
        "AssetMessage",
        "AssetPasses",
        "EDataValidationResult::Invalid",
    ],
    "world validator implementation must enforce static mesh and world-framework promotion rules",
)
check_all(
    runtime_source,
    [
        "SpawnAuthoredPropsForCity",
        "SpawnStaticMeshProp",
        "LoadCodeRescueCityBuildingMesh",
        "LoadCodeRescueBridgeMesh",
        "SpawnBlock(Loc",
        "HoudiniProceduralWorldDesign",
        "PCGWorldPartitionCell",
        "WorldPartitionReady",
        "PCGRouteSplineReady",
        "MajorCityDistrictKit",
        "InteriorMissionSpaceReady",
        "HumanScaleBuildingProportion",
        "EnsureCampaignCityLoaded",
        "ClearStreamedCampaignActors",
        "RegisterStreamedActor",
        "ApplyUSCitySkyRealization",
        "SpawnPerZonePostProcessVolume",
        "SpawnWeatherForCity",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
    ],
    "runtime game mode must expose PCG/world review cells, authored mesh fallback, streaming cleanup, and Mac promotion gates",
)
check_all(
    unreal_smoke,
    [
        "CodeRescueWorldPromotionValidator",
        "code_rescue_world_promotion_validation.json",
        "WORLD_CANDIDATE_PREFIXES",
        "STRICT_PROMOTION_TOKENS",
        "world_candidate_static_mesh_count",
        "strict_runtime_promoted_static_mesh_count",
        "simple_collision_count",
        "framework_counts",
        "source_checks",
    ],
    "Unreal smoke must validate world promotion surfaces and write a report",
)
check_all(
    editor_contract,
    [
        "UCodeRescueWorldPromotionValidator",
        "Promoted city modules, HLOD/PLA kits, PCG outputs, Data Layer migrations, and imported environment packs",
        "Scripts/verify_world_promotion_validation_unreal.py",
    ],
    "editor Data Validation contract must list the current world validator",
)
check("FutureWorldRuntimeValidator" not in editor_contract,
      "world validator should no longer be listed as a future-only surface")
check_all(
    world_contract,
    [
        "Authored city module Static Meshes",
        "Current C++ city fallback",
        "PCG World Partition staging",
        "Packed Level Actor and HLOD kits",
        "Data Layer migration",
        "Human-scale collision and accessibility",
        "Apple Silicon streaming budget",
        "UCodeRescueWorldPromotionValidator",
        "verify_world_promotion_validation_unreal.py",
    ],
    "world promotion manifest must cover current and future world-development surfaces",
)
check_all(
    creative_plan,
    [
        "Houdini modular city output",
        "DataValidation plus verify_editor_data_validation_contract_pass.py plus verify_world_promotion_validation_unreal.py plus visual review",
    ],
    "creative plan must route Houdini/PCG world work through the world promotion smoke",
)
check_all(
    human_qa,
    [
        "world promotion validation",
        "city-module/PCG/streaming gates",
    ],
    "human QA checklist must expose world promotion validation",
)
check_all(
    performance_budget,
    [
        "WorldPromotionValidation",
        "Promoted city modules, PCG/PLA/HLOD outputs, Data Layer migrations, and imported environment packs",
        "streaming budgets",
    ],
    "performance budget must include the world promotion validation row",
)
check_all(
    mac_budget,
    [
        "Static city modules and interiors",
        "authored fallback coverage",
        "verify_mac_asset_import_budget_gate_slice_pass.py plus verify_world_promotion_validation_contract_pass.py",
    ],
    "Mac budget gate must route static city modules through this validation slice",
)
check_all(
    visual_targets,
    [
        "WorldPromotionValidation",
        "Houdini/PCG city design cells",
        "city-module, PCG, human-scale, and streaming validation",
    ],
    "visual regression targets must include the world promotion validation surface",
)
check("verify_world_promotion_validation_contract_pass.py" in full_qa,
      "full QA must run the static world promotion verifier")
check("verify_world_promotion_validation_unreal.py" in full_qa,
      "full QA must run the Unreal-side world promotion smoke")
check("verify_world_promotion_validation_contract_pass.py" in local_ci,
      "local CI must run the static world promotion verifier")
check_all(
    slice_doc,
    [
        "World Promotion Validation Slice",
        "UCodeRescueWorldPromotionValidator",
        "CodeRescueWorldPromotionValidator",
        "code_rescue_world_promotion_validation.json",
        "World Partition",
        "PCG",
        "Packed Level Actor",
        "HLOD",
        "Data Layer",
        "simple collision",
        "WORLD_DEVELOPMENT_DEEPDIVE",
    ],
    "slice documentation must explain the world validator and validation boundary",
)
check_all(
    world_doc,
    [
        "World Partition",
        "Data Layers",
        "Packed Level Actors",
        "PCG",
        "trim sheet",
        "Master material",
        "HLODs and instancing",
        "streaming/memory budgets",
        "Data Validation",
    ],
    "world deep dive must contain the validation and authored/PCG guidance this slice implements",
)
check_all(
    mac_asset_doc,
    [
        "static city modules",
        "trim sheets",
        "HLOD",
        "Data Validation",
    ],
    "Mac asset budget gate must remain aligned with the world validation slice",
)
check_all(
    feature_doc,
    [
        "Nanite hero geometry",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
    ],
    "Mac feature capability slice must remain available for world promotion validation",
)
check_all(
    editor_doc,
    [
        "city-kit outputs",
        "HLOD proxy",
        "Data Validation",
    ],
    "editor Data Validation docs must remain available for this validation layer",
)
check_all(
    progress,
    [
        "World promotion validation slice",
        "UCodeRescueWorldPromotionValidator",
        "verify_world_promotion_validation_unreal.py",
        "code_rescue_world_promotion_validation.json",
    ],
    "progress log must record the world promotion validation slice",
)
check("verify_world_promotion_validation_contract_pass.py" in static_self,
      "static verifier should be self-identifying")

if errors:
    for error in errors:
        print(f"[verify_world_promotion_validation_contract_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_world_promotion_validation_contract_pass] PASS")
