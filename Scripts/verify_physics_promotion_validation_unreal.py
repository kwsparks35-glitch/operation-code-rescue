"""
Unreal-side smoke check for promoted physics validation.

Run from the project root through UnrealEditor-Cmd:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=pythonscript \
        -script=Scripts/verify_physics_promotion_validation_unreal.py \
        -unattended -NoSound -NullRHI
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import unreal


DT_PATH = "/Game/CodeRescueAssets/DT_ZombieVariants"
FALLBACK_ROWS = {"Default", "BaseMesh"}


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve()


def report_path() -> Path:
    return project_root() / "Saved/DataValidation/code_rescue_physics_promotion_validation.json"


def require_unreal_class(name: str):
    cls = getattr(unreal, name, None)
    if cls is None:
        raise RuntimeError(f"missing Unreal Python class binding: unreal.{name}")
    return cls


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def unwrap_path(value: Any) -> str:
    if value is None:
        return ""
    if isinstance(value, dict):
        for key in ("AssetPathName", "ObjectPath", "AssetPath", "Path", "ClassPathName"):
            if key in value:
                return unwrap_path(value[key])
        return ""
    text = str(value).strip()
    if text in {"", "None", "null"}:
        return ""
    if text.startswith("'") and text.endswith("'"):
        text = text[1:-1]
    return text


def generated_class_to_asset_path(path: str) -> str:
    if "." not in path:
        return path
    package, object_name = path.rsplit(".", 1)
    if object_name.endswith("_C"):
        object_name = object_name[:-2]
    return f"{package}.{object_name}"


def load_soft_path(path: str):
    if not path:
        return None
    try:
        loaded = unreal.load_object(None, path)
        if loaded:
            return loaded
    except Exception:
        pass
    try:
        return unreal.load_asset(generated_class_to_asset_path(path))
    except Exception:
        return None


def row_name(row: dict[str, Any]) -> str:
    return str(row.get("Name") or row.get("RowName") or "").strip()


def is_fallback_row(row: dict[str, Any]) -> bool:
    name = row_name(row)
    variant = str(row.get("Variant", ""))
    return name in FALLBACK_ROWS or "Default" in variant


def editor_property(obj: Any, *names: str):
    if obj is None:
        return None
    for name in names:
        try:
            value = obj.get_editor_property(name)
        except Exception:
            continue
        if value is not None:
            return value
    return None


def editor_property_len(obj: Any, *names: str) -> int:
    value = editor_property(obj, *names)
    if value is None:
        return 0
    try:
        return len(value)
    except Exception:
        return 0


def asset_registry():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.search_all_assets(True)
    return registry


def normalized_package_path(path: str) -> str:
    if not path:
        return ""
    if "." in path:
        return path.rsplit(".", 1)[0]
    return path


def asset_data_for_path(path: str):
    package_path = normalized_package_path(path)
    if not package_path:
        return None
    registry = asset_registry()
    for asset in registry.get_assets_by_path("/Game", recursive=True):
        if str(asset.package_name) == package_path:
            return asset
    return None


def asset_tag(asset_data: Any, key: str) -> str:
    if asset_data is None:
        return ""
    try:
        value = asset_data.get_tag_value(key)
        if isinstance(value, tuple):
            return str(value[1]) if value and value[0] else ""
        return str(value)
    except Exception:
        pass
    try:
        values = asset_data.tags_and_values
        for lookup_key in (key, unreal.Name(key)):
            try:
                if lookup_key in values:
                    return str(values[lookup_key])
            except Exception:
                continue
    except Exception:
        pass
    return ""


def int_tag(asset_data: Any, key: str) -> int:
    text = asset_tag(asset_data, key).strip()
    try:
        return int(text)
    except (TypeError, ValueError):
        return 0


def physics_asset_counts(path: str, loaded_asset: Any = None) -> tuple[int, int]:
    data = asset_data_for_path(path)
    body_count = int_tag(data, "Bodies")
    constraint_count = int_tag(data, "Constraints")
    if body_count <= 0 and loaded_asset is not None:
        body_count = editor_property_len(loaded_asset, "skeletal_body_setups", "SkeletalBodySetups")
    if constraint_count <= 0 and loaded_asset is not None:
        constraint_count = editor_property_len(loaded_asset, "constraint_setup", "ConstraintSetup")
    return body_count, constraint_count


def object_path(obj: Any) -> str:
    if obj is None:
        return ""
    try:
        return str(obj.get_path_name())
    except Exception:
        return str(obj)


def mesh_physics_asset(mesh: Any):
    return editor_property(mesh, "physics_asset", "PhysicsAsset")


def class_name(asset_data: Any) -> str:
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        return ""


def list_asset_paths(asset_class_name: str) -> list[str]:
    registry = asset_registry()
    game_assets = registry.get_assets_by_path("/Game", recursive=True)
    paths: list[str] = []
    for asset in game_assets:
        if class_name(asset) == asset_class_name:
            paths.append(str(asset.package_name))
    return sorted(paths)


def summarize_physics_asset(path: str) -> dict[str, Any]:
    asset = load_soft_path(path)
    body_count, constraint_count = physics_asset_counts(path, asset)
    return {
        "asset": path,
        "loaded": asset is not None,
        "body_count": body_count,
        "constraint_count": constraint_count,
    }


def promoted_zombie_physics_report() -> tuple[list[dict[str, Any]], list[str]]:
    errors: list[str] = []
    data_table = unreal.load_asset(DT_PATH)
    if data_table is None:
        return [], [f"missing required zombie variant DataTable: {DT_PATH}"]

    rows_json = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(data_table)
    rows = json.loads(rows_json) if rows_json else []
    if not isinstance(rows, list):
        return [], [f"unexpected DataTable JSON shape for {DT_PATH}: {type(rows).__name__}"]

    reports: list[dict[str, Any]] = []
    for row in rows:
        if is_fallback_row(row):
            continue

        name = row_name(row)
        mesh_path = unwrap_path(row.get("SkeletalMesh"))
        mesh = load_soft_path(mesh_path)
        physics_asset = mesh_physics_asset(mesh)
        physics_asset_path = object_path(physics_asset)
        body_count, constraint_count = physics_asset_counts(physics_asset_path, physics_asset)

        report = {
            "row": name,
            "skeletal_mesh": mesh_path,
            "mesh_loaded": mesh is not None,
            "physics_asset": physics_asset_path,
            "physics_asset_body_count": body_count,
            "physics_asset_constraint_count": constraint_count,
            "ragdoll_promotion_ready": bool(mesh and physics_asset and body_count >= 6 and constraint_count > 0),
        }
        reports.append(report)

        if not mesh:
            errors.append(f"{name}: promoted zombie mesh does not load: {mesh_path}")
        if not physics_asset:
            errors.append(f"{name}: promoted zombie mesh has no Physics Asset")
        if physics_asset and body_count < 6:
            errors.append(f"{name}: promoted zombie Physics Asset needs at least 6 bodies, found {body_count}")
        if physics_asset and constraint_count <= 0:
            errors.append(f"{name}: promoted zombie Physics Asset has no constraints")

    if not reports:
        errors.append(f"{DT_PATH} has no promoted non-fallback zombie rows for physics validation")

    return reports, errors


def source_contract_report() -> tuple[dict[str, bool], list[str]]:
    root = project_root()
    files = {
        "config": read_text(root / "Config/DefaultEngine.ini"),
        "barricade_h": read_text(root / "Source/CodeRescueUnreal/BarricadeActor.h"),
        "barricade_cpp": read_text(root / "Source/CodeRescueUnreal/BarricadeActor.cpp"),
        "zombie_cpp": read_text(root / "Source/CodeRescueUnreal/CodeZombieActor.cpp"),
    }
    checks = {
        "substepping_enabled": "bSubstepping=True" in files["config"],
        "max_substep_delta_60hz": "MaxSubstepDeltaTime=0.016667" in files["config"],
        "physical_surfaces_defined": all(
            token in files["config"]
            for token in ("Concrete", "Metal", "Wood", "Glass", "Flesh", "Dirt")
        ),
        "custom_trace_channels_defined": all(
            token in files["config"]
            for token in ("WeaponTrace", "AISightTrace", "InteractionTrace")
        ),
        "debris_sleep_disable_runtime": all(
            token in files["barricade_h"] + files["barricade_cpp"]
            for token in (
                "DebrisSleepDisableDelay",
                "ScheduleDebrisSleepDisable",
                "PutRigidBodyToSleep",
                "SetSimulatePhysics(false)",
                "ChaosDebrisSleepDisableFallback",
                "ChaosDebrisSleepDisabled",
            )
        ),
        "ragdoll_asset_gate_runtime": all(
            token in files["zombie_cpp"]
            for token in (
                "CodeRescueMaxActiveRagdollCorpses",
                "GetPhysicsAsset()",
                "SetAllBodiesSimulatePhysics(true)",
                "SetCollisionProfileName(TEXT(\"Ragdoll\"))",
            )
        ),
    }
    errors = [name for name, passed in checks.items() if not passed]
    return checks, errors


def main() -> None:
    unreal.log("[cr-physics-validation] === Physics promotion validation smoke START ===")
    validator_class = require_unreal_class("CodeRescuePhysicsPromotionValidator")

    zombie_reports, zombie_errors = promoted_zombie_physics_report()
    source_checks, source_errors = source_contract_report()
    physics_asset_paths = list_asset_paths("PhysicsAsset")
    geometry_collection_paths = list_asset_paths("GeometryCollection")
    sampled_physics_assets = [
        summarize_physics_asset(path)
        for path in physics_asset_paths
        if any(token in path for token in ("UrbanZombie4", "Zombie", "DogZombie"))
    ]

    errors = []
    errors.extend(zombie_errors)
    errors.extend(f"source contract failed: {name}" for name in source_errors)
    for asset_report in sampled_physics_assets:
        if not asset_report["loaded"]:
            errors.append(f"Physics Asset did not load: {asset_report['asset']}")
        elif asset_report["body_count"] <= 0:
            errors.append(f"Physics Asset has no bodies: {asset_report['asset']}")

    output = {
        "generated_by": "verify_physics_promotion_validation_unreal.py",
        "validator_class": str(validator_class.static_class().get_path_name()),
        "native_validator": "UEditorValidatorBase",
        "zombie_variant_table": DT_PATH,
        "promoted_zombie_rows": zombie_reports,
        "physics_asset_count": len(physics_asset_paths),
        "sampled_physics_assets": sampled_physics_assets,
        "geometry_collection_count": len(geometry_collection_paths),
        "geometry_collections": geometry_collection_paths,
        "source_checks": source_checks,
        "errors": errors,
    }

    path = report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            unreal.log_error(f"[cr-physics-validation] {error}")
        raise RuntimeError(f"physics promotion validation failed with {len(errors)} error(s); see {path}")

    unreal.log(
        f"[cr-physics-validation] validated {len(zombie_reports)} promoted zombie physics rows, "
        f"{len(sampled_physics_assets)} sampled Physics Assets, "
        f"{len(geometry_collection_paths)} Geometry Collections via {output['validator_class']}"
    )
    unreal.log(f"[cr-physics-validation] wrote {path}")
    unreal.log("[cr-physics-validation] === Physics promotion validation smoke PASSED ===")


main()
