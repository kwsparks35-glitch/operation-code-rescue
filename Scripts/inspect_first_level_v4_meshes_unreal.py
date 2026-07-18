"""Unreal-side render-data inspection for the first-level V4 Blender imports."""

import unreal


ASSET_NAMES = (
    "FirstLevelStorefrontV4",
    "FieldArmoryV4",
    "TriageCheckpointV4",
    "SandbagCoverV4",
    "GrenadeV4",
    "CombatKnifeV4",
    "RocketLauncherV4",
    "WoundCavityV4",
    "BiteWoundV4",
)

failures = []
for name in ASSET_NAMES:
    path = f"/Game/CodeRescueArt/FirstLevelV4/{name}/{name}/StaticMeshes/{name}.{name}"
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        failures.append(f"{name}: missing StaticMesh at {path}")
        continue

    bounds = mesh.get_bounding_box()
    size_x = bounds.max.x - bounds.min.x
    size_y = bounds.max.y - bounds.min.y
    size_z = bounds.max.z - bounds.min.z
    triangles = mesh.get_num_triangles(0)
    lods = mesh.get_num_lods()
    materials = len(mesh.get_editor_property("static_materials"))
    unreal.log(
        f"[FirstLevelV4MeshAudit] {name} triangles={triangles} lods={lods} "
        f"materials={materials} size=({size_x:.4f},{size_y:.4f},{size_z:.4f})"
    )
    if triangles <= 0:
        failures.append(f"{name}: LOD0 has no triangles")
    if max(size_x, size_y, size_z) <= 0.001:
        failures.append(f"{name}: render bounds are empty")
    if materials <= 0:
        failures.append(f"{name}: no material slots")

if failures:
    raise RuntimeError("First-level V4 mesh audit failed: " + "; ".join(failures))

unreal.log(f"[FirstLevelV4MeshAudit] COMPLETE PASS meshes={len(ASSET_NAMES)}")
