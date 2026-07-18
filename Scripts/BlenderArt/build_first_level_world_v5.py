"""Build the first-level accessible-building and sky art pass in Blender 5.x.

Geometry is authored in meters and exported as one GLB per asset. Building
shells have literal doorway openings, walkable floors, and interior fixtures;
the star field contains only small emissive stars, never a camera-blocking dome.
"""

import math
import os
import random

import bpy


PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "FirstLevelV5")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.context.scene.unit_settings.system = "METRIC"
bpy.context.scene.unit_settings.scale_length = 1.0


def material(name, color, roughness=0.72, metallic=0.0, emission=None, emission_strength=0.0):
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
    "brick": material("CRV5_Brick", (0.23, 0.075, 0.045), 0.94),
    "brick_light": material("CRV5_BrickLight", (0.38, 0.13, 0.07), 0.92),
    "concrete": material("CRV5_Concrete", (0.34, 0.35, 0.34), 0.88),
    "concrete_dark": material("CRV5_ConcreteDark", (0.095, 0.105, 0.11), 0.9),
    "tile": material("CRV5_InteriorTile", (0.35, 0.40, 0.39), 0.78),
    "steel": material("CRV5_Steel", (0.13, 0.15, 0.16), 0.34, 0.78),
    "steel_light": material("CRV5_SteelLight", (0.40, 0.43, 0.44), 0.28, 0.86),
    "glass": material("CRV5_Glass", (0.035, 0.10, 0.12), 0.12, 0.08),
    "wood": material("CRV5_Wood", (0.27, 0.12, 0.045), 0.82),
    "market": material("CRV5_MarketGreen", (0.055, 0.25, 0.14), 0.58, 0.05),
    "clinic": material("CRV5_ClinicBlue", (0.045, 0.18, 0.29), 0.54, 0.08),
    "cafe": material("CRV5_CafeRed", (0.42, 0.055, 0.035), 0.62, 0.04),
    "amber": material("CRV5_Amber", (0.42, 0.18, 0.035), 0.30, 0.12, (1.0, 0.42, 0.08), 8.0),
    "medical": material("CRV5_Medical", (0.60, 0.035, 0.025), 0.50, 0.08, (1.0, 0.05, 0.03), 2.5),
    "star_white": material("CRV5_StarWhite", (0.75, 0.80, 1.0), 0.1, 0.0, (0.82, 0.88, 1.0), 18.0),
    "star_blue": material("CRV5_StarBlue", (0.40, 0.58, 1.0), 0.1, 0.0, (0.48, 0.66, 1.0), 15.0),
    "star_warm": material("CRV5_StarWarm", (1.0, 0.72, 0.42), 0.1, 0.0, (1.0, 0.66, 0.34), 14.0),
    "moon": material("CRV5_Moon", (0.68, 0.70, 0.66), 0.82, 0.0, (0.56, 0.60, 0.64), 2.2),
    "moon_dark": material("CRV5_MoonCrater", (0.18, 0.20, 0.20), 0.95),
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
        modifier = obj.modifiers.new("CRV5_EdgeRadius", "BEVEL")
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
    return finish_part(obj, mat, 0.004)


def wedge_ramp(name, center_x, front_y, width, run, rise, mat):
    y0 = front_y - run
    y1 = front_y
    x0 = center_x - width * 0.5
    x1 = center_x + width * 0.5
    vertices = [
        (x0, y0, 0.0), (x1, y0, 0.0), (x0, y1, 0.0), (x1, y1, 0.0),
        (x0, y1, rise), (x1, y1, rise),
    ]
    faces = [(0, 1, 3, 2), (2, 3, 5, 4), (0, 2, 4), (1, 5, 3), (0, 4, 5, 1)]
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return finish_part(obj, mat, 0.012)


def export_asset(name, max_dimension, max_triangles):
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
    print(f"[FirstLevelV5] {name}: dims={tuple(round(v, 3) for v in asset.dimensions)} tris={triangles} -> {path}")


def shell_base(accent, width=15.5, depth=10.0, height=8.8):
    front_y = -depth * 0.5
    wall = 0.28
    box("WalkableFloor", (0.0, 0.0, 0.10), (width, depth, 0.20), MAT["tile"], bevel=0.018)
    box("RearWall", (0.0, depth * 0.5 - wall * 0.5, height * 0.5), (width, wall, height), MAT["brick"], bevel=0.025)
    box("LeftWall", (-width * 0.5 + wall * 0.5, 0.0, height * 0.5), (wall, depth, height), MAT["brick"], bevel=0.025)
    box("RightWall", (width * 0.5 - wall * 0.5, 0.0, height * 0.5), (wall, depth, height), MAT["brick"], bevel=0.025)
    box("Roof", (0.0, 0.0, height - 0.14), (width + 0.2, depth + 0.2, 0.28), MAT["concrete_dark"], bevel=0.035)
    # Front wall is assembled around a literal 3.2m x 3.5m doorway.
    side_width = (width - 3.2) * 0.5
    for side in (-1.0, 1.0):
        x = side * (3.2 * 0.5 + side_width * 0.5)
        box("FrontWallSide", (x, front_y + wall * 0.5, 2.25), (side_width, wall, 4.5), MAT["brick_light"], bevel=0.022)
    box("FrontUpper", (0.0, front_y + wall * 0.5, 6.65), (width, wall, 4.3), MAT["brick"], bevel=0.022)
    for x in (-5.25, -2.6, 2.6, 5.25):
        box("UpperWindow", (x, front_y - 0.05, 6.75), (1.75, 0.12, 1.65), MAT["glass"], bevel=0.012)
        box("WindowSill", (x, front_y - 0.12, 5.85), (2.0, 0.28, 0.14), MAT["concrete"], bevel=0.018)
    for x in (-1.72, 1.72):
        box("DoorFramePost", (x, front_y - 0.05, 1.80), (0.18, 0.34, 3.60), MAT["steel"], bevel=0.014)
    box("DoorFrameHeader", (0.0, front_y - 0.05, 3.52), (3.62, 0.34, 0.20), MAT["steel"], bevel=0.014)
    box("EntryAwning", (0.0, front_y - 1.05, 4.18), (6.4, 2.25, 0.20), accent, rotation=(math.radians(6.0), 0.0, 0.0), bevel=0.04)
    box("EntrySign", (0.0, front_y - 0.20, 4.78), (5.4, 0.22, 0.76), accent, bevel=0.035)
    for x in (-2.0, -1.0, 0.0, 1.0, 2.0):
        box("SignLamp", (x, front_y - 0.35, 4.78), (0.32, 0.10, 0.20), MAT["amber"], bevel=0.035)
    wedge_ramp("AccessibleEntryRamp", 0.0, front_y + 0.35, 3.05, 2.4, 0.22, MAT["concrete"])
    # Interior ceiling strips stay visible from the street and make the open
    # doorway read as a destination even before runtime point lights engage.
    for x in (-4.5, 0.0, 4.5):
        box("InteriorLight", (x, 0.5, height - 0.48), (2.4, 0.24, 0.10), MAT["amber"], bevel=0.03)
    return front_y, height


def build_accessible_market():
    clean_scene()
    front_y, _ = shell_base(MAT["market"])
    box("MarketCounter", (-4.3, 1.6, 0.72), (4.6, 1.0, 1.25), MAT["wood"], bevel=0.05)
    for x in (-5.2, -2.7, 2.9, 5.2):
        box("MarketShelf", (x, 3.2, 1.45), (1.4, 0.55, 2.7), MAT["steel"], bevel=0.035)
        for z in (0.65, 1.45, 2.25):
            box("ShelfStock", (x, 2.85, z), (1.05, 0.22, 0.28), MAT["market"], bevel=0.022)
    box("CheckoutRegister", (-4.2, 0.94, 1.55), (0.72, 0.55, 0.45), MAT["steel_light"], bevel=0.045)
    box("OpenThreshold", (0.0, front_y - 0.02, 0.24), (2.9, 0.58, 0.08), MAT["market"], bevel=0.018)
    export_asset("AccessibleMarketV5", 22.0, 45000)


def build_accessible_clinic():
    clean_scene()
    front_y, _ = shell_base(MAT["clinic"])
    for x in (-4.5, 0.0, 4.5):
        box("ClinicCot", (x, 2.4, 0.52), (3.0, 1.15, 0.42), MAT["concrete"], bevel=0.07)
        box("ClinicPillow", (x, 2.82, 0.80), (0.92, 0.48, 0.22), MAT["tile"], bevel=0.08)
        cylinder("IVStand", (x + 1.15, 1.95, 1.15), 0.035, 2.3, MAT["steel_light"], bevel=0.006)
    box("ClinicDesk", (-4.8, -1.0, 0.82), (3.9, 1.0, 1.35), MAT["steel_light"], bevel=0.045)
    box("MedicalCrossVertical", (0.0, front_y - 0.36, 4.80), (0.30, 0.08, 0.60), MAT["medical"], bevel=0.025)
    box("MedicalCrossHorizontal", (0.0, front_y - 0.37, 4.80), (0.60, 0.08, 0.30), MAT["medical"], bevel=0.025)
    box("OpenThreshold", (0.0, front_y - 0.02, 0.24), (2.9, 0.58, 0.08), MAT["clinic"], bevel=0.018)
    export_asset("AccessibleClinicV5", 22.0, 50000)


def build_open_street_cafe():
    clean_scene()
    front_y, _ = shell_base(MAT["cafe"], width=12.5, depth=8.2, height=6.3)
    box("CafeBar", (0.0, 2.55, 0.80), (7.2, 0.92, 1.30), MAT["wood"], bevel=0.055)
    for x in (-4.0, -1.35, 1.35, 4.0):
        cylinder("CafeTablePost", (x, 0.0, 0.52), 0.065, 1.0, MAT["steel"], bevel=0.008)
        cylinder("CafeTableTop", (x, 0.0, 1.04), 0.72, 0.10, MAT["wood"], bevel=0.025)
        for y in (-1.05, 1.05):
            box("CafeBench", (x, y, 0.58), (1.35, 0.45, 0.20), MAT["steel_light"], bevel=0.045)
    box("OpenThreshold", (0.0, front_y - 0.02, 0.24), (2.9, 0.58, 0.08), MAT["cafe"], bevel=0.018)
    export_asset("OpenStreetCafeV5", 20.0, 50000)


def build_star_field():
    clean_scene()
    rng = random.Random(570109)
    star_materials = (MAT["star_white"], MAT["star_white"], MAT["star_blue"], MAT["star_warm"])
    for index in range(210):
        azimuth = rng.uniform(0.0, math.tau)
        elevation = rng.uniform(math.radians(12.0), math.radians(82.0))
        radius = rng.uniform(125.0, 170.0)
        location = (
            math.cos(elevation) * math.cos(azimuth) * radius,
            math.cos(elevation) * math.sin(azimuth) * radius,
            math.sin(elevation) * radius,
        )
        size = rng.uniform(0.055, 0.14) * (1.8 if index % 41 == 0 else 1.0)
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=size, location=location)
        star = bpy.context.object
        star.name = "NightStar"
        finish_part(star, star_materials[index % len(star_materials)], 0.0)
    export_asset("PointStarFieldV5", 380.0, 12000)


def build_moon():
    clean_scene()
    sphere("MoonBody", (0.0, 0.0, 0.0), (4.5, 4.5, 4.5), MAT["moon"], 48, 24)
    crater_specs = [
        ((-4.30, -0.70, 0.65), (0.18, 0.72, 0.64)),
        ((-4.35, 1.45, 1.15), (0.12, 0.48, 0.45)),
        ((-4.38, -1.65, -1.05), (0.10, 0.55, 0.50)),
        ((-4.40, 0.85, -1.65), (0.08, 0.40, 0.38)),
    ]
    for location, scale in crater_specs:
        sphere("MoonCrater", location, scale, MAT["moon_dark"], 18, 10)
    export_asset("MoonDetailedV5", 12.0, 35000)


for builder in (
    build_accessible_market,
    build_accessible_clinic,
    build_open_street_cafe,
    build_star_field,
    build_moon,
):
    builder()

print("[FirstLevelV5] complete: 5 authored assets")

