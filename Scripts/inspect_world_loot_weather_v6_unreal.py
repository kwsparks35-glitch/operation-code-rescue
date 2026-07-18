"""Inspect render data and ground-origin contracts for the V6 authored kit."""

import unreal


ASSET_NAMES = (
    "PickupAmmoV6",
    "PickupMedicalV6",
    "PickupArmorV6",
    "PickupUtilityV6",
    "PickupTechV6",
    "PickupSalvageV6",
    "ThreatGroundRingV6",
    "ResponderPackV6",
    "RainStreakV6",
    "WindDebrisV6",
    "FieldLogisticsDepotV6",
    "WeatherRelayV6",
    "QuarantineCheckpointV6",
)

failures = []
for name in ASSET_NAMES:
    path = "/Game/CodeRescueArt/WorldLootWeatherV6/{0}/{0}/StaticMeshes/{0}.{0}".format(name)
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        failures.append("{}: missing StaticMesh at {}".format(name, path))
        continue

    bounds = mesh.get_bounding_box()
    sizes = (
        bounds.max.x - bounds.min.x,
        bounds.max.y - bounds.min.y,
        bounds.max.z - bounds.min.z,
    )
    triangles = mesh.get_num_triangles(0)
    lods = mesh.get_num_lods()
    sections = mesh.get_num_sections(0)
    materials = len(mesh.get_editor_property("static_materials"))
    unreal.log(
        "[WorldLootWeatherV6MeshAudit] {} triangles={} lods={} sections={} materials={} "
        "size=({:.3f},{:.3f},{:.3f}) min_z={:.3f}".format(
            name, triangles, lods, sections, materials, sizes[0], sizes[1], sizes[2], bounds.min.z
        )
    )
    if triangles <= 0:
        failures.append("{}: LOD0 has no triangles".format(name))
    if max(sizes) <= 0.001:
        failures.append("{}: render bounds are empty".format(name))
    if materials <= 0:
        failures.append("{}: no material slots".format(name))
    if sections <= 0:
        failures.append("{}: LOD0 has no render sections".format(name))
    if name not in ("RainStreakV6", "WindDebrisV6", "ResponderPackV6") and abs(bounds.min.z) > 3.0:
        failures.append("{}: grounded origin drifted (min_z={:.3f})".format(name, bounds.min.z))
    if name in ("RainStreakV6", "WindDebrisV6"):
        for static_material in mesh.get_editor_property("static_materials"):
            weather_interface = static_material.get_editor_property("material_interface")
            weather_material = (weather_interface.get_base_material()
                                if isinstance(weather_interface, unreal.MaterialInterface) else None)
            if (not isinstance(weather_material, unreal.Material) or
                    not weather_material.get_editor_property("used_with_instanced_static_meshes")):
                failures.append("{}: material lacks instanced-static-mesh usage".format(name))

if failures:
    raise RuntimeError("World/Loot/Weather V6 mesh audit failed: " + "; ".join(failures))

unreal.log("[WorldLootWeatherV6MeshAudit] COMPLETE PASS meshes={} grounded_contract=1".format(len(ASSET_NAMES)))

if "-WorldLootWeatherInspectAndQuit" in unreal.SystemLibrary.get_command_line():
    unreal.log("[WorldLootWeatherV6MeshAudit] inspection complete; requesting editor shutdown")
    unreal.SystemLibrary.quit_editor()
