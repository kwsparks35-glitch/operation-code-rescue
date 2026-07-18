# import_art_pass_v3_v4.py — imports the 2026-07-11 art+physics drop.
#
#   RawArt/CharactersV3/*.fbx  -> /Game/CodeRescueArt/CharactersV3/<Name>/...
#       legacy FBX importer (Interchange OFF — 2026-07-04 lesson), morph targets
#       ON, and CREATE PHYSICS ASSET ON (this is what re-enables ragdoll deaths
#       and physical hit reactions for authored zombies — see CodeZombieActor).
#   RawArt/WeaponsV4/*.glb     -> /Game/CodeRescueArt/WeaponsV4/<Name>/...
#       Interchange GLB path (double-nests folders, same as WeaponsV3).
#
# Run headless:
#   UnrealEditor-Cmd <uproject> -run=pythonscript -script="<this file>" -stdout
# Idempotent: replace_existing everywhere.
import os

import unreal

# Character FBX must go through the LEGACY FBX importer.
unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")

# CLEAN SLATE for characters: a replace_existing "reimport" reuses the ORIGINAL
# import options (observed 2026-07-11: a crashed first import left mesh+skeleton
# only, and reimport then refused to add anims/physics assets). Delete + fresh
# import keeps the options authoritative. log_error markers are used for phase
# output because -stdout filters Display-level LogPython lines in the editor.
if unreal.EditorAssetLibrary.does_directory_exist("/Game/CodeRescueArt/CharactersV3"):
    unreal.log_error("[ArtPassV3V4] deleting stale /Game/CodeRescueArt/CharactersV3 for a clean import")
    unreal.EditorAssetLibrary.delete_directory("/Game/CodeRescueArt/CharactersV3")

PROJ = unreal.SystemLibrary.get_project_directory()
RAW = os.path.join(PROJ, "RawArt")
CHARS = ("SurvivorKennyV3", "SurvivorMayaV3", "ZombieShamblerV3",
         "ZombieBruteV3", "ZombieRunnerV3")
EXPECTED_ACTIONS = {
    "SurvivorKennyV3": ("Idle", "Walk", "Run", "Wave"),
    "SurvivorMayaV3": ("Idle", "Walk", "Run", "Wave"),
    "ZombieShamblerV3": ("Idle", "Walk", "Run", "Attack", "Flinch", "Death"),
    "ZombieBruteV3": ("Idle", "Walk", "Run", "Attack", "Flinch", "Death"),
    "ZombieRunnerV3": ("Idle", "Walk", "Run", "Attack", "Flinch", "Death"),
}

tasks = []
for name in CHARS:
    f = os.path.join(RAW, "CharactersV3", name + ".fbx")
    if not os.path.exists(f):
        unreal.log_warning("[ArtPassV3V4] missing " + f)
        continue
    t = unreal.AssetImportTask()
    t.filename = f
    t.destination_path = "/Game/CodeRescueArt/CharactersV3/" + name
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

wdir = os.path.join(RAW, "WeaponsV4")
for fn in sorted(os.listdir(wdir)):
    if fn.lower().endswith(".glb"):
        t = unreal.AssetImportTask()
        t.filename = os.path.join(wdir, fn)
        t.destination_path = "/Game/CodeRescueArt/WeaponsV4/" + os.path.splitext(fn)[0]
        t.automated = True
        t.save = True
        t.replace_existing = True
        tasks.append(t)

unreal.log_error("[ArtPassV3V4] importing {} sources".format(len(tasks)))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
imported = []
for t in tasks:
    imported.extend(str(p) for p in t.imported_object_paths)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_error("[ArtPassV3V4] imported/reimported {} objects".format(len(imported)))
for p in imported:
    unreal.log_error("[ArtPassV3V4]   " + p)

# ---------------- post-import hardening ----------------
failures = []

def _load(path):
    try:
        return unreal.load_asset(path)
    except Exception:
        return None

# 1) Deliberate ragdoll physics assets: the FBX auto-fitter reduces the
#    authored 17-bone rigs to a 2-body asset, so rebuild via the editor
#    library (16 capsule/sphere bodies + 15 cone-limited constraints).
for name in CHARS:
    mesh = _load("/Game/CodeRescueArt/CharactersV3/{0}/{0}".format(name))
    if mesh:
        count = unreal.CodeRescueV3PhysicsLibrary.rebuild_authored_physics_asset(mesh, 5.0)
        if count < 6:
            failures.append("{0}: physics rebuild produced only {1} bodies".format(name, count))

# 2) Material usage flags: morph-target skeletal materials render as the
#    DEFAULT MATERIAL in cooked builds without these (2026-07-02 lesson).
for ap in unreal.EditorAssetLibrary.list_assets("/Game/CodeRescueArt/CharactersV3", recursive=True):
    asset = unreal.EditorAssetLibrary.load_asset(ap)
    if isinstance(asset, unreal.Material):
        for prop in ("used_with_skeletal_mesh", "used_with_morph_targets"):
            if not asset.get_editor_property(prop):
                asset.set_editor_property(prop, True)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

# ---------------- validation ----------------

for name in CHARS:
    base = "/Game/CodeRescueArt/CharactersV3/{0}".format(name)
    mesh = _load("{0}/{1}".format(base, name))
    if not mesh:
        failures.append(name + ": skeletal mesh MISSING")
        continue
    pa = mesh.get_editor_property("physics_asset")
    if pa:
        # SkeletalBodySetups is not Python-exposed in UE5.7; body-count QA
        # happens at runtime in CodeZombieActor (MatchedBodies gate + log).
        try:
            bodies = pa.get_editor_property("skeletal_body_setups")
            names = [str(b.get_editor_property("bone_name")) for b in bodies if b]
            unreal.log_error("[ArtPassV3V4] {0}: physics asset '{1}' bodies={2} -> {3}".format(
                name, pa.get_name(), len(names), ",".join(names)))
        except Exception:
            unreal.log_error("[ArtPassV3V4] {0}: physics asset '{1}' present (body list not Python-exposed)".format(
                name, pa.get_name()))
    else:
        failures.append(name + ": NO physics asset (ragdoll path will stay off)")
    for act in EXPECTED_ACTIONS[name]:
        candidates = (
            "{0}/{1}_Anim_{1}_{2}".format(base, name, act),
            "{0}/{1}_{2}".format(base, name, act),
            "{0}/{1}{1}_{2}".format(base, name, act),
        )
        found = None
        for c in candidates:
            if _load(c):
                found = c
                break
        if found:
            unreal.log_error("[ArtPassV3V4] {0}: anim {1} -> {2}".format(name, act, found))
        else:
            failures.append("{0}: anim {1} MISSING".format(name, act))

for wep in ("PistolV4", "ShotgunV4", "RifleV4", "SMGV4", "CrossbowV4"):
    candidates = (
        "/Game/CodeRescueArt/WeaponsV4/{0}/{0}/StaticMeshes/{0}".format(wep),
        "/Game/CodeRescueArt/WeaponsV4/{0}/StaticMeshes/{0}".format(wep),
        "/Game/CodeRescueArt/WeaponsV4/{0}/{0}".format(wep),
    )
    found = None
    for c in candidates:
        if _load(c):
            found = c
            break
    if found:
        unreal.log_error("[ArtPassV3V4] weapon {0} -> {1}".format(wep, found))
    else:
        failures.append("weapon {0} MISSING".format(wep))

if failures:
    for f in failures:
        unreal.log_error("[ArtPassV3V4] FAIL " + f)
    unreal.log_error("[ArtPassV3V4] VALIDATION FAILED ({0} issues)".format(len(failures)))
else:
    unreal.log("[ArtPassV3V4] VALIDATION PASSED — all meshes, physics assets, anims, weapons present")
