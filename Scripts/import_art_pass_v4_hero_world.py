# import_art_pass_v4_hero_world.py — 2026-07-11 pass 4 asset drop.
#
#   RawArt/CharactersV4/SurvivorKennyV4.fbx -> /Game/CodeRescueArt/CharactersV4/
#       (legacy FBX importer, morphs ON, create_physics_asset ON, clean slate,
#        then the analytic 16-body physics rebuild + material usage flags)
#   RawArt/WorldKitV4/*.glb -> /Game/CodeRescueArt/WorldKitV4/<Name>/
#       (SM_Door_Steel swinging leaf + SM_Curb_Ramp)
#
# Run: UnrealEditor <uproject> -ExecutePythonScript="<this file>" -stdout -unattended
import os

import unreal

unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")

if unreal.EditorAssetLibrary.does_directory_exist("/Game/CodeRescueArt/CharactersV4"):
    unreal.log_error("[ArtPassV4] deleting stale /Game/CodeRescueArt/CharactersV4 for a clean import")
    unreal.EditorAssetLibrary.delete_directory("/Game/CodeRescueArt/CharactersV4")

PROJ = unreal.SystemLibrary.get_project_directory()
RAW = os.path.join(PROJ, "RawArt")
HERO = "SurvivorKennyV4"
HERO_ACTIONS = ("Idle", "Walk", "Run", "Wave")

tasks = []
f = os.path.join(RAW, "CharactersV4", HERO + ".fbx")
if os.path.exists(f):
    t = unreal.AssetImportTask()
    t.filename = f
    t.destination_path = "/Game/CodeRescueArt/CharactersV4/" + HERO
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
    # 2026-07-16 ROOT CAUSE of the invisible characters: this import context
    # lands rigs at 1/100 scale without an explicit unit conversion.
    o.skeletal_mesh_import_data.set_editor_property("import_uniform_scale", 100.0)
    o.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    t.options = o
    tasks.append(t)
else:
    unreal.log_error("[ArtPassV4] MISSING " + f)

for sub in ("WorldKitV4", "WeaponsV5"):
    wdir = os.path.join(RAW, sub)
    if not os.path.isdir(wdir):
        continue
    for fn in sorted(os.listdir(wdir)):
        if fn.lower().endswith(".glb"):
            t = unreal.AssetImportTask()
            t.filename = os.path.join(wdir, fn)
            t.destination_path = "/Game/CodeRescueArt/{0}/{1}".format(sub, os.path.splitext(fn)[0])
            t.automated = True
            t.save = True
            t.replace_existing = True
            tasks.append(t)

unreal.log_error("[ArtPassV4] importing {} sources".format(len(tasks)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for t in tasks:
    imported.extend(str(p) for p in t.imported_object_paths)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_error("[ArtPassV4] imported {} objects".format(len(imported)))

failures = []

def _load(path):
    try:
        return unreal.load_asset(path)
    except Exception:
        return None

# analytic physics rebuild for the hero (same 17-bone rig, 0.956 scale)
mesh = _load("/Game/CodeRescueArt/CharactersV4/{0}/{0}".format(HERO))
if mesh:
    count = unreal.CodeRescueV3PhysicsLibrary.rebuild_authored_physics_asset(mesh, 5.0)
    if count < 6:
        failures.append("{0}: physics rebuild produced only {1} bodies".format(HERO, count))
    # render-bounds contract (pass-5 harness: the import produced ~zero bounds
    # and the hero was frustum-culled invisible) — inflate the asset bounds
    mesh.set_editor_property("positive_bounds_extension", unreal.Vector(120.0, 120.0, 30.0))
    mesh.set_editor_property("negative_bounds_extension", unreal.Vector(120.0, 120.0, 200.0))
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
else:
    failures.append(HERO + ": skeletal mesh MISSING")

# material usage flags (cooked-build contract)
for ap in unreal.EditorAssetLibrary.list_assets("/Game/CodeRescueArt/CharactersV4", recursive=True):
    asset = unreal.EditorAssetLibrary.load_asset(ap)
    if isinstance(asset, unreal.Material):
        for prop in ("used_with_skeletal_mesh", "used_with_morph_targets"):
            if not asset.get_editor_property(prop):
                asset.set_editor_property(prop, True)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

# validation
for act in HERO_ACTIONS:
    candidates = (
        "/Game/CodeRescueArt/CharactersV4/{0}/{0}_Anim_{0}_{1}".format(HERO, act),
        "/Game/CodeRescueArt/CharactersV4/{0}/{0}_{1}".format(HERO, act),
        "/Game/CodeRescueArt/CharactersV4/{0}/{0}{0}_{1}".format(HERO, act),
    )
    found = None
    for c in candidates:
        if _load(c):
            found = c
            break
    if found:
        unreal.log_error("[ArtPassV4] hero anim {0} -> {1}".format(act, found))
    else:
        failures.append("hero anim {0} MISSING".format(act))

for piece in ("SM_Door_Steel", "SM_Curb_Ramp"):
    candidates = (
        "/Game/CodeRescueArt/WorldKitV4/{0}/{0}/StaticMeshes/{0}".format(piece),
        "/Game/CodeRescueArt/WorldKitV4/{0}/StaticMeshes/{0}".format(piece),
        "/Game/CodeRescueArt/WorldKitV4/{0}/{0}".format(piece),
    )
    found = None
    for c in candidates:
        if _load(c):
            found = c
            break
    if found:
        unreal.log_error("[ArtPassV4] world kit {0} -> {1}".format(piece, found))
    else:
        failures.append("world kit {0} MISSING".format(piece))

if failures:
    for x in failures:
        unreal.log_error("[ArtPassV4] FAIL " + x)
    unreal.log_error("[ArtPassV4] VALIDATION FAILED ({0} issues)".format(len(failures)))
else:
    unreal.log_error("[ArtPassV4] VALIDATION PASSED — hero + world kit ready")
