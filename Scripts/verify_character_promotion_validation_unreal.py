"""
Unreal-side smoke check for promoted character asset validation.

Run from the project root through UnrealEditor-Cmd:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=pythonscript \
        -script=Scripts/verify_character_promotion_validation_unreal.py \
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
    return project_root() / "Saved/DataValidation/code_rescue_character_promotion_validation.json"


def require_unreal_class(name: str):
    cls = getattr(unreal, name, None)
    if cls is None:
        raise RuntimeError(f"missing Unreal Python class binding: unreal.{name}")
    return cls


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


def load_soft_path(path: str) -> bool:
    if not path:
        return False
    try:
        if unreal.load_object(None, path):
            return True
    except Exception:
        pass
    try:
        asset_path = generated_class_to_asset_path(path)
        return unreal.load_asset(asset_path) is not None
    except Exception:
        return False


def numeric(value: Any, default: float = 0.0) -> float:
    if isinstance(value, dict):
        for key in ("Value", "value", "FloatValue", "Number", "number"):
            if key in value:
                return numeric(value[key], default)
        if len(value) == 1:
            return numeric(next(iter(value.values())), default)
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def row_name(row: dict[str, Any]) -> str:
    return str(row.get("Name") or row.get("RowName") or "").strip()


def is_fallback_row(row: dict[str, Any]) -> bool:
    name = row_name(row)
    variant = str(row.get("Variant", ""))
    return name in FALLBACK_ROWS or "Default" in variant


def validate_rows(rows: list[dict[str, Any]]) -> tuple[list[dict[str, Any]], list[str]]:
    errors: list[str] = []
    summaries: list[dict[str, Any]] = []
    promoted_count = 0

    if not rows:
        errors.append(f"{DT_PATH} has no rows")

    for row in rows:
        name = row_name(row)
        fallback = is_fallback_row(row)
        if not fallback:
            promoted_count += 1

        display_name = str(row.get("DisplayName", "")).strip()
        mesh_path = unwrap_path(row.get("SkeletalMesh"))
        anim_path = unwrap_path(row.get("AnimBPClass"))

        if not name:
            errors.append("encountered a zombie variant row without a name")
        if not display_name:
            errors.append(f"{name}: DisplayName is required")

        for field_name in ("HealthMultiplier", "DamageMultiplier", "SpeedMultiplier", "MeshScale"):
            value = numeric(row.get(field_name), default=-1.0)
            if value < 0.1 or value > 5.0:
                errors.append(f"{name}: {field_name}={value} is outside 0.1..5.0")

        zone_weights = row.get("ZoneWeights") or {}
        normalized_zone_weights: dict[str, float] = {}
        if isinstance(zone_weights, dict):
            for key, value in zone_weights.items():
                try:
                    zone_index = int(key)
                except (TypeError, ValueError):
                    errors.append(f"{name}: ZoneWeights key {key!r} is not an integer")
                    continue
                zone_value = numeric(value, default=-1.0)
                normalized_zone_weights[str(zone_index)] = zone_value
                if zone_index < 0:
                    errors.append(f"{name}: ZoneWeights key {zone_index} cannot be negative")
                if zone_value < 0.0 or zone_value > 10.0:
                    errors.append(f"{name}: ZoneWeights[{zone_index}]={zone_value} is outside 0.0..10.0")
        elif isinstance(zone_weights, list):
            for entry in zone_weights:
                if not isinstance(entry, dict):
                    errors.append(f"{name}: ZoneWeights entry {entry!r} is not a key/value object")
                    continue
                key = entry.get("Key", entry.get("key"))
                value = entry.get("Value", entry.get("value"))
                try:
                    zone_index = int(key)
                except (TypeError, ValueError):
                    errors.append(f"{name}: ZoneWeights key {key!r} is not an integer")
                    continue
                zone_value = numeric(value, default=-1.0)
                normalized_zone_weights[str(zone_index)] = zone_value
                if zone_index < 0:
                    errors.append(f"{name}: ZoneWeights key {zone_index} cannot be negative")
                if zone_value < 0.0 or zone_value > 10.0:
                    errors.append(f"{name}: ZoneWeights[{zone_index}]={zone_value} is outside 0.0..10.0")

        if not fallback:
            if not mesh_path:
                errors.append(f"{name}: promoted zombie row requires SkeletalMesh")
            elif not load_soft_path(mesh_path):
                errors.append(f"{name}: SkeletalMesh does not load: {mesh_path}")
            if not anim_path:
                errors.append(f"{name}: promoted zombie row requires AnimBPClass")
            elif not load_soft_path(anim_path):
                errors.append(f"{name}: AnimBPClass does not load: {anim_path}")
        else:
            if mesh_path and not load_soft_path(mesh_path):
                errors.append(f"{name}: fallback SkeletalMesh is assigned but does not load: {mesh_path}")
            if anim_path and not load_soft_path(anim_path):
                errors.append(f"{name}: fallback AnimBPClass is assigned but does not load: {anim_path}")

        for label, path in (("SkeletalMesh", mesh_path), ("AnimBPClass", anim_path)):
            if "/Game/Grooms" in path or "GroomStrands" in path or "StrandGroom" in path:
                errors.append(f"{name}: {label} points at a Mac-incompatible strand groom path: {path}")

        summaries.append({
            "row": name,
            "variant": str(row.get("Variant", "")),
            "fallback_exception": fallback,
            "display_name": display_name,
            "skeletal_mesh": mesh_path,
            "anim_bp_class": anim_path,
            "zone_weights": normalized_zone_weights,
        })

    if promoted_count <= 0:
        errors.append(f"{DT_PATH} has no promoted non-fallback zombie variants")

    return summaries, errors


def main() -> None:
    unreal.log("[cr-character-validation] === Character promotion validation smoke START ===")
    validator_class = require_unreal_class("CodeRescueZombieVariantTableValidator")

    data_table = unreal.load_asset(DT_PATH)
    if data_table is None:
        raise RuntimeError(f"missing required zombie variant DataTable: {DT_PATH}")

    rows_json = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(data_table)
    rows = json.loads(rows_json) if rows_json else []
    if not isinstance(rows, list):
        raise RuntimeError(f"unexpected DataTable JSON shape for {DT_PATH}: {type(rows).__name__}")

    summaries, errors = validate_rows(rows)
    output = {
        "generated_by": "verify_character_promotion_validation_unreal.py",
        "validator_class": str(validator_class.static_class().get_path_name()),
        "native_validator": "UEditorValidatorBase",
        "data_table": DT_PATH,
        "row_count": len(rows),
        "promoted_row_count": sum(1 for row in rows if not is_fallback_row(row)),
        "fallback_rows": sorted(row["row"] for row in summaries if row["fallback_exception"]),
        "rows": summaries,
        "errors": errors,
    }

    path = report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            unreal.log_error(f"[cr-character-validation] {error}")
        raise RuntimeError(f"character promotion validation failed with {len(errors)} error(s); see {path}")

    unreal.log(
        f"[cr-character-validation] validated {output['row_count']} rows, "
        f"{output['promoted_row_count']} promoted variants via {output['validator_class']}"
    )
    unreal.log(f"[cr-character-validation] wrote {path}")
    unreal.log("[cr-character-validation] === Character promotion validation smoke PASSED ===")


main()
