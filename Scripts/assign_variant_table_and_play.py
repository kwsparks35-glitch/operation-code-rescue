"""
Assign /Game/CodeRescueAssets/DT_ZombieVariants to
ACodeRescueGameMode::ZombieVariantTable and start PIE.

Two assignment paths attempted in order:
  1. Patch the C++ class CDO via set_editor_property (works for current
     editor session; does NOT persist across editor restart since CDOs
     can't be re-serialized from Python without a Blueprint shell).
  2. If a Blueprint subclass BP_CodeRescueGameMode exists at
     /Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode, set the
     property on its CDO and save it (this DOES persist).

For PIE: invoke the editor's Play-In-Editor function. The user can also
just press Alt+P / click the Play button in the toolbar.
"""

import unreal

DT_PATH = "/Game/CodeRescueAssets/DT_ZombieVariants"
GM_CLASS_PATH = "/Script/CodeRescueUnreal.CodeRescueGameMode"
BP_GM_PATH = "/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode"

log = unreal.log


def main():
    log("[zv-assign] === START ===")
    dt = unreal.load_asset(DT_PATH)
    if dt is None:
        log(f"[zv-assign] FATAL: data table not found at {DT_PATH}")
        return
    log(f"[zv-assign] loaded {DT_PATH}")

    # --- Path 1: patch the C++ CDO. Always works for the current session.
    gm_class = unreal.load_class(None, GM_CLASS_PATH)
    if gm_class is None:
        log(f"[zv-assign] FATAL: GameMode class not found at {GM_CLASS_PATH}")
        return
    cdo = unreal.get_default_object(gm_class)
    cdo.set_editor_property("ZombieVariantTable", dt)
    log(f"[zv-assign] CDO patched: {gm_class.get_name()}.ZombieVariantTable = "
        f"{dt.get_path_name()}")

    # --- Path 2: also save it on a Blueprint subclass for persistence.
    asset_lib = unreal.EditorAssetLibrary
    if not asset_lib.does_asset_exist(BP_GM_PATH):
        log(f"[zv-assign] no BP subclass at {BP_GM_PATH} — creating one")
        bp_dir = "/Game/CodeRescueAssets/Blueprints"
        if not asset_lib.does_directory_exist(bp_dir):
            asset_lib.make_directory(bp_dir)
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", gm_class)
        bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name="BP_CodeRescueGameMode",
            package_path=bp_dir,
            asset_class=unreal.Blueprint,
            factory=factory,
        )
    else:
        bp = asset_lib.load_asset(BP_GM_PATH)

    if bp is not None:
        # Generated class is what we mutate; the .uasset on disk is the
        # Blueprint; its CDO is the one whose properties get serialized.
        gen_class = bp.generated_class()
        if gen_class is not None:
            bp_cdo = unreal.get_default_object(gen_class)
            bp_cdo.set_editor_property("ZombieVariantTable", dt)
            unreal.EditorAssetLibrary.save_loaded_asset(bp)
            log(f"[zv-assign] saved {BP_GM_PATH} with ZombieVariantTable bound")
        else:
            log("[zv-assign] WARNING: generated_class() returned None; "
                "Blueprint may not be fully compiled yet")
    else:
        log("[zv-assign] WARNING: failed to create or load BP subclass")

    log("[zv-assign] === DONE ===")
    log("[zv-assign] To make BP the project's default GameMode permanently:")
    log("[zv-assign]   Edit → Project Settings → Maps & Modes → "
        "Default GameMode → BP_CodeRescueGameMode")


main()
