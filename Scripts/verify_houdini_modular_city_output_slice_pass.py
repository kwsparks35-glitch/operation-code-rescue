#!/usr/bin/env python3
"""Static verifier for the Houdini modular city output runtime slice."""

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


mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "houdini_modular_city_output_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
performance_budget = read(DATA / "performance_city_layer_budget.tsv")
world_contract = read(DATA / "world_promotion_validation_contract.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "HOUDINI_MODULAR_CITY_OUTPUT_SLICE.md")
world_doc = read(DOC_DIR / "WORLD_PROMOTION_VALIDATION_SLICE.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_houdini_modular_city_output_slice_pass.py")

systems_body = function_body(mode_cpp, "void ACodeRescueGameMode::SpawnUnrealSystemsCharacterWorldLayer")

check_all(
    systems_body,
    [
        "HOUDINI OUTPUT RECIPE",
        "HoudiniCitySeed",
        "Mission.ArtKitName",
        "Mission.DistrictStyle",
        "HoudiniModularCityOutput",
        "HoudiniCityKitRecipe",
        "PCGDeterministicCitySeed",
        "PCGStreamingBudgetCell",
        "FHoudiniOutputSpec",
    ],
    "runtime layer must expose deterministic Houdini output recipe data",
)
check_all(
    systems_body,
    [
        "FACADE KIT",
        "SAFEHOUSE CELL",
        "RUBBLE VARIATION",
        "COLLISION PROXY",
        "STREAMING CELL",
        "HoudiniFacadeModuleRecipe",
        "PCGSafehouseCellModule",
        "PCGRubbleVariationSet",
        "PCGCollisionProxyReady",
    ],
    "runtime layer must present modular output targets",
)
check_all(
    systems_body,
    [
        "Houdini Rubble Variant",
        "MacLODBudgetReviewGate",
        "Houdini Route Spline Knot",
        "PCGRouteSplineReady",
        "PCGWorldPartitionBakeReview",
        "NoAccessBlocker",
        "const FVector ChaosHub = PcgHub + FVector(0.0f, 1290.0f, 0.0f)",
    ],
    "runtime layer must add rubble, spline, and spacing coverage",
)

check_all(
    manifest,
    [
        "Houdini output recipe",
        "Facade kit cell",
        "Safehouse cell",
        "Rubble variation set",
        "Collision proxy lane",
        "Streaming budget cell",
        "Route spline knots",
        "World partition bake review",
        "runtime_review_ready",
    ],
    "Houdini modular output manifest must document every runtime review surface",
)
check("Houdini modular city output" in creative_plan, "creative plan must retain Houdini modular city output row")
check("verify_houdini_modular_city_output_slice_pass.py" in creative_plan, "creative plan must include the new Houdini verifier")
check("verify_world_promotion_validation_unreal.py" in creative_plan, "creative plan must preserve world promotion validation")
check("HoudiniModularCityOutput" in human_qa, "human QA checklist must include HoudiniModularCityOutput")
check("HoudiniModularCityOutput" in visual_targets, "visual regression targets must include HoudiniModularCityOutput")
check("HoudiniModularCityOutput" in performance_budget, "performance budget must include HoudiniModularCityOutput")
check("PCG World Partition staging" in world_contract, "world promotion contract must still include PCG World Partition staging")
check("verify_houdini_modular_city_output_slice_pass.py" in full_qa, "full QA audit must run the Houdini output verifier")
check("verify_houdini_modular_city_output_slice_pass.py" in local_ci, "local CI readiness must run the Houdini output verifier")
check("Houdini modular city output slice" in progress, "progress log must include the Houdini modular city output slice")
check("HOUDINI_MODULAR_CITY_OUTPUT_SLICE.md" in self_source, "verifier must check the slice documentation file")
check_all(
    slice_doc,
    [
        "Houdini Modular City Output Slice",
        "SpawnUnrealSystemsCharacterWorldLayer",
        "HOUDINI OUTPUT RECIPE",
        "houdini_modular_city_output_manifest.tsv",
        "PCGRouteSplineReady",
        "PCGWorldPartitionBakeReview",
        "Boundary",
    ],
    "slice documentation must explain runtime work, validation, and boundary",
)
check_all(
    world_doc,
    [
        "World Promotion Validation Slice",
        "UCodeRescueWorldPromotionValidator",
        "PCG",
        "World Partition",
    ],
    "world promotion validation docs must remain aligned with this generated-output slice",
)

if errors:
    for error in errors:
        print(f"[houdini-modular-city-output] ERROR: {error}")
    sys.exit(1)

print("[houdini-modular-city-output] OK")
