"""Idempotently reimport the production city and weapon GLBs into Unreal 5.7."""

import os

import unreal


PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
RAW_DIR = os.path.join(PROJECT_DIR, "RawArt")
TASKS = []


def queue_glb(source_path, destination_path):
    task = unreal.AssetImportTask()
    task.filename = source_path
    task.destination_path = destination_path
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    TASKS.append(task)


for source_folder, content_folder in (
    ("CityKitV3", "CityKitV3"),
    ("WeaponsV3", "WeaponsV3"),
):
    folder = os.path.join(RAW_DIR, source_folder)
    for filename in sorted(os.listdir(folder)):
        if not filename.lower().endswith(".glb"):
            continue
        asset_name = os.path.splitext(filename)[0]
        queue_glb(
            os.path.join(folder, filename),
            "/Game/CodeRescueArt/{}/{}".format(content_folder, asset_name),
        )


unreal.log("[ProductionArtV3] importing {} GLB sources".format(len(TASKS)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(TASKS)
imported = []
for task in TASKS:
    imported.extend(str(path) for path in task.imported_object_paths)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("[ProductionArtV3] imported/reimported {} Unreal objects".format(len(imported)))
for path in imported:
    unreal.log("[ProductionArtV3]   " + path)
