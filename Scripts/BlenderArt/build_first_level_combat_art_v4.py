"""Build the first-level combat, wound, and armory art pass in Blender 5.x.

The script is deterministic and headless-safe. Geometry is authored in meters;
glTF/Interchange converts it to Unreal centimeters. Each exported GLB is a
single movable mesh with Principled materials and a ground/grip/contact origin.
"""

import math
import os

import bpy


PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "FirstLevelV4")
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
    "brick": material("CRV4_Brick", (0.19, 0.075, 0.052), 0.93),
    "mortar": material("CRV4_Mortar", (0.31, 0.30, 0.28), 0.9),
    "concrete": material("CRV4_Concrete", (0.30, 0.30, 0.29), 0.88),
    "concrete_dark": material("CRV4_ConcreteDark", (0.12, 0.12, 0.115), 0.9),
    "steel": material("CRV4_Steel", (0.16, 0.17, 0.18), 0.38, 0.85),
    "steel_light": material("CRV4_SteelLight", (0.38, 0.40, 0.41), 0.33, 0.9),
    "black": material("CRV4_BlackPolymer", (0.025, 0.027, 0.029), 0.58, 0.15),
    "glass": material("CRV4_GlassDark", (0.025, 0.055, 0.065), 0.16, 0.15),
    "amber": material("CRV4_AmberLamp", (0.35, 0.18, 0.04), 0.35, 0.15, (1.0, 0.48, 0.10), 5.5),
    "green": material("CRV4_MedicalGreen", (0.055, 0.22, 0.13), 0.62, 0.1),
    "red": material("CRV4_MedicalRed", (0.42, 0.025, 0.018), 0.55, 0.15),
    "canvas": material("CRV4_Canvas", (0.20, 0.23, 0.18), 0.95),
    "sandbag": material("CRV4_Sandbag", (0.30, 0.285, 0.22), 0.98),
    "wood": material("CRV4_Wood", (0.22, 0.11, 0.052), 0.82),
    "blade": material("CRV4_Blade", (0.46, 0.48, 0.49), 0.22, 0.95),
    "olive": material("CRV4_OlivePaint", (0.15, 0.19, 0.105), 0.58, 0.5),
    "wound": material("CRV4_Wound", (0.24, 0.006, 0.008), 0.72),
    "wound_dark": material("CRV4_WoundDark", (0.035, 0.002, 0.002), 0.9),
    "skin_edge": material("CRV4_WoundEdge", (0.34, 0.035, 0.026), 0.8),
}


def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)


def finish_part(obj, mat, bevel=0.0):
    obj.data.materials.append(mat)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel > 0.0:
        modifier = obj.modifiers.new("CRV4_EdgeRadius", "BEVEL")
        modifier.width = bevel
        modifier.segments = 2
        modifier.limit_method = "ANGLE"
        modifier.angle_limit = math.radians(28.0)
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)
    return obj


def box(name, location, size, mat, rotation=(0.0, 0.0, 0.0), bevel=0.025):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.scale = (size[0] * 0.5, size[1] * 0.5, size[2] * 0.5)
    return finish_part(obj, mat, bevel)


def cylinder(name, location, radius, depth, mat, axis="Z", vertices=24, bevel=0.008):
    rotation = (0.0, math.pi * 0.5, 0.0) if axis == "X" else (math.pi * 0.5, 0.0, 0.0) if axis == "Y" else (0.0, 0.0, 0.0)
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    return finish_part(obj, mat, bevel)


def sphere(name, location, scale, mat, segments=20, rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    return finish_part(obj, mat, 0.006)


def torus(name, location, major_radius, minor_radius, mat, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_torus_add(
        major_segments=32,
        minor_segments=8,
        location=location,
        rotation=rotation,
        major_radius=major_radius,
        minor_radius=minor_radius,
    )
    obj = bpy.context.object
    obj.name = name
    return finish_part(obj, mat, 0.0)


def wedge(name, vertices, faces, mat, bevel=0.008):
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return finish_part(obj, mat, bevel)


def export_asset(name, max_dimension=30.0, max_triangles=40000):
    parts = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    assert parts, f"{name}: no mesh parts"
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
    print(f"[FirstLevelV4] {name}: dims={tuple(round(v, 3) for v in asset.dimensions)} tris={triangles} -> {path}")


def build_storefront():
    clean_scene()
    box("ClosedBrickShell", (0, 0.4, 6.1), (18.0, 7.0, 12.2), MAT["brick"], bevel=0.045)
    box("Foundation", (0, 0.3, 0.22), (18.4, 7.25, 0.44), MAT["concrete_dark"], bevel=0.025)
    box("Cornice", (0, 0.2, 11.55), (18.5, 7.35, 0.42), MAT["concrete"], bevel=0.035)
    box("Parapet", (0, 0.25, 12.2), (18.15, 7.1, 0.85), MAT["brick"], bevel=0.03)
    front_y = -3.13
    box("StorefrontLintel", (0, front_y, 3.65), (17.4, 0.34, 0.42), MAT["steel"], bevel=0.018)
    for x in (-6.6, -3.4, 0.0, 3.4, 6.6):
        box("StorefrontMullion", (x, front_y - 0.06, 1.82), (0.18, 0.30, 3.55), MAT["steel"], bevel=0.012)
    for x in (-5.0, -1.7, 1.7, 5.0):
        box("StorefrontGlass", (x, front_y - 0.08, 2.0), (3.0, 0.12, 2.95), MAT["glass"], bevel=0.005)
    box("DoubleDoor", (0, front_y - 0.16, 1.52), (2.3, 0.19, 3.05), MAT["steel"], bevel=0.02)
    box("Awning", (0, front_y - 0.78, 4.05), (10.8, 1.55, 0.18), MAT["canvas"], rotation=(math.radians(8), 0, 0), bevel=0.025)
    box("ArmorySign", (0, front_y - 0.30, 5.15), (7.8, 0.22, 0.82), MAT["steel"], bevel=0.025)
    for sign_x in (-2.7, -1.8, -0.9, 0.0, 0.9, 1.8, 2.7):
        box("SignLamp", (sign_x, front_y - 0.45, 5.15), (0.38, 0.12, 0.28), MAT["amber"], bevel=0.04)
    for floor_z in (7.05, 9.6):
        for x in (-6.3, -3.15, 0.0, 3.15, 6.3):
            box("WindowRecess", (x, front_y - 0.02, floor_z), (1.85, 0.20, 1.55), MAT["concrete_dark"], bevel=0.018)
            box("WindowGlass", (x, front_y - 0.15, floor_z), (1.56, 0.10, 1.28), MAT["glass"], bevel=0.005)
            box("WindowCrossbar", (x, front_y - 0.23, floor_z), (1.68, 0.08, 0.09), MAT["steel"], bevel=0.005)
    # Fire escape and roof detail make the structure read as complete from side cameras.
    for z in (6.3, 8.8, 11.0):
        box("EscapeLanding", (6.3, front_y - 0.75, z), (4.0, 1.25, 0.12), MAT["steel"], bevel=0.01)
        for x in (4.4, 8.2):
            box("EscapeRail", (x, front_y - 1.25, z + 0.52), (0.08, 0.08, 1.05), MAT["steel_light"], bevel=0.008)
    for step in range(8):
        box("EscapeStair", (4.5 + step * 0.43, front_y - 1.1, 6.3 + step * 0.31), (0.48, 0.9, 0.09), MAT["steel"], bevel=0.006)
    box("RoofHVAC", (4.6, 0.8, 13.0), (2.5, 1.8, 1.4), MAT["steel"], bevel=0.035)
    cylinder("RoofVent", (-4.8, 0.5, 13.0), 0.38, 1.5, MAT["steel_light"], bevel=0.015)
    export_asset("FirstLevelStorefrontV4", 25.0, 30000)


def build_field_armory():
    clean_scene()
    box("ArmoryPlinth", (0, 0, 0.09), (4.9, 2.2, 0.18), MAT["concrete_dark"], bevel=0.035)
    box("RearCabinet", (0, 0.48, 1.5), (4.7, 0.72, 2.75), MAT["steel"], bevel=0.035)
    for x in (-1.72, -0.58, 0.58, 1.72):
        box("LockerDoor", (x, 0.08, 1.6), (1.02, 0.09, 2.36), MAT["olive"], bevel=0.025)
        box("LockerVent", (x, 0.015, 2.25), (0.48, 0.05, 0.22), MAT["black"], bevel=0.012)
        cylinder("LockerHandle", (x + 0.32, -0.055, 1.5), 0.035, 0.24, MAT["steel_light"], axis="Z", bevel=0.005)
    box("Workbench", (0, -0.57, 1.02), (4.65, 1.18, 0.17), MAT["wood"], bevel=0.035)
    for x in (-2.05, 2.05):
        box("BenchLeg", (x, -0.47, 0.52), (0.18, 0.18, 0.95), MAT["steel"], bevel=0.018)
    box("AmmoCrateA", (-1.15, -0.62, 1.34), (1.35, 0.78, 0.48), MAT["olive"], bevel=0.045)
    box("AmmoCrateB", (0.55, -0.62, 1.26), (1.4, 0.78, 0.32), MAT["steel"], bevel=0.035)
    box("TaskLightBar", (0, -0.04, 2.98), (3.3, 0.16, 0.12), MAT["amber"], bevel=0.035)
    # Readable display silhouettes: long gun, handgun, knife.
    box("DisplayRifle", (-0.7, -0.02, 2.05), (2.25, 0.12, 0.13), MAT["black"], rotation=(0, math.radians(-4), 0), bevel=0.035)
    box("DisplayRifleStock", (-1.76, -0.02, 2.06), (0.48, 0.14, 0.42), MAT["wood"], bevel=0.035)
    box("DisplayPistol", (1.25, -0.02, 2.12), (0.72, 0.13, 0.18), MAT["black"], bevel=0.03)
    box("DisplayPistolGrip", (1.05, -0.02, 1.87), (0.20, 0.14, 0.42), MAT["black"], rotation=(0, math.radians(-12), 0), bevel=0.025)
    box("DisplayKnife", (1.15, -0.02, 2.58), (1.15, 0.08, 0.07), MAT["blade"], rotation=(0, math.radians(6), 0), bevel=0.018)
    export_asset("FieldArmoryV4", 8.0, 18000)


def build_triage_checkpoint():
    clean_scene()
    for x in (-2.8, 2.8):
        for y in (-1.9, 1.9):
            cylinder("CanopyPost", (x, y, 1.55), 0.075, 3.1, MAT["steel"], bevel=0.008)
    box("CanopyRoof", (0, 0, 3.18), (6.2, 4.3, 0.18), MAT["canvas"], bevel=0.05)
    box("RearPrivacy", (0, 1.82, 1.55), (5.7, 0.10, 2.75), MAT["green"], bevel=0.015)
    box("TriageDesk", (-0.6, -0.45, 0.82), (3.2, 1.0, 0.18), MAT["steel_light"], bevel=0.035)
    for x in (-1.8, 0.6):
        box("TriageDeskLeg", (x, -0.45, 0.4), (0.16, 0.16, 0.82), MAT["steel"], bevel=0.015)
    box("MedicalCrate", (2.05, 0.65, 0.48), (1.25, 0.9, 0.9), MAT["green"], bevel=0.055)
    box("MedicalSign", (0, 1.73, 2.35), (2.3, 0.12, 0.75), MAT["concrete"], bevel=0.035)
    box("CrossVertical", (0, 1.63, 2.35), (0.23, 0.10, 0.60), MAT["red"], bevel=0.025)
    box("CrossHorizontal", (0, 1.62, 2.35), (0.60, 0.10, 0.23), MAT["red"], bevel=0.025)
    cylinder("TriageLamp", (-2.45, -1.65, 2.75), 0.13, 0.24, MAT["amber"], axis="Y", bevel=0.015)
    export_asset("TriageCheckpointV4", 8.0, 16000)


def build_sandbag_cover():
    clean_scene()
    for row, z in enumerate((0.22, 0.58, 0.94)):
        count = 7 - row
        for index in range(count):
            x = (index - (count - 1) * 0.5) * 0.62 + (0.28 if row % 2 else 0.0)
            sphere("Sandbag", (x, 0, z), (0.38, 0.28, 0.18), MAT["sandbag"], 16, 8)
    box("CoverBase", (0, 0, 0.055), (4.6, 0.9, 0.11), MAT["concrete_dark"], bevel=0.025)
    export_asset("SandbagCoverV4", 6.0, 25000)


def build_grenade():
    clean_scene()
    cylinder("GrenadeBody", (0.03, 0, 0), 0.052, 0.105, MAT["olive"], axis="X", vertices=24, bevel=0.006)
    for x in (-0.035, 0.0, 0.035):
        torus("BodyRib", (x, 0, 0), 0.052, 0.004, MAT["steel"], rotation=(0, math.pi * 0.5, 0))
    cylinder("FuseNeck", (-0.045, 0, 0.047), 0.022, 0.045, MAT["steel"], axis="Z", vertices=16, bevel=0.004)
    box("SafetyLever", (-0.035, 0, 0.078), (0.092, 0.028, 0.010), MAT["steel_light"], rotation=(0, math.radians(-8), 0), bevel=0.004)
    torus("PullRing", (-0.045, -0.045, 0.075), 0.025, 0.003, MAT["steel_light"], rotation=(math.pi * 0.5, 0, 0))
    export_asset("GrenadeV4", 0.4, 12000)


def build_knife():
    clean_scene()
    verts = [
        (0.0, -0.025, -0.018), (0.0, 0.025, -0.018), (0.0, 0.025, 0.018), (0.0, -0.025, 0.018),
        (0.55, -0.016, -0.012), (0.55, 0.016, -0.012), (0.55, 0.0, 0.015), (0.55, 0.0, 0.015),
    ]
    faces = [(0, 1, 2, 3), (0, 4, 5, 1), (3, 2, 6), (0, 3, 6, 4), (1, 5, 6, 2), (4, 6, 5)]
    wedge("KnifeBlade", verts, faces, MAT["blade"], 0.006)
    box("KnifeGuard", (-0.035, 0, 0), (0.055, 0.18, 0.035), MAT["steel"], bevel=0.008)
    cylinder("KnifeHandle", (-0.19, 0, 0), 0.035, 0.33, MAT["black"], axis="X", vertices=16, bevel=0.006)
    for x in (-0.08, -0.14, -0.20, -0.26, -0.32):
        torus("GripRib", (x, 0, 0), 0.036, 0.004, MAT["olive"], rotation=(0, math.pi * 0.5, 0))
    box("KnifePommel", (-0.37, 0, 0), (0.05, 0.09, 0.08), MAT["steel"], bevel=0.012)
    export_asset("CombatKnifeV4", 1.0, 12000)


def build_launcher():
    clean_scene()
    cylinder("LauncherTube", (0, 0, 0.10), 0.092, 1.32, MAT["olive"], axis="X", vertices=28, bevel=0.008)
    cylinder("FrontCollar", (0.59, 0, 0.10), 0.118, 0.12, MAT["steel"], axis="X", vertices=28, bevel=0.01)
    cylinder("RearCollar", (-0.60, 0, 0.10), 0.112, 0.13, MAT["steel"], axis="X", vertices=28, bevel=0.01)
    cylinder("WarheadNose", (0.73, 0, 0.10), 0.08, 0.22, MAT["olive"], axis="X", vertices=24, bevel=0.01)
    box("LauncherGrip", (-0.15, 0, -0.08), (0.20, 0.105, 0.32), MAT["black"], rotation=(0, math.radians(-10), 0), bevel=0.025)
    box("ShoulderPad", (-0.70, 0, 0.05), (0.13, 0.24, 0.28), MAT["black"], bevel=0.035)
    box("SightRail", (0.08, 0, 0.22), (0.55, 0.055, 0.045), MAT["steel_light"], bevel=0.012)
    box("SightPost", (0.22, 0, 0.29), (0.055, 0.06, 0.13), MAT["steel_light"], bevel=0.01)
    export_asset("RocketLauncherV4", 2.0, 16000)


def build_wound_cavity():
    clean_scene()
    cylinder("CavityCore", (0, 0, 0), 0.035, 0.009, MAT["wound_dark"], axis="X", vertices=24, bevel=0.002)
    torus("TornEdge", (-0.003, 0, 0), 0.038, 0.009, MAT["wound"], rotation=(0, math.pi * 0.5, 0))
    for angle in (0.2, 1.65, 3.0, 4.45, 5.6):
        y = math.cos(angle) * 0.043
        z = math.sin(angle) * 0.043
        sphere("TissueTear", (-0.008, y, z), (0.010, 0.012, 0.008), MAT["skin_edge"], 12, 6)
    export_asset("WoundCavityV4", 0.2, 12000)


def build_bite_wound():
    clean_scene()
    cylinder("BiteBruise", (0, 0, 0), 0.055, 0.007, MAT["wound"], axis="X", vertices=28, bevel=0.002)
    for arc_sign in (-1.0, 1.0):
        for index in range(5):
            angle = math.radians(-55.0 + index * 27.5)
            y = math.sin(angle) * 0.052
            z = arc_sign * (0.024 + math.cos(angle) * 0.022)
            sphere("ToothPuncture", (-0.009, y, z), (0.010, 0.008, 0.008), MAT["wound_dark"], 12, 6)
            torus("PunctureEdge", (-0.012, y, z), 0.008, 0.002, MAT["skin_edge"], rotation=(0, math.pi * 0.5, 0))
    export_asset("BiteWoundV4", 0.2, 18000)


for builder in (
    build_storefront,
    build_field_armory,
    build_triage_checkpoint,
    build_sandbag_cover,
    build_grenade,
    build_knife,
    build_launcher,
    build_wound_cavity,
    build_bite_wound,
):
    builder()

print("[FirstLevelV4] complete: 9 authored assets")
