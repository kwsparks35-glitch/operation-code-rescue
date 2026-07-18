"""Idempotently import the Blender-authored first-level V5 GLBs into UE 5.7."""

import os

import unreal


project_dir = unreal.SystemLibrary.get_project_directory()
source_dir = os.path.join(project_dir, "RawArt", "FirstLevelV5")
tasks = []

for filename in sorted(os.listdir(source_dir)):
    if not filename.lower().endswith(".glb"):
        continue
    asset_name = os.path.splitext(filename)[0]
    task = unreal.AssetImportTask()
    task.filename = os.path.join(source_dir, filename)
    task.destination_path = "/Game/CodeRescueArt/FirstLevelV5/{}".format(asset_name)
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    tasks.append(task)

unreal.log("[FirstLevelV5] importing {} authored GLBs".format(len(tasks)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for task in tasks:
    imported.extend(str(path) for path in task.imported_object_paths)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("[FirstLevelV5] imported/reimported {} Unreal objects".format(len(imported)))
for path in imported:
    unreal.log("[FirstLevelV5]   " + path)

# Interchange may generate a scene-wide convex hull for a GLB. Persist
# complex-as-simple on access buildings so their authored doorway openings are
# represented by collision instead of being sealed by that hull.
exec(
    open(os.path.join(project_dir, "Scripts", "configure_first_level_v5_collision_unreal.py"), encoding="utf-8").read(),
    {"__name__": "__main__"},
)

if "-FirstLevelV5ImportAndQuit" in unreal.SystemLibrary.get_command_line():
    unreal.log("[FirstLevelV5] import complete; requesting editor shutdown")
    unreal.SystemLibrary.quit_editor()
