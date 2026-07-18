"""build_world_kit_v4.py — enterable-building hardware (2026-07-11 pass 4).

Two pieces for the first-level environment-physics work:
  * SM_Door_Steel  — swinging door leaf for the safehouse doorways.
      ORIGIN AT THE HINGE EDGE, base at z=0: the actor yaws to swing.
      Leaf spans x 0..1.04 m, thickness in Y, height 2.06 m. Panels, kick
      plate, push bar + knob on both faces, small wired viewport slit.
  * SM_Curb_Ramp  — 1.2 m wide wedge (0.60 m run, 0.15 m rise) used by the
      ground-unification pass wherever an INTENDED ground-level change
      remains (no more sheer curbs/steps between "grounds").

Meters scale, GLB out to RawArt/WorldKitV4/. Same B-builder as weapons v4.
Run inside Blender:  exec(open(r"<this file>").read())
"""
import bpy
import bmesh
import math
import os

PROJECT_ROOT = os.path.expanduser(
    "~/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix")
OUT_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WorldKitV4")
PREV_DIR = os.path.join(OUT_DIR, "previews_v4")
os.makedirs(OUT_DIR, exist_ok=True)
os.makedirs(PREV_DIR, exist_ok=True)


def wmat(name, color, rough=0.5, metal=0.8):
    m = bpy.data.materials.get(name)
    if m is None:
        m = bpy.data.materials.new(name)
        m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*color, 1.0)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    m.diffuse_color = (*color, 1.0)
    m.metallic = metal
    m.roughness = rough
    return m


DOORSTEEL = wmat("WK_DoorSteel", (0.16, 0.17, 0.19), 0.55, 0.85)
DOORTRIM  = wmat("WK_DoorTrim",  (0.08, 0.085, 0.09), 0.4, 0.9)
BRASSY    = wmat("WK_Handle",    (0.45, 0.34, 0.12), 0.35, 0.95)
CONCRETE  = wmat("WK_Concrete",  (0.42, 0.42, 0.40), 0.88, 0.0)
HAZARD    = wmat("WK_Hazard",    (0.75, 0.55, 0.08), 0.7, 0.1)


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

    def box(self, cx, cy, cz, sx, sy, sz, m):
        i = self._mi(m)
        r = bmesh.ops.create_cube(self.bm, size=1.0)
        for v in r["verts"]:
            v.co.x = v.co.x * sx + cx
            v.co.y = v.co.y * sy + cy
            v.co.z = v.co.z * sz + cz
        for f in {f for v in r["verts"] for f in v.link_faces}:
            f.material_index = i

    def wedge(self, cx, cy, cz, sx, sy, sz, m):
        """Right wedge: full height at -X side, tapering to 0 at +X side."""
        i = self._mi(m)
        r = bmesh.ops.create_cube(self.bm, size=1.0)
        for v in r["verts"]:
            if v.co.x > 0:
                v.co.z = -0.5              # collapse the +X top edge down
            v.co.x = v.co.x * sx + cx
            v.co.y = v.co.y * sy + cy
            v.co.z = v.co.z * sz + cz
        for f in {f for v in r["verts"] for f in v.link_faces}:
            f.material_index = i

    def done(self, max_dim, max_tris=3000):
        self.bm.to_mesh(self.mesh)
        self.bm.free()
        for m in self.mats:
            self.mesh.materials.append(m)
        dims = self.obj.dimensions
        tris = sum(len(p.vertices) - 2 for p in self.mesh.polygons)
        assert max(dims) <= max_dim + 1e-3, f"{self.obj.name} too big {tuple(dims)}"
        assert tris <= max_tris, f"{self.obj.name} {tris} tris"
        print(f"[world_kit_v4] OK {self.obj.name} dims=({dims.x:.2f},{dims.y:.2f},{dims.z:.2f}) tris={tris}")
        bpy.ops.object.select_all(action="DESELECT")
        self.obj.select_set(True)
        bpy.context.view_layer.objects.active = self.obj
        self.mesh.validate(clean_customdata=False)
        self._preview()
        path = os.path.join(OUT_DIR, f"{self.obj.name}.glb")
        bpy.ops.export_scene.gltf(filepath=path, use_selection=True,
                                  export_format="GLB", export_apply=True, export_yup=True)
        print(f"[world_kit_v4] exported {path}")

    def _preview(self):
        import mathutils
        scn = bpy.context.scene
        scn.render.engine = "BLENDER_WORKBENCH"
        scn.display.shading.light = "STUDIO"
        scn.display.shading.color_type = "MATERIAL"
        scn.display.shading.show_shadows = True
        scn.render.resolution_x = scn.render.resolution_y = 640
        cam_data = bpy.data.cameras.get("WKCam") or bpy.data.cameras.new("WKCam")
        cam = bpy.data.objects.get("WKCamO")
        if cam is None:
            cam = bpy.data.objects.new("WKCamO", cam_data)
        if cam.name not in {o.name for o in bpy.context.collection.objects}:
            bpy.context.collection.objects.link(cam)
        import mathutils as mu
        c = mu.Vector((self.obj.dimensions.x * 0.5, 0, self.obj.dimensions.z * 0.45))
        dist = max(self.obj.dimensions) * 1.6 + 0.3
        cam.location = c + mu.Vector((dist * 0.5, -dist, dist * 0.35))
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


def door_steel():
    b = B("SM_Door_Steel")
    W, H, T = 1.04, 2.06, 0.05
    b.box(W / 2, 0, H / 2, W, T, H, DOORSTEEL)                     # leaf
    for zc in (0.62, 1.30):                                        # recessed panels
        b.box(W / 2, T / 2 + 0.004, zc, W - 0.24, 0.012, 0.44, DOORTRIM)
        b.box(W / 2, -T / 2 - 0.004, zc, W - 0.24, 0.012, 0.44, DOORTRIM)
    b.box(W / 2, 0, 0.09, W - 0.06, T + 0.02, 0.18, DOORTRIM)      # kick plate
    b.box(W / 2, 0, 1.72, 0.16, T + 0.016, 0.30, DOORTRIM)         # viewport slit frame
    b.box(W - 0.10, T / 2 + 0.030, 1.02, 0.16, 0.028, 0.035, BRASSY)   # push bar (front)
    b.box(W - 0.085, -T / 2 - 0.028, 1.02, 0.045, 0.05, 0.045, BRASSY) # knob (back)
    b.box(0.028, 0, 0.35, 0.05, T + 0.03, 0.10, DOORTRIM)          # hinge straps
    b.box(0.028, 0, 1.05, 0.05, T + 0.03, 0.10, DOORTRIM)
    b.box(0.028, 0, 1.75, 0.05, T + 0.03, 0.10, DOORTRIM)
    b.done(2.10)


def curb_ramp():
    b = B("SM_Curb_Ramp")
    b.wedge(0.30, 0, 0.075, 0.60, 1.20, 0.15, CONCRETE)            # main wedge
    b.box(-0.02, 0, 0.075, 0.06, 1.20, 0.15, CONCRETE)             # back riser
    b.box(0.02, 0, 0.148, 0.10, 1.18, 0.012, HAZARD)               # hazard lip
    b.done(1.25)


wipe(); door_steel()
wipe(); curb_ramp()
print("[world_kit_v4] KIT COMPLETE")
