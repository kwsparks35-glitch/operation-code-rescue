"""Idempotently import the Blender-authored first-level V4 GLBs into UE 5.7."""

import os

import unreal


project_dir = unreal.SystemLibrary.get_project_directory()
source_dir = os.path.join(project_dir, "RawArt", "FirstLevelV4")
tasks = []

for filename in sorted(os.listdir(source_dir)):
    if not filename.lower().endswith(".glb"):
        continue
    asset_name = os.path.splitext(filename)[0]
    task = unreal.AssetImportTask()
    task.filename = os.path.join(source_dir, filename)
    task.destination_path = "/Game/CodeRescueArt/FirstLevelV4/{}".format(asset_name)
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    tasks.append(task)

unreal.log("[FirstLevelV4] importing {} authored GLBs".format(len(tasks)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for task in tasks:
    imported.extend(str(path) for path in task.imported_object_paths)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("[FirstLevelV4] imported/reimported {} Unreal objects".format(len(imported)))
for path in imported:
    unreal.log("[FirstLevelV4]   " + path)

if "-FirstLevelImportAndQuit" in unreal.SystemLibrary.get_command_line():
    unreal.log("[FirstLevelV4] import complete; requesting clean editor shutdown")
    unreal.SystemLibrary.quit_editor()
