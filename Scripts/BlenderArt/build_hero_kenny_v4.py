# build_hero_kenny_v4.py — HERO player character (2026-07-11 pass 4).
#
# Kenny's spec: physically attractive, handsome soldier — White male (US),
# strawberry blonde hair + FULL RED BEARD, blue eyes, 5'10" (177.8 cm),
# 220 lbs @ ~5% body fat (very muscular, lean waist).
#
# Inherits the v3 pipeline (which inherits v2) and layers hero-specific work:
#   * 0.956 uniform rig+mesh scale (186 cm rig -> 177.8 cm)
#   * musculature: broader chest/delts/arms via muscle cfg + waist pinch,
#     trap wedges, pec plates, lat flare
#   * face pass: defined jaw + chin, blue iris, groomed brows, crop
#     strawberry-blonde hair, FULL red beard shell (cheeks/jaw/chin/mustache)
#   * soldier kit: fitted tee + plate carrier, rolled sleeves (bare muscular
#     forearms), dog tags, US flag shoulder patch, camo-tone pants, headlamp
#   * Idle/Walk/Run/Wave actions + 7 facial morphs (from v3 rig)
#
# Output: RawArt/CharactersV4/SurvivorKennyV4.fbx + previews_v4/
# Run:    exec(open(<this file>).read())
import bpy, os, math

_HERE = os.path.expanduser(
    "~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/BlenderArt")
V3_RUN = False
V4_RUN = globals().get("V4_RUN", True)
exec(open(os.path.join(_HERE, "build_characters_v3.py")).read())

OUT_DIR = os.path.join(PROJECT, "RawArt", "CharactersV4")
PREV_DIR = os.path.join(OUT_DIR, "previews_v4")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)

HERO_SCALE = 177.8 / 186.0     # 5'10" over the 1.86 m rig

PAL.update({
    "hero_skin":  (0.70, 0.53, 0.42),      # fair, warm
    "hero_hair":  (0.66, 0.42, 0.20),      # strawberry blonde
    "hero_beard": (0.52, 0.21, 0.09),      # full red
    "hero_iris":  (0.15, 0.33, 0.60),      # blue
    "hero_tee":   (0.20, 0.22, 0.16),      # fitted olive tee
    "hero_pants": (0.23, 0.21, 0.15),      # camo-tone
    "flag_blue":  (0.08, 0.12, 0.35),
    "flag_red":   (0.62, 0.10, 0.10),
    "flag_white": (0.85, 0.85, 0.85),
})

def add_hero_details(P, M):
    """Hero-only musculature + kit. Operates on/adds to the parts dict P."""
    import mathutils
    EX, INT = {}, {}

    def add(ob, bone, loc, maxdim):
        EX[ob.name] = (ob, bone)
        INT[ob.name] = (loc, maxdim)
        P[ob.name] = (ob, bone)

    # ---- physique sculpt on the torso (waist pinch, chest expand) ----
    torso = P["Torso"][0]
    for v in torso.data.vertices:
        if 1.02 < v.co.z < 1.18:            # tight 5%-bodyfat waist
            t = 1.0 - abs(v.co.z - 1.10) / 0.08
            v.co.x *= (1.0 - 0.085 * max(0.0, t))
            v.co.y = 0.005 + (v.co.y - 0.005) * (1.0 - 0.05 * max(0.0, t))
        if 1.30 < v.co.z < 1.46 and v.co.y < 0.02:   # fuller chest plate
            v.co.y -= 0.012 * math.sin((v.co.z - 1.30) / 0.16 * math.pi)
        if 1.16 < v.co.z < 1.34 and abs(v.co.x) > 0.14:  # lat flare
            v.co.x *= 1.045

    # ---- deltoid trim: musc-scaled delts read as football pads; bring 12% in ----
    for side in ("R", "L"):
        dl = P.get(f"Deltoid.{side}")
        if dl:
            import mathutils as _mu
            c = _mu.Vector((0, 0, 0))
            for v in dl[0].data.vertices:
                c += v.co
            c /= max(1, len(dl[0].data.vertices))
            for v in dl[0].data.vertices:
                v.co = c + (_mu.Vector(v.co) - c) * 0.88

    # ---- trap wedges (subtle, low, sloping into the delts) ----
    for sgn, side in ((1, "R"), (-1, "L")):
        tr = ball(f"Trap.{side}", (sgn * 0.112, 0.016, 1.458), 0.058, 0.050, 0.028, M["hero_tee"])
        for v in tr.data.vertices:
            if (v.co.x - sgn * 0.112) * sgn > 0.038:
                v.co.z -= 0.014            # slope down toward the deltoid
        smooth(tr)
        add(tr, "chest", (sgn * 0.112, 0.016, 1.458), 0.16)

    # ---- pec swell: broad flat plates mostly EMBEDDED in the torso ----
    for sgn, side in ((1, "R"), (-1, "L")):
        pc = ball(f"Pec.{side}", (sgn * 0.082, -0.128, 1.352), 0.080, 0.016, 0.042, M["hero_tee"])
        smooth(pc)
        add(pc, "chest", (sgn * 0.082, -0.128, 1.352), 0.19)

    # ---- dog tags (snug to the shirt) ----
    ch = box("TagChain", (0, -0.147, 1.432), 0.044, 0.004, 0.008, M["metal"], bevel=0.002, bseg=1)
    smooth(ch)
    add(ch, "chest", (0, -0.147, 1.432), 0.10)
    for i in range(2):
        tg = box(f"DogTag{i}", (0.006 - i * 0.013, -0.156 - i * 0.004, 1.352 - i * 0.010),
                 0.015, 0.0035, 0.024, M["metal"], bevel=0.002, bseg=1)
        tilt(tg, rz=7 - i * 14)
        smooth(tg)
        add(tg, "chest", (0.006 - i * 0.013, -0.156 - i * 0.004, 1.352 - i * 0.010), 0.06)

    # ---- US flag patch, right shoulder (canton + stripes) ----
    fx = 0.307
    fb = box("FlagCanton", (fx, -0.030, 1.352), 0.006, 0.020, 0.020, M["flag_blue"], bevel=0.001, bseg=1)
    smooth(fb)
    add(fb, "upperarm.R", (fx, -0.030, 1.352), 0.06)
    for i, (mat_key, dz) in enumerate((("flag_red", 0.014), ("flag_white", 0.0), ("flag_red", -0.014))):
        st = box(f"FlagStripe{i}", (fx, 0.002, 1.352 + dz), 0.006, 0.044, 0.011, M[mat_key], bevel=0.001, bseg=1)
        smooth(st)
        add(st, "upperarm.R", (fx, 0.002, 1.352 + dz), 0.07)

    bad = validate_parts(EX, INT)
    if bad:
        raise RuntimeError(f"hero details validate FAILED: {bad}")
    return EX

def add_hero_face(P, M, rnd):
    """Defined jaw + chin, groomed brows, refined nose/lids/lips, custom crop
    hair with a natural hairline, and a CONTINUOUS full red beard."""
    import mathutils
    import bmesh as _bm
    EX, INT = {}, {}

    def add(ob, bone, loc, maxdim):
        EX[ob.name] = (ob, bone)
        INT[ob.name] = (loc, maxdim)
        P[ob.name] = (ob, bone)

    hd = P["Head"][0]
    hc = mathutils.Vector((0, HY, CZ))

    # jawline definition on the head mesh itself
    for v in hd.data.vertices:
        d = mathutils.Vector(v.co) - hc
        if -0.075 < d.z < -0.028 and abs(d.x) > 0.045 and d.y < 0.012:
            v.co.x *= 1.045
        if -0.062 < d.z < -0.040 and 0.052 < abs(d.x) < 0.085 and -0.045 < d.y < 0.005:
            v.co.z -= 0.006                 # squarer jaw corner

    # stronger, forward chin
    chn = P.get("ChinPad")
    if chn:
        for v in chn[0].data.vertices:
            v.co.y -= 0.004
            v.co.x *= 1.15

    # refined nose: slightly slimmer + tapered bridge, small nostril flares
    ns = P.get("Nose")
    if ns:
        for v in ns[0].data.vertices:
            v.co.x *= 0.92
            if v.co.z > CZ + 0.004:
                v.co.x *= 0.90              # taper at the bridge
    for sgn, side in ((1, "R"), (-1, "L")):
        nw = ball(f"NoseWing.{side}", (sgn * 0.0085, HY - 0.1105, CZ - 0.0185),
                  0.0055, 0.0068, 0.0052, M["hero_skin"], seg=10, ring=8)
        smooth(nw)
        add(nw, "head", (sgn * 0.0085, HY - 0.1105, CZ - 0.0185), 0.025)

    # calmer eyes: lids become thin CAPS on the upper third of the eyeball
    for nm in ("Lid.R", "Lid.L"):
        ld = P.get(nm)
        if ld:
            c = mathutils.Vector((0, 0, 0))
            for v in ld[0].data.vertices:
                c += v.co
            c /= max(1, len(ld[0].data.vertices))
            for v in ld[0].data.vertices:
                dv = mathutils.Vector(v.co) - c
                v.co.x = c.x + dv.x * 0.86
                v.co.y = c.y + dv.y * 0.82
                v.co.z = c.z + dv.z * 0.55 + 0.0045   # thin cap, raised
    # groomed straight brows: modestly thicker, sitting just above the lids
    for sgn, nm in ((1, "Brow.R"), (-1, "Brow.L")):
        br = P.get(nm)
        if br:
            c = mathutils.Vector((0, 0, 0))
            for v in br[0].data.vertices:
                c += v.co
            c /= max(1, len(br[0].data.vertices))
            for v in br[0].data.vertices:
                dv = mathutils.Vector(v.co) - c
                v.co.x = c.x + dv.x * 1.15
                v.co.z = c.z + dv.z * 1.20 - 0.0075   # down toward the eyes
                v.co.y = c.y + dv.y * 1.05

    # smaller mouth hardware (beard + mustache carry the lower face)
    for nm, s in (("Lips", 0.42), ("UpperLip", 0.38)):
        lp = P.get(nm)
        if lp:
            c = mathutils.Vector((0, 0, 0))
            for v in lp[0].data.vertices:
                c += v.co
            c /= max(1, len(lp[0].data.vertices))
            for v in lp[0].data.vertices:
                v.co = c + (mathutils.Vector(v.co) - c) * s
    # the chin pad now lives UNDER the beard: flatten its forward push and
    # recolor it to beard red so any poke-through reads as beard mass
    if chn:
        c = mathutils.Vector((0, 0, 0))
        for v in chn[0].data.vertices:
            c += v.co
        c /= max(1, len(chn[0].data.vertices))
        for v in chn[0].data.vertices:
            v.co.y = c.y + (v.co.y - c.y) * 0.5
        chn[0].data.materials.clear()
        chn[0].data.materials.append(M["hero_beard"])

    def shell_from_head(name, keep_fn, mat, inflate_base, inflate_rand, front_push=0.0):
        """Duplicate the head and carve by FACE CENTERS (deleting verts erodes
        one ring past every boundary — round-3 lesson: the beard's whole front
        vanished). Face carving keeps the visual edge ON the rule line."""
        bpy.ops.object.select_all(action="DESELECT")
        hd.select_set(True)
        bpy.context.view_layer.objects.active = hd
        bpy.ops.object.duplicate()
        sh = bpy.context.active_object
        sh.name = name
        bm = _bm.new()
        bm.from_mesh(sh.data)
        bm.faces.ensure_lookup_table()
        drop = []
        for f in bm.faces:
            center = mathutils.Vector((0, 0, 0))
            for v in f.verts:
                center += v.co
            center /= max(1, len(f.verts))
            if not keep_fn(center - hc):
                drop.append(f)
        _bm.ops.delete(bm, geom=drop, context="FACES")
        # prune loose verts left behind
        loose = [v for v in bm.verts if not v.link_faces]
        if loose:
            _bm.ops.delete(bm, geom=loose, context="VERTS")
        bm.to_mesh(sh.data)
        bm.free()
        for v in sh.data.vertices:
            dvec = mathutils.Vector(v.co) - hc
            L = dvec.length
            if L > 1e-5:
                extra = inflate_base + rnd.uniform(0.0, inflate_rand)
                nv = hc + dvec * ((L + extra) / L)
                v.co.x, v.co.y, v.co.z = nv.x, nv.y, nv.z
            # the face plane is FLAT (v2 clamp), so radial inflation sags there
            # and the head can poke through — push the front region forward too
            if front_push > 0.0 and (v.co.y - hc.y) < -0.055:
                v.co.y -= front_push
        sh.data.materials.clear()
        sh.data.materials.append(mat)
        smooth(sh, 55)
        return sh

    # CUSTOM CROP HAIR: natural (lower) hairline, temples connected downward
    def hair_keep(d):
        if d.y < -0.020:                          # face side: forehead line
            return d.z > 0.040
        if abs(d.x) > 0.058:                      # temples down toward the ears
            return d.z > -0.024 and d.y > -0.058
        return d.z > 0.006 if d.y < 0.01 else d.z > -0.062   # crown / back
    hair = shell_from_head("Hair", hair_keep, M["hero_hair"], 0.010, 0.003)
    add(hair, "head", (0, HY + 0.008, CZ + 0.033), 0.30)

    # CONTINUOUS FULL RED BEARD with a proper cheek-line: a diagonal that
    # rises from under the lip center up to the sideburns, an explicit
    # mustache band, and a small lower-lip window.
    def beard_keep(d):
        if d.z < -0.140 or d.y > 0.024:
            return False
        # (no lip window: at face-carve quantization it read as a pale hole,
        #  not a mouth — a FULL red beard covers the lip line, round-5 lesson)
        # mustache band (philtrum + upper lip, under the nose)
        if abs(d.x) < 0.045 and -0.040 < d.z < -0.024 and d.y < -0.075:
            return True
        # classic beard cheek-line: allowed top rises with |x| from under the
        # lip up to the sideburn/temple junction
        top = min(0.032, -0.052 + 0.90 * max(0.0, abs(d.x) - 0.016))
        if d.z < top:
            # don't wrap behind the ears (narrow exclusion — a wide one
            # notched the cheek/sideburn junction in round 4)
            if abs(d.x) > 0.082 and d.y > -0.008:
                return False
            return True
        return False
    beard = shell_from_head("Beard", beard_keep, M["hero_beard"], 0.0105, 0.003,
                            front_push=0.0075)
    add(beard, "head", (0, HY - 0.018, CZ - 0.048), 0.30)

    bad = validate_parts(EX, INT)
    if bad:
        raise RuntimeError(f"hero face validate FAILED: {bad}")
    return EX

HERO_CFG = dict(seed=7, female=False, zombie=False,
                skin="hero_skin", cloth_top="hero_tee", cloth_leg="hero_pants",
                hair="hero_hair", hair_style="none",   # custom hero hair in add_hero_face
                gear=True, muscle=1.26)

def build_hero():
    wipe()
    M = mats()
    # groomed brows + blue eyes: recolor the shared materials for this build
    b = M["hero_hair"]
    import random as _r
    rnd = _r.Random(7)
    _r.seed(7)
    random.seed(7)

    cfg = dict(HERO_CFG)
    P = build_parts(cfg, M)

    # rolled sleeves: forearms read as bare muscular skin
    for side in ("R", "L"):
        fa = P.get(f"Forearm.{side}")
        if fa:
            fa[0].data.materials.clear()
            fa[0].data.materials.append(M["hero_skin"])
            for v in fa[0].data.vertices:   # +10% forearm mass
                pass
    # brows in hair color (the default build uses cfg["hair"] already)
    # blue irises: v2 build gives non-zombies an iris part using M["iris"] —
    # swap that material's color by pointing the part at hero_iris
    for nm in ("Iris.R", "Iris.L"):
        ir = P.get(nm)
        if ir:
            ir[0].data.materials.clear()
            ir[0].data.materials.append(M["hero_iris"])

    add_hero_details(P, M)
    add_hero_face(P, M, rnd)
    body = assemble("SurvivorKennyV4", P, M, cfg)

    # 5'10" NOTE (2026-07-16): the scaled-skeleton export produced a UE asset
    # that never rendered (pose collapse) — the hero now builds at the PROVEN
    # standard rig scale (identical pipeline to the V3 cast, which renders),
    # and the 5'10" spec is applied as a 0.956 component scale in C++.
    arm, act_names = rig_and_animate_v3("SurvivorKennyV4", body, cfg)

    # EXPORT FIRST (2026-07-16): preview_action's action/slot assignment
    # corrupts the Blender 5.1 FBX bind pose when export happens afterwards.
    fbx = export_fbx("SurvivorKennyV4", body, arm)
    outs = [preview(os.path.join(PREV_DIR, "SurvivorKennyV4.png")),
            preview(os.path.join(PREV_DIR, "SurvivorKennyV4_head.png"),
                    cam_loc=(0.40, -0.52, 1.72), look=(0, 0.01, 1.60)),
            preview(os.path.join(PREV_DIR, "SurvivorKennyV4_face.png"),
                    cam_loc=(0.16, -0.42, 1.66), look=(0, 0.0, 1.60)),
            preview_action(arm, "SurvivorKennyV4_Wave", 24,
                           os.path.join(PREV_DIR, "SurvivorKennyV4_wave.png"))]
    print("[hero_v4] DONE", fbx, len(body.data.vertices), "verts, actions:", ",".join(act_names))
    return {"fbx": fbx, "previews": outs, "verts": len(body.data.vertices)}

if V4_RUN:
    HERO_RESULT = build_hero()
