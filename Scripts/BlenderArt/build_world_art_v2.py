# build_world_art_v2.py — Operation Code Rescue world-art + weapons pipeline (2026-07-04)
#
# Builds and exports GLB static meshes (UE 5.7 imports natively, same path as the
# proven 2026-07-01 city kit):
#   RawArt/Weapons:  SM_Rifle_Scout, SM_Pistol_Compact, SM_Shotgun_Breacher,
#                    SM_Machete_Field, SM_Wrench_Heavy
#   RawArt/Vehicles: SM_Sedan_Wreck, SM_Van_Delivery, SM_Police_Cruiser
#   RawArt/Nature:   SM_Tree_Oak_8m, SM_Tree_Dead_6m, SM_Bush_Round
#   RawArt/CityKit:  SM_Road_Straight_12m, SM_Crosswalk_8m, SM_Sidewalk_6m,
#                    SM_StreetSign_Stop, SM_TrafficLight
#   RawArt/Sky:      SM_SkyDome_Stars (inward normals, emissive stars), SM_Moon
#
# All props have their base at z=0 so the game's GroundZ placement rests them on
# the ground. Vertex coords are world-baked on Blender 5.1 (see characters_v2).
# Gallery previews are rendered per group into RawArt/previews_v2/.

import bpy, bmesh, math, os, random
import mathutils

PROJECT = os.path.expanduser("~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix")
RAW = os.path.join(PROJECT, "RawArt")
PREV = os.path.join(RAW, "previews_v2")
for sub in ("Weapons", "Vehicles", "Nature", "CityKit", "Sky"):
    os.makedirs(os.path.join(RAW, sub), exist_ok=True)
os.makedirs(PREV, exist_ok=True)

rnd = random.Random(20260704)

PALW = {
    "gun_dark": (0.055, 0.055, 0.06), "gun_grey": (0.16, 0.165, 0.17),
    "wood": (0.23, 0.14, 0.08), "strap": (0.06, 0.06, 0.06),
    "blade": (0.55, 0.57, 0.60), "rust": (0.30, 0.14, 0.07),
    "paint_red": (0.38, 0.10, 0.08), "paint_blue": (0.10, 0.14, 0.24),
    "paint_white": (0.72, 0.73, 0.74), "paint_police": (0.10, 0.11, 0.13),
    "tire": (0.035, 0.035, 0.038), "glass_dim": (0.12, 0.16, 0.18),
    "chrome": (0.62, 0.64, 0.66), "asphalt": (0.055, 0.056, 0.06),
    "lane_paint": (0.70, 0.68, 0.60), "concrete": (0.32, 0.31, 0.29),
    "curb": (0.24, 0.235, 0.225), "bark": (0.16, 0.10, 0.06),
    "leaf_a": (0.10, 0.16, 0.07), "leaf_b": (0.14, 0.19, 0.08),
    "leaf_dead": (0.20, 0.15, 0.08), "sign_red": (0.45, 0.06, 0.05),
    "sign_pole": (0.30, 0.31, 0.32), "night_shell": (0.004, 0.006, 0.012),
    "star": (1.0, 0.97, 0.90), "moon": (0.82, 0.80, 0.74), "moon_dark": (0.55, 0.53, 0.50),
    "tl_red": (0.9, 0.1, 0.05), "tl_amber": (0.9, 0.6, 0.05), "tl_green": (0.1, 0.8, 0.2),
}

def matw(name, key, rough=0.7, metal=0.0, emissive=0.0):
    rgb = PALW[key]
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes.get("Principled BSDF")
    if b:
        b.inputs["Base Color"].default_value = (*rgb, 1.0)
        b.inputs["Roughness"].default_value = rough
        b.inputs["Metallic"].default_value = metal
        if emissive > 0:
            for k in ("Emission Color", "Emission"):
                if k in b.inputs:
                    b.inputs[k].default_value = (*rgb, 1.0)
                    break
            if "Emission Strength" in b.inputs:
                b.inputs["Emission Strength"].default_value = emissive
    m.diffuse_color = (*rgb, 1.0)
    return m

M = {}
def mats():
    global M
    M = {
        "gun_dark": matw("CRW_gun_dark", "gun_dark", 0.35, 0.8),
        "gun_grey": matw("CRW_gun_grey", "gun_grey", 0.4, 0.7),
        "wood": matw("CRW_wood", "wood", 0.75),
        "strap": matw("CRW_strap", "strap", 0.85),
        "blade": matw("CRW_blade", "blade", 0.25, 0.9),
        "rust": matw("CRW_rust", "rust", 0.9),
        "paint_red": matw("CRW_paint_red", "paint_red", 0.5),
        "paint_blue": matw("CRW_paint_blue", "paint_blue", 0.5),
        "paint_white": matw("CRW_paint_white", "paint_white", 0.5),
        "paint_police": matw("CRW_paint_police", "paint_police", 0.45),
        "tire": matw("CRW_tire", "tire", 0.95),
        "glass_dim": matw("CRW_glass_dim", "glass_dim", 0.2, 0.1),
        "chrome": matw("CRW_chrome", "chrome", 0.2, 0.9),
        "asphalt": matw("CRW_asphalt", "asphalt", 0.95),
        "lane_paint": matw("CRW_lane_paint", "lane_paint", 0.8),
        "concrete": matw("CRW_concrete", "concrete", 0.9),
        "curb": matw("CRW_curb", "curb", 0.9),
        "bark": matw("CRW_bark", "bark", 0.95),
        "leaf_a": matw("CRW_leaf_a", "leaf_a", 0.9),
        "leaf_b": matw("CRW_leaf_b", "leaf_b", 0.9),
        "leaf_dead": matw("CRW_leaf_dead", "leaf_dead", 0.95),
        "sign_red": matw("CRW_sign_red", "sign_red", 0.5),
        "sign_pole": matw("CRW_sign_pole", "sign_pole", 0.4, 0.8),
        "night_shell": matw("CRW_night_shell", "night_shell", 1.0),
        "star": matw("CRW_star", "star", 1.0, 0.0, 24.0),
        "moon": matw("CRW_moon", "moon", 1.0, 0.0, 6.0),
        "moon_dark": matw("CRW_moon_dark", "moon_dark", 1.0, 0.0, 3.0),
        "tl_red": matw("CRW_tl_red", "tl_red", 0.6, 0.0, 8.0),
        "tl_amber": matw("CRW_tl_amber", "tl_amber", 0.6, 0.0, 8.0),
        "tl_green": matw("CRW_tl_green", "tl_green", 0.6, 0.0, 8.0),
    }
    return M

def wipe():
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for blk in (bpy.data.meshes, bpy.data.cameras):
        for it in list(blk):
            if it.users == 0:
                blk.remove(it)

def smootho(ob, angle=50.0):
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)
    bpy.ops.object.shade_smooth()
    try:
        bpy.ops.object.shade_smooth_by_angle(angle=math.radians(angle))
    except Exception:
        pass

def bx(name, loc, sx, sy, sz, m, bevel=0.012, bseg=2):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = (sx, sy, sz)
    bpy.ops.object.transform_apply(scale=True)
    if bevel > 0:
        md = ob.modifiers.new("BV", "BEVEL")
        md.width = bevel
        md.segments = bseg
        bpy.ops.object.modifier_apply(modifier=md.name)
    ob.data.materials.append(m)
    return ob

def cyl(name, loc, r, depth, m, axis="Z", verts=16):
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=r, depth=depth, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    if axis == "X":
        ob.rotation_euler = (0, math.radians(90), 0)
    elif axis == "Y":
        ob.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    ob.data.materials.append(m)
    return ob

def ico(name, loc, r, m, subdiv=2):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=subdiv, radius=r, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.data.materials.append(m)
    return ob

def join(objs, name):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.object.join()
    ob = bpy.context.active_object
    ob.name = name
    return ob

def export_glb(ob, subdir, name):
    bpy.ops.object.select_all(action="DESELECT")
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    path = os.path.join(RAW, subdir, f"{name}.glb")
    bpy.ops.export_scene.gltf(filepath=path, use_selection=True, export_format="GLB",
                              export_apply=True, export_animations=False)
    return path

def gallery(paths_names, path, cam_loc, look, res=900):
    scn = bpy.context.scene
    scn.render.engine = "BLENDER_WORKBENCH"
    scn.display.shading.light = "STUDIO"
    scn.display.shading.color_type = "MATERIAL"
    scn.display.shading.show_shadows = True
    scn.render.resolution_x = res
    scn.render.resolution_y = res
    cam_data = bpy.data.cameras.get("GalCam") or bpy.data.cameras.new("GalCam")
    cam = bpy.data.objects.get("GalCamO")
    if cam is None:
        cam = bpy.data.objects.new("GalCamO", cam_data)
    if cam.name not in {o.name for o in bpy.context.collection.objects}:
        bpy.context.collection.objects.link(cam)
    cam.location = cam_loc
    d = mathutils.Vector(look) - mathutils.Vector(cam_loc)
    cam.rotation_mode = "QUATERNION"
    cam.rotation_quaternion = d.to_track_quat('-Z', 'Y')
    scn.camera = cam
    scn.render.filepath = path
    bpy.ops.render.render(write_still=True)
    return path

# ============================================================== WEAPONS
def w_rifle():
    p = []
    p.append(bx("recv", (0, 0, 1.50), 0.30, 0.045, 0.06, M["gun_dark"]))
    p.append(cyl("barrel", (0.34, 0, 1.515), 0.014, 0.42, M["gun_grey"], axis="X", verts=12))
    p.append(bx("handguard", (0.28, 0, 1.50), 0.16, 0.038, 0.045, M["gun_dark"]))
    st = bx("stock", (-0.24, 0, 1.485), 0.14, 0.035, 0.05, M["wood"])
    for v in st.data.vertices:
        if v.co.x < -0.28:
            v.co.z -= 0.02
    p.append(st)
    p.append(bx("grip", (-0.05, 0, 1.43), 0.030, 0.032, 0.06, M["wood"]))
    mg = bx("mag", (0.05, 0, 1.42), 0.035, 0.030, 0.07, M["gun_grey"])
    for v in mg.data.vertices:
        if v.co.z < 1.40:
            v.co.x += 0.02
    p.append(mg)
    p.append(bx("rail", (0.02, 0, 1.545), 0.16, 0.018, 0.012, M["gun_grey"]))
    p.append(bx("sight_rear", (-0.06, 0, 1.565), 0.015, 0.016, 0.02, M["gun_dark"]))
    p.append(bx("sight_front", (0.42, 0, 1.56), 0.010, 0.012, 0.022, M["gun_dark"]))
    ob = join(p, "SM_Rifle_Scout")
    # drop to base z=0 (weapons pivot at grip height is fine for attach; use origin center)
    for v in ob.data.vertices:
        v.co.z -= 1.38
    smootho(ob)
    return ob

def w_pistol():
    p = []
    p.append(bx("slide", (0, 0, 1.52), 0.11, 0.030, 0.032, M["gun_grey"]))
    p.append(bx("frame", (0.01, 0, 1.49), 0.10, 0.028, 0.026, M["gun_dark"]))
    gr = bx("grip", (-0.035, 0, 1.44), 0.030, 0.030, 0.07, M["gun_dark"])
    for v in gr.data.vertices:
        if v.co.z < 1.46:
            v.co.x -= 0.018
    p.append(gr)
    p.append(bx("tguard", (0.02, 0, 1.465), 0.035, 0.008, 0.008, M["gun_dark"]))
    p.append(cyl("muzzle", (0.065, 0, 1.52), 0.009, 0.03, M["gun_grey"], axis="X", verts=10))
    ob = join(p, "SM_Pistol_Compact")
    for v in ob.data.vertices:
        v.co.z -= 1.40
    smootho(ob)
    return ob

def w_shotgun():
    p = []
    p.append(cyl("tube", (0.10, 0, 1.52), 0.020, 0.55, M["gun_dark"], axis="X", verts=14))
    p.append(cyl("undertube", (0.14, 0, 1.487), 0.013, 0.42, M["gun_grey"], axis="X", verts=10))
    p.append(bx("pump", (0.16, 0, 1.487), 0.07, 0.024, 0.024, M["wood"]))
    p.append(bx("recv", (-0.14, 0, 1.51), 0.10, 0.038, 0.05, M["gun_dark"]))
    st = bx("stock", (-0.30, 0, 1.49), 0.12, 0.034, 0.05, M["wood"])
    for v in st.data.vertices:
        if v.co.x < -0.34:
            v.co.z -= 0.025
    p.append(st)
    ob = join(p, "SM_Shotgun_Breacher")
    for v in ob.data.vertices:
        v.co.z -= 1.38
    smootho(ob)
    return ob

def w_machete():
    p = []
    bl = bx("blade", (0.16, 0, 1.50), 0.24, 0.004, 0.035, M["blade"], bevel=0.002, bseg=1)
    for v in bl.data.vertices:
        if v.co.z < 1.49:
            pass
        if v.co.x > 0.34:                      # tip sweep
            v.co.z += (v.co.x - 0.34) * 0.5
    p.append(bl)
    p.append(bx("handle", (-0.06, 0, 1.50), 0.06, 0.014, 0.024, M["strap"], bevel=0.006))
    p.append(bx("guard", (-0.005, 0, 1.50), 0.008, 0.018, 0.042, M["gun_grey"], bevel=0.002, bseg=1))
    ob = join(p, "SM_Machete_Field")
    for v in ob.data.vertices:
        v.co.z -= 1.46
    smootho(ob)
    return ob

def w_wrench():
    p = []
    p.append(bx("shaft", (0, 0, 1.48), 0.20, 0.016, 0.030, M["rust"], bevel=0.006))
    p.append(bx("jaw_a", (0.115, 0, 1.515), 0.045, 0.020, 0.045, M["rust"], bevel=0.006))
    p.append(bx("jaw_b", (0.145, 0, 1.545), 0.06, 0.020, 0.018, M["rust"], bevel=0.006))
    p.append(cyl("ring", (-0.115, 0, 1.48), 0.030, 0.02, M["rust"], axis="Y", verts=14))
    ob = join(p, "SM_Wrench_Heavy")
    for v in ob.data.vertices:
        v.co.z -= 1.44
    smootho(ob)
    return ob

# ============================================================== VEHICLES
def vehicle(name, paint, kind="sedan", wreck=0.0, lightbar=False):
    p = []
    pm = M[paint]
    if kind == "sedan":
        body = bx("body", (0, 0, 0.55), 2.30, 0.92, 0.28, pm, bevel=0.06, bseg=3)
        cab = bx("cab", (-0.15, 0, 0.92), 1.15, 0.84, 0.24, pm, bevel=0.08, bseg=3)
        for v in cab.data.vertices:                    # raked windshield / rear glass
            if v.co.z > 0.98:
                v.co.x = -0.15 + (v.co.x + 0.15) * 0.68
        glass = bx("glass", (-0.15, 0, 0.945), 1.02, 0.86, 0.17, M["glass_dim"], bevel=0.05, bseg=2)
        for v in glass.data.vertices:
            if v.co.z > 0.98:
                v.co.x = -0.15 + (v.co.x + 0.15) * 0.62
        p += [body, cab, glass]
        wheelbase, halfw, wr = 0.82, 0.86, 0.30
    else:  # van
        body = bx("body", (0.1, 0, 0.85), 2.45, 0.98, 0.62, pm, bevel=0.07, bseg=3)
        nose = bx("nose", (1.35, 0, 0.62), 0.35, 0.94, 0.30, pm, bevel=0.06, bseg=2)
        glass = bx("glass", (1.19, 0, 0.98), 0.16, 0.88, 0.20, M["glass_dim"], bevel=0.03, bseg=1)
        p += [body, nose, glass]
        wheelbase, halfw, wr = 0.95, 0.90, 0.33
    p.append(bx("bumper_f", ((2.30 if kind == "sedan" else 2.62) / 2 + 0.06, 0, 0.35), 0.06, 0.90, 0.09, M["chrome"], bevel=0.02, bseg=1))
    p.append(bx("bumper_r", (-(2.30 if kind == "sedan" else 2.45) / 2 - 0.06, 0, 0.35), 0.06, 0.90, 0.09, M["chrome"], bevel=0.02, bseg=1))
    flat = rnd.randrange(4) if wreck > 0 else -1
    for i, (sx, sy) in enumerate(((1, 1), (1, -1), (-1, 1), (-1, -1))):
        wz = wr if i != flat else wr * 0.62
        wh = cyl(f"wheel{i}", (sx * wheelbase, sy * halfw, wz), wr, 0.16, M["tire"], axis="Y", verts=18)
        if i == flat:
            for v in wh.data.vertices:
                if v.co.z < wr * 0.35:
                    v.co.z = max(v.co.z * 0.4, 0.02)
        p.append(wh)
        p.append(cyl(f"hub{i}", (sx * wheelbase, sy * (halfw + 0.085), wz), wr * 0.4, 0.02, M["chrome"], axis="Y", verts=12))
    if lightbar:
        p.append(bx("bar_base", (-0.15, 0, 1.09), 0.30, 0.55, 0.035, M["gun_dark"], bevel=0.01, bseg=1))
        p.append(bx("bar_red", (-0.15, 0.16, 1.115), 0.28, 0.16, 0.030, matw("CRW_bar_red", "tl_red", 0.4, 0, 10.0), bevel=0.008, bseg=1))
        p.append(bx("bar_blue", (-0.15, -0.16, 1.115), 0.28, 0.16, 0.030, matw("CRW_bar_blue", "paint_blue", 0.4, 0, 10.0), bevel=0.008, bseg=1))
    ob = join(p, name)
    if wreck > 0:
        for v in ob.data.vertices:                     # crumple + sag
            v.co.x += rnd.uniform(-wreck, wreck) * 0.03
            v.co.y += rnd.uniform(-wreck, wreck) * 0.03
            if v.co.x > 0.8 and v.co.z > 0.4:
                v.co.z -= wreck * 0.06
    smootho(ob, 40)
    return ob

# ============================================================== NATURE
def tree_oak():
    p = []
    tr = cyl("trunk", (0, 0, 1.6), 0.16, 3.2, M["bark"], verts=10)
    for v in tr.data.vertices:
        s = 1.0 - (v.co.z / 3.6) * 0.45
        v.co.x *= s
        v.co.y *= s
    p.append(tr)
    for i in range(6):
        a = i * math.pi / 3 + rnd.uniform(-0.3, 0.3)
        r = rnd.uniform(0.5, 1.1)
        h = rnd.uniform(3.0, 4.6)
        can = ico(f"can{i}", (math.cos(a) * r, math.sin(a) * r, h), rnd.uniform(0.9, 1.5),
                  M["leaf_a" if i % 2 else "leaf_b"], subdiv=2)
        for v in can.data.vertices:
            L = (mathutils.Vector(v.co) - mathutils.Vector((math.cos(a) * r, math.sin(a) * r, h))).length
            v.co.z += rnd.uniform(-0.06, 0.06)
        p.append(can)
    p.append(ico("crown", (0, 0, 5.1), 1.3, M["leaf_b"], subdiv=2))
    ob = join(p, "SM_Tree_Oak_8m")
    smootho(ob, 45)
    return ob

def tree_dead():
    p = [cyl("trunk", (0, 0, 1.4), 0.13, 2.8, M["bark"], verts=9)]
    for i in range(5):
        a = i * 2.3 + 0.4
        L = rnd.uniform(0.9, 1.6)
        z0 = rnd.uniform(1.6, 2.9)
        d = mathutils.Vector((math.cos(a), math.sin(a), rnd.uniform(0.6, 1.2))).normalized()
        mid = mathutils.Vector((0, 0, z0)) + d * (L / 2)
        bpy.ops.mesh.primitive_cylinder_add(vertices=7, radius=0.05, depth=L, location=mid)
        br = bpy.context.active_object
        br.name = f"branch{i}"
        br.rotation_mode = "QUATERNION"
        br.rotation_quaternion = d.to_track_quat('Z', 'Y')
        bpy.ops.object.transform_apply(rotation=True)
        br.data.materials.append(M["bark"])
        for v in br.data.vertices:
            pass
        p.append(br)
    ob = join(p, "SM_Tree_Dead_6m")
    smootho(ob, 45)
    return ob

def bush():
    p = []
    for i in range(3):
        p.append(ico(f"b{i}", (rnd.uniform(-0.25, 0.25), rnd.uniform(-0.25, 0.25), 0.35 + i * 0.12),
                     0.42 - i * 0.06, M["leaf_a" if i % 2 else "leaf_dead"], subdiv=2))
    ob = join(p, "SM_Bush_Round")
    smootho(ob, 45)
    return ob

# ============================================================== STREET
def road_straight():
    p = [bx("slab", (0, 0, 0.06), 12.0, 8.0, 0.12, M["asphalt"], bevel=0.0)]
    for i in range(-2, 3):
        p.append(bx(f"dash{i}", (i * 2.4, 0, 0.128), 1.1, 0.14, 0.012, M["lane_paint"], bevel=0.0))
    for s in (1, -1):
        p.append(bx(f"edge{s}", (0, s * 3.7, 0.128), 12.0, 0.12, 0.012, M["lane_paint"], bevel=0.0))
    ob = join(p, "SM_Road_Straight_12m")
    return ob

def crosswalk():
    p = [bx("slab", (0, 0, 0.06), 8.0, 8.0, 0.12, M["asphalt"], bevel=0.0)]
    for i in range(-3, 4):
        p.append(bx(f"zeb{i}", (i * 1.05, 0, 0.128), 0.55, 6.4, 0.012, M["lane_paint"], bevel=0.0))
    ob = join(p, "SM_Crosswalk_8m")
    return ob

def sidewalk():
    p = [bx("walk", (0, 0, 0.10), 6.0, 2.4, 0.20, M["concrete"], bevel=0.01, bseg=1),
         bx("curb", (0, -1.26, 0.14), 6.0, 0.14, 0.28, M["curb"], bevel=0.01, bseg=1)]
    for i in range(-2, 3):
        p.append(bx(f"seam{i}", (i * 1.2, 0.05, 0.202), 0.02, 2.3, 0.004, M["curb"], bevel=0.0))
    ob = join(p, "SM_Sidewalk_6m")
    return ob

def stop_sign():
    p = [cyl("pole", (0, 0, 1.5), 0.035, 3.0, M["sign_pole"], verts=10)]
    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.42, depth=0.03, location=(0, 0, 2.6))
    oc = bpy.context.active_object
    oc.name = "octagon"
    oc.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    oc.data.materials.append(M["sign_red"])
    p.append(oc)
    p.append(bx("band", (0, -0.022, 2.6), 0.56, 0.012, 0.09, M["paint_white"], bevel=0.0))
    ob = join(p, "SM_StreetSign_Stop")
    smootho(ob, 40)
    return ob

def traffic_light():
    p = [cyl("pole", (0, 0, 2.4), 0.06, 4.8, M["sign_pole"], verts=12),
         cyl("arm", (1.1, 0, 4.55), 0.045, 2.2, M["sign_pole"], axis="X", verts=10),
         bx("head", (2.05, 0, 4.25), 0.16, 0.14, 0.55, M["gun_dark"], bevel=0.02)]
    for i, mk in enumerate(("tl_red", "tl_amber", "tl_green")):
        p.append(cyl(f"lamp{i}", (2.05, -0.078, 4.42 - i * 0.17), 0.055, 0.02, M[mk], axis="Y", verts=12))
    ob = join(p, "SM_TrafficLight")
    smootho(ob, 40)
    return ob

# ============================================================== SKY
def skydome():
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=3, radius=1.0, location=(0, 0, 0))
    dome = bpy.context.active_object
    dome.name = "shell"
    bm = bmesh.new()
    bm.from_mesh(dome.data)
    bm.verts.ensure_lookup_table()
    low = [v for v in bm.verts if v.co.z < -0.15]
    bmesh.ops.delete(bm, geom=low, context="VERTS")
    for f in bm.faces:
        f.normal_flip()
    bm.to_mesh(dome.data)
    bm.free()
    dome.data.materials.append(M["night_shell"])
    stars = []
    bm2 = bmesh.new()
    for i in range(420):
        # random point on upper sphere, radius just inside the shell
        while True:
            v = mathutils.Vector((rnd.uniform(-1, 1), rnd.uniform(-1, 1), rnd.uniform(-0.05, 1)))
            if 0.15 < v.length <= 1.0:
                break
        n = v.normalized()
        pos = n * 0.985
        size = rnd.uniform(0.0022, 0.0062)
        up = mathutils.Vector((0, 0, 1))
        t1 = n.cross(up)
        if t1.length < 1e-4:
            t1 = mathutils.Vector((1, 0, 0))
        t1.normalize()
        t2 = n.cross(t1).normalized()
        vs = [bm2.verts.new(pos + (t1 * s1 + t2 * s2) * size)
              for s1, s2 in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
        f = bm2.faces.new(vs)
        f.normal_update()
        if f.normal.dot(n) > 0:                        # face must look inward (toward center = -n)
            f.normal_flip()
    star_me = bpy.data.meshes.new("stars")
    bm2.to_mesh(star_me)
    bm2.free()
    star_ob = bpy.data.objects.new("stars", star_me)
    bpy.context.collection.objects.link(star_ob)
    star_ob.data.materials.append(M["star"])
    ob = join([dome, star_ob], "SM_SkyDome_Stars")
    return ob

def moon():
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=16, radius=1.0, location=(0, 0, 0))
    mo = bpy.context.active_object
    mo.name = "SM_Moon"
    mo.data.materials.append(M["moon"])
    mo.data.materials.append(M["moon_dark"])
    # blotches: assign second material to a few random face clusters
    me = mo.data
    seeds = [rnd.randrange(len(me.polygons)) for _ in range(7)]
    centers = [me.polygons[s].center.copy() for s in seeds]
    for poly in me.polygons:
        for c in centers:
            if (poly.center - c).length < rnd.uniform(0.18, 0.34):
                poly.material_index = 1
                break
    smootho(mo, 80)
    return mo

# ============================================================== BUILD ALL
def run():
    mats()
    results = {}
    groups = {
        "Weapons": [("SM_Rifle_Scout", w_rifle), ("SM_Pistol_Compact", w_pistol),
                    ("SM_Shotgun_Breacher", w_shotgun), ("SM_Machete_Field", w_machete),
                    ("SM_Wrench_Heavy", w_wrench)],
        "Vehicles": [("SM_Sedan_Wreck", lambda: vehicle("SM_Sedan_Wreck", "paint_red", "sedan", wreck=1.0)),
                     ("SM_Van_Delivery", lambda: vehicle("SM_Van_Delivery", "paint_white", "van")),
                     ("SM_Police_Cruiser", lambda: vehicle("SM_Police_Cruiser", "paint_police", "sedan", lightbar=True))],
        "Nature": [("SM_Tree_Oak_8m", tree_oak), ("SM_Tree_Dead_6m", tree_dead), ("SM_Bush_Round", bush)],
        "CityKit": [("SM_Road_Straight_12m", road_straight), ("SM_Crosswalk_8m", crosswalk),
                    ("SM_Sidewalk_6m", sidewalk), ("SM_StreetSign_Stop", stop_sign),
                    ("SM_TrafficLight", traffic_light)],
        "Sky": [("SM_SkyDome_Stars", skydome), ("SM_Moon", moon)],
    }
    gal_cfg = {
        "Weapons": ((1.6, -2.2, 0.9), (0.4, 0.6, 0.15), 3.2),
        "Vehicles": ((7.5, -9.5, 4.5), (0.5, 3.5, 0.6), 9.0),
        "Nature": ((9.0, -12.0, 5.0), (0.0, 4.0, 2.5), 11.0),
        "CityKit": ((10.0, -14.0, 7.0), (0.0, 6.0, 0.5), 14.0),
        "Sky": ((0.0, -2.6, 0.6), (0, 0, 0.3), 0.0),
    }
    for grp, items in groups.items():
        wipe()
        mats()
        built = []
        for name, fn in items:
            ob = fn()
            path = export_glb(ob, grp, name)
            results[name] = {"path": path, "verts": len(ob.data.vertices)}
            built.append(ob)
        # gallery layout
        cam_loc, look, spread = gal_cfg[grp]
        for i, ob in enumerate(built):
            ob.location.y = i * spread / max(1, len(built) - 1) if len(built) > 1 else 0
        gpath = os.path.join(PREV, f"gallery_{grp}.png")
        gallery(None, gpath, cam_loc, look)
        results[f"_gallery_{grp}"] = gpath
    return results

if globals().get("RUN_WORLD_BUILD", True):
    WORLD_RESULTS = run()
    print("[world_art_v2] DONE")
    for k, v in WORLD_RESULTS.items():
        print("  ", k, v)
