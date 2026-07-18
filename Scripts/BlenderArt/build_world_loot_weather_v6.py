"""Author the V6 loot, weather, character-gear, and purpose-landmark kit.

All dimensions are meters. Every grounded asset uses a Z=0 contact origin so
Unreal can place it directly on the canonical mission floor. The script is
deterministic, headless-safe, and exports one GLB per production asset.
"""

import math
import os

import bpy


PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WorldLootWeatherV6")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.context.scene.unit_settings.system = "METRIC"
bpy.context.scene.unit_settings.scale_length = 1.0


def material(name, color, roughness=0.7, metallic=0.0, emission=None, emission_strength=0.0):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    shader = mat.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Base Color"].default_value = (*color, 1.0)
    shader.inputs["Roughness"].default_value = roughness
    shader.inputs["Metallic"].default_value = metallic
    if emission is not None:
        shader.inputs["Emission Color"].default_value = (*emission, 1.0)
        shader.inputs["Emission Strength"].default_value = emission_strength
    return mat


MAT = {
    "black": material("CRV6_BlackPolymer", (0.018, 0.022, 0.026), 0.54, 0.18),
    "steel": material("CRV6_Steel", (0.15, 0.18, 0.21), 0.34, 0.88),
    "steel_light": material("CRV6_SteelLight", (0.42, 0.46, 0.50), 0.28, 0.92),
    "olive": material("CRV6_AmmoOlive", (0.12, 0.18, 0.10), 0.63, 0.35),
    "brass": material("CRV6_Brass", (0.52, 0.28, 0.055), 0.28, 0.86,
                      (1.0, 0.48, 0.08), 0.8),
    "medical": material("CRV6_MedicalRed", (0.46, 0.018, 0.024), 0.58, 0.08),
    "white": material("CRV6_SymbolWhite", (0.82, 0.88, 0.91), 0.48, 0.18,
                      (0.72, 0.92, 1.0), 1.4),
    "armor": material("CRV6_ArmorBlue", (0.055, 0.18, 0.28), 0.48, 0.58),
    "utility": material("CRV6_UtilityOrange", (0.50, 0.16, 0.025), 0.56, 0.25),
    "tech": material("CRV6_TechBlue", (0.025, 0.20, 0.34), 0.42, 0.46),
    "salvage": material("CRV6_SalvageYellow", (0.52, 0.33, 0.035), 0.72, 0.30),
    "cyan": material("CRV6_CyanSignal", (0.015, 0.22, 0.28), 0.32, 0.28, (0.04, 0.72, 1.0), 5.0),
    "red_glow": material("CRV6_ThreatSignal", (0.28, 0.008, 0.012), 0.38, 0.22, (1.0, 0.018, 0.028), 5.5),
    "rain": material("CRV6_Rain", (0.18, 0.42, 0.62), 0.18, 0.12, (0.24, 0.62, 1.0), 2.2),
    "leaf": material("CRV6_WindDebris", (0.28, 0.22, 0.10), 0.92, 0.0),
    "concrete": material("CRV6_Concrete", (0.22, 0.24, 0.25), 0.91, 0.0),
    "concrete_dark": material("CRV6_ConcreteDark", (0.065, 0.075, 0.082), 0.90, 0.0),
    "hazard": material("CRV6_HazardAmber", (0.60, 0.31, 0.025), 0.55, 0.18, (1.0, 0.34, 0.03), 2.2),
    "canvas": material("CRV6_Canvas", (0.18, 0.23, 0.19), 0.96, 0.0),
    "glass": material("CRV6_GlassDark", (0.025, 0.075, 0.09), 0.24, 0.28),
    "green": material("CRV6_RescueGreen", (0.035, 0.28, 0.12), 0.58, 0.22, (0.04, 0.72, 0.20), 1.8),
}


def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)


def finish(obj, mat, bevel=0.0):
    obj.data.materials.append(mat)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel > 0.0:
        modifier = obj.modifiers.new("CRV6_EdgeRadius", "BEVEL")
        modifier.width = bevel
        modifier.segments = 2
        modifier.limit_method = "ANGLE"
        modifier.angle_limit = math.radians(25.0)
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)
    return obj


def box(name, location, size, mat, rotation=(0.0, 0.0, 0.0), bevel=0.02):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.scale = (size[0] * 0.5, size[1] * 0.5, size[2] * 0.5)
    return finish(obj, mat, bevel)


def cylinder(name, location, radius, depth, mat, axis="Z", vertices=20, bevel=0.006):
    rotation = (0.0, math.pi * 0.5, 0.0) if axis == "X" else (math.pi * 0.5, 0.0, 0.0) if axis == "Y" else (0.0, 0.0, 0.0)
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    return finish(obj, mat, bevel)


def sphere(name, location, scale, mat, segments=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    return finish(obj, mat, 0.004)


def torus(name, location, major_radius, minor_radius, mat, rotation=(0.0, 0.0, 0.0), major_segments=28):
    bpy.ops.mesh.primitive_torus_add(
        major_segments=major_segments,
        minor_segments=8,
        location=location,
        rotation=rotation,
        major_radius=major_radius,
        minor_radius=minor_radius,
    )
    obj = bpy.context.object
    obj.name = name
    return finish(obj, mat, 0.0)


def wedge(name, vertices, faces, mat, bevel=0.006):
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return finish(obj, mat, bevel)


def export_asset(name, max_dimension=20.0, max_triangles=45000):
    parts = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    assert parts, f"{name}: no geometry"
    bpy.ops.object.select_all(action="DESELECT")
    for part in parts:
        part.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    asset = bpy.context.object
    asset.name = name
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR")
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    asset.data.validate(clean_customdata=False)
    triangles = sum(max(0, len(poly.vertices) - 2) for poly in asset.data.polygons)
    assert max(asset.dimensions) <= max_dimension, f"{name}: dimensions {tuple(asset.dimensions)}"
    assert triangles <= max_triangles, f"{name}: {triangles} triangles"
    path = os.path.join(OUT_DIR, name + ".glb")
    bpy.ops.export_scene.gltf(filepath=path, use_selection=True, export_format="GLB", export_apply=True, export_yup=True)
    print(f"[WorldLootWeatherV6] {name}: dims={tuple(round(v, 3) for v in asset.dimensions)} tris={triangles} -> {path}")


def pickup_case(name, shell_mat, icon_builder):
    clean_scene()
    box("GroundedCase", (0.0, 0.0, 0.23), (0.76, 0.52, 0.46), shell_mat, bevel=0.055)
    box("CaseLid", (0.0, -0.005, 0.475), (0.72, 0.49, 0.075), MAT["black"], bevel=0.025)
    box("FrontPanel", (0.0, -0.272, 0.25), (0.56, 0.026, 0.29), MAT["black"], bevel=0.018)
    for x in (-0.25, 0.25):
        box("Latch", (x, -0.296, 0.405), (0.10, 0.055, 0.11), MAT["steel_light"], bevel=0.012)
    box("Handle", (0.0, 0.285, 0.30), (0.26, 0.055, 0.055), MAT["steel"], bevel=0.018)
    objects_before_icon = set(bpy.context.scene.objects)
    icon_builder()

    # Mirror the physical symbol onto the handle side as well. Pickups turn
    # slowly in play, but a recognizable category mark should never depend on
    # the player's approach direction or a particular lighting angle.
    front_icon_objects = [
        obj for obj in bpy.context.scene.objects
        if obj not in objects_before_icon and obj.type == "MESH"
    ]
    for front in front_icon_objects:
        rear = front.copy()
        rear.data = front.data.copy()
        rear.name = "Rear" + front.name
        bpy.context.collection.objects.link(rear)
        if abs(front.location.y) > 0.0001:
            rear.location.y = -front.location.y
            rear.rotation_euler.y = -front.rotation_euler.y
            rear.rotation_euler.z = -front.rotation_euler.z
        else:
            for vertex in rear.data.vertices:
                vertex.co.y *= -1.0
            for polygon in rear.data.polygons:
                polygon.flip()
    export_asset(name, 1.2, 18000)


def icon_ammo():
    for x in (-0.13, 0.0, 0.13):
        cylinder("Cartridge", (x, -0.305, 0.245), 0.035, 0.22, MAT["brass"], axis="Z", vertices=16)
        cylinder("CartridgeTip", (x, -0.306, 0.375), 0.025, 0.055, MAT["brass"], axis="Z", vertices=16)


def icon_medical():
    box("CrossVertical", (0.0, -0.308, 0.25), (0.085, 0.045, 0.26), MAT["white"], bevel=0.014)
    box("CrossHorizontal", (0.0, -0.309, 0.25), (0.26, 0.045, 0.085), MAT["white"], bevel=0.014)


def icon_armor():
    verts = [
        (-0.16, -0.31, 0.36), (0.16, -0.31, 0.36), (0.13, -0.31, 0.19),
        (0.0, -0.31, 0.10), (-0.13, -0.31, 0.19),
        (-0.16, -0.35, 0.36), (0.16, -0.35, 0.36), (0.13, -0.35, 0.19),
        (0.0, -0.35, 0.10), (-0.13, -0.35, 0.19),
    ]
    faces = [(0, 1, 2, 3, 4), (9, 8, 7, 6, 5), (0, 5, 6, 1), (1, 6, 7, 2), (2, 7, 8, 3), (3, 8, 9, 4), (4, 9, 5, 0)]
    wedge("ShieldSymbol", verts, faces, MAT["white"], 0.008)


def icon_utility():
    verts = [
        (-0.04, -0.31, 0.10), (0.10, -0.31, 0.24), (0.03, -0.31, 0.24),
        (0.13, -0.31, 0.40), (-0.08, -0.31, 0.28), (-0.01, -0.31, 0.28),
        (-0.04, -0.35, 0.10), (0.10, -0.35, 0.24), (0.03, -0.35, 0.24),
        (0.13, -0.35, 0.40), (-0.08, -0.35, 0.28), (-0.01, -0.35, 0.28),
    ]
    faces = [(0, 1, 2, 3, 4, 5), (11, 10, 9, 8, 7, 6), (0, 6, 7, 1), (1, 7, 8, 2), (2, 8, 9, 3), (3, 9, 10, 4), (4, 10, 11, 5), (5, 11, 6, 0)]
    wedge("UtilityBolt", verts, faces, MAT["white"], 0.006)


def icon_tech():
    icon_utility()
    torus("ScannerArc", (0.0, -0.33, 0.25), 0.18, 0.018, MAT["cyan"], rotation=(math.pi * 0.5, 0.0, 0.0), major_segments=20)


def icon_salvage():
    torus("SalvageGear", (0.0, -0.33, 0.25), 0.12, 0.032, MAT["white"], rotation=(math.pi * 0.5, 0.0, 0.0), major_segments=20)
    for index in range(8):
        angle = index * math.pi / 4.0
        x = math.cos(angle) * 0.17
        z = 0.25 + math.sin(angle) * 0.17
        box("GearTooth", (x, -0.334, z), (0.065, 0.045, 0.065), MAT["white"], rotation=(0.0, angle, 0.0), bevel=0.008)


def build_threat_ring():
    clean_scene()
    torus("ThreatRing", (0.0, 0.0, 0.025), 0.66, 0.028, MAT["red_glow"], major_segments=36)
    for index in range(4):
        angle = index * math.pi * 0.5
        x = math.cos(angle) * 0.66
        y = math.sin(angle) * 0.66
        box("ThreatTick", (x, y, 0.035), (0.18, 0.07, 0.035), MAT["hazard"], rotation=(0.0, 0.0, angle), bevel=0.008)
    export_asset("ThreatGroundRingV6", 2.0, 12000)


def build_responder_pack():
    clean_scene()
    box("PackBody", (0.0, 0.0, 0.36), (0.43, 0.22, 0.66), MAT["olive"], bevel=0.065)
    box("PackFlap", (0.0, -0.13, 0.52), (0.39, 0.075, 0.26), MAT["canvas"], bevel=0.035)
    box("RadioPouch", (0.16, -0.13, 0.25), (0.13, 0.09, 0.22), MAT["black"], bevel=0.025)
    cylinder("Antenna", (0.17, -0.13, 0.58), 0.012, 0.34, MAT["steel"], axis="Z", vertices=10, bevel=0.002)
    for x in (-0.16, 0.16):
        box("Harness", (x, 0.13, 0.37), (0.055, 0.055, 0.60), MAT["black"], bevel=0.012)
    box("RoleSignal", (-0.12, -0.155, 0.29), (0.12, 0.035, 0.12), MAT["cyan"], bevel=0.025)
    export_asset("ResponderPackV6", 1.0, 12000)


def build_rain_streak():
    clean_scene()
    cylinder("RainStreak", (0.0, 0.0, 0.45), 0.007, 0.90, MAT["rain"], axis="Z", vertices=8, bevel=0.0)
    export_asset("RainStreakV6", 1.0, 2000)


def build_wind_debris():
    clean_scene()
    verts = [(-0.11, -0.04, 0.0), (0.12, -0.025, 0.025), (0.09, 0.045, 0.0), (-0.08, 0.055, -0.018)]
    wedge("WindLeaf", verts, [(0, 1, 2, 3)], MAT["leaf"], 0.003)
    export_asset("WindDebrisV6", 0.4, 1000)


def build_logistics_depot():
    clean_scene()
    box("DepotFoundation", (0.0, 0.0, 0.10), (8.4, 5.2, 0.20), MAT["concrete_dark"], bevel=0.045)
    for x in (-3.85, 3.85):
        for y in (-2.25, 2.25):
            cylinder("DepotPost", (x, y, 1.65), 0.09, 3.2, MAT["steel"], vertices=16, bevel=0.008)
    box("DepotCanopy", (0.0, 0.0, 3.28), (8.5, 5.3, 0.22), MAT["canvas"], bevel=0.055)
    box("DepotRearRack", (0.0, 2.15, 1.35), (7.6, 0.34, 2.4), MAT["steel"], bevel=0.025)
    for z in (0.48, 1.18, 1.88):
        box("RackShelf", (0.0, 1.93, z), (7.2, 0.75, 0.11), MAT["steel_light"], bevel=0.012)
    for index in range(8):
        x = -3.05 + (index % 4) * 2.02
        y = 1.65
        z = 0.45 + (index // 4) * 0.70
        box("SupplyCase", (x, y, z), (1.15, 0.68, 0.55), MAT["olive"] if index % 2 == 0 else MAT["utility"], bevel=0.06)
    # Three stacked physical boxes are the depot symbol; no paragraph label.
    box("DepotIconA", (-0.52, -2.45, 2.40), (0.62, 0.12, 0.52), MAT["hazard"], bevel=0.035)
    box("DepotIconB", (0.52, -2.45, 2.40), (0.62, 0.12, 0.52), MAT["hazard"], bevel=0.035)
    box("DepotIconC", (0.0, -2.45, 2.96), (0.62, 0.12, 0.52), MAT["hazard"], bevel=0.035)
    # A unique package name avoids retaining render sections from the first
    # interrupted Unreal import of the original LogisticsDepotV6 package.
    export_asset("FieldLogisticsDepotV6", 12.0, 26000)


def build_weather_relay():
    clean_scene()
    box("RelayFoundation", (0.0, 0.0, 0.10), (5.8, 5.8, 0.20), MAT["concrete_dark"], bevel=0.06)
    box("RelayCabinet", (0.0, 0.55, 0.85), (1.7, 1.1, 1.5), MAT["tech"], bevel=0.08)
    box("RelayScreen", (0.0, -0.025, 0.95), (1.15, 0.06, 0.58), MAT["cyan"], bevel=0.035)
    cylinder("RelayMast", (0.0, 0.8, 3.65), 0.085, 5.8, MAT["steel"], vertices=18, bevel=0.006)
    cylinder("AnemometerCrossX", (0.0, 0.8, 6.24), 0.035, 1.42, MAT["steel_light"], axis="X", vertices=12)
    cylinder("AnemometerCrossY", (0.0, 0.8, 6.24), 0.035, 1.42, MAT["steel_light"], axis="Y", vertices=12)
    for x, y in ((0.72, 0.8), (-0.72, 0.8), (0.0, 1.52), (0.0, 0.08)):
        sphere("WindCup", (x, y, 6.24), (0.15, 0.10, 0.10), MAT["hazard"], 14, 7)
    cylinder("RainGauge", (1.75, -0.65, 0.72), 0.18, 1.25, MAT["glass"], vertices=20, bevel=0.018)
    cylinder("RainGaugeRim", (1.75, -0.65, 1.38), 0.30, 0.12, MAT["steel_light"], vertices=20, bevel=0.012)
    for x in (-2.15, 2.15):
        cylinder("FogLampPost", (x, -1.75, 0.82), 0.045, 1.55, MAT["steel"], vertices=12)
        sphere("FogLamp", (x, -1.75, 1.62), (0.16, 0.16, 0.12), MAT["cyan"], 14, 7)
    export_asset("WeatherRelayV6", 8.0, 26000)


def build_quarantine_checkpoint():
    clean_scene()
    box("CheckpointFoundation", (0.0, 0.0, 0.10), (9.8, 5.4, 0.20), MAT["concrete_dark"], bevel=0.05)
    for x in (-3.55, 3.55):
        box("GatePillar", (x, 0.6, 1.75), (0.62, 0.62, 3.3), MAT["concrete"], bevel=0.06)
        box("GateLamp", (x, 0.25, 2.55), (0.30, 0.10, 0.42), MAT["hazard"], bevel=0.035)
    box("GateHeader", (0.0, 0.6, 3.28), (7.65, 0.48, 0.44), MAT["steel"], bevel=0.045)
    # Angled barriers guide toward the open center instead of imprisoning AI.
    box("BarrierLeft", (-3.18, -1.15, 0.55), (3.25, 0.48, 0.86), MAT["concrete"], rotation=(0.0, 0.0, math.radians(18)), bevel=0.08)
    box("BarrierRight", (3.18, -1.15, 0.55), (3.25, 0.48, 0.86), MAT["concrete"], rotation=(0.0, 0.0, math.radians(-18)), bevel=0.08)
    for x in (-2.2, 2.2):
        cylinder("OpenLaneBollard", (x, -1.72, 0.46), 0.10, 0.82, MAT["hazard"], vertices=16, bevel=0.012)
    # Physical warning triangle with a central signal point.
    verts = [(-0.62, 0.31, 3.62), (0.62, 0.31, 3.62), (0.0, 0.31, 4.58), (-0.62, 0.20, 3.62), (0.62, 0.20, 3.62), (0.0, 0.20, 4.58)]
    faces = [(0, 1, 2), (5, 4, 3), (0, 3, 4, 1), (1, 4, 5, 2), (2, 5, 3, 0)]
    wedge("HazardTriangle", verts, faces, MAT["hazard"], 0.018)
    cylinder("HazardPoint", (0.0, 0.16, 3.95), 0.08, 0.12, MAT["black"], axis="Y", vertices=16)
    sphere("HazardDot", (0.0, 0.14, 3.74), (0.09, 0.06, 0.09), MAT["black"], 12, 6)
    export_asset("QuarantineCheckpointV6", 12.0, 24000)


pickup_case("PickupAmmoV6", MAT["olive"], icon_ammo)
pickup_case("PickupMedicalV6", MAT["medical"], icon_medical)
pickup_case("PickupArmorV6", MAT["armor"], icon_armor)
pickup_case("PickupUtilityV6", MAT["utility"], icon_utility)
pickup_case("PickupTechV6", MAT["tech"], icon_tech)
pickup_case("PickupSalvageV6", MAT["salvage"], icon_salvage)
build_threat_ring()
build_responder_pack()
build_rain_streak()
build_wind_debris()
build_logistics_depot()
build_weather_relay()
build_quarantine_checkpoint()

print("[WorldLootWeatherV6] COMPLETE PASS assets=13 contact_origins=grounded icon_first=1")
