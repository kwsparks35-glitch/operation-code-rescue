"""
Unreal-side smoke check for the Code Rescue editor Data Validation contract.

Run from the project root through UnrealEditor-Cmd:

    UnrealEditor-Cmd CodeRescueUnreal.uproject -run=pythonscript \
        -script=Scripts/verify_code_rescue_data_validation_unreal.py \
        -unattended -NoSound -NullRHI
"""

from __future__ import annotations

import json
from pathlib import Path

import unreal


def project_root() -> Path:
    return Path(unreal.Paths.project_dir()).resolve()


def report_path() -> Path:
    return project_root() / "Saved/DataValidation/code_rescue_data_validation_contract.json"


def require_unreal_class(name: str):
    cls = getattr(unreal, name, None)
    if cls is None:
        raise RuntimeError(f"missing Unreal Python class binding: unreal.{name}")
    return cls


def main() -> None:
    unreal.log("[cr-data-validation] === Code Rescue Data Validation smoke START ===")
    manifest_class = require_unreal_class("CodeRescueAssetManifest")
    validator_class = require_unreal_class("CodeRescueAssetManifestValidator")

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_registry.search_all_assets(True)
    game_assets = asset_registry.get_assets_by_path("/Game", recursive=True)
    assets = [
        asset for asset in game_assets
        if str(asset.asset_class_path.asset_name) == "CodeRescueAssetManifest"
    ]
    asset_paths = [str(asset.package_name) for asset in assets]

    output = {
        "generated_by": "verify_code_rescue_data_validation_unreal.py",
        "manifest_class": str(manifest_class.static_class().get_path_name()),
        "validator_class": str(validator_class.static_class().get_path_name()),
        "manifest_asset_count": len(asset_paths),
        "manifest_assets": asset_paths,
        "native_validator": "UEditorValidatorBase",
        "required_gate": "CodeRescueAssetManifestValidator",
    }
    path = report_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    unreal.log(
        f"[cr-data-validation] registered {output['validator_class']} for {output['manifest_class']}; "
        f"manifest assets={len(asset_paths)}"
    )
    unreal.log(f"[cr-data-validation] wrote {path}")
    unreal.log("[cr-data-validation] === Code Rescue Data Validation smoke PASSED ===")


main()
