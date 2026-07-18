# fix_hero_bounds.py — render-bounds contract for EVERY authored character.
# The FBX import intermittently produces ~zero render bounds on these rigs
# (found on SurvivorKennyV4 AND SurvivorKennyV3): the mesh then frustum-culls
# to invisible. Inflate bounds extensions on all authored character assets.
import unreal

TARGETS = (
    "/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4",
    "/Game/CodeRescueArt/CharactersV3/SurvivorKennyV3/SurvivorKennyV3",
    "/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3",
    "/Game/CodeRescueArt/CharactersV3/ZombieShamblerV3/ZombieShamblerV3",
    "/Game/CodeRescueArt/CharactersV3/ZombieBruteV3/ZombieBruteV3",
    "/Game/CodeRescueArt/CharactersV3/ZombieRunnerV3/ZombieRunnerV3",
)
for path in TARGETS:
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_error("[HeroBounds] MISSING " + path)
        continue
    b = mesh.get_bounds()
    if b.sphere_radius < 60.0:
        mesh.set_editor_property("positive_bounds_extension", unreal.Vector(120.0, 120.0, 30.0))
        mesh.set_editor_property("negative_bounds_extension", unreal.Vector(120.0, 120.0, 200.0))
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        unreal.log_error("[HeroBounds] EXTENDED {0}: was r={1:.1f} now r={2:.1f}".format(
            mesh.get_name(), b.sphere_radius, mesh.get_bounds().sphere_radius))
    else:
        unreal.log_error("[HeroBounds] ok {0} r={1:.1f}".format(mesh.get_name(), b.sphere_radius))
