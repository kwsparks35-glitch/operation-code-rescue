#!/usr/bin/env python3
"""Static verifier for the Mac rendering anti-aliasing readiness slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
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
        errors.append(f"missing config section {header}")
        return ""
    next_header = source.find("\n[", start + len(header))
    if next_header < 0:
        return source[start:]
    return source[start:next_header]


default_engine = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
renderer_settings = section(default_engine, "[/Script/Engine.RendererSettings]")
performance_budget = read(PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MAC_RENDERING_AA_READINESS_SLICE.md")
source_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")

check_all(
    renderer_settings,
    [
        "Apple Silicon / Metal readiness",
        "TSR hardware-limit pressure",
        "ghosting on fast-moving zombies",
        "2=TAA",
        "4=TSR",
        "r.TemporalAA.Upsampling=False",
        "r.AntiAliasingMethod=2",
    ],
    "renderer settings must document and enforce the Apple Silicon TAA baseline",
)
check("r.AntiAliasingMethod=4" not in renderer_settings,
      "renderer settings must not keep TSR as the packaged default")
check("r.TemporalAA.Upsampling=True" not in renderer_settings,
      "renderer settings must not keep temporal upsampling enabled for the packaged baseline")
check_all(
    performance_budget,
    [
        "RendererProfile",
        "Apple Silicon packaged default uses TAA",
        "temporal upsampling disabled",
        "TSR is kept out of the Mac runtime baseline",
    ],
    "performance city-layer budget must include the renderer profile row",
)
check_all(
    creative_plan,
    [
        "LOD texture and shader budget pass",
        "Keep imported assets smooth on Mac",
        "verify_mac_rendering_aa_readiness_slice_pass.py plus package smoke",
    ],
    "creative development inclusion plan must point the performance slice at the new verifier",
)
check_all(
    human_qa,
    [
        "Mac TAA renderer profile",
        "TSR excluded from the packaged Apple Silicon baseline",
    ],
    "human QA checklist must expose the Mac renderer profile review signal",
)
check_all(
    visual_targets,
    [
        "MacRenderingAAProfile",
        "Packaged render smoke",
        "Apple Silicon TAA baseline",
        "no TSR temporal upsampling",
        "TSR-style ghost trails",
    ],
    "visual regression targets must include the renderer AA profile",
)
check("verify_mac_rendering_aa_readiness_slice_pass.py" in full_qa,
      "full QA must run the Mac rendering AA readiness verifier")
check("verify_mac_rendering_aa_readiness_slice_pass.py" in local_ci,
      "local CI must run the Mac rendering AA readiness verifier")
check_all(
    slice_doc,
    [
        "Mac Rendering AA Readiness Slice",
        "r.AntiAliasingMethod=2",
        "r.TemporalAA.Upsampling=False",
        "Temporal Super Resolution",
        "Apple Silicon",
        "packaged render smoke",
    ],
    "slice documentation must explain the renderer change and validation boundary",
)
check_all(
    source_doc,
    [
        "Temporal Super Resolution (TSR)",
        "higher runtime cost",
        "ghosting on fast-moving zombies",
        "alternative AA",
    ],
    "June 25 source document must contain the AA guidance this slice implements",
)
check_all(
    progress,
    [
        "Mac rendering AA readiness slice",
        "r.AntiAliasingMethod=2",
        "r.TemporalAA.Upsampling=False",
    ],
    "progress log must record the Mac rendering AA readiness slice",
)

if errors:
    for error in errors:
        print(f"[verify_mac_rendering_aa_readiness_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_mac_rendering_aa_readiness_slice_pass] PASS")
