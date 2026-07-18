# build_characters_v2.py — Operation Code Rescue character pipeline v2 (2026-07-04)
#
# "Comfortably realistic" survivors + zombies: organic subsurf bodies, layered clothing,
# faces with eyes/brows/nose/lips, hair, FACIAL SHAPE KEYS (morph targets in UE),
# 17-bone rig with joint-blended weights, Idle/Walk/Run actions.
# Exports UE-ready FBX to RawArt/Characters/.
#
# IMPORTANT (learned 2026-07-04 on Blender 5.1): after primitive_add(location=..) +
# transform_apply, VERTEX COORDS ARE WORLD-SPACE (object sits at origin). Every vertex
# edit below therefore works in world space, and validate_parts() guards regressions.
#
# Run:  exec(open(<this file>).read())          (Blender MCP / Scripting tab)
#       blender --background --python <this file>
# Subset: set BUILD_ONLY = ["SurvivorKenny"] in globals before exec.

import bpy, bmesh, math, os, random

PROJECT = os.path.expanduser("~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix")
OUT_DIR = os.path.join(PROJECT, "RawArt", "Characters")
PREV_DIR = os.path.join(OUT_DIR, "previews_v2")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)

BUILD_ONLY = globals().get("BUILD_ONLY", None)

PAL = {
    "skin":   (0.58, 0.42, 0.32), "skin_f": (0.63, 0.47, 0.37),
    "jacket": (0.155, 0.165, 0.125), "jacket_f": (0.16, 0.19, 0.20),
    "vest":   (0.09, 0.095, 0.10), "pants": (0.12, 0.12, 0.13),
    "boots":  (0.10, 0.075, 0.055), "gloves": (0.08, 0.08, 0.085),
    "hair_m": (0.06, 0.045, 0.035), "hair_f": (0.16, 0.09, 0.045),
    "strap":  (0.06, 0.06, 0.06), "metal": (0.35, 0.36, 0.38),
    "zskin_a": (0.42, 0.46, 0.36), "zskin_b": (0.38, 0.38, 0.31),
    "wound":  (0.22, 0.045, 0.035), "rags_a": (0.13, 0.12, 0.11),
    "rags_b": (0.10, 0.11, 0.12), "eye_w": (0.85, 0.84, 0.80),
    "eye_z":  (0.74, 0.72, 0.58), "iris": (0.09, 0.13, 0.16),
    "lips":   (0.47, 0.30, 0.24), "cross": (0.75, 0.12, 0.10),
}

def mat(name, rgb, rough=0.8, metal=0.0):
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes.get("Principled BSDF")
    if b:
        b.inputs["Base Color"].default_value = (*rgb, 1.0)
        b.inputs["Roughness"].default_value = rough
        b.inputs["Metallic"].default_value = metal
    m.diffuse_color = (*rgb, 1.0)
    return m

def mats():
    M = {}
    for k, v in PAL.items():
        rough = (0.5 if "skin" in k else 0.88 if ("jacket" in k or k.startswith("rags") or k == "pants")
                 else 0.62 if k == "boots" else 0.3 if k == "metal" else 0.25 if "eye" in k else 0.8)
        M[k] = mat("CRV2_" + k, v, rough=rough, metal=(0.85 if k == "metal" else 0.0))
    return M

def wipe():
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for blk in (bpy.data.meshes, bpy.data.armatures, bpy.data.actions, bpy.data.cameras):
        for it in list(blk):
            if it.users == 0:
                blk.remove(it)

def smooth(ob, angle=60.0):
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)
    bpy.ops.object.shade_smooth()
    try:
        bpy.ops.object.shade_smooth_by_angle(angle=math.radians(angle))
    except Exception:
        try:
            ob.data.use_auto_smooth = True
            ob.data.auto_smooth_angle = math.radians(angle)
        except Exception:
            pass

def subsurf(ob, levels=1):
    bpy.context.view_layer.objects.active = ob
    md = ob.modifiers.new("SS", "SUBSURF")
    md.levels = levels
    md.render_levels = levels
    bpy.ops.object.modifier_apply(modifier=md.name)

def ball(name, loc, sx, sy, sz, m, seg=24, ring=16):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=seg, ring_count=ring, radius=1.0, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = (sx, sy, sz)
    bpy.ops.object.transform_apply(scale=True)
    ob.data.materials.append(m)
    return ob

def box(name, loc, sx, sy, sz, m, bevel=0.02, bseg=2):
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

def limb(name, p0, p1, r0, r1, m, verts=14, bulge=1.12, seg=4):
    """Tapered organic limb. Taper/bulge measured RADIALLY FROM THE LIMB AXIS
    (world-space projection), so off-center limbs don't wobble."""
    import mathutils
    v0, v1 = mathutils.Vector(p0), mathutils.Vector(p1)
    mid, axis = (v0 + v1) / 2, (v1 - v0)
    L = axis.length
    d = axis.normalized()
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=1.0, depth=1.0, location=mid)
    ob = bpy.context.active_object
    ob.name = name
    ob.rotation_mode = "QUATERNION"
    ob.rotation_quaternion = axis.to_track_quat('Z', 'Y')
    ob.scale = (r0, r0, L)
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.subdivide(number_cuts=seg)
    bpy.ops.object.mode_set(mode="OBJECT")
    for v in ob.data.vertices:
        w = mathutils.Vector(v.co)                     # world-ish coords (see header note)
        u = (w - v0).dot(d)
        t = max(0.0, min(1.0, u / max(1e-6, L)))
        axis_pt = v0 + d * u
        rad = w - axis_pt
        r = r0 * (1 - t) + r1 * t
        b = 1.0 + (bulge - 1.0) * math.sin(t * math.pi)
        s = (r * b) / max(1e-6, r0)
        nw = axis_pt + rad * s
        v.co.x, v.co.y, v.co.z = nw.x, nw.y, nw.z
    ob.data.materials.append(m)
    return ob

def tilt(ob, rx=0.0, ry=0.0, rz=0.0):
    """Rotate mesh verts about the mesh's own centroid (world coords safe)."""
    import mathutils
    R = mathutils.Euler((math.radians(rx), math.radians(ry), math.radians(rz)), 'XYZ').to_matrix()
    c = mathutils.Vector((0, 0, 0))
    for v in ob.data.vertices:
        c += v.co
    c /= max(1, len(ob.data.vertices))
    for v in ob.data.vertices:
        v.co = c + R @ (mathutils.Vector(v.co) - c)

def validate_parts(P, intents):
    """QA gate: every part's real bbox center/size must be near its intent."""
    import mathutils
    bad = []
    for name, (ob, bone) in P.items():
        bb = [ob.matrix_world @ mathutils.Vector(c) for c in ob.bound_box]
        cx = sum(v.x for v in bb) / 8
        cy = sum(v.y for v in bb) / 8
        cz = sum(v.z for v in bb) / 8
        dx = max(v.x for v in bb) - min(v.x for v in bb)
        dy = max(v.y for v in bb) - min(v.y for v in bb)
        dz = max(v.z for v in bb) - min(v.z for v in bb)
        loc, maxdim = intents.get(name, (None, None))
        if loc is not None:
            drift = math.sqrt((cx - loc[0]) ** 2 + (cy - loc[1]) ** 2 + (cz - loc[2]) ** 2)
            if drift > 0.09:
                bad.append((name, "drift", round(drift, 3)))
        if maxdim is not None and max(dx, dy, dz) > maxdim * 2.2:
            bad.append((name, "oversize", round(max(dx, dy, dz), 3)))
    return bad

BONES = [
    ("pelvis", (0, 0, 0.88), (0, 0, 1.02), None),
    ("spine", (0, 0, 1.02), (0, 0, 1.22), "pelvis"),
    ("chest", (0, 0, 1.22), (0, 0, 1.47), "spine"),
    ("neck", (0, 0, 1.47), (0, 0, 1.58), "chest"),
    ("head", (0, 0, 1.58), (0, 0, 1.86), "neck"),
    ("upperarm.R", (0.20, 0, 1.44), (0.338, -0.028, 1.19), "chest"),
    ("forearm.R", (0.338, -0.028, 1.19), (0.374, -0.10, 0.96), "upperarm.R"),
    ("hand.R", (0.374, -0.10, 0.96), (0.374, -0.12, 0.83), "forearm.R"),
    ("upperarm.L", (-0.20, 0, 1.44), (-0.338, -0.028, 1.19), "chest"),
    ("forearm.L", (-0.338, -0.028, 1.19), (-0.374, -0.10, 0.96), "upperarm.L"),
    ("hand.L", (-0.374, -0.10, 0.96), (-0.374, -0.12, 0.83), "forearm.L"),
    ("thigh.R", (0.118, 0, 0.92), (0.125, -0.012, 0.53), "pelvis"),
    ("shin.R", (0.125, -0.012, 0.53), (0.122, 0.02, 0.135), "thigh.R"),
    ("foot.R", (0.122, 0.02, 0.135), (0.122, -0.16, 0.03), "shin.R"),
    ("thigh.L", (-0.118, 0, 0.92), (-0.125, -0.012, 0.53), "pelvis"),
    ("shin.L", (-0.125, -0.012, 0.53), (-0.122, 0.02, 0.135), "thigh.L"),
    ("foot.L", (-0.122, 0.02, 0.135), (-0.122, -0.16, 0.03), "shin.L"),
]

CZ = 1.692            # head center height
HY = 0.008            # head center Y

def build_parts(cfg, M):
    P = {}
    INTENT = {}
    rnd = random.Random(cfg.get("seed", 7))
    female = cfg.get("female", False)
    zombie = cfg.get("zombie", False)
    skin = M[cfg.get("skin", "skin")]
    top = M[cfg.get("cloth_top", "jacket")]
    leg = M[cfg.get("cloth_leg", "pants")]
    glove = M["gloves"] if not zombie else skin
    bootm = M["boots"] if not zombie else M["rags_b"]
    wS = 0.90 if female else 1.0
    wH = 1.08 if female else 1.0
    rA = 0.85 if female else 1.0
    rL = 0.90 if female else 1.0
    musc = cfg.get("muscle", 1.0)

    def add(ob, bone, loc=None, maxdim=None):
        P[ob.name] = (ob, bone)
        INTENT[ob.name] = (loc, maxdim)
        return ob

    # ---- torso column
    prof = [
        (1.485, 0.185 * wS, 0.130), (1.44, 0.225 * wS * musc, 0.150),
        (1.36, 0.215 * wS * musc, 0.128), (1.24, 0.190 * wS, 0.115),
        (1.10, 0.162 * (0.94 if female else 1.0), 0.103),
        (1.00, 0.180 * wH, 0.115), (0.90, 0.186 * wH, 0.120),
    ]
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=1.0, depth=1.0, location=(0, 0.005, (1.485 + 0.90) / 2))
    to = bpy.context.active_object
    to.name = "Torso"
    to.scale = (1, 1, 1.485 - 0.90)
    bpy.ops.object.transform_apply(scale=True)
    bpy.context.view_layer.objects.active = to
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.subdivide(number_cuts=6)
    bpy.ops.object.mode_set(mode="OBJECT")

    def prof_at(z):
        if z <= prof[-1][0]:
            return prof[-1][1], prof[-1][2]
        if z >= prof[0][0]:
            return prof[0][1], prof[0][2]
        for i in range(len(prof) - 1):
            z0, x0, y0 = prof[i]
            z1, x1, y1 = prof[i + 1]
            if z1 <= z <= z0:
                t = (z - z1) / (z0 - z1)
                return x1 + (x0 - x1) * t, y1 + (y0 - y1) * t
        return prof[-1][1], prof[-1][2]

    for v in to.data.vertices:
        hx, hy = prof_at(v.co.z)
        v.co.x *= hx
        v.co.y = 0.005 + (v.co.y - 0.005) * hy
        if female and 1.28 < v.co.z < 1.42 and v.co.y < -0.02:
            v.co.y -= 0.020 * math.sin((v.co.z - 1.28) / 0.14 * math.pi)
        if zombie and v.co.y < -0.02 and 1.05 < v.co.z < 1.30:
            v.co.y += 0.014
    subsurf(to, 1)
    smooth(to, 75)
    to.data.materials.clear()
    to.data.materials.append(top)
    add(to, "chest", (0, 0.005, 1.19), 0.62)

    hipm = leg if not zombie else M["rags_a"]
    hip = box("PantsHip", (0, 0.005, 0.885), 0.365 * wH, 0.235, 0.17, hipm, bevel=0.05, bseg=3)
    subsurf(hip, 2)
    smooth(hip)
    add(hip, "pelvis", (0, 0.005, 0.885), 0.80)

    hem = box("JacketHem", (0, 0.008, 0.975), 0.395 * wH, 0.245, 0.06, top, bevel=0.025, bseg=2)
    for v in hem.data.vertices:
        if v.co.z < 0.972:
            v.co.x *= 1.05
            v.co.y = 0.008 + (v.co.y - 0.008) * 1.05
        if zombie:
            v.co.z += rnd.uniform(-0.028, 0.004)
    subsurf(hem, 1)
    smooth(hem)
    add(hem, "pelvis", (0, 0.008, 0.975), 0.90)

    # deltoids (world-coord edits about their own center)
    for sgn, side in ((1, "R"), (-1, "L")):
        dcx, dcz = sgn * 0.235 * wS, 1.432
        d = ball(f"Deltoid.{side}", (dcx, 0.0, dcz), 0.096 * musc, 0.110, 0.075, top)
        for v in d.data.vertices:
            if (v.co.x - dcx) * sgn < 0:
                v.co.x = dcx + (v.co.x - dcx) * 0.72
            if v.co.z > dcz + 0.040:
                v.co.z = dcz + 0.040 + (v.co.z - dcz - 0.040) * 0.5
        smooth(d)
        add(d, f"upperarm.{side}", (dcx, 0.0, dcz), 0.30)

    nk = limb("Neck", (0, 0.015, 1.472), (0, 0.006, 1.578), 0.060, 0.054, skin, verts=16, bulge=1.0)
    smooth(nk, 75)
    add(nk, "neck", (0, 0.01, 1.525), 0.20)

    # ---- arms
    for sgn, side in ((1, "R"), (-1, "L")):
        ua = limb(f"UpperArm.{side}", (sgn * 0.262 * wS, 0.0, 1.418), (sgn * 0.338, -0.028, 1.19),
                  0.070 * rA * musc, 0.055 * rA, top, bulge=1.16 * musc)
        smooth(ua, 75)
        add(ua, f"upperarm.{side}", (sgn * 0.30, -0.014, 1.30), 0.40)
        eb = ball(f"Elbow.{side}", (sgn * 0.338, -0.028, 1.19), 0.052 * rA, 0.052 * rA, 0.058, top)
        smooth(eb)
        add(eb, f"forearm.{side}", (sgn * 0.338, -0.028, 1.19), 0.16)
        sleeve = top if not zombie else skin
        fa = limb(f"Forearm.{side}", (sgn * 0.338, -0.028, 1.19), (sgn * 0.368, -0.082, 0.98),
                  0.054 * rA, 0.040 * rA, sleeve, bulge=1.10)
        smooth(fa, 75)
        add(fa, f"forearm.{side}", (sgn * 0.353, -0.055, 1.085), 0.35)
        px, py, pz = sgn * 0.374, -0.10, 0.925
        palm = box(f"Palm.{side}", (px, py, pz), 0.056 * rA, 0.088, 0.095, glove, bevel=0.022, bseg=2)
        subsurf(palm, 1)
        smooth(palm)
        add(palm, f"hand.{side}", (px, py, pz), 0.22)
        fcy, fcz = py - 0.022, pz - 0.072
        fing = box(f"Fingers.{side}", (px, fcy, fcz), 0.054 * rA, 0.078, 0.07, glove, bevel=0.02, bseg=2)
        for v in fing.data.vertices:
            if v.co.z < fcz - 0.012:
                v.co.y = fcy + (v.co.y - fcy) * 0.9
        subsurf(fing, 1)
        smooth(fing)
        add(fing, f"hand.{side}", (px, fcy, fcz), 0.20)
        th = ball(f"Thumb.{side}", (px - sgn * 0.048, py - 0.046, pz + 0.008), 0.023, 0.052, 0.03, glove)
        smooth(th)
        add(th, f"hand.{side}", (px - sgn * 0.048, py - 0.046, pz + 0.008), 0.13)

    # ---- legs
    for sgn, side in ((1, "R"), (-1, "L")):
        legm = leg if not zombie else M["rags_b"]
        th_ = limb(f"Thigh.{side}", (sgn * 0.118 * wH, 0.005, 0.92), (sgn * 0.125, -0.012, 0.53),
                   0.090 * rL, 0.064 * rL, legm, bulge=1.09)
        smooth(th_, 75)
        add(th_, f"thigh.{side}", (sgn * 0.121, -0.004, 0.725), 0.50)
        kn = ball(f"Knee.{side}", (sgn * 0.125, -0.012, 0.53), 0.058 * rL, 0.058 * rL, 0.066, legm)
        smooth(kn)
        add(kn, f"shin.{side}", (sgn * 0.125, -0.012, 0.53), 0.18)
        shinm = legm if not zombie else skin
        sh_ = limb(f"Shin.{side}", (sgn * 0.125, -0.012, 0.53), (sgn * 0.122, 0.02, 0.135),
                   0.062 * rL, 0.044 * rL, shinm, bulge=1.13)
        smooth(sh_, 75)
        add(sh_, f"shin.{side}", (sgn * 0.123, 0.004, 0.33), 0.48)
        if not zombie:
            cp = box(f"CargoPocket.{side}", (sgn * 0.192, -0.012, 0.71), 0.032, 0.088, 0.098, legm, bevel=0.014, bseg=2)
            smooth(cp)
            add(cp, f"thigh.{side}", (sgn * 0.192, -0.012, 0.71), 0.22)
            kp = ball(f"KneePad.{side}", (sgn * 0.125, -0.055, 0.525), 0.05, 0.035, 0.06, M["vest"])
            smooth(kp)
            add(kp, f"shin.{side}", (sgn * 0.125, -0.055, 0.525), 0.15)
        bs = limb(f"BootShaft.{side}", (sgn * 0.122, 0.02, 0.16), (sgn * 0.122, 0.02, 0.04), 0.054, 0.060, bootm, bulge=1.0)
        smooth(bs)
        add(bs, f"foot.{side}", (sgn * 0.122, 0.02, 0.10), 0.16)
        ftx = sgn * 0.122
        ft = box(f"Foot.{side}", (ftx, -0.055, 0.045), 0.056, 0.132, 0.046, bootm, bevel=0.02, bseg=2)
        for v in ft.data.vertices:
            if v.co.y < -0.10:
                v.co.z = max(v.co.z, 0.030)
                v.co.x = ftx + (v.co.x - ftx) * 0.9
        subsurf(ft, 1)
        smooth(ft)
        add(ft, f"foot.{side}", (ftx, -0.055, 0.045), 0.30)

    # ---- head (all edits in world coords relative to (0, HY, CZ))
    hd = ball("Head", (0, HY, CZ), 0.096, 0.104, 0.118, skin, seg=28, ring=20)
    for v in hd.data.vertices:
        dz = CZ - v.co.z                          # + below center
        if v.co.y > HY + 0.022:
            v.co.y = HY + (v.co.y - HY) * 1.06    # fuller back skull
        # face plane FIRST (compress raw sphere bulge only) — clamping after the
        # jaw/chin pushes folds the mouth region inside-out (2026-07-04 lesson)
        if v.co.y < HY - 0.080:
            v.co.y = (HY - 0.080) - (abs(v.co.y - HY) - 0.080) * 0.45
        if dz > 0.008 and v.co.y < HY + 0.002:
            f = min((dz - 0.008) / 0.095, 1.0)
            v.co.x *= (1.0 - (0.30 if female else 0.36) * f)
            v.co.y -= (0.020 if female else 0.026) * f
        if dz > 0.062 and v.co.y < HY - 0.028:
            v.co.y -= 0.014 * min((dz - 0.062) / 0.05, 1.0)
        if 0.0 < (v.co.z - CZ) < 0.030 and abs(v.co.x) > 0.05 and v.co.y < HY - 0.038:
            v.co.x *= 1.04                        # cheekbones
        if zombie and v.co.y < HY - 0.048 and -0.02 < (v.co.z - CZ) < 0.035 and abs(v.co.x) > 0.035:
            v.co.y += 0.012                       # sunken cheeks
    smooth(hd, 70)
    add(hd, "head", (0, HY, CZ), 0.30)

    eyem = M["eye_w"] if not zombie else M["eye_z"]
    for sgn, side in ((1, "R"), (-1, "L")):
        ew = ball(f"Eye.{side}", (sgn * 0.034, HY - 0.080, CZ + 0.011), 0.0165, 0.0165, 0.0165, eyem, seg=16, ring=12)
        smooth(ew)
        add(ew, "head", (sgn * 0.034, HY - 0.080, CZ + 0.011), 0.05)
        if not zombie:
            ir = ball(f"Iris.{side}", (sgn * 0.034, HY - 0.0952, CZ + 0.011), 0.0075, 0.0032, 0.0075, M["iris"], seg=12, ring=10)
            smooth(ir)
            add(ir, "head", (sgn * 0.034, HY - 0.0952, CZ + 0.011), 0.03)
        lcz = CZ + 0.0245
        ld = ball(f"Lid.{side}", (sgn * 0.034, HY - 0.0790, lcz), 0.0185, 0.0160, 0.0085, skin, seg=16, ring=10)
        for v in ld.data.vertices:
            if v.co.z < lcz - 0.0012:
                v.co.z = lcz - 0.0012             # upper half-shell only
        smooth(ld)
        add(ld, "head", (sgn * 0.034, HY - 0.0790, lcz + 0.003), 0.05)
        br = box(f"Brow.{side}", (sgn * 0.037, HY - 0.0955, CZ + 0.0265), 0.0235, 0.0075, 0.0065,
                 M[cfg.get("hair", "hair_m")], bevel=0.002, bseg=1)
        tilt(br, rz=6 * sgn)
        smooth(br)
        add(br, "head", (sgn * 0.037, HY - 0.0955, CZ + 0.0315), 0.06)
        er = ball(f"Ear.{side}", (sgn * 0.097, HY + 0.004, CZ + 0.005), 0.009, 0.021, 0.026, skin, seg=12, ring=10)
        smooth(er)
        add(er, "head", (sgn * 0.097, HY + 0.004, CZ + 0.005), 0.06)

    ncy, ncz = HY - 0.102, CZ - 0.006
    ns_ = box("Nose", (0, ncy, ncz), 0.0100, 0.020, 0.031, skin, bevel=0.004, bseg=2)
    for v in ns_.data.vertices:
        if v.co.z > ncz + 0.008:
            v.co.y += 0.006                       # bridge recedes
            v.co.x *= 0.8                         # narrow top
        if v.co.z < ncz - 0.010:
            v.co.y -= 0.0055                      # tip projects
    subsurf(ns_, 1)
    smooth(ns_)
    add(ns_, "head", (0, ncy, ncz), 0.08)

    # subtle mouth: thin crease line + soft upper-lip ridge (skin tones, no slab)
    mouth_dark = mat("CRV2_mouthline", tuple(c * 0.55 for c in PAL[cfg.get("skin", "skin")]), rough=0.6)
    lcy, lcz2 = HY - 0.0962, CZ - 0.0405
    lp = box("Lips", (0, lcy, lcz2), 0.0225, 0.0045, 0.0028, mouth_dark, bevel=0.001, bseg=1)
    for v in lp.data.vertices:
        v.co.x *= (1.0 - 0.30 * min(1.0, abs(v.co.z - lcz2) / 0.004))
    smooth(lp)
    add(lp, "head", (0, lcy, lcz2), 0.06)
    ulip = ball("UpperLip", (0, HY - 0.0930, CZ - 0.0345), 0.020, 0.0065, 0.0058, skin, seg=14, ring=8)
    smooth(ulip)
    add(ulip, "head", (0, HY - 0.0930, CZ - 0.0345), 0.05)

    chn = ball("ChinPad", (0, HY - 0.0855, CZ - 0.075), 0.021, 0.011, 0.015, skin, seg=14, ring=10)
    smooth(chn)
    add(chn, "head", (0, HY - 0.0855, CZ - 0.075), 0.05)

    # ---- hair (local math around head center)
    style = cfg.get("hair_style", "crop")
    if style != "none":
        import mathutils
        hairm = M[cfg.get("hair", "hair_m")]
        hc = mathutils.Vector((0, HY, CZ))
        bpy.ops.object.select_all(action="DESELECT")
        hd.select_set(True)
        bpy.context.view_layer.objects.active = hd
        bpy.ops.object.duplicate()
        hr = bpy.context.active_object
        hr.name = "Hair"
        if style == "crop":
            def keep(d):
                if d.y < -0.02:                       # face side: high hairline
                    return d.z > 0.052
                if abs(d.x) > 0.070:                  # temples/sides
                    return d.z > -0.010 and d.y > -0.040
                return d.z > 0.014 if d.y < 0.01 else d.z > -0.058   # crown / back
        elif style == "ponytail":
            def keep(d):
                if d.y < -0.02:
                    return d.z > 0.046
                if abs(d.x) > 0.066:
                    return d.z > -0.022 and d.y > -0.045
                return d.z > 0.010 if d.y < 0.01 else d.z > -0.078
        else:  # patchy
            def keep(d):
                return d.z > 0.045 and rnd.random() < 0.5
        bm = bmesh.new()
        bm.from_mesh(hr.data)
        bm.verts.ensure_lookup_table()
        rm = [v for v in bm.verts if not keep(mathutils.Vector(v.co) - hc)]
        bmesh.ops.delete(bm, geom=rm, context="VERTS")
        bm.to_mesh(hr.data)
        bm.free()
        for v in hr.data.vertices:
            dvec = mathutils.Vector(v.co) - hc
            Lv = dvec.length
            if Lv > 1e-5:
                extra = 0.011 + (rnd.uniform(0.0, 0.004) if style != "patchy" else rnd.uniform(0.0, 0.008))
                nv = hc + dvec * ((Lv + extra) / Lv)
                v.co.x, v.co.y, v.co.z = nv.x, nv.y, nv.z
        hr.data.materials.clear()
        hr.data.materials.append(hairm)
        smooth(hr, 55)
        P[hr.name] = (hr, "head")
        INTENT[hr.name] = ((0, HY + 0.01, CZ + 0.035), 0.30)
        if style == "ponytail":
            tail = limb("Ponytail", (0, HY + 0.095, CZ + 0.045), (0, HY + 0.16, CZ - 0.13), 0.030, 0.012, hairm, bulge=1.05)
            smooth(tail)
            P[tail.name] = (tail, "head")
            INTENT[tail.name] = ((0, HY + 0.127, CZ - 0.042), 0.30)
            hb = box("HairBand", (0, HY + 0.098, CZ + 0.040), 0.030, 0.022, 0.014, M["strap"], bevel=0.004, bseg=1)
            smooth(hb)
            P[hb.name] = (hb, "head")
            INTENT[hb.name] = ((0, HY + 0.098, CZ + 0.040), 0.08)

    # ---- gear
    if cfg.get("gear", True):
        bandm = M["strap"]
        band = box("RigBand", (0, -0.002, 1.30), 0.222 * wS, 0.148, 0.035, bandm, bevel=0.008, bseg=1)
        smooth(band)
        add(band, "chest", (0, -0.002, 1.30), 0.50)
        for sgn, side in ((1, "R"), (-1, "L")):
            vs = box(f"RigStrap.{side}", (sgn * 0.10, -0.002, 1.415), 0.032, 0.142, 0.10, bandm, bevel=0.006, bseg=1)
            smooth(vs)
            add(vs, "chest", (sgn * 0.10, -0.002, 1.415), 0.32)
            for j in range(2):
                mp = box(f"Pouch{j}.{side}", (sgn * (0.055 + j * 0.075), -0.132, 1.278), 0.030, 0.022, 0.048, M["vest"], bevel=0.006, bseg=2)
                smooth(mp)
                add(mp, "chest", (sgn * (0.055 + j * 0.075), -0.132, 1.278), 0.12)
        bp = box("Backpack", (0, 0.155, 1.27), 0.155, 0.065, 0.20, M["vest"], bevel=0.03, bseg=3)
        subsurf(bp, 1)
        smooth(bp)
        add(bp, "chest", (0, 0.155, 1.27), 0.45)
        hl = box("Holster", (0.196, -0.045, 0.80), 0.030, 0.055, 0.095, bandm, bevel=0.008, bseg=2)
        tilt(hl, ry=6)
        smooth(hl)
        add(hl, "thigh.R", (0.196, -0.045, 0.80), 0.22)
        wt = box("Watch", (-0.368, -0.082, 0.99), 0.030, 0.030, 0.018, M["metal"], bevel=0.006, bseg=2)
        smooth(wt)
        add(wt, "forearm.L", (-0.368, -0.082, 0.99), 0.08)
        col = box("Collar", (0, 0.02, 1.492), 0.118, 0.105, 0.032, top, bevel=0.012, bseg=2)
        for v in col.data.vertices:
            if v.co.y < -0.04:
                v.co.z -= 0.012
        subsurf(col, 1)
        smooth(col)
        add(col, "neck", (0, 0.02, 1.488), 0.28)
        zp = box("Zipper", (0, -0.128, 1.20), 0.006, 0.012, 0.20, bandm, bevel=0.002, bseg=1)
        smooth(zp)
        add(zp, "spine", (0, -0.128, 1.20), 0.42)
        if female:
            ab_ = box("MedicArmband", (-0.30, -0.014, 1.33), 0.045, 0.062, 0.045, M["eye_w"], bevel=0.006, bseg=1)
            smooth(ab_)
            add(ab_, "upperarm.L", (-0.30, -0.014, 1.33), 0.14)
            cr1 = box("CrossV", (-0.348, -0.014, 1.330), 0.006, 0.011, 0.030, M["cross"], bevel=0.001, bseg=1)
            smooth(cr1)
            add(cr1, "upperarm.L", (-0.348, -0.014, 1.330), 0.07)
            cr2 = box("CrossH", (-0.348, -0.014, 1.330), 0.006, 0.030, 0.011, M["cross"], bevel=0.001, bseg=1)
            smooth(cr2)
            add(cr2, "upperarm.L", (-0.348, -0.014, 1.330), 0.07)

    if cfg.get("wounds", 0) > 0:
        spots = [((0.11, -0.115, 1.33), 0.05, "chest"), ((-0.14, -0.09, 1.16), 0.04, "spine"),
                 ((0.05, -0.095, 1.71), 0.028, "head"), ((-0.30, -0.05, 1.24), 0.032, "upperarm.L"),
                 ((0.15, 0.05, 0.75), 0.045, "thigh.R")][: cfg["wounds"]]
        for i, (loc, r, bone) in enumerate(spots):
            w = ball(f"Wound{i}", loc, r, 0.018, r * 1.15, M["wound"], seg=12, ring=8)
            smooth(w)
            add(w, bone, loc, r * 3.2)

    bad = validate_parts(P, INTENT)
    if bad:
        raise RuntimeError(f"validate_parts FAILED: {bad}")
    return P

def assemble(name, P, M, cfg):
    face_tag = {"Lid.": "KEY_LIDS", "Brow.": "KEY_BROWS"}
    for pname, (ob, bone) in P.items():
        vg = ob.vertex_groups.new(name=bone)
        vg.add(range(len(ob.data.vertices)), 1.0, "REPLACE")
        key = None
        for pre, kn in face_tag.items():
            if pname.startswith(pre):
                key = kn
        if pname in ("Lips", "UpperLip"):
            key = "KEY_MOUTH"
        if pname == "ChinPad":
            key = "KEY_JAW"
        if key:
            kg = ob.vertex_groups.new(name=key)
            kg.add(range(len(ob.data.vertices)), 1.0, "REPLACE")

    objs = [ob for ob, b in P.values()]
    bpy.ops.object.select_all(action="DESELECT")
    for ob in objs:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.object.join()
    body = bpy.context.active_object
    body.name = f"{name}_Mesh"
    me = body.data

    jaw_g = body.vertex_groups.get("KEY_JAW") or body.vertex_groups.new(name="KEY_JAW")
    mouth_g = body.vertex_groups.get("KEY_MOUTH") or body.vertex_groups.new(name="KEY_MOUTH")
    head_g = body.vertex_groups.get("head")
    if head_g:
        hidx = head_g.index
        for v in me.vertices:
            for g in v.groups:
                if g.group == hidx and g.weight > 0.5:
                    if v.co.z < CZ - 0.040 and v.co.y < HY - 0.030:
                        jaw_g.add([v.index], 1.0, "REPLACE")
                    if (CZ - 0.054) < v.co.z < (CZ - 0.024) and v.co.y < HY - 0.083 and abs(v.co.x) < 0.045:
                        mouth_g.add([v.index], 0.8, "REPLACE")

    def blend(gA, gB, verts_pred, tfun):
        ga, gb = body.vertex_groups.get(gA), body.vertex_groups.get(gB)
        if not ga or not gb:
            return
        ia, ib = ga.index, gb.index
        for v in me.vertices:
            wa = wb = 0.0
            for g in v.groups:
                if g.group == ia:
                    wa = g.weight
                elif g.group == ib:
                    wb = g.weight
            if wa + wb < 0.01 or not verts_pred(v.co):
                continue
            t = max(0.0, min(1.0, tfun(v.co)))
            t = t * t * (3 - 2 * t)
            ga.add([v.index], 1.0 - t, "REPLACE")
            gb.add([v.index], t, "REPLACE")

    for sgn, side in ((1, "R"), (-1, "L")):
        blend(f"upperarm.{side}", f"forearm.{side}",
              lambda co, s=sgn: co.x * s > 0.27 and 1.10 < co.z < 1.28,
              lambda co: (1.28 - co.z) / 0.18)
        blend(f"thigh.{side}", f"shin.{side}",
              lambda co, s=sgn: co.x * s > 0.05 and 0.46 < co.z < 0.60,
              lambda co: (0.60 - co.z) / 0.14)
        blend("chest", f"upperarm.{side}",
              lambda co, s=sgn: 0.18 < co.x * s < 0.30 and co.z > 1.36,
              lambda co, s=sgn: (co.x * s - 0.18) / 0.12)

    body.shape_key_add(name="Basis", from_mix=False)

    def key_from_group(kname, group, move):
        sk = body.shape_key_add(name=kname, from_mix=False)
        g = body.vertex_groups.get(group)
        if not g:
            return sk
        gi = g.index
        for v in me.vertices:
            w = 0.0
            for gr in v.groups:
                if gr.group == gi:
                    w = gr.weight
                    break
            if w <= 0.0:
                continue
            dx, dy, dz = move(v.co)
            sk.data[v.index].co.x = v.co.x + dx * w
            sk.data[v.index].co.y = v.co.y + dy * w
            sk.data[v.index].co.z = v.co.z + dz * w
        return sk

    key_from_group("Blink", "KEY_LIDS", lambda co: (0, -0.004, -0.020))
    key_from_group("BrowRaise", "KEY_BROWS", lambda co: (0, 0, 0.009))
    key_from_group("BrowAngry", "KEY_BROWS",
                   lambda co: (-0.004 * (1 if co.x > 0 else -1), -0.003, -0.007))
    key_from_group("Smile", "KEY_MOUTH",
                   lambda co: (0.012 * (1 if co.x > 0.004 else (-1 if co.x < -0.004 else 0)), -0.002,
                               0.008 if abs(co.x) > 0.004 else 0.002))
    key_from_group("Grimace", "KEY_MOUTH",
                   lambda co: (0.006 * (1 if co.x > 0.004 else (-1 if co.x < -0.004 else 0)), 0.002,
                               -0.008 if abs(co.x) > 0.004 else -0.002))
    key_from_group("JawOpen", "KEY_JAW", lambda co: (0, 0.006, -0.024))

    sk = body.shape_key_add(name="Alarm", from_mix=False)
    for gname, mv in (("KEY_BROWS", (0, 0, 0.012)), ("KEY_JAW", (0, 0.007, -0.028)), ("KEY_LIDS", (0, -0.002, 0.006))):
        g = body.vertex_groups.get(gname)
        if not g:
            continue
        gi = g.index
        for v in me.vertices:
            for gr in v.groups:
                if gr.group == gi and gr.weight > 0:
                    sk.data[v.index].co.x = v.co.x + mv[0] * gr.weight
                    sk.data[v.index].co.y = v.co.y + mv[1] * gr.weight
                    sk.data[v.index].co.z = v.co.z + mv[2] * gr.weight
    return body

def rig_and_animate(name, body, cfg):
    arm_data = bpy.data.armatures.new(f"{name}_Skeleton")
    arm = bpy.data.objects.new(f"{name}_Rig", arm_data)
    bpy.context.collection.objects.link(arm)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    ebs = {}
    for bname, head, tail, parent in BONES:
        eb = arm_data.edit_bones.new(bname)
        eb.head, eb.tail = head, tail
        if parent:
            eb.parent = ebs[parent]
        ebs[bname] = eb
    bpy.ops.object.mode_set(mode="OBJECT")
    md = body.modifiers.new("Armature", "ARMATURE")
    md.object = arm
    body.parent = arm

    hunch = math.radians(cfg.get("hunch", 0.0))

    def key_pose(aname, length, pose_fn):
        arm.animation_data_create()
        act = bpy.data.actions.new(f"{name}_{aname}")
        arm.animation_data.action = act
        scn = bpy.context.scene
        scn.frame_start, scn.frame_end = 1, length
        for f in range(1, length + 1, 4):
            scn.frame_set(f)
            t = (f - 1) / max(1, length - 1)
            for pb in arm.pose.bones:
                pb.rotation_mode = "XYZ"
                pb.rotation_euler = (0, 0, 0)
            pose_fn(arm.pose.bones, t)
            for pb in arm.pose.bones:
                pb.keyframe_insert("rotation_euler", frame=f)
        return act

    def with_hunch(b):
        if hunch:
            b["chest"].rotation_euler.x = hunch
            b["head"].rotation_euler.x = -hunch * 0.55
            b["upperarm.R"].rotation_euler.x = hunch * 1.2
            b["upperarm.L"].rotation_euler.x = hunch * 1.2

    def idle(b, t):
        s = math.sin(t * 2 * math.pi)
        with_hunch(b)
        b["chest"].rotation_euler.x += math.radians(1.6) * s
        b["head"].rotation_euler.z = math.radians(2.5) * s
        b["upperarm.R"].rotation_euler.z = math.radians(1.8) * s
        b["upperarm.L"].rotation_euler.z = -math.radians(1.8) * s

    zshuffle = cfg.get("zombie", False)

    def walk(b, t):
        s = math.sin(t * 2 * math.pi)
        c = math.sin(t * 2 * math.pi + math.pi)
        swing = math.radians(20 if zshuffle else 26)
        dragL = 0.55 if zshuffle else 1.0
        with_hunch(b)
        b["thigh.R"].rotation_euler.x = swing * s
        b["thigh.L"].rotation_euler.x = swing * c * dragL
        b["shin.R"].rotation_euler.x = max(0.0, -swing * s) * 0.9
        b["shin.L"].rotation_euler.x = max(0.0, -swing * c) * 0.9 * dragL
        b["upperarm.R"].rotation_euler.x += swing * 0.5 * c
        b["upperarm.L"].rotation_euler.x += swing * 0.5 * s
        b["chest"].rotation_euler.z = math.radians(6.0 if zshuffle else 3.0) * s
        b["foot.R"].rotation_euler.x = -swing * 0.3 * s
        b["foot.L"].rotation_euler.x = -swing * 0.3 * c

    def run(b, t):
        s = math.sin(t * 2 * math.pi)
        c = math.sin(t * 2 * math.pi + math.pi)
        swing = math.radians(42)
        with_hunch(b)
        b["chest"].rotation_euler.x += math.radians(9)
        b["thigh.R"].rotation_euler.x = swing * s
        b["thigh.L"].rotation_euler.x = swing * c
        b["shin.R"].rotation_euler.x = max(0.0, -swing * s) * 1.1
        b["shin.L"].rotation_euler.x = max(0.0, -swing * c) * 1.1
        b["upperarm.R"].rotation_euler.x = swing * 0.8 * c
        b["upperarm.L"].rotation_euler.x = swing * 0.8 * s
        b["forearm.R"].rotation_euler.x = math.radians(-55)
        b["forearm.L"].rotation_euler.x = math.radians(-55)
        b["chest"].rotation_euler.z = math.radians(4.0) * s

    acts = [key_pose("Idle", 60, idle), key_pose("Walk", 32, walk), key_pose("Run", 20, run)]
    for act in acts:
        tr = arm.animation_data.nla_tracks.new()
        tr.name = act.name
        tr.strips.new(act.name, 1, act)
    arm.animation_data.action = None
    return arm

def preview(path, cam_loc=(2.0, -2.2, 1.42), look=(0, 0, 1.0), res=760):
    import mathutils
    scn = bpy.context.scene
    scn.render.engine = "BLENDER_WORKBENCH"
    scn.display.shading.light = "STUDIO"
    scn.display.shading.color_type = "MATERIAL"
    scn.display.shading.show_shadows = True
    scn.render.resolution_x = scn.render.resolution_y = res
    cam_data = bpy.data.cameras.get("PrevCam") or bpy.data.cameras.new("PrevCam")
    cam = bpy.data.objects.get("PrevCamO")
    if cam is None:
        cam = bpy.data.objects.new("PrevCamO", cam_data)
    if cam.name not in {o.name for o in bpy.context.collection.objects}:
        bpy.context.collection.objects.link(cam)
    cam.location = cam_loc
    d = mathutils.Vector(look) - mathutils.Vector(cam_loc)
    cam.rotation_mode = "QUATERNION"
    cam.rotation_quaternion = d.to_track_quat('-Z', 'Y')
    scn.camera = cam
    scn.frame_set(1)
    scn.render.filepath = path
    bpy.ops.render.render(write_still=True)
    return path

def export_fbx(name, body, arm):
    bpy.ops.object.select_all(action="DESELECT")
    arm.select_set(True)
    body.select_set(True)
    bpy.context.view_layer.objects.active = arm
    fbx = os.path.join(OUT_DIR, f"{name}.fbx")
    bpy.ops.export_scene.fbx(
        filepath=fbx, use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False, use_armature_deform_only=True,
        bake_anim=True, bake_anim_use_nla_strips=True, bake_anim_use_all_actions=False,
        apply_scale_options="FBX_SCALE_ALL", axis_forward="-Y", axis_up="Z",
        mesh_smooth_type="FACE",
    )
    return fbx

CHARACTERS = {
    "SurvivorKenny": dict(seed=7, female=False, zombie=False, skin="skin", cloth_top="jacket",
                          cloth_leg="pants", hair="hair_m", hair_style="crop", gear=True, muscle=1.04),
    "SurvivorMaya": dict(seed=21, female=True, zombie=False, skin="skin_f", cloth_top="jacket_f",
                         cloth_leg="pants", hair="hair_f", hair_style="ponytail", gear=True, muscle=0.95),
    "ZombieShamblerV2": dict(seed=33, female=False, zombie=True, skin="zskin_a", cloth_top="rags_a",
                             cloth_leg="rags_b", hair="hair_m", hair_style="patchy", gear=False,
                             muscle=0.92, wounds=4, hunch=26.0),
    "ZombieBruteV2": dict(seed=44, female=False, zombie=True, skin="zskin_b", cloth_top="rags_b",
                          cloth_leg="rags_a", hair="hair_m", hair_style="none", gear=False,
                          muscle=1.35, wounds=5, hunch=12.0),
}

def build_one(name):
    cfg = CHARACTERS[name]
    wipe()
    M = mats()
    random.seed(cfg["seed"])
    P = build_parts(cfg, M)
    body = assemble(name, P, M, cfg)
    arm = rig_and_animate(name, body, cfg)
    full = preview(os.path.join(PREV_DIR, f"{name}.png"))
    headp = preview(os.path.join(PREV_DIR, f"{name}_head.png"), cam_loc=(0.42, -0.55, 1.80), look=(0, 0.01, 1.69))
    fbx = export_fbx(name, body, arm)
    return {"name": name, "fbx": fbx, "verts": len(body.data.vertices),
            "shape_keys": [k.name for k in body.data.shape_keys.key_blocks] if body.data.shape_keys else [],
            "previews": [full, headp]}

if globals().get("RUN_BUILD", True):
    RESULTS = []
    for cname in (BUILD_ONLY or list(CHARACTERS.keys())):
        RESULTS.append(build_one(cname))
    print("[characters_v2] DONE")
    for r in RESULTS:
        print("   ", r["name"], r["fbx"], r["verts"], "verts, keys:", ",".join(r["shape_keys"]))
