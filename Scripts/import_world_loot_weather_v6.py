"""Idempotently import the Blender-authored World/Loot/Weather V6 GLBs."""

import os

import unreal


project_dir = unreal.SystemLibrary.get_project_directory()
source_dir = os.path.join(project_dir, "RawArt", "WorldLootWeatherV6")
tasks = []

for filename in sorted(os.listdir(source_dir)):
    if not filename.lower().endswith(".glb"):
        continue
    asset_name = os.path.splitext(filename)[0]
    task = unreal.AssetImportTask()
    task.filename = os.path.join(source_dir, filename)
    task.destination_path = "/Game/CodeRescueArt/WorldLootWeatherV6/{}".format(asset_name)
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    tasks.append(task)

unreal.log("[WorldLootWeatherV6] importing {} authored GLBs".format(len(tasks)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for task in tasks:
    imported.extend(str(path) for path in task.imported_object_paths)

# Rain and wind are rendered through hierarchical instanced static meshes.
# glTF import does not infer that material usage, so set and persist it here;
# otherwise UE substitutes the default material and recompiles every boot.
instanced_material_paths = (
    "/Game/CodeRescueArt/WorldLootWeatherV6/RainStreakV6/RainStreakV6/Materials/CRV6_Rain.CRV6_Rain",
    "/Game/CodeRescueArt/WorldLootWeatherV6/WindDebrisV6/WindDebrisV6/Materials/CRV6_WindDebris.CRV6_WindDebris",
)
for material_path in instanced_material_paths:
    instanced_material_interface = unreal.load_asset(material_path)
    if not isinstance(instanced_material_interface, unreal.MaterialInterface):
        raise RuntimeError("Missing instanced-weather material: " + material_path)
    instanced_material = instanced_material_interface.get_base_material()
    if not isinstance(instanced_material, unreal.Material):
        raise RuntimeError("Weather material has no editable base: " + material_path)
    instanced_material.set_editor_property("used_with_instanced_static_meshes", True)
    unreal.EditorAssetLibrary.save_loaded_asset(instanced_material)
    unreal.log("[WorldLootWeatherV6] enabled instanced-static-mesh usage: " + material_path)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("[WorldLootWeatherV6] imported/reimported {} Unreal objects".format(len(imported)))
for path in imported:
    unreal.log("[WorldLootWeatherV6]   " + path)

if "-WorldLootWeatherImportAndQuit" in unreal.SystemLibrary.get_command_line():
    unreal.log("[WorldLootWeatherV6] import complete; requesting clean editor shutdown")
    unreal.SystemLibrary.quit_editor()
