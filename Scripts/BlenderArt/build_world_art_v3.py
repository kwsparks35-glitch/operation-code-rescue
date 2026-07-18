"""build_world_art_v3.py — Operation Code Rescue city kit, third generation.

Kenny's direction (2026-07-06): the world must "actually look realistic" —
completed structures, real sidewalks, vehicles with wheels and glass, street
furniture — comfortably realistic, Resident Evil Requiem mood, NOT photoreal.

Everything is built procedurally in meters (glTF exports meters; UE imports
x100 to cm). Every mesh gets principled-BSDF materials (base color, roughness,
metallic, optional emission) which UE's glTF importer converts to real
material instances — this is the single biggest realism jump over v2's flat
vertex colors.

Run inside Blender 5.x:  exec(open(r"<this file>").read())
Outputs:  RawArt/CityKitV3/<Name>.glb   (one file per asset)

Blender 5.1 lessons honored (see art memory 2026-07-04):
  * after primitive_add(location=..)+transform_apply, vert coords are WORLD
    space — all math below treats them as such;
  * every asset is validated (bounds + tri budget) before export;
  * objects are single-user, transforms applied, origins at sensible points
    (ground contact for buildings/props, axle height for vehicles).
"""
import bpy
import bmesh
import math
import os
import random

PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "CityKitV3")
os.makedirs(OUT_DIR, exist_ok=True)

random.seed(20260706)

# ---------------------------------------------------------------- helpers --

def clean_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    # NOTE: only meshes are purged. Materials must SURVIVE between builds —
    # the MAT dict holds live references, and removing zero-user materials
    # here turned them into dangling StructRNA on the second asset.
    for block in list(bpy.data.meshes):
        if block.users == 0:
            bpy.data.meshes.remove(block)


def make_mat(name, color, rough=0.8, metal=0.0, emit=None, emit_strength=3.0, alpha=1.0):
    m = bpy.data.materials.get(name)
    if m:
        return m
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = rough
    bsdf.inputs["Metallic"].default_value = metal
    if alpha < 1.0:
        bsdf.inputs["Alpha"].default_value = alpha
        m.blend_method = "BLEND"
    if emit is not None:
        bsdf.inputs["Emission Color"].default_value = (*emit, 1.0)
        bsdf.inputs["Emission Strength"].default_value = emit_strength
    return m


# Shared palette — muted, weathered, survival-horror. No saturated primaries.
MAT = {
    "brick_red":    make_mat("CR_BrickRed",    (0.196, 0.102, 0.072), 0.94),
    "brick_brown":  make_mat("CR_BrickBrown",  (0.165, 0.118, 0.088), 0.94),
    "concrete":     make_mat("CR_Concrete",    (0.372, 0.368, 0.352), 0.88),
    "concrete_drk": make_mat("CR_ConcreteDrk", (0.252, 0.250, 0.242), 0.90),
    "stucco":       make_mat("CR_Stucco",      (0.430, 0.402, 0.348), 0.86),
    "asphalt":      make_mat("CR_Asphalt",     (0.058, 0.058, 0.062), 0.95),
    "asphalt_worn": make_mat("CR_AsphaltWorn", (0.085, 0.084, 0.086), 0.93),
    "lane_paint":   make_mat("CR_LanePaint",   (0.520, 0.500, 0.420), 0.75),
    "sidewalk":     make_mat("CR_Sidewalk",    (0.310, 0.305, 0.290), 0.87),
    "curb":         make_mat("CR_Curb",        (0.360, 0.352, 0.335), 0.85),
    "glass":        make_mat("CR_Glass",       (0.045, 0.065, 0.070), 0.12, 0.0, alpha=0.42),
    "glass_dark":   make_mat("CR_GlassDark",   (0.020, 0.028, 0.030), 0.10, 0.0, alpha=0.55),
    "window_lit":   make_mat("CR_WindowLit",   (0.06, 0.05, 0.03), 0.4, 0.0, emit=(1.0, 0.72, 0.38), emit_strength=2.2),
    "metal_dark":   make_mat("CR_MetalDark",   (0.090, 0.092, 0.098), 0.45, 0.85),
    "metal_rust":   make_mat("CR_MetalRust",   (0.238, 0.130, 0.082), 0.80, 0.35),
    "metal_paint":  make_mat("CR_MetalPaint",  (0.135, 0.160, 0.148), 0.55, 0.60),
    "steel":        make_mat("CR_Steel",       (0.320, 0.325, 0.335), 0.35, 0.90),
    "car_red":      make_mat("CR_CarRed",      (0.262, 0.052, 0.042), 0.42, 0.70),
    "car_blue":     make_mat("CR_CarBlue",     (0.052, 0.082, 0.132), 0.40, 0.70),
    "car_white":    make_mat("CR_CarWhite",    (0.520, 0.522, 0.512), 0.45, 0.65),
    "car_police":   make_mat("CR_CarPolice",   (0.062, 0.066, 0.078), 0.38, 0.70),
    "tire":         make_mat("CR_Tire",        (0.028, 0.028, 0.028), 0.96),
    "rim":          make_mat("CR_Rim",         (0.250, 0.252, 0.258), 0.35, 0.9),
    "headlight":    make_mat("CR_Headlight",   (0.7, 0.7, 0.62), 0.2, 0.1, emit=(0.9, 0.88, 0.7), emit_strength=0.6),
    "taillight":    make_mat("CR_Taillight",   (0.25, 0.02, 0.02), 0.3, 0.1, emit=(0.8, 0.05, 0.03), emit_strength=0.8),
    "lightbar_r":   make_mat("CR_LightbarR",   (0.3, 0.02, 0.02), 0.3, 0.0, emit=(1.0, 0.05, 0.05), emit_strength=4.0),
    "lightbar_b":   make_mat("CR_LightbarB",   (0.02, 0.05, 0.3), 0.3, 0.0, emit=(0.10, 0.25, 1.0), emit_strength=4.0),
    "street_lamp":  make_mat("CR_StreetLamp",  (0.95, 0.85, 0.60), 0.3, 0.0, emit=(1.0, 0.83, 0.52), emit_strength=6.0),
    "traffic_red":  make_mat("CR_TrafficRed",  (0.3, 0.03, 0.02), 0.4, 0.0, emit=(1.0, 0.10, 0.05), emit_strength=3.0),
    "hydrant":      make_mat("CR_Hydrant",     (0.310, 0.075, 0.055), 0.55, 0.30),
    "awning":       make_mat("CR_Awning",      (0.185, 0.115, 0.085), 0.85),
    "door_wood":    make_mat("CR_DoorWood",    (0.180, 0.115, 0.072), 0.78),
    "trash_bag":    make_mat("CR_TrashBag",    (0.045, 0.048, 0.050), 0.60),
    "roof_tar":     make_mat("CR_RoofTar",     (0.085, 0.082, 0.080), 0.94),
    "plate":        make_mat("CR_Plate",       (0.42, 0.42, 0.38), 0.5, 0.3),
}


def new_obj(name):
    mesh = bpy.data.meshes.new(name)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return obj


class Builder:
    """Accumulates boxes/cylinders into one bmesh with per-face materials."""

    def __init__(self, name):
        self.obj = new_obj(name)
        self.bm = bmesh.new()
        self.mats = []

    def _mat_index(self, mat):
        if mat.name not in [m.name for m in self.mats]:
            self.mats.append(mat)
        return [m.name for m in self.mats].index(mat.name)

    def box(self, cx, cy, cz, sx, sy, sz, mat, rz=0.0):
        idx = self._mat_index(mat)
        res = bmesh.ops.create_cube(self.bm, size=1.0)
        verts = res["verts"]
        for v in verts:
            v.co.x *= sx
            v.co.y *= sy
            v.co.z *= sz
            if rz:
                x, y = v.co.x, v.co.y
                c, s = math.cos(rz), math.sin(rz)
                v.co.x = x * c - y * s
                v.co.y = x * s + y * c
            v.co.x += cx
            v.co.y += cy
            v.co.z += cz
        for f in {f for v in verts for f in v.link_faces}:
            f.material_index = idx
        return verts

    def cyl(self, cx, cy, cz, radius, depth, mat, segments=14, axis="Z"):
        idx = self._mat_index(mat)
        res = bmesh.ops.create_cone(
            self.bm, cap_ends=True, segments=segments,
            radius1=radius, radius2=radius, depth=depth)
        verts = res["verts"]
        for v in verts:
            if axis == "X":
                v.co.x, v.co.z = v.co.z, -v.co.x
            elif axis == "Y":
                v.co.y, v.co.z = v.co.z, -v.co.y
            v.co.x += cx
            v.co.y += cy
            v.co.z += cz
        for f in {f for v in verts for f in v.link_faces}:
            f.material_index = idx
        return verts

    def tapered_box(self, cx, cy, cz, bottom_x, bottom_y, top_x, top_y,
                    height, mat, top_shift_x=0.0, rz=0.0):
        """Closed frustum used for sloped vehicle cabins and architectural caps."""
        idx = self._mat_index(mat)
        coords = [
            (-bottom_x / 2, -bottom_y / 2, -height / 2),
            ( bottom_x / 2, -bottom_y / 2, -height / 2),
            ( bottom_x / 2,  bottom_y / 2, -height / 2),
            (-bottom_x / 2,  bottom_y / 2, -height / 2),
            (-top_x / 2 + top_shift_x, -top_y / 2, height / 2),
            ( top_x / 2 + top_shift_x, -top_y / 2, height / 2),
            ( top_x / 2 + top_shift_x,  top_y / 2, height / 2),
            (-top_x / 2 + top_shift_x,  top_y / 2, height / 2),
        ]
        verts = []
        c, s = math.cos(rz), math.sin(rz)
        for x, y, z in coords:
            verts.append(self.bm.verts.new((x * c - y * s + cx,
                                            x * s + y * c + cy,
                                            z + cz)))
        for indices in ((0, 1, 2, 3), (4, 7, 6, 5),
                        (0, 4, 5, 1), (1, 5, 6, 2),
                        (2, 6, 7, 3), (3, 7, 4, 0)):
            face = self.bm.faces.new([verts[i] for i in indices])
            face.material_index = idx
        return verts

    def finish(self):
        self.bm.to_mesh(self.obj.data)
        self.bm.free()
        for m in self.mats:
            self.obj.data.materials.append(m)
        return self.obj


def validate(obj, max_dim, max_tris, min_dim=0.05):
    dims = obj.dimensions
    tris = sum(len(p.vertices) - 2 for p in obj.data.polygons)
    assert max(dims) <= max_dim + 1e-3, f"{obj.name}: too big {tuple(dims)} > {max_dim}"
    assert max(dims) >= min_dim, f"{obj.name}: degenerate {tuple(dims)}"
    assert tris <= max_tris, f"{obj.name}: {tris} tris > {max_tris}"
    print(f"[v3] OK {obj.name}: dims=({dims.x:.2f},{dims.y:.2f},{dims.z:.2f}) tris={tris}")


def export(obj, name):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    # Production edge treatment. The authored forms are intentionally
    # economical, but perfectly sharp 90-degree edges make full-scale props
    # read as blocking volumes. Small real-world edge radii create stable
    # specular highlights and dramatically improve silhouette readability.
    if name.startswith(("Road", "Sidewalk")):
        bevel_width, bevel_segments = 0.006, 1
    elif name.startswith("Building"):
        bevel_width, bevel_segments = 0.025, 2
    elif name.endswith(("SedanCleanV3", "SedanCrashedV3", "CruiserV3", "VanV3", "PickupV3")):
        bevel_width, bevel_segments = 0.035, 2
    else:
        bevel_width, bevel_segments = 0.015, 2
    bevel = obj.modifiers.new(name="CR_ProductionEdgeRadius", type="BEVEL")
    bevel.width = bevel_width
    bevel.segments = bevel_segments
    bevel.limit_method = "ANGLE"
    bevel.angle_limit = math.radians(28.0)
    bevel.harden_normals = True
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    obj.data.validate(clean_customdata=False)
    path = os.path.join(OUT_DIR, f"{name}.glb")
    bpy.ops.export_scene.gltf(
        filepath=path, use_selection=True, export_format="GLB",
        export_apply=True, export_yup=True)
    print(f"[v3] exported {path}")


# ------------------------------------------------------------- buildings --

def build_building(name, floors, width, depth, wall_mat, lit_ratio=0.25,
                   storefront=True, fire_escape=False):
    """A CLOSED building volume with inset window grid, storefront, cornice,
    parapet and rooftop clutter. Origin at ground center."""
    b = Builder(name)
    floor_h = 3.0
    h = floors * floor_h + 1.2
    # main shell (closed)
    b.box(0, 0, h / 2, width, depth, h, wall_mat)
    # foundation skirt + cornice + parapet
    b.box(0, 0, 0.18, width + 0.22, depth + 0.22, 0.36, MAT["concrete_drk"])
    b.box(0, 0, h - 0.55, width + 0.30, depth + 0.30, 0.28, MAT["concrete_drk"])
    b.box(0, 0, h + 0.14, width + 0.16, depth + 0.16, 0.30, wall_mat)
    # tar roof + clutter
    b.box(0, 0, h + 0.30, width - 0.5, depth - 0.5, 0.06, MAT["roof_tar"])
    b.box(width * 0.22, depth * 0.18, h + 0.75, 1.4, 1.1, 0.9, MAT["metal_paint"])   # AC unit
    b.cyl(-width * 0.25, -depth * 0.2, h + 0.85, 0.28, 1.1, MAT["metal_rust"])       # vent pipe
    b.box(-width * 0.05, depth * 0.28, h + 1.0, 0.010, 0.010, 1.4, MAT["steel"])     # antenna
    # window grid on front (+Y) and back (-Y); sides get sparse windows
    win_w, win_h, win_d = 0.95, 1.35, 0.12
    cols = max(2, int((width - 1.6) // 1.7))
    x0 = -(cols - 1) * 1.7 / 2.0
    start_floor = 1 if storefront else 0
    for fl in range(start_floor, floors):
        zc = fl * floor_h + 1.9
        for c in range(cols):
            xc = x0 + c * 1.7
            lit = random.random() < lit_ratio
            wmat = MAT["window_lit"] if lit else MAT["glass_dark"]
            for ysign in (1, -1):
                yc = ysign * (depth / 2)
                # inset frame + glass pane proud of the wall by ~2cm
                b.box(xc, yc + ysign * 0.015, zc, win_w + 0.18, 0.10, win_h + 0.18, MAT["concrete_drk"])
                b.box(xc, yc + ysign * 0.035, zc, win_w, win_d, win_h, wmat)
                # sill
                b.box(xc, yc + ysign * 0.09, zc - win_h / 2 - 0.09, win_w + 0.26, 0.22, 0.08, MAT["concrete_drk"])
    # sparse side windows
    side_cols = max(1, int((depth - 1.6) // 2.2))
    y0 = -(side_cols - 1) * 2.2 / 2.0
    for fl in range(1, floors):
        zc = fl * floor_h + 1.9
        for c in range(side_cols):
            yc = y0 + c * 2.2
            for xsign in (1, -1):
                xc = xsign * (width / 2)
                b.box(xc + xsign * 0.015, yc, zc, 0.10, win_w + 0.18, win_h + 0.18, MAT["concrete_drk"])
                b.box(xc + xsign * 0.035, yc, zc, win_d, win_w, win_h, MAT["glass_dark"])
    if storefront:
        # ground floor: big glass, door, awning
        b.box(0, depth / 2 + 0.02, 1.5, width - 1.2, 0.14, 2.4, MAT["glass"])
        b.box(0, depth / 2 + 0.01, 1.5, width - 1.0, 0.06, 2.6, MAT["metal_dark"])
        b.box(width * 0.30, depth / 2 + 0.10, 1.15, 1.05, 0.16, 2.3, MAT["door_wood"])
        b.box(0, depth / 2 + 0.55, 2.85, width - 0.9, 1.1, 0.10, MAT["awning"])
        b.box(0, depth / 2 + 0.55, 2.70, width - 0.95, 1.05, 0.22, MAT["awning"])
    if fire_escape:
        # zig-zag platforms + rails on the front-left
        fx = -width * 0.30
        for fl in range(1, floors):
            zc = fl * floor_h + 0.55
            b.box(fx, depth / 2 + 0.65, zc, 2.4, 1.2, 0.07, MAT["metal_rust"])
            for rx in (-1.15, 1.15):
                b.box(fx + rx, depth / 2 + 0.65, zc + 0.5, 0.05, 1.15, 1.0, MAT["metal_rust"])
            b.box(fx, depth / 2 + 1.22, zc + 0.5, 2.4, 0.05, 1.0, MAT["metal_rust"])
            # slanted ladder suggestion (thin box)
            b.box(fx + (0.9 if fl % 2 else -0.9), depth / 2 + 0.62, zc + floor_h / 2 - 0.2,
                  0.5, 0.08, floor_h * 0.96, MAT["metal_dark"], rz=0.0)
    obj = b.finish()
    validate(obj, max_dim=max(floors * 3.2 + 3.5, width + 3, depth + 3), max_tris=26000)
    export(obj, name)
    return obj


# ---------------------------------------------------------------- street --

def build_road_straight():
    b = Builder("RoadStraightV3")
    b.box(0, 0, -0.04, 8.0, 7.0, 0.08, MAT["asphalt"])
    # worn center patches
    for i in range(4):
        b.box(-3.0 + i * 2.0, random.uniform(-1.6, 1.6), 0.002, 1.7, 1.3, 0.004, MAT["asphalt_worn"])
    # dashed center line + edge lines
    for i in range(5):
        b.box(-3.2 + i * 1.6, 0, 0.006, 0.85, 0.14, 0.004, MAT["lane_paint"])
    for ysign in (1, -1):
        b.box(0, ysign * 3.1, 0.006, 8.0, 0.13, 0.004, MAT["lane_paint"])
    # Repairs and drainage detail break the uniform procedural surface.
    b.cyl(2.15, -1.35, 0.010, 0.34, 0.015, MAT["metal_dark"], segments=20)
    b.cyl(2.15, -1.35, 0.019, 0.25, 0.010, MAT["metal_rust"], segments=20)
    for x, y, length, angle in ((-2.6, 1.5, 1.5, 0.35), (0.4, -2.2, 1.1, -0.55), (3.0, 2.0, 0.9, 0.2)):
        b.box(x, y, 0.010, length, 0.025, 0.006, MAT["asphalt_worn"], rz=angle)
    obj = b.finish()
    validate(obj, 9.0, 2000)
    export(obj, "RoadStraightV3")


def build_road_intersection():
    b = Builder("RoadIntersectionV3")
    b.box(0, 0, -0.04, 8.0, 8.0, 0.08, MAT["asphalt"])
    # crosswalk bars on all 4 approaches
    for side in range(4):
        rz = side * math.pi / 2
        for i in range(6):
            off = -1.9 + i * 0.76
            x = math.cos(rz) * 3.2 - math.sin(rz) * off
            y = math.sin(rz) * 3.2 + math.cos(rz) * off
            b.box(x, y, 0.006, 0.5, 0.62, 0.004, MAT["lane_paint"], rz=rz)
    obj = b.finish()
    validate(obj, 9.0, 2200)
    export(obj, "RoadIntersectionV3")


def build_sidewalk():
    b = Builder("SidewalkV3")
    b.box(0, 0, 0.075, 8.0, 2.4, 0.15, MAT["sidewalk"])
    b.box(0, -1.28, 0.09, 8.0, 0.16, 0.18, MAT["curb"])
    # expansion joints
    for i in range(5):
        b.box(-3.2 + i * 1.6, 0.05, 0.152, 0.03, 2.3, 0.004, MAT["concrete_drk"])
    # Curb drain and tactile warning strip make the pedestrian edge legible.
    for i in range(7):
        b.box(2.8 + i * 0.10, -1.285, 0.195, 0.045, 0.18, 0.018, MAT["metal_dark"])
    for i in range(4):
        for j in range(3):
            b.cyl(-3.35 + i * 0.12, -0.83 + j * 0.12, 0.166, 0.025, 0.020,
                  MAT["lane_paint"], segments=8)
    obj = b.finish()
    validate(obj, 9.0, 900)
    export(obj, "SidewalkV3")


def build_streetlight():
    b = Builder("StreetlightV3")
    b.box(0, 0, 0.10, 0.42, 0.42, 0.20, MAT["concrete_drk"])
    b.cyl(0, 0, 3.6, 0.075, 7.0, MAT["metal_paint"])
    b.cyl(0.95, 0, 7.02, 0.055, 2.1, MAT["metal_paint"], axis="X")
    b.box(1.95, 0, 6.94, 0.85, 0.30, 0.16, MAT["metal_dark"])
    b.box(1.95, 0, 6.85, 0.62, 0.20, 0.05, MAT["street_lamp"])
    obj = b.finish()
    validate(obj, 7.6, 1400)
    export(obj, "StreetlightV3")


def build_traffic_light():
    b = Builder("TrafficLightV3")
    b.box(0, 0, 0.08, 0.4, 0.4, 0.16, MAT["concrete_drk"])
    b.cyl(0, 0, 2.6, 0.06, 5.0, MAT["metal_dark"])
    b.cyl(0.8, 0, 5.04, 0.05, 1.7, MAT["metal_dark"], axis="X")
    b.box(1.65, 0, 4.62, 0.34, 0.24, 0.95, MAT["metal_dark"])
    b.cyl(1.65, 0.13, 4.95, 0.085, 0.03, MAT["traffic_red"], axis="Y")
    b.cyl(1.65, 0.13, 4.62, 0.085, 0.03, make_mat("CR_TrafficAmber", (0.3, 0.2, 0.02), 0.4, 0.0, emit=(1.0, 0.65, 0.05), emit_strength=1.2), axis="Y")
    b.cyl(1.65, 0.13, 4.29, 0.085, 0.03, make_mat("CR_TrafficGreen", (0.02, 0.2, 0.06), 0.4, 0.0, emit=(0.1, 0.9, 0.25), emit_strength=1.2), axis="Y")
    obj = b.finish()
    validate(obj, 5.6, 1600)
    export(obj, "TrafficLightV3")


def build_hydrant():
    b = Builder("HydrantV3")
    b.cyl(0, 0, 0.42, 0.16, 0.84, MAT["hydrant"], segments=10)
    b.cyl(0, 0, 0.90, 0.11, 0.14, MAT["hydrant"], segments=10)
    b.cyl(0, 0, 0.98, 0.05, 0.08, MAT["metal_dark"], segments=8)
    for a in (0, math.pi / 2, math.pi, 3 * math.pi / 2):
        b.cyl(math.cos(a) * 0.18, math.sin(a) * 0.18, 0.52, 0.055, 0.16,
              MAT["metal_dark"], segments=8, axis="X" if abs(math.cos(a)) > 0.5 else "Y")
    obj = b.finish()
    validate(obj, 1.2, 1500)
    export(obj, "HydrantV3")


def build_trash_cluster():
    b = Builder("TrashClusterV3")
    b.cyl(0, 0, 0.5, 0.34, 1.0, MAT["metal_rust"], segments=12)
    b.cyl(0, 0, 1.02, 0.36, 0.05, MAT["metal_dark"], segments=12)
    for i, (dx, dy) in enumerate(((0.7, 0.15), (0.95, -0.3), (0.55, -0.5))):
        b.box(dx, dy, 0.20, 0.55, 0.45, 0.40, MAT["trash_bag"], rz=i * 0.7)
        b.box(dx + 0.1, dy + 0.05, 0.44, 0.30, 0.25, 0.18, MAT["trash_bag"], rz=i * 1.1)
    obj = b.finish()
    validate(obj, 2.0, 2200)
    export(obj, "TrashClusterV3")


def build_bus_stop():
    b = Builder("BusStopV3")
    b.box(0, 0, 0.05, 3.6, 1.4, 0.10, MAT["concrete"])
    for x in (-1.6, 1.6):
        b.box(x, -0.55, 1.3, 0.09, 0.09, 2.5, MAT["metal_paint"])
        b.box(x, 0.55, 1.3, 0.09, 0.09, 2.5, MAT["metal_paint"])
    b.box(0, 0, 2.62, 3.8, 1.5, 0.08, MAT["metal_paint"])
    b.box(0, -0.62, 1.45, 3.4, 0.05, 2.1, MAT["glass"])
    b.box(0, 0.35, 0.55, 3.0, 0.42, 0.08, MAT["metal_dark"])   # bench
    for x in (-1.2, 0, 1.2):
        b.box(x, 0.35, 0.28, 0.08, 0.38, 0.46, MAT["metal_dark"])
    obj = b.finish()
    validate(obj, 4.2, 2600)
    export(obj, "BusStopV3")


def build_power_pole():
    b = Builder("PowerPoleV3")
    b.cyl(0, 0, 4.0, 0.13, 8.0, MAT["door_wood"], segments=10)
    b.box(0, 0, 7.3, 2.4, 0.10, 0.12, MAT["door_wood"])
    b.box(0, 0, 6.6, 1.8, 0.09, 0.11, MAT["door_wood"])
    for x in (-1.05, -0.35, 0.35, 1.05):
        b.cyl(x, 0, 7.42, 0.035, 0.12, MAT["metal_dark"], segments=6)
    obj = b.finish()
    validate(obj, 8.5, 1200)
    export(obj, "PowerPoleV3")


def build_barrier():
    b = Builder("JerseyBarrierV3")
    b.box(0, 0, 0.12, 2.4, 0.62, 0.24, MAT["concrete"])
    b.box(0, 0, 0.42, 2.4, 0.44, 0.40, MAT["concrete"])
    b.box(0, 0, 0.75, 2.4, 0.24, 0.28, MAT["concrete"])
    obj = b.finish()
    validate(obj, 2.6, 400)
    export(obj, "JerseyBarrierV3")


# --------------------------------------------------------------- vehicles --

def build_vehicle(name, body_mat, kind="sedan", crashed=False, police=False):
    b = Builder(name)
    rnd = random.Random(hash(name) & 0xFFFF)
    L, W = (4.6, 1.85) if kind == "sedan" else (5.2, 2.0) if kind == "van" else (5.1, 1.95)
    wheel_r = 0.34
    body_z = wheel_r + 0.18
    tilt = 0.035 if crashed else 0.0
    # lower body
    b.box(0, 0, body_z + 0.28, L, W, 0.62, body_mat, rz=tilt)
    # hood + trunk taper via smaller boxes
    b.box(L * 0.335, 0, body_z + 0.34, L * 0.30, W * 0.96, 0.50, body_mat, rz=tilt)
    if kind == "sedan":
        # cabin
        b.tapered_box(-L * 0.06, 0, body_z + 0.82,
                      L * 0.50, W * 0.90, L * 0.34, W * 0.72, 0.68,
                      body_mat, top_shift_x=-L * 0.035, rz=tilt)
        # Glass belongs on the exterior surfaces; the old inner glass box was
        # fully occluded by the body and made every car read as a solid brick.
        for ysign in (1, -1):
            b.box(-L * 0.06, ysign * W * 0.445, body_z + 0.89,
                  L * 0.34, 0.025, 0.34, MAT["glass_dark"], rz=tilt)
            b.box(-L * 0.06, ysign * W * 0.463, body_z + 0.89,
                  0.045, 0.035, 0.46, body_mat, rz=tilt)  # B pillar
        for xsign in (1, -1):
            b.box(-L * 0.06 + xsign * L * 0.205, 0, body_z + 0.89,
                  0.025, W * 0.72, 0.34, MAT["glass_dark"], rz=tilt)
    elif kind == "van":
        b.box(-L * 0.12, 0, body_z + 0.95, L * 0.70, W * 0.98, 0.85, body_mat, rz=tilt)
        b.box(L * 0.235, 0, body_z + 0.99, 0.030, W * 0.84, 0.58, MAT["glass_dark"], rz=tilt)
        for ysign in (1, -1):
            b.box(L * 0.16, ysign * W * 0.50, body_z + 0.98,
                  L * 0.18, 0.025, 0.56, MAT["glass_dark"], rz=tilt)
            b.box(-L * 0.10, ysign * W * 0.505, body_z + 0.78,
                  0.025, 0.020, 0.58, MAT["metal_dark"], rz=tilt)  # sliding-door seam
    else:  # pickup
        b.tapered_box(L * 0.10, 0, body_z + 0.86,
                      L * 0.34, W * 0.90, L * 0.27, W * 0.74, 0.62,
                      body_mat, top_shift_x=-L * 0.018, rz=tilt)
        b.box(L * 0.235, 0, body_z + 0.93, 0.030, W * 0.78, 0.43, MAT["glass_dark"], rz=tilt)
        for ysign in (1, -1):
            b.box(L * 0.10, ysign * W * 0.455, body_z + 0.93,
                  L * 0.20, 0.025, 0.42, MAT["glass_dark"], rz=tilt)
        b.box(-L * 0.28, 0, body_z + 0.62, L * 0.38, W * 0.94, 0.30, body_mat, rz=tilt)
    # bumpers, plates
    for xsign in (1, -1):
        b.box(xsign * (L / 2 - 0.04), 0, body_z + 0.10, 0.14, W * 0.98, 0.22, MAT["metal_dark"], rz=tilt)
        b.box(xsign * (L / 2 + 0.01), 0, body_z + 0.16, 0.02, 0.42, 0.14, MAT["plate"], rz=tilt)
    # lights
    for ysign in (1, -1):
        b.box(L / 2 - 0.02, ysign * W * 0.33, body_z + 0.42, 0.06, 0.30, 0.14, MAT["headlight"], rz=tilt)
        b.box(-L / 2 + 0.02, ysign * W * 0.33, body_z + 0.42, 0.06, 0.32, 0.13, MAT["taillight"], rz=tilt)
    # wheels + arches
    for xs, ys in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
        wx, wy = xs * L * 0.315, ys * (W / 2 - 0.02)
        flat = crashed and xs > 0 and ys > 0
        r = wheel_r * (0.72 if flat else 1.0)
        zc = r
        b.cyl(wx, wy, zc, r, 0.26, MAT["tire"], segments=16, axis="Y")
        b.cyl(wx, wy + ys * 0.01, zc, r * 0.55, 0.27, MAT["rim"], segments=10, axis="Y")
        b.cyl(wx, wy + ys * 0.02, zc, r * 0.20, 0.28, MAT["metal_dark"], segments=10, axis="Y")
    # Door cuts, rocker panels and handles sell scale at shoulder camera height.
    for ysign in (1, -1):
        b.box(-L * 0.02, ysign * (W / 2 + 0.012), body_z + 0.37,
              0.018, 0.018, 0.52, MAT["metal_dark"], rz=tilt)
        b.box(-L * 0.04, ysign * (W / 2 + 0.020), body_z + 0.58,
              0.20, 0.025, 0.025, MAT["metal_dark"], rz=tilt)
        b.box(0.0, ysign * (W / 2 + 0.010), body_z + 0.04,
              L * 0.70, 0.025, 0.08, MAT["metal_dark"], rz=tilt)
    # mirrors
    for ysign in (1, -1):
        b.box(L * 0.18, ysign * (W / 2 + 0.09), body_z + 0.78, 0.16, 0.16, 0.12, MAT["metal_dark"], rz=tilt)
    if police:
        b.box(-L * 0.02, 0, body_z + 1.22, 0.55, W * 0.60, 0.13, MAT["metal_dark"])
        b.box(-L * 0.02, W * 0.15, body_z + 1.24, 0.50, 0.16, 0.11, MAT["lightbar_b"])
        b.box(-L * 0.02, -W * 0.15, body_z + 1.24, 0.50, 0.16, 0.11, MAT["lightbar_r"])
        b.box(L * 0.05, 0, body_z + 0.29, L * 0.42, W + 0.02, 0.18, MAT["car_white"])
    if crashed:
        # crumpled hood: skewed overlapping plates + open door
        b.box(L * 0.42, 0.15, body_z + 0.62, 0.9, 0.8, 0.16, body_mat, rz=0.35)
        b.box(L * 0.40, -0.2, body_z + 0.55, 0.7, 0.7, 0.14, MAT["metal_dark"], rz=-0.5)
        b.box(0.2, W / 2 + 0.42, body_z + 0.55, 0.9, 0.06, 0.9, body_mat, rz=1.05)
    obj = b.finish()
    validate(obj, 6.0, 9000)
    export(obj, name)


# ------------------------------------------------------------------ main --

clean_scene()

build_building("BuildingBrickV3", floors=4, width=7.2, depth=6.4,
               wall_mat=MAT["brick_red"], lit_ratio=0.22, storefront=True, fire_escape=True)
clean_scene()
build_building("BuildingConcreteV3", floors=6, width=8.0, depth=7.0,
               wall_mat=MAT["concrete"], lit_ratio=0.15, storefront=True, fire_escape=False)
clean_scene()
build_building("BuildingStuccoV3", floors=3, width=6.4, depth=6.0,
               wall_mat=MAT["stucco"], lit_ratio=0.30, storefront=True, fire_escape=False)
clean_scene()
build_building("BuildingBrownstoneV3", floors=5, width=6.8, depth=6.2,
               wall_mat=MAT["brick_brown"], lit_ratio=0.18, storefront=False, fire_escape=True)
clean_scene()

build_road_straight();        clean_scene()
build_road_intersection();    clean_scene()
build_sidewalk();             clean_scene()
build_streetlight();          clean_scene()
build_traffic_light();        clean_scene()
build_hydrant();              clean_scene()
build_trash_cluster();        clean_scene()
build_bus_stop();             clean_scene()
build_power_pole();           clean_scene()
build_barrier();              clean_scene()

build_vehicle("SedanCleanV3", MAT["car_blue"], kind="sedan");                    clean_scene()
build_vehicle("SedanCrashedV3", MAT["car_red"], kind="sedan", crashed=True);     clean_scene()
build_vehicle("PoliceCruiserV3", MAT["car_police"], kind="sedan", police=True);  clean_scene()
build_vehicle("DeliveryVanV3", MAT["car_white"], kind="van");                    clean_scene()
build_vehicle("PickupV3", MAT["metal_paint"], kind="pickup", crashed=True);      clean_scene()

print("[v3] WORLD KIT COMPLETE — 19 assets in", OUT_DIR)
