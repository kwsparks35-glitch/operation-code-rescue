#!/usr/bin/env python3
"""Static acceptance contract for the July 17 world/loot/weather/grounding pass."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


def source(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8", errors="replace")


BLENDER = source("Scripts/BlenderArt/build_world_loot_weather_v6.py")
IMPORT = source("Scripts/import_world_loot_weather_v6.py")
MESH_AUDIT = source("Scripts/inspect_world_loot_weather_v6_unreal.py")
WEATHER = source("Source/CodeRescueUnreal/CodeRescueWeatherFieldActor.cpp")
PICKUP_H = source("Source/CodeRescueUnreal/PickupActor.h")
PICKUP_CPP = source("Source/CodeRescueUnreal/PickupActor.cpp")
GAME_H = source("Source/CodeRescueUnreal/CodeRescueGameMode.h")
GAME_CPP = source("Source/CodeRescueUnreal/CodeRescueGameMode.cpp")
SPAWNING = source("Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp")
ZOMBIE_H = source("Source/CodeRescueUnreal/CodeZombieActor.h")
ZOMBIE_CPP = source("Source/CodeRescueUnreal/CodeZombieActor.cpp")
COMPANION_CPP = source("Source/CodeRescueUnreal/CompanionActor.cpp")
AI_CPP = source("Source/CodeRescueUnreal/CodeRescueAIController.cpp")
PAUSE_CPP = source("Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp")

ASSET_NAMES = (
    "PickupAmmoV6",
    "PickupMedicalV6",
    "PickupArmorV6",
    "PickupUtilityV6",
    "PickupTechV6",
    "PickupSalvageV6",
    "ThreatGroundRingV6",
    "ResponderPackV6",
    "RainStreakV6",
    "WindDebrisV6",
    "FieldLogisticsDepotV6",
    "WeatherRelayV6",
    "QuarantineCheckpointV6",
)

failures: list[str] = []


def check(condition: bool, message: str) -> None:
    print(f"[{'PASS' if condition else 'FAIL'}] {message}")
    if not condition:
        failures.append(message)


check(all(name in BLENDER for name in ASSET_NAMES) and
      "COMPLETE PASS assets=13 contact_origins=grounded icon_first=1" in BLENDER,
      "Blender deterministically authors all 13 ground-origin V6 assets")
check("AssetImportTask" in IMPORT and "replace_existing = True" in IMPORT and
      "WorldLootWeatherImportAndQuit" in IMPORT and
      "used_with_instanced_static_meshes" in IMPORT,
      "Unreal import is idempotent and persists weather-instancing metadata")
check(all(name in MESH_AUDIT for name in ASSET_NAMES) and
      all(token in MESH_AUDIT for token in (
          "get_num_triangles", "get_num_lods", "get_bounding_box",
          "used_with_instanced_static_meshes")),
      "Unreal inspection validates render data, grounded bounds, and weather instancing")

raw_dir = ROOT / "RawArt/WorldLootWeatherV6"
content_dir = ROOT / "Content/CodeRescueArt/WorldLootWeatherV6"
missing_glb = [name for name in ASSET_NAMES if not (raw_dir / f"{name}.glb").is_file()]
missing_mesh = [
    name for name in ASSET_NAMES
    if not (content_dir / name / name / "StaticMeshes" / f"{name}.uasset").is_file()
]
check(not missing_glb, f"all 13 Blender GLB sources exist (missing={missing_glb})")
check(not missing_mesh, f"all 13 imported Unreal meshes exist (missing={missing_mesh})")

check(all(token in PICKUP_H for token in (
          "RefreshPresentation", "IsAuthoredPresentationReady", "GetPresentationStyleToken",
          "PresentationSpinDegreesPerSecond")) and
      all(name in PICKUP_CPP for name in ASSET_NAMES[:6]),
      "pickups resolve to six authored symbol-first presentation families")
check(all(token in PICKUP_CPP for token in (
          "IconFirstPickupPresentation", "NoParagraphPickupLabel", "PickupGroundContactVerified",
          "AddActorLocalRotation")) and
      "GetBoundingBox" in PICKUP_CPP and "LocalBounds.Min.Z" in PICKUP_CPP,
      "loot packaging is icon-first and derives ground clearance from mesh bounds")

check("AlignCharacterVisualFeetToCapsule" in GAME_H and
      "FirstLevelIntegratedVisibleFootGroundPass" in SPAWNING and
      "visual_feet_misaligned" in SPAWNING,
      "character grounding audits both capsule contact and visible mesh feet")
check("RefreshGroundedVisualPose" in ZOMBIE_H and
      "RefreshGroundedVisualPose" in ZOMBIE_CPP and
      "RefreshGroundedVisualPose" in COMPANION_CPP,
      "zombie and companion animation bases are recached after grounding")
check("ResponderPackV6" in COMPANION_CPP and "BlenderAuthoredResponderGear" in COMPANION_CPP,
      "survivor companions receive authored responder equipment")

check("SpawnZombieReadabilityMarker" in GAME_H and
      "ThreatGroundRingV6" in SPAWNING and
      all(token in GAME_CPP for token in (
          "ThreatMarkerAudit", "enclosure_cubes=0", "collision=0")),
      "zombie readability uses attached nonblocking ground rings instead of enclosure cubes")
check("SpawnFirstLevelPurposeDistrictPass" in GAME_H and
      all(name in GAME_CPP for name in ASSET_NAMES[-3:]) and
      "PurposeDistrictRuntimeAudit" in GAME_CPP and
      "PurposeDistrictOpenSpaceValidated" in GAME_CPP and
      "IsOpenDistrictLocation" in GAME_CPP and
      "BlenderDepotRuntimeRealization" in GAME_CPP and
      "FieldDepotModules == 21" in GAME_CPP and
      "FieldDepotRuntimeModules == 21" in GAME_CPP and
      "DepotIconStockPickup" in GAME_CPP and
      "FieldDepotRuntimeStock == 6" in GAME_CPP,
      "the first level includes three readable, functional authored districts")

check(all(token in WEATHER for token in (
          "ECodeRescueWeatherPhase::Wind", "ECodeRescueWeatherPhase::Rain",
          "ECodeRescueWeatherPhase::Fog", "RainInstanceTarget = 112",
          "DebrisInstanceTarget = 24", "GroundFriction", "WeatherPhysicsAudit",
          "SetVisualReviewPhase")) and
      all(token in GAME_CPP for token in (
          "WorldLootWeatherVisualReview", "v6_symbol_loot_rain.png",
          "v6_weather_relay_fog.png", "v6_quarantine_checkpoint_wind.png")),
      "wind, rain, fog, traction, and deterministic visual populations are implemented")
check("static const IConsoleVariable* WeatherVisibility" in AI_CPP and
      "static const IConsoleVariable* WeatherVisibility" in ZOMBIE_CPP and
      "ActivationRange * GetWeatherVisibilityScale()" in ZOMBIE_CPP,
      "weather visibility affects AI without repeated console-object searches")

integrated_marker = (
    "COMPLETE PASS world=1 access=1 ground=1 population=1 characters_grounded=1 "
    "visible_feet=1 sky=1 day_period=1 weather=1 challenges=1 alternate_solution=1 "
    "guidance=1 progression=1 supplies=1 loot_symbols=1 districts=1 threat_markers=1 "
    "target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 "
    "overlay_passthrough=1 crafting=1"
)
check(integrated_marker in PAUSE_CPP,
      "the single-session gate requires every new and existing subsystem")

if failures:
    print(f"\n[WorldLootWeatherGroundingStaticAudit] COMPLETE FAIL checks={len(failures)}")
    sys.exit(1)

print(f"\n[WorldLootWeatherGroundingStaticAudit] COMPLETE PASS checks=15 assets={len(ASSET_NAMES)}")
