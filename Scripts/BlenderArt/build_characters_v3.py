# build_characters_v3.py — Operation Code Rescue character pipeline v3 (2026-07-11)
#
# Builds ON TOP of build_characters_v2.py (exec'd below with RUN_BUILD=False so we
# inherit its proven helpers: mats/ball/box/limb/tilt/validate_parts/build_parts/
# assemble/preview/export_fbx and all the Blender-5.1 world-space lessons).
#
# What v3 adds over v2:
#   * 5th character: ZombieRunnerV3 (lean sprinter — horde variety).
#   * Gore/gear detail parts (rib shadows, exposed ribs, spine ridge, belt,
#     shoulder patches, Kenny headlamp w/ EMISSIVE lens, Maya medic satchel).
#   * EMISSIVE zombie eyes (night readability + menace).
#   * NEW ANIMATIONS exported as FBX takes alongside Idle/Walk/Run:
#       zombies   -> Attack (22f one-shot), Flinch (12f), Death (44f collapse
#                    w/ pelvis translation keys — used when ragdoll budget is out)
#       survivors -> Wave (48f rescue celebration)
#       runner    -> Run tuned as frantic sprint-lunge
#   * Per-action preview stills (solo'd NLA track) for visual QA.
#
# Output: RawArt/CharactersV3/<Name>.fbx  previews: RawArt/CharactersV3/previews_v3/
# Run:    exec(open(<this file>).read())     (Blender MCP / Scripting tab)
#         blender --background --python <this file>
# Subset: set V3_BUILD_ONLY = ["ZombieRunnerV3"] in globals before exec.

import bpy, os, math

_HERE = os.path.expanduser(
    "~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Scripts/BlenderArt")
V3_BUILD_ONLY = globals().get("V3_BUILD_ONLY", None)
V3_RUN = globals().get("V3_RUN", True)

# ---- inherit v2 pipeline without running its build ----
RUN_BUILD = False
BUILD_ONLY = None
exec(open(os.path.join(_HERE, "build_characters_v2.py")).read())

# ---- v3 output locations (export_fbx/preview read these module globals) ----
OUT_DIR = os.path.join(PROJECT, "RawArt", "CharactersV3")
PREV_DIR = os.path.join(OUT_DIR, "previews_v3")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)

# ---- v3 palette additions (mats() picks these up; "skin" substring -> rough .5) ----
PAL.update({
    "zskin_c": (0.47, 0.44, 0.38),          # runner: pale grey-green
    "patch_o": (0.80, 0.34, 0.06),          # Kenny shoulder patch (beacon orange)
    "patch_t": (0.06, 0.48, 0.44),          # Maya shoulder patch (beacon teal)
    "ribpale": (0.70, 0.66, 0.55),          # exposed rib
    "ribshad": (0.15, 0.12, 0.10),          # rib shadow band
    "rubber":  (0.05, 0.05, 0.055),
})

def make_emissive(m, rgb, strength):
    """Add emission to a Principled material (Blender 4/5 input names)."""
    b = m.node_tree.nodes.get("Principled BSDF")
    if not b:
        return
    for nm in ("Emission Color", "Emission"):
        if nm in b.inputs:
            b.inputs[nm].default_value = (*rgb, 1.0)
            break
    if "Emission Strength" in b.inputs:
        b.inputs["Emission Strength"].default_value = strength

def add_v3_extras(P, cfg, M):
    """Detail parts on top of build_parts(). Same (ob, bone) contract + intent QA."""
    import random as _r
    rnd = _r.Random(cfg.get("seed", 7) + 100)
    zombie = cfg.get("zombie", False)
    female = cfg.get("female", False)
    EX, INT = {}, {}

    def add(ob, bone, loc, maxdim):
        EX[ob.name] = (ob, bone)
        INT[ob.name] = (loc, maxdim)
        P[ob.name] = (ob, bone)

    if zombie:
        # rib shadow bands + pale exposed ribs on the sunken chest
        for i, z in enumerate((1.245, 1.295)):
            rb = box(f"RibShadow{i}", (0.045, -0.132, z), 0.062, 0.007, 0.011,
                     M["ribshad"], bevel=0.002, bseg=1)
            tilt(rb, rz=-10)
            smooth(rb)
            add(rb, "chest", (0.045, -0.132, z), 0.16)
        for i, (x, z) in enumerate(((0.018, 1.268), (0.052, 1.272))):
            xr = box(f"ExposedRib{i}", (x, -0.136, z), 0.026, 0.006, 0.007,
                     M["ribpale"], bevel=0.002, bseg=1)
            tilt(xr, rz=-12)
            smooth(xr)
            add(xr, "chest", (x, -0.136, z), 0.07)
        if cfg.get("muscle", 1.0) > 1.2:      # brute: knobby spine ridge
            for i, z in enumerate((1.20, 1.30, 1.40)):
                sp = ball(f"SpineRidge{i}", (0.0, 0.148 + 0.012 * math.sin(i), z),
                          0.030, 0.026, 0.024, M["ribshad"], seg=12, ring=8)
                smooth(sp)
                add(sp, "chest" if z > 1.22 else "spine", (0.0, 0.148 + 0.012 * math.sin(i), z), 0.09)
    else:
        # belt under the jacket hem
        blt = box("Belt", (0, 0.005, 0.944), 0.372, 0.238, 0.018, M["strap"], bevel=0.005, bseg=1)
        smooth(blt)
        add(blt, "pelvis", (0, 0.005, 0.944), 0.78)
        # right-shoulder faction patch (beacon color language)
        pm = M["patch_t"] if female else M["patch_o"]
        pt = box("ShoulderPatch", (0.303, -0.014, 1.335), 0.008, 0.042, 0.046, pm, bevel=0.003, bseg=1)
        smooth(pt)
        add(pt, "upperarm.R", (0.303, -0.014, 1.335), 0.12)
        if female:
            # hip medic satchel
            st = box("MedicSatchel", (-0.195, 0.03, 0.855), 0.045, 0.095, 0.115,
                     M["vest"], bevel=0.012, bseg=2)
            smooth(st)
            add(st, "thigh.L", (-0.195, 0.03, 0.855), 0.28)
        else:
            # headlamp: strap + housing + emissive lens
            hs = box("LampStrap", (0, HY, CZ + 0.052), 0.104, 0.106, 0.016, M["strap"], bevel=0.004, bseg=1)
            smooth(hs)
            add(hs, "head", (0, HY, CZ + 0.052), 0.26)
            hb = box("LampBody", (0, HY - 0.104, CZ + 0.052), 0.020, 0.014, 0.020, M["metal"], bevel=0.004, bseg=1)
            smooth(hb)
            add(hb, "head", (0, HY - 0.104, CZ + 0.052), 0.06)
            lamp_m = mat("CRV3_lamplens", (1.0, 0.92, 0.62), rough=0.25)
            make_emissive(lamp_m, (1.0, 0.88, 0.55), 5.0)
            hl = box("LampLens", (0, HY - 0.113, CZ + 0.052), 0.013, 0.004, 0.013, lamp_m, bevel=0.001, bseg=1)
            smooth(hl)
            add(hl, "head", (0, HY - 0.113, CZ + 0.052), 0.04)

    bad = validate_parts(EX, INT)
    if bad:
        raise RuntimeError(f"v3 extras validate FAILED: {bad}")
    return EX

# ---- v3 rig & animation (v2 actions + Attack/Flinch/Death/Wave/Sprint) ----
def rig_and_animate_v3(name, body, cfg):
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
    zombie = cfg.get("zombie", False)
    sprint = cfg.get("sprint", False)

    def key_pose(aname, length, pose_fn, loc_bones=()):
        arm.animation_data_create()
        act = bpy.data.actions.new(f"{name}_{aname}")
        arm.animation_data.action = act
        scn = bpy.context.scene
        scn.frame_start, scn.frame_end = 1, length
        for f in range(1, length + 1, 2):
            scn.frame_set(f)
            t = (f - 1) / max(1, length - 1)
            for pb in arm.pose.bones:
                pb.rotation_mode = "XYZ"
                pb.rotation_euler = (0, 0, 0)
                if pb.name in loc_bones:
                    pb.location = (0, 0, 0)
            pose_fn(arm.pose.bones, t)
            for pb in arm.pose.bones:
                pb.keyframe_insert("rotation_euler", frame=f)
                if pb.name in loc_bones:
                    pb.keyframe_insert("location", frame=f)
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

    def walk(b, t):
        s = math.sin(t * 2 * math.pi)
        c = math.sin(t * 2 * math.pi + math.pi)
        swing = math.radians(20 if zombie else 26)
        dragL = 0.55 if zombie else 1.0
        with_hunch(b)
        b["thigh.R"].rotation_euler.x = swing * s
        b["thigh.L"].rotation_euler.x = swing * c * dragL
        b["shin.R"].rotation_euler.x = max(0.0, -swing * s) * 0.9
        b["shin.L"].rotation_euler.x = max(0.0, -swing * c) * 0.9 * dragL
        b["upperarm.R"].rotation_euler.x += swing * 0.5 * c
        b["upperarm.L"].rotation_euler.x += swing * 0.5 * s
        b["chest"].rotation_euler.z = math.radians(6.0 if zombie else 3.0) * s
        b["foot.R"].rotation_euler.x = -swing * 0.3 * s
        b["foot.L"].rotation_euler.x = -swing * 0.3 * c

    def run(b, t):
        s = math.sin(t * 2 * math.pi)
        c = math.sin(t * 2 * math.pi + math.pi)
        swing = math.radians(50 if sprint else 42)
        with_hunch(b)
        b["chest"].rotation_euler.x += math.radians(16 if sprint else 9)
        b["thigh.R"].rotation_euler.x = swing * s
        b["thigh.L"].rotation_euler.x = swing * c
        b["shin.R"].rotation_euler.x = max(0.0, -swing * s) * 1.1
        b["shin.L"].rotation_euler.x = max(0.0, -swing * c) * 1.1
        if sprint:
            # frantic zombie sprint: arms trail behind, head thrust forward
            b["upperarm.R"].rotation_euler.x = math.radians(28) + swing * 0.25 * c
            b["upperarm.L"].rotation_euler.x = math.radians(28) + swing * 0.25 * s
            b["forearm.R"].rotation_euler.x = math.radians(-18)
            b["forearm.L"].rotation_euler.x = math.radians(-18)
            b["head"].rotation_euler.x += math.radians(-9)
            b["chest"].rotation_euler.z = math.radians(7.0) * s
        else:
            b["upperarm.R"].rotation_euler.x = swing * 0.8 * c
            b["upperarm.L"].rotation_euler.x = swing * 0.8 * s
            b["forearm.R"].rotation_euler.x = math.radians(-55)
            b["forearm.L"].rotation_euler.x = math.radians(-55)
            b["chest"].rotation_euler.z = math.radians(4.0) * s

    def _ease(u):
        u = max(0.0, min(1.0, u))
        return u * u * (3 - 2 * u)

    def attack(b, t):
        with_hunch(b)
        if t < 0.35:                       # windup: rear back, arms overhead
            w = _ease(t / 0.35)
            b["chest"].rotation_euler.x += math.radians(-14) * w
            b["upperarm.R"].rotation_euler.x += math.radians(-118) * w
            b["upperarm.L"].rotation_euler.x += math.radians(-104) * w
            b["forearm.R"].rotation_euler.x = math.radians(-46) * w
            b["forearm.L"].rotation_euler.x = math.radians(-38) * w
            b["head"].rotation_euler.x += math.radians(-8) * w
        elif t < 0.60:                     # strike: whip forward/down
            s = _ease((t - 0.35) / 0.25)
            b["chest"].rotation_euler.x += math.radians(-14 + 36 * s)
            b["upperarm.R"].rotation_euler.x += math.radians(-118 + 96 * s)
            b["upperarm.L"].rotation_euler.x += math.radians(-104 + 86 * s)
            b["forearm.R"].rotation_euler.x = math.radians(-46 + 30 * s)
            b["forearm.L"].rotation_euler.x = math.radians(-38 + 26 * s)
            b["head"].rotation_euler.x += math.radians(-8 + 14 * s)
            b["chest"].rotation_euler.z = math.radians(6) * s
        else:                              # recover to neutral hunch
            r = _ease((t - 0.60) / 0.40)
            b["chest"].rotation_euler.x += math.radians(22) * (1 - r)
            b["upperarm.R"].rotation_euler.x += math.radians(-22) * (1 - r)
            b["upperarm.L"].rotation_euler.x += math.radians(-18) * (1 - r)
            b["head"].rotation_euler.x += math.radians(6) * (1 - r)

    def flinch(b, t):
        with_hunch(b)
        s = math.sin(min(1.0, t) * math.pi)
        b["chest"].rotation_euler.x += math.radians(-13) * s
        b["head"].rotation_euler.x += math.radians(-11) * s
        b["chest"].rotation_euler.z = math.radians(7) * s
        b["upperarm.R"].rotation_euler.x += math.radians(-16) * s
        b["upperarm.L"].rotation_euler.x += math.radians(-10) * s

    def death(b, t):
        # forward collapse: knees fold, chest pitches, pelvis drops to ground.
        with_hunch(b)
        k = _ease(min(1.0, t / 0.55))          # main fall 0..0.55, hold after
        settle = _ease(max(0.0, (t - 0.55) / 0.35))
        b["thigh.R"].rotation_euler.x = math.radians(74) * k
        b["thigh.L"].rotation_euler.x = math.radians(64) * k
        b["shin.R"].rotation_euler.x = math.radians(-98) * k
        b["shin.L"].rotation_euler.x = math.radians(-88) * k
        b["chest"].rotation_euler.x += math.radians(52) * k + math.radians(12) * settle
        b["spine"].rotation_euler.x = math.radians(18) * k
        b["head"].rotation_euler.x += math.radians(20) * k
        b["chest"].rotation_euler.z = math.radians(9) * k
        b["upperarm.R"].rotation_euler.x += math.radians(-52) * k
        b["upperarm.L"].rotation_euler.x += math.radians(-38) * k
        b["upperarm.R"].rotation_euler.z = math.radians(26) * k
        b["upperarm.L"].rotation_euler.z = math.radians(-30) * k
        # pelvis bone Y runs along world +Z (bone axis) -> -Y = drop toward ground
        b["pelvis"].location.y = -(0.50 * k + 0.16 * settle)
        b["pelvis"].rotation_euler.x = math.radians(34) * k + math.radians(10) * settle

    def wave(b, t):
        # rescue celebration: raise right arm overhead and wave, open posture
        if t < 0.22:
            u = _ease(t / 0.22)
            raise_amt = u
            wig = 0.0
        elif t < 0.82:
            raise_amt = 1.0
            wig = math.sin((t - 0.22) / 0.60 * math.pi * 4)
        else:
            raise_amt = 1.0 - _ease((t - 0.82) / 0.18)
            wig = 0.0
        b["upperarm.R"].rotation_euler.x = math.radians(-152) * raise_amt
        b["forearm.R"].rotation_euler.x = math.radians(-24) * raise_amt
        b["forearm.R"].rotation_euler.z = math.radians(20) * wig * raise_amt
        b["hand.R"].rotation_euler.z = math.radians(14) * wig * raise_amt
        b["chest"].rotation_euler.x = math.radians(-5) * raise_amt
        b["head"].rotation_euler.z = math.radians(6) * wig * raise_amt
        b["head"].rotation_euler.x = math.radians(-6) * raise_amt
        b["upperarm.L"].rotation_euler.z = -math.radians(9) * raise_amt

    acts = [key_pose("Idle", 60, idle), key_pose("Walk", 32, walk), key_pose("Run", 20, run)]
    if zombie:
        acts += [key_pose("Attack", 22, attack), key_pose("Flinch", 12, flinch),
                 key_pose("Death", 44, death, loc_bones=("pelvis",))]
    else:
        acts += [key_pose("Wave", 48, wave)]
    for act in acts:
        tr = arm.animation_data.nla_tracks.new()
        tr.name = act.name
        tr.strips.new(act.name, 1, act)
    arm.animation_data.action = None
    return arm, [a.name for a in acts]

def preview_frame(path, frame, cam_loc=(2.0, -2.2, 1.42), look=(0, 0, 1.0), res=760):
    """v2 preview() but at a chosen frame (v2 hard-resets to frame 1 internally)."""
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
    scn.frame_set(frame)
    bpy.context.view_layer.update()
    scn.render.filepath = path
    bpy.ops.render.render(write_still=True)
    return path

def preview_action(arm, act_name, frame, path, cam_loc=(2.0, -2.2, 1.42), look=(0, 0, 1.0)):
    """Render a still of ONE action: mute all NLA tracks and drive the action
    directly (NLA muting alone does not force pose re-evaluation for renders)."""
    act = bpy.data.actions.get(act_name)
    prev_mutes = [(tr, tr.mute) for tr in arm.animation_data.nla_tracks]
    for tr in arm.animation_data.nla_tracks:
        tr.mute = True
    arm.animation_data.action = act
    # Blender 5.x slotted actions: without the slot the action does NOT evaluate.
    if act is not None and hasattr(arm.animation_data, "action_slot") and getattr(act, "slots", None):
        arm.animation_data.action_slot = act.slots[0]
    out = preview_frame(path, frame, cam_loc=cam_loc, look=look)
    arm.animation_data.action = None
    # 2026-07-16 CRITICAL: a dangling action_slot after action=None corrupts
    # the Blender 5.1 FBX bind pose (collapsed rigs -> invisible characters in
    # UE). Clear it AND export before previews (see build_one_v3 ordering).
    try:
        arm.animation_data.action_slot = None
    except Exception:
        pass
    for tr, m in prev_mutes:
        tr.mute = m
    bpy.context.scene.frame_set(1)
    return out

CHARACTERS_V3 = {
    "SurvivorKennyV3": dict(seed=7, female=False, zombie=False, skin="skin", cloth_top="jacket",
                            cloth_leg="pants", hair="hair_m", hair_style="crop", gear=True, muscle=1.06),
    "SurvivorMayaV3": dict(seed=21, female=True, zombie=False, skin="skin_f", cloth_top="jacket_f",
                           cloth_leg="pants", hair="hair_f", hair_style="ponytail", gear=True, muscle=0.95),
    "ZombieShamblerV3": dict(seed=33, female=False, zombie=True, skin="zskin_a", cloth_top="rags_a",
                             cloth_leg="rags_b", hair="hair_m", hair_style="patchy", gear=False,
                             muscle=0.92, wounds=4, hunch=26.0),
    "ZombieBruteV3": dict(seed=44, female=False, zombie=True, skin="zskin_b", cloth_top="rags_b",
                          cloth_leg="rags_a", hair="hair_m", hair_style="none", gear=False,
                          muscle=1.38, wounds=5, hunch=12.0),
    "ZombieRunnerV3": dict(seed=55, female=False, zombie=True, skin="zskin_c", cloth_top="rags_b",
                           cloth_leg="rags_a", hair="hair_m", hair_style="patchy", gear=False,
                           muscle=0.84, wounds=3, hunch=7.0, sprint=True),
}

def build_one_v3(name):
    cfg = CHARACTERS_V3[name]
    wipe()
    M = mats()
    if cfg.get("zombie"):
        make_emissive(M["eye_z"], (0.55, 0.85, 0.28), 3.5)   # sickly glowing eyes
    import random as _r
    _r.seed(cfg["seed"])
    random.seed(cfg["seed"])
    P = build_parts(cfg, M)
    add_v3_extras(P, cfg, M)
    body = assemble(name, P, M, cfg)
    arm, act_names = rig_and_animate_v3(name, body, cfg)
    # EXPORT FIRST (2026-07-16): preview_action assigns actions/slots to the
    # armature; exporting afterwards produced corrupt bind poses on Blender
    # 5.1 (rigs collapsed to ~26 uu in UE -> invisible characters).
    fbx = export_fbx(name, body, arm)
    outs = [preview(os.path.join(PREV_DIR, f"{name}.png")),
            preview(os.path.join(PREV_DIR, f"{name}_head.png"),
                    cam_loc=(0.42, -0.55, 1.80), look=(0, 0.01, 1.69))]
    if cfg.get("zombie"):
        outs.append(preview_action(arm, f"{name}_Attack", 9,
                    os.path.join(PREV_DIR, f"{name}_attack_windup.png")))
        outs.append(preview_action(arm, f"{name}_Attack", 13,
                    os.path.join(PREV_DIR, f"{name}_attack_strike.png")))
        outs.append(preview_action(arm, f"{name}_Death", 44,
                    os.path.join(PREV_DIR, f"{name}_death_final.png"),
                    cam_loc=(1.9, -2.1, 1.1), look=(0, 0, 0.45)))
    else:
        outs.append(preview_action(arm, f"{name}_Wave", 24,
                    os.path.join(PREV_DIR, f"{name}_wave_mid.png")))
    return {"name": name, "fbx": fbx, "verts": len(body.data.vertices),
            "actions": act_names,
            "shape_keys": [k.name for k in body.data.shape_keys.key_blocks] if body.data.shape_keys else [],
            "previews": outs}

if V3_RUN:
    V3_RESULTS = []
    for cname in (V3_BUILD_ONLY or list(CHARACTERS_V3.keys())):
        V3_RESULTS.append(build_one_v3(cname))
    print("[characters_v3] DONE")
    for r in V3_RESULTS:
        print("   ", r["name"], r["fbx"], r["verts"], "verts, actions:", ",".join(r["actions"]))
