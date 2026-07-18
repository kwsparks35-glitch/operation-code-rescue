"""
Actively verifies camera perspective switching and the character roster.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_camera_perspectives_and_character_roster.py" \
        -unattended -NoSound -NullRHI
"""

import unreal


CAMERA_EXPECTATIONS = [
    (0, "First-Person", True, False, 320.0),
    (1, "Third-Person", False, True, 320.0),
    (2, "Tactical Third-Person", False, True, 650.0),
    (3, "Top-Down", False, True, 1150.0),
    (4, "Isometric", False, True, 1250.0),
    (5, "Side-View 2.5D", False, True, 920.0),
]

ROSTER_CLASSES = {
    "player": "/Script/CodeRescueUnreal.CodeRescueCharacter",
    "survivor": "/Script/CodeRescueUnreal.SurvivorActor",
    "friendly_npc": "/Script/CodeRescueUnreal.FriendlyNPCActor",
    "companion": "/Script/CodeRescueUnreal.CompanionActor",
    "zombie": "/Script/CodeRescueUnreal.CodeZombieActor",
    "boss_zombie": "/Script/CodeRescueUnreal.BossZombieActor",
}

ROSTER_INTERACTION_METHODS = {
    "player": ("select_camera_perspective", "get_camera_perspective_index"),
    "survivor": ("rescue",),
    "friendly_npc": ("interact",),
    "companion": ("take_companion_damage",),
    "zombie": ("apply_rescue_damage", "initialize_from_variant"),
    "boss_zombie": ("apply_rescue_damage",),
}

ROSTER_ASSETS = {
    "player_manny_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny",
    "survivor_quinn_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn",
    "companion_quinn_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn",
    "manny_anim": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny",
    "quinn_anim": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn",
    "ue4_legacy_mannequin": "/Game/YI_ModularZombies/Demo/Characters/Mannequin_UE4/Meshes/SK_Mannequin",
    "dog_zombie": "/Game/DogZombie/Meshes/SK_DogZombie",
    "urban_zombie": "/Game/UrbanZombie4/Mesh/SK_UrbanZombie4",
    "business_zombie": "/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01",
    "bloated_zombie": "/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01",
    "nurse_zombie": "/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit",
    "base_zombie": "/Game/Zombie/BaseMesh/SK_Zombie",
}


def fail(message):
    unreal.log_error(f"[cr-perspective-roster] {message}")
    raise RuntimeError(message)


def load_class(label, path):
    cls = unreal.load_class(None, path)
    if not cls:
        fail(f"missing class {label}: {path}")
    unreal.log(f"[cr-perspective-roster] OK class {label}: {path}")
    return cls


def get_editor_world():
    subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if subsystem:
        world = subsystem.get_editor_world()
        if world:
            return world
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            return world
    except Exception:
        pass
    fail("could not resolve an editor world for active spawn checks")


def get_component(actor, component_class, expected_name):
    for component in actor.get_components_by_class(component_class):
        if component.get_name() == expected_name:
            return component
    fail(f"{actor.get_name()} is missing component {expected_name}")


def destroy_actor(actor):
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem:
        subsystem.destroy_actor(actor)
    else:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def component_is_active(component):
    if hasattr(component, "is_active"):
        return bool(component.is_active())
    return bool(component.get_editor_property("b_is_active"))


def expected_asset_name(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    return leaf.rsplit(".", 1)[-1]


def verify_asset(label, asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log_error(f"[cr-perspective-roster] MISSING asset {label}: {asset_path}")
        return False
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        unreal.log_error(f"[cr-perspective-roster] UNLOADABLE asset {label}: {asset_path}")
        return False
    expected_name = expected_asset_name(asset_path)
    actual_name = asset.get_name()
    if actual_name != expected_name:
        unreal.log_error(
            f"[cr-perspective-roster] asset object mismatch {label}: expected {expected_name}, loaded {actual_name}"
        )
        return False
    unreal.log(f"[cr-perspective-roster] OK asset {label}: {asset.get_path_name()}")
    return True


def verify_assets():
    missing = []
    for label, path in ROSTER_ASSETS.items():
        if not verify_asset(label, path):
            missing.append(label)
    if missing:
        fail("missing roster assets: " + ", ".join(missing))


def verify_roster_classes():
    classes = {label: load_class(label, path) for label, path in ROSTER_CLASSES.items()}
    world = get_editor_world()
    spawned = []
    try:
        for index, (label, cls) in enumerate(classes.items()):
            location = unreal.Vector(float(index * 180), 0.0, 180.0)
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, location, unreal.Rotator(0.0, 0.0, 0.0))
            if not actor:
                fail(f"could not spawn roster actor {label}")
            actor.set_actor_label(f"CR_Verify_{label}")
            spawned.append(actor)
            for method_name in ROSTER_INTERACTION_METHODS.get(label, ()):
                if not hasattr(actor, method_name):
                    fail(f"{label} is missing interaction method {method_name}")
            skeletal_components = actor.get_components_by_class(unreal.SkeletalMeshComponent)
            if not skeletal_components:
                fail(f"{label} is missing a skeletal mesh component for visual identity")
            unreal.log(f"[cr-perspective-roster] OK spawn {label}: {actor.get_name()}")
    finally:
        for actor in spawned:
            try:
                destroy_actor(actor)
            except Exception:
                pass


def verify_camera_modes(repeat_count=3):
    character_class = load_class("camera_test_player", ROSTER_CLASSES["player"])
    world = get_editor_world()
    character = unreal.EditorLevelLibrary.spawn_actor_from_class(
        character_class,
        unreal.Vector(0.0, 0.0, 220.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    if not character:
        fail("could not spawn player for camera perspective verification")

    try:
        first_camera = get_component(character, unreal.CameraComponent, "FirstPersonCamera")
        third_camera = get_component(character, unreal.CameraComponent, "ThirdPersonCamera")
        boom = get_component(character, unreal.SpringArmComponent, "CameraBoom")

        for pass_index in range(repeat_count):
            unreal.log(f"[cr-perspective-roster] camera cycle pass {pass_index + 1}/{repeat_count}")
            for index, expected_label, expected_first_active, expected_third_active, expected_arm in CAMERA_EXPECTATIONS:
                character.select_camera_perspective(index)
                actual_index = character.get_camera_perspective_index()
                actual_label = character.get_camera_perspective_label()
                if actual_index != index:
                    fail(f"perspective index mismatch: expected {index}, got {actual_index}")
                if actual_label != expected_label:
                    fail(f"perspective label mismatch: expected {expected_label}, got {actual_label}")
                if component_is_active(first_camera) != expected_first_active:
                    fail(f"{expected_label}: first-person camera active state mismatch")
                if component_is_active(third_camera) != expected_third_active:
                    fail(f"{expected_label}: third-person camera active state mismatch")
                arm = float(boom.get_editor_property("target_arm_length"))
                if abs(arm - expected_arm) > 0.5:
                    fail(f"{expected_label}: expected arm {expected_arm}, got {arm}")
                unreal.log(f"[cr-perspective-roster] OK camera {index}: {actual_label} arm={arm:.1f}")
    finally:
        destroy_actor(character)


def main():
    unreal.log("[cr-perspective-roster] === camera perspective + character roster verification START ===")
    verify_assets()
    verify_roster_classes()
    verify_camera_modes()
    unreal.log("[cr-perspective-roster] === camera perspective + character roster verification PASSED ===")


main()
