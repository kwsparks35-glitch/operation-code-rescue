"""Render the July 9 production city and weapon assets for visual QA."""

import math
import os

import bpy
from mathutils import Vector


PROJECT_ROOT = "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
CITY_DIR = os.path.join(PROJECT_ROOT, "RawArt", "CityKitV3")
WEAPON_DIR = os.path.join(PROJECT_ROOT, "RawArt", "WeaponsV3")
OUT_DIR = os.path.join(
    PROJECT_ROOT, "Documentation", "improvement_pass_2026-07-09", "Renders")
os.makedirs(OUT_DIR, exist_ok=True)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.cameras, bpy.data.lights):
        for block in list(datablocks):
            if block.users == 0:
                datablocks.remove(block)


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def import_glb(folder, name, location, rotation=(0.0, 0.0, 0.0), scale=1.0):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(folder, name + ".glb"))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    for obj in imported:
        obj.location.x += location[0]
        obj.location.y += location[1]
        obj.location.z += location[2]
        obj.rotation_euler.rotate_axis("Z", rotation[2])
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


def add_ground(size=(32.0, 24.0), color=(0.055, 0.060, 0.065)):
    bpy.ops.mesh.primitive_plane_add(size=2.0, location=(0.0, 0.0, -0.03))
    ground = bpy.context.object
    ground.name = "QA Ground"
    ground.scale = (size[0] / 2.0, size[1] / 2.0, 1.0)
    ground.data.materials.append(material("QA_Ground", color, 0.92))


def add_lighting(target=(0.0, 0.0, 3.0)):
    world = bpy.context.scene.world or bpy.data.worlds.new("QA World")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.015, 0.020, 0.028, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.22

    for name, location, energy, color, size in (
        ("Cool Key", (5.0, -10.0, 16.0), 1900.0, (0.62, 0.76, 1.0), 8.0),
        ("Warm Fill", (-10.0, -3.0, 9.0), 1250.0, (1.0, 0.55, 0.30), 6.0),
        ("Rim", (2.0, 12.0, 13.0), 2300.0, (0.50, 0.68, 1.0), 7.0),
    ):
        data = bpy.data.lights.new(name, type="AREA")
        data.energy = energy
        data.color = color
        data.shape = "DISK"
        data.size = size
        light = bpy.data.objects.new(name, data)
        bpy.context.collection.objects.link(light)
        light.location = location
        look_at(light, target)


def setup_camera(location, target, lens=48.0):
    data = bpy.data.cameras.new("QA Camera")
    camera = bpy.data.objects.new("QA Camera", data)
    bpy.context.collection.objects.link(camera)
    camera.location = location
    data.lens = lens
    look_at(camera, target)
    bpy.context.scene.camera = camera


def render(path, resolution=(1600, 900)):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = path
    scene.render.film_transparent = False
    scene.render.image_settings.color_mode = "RGBA"
    scene.view_settings.look = "AgX - Medium High Contrast"
    bpy.ops.render.render(write_still=True)
    print("[production_review] rendered", path)


def render_city():
    clear_scene()
    add_ground()
    import_glb(CITY_DIR, "BuildingBrickV3", (-8.5, 4.5, 0.0))
    import_glb(CITY_DIR, "BuildingConcreteV3", (0.0, 6.0, 0.0))
    import_glb(CITY_DIR, "BuildingStuccoV3", (8.4, 4.8, 0.0))
    for name, location, angle in (
        ("SedanCleanV3", (-8.2, -4.2, 0.0), math.radians(-8.0)),
        ("PoliceCruiserV3", (-2.8, -4.1, 0.0), math.radians(5.0)),
        ("DeliveryVanV3", (3.1, -3.8, 0.0), math.radians(-4.0)),
        ("PickupV3", (8.6, -3.9, 0.0), math.radians(8.0)),
    ):
        import_glb(CITY_DIR, name, location, rotation=(0.0, 0.0, angle))
    import_glb(CITY_DIR, "StreetlightV3", (-12.0, -1.0, 0.0))
    import_glb(CITY_DIR, "BusStopV3", (11.5, 0.2, 0.0), rotation=(0.0, 0.0, math.pi))
    add_lighting(target=(0.0, 1.5, 4.5))
    setup_camera((25.0, -34.0, 18.0), (0.0, 1.5, 4.6), lens=52.0)
    render(os.path.join(OUT_DIR, "production_city_assets.png"))


def render_weapons():
    clear_scene()
    add_ground(size=(12.0, 8.0), color=(0.040, 0.043, 0.048))
    placements = (
        ("PistolV3", (-3.4, 1.5, 0.45), 3.2),
        ("SMGV3", (-0.8, 1.2, 0.55), 2.2),
        ("RifleV3", (2.2, 1.5, 0.55), 1.6),
        ("ShotgunV3", (-1.8, -1.4, 0.55), 1.6),
        ("CrossbowV3", (2.1, -1.5, 0.55), 1.6),
    )
    for name, location, scale in placements:
        import_glb(WEAPON_DIR, name, location, rotation=(0.0, 0.0, math.radians(8.0)), scale=scale)
    add_lighting(target=(0.0, 0.0, 0.5))
    setup_camera((6.8, -9.5, 8.3), (0.0, 0.0, 0.55), lens=55.0)
    render(os.path.join(OUT_DIR, "production_weapon_assets.png"))


render_city()
render_weapons()
print("[production_review] COMPLETE")
