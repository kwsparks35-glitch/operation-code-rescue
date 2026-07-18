#!/usr/bin/env python3
"""
Scan active Fab/MetaHuman download locations and record a safe intake table.

The scanner does not copy or import licensed assets. It classifies what is
visible on disk so the game can promote only assets that have source/license,
Mac, Unreal, and gameplay validation evidence.
"""

from __future__ import annotations

import json
import re
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
FAB_ROOT = Path("/Users/Shared/UnrealEngine/Launcher/VaultCache/FabLibrary")
METAHUMAN_ROOT = Path("/Users/labcomputer/UnrealEngine/MetaHuman_Downloads")
CONTENT_ROOT = PROJECT_ROOT / "Content"
OUT_TSV = PROJECT_ROOT / "Content/CodeRescueData/active_download_asset_intake_2026_05_28.tsv"
OUT_JSON = PROJECT_ROOT / "Saved/MCPFabUnreal/may28_active_asset_intake.json"


def slug_title(folder_name: str) -> str:
    title = re.sub(r"-[0-9a-f]{8}$", "", folder_name, flags=re.IGNORECASE)
    return title.replace("_", " ").strip()


def classify(title: str, files: list[Path]) -> tuple[str, str, str]:
    lower = title.lower()
    suffixes = {p.suffix.lower() for p in files}
    has_plugin = ".uplugin" in suffixes or "plugin" in lower or "library" in lower
    has_metahuman = ".mhpkg" in suffixes or "metahuman" in lower or "groom" in lower
    has_zombie = "zombie" in lower
    has_ai = "convai" in lower or "ai" in lower or "npc" in lower
    has_physics = "physics" in lower
    has_quest = "quest" in lower
    has_world = any(token in lower for token in ("bridge", "building", "interior", "city"))

    if has_ai:
        return (
            "ai_or_npc_plugin",
            "NPC dialogue/behavior experiments behind Mac plugin validation and feature flags",
            "plugin_source_mac_review_required",
        )
    if has_metahuman:
        return (
            "character_meta_human_or_groom",
            "MetaHuman body slots, Mac hair-card/mesh fallback, Control Rig, IK retarget, and groom strand review",
            "manual_mhpkg_or_dcc_materialization_required",
        )
    if has_zombie:
        return (
            "enemy_character_pack",
            "Combat-district zombie family variants, direct pursuit, damage tuning, and safe-zone exclusion",
            "import_or_retarget_then_validate",
        )
    if has_physics:
        return (
            "physics_plugin",
            "Async/Chaos physics stress rig and movable cover promotion",
            "plugin_source_mac_review_required",
        )
    if has_quest:
        return (
            "quest_mission_plugin",
            "Mission-board, survivor-intel, and quest kit experiments after Blueprint compile",
            "plugin_source_mac_review_required",
        )
    if has_world:
        return (
            "world_or_environment_pack",
            "Major-city district kits, interiors, bridges, storefronts, and human-scale collision",
            "content_import_validation_required",
        )
    if has_plugin:
        return (
            "plugin_or_library",
            "Unreal extended-library candidate behind compile and package validation",
            "plugin_source_mac_review_required",
        )
    return (
        "unclassified_download",
        "Manual review before gameplay use",
        "manual_review_required",
    )


def file_sample(root: Path) -> list[Path]:
    if not root.exists():
        return []
    matches: list[Path] = []
    for pattern in ("*.uplugin", "*.uasset", "*.umap", "*.mhpkg", "*.fbx", "*.abc", "*.usd", "*.json", "manifest"):
        matches.extend(root.rglob(pattern))
        if len(matches) >= 40:
            break
    return sorted(matches[:40])


def scan_fab() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    if not FAB_ROOT.exists():
        return rows
    for folder in sorted(p for p in FAB_ROOT.iterdir() if p.is_dir()):
        title = slug_title(folder.name)
        files = file_sample(folder)
        category, target, next_action = classify(title, files)
        status = "cache_visible"
        if any(p.suffix.lower() == ".mhpkg" for p in files):
            status = "metahuman_package_visible"
        elif any(p.name == "manifest" for p in files) and not any(p.suffix.lower() in {".uasset", ".uplugin", ".umap"} for p in files):
            status = "manifest_only_or_binary_cache"
        elif any(p.suffix.lower() == ".uplugin" for p in files):
            status = "plugin_candidate_visible"
        elif any(p.suffix.lower() in {".uasset", ".umap"} for p in files):
            status = "content_candidate_visible"
        rows.append(
            {
                "title": title,
                "status": status,
                "category": category,
                "source_path": str(folder),
                "file_evidence": ", ".join(sorted({p.suffix.lower() or p.name for p in files})[:8]) or "folder_only",
                "implementation_target": target,
                "next_action": next_action,
            }
        )
    return rows


def scan_project_content() -> list[dict[str, str]]:
    known_roots = [
        "YI_ModularZombies",
        "DogZombie",
        "Zombie",
        "ZombieFemale",
        "UrbanZombie4",
        "ModernBridges",
        "Parallax_Night_Building_Material",
        "Grooms",
        "StarterContent",
        "Characters",
        "CodeRescueAssets",
    ]
    rows = []
    for name in known_roots:
        path = CONTENT_ROOT / name
        if not path.exists():
            continue
        files = file_sample(path)
        category, target, _ = classify(name, files)
        rows.append(
            {
                "title": f"Project Content: {name}",
                "status": "already_in_project",
                "category": category,
                "source_path": str(path),
                "file_evidence": ", ".join(sorted({p.suffix.lower() or p.name for p in files})[:8]) or "folder_only",
                "implementation_target": target,
                "next_action": "active_or_ready_for_review_cell_validation",
            }
        )
    return rows


def scan_metahuman_staging() -> list[dict[str, str]]:
    rows = []
    if not METAHUMAN_ROOT.exists():
        return rows
    files = file_sample(METAHUMAN_ROOT)
    if files:
        rows.append(
            {
                "title": "MetaHuman Downloads Staging",
                "status": "local_staging_visible",
                "category": "character_meta_human_or_groom",
                "source_path": str(METAHUMAN_ROOT),
                "file_evidence": ", ".join(sorted({p.suffix.lower() or p.name for p in files})[:8]),
                "implementation_target": "MetaHuman playable/NPC cast slots, Mac hair-card/mesh fallback, and groom strand review",
                "next_action": "manual_mhpkg_or_dcc_materialization_required",
            }
        )
    return rows


def write_tsv(rows: list[dict[str, str]]) -> None:
    OUT_TSV.parent.mkdir(parents=True, exist_ok=True)
    header = [
        "title",
        "status",
        "category",
        "source_path",
        "file_evidence",
        "implementation_target",
        "next_action",
    ]
    lines = ["\t".join(header)]
    for row in rows:
        lines.append("\t".join(row.get(col, "").replace("\t", " ").replace("\n", " ") for col in header))
    OUT_TSV.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_json_report(rows: list[dict[str, str]]) -> None:
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(
        json.dumps(
            {
                "fab_root": str(FAB_ROOT),
                "metahuman_root": str(METAHUMAN_ROOT),
                "project_root": str(PROJECT_ROOT),
                "row_count": len(rows),
                "rows": rows,
            },
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )


def main() -> int:
    rows = scan_fab() + scan_metahuman_staging() + scan_project_content()
    rows = sorted(rows, key=lambda r: (r["status"], r["category"], r["title"]))
    write_tsv(rows)
    write_json_report(rows)
    print(f"[scan-may28-active-assets] rows={len(rows)}")
    print(f"[scan-may28-active-assets] wrote {OUT_TSV}")
    print(f"[scan-may28-active-assets] wrote {OUT_JSON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
