# build_characters.py — Operation Code Rescue character art pipeline (Blender 3.x/4.x, pure bpy)
#
# Builds THREE rigged, animated, stylized low-poly characters and exports each as a UE-ready FBX:
#   * Survivor          — the rescue target (warm jacket palette)
#   * ZombieShambler    — hunched silhouette, pale necrotic palette   (Top-50 item 17: unique silhouettes)
#   * ZombieBrute       — broad heavy silhouette, darker palette
#
# Each character: 16-bone armature, deterministic per-part skinning (vertex groups, no auto-weight
# surprises), an Idle and a Walk action, and a rendered preview PNG for review.
#
# Run it any of three ways:
#   1) Claude, live over the Blender MCP (execute_blender_code) once Blender is connected.
#   2) Double-click Run_Build_Characters_Blender.command (background Blender).
#   3) Blender's Scripting tab -> open this file -> Run.
#
# Output:  <project>/RawArt/Characters/<Name>.fbx  +  previews/<Name>.png
# Import:  queued bridge command imports RawArt into /Game/CodeRescueArt next time the editor opens.

import bpy, math, os

PROJECT = os.path.expanduser("~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix")
OUT_DIR = os.path.join(PROJECT, "RawArt", "Characters")
PREVIEW_DIR = os.path.join(OUT_DIR, "previews")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREVIEW_DIR, exist_ok=True)

# ----------------------------------------------------------------- palettes
PALETTES = {
    "Survivor": {
        "skin": (0.72, 0.55, 0.42), "torso": (0.75, 0.45, 0.12), "limbs": (0.20, 0.24, 0.30),
        "boots": (0.10, 0.10, 0.11), "accent": (0.90, 0.85, 0.70),
    },
    "ZombieShambler": {
        "skin": (0.55, 0.62, 0.48), "torso": (0.16, 0.15, 0.14), "limbs": (0.13, 0.14, 0.13),
        "boots": (0.08, 0.08, 0.08), "accent": (0.35, 0.10, 0.08),
    },
    "ZombieBrute": {
        "skin": (0.48, 0.50, 0.40), "torso": (0.11, 0.10, 0.10), "limbs": (0.10, 0.10, 0.11),
        "boots": (0.06, 0.06, 0.06), "accent": (0.45, 0.08, 0.06),
    },
}

# Body plan: part name -> (bone, center(x,y,z), size(x,y,z), material_key)
# X = right(+)/left(-), Y = forward(-)/back(+) (Blender -Y forward), Z = up. Units: meters.
def body_plan(brute=False):
    w = 1.25 if brute else 1.0     # width scale for the brute
    a = 1.18 if brute else 1.0     # arm thickness scale
    return [
        ("Pelvis",     "pelvis",       (0, 0, 0.95), (0.34*w, 0.22, 0.16), "limbs"),
        ("SpineBox",   "spine",        (0, 0, 1.12), (0.32*w, 0.20, 0.18), "torso"),
        ("ChestBox",   "chest",        (0, 0, 1.34), (0.40*w, 0.24, 0.26), "torso"),
        ("NeckBox",    "neck",         (0, 0, 1.52), (0.12, 0.12, 0.10), "skin"),
        ("HeadBox",    "head",         (0, 0, 1.70), (0.24, 0.26, 0.28), "skin"),
        ("UpArm.R",    "upperarm.R",   ( 0.30*w+0.11, 0, 1.38), (0.11*a, 0.12*a, 0.30), "torso"),
        ("LoArm.R",    "forearm.R",    ( 0.30*w+0.11, 0, 1.08), (0.09*a, 0.10*a, 0.28), "skin"),
        ("Hand.R",     "hand.R",       ( 0.30*w+0.11, 0, 0.88), (0.09, 0.10, 0.12), "skin"),
        ("UpArm.L",    "upperarm.L",   (-0.30*w-0.11, 0, 1.38), (0.11*a, 0.12*a, 0.30), "torso"),
        ("LoArm.L",    "forearm.L",    (-0.30*w-0.11, 0, 1.08), (0.09*a, 0.10*a, 0.28), "skin"),
        ("Hand.L",     "hand.L",       (-0.30*w-0.11, 0, 0.88), (0.09, 0.10, 0.12), "skin"),
        ("Thigh.R",    "thigh.R",      ( 0.11, 0, 0.70), (0.14, 0.16, 0.34), "limbs"),
        ("Shin.R",     "shin.R",       ( 0.11, 0, 0.36), (0.12, 0.13, 0.32), "limbs"),
        ("Foot.R",     "foot.R",       ( 0.11, -0.06, 0.09), (0.12, 0.26, 0.10), "boots"),
        ("Thigh.L",    "thigh.L",      (-0.11, 0, 0.70), (0.14, 0.16, 0.34), "limbs"),
        ("Shin.L",     "shin.L",       (-0.11, 0, 0.36), (0.12, 0.13, 0.32), "limbs"),
        ("Foot.L",     "foot.L",       (-0.11, -0.06, 0.09), (0.12, 0.26, 0.10), "boots"),
        ("Strap",      "chest",        (0.06*w, -0.13, 1.34), (0.10, 0.03, 0.30), "accent"),
    ]

# bone -> (head(x,y,z), tail(x,y,z), parent)
BONES = [
    ("pelvis",     (0, 0, 0.88), (0, 0, 1.02), None),
    ("spine",      (0, 0, 1.02), (0, 0, 1.22), "pelvis"),
    ("chest",      (0, 0, 1.22), (0, 0, 1.47), "spine"),
    ("neck",       (0, 0, 1.47), (0, 0, 1.56), "chest"),
    ("head",       (0, 0, 1.56), (0, 0, 1.84), "neck"),
    ("upperarm.R", ( 0.20, 0, 1.47), ( 0.41, 0, 1.24), "chest"),
    ("forearm.R",  ( 0.41, 0, 1.24), ( 0.41, 0, 0.96), "upperarm.R"),
    ("hand.R",     ( 0.41, 0, 0.96), ( 0.41, 0, 0.82), "forearm.R"),
    ("upperarm.L", (-0.20, 0, 1.47), (-0.41, 0, 1.24), "chest"),
    ("forearm.L",  (-0.41, 0, 1.24), (-0.41, 0, 0.96), "upperarm.L"),
    ("hand.L",     (-0.41, 0, 0.96), (-0.41, 0, 0.82), "forearm.L"),
    ("thigh.R",    ( 0.11, 0, 0.88), ( 0.11, 0, 0.52), "pelvis"),
    ("shin.R",     ( 0.11, 0, 0.52), ( 0.11, 0, 0.16), "thigh.R"),
    ("foot.R",     ( 0.11, 0, 0.16), ( 0.11, -0.20, 0.05), "shin.R"),
    ("thigh.L",    (-0.11, 0, 0.88), (-0.11, 0, 0.52), "pelvis"),
    ("shin.L",     (-0.11, 0, 0.52), (-0.11, 0, 0.16), "thigh.L"),
    ("foot.L",     (-0.11, 0, 0.16), (-0.11, -0.20, 0.05), "shin.L"),
]

def wipe_scene():
    bpy.ops.object.mode_set(mode="OBJECT") if bpy.context.object and bpy.context.object.mode != "OBJECT" else None
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for block in (bpy.data.meshes, bpy.data.armatures, bpy.data.materials, bpy.data.actions, bpy.data.cameras, bpy.data.lights):
        for item in list(block):
            if item.users == 0:
                block.remove(item)

def make_material(name, rgb):
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*rgb, 1.0)
        bsdf.inputs["Roughness"].default_value = 0.85
    m.diffuse_color = (*rgb, 1.0)
    return m

def build_character(char_name, hunch_deg=0.0, brute=False):
    wipe_scene()
    pal = PALETTES[char_name]
    mats = {k: make_material(f"{char_name}_{k}", rgb) for k, rgb in pal.items()}

    # ---- armature
    arm_data = bpy.data.armatures.new(f"{char_name}_Skeleton")
    arm_obj = bpy.data.objects.new(f"{char_name}_Rig", arm_data)
    bpy.context.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.mode_set(mode="EDIT")
    ebs = {}
    for bname, head, tail, parent in BONES:
        eb = arm_data.edit_bones.new(bname)
        eb.head, eb.tail = head, tail
        if parent: eb.parent = ebs[parent]
        ebs[bname] = eb
    bpy.ops.object.mode_set(mode="OBJECT")

    # ---- body parts (each fully weighted to its bone, then joined)
    part_objs = []
    for pname, bone, center, size, matkey in body_plan(brute=brute):
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=center)
        ob = bpy.context.active_object
        ob.name = f"{char_name}_{pname}"
        ob.scale = (size[0], size[1], size[2])
        bpy.ops.object.transform_apply(scale=True)
        ob.data.materials.append(mats[matkey])
        vg = ob.vertex_groups.new(name=bone)
        vg.add(range(len(ob.data.vertices)), 1.0, "REPLACE")
        part_objs.append(ob)

    bpy.ops.object.select_all(action="DESELECT")
    for ob in part_objs: ob.select_set(True)
    bpy.context.view_layer.objects.active = part_objs[0]
    bpy.ops.object.join()
    mesh_obj = bpy.context.active_object
    mesh_obj.name = f"{char_name}_Mesh"

    # bind: armature modifier + parent (vertex groups already carry the weights)
    mod = mesh_obj.modifiers.new(name="Armature", type="ARMATURE")
    mod.object = arm_obj
    mesh_obj.parent = arm_obj

    # ---- animation helpers
    def key_pose(action_name, length, pose_fn):
        arm_obj.animation_data_create()
        act = bpy.data.actions.new(f"{char_name}_{action_name}")
        arm_obj.animation_data.action = act
        scn = bpy.context.scene
        scn.frame_start, scn.frame_end = 1, length
        for f in range(1, length + 1, 4):
            scn.frame_set(f)
            t = (f - 1) / max(1, length - 1)          # 0..1
            for pb in arm_obj.pose.bones:
                pb.rotation_mode = "XYZ"
                pb.rotation_euler = (0, 0, 0)
            pose_fn(arm_obj.pose.bones, t)
            for pb in arm_obj.pose.bones:
                pb.keyframe_insert("rotation_euler", frame=f)
        return act

    H = math.radians(hunch_deg)
    def with_hunch(bones):
        if hunch_deg:
            bones["chest"].rotation_euler.x = H
            bones["head"].rotation_euler.x = -H * 0.55
            bones["upperarm.R"].rotation_euler.x = H * 1.2
            bones["upperarm.L"].rotation_euler.x = H * 1.2

    def idle_pose(bones, t):
        s = math.sin(t * 2 * math.pi)
        with_hunch(bones)
        bones["chest"].rotation_euler.x += math.radians(2.0) * s
        bones["head"].rotation_euler.z = math.radians(3.0) * s
        bones["upperarm.R"].rotation_euler.z = math.radians(2.0) * s
        bones["upperarm.L"].rotation_euler.z = -math.radians(2.0) * s

    def walk_pose(bones, t):
        s = math.sin(t * 2 * math.pi)
        c = math.sin(t * 2 * math.pi + math.pi)      # opposite phase
        swing = math.radians(28 if hunch_deg == 0 else 20)
        with_hunch(bones)
        bones["thigh.R"].rotation_euler.x = swing * s
        bones["thigh.L"].rotation_euler.x = swing * c
        bones["shin.R"].rotation_euler.x = max(0.0, -swing * s) * 0.9
        bones["shin.L"].rotation_euler.x = max(0.0, -swing * c) * 0.9
        bones["upperarm.R"].rotation_euler.x += swing * 0.55 * c
        bones["upperarm.L"].rotation_euler.x += swing * 0.55 * s
        bones["chest"].rotation_euler.z = math.radians(3.0) * s

    idle = key_pose("Idle", 60, idle_pose)
    walk = key_pose("Walk", 32, walk_pose)
    # stash both actions so the FBX exporter bakes them as separate takes
    for act in (idle, walk):
        track = arm_obj.animation_data.nla_tracks.new()
        track.name = act.name
        track.strips.new(act.name, 1, act)
    arm_obj.animation_data.action = None

    # ---- preview render (Workbench works headless)
    scn = bpy.context.scene
    scn.render.engine = "BLENDER_WORKBENCH"
    scn.display.shading.light = "STUDIO"
    scn.display.shading.color_type = "MATERIAL"
    scn.render.resolution_x = scn.render.resolution_y = 640
    cam_data = bpy.data.cameras.new("PrevCam")
    cam = bpy.data.objects.new("PrevCam", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (2.6, -2.6, 1.7)
    cam.rotation_euler = (math.radians(75), 0, math.radians(45))
    scn.camera = cam
    scn.frame_set(1)
    scn.render.filepath = os.path.join(PREVIEW_DIR, f"{char_name}.png")
    bpy.ops.render.render(write_still=True)

    # ---- FBX export (UE-friendly)
    bpy.ops.object.select_all(action="DESELECT")
    arm_obj.select_set(True); mesh_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    fbx_path = os.path.join(OUT_DIR, f"{char_name}.fbx")
    bpy.ops.export_scene.fbx(
        filepath=fbx_path, use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False, use_armature_deform_only=True,
        bake_anim=True, bake_anim_use_nla_strips=True, bake_anim_use_all_actions=False,
        apply_scale_options="FBX_SCALE_ALL", axis_forward="-Y", axis_up="Z",
        mesh_smooth_type="FACE",
    )
    print(f"[characters] exported {fbx_path}")
    return fbx_path

built = []
built.append(build_character("Survivor", hunch_deg=0.0, brute=False))
built.append(build_character("ZombieShambler", hunch_deg=28.0, brute=False))
built.append(build_character("ZombieBrute", hunch_deg=14.0, brute=True))
print("[characters] DONE:")
for b in built:
    print("   ", b, os.path.getsize(b) if os.path.exists(b) else "MISSING")
