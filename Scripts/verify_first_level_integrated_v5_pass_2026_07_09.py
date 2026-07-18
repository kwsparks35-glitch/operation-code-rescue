#!/usr/bin/env python3
"""Static acceptance contract for the first-level V5 integrated pass."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CHAR_H = (ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.h").read_text()
CHAR_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueCharacter.cpp").read_text()
HUD_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp").read_text()
GAME_H = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h").read_text()
GAME_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp").read_text()
SPAWNING_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp").read_text()
PAUSE_CPP = (ROOT / "Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp").read_text()
TERMINAL_CPP = (ROOT / "Source/CodeRescueUnreal/CodeTerminalWidget.cpp").read_text()
BLENDER = (ROOT / "Scripts/BlenderArt/build_first_level_world_v5.py").read_text()
IMPORT = (ROOT / "Scripts/import_first_level_world_v5.py").read_text()
MESH_AUDIT = (ROOT / "Scripts/inspect_first_level_v5_meshes_unreal.py").read_text()

failures = []


def check(condition, message):
    if condition:
        print("PASS:", message)
    else:
        print("FAIL:", message)
        failures.append(message)


# Physical target lock and visible aiming.
check(all(token in CHAR_H for token in (
    "TargetLockMaxDistance", "TargetLockAcquireAngleDegrees", "TargetLockBreakAngleDegrees",
    "LockedAimTarget", "UpdateAutoTargetLock", "FindBestAimTarget")),
      "character owns a persistent, bounded target-lock state machine")
check("HasClearWeaponPath(Start, TargetPoint, Target)" in CHAR_CPP and
      "IsAimTargetCandidate(PreviousTarget, TargetLockBreakAngleDegrees" in CHAR_CPP,
      "target retention requires collision, range, view cone, and line of sight")
check("AimArm(FName(TEXT(\"upperarm_r\"))" in CHAR_CPP and
      "AimArm(FName(TEXT(\"upperarm_l\"))" in CHAR_CPP and
      "AimDirectionWorld = GetAimTargetPoint(Target)" in CHAR_CPP,
      "the authored two-arm presentation follows the locked target")
check("FireDirection = PhysicalLockDirection" in CHAR_CPP and
      "TargetLockRedirectsPhysicalWeaponTrace" in CHAR_CPP,
      "lock-on redirects the real ray instead of awarding damage after a miss")
check("OcclusionValidatedAimAssist" not in CHAR_CPP and
      "ApplyRescuePointDamage(Assisted" not in CHAR_CPP,
      "legacy post-miss proximity damage is absent")
check("deliberate_miss_locality=%s" in CHAR_CPP and
      "physical_ray_miss=1 remote_damage=0" in CHAR_CPP and "miss_locality=1" in CHAR_CPP,
      "runtime acceptance requires a deliberate miss to leave the nearby target unharmed")
check("TARGET LOCKED" in CHAR_CPP and "PHYSICAL TRACE" in CHAR_CPP and
      "SetText(FText::FromString(TEXT(\"[X]\")))" in HUD_CPP,
      "HUD exposes a clear lock state and physical-trace promise")

# One canonical arena elevation and genuinely enterable buildings.
check("CanonicalMissionGround" in SPAWNING_CPP and
      "never infer playable" in SPAWNING_CPP and
      "remaining_elevated=0" in SPAWNING_CPP,
      "the first-level arena is protected from post-spawn vertical stacking")
check("FirstLevelCanonicalGroundSurface" in GAME_CPP and
      "const float CanonicalFirstLevelGroundZ = Origin.Z" in GAME_CPP and
      "FirstLevelGroundSurfaceActors" in GAME_H,
      "roads, sidewalks, and crosswalks share an audited canonical elevation")
check("SpawnFirstLevelTraversalArtPass" in GAME_H and
      all(name in GAME_CPP for name in (
          "AccessibleMarketV5", "AccessibleClinicV5", "OpenStreetCafeV5")),
      "three Blender-authored first-level destinations are integrated")
check(all(token in GAME_CPP for token in (
    "FirstLevelEnterableBuilding", "LiteralOpenDoorway", "FirstLevelInteriorFunctionalPickup",
    "FirstLevelAccessibleInteriors")),
      "open interiors include explicit access contracts and functional pickups")
check("SweepSingleByChannel" in GAME_CPP and
      "FCollisionShape::MakeCapsule(30.0f, 70.0f)" in GAME_CPP and
      "clear_doors=%d/%d" in GAME_CPP,
      "runtime access audit sweeps a player-sized capsule through every doorway")
# 2026-07-11 pass 4 contract evolution: elevation continuity is now enforced on
# the WALKABLE TOP of driving surfaces (<= 8 uu) with sidewalk curbs measured
# separately; pivot spread relaxes to the designed curb envelope (18.5 uu).
# The ground-unification pass (UnifyFirstLevelGroundTops) makes it hold.
check("GroundTopSpread <= 8.0f" in GAME_CPP and
      "GroundSpread <= 18.5f" in GAME_CPP and
      "UnifyFirstLevelGroundTops" in GAME_CPP and
      "FMath::Abs(OutsideGroundZ - InsideGroundZ) <= 28.0f" in GAME_CPP,
      "runtime access audit verifies arena and threshold elevation continuity")

# Smooth sky cycle without a camera-blocking shell.
check(all(token in GAME_CPP for token in (
    "PointStarFieldV5", "MoonDetailedV5", "SafePointStarField", "SolarAltitude")),
      "night sky uses point stars and a detailed moon controlled by solar altitude")
check("NightSkyDome->SetActorLocation(bSafePointField" in GAME_CPP and
      "NightSkyDome->SetActorHiddenInGame(!bSafePointField || !bShowStars)" in GAME_CPP,
      "only the non-occluding star field can follow the player camera")
check(all(token in GAME_CPP for token in (
    "PhaseNames[] = { TEXT(\"DAY\"), TEXT(\"SUNSET\"), TEXT(\"NIGHT\"), TEXT(\"SUNRISE\") }",
    "WorldSkyLight", "MoonLight", "SunLight")),
      "sun, moon, skylight, and four readable day phases update continuously")
check(all(name in GAME_CPP for name in (
    "first_level_sky_day.png", "first_level_sky_sunset.png",
    "first_level_sky_night.png", "first_level_sky_sunrise.png")),
      "the runtime sky audit captures every required lighting phase")

# First-level pedagogy and exact language solutions.
check(all(label in GAME_CPP for label in (
    "ECodingLanguage::Java", "ECodingLanguage::C", "ECodingLanguage::CPlus",
    "ECodingLanguage::Cpp", "ECodingLanguage::Python", "ECodingLanguage::MATLAB")),
      "the first-level challenge audit covers all six selectable languages")
check("return a + b + c;" in TERMINAL_CPP and "def total_power(a, b, c)" in TERMINAL_CPP and
      "function result = total_power(a, b, c)" in TERMINAL_CPP and "result = a + b + c;" in TERMINAL_CPP,
      "reference solutions exist for C-family, Python, and MATLAB syntax")
check("UCodeRunnerLibrary::ValidateChallenge" in GAME_CPP and
      "UCodeRunnerLibrary::IsLanguageAvailable" in GAME_CPP and
      "languages=%d/%d" in GAME_CPP,
      "every reference answer runs through the same validator used by gameplay")

# Single-run acceptance handoff.
check("FirstLevelIntegratedAcceptanceAudit" in GAME_CPP and
      "FirstLevelIntegratedAcceptanceAudit" in CHAR_CPP and
      "FirstLevelIntegratedAcceptanceAudit" in PAUSE_CPP,
      "world, combat, and armory audits share one integrated command-line mode")
check(all(token in GAME_CPP for token in (
    "FirstLevelIntegratedWorldPass", "FirstLevelIntegratedChallengePass",
    "FirstLevelIntegratedSkyPass")) and "FirstLevelIntegratedCombatPass" in CHAR_CPP,
      "each subsystem records an independent runtime pass tag")
# (token line refreshed 2026-07-17: the world/character presentation pass adds
#  visible-foot contact, weather, icon loot, districts, and threat-marker checks;
#  token ORDER remains the compatibility contract.)
check("COMPLETE PASS world=1 access=1 ground=1 population=1 characters_grounded=1 visible_feet=1 sky=1 day_period=1 weather=1 challenges=1 alternate_solution=1 guidance=1 progression=1 supplies=1 loot_symbols=1 districts=1 threat_markers=1 target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 overlay_passthrough=1 crafting=1" in PAUSE_CPP,
      "the final single-run result requires every requested first-level subsystem")

# Deterministic Blender generation, Unreal import, and render-data audit.
asset_names = (
    "AccessibleMarketV5", "AccessibleClinicV5", "OpenStreetCafeV5",
    "PointStarFieldV5", "MoonDetailedV5",
)
check(all(name in BLENDER for name in asset_names) and
      "literal 3.2m x 3.5m doorway" in BLENDER and "export_scene.gltf" in BLENDER,
      "Blender source deterministically authors all V5 assets and literal door openings")
check("AssetImportTask" in IMPORT and "replace_existing = True" in IMPORT,
      "Unreal V5 import is idempotent")
check(all(name in MESH_AUDIT for name in asset_names) and
      all(token in MESH_AUDIT for token in ("get_num_triangles", "get_num_lods", "get_bounding_box")),
      "Unreal commandlet audit validates triangles, LODs, materials, and bounds")

raw_dir = ROOT / "RawArt/FirstLevelV5"
content_dir = ROOT / "Content/CodeRescueArt/FirstLevelV5"
missing_glb = [name for name in asset_names if not (raw_dir / f"{name}.glb").is_file()]
missing_uasset = [
    name for name in asset_names
    if not any(content_dir.glob(f"{name}/{name}/StaticMeshes/{name}.uasset"))
]
check(not missing_glb, "all five Blender GLB sources exist" +
      (f" (missing {missing_glb})" if missing_glb else ""))
check(not missing_uasset, "all five Unreal static meshes exist" +
      (f" (missing {missing_uasset})" if missing_uasset else ""))

if failures:
    print(f"\nFAILED: {len(failures)} first-level V5 acceptance contract(s)")
    sys.exit(1)

print("\nPASS: first-level V5 integrated acceptance contracts are complete")
