"""build_weapons_v4.py — high-detail weapon set (2026-07-11).

Upgrades the five WeaponsV3 silhouettes with real hardware landmarks so first-
person AND third-person reads improve: serrations, rails, charging handles,
brass deflectors, vent ribs, side-saddle shells, folding stocks, cams/quiver,
EMISSIVE sight dots (night readability). Same contract as v3: meters scale,
+X down the muzzle, origin at the grip for trivial hand_R socket attachment.

Run inside Blender:  exec(open(r"<this file>").read())
Outputs: RawArt/WeaponsV4/<Name>V4.glb + previews_v4/<Name>V4.png
"""
import bpy
import bmesh
import math
import os

PROJECT_ROOT = os.path.expanduser(
    "~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix")
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WeaponsV4")
PREV_DIR = os.path.join(OUT_DIR, "previews_v4")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)


def wmat(name, color, rough=0.5, metal=0.8, emit=None, estr=0.0):
    m = bpy.data.materials.get(name)
    if m is None:
        m = bpy.data.materials.new(name)
        m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*color, 1.0)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    if emit is not None:
        for nm in ("Emission Color", "Emission"):
            if nm in b.inputs:
                b.inputs[nm].default_value = (*emit, 1.0)
                break
        if "Emission Strength" in b.inputs:
            b.inputs["Emission Strength"].default_value = estr
    m.diffuse_color = (*color, 1.0)   # workbench preview color
    m.metallic = metal
    m.roughness = rough
    return m


STEEL   = wmat("W_Steel",   (0.055, 0.057, 0.062), 0.38, 0.92)
DARKST  = wmat("W_DarkSteel", (0.030, 0.031, 0.035), 0.30, 0.95)
POLYMER = wmat("W_Polymer", (0.052, 0.052, 0.055), 0.72, 0.05)
WOOD    = wmat("W_Wood",    (0.190, 0.115, 0.062), 0.62, 0.00)
BRASS   = wmat("W_Brass",   (0.420, 0.300, 0.080), 0.35, 0.95)
RUBBER  = wmat("W_Rubber",  (0.040, 0.040, 0.042), 0.85, 0.00)
TAN     = wmat("W_Tan",     (0.240, 0.190, 0.120), 0.70, 0.02)
GLOW    = wmat("W_GlowDot", (0.20, 1.00, 0.30), 0.3, 0.0, emit=(0.25, 1.0, 0.3), estr=6.0)
REDDOT  = wmat("W_RedDot",  (1.00, 0.12, 0.05), 0.3, 0.0, emit=(1.0, 0.12, 0.05), estr=7.0)
STRING  = wmat("W_String",  (0.30, 0.29, 0.26), 0.8, 0.0)


class B:
    def __init__(self, name):
        self.mesh = bpy.data.meshes.new(name)
        self.obj = bpy.data.objects.new(name, self.mesh)
        bpy.context.collection.objects.link(self.obj)
        self.bm = bmesh.new()
        self.mats = []

    def _mi(self, m):
        names = [x.name for x in self.mats]
        if m.name not in names:
            self.mats.append(m)
            names.append(m.name)
        return names.index(m.name)

    def box(self, cx, cy, cz, sx, sy, sz, m, ry=0.0, rx=0.0):
        i = self._mi(m)
        r = bmesh.ops.create_cube(self.bm, size=1.0)
        for v in r["verts"]:
            v.co.x *= sx; v.co.y *= sy; v.co.z *= sz
            if ry:
                x, z = v.co.x, v.co.z
                c, s = math.cos(ry), math.sin(ry)
                v.co.x = x * c + z * s
                v.co.z = -x * s + z * c
            if rx:
                y, z = v.co.y, v.co.z
                c, s = math.cos(rx), math.sin(rx)
                v.co.y = y * c - z * s
                v.co.z = y * s + z * c
            v.co.x += cx; v.co.y += cy; v.co.z += cz
        for f in {f for v in r["verts"] for f in v.link_faces}:
            f.material_index = i

    def cylx(self, cx, cy, cz, radius, length, m, seg=12):
        i = self._mi(m)
        r = bmesh.ops.create_cone(self.bm, cap_ends=True, segments=seg,
                                  radius1=radius, radius2=radius, depth=length)
        for v in r["verts"]:
            v.co.x, v.co.z = v.co.z, -v.co.x
            v.co.x += cx; v.co.y += cy; v.co.z += cz
        for f in {f for v in r["verts"] for f in v.link_faces}:
            f.material_index = i

    def done(self, max_dim, max_tris=9000):
        self.bm.to_mesh(self.mesh)
        self.bm.free()
        for m in self.mats:
            self.mesh.materials.append(m)
        dims = self.obj.dimensions
        tris = sum(len(p.vertices) - 2 for p in self.mesh.polygons)
        assert max(dims) <= max_dim + 1e-3, f"{self.obj.name} too big {tuple(dims)}"
        assert tris <= max_tris, f"{self.obj.name} {tris} tris"
        print(f"[weapons_v4] OK {self.obj.name} dims=({dims.x:.2f},{dims.y:.2f},{dims.z:.2f}) tris={tris}")
        bpy.ops.object.select_all(action="DESELECT")
        self.obj.select_set(True)
        bpy.context.view_layer.objects.active = self.obj
        bevel = self.obj.modifiers.new(name="CR_ProductionEdgeRadius", type="BEVEL")
        bevel.width = 0.0022
        bevel.segments = 2
        bevel.limit_method = "ANGLE"
        bevel.angle_limit = math.radians(24.0)
        bevel.harden_normals = True
        bpy.ops.object.modifier_apply(modifier=bevel.name)
        self.mesh.validate(clean_customdata=False)
        self._preview()
        path = os.path.join(OUT_DIR, f"{self.obj.name}.glb")
        bpy.ops.export_scene.gltf(filepath=path, use_selection=True,
                                  export_format="GLB", export_apply=True, export_yup=True)
        print(f"[weapons_v4] exported {path}")

    def _preview(self):
        import mathutils
        scn = bpy.context.scene
        scn.render.engine = "BLENDER_WORKBENCH"
        scn.display.shading.light = "STUDIO"
        scn.display.shading.color_type = "MATERIAL"
        scn.display.shading.show_shadows = True
        scn.render.resolution_x = scn.render.resolution_y = 700
        cam_data = bpy.data.cameras.get("WCam") or bpy.data.cameras.new("WCam")
        cam = bpy.data.objects.get("WCamO")
        if cam is None:
            cam = bpy.data.objects.new("WCamO", cam_data)
        if cam.name not in {o.name for o in bpy.context.collection.objects}:
            bpy.context.collection.objects.link(cam)
        c = self.obj.location + mathutils.Vector(
            (self.obj.dimensions.x * 0.18, 0, self.obj.dimensions.z * 0.1))
        dist = max(self.obj.dimensions) * 1.35 + 0.12
        cam.location = c + mathutils.Vector((dist * 0.35, -dist, dist * 0.42))
        d = c - cam.location
        cam.rotation_mode = "QUATERNION"
        cam.rotation_quaternion = d.to_track_quat('-Z', 'Y')
        scn.camera = cam
        scn.render.filepath = os.path.join(PREV_DIR, f"{self.obj.name}.png")
        bpy.ops.render.render(write_still=True)


def wipe():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for me in list(bpy.data.meshes):
        if me.users == 0:
            bpy.data.meshes.remove(me)


def pistol():
    b = B("PistolV4")
    b.box(0.055, 0, 0.075, 0.19, 0.030, 0.036, STEEL)               # slide
    for i in range(5):                                              # rear serrations
        b.box(-0.022 + i * 0.011, 0, 0.094, 0.005, 0.032, 0.004, DARKST)
    for i in range(4):                                              # front serrations
        b.box(0.112 + i * 0.010, 0, 0.094, 0.005, 0.032, 0.004, DARKST)
    b.box(0.05, 0, 0.045, 0.16, 0.028, 0.026, POLYMER)              # frame
    for i in range(3):                                              # accessory rail slats
        b.box(0.085 + i * 0.016, 0, 0.0335, 0.007, 0.030, 0.005, DARKST)
    b.cylx(0.155, 0, 0.078, 0.007, 0.034, DARKST, seg=10)           # barrel/crown
    b.box(0.0, 0, -0.02, 0.032, 0.026, 0.085, POLYMER, ry=0.20)     # grip (raked)
    for i in range(5):                                              # grip stippling (on rake axis)
        zo = -0.004 - i * 0.0085
        gx, gz = zo * math.sin(0.20), -0.02 + zo * math.cos(0.20)
        b.box(gx, 0.0135, gz, 0.018, 0.0025, 0.0028, DARKST, ry=0.20)
        b.box(gx, -0.0135, gz, 0.018, 0.0025, 0.0028, DARKST, ry=0.20)
    b.box(-0.028, 0, 0.030, 0.020, 0.024, 0.014, POLYMER, ry=0.35)  # beavertail
    b.box(0.0, 0, -0.0645, 0.038, 0.030, 0.008, DARKST, ry=0.20)    # mag baseplate
    b.box(0.055, 0, 0.018, 0.055, 0.024, 0.012, POLYMER)            # trigger guard bottom
    b.box(0.084, 0, 0.028, 0.010, 0.024, 0.010, POLYMER)            # guard front
    b.box(0.032, 0, 0.032, 0.008, 0.018, 0.020, STEEL)              # trigger
    b.box(0.008, 0.016, 0.062, 0.030, 0.004, 0.008, STEEL)          # slide stop
    b.box(0.145, 0, 0.0965, 0.008, 0.008, 0.009, STEEL)             # front sight
    b.box(0.145, 0, 0.1015, 0.004, 0.004, 0.003, GLOW)              # front tritium dot
    b.box(-0.035, 0, 0.096, 0.014, 0.020, 0.009, STEEL)             # rear sight
    b.box(-0.035, 0.007, 0.1015, 0.003, 0.003, 0.003, GLOW)         # rear dots
    b.box(-0.035, -0.007, 0.1015, 0.003, 0.003, 0.003, GLOW)
    b.box(0.045, 0.016, 0.076, 0.050, 0.003, 0.017, DARKST)         # ejection port
    b.box(0.018, 0.0165, 0.076, 0.006, 0.003, 0.014, STEEL)         # extractor
    b.done(0.30)


def shotgun():
    b = B("ShotgunV4")
    b.box(0.10, 0, 0.05, 0.24, 0.042, 0.062, STEEL)                 # receiver
    b.box(0.10, 0, 0.018, 0.10, 0.030, 0.008, DARKST)               # loading port
    b.cylx(0.52, 0, 0.072, 0.011, 0.62, STEEL)                      # barrel
    b.box(0.53, 0, 0.0885, 0.56, 0.007, 0.005, DARKST)              # vent rib
    for i in range(7):                                              # rib posts
        b.box(0.30 + i * 0.077, 0, 0.0835, 0.006, 0.006, 0.006, DARKST)
    b.cylx(0.48, 0, 0.038, 0.009, 0.54, STEEL)                      # mag tube
    b.cylx(0.755, 0, 0.038, 0.011, 0.016, DARKST)                   # mag cap
    b.box(0.42, 0, 0.036, 0.13, 0.034, 0.030, WOOD)                 # pump
    for i in range(8):                                              # pump grooves
        b.box(0.368 + i * 0.0145, 0, 0.0205, 0.005, 0.036, 0.004, DARKST)
    b.box(0.28, 0.012, 0.045, 0.16, 0.005, 0.008, STEEL)            # action bar R
    b.box(0.28, -0.012, 0.045, 0.16, 0.005, 0.008, STEEL)           # action bar L
    b.box(-0.16, 0, 0.028, 0.30, 0.040, 0.055, WOOD, ry=-0.10)      # stock
    b.box(-0.175, 0, 0.062, 0.16, 0.032, 0.018, WOOD, ry=-0.10)     # comb riser
    b.box(-0.295, 0, 0.012, 0.040, 0.046, 0.095, RUBBER, ry=-0.06)  # recoil pad
    for i in range(4):                                              # side-saddle shells
        b.cylx(0.055 + i * 0.038, 0.0245, 0.058, 0.0075, 0.028, BRASS, seg=8)
        b.cylx(0.041 + i * 0.038, 0.0245, 0.058, 0.008, 0.005, DARKST, seg=8)
    b.box(0.005, 0, 0.008, 0.05, 0.024, 0.014, STEEL)               # trigger guard
    b.box(0.022, 0, 0.020, 0.007, 0.016, 0.014, STEEL)              # trigger
    b.box(0.825, 0, 0.086, 0.006, 0.006, 0.007, BRASS)              # front bead
    b.box(-0.045, 0, 0.088, 0.012, 0.022, 0.010, STEEL)             # ghost ring base
    b.box(-0.045, 0.009, 0.096, 0.003, 0.004, 0.010, STEEL)         # ring posts
    b.box(-0.045, -0.009, 0.096, 0.003, 0.004, 0.010, STEEL)
    b.done(1.25)


def rifle():
    b = B("RifleV4")
    b.box(0.06, 0, 0.062, 0.30, 0.036, 0.050, POLYMER)              # upper
    b.box(0.03, 0, 0.020, 0.22, 0.034, 0.040, POLYMER)              # lower
    b.box(0.075, 0.0185, 0.058, 0.055, 0.004, 0.030, DARKST)        # dust cover door
    b.box(0.115, 0.020, 0.052, 0.020, 0.006, 0.018, STEEL, ry=0.5)  # brass deflector
    b.box(-0.075, 0, 0.092, 0.030, 0.012, 0.010, DARKST)            # charging handle
    b.box(-0.075, 0.015, 0.092, 0.012, 0.018, 0.008, DARKST)        # charging latch
    b.box(0.36, 0, 0.062, 0.30, 0.032, 0.042, POLYMER)              # handguard
    for i in range(8):                                              # top rail slats
        b.box(0.235 + i * 0.036, 0, 0.090, 0.014, 0.028, 0.006, DARKST)
    for s in (1, -1):                                               # handguard vents
        for i in range(3):
            b.box(0.30 + i * 0.075, s * 0.0165, 0.062, 0.030, 0.002, 0.014, DARKST)
    b.box(0.44, 0, 0.030, 0.045, 0.024, 0.030, TAN, ry=0.35)        # angled foregrip
    b.cylx(0.60, 0, 0.062, 0.008, 0.22, STEEL)                      # barrel
    b.cylx(0.645, 0, 0.062, 0.012, 0.020, DARKST)                   # gas block
    b.cylx(0.715, 0, 0.062, 0.011, 0.045, DARKST)                   # flash hider body
    for ang in (0.0, 1.5708):                                       # 4 prongs
        b.box(0.744, 0.0 if ang else 0.0, 0.062, 0.014, 0.004 if ang else 0.020,
              0.020 if ang else 0.004, DARKST)
    b.box(-0.02, 0, -0.045, 0.045, 0.028, 0.10, POLYMER, ry=0.42)   # magazine seg 1
    b.box(-0.055, 0, -0.115, 0.045, 0.028, 0.045, POLYMER, ry=0.62) # magazine seg 2
    b.box(-0.075, 0, -0.140, 0.048, 0.030, 0.012, TAN, ry=0.62)     # mag baseplate
    for i in range(3):                                              # mag ribs
        b.box(-0.030 - i * 0.014, 0, -0.070 - i * 0.020, 0.006, 0.030, 0.006, DARKST, ry=0.55)
    b.box(-0.055, 0, -0.020, 0.030, 0.026, 0.075, POLYMER, ry=0.18) # pistol grip
    for i in range(3):                                              # grip ridges (on rake axis)
        zo = -0.010 - i * 0.011
        b.box(-0.055 + zo * math.sin(0.18), 0, -0.020 + zo * math.cos(0.18),
              0.022, 0.028, 0.003, DARKST, ry=0.18)
    b.box(-0.24, 0, 0.045, 0.20, 0.030, 0.036, POLYMER)             # buffer tube
    b.box(-0.315, 0, 0.038, 0.075, 0.036, 0.062, TAN)               # adjustable stock
    b.box(-0.335, 0, 0.070, 0.045, 0.030, 0.016, TAN)               # cheek riser
    b.box(-0.352, 0, 0.030, 0.012, 0.040, 0.080, RUBBER)            # butt pad
    b.box(0.62, 0, 0.098, 0.008, 0.006, 0.014, STEEL)               # front post
    b.box(-0.04, 0, 0.100, 0.016, 0.020, 0.012, STEEL)              # rear BUIS
    b.box(0.13, 0, 0.098, 0.08, 0.022, 0.015, STEEL)                # optic mount
    b.cylx(0.13, 0, 0.128, 0.019, 0.11, DARKST, seg=16)             # red dot tube
    b.cylx(0.185, 0, 0.128, 0.021, 0.010, STEEL, seg=16)            # hood ring
    b.box(0.088, 0, 0.128, 0.004, 0.010, 0.010, REDDOT)             # emissive dot
    b.cylx(-0.145, 0, 0.028, 0.008, 0.010, STEEL, seg=8)            # QD socket
    b.done(1.15)


def smg():
    b = B("SMGV4")
    b.box(0.05, 0, 0.045, 0.26, 0.034, 0.052, POLYMER)              # body
    for i in range(5):                                              # top rail
        b.box(-0.045 + i * 0.032, 0, 0.0755, 0.012, 0.026, 0.005, DARKST)
    b.box(0.062, 0.0175, 0.052, 0.048, 0.003, 0.015, DARKST)        # ejection port
    b.cylx(0.145, 0.014, 0.030, 0.006, 0.030, STEEL, seg=8)         # charging knob
    b.box(0.028, -0.0178, 0.038, 0.016, 0.003, 0.008, STEEL)        # selector
    b.cylx(0.26, 0, 0.052, 0.009, 0.16, STEEL)                      # barrel
    b.cylx(0.365, 0, 0.052, 0.0145, 0.115, DARKST)                  # suppressor
    for i in range(5):                                              # cooling rings
        b.cylx(0.325 + i * 0.020, 0, 0.052, 0.0155, 0.006, STEEL, seg=12)
    b.box(0.015, 0, -0.019, 0.032, 0.026, 0.085, POLYMER, ry=0.12)  # grip (meets body)
    for i in range(4):                                              # grip stippling (on rake axis)
        zo = -0.006 - i * 0.010
        b.box(0.015 + zo * math.sin(0.12), 0.0135, -0.019 + zo * math.cos(0.12),
              0.018, 0.0025, 0.0028, DARKST, ry=0.12)
    b.box(0.085, 0, -0.004, 0.028, 0.024, 0.055, STEEL, ry=0.05)    # mag seg 1 (in well)
    b.box(0.095, 0, -0.049, 0.028, 0.024, 0.045, STEEL, ry=0.22)    # mag seg 2 (curve)
    b.box(0.102, 0, -0.074, 0.030, 0.026, 0.010, DARKST, ry=0.22)   # baseplate
    b.box(0.155, 0, -0.002, 0.022, 0.022, 0.050, POLYMER)           # vertical foregrip
    b.box(-0.085, 0.020, 0.045, 0.014, 0.008, 0.020, STEEL)         # stock hinge
    b.box(-0.155, 0.020, 0.058, 0.13, 0.007, 0.010, STEEL)          # folded stock top bar
    b.box(-0.155, 0.020, 0.026, 0.13, 0.007, 0.010, STEEL)          # folded stock bottom bar
    b.box(-0.215, 0.020, 0.042, 0.012, 0.009, 0.045, RUBBER)        # buttplate (folded)
    b.box(0.205, 0, 0.0805, 0.006, 0.006, 0.007, STEEL)             # front sight
    b.box(0.205, 0, 0.0855, 0.0035, 0.0035, 0.003, GLOW)            # tritium dot
    b.cylx(-0.035, 0, 0.090, 0.011, 0.040, DARKST, seg=12)          # micro red-dot
    b.box(-0.052, 0, 0.090, 0.003, 0.007, 0.007, REDDOT)
    b.done(0.66)


def crossbow():
    b = B("CrossbowV4")
    b.box(0.02, 0, 0.040, 0.55, 0.030, 0.035, WOOD)                 # stock rail
    b.box(0.14, 0, 0.0585, 0.30, 0.008, 0.004, DARKST)              # flight groove
    b.box(-0.24, 0, 0.010, 0.10, 0.030, 0.075, WOOD, ry=-0.15)      # shoulder
    b.box(-0.285, 0, 0.032, 0.035, 0.028, 0.030, RUBBER, ry=-0.15)  # cheek pad
    b.box(-0.02, 0, -0.030, 0.028, 0.024, 0.065, POLYMER, ry=0.15)  # grip
    b.box(0.010, 0, -0.006, 0.020, 0.020, 0.012, STEEL)             # trigger box
    b.box(0.30, 0, 0.046, 0.06, 0.036, 0.030, POLYMER)              # riser
    for ys in (1, -1):                                              # split limbs (2 seg, slim chord)
        b.box(0.285, ys * 0.085, 0.052, 0.075, 0.15, 0.012, POLYMER, rx=ys * 0.12)
        b.box(0.253, ys * 0.195, 0.052, 0.052, 0.11, 0.010, POLYMER, rx=ys * 0.28)
        b.cylx(0.230, ys * 0.245, 0.052, 0.016, 0.020, DARKST, seg=10)  # cams
    b.box(0.05, 0, 0.052, 0.0035, 0.50, 0.0035, STRING)             # string
    b.box(0.13, 0.12, 0.052, 0.20, 0.0025, 0.0025, STRING, rx=0.0)  # cable R
    b.box(0.13, -0.12, 0.052, 0.20, 0.0025, 0.0025, STRING)         # cable L
    b.box(0.345, 0, 0.020, 0.055, 0.055, 0.008, STEEL)              # stirrup base
    b.box(0.372, 0.026, -0.004, 0.008, 0.006, 0.045, STEEL)         # stirrup sides
    b.box(0.372, -0.026, -0.004, 0.008, 0.006, 0.045, STEEL)
    b.box(0.372, 0, -0.028, 0.008, 0.058, 0.006, STEEL)
    b.cylx(0.16, 0, 0.082, 0.013, 0.11, DARKST, seg=12)             # scope tube
    b.cylx(0.115, 0, 0.082, 0.015, 0.008, STEEL, seg=12)            # ocular ring
    b.box(0.104, 0, 0.082, 0.003, 0.008, 0.008, REDDOT)             # emissive reticle
    b.box(0.16, 0, 0.064, 0.030, 0.008, 0.012, STEEL)               # scope mount (flush to rail)
    b.box(0.30, 0, 0.048, 0.30, 0.006, 0.006, STEEL)                # loaded bolt
    b.cylx(0.462, 0, 0.048, 0.0055, 0.020, BRASS, seg=8)            # bolt tip
    b.box(0.055, 0, 0.005, 0.020, 0.024, 0.038, DARKST)             # quiver bracket F
    b.box(0.155, 0, 0.005, 0.020, 0.024, 0.038, DARKST)             # quiver bracket R
    for i in range(3):                                              # quiver bolts (snug under rail)
        b.cylx(0.105, 0.0, -0.020 - i * 0.012, 0.004, 0.22, TAN, seg=6)
        b.cylx(0.219, 0.0, -0.020 - i * 0.012, 0.005, 0.014, BRASS, seg=6)
    b.done(0.80)


wipe(); pistol()
wipe(); shotgun()
wipe(); rifle()
wipe(); smg()
wipe(); crossbow()
print("[weapons_v4] WEAPON KIT COMPLETE — 5 assets in", OUT_DIR)
