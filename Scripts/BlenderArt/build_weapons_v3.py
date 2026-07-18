"""build_weapons_v3.py — real-silhouette weapon set (2026-07-06).

Kenny: "weapon inclusions and viewable availabilities" — the first-person
weapon and the on-body holstered weapon must READ as real firearms at a
glance. Five weapons, correct proportions, principled materials (blued steel,
polymer, worn wood), muzzle/sight/magazine landmarks. Meters scale, +X is the
muzzle direction, origin at the grip so UE socket attachment is trivial.

Run inside Blender:  exec(open(r"<this file>").read())
Outputs: RawArt/WeaponsV3/<Name>.glb
"""
import bpy
import bmesh
import math
import os

PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WeaponsV3")
os.makedirs(OUT_DIR, exist_ok=True)


def wmat(name, color, rough=0.5, metal=0.8):
    m = bpy.data.materials.get(name)
    if m:
        return m
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*color, 1.0)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    return m


STEEL   = wmat("W_Steel",   (0.055, 0.057, 0.062), 0.38, 0.92)
POLYMER = wmat("W_Polymer", (0.052, 0.052, 0.055), 0.72, 0.05)
WOOD    = wmat("W_Wood",    (0.190, 0.115, 0.062), 0.62, 0.00)
BRASS   = wmat("W_Brass",   (0.420, 0.300, 0.080), 0.35, 0.95)
SIGHT   = wmat("W_Sight",   (0.65, 0.35, 0.05), 0.4, 0.2)
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

    def box(self, cx, cy, cz, sx, sy, sz, m, ry=0.0):
        i = self._mi(m)
        r = bmesh.ops.create_cube(self.bm, size=1.0)
        for v in r["verts"]:
            v.co.x *= sx; v.co.y *= sy; v.co.z *= sz
            if ry:
                x, z = v.co.x, v.co.z
                c, s = math.cos(ry), math.sin(ry)
                v.co.x = x * c + z * s
                v.co.z = -x * s + z * c
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

    def done(self, max_dim, max_tris=4000):
        self.bm.to_mesh(self.mesh)
        self.bm.free()
        for m in self.mats:
            self.mesh.materials.append(m)
        dims = self.obj.dimensions
        tris = sum(len(p.vertices) - 2 for p in self.mesh.polygons)
        assert max(dims) <= max_dim + 1e-3, f"{self.obj.name} too big {tuple(dims)}"
        assert tris <= max_tris, f"{self.obj.name} {tris} tris"
        print(f"[weapons_v3] OK {self.obj.name} dims=({dims.x:.2f},{dims.y:.2f},{dims.z:.2f}) tris={tris}")
        bpy.ops.object.select_all(action="DESELECT")
        self.obj.select_set(True)
        bpy.context.view_layer.objects.active = self.obj
        bevel = self.obj.modifiers.new(name="CR_ProductionEdgeRadius", type="BEVEL")
        bevel.width = 0.0025
        bevel.segments = 2
        bevel.limit_method = "ANGLE"
        bevel.angle_limit = math.radians(24.0)
        bevel.harden_normals = True
        bpy.ops.object.modifier_apply(modifier=bevel.name)
        self.mesh.validate(clean_customdata=False)
        path = os.path.join(OUT_DIR, f"{self.obj.name}.glb")
        bpy.ops.export_scene.gltf(filepath=path, use_selection=True,
                                  export_format="GLB", export_apply=True, export_yup=True)
        print(f"[weapons_v3] exported {path}")


def wipe():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for me in list(bpy.data.meshes):
        if me.users == 0:
            bpy.data.meshes.remove(me)


def pistol():
    b = B("PistolV3")
    b.box(0.055, 0, 0.075, 0.19, 0.030, 0.036, STEEL)             # slide
    for i in range(5):                                            # serrations
        b.box(-0.02 + i * 0.012, 0, 0.094, 0.006, 0.032, 0.004, POLYMER)
    b.box(0.05, 0, 0.045, 0.16, 0.028, 0.026, POLYMER)            # frame
    b.cylx(0.155, 0, 0.078, 0.007, 0.030, STEEL, seg=10)          # muzzle
    b.box(0.0, 0, -0.02, 0.032, 0.026, 0.085, POLYMER, ry=0.20)   # grip (raked)
    b.box(0.055, 0, 0.018, 0.055, 0.024, 0.012, POLYMER)          # trigger guard bottom
    b.box(0.032, 0, 0.032, 0.008, 0.020, 0.020, STEEL)            # trigger
    b.box(0.145, 0, 0.096, 0.008, 0.008, 0.008, SIGHT)            # front sight
    b.box(-0.035, 0, 0.096, 0.012, 0.018, 0.008, STEEL)           # rear sight
    b.box(0.045, 0.016, 0.076, 0.050, 0.003, 0.017, POLYMER)      # ejection port
    for i in range(4):                                            # grip ribs
        b.box(-0.005 - i * 0.006, 0.014, -0.018 - i * 0.010,
              0.022, 0.003, 0.003, STEEL, ry=0.20)
    b.done(0.30)


def shotgun():
    b = B("ShotgunV3")
    b.box(0.10, 0, 0.05, 0.24, 0.042, 0.062, STEEL)               # receiver
    b.cylx(0.52, 0, 0.072, 0.011, 0.62, STEEL)                    # barrel
    b.cylx(0.48, 0, 0.038, 0.009, 0.54, STEEL)                    # mag tube
    b.box(0.42, 0, 0.036, 0.13, 0.034, 0.030, WOOD)               # pump
    b.box(-0.16, 0, 0.028, 0.30, 0.040, 0.055, WOOD, ry=-0.10)    # stock
    b.box(-0.29, 0, 0.012, 0.045, 0.046, 0.095, WOOD, ry=-0.06)   # butt
    b.box(0.005, 0, 0.008, 0.05, 0.024, 0.014, STEEL)             # trigger guard
    b.box(0.825, 0, 0.086, 0.006, 0.006, 0.007, BRASS)            # bead sight
    for i in range(5):
        b.box(0.37 + i * 0.025, 0.035, 0.036, 0.010, 0.004, 0.028, STEEL)
    b.done(1.25)


def rifle():
    b = B("RifleV3")
    b.box(0.06, 0, 0.062, 0.30, 0.036, 0.050, POLYMER)            # upper
    b.box(0.03, 0, 0.020, 0.22, 0.034, 0.040, POLYMER)            # lower
    b.box(0.36, 0, 0.062, 0.30, 0.032, 0.042, POLYMER)            # handguard
    for i in range(6):                                            # rail bumps
        b.box(0.24 + i * 0.045, 0, 0.090, 0.018, 0.028, 0.006, STEEL)
    b.cylx(0.60, 0, 0.062, 0.008, 0.22, STEEL)                    # barrel
    b.cylx(0.72, 0, 0.062, 0.012, 0.05, STEEL)                    # flash hider
    b.box(-0.02, 0, -0.052, 0.045, 0.028, 0.10, POLYMER, ry=0.42) # magazine (curved suggestion)
    b.box(-0.055, 0, -0.115, 0.045, 0.028, 0.045, POLYMER, ry=0.62)
    b.box(-0.055, 0, -0.020, 0.030, 0.026, 0.075, POLYMER, ry=0.18)  # pistol grip
    b.box(-0.24, 0, 0.045, 0.20, 0.030, 0.036, POLYMER)           # buffer tube+stock
    b.box(-0.335, 0, 0.032, 0.05, 0.040, 0.085, POLYMER)          # butt pad
    b.box(0.62, 0, 0.098, 0.008, 0.006, 0.014, SIGHT)             # front post
    b.box(-0.04, 0, 0.100, 0.016, 0.020, 0.012, STEEL)            # rear sight
    b.cylx(0.13, 0, 0.125, 0.018, 0.12, STEEL, seg=16)            # compact optic
    b.cylx(0.13, 0, 0.125, 0.010, 0.125, SIGHT, seg=16)
    b.box(0.13, 0, 0.098, 0.08, 0.022, 0.015, STEEL)              # optic mount
    b.done(1.15)


def smg():
    b = B("SMGV3")
    b.box(0.05, 0, 0.045, 0.26, 0.034, 0.052, POLYMER)            # body
    b.cylx(0.26, 0, 0.052, 0.009, 0.16, STEEL)                    # barrel
    b.cylx(0.36, 0, 0.052, 0.014, 0.11, STEEL)                    # suppressor
    b.box(0.015, 0, -0.045, 0.032, 0.026, 0.085, POLYMER, ry=0.12)  # grip
    b.box(0.085, 0, -0.030, 0.028, 0.024, 0.075, STEEL, ry=0.05)  # mag
    b.box(-0.13, 0, 0.052, 0.14, 0.014, 0.014, STEEL)             # folded wire stock top
    b.box(-0.20, 0, 0.020, 0.014, 0.014, 0.062, STEEL)            # wire stock rear
    b.box(0.24, 0, 0.082, 0.007, 0.007, 0.009, SIGHT)
    b.done(0.66)   # 62cm with suppressor — realistic for a suppressed SMG


def crossbow():
    b = B("CrossbowV3")
    b.box(0.02, 0, 0.040, 0.55, 0.030, 0.035, WOOD)               # stock/rail
    b.box(-0.24, 0, 0.010, 0.10, 0.030, 0.075, WOOD, ry=-0.15)    # shoulder
    b.box(-0.02, 0, -0.030, 0.028, 0.024, 0.065, POLYMER, ry=0.15)  # grip
    for ys in (1, -1):                                            # limbs (swept)
        b.box(0.26, ys * 0.135, 0.052, 0.16, 0.24, 0.012, POLYMER, ry=0.0)
    b.box(0.245, 0, 0.052, 0.03, 0.56, 0.010, POLYMER)            # limb bridge
    b.box(0.05, 0, 0.052, 0.0035, 0.52, 0.0035, STRING)           # string
    b.cylx(0.16, 0, 0.075, 0.012, 0.10, STEEL)                    # scope
    b.box(0.30, 0, 0.048, 0.30, 0.006, 0.006, STEEL)              # bolt
    b.done(0.80, max_tris=4500)


wipe(); pistol()
wipe(); shotgun()
wipe(); rifle()
wipe(); smg()
wipe(); crossbow()
print("[weapons_v3] WEAPON KIT COMPLETE — 5 assets in", OUT_DIR)
