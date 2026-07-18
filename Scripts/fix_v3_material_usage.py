# fix_v3_material_usage.py — packaged-build material contract for CharactersV3.
#
# The 2026-07-02 lesson: a material missing a usage flag renders as the engine
# DEFAULT MATERIAL in cooked builds (editor -game only warns + recompiles).
# The V3 character FBX imports produced plain Materials that are used on a
# morph-target skeletal mesh, so they need used_with_skeletal_mesh AND
# used_with_morph_targets. Idempotent; safe to run any time.
import unreal

FIXED, SKIPPED = [], []
for root in ("/Game/CodeRescueArt/CharactersV3",):
    for ap in unreal.EditorAssetLibrary.list_assets(root, recursive=True):
        asset = unreal.EditorAssetLibrary.load_asset(ap)
        if isinstance(asset, unreal.Material):
            changed = False
            for prop in ("used_with_skeletal_mesh", "used_with_morph_targets"):
                try:
                    if not asset.get_editor_property(prop):
                        asset.set_editor_property(prop, True)
                        changed = True
                except Exception as exc:  # property rename safety
                    unreal.log_error("[V3MatFix] {0}: {1} -> {2}".format(ap, prop, exc))
            if changed:
                unreal.EditorAssetLibrary.save_loaded_asset(asset)
                FIXED.append(str(ap))
            else:
                SKIPPED.append(str(ap))

unreal.log_error("[V3MatFix] fixed={0} already-ok={1}".format(len(FIXED), len(SKIPPED)))
for p in FIXED:
    unreal.log_error("[V3MatFix]   set usage flags on " + p)
