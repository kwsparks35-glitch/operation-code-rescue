# import_art_pass_v2.py — imports the 2026-07-04 RawArt drop into /Game/CodeRescueArt.
# Run headless:  UnrealEditor-Cmd <uproject> -run=pythonscript -script="<this file>"
# (Same logic as the queued bridge job 0200_import_art_pass_v2.json; idempotent.)
import unreal, os

PROJ = unreal.SystemLibrary.get_project_directory()
RAW = os.path.join(PROJ, "RawArt")
tasks = []

for name in ("SurvivorKenny", "SurvivorMaya", "ZombieShamblerV2", "ZombieBruteV2"):
    f = os.path.join(RAW, "Characters", name + ".fbx")
    if not os.path.exists(f):
        unreal.log_warning("[ArtPassV2] missing " + f)
        continue
    t = unreal.AssetImportTask()
    t.filename = f
    t.destination_path = "/Game/CodeRescueArt/CharactersV2"
    t.automated = True
    t.save = True
    t.replace_existing = True
    o = unreal.FbxImportUI()
    o.import_mesh = True
    o.import_as_skeletal = True
    o.import_animations = True
    o.import_materials = True
    o.import_textures = False
    o.skeletal_mesh_import_data.set_editor_property("import_morph_targets", True)
    o.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    t.options = o
    tasks.append(t)

for sub in ("Weapons", "Vehicles", "Nature", "Sky"):
    d = os.path.join(RAW, sub)
    if not os.path.isdir(d):
        continue
    for fn in sorted(os.listdir(d)):
        if fn.endswith(".glb"):
            t = unreal.AssetImportTask()
            t.filename = os.path.join(d, fn)
            t.destination_path = "/Game/CodeRescueArt/" + sub
            t.automated = True
            t.save = True
            t.replace_existing = True
            tasks.append(t)

d = os.path.join(RAW, "CityKit")
for fn in ("SM_Road_Straight_12m.glb", "SM_Crosswalk_8m.glb", "SM_Sidewalk_6m.glb",
           "SM_StreetSign_Stop.glb", "SM_TrafficLight.glb"):
    p = os.path.join(d, fn)
    if os.path.exists(p):
        t = unreal.AssetImportTask()
        t.filename = p
        t.destination_path = "/Game/CodeRescueArt/CityKit"
        t.automated = True
        t.save = True
        t.replace_existing = True
        tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for t in tasks:
    imported.extend(list(t.imported_object_paths))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
print("[ArtPassV2] imported {} assets".format(len(imported)))
for p in imported:
    print("[ArtPassV2]   " + p)
