#!/usr/bin/env python3
"""Static verifier for the Mac Nanite/SM6 feature-capability gate slice."""

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


def section(source: str, header: str) -> str:
    start = source.find(header)
    if start < 0:
        errors.append(f"missing section {header}")
        return ""
    next_header = source.find("\n[", start + len(header))
    return source[start:] if next_header < 0 else source[start:next_header]


default_engine = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
renderer_settings = section(default_engine, "[/Script/Engine.RendererSettings]")
default_game = read(PROJECT_ROOT / "Config/DefaultGame.ini")
game_mode = read(SRC / "CodeRescueGameMode.cpp")
feature_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_feature_capability_manifest.tsv")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
runtime_contracts = read(PROJECT_ROOT / "Scripts/verify_runtime_log_contracts.py")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MAC_FEATURE_CAPABILITY_GATE_SLICE.md")
character_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")

check_all(
    renderer_settings,
    [
        "MacNaniteSM6ReviewGate",
        "M1",
        "SM6 requires newer macOS",
        "r.Shadow.Virtual.Enable=1",
        "r.RayTracing=False",
        "Runtime play",
    ],
    "renderer settings must document the Mac Nanite/SM6 review gate and RT/VSM defaults",
)
check_all(
    game_mode,
    [
        "Nanite/SM6: M2+ and macOS 15+ review; non-Nanite fallback required",
        "Nanite SM6",
        "Fallback LOD",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "Nanite/SM6 review gates",
        "Non-Nanite",
        "r.Shadow.Virtual.Enable 0",
    ],
    "game mode must expose the Nanite/SM6 gate in-world and disable VSM for runtime fallback play",
)
check(game_mode.count("r.Shadow.Virtual.Enable 0") >= 2,
      "both launch-language scene and gameplay world must disable VSMs at runtime")
check("/Game/Nanite" not in game_mode,
      "runtime C++ must not hard-reference a Nanite content path")
check("/Game/Nanite" not in default_game,
      "packaging settings must not force-cook a Nanite-only content path")
check_all(
    feature_manifest,
    [
        "SM6 renderer",
        "M2-or-newer",
        "macOS 15+",
        "Nanite hero geometry",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "Virtual Shadow Maps",
        "Runtime play disables VSMs",
        "Lumen hardware ray tracing",
    ],
    "Mac feature capability manifest must classify SM6, Nanite, VSM, Lumen, and PCG/Fab gates",
)
check_all(
    performance_budget,
    [
        "MacFeatureProfile",
        "Nanite/SM6 and VSM remain promotion-gated for Mac",
        "non-Nanite fallbacks",
        "disables VSMs",
    ],
    "performance budget manifest must include Mac feature gate coverage",
)
check_all(
    creative_plan,
    [
        "LOD texture and shader budget pass",
        "Nanite review",
        "verify_mac_feature_capability_gate_slice_pass.py plus verify_mac_compatibility_validation_unreal.py plus verify_mac_rendering_aa_readiness_slice_pass.py plus package smoke",
    ],
    "creative inclusion plan must wire the performance pass to the Mac feature gate verifier",
)
check_all(
    human_qa,
    [
        "Mac Nanite/SM6 feature gate",
        "Nanite/VSM content held behind fallback review",
    ],
    "human QA checklist must expose the Mac feature gate",
)
check_all(
    visual_targets,
    [
        "MacFeatureCapabilityGate",
        "Nanite SM6",
        "Fallback LOD",
        "non-Nanite fallback required",
    ],
    "visual regression targets must include the feature capability gate",
)
check_all(
    runtime_contracts,
    [
        "[CodeRescueCreativeImplementation]",
        "Nanite/SM6 review gates",
    ],
    "runtime log contract must require the Nanite/SM6 review gate marker",
)
check("verify_mac_feature_capability_gate_slice_pass.py" in full_qa,
      "full QA must run the Mac feature capability verifier")
check("verify_mac_feature_capability_gate_slice_pass.py" in local_ci,
      "local CI must run the Mac feature capability verifier")
check_all(
    slice_doc,
    [
        "Mac Feature Capability Gate Slice",
        "Nanite",
        "SM6",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
        "r.Shadow.Virtual.Enable 0",
        "packaged render smoke",
    ],
    "slice documentation must explain the Mac feature gate implementation and validation",
)
check_all(
    character_doc,
    [
        "Nanite and SM6",
        "SM6 requires macOS 15.x+",
        "M1 hardware is not supported for Nanite",
    ],
    "character deep dive must contain the Nanite/SM6 guidance this slice implements",
)
check_all(
    world_doc,
    [
        "Nanite for hero detail",
        "Budget VSM and Lumen aggressively",
        "validate frame time on the actual development Mac",
    ],
    "world deep dive must contain the Nanite/VSM budgeting guidance this slice implements",
)
check_all(
    progress,
    [
        "Mac feature capability gate slice",
        "MacNaniteSM6ReviewGate",
        "MacNonNaniteFallbackReady",
    ],
    "progress log must record the Mac feature capability gate slice",
)

if errors:
    for error in errors:
        print(f"[verify_mac_feature_capability_gate_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_mac_feature_capability_gate_slice_pass] PASS")
