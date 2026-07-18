# diag_reimport.py — A/B isolation: run MY import method on the PROVEN July-4
# SurvivorKenny.fbx (which renders from its original import). If this copy
# renders too, the import pipeline is fine and the V3/V4 FBX FILES are broken;
# if it collapses like V3/V4, the import context is what corrupts rigs.
import os
import unreal

unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
if unreal.EditorAssetLibrary.does_directory_exist("/Game/DiagReimport"):
    unreal.EditorAssetLibrary.delete_directory("/Game/DiagReimport")

PROJ = unreal.SystemLibrary.get_project_directory()
f = os.path.join(PROJ, "RawArt", "Characters", "SurvivorKenny.fbx")
t = unreal.AssetImportTask()
t.filename = f
t.destination_path = "/Game/DiagReimport"
t.automated = True
t.save = True
t.replace_existing = True
o = unreal.FbxImportUI()
o.import_mesh = True
o.import_as_skeletal = True
o.import_animations = True
o.import_materials = True
o.import_textures = False
o.create_physics_asset = True
o.skeletal_mesh_import_data.set_editor_property("import_morph_targets", True)
# unit-scale hypothesis: my import context lands the rig at 1/100 scale
o.skeletal_mesh_import_data.set_editor_property("import_uniform_scale", 100.0)
o.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
t.options = o
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

mesh = unreal.load_asset("/Game/DiagReimport/SurvivorKenny")
if mesh:
    b = mesh.get_bounds()
    unreal.log_error("[DiagReimport] reimported SurvivorKenny bounds_r={0:.1f}".format(b.sphere_radius))
else:
    unreal.log_error("[DiagReimport] reimport FAILED")
