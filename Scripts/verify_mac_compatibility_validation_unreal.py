"""
Unreal-side smoke check for promoted Mac compatibility validation.

Run from the project root through UnrealEditor-Cmd:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=pythonscript \
        -script=Scripts/verify_mac_compatibility_validation_unreal.py \
        -unattended -NoSound -NullRHI
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import unreal


STRICT_PROMOTION_TOKENS = (
    "MacRuntime",
    "MacCompatible",
    "RuntimeReady",
    "Promoted",
    "MacHairCardRuntimeReady",
    "MacNonNaniteFallbackReady",
)


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve()


def report_path() -> Path:
    return project_root() / "Saved/DataValidation/code_rescue_mac_compatibility_validation.json"


def require_unreal_class(name: str):
    cls = getattr(unreal, name, None)
    if cls is None:
        raise RuntimeError(f"missing Unreal Python class binding: unreal.{name}")
    return cls


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def read_all_runtime_source() -> str:
    root = project_root() / "Source/CodeRescueUnreal"
    chunks: list[str] = []
    for path in sorted(root.glob("*.[ch]pp")) + sorted(root.glob("*.h")):
        chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(chunks)


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


def load_asset(asset_data: Any):
    try:
        return unreal.load_asset(object_path_from_asset_data(asset_data))
    except Exception:
        return None


def is_strict_promoted(asset_data: Any) -> bool:
    text = asset_text(asset_data)
    return any(token in text for token in STRICT_PROMOTION_TOKENS)


def has_fallback_token(text: str) -> bool:
    return any(token in text for token in ("Fallback", "Card", "MeshHair", "MacHairCardRuntimeReady", "MacNonNaniteFallbackReady"))


def is_groom_like(asset_data: Any) -> bool:
    text = asset_text(asset_data)
    return "Groom" in text or "/Game/Grooms" in text or "HairStrands" in text


def is_skeletal_candidate(asset_data: Any) -> bool:
    if class_name(asset_data) != "SkeletalMesh":
        return False
    text = asset_text(asset_data)
    return any(
        token in text
        for token in (
            "/Game/Characters",
            "/Game/YI_ModularZombies",
            "/Game/Zombie",
            "/Game/ZombieFemale",
            "/Game/DogZombie",
            "/Game/UrbanZombie4",
            "MetaHuman",
        )
    )


def is_static_candidate(asset_data: Any) -> bool:
    if class_name(asset_data) != "StaticMesh":
        return False
    text = asset_text(asset_data)
    return any(
        token in text
        for token in (
            "/Game/World/",
            "/Game/ModernBridges",
            "/Game/Parallax_Night_Building_Material",
            "/Game/CodeRescueAssets/World",
            "Nanite",
            "HLOD",
        )
    )


def is_material_candidate(asset_data: Any) -> bool:
    if class_name(asset_data) not in {"Material", "MaterialInstanceConstant", "MaterialFunction"}:
        return False
    text = asset_text(asset_data)
    return any(
        token in text
        for token in (
            "/Game/Parallax_Night_Building_Material",
            "/Game/ModernBridges",
            "/Game/World/",
            "/Game/Grooms",
            "Shader",
            "VFX",
        )
    )


def lod_count(asset_data: Any, loaded_asset: Any) -> int:
    for tag in ("LODs", "NumLODs", "LODCount"):
        value = int_tag(asset_data, tag)
        if value > 0:
            return value
    source_models = editor_property(loaded_asset, "source_models", "SourceModels")
    count = sequence_len(source_models)
    if count > 0:
        return count
    lod_info = editor_property(loaded_asset, "lod_info", "LODInfo")
    count = sequence_len(lod_info)
    return count if count > 0 else (1 if loaded_asset else 0)


def summarize_mesh(asset_data: Any) -> dict[str, Any]:
    loaded = load_asset(asset_data)
    text = asset_text(asset_data)
    l_count = lod_count(asset_data, loaded)
    strict = is_strict_promoted(asset_data)
    nanite_tag = asset_tag(asset_data, "NaniteEnabled") or asset_tag(asset_data, "Nanite")
    return {
        "asset": str(asset_data.package_name),
        "class": class_name(asset_data),
        "loaded": loaded is not None,
        "strict_runtime_promotion": strict,
        "lod_count": l_count,
        "nanite_enabled_tag": nanite_tag,
        "has_mac_fallback_token": has_fallback_token(text),
        "promotion_ready": bool(loaded and l_count > 0 and (not strict or l_count >= 2 or "Hero" in text)),
    }


def list_game_assets() -> list[Any]:
    registry = asset_registry()
    return list(registry.get_assets_by_path("/Game", recursive=True))


def source_contract_report() -> tuple[dict[str, bool], list[str]]:
    root = project_root()
    source = read_all_runtime_source()
    game_mode = read_text(root / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp")
    default_engine = read_text(root / "Config/DefaultEngine.ini")
    default_game = read_text(root / "Config/DefaultGame.ini")
    editor_contract = read_text(root / "Content/CodeRescueData/editor_data_validation_contract.tsv")
    mac_contract = read_text(root / "Content/CodeRescueData/mac_compatibility_validation_contract.tsv")
    hair_manifest = read_text(root / "Content/CodeRescueData/mac_hair_compatibility_manifest.tsv")
    feature_manifest = read_text(root / "Content/CodeRescueData/mac_feature_capability_manifest.tsv")
    asset_budget = read_text(root / "Content/CodeRescueData/mac_asset_import_budget_gate.tsv")

    checks = {
        "native_validator_contract": all(
            token in editor_contract
            for token in (
                "UCodeRescueMacCompatibilityValidator",
                "Scripts/verify_mac_compatibility_validation_unreal.py",
            )
        ),
        "mac_contract_rows": all(
            token in mac_contract
            for token in (
                "Groom strand review inputs",
                "Mac hair-card runtime fallback",
                "Nanite and SM6 world candidates",
                "Static and skeletal LOD budget",
                "Shader, VFX, and texture-memory review",
                "UCodeRescueMacCompatibilityValidator",
            )
        ),
        "hair_fallback_runtime_gate": all(
            token in game_mode
            for token in (
                "Mac hair-card fallback",
                "GroomStrandReviewOnlyMac",
                "MacHairCardRuntimeReady",
            )
        ),
        "nanite_vsm_runtime_gate": all(
            token in game_mode
            for token in (
                "MacNaniteSM6ReviewGate",
                "MacNonNaniteFallbackReady",
                "r.Shadow.Virtual.Enable 0",
            )
        ) and game_mode.count("r.Shadow.Virtual.Enable 0") >= 2,
        "import_budget_runtime_gate": all(
            token in game_mode
            for token in (
                "MacLODBudgetReviewGate",
                "MacTextureMemoryReviewGate",
                "MacShaderComplexityReviewGate",
            )
        ),
        "taa_mac_baseline": "r.AntiAliasingMethod=2" in default_engine and "r.AntiAliasingMethod=4" not in default_engine,
        "renderer_feature_review_config": all(
            token in default_engine
            for token in (
                "MacNaniteSM6ReviewGate",
                "r.Shadow.Virtual.Enable=1",
                "r.RayTracing=False",
            )
        ),
        "no_runtime_groom_hard_reference": "/Game/Grooms" not in source,
        "no_nanite_forced_cook": "/Game/Nanite" not in default_game and "/Game/Nanite" not in source,
        "hair_manifest_alignment": all(
            token in hair_manifest
            for token in ("GroomStrandReviewOnlyMac", "MacHairCardRuntimeReady", "card or mesh hair fallback")
        ),
        "feature_manifest_alignment": all(
            token in feature_manifest
            for token in ("MacNaniteSM6ReviewGate", "MacNonNaniteFallbackReady", "Runtime play disables VSMs")
        ),
        "asset_budget_alignment": all(
            token in asset_budget
            for token in ("MacLODBudgetReviewGate", "MacTextureMemoryReviewGate", "MacShaderComplexityReviewGate")
        ),
    }
    errors = [name for name, passed in checks.items() if not passed]
    return checks, errors


def filesystem_review_counts() -> dict[str, int]:
    root = project_root()
    return {
        "content_groom_uasset_count": len(list((root / "Content/Grooms").rglob("*.uasset"))) if (root / "Content/Grooms").exists() else 0,
        "metahuman_mhpkg_count": len(list((root / "MetaHuman_Downloads").glob("*.mhpkg"))),
        "groom_artsource_mhpkg_count": len(list((root / "Content/Grooms/ArtSource").glob("*.mhpkg"))),
    }


def main() -> None:
    unreal.log("[cr-mac-validation] === Mac compatibility validation smoke START ===")
    validator_class = require_unreal_class("CodeRescueMacCompatibilityValidator")

    game_assets = list_game_assets()
    groom_like_assets = [asset for asset in game_assets if is_groom_like(asset)]
    skeletal_candidates = [asset for asset in game_assets if is_skeletal_candidate(asset)]
    static_candidates = [asset for asset in game_assets if is_static_candidate(asset)]
    material_candidates = [asset for asset in game_assets if is_material_candidate(asset)]
    strict_assets = [asset for asset in groom_like_assets + skeletal_candidates + static_candidates if is_strict_promoted(asset)]

    sampled_skeletal = [summarize_mesh(asset) for asset in skeletal_candidates[:30]]
    sampled_static = [summarize_mesh(asset) for asset in static_candidates[:30]]
    strict_reports = [summarize_mesh(asset) for asset in strict_assets]
    source_checks, source_errors = source_contract_report()
    filesystem_counts = filesystem_review_counts()

    errors: list[str] = []
    errors.extend(f"source contract failed: {name}" for name in source_errors)
    for asset in groom_like_assets:
        text = asset_text(asset)
        if is_strict_promoted(asset) and not has_fallback_token(text):
            errors.append(f"strict Mac-promoted groom has no fallback token: {asset.package_name}")
    for report in strict_reports:
        if report["class"] in {"SkeletalMesh", "StaticMesh"}:
            if not report["loaded"]:
                errors.append(f"strict Mac asset did not load: {report['asset']}")
            if report["lod_count"] <= 0:
                errors.append(f"strict Mac asset has no LOD data: {report['asset']}")
            if "Nanite" in report["asset"] and not report["has_mac_fallback_token"]:
                errors.append(f"strict Nanite-like Mac asset has no fallback token: {report['asset']}")

    output = {
        "generated_by": "verify_mac_compatibility_validation_unreal.py",
        "validator_class": str(validator_class.static_class().get_path_name()),
        "native_validator": "UEditorValidatorBase",
        "groom_like_asset_count": len(groom_like_assets),
        "skeletal_candidate_count": len(skeletal_candidates),
        "static_candidate_count": len(static_candidates),
        "material_candidate_count": len(material_candidates),
        "strict_runtime_promoted_asset_count": len(strict_assets),
        "sampled_skeletal_candidates": sampled_skeletal,
        "sampled_static_candidates": sampled_static,
        "strict_assets": strict_reports,
        "filesystem_review_counts": filesystem_counts,
        "source_checks": source_checks,
        "errors": errors,
    }

    path = report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            unreal.log_error(f"[cr-mac-validation] {error}")
        raise RuntimeError(f"Mac compatibility validation failed with {len(errors)} error(s); see {path}")

    unreal.log(
        f"[cr-mac-validation] validated {len(groom_like_assets)} groom-like assets, "
        f"{len(skeletal_candidates)} skeletal candidates, {len(static_candidates)} static candidates, "
        f"{len(material_candidates)} material candidates via {output['validator_class']}"
    )
    unreal.log(f"[cr-mac-validation] wrote {path}")
    unreal.log("[cr-mac-validation] === Mac compatibility validation smoke PASSED ===")


main()
