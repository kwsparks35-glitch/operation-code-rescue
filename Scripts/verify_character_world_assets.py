"""
Verify the character and world-development assets used by the runtime generator.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_character_world_assets.py" \
        -unattended -NoSound -NullRHI -NoLoadStartupPackages
"""

import unreal


REQUIRED_ASSETS = {
    "player_manny_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny",
    "survivor_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn",
    "companion_quinn_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn",
    "ue5_manny_simple_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny_Simple",
    "ue5_quinn_simple_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
    "ue4_legacy_mannequin_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequin_UE4/Meshes/SK_Mannequin",
    "friendly_npc_manny_mesh": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny",
    "friendly_npc_quinn_anim": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn",
    "friendly_npc_manny_anim": "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny",
    "mannequin_compat_foot_ik": "/Game/Characters/Mannequins/Rigs/CR_Mannequin_BasicFootIK",
    "mannequin_compat_body_rig": "/Game/Characters/Mannequins/Rigs/CR_Mannequin_Body",
    "mannequin_compat_procedural_rig": "/Game/Characters/Mannequins/Rigs/CR_Mannequin_Procedural",
    "zombie_dog_mesh": "/Game/DogZombie/Meshes/SK_DogZombie",
    "zombie_urban_mesh": "/Game/UrbanZombie4/Mesh/SK_UrbanZombie4",
    "zombie_business_mesh": "/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01",
    "zombie_bloated_mesh": "/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01",
    "zombie_nurse_mesh": "/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit",
    "zombie_base_mesh": "/Game/Zombie/BaseMesh/SK_Zombie",
    "city_building_01": "/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building01",
    "city_building_02": "/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building02",
    "city_building_03": "/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building03",
    "city_building_04": "/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building04",
    "city_building_05": "/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building05",
    "city_bridge_01": "/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_001",
    "city_bridge_10": "/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_010",
    "relief_camp_chair": "/Game/StarterContent/Props/SM_Chair",
    "relief_camp_table": "/Game/StarterContent/Props/SM_TableRound",
    "relief_camp_shelf": "/Game/StarterContent/Props/SM_Shelf",
    "mission_diorama_door_frame": "/Game/StarterContent/Props/SM_DoorFrame",
    "mission_diorama_pillar_frame": "/Game/StarterContent/Props/SM_PillarFrame",
    "mission_diorama_classroom_wall": "/Game/StarterContent/Architecture/Wall_Window_400x300",
    "mission_diorama_observation_glass": "/Game/StarterContent/Props/SM_GlassWindow",
    "world_composition_lamp": "/Game/StarterContent/Props/SM_Lamp_Ceiling",
    "enterable_safehouse_floor": "/Game/StarterContent/Materials/M_Brick_Hewn_Stone",
    "enterable_safehouse_window_frame": "/Game/StarterContent/Props/SM_WindowFrame",
    "character_identity_floor": "/Game/StarterContent/Materials/M_Concrete_Poured",
    "mission_diorama_wood_floor": "/Game/StarterContent/Materials/M_Wood_Floor_Walnut_Worn",
    "mission_diorama_debug_floor": "/Game/StarterContent/Materials/M_Tech_Checker_Dot",
    "mission_diorama_quarantine_floor": "/Game/StarterContent/Materials/M_Metal_Rust",
    "landscape_rock_mesh": "/Game/StarterContent/Props/SM_Rock",
    "landscape_bush_mesh": "/Game/StarterContent/Props/SM_Bush",
    "landscape_terrain_material": "/Game/StarterContent/Materials/M_Ground_Moss",
    "landscape_road_material": "/Game/StarterContent/Materials/M_Concrete_Grime",
    "safe_hub_plaza_material": "/Game/StarterContent/Materials/M_Concrete_Panels",
    "objective_pad_material": "/Game/StarterContent/Materials/M_Tech_Panel",
    "zombie_variant_table": "/Game/CodeRescueAssets/DT_ZombieVariants",
}

def expected_asset_name(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    return leaf.rsplit(".", 1)[-1]


def verify_asset(label, asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log_error(f"[cr-assets] MISSING {label}: {asset_path}")
        return False

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        unreal.log_error(f"[cr-assets] UNLOADABLE {label}: {asset_path}")
        return False

    expected_name = expected_asset_name(asset_path)
    actual_name = asset.get_name()
    if actual_name != expected_name:
        unreal.log_error(
            f"[cr-assets] OBJECT NAME MISMATCH {label}: expected {expected_name}, loaded {actual_name}"
        )
        return False

    unreal.log(f"[cr-assets] OK      {label}: {asset.get_path_name()}")
    return True


def main():
    unreal.log("[cr-assets] === character/world asset verification START ===")
    missing = []
    for label, asset_path in REQUIRED_ASSETS.items():
        if not verify_asset(label, asset_path):
            missing.append(label)

    if missing:
        raise RuntimeError(f"Missing required Code Rescue assets: {', '.join(missing)}")

    table = unreal.EditorAssetLibrary.load_asset(REQUIRED_ASSETS["zombie_variant_table"])
    if table:
        try:
            rows = unreal.DataTableFunctionLibrary.get_data_table_row_names(table)
            unreal.log(f"[cr-assets] zombie variant table rows: {', '.join(str(row) for row in rows)}")
        except Exception as exc:
            unreal.log_warning(f"[cr-assets] could not enumerate DT_ZombieVariants rows: {exc}")

    unreal.log("[cr-assets] === character/world asset verification PASSED ===")


main()
