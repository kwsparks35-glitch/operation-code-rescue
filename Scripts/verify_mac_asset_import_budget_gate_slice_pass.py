#!/usr/bin/env python3
"""Static verifier for the Mac imported-asset budget gate slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
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


game_mode = read(SRC / "CodeRescueGameMode.cpp")
asset_gate_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")
feature_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_feature_capability_manifest.tsv")
hair_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_hair_compatibility_manifest.tsv")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
runtime_contracts = read(PROJECT_ROOT / "Scripts/verify_runtime_log_contracts.py")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md")
character_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
physics_doc = read(SOURCE_DOC_DIR / "GAME_PHYSICS_DEEPDIVE.md")
top_50_doc = read(SOURCE_DOC_DIR / "TOP_50_RECOMMENDATIONS_2026-06-25.md")

check_all(
    game_mode,
    [
        "LOD/texture/shader: Mac budget gates required before runtime promotion",
        "LOD audit",
        "Texture cap",
        "Shader trim",
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
        "Mac LOD/texture/shader asset budget gates",
    ],
    "game mode must expose imported-asset budget gates in the intake board and runtime breadcrumb",
)
check_all(
    asset_gate_manifest,
    [
        "Crowd zombie skeletal meshes",
        "LOD0-to-LOD3 chain",
        "lower-LOD bone influence caps",
        "animation URO",
        "Hero and survivor skeletal meshes",
        "Static city modules and interiors",
        "trim sheets",
        "master-material parameters",
        "Nanite hero detail and HLOD proxies",
        "Textures and material instances",
        "Block unreviewed 4K texture sets",
        "Shader, VFX, fog, and translucent effects",
        "shader complexity review",
        "Physics and destruction props",
        "MacPhysicsBudgetReviewGate",
    ],
    "Mac asset import budget manifest must cover LOD, texture, shader/VFX, HLOD, and physics gates",
)
check_all(
    feature_manifest,
    [
        "MacNonNaniteFallbackReady assets and LOD texture budget review",
        "Large generated cities and Nanite foliage require target-hardware profiling before promotion",
    ],
    "Mac feature manifest must point Nanite/PCG content back to LOD and profiling budget review",
)
check_all(
    hair_manifest,
    [
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
    ],
    "hair compatibility manifest must remain available for imported character budget review",
)
check_all(
    performance_budget,
    [
        "ImportedAssetBudgetGate",
        "Imported skeletal meshes, city modules, textures, shaders, VFX, and physics props",
        "Mac LOD, texture-memory, shader-complexity, and fallback review",
    ],
    "performance city-layer budget must include imported asset gate coverage",
)
check_all(
    creative_plan,
    [
        "LOD texture and shader budget pass",
        "trim sheets",
        "master materials",
        "shader complexity",
        "Data Validation",
        "verify_mac_asset_import_budget_gate_slice_pass.py plus verify_mac_feature_capability_gate_slice_pass.py",
    ],
    "creative inclusion plan must wire the performance pass to the imported-asset budget gate verifier",
)
check_all(
    human_qa,
    [
        "Mac asset import budget gate",
        "imported LOD/texture/shader-heavy content blocked until budget review passes",
    ],
    "human QA checklist must expose imported asset budget review",
)
check_all(
    visual_targets,
    [
        "MacAssetImportBudgetGate",
        "LOD audit",
        "Texture cap",
        "Shader trim",
        "budget-gated before runtime promotion",
    ],
    "visual regression targets must include the imported asset budget board",
)
check_all(
    runtime_contracts,
    [
        "[CodeRescueCreativeImplementation]",
        "Mac LOD/texture/shader asset budget gates",
    ],
    "runtime log contract must require the imported-asset budget marker",
)
check("verify_mac_asset_import_budget_gate_slice_pass.py" in full_qa,
      "full QA must run the Mac asset import budget gate verifier")
check("verify_mac_asset_import_budget_gate_slice_pass.py" in local_ci,
      "local CI must run the Mac asset import budget gate verifier")
check_all(
    slice_doc,
    [
        "Mac Asset Import Budget Gate Slice",
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
        "MacPhysicsBudgetReviewGate",
        "Skeletal Mesh Reduction Tool",
        "trim sheets",
        "HLOD",
        "Data Validation",
        "packaged render smoke",
    ],
    "slice documentation must explain the import budget gate implementation and validation boundary",
)
check_all(
    character_doc,
    [
        "Skeletal Mesh Setup, LODs, Skin Weights, and Apple-GPU Performance",
        "Skeletal Mesh Reduction Tool",
        "Max Triangle Count",
        "Max Bones Influence",
        "LOD0 hero detail down to a heavily reduced LOD3",
        "Performance budgets",
        "Data Validation",
    ],
    "character deep dive must contain the LOD and validation guidance this slice implements",
)
check_all(
    world_doc,
    [
        "trim sheet",
        "Master material",
        "Nanite for hero detail",
        "HLODs and instancing",
        "Data Validation",
        "streaming/memory budgets",
    ],
    "world deep dive must contain the texture/material/HLOD/Data Validation guidance this slice implements",
)
check_all(
    physics_doc,
    [
        "Performance budgeting",
        "piece counts low",
        "Sleep/Disable Fields",
        "Data Validation",
    ],
    "physics deep dive must contain the physics budget and validation guidance this slice implements",
)
check_all(
    top_50_doc,
    [
        "Performance budgets on Apple Silicon",
        "HLODs",
        "instancing",
        "LODs",
    ],
    "top 50 recommendations must contain the Apple Silicon performance budget recommendation",
)
check_all(
    progress,
    [
        "Mac asset import budget gate slice",
        "MacLODBudgetReviewGate",
        "MacTextureMemoryReviewGate",
        "MacShaderComplexityReviewGate",
    ],
    "progress log must record the Mac asset import budget gate slice",
)

if errors:
    for error in errors:
        print(f"[verify_mac_asset_import_budget_gate_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_mac_asset_import_budget_gate_slice_pass] PASS")
