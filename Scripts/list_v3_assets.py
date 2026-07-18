# list_v3_assets.py — dump everything under /Game/CodeRescueArt/{CharactersV3,WeaponsV4}
# (log_error so lines always reach -stdout regardless of verbosity filtering)
import unreal

ar = unreal.AssetRegistryHelpers.get_asset_registry()
for root in ("/Game/CodeRescueArt/CharactersV3", "/Game/CodeRescueArt/WeaponsV4"):
    assets = ar.get_assets_by_path(root, recursive=True)
    unreal.log_error("[CRLIST] {0}: {1} assets".format(root, len(assets)))
    for a in assets:
        unreal.log_error("[CRLIST]   {0} ({1})".format(
            a.get_editor_property("object_path") if hasattr(a, "object_path") else a.package_name,
            a.asset_class_path.asset_name))
