# diag_hero_asset.py — interrogate the imported SurvivorKennyV4 asset: bounds,
# LOD geometry, materials, skeleton — versus the known-good SurvivorKennyV3.
import unreal

for path in ("/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4",
             "/Game/CodeRescueArt/CharactersV3/SurvivorKennyV3/SurvivorKennyV3"):
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_error("[HeroDiag] MISSING " + path)
        continue
    b = mesh.get_bounds()
    box = b.box_extent
    mats = mesh.get_editor_property("materials")
    mat_names = []
    for m in mats:
        mi = m.get_editor_property("material_interface")
        mat_names.append(mi.get_name() if mi else "NULL")
    try:
        num_lods = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem).get_lod_count(mesh)
    except Exception:
        num_lods = -1
    try:
        verts = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem).get_num_verts(mesh, 0)
    except Exception:
        verts = -1
    unreal.log_error("[HeroDiag] {0}: sphere_r={1:.1f} box=({2:.0f},{3:.0f},{4:.0f}) lods={5} lod0_verts={6} mats={7}".format(
        mesh.get_name(), b.sphere_radius, box.x, box.y, box.z, num_lods, verts, ",".join(mat_names)))
