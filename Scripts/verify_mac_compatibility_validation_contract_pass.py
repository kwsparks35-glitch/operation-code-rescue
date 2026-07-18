#!/usr/bin/env python3
"""Static verifier for the Mac compatibility validation slice."""

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


def runtime_source_text() -> str:
    chunks: list[str] = []
    for path in sorted(RUNTIME_SRC.glob("*.[ch]pp")) + sorted(RUNTIME_SRC.glob("*.h")):
        chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(chunks)


validator_h = read(EDITOR_SRC / "Public/CodeRescueMacCompatibilityValidator.h")
validator_cpp = read(EDITOR_SRC / "Private/CodeRescueMacCompatibilityValidator.cpp")
game_mode_cpp = read(RUNTIME_SRC / "CodeRescueGameMode.cpp")
runtime_source = runtime_source_text()
default_engine = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
default_game = read(PROJECT_ROOT / "Config/DefaultGame.ini")
unreal_smoke = read(PROJECT_ROOT / "Scripts/verify_mac_compatibility_validation_unreal.py")
static_self = read(PROJECT_ROOT / "Scripts/verify_mac_compatibility_validation_contract_pass.py")
editor_contract = read(PROJECT_ROOT / "Content/CodeRescueData/editor_data_validation_contract.tsv")
mac_contract = read(PROJECT_ROOT / "Content/CodeRescueData/mac_compatibility_validation_contract.tsv")
hair_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_hair_compatibility_manifest.tsv")
feature_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_feature_capability_manifest.tsv")
asset_budget = read(PROJECT_ROOT / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MAC_COMPATIBILITY_VALIDATION_SLICE.md")
hair_doc = read(DOC_DIR / "MAC_HAIR_CARD_COMPATIBILITY_SLICE.md")
feature_doc = read(DOC_DIR / "MAC_FEATURE_CAPABILITY_GATE_SLICE.md")
asset_doc = read(DOC_DIR / "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md")
render_doc = read(DOC_DIR / "MAC_RENDERING_AA_READINESS_SLICE.md")
character_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")

check_all(
    validator_h,
    [
        "UCodeRescueMacCompatibilityValidator",
        "UEditorValidatorBase",
        "CanValidateAsset_Implementation",
        "ValidateLoadedAsset_Implementation",
    ],
    "Mac compatibility validator header must expose a native UEditorValidatorBase subclass",
)
check_all(
    validator_cpp,
    [
        "USkeletalMesh",
        "UStaticMesh",
        "UMaterialInterface",
        "Groom",
        "HairStrands",
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "GetLODNum",
        "GetNumLODs",
        "shader-complexity",
        "AssetMessage",
        "AssetPasses",
        "EDataValidationResult::Invalid",
    ],
    "Mac validator implementation must enforce groom, mesh, Nanite/fallback, and material promotion rules",
)
check_all(
    game_mode_cpp,
    [
        "Mac hair-card fallback",
        "GroomStrandReviewOnlyMac",
        "MacHairCardRuntimeReady",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
        "r.Shadow.Virtual.Enable 0",
    ],
    "runtime game mode must expose Mac hair, Nanite/fallback, budget, and VSM fallback gates",
)
check(game_mode_cpp.count("r.Shadow.Virtual.Enable 0") >= 2,
      "both launch-language scene and gameplay world must disable VSMs at runtime")
check("/Game/Grooms" not in runtime_source,
      "runtime C++ must not hard-reference /Game/Grooms strand assets for packaged gameplay")
check("/Game/Nanite" not in default_game and "/Game/Nanite" not in runtime_source,
      "runtime/package settings must not force Nanite-only content")
check_all(
    default_engine,
    [
        "r.AntiAliasingMethod=2",
        "MacNaniteSM6ReviewGate",
        "r.Shadow.Virtual.Enable=1",
        "r.RayTracing=False",
    ],
    "renderer config must keep TAA baseline and Mac feature review defaults",
)
check("r.AntiAliasingMethod=4" not in default_engine,
      "renderer config must not return TSR as the packaged Mac baseline")
check_all(
    unreal_smoke,
    [
        "CodeRescueMacCompatibilityValidator",
        "code_rescue_mac_compatibility_validation.json",
        "groom_like_asset_count",
        "skeletal_candidate_count",
        "static_candidate_count",
        "material_candidate_count",
        "strict_runtime_promoted_asset_count",
        "source_checks",
    ],
    "Unreal smoke must validate Mac compatibility surfaces and write a report",
)
check_all(
    editor_contract,
    [
        "UCodeRescueMacCompatibilityValidator",
        "Promoted MetaHuman, groom, Nanite, VSM, Lumen, shader-heavy content, texture-heavy imports, and Mac renderer fallback surfaces",
        "GroomStrandReviewOnlyMac",
        "Scripts/verify_mac_compatibility_validation_unreal.py",
    ],
    "editor Data Validation contract must list the current Mac validator",
)
check("FutureMacCompatibilityValidator" not in editor_contract,
      "Mac validator should no longer be listed as a future-only surface")
check_all(
    mac_contract,
    [
        "Groom strand review inputs",
        "Mac hair-card runtime fallback",
        "Nanite and SM6 world candidates",
        "Static and skeletal LOD budget",
        "Shader, VFX, and texture-memory review",
        "Renderer fallback contract",
        "UCodeRescueMacCompatibilityValidator",
        "verify_mac_compatibility_validation_unreal.py",
    ],
    "Mac compatibility manifest must cover current and future Mac-risk surfaces",
)
check_all(
    hair_manifest,
    [
        "GroomStrandReviewOnlyMac",
        "MacHairCardRuntimeReady",
        "card or mesh hair fallback",
    ],
    "hair compatibility manifest must align with the Mac validator",
)
check_all(
    feature_manifest,
    [
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "Runtime play disables VSMs",
    ],
    "Mac feature manifest must align with the Mac validator",
)
check_all(
    asset_budget,
    [
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
    ],
    "Mac asset budget manifest must align with the Mac validator",
)
check_all(
    creative_plan,
    [
        "Mac compatibility validation",
        "verify_mac_feature_capability_gate_slice_pass.py plus verify_mac_compatibility_validation_unreal.py plus verify_mac_rendering_aa_readiness_slice_pass.py plus package smoke",
    ],
    "creative plan must route Mac performance work through Mac compatibility validation",
)
check_all(
    human_qa,
    [
        "Mac compatibility validation",
        "groom/Nanite/shader Mac fallback gates",
    ],
    "human QA checklist must expose Mac compatibility validation",
)
check_all(
    performance_budget,
    [
        "MacCompatibilityValidation",
        "Groom, MetaHuman, Nanite/SM6, VSM/Lumen, shader-heavy, texture-heavy, skeletal, and static content",
        "packaged render-smoke validation",
    ],
    "performance budget must include the Mac compatibility validation row",
)
check_all(
    visual_targets,
    [
        "MacCompatibilityValidation",
        "Groom review, Hair cards, Nanite SM6, Fallback LOD, LOD audit, Texture cap, and Shader trim",
        "fallback-gated promotion",
    ],
    "visual regression targets must include the Mac compatibility validation surface",
)
check("verify_mac_compatibility_validation_contract_pass.py" in full_qa,
      "full QA must run the static Mac compatibility verifier")
check("verify_mac_compatibility_validation_unreal.py" in full_qa,
      "full QA must run the Unreal-side Mac compatibility smoke")
check("verify_mac_compatibility_validation_contract_pass.py" in local_ci,
      "local CI must run the static Mac compatibility verifier")
check_all(
    slice_doc,
    [
        "Mac Compatibility Validation Slice",
        "UCodeRescueMacCompatibilityValidator",
        "CodeRescueMacCompatibilityValidator",
        "code_rescue_mac_compatibility_validation.json",
        "GroomStrandReviewOnlyMac",
        "MacHairCardRuntimeReady",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "shader-complexity",
        "packaged render smoke",
    ],
    "slice documentation must explain the Mac compatibility validator and validation boundary",
)
check_all(
    hair_doc,
    [
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "packaged render smoke",
    ],
    "hair compatibility docs must remain aligned with the Mac validator",
)
check_all(
    feature_doc,
    [
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "r.Shadow.Virtual.Enable 0",
    ],
    "Mac feature docs must remain aligned with the Mac validator",
)
check_all(
    asset_doc,
    [
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
    ],
    "Mac asset budget docs must remain aligned with the Mac validator",
)
check_all(
    render_doc,
    [
        "r.AntiAliasingMethod=2",
        "TSR is no longer the default anti-aliasing method",
    ],
    "Mac rendering docs must remain aligned with the Mac validator",
)
check_all(
    character_doc,
    [
        "Groom / hair strands are not supported on macOS",
        "hair cards and hair meshes are supported",
        "Nanite and SM6",
    ],
    "character deep dive must contain Mac compatibility guidance this slice implements",
)
check_all(
    world_doc,
    [
        "Nanite for hero detail",
        "Budget VSM and Lumen aggressively",
        "validate frame time on the actual development Mac",
    ],
    "world deep dive must contain Mac world-feature guidance this slice implements",
)
check_all(
    progress,
    [
        "Mac compatibility validation slice",
        "UCodeRescueMacCompatibilityValidator",
        "verify_mac_compatibility_validation_unreal.py",
        "code_rescue_mac_compatibility_validation.json",
    ],
    "progress log must record the Mac compatibility validation slice",
)
check("verify_mac_compatibility_validation_contract_pass.py" in static_self,
      "static verifier should be self-identifying")

if errors:
    for error in errors:
        print(f"[verify_mac_compatibility_validation_contract_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_mac_compatibility_validation_contract_pass] PASS")
