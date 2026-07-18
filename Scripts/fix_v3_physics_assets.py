# fix_v3_physics_assets.py — rebuild the CharactersV3 physics assets with
# deliberate parameters (the FBX auto-create produced 2-body assets; see
# Source/CodeRescueUnrealEditor/CodeRescueV3PhysicsLibrary). Idempotent.
import unreal

CHARS = ("SurvivorKennyV3", "SurvivorMayaV3", "ZombieShamblerV3",
         "ZombieBruteV3", "ZombieRunnerV3")
fails = []
for name in CHARS:
    mesh = unreal.load_asset("/Game/CodeRescueArt/CharactersV3/{0}/{0}".format(name))
    if not mesh:
        fails.append(name + ": mesh missing")
        continue
    count = unreal.CodeRescueV3PhysicsLibrary.rebuild_authored_physics_asset(mesh, 5.0)
    if count < 6:
        fails.append("{0}: only {1} bodies".format(name, count))

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
if fails:
    for f in fails:
        unreal.log_error("[V3PhysFix] FAIL " + f)
else:
    unreal.log_error("[V3PhysFix] ALL PHYSICS ASSETS REBUILT + SAVED (>=6 bodies each)")
