#!/usr/bin/env python3
"""Static verifier for the Mac hair-card / groom compatibility slice."""

from __future__ import annotations

import json
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


def source_text() -> str:
    chunks: list[str] = []
    for path in sorted(SRC.glob("*.[ch]pp")) + sorted(SRC.glob("*.h")):
        chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(chunks)


game_source = source_text()
game_mode = read(SRC / "CodeRescueGameMode.cpp")
uproject_text = read(PROJECT_ROOT / "CodeRescueUnreal.uproject")
hair_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/mac_hair_compatibility_manifest.tsv")
systems_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/unreal_systems_character_world_manifest.tsv")
design_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/novel_character_world_design_manifest.tsv")
creative_plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
active_intake = read(PROJECT_ROOT / "Content/CodeRescueData/active_download_asset_intake_2026_05_28.tsv")
scanner = read(PROJECT_ROOT / "Scripts/scan_may28_active_asset_downloads.py")
human_qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual_targets = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MAC_HAIR_CARD_COMPATIBILITY_SLICE.md")
source_doc = read(SOURCE_DOC_DIR / "CHARACTER_ANIMATION_DEEPDIVE.md")

check_all(
    game_mode,
    [
        "Mac hair-card fallback",
        "Mac hair-card/mesh fallback",
        "strand grooms stay review-only on Apple GPUs",
        "Groom review",
        "Hair cards",
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "MetaHumanBodyRuntimeCandidate",
    ],
    "in-game character/intake surfaces must present the Mac hair-card fallback and groom review boundary",
)
check("/Game/Grooms" not in game_source,
      "runtime C++ must not hard-reference /Game/Grooms strand assets for packaged gameplay")
check_all(
    hair_manifest,
    [
        "groom_strand_review_only",
        "groom_mhpkg_review_only",
        "hair_mesh_runtime_candidate",
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "card or mesh hair fallback",
    ],
    "hair compatibility manifest must classify groom sources and Mac hair fallbacks",
)
check_all(
    systems_manifest,
    [
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "groom review-only assets",
        "Mac hair-card/mesh fallback assets",
        "verify_mac_hair_card_compatibility_slice_pass.py",
        "Strand grooms are not promoted to Mac runtime",
    ],
    "Unreal systems manifest must expose the Mac hair compatibility contract",
)
check_all(
    design_manifest,
    [
        "Mika Stone",
        "Field medic with Mac hair-card/mesh fallback",
        "strand groom review-only cue",
    ],
    "novel character manifest must identify Mika's Mac hair fallback role",
)
check_all(
    creative_plan,
    [
        "MetaHuman face and groom pass",
        "groom review assets",
        "Mac hair-card/mesh fallback assets",
        "verify_mac_hair_card_compatibility_slice_pass.py plus verify_may27_unreal_systems_character_world_pass.py",
    ],
    "creative development plan must point the MetaHuman/groom pass at the new verifier",
)
check_all(
    active_intake,
    [
        "Mac hair-card/mesh fallback",
        "groom strand review",
        "character_meta_human_or_groom",
    ],
    "active asset intake table must use the Mac hair/groom review wording",
)
check_all(
    scanner,
    [
        "Mac hair-card/mesh fallback",
        "groom strand review",
    ],
    "asset scanner must regenerate the new Mac hair/groom review wording",
)
check_all(
    human_qa,
    [
        "Mac hair-card/groom review intake",
        "strand groom sources are not presented as packaged Mac runtime hair",
    ],
    "human QA checklist must include hair compatibility review",
)
check_all(
    visual_targets,
    [
        "MacHairCompatibilityBoard",
        "Groom review",
        "Hair cards",
        "card/mesh hair reads as the Mac runtime path",
    ],
    "visual regression targets must include the Mac hair compatibility board",
)
check("verify_mac_hair_card_compatibility_slice_pass.py" in full_qa,
      "full QA must run the Mac hair compatibility verifier")
check("verify_mac_hair_card_compatibility_slice_pass.py" in local_ci,
      "local CI must run the Mac hair compatibility verifier")
check_all(
    slice_doc,
    [
        "Mac Hair-Card Compatibility Slice",
        "Content/Grooms",
        "MetaHuman_Downloads",
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
        "packaged render smoke",
    ],
    "slice documentation must explain the hair compatibility implementation and validation",
)
check_all(
    source_doc,
    [
        "Groom / hair strands are not supported on macOS",
        "hair cards and hair meshes are supported",
        "no macOS-incompatible Groom strand asset is referenced on the Mac target",
    ],
    "June 25 source document must contain the groom compatibility guidance this slice implements",
)
check_all(
    progress,
    [
        "Mac hair-card compatibility slice",
        "MacHairCardRuntimeReady",
        "GroomStrandReviewOnlyMac",
    ],
    "progress log must record the Mac hair compatibility slice",
)

groom_root = PROJECT_ROOT / "Content/Grooms"
groom_assets = list(groom_root.rglob("*.uasset")) if groom_root.exists() else []
project_mhpkg = list((PROJECT_ROOT / "MetaHuman_Downloads").glob("*.mhpkg"))
art_source_mhpkg = list((PROJECT_ROOT / "Content/Grooms/ArtSource").glob("*.mhpkg"))
check(len(groom_assets) > 0, "expected local Content/Grooms assets to be present for review-only classification")
check(len(project_mhpkg) + len(art_source_mhpkg) > 0,
      "expected local MetaHuman groom .mhpkg sources to be present for review-only classification")

try:
    uproject = json.loads(uproject_text)
    plugins = uproject.get("Plugins", [])
    hair_strands_enabled = any(
        plugin.get("Name") == "HairStrands" and plugin.get("Enabled") is True
        for plugin in plugins
        if isinstance(plugin, dict)
    )
    check(not hair_strands_enabled,
          "CodeRescueUnreal.uproject must not explicitly enable HairStrands as a gameplay dependency")
except json.JSONDecodeError as exc:
    errors.append(f"invalid CodeRescueUnreal.uproject json: {exc}")

if errors:
    for error in errors:
        print(f"[verify_mac_hair_card_compatibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print(
    "[verify_mac_hair_card_compatibility_slice_pass] PASS: "
    f"{len(groom_assets)} groom assets and {len(project_mhpkg) + len(art_source_mhpkg)} mhpkg sources classified"
)
