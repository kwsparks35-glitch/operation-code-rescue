"""
Unreal-side validation for the Fab/Unreal macOS MCP asset plan.

Run inside UnrealEditor-Cmd after `Run_Fab_Unreal_MCP_Audit.command` has written
Content/CodeRescueData/fab_unreal_mcp_asset_plan.json:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=PythonScript \
        -script=Scripts/mcp_fab_unreal_import_validate.py -unattended -NoSound -NullRHI
"""

from __future__ import annotations

import json
from pathlib import Path

import unreal


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve()


def saved_report_path() -> Path:
    return project_root() / "Saved/MCPFabUnreal/unreal_asset_validation_report.json"


def manifest_path() -> Path:
    return project_root() / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"


def load_manifest() -> dict:
    path = manifest_path()
    if not path.exists():
        raise RuntimeError(f"missing MCP Fab/Unreal manifest: {path}")
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def class_counts(assets) -> dict[str, int]:
    counts: dict[str, int] = {}
    for asset_data in assets:
        class_name = str(asset_data.asset_class_path.asset_name)
        counts[class_name] = counts.get(class_name, 0) + 1
    return dict(sorted(counts.items()))


def scan_game_root(asset_registry, root_name: str) -> dict:
    package_path = f"/Game/{root_name}"
    assets = asset_registry.get_assets_by_path(package_path, recursive=True)
    counts = class_counts(assets)
    sample = [str(asset.package_name) for asset in assets[:20]]
    return {
        "package_path": package_path,
        "asset_count": len(assets),
        "class_counts": counts,
        "sample_packages": sample,
    }


def validate_entry(asset_registry, entry: dict) -> dict:
    item = entry["item"]
    title = item["title"]
    presence = entry["project_presence"]
    roots = presence.get("present_content_roots") or []
    scans = [scan_game_root(asset_registry, root) for root in roots]
    asset_count = sum(scan["asset_count"] for scan in scans)
    traits = entry["analysis"].get("traits", {})
    warnings = []

    if presence.get("present") and asset_count == 0:
        warnings.append("manifest marks content present, but the Asset Registry returned no assets")
    if traits.get("has_character") and presence.get("present"):
        merged_counts: dict[str, int] = {}
        for scan in scans:
            for class_name, count in scan["class_counts"].items():
                merged_counts[class_name] = merged_counts.get(class_name, 0) + count
        if not any(name in merged_counts for name in ("SkeletalMesh", "Skeleton")):
            warnings.append("character-class item is present but no SkeletalMesh/Skeleton was found")
        if traits.get("has_animations") and "AnimSequence" not in merged_counts:
            warnings.append("animation-class item is present but no AnimSequence was found")
    if traits.get("has_native_code_or_plugin"):
        warnings.append("native/plugin item requires external Mac rebuild validation outside asset registry")

    return {
        "title": title,
        "verdict": entry["analysis"]["verdict"],
        "project_present": bool(presence.get("present")),
        "content_roots": roots,
        "root_scans": scans,
        "warnings": warnings,
    }


def main() -> None:
    unreal.log("[mcp-fab-validate] === MCP Fab/Unreal validation START ===")
    manifest = load_manifest()
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_registry.search_all_assets(True)

    entries = manifest.get("items", [])
    if not entries:
        raise RuntimeError("MCP Fab/Unreal manifest has no items")

    reports = [validate_entry(asset_registry, entry) for entry in entries]
    warning_count = sum(len(report["warnings"]) for report in reports)
    present_count = sum(1 for report in reports if report["project_present"])
    output = {
        "generated_by": "mcp_fab_unreal_import_validate.py",
        "manifest": str(manifest_path()),
        "item_count": len(reports),
        "project_present_count": present_count,
        "warning_count": warning_count,
        "reports": reports,
    }
    path = saved_report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    for report in reports:
        if report["project_present"]:
            unreal.log(
                f"[mcp-fab-validate] present {report['title']}: "
                f"roots={','.join(report['content_roots'])} warnings={len(report['warnings'])}"
            )
    unreal.log(f"[mcp-fab-validate] wrote {path}")
    unreal.log("[mcp-fab-validate] === MCP Fab/Unreal validation PASSED ===")


if __name__ == "__main__":
    main()

