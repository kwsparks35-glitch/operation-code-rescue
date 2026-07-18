"""Wire up a downloaded MetaHuman as the survivor mesh.

This script runs INSIDE the UE editor's Python console after you've
downloaded a MetaHuman via Quixel Bridge (Window → Quixel Bridge →
MetaHumans → "Add" your chosen archetype).

Usage (inside UE editor):
    >>> import importlib, import_metahuman_survivor
    >>> importlib.reload(import_metahuman_survivor)
    >>> import_metahuman_survivor.run("Olivia")     # name from MetaHumans/

It locates the MetaHuman's Body skeletal mesh and AnimBP, then writes them
onto BP_SurvivorActor's ProfessionalSurvivorMesh / ProfessionalSurvivorAnimClass
properties. Saves the asset.

If the BP doesn't exist yet, prompt user to create it first via
right-click → Blueprint Class → ASurvivorActor.
"""

from __future__ import annotations

import unreal


SURVIVOR_BP_PATH = "/Game/CodeRescueAssets/BP_SurvivorActor"
METAHUMANS_ROOT = "/Game/MetaHumans"


def find_body_skel_mesh(name: str) -> unreal.SkeletalMesh | None:
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    path = f"{METAHUMANS_ROOT}/{name}"
    assets = asset_registry.get_assets_by_path(path, recursive=True)
    for ad in assets:
        if str(ad.asset_class_path.asset_name) != "SkeletalMesh":
            continue
        asset_name = str(ad.asset_name)
        if asset_name.endswith("_Body") or "BodyMesh" in asset_name:
            return unreal.load_asset(str(ad.package_name))
    return None


def find_animbp(name: str) -> unreal.Object | None:
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    path = f"{METAHUMANS_ROOT}/{name}"
    assets = asset_registry.get_assets_by_path(path, recursive=True)
    for ad in assets:
        cls = str(ad.asset_class_path.asset_name)
        if cls != "AnimBlueprint":
            continue
        asset_name = str(ad.asset_name)
        if "Body" in asset_name or "MetaHuman" in asset_name:
            return unreal.load_asset(str(ad.package_name))
    return None


def run(metahuman_name: str) -> None:
    bp = unreal.load_asset(SURVIVOR_BP_PATH)
    if bp is None:
        unreal.log_error(
            f"[import_metahuman_survivor] {SURVIVOR_BP_PATH} not found. "
            "Right-click in Content Browser → Blueprint Class → ASurvivorActor "
            "and save it under that path first."
        )
        return

    skel_mesh = find_body_skel_mesh(metahuman_name)
    if skel_mesh is None:
        unreal.log_error(
            f"[import_metahuman_survivor] No body SkeletalMesh found under "
            f"{METAHUMANS_ROOT}/{metahuman_name}. Did Quixel Bridge finish?"
        )
        return

    cdo = unreal.get_default_object(bp.generated_class())
    cdo.set_editor_property("ProfessionalSurvivorMesh", skel_mesh)
    unreal.log(f"[import_metahuman_survivor] Set ProfessionalSurvivorMesh = {skel_mesh.get_name()}")

    animbp = find_animbp(metahuman_name)
    if animbp is not None:
        cdo.set_editor_property("ProfessionalSurvivorAnimClass", animbp.generated_class())
        unreal.log(f"[import_metahuman_survivor] Set ProfessionalSurvivorAnimClass = {animbp.get_name()}")
    else:
        unreal.log_warning(
            f"[import_metahuman_survivor] Body mesh assigned but no AnimBP "
            "found — survivor will stand in T-pose. Create or assign one manually."
        )

    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    unreal.log("[import_metahuman_survivor] Done. Restart PIE to see the survivor.")


if __name__ == "__main__":
    unreal.log_error("Run this script from the UE editor's Python console, not standalone.")
