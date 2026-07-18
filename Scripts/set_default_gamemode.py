"""
Set BP_CodeRescueGameMode as the project's default GameMode and verify the
data-table assignment landed on its CDO. After this runs, restart PIE — the
spawned ACodeRescueGameMode will be the BP subclass with ZombieVariantTable
already bound, so variant rows will actually drive zombie spawns.
"""

import unreal

DT_PATH = "/Game/CodeRescueAssets/DT_ZombieVariants"
BP_GM_PATH = "/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode"

log = unreal.log


def main():
    log("[zv-default] === START ===")

    bp = unreal.EditorAssetLibrary.load_asset(BP_GM_PATH)
    if bp is None:
        log(f"[zv-default] FATAL: BP not found at {BP_GM_PATH}")
        return

    gen_class = bp.generated_class()
    if gen_class is None:
        log("[zv-default] FATAL: BP has no generated class")
        return

    # Verify the table is bound on the BP CDO.
    bp_cdo = unreal.get_default_object(gen_class)
    bound = bp_cdo.get_editor_property("ZombieVariantTable")
    log(f"[zv-default] BP CDO ZombieVariantTable currently = {bound}")
    if bound is None:
        dt = unreal.load_asset(DT_PATH)
        bp_cdo.set_editor_property("ZombieVariantTable", dt)
        unreal.EditorAssetLibrary.save_loaded_asset(bp)
        log(f"[zv-default] Re-bound and saved {BP_GM_PATH}")

    # Stamp it into ProjectSettings.
    # UE 5.7 exposes UGameMapsSettings via unreal.GameMapsSettings.
    settings = unreal.get_default_object(unreal.GameMapsSettings)
    soft_class = unreal.SoftClassPath(gen_class.get_path_name())
    settings.set_editor_property("global_default_game_mode", soft_class)
    settings.save_config()
    log(f"[zv-default] global_default_game_mode = {soft_class}")
    log("[zv-default] === DONE — restart PIE to pick up the new default ===")


main()
