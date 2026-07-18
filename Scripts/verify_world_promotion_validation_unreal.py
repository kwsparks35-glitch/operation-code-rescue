"""
Unreal-side smoke check for promoted world/city validation.

Run from the project root through UnrealEditor-Cmd:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=pythonscript \
        -script=Scripts/verify_world_promotion_validation_unreal.py \
        -unattended -NoSound -NullRHI
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import unreal


WORLD_CANDIDATE_PREFIXES = (
    "/Game/World/",
    "/Game/CodeRescueAssets/World",
    "/Game/ModernBridges",
    "/Game/Parallax_Night_Building_Material",
)

STRICT_PROMOTION_TOKENS = (
    "WorldPromotion",
    "Promoted",
    "RuntimeReady",
    "CollisionRequired",
    "PlayerBlocker",
    "NavigationBlocker",
    "WalkableCityModule",
)


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve()


def report_path() -> Path:
    return project_root() / "Saved/DataValidation/code_rescue_world_promotion_validation.json"


def require_unreal_class(name: str):
    cls = getattr(unreal, name, None)
    if cls is None:
        raise RuntimeError(f"missing Unreal Python class binding: unreal.{name}")
    return cls


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def asset_registry():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.search_all_assets(True)
    return registry


def class_name(asset_data: Any) -> str:
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        return ""


def object_path_from_asset_data(asset_data: Any) -> str:
    package_name = str(asset_data.package_name)
    asset_name = str(asset_data.asset_name)
    return f"{package_name}.{asset_name}"


def asset_text(asset_data: Any) -> str:
    return " ".join(
        [
            str(asset_data.package_name),
            str(asset_data.asset_name),
            class_name(asset_data),
        ]
    )


def is_world_candidate_static_mesh(asset_data: Any) -> bool:
    return class_name(asset_data) == "StaticMesh" and str(asset_data.package_name).startswith(WORLD_CANDIDATE_PREFIXES)


def is_strict_promoted_asset(asset_data: Any) -> bool:
    text = asset_text(asset_data)
    return any(token in text for token in STRICT_PROMOTION_TOKENS)


def load_asset(asset_data: Any):
    try:
        return unreal.load_asset(object_path_from_asset_data(asset_data))
    except Exception:
        return None


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


def asset_tag(asset_data: Any, key: str) -> str:
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


def sequence_len(value: Any) -> int:
    if value is None:
        return 0
    try:
        return len(value)
    except Exception:
        return 0


def simple_collision_count(static_mesh: Any) -> int:
    body_setup = editor_property(static_mesh, "body_setup", "BodySetup")
    agg_geom = editor_property(body_setup, "agg_geom", "AggGeom")
    if agg_geom is None:
        return 0

    total = 0
    for name in (
        "box_elems",
        "sphere_elems",
        "sphyl_elems",
        "convex_elems",
        "tapered_capsule_elems",
        "level_set_elems",
        "skinned_level_set_elems",
    ):
        total += sequence_len(editor_property(agg_geom, name))
    return total


def material_slot_count(static_mesh: Any) -> int:
    static_materials = editor_property(static_mesh, "static_materials", "StaticMaterials")
    return sequence_len(static_materials)


def lod_count(asset_data: Any, static_mesh: Any) -> int:
    # Asset registry tags are cheaper and reliable for cooked editor metadata.
    for tag in ("LODs", "NumLODs", "LODCount"):
        value = int_tag(asset_data, tag)
        if value > 0:
            return value

    # Fallback to editor subsystems where available.
    try:
        subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
        return int(subsystem.get_lod_count(static_mesh))
    except Exception:
        pass

    source_models = editor_property(static_mesh, "source_models", "SourceModels")
    count = sequence_len(source_models)
    return count if count > 0 else 1


def summarize_static_mesh(asset_data: Any) -> dict[str, Any]:
    mesh = load_asset(asset_data)
    material_count = material_slot_count(mesh)
    simple_collision = simple_collision_count(mesh)
    l_count = lod_count(asset_data, mesh) if mesh else 0
    nanite_enabled = asset_tag(asset_data, "NaniteEnabled") or asset_tag(asset_data, "Nanite")
    strict = is_strict_promoted_asset(asset_data)
    return {
        "asset": str(asset_data.package_name),
        "loaded": mesh is not None,
        "strict_runtime_promotion": strict,
        "lod_count": l_count,
        "material_slot_count": material_count,
        "simple_collision_count": simple_collision,
        "nanite_enabled_tag": nanite_enabled,
        "promotion_ready": bool(mesh and material_count > 0 and l_count > 0 and (simple_collision > 0 or not strict)),
    }


def list_game_assets() -> list[Any]:
    registry = asset_registry()
    return list(registry.get_assets_by_path("/Game", recursive=True))


def framework_counts(assets: list[Any]) -> dict[str, int]:
    counts = {
        "map_world_assets": 0,
        "packed_level_actor_like_assets": 0,
        "level_instance_like_assets": 0,
        "hlod_like_assets": 0,
        "pcg_like_assets": 0,
        "data_layer_like_assets": 0,
    }
    for asset_data in assets:
        text = asset_text(asset_data)
        cls = class_name(asset_data)
        if cls == "World":
            counts["map_world_assets"] += 1
        if "PackedLevelActor" in text:
            counts["packed_level_actor_like_assets"] += 1
        if "LevelInstance" in text:
            counts["level_instance_like_assets"] += 1
        if "HLOD" in text:
            counts["hlod_like_assets"] += 1
        if "PCG" in text:
            counts["pcg_like_assets"] += 1
        if "DataLayer" in text:
            counts["data_layer_like_assets"] += 1
    return counts


def source_contract_report() -> tuple[dict[str, bool], list[str]]:
    root = project_root()
    game_mode = read_text(root / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp")
    game_mode_h = read_text(root / "Source/CodeRescueUnreal/CodeRescueGameMode.h")
    spawning = read_text(root / "Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp")
    editor_contract = read_text(root / "Content/CodeRescueData/editor_data_validation_contract.tsv")
    world_contract = read_text(root / "Content/CodeRescueData/world_promotion_validation_contract.tsv")
    perf_budget = read_text(root / "Content/CodeRescueData/performance_city_layer_budget.tsv")
    mac_budget = read_text(root / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")

    source = "\n".join([game_mode, game_mode_h, spawning])
    checks = {
        "native_validator_contract": all(
            token in editor_contract
            for token in (
                "UCodeRescueWorldPromotionValidator",
                "Scripts/verify_world_promotion_validation_unreal.py",
            )
        ),
        "pcg_world_partition_staging": all(
            token in source
            for token in (
                "HoudiniProceduralWorldDesign",
                "PCGWorldPartitionCell",
                "WorldPartitionReady",
                "PCGRouteSplineReady",
            )
        ),
        "district_kit_human_scale_review": all(
            token in source
            for token in (
                "MajorCityDistrictKit",
                "InteriorMissionSpaceReady",
                "HumanScaleBuildingProportion",
            )
        ),
        "authored_static_mesh_fallback_path": all(
            token in source
            for token in (
                "SpawnAuthoredPropsForCity",
                "SpawnStaticMeshProp",
                "LoadCodeRescueCityBuildingMesh",
                "LoadCodeRescueBridgeMesh",
                "SpawnBlock(Loc",
            )
        ),
        "streamed_actor_cleanup_path": all(
            token in source
            for token in (
                "EnsureCampaignCityLoaded",
                "ClearStreamedCampaignActors",
                "RegisterStreamedActor",
                "StreamedCampaignActors",
            )
        ),
        "lighting_weather_grade_path": all(
            token in source
            for token in (
                "ApplyUSCitySkyRealization",
                "SpawnPerZonePostProcessVolume",
                "SpawnWeatherForCity",
                "CodeRescueGrade_",
            )
        ),
        "mac_world_asset_budget_gate": all(
            token in source
            for token in (
                "MacNaniteSM6ReviewGate",
                "MacNonNaniteFallbackReady",
                "MacLODBudgetReviewGate",
                "MacTextureMemoryReviewGate",
                "MacShaderComplexityReviewGate",
            )
        ),
        "world_manifest_rows": all(
            token in world_contract
            for token in (
                "Authored city module Static Meshes",
                "PCG World Partition staging",
                "Packed Level Actor and HLOD kits",
                "Data Layer migration",
                "Human-scale collision and accessibility",
                "Apple Silicon streaming budget",
                "UCodeRescueWorldPromotionValidator",
            )
        ),
        "performance_budget_row": all(
            token in perf_budget
            for token in (
                "WorldPromotionValidation",
                "PCG/PLA/HLOD",
                "streaming budgets",
            )
        ),
        "mac_budget_row": "Static city modules and interiors" in mac_budget
            and "verify_world_promotion_validation_contract_pass.py" in mac_budget,
    }
    errors = [name for name, passed in checks.items() if not passed]
    return checks, errors


def main() -> None:
    unreal.log("[cr-world-validation] === World promotion validation smoke START ===")
    validator_class = require_unreal_class("CodeRescueWorldPromotionValidator")

    game_assets = list_game_assets()
    candidate_static_meshes = [asset for asset in game_assets if is_world_candidate_static_mesh(asset)]
    strict_static_meshes = [asset for asset in candidate_static_meshes if is_strict_promoted_asset(asset)]
    sampled_static_meshes = [summarize_static_mesh(asset) for asset in candidate_static_meshes[:40]]
    strict_reports = [summarize_static_mesh(asset) for asset in strict_static_meshes]
    counts = framework_counts(game_assets)
    source_checks, source_errors = source_contract_report()

    errors: list[str] = []
    errors.extend(f"source contract failed: {name}" for name in source_errors)
    for report in strict_reports:
        if not report["loaded"]:
            errors.append(f"strict world static mesh did not load: {report['asset']}")
        if report["material_slot_count"] <= 0:
            errors.append(f"strict world static mesh has no material slots: {report['asset']}")
        if report["lod_count"] <= 0:
            errors.append(f"strict world static mesh has no LOD data: {report['asset']}")
        if report["simple_collision_count"] <= 0:
            errors.append(f"strict world static mesh has no simple collision: {report['asset']}")

    output = {
        "generated_by": "verify_world_promotion_validation_unreal.py",
        "validator_class": str(validator_class.static_class().get_path_name()),
        "native_validator": "UEditorValidatorBase",
        "candidate_prefixes": WORLD_CANDIDATE_PREFIXES,
        "world_candidate_static_mesh_count": len(candidate_static_meshes),
        "strict_runtime_promoted_static_mesh_count": len(strict_static_meshes),
        "sampled_static_meshes": sampled_static_meshes,
        "strict_static_meshes": strict_reports,
        "framework_counts": counts,
        "source_checks": source_checks,
        "errors": errors,
    }

    path = report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            unreal.log_error(f"[cr-world-validation] {error}")
        raise RuntimeError(f"world promotion validation failed with {len(errors)} error(s); see {path}")

    unreal.log(
        f"[cr-world-validation] validated {len(candidate_static_meshes)} world mesh candidates, "
        f"{len(strict_static_meshes)} strict promoted meshes, "
        f"{counts['map_world_assets']} map assets via {output['validator_class']}"
    )
    unreal.log(f"[cr-world-validation] wrote {path}")
    unreal.log("[cr-world-validation] === World promotion validation smoke PASSED ===")


main()
