"""Render the V5 enterable first-level structures for visual review."""

import math
import os

import bpy
from mathutils import Vector


PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
SOURCE_DIR = os.path.join(PROJECT_ROOT, "RawArt", "FirstLevelV5")
OUTPUT_PATH = os.path.join(
    PROJECT_ROOT,
    "Documentation",
    "improvement_pass_2026-07-09",
    "Renders",
    "first_level_v5_accessible_structures.png",
)
os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def import_glb(name, location, scale=0.72):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(SOURCE_DIR, name + ".glb"))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    for obj in imported:
        obj.location += Vector(location)
        obj.scale *= scale
    return imported


def material(name, color, roughness=0.8, metallic=0.0):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    return mat


def add_ground():
    bpy.ops.mesh.primitive_plane_add(size=2.0, location=(0.0, 2.0, -0.03))
    ground = bpy.context.object
    ground.scale = (23.0, 12.0, 1.0)
    ground.data.materials.append(material("Review Asphalt", (0.035, 0.042, 0.052), 0.92))

    for x in (-14.0, 0.0, 14.0):
        bpy.ops.mesh.primitive_cube_add(location=(x, -5.0, 0.06), scale=(4.2, 0.55, 0.06))
        curb = bpy.context.object
        curb.data.materials.append(material("Review Sidewalk", (0.30, 0.32, 0.34), 0.88))


def add_lighting():
    world = bpy.context.scene.world or bpy.data.worlds.new("V5 Review World")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.020, 0.028, 0.042, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.32

    for name, location, energy, color, size in (
        ("Warm Street Key", (-12.0, -14.0, 18.0), 3100.0, (1.0, 0.63, 0.38), 10.0),
        ("Cool Moon Fill", (16.0, -5.0, 22.0), 2600.0, (0.54, 0.70, 1.0), 11.0),
        ("Interior Rim", (0.0, 14.0, 14.0), 3800.0, (0.52, 0.78, 1.0), 13.0),
    ):
        light_data = bpy.data.lights.new(name, type="AREA")
        light_data.energy = energy
        light_data.color = color
        light_data.shape = "DISK"
        light_data.size = size
        light = bpy.data.objects.new(name, light_data)
        bpy.context.collection.objects.link(light)
        light.location = location
        look_at(light, (0.0, 2.5, 3.2))


def add_camera():
    camera_data = bpy.data.cameras.new("V5 Review Camera")
    camera = bpy.data.objects.new("V5 Review Camera", camera_data)
    bpy.context.collection.objects.link(camera)
    camera.location = (25.0, -42.0, 18.5)
    camera_data.lens = 52.0
    look_at(camera, (0.0, 1.5, 3.4))
    bpy.context.scene.camera = camera


clear_scene()
add_ground()
import_glb("AccessibleMarketV5", (-14.0, 2.5, 0.0))
import_glb("AccessibleClinicV5", (0.0, 3.0, 0.0))
import_glb("OpenStreetCafeV5", (14.0, 2.5, 0.0))
add_lighting()
add_camera()

scene = bpy.context.scene
scene.render.engine = "BLENDER_EEVEE"
scene.render.resolution_x = 1800
scene.render.resolution_y = 1000
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = "PNG"
scene.render.filepath = OUTPUT_PATH
scene.view_settings.look = "AgX - Medium High Contrast"
scene.render.film_transparent = False
bpy.ops.render.render(write_still=True)
print("[first_level_v5_review] COMPLETE", OUTPUT_PATH)
