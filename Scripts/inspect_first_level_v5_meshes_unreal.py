"""Inspect render data for all first-level V5 Blender imports in Unreal."""

import unreal


ASSET_NAMES = (
    "AccessibleMarketV5",
    "AccessibleClinicV5",
    "OpenStreetCafeV5",
    "PointStarFieldV5",
    "MoonDetailedV5",
)

failures = []
for name in ASSET_NAMES:
    path = f"/Game/CodeRescueArt/FirstLevelV5/{name}/{name}/StaticMeshes/{name}.{name}"
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        failures.append(f"{name}: missing StaticMesh at {path}")
        continue

    bounds = mesh.get_bounding_box()
    sizes = (
        bounds.max.x - bounds.min.x,
        bounds.max.y - bounds.min.y,
        bounds.max.z - bounds.min.z,
    )
    triangles = mesh.get_num_triangles(0)
    lods = mesh.get_num_lods()
    materials = len(mesh.get_editor_property("static_materials"))
    unreal.log(
        f"[FirstLevelV5MeshAudit] {name} triangles={triangles} lods={lods} "
        f"materials={materials} size=({sizes[0]:.3f},{sizes[1]:.3f},{sizes[2]:.3f})"
    )
    if triangles <= 0:
        failures.append(f"{name}: LOD0 has no triangles")
    if max(sizes) <= 0.001:
        failures.append(f"{name}: render bounds are empty")
    if materials <= 0:
        failures.append(f"{name}: no material slots")

if failures:
    raise RuntimeError("First-level V5 mesh audit failed: " + "; ".join(failures))

unreal.log(f"[FirstLevelV5MeshAudit] COMPLETE PASS meshes={len(ASSET_NAMES)}")

