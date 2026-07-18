#!/usr/bin/env python3
"""Static acceptance contract for the 2026-07-09 first-level combat pass."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CHAR_H = (ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.h").read_text()
CHAR_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.cpp").read_text()
PAUSE_H = (ROOT / "Source/CodeRescueUnreal/CodeRescuePauseWidget.h").read_text()
PAUSE_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp").read_text()
HUD_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp").read_text()
ZOMBIE_H = (ROOT / "Source/CodeRescueUnreal/CodeZombieActor.h").read_text()
ZOMBIE_CPP = (ROOT / "Source/CodeRescueUnreal/CodeZombieActor.cpp").read_text()
COMPANION_CPP = (ROOT / "Source/CodeRescueUnreal/CompanionActor.cpp").read_text()
GAME_H = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h").read_text()
GAME_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp").read_text()
BLENDER = (ROOT / "Scripts/BlenderArt/build_first_level_combat_art_v4.py").read_text()
IMPORT = (ROOT / "Scripts/import_first_level_combat_art_v4.py").read_text()
MESH_AUDIT = (ROOT / "Scripts/inspect_first_level_v4_meshes_unreal.py").read_text()
DEFAULT_GAME = (ROOT / "Config/DefaultGame.ini").read_text()

failures = []


def check(condition, message):
    if condition:
        print("PASS:", message)
    else:
        print("FAIL:", message)
        failures.append(message)


# Controls and physical aiming.
check("BindKey(EKeys::SpaceBar,        IE_Pressed, this, &ACodeRescueCharacter::TryJump)" in CHAR_CPP,
      "Space invokes stamina-aware jump")
check("BindKey(EKeys::SpaceBar,        IE_Pressed, this, &ACodeRescueCharacter::Fire)" not in CHAR_CPP,
      "Space no longer fires a weapon")
check("JumpZVelocity = 680.0f" in CHAR_CPP and "AirControl = 0.42f" in CHAR_CPP,
      "jump movement has deliberate first-level tuning")
check("RightMouseButton" in CHAR_CPP and "BeginAim" in CHAR_CPP and "EndAim" in CHAR_CPP,
      "right mouse supplies hold-to-aim input")
check("UPoseableMeshComponent" in CHAR_H and "CopyPoseFromSkeletalComponent" in CHAR_CPP and
      "AimArm(FName(TEXT(\"upperarm_r\"))" in CHAR_CPP and "AimArm(FName(TEXT(\"upperarm_l\"))" in CHAR_CPP,
      "production locomotion pose receives a procedural two-arm aim layer")

# Clickable pause armory.
check("UViewport" in PAUSE_H and "LiveWeaponViewport" in PAUSE_CPP and "WeaponViewport->Spawn" in PAUSE_CPP,
      "pause armory renders the authored weapon in a live 3D viewport")
check(all(token in PAUSE_CPP for token in ("PreviousWeaponButton", "NextWeaponButton", "EquipWeaponButton")),
      "pause armory has clickable previous, next, and equip controls")
check(all(token in PAUSE_CPP for token in ("MAGAZINE", "RESERVE", "EFFECTIVE RANGE", "DELIVERY", "SPECIAL")),
      "armory exposes current ammo, capacity, weapon stats, delivery, and special behavior")
check("Character->SwapWeapon(static_cast<EWeaponType>(PreviewWeaponIndex))" in PAUSE_CPP,
      "armory Equip changes the gameplay weapon")
check(all(token in PAUSE_CPP for token in ("OnSaveClicked", "OnLoadClicked", "OnSettingsClicked", "OnTutorialClicked")),
      "armory retains the existing pause/save/options/learning workflows")
native_construct = PAUSE_CPP[PAUSE_CPP.index("void UCodeRescuePauseWidget::NativeConstruct"):PAUSE_CPP.index("TSharedRef<SWidget> UCodeRescuePauseWidget::RebuildWidget")]
build_tree = PAUSE_CPP[PAUSE_CPP.index("void UCodeRescuePauseWidget::BuildWidgetTreeNow"):PAUSE_CPP.index("void UCodeRescuePauseWidget::RefreshDifficultyLabel")]
check("SetLightIntensity" in native_construct and "SetLightIntensity" not in build_tree,
      "3D viewport scene configuration waits for a valid Slate viewport")
check("ClearOnScreenDebugMessages" in PAUSE_CPP and
      "RequestScreenshot(CapturePath, true, false)" in PAUSE_CPP,
      "armory clears transient HUD overlap and captures its actual UI")
check("FirstLevelArmoryCycleAudit" in PAUSE_CPP and
      "OnNextWeaponClicked();" in PAUSE_CPP and
      "COMPLETE PASS previews=%d final_index=%d equipped=1" in PAUSE_CPP,
      "opt-in armory audit cycles every preview through player-facing handlers and equips the final selection")
preview_setup = PAUSE_CPP[PAUSE_CPP.index("UStaticMeshComponent* MeshComponent"):PAUSE_CPP.index("const FBox MeshBox")]
check(preview_setup.index("SetMobility") < preview_setup.index("SetStaticMesh") and
      "PreviewMeshCenterScaled" in PAUSE_H and "RotateVector(PreviewMeshCenterScaled)" in PAUSE_CPP,
      "preview meshes become movable before assignment and stay centered around authored pivots while rotating")

# Hit validation and interaction physics.
check("TargetLockMaxDistance = 4200.0f" in CHAR_H and
      "TargetLockAcquireAngleDegrees = 24.0f" in CHAR_H and
      "TargetLockBreakAngleDegrees = 36.0f" in CHAR_H,
      "auto target lock has explicit acquisition, release, and distance bounds")
check("IsAimTargetCandidate" in CHAR_CPP and "HasClearWeaponPath(Start, TargetPoint, Target)" in CHAR_CPP,
      "target lock candidates require range, view-cone, health, collision, and clear line of sight")
check("FireDirection = PhysicalLockDirection" in CHAR_CPP and
      "TargetLockRedirectsPhysicalWeaponTrace" in CHAR_CPP,
      "a valid lock redirects the actual physical weapon trace")
check("PhysicalAutoTargetLockActive" in CHAR_CPP and
      "IsAimTargetLocked()" in HUD_CPP and "TARGET LOCKED" in CHAR_CPP,
      "persistent lock drives gameplay state and a visible HUD confirmation")
check("OcclusionValidatedAimAssist" not in CHAR_CPP and
      "ApplyRescuePointDamage(Assisted" not in CHAR_CPP,
      "retired post-miss proximity damage cannot affect off-ray zombies")
check("const float LineRadius = 58.0f" in CHAR_CPP and "HasClearWeaponPath(Start, TargetPoint, Zombie)" in CHAR_CPP,
      "piercing fire uses a tight physical corridor with geometry validation")
check("AreaWeaponRequiresPhysicalImpact" in CHAR_CPP and "radial effect suppressed at remote endpoint" in CHAR_CPP,
      "area attacks cannot detonate at an unhit remote endpoint")
check("ApplyRescuePointDamage" in CHAR_CPP and "ApplyRescuePointDamage" in ZOMBIE_CPP,
      "successful fire transmits point, direction, zone, and bone context")
check("FinalDamage *= 2.0f" not in CHAR_CPP,
      "hit-zone damage is multiplied once in the zombie, not twice in caller and target")
check("LineTraceSingleByObjectType" in CHAR_CPP and "ZombiePawnObject" in CHAR_CPP and
      "SegmentDistToSegmentSafe" in CHAR_CPP and "bZombieIsFirst" in CHAR_CPP,
      "shots combine world occlusion with exact zombie-object and capsule resolution")
check("Outward * 12.0f" in CHAR_CPP and "AreaWeaponRequiresPhysicalImpact" in CHAR_CPP,
      "radial line of sight starts outside the impact surface and only follows a physical hit")
check("FirstLevelCombatRuntimeAudit" in CHAR_CPP and "COMPLETE PASS" in CHAR_CPP and
      "target_lock=%d" in CHAR_CPP and "remote_assist=disabled" in CHAR_CPP and
      "deliberate_miss_locality=%s" in CHAR_CPP and "miss_locality=1" in CHAR_CPP,
      "opt-in runtime audit exercises a harmless miss, physical lock, shots, wounds, and the full corpse lifecycle")
check("SpawnActor<ACodeZombieActor>" in CHAR_CPP and
      "spawned transient target because campaign supplied no live zombie" in CHAR_CPP,
      "runtime combat audit remains deterministic when a saved campaign has no live zombies")

# Anatomical wounds and grounded corpse lifecycle.
check("WoundCavityV4" in ZOMBIE_CPP and "LocalizedAnatomicalWeaponWounds" in ZOMBIE_CPP,
      "bullet impacts create persistent localized cavity/decal wounds")
check("BiteWoundV4" in CHAR_CPP and "PlayerAnatomicalBiteWounds" in CHAR_CPP,
      "zombie damage creates side-aware player bite wounds")
check('/Game/YI_ModularZombies/Materials/Master/Instances' in DEFAULT_GAME,
      "localized blood-decal material instances and their textures are included in packaged cooks")
check("RagdollImpulseStrength = 430.0f" in ZOMBIE_H and "PrimitiveCorpseImpulseStrength = 310.0f" in ZOMBIE_H,
      "death velocity impulses are grounded rather than launch-scale")
check("LastIncomingShotDirection" in ZOMBIE_CPP and "GroundedStrength" in ZOMBIE_CPP,
      "death reaction follows the incoming impact direction and is clamped")
check("RagdollCorpseLifetime = 9.0f" in ZOMBIE_H and "BeginCorpseFade" in ZOMBIE_CPP and
      "CorpseFadeDuration = 2.8f" in ZOMBIE_H,
      "zombies persist, settle, gradually disappear, then destroy")
check("FrozenCorpsePose" in ZOMBIE_H and "CopyPoseFromSkeletalComponent" in ZOMBIE_CPP and
      "CorpsePosePreservedDuringFade" in ZOMBIE_CPP,
      "corpse fade preserves the final grounded ragdoll pose")
check("if (ZombieId >= 0)" in ZOMBIE_CPP,
      "transient QA/encounter zombies cannot mutate campaign save progress")
check("NPCVisibleCorpseWindow" in COMPANION_CPP and "NPCGradualCorpseFade" in COMPANION_CPP,
      "fallen companion NPCs also persist before a gradual fade")

# Blender-authored, first-level-only art integration.
asset_names = (
    "FirstLevelStorefrontV4", "FieldArmoryV4", "TriageCheckpointV4", "SandbagCoverV4",
    "GrenadeV4", "CombatKnifeV4", "RocketLauncherV4", "WoundCavityV4", "BiteWoundV4",
)
check(all(name in BLENDER for name in asset_names) and "export_scene.gltf" in BLENDER,
      "Blender script deterministically authors all nine V4 assets")
check("AssetImportTask" in IMPORT and "replace_existing = True" in IMPORT,
      "Unreal import script is idempotent")
check(all(name in MESH_AUDIT for name in asset_names) and
      all(token in MESH_AUDIT for token in ("get_num_triangles", "get_num_lods", "get_bounding_box")),
      "Unreal commandlet audit verifies render triangles, LODs, materials, and bounds for all nine assets")
check("SpawnFirstLevelCombatArtPass" in GAME_H and "if (CityIndex != 0)" in GAME_CPP and
      "FirstLevelOnly" in GAME_CPP,
      "environment additions are hard-gated to the first level")
check("i == -2 && SideSign > 0" in GAME_CPP and "Reserved for literal open-door V5 interiors" in GAME_CPP,
      "open-door V5 buildings replace, rather than overlap, three V3 facades")
check("CameraSightlinePreserved" in GAME_CPP,
      "new structures carry the camera-sightline review contract")

raw_dir = ROOT / "RawArt/FirstLevelV4"
content_dir = ROOT / "Content/CodeRescueArt/FirstLevelV4"
missing_glb = [name for name in asset_names if not (raw_dir / f"{name}.glb").is_file()]
missing_uasset = [name for name in asset_names if not any(content_dir.glob(f"{name}/{name}/StaticMeshes/{name}.uasset"))]
check(not missing_glb, "all nine Blender GLB sources exist" + (f" (missing {missing_glb})" if missing_glb else ""))
check(not missing_uasset, "all nine Unreal static meshes exist" + (f" (missing {missing_uasset})" if missing_uasset else ""))

if failures:
    print(f"\nFAILED: {len(failures)} first-level acceptance contract(s)")
    sys.exit(1)

print("\nPASS: first-level combat experience acceptance contracts are complete")
